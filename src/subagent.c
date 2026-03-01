#include "seaclaw/subagent.h"
#include "seaclaw/core/string.h"
#include <stdlib.h>
#include <string.h>
#include <time.h>

#if !defined(SC_IS_TEST) || SC_IS_TEST == 0
#include <pthread.h>
#endif

#define SC_MAX_TASKS 64
#define SC_DEFAULT_MAX_CONCURRENT 4

typedef struct sc_task_state {
    sc_task_status_t status;
    uint64_t task_id;
    char *label;
    char *task;
    char *result;
    char *error_msg;
    int64_t started_at;
    int64_t completed_at;
#if !defined(SC_IS_TEST) || SC_IS_TEST == 0
    pthread_t thread;
#endif
    bool thread_valid;
} sc_task_state_t;

struct sc_subagent_manager {
    sc_allocator_t *alloc;
    sc_task_state_t tasks[SC_MAX_TASKS];
    bool task_used[SC_MAX_TASKS];
    uint64_t next_id;
    uint32_t max_concurrent;
#if !defined(SC_IS_TEST) || SC_IS_TEST == 0
    pthread_mutex_t mutex;
#endif
};

#if !defined(SC_IS_TEST) || SC_IS_TEST == 0
typedef struct sc_thread_ctx {
    sc_subagent_manager_t *mgr;
    uint32_t slot;
} sc_thread_ctx_t;

static void *subagent_thread_fn(void *arg) {
    sc_thread_ctx_t *ctx = (sc_thread_ctx_t *)arg;
    sc_subagent_manager_t *mgr = ctx->mgr;
    uint32_t slot = ctx->slot;
    sc_allocator_t *alloc = mgr->alloc;
    sc_task_state_t *ts = &mgr->tasks[slot];

    /* For now: complete immediately with task text as result (Layer 4 adds agent loop) */
    char *result_str = sc_strndup(alloc, ts->task, ts->task ? strlen(ts->task) : 0);
    if (!result_str)
        result_str = sc_strndup(alloc, "(no result)", 11);

    pthread_mutex_lock(&mgr->mutex);
    ts->status = SC_TASK_COMPLETED;
    ts->result = result_str;
    ts->completed_at = (int64_t)time(NULL);
    ts->thread_valid = false;
    pthread_mutex_unlock(&mgr->mutex);

    alloc->free(alloc->ctx, ctx, sizeof(sc_thread_ctx_t));
    return NULL;
}
#endif

static uint32_t get_max_concurrent(const sc_config_t *cfg) {
    (void)cfg;
    /* Config does not yet have subagent max_concurrent; use default */
    return SC_DEFAULT_MAX_CONCURRENT;
}

static size_t running_count_locked(sc_subagent_manager_t *mgr) {
    size_t count = 0;
    for (uint32_t i = 0; i < SC_MAX_TASKS; i++) {
        if (mgr->task_used[i] && mgr->tasks[i].status == SC_TASK_RUNNING)
            count++;
    }
    return count;
}

static sc_task_state_t *find_task_by_id(sc_subagent_manager_t *mgr, uint64_t task_id) {
    for (uint32_t i = 0; i < SC_MAX_TASKS; i++) {
        if (mgr->task_used[i] && mgr->tasks[i].task_id == task_id)
            return &mgr->tasks[i];
    }
    return NULL;
}

sc_subagent_manager_t *sc_subagent_create(sc_allocator_t *alloc, const sc_config_t *cfg) {
    if (!alloc)
        return NULL;

    sc_subagent_manager_t *mgr = (sc_subagent_manager_t *)alloc->alloc(
        alloc->ctx, sizeof(sc_subagent_manager_t));
    if (!mgr)
        return NULL;

    memset(mgr, 0, sizeof(*mgr));
    mgr->alloc = alloc;
    mgr->next_id = 1;
    mgr->max_concurrent = get_max_concurrent(cfg);

#if !defined(SC_IS_TEST) || SC_IS_TEST == 0
    if (pthread_mutex_init(&mgr->mutex, NULL) != 0) {
        alloc->free(alloc->ctx, mgr, sizeof(sc_subagent_manager_t));
        return NULL;
    }
#endif

    return mgr;
}

void sc_subagent_destroy(sc_allocator_t *alloc, sc_subagent_manager_t *mgr) {
    if (!alloc || !mgr)
        return;

#if !defined(SC_IS_TEST) || SC_IS_TEST == 0
    pthread_mutex_lock(&mgr->mutex);
#endif

    for (uint32_t i = 0; i < SC_MAX_TASKS; i++) {
        if (!mgr->task_used[i])
            continue;

        sc_task_state_t *ts = &mgr->tasks[i];

#if !defined(SC_IS_TEST) || SC_IS_TEST == 0
        if (ts->thread_valid) {
            pthread_t th = ts->thread;
            ts->thread_valid = false;
            pthread_mutex_unlock(&mgr->mutex);
            pthread_join(th, NULL);
            pthread_mutex_lock(&mgr->mutex);
        }
#endif

        if (ts->label)
            alloc->free(alloc->ctx, ts->label, strlen(ts->label) + 1);
        if (ts->task)
            alloc->free(alloc->ctx, ts->task, strlen(ts->task) + 1);
        if (ts->result)
            alloc->free(alloc->ctx, ts->result, strlen(ts->result) + 1);
        if (ts->error_msg)
            alloc->free(alloc->ctx, ts->error_msg, strlen(ts->error_msg) + 1);

        mgr->task_used[i] = false;
    }

#if !defined(SC_IS_TEST) || SC_IS_TEST == 0
    pthread_mutex_unlock(&mgr->mutex);
    pthread_mutex_destroy(&mgr->mutex);
#endif

    alloc->free(alloc->ctx, mgr, sizeof(sc_subagent_manager_t));
}

sc_error_t sc_subagent_spawn(sc_subagent_manager_t *mgr,
    const char *task, size_t task_len,
    const char *label, const char *channel, const char *chat_id,
    uint64_t *out_task_id) {
    (void)channel;
    (void)chat_id;

    if (!mgr || !out_task_id)
        return SC_ERR_INVALID_ARGUMENT;

    sc_allocator_t *alloc = mgr->alloc;

#if !defined(SC_IS_TEST) || SC_IS_TEST == 0
    pthread_mutex_lock(&mgr->mutex);
#endif

    if (running_count_locked(mgr) >= mgr->max_concurrent) {
#if !defined(SC_IS_TEST) || SC_IS_TEST == 0
        pthread_mutex_unlock(&mgr->mutex);
#endif
        return SC_ERR_SUBAGENT_TOO_MANY;
    }

    uint32_t slot = SC_MAX_TASKS;
    for (uint32_t i = 0; i < SC_MAX_TASKS; i++) {
        if (!mgr->task_used[i]) {
            slot = i;
            break;
        }
    }
    if (slot >= SC_MAX_TASKS) {
#if !defined(SC_IS_TEST) || SC_IS_TEST == 0
        pthread_mutex_unlock(&mgr->mutex);
#endif
        return SC_ERR_OUT_OF_MEMORY;
    }

    uint64_t task_id = mgr->next_id++;
    sc_task_state_t *ts = &mgr->tasks[slot];

    char *task_copy = sc_strndup(alloc, task, task_len);
    if (!task_copy) {
#if !defined(SC_IS_TEST) || SC_IS_TEST == 0
        pthread_mutex_unlock(&mgr->mutex);
#endif
        return SC_ERR_OUT_OF_MEMORY;
    }

    char *label_copy = label ? sc_strndup(alloc, label, strlen(label)) : NULL;
    /* label_copy can be NULL - we allow it */

    ts->task_id = task_id;
    ts->status = SC_TASK_RUNNING;
    ts->label = label_copy;
    ts->task = task_copy;
    ts->result = NULL;
    ts->error_msg = NULL;
    ts->started_at = (int64_t)time(NULL);
    ts->completed_at = 0;
    ts->thread_valid = false;
    mgr->task_used[slot] = true;

#if defined(SC_IS_TEST) && SC_IS_TEST == 1
    /* Test mode: synchronous, no thread. Set status = COMPLETED, result = "completed: <task>" */
    ts->result = sc_sprintf(alloc, "completed: %s", task_copy);
    if (!ts->result)
        ts->result = sc_strndup(alloc, "completed: (task)", 17);
    ts->status = SC_TASK_COMPLETED;
    ts->completed_at = (int64_t)time(NULL);
#else
    sc_thread_ctx_t *ctx = (sc_thread_ctx_t *)alloc->alloc(
        alloc->ctx, sizeof(sc_thread_ctx_t));
    if (!ctx) {
        if (ts->label) alloc->free(alloc->ctx, ts->label, strlen(ts->label) + 1);
        alloc->free(alloc->ctx, ts->task, strlen(ts->task) + 1);
        mgr->task_used[slot] = false;
        pthread_mutex_unlock(&mgr->mutex);
        return SC_ERR_OUT_OF_MEMORY;
    }
    ctx->mgr = mgr;
    ctx->slot = slot;

    if (pthread_create(&ts->thread, NULL, subagent_thread_fn, ctx) != 0) {
        alloc->free(alloc->ctx, ctx, sizeof(sc_thread_ctx_t));
        if (ts->label) alloc->free(alloc->ctx, ts->label, strlen(ts->label) + 1);
        alloc->free(alloc->ctx, ts->task, strlen(ts->task) + 1);
        mgr->task_used[slot] = false;
        pthread_mutex_unlock(&mgr->mutex);
        return SC_ERR_INTERNAL;
    }
    ts->thread_valid = true;
    pthread_mutex_unlock(&mgr->mutex);
#endif

    *out_task_id = task_id;
    return SC_OK;
}

sc_task_status_t sc_subagent_get_status(sc_subagent_manager_t *mgr, uint64_t task_id) {
    if (!mgr)
        return SC_TASK_FAILED;

#if !defined(SC_IS_TEST) || SC_IS_TEST == 0
    pthread_mutex_lock(&mgr->mutex);
#endif

    sc_task_state_t *ts = find_task_by_id(mgr, task_id);
    sc_task_status_t status = ts ? ts->status : SC_TASK_FAILED;

#if !defined(SC_IS_TEST) || SC_IS_TEST == 0
    pthread_mutex_unlock(&mgr->mutex);
#endif

    return status;
}

const char *sc_subagent_get_result(sc_subagent_manager_t *mgr, uint64_t task_id) {
    if (!mgr)
        return NULL;

#if !defined(SC_IS_TEST) || SC_IS_TEST == 0
    pthread_mutex_lock(&mgr->mutex);
#endif

    sc_task_state_t *ts = find_task_by_id(mgr, task_id);
    const char *result = ts ? ts->result : NULL;

#if !defined(SC_IS_TEST) || SC_IS_TEST == 0
    pthread_mutex_unlock(&mgr->mutex);
#endif

    return result;
}

size_t sc_subagent_running_count(sc_subagent_manager_t *mgr) {
    if (!mgr)
        return 0;

#if !defined(SC_IS_TEST) || SC_IS_TEST == 0
    pthread_mutex_lock(&mgr->mutex);
#endif

    size_t count = running_count_locked(mgr);

#if !defined(SC_IS_TEST) || SC_IS_TEST == 0
    pthread_mutex_unlock(&mgr->mutex);
#endif

    return count;
}

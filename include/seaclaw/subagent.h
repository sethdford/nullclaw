#ifndef SC_SUBAGENT_H
#define SC_SUBAGENT_H

#include "seaclaw/core/allocator.h"
#include "seaclaw/core/error.h"
#include "seaclaw/config.h"
#include <stddef.h>
#include <stdint.h>
#include <stdbool.h>

typedef enum sc_task_status {
    SC_TASK_RUNNING,
    SC_TASK_COMPLETED,
    SC_TASK_FAILED,
} sc_task_status_t;

typedef struct sc_subagent_manager sc_subagent_manager_t;

sc_subagent_manager_t *sc_subagent_create(sc_allocator_t *alloc, const sc_config_t *cfg);
void sc_subagent_destroy(sc_allocator_t *alloc, sc_subagent_manager_t *mgr);

sc_error_t sc_subagent_spawn(sc_subagent_manager_t *mgr,
    const char *task, size_t task_len,
    const char *label, const char *channel, const char *chat_id,
    uint64_t *out_task_id);

sc_task_status_t sc_subagent_get_status(sc_subagent_manager_t *mgr, uint64_t task_id);
const char *sc_subagent_get_result(sc_subagent_manager_t *mgr, uint64_t task_id);
size_t sc_subagent_running_count(sc_subagent_manager_t *mgr);

#endif /* SC_SUBAGENT_H */

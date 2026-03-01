#include "seaclaw/security/tracker.h"
#include "seaclaw/core/allocator.h"
#include <stdlib.h>
#include <string.h>

#ifdef _WIN32
#include <windows.h>
static uint64_t monotonic_ns(void) {
    LARGE_INTEGER freq, count;
    QueryPerformanceFrequency(&freq);
    QueryPerformanceCounter(&count);
    return (uint64_t)(count.QuadPart * 1000000000ULL / freq.QuadPart);
}
#else
#include <time.h>
static uint64_t monotonic_ns(void) {
    struct timespec ts;
    clock_gettime(CLOCK_MONOTONIC, &ts);
    return (uint64_t)ts.tv_sec * 1000000000ULL + (uint64_t)ts.tv_nsec;
}
#endif

#define SC_TRACKER_INITIAL_CAP 32

struct sc_rate_tracker {
    sc_allocator_t *alloc;
    uint64_t *timestamps;
    size_t count;
    size_t cap;
    uint64_t window_ns;
    uint32_t max_actions;
};

static void prune(sc_rate_tracker_t *t) {
    uint64_t now = monotonic_ns();
    uint64_t cutoff = now - t->window_ns;
    size_t write = 0;
    for (size_t i = 0; i < t->count; i++) {
        if (t->timestamps[i] > cutoff) {
            t->timestamps[write++] = t->timestamps[i];
        }
    }
    t->count = write;
}

sc_rate_tracker_t *sc_rate_tracker_create(sc_allocator_t *alloc,
    uint32_t max_actions, uint64_t window_ns) {
    if (!alloc) return NULL;
    sc_rate_tracker_t *t = (sc_rate_tracker_t *)alloc->alloc(alloc->ctx, sizeof(*t));
    if (!t) return NULL;
    t->alloc = alloc;
    t->timestamps = (uint64_t *)alloc->alloc(alloc->ctx, sizeof(uint64_t) * SC_TRACKER_INITIAL_CAP);
    if (!t->timestamps) {
        alloc->free(alloc->ctx, t, sizeof(*t));
        return NULL;
    }
    t->count = 0;
    t->cap = SC_TRACKER_INITIAL_CAP;
    t->window_ns = window_ns;
    t->max_actions = max_actions;
    return t;
}

void sc_rate_tracker_destroy(sc_allocator_t *alloc, sc_rate_tracker_t *t) {
    if (!alloc || !t) return;
    if (t->timestamps)
        alloc->free(alloc->ctx, t->timestamps, sizeof(uint64_t) * t->cap);
    alloc->free(alloc->ctx, t, sizeof(*t));
}

bool sc_rate_tracker_record(sc_rate_tracker_t *t) {
    if (!t) return false;
    prune(t);
    if (t->count >= t->cap) {
        size_t new_cap = t->cap * 2;
        uint64_t *n = (uint64_t *)t->alloc->alloc(t->alloc->ctx, sizeof(uint64_t) * new_cap);
        if (!n) return false;
        memcpy(n, t->timestamps, t->count * sizeof(uint64_t));
        t->alloc->free(t->alloc->ctx, t->timestamps, sizeof(uint64_t) * t->cap);
        t->timestamps = n;
        t->cap = new_cap;
    }
    t->timestamps[t->count++] = monotonic_ns();
    return t->count <= t->max_actions;
}

bool sc_rate_tracker_is_limited(sc_rate_tracker_t *t) {
    if (!t) return true;
    prune(t);
    return t->count >= t->max_actions;
}

size_t sc_rate_tracker_count(sc_rate_tracker_t *t) {
    if (!t) return 0;
    prune(t);
    return t->count;
}

uint32_t sc_rate_tracker_remaining(sc_rate_tracker_t *t) {
    if (!t) return 0;
    prune(t);
    size_t used = t->count;
    if (used > t->max_actions) used = t->max_actions;
    return t->max_actions - (uint32_t)used;
}

void sc_rate_tracker_reset(sc_rate_tracker_t *t) {
    if (t) t->count = 0;
}

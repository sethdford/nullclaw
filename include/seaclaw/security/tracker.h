#ifndef SC_TRACKER_H
#define SC_TRACKER_H

#include "seaclaw/core/allocator.h"
#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

/* Sliding-window rate tracker for action limiting. */
typedef struct sc_rate_tracker sc_rate_tracker_t;

sc_rate_tracker_t *sc_rate_tracker_create(sc_allocator_t *alloc,
    uint32_t max_actions, uint64_t window_ns);

void sc_rate_tracker_destroy(sc_allocator_t *alloc, sc_rate_tracker_t *t);

/* Record action. Returns true if allowed, false if rate-limited. */
bool sc_rate_tracker_record(sc_rate_tracker_t *t);

/* Check if would be limited without recording. */
bool sc_rate_tracker_is_limited(sc_rate_tracker_t *t);

/* Current count in window. */
size_t sc_rate_tracker_count(sc_rate_tracker_t *t);

/* Remaining allowed. */
uint32_t sc_rate_tracker_remaining(sc_rate_tracker_t *t);

void sc_rate_tracker_reset(sc_rate_tracker_t *t);

#endif /* SC_TRACKER_H */

#ifndef SC_AB_RESPONSE_H
#define SC_AB_RESPONSE_H

#include "seaclaw/core/allocator.h"
#include "seaclaw/core/error.h"
#include "seaclaw/channel.h"
#include <stdbool.h>
#include <stddef.h>

#define SC_AB_MAX_CANDIDATES 3

typedef struct sc_ab_candidate {
    char *response;
    size_t response_len;
    int quality_score;
    bool needs_revision;
} sc_ab_candidate_t;

typedef struct sc_ab_result {
    sc_ab_candidate_t candidates[SC_AB_MAX_CANDIDATES];
    size_t candidate_count;
    size_t best_idx;
} sc_ab_result_t;

/* Score multiple pre-generated response candidates and pick the best.
 * Does NOT generate responses — just evaluates and selects.
 * Caller provides the candidates (response + response_len), function fills in scores.
 * Returns SC_OK and sets best_idx to the highest-scoring candidate. */
sc_error_t sc_ab_evaluate(sc_allocator_t *alloc, sc_ab_result_t *result,
                        const sc_channel_history_entry_t *entries, size_t entry_count,
                        uint32_t max_chars);

void sc_ab_result_deinit(sc_ab_result_t *result, sc_allocator_t *alloc);

#endif

#ifndef SC_PERSONA_REPLAY_H
#define SC_PERSONA_REPLAY_H

#include "seaclaw/core/allocator.h"
#include "seaclaw/core/error.h"
#include "seaclaw/channel.h"
#include <stddef.h>
#include <stdint.h>

#define SC_REPLAY_MAX_INSIGHTS 10

typedef struct sc_replay_insight {
    char *observation;      /* "Response to 'how was your day' felt generic" */
    size_t observation_len;
    char *recommendation;   /* "Use more specific follow-up questions" */
    size_t recommendation_len;
    int score_delta;        /* positive = good, negative = needs improvement */
} sc_replay_insight_t;

typedef struct sc_replay_result {
    sc_replay_insight_t insights[SC_REPLAY_MAX_INSIGHTS];
    size_t insight_count;
    int overall_score;      /* 0-100 */
    char *summary;          /* "Conversation was mostly natural but 2 responses felt robotic" */
    size_t summary_len;
} sc_replay_result_t;

sc_error_t sc_replay_analyze(sc_allocator_t *alloc,
                             const sc_channel_history_entry_t *entries, size_t entry_count,
                             uint32_t max_chars, sc_replay_result_t *out);
void sc_replay_result_deinit(sc_replay_result_t *result, sc_allocator_t *alloc);

/* Build a context string from replay insights for future prompt injection.
 * Summarizes what worked and what didn't. Caller owns returned string. */
char *sc_replay_build_context(sc_allocator_t *alloc, const sc_replay_result_t *result,
                              size_t *out_len);

#endif

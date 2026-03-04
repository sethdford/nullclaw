#ifndef SC_MEMORY_RERANK_H
#define SC_MEMORY_RERANK_H

#include "seaclaw/core/error.h"
#include <stddef.h>

typedef struct sc_search_result {
    char *content;
    float score;        /* original score (BM25 or cosine) */
    float rerank_score; /* after reranking */
    size_t original_rank;
} sc_search_result_t;

/* Reciprocal Rank Fusion — merge keyword + vector results.
 * rrf_score = sum(1.0 / (k + rank)) across lists. k default 60.0. */
sc_error_t sc_rerank_rrf(sc_search_result_t *keyword_results, size_t keyword_count,
                         sc_search_result_t *vector_results, size_t vector_count,
                         sc_search_result_t *merged_out, size_t max_results, size_t *merged_count,
                         float k);

/* Cross-encoder rerank (simple: score by query-document term overlap).
 * rerank_score = matching_query_words / total_query_words. Re-sorts in place. */
sc_error_t sc_rerank_cross_encoder(const char *query, sc_search_result_t *results, size_t count);

/* Free content of results (caller owns the array). */
void sc_rerank_free_results(sc_search_result_t *results, size_t count);

#endif /* SC_MEMORY_RERANK_H */

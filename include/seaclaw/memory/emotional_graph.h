#ifndef SC_EMOTIONAL_GRAPH_H
#define SC_EMOTIONAL_GRAPH_H

#include "seaclaw/core/allocator.h"
#include "seaclaw/core/error.h"
#include "seaclaw/memory/stm.h"
#include <stddef.h>
#include <stdint.h>

#define SC_EGRAPH_MAX_EDGES 64
#define SC_EGRAPH_MAX_NODES 32

typedef struct sc_egraph_node {
    char *topic;
    size_t topic_len;
} sc_egraph_node_t;

typedef struct sc_egraph_edge {
    size_t topic_idx;
    sc_emotion_tag_t emotion;
    double intensity; /* average intensity across occurrences */
    uint32_t occurrence_count;
} sc_egraph_edge_t;

typedef struct sc_emotional_graph {
    sc_allocator_t alloc;
    sc_egraph_node_t nodes[SC_EGRAPH_MAX_NODES];
    size_t node_count;
    sc_egraph_edge_t edges[SC_EGRAPH_MAX_EDGES];
    size_t edge_count;
} sc_emotional_graph_t;

sc_error_t sc_egraph_init(sc_emotional_graph_t *g, sc_allocator_t alloc);

/* Record an emotional association: topic X evokes emotion Y at intensity Z.
 * If the topic+emotion edge already exists, update the running average. */
sc_error_t sc_egraph_record(sc_emotional_graph_t *g, const char *topic, size_t topic_len,
                            sc_emotion_tag_t emotion, double intensity);

/* Query the dominant emotion for a given topic.
 * Returns the emotion with the highest average intensity.
 * If the topic isn't found, returns SC_EMOTION_NEUTRAL with intensity 0. */
sc_emotion_tag_t sc_egraph_query(const sc_emotional_graph_t *g, const char *topic, size_t topic_len,
                                 double *avg_intensity);

/* Build a context string for the prompt that maps topics to emotions.
 * Output like: "Topics and their emotional associations:\n- work → stress (high)\n- cooking → joy (moderate)\n"
 * Returns NULL if no edges. Caller owns returned string. */
char *sc_egraph_build_context(sc_allocator_t *alloc, const sc_emotional_graph_t *g, size_t *out_len);

/* Populate the graph from STM buffer. Scans turns for topics (primary_topic)
 * and emotions, creating edges between them. */
sc_error_t sc_egraph_populate_from_stm(sc_emotional_graph_t *g, const sc_stm_buffer_t *buf);

void sc_egraph_deinit(sc_emotional_graph_t *g);

#endif

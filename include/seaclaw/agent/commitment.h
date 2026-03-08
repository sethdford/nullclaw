#ifndef SC_AGENT_COMMITMENT_H
#define SC_AGENT_COMMITMENT_H

#include "seaclaw/core/allocator.h"
#include "seaclaw/core/error.h"
#include <stddef.h>

/* ──────────────────────────────────────────────────────────────────────────
 * Commitment detection — promises, intentions, reminders, goals from text
 * ────────────────────────────────────────────────────────────────────────── */

typedef enum sc_commitment_type {
    SC_COMMITMENT_PROMISE,
    SC_COMMITMENT_INTENTION,
    SC_COMMITMENT_REMINDER,
    SC_COMMITMENT_GOAL,
} sc_commitment_type_t;

typedef enum sc_commitment_status {
    SC_COMMITMENT_ACTIVE,
    SC_COMMITMENT_COMPLETED,
    SC_COMMITMENT_EXPIRED,
    SC_COMMITMENT_CANCELLED,
} sc_commitment_status_t;

typedef struct sc_commitment {
    char *id;           /* owned */
    char *statement;    /* owned; full text */
    size_t statement_len;
    char *summary;      /* owned; extracted clause */
    size_t summary_len;
    sc_commitment_type_t type;
    sc_commitment_status_t status;
    char *created_at;   /* owned; ISO8601 */
    char *emotional_weight; /* owned; optional, e.g. "0.5" */
    char *owner;        /* owned; "user" or "assistant" */
} sc_commitment_t;

#define SC_COMMITMENT_DETECT_MAX 5

typedef struct sc_commitment_detect_result {
    sc_commitment_t commitments[SC_COMMITMENT_DETECT_MAX];
    size_t count;
} sc_commitment_detect_result_t;

/* Detect commitments from text. role = "user" or "assistant". */
sc_error_t sc_commitment_detect(sc_allocator_t *alloc, const char *text, size_t text_len,
                               const char *role, size_t role_len,
                               sc_commitment_detect_result_t *result);

void sc_commitment_deinit(sc_commitment_t *c, sc_allocator_t *alloc);
void sc_commitment_detect_result_deinit(sc_commitment_detect_result_t *r, sc_allocator_t *alloc);

#endif /* SC_AGENT_COMMITMENT_H */

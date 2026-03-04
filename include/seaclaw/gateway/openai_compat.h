#ifndef SC_OPENAI_COMPAT_H
#define SC_OPENAI_COMPAT_H

#include "seaclaw/core/allocator.h"
#include "seaclaw/gateway/control_protocol.h"
#include <stddef.h>

/* ──────────────────────────────────────────────────────────────────────────
 * OpenAI-compatible API handlers for gateway
 * ────────────────────────────────────────────────────────────────────────── */

/* Handle POST /v1/chat/completions. Allocates *out_body (caller frees).
 * Sets *out_status and *out_body, *out_body_len. Non-streaming only. */
void sc_openai_compat_handle_chat_completions(const char *body, size_t body_len,
                                               sc_allocator_t *alloc,
                                               const sc_app_context_t *app_ctx,
                                               int *out_status, char **out_body,
                                               size_t *out_body_len);

/* Handle GET /v1/models. Allocates *out_body (caller frees). */
void sc_openai_compat_handle_models(sc_allocator_t *alloc,
                                    const sc_app_context_t *app_ctx,
                                    int *out_status, char **out_body,
                                    size_t *out_body_len);

#endif /* SC_OPENAI_COMPAT_H */

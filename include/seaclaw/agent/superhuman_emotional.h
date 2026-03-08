#ifndef SC_SUPERHUMAN_EMOTIONAL_H
#define SC_SUPERHUMAN_EMOTIONAL_H

#include "seaclaw/agent/superhuman.h"
#include "seaclaw/core/allocator.h"
#include "seaclaw/core/error.h"
#include <stddef.h>

#define SC_EMOTIONAL_LAST_TEXT_CAP 4096

typedef struct sc_superhuman_emotional_ctx {
    char last_text[SC_EMOTIONAL_LAST_TEXT_CAP];
    size_t last_text_len;
    char last_role[64];
    size_t last_role_len;
} sc_superhuman_emotional_ctx_t;

sc_error_t sc_superhuman_emotional_service(sc_superhuman_emotional_ctx_t *ctx,
                                           sc_superhuman_service_t *out);

#endif /* SC_SUPERHUMAN_EMOTIONAL_H */

#include "seaclaw/core/error.h"
#include "seaclaw/persona.h"
#include <stdio.h>
#include <string.h>

sc_error_t sc_persona_sampler_imessage_query(char *buf, size_t cap, size_t *out_len, size_t limit) {
    if (!buf || !out_len || cap < 64)
        return SC_ERR_INVALID_ARGUMENT;
    /* iMessage chat.db: message table has text, handle_id, date, is_from_me.
     * Select user's outgoing messages, non-empty text, recency bias (ORDER BY date DESC). */
    int n = snprintf(buf, cap,
                     "SELECT text, handle_id, date, is_from_me FROM message "
                     "WHERE is_from_me = 1 AND text IS NOT NULL AND text != '' "
                     "ORDER BY date DESC LIMIT %zu",
                     limit);
    if (n < 0 || (size_t)n >= cap)
        return SC_ERR_INVALID_ARGUMENT;
    *out_len = (size_t)n;
    return SC_OK;
}

sc_error_t sc_persona_sampler_facebook_parse(const char *json, size_t json_len, char ***out,
                                             size_t *out_count) {
    (void)json;
    (void)json_len;
    (void)out;
    (void)out_count;
    return SC_ERR_NOT_SUPPORTED;
}

#ifndef SC_PROVIDER_HTTP_H
#define SC_PROVIDER_HTTP_H

#include "seaclaw/core/allocator.h"
#include "seaclaw/core/error.h"
#include "seaclaw/core/json.h"
#include <stddef.h>

/* ──────────────────────────────────────────────────────────────────────────
 * Shared HTTP POST + JSON parse for provider chat APIs
 * ────────────────────────────────────────────────────────────────────────── */

/* POST JSON body to url with optional auth and extra headers.
 * On success: *parsed_out is set; caller must sc_json_free(alloc, *parsed_out).
 * auth_header: e.g. "Bearer sk-xxx", or NULL if using extra_headers for auth.
 * extra_headers: optional, e.g. "x-api-key: val\r\nanthropic-version: 2023-06-01\r\n"
 */
sc_error_t sc_provider_http_post_json(sc_allocator_t *alloc, const char *url,
                                      const char *auth_header, const char *extra_headers,
                                      const char *body, size_t body_len,
                                      sc_json_value_t **parsed_out);

#endif /* SC_PROVIDER_HTTP_H */

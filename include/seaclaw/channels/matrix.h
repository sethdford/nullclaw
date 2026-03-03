#ifndef SC_CHANNELS_MATRIX_H
#define SC_CHANNELS_MATRIX_H

#include "seaclaw/channel.h"
#include "seaclaw/channel_loop.h"
#include "seaclaw/core/allocator.h"
#include "seaclaw/core/error.h"
#include <stddef.h>

sc_error_t sc_matrix_create(sc_allocator_t *alloc, const char *homeserver, size_t homeserver_len,
                            const char *access_token, size_t access_token_len, sc_channel_t *out);

sc_error_t sc_matrix_poll(void *channel_ctx, sc_allocator_t *alloc, sc_channel_loop_msg_t *msgs,
                          size_t max_msgs, size_t *out_count);

void sc_matrix_destroy(sc_channel_t *ch);

#endif /* SC_CHANNELS_MATRIX_H */

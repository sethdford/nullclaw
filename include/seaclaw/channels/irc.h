#ifndef SC_CHANNELS_IRC_H
#define SC_CHANNELS_IRC_H

#include "seaclaw/channel.h"
#include "seaclaw/channel_loop.h"
#include "seaclaw/core/allocator.h"
#include "seaclaw/core/error.h"
#include <stddef.h>
#include <stdint.h>

sc_error_t sc_irc_create(sc_allocator_t *alloc, const char *server, size_t server_len,
                         uint16_t port, sc_channel_t *out);

sc_error_t sc_irc_poll(void *channel_ctx, sc_allocator_t *alloc, sc_channel_loop_msg_t *msgs,
                       size_t max_msgs, size_t *out_count);

void sc_irc_destroy(sc_channel_t *ch);

#endif /* SC_CHANNELS_IRC_H */

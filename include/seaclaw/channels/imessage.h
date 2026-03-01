#ifndef SC_CHANNELS_IMESSAGE_H
#define SC_CHANNELS_IMESSAGE_H

#include "seaclaw/channel.h"
#include "seaclaw/core/allocator.h"
#include "seaclaw/core/error.h"
#include <stdbool.h>
#include <stddef.h>

sc_error_t sc_imessage_create(sc_allocator_t *alloc,
    const char *default_target, size_t default_target_len,
    sc_channel_t *out);
void sc_imessage_destroy(sc_channel_t *ch);

/** Returns true if default target (phone/email) is configured. */
bool sc_imessage_is_configured(sc_channel_t *ch);

#endif /* SC_CHANNELS_IMESSAGE_H */

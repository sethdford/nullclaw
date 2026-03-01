#ifndef SC_CHANNELS_EMAIL_H
#define SC_CHANNELS_EMAIL_H

#include "seaclaw/channel.h"
#include "seaclaw/core/allocator.h"
#include "seaclaw/core/error.h"
#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

sc_error_t sc_email_create(sc_allocator_t *alloc,
    const char *smtp_host, size_t smtp_host_len,
    uint16_t smtp_port,
    const char *from_address, size_t from_len,
    sc_channel_t *out);
void sc_email_destroy(sc_channel_t *ch);

bool sc_email_is_configured(sc_channel_t *ch);

#if SC_IS_TEST
const char *sc_email_test_last_message(sc_channel_t *ch);
#endif

#endif /* SC_CHANNELS_EMAIL_H */

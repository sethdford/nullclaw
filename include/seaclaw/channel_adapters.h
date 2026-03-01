#ifndef SC_CHANNEL_ADAPTERS_H
#define SC_CHANNEL_ADAPTERS_H

#include <stddef.h>

/* Channel adapters: polling descriptors, inbound route descriptors.
 * SeaClaw: minimal stub — parsePeerKind, findPollingDescriptor. */

typedef enum sc_chat_type {
    SC_CHAT_DIRECT,
    SC_CHAT_GROUP,
    SC_CHAT_CHANNEL,
} sc_chat_type_t;

/* Parse peer kind string to ChatType. Returns -1 if invalid. */
int sc_channel_adapters_parse_peer_kind(const char *raw, size_t len);

/* Find polling descriptor by channel name. Returns NULL if not found. */
const void *sc_channel_adapters_find_polling_descriptor(const char *channel_name, size_t len);

#endif /* SC_CHANNEL_ADAPTERS_H */

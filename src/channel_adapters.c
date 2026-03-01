#include "seaclaw/channel_adapters.h"
#include <string.h>

/* Minimal stub: parse peer kind for agent_routing compatibility. */
int sc_channel_adapters_parse_peer_kind(const char *raw, size_t len) {
    if (!raw) return -1;
    if (len >= 6 && strncmp(raw, "direct", 6) == 0) return (int)SC_CHAT_DIRECT;
    if (len >= 2 && strncmp(raw, "dm", 2) == 0) return (int)SC_CHAT_DIRECT;
    if (len >= 5 && strncmp(raw, "group", 5) == 0) return (int)SC_CHAT_GROUP;
    if (len >= 7 && strncmp(raw, "channel", 7) == 0) return (int)SC_CHAT_CHANNEL;
    return -1;
}

/* Stub: no polling descriptors in minimal build. */
const void *sc_channel_adapters_find_polling_descriptor(const char *channel_name, size_t len) {
    (void)channel_name;
    (void)len;
    return NULL;
}

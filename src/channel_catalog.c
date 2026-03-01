#include "seaclaw/channel_catalog.h"
#include "seaclaw/config.h"
#include <string.h>

/* Static catalog for build-enabled channels */
static const sc_channel_meta_t catalog[] = {
    { SC_CHANNEL_CLI, "cli", "CLI", "", SC_LISTENER_NONE },
#ifdef SC_HAS_TELEGRAM
    { SC_CHANNEL_TELEGRAM, "telegram", "Telegram", "", SC_LISTENER_POLLING },
#endif
#ifdef SC_HAS_DISCORD
    { SC_CHANNEL_DISCORD, "discord", "Discord", "", SC_LISTENER_GATEWAY },
#endif
#ifdef SC_HAS_SLACK
    { SC_CHANNEL_SLACK, "slack", "Slack", "", SC_LISTENER_GATEWAY },
#endif
};
static const size_t catalog_len = sizeof(catalog) / sizeof(catalog[0]);

const sc_channel_meta_t *sc_channel_catalog_all(size_t *out_count) {
    *out_count = catalog_len;
    return catalog;
}

bool sc_channel_catalog_is_build_enabled(sc_channel_id_t id) {
    (void)id;
#ifdef SC_HAS_CLI
    if (id == SC_CHANNEL_CLI) return true;
#endif
    return false;
}

size_t sc_channel_catalog_configured_count(const sc_config_t *cfg, sc_channel_id_t id) {
    (void)cfg;
    (void)id;
    return 0;
}

bool sc_channel_catalog_is_configured(const sc_config_t *cfg, sc_channel_id_t id) {
    return sc_channel_catalog_configured_count(cfg, id) > 0;
}

const char *sc_channel_catalog_status_text(const sc_config_t *cfg,
    const sc_channel_meta_t *meta, char *buf, size_t buf_size) {
    (void)cfg;
    if (buf_size > 0) buf[0] = '\0';
    return meta ? meta->key : buf;
}

const sc_channel_meta_t *sc_channel_catalog_find_by_key(const char *key) {
    if (!key) return NULL;
    size_t len = strlen(key);
    for (size_t i = 0; i < catalog_len; i++) {
        if (strncmp(catalog[i].key, key, len) == 0 && catalog[i].key[len] == '\0')
            return &catalog[i];
    }
    return NULL;
}

bool sc_channel_catalog_has_any_configured(const sc_config_t *cfg, bool include_cli) {
    (void)cfg;
    (void)include_cli;
    return false;
}

bool sc_channel_catalog_contributes_to_daemon(sc_channel_id_t id) {
    (void)id;
    return false;
}

bool sc_channel_catalog_requires_runtime(sc_channel_id_t id) {
    (void)id;
    return false;
}

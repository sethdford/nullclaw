#ifndef SC_BOOTSTRAP_H
#define SC_BOOTSTRAP_H

#include "seaclaw/agent.h"
#include "seaclaw/bus.h"
#include "seaclaw/config.h"
#include "seaclaw/core/allocator.h"
#include "seaclaw/core/error.h"
#include "seaclaw/daemon.h"
#include "seaclaw/provider.h"
#include "seaclaw/security.h"
#include "seaclaw/tool.h"
#include <stdbool.h>
#include <stddef.h>

/* ──────────────────────────────────────────────────────────────────────────
 * App bootstrap context — holds all subsystems created by sc_app_bootstrap.
 * Caller uses these pointers; sc_app_teardown frees everything.
 * ────────────────────────────────────────────────────────────────────────── */

typedef struct sc_app_ctx {
    sc_allocator_t *alloc;
    sc_config_t *cfg;
    void *plugin_reg; /* sc_plugin_registry_t * */
    sc_security_policy_t *policy;
    sc_sandbox_storage_t *sb_storage;
    sc_sandbox_alloc_t sb_alloc;

    sc_tool_t *tools;
    size_t tools_count;

    /* When with_agent: provider, memory, agent, retrieval, etc. */
    sc_provider_t *provider;
    sc_memory_t *memory;
    sc_agent_t *agent;
    void *embedder;      /* sc_embedder_t * */
    void *vector_store;  /* sc_vector_store_t * */
    void *retrieval;     /* sc_retrieval_engine_t * */
    void *session_store; /* sc_session_store_t * */
    void *agent_pool;    /* sc_agent_pool_t * */
    void *mailbox;       /* sc_mailbox_t * */
    void *cron;          /* sc_cron_scheduler_t * */

    /* When with_channels: service channels for polling */
    sc_service_channel_t *channels;
    size_t channel_count;
    void *channel_instances; /* opaque storage for channel vtables; teardown uses it */

    /* Flags set by bootstrap */
    bool provider_ok;
    bool agent_ok;
} sc_app_ctx_t;

/* Initialize the full app context: load config, create provider, tools, security,
 * optionally channels and agent. config_path may be NULL (use default).
 * with_agent: create provider, memory, agent, retrieval.
 * with_channels: create service channels from config (for service-loop mode). */
sc_error_t sc_app_bootstrap(sc_app_ctx_t *ctx, sc_allocator_t *alloc, const char *config_path,
                            bool with_agent, bool with_channels);

/* Clean up everything in reverse order. Safe to call on partially-initialized ctx. */
void sc_app_teardown(sc_app_ctx_t *ctx);

#endif /* SC_BOOTSTRAP_H */

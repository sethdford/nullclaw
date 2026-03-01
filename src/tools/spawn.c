#include "seaclaw/tool.h"
#include "seaclaw/core/allocator.h"
#include "seaclaw/core/error.h"
#include "seaclaw/core/string.h"
#include "seaclaw/core/json.h"
#include "seaclaw/security.h"
#include "seaclaw/config.h"
#include <string.h>
#include <stdlib.h>

#ifndef _WIN32
#include <unistd.h>
#include <sys/wait.h>
#include <sys/types.h>
#endif

#define SC_SPAWN_NAME "spawn"
#define SC_SPAWN_DESC "Spawn child process"
#define SC_SPAWN_PARAMS "{\"type\":\"object\",\"properties\":{\"command\":{\"type\":\"string\"},\"args\":{\"type\":\"array\"}},\"required\":[\"command\"]}"

typedef struct sc_spawn_ctx {
    const char *workspace_dir;
    size_t workspace_dir_len;
    sc_security_policy_t *policy;
} sc_spawn_ctx_t;

static sc_error_t spawn_execute(void *ctx, sc_allocator_t *alloc,
    const sc_json_value_t *args,
    sc_tool_result_t *out)
{
    sc_spawn_ctx_t *c = (sc_spawn_ctx_t *)ctx;
    if (!c || !args || !out) {
        *out = sc_tool_result_fail("invalid args", 12);
        return SC_ERR_INVALID_ARGUMENT;
    }
    const char *cmd = sc_json_get_string(args, "command");
    if (!cmd || strlen(cmd) == 0) {
        *out = sc_tool_result_fail("missing command", 14);
        return SC_OK;
    }
#if SC_IS_TEST
    char *msg = sc_strndup(alloc, "(spawn disabled in test)", 24);
    if (!msg) { *out = sc_tool_result_fail("out of memory", 12); return SC_ERR_OUT_OF_MEMORY; }
    *out = sc_tool_result_ok_owned(msg, 24);
    return SC_OK;
#else
#ifndef _WIN32
    if (c->policy && !sc_security_shell_allowed(c->policy)) {
        *out = sc_tool_result_fail("spawn not allowed by policy", 27);
        return SC_OK;
    }
    *out = sc_tool_result_fail("spawn not implemented", 22);
    return SC_OK;
#else
    (void)c;
    (void)alloc;
    *out = sc_tool_result_fail("spawn not supported on platform", 31);
    return SC_OK;
#endif
#endif
}

static const char *spawn_name(void *ctx) { (void)ctx; return SC_SPAWN_NAME; }
static const char *spawn_description(void *ctx) { (void)ctx; return SC_SPAWN_DESC; }
static const char *spawn_parameters_json(void *ctx) { (void)ctx; return SC_SPAWN_PARAMS; }
static void spawn_deinit(void *ctx, sc_allocator_t *alloc) { (void)alloc; if (ctx) free(ctx); }

static const sc_tool_vtable_t spawn_vtable = {
    .execute = spawn_execute, .name = spawn_name,
    .description = spawn_description, .parameters_json = spawn_parameters_json,
    .deinit = spawn_deinit,
};

sc_error_t sc_spawn_create(sc_allocator_t *alloc,
    const char *workspace_dir, size_t workspace_dir_len,
    sc_security_policy_t *policy,
    sc_tool_t *out)
{
    sc_spawn_ctx_t *c = (sc_spawn_ctx_t *)calloc(1, sizeof(*c));
    if (!c) return SC_ERR_OUT_OF_MEMORY;
    if (workspace_dir && workspace_dir_len > 0) {
        c->workspace_dir = sc_strndup(alloc, workspace_dir, workspace_dir_len);
        if (!c->workspace_dir) { free(c); return SC_ERR_OUT_OF_MEMORY; }
        c->workspace_dir_len = workspace_dir_len;
    }
    c->policy = policy;
    out->ctx = c;
    out->vtable = &spawn_vtable;
    return SC_OK;
}

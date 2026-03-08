#include "seaclaw/agent/tool_router.h"
#include "seaclaw/core/allocator.h"
#include "seaclaw/core/json.h"
#include "seaclaw/tool.h"
#include "test_framework.h"
#include <string.h>

static const char *mock_name_memory_store(void *ctx) {
    (void)ctx;
    return "memory_store";
}
static const char *mock_desc_memory_store(void *ctx) {
    (void)ctx;
    return "Store a fact in memory";
}
static const char *mock_name_obscure(void *ctx) {
    (void)ctx;
    return "obscure_tool";
}
static const char *mock_desc_obscure(void *ctx) {
    (void)ctx;
    return "Does something obscure";
}
static const char *mock_name_web_lookup(void *ctx) {
    (void)ctx;
    return "web_lookup";
}
static const char *mock_desc_web_lookup(void *ctx) {
    (void)ctx;
    return "Search the web for information";
}
static const char *mock_name_file_scan(void *ctx) {
    (void)ctx;
    return "file_scan";
}
static const char *mock_desc_file_scan(void *ctx) {
    (void)ctx;
    return "Scan file contents";
}

static sc_error_t mock_exec_noop(void *ctx, sc_allocator_t *alloc, const sc_json_value_t *args,
                                 sc_tool_result_t *out) {
    (void)ctx;
    (void)alloc;
    (void)args;
    *out = sc_tool_result_ok("ok", 2);
    return SC_OK;
}

static void router_always_includes_core_tools(void) {
    sc_allocator_t alloc = sc_system_allocator();
    static const sc_tool_vtable_t vtable_memory = {
        .execute = mock_exec_noop,
        .name = mock_name_memory_store,
        .description = mock_desc_memory_store,
        .parameters_json = NULL,
        .deinit = NULL,
    };
    static const sc_tool_vtable_t vtable_obscure = {
        .execute = mock_exec_noop,
        .name = mock_name_obscure,
        .description = mock_desc_obscure,
        .parameters_json = NULL,
        .deinit = NULL,
    };
    sc_tool_t tools[] = {
        {.ctx = NULL, .vtable = &vtable_memory},
        {.ctx = NULL, .vtable = &vtable_obscure},
    };
    sc_tool_selection_t sel;
    memset(&sel, 0, sizeof(sel));

    SC_ASSERT_EQ(sc_tool_router_select(&alloc, "hello", 5, tools, 2, &sel), SC_OK);
    SC_ASSERT_TRUE(sel.count >= 1);

    bool has_memory_store = false;
    for (size_t i = 0; i < sel.count; i++) {
        size_t idx = sel.indices[i];
        const char *name = tools[idx].vtable->name(tools[idx].ctx);
        if (name && strcmp(name, "memory_store") == 0)
            has_memory_store = true;
    }
    SC_ASSERT_TRUE(has_memory_store);
}

static void router_limits_to_max_selected(void) {
    sc_allocator_t alloc = sc_system_allocator();
    static const sc_tool_vtable_t vtable = {
        .execute = mock_exec_noop,
        .name = mock_name_obscure,
        .description = mock_desc_obscure,
        .parameters_json = NULL,
        .deinit = NULL,
    };
    sc_tool_t tools[22];
    for (size_t i = 0; i < 22; i++)
        tools[i] = (sc_tool_t){.ctx = NULL, .vtable = &vtable};

    sc_tool_selection_t sel;
    memset(&sel, 0, sizeof(sel));
    SC_ASSERT_EQ(sc_tool_router_select(&alloc, "test message", 12, tools, 22, &sel), SC_OK);
    SC_ASSERT_TRUE(sel.count <= SC_TOOL_ROUTER_MAX_SELECTED);
}

static void router_selects_relevant_by_keyword(void) {
    sc_allocator_t alloc = sc_system_allocator();
    static const sc_tool_vtable_t vtable_web = {
        .execute = mock_exec_noop,
        .name = mock_name_web_lookup,
        .description = mock_desc_web_lookup,
        .parameters_json = NULL,
        .deinit = NULL,
    };
    static const sc_tool_vtable_t vtable_file = {
        .execute = mock_exec_noop,
        .name = mock_name_file_scan,
        .description = mock_desc_file_scan,
        .parameters_json = NULL,
        .deinit = NULL,
    };
    sc_tool_t tools[] = {
        {.ctx = NULL, .vtable = &vtable_web},
        {.ctx = NULL, .vtable = &vtable_file},
    };
    sc_tool_selection_t sel;
    memset(&sel, 0, sizeof(sel));

    const char *msg = "search the web for news";
    SC_ASSERT_EQ(sc_tool_router_select(&alloc, msg, strlen(msg), tools, 2, &sel), SC_OK);

    /* web_lookup matches "search","web" - higher score than file_scan which matches little */
    bool has_web = false;
    for (size_t i = 0; i < sel.count; i++) {
        size_t idx = sel.indices[i];
        const char *name = tools[idx].vtable->name(tools[idx].ctx);
        if (name && strcmp(name, "web_lookup") == 0)
            has_web = true;
    }
    SC_ASSERT_TRUE(has_web);
}

void run_tool_router_tests(void) {
    SC_TEST_SUITE("tool_router");
    SC_RUN_TEST(router_always_includes_core_tools);
    SC_RUN_TEST(router_limits_to_max_selected);
    SC_RUN_TEST(router_selects_relevant_by_keyword);
}

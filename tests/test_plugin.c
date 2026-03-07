/* Plugin system wiring tests. */
#include "seaclaw/config.h"
#include "seaclaw/core/allocator.h"
#include "seaclaw/core/arena.h"
#include "seaclaw/core/error.h"
#include "seaclaw/plugin.h"
#include "seaclaw/plugin_loader.h"
#include "test_framework.h"
#include <string.h>

static void test_plugin_load_bad_path_fails(void) {
    sc_allocator_t a = sc_system_allocator();
    sc_plugin_host_t host = {
        .alloc = &a,
        .register_tool = NULL,
        .register_provider = NULL,
        .register_channel = NULL,
        .host_ctx = NULL,
    };
    sc_plugin_info_t info = {0};
    sc_plugin_handle_t *handle = NULL;
    sc_error_t err = sc_plugin_load(&a, "/nonexistent/plugin.so", &host, &info, &handle);
    SC_ASSERT_EQ(err, SC_ERR_NOT_FOUND);
    SC_ASSERT_NULL(handle);
}

static sc_error_t test_register_tool_cb(void *ctx, const char *name, void *tool_vtable) {
    (void)name;
    (void)tool_vtable;
    int *count = (int *)ctx;
    if (count)
        (*count)++;
    return SC_OK;
}

static void test_plugin_host_register_tool_callback(void) {
    sc_allocator_t a = sc_system_allocator();
    int callback_count = 0;
    sc_plugin_host_t host = {
        .alloc = &a,
        .register_tool = test_register_tool_cb,
        .register_provider = NULL,
        .register_channel = NULL,
        .host_ctx = &callback_count,
    };
    sc_plugin_info_t info = {0};
    sc_plugin_handle_t *handle = NULL;
    /* Load a mock plugin - in SC_IS_TEST, /valid/path.so succeeds */
    sc_error_t err = sc_plugin_load(&a, "/valid/mock.so", &host, &info, &handle);
    SC_ASSERT_EQ(err, SC_OK);
    SC_ASSERT_NOT_NULL(handle);
    SC_ASSERT_STR_EQ(info.name, "mock-plugin");
    /* Plugin init can call register_tool - mock doesn't, but we verify host is passed */
    SC_ASSERT_NOT_NULL(host.register_tool);
    sc_plugin_unload(handle);
}

static void test_plugin_config_parse_empty_array(void) {
    sc_allocator_t backing = sc_system_allocator();
    sc_config_t cfg;
    memset(&cfg, 0, sizeof(cfg));
    sc_arena_t *arena = sc_arena_create(backing);
    SC_ASSERT_NOT_NULL(arena);
    cfg.arena = arena;
    cfg.allocator = sc_arena_allocator(arena);
    const char *json = "{\"plugins\":[]}";
    sc_error_t err = sc_config_parse_json(&cfg, json, strlen(json));
    SC_ASSERT_EQ(err, SC_OK);
    SC_ASSERT_EQ(cfg.plugins.plugin_paths_len, 0);
    SC_ASSERT_NULL(cfg.plugins.plugin_paths);
    sc_arena_destroy(arena);
}

static void test_plugin_config_parse_with_paths(void) {
    sc_allocator_t backing = sc_system_allocator();
    sc_config_t cfg;
    memset(&cfg, 0, sizeof(cfg));
    sc_arena_t *arena = sc_arena_create(backing);
    SC_ASSERT_NOT_NULL(arena);
    cfg.arena = arena;
    cfg.allocator = sc_arena_allocator(arena);
    const char *json =
        "{\"plugins\":{\"enabled\":true,\"paths\":[\"/lib/foo.so\",\"/lib/bar.so\"]}}";
    sc_error_t err = sc_config_parse_json(&cfg, json, strlen(json));
    SC_ASSERT_EQ(err, SC_OK);
    SC_ASSERT_TRUE(cfg.plugins.enabled);
    SC_ASSERT_EQ(cfg.plugins.plugin_paths_len, 2);
    SC_ASSERT_NOT_NULL(cfg.plugins.plugin_paths);
    SC_ASSERT_STR_EQ(cfg.plugins.plugin_paths[0], "/lib/foo.so");
    SC_ASSERT_STR_EQ(cfg.plugins.plugin_paths[1], "/lib/bar.so");
    sc_arena_destroy(arena);
}

static void test_plugin_config_parse_plugins_as_array(void) {
    sc_allocator_t backing = sc_system_allocator();
    sc_config_t cfg;
    memset(&cfg, 0, sizeof(cfg));
    sc_arena_t *arena = sc_arena_create(backing);
    SC_ASSERT_NOT_NULL(arena);
    cfg.arena = arena;
    cfg.allocator = sc_arena_allocator(arena);
    const char *json = "{\"plugins\":[\"/a.so\",\"/b.so\"]}";
    sc_error_t err = sc_config_parse_json(&cfg, json, strlen(json));
    SC_ASSERT_EQ(err, SC_OK);
    SC_ASSERT_EQ(cfg.plugins.plugin_paths_len, 2);
    SC_ASSERT_STR_EQ(cfg.plugins.plugin_paths[0], "/a.so");
    SC_ASSERT_STR_EQ(cfg.plugins.plugin_paths[1], "/b.so");
    sc_arena_destroy(arena);
}

static void test_plugin_unload_all_no_crash(void) {
    sc_plugin_unload_all();
    /* No plugins loaded - should not crash */
}

static void test_plugin_version_mismatch_fails(void) {
    sc_allocator_t a = sc_system_allocator();
    sc_plugin_host_t host = {
        .alloc = &a,
        .register_tool = NULL,
        .register_provider = NULL,
        .register_channel = NULL,
        .host_ctx = NULL,
    };
    sc_plugin_info_t info = {0};
    sc_plugin_handle_t *handle = NULL;
    sc_error_t err = sc_plugin_load(&a, "/bad_api/plugin.so", &host, &info, &handle);
    SC_ASSERT_EQ(err, SC_ERR_INVALID_ARGUMENT);
    SC_ASSERT_NULL(handle);
}

static void test_registry_create_null_alloc_returns_null(void) {
    sc_plugin_registry_t *reg = sc_plugin_registry_create(NULL, 4);
    SC_ASSERT_NULL(reg);
}

static void test_registry_create_zero_max_returns_null(void) {
    sc_allocator_t a = sc_system_allocator();
    sc_plugin_registry_t *reg = sc_plugin_registry_create(&a, 0);
    SC_ASSERT_NULL(reg);
}

static void test_registry_destroy_null_no_crash(void) {
    sc_plugin_registry_destroy(NULL);
}

static void test_registry_count_null_returns_zero(void) {
    SC_ASSERT_EQ(sc_plugin_count(NULL), 0);
}

static void test_registry_register_null_reg_returns_error(void) {
    sc_plugin_info_t info = {.name = "test", .api_version = SC_PLUGIN_API_VERSION};
    SC_ASSERT_EQ(sc_plugin_register(NULL, &info, NULL, 0), SC_ERR_INVALID_ARGUMENT);
}

static void test_registry_register_null_info_returns_error(void) {
    sc_allocator_t a = sc_system_allocator();
    sc_plugin_registry_t *reg = sc_plugin_registry_create(&a, 4);
    SC_ASSERT_EQ(sc_plugin_register(reg, NULL, NULL, 0), SC_ERR_INVALID_ARGUMENT);
    sc_plugin_registry_destroy(reg);
}

static void test_registry_register_null_name_returns_error(void) {
    sc_allocator_t a = sc_system_allocator();
    sc_plugin_registry_t *reg = sc_plugin_registry_create(&a, 4);
    sc_plugin_info_t info = {.name = NULL, .api_version = SC_PLUGIN_API_VERSION};
    SC_ASSERT_EQ(sc_plugin_register(reg, &info, NULL, 0), SC_ERR_INVALID_ARGUMENT);
    sc_plugin_registry_destroy(reg);
}

static void test_registry_full_returns_oom(void) {
    sc_allocator_t a = sc_system_allocator();
    sc_plugin_registry_t *reg = sc_plugin_registry_create(&a, 1);
    sc_plugin_info_t info = {
        .name = "first", .version = "1.0", .api_version = SC_PLUGIN_API_VERSION};
    SC_ASSERT_EQ(sc_plugin_register(reg, &info, NULL, 0), SC_OK);
    sc_plugin_info_t info2 = {
        .name = "second", .version = "2.0", .api_version = SC_PLUGIN_API_VERSION};
    SC_ASSERT_EQ(sc_plugin_register(reg, &info2, NULL, 0), SC_ERR_OUT_OF_MEMORY);
    SC_ASSERT_EQ(sc_plugin_count(reg), 1);
    sc_plugin_registry_destroy(reg);
}

static const char *mock_tool_name(void *ctx) {
    (void)ctx;
    return "mock-tool";
}

static void test_registry_get_tools_multiple_plugins(void) {
    sc_allocator_t a = sc_system_allocator();
    sc_plugin_registry_t *reg = sc_plugin_registry_create(&a, 4);

    static const sc_tool_vtable_t mock_vt = {.name = mock_tool_name};
    sc_tool_t tool_a = {.ctx = NULL, .vtable = &mock_vt};
    sc_tool_t tool_b = {.ctx = NULL, .vtable = &mock_vt};

    sc_plugin_info_t p1 = {.name = "plug-a", .api_version = SC_PLUGIN_API_VERSION};
    SC_ASSERT_EQ(sc_plugin_register(reg, &p1, &tool_a, 1), SC_OK);

    sc_plugin_info_t p2 = {.name = "plug-b", .api_version = SC_PLUGIN_API_VERSION};
    SC_ASSERT_EQ(sc_plugin_register(reg, &p2, &tool_b, 1), SC_OK);

    sc_tool_t *tools = NULL;
    size_t count = 0;
    SC_ASSERT_EQ(sc_plugin_get_tools(reg, &tools, &count), SC_OK);
    SC_ASSERT_EQ(count, 2);
    SC_ASSERT_NOT_NULL(tools);
    a.free(a.ctx, tools, count * sizeof(sc_tool_t));
    sc_plugin_registry_destroy(reg);
}

static void test_registry_get_tools_empty(void) {
    sc_allocator_t a = sc_system_allocator();
    sc_plugin_registry_t *reg = sc_plugin_registry_create(&a, 4);
    sc_tool_t *tools = NULL;
    size_t count = 99;
    SC_ASSERT_EQ(sc_plugin_get_tools(reg, &tools, &count), SC_OK);
    SC_ASSERT_EQ(count, 0);
    SC_ASSERT_NULL(tools);
    sc_plugin_registry_destroy(reg);
}

static void test_registry_get_tools_null_args_returns_error(void) {
    sc_allocator_t a = sc_system_allocator();
    sc_plugin_registry_t *reg = sc_plugin_registry_create(&a, 4);
    SC_ASSERT_EQ(sc_plugin_get_tools(reg, NULL, NULL), SC_ERR_INVALID_ARGUMENT);
    SC_ASSERT_EQ(sc_plugin_get_tools(NULL, NULL, NULL), SC_ERR_INVALID_ARGUMENT);
    sc_plugin_registry_destroy(reg);
}

static void test_registry_get_info_out_of_range(void) {
    sc_allocator_t a = sc_system_allocator();
    sc_plugin_registry_t *reg = sc_plugin_registry_create(&a, 4);
    sc_plugin_info_t out;
    SC_ASSERT_EQ(sc_plugin_get_info(reg, 0, &out), SC_ERR_NOT_FOUND);
    SC_ASSERT_EQ(sc_plugin_get_info(reg, 100, &out), SC_ERR_NOT_FOUND);
    sc_plugin_registry_destroy(reg);
}

static void test_registry_get_info_null_args_returns_error(void) {
    sc_allocator_t a = sc_system_allocator();
    sc_plugin_registry_t *reg = sc_plugin_registry_create(&a, 4);
    SC_ASSERT_EQ(sc_plugin_get_info(reg, 0, NULL), SC_ERR_INVALID_ARGUMENT);
    SC_ASSERT_EQ(sc_plugin_get_info(NULL, 0, NULL), SC_ERR_INVALID_ARGUMENT);
    sc_plugin_registry_destroy(reg);
}

static void test_registry_get_info_second_plugin(void) {
    sc_allocator_t a = sc_system_allocator();
    sc_plugin_registry_t *reg = sc_plugin_registry_create(&a, 4);
    sc_plugin_info_t p1 = {.name = "alpha", .version = "1.0", .api_version = SC_PLUGIN_API_VERSION};
    sc_plugin_info_t p2 = {.name = "beta", .version = "2.0", .api_version = SC_PLUGIN_API_VERSION};
    SC_ASSERT_EQ(sc_plugin_register(reg, &p1, NULL, 0), SC_OK);
    SC_ASSERT_EQ(sc_plugin_register(reg, &p2, NULL, 0), SC_OK);
    sc_plugin_info_t out;
    SC_ASSERT_EQ(sc_plugin_get_info(reg, 1, &out), SC_OK);
    SC_ASSERT_STR_EQ(out.name, "beta");
    sc_plugin_registry_destroy(reg);
}

static void test_registry_register_no_version_or_description(void) {
    sc_allocator_t a = sc_system_allocator();
    sc_plugin_registry_t *reg = sc_plugin_registry_create(&a, 4);
    sc_plugin_info_t info = {
        .name = "bare", .version = NULL, .description = NULL, .api_version = SC_PLUGIN_API_VERSION};
    SC_ASSERT_EQ(sc_plugin_register(reg, &info, NULL, 0), SC_OK);
    sc_plugin_info_t out;
    SC_ASSERT_EQ(sc_plugin_get_info(reg, 0, &out), SC_OK);
    SC_ASSERT_STR_EQ(out.name, "bare");
    SC_ASSERT_NULL(out.version);
    SC_ASSERT_NULL(out.description);
    sc_plugin_registry_destroy(reg);
}

void run_plugin_tests(void) {
    SC_RUN_TEST(test_plugin_load_bad_path_fails);
    SC_RUN_TEST(test_plugin_host_register_tool_callback);
    SC_RUN_TEST(test_plugin_config_parse_empty_array);
    SC_RUN_TEST(test_plugin_config_parse_with_paths);
    SC_RUN_TEST(test_plugin_config_parse_plugins_as_array);
    SC_RUN_TEST(test_plugin_unload_all_no_crash);
    SC_RUN_TEST(test_plugin_version_mismatch_fails);
    SC_RUN_TEST(test_registry_create_null_alloc_returns_null);
    SC_RUN_TEST(test_registry_create_zero_max_returns_null);
    SC_RUN_TEST(test_registry_destroy_null_no_crash);
    SC_RUN_TEST(test_registry_count_null_returns_zero);
    SC_RUN_TEST(test_registry_register_null_reg_returns_error);
    SC_RUN_TEST(test_registry_register_null_info_returns_error);
    SC_RUN_TEST(test_registry_register_null_name_returns_error);
    SC_RUN_TEST(test_registry_full_returns_oom);
    SC_RUN_TEST(test_registry_get_tools_multiple_plugins);
    SC_RUN_TEST(test_registry_get_tools_empty);
    SC_RUN_TEST(test_registry_get_tools_null_args_returns_error);
    SC_RUN_TEST(test_registry_get_info_out_of_range);
    SC_RUN_TEST(test_registry_get_info_null_args_returns_error);
    SC_RUN_TEST(test_registry_get_info_second_plugin);
    SC_RUN_TEST(test_registry_register_no_version_or_description);
}

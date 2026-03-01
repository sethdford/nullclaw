#include "test_framework.h"
#include "seaclaw/mcp.h"
#include "seaclaw/core/allocator.h"
#include <string.h>

static void test_mcp_server_create_destroy(void) {
    sc_allocator_t alloc = sc_system_allocator();
    sc_mcp_server_config_t cfg = { .command = "echo", .args = NULL, .args_count = 0 };
    sc_mcp_server_t *srv = sc_mcp_server_create(&alloc, &cfg);
    SC_ASSERT_NOT_NULL(srv);
    sc_mcp_server_destroy(srv);
}

static void test_mcp_server_connect_test_mode(void) {
    sc_allocator_t alloc = sc_system_allocator();
    sc_mcp_server_config_t cfg = { .command = "echo", .args = NULL, .args_count = 0 };
    sc_mcp_server_t *srv = sc_mcp_server_create(&alloc, &cfg);
    SC_ASSERT_NOT_NULL(srv);
    sc_error_t err = sc_mcp_server_connect(srv);
    SC_ASSERT_EQ(err, SC_OK);
    sc_mcp_server_destroy(srv);
}

static void test_mcp_server_list_tools_mock(void) {
    sc_allocator_t alloc = sc_system_allocator();
    sc_mcp_server_config_t cfg = { .command = "echo", .args = NULL, .args_count = 0 };
    sc_mcp_server_t *srv = sc_mcp_server_create(&alloc, &cfg);
    sc_mcp_server_connect(srv);
    char **names = NULL, **descs = NULL;
    size_t count = 0;
    sc_error_t err = sc_mcp_server_list_tools(srv, &alloc, &names, &descs, &count);
    SC_ASSERT_EQ(err, SC_OK);
    SC_ASSERT_EQ(count, 1u);
    SC_ASSERT_STR_EQ(names[0], "mock_tool");
    for (size_t i = 0; i < count; i++) {
        alloc.free(alloc.ctx, names[i], strlen(names[i]) + 1);
        alloc.free(alloc.ctx, descs[i], strlen(descs[i]) + 1);
    }
    alloc.free(alloc.ctx, names, count * sizeof(char *));
    alloc.free(alloc.ctx, descs, count * sizeof(char *));
    sc_mcp_server_destroy(srv);
}

static void test_mcp_server_call_tool_mock(void) {
    sc_allocator_t alloc = sc_system_allocator();
    sc_mcp_server_config_t cfg = { .command = "echo", .args = NULL, .args_count = 0 };
    sc_mcp_server_t *srv = sc_mcp_server_create(&alloc, &cfg);
    sc_mcp_server_connect(srv);
    char *result = NULL;
    size_t result_len = 0;
    sc_error_t err = sc_mcp_server_call_tool(srv, &alloc, "mock_tool", "{}",
        &result, &result_len);
    SC_ASSERT_EQ(err, SC_OK);
    SC_ASSERT_NOT_NULL(result);
    alloc.free(alloc.ctx, result, result_len + 1);
    sc_mcp_server_destroy(srv);
}

static void test_mcp_init_tools_mock(void) {
    sc_allocator_t alloc = sc_system_allocator();
    sc_mcp_server_config_t cfg = { .command = "echo", .args = NULL, .args_count = 0 };
    sc_tool_t *tools = NULL;
    size_t count = 0;
    sc_error_t err = sc_mcp_init_tools(&alloc, &cfg, 1, &tools, &count);
    SC_ASSERT_EQ(err, SC_OK);
    SC_ASSERT_TRUE(count >= 1);
    sc_mcp_free_tools(&alloc, tools, count);
}

void run_mcp_tests(void) {
    SC_TEST_SUITE("MCP");
    SC_RUN_TEST(test_mcp_server_create_destroy);
    SC_RUN_TEST(test_mcp_server_connect_test_mode);
    SC_RUN_TEST(test_mcp_server_list_tools_mock);
    SC_RUN_TEST(test_mcp_server_call_tool_mock);
    SC_RUN_TEST(test_mcp_init_tools_mock);
}

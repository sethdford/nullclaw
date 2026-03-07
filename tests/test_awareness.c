/* Awareness module tests — bus event tracking + context string */
#include "seaclaw/agent/awareness.h"
#include "seaclaw/bus.h"
#include "seaclaw/core/allocator.h"
#include "test_framework.h"
#include <string.h>

static void test_awareness_init_null(void) {
    sc_awareness_t aw;
    SC_ASSERT_EQ(sc_awareness_init(NULL, NULL), SC_ERR_INVALID_ARGUMENT);
    sc_bus_t bus;
    sc_bus_init(&bus);
    SC_ASSERT_EQ(sc_awareness_init(&aw, NULL), SC_ERR_INVALID_ARGUMENT);
    SC_ASSERT_EQ(sc_awareness_init(NULL, &bus), SC_ERR_INVALID_ARGUMENT);
}

static void test_awareness_init_deinit(void) {
    sc_bus_t bus;
    sc_bus_init(&bus);
    sc_awareness_t aw;
    sc_error_t err = sc_awareness_init(&aw, &bus);
    SC_ASSERT_EQ(err, SC_OK);
    SC_ASSERT_EQ(aw.state.messages_received, 0);
    SC_ASSERT_EQ(aw.state.messages_sent, 0);
    SC_ASSERT_EQ(aw.state.tool_calls, 0);
    SC_ASSERT_EQ(aw.state.total_errors, 0);
    SC_ASSERT_FALSE(aw.state.health_degraded);
    sc_awareness_deinit(&aw);
}

static void test_awareness_tracks_messages(void) {
    sc_bus_t bus;
    sc_bus_init(&bus);
    sc_awareness_t aw;
    sc_awareness_init(&aw, &bus);

    sc_bus_publish_simple(&bus, SC_BUS_MESSAGE_RECEIVED, "cli", "s1", "hello");
    sc_bus_publish_simple(&bus, SC_BUS_MESSAGE_RECEIVED, "cli", "s1", "world");
    sc_bus_publish_simple(&bus, SC_BUS_MESSAGE_SENT, "cli", "s1", "reply");
    SC_ASSERT_EQ(aw.state.messages_received, 2);
    SC_ASSERT_EQ(aw.state.messages_sent, 1);

    sc_awareness_deinit(&aw);
}

static void test_awareness_tracks_tool_calls(void) {
    sc_bus_t bus;
    sc_bus_init(&bus);
    sc_awareness_t aw;
    sc_awareness_init(&aw, &bus);

    sc_bus_publish_simple(&bus, SC_BUS_TOOL_CALL, "cli", "s1", "shell");
    sc_bus_publish_simple(&bus, SC_BUS_TOOL_CALL, "cli", "s1", "http");
    SC_ASSERT_EQ(aw.state.tool_calls, 2);

    sc_awareness_deinit(&aw);
}

static void test_awareness_tracks_errors(void) {
    sc_bus_t bus;
    sc_bus_init(&bus);
    sc_awareness_t aw;
    sc_awareness_init(&aw, &bus);

    sc_bus_publish_simple(&bus, SC_BUS_ERROR, "", "", "connection timeout");
    sc_bus_publish_simple(&bus, SC_BUS_ERROR, "", "", "rate limited");
    SC_ASSERT_EQ(aw.state.total_errors, 2);
    SC_ASSERT_EQ(aw.state.error_write_idx, 2);

    sc_awareness_deinit(&aw);
}

static void test_awareness_error_circular_buffer(void) {
    sc_bus_t bus;
    sc_bus_init(&bus);
    sc_awareness_t aw;
    sc_awareness_init(&aw, &bus);

    for (int i = 0; i < SC_AWARENESS_MAX_RECENT_ERRORS + 3; i++) {
        char msg[32];
        snprintf(msg, sizeof(msg), "error-%d", i);
        sc_bus_publish_simple(&bus, SC_BUS_ERROR, "", "", msg);
    }
    SC_ASSERT_EQ(aw.state.total_errors, (size_t)(SC_AWARENESS_MAX_RECENT_ERRORS + 3));
    SC_ASSERT_EQ(aw.state.error_write_idx, (size_t)(SC_AWARENESS_MAX_RECENT_ERRORS + 3));

    sc_awareness_deinit(&aw);
}

static void test_awareness_tracks_channels(void) {
    sc_bus_t bus;
    sc_bus_init(&bus);
    sc_awareness_t aw;
    sc_awareness_init(&aw, &bus);

    sc_bus_publish_simple(&bus, SC_BUS_MESSAGE_RECEIVED, "telegram", "u1", "hi");
    sc_bus_publish_simple(&bus, SC_BUS_MESSAGE_RECEIVED, "discord", "u2", "hey");
    sc_bus_publish_simple(&bus, SC_BUS_MESSAGE_RECEIVED, "telegram", "u3", "dup");
    SC_ASSERT_EQ(aw.state.active_channel_count, 2);
    SC_ASSERT_STR_EQ(aw.state.active_channels[0], "telegram");
    SC_ASSERT_STR_EQ(aw.state.active_channels[1], "discord");

    sc_awareness_deinit(&aw);
}

static void test_awareness_health_change(void) {
    sc_bus_t bus;
    sc_bus_init(&bus);
    sc_awareness_t aw;
    sc_awareness_init(&aw, &bus);

    sc_bus_publish_simple(&bus, SC_BUS_HEALTH_CHANGE, "", "", "degraded");
    SC_ASSERT_TRUE(aw.state.health_degraded);

    sc_bus_publish_simple(&bus, SC_BUS_HEALTH_CHANGE, "", "", "");
    SC_ASSERT_FALSE(aw.state.health_degraded);

    sc_awareness_deinit(&aw);
}

static void test_awareness_context_null_when_empty(void) {
    sc_bus_t bus;
    sc_bus_init(&bus);
    sc_awareness_t aw;
    sc_awareness_init(&aw, &bus);

    sc_allocator_t alloc = sc_system_allocator();
    size_t len = 0;
    char *ctx = sc_awareness_context(&aw, &alloc, &len);
    SC_ASSERT_NULL(ctx);
    SC_ASSERT_EQ(len, 0);

    sc_awareness_deinit(&aw);
}

static void test_awareness_context_with_activity(void) {
    sc_bus_t bus;
    sc_bus_init(&bus);
    sc_awareness_t aw;
    sc_awareness_init(&aw, &bus);

    sc_bus_publish_simple(&bus, SC_BUS_MESSAGE_RECEIVED, "cli", "s1", "hello");
    sc_bus_publish_simple(&bus, SC_BUS_TOOL_CALL, "cli", "s1", "shell");

    sc_allocator_t alloc = sc_system_allocator();
    size_t len = 0;
    char *ctx = sc_awareness_context(&aw, &alloc, &len);
    SC_ASSERT_NOT_NULL(ctx);
    SC_ASSERT_TRUE(len > 0);
    SC_ASSERT_TRUE(strstr(ctx, "Situational Awareness") != NULL);
    SC_ASSERT_TRUE(strstr(ctx, "1 msgs received") != NULL);
    alloc.free(alloc.ctx, ctx, len + 1);

    sc_awareness_deinit(&aw);
}

static void test_awareness_context_with_errors(void) {
    sc_bus_t bus;
    sc_bus_init(&bus);
    sc_awareness_t aw;
    sc_awareness_init(&aw, &bus);

    sc_bus_publish_simple(&bus, SC_BUS_ERROR, "", "", "timeout");

    sc_allocator_t alloc = sc_system_allocator();
    size_t len = 0;
    char *ctx = sc_awareness_context(&aw, &alloc, &len);
    SC_ASSERT_NOT_NULL(ctx);
    SC_ASSERT_TRUE(strstr(ctx, "Recent errors") != NULL);
    SC_ASSERT_TRUE(strstr(ctx, "timeout") != NULL);
    alloc.free(alloc.ctx, ctx, len + 1);

    sc_awareness_deinit(&aw);
}

static void test_awareness_context_health_degraded(void) {
    sc_bus_t bus;
    sc_bus_init(&bus);
    sc_awareness_t aw;
    sc_awareness_init(&aw, &bus);

    sc_bus_publish_simple(&bus, SC_BUS_HEALTH_CHANGE, "", "", "degraded");

    sc_allocator_t alloc = sc_system_allocator();
    size_t len = 0;
    char *ctx = sc_awareness_context(&aw, &alloc, &len);
    SC_ASSERT_NOT_NULL(ctx);
    SC_ASSERT_TRUE(strstr(ctx, "health is degraded") != NULL);
    alloc.free(alloc.ctx, ctx, len + 1);

    sc_awareness_deinit(&aw);
}

static void test_awareness_deinit_null_safe(void) {
    sc_awareness_deinit(NULL);
    sc_awareness_t aw;
    memset(&aw, 0, sizeof(aw));
    sc_awareness_deinit(&aw);
}

static void test_awareness_context_null_args(void) {
    SC_ASSERT_NULL(sc_awareness_context(NULL, NULL, NULL));
}

void run_awareness_tests(void) {
    SC_TEST_SUITE("Awareness");
    SC_RUN_TEST(test_awareness_init_null);
    SC_RUN_TEST(test_awareness_init_deinit);
    SC_RUN_TEST(test_awareness_tracks_messages);
    SC_RUN_TEST(test_awareness_tracks_tool_calls);
    SC_RUN_TEST(test_awareness_tracks_errors);
    SC_RUN_TEST(test_awareness_error_circular_buffer);
    SC_RUN_TEST(test_awareness_tracks_channels);
    SC_RUN_TEST(test_awareness_health_change);
    SC_RUN_TEST(test_awareness_context_null_when_empty);
    SC_RUN_TEST(test_awareness_context_with_activity);
    SC_RUN_TEST(test_awareness_context_with_errors);
    SC_RUN_TEST(test_awareness_context_health_degraded);
    SC_RUN_TEST(test_awareness_deinit_null_safe);
    SC_RUN_TEST(test_awareness_context_null_args);
}

#include "seaclaw/agent/commitment.h"
#include "seaclaw/agent/proactive.h"
#include "seaclaw/core/allocator.h"
#include "seaclaw/core/error.h"
#include "test_framework.h"
#include <string.h>

static void proactive_milestone_at_10_sessions(void) {
    sc_allocator_t alloc = sc_system_allocator();
    sc_proactive_result_t result;
    memset(&result, 0, sizeof(result));

    SC_ASSERT_EQ(sc_proactive_check(&alloc, 10, 14, &result), SC_OK);
    SC_ASSERT_TRUE(result.count > 0);

    bool has_milestone = false;
    for (size_t i = 0; i < result.count; i++) {
        if (result.actions[i].type == SC_PROACTIVE_MILESTONE) {
            has_milestone = true;
            SC_ASSERT_NOT_NULL(result.actions[i].message);
            SC_ASSERT_TRUE(strstr(result.actions[i].message, "session 10") != NULL);
            SC_ASSERT_TRUE(strstr(result.actions[i].message, "milestone") != NULL);
            break;
        }
    }
    SC_ASSERT_TRUE(has_milestone);

    sc_proactive_result_deinit(&result, &alloc);
}

static void proactive_morning_briefing_at_9am(void) {
    sc_allocator_t alloc = sc_system_allocator();
    sc_proactive_result_t result;
    memset(&result, 0, sizeof(result));

    SC_ASSERT_EQ(sc_proactive_check(&alloc, 5, 9, &result), SC_OK);
    SC_ASSERT_TRUE(result.count > 0);

    bool has_briefing = false;
    for (size_t i = 0; i < result.count; i++) {
        if (result.actions[i].type == SC_PROACTIVE_MORNING_BRIEFING) {
            has_briefing = true;
            SC_ASSERT_NOT_NULL(result.actions[i].message);
            SC_ASSERT_TRUE(strstr(result.actions[i].message, "Good morning") != NULL);
            break;
        }
    }
    SC_ASSERT_TRUE(has_briefing);

    sc_proactive_result_deinit(&result, &alloc);
}

static void proactive_no_milestone_at_7_sessions(void) {
    sc_allocator_t alloc = sc_system_allocator();
    sc_proactive_result_t result;
    memset(&result, 0, sizeof(result));

    SC_ASSERT_EQ(sc_proactive_check(&alloc, 7, 14, &result), SC_OK);

    bool has_milestone = false;
    for (size_t i = 0; i < result.count; i++) {
        if (result.actions[i].type == SC_PROACTIVE_MILESTONE) {
            has_milestone = true;
            break;
        }
    }
    SC_ASSERT_FALSE(has_milestone);

    sc_proactive_result_deinit(&result, &alloc);
}

static void proactive_commitment_follow_up(void) {
    sc_allocator_t alloc = sc_system_allocator();
    sc_proactive_result_t result;
    memset(&result, 0, sizeof(result));

    sc_commitment_t commitment;
    memset(&commitment, 0, sizeof(commitment));
    commitment.status = SC_COMMITMENT_ACTIVE;
    commitment.summary = "finish the report by Friday";
    commitment.summary_len = strlen(commitment.summary);
    commitment.created_at = "2024-01-15T10:00:00Z";

    SC_ASSERT_EQ(sc_proactive_check_extended(&alloc, 5, 14, &commitment, 1, NULL, NULL, 0,
                                             &result),
                 SC_OK);

    bool has_follow_up = false;
    for (size_t i = 0; i < result.count; i++) {
        if (result.actions[i].type == SC_PROACTIVE_COMMITMENT_FOLLOW_UP) {
            has_follow_up = true;
            SC_ASSERT_NOT_NULL(result.actions[i].message);
            SC_ASSERT_TRUE(strstr(result.actions[i].message, "finish the report") != NULL);
            SC_ASSERT_TRUE(strstr(result.actions[i].message, "follow up") != NULL);
            SC_ASSERT_EQ(result.actions[i].priority, 0.8);
            break;
        }
    }
    SC_ASSERT_TRUE(has_follow_up);

    sc_proactive_result_deinit(&result, &alloc);
}

static void proactive_pattern_insight(void) {
    sc_allocator_t alloc = sc_system_allocator();
    sc_proactive_result_t result;
    memset(&result, 0, sizeof(result));

    const char *subjects[] = {"exercise", "project deadlines"};
    const uint32_t counts[] = {7, 3};

    SC_ASSERT_EQ(sc_proactive_check_extended(&alloc, 5, 14, NULL, 0, subjects, counts, 2, &result),
                 SC_OK);

    bool has_insight = false;
    for (size_t i = 0; i < result.count; i++) {
        if (result.actions[i].type == SC_PROACTIVE_PATTERN_INSIGHT) {
            has_insight = true;
            SC_ASSERT_NOT_NULL(result.actions[i].message);
            SC_ASSERT_TRUE(strstr(result.actions[i].message, "exercise") != NULL);
            SC_ASSERT_TRUE(strstr(result.actions[i].message, "7 times") != NULL);
            SC_ASSERT_TRUE(strstr(result.actions[i].message, "important to you") != NULL);
            SC_ASSERT_EQ(result.actions[i].priority, 0.6);
            break;
        }
    }
    SC_ASSERT_TRUE(has_insight);

    sc_proactive_result_deinit(&result, &alloc);
}

static void proactive_extended_no_data(void) {
    sc_allocator_t alloc = sc_system_allocator();
    sc_proactive_result_t result_basic;
    sc_proactive_result_t result_extended;
    memset(&result_basic, 0, sizeof(result_basic));
    memset(&result_extended, 0, sizeof(result_extended));

    SC_ASSERT_EQ(sc_proactive_check(&alloc, 5, 14, &result_basic), SC_OK);
    SC_ASSERT_EQ(sc_proactive_check_extended(&alloc, 5, 14, NULL, 0, NULL, NULL, 0, &result_extended),
                 SC_OK);

    SC_ASSERT_EQ(result_basic.count, result_extended.count);
    for (size_t i = 0; i < result_basic.count; i++) {
        SC_ASSERT_EQ(result_basic.actions[i].type, result_extended.actions[i].type);
    }

    sc_proactive_result_deinit(&result_basic, &alloc);
    sc_proactive_result_deinit(&result_extended, &alloc);
}

static void proactive_build_context_formats(void) {
    sc_allocator_t alloc = sc_system_allocator();
    sc_proactive_result_t result;
    memset(&result, 0, sizeof(result));

    SC_ASSERT_EQ(sc_proactive_check(&alloc, 10, 9, &result), SC_OK);
    SC_ASSERT_TRUE(result.count >= 2);

    char *ctx = NULL;
    size_t ctx_len = 0;
    SC_ASSERT_EQ(sc_proactive_build_context(&result, &alloc, 8, &ctx, &ctx_len), SC_OK);
    SC_ASSERT_NOT_NULL(ctx);
    SC_ASSERT_TRUE(ctx_len > 0);
    SC_ASSERT_TRUE(strstr(ctx, "### Proactive Awareness") != NULL);
    SC_ASSERT_TRUE(strstr(ctx, "session 10") != NULL);
    SC_ASSERT_TRUE(strstr(ctx, "Good morning") != NULL);

    alloc.free(alloc.ctx, ctx, ctx_len + 1);
    sc_proactive_result_deinit(&result, &alloc);
}

static void proactive_check_null_out_fails(void) {
    sc_allocator_t alloc = sc_system_allocator();
    sc_error_t err = sc_proactive_check(&alloc, 5, 14, NULL);
    SC_ASSERT_EQ(err, SC_ERR_INVALID_ARGUMENT);
}

static void proactive_result_deinit_null_safe(void) {
    sc_allocator_t alloc = sc_system_allocator();
    sc_proactive_result_deinit(NULL, &alloc);
}

static void proactive_no_actions_at_normal_time(void) {
    sc_allocator_t alloc = sc_system_allocator();
    sc_proactive_result_t result;
    memset(&result, 0, sizeof(result));

    SC_ASSERT_EQ(sc_proactive_check(&alloc, 3, 14, &result), SC_OK);

    bool has_milestone = false;
    bool has_briefing = false;
    for (size_t i = 0; i < result.count; i++) {
        if (result.actions[i].type == SC_PROACTIVE_MILESTONE)
            has_milestone = true;
        if (result.actions[i].type == SC_PROACTIVE_MORNING_BRIEFING)
            has_briefing = true;
    }
    SC_ASSERT_FALSE(has_milestone);
    SC_ASSERT_FALSE(has_briefing);
    SC_ASSERT_TRUE(result.count >= 1u);

    sc_proactive_result_deinit(&result, &alloc);
}

void run_proactive_tests(void) {
    SC_TEST_SUITE("proactive");
    SC_RUN_TEST(proactive_milestone_at_10_sessions);
    SC_RUN_TEST(proactive_morning_briefing_at_9am);
    SC_RUN_TEST(proactive_no_milestone_at_7_sessions);
    SC_RUN_TEST(proactive_commitment_follow_up);
    SC_RUN_TEST(proactive_pattern_insight);
    SC_RUN_TEST(proactive_extended_no_data);
    SC_RUN_TEST(proactive_build_context_formats);
    SC_RUN_TEST(proactive_check_null_out_fails);
    SC_RUN_TEST(proactive_result_deinit_null_safe);
    SC_RUN_TEST(proactive_no_actions_at_normal_time);
}

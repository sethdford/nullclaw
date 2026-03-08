#include "seaclaw/agent/commitment.h"
#include "seaclaw/core/allocator.h"
#include "test_framework.h"
#include <string.h>

static void commitment_detects_promise(void) {
    sc_allocator_t alloc = sc_system_allocator();
    sc_commitment_detect_result_t result;
    const char *text = "I will finish the report";
    sc_error_t err =
        sc_commitment_detect(&alloc, text, strlen(text), "user", 4, &result);
    SC_ASSERT_EQ(err, SC_OK);
    SC_ASSERT_EQ(result.count, 1);
    SC_ASSERT_EQ(result.commitments[0].type, SC_COMMITMENT_PROMISE);
    SC_ASSERT_NOT_NULL(result.commitments[0].summary);
    SC_ASSERT_TRUE(strstr(result.commitments[0].summary, "finish the report") != NULL);
    sc_commitment_detect_result_deinit(&result, &alloc);
}

static void commitment_detects_intention(void) {
    sc_allocator_t alloc = sc_system_allocator();
    sc_commitment_detect_result_t result;
    const char *text = "I'm going to call her tomorrow";
    sc_error_t err =
        sc_commitment_detect(&alloc, text, strlen(text), "user", 4, &result);
    SC_ASSERT_EQ(err, SC_OK);
    SC_ASSERT_EQ(result.count, 1);
    SC_ASSERT_EQ(result.commitments[0].type, SC_COMMITMENT_INTENTION);
    sc_commitment_detect_result_deinit(&result, &alloc);
}

static void commitment_detects_reminder(void) {
    sc_allocator_t alloc = sc_system_allocator();
    sc_commitment_detect_result_t result;
    const char *text = "remind me to buy groceries";
    sc_error_t err =
        sc_commitment_detect(&alloc, text, strlen(text), "user", 4, &result);
    SC_ASSERT_EQ(err, SC_OK);
    SC_ASSERT_EQ(result.count, 1);
    SC_ASSERT_EQ(result.commitments[0].type, SC_COMMITMENT_REMINDER);
    sc_commitment_detect_result_deinit(&result, &alloc);
}

static void commitment_detects_goal(void) {
    sc_allocator_t alloc = sc_system_allocator();
    sc_commitment_detect_result_t result;
    const char *text = "I want to learn piano";
    sc_error_t err =
        sc_commitment_detect(&alloc, text, strlen(text), "user", 4, &result);
    SC_ASSERT_EQ(err, SC_OK);
    SC_ASSERT_EQ(result.count, 1);
    SC_ASSERT_EQ(result.commitments[0].type, SC_COMMITMENT_GOAL);
    sc_commitment_detect_result_deinit(&result, &alloc);
}

static void commitment_ignores_negation(void) {
    sc_allocator_t alloc = sc_system_allocator();
    sc_commitment_detect_result_t result;
    const char *text = "I will not do that";
    sc_error_t err =
        sc_commitment_detect(&alloc, text, strlen(text), "user", 4, &result);
    SC_ASSERT_EQ(err, SC_OK);
    SC_ASSERT_EQ(result.count, 0);
    sc_commitment_detect_result_deinit(&result, &alloc);
}

static void commitment_handles_empty_text(void) {
    sc_allocator_t alloc = sc_system_allocator();
    sc_commitment_detect_result_t result;
    sc_error_t err = sc_commitment_detect(&alloc, "", 0, "user", 4, &result);
    SC_ASSERT_EQ(err, SC_OK);
    SC_ASSERT_EQ(result.count, 0);
    sc_commitment_detect_result_deinit(&result, &alloc);
}

void run_commitment_tests(void) {
    SC_TEST_SUITE("commitment");
    SC_RUN_TEST(commitment_detects_promise);
    SC_RUN_TEST(commitment_detects_intention);
    SC_RUN_TEST(commitment_detects_reminder);
    SC_RUN_TEST(commitment_detects_goal);
    SC_RUN_TEST(commitment_ignores_negation);
    SC_RUN_TEST(commitment_handles_empty_text);
}

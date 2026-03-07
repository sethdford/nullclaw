/* Reflection module tests — quality evaluation + critique prompt */
#include "seaclaw/agent/reflection.h"
#include "seaclaw/core/allocator.h"
#include "test_framework.h"
#include <string.h>

static void test_reflection_null_response(void) {
    SC_ASSERT_EQ(sc_reflection_evaluate("q", 1, NULL, 0, NULL), SC_QUALITY_NEEDS_RETRY);
}

static void test_reflection_empty_response(void) {
    SC_ASSERT_EQ(sc_reflection_evaluate("q", 1, "", 0, NULL), SC_QUALITY_NEEDS_RETRY);
}

static void test_reflection_trivial_short_response(void) {
    SC_ASSERT_EQ(sc_reflection_evaluate("q", 1, "ok", 2, NULL), SC_QUALITY_NEEDS_RETRY);
    SC_ASSERT_EQ(sc_reflection_evaluate("q", 1, "12345678", 8, NULL), SC_QUALITY_NEEDS_RETRY);
}

static void test_reflection_refusal_i_cannot(void) {
    const char *resp = "I cannot help with that request due to policy restrictions.";
    SC_ASSERT_EQ(sc_reflection_evaluate("help me", 7, resp, strlen(resp), NULL),
                 SC_QUALITY_ACCEPTABLE);
}

static void test_reflection_refusal_i_cant(void) {
    const char *resp = "I can't assist with this kind of task.";
    SC_ASSERT_EQ(sc_reflection_evaluate("help me", 7, resp, strlen(resp), NULL),
                 SC_QUALITY_ACCEPTABLE);
}

static void test_reflection_refusal_unable(void) {
    const char *resp = "I'm unable to perform that operation right now.";
    SC_ASSERT_EQ(sc_reflection_evaluate("help me", 7, resp, strlen(resp), NULL),
                 SC_QUALITY_ACCEPTABLE);
}

static void test_reflection_refusal_as_an_ai(void) {
    const char *resp = "As an AI language model, I don't have access to your files.";
    SC_ASSERT_EQ(sc_reflection_evaluate("help me", 7, resp, strlen(resp), NULL),
                 SC_QUALITY_ACCEPTABLE);
}

static void test_reflection_question_short_answer(void) {
    const char *resp = "Yes, that's correct.";
    SC_ASSERT_EQ(sc_reflection_evaluate("Is C faster than Python?", 24, resp, strlen(resp), NULL),
                 SC_QUALITY_ACCEPTABLE);
}

static void test_reflection_question_good_answer(void) {
    const char *resp =
        "C is generally faster than Python because it compiles to native machine code "
        "while Python is interpreted. However, Python with C extensions or NumPy can be "
        "competitive.";
    SC_ASSERT_EQ(sc_reflection_evaluate("Is C faster than Python?", 24, resp, strlen(resp), NULL),
                 SC_QUALITY_GOOD);
}

static void test_reflection_no_question_good(void) {
    const char *resp = "Here is the implementation for the sorting algorithm you requested.";
    SC_ASSERT_EQ(sc_reflection_evaluate("sort this array", 15, resp, strlen(resp), NULL),
                 SC_QUALITY_GOOD);
}

static void test_reflection_min_tokens_below(void) {
    sc_reflection_config_t cfg = {.enabled = true, .min_response_tokens = 20, .max_retries = 1};
    const char *resp = "Short reply here.";
    SC_ASSERT_EQ(sc_reflection_evaluate("tell me more", 12, resp, strlen(resp), &cfg),
                 SC_QUALITY_ACCEPTABLE);
}

static void test_reflection_min_tokens_above(void) {
    sc_reflection_config_t cfg = {.enabled = true, .min_response_tokens = 5, .max_retries = 1};
    const char *resp =
        "This is a longer response that contains more than five words and should pass.";
    SC_ASSERT_EQ(sc_reflection_evaluate("tell me more", 12, resp, strlen(resp), &cfg),
                 SC_QUALITY_GOOD);
}

static void test_reflection_case_insensitive_refusal(void) {
    const char *resp = "I CANNOT fulfill that request under current policy.";
    SC_ASSERT_EQ(sc_reflection_evaluate("help", 4, resp, strlen(resp), NULL),
                 SC_QUALITY_ACCEPTABLE);
}

static void test_reflection_critique_prompt_null(void) {
    SC_ASSERT_EQ(sc_reflection_build_critique_prompt(NULL, "q", 1, "r", 1, NULL, NULL),
                 SC_ERR_INVALID_ARGUMENT);
}

static void test_reflection_critique_prompt_basic(void) {
    sc_allocator_t alloc = sc_system_allocator();
    char *prompt = NULL;
    size_t prompt_len = 0;

    sc_error_t err = sc_reflection_build_critique_prompt(
        &alloc, "What is C?", 10, "C is a language.", 16, &prompt, &prompt_len);
    SC_ASSERT_EQ(err, SC_OK);
    SC_ASSERT_NOT_NULL(prompt);
    SC_ASSERT_TRUE(prompt_len > 0);
    SC_ASSERT_TRUE(strstr(prompt, "What is C?") != NULL);
    SC_ASSERT_TRUE(strstr(prompt, "C is a language.") != NULL);
    SC_ASSERT_TRUE(strstr(prompt, "Evaluate") != NULL);
    alloc.free(alloc.ctx, prompt, prompt_len + 1);
}

static void test_reflection_critique_prompt_empty_query(void) {
    sc_allocator_t alloc = sc_system_allocator();
    char *prompt = NULL;
    size_t prompt_len = 0;

    sc_error_t err =
        sc_reflection_build_critique_prompt(&alloc, NULL, 0, "response", 8, &prompt, &prompt_len);
    SC_ASSERT_EQ(err, SC_OK);
    SC_ASSERT_NOT_NULL(prompt);
    SC_ASSERT_TRUE(strstr(prompt, "response") != NULL);
    alloc.free(alloc.ctx, prompt, prompt_len + 1);
}

static void test_reflection_result_free_null_safe(void) {
    sc_reflection_result_free(NULL, NULL);
    sc_allocator_t alloc = sc_system_allocator();
    sc_reflection_result_free(&alloc, NULL);
    sc_reflection_result_t r = {.quality = SC_QUALITY_GOOD, .feedback = NULL, .feedback_len = 0};
    sc_reflection_result_free(&alloc, &r);
}

static void test_reflection_result_free_with_feedback(void) {
    sc_allocator_t alloc = sc_system_allocator();
    char *fb = (char *)alloc.alloc(alloc.ctx, 16);
    memcpy(fb, "needs more info", 15);
    fb[15] = '\0';

    sc_reflection_result_t r = {
        .quality = SC_QUALITY_ACCEPTABLE, .feedback = fb, .feedback_len = 15};
    sc_reflection_result_free(&alloc, &r);
    SC_ASSERT_NULL(r.feedback);
    SC_ASSERT_EQ(r.feedback_len, 0);
}

void run_reflection_tests(void) {
    SC_TEST_SUITE("Reflection");
    SC_RUN_TEST(test_reflection_null_response);
    SC_RUN_TEST(test_reflection_empty_response);
    SC_RUN_TEST(test_reflection_trivial_short_response);
    SC_RUN_TEST(test_reflection_refusal_i_cannot);
    SC_RUN_TEST(test_reflection_refusal_i_cant);
    SC_RUN_TEST(test_reflection_refusal_unable);
    SC_RUN_TEST(test_reflection_refusal_as_an_ai);
    SC_RUN_TEST(test_reflection_question_short_answer);
    SC_RUN_TEST(test_reflection_question_good_answer);
    SC_RUN_TEST(test_reflection_no_question_good);
    SC_RUN_TEST(test_reflection_min_tokens_below);
    SC_RUN_TEST(test_reflection_min_tokens_above);
    SC_RUN_TEST(test_reflection_case_insensitive_refusal);
    SC_RUN_TEST(test_reflection_critique_prompt_null);
    SC_RUN_TEST(test_reflection_critique_prompt_basic);
    SC_RUN_TEST(test_reflection_critique_prompt_empty_query);
    SC_RUN_TEST(test_reflection_result_free_null_safe);
    SC_RUN_TEST(test_reflection_result_free_with_feedback);
}

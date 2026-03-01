/* Skills module tests. API is currently a stub (list returns 0, free is no-op). */
#include "test_framework.h"
#include "seaclaw/skills.h"
#include "seaclaw/core/allocator.h"
#include "seaclaw/core/error.h"
#include <string.h>

static void test_skills_list_returns_empty(void) {
    sc_allocator_t alloc = sc_system_allocator();
    sc_skill_t *skills = NULL;
    size_t count = 99;
    sc_error_t err = sc_skills_list(&alloc, "/tmp", &skills, &count);
    SC_ASSERT_EQ(err, SC_OK);
    SC_ASSERT_NULL(skills);
    SC_ASSERT_EQ(count, 0u);
}

static void test_skills_list_null_workspace_ok(void) {
    sc_allocator_t alloc = sc_system_allocator();
    sc_skill_t *skills = NULL;
    size_t count = 0;
    sc_error_t err = sc_skills_list(&alloc, NULL, &skills, &count);
    SC_ASSERT_EQ(err, SC_OK);
    SC_ASSERT_NULL(skills);
    SC_ASSERT_EQ(count, 0u);
}

static void test_skills_free_null_safe(void) {
    sc_allocator_t alloc = sc_system_allocator();
    sc_skills_free(&alloc, NULL, 0);
}

static void test_skills_free_with_null_skills(void) {
    sc_allocator_t alloc = sc_system_allocator();
    sc_skills_free(&alloc, NULL, 5);
}

static void test_skills_list_resets_output(void) {
    sc_allocator_t alloc = sc_system_allocator();
    sc_skill_t *skills = (sc_skill_t *)0xdeadbeef;
    size_t count = 42;
    sc_skills_list(&alloc, ".", &skills, &count);
    SC_ASSERT_NULL(skills);
    SC_ASSERT_EQ(count, 0u);
}

static void test_skills_list_null_out_returns_error(void) {
    sc_allocator_t alloc = sc_system_allocator();
    sc_skill_t *skills = NULL;
    size_t count = 0;
    sc_error_t err = sc_skills_list(&alloc, "/tmp", NULL, &count);
    SC_ASSERT_NEQ(err, SC_OK);
    err = sc_skills_list(&alloc, "/tmp", &skills, NULL);
    SC_ASSERT_NEQ(err, SC_OK);
}

static void test_skills_list_empty_workspace_path(void) {
    sc_allocator_t alloc = sc_system_allocator();
    sc_skill_t *skills = NULL;
    size_t count = 0;
    sc_error_t err = sc_skills_list(&alloc, "", &skills, &count);
    SC_ASSERT_EQ(err, SC_OK);
    SC_ASSERT_NULL(skills);
    SC_ASSERT_EQ(count, 0u);
}

static void test_skills_list_dot_workspace(void) {
    sc_allocator_t alloc = sc_system_allocator();
    sc_skill_t *skills = NULL;
    size_t count = 0;
    sc_error_t err = sc_skills_list(&alloc, ".", &skills, &count);
    SC_ASSERT_EQ(err, SC_OK);
    SC_ASSERT_NULL(skills);
    SC_ASSERT_EQ(count, 0u);
}

void run_skills_tests(void) {
    SC_TEST_SUITE("Skills");
    SC_RUN_TEST(test_skills_list_returns_empty);
    SC_RUN_TEST(test_skills_list_null_workspace_ok);
    SC_RUN_TEST(test_skills_free_null_safe);
    SC_RUN_TEST(test_skills_free_with_null_skills);
    SC_RUN_TEST(test_skills_list_resets_output);
    SC_RUN_TEST(test_skills_list_null_out_returns_error);
    SC_RUN_TEST(test_skills_list_empty_workspace_path);
    SC_RUN_TEST(test_skills_list_dot_workspace);
}

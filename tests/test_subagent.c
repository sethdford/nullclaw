#include "test_framework.h"
#include "seaclaw/subagent.h"
#include "seaclaw/core/allocator.h"
#include "seaclaw/config.h"
#include <string.h>

static void test_subagent_create_destroy(void) {
    sc_allocator_t alloc = sc_system_allocator();
    sc_subagent_manager_t *mgr = sc_subagent_create(&alloc, NULL);
    SC_ASSERT_NOT_NULL(mgr);
    sc_subagent_destroy(&alloc, mgr);
}

static void test_subagent_spawn_test_mode(void) {
    sc_allocator_t alloc = sc_system_allocator();
    sc_subagent_manager_t *mgr = sc_subagent_create(&alloc, NULL);
    SC_ASSERT_NOT_NULL(mgr);
    uint64_t task_id = 0;
    sc_error_t err = sc_subagent_spawn(mgr, "test task", 9, "test", NULL, NULL, &task_id);
    SC_ASSERT_EQ(err, SC_OK);
    SC_ASSERT(task_id > 0);
    /* In test mode, task completes synchronously */
    sc_task_status_t status = sc_subagent_get_status(mgr, task_id);
    SC_ASSERT_EQ(status, SC_TASK_COMPLETED);
    sc_subagent_destroy(&alloc, mgr);
}

static void test_subagent_get_result(void) {
    sc_allocator_t alloc = sc_system_allocator();
    sc_subagent_manager_t *mgr = sc_subagent_create(&alloc, NULL);
    uint64_t task_id = 0;
    sc_subagent_spawn(mgr, "hello", 5, "greet", NULL, NULL, &task_id);
    const char *result = sc_subagent_get_result(mgr, task_id);
    SC_ASSERT_NOT_NULL(result);
    SC_ASSERT_STR_EQ(result, "completed: hello");
    sc_subagent_destroy(&alloc, mgr);
}

static void test_subagent_running_count(void) {
    sc_allocator_t alloc = sc_system_allocator();
    sc_subagent_manager_t *mgr = sc_subagent_create(&alloc, NULL);
    /* In test mode all complete immediately so running count stays 0 */
    size_t count = sc_subagent_running_count(mgr);
    SC_ASSERT_EQ(count, 0u);
    sc_subagent_destroy(&alloc, mgr);
}

static void test_subagent_invalid_task_id(void) {
    sc_allocator_t alloc = sc_system_allocator();
    sc_subagent_manager_t *mgr = sc_subagent_create(&alloc, NULL);
    sc_task_status_t status = sc_subagent_get_status(mgr, 999);
    SC_ASSERT_EQ(status, SC_TASK_FAILED); /* not found */
    const char *result = sc_subagent_get_result(mgr, 999);
    SC_ASSERT(result == NULL);
    sc_subagent_destroy(&alloc, mgr);
}

void run_subagent_tests(void) {
    SC_TEST_SUITE("Subagent");
    SC_RUN_TEST(test_subagent_create_destroy);
    SC_RUN_TEST(test_subagent_spawn_test_mode);
    SC_RUN_TEST(test_subagent_get_result);
    SC_RUN_TEST(test_subagent_running_count);
    SC_RUN_TEST(test_subagent_invalid_task_id);
}

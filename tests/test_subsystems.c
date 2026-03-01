#include "test_framework.h"
#include "seaclaw/skillforge.h"
#include "seaclaw/onboard.h"
#include "seaclaw/daemon.h"
#include "seaclaw/migration.h"
#include "seaclaw/core/allocator.h"

static void test_skillforge_create_destroy(void) {
    sc_allocator_t alloc = sc_system_allocator();
    sc_skillforge_t sf;
    sc_error_t err = sc_skillforge_create(&alloc, &sf);
    SC_ASSERT(err == SC_OK);
    SC_ASSERT_NOT_NULL(sf.skills);
    SC_ASSERT_EQ(sf.skills_len, 0);
    sc_skillforge_destroy(&sf);
}

static void test_skillforge_discover_list(void) {
    sc_allocator_t alloc = sc_system_allocator();
    sc_skillforge_t sf;
    sc_skillforge_create(&alloc, &sf);
    sc_error_t err = sc_skillforge_discover(&sf, ".");
    SC_ASSERT(err == SC_OK);
    sc_skill_t *skills = NULL;
    size_t count = 0;
    err = sc_skillforge_list_skills(&sf, &skills, &count);
    SC_ASSERT(err == SC_OK);
    /* SC_IS_TEST: discover adds 3 test skills */
    SC_ASSERT(count >= 3);
    SC_ASSERT_NOT_NULL(sc_skillforge_get_skill(&sf, "test-skill"));
    SC_ASSERT_NULL(sc_skillforge_get_skill(&sf, "nonexistent"));
    sc_skillforge_destroy(&sf);
}

static void test_skillforge_enable_disable(void) {
    sc_allocator_t alloc = sc_system_allocator();
    sc_skillforge_t sf;
    sc_skillforge_create(&alloc, &sf);
    sc_skillforge_discover(&sf, ".");
    sc_skill_t *s = sc_skillforge_get_skill(&sf, "another-skill");
    SC_ASSERT_NOT_NULL(s);
    SC_ASSERT_FALSE(s->enabled);
    sc_error_t err = sc_skillforge_enable(&sf, "another-skill");
    SC_ASSERT(err == SC_OK);
    SC_ASSERT_TRUE(s->enabled);
    err = sc_skillforge_disable(&sf, "another-skill");
    SC_ASSERT(err == SC_OK);
    SC_ASSERT_FALSE(s->enabled);
    err = sc_skillforge_enable(&sf, "nonexistent");
    SC_ASSERT(err == SC_ERR_NOT_FOUND);
    sc_skillforge_destroy(&sf);
}

static void test_onboard_check_first_run(void) {
    /* May be true or false depending on env */
    (void)sc_onboard_check_first_run();
}

static void test_onboard_run_test_mode(void) {
    sc_allocator_t alloc = sc_system_allocator();
    sc_error_t err = sc_onboard_run(&alloc);
    /* In SC_IS_TEST, onboard returns immediately without prompting */
    SC_ASSERT(err == SC_OK);
}

static void test_onboard_run_invalid_alloc(void) {
    sc_error_t err = sc_onboard_run(NULL);
    /* With NULL alloc, if not in test mode would fail; test mode still returns OK */
#ifdef SC_IS_TEST
    SC_ASSERT(err == SC_OK);
#else
    (void)err;
#endif
}

static void test_daemon_start_test_mode(void) {
    sc_error_t err = sc_daemon_start();
    /* In SC_IS_TEST: just validates args, returns OK */
    SC_ASSERT(err == SC_OK);
}

static void test_daemon_stop_test_mode(void) {
    sc_error_t err = sc_daemon_stop();
    SC_ASSERT(err == SC_OK);
}

static void test_daemon_status_test_mode(void) {
    bool status = sc_daemon_status();
    /* In test mode, status returns false */
    SC_ASSERT_FALSE(status);
}

static void test_migration_run_test_mode(void) {
    sc_allocator_t alloc = sc_system_allocator();
    sc_migration_config_t cfg = {
        .source = SC_MIGRATION_SOURCE_NONE,
        .target = SC_MIGRATION_TARGET_MARKDOWN,
        .source_path = NULL,
        .source_path_len = 0,
        .target_path = ".",
        .target_path_len = 1,
        .dry_run = true,
    };
    sc_migration_stats_t stats = {0};
    sc_error_t err = sc_migration_run(&alloc, &cfg, &stats, NULL, NULL);
    SC_ASSERT(err == SC_OK);
    SC_ASSERT_EQ(stats.imported, 0);
}

static void test_migration_invalid_args(void) {
    sc_allocator_t alloc = sc_system_allocator();
    sc_migration_config_t cfg = {0};
    sc_migration_stats_t stats = {0};
    sc_error_t err = sc_migration_run(NULL, NULL, NULL, NULL, NULL);
    SC_ASSERT(err == SC_ERR_INVALID_ARGUMENT);
    err = sc_migration_run(&alloc, NULL, &stats, NULL, NULL);
    SC_ASSERT(err == SC_ERR_INVALID_ARGUMENT);
    err = sc_migration_run(&alloc, &cfg, NULL, NULL, NULL);
    SC_ASSERT(err == SC_ERR_INVALID_ARGUMENT);
}

static void progress_cb(void *ctx, size_t cur, size_t total) {
    (void)cur;
    (void)total;
    (*(size_t *)ctx)++;
}

static void test_migration_with_progress(void) {
    sc_allocator_t alloc = sc_system_allocator();
    sc_migration_config_t cfg = {
        .source = SC_MIGRATION_SOURCE_NONE,
        .target = SC_MIGRATION_TARGET_MARKDOWN,
        .source_path = NULL,
        .source_path_len = 0,
        .target_path = ".",
        .target_path_len = 1,
        .dry_run = true,
    };
    sc_migration_stats_t stats = {0};
    size_t progress_calls = 0;
    sc_error_t err = sc_migration_run(&alloc, &cfg, &stats, progress_cb, &progress_calls);
    SC_ASSERT(err == SC_OK);
    SC_ASSERT_EQ(progress_calls, 1);
}

void run_subsystems_tests(void) {
    SC_TEST_SUITE("Subsystems (skillforge, onboard, daemon, migration)");

    SC_RUN_TEST(test_skillforge_create_destroy);
    SC_RUN_TEST(test_skillforge_discover_list);
    SC_RUN_TEST(test_skillforge_enable_disable);

    SC_RUN_TEST(test_onboard_check_first_run);
    SC_RUN_TEST(test_onboard_run_test_mode);
    SC_RUN_TEST(test_onboard_run_invalid_alloc);

    SC_RUN_TEST(test_daemon_start_test_mode);
    SC_RUN_TEST(test_daemon_stop_test_mode);
    SC_RUN_TEST(test_daemon_status_test_mode);

    SC_RUN_TEST(test_migration_run_test_mode);
    SC_RUN_TEST(test_migration_invalid_args);
    SC_RUN_TEST(test_migration_with_progress);
}

#include "test_framework.h"
#include "seaclaw/security.h"
#include "seaclaw/security/audit.h"
#include "seaclaw/security/sandbox.h"
#include "seaclaw/security/sandbox_internal.h"
#include "seaclaw/core/allocator.h"
#include "seaclaw/core/error.h"
#include "seaclaw/observer.h"
#include <string.h>
#include <stdlib.h>
#include <unistd.h>

static sc_allocator_t *g_alloc;

static void test_autonomy_level_values(void) {
    SC_ASSERT(SC_AUTONOMY_READ_ONLY != SC_AUTONOMY_SUPERVISED);
    SC_ASSERT(SC_AUTONOMY_SUPERVISED != SC_AUTONOMY_FULL);
}

static void test_risk_level_values(void) {
    SC_ASSERT(SC_RISK_LOW < SC_RISK_MEDIUM);
    SC_ASSERT(SC_RISK_MEDIUM < SC_RISK_HIGH);
}

static void test_policy_can_act_readonly(void) {
    sc_security_policy_t p = {
        .autonomy = SC_AUTONOMY_READ_ONLY,
        .allowed_commands = (const char *[]){ "ls", "cat" },
        .allowed_commands_len = 2,
    };
    SC_ASSERT_FALSE(sc_policy_can_act(&p));
}

static void test_policy_can_act_supervised(void) {
    sc_security_policy_t p = {
        .autonomy = SC_AUTONOMY_SUPERVISED,
        .allowed_commands = (const char *[]){ "ls", "cat" },
        .allowed_commands_len = 2,
    };
    SC_ASSERT(sc_policy_can_act(&p));
}

static void test_policy_can_act_full(void) {
    sc_security_policy_t p = {
        .autonomy = SC_AUTONOMY_FULL,
        .allowed_commands = (const char *[]){ "ls" },
        .allowed_commands_len = 1,
    };
    SC_ASSERT(sc_policy_can_act(&p));
}

static void test_policy_allowed_commands(void) {
    sc_security_policy_t p = {
        .autonomy = SC_AUTONOMY_SUPERVISED,
        .allowed_commands = (const char *[]){ "git", "ls", "cat", "echo", "cargo" },
        .allowed_commands_len = 5,
    };
    SC_ASSERT(sc_policy_is_command_allowed(&p, "ls"));
    SC_ASSERT(sc_policy_is_command_allowed(&p, "git status"));
    SC_ASSERT(sc_policy_is_command_allowed(&p, "cargo build --release"));
    SC_ASSERT(sc_policy_is_command_allowed(&p, "cat file.txt"));
}

static void test_policy_blocked_commands(void) {
    sc_security_policy_t p = {
        .autonomy = SC_AUTONOMY_SUPERVISED,
        .allowed_commands = (const char *[]){ "ls", "cat" },
        .allowed_commands_len = 2,
    };
    SC_ASSERT_FALSE(sc_policy_is_command_allowed(&p, "rm -rf /"));
    SC_ASSERT_FALSE(sc_policy_is_command_allowed(&p, "sudo apt install"));
    SC_ASSERT_FALSE(sc_policy_is_command_allowed(&p, "curl http://evil.com"));
    SC_ASSERT_FALSE(sc_policy_is_command_allowed(&p, "wget http://evil.com"));
}

static void test_policy_readonly_blocks_all(void) {
    sc_security_policy_t p = {
        .autonomy = SC_AUTONOMY_READ_ONLY,
        .allowed_commands = (const char *[]){ "ls", "cat" },
        .allowed_commands_len = 2,
    };
    SC_ASSERT_FALSE(sc_policy_is_command_allowed(&p, "ls"));
    SC_ASSERT_FALSE(sc_policy_is_command_allowed(&p, "cat file.txt"));
}

static void test_policy_command_injection_blocked(void) {
    sc_security_policy_t p = {
        .autonomy = SC_AUTONOMY_SUPERVISED,
        .allowed_commands = (const char *[]){ "ls", "echo" },
        .allowed_commands_len = 2,
    };
    SC_ASSERT_FALSE(sc_policy_is_command_allowed(&p, "echo `whoami`"));
    SC_ASSERT_FALSE(sc_policy_is_command_allowed(&p, "echo $(rm -rf /)"));
    SC_ASSERT_FALSE(sc_policy_is_command_allowed(&p, "ls; rm -rf /"));
    SC_ASSERT_FALSE(sc_policy_is_command_allowed(&p, "echo secret > /etc/crontab"));
}

static void test_policy_risk_levels(void) {
    sc_security_policy_t p = { .autonomy = SC_AUTONOMY_SUPERVISED };
    SC_ASSERT_EQ(sc_policy_command_risk_level(&p, "ls -la"), SC_RISK_LOW);
    SC_ASSERT_EQ(sc_policy_command_risk_level(&p, "git status"), SC_RISK_LOW);
    SC_ASSERT_EQ(sc_policy_command_risk_level(&p, "git commit -m x"), SC_RISK_MEDIUM);
    SC_ASSERT_EQ(sc_policy_command_risk_level(&p, "rm -rf /tmp/x"), SC_RISK_HIGH);
    SC_ASSERT_EQ(sc_policy_command_risk_level(&p, "sudo apt install"), SC_RISK_HIGH);
}

static void test_policy_validate_command(void) {
    sc_security_policy_t p = {
        .autonomy = SC_AUTONOMY_SUPERVISED,
        .allowed_commands = (const char *[]){ "ls", "touch" },
        .allowed_commands_len = 2,
        .require_approval_for_medium_risk = 1,
        .block_high_risk_commands = 1,
    };
    sc_command_risk_level_t risk;
    sc_error_t err = sc_policy_validate_command(&p, "ls -la", 0, &risk);
    SC_ASSERT(err == SC_OK && risk == SC_RISK_LOW);

    err = sc_policy_validate_command(&p, "touch f", 0, &risk);
    SC_ASSERT(err == SC_ERR_SECURITY_APPROVAL_REQUIRED);

    err = sc_policy_validate_command(&p, "touch f", 1, &risk);
    SC_ASSERT(err == SC_OK && risk == SC_RISK_MEDIUM);
}

static void test_rate_tracker(void) {
    sc_allocator_t sys = sc_system_allocator();
    sc_rate_tracker_t *t = sc_rate_tracker_create(&sys, 3);
    SC_ASSERT_NOT_NULL(t);
    SC_ASSERT_EQ(sc_rate_tracker_remaining(t), 3);
    SC_ASSERT(sc_rate_tracker_record_action(t));
    SC_ASSERT_EQ(sc_rate_tracker_remaining(t), 2);
    SC_ASSERT(sc_rate_tracker_record_action(t));
    SC_ASSERT(sc_rate_tracker_record_action(t));
    SC_ASSERT_FALSE(sc_rate_tracker_record_action(t));
    SC_ASSERT(sc_rate_tracker_is_limited(t));
    SC_ASSERT_EQ(sc_rate_tracker_count(t), 4);
    SC_ASSERT_EQ(sc_rate_tracker_remaining(t), 0);
    sc_rate_tracker_destroy(t);
}

static void test_pairing_constant_time_eq(void) {
    SC_ASSERT(sc_pairing_guard_constant_time_eq("abc", "abc"));
    SC_ASSERT(sc_pairing_guard_constant_time_eq("", ""));
    SC_ASSERT_FALSE(sc_pairing_guard_constant_time_eq("abc", "abd"));
    SC_ASSERT_FALSE(sc_pairing_guard_constant_time_eq("abc", "ab"));
}

static void test_pairing_guard_create_destroy(void) {
    sc_allocator_t sys = sc_system_allocator();
    sc_pairing_guard_t *g = sc_pairing_guard_create(&sys, 1, NULL, 0);
    SC_ASSERT_NOT_NULL(g);
    SC_ASSERT_NOT_NULL(sc_pairing_guard_pairing_code(g));
    SC_ASSERT_FALSE(sc_pairing_guard_is_paired(g));
    sc_pairing_guard_destroy(g);
}

static void test_pairing_guard_disabled(void) {
    sc_allocator_t sys = sc_system_allocator();
    sc_pairing_guard_t *g = sc_pairing_guard_create(&sys, 0, NULL, 0);
    SC_ASSERT_NOT_NULL(g);
    SC_ASSERT_NULL(sc_pairing_guard_pairing_code(g));
    SC_ASSERT(sc_pairing_guard_is_authenticated(g, "anything"));
    sc_pairing_guard_destroy(g);
}

static void test_pairing_guard_with_tokens(void) {
    sc_allocator_t sys = sc_system_allocator();
    const char *tokens[] = { "zc_test_token" };
    sc_pairing_guard_t *g = sc_pairing_guard_create(&sys, 1, tokens, 1);
    SC_ASSERT_NOT_NULL(g);
    SC_ASSERT_NULL(sc_pairing_guard_pairing_code(g));
    SC_ASSERT(sc_pairing_guard_is_paired(g));
    SC_ASSERT(sc_pairing_guard_is_authenticated(g, "zc_test_token"));
    SC_ASSERT_FALSE(sc_pairing_guard_is_authenticated(g, "zc_wrong"));
    sc_pairing_guard_destroy(g);
}

static void test_pairing_attempt_pair(void) {
    sc_allocator_t sys = sc_system_allocator();
    sc_pairing_guard_t *g = sc_pairing_guard_create(&sys, 1, NULL, 0);
    SC_ASSERT_NOT_NULL(g);
    const char *code = sc_pairing_guard_pairing_code(g);
    SC_ASSERT_NOT_NULL(code);

    char *token = NULL;
    sc_pair_attempt_result_t r = sc_pairing_guard_attempt_pair(g, code, &token);
    SC_ASSERT(r == SC_PAIR_PAIRED);
    SC_ASSERT_NOT_NULL(token);
    SC_ASSERT(strncmp(token, "zc_", 3) == 0);
    sys.free(sys.ctx, token, strlen(token) + 1);

    SC_ASSERT_NULL(sc_pairing_guard_pairing_code(g));
    SC_ASSERT(sc_pairing_guard_is_paired(g));
    sc_pairing_guard_destroy(g);
}

static void test_secret_store_is_encrypted(void) {
    SC_ASSERT(sc_secret_store_is_encrypted("enc2:aabbcc"));
    SC_ASSERT_FALSE(sc_secret_store_is_encrypted("plaintext"));
    SC_ASSERT_FALSE(sc_secret_store_is_encrypted("enc"));
    SC_ASSERT_FALSE(sc_secret_store_is_encrypted(""));
}

static void test_hex_encode_decode(void) {
    uint8_t data[] = { 0x00, 0x01, 0xfe, 0xff };
    char hex[16];
    sc_hex_encode(data, 4, hex);
    SC_ASSERT_STR_EQ(hex, "0001feff");

    uint8_t out[4];
    size_t len;
    sc_error_t err = sc_hex_decode("0001feff", 8, out, 4, &len);
    SC_ASSERT(err == SC_OK);
    SC_ASSERT_EQ(len, 4);
    SC_ASSERT(memcmp(out, data, 4) == 0);
}

static void test_secret_store_encrypt_decrypt(void) {
    sc_allocator_t sys = sc_system_allocator();
    char tmp[] = "/tmp/sc_secret_XXXXXX";
    char *dir = mkdtemp(tmp);
    SC_ASSERT_NOT_NULL(dir);

    sc_secret_store_t *store = sc_secret_store_create(&sys, dir, 1);
    SC_ASSERT_NOT_NULL(store);

    char *enc = NULL;
    sc_error_t err = sc_secret_store_encrypt(store, &sys, "sk-my-secret-key", &enc);
    SC_ASSERT(err == SC_OK);
    SC_ASSERT_NOT_NULL(enc);
    SC_ASSERT(strncmp(enc, "enc2:", 5) == 0);

    char *dec = NULL;
    err = sc_secret_store_decrypt(store, &sys, enc, &dec);
    SC_ASSERT(err == SC_OK);
    SC_ASSERT_STR_EQ(dec, "sk-my-secret-key");

    sys.free(sys.ctx, enc, strlen(enc) + 1);
    sys.free(sys.ctx, dec, strlen(dec) + 1);
    sc_secret_store_destroy(store, &sys);

    char keypath[256];
    snprintf(keypath, sizeof(keypath), "%s/.secret_key", dir);
    (void)unlink(keypath);
    (void)rmdir(dir);
}

static void test_secret_store_plaintext_passthrough(void) {
    sc_allocator_t sys = sc_system_allocator();
    char tmp[] = "/tmp/sc_secret2_XXXXXX";
    char *dir = mkdtemp(tmp);
    SC_ASSERT_NOT_NULL(dir);

    sc_secret_store_t *store = sc_secret_store_create(&sys, dir, 1);
    SC_ASSERT_NOT_NULL(store);

    char *dec = NULL;
    sc_error_t err = sc_secret_store_decrypt(store, &sys, "sk-plaintext-key", &dec);
    SC_ASSERT(err == SC_OK);
    SC_ASSERT_STR_EQ(dec, "sk-plaintext-key");

    sys.free(sys.ctx, dec, strlen(dec) + 1);
    sc_secret_store_destroy(store, &sys);
    rmdir(dir);
}

static void test_audit_event_init(void) {
    sc_audit_event_t e1, e2;
    sc_audit_event_init(&e1, SC_AUDIT_COMMAND_EXECUTION);
    sc_audit_event_init(&e2, SC_AUDIT_COMMAND_EXECUTION);
    SC_ASSERT(e1.event_id != e2.event_id);
    SC_ASSERT(e1.timestamp_s > 0);
}

static void test_audit_event_write_json(void) {
    sc_audit_event_t ev;
    sc_audit_event_init(&ev, SC_AUDIT_COMMAND_EXECUTION);
    sc_audit_event_with_actor(&ev, "telegram", "123", "@alice");
    sc_audit_event_with_action(&ev, "ls -la", "low", false, true);
    char buf[1024];
    size_t n = sc_audit_event_write_json(&ev, buf, sizeof(buf));
    SC_ASSERT(n > 0);
    SC_ASSERT(strstr(buf, "command_execution") != NULL);
    SC_ASSERT(strstr(buf, "telegram") != NULL);
}

static void test_audit_logger_disabled(void) {
    sc_allocator_t sys = sc_system_allocator();
    sc_audit_config_t cfg = { .enabled = false, .log_path = "audit.log", .max_size_mb = 10 };
    sc_audit_logger_t *log = sc_audit_logger_create(&sys, &cfg, "/tmp");
    SC_ASSERT_NOT_NULL(log);
    sc_audit_event_t ev;
    sc_audit_event_init(&ev, SC_AUDIT_COMMAND_EXECUTION);
    sc_error_t err = sc_audit_logger_log(log, &ev);
    SC_ASSERT(err == SC_OK);
    sc_audit_logger_destroy(log, &sys);
}

static void test_sandbox_noop(void) {
    sc_allocator_t sys = sc_system_allocator();
    sc_sandbox_alloc_t alloc = {
        .ctx = sys.ctx,
        .alloc = sys.alloc,
        .free = sys.free,
    };
    sc_sandbox_storage_t *st = sc_sandbox_storage_create(&alloc);
    SC_ASSERT_NOT_NULL(st);
    sc_sandbox_t sb = sc_sandbox_create(SC_SANDBOX_NONE, "/tmp/workspace", st, &alloc);
    SC_ASSERT(sb.ctx != NULL);
    SC_ASSERT(sb.vtable != NULL);
    SC_ASSERT(strcmp(sc_sandbox_name(&sb), "none") == 0);
    const char *argv[] = { "echo", "test" };
    const char *out[16];
    size_t out_count = 0;
    sc_error_t err = sc_sandbox_wrap_command(&sb, argv, 2, out, 16, &out_count);
    SC_ASSERT(err == SC_OK);
    SC_ASSERT_EQ(out_count, 2);
    sc_sandbox_storage_destroy(st, &alloc);
}

/* Test backend vtables directly (bypass create fallback to noop) */
static void test_sandbox_landlock_non_linux_or_test(void) {
    sc_landlock_ctx_t ctx;
    sc_landlock_sandbox_init(&ctx, "/tmp");
    sc_sandbox_t sb = sc_landlock_sandbox_get(&ctx);
    bool avail = sc_sandbox_is_available(&sb);
#if defined(__linux__) && !SC_IS_TEST
    (void)avail; /* May be true if kernel has Landlock */
#else
    SC_ASSERT_FALSE(avail); /* macOS or SC_IS_TEST: not available */
#endif
    const char *argv[] = { "echo", "x" };
    const char *out[16];
    size_t out_count = 0;
    sc_error_t err = sc_sandbox_wrap_command(&sb, argv, 2, out, 16, &out_count);
#ifndef __linux__
    SC_ASSERT_EQ(err, SC_ERR_NOT_SUPPORTED);
#else
    SC_ASSERT(err == SC_OK || err == SC_ERR_NOT_SUPPORTED);
#endif
}

static void test_sandbox_firejail_non_linux(void) {
#ifndef __linux__
    sc_firejail_ctx_t ctx;
    sc_firejail_sandbox_init(&ctx, "/tmp");
    sc_sandbox_t sb = sc_firejail_sandbox_get(&ctx);
    SC_ASSERT_FALSE(sc_sandbox_is_available(&sb));
    const char *argv[] = { "echo", "x" };
    const char *out[16];
    size_t out_count = 0;
    sc_error_t err = sc_sandbox_wrap_command(&sb, argv, 2, out, 16, &out_count);
    SC_ASSERT_EQ(err, SC_ERR_NOT_SUPPORTED);
#endif
}

static void test_sandbox_bubblewrap_non_linux(void) {
#ifndef __linux__
    sc_bubblewrap_ctx_t ctx;
    sc_bubblewrap_sandbox_init(&ctx, "/tmp");
    sc_sandbox_t sb = sc_bubblewrap_sandbox_get(&ctx);
    SC_ASSERT_FALSE(sc_sandbox_is_available(&sb));
    const char *argv[] = { "echo", "x" };
    const char *out[16];
    size_t out_count = 0;
    sc_error_t err = sc_sandbox_wrap_command(&sb, argv, 2, out, 16, &out_count);
    SC_ASSERT_EQ(err, SC_ERR_NOT_SUPPORTED);
#endif
}

static void test_observer_noop(void) {
    sc_observer_t obs = sc_observer_noop();
    SC_ASSERT(strcmp(sc_observer_name(obs), "noop") == 0);
    sc_observer_event_t ev = { .tag = SC_OBSERVER_EVENT_HEARTBEAT_TICK };
    sc_observer_record_event(obs, &ev);
}

static void test_observer_metrics(void) {
    sc_metrics_observer_ctx_t ctx;
    sc_observer_t obs = sc_observer_metrics_create(&ctx);
    SC_ASSERT(strcmp(sc_observer_name(obs), "metrics") == 0);
    sc_observer_metric_t m = { .tag = SC_OBSERVER_METRIC_TOKENS_USED, .value = 42 };
    sc_observer_record_metric(obs, &m);
    SC_ASSERT_EQ(sc_observer_metrics_get(&ctx, SC_OBSERVER_METRIC_TOKENS_USED), 42);
}

static void test_observer_registry(void) {
    sc_observer_t o = sc_observer_registry_create("noop", NULL);
    SC_ASSERT(strcmp(sc_observer_name(o), "noop") == 0);
    o = sc_observer_registry_create("log", NULL);
    SC_ASSERT(strcmp(sc_observer_name(o), "log") == 0);
}

static void test_secret_store_disabled_returns_plaintext(void) {
    sc_allocator_t sys = sc_system_allocator();
    sc_secret_store_t *store = sc_secret_store_create(&sys, "/tmp", 0);
    SC_ASSERT_NOT_NULL(store);

    char *enc = NULL;
    sc_error_t err = sc_secret_store_encrypt(store, &sys, "sk-secret", &enc);
    SC_ASSERT(err == SC_OK);
    SC_ASSERT_STR_EQ(enc, "sk-secret");

    sys.free(sys.ctx, enc, strlen(enc) + 1);
    sc_secret_store_destroy(store, &sys);
}

void run_security_tests(void) {
    static sc_allocator_t sys = {0};
    sys = sc_system_allocator();
    g_alloc = &sys;

    SC_TEST_SUITE("Security Policy");
    SC_RUN_TEST(test_autonomy_level_values);
    SC_RUN_TEST(test_risk_level_values);
    SC_RUN_TEST(test_policy_can_act_readonly);
    SC_RUN_TEST(test_policy_can_act_supervised);
    SC_RUN_TEST(test_policy_can_act_full);
    SC_RUN_TEST(test_policy_allowed_commands);
    SC_RUN_TEST(test_policy_blocked_commands);
    SC_RUN_TEST(test_policy_readonly_blocks_all);
    SC_RUN_TEST(test_policy_command_injection_blocked);
    SC_RUN_TEST(test_policy_risk_levels);
    SC_RUN_TEST(test_policy_validate_command);
    SC_RUN_TEST(test_rate_tracker);

    SC_TEST_SUITE("Pairing");
    SC_RUN_TEST(test_pairing_constant_time_eq);
    SC_RUN_TEST(test_pairing_guard_create_destroy);
    SC_RUN_TEST(test_pairing_guard_disabled);
    SC_RUN_TEST(test_pairing_guard_with_tokens);
    SC_RUN_TEST(test_pairing_attempt_pair);

    SC_TEST_SUITE("Audit");
    SC_RUN_TEST(test_audit_event_init);
    SC_RUN_TEST(test_audit_event_write_json);
    SC_RUN_TEST(test_audit_logger_disabled);

    SC_TEST_SUITE("Sandbox");
    SC_RUN_TEST(test_sandbox_noop);
    SC_RUN_TEST(test_sandbox_landlock_non_linux_or_test);
    SC_RUN_TEST(test_sandbox_firejail_non_linux);
    SC_RUN_TEST(test_sandbox_bubblewrap_non_linux);

    SC_TEST_SUITE("Observer");
    SC_RUN_TEST(test_observer_noop);
    SC_RUN_TEST(test_observer_metrics);
    SC_RUN_TEST(test_observer_registry);

    SC_TEST_SUITE("Secrets");
    SC_RUN_TEST(test_secret_store_is_encrypted);
    SC_RUN_TEST(test_hex_encode_decode);
    SC_RUN_TEST(test_secret_store_encrypt_decrypt);
    SC_RUN_TEST(test_secret_store_plaintext_passthrough);
    SC_RUN_TEST(test_secret_store_disabled_returns_plaintext);
}

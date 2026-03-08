#include "seaclaw/agent/superhuman.h"
#include "seaclaw/core/allocator.h"
#include "seaclaw/core/error.h"
#include "test_framework.h"
#include <stdio.h>
#include <string.h>

static sc_error_t mock_build_a(void *ctx, sc_allocator_t *alloc, char **out, size_t *out_len) {
    (void)ctx;
    *out = (char *)alloc->alloc(alloc->ctx, 6);
    if (!*out)
        return SC_ERR_OUT_OF_MEMORY;
    memcpy(*out, "ctx_a", 5);
    (*out)[5] = '\0';
    *out_len = 5;
    return SC_OK;
}

static sc_error_t mock_build_b(void *ctx, sc_allocator_t *alloc, char **out, size_t *out_len) {
    (void)ctx;
    *out = (char *)alloc->alloc(alloc->ctx, 6);
    if (!*out)
        return SC_ERR_OUT_OF_MEMORY;
    memcpy(*out, "ctx_b", 5);
    (*out)[5] = '\0';
    *out_len = 5;
    return SC_OK;
}

static void superhuman_register_and_count(void) {
    sc_superhuman_registry_t reg;
    SC_ASSERT_EQ(sc_superhuman_registry_init(&reg), SC_OK);
    SC_ASSERT_EQ(reg.count, 0u);

    sc_superhuman_service_t svc_a = {
        .name = "service_a",
        .build_context = mock_build_a,
        .observe = NULL,
        .ctx = NULL,
    };
    SC_ASSERT_EQ(sc_superhuman_register(&reg, svc_a), SC_OK);
    SC_ASSERT_EQ(reg.count, 1u);

    sc_superhuman_service_t svc_b = {
        .name = "service_b",
        .build_context = mock_build_b,
        .observe = NULL,
        .ctx = NULL,
    };
    SC_ASSERT_EQ(sc_superhuman_register(&reg, svc_b), SC_OK);
    SC_ASSERT_EQ(reg.count, 2u);
}

static void superhuman_build_context_calls_all(void) {
    sc_allocator_t alloc = sc_system_allocator();
    sc_superhuman_registry_t reg;
    SC_ASSERT_EQ(sc_superhuman_registry_init(&reg), SC_OK);

    sc_superhuman_service_t svc_a = {
        .name = "service_a",
        .build_context = mock_build_a,
        .observe = NULL,
        .ctx = NULL,
    };
    SC_ASSERT_EQ(sc_superhuman_register(&reg, svc_a), SC_OK);

    sc_superhuman_service_t svc_b = {
        .name = "service_b",
        .build_context = mock_build_b,
        .observe = NULL,
        .ctx = NULL,
    };
    SC_ASSERT_EQ(sc_superhuman_register(&reg, svc_b), SC_OK);

    char *ctx = NULL;
    size_t ctx_len = 0;
    SC_ASSERT_EQ(sc_superhuman_build_context(&reg, &alloc, &ctx, &ctx_len), SC_OK);
    SC_ASSERT_NOT_NULL(ctx);
    SC_ASSERT_TRUE(ctx_len > 0);
    SC_ASSERT_TRUE(strstr(ctx, "### Superhuman Insights") != NULL);
    SC_ASSERT_TRUE(strstr(ctx, "#### service_a") != NULL);
    SC_ASSERT_TRUE(strstr(ctx, "ctx_a") != NULL);
    SC_ASSERT_TRUE(strstr(ctx, "#### service_b") != NULL);
    SC_ASSERT_TRUE(strstr(ctx, "ctx_b") != NULL);

    alloc.free(alloc.ctx, ctx, ctx_len + 1);
}

static void superhuman_register_at_max(void) {
    sc_superhuman_registry_t reg;
    SC_ASSERT_EQ(sc_superhuman_registry_init(&reg), SC_OK);

    static char names[SC_SUPERHUMAN_MAX_SERVICES][16];
    sc_superhuman_service_t svc = {
        .build_context = mock_build_a,
        .observe = NULL,
        .ctx = NULL,
    };

    for (size_t i = 0; i < SC_SUPERHUMAN_MAX_SERVICES; i++) {
        int n = snprintf(names[i], sizeof(names[i]), "svc_%zu", i);
        svc.name = names[i];
        (void)n;
        SC_ASSERT_EQ(sc_superhuman_register(&reg, svc), SC_OK);
    }

    svc.name = "extra";
    sc_error_t err = sc_superhuman_register(&reg, svc);
    SC_ASSERT_EQ(err, SC_ERR_OUT_OF_MEMORY);
}

static void superhuman_observe_null_text_ok(void) {
    sc_allocator_t alloc = sc_system_allocator();
    sc_superhuman_registry_t reg;
    SC_ASSERT_EQ(sc_superhuman_registry_init(&reg), SC_OK);

    sc_superhuman_service_t svc = {
        .name = "svc",
        .build_context = mock_build_a,
        .observe = NULL,
        .ctx = NULL,
    };
    SC_ASSERT_EQ(sc_superhuman_register(&reg, svc), SC_OK);

    sc_error_t err = sc_superhuman_observe_all(&reg, &alloc, NULL, 0, "user", 4);
    SC_ASSERT_EQ(err, SC_OK);
}

static void superhuman_build_context_empty(void) {
    sc_allocator_t alloc = sc_system_allocator();
    sc_superhuman_registry_t reg;
    SC_ASSERT_EQ(sc_superhuman_registry_init(&reg), SC_OK);

    char *ctx = NULL;
    size_t ctx_len = 0;
    sc_error_t err = sc_superhuman_build_context(&reg, &alloc, &ctx, &ctx_len);
    SC_ASSERT_EQ(err, SC_OK);
    SC_ASSERT_NULL(ctx);
    SC_ASSERT_EQ(ctx_len, 0u);
}

void run_superhuman_tests(void) {
    SC_TEST_SUITE("superhuman");
    SC_RUN_TEST(superhuman_register_and_count);
    SC_RUN_TEST(superhuman_build_context_calls_all);
    SC_RUN_TEST(superhuman_register_at_max);
    SC_RUN_TEST(superhuman_observe_null_text_ok);
    SC_RUN_TEST(superhuman_build_context_empty);
}

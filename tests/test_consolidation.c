#include "seaclaw/core/allocator.h"
#include "seaclaw/memory.h"
#include "seaclaw/memory/consolidation.h"
#include "seaclaw/memory/engines.h"
#include "test_framework.h"
#include <stdio.h>
#include <string.h>

static void similarity_identical_returns_100(void) {
    const char *a = "hello world";
    const char *b = "hello world";
    uint32_t score = sc_similarity_score(a, 11, b, 11);
    SC_ASSERT_EQ(score, 100u);
}

static void similarity_completely_different_returns_0(void) {
    const char *a = "alpha beta gamma";
    const char *b = "foo bar baz";
    uint32_t score = sc_similarity_score(a, 15, b, 11);
    SC_ASSERT_EQ(score, 0u);
}

static void similarity_partial_overlap_returns_reasonable(void) {
    const char *a = "hello world foo";
    const char *b = "hello world bar";
    uint32_t score = sc_similarity_score(a, 14, b, 14);
    SC_ASSERT_TRUE(score >= 50u);
    SC_ASSERT_TRUE(score <= 100u);
}

static void consolidation_removes_duplicates(void) {
    sc_allocator_t alloc = sc_system_allocator();
    sc_memory_t mem = sc_memory_lru_create(&alloc, 100);
    sc_memory_category_t cat = {.tag = SC_MEMORY_CATEGORY_CORE, .data = {{0}}};

    const char *content1 = "user likes coffee and tea";
    const char *content2 = "user likes coffee and tea";
    sc_error_t err = mem.vtable->store(mem.ctx, "key:a", 5, content1, 25, &cat, NULL, 0);
    SC_ASSERT_EQ(err, SC_OK);

    err = mem.vtable->store(mem.ctx, "key:b", 5, content2, 25, &cat, NULL, 0);
    SC_ASSERT_EQ(err, SC_OK);

    size_t count_before = 0;
    err = mem.vtable->count(mem.ctx, &count_before);
    SC_ASSERT_EQ(err, SC_OK);
    SC_ASSERT_EQ(count_before, 2u);

    sc_consolidation_config_t config = SC_CONSOLIDATION_DEFAULTS;
    config.dedup_threshold = 80;

    err = sc_memory_consolidate(&alloc, &mem, &config);
    SC_ASSERT_EQ(err, SC_OK);

    size_t count_after = 0;
    err = mem.vtable->count(mem.ctx, &count_after);
    SC_ASSERT_EQ(err, SC_OK);
    SC_ASSERT_EQ(count_after, 1u);

    if (mem.vtable->deinit)
        mem.vtable->deinit(mem.ctx);
}

void run_consolidation_tests(void) {
    SC_TEST_SUITE("consolidation");
    SC_RUN_TEST(similarity_identical_returns_100);
    SC_RUN_TEST(similarity_completely_different_returns_0);
    SC_RUN_TEST(similarity_partial_overlap_returns_reasonable);
    SC_RUN_TEST(consolidation_removes_duplicates);
}

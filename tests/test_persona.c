#include "seaclaw/core/allocator.h"
#include "seaclaw/core/string.h"
#include "seaclaw/persona.h"
#include "test_framework.h"
#include <string.h>

static void test_persona_types_exist(void) {
    sc_persona_t p;
    sc_persona_overlay_t ov;
    sc_persona_example_t ex;
    sc_persona_example_bank_t bank;

    memset(&p, 0, sizeof(p));
    memset(&ov, 0, sizeof(ov));
    memset(&ex, 0, sizeof(ex));
    memset(&bank, 0, sizeof(bank));

    SC_ASSERT_NULL(p.name);
    SC_ASSERT_NULL(p.identity);
    SC_ASSERT_NULL(p.overlays);
    SC_ASSERT_EQ(p.overlays_count, 0);
    SC_ASSERT_NULL(ov.channel);
    SC_ASSERT_NULL(ex.context);
    SC_ASSERT_NULL(bank.examples);
}

static void test_persona_find_overlay_found(void) {
    sc_allocator_t alloc = sc_system_allocator();
    sc_persona_t persona;
    memset(&persona, 0, sizeof(persona));

    persona.overlays = (sc_persona_overlay_t *)alloc.alloc(alloc.ctx, sizeof(sc_persona_overlay_t));
    SC_ASSERT_NOT_NULL(persona.overlays);
    persona.overlays_count = 1;

    persona.overlays[0].channel = sc_strndup(&alloc, "telegram", 8);
    persona.overlays[0].formality = NULL;
    persona.overlays[0].avg_length = NULL;
    persona.overlays[0].emoji_usage = NULL;
    persona.overlays[0].style_notes = NULL;
    persona.overlays[0].style_notes_count = 0;

    const sc_persona_overlay_t *found = sc_persona_find_overlay(&persona, "telegram", 8);
    SC_ASSERT_NOT_NULL(found);
    SC_ASSERT_STR_EQ(found->channel, "telegram");

    sc_persona_deinit(&alloc, &persona);
}

static void test_persona_find_overlay_not_found(void) {
    sc_allocator_t alloc = sc_system_allocator();
    sc_persona_t persona;
    memset(&persona, 0, sizeof(persona));

    persona.overlays = (sc_persona_overlay_t *)alloc.alloc(alloc.ctx, sizeof(sc_persona_overlay_t));
    SC_ASSERT_NOT_NULL(persona.overlays);
    persona.overlays_count = 1;
    persona.overlays[0].channel = sc_strndup(&alloc, "telegram", 8);
    persona.overlays[0].formality = NULL;
    persona.overlays[0].avg_length = NULL;
    persona.overlays[0].emoji_usage = NULL;
    persona.overlays[0].style_notes = NULL;
    persona.overlays[0].style_notes_count = 0;

    const sc_persona_overlay_t *found = sc_persona_find_overlay(&persona, "discord", 7);
    SC_ASSERT_NULL(found);

    found = sc_persona_find_overlay(&persona, "tel", 3);
    SC_ASSERT_NULL(found);

    sc_persona_deinit(&alloc, &persona);
}

static void test_persona_deinit_null_safe(void) {
    sc_allocator_t alloc = sc_system_allocator();
    sc_persona_t persona;
    memset(&persona, 0, sizeof(persona));

    sc_persona_deinit(&alloc, &persona);

    SC_ASSERT_NULL(persona.name);
    SC_ASSERT_EQ(persona.overlays_count, 0);
}

void run_persona_tests(void) {
    SC_TEST_SUITE("Persona");

    SC_RUN_TEST(test_persona_types_exist);
    SC_RUN_TEST(test_persona_find_overlay_found);
    SC_RUN_TEST(test_persona_find_overlay_not_found);
    SC_RUN_TEST(test_persona_deinit_null_safe);
}

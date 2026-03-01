#include "seaclaw/skills.h"

sc_error_t sc_skills_list(sc_allocator_t *alloc, const char *workspace_dir,
    sc_skill_t **out_skills, size_t *out_count) {
    (void)alloc;
    (void)workspace_dir;
    if (!out_skills || !out_count) return SC_ERR_INVALID_ARGUMENT;
    *out_skills = NULL;
    *out_count = 0;
    return SC_OK;
}

void sc_skills_free(sc_allocator_t *alloc, sc_skill_t *skills, size_t count) {
    (void)alloc;
    (void)skills;
    (void)count;
}

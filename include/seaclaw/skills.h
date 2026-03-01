#ifndef SC_SKILLS_H
#define SC_SKILLS_H

#include "seaclaw/core/allocator.h"
#include "seaclaw/core/error.h"
#include <stddef.h>

typedef struct sc_skill {
    char *name;
    char *description;
    char *instructions;
    bool available;
    char *missing_deps;
} sc_skill_t;

sc_error_t sc_skills_list(sc_allocator_t *alloc, const char *workspace_dir,
    sc_skill_t **out_skills, size_t *out_count);

void sc_skills_free(sc_allocator_t *alloc, sc_skill_t *skills, size_t count);

#endif /* SC_SKILLS_H */

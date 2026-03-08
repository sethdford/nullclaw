#ifndef SC_LLM_COMPILER_H
#define SC_LLM_COMPILER_H

#include "seaclaw/agent/dag.h"
#include "seaclaw/core/allocator.h"
#include "seaclaw/core/error.h"
#include "seaclaw/tool.h"
#include <stddef.h>

sc_error_t sc_llm_compiler_build_prompt(sc_allocator_t *alloc, const char *goal, size_t goal_len,
                                        const char **tool_names, size_t tool_count, char **out,
                                        size_t *out_len);
sc_error_t sc_llm_compiler_parse_plan(sc_allocator_t *alloc, const char *response,
                                      size_t response_len, sc_dag_t *dag);

#endif

#ifndef SC_TOOL_ROUTER_H
#define SC_TOOL_ROUTER_H

#include "seaclaw/core/allocator.h"
#include "seaclaw/core/error.h"
#include "seaclaw/tool.h"
#include <stddef.h>

#define SC_TOOL_ROUTER_MAX_SELECTED 15

typedef struct sc_tool_selection {
    size_t indices[SC_TOOL_ROUTER_MAX_SELECTED];
    size_t count;
} sc_tool_selection_t;

sc_error_t sc_tool_router_select(sc_allocator_t *alloc, const char *message, size_t message_len,
                                  sc_tool_t *all_tools, size_t all_tools_count,
                                  sc_tool_selection_t *out);

#endif /* SC_TOOL_ROUTER_H */

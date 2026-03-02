#ifndef SC_TOOLS_SCHEDULE_H
#define SC_TOOLS_SCHEDULE_H

#include "seaclaw/core/allocator.h"
#include "seaclaw/core/error.h"
#include "seaclaw/cron.h"
#include "seaclaw/tool.h"

sc_error_t sc_schedule_create(sc_allocator_t *alloc, sc_cron_scheduler_t *sched, sc_tool_t *out);

#endif

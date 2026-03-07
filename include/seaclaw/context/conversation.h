#ifndef SC_CONTEXT_CONVERSATION_H
#define SC_CONTEXT_CONVERSATION_H

#include "seaclaw/channel.h"
#include "seaclaw/core/allocator.h"
#include "seaclaw/core/error.h"
#include <stddef.h>
#include <stdint.h>

/* Build a complete conversation awareness context string from channel history.
 * Includes: conversation thread, emotional analysis, verbosity mirroring,
 * conversation phase, time-of-day triggers, detected user states.
 * Caller owns returned string. Returns NULL if no entries. */
char *sc_conversation_build_awareness(sc_allocator_t *alloc,
                                      const sc_channel_history_entry_t *entries, size_t count,
                                      size_t *out_len);

#endif /* SC_CONTEXT_CONVERSATION_H */

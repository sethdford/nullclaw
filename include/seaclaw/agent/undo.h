#ifndef SC_UNDO_H
#define SC_UNDO_H

#include "seaclaw/core/allocator.h"
#include "seaclaw/core/error.h"
#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

typedef enum sc_undo_type {
    SC_UNDO_FILE_WRITE,
    SC_UNDO_FILE_CREATE,
    SC_UNDO_SHELL_COMMAND,
} sc_undo_type_t;

typedef struct sc_undo_entry {
    sc_undo_type_t type;
    char *description;
    char *path;
    char *original_content;
    size_t original_content_len;
    int64_t timestamp;
    bool reversible;
} sc_undo_entry_t;

typedef struct sc_undo_stack sc_undo_stack_t;

sc_undo_stack_t *sc_undo_stack_create(sc_allocator_t *alloc, size_t max_entries);
void sc_undo_stack_destroy(sc_undo_stack_t *stack);
sc_error_t sc_undo_stack_push(sc_undo_stack_t *stack, const sc_undo_entry_t *entry);
size_t sc_undo_stack_count(const sc_undo_stack_t *stack);
sc_error_t sc_undo_stack_execute_undo(sc_undo_stack_t *stack, sc_allocator_t *alloc);
void sc_undo_entry_free(sc_allocator_t *alloc, sc_undo_entry_t *entry);

#endif

#ifndef SC_PROCESS_UTIL_H
#define SC_PROCESS_UTIL_H

#include "seaclaw/core/allocator.h"
#include "seaclaw/core/error.h"
#include <stdbool.h>
#include <stddef.h>

typedef struct sc_run_result {
    char *stdout_buf;
    size_t stdout_len;
    char *stderr_buf;
    size_t stderr_len;
    bool success;
    int exit_code;  /* -1 if terminated by signal */
} sc_run_result_t;

void sc_run_result_free(sc_allocator_t *alloc, sc_run_result_t *r);

/**
 * Run a child process, capture stdout and stderr.
 * Caller must call sc_run_result_free on the result.
 * argv[0] is the program, argv[argc] must be NULL.
 *
 * @param alloc Allocator for buffers
 * @param argv NULL-terminated argv (argv[0]=program, argv[argc]=NULL)
 * @param cwd Working directory (NULL = inherit)
 * @param max_output_bytes Max bytes to capture per stream (default 1MB)
 * @param out Result; caller must free with sc_run_result_free
 */
sc_error_t sc_process_run(sc_allocator_t *alloc,
    const char *const *argv,
    const char *cwd,
    size_t max_output_bytes,
    sc_run_result_t *out);

#endif /* SC_PROCESS_UTIL_H */

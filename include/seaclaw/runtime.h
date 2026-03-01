#ifndef SC_RUNTIME_H
#define SC_RUNTIME_H

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

/* ──────────────────────────────────────────────────────────────────────────
 * Runtime vtable
 * ────────────────────────────────────────────────────────────────────────── */

struct sc_runtime_vtable;

typedef struct sc_runtime {
    void *ctx;
    const struct sc_runtime_vtable *vtable;
} sc_runtime_t;

typedef struct sc_runtime_vtable {
    const char *(*name)(void *ctx);
    bool (*has_shell_access)(void *ctx);
    bool (*has_filesystem_access)(void *ctx);
    const char *(*storage_path)(void *ctx);
    bool (*supports_long_running)(void *ctx);
    uint64_t (*memory_budget)(void *ctx);
} sc_runtime_vtable_t;

/* ──────────────────────────────────────────────────────────────────────────
 * Factory
 * ────────────────────────────────────────────────────────────────────────── */

typedef enum sc_runtime_kind {
    SC_RUNTIME_NATIVE,
    SC_RUNTIME_DOCKER,
    SC_RUNTIME_WASM,
    SC_RUNTIME_CLOUDFLARE,
} sc_runtime_kind_t;

sc_runtime_t sc_runtime_native(void);
sc_runtime_t sc_runtime_docker(bool mount_workspace, uint64_t memory_limit_mb);
sc_runtime_t sc_runtime_wasm(uint64_t memory_limit_mb);
sc_runtime_t sc_runtime_cloudflare(void);

#endif /* SC_RUNTIME_H */

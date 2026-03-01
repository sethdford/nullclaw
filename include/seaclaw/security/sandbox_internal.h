#ifndef SC_SANDBOX_INTERNAL_H
#define SC_SANDBOX_INTERNAL_H

#include "seaclaw/security/sandbox.h"

/* Internal types and init — only for detect.c */

typedef struct { char _; } sc_noop_sandbox_ctx_t;
sc_sandbox_t sc_noop_sandbox_get(sc_noop_sandbox_ctx_t *ctx);

typedef struct { char workspace_dir[1024]; } sc_landlock_ctx_t;
void sc_landlock_sandbox_init(sc_landlock_ctx_t *ctx, const char *workspace_dir);
sc_sandbox_t sc_landlock_sandbox_get(sc_landlock_ctx_t *ctx);

typedef struct {
    char private_arg[256];
    size_t private_len;
} sc_firejail_ctx_t;
void sc_firejail_sandbox_init(sc_firejail_ctx_t *ctx, const char *workspace_dir);
sc_sandbox_t sc_firejail_sandbox_get(sc_firejail_ctx_t *ctx);

typedef struct { char workspace_dir[2048]; } sc_bubblewrap_ctx_t;
void sc_bubblewrap_sandbox_init(sc_bubblewrap_ctx_t *ctx, const char *workspace_dir);
sc_sandbox_t sc_bubblewrap_sandbox_get(sc_bubblewrap_ctx_t *ctx);

typedef struct {
    void *alloc_ctx;
    void *(*alloc_fn)(void *, size_t);
    void (*free_fn)(void *, void *, size_t);
    char mount_arg[4097];
    size_t mount_len;
    char image[128];
} sc_docker_ctx_t;
void sc_docker_sandbox_init(sc_docker_ctx_t *ctx, const char *workspace_dir,
    const char *image, void *alloc_ctx,
    void *(*alloc_fn)(void *, size_t),
    void (*free_fn)(void *, void *, size_t));
sc_sandbox_t sc_docker_sandbox_get(sc_docker_ctx_t *ctx);

#endif /* SC_SANDBOX_INTERNAL_H */

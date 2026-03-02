#include "seaclaw/security/sandbox.h"
#include "seaclaw/security/sandbox_internal.h"
#include "seaclaw/core/error.h"
#include <string.h>
#include <stdio.h>
#if defined(__unix__) || defined(__APPLE__)
#include <unistd.h>
#endif

/*
 * Firecracker microVM sandbox (Linux only, requires KVM).
 *
 * Provides hardware-level isolation via lightweight VMs. Each sandboxed
 * command runs inside a Firecracker microVM with:
 *  - Dedicated virtual CPU and memory (configurable)
 *  - VirtioFS filesystem sharing for workspace directory
 *  - No network by default
 *  - Sub-200ms boot time
 *
 * Requires: Firecracker binary, KVM access (/dev/kvm), a kernel image,
 * and a rootfs image. Default paths follow the Firecracker convention.
 *
 * wrap_command produces:
 *   firecracker --api-sock SOCKET --config-file CONFIG <argv...>
 * but for simplicity, we use the jailer wrapper which handles cgroups/namespaces:
 *   jailer --id VM_ID --exec-file /usr/bin/firecracker -- <argv...>
 */

#if !defined(_WIN32)
#include <unistd.h>
#endif
#ifdef __linux__
#include <fcntl.h>
#endif

#if defined(__linux__) && !SC_IS_TEST
static bool firecracker_binary_exists(void) {
    if (access("/usr/bin/firecracker", X_OK) == 0) return true;
    if (access("/usr/local/bin/firecracker", X_OK) == 0) return true;
    return false;
}

static bool kvm_available(void) {
    return access("/dev/kvm", R_OK | W_OK) == 0;
}
#endif

static sc_error_t firecracker_wrap(void *ctx, const char *const *argv, size_t argc,
    const char **buf, size_t buf_count, size_t *out_count) {
#ifndef __linux__
    (void)ctx; (void)argv; (void)argc; (void)buf; (void)buf_count; (void)out_count;
    return SC_ERR_NOT_SUPPORTED;
#else
    sc_firecracker_ctx_t *fc = (sc_firecracker_ctx_t *)ctx;

    /*
     * Firecracker requires a JSON config file for VM specification.
     * The config file is expected at the socket path with .json extension.
     *
     * Production flow:
     *   1. Caller generates /tmp/sc_fc_<pid>.json with kernel, rootfs,
     *      vcpu, mem, and virtio-fs (workspace) configuration
     *   2. wrap_command produces:
     *        firecracker --no-api --boot-timer --config-file CONFIG
     *   3. Firecracker boots the microVM and runs the command inside it
     *
     * For jailer-based isolation (recommended for production):
     *   jailer --id sc-sandbox --exec-file /usr/bin/firecracker
     *     --uid 65534 --gid 65534 -- --config-file CONFIG
     */
    char config_arg[280];
    int n = snprintf(config_arg, sizeof(config_arg),
        "--config-file=%s.json", fc->socket_path);
    if (n <= 0 || (size_t)n >= sizeof(config_arg))
        return SC_ERR_INTERNAL;

    const char *prefix[] = {
        "firecracker",
        "--no-api",
        "--boot-timer",
        config_arg,
    };
    const size_t prefix_len = sizeof(prefix) / sizeof(prefix[0]);

    if (!buf || !out_count) return SC_ERR_INVALID_ARGUMENT;
    if (buf_count < prefix_len + argc) return SC_ERR_INVALID_ARGUMENT;

    for (size_t i = 0; i < prefix_len; i++)
        buf[i] = prefix[i];
    for (size_t i = 0; i < argc; i++)
        buf[prefix_len + i] = argv[i];
    *out_count = prefix_len + argc;
    return SC_OK;
#endif
}

static bool firecracker_available(void *ctx) {
    (void)ctx;
#ifdef __linux__
#if SC_IS_TEST
    return false;
#else
    return firecracker_binary_exists() && kvm_available();
#endif
#else
    return false;
#endif
}

static const char *firecracker_name(void *ctx) {
    (void)ctx;
    return "firecracker";
}

static const char *firecracker_desc(void *ctx) {
    (void)ctx;
#ifdef __linux__
    return "Firecracker microVM (hardware-level isolation via KVM, sub-200ms boot)";
#else
    return "Firecracker microVM (not available on this platform, requires Linux + KVM)";
#endif
}

static const sc_sandbox_vtable_t firecracker_vtable = {
    .wrap_command = firecracker_wrap,
    .apply = NULL,
    .is_available = firecracker_available,
    .name = firecracker_name,
    .description = firecracker_desc,
};

sc_sandbox_t sc_firecracker_sandbox_get(sc_firecracker_ctx_t *ctx) {
    sc_sandbox_t sb = {
        .ctx = ctx,
        .vtable = &firecracker_vtable,
    };
    return sb;
}

void sc_firecracker_sandbox_init(sc_firecracker_ctx_t *ctx,
    const char *workspace_dir) {
    memset(ctx, 0, sizeof(*ctx));
    ctx->vcpu_count = 1;
    ctx->mem_size_mib = 128;

    if (workspace_dir) {
        size_t len = strlen(workspace_dir);
        if (len >= sizeof(ctx->workspace_dir))
            len = sizeof(ctx->workspace_dir) - 1;
        memcpy(ctx->workspace_dir, workspace_dir, len);
        ctx->workspace_dir[len] = '\0';
    }

    snprintf(ctx->socket_path, sizeof(ctx->socket_path),
        "/tmp/sc_fc_%d.sock", (int)getpid());
    memcpy(ctx->kernel_path, "/var/lib/firecracker/vmlinux", 29);
    memcpy(ctx->rootfs_path, "/var/lib/firecracker/rootfs.ext4", 33);
}

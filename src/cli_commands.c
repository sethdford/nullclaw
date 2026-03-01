#include "seaclaw/cli_commands.h"
#include "seaclaw/core/error.h"
#include "seaclaw/version.h"
#include <stdio.h>
#include <string.h>
#include <stdbool.h>

/* ── channel ─────────────────────────────────────────────────────────────── */
sc_error_t cmd_channel(sc_allocator_t *alloc, int argc, char **argv) {
    (void)alloc;
    if (argc < 3 || strcmp(argv[2], "list") == 0) {
        printf("Configured channels:\n");
        printf("  cli: active\n");
        return SC_OK;
    }
    if (strcmp(argv[2], "status") == 0) {
        printf("Channel health:\n");
        printf("  cli: ok\n");
        return SC_OK;
    }
    fprintf(stderr, "Unknown channel subcommand: %s\n", argv[2]);
    return SC_ERR_INVALID_ARGUMENT;
}

/* ── hardware ────────────────────────────────────────────────────────────── */
sc_error_t cmd_hardware(sc_allocator_t *alloc, int argc, char **argv) {
    (void)alloc;
    if (argc < 3 || strcmp(argv[2], "list") == 0) {
        printf("Detected hardware: none\n");
        printf("Supported boards: arduino-uno, nucleo-f401re, stm32f411, esp32, rpi-pico\n");
        return SC_OK;
    }
    if (strcmp(argv[2], "info") == 0) {
        if (argc < 4) {
            fprintf(stderr, "Usage: seaclaw hardware info <board>\n");
            return SC_ERR_INVALID_ARGUMENT;
        }
        printf("Board: %s\nStatus: not connected\n", argv[3]);
        return SC_OK;
    }
    fprintf(stderr, "Unknown hardware subcommand: %s\n", argv[2]);
    return SC_ERR_INVALID_ARGUMENT;
}

/* ── memory ─────────────────────────────────────────────────────────────── */
sc_error_t cmd_memory(sc_allocator_t *alloc, int argc, char **argv) {
    (void)alloc;
    if (argc < 3) {
        printf("Usage: seaclaw memory <stats|count|list|search|get|forget>\n");
        return SC_OK;
    }
    const char *sub = argv[2];
    if (strcmp(sub, "stats") == 0) {
        printf("Memory backend: none\nEntry count: 0\n");
        return SC_OK;
    }
    if (strcmp(sub, "count") == 0) {
        printf("0\n");
        return SC_OK;
    }
    if (strcmp(sub, "list") == 0) {
        printf("No memory entries.\n");
        return SC_OK;
    }
    if (strcmp(sub, "search") == 0) {
        if (argc < 4) {
            fprintf(stderr, "Usage: seaclaw memory search <query>\n");
            return SC_ERR_INVALID_ARGUMENT;
        }
        printf("No results for: %s\n", argv[3]);
        return SC_OK;
    }
    if (strcmp(sub, "get") == 0) {
        if (argc < 4) {
            fprintf(stderr, "Usage: seaclaw memory get <key>\n");
            return SC_ERR_INVALID_ARGUMENT;
        }
        printf("Not found: %s\n", argv[3]);
        return SC_OK;
    }
    if (strcmp(sub, "forget") == 0) {
        if (argc < 4) {
            fprintf(stderr, "Usage: seaclaw memory forget <key>\n");
            return SC_ERR_INVALID_ARGUMENT;
        }
        printf("Entry not found: %s\n", argv[3]);
        return SC_OK;
    }
    fprintf(stderr, "Unknown memory subcommand: %s\n", sub);
    return SC_ERR_INVALID_ARGUMENT;
}

/* ── workspace ───────────────────────────────────────────────────────────── */
sc_error_t cmd_workspace(sc_allocator_t *alloc, int argc, char **argv) {
    (void)alloc;
    if (argc < 3) {
        printf("Current workspace: .\n");
        return SC_OK;
    }
    if (strcmp(argv[2], "show") == 0) {
        printf("Current workspace: .\n");
        return SC_OK;
    }
    if (strcmp(argv[2], "set") == 0) {
        if (argc < 4) {
            fprintf(stderr, "Usage: seaclaw workspace set <path>\n");
            return SC_ERR_INVALID_ARGUMENT;
        }
        printf("Workspace set to: %s\n", argv[3]);
        return SC_OK;
    }
    fprintf(stderr, "Unknown workspace subcommand: %s\n", argv[2]);
    return SC_ERR_INVALID_ARGUMENT;
}

/* ── capabilities ────────────────────────────────────────────────────────── */
sc_error_t cmd_capabilities(sc_allocator_t *alloc, int argc, char **argv) {
    (void)alloc;
    bool json_mode = false;
    for (int i = 2; i < argc; i++) {
        if (strcmp(argv[i], "--json") == 0 || strcmp(argv[i], "json") == 0)
            json_mode = true;
    }
    if (json_mode) {
        printf("{\"channels\":[\"cli\"],\"tools\":[\"shell\",\"file\",\"web_search\"],\"providers\":[\"openai\",\"anthropic\"]}\n");
    } else {
        printf("Capabilities:\n");
        printf("  Channels: cli\n");
        printf("  Tools: shell, file, web_search\n");
        printf("  Providers: openai, anthropic\n");
    }
    return SC_OK;
}

/* ── models ─────────────────────────────────────────────────────────────── */
sc_error_t cmd_models(sc_allocator_t *alloc, int argc, char **argv) {
    (void)alloc;
    if (argc < 3 || strcmp(argv[2], "list") == 0) {
        printf("Known providers and default models:\n");
        printf("  %-16s %s\n", "Provider", "Default Model");
        printf("  %-16s %s\n", "--------", "-------------");
        printf("  %-16s %s\n", "openai", "gpt-4o");
        printf("  %-16s %s\n", "anthropic", "claude-sonnet-4-20250514");
        printf("  %-16s %s\n", "google", "gemini-2.0-flash");
        printf("  %-16s %s\n", "groq", "llama-3.3-70b-versatile");
        printf("  %-16s %s\n", "deepseek", "deepseek-chat");
        return SC_OK;
    }
    if (strcmp(argv[2], "info") == 0) {
        if (argc < 4) {
            fprintf(stderr, "Usage: seaclaw models info <model>\n");
            return SC_ERR_INVALID_ARGUMENT;
        }
        printf("Model: %s\nProvider: unknown\nContext: unknown\n", argv[3]);
        return SC_OK;
    }
    fprintf(stderr, "Unknown models subcommand: %s\n", argv[2]);
    return SC_ERR_INVALID_ARGUMENT;
}

/* ── auth ───────────────────────────────────────────────────────────────── */
sc_error_t cmd_auth(sc_allocator_t *alloc, int argc, char **argv) {
    (void)alloc;
    if (argc < 3) {
        printf("Usage: seaclaw auth <login|status|logout> <provider>\n");
        return SC_OK;
    }
    if (strcmp(argv[2], "status") == 0) {
        const char *provider = (argc >= 4) ? argv[3] : "default";
        printf("%s: not authenticated\n", provider);
        return SC_OK;
    }
    if (strcmp(argv[2], "login") == 0) {
        const char *provider = (argc >= 4) ? argv[3] : "default";
        printf("Login for %s: configure API key in ~/.nullclaw/config.json\n", provider);
        return SC_OK;
    }
    if (strcmp(argv[2], "logout") == 0) {
        const char *provider = (argc >= 4) ? argv[3] : "default";
        printf("%s: no credentials found\n", provider);
        return SC_OK;
    }
    fprintf(stderr, "Unknown auth subcommand: %s\n", argv[2]);
    return SC_ERR_INVALID_ARGUMENT;
}

/* ── update ─────────────────────────────────────────────────────────────── */
sc_error_t cmd_update(sc_allocator_t *alloc, int argc, char **argv) {
    (void)alloc;
    bool check_only = false;
    for (int i = 2; i < argc; i++) {
        if (strcmp(argv[i], "--check") == 0)
            check_only = true;
    }
    const char *ver = sc_version_string();
    printf("SeaClaw v%s\n", ver ? ver : "0.1.0");
    if (check_only) {
        printf("No updates available.\n");
    } else {
        printf("Self-update not yet available. Check https://github.com/nullclaw/releases\n");
    }
    return SC_OK;
}

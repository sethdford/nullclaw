#include "seaclaw/memory/consolidation.h"
#include "seaclaw/core/error.h"
#include "seaclaw/core/string.h"
#include <ctype.h>
#include <stdlib.h>
#include <string.h>

#define SC_CONS_MAX_TOKENS 64

static void tokenize(const char *s, size_t len, const char **tokens, size_t *count) {
    *count = 0;
    if (!s || len == 0)
        return;
    size_t i = 0;
    while (i < len && *count < SC_CONS_MAX_TOKENS) {
        while (i < len && (unsigned char)s[i] <= ' ')
            i++;
        if (i >= len)
            break;
        tokens[(*count)++] = s + i;
        while (i < len && (unsigned char)s[i] > ' ')
            i++;
    }
}

static size_t token_len(const char *token, const char *end) {
    size_t n = 0;
    while (token + n < end && (unsigned char)token[n] > ' ')
        n++;
    return n;
}

uint32_t sc_similarity_score(const char *a, size_t a_len, const char *b, size_t b_len) {
    if (!a || !b)
        return 0;
    if (a_len == 0 && b_len == 0)
        return 100;

    const char *a_tokens[SC_CONS_MAX_TOKENS];
    const char *b_tokens[SC_CONS_MAX_TOKENS];
    size_t a_count = 0, b_count = 0;
    tokenize(a, a_len, a_tokens, &a_count);
    tokenize(b, b_len, b_tokens, &b_count);

    if (a_count == 0 && b_count == 0)
        return 100;
    if (a_count == 0 || b_count == 0)
        return 0;

    size_t shared = 0;
    const char *a_end = a + a_len;
    const char *b_end = b + b_len;

    for (size_t i = 0; i < a_count; i++) {
        size_t ai_len = token_len(a_tokens[i], a_end);
        for (size_t j = 0; j < b_count; j++) {
            size_t bj_len = token_len(b_tokens[j], b_end);
            if (ai_len == bj_len && memcmp(a_tokens[i], b_tokens[j], ai_len) == 0) {
                shared++;
                break;
            }
        }
    }

    size_t total_unique = a_count + b_count - shared;
    if (total_unique == 0)
        return 100;
    return (uint32_t)((shared * 100) / total_unique);
}

static int compare_timestamp(const char *ts_a, size_t ts_a_len, const char *ts_b, size_t ts_b_len) {
    if (!ts_a || ts_a_len == 0)
        return 1;
    if (!ts_b || ts_b_len == 0)
        return -1;
    long la = strtol(ts_a, NULL, 10);
    long lb = strtol(ts_b, NULL, 10);
    if (la < lb)
        return -1;
    if (la > lb)
        return 1;
    return 0;
}

sc_error_t sc_memory_consolidate(sc_allocator_t *alloc, sc_memory_t *memory,
                                 const sc_consolidation_config_t *config) {
    if (!alloc || !memory || !memory->vtable || !config)
        return SC_ERR_INVALID_ARGUMENT;
    if (!memory->vtable->list || !memory->vtable->forget)
        return SC_ERR_NOT_SUPPORTED;

    sc_memory_entry_t *entries = NULL;
    size_t count = 0;
    sc_error_t err = memory->vtable->list(memory->ctx, alloc, NULL, NULL, 0, &entries, &count);
    if (err != SC_OK)
        return err;
    if (!entries || count == 0)
        return SC_OK;

    bool *to_forget = (bool *)alloc->alloc(alloc->ctx, count * sizeof(bool));
    if (!to_forget) {
        for (size_t i = 0; i < count; i++)
            sc_memory_entry_free_fields(alloc, &entries[i]);
        alloc->free(alloc->ctx, entries, count * sizeof(sc_memory_entry_t));
        return SC_ERR_OUT_OF_MEMORY;
    }
    memset(to_forget, 0, count * sizeof(bool));

    for (size_t i = 0; i < count; i++) {
        if (to_forget[i])
            continue;
        for (size_t j = i + 1; j < count; j++) {
            if (to_forget[j])
                continue;
            uint32_t sim = sc_similarity_score(entries[i].content, entries[i].content_len,
                                               entries[j].content, entries[j].content_len);
            if (sim >= config->dedup_threshold) {
                int cmp = compare_timestamp(entries[i].timestamp, entries[i].timestamp_len,
                                            entries[j].timestamp, entries[j].timestamp_len);
                if (cmp <= 0)
                    to_forget[i] = true;
                else
                    to_forget[j] = true;
            }
        }
    }

    for (size_t i = 0; i < count; i++) {
        if (to_forget[i] && entries[i].key) {
            bool deleted = false;
            (void)memory->vtable->forget(memory->ctx, entries[i].key, entries[i].key_len, &deleted);
        }
    }

    alloc->free(alloc->ctx, to_forget, count * sizeof(bool));

    for (size_t i = 0; i < count; i++)
        sc_memory_entry_free_fields(alloc, &entries[i]);
    alloc->free(alloc->ctx, entries, count * sizeof(sc_memory_entry_t));

    size_t new_count = 0;
    err = memory->vtable->count(memory->ctx, &new_count);
    if (err != SC_OK)
        return SC_OK;

    if (new_count <= config->max_entries)
        return SC_OK;

    err = memory->vtable->list(memory->ctx, alloc, NULL, NULL, 0, &entries, &count);
    if (err != SC_OK || !entries || count <= config->max_entries)
        return SC_OK;

    for (size_t i = 0; i < count; i++) {
        for (size_t j = i + 1; j < count; j++) {
            int cmp = compare_timestamp(entries[i].timestamp, entries[i].timestamp_len,
                                        entries[j].timestamp, entries[j].timestamp_len);
            if (cmp > 0) {
                sc_memory_entry_t tmp = entries[i];
                entries[i] = entries[j];
                entries[j] = tmp;
            }
        }
    }

    size_t to_remove = count - config->max_entries;
    for (size_t i = 0; i < to_remove && i < count; i++) {
        if (entries[i].key) {
            bool deleted = false;
            (void)memory->vtable->forget(memory->ctx, entries[i].key, entries[i].key_len, &deleted);
        }
    }

    for (size_t i = 0; i < count; i++)
        sc_memory_entry_free_fields(alloc, &entries[i]);
    alloc->free(alloc->ctx, entries, count * sizeof(sc_memory_entry_t));

    return SC_OK;
}

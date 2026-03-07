# Audit Cycle 5 — Deep Code Review Report

**Scope:** config.c, control_protocol.c, http.c, string.c, sse_client.c, websocket.c  
**Confidence threshold:** ≥90%  
**Note:** src/sse/parser.c and src/websocket/client.c do not exist; reviewed src/sse/sse_client.c and src/websocket/websocket.c instead.

**Status:** All 4 bugs have been fixed. Build and tests pass.

---

## Critical

### 1. parse_providers OOM leak — config.c:229–241

**File:line:** `src/config.c:229-241`  
**Severity:** Critical (memory leak under OOM)  
**Confidence:** 95%

**Description:** When `sc_strdup(a, name)` fails for a provider entry, the loop does not increment `n` but has already allocated `api_key` and `base_url` for `providers[n]`. On the next iteration, those allocations are overwritten and never freed, causing a leak.

**Fix:**

```c
providers[n].name = sc_strdup(a, name);
if (!providers[n].name)
    continue;  /* Skip this entry but free any partial allocations */
const char *api_key = sc_json_get_string(item, "api_key");
if (api_key)
    providers[n].api_key = sc_strdup(a, api_key);
const char *base_url = sc_json_get_string(item, "base_url");
if (base_url)
    providers[n].base_url = sc_strdup(a, base_url);
providers[n].native_tools = sc_json_get_bool(item, "native_tools", true);
providers[n].ws_streaming = sc_json_get_bool(item, "ws_streaming", false);

/* Rollback on OOM: free any allocations we made for this entry */
if (!providers[n].name) {
    if (providers[n].api_key)
        a->free(a->ctx, providers[n].api_key, strlen(providers[n].api_key) + 1);
    if (providers[n].base_url)
        a->free(a->ctx, providers[n].base_url, strlen(providers[n].base_url) + 1);
    continue;
}
n++;
```

Actually the simpler fix: only increment n when name succeeds, and free api_key/base_url when name fails:

```c
providers[n].name = sc_strdup(a, name);
const char *api_key = sc_json_get_string(item, "api_key");
if (api_key)
    providers[n].api_key = sc_strdup(a, api_key);
const char *base_url = sc_json_get_string(item, "base_url");
if (base_url)
    providers[n].base_url = sc_strdup(a, base_url);
providers[n].native_tools = sc_json_get_bool(item, "native_tools", true);
providers[n].ws_streaming = sc_json_get_bool(item, "ws_streaming", false);

if (providers[n].name) {
    n++;
} else {
    /* OOM on name; free partial allocations before overwriting on next iter */
    if (providers[n].api_key) {
        a->free(a->ctx, providers[n].api_key, strlen(providers[n].api_key) + 1);
        providers[n].api_key = NULL;
    }
    if (providers[n].base_url) {
        a->free(a->ctx, providers[n].base_url, strlen(providers[n].base_url) + 1);
        providers[n].base_url = NULL;
    }
}
```

---

### 2. sc_str_concat / sc_str_join integer overflow — string.c:45–89

**File:line:** `src/core/string.c:45-46`, `src/core/string.c:66-72`  
**Severity:** Critical (buffer overflow / alloc truncation)  
**Confidence:** 95%

**Description:** `total = a.len + b.len` in `sc_str_concat` and the analogous sum in `sc_str_join` can overflow. If `a.len + b.len` wraps (e.g. both near `SIZE_MAX/2`), `total` becomes small, `alloc(total + 1)` allocates too little, and `memcpy` can overflow the buffer.

**Fix for sc_str_concat:**

```c
char *sc_str_concat(sc_allocator_t *alloc, sc_str_t a, sc_str_t b) {
    if (a.len > SIZE_MAX - b.len)
        return NULL;
    size_t total = a.len + b.len;
    char *out = (char *)alloc->alloc(alloc->ctx, total + 1);
    /* ... rest unchanged */
}
```

**Fix for sc_str_join:** Add overflow check in the accumulation loop:

```c
size_t total = 0;
for (size_t i = 0; i < count; i++) {
    if (parts[i].len > SIZE_MAX - total)
        return NULL;
    total += parts[i].len;
    if (i + 1 < count) {
        if (sep.len > SIZE_MAX - total)
            return NULL;
        total += sep.len;
    }
}
if (total > SIZE_MAX - 1)
    return NULL;
```

---

### 3. SSE event_type alloc OOM leak — sse_client.c:219–221

**File:line:** `src/sse/sse_client.c:219-221`  
**Severity:** Critical (memory leak under OOM)  
**Confidence:** 95%

**Description:** In the `"event"` field branch, when `ctx->alloc->alloc(..., value_len + 1)` fails, the function returns `SC_ERR_OUT_OF_MEMORY` without freeing `data`, which may have been allocated by earlier `"data"` lines.

**Fix:**

```c
} else if (field_eq(field, field_len, "event")) {
    if (event_type)
        ctx->alloc->free(ctx->alloc->ctx, event_type, event_type_len + 1);
    event_type_len = value_len;
    event_type = (char *)ctx->alloc->alloc(ctx->alloc->ctx, value_len + 1);
    if (!event_type) {
        if (data)
            ctx->alloc->free(ctx->alloc->ctx, data, data_cap);
        return SC_ERR_OUT_OF_MEMORY;
    }
    memcpy(event_type, value, value_len);
    event_type[value_len] = '\0';
}
```

---

## Important

### 4. parse_line_channel NULL dereference — config.c:921–924

**File:line:** `src/config.c:921-924`  
**Severity:** Important (crash on malformed config)  
**Confidence:** 90%

**Description:** When `obj->type == SC_JSON_ARRAY`, `obj->data.array.len > 0`, and `obj->data.array.items` is non-null, the code sets `val = obj->data.array.items[0]` without checking that `items[0]` is non-null. If `items[0]` is NULL, `val->type` causes a NULL dereference. The same pattern appears in `parse_google_chat_channel`, `parse_facebook_channel`, `parse_whatsapp_channel`, `parse_telegram_channel`, `parse_discord_channel`, `parse_slack_channel`, and `parse_instagram_channel`.

**Fix:** Only use `items[0]` when it is non-null:

```c
if (obj->type == SC_JSON_ARRAY && obj->data.array.len > 0 && obj->data.array.items) {
    if (obj->data.array.items[0])
        val = obj->data.array.items[0];
}
if (!val || val->type != SC_JSON_OBJECT)
    return;
```

Apply the same pattern to all channel parsers that use `val = obj->data.array.items[0]`.

---

## No issues found (high confidence)

- **control_protocol.c:** Auth checks present; no command injection; payload freed correctly (system allocator ignores size).
- **http.c:** Response body freed via `sc_http_response_free`; callers use it; curl errors propagated.
- **websocket.c:** Frame parsing capped by `SC_WS_MAX_FRAME_PAYLOAD`; payload freed on all paths; masking applied when `hdr.masked`.

#include "seaclaw/core/http.h"
#include "synthetic_harness.h"

static const char SC_SYNTH_AGENT_PROMPT[] =
    "You are a test case generator for an AI agent via OpenAI-compatible API.\n"
    "Generate %d diverse test cases for /v1/chat/completions.\n\n"
    "Return a JSON array where each element has:\n"
    "- \"name\": test name\n"
    "- \"model\": \"gemini-2.5-flash\"\n"
    "- \"messages\": [{\"role\":\"user\",\"content\":\"...\"}]\n"
    "- \"max_tokens\": integer (50-500)\n"
    "- \"expect_success\": boolean\n\n"
    "Scenarios: simple questions, tool-triggering, multi-turn, edge cases.\n"
    "Mix: 50%% simple, 30%% tool-use, 20%% edge.\nReturn ONLY a JSON array.";

sc_error_t sc_synth_run_agent(sc_allocator_t *alloc, const sc_synth_config_t *cfg,
                              sc_synth_gemini_ctx_t *gemini, sc_synth_metrics_t *metrics) {
    SC_SYNTH_LOG("=== Agent Tests ===");
    sc_synth_metrics_init(metrics);
    int count = cfg->tests_per_category > 0 ? cfg->tests_per_category : 10;
    char prompt[4096];
    snprintf(prompt, sizeof(prompt), SC_SYNTH_AGENT_PROMPT, count);
    SC_SYNTH_LOG("generating %d agent test cases via Gemini...", count);
    char *response = NULL;
    size_t response_len = 0;
    sc_error_t err =
        sc_synth_gemini_generate(alloc, gemini, prompt, strlen(prompt), &response, &response_len);
    if (err != SC_OK) {
        SC_SYNTH_LOG("Gemini generation failed: %s", sc_error_string(err));
        return err;
    }
    sc_json_value_t *root = NULL;
    err = sc_json_parse(alloc, response, response_len, &root);
    sc_synth_strfree(alloc, response, response_len);
    if (err != SC_OK || !root || root->type != SC_JSON_ARRAY) {
        SC_SYNTH_LOG("failed to parse test cases");
        if (root)
            sc_json_free(alloc, root);
        return SC_ERR_PARSE;
    }
    size_t n = root->data.array.len;
    SC_SYNTH_LOG("executing %zu agent tests (may be slow)...", n);
    char url[128];
    snprintf(url, sizeof(url), "http://127.0.0.1:%u/v1/chat/completions", cfg->gateway_port);
    for (size_t i = 0; i < n; i++) {
        sc_json_value_t *item = root->data.array.items[i];
        const char *name = sc_json_get_string(item, "name");
        bool exp_ok = sc_json_get_bool(item, "expect_success", true);
        const char *model = sc_json_get_string(item, "model");
        if (!model)
            model = "gemini-2.5-flash";
        int mt = (int)sc_json_get_number(item, "max_tokens", 100);
        sc_json_value_t *msgs = sc_json_object_get(item, "messages");
        char *msgs_str = NULL;
        size_t msgs_len = 0;
        if (msgs)
            (void)sc_json_stringify(alloc, msgs, &msgs_str, &msgs_len);
        if (!msgs_str)
            msgs_str = sc_synth_strdup(alloc, "[]", 2), msgs_len = 2;
        char header[512];
        int hlen = snprintf(header, sizeof(header), "{\"model\":\"%s\",\"messages\":", model);
        char tail[64];
        int tlen = snprintf(tail, sizeof(tail), ",\"max_tokens\":%d,\"stream\":false}", mt);
        size_t bl = (size_t)hlen + msgs_len + (size_t)tlen;
        char *body = (char *)alloc->alloc(alloc->ctx, bl + 1);
        if (!body) {
            sc_synth_strfree(alloc, msgs_str, msgs_len);
            continue;
        }
        memcpy(body, header, (size_t)hlen);
        memcpy(body + hlen, msgs_str, msgs_len);
        memcpy(body + hlen + msgs_len, tail, (size_t)tlen);
        body[bl] = '\0';
        sc_synth_strfree(alloc, msgs_str, msgs_len);
        sc_http_response_t resp = {0};
        double t0 = sc_synth_now_ms();
        err = sc_http_post_json(alloc, url, NULL, body, bl, &resp);
        double lat = sc_synth_now_ms() - t0;
        sc_synth_verdict_t v = SC_SYNTH_PASS;
        if (err != SC_OK)
            v = SC_SYNTH_ERROR;
        else if ((resp.status_code == 200) != exp_ok)
            v = SC_SYNTH_FAIL;
        sc_synth_metrics_record(alloc, metrics, lat, v);
        if (v == SC_SYNTH_PASS) {
            SC_SYNTH_VERBOSE(cfg, "PASS  %s (%.1fms)", name ? name : "?", lat);
        } else {
            SC_SYNTH_LOG("%-5s %s (status=%ld)", sc_synth_verdict_str(v), name ? name : "?",
                         resp.status_code);
            if (cfg->regression_dir) {
                char reason[256];
                snprintf(reason, sizeof(reason), "status=%ld expected_ok=%s", resp.status_code,
                         exp_ok ? "true" : "false");
                sc_synth_test_case_t tc = {.name = (char *)(name ? name : "agent_test"),
                                           .category = (char *)"agent",
                                           .input_json = body,
                                           .verdict = v,
                                           .verdict_reason = reason,
                                           .latency_ms = lat};
                sc_synth_regression_save(alloc, cfg->regression_dir, &tc);
            }
        }
        alloc->free(alloc->ctx, body, bl + 1);
        sc_http_response_free(alloc, &resp);
    }
    sc_json_free(alloc, root);
    return SC_OK;
}

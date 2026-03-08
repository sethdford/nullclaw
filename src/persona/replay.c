#include "seaclaw/context/conversation.h"
#include "seaclaw/core/string.h"
#include "seaclaw/persona/replay.h"
#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

#define SC_REPLAY_LONG_PAUSE_SEC 1800  /* 30 min */
#define SC_REPLAY_RAPID_WINDOW_SEC 120 /* 2 min for "rapid exchange" */

static bool str_contains_ci(const char *haystack, size_t hlen, const char *needle) {
    size_t nlen = strlen(needle);
    if (nlen > hlen)
        return false;
    for (size_t i = 0; i + nlen <= hlen; i++) {
        bool match = true;
        for (size_t j = 0; j < nlen; j++) {
            char a = haystack[i + j];
            char b = needle[j];
            if (a >= 'A' && a <= 'Z')
                a += 32;
            if (b >= 'A' && b <= 'Z')
                b += 32;
            if (a != b) {
                match = false;
                break;
            }
        }
        if (match)
            return true;
    }
    return false;
}

static bool str_starts_with_ci(const char *haystack, size_t hlen, const char *prefix) {
    size_t plen = strlen(prefix);
    if (plen > hlen)
        return false;
    for (size_t i = 0; i < plen; i++) {
        char a = haystack[i];
        char b = prefix[i];
        if (a >= 'A' && a <= 'Z')
            a += 32;
        if (b >= 'A' && b <= 'Z')
            b += 32;
        if (a != b)
            return false;
    }
    return true;
}

/* Parse timestamp to time_t. Returns (time_t)-1 on failure. */
static time_t parse_timestamp(const char *ts) {
    if (!ts || !ts[0])
        return (time_t)-1;
    struct tm tm = {0};
    char *p = strptime(ts, "%Y-%m-%d %H:%M", &tm);
    if (p && *p == '\0') {
        return mktime(&tm);
    }
    tm = (struct tm){0};
    p = strptime(ts, "%H:%M", &tm);
    if (p && *p == '\0') {
        /* Assume today for time-only format */
        time_t now = time(NULL);
        struct tm *now_tm = localtime(&now);
        if (now_tm) {
            tm.tm_year = now_tm->tm_year;
            tm.tm_mon = now_tm->tm_mon;
            tm.tm_mday = now_tm->tm_mday;
            return mktime(&tm);
        }
    }
    return (time_t)-1;
}

/* Positive signals: short enthusiastic replies after our messages */
static bool is_positive_signal(const char *text, size_t len) {
    if (len > 80)
        return false;
    const char *signals[] = {"haha", "exactly!", "omg", "yes!", "lol", "love it",
                             "perfect", "nice", "awesome", "great", "yes!!",
                             "hahaha", "lmao", "yesss", "so true", NULL};
    for (int i = 0; signals[i]; i++) {
        if (str_contains_ci(text, len, signals[i]))
            return true;
    }
    return false;
}

/* Negative signals: short disengaged replies */
static bool is_negative_signal(const char *text, size_t len) {
    if (len > 30)
        return false;
    const char *signals[] = {"k", "ok", "sure", "cool", "alright", "fine",
                             "whatever", "nah", "nope", NULL};
    for (int i = 0; signals[i]; i++) {
        if (str_contains_ci(text, len, signals[i]) && len < 15)
            return true;
    }
    return false;
}

/* Robotic tells: our response starts with these phrases */
static bool is_robotic_tell(const char *text, size_t len) {
    const char *tells[] = {"i understand", "that makes sense", "i understand how",
                            "that makes sense.", "i understand.", NULL};
    for (int i = 0; tells[i]; i++) {
        if (str_starts_with_ci(text, len, tells[i]))
            return true;
    }
    return false;
}

sc_error_t sc_replay_analyze(sc_allocator_t *alloc,
                             const sc_channel_history_entry_t *entries, size_t entry_count,
                             uint32_t max_chars, sc_replay_result_t *out) {
    if (!alloc || !out)
        return SC_ERR_INVALID_ARGUMENT;
    if (!entries && entry_count > 0)
        return SC_ERR_INVALID_ARGUMENT;

    memset(out, 0, sizeof(*out));

    if (entry_count == 0)
        return SC_OK;

    int total_quality = 0;
    size_t our_response_count = 0;
    int positive_count = 0;
    int negative_count = 0;

    for (size_t i = 0; i < entry_count && out->insight_count < SC_REPLAY_MAX_INSIGHTS; i++) {
        if (!entries[i].from_me)
            continue;

        size_t my_len = strlen(entries[i].text);
        if (my_len == 0)
            continue;

        /* Evaluate quality of our response */
        sc_quality_score_t q = sc_conversation_evaluate_quality(
            entries[i].text, my_len, entries, i + 1, max_chars);
        total_quality += q.total;
        our_response_count++;

        /* Look at next message (their reply) for positive/negative signals */
        if (i + 1 < entry_count && !entries[i + 1].from_me) {
            const char *their = entries[i + 1].text;
            size_t their_len = strlen(their);
            if (is_positive_signal(their, their_len)) {
                positive_count++;
                if (out->insight_count < SC_REPLAY_MAX_INSIGHTS) {
                    sc_replay_insight_t *ins = &out->insights[out->insight_count];
                    ins->observation = sc_strndup(alloc, "Their enthusiastic reply after our message", 43);
                    ins->observation_len = ins->observation ? 43 : 0;
                    ins->recommendation = sc_strndup(alloc, "Keep this tone and brevity", 26);
                    ins->recommendation_len = ins->recommendation ? 26 : 0;
                    ins->score_delta = 10;
                    if (ins->observation && ins->recommendation)
                        out->insight_count++;
                    else {
                        if (ins->observation)
                            alloc->free(alloc->ctx, ins->observation, 44);
                        if (ins->recommendation)
                            alloc->free(alloc->ctx, ins->recommendation, 27);
                    }
                }
            } else if (is_negative_signal(their, their_len)) {
                negative_count++;
                if (out->insight_count < SC_REPLAY_MAX_INSIGHTS) {
                    sc_replay_insight_t *ins = &out->insights[out->insight_count];
                    ins->observation = sc_strndup(alloc, "Short disengaged reply after our message", 40);
                    ins->observation_len = ins->observation ? 40 : 0;
                    ins->recommendation = sc_strndup(alloc, "Try shorter or more engaging follow-ups", 37);
                    ins->recommendation_len = ins->recommendation ? 37 : 0;
                    ins->score_delta = -10;
                    if (ins->observation && ins->recommendation)
                        out->insight_count++;
                    else {
                        if (ins->observation)
                            alloc->free(alloc->ctx, ins->observation, 41);
                        if (ins->recommendation)
                            alloc->free(alloc->ctx, ins->recommendation, 38);
                    }
                }
            }
        }

        /* Robotic tell: our response starts with "I understand" etc */
        if (is_robotic_tell(entries[i].text, my_len) &&
            out->insight_count < SC_REPLAY_MAX_INSIGHTS) {
            sc_replay_insight_t *ins = &out->insights[out->insight_count];
            ins->observation = sc_strndup(alloc, "Response started with robotic validation phrase", 46);
            ins->observation_len = ins->observation ? 46 : 0;
            ins->recommendation = sc_strndup(alloc, "Use more specific follow-up questions", 37);
            ins->recommendation_len = ins->recommendation ? 37 : 0;
            ins->score_delta = -15;
            if (ins->observation && ins->recommendation)
                out->insight_count++;
            else {
                if (ins->observation)
                    alloc->free(alloc->ctx, ins->observation, 47);
                if (ins->recommendation)
                    alloc->free(alloc->ctx, ins->recommendation, 38);
            }
        }

        /* Long pause: gap between our message and their next reply */
        if (i + 1 < entry_count && !entries[i + 1].from_me) {
            time_t my_ts = parse_timestamp(entries[i].timestamp);
            time_t their_ts = parse_timestamp(entries[i + 1].timestamp);
            if (my_ts != (time_t)-1 && their_ts != (time_t)-1) {
                double gap = difftime(their_ts, my_ts);
                if (gap > (double)SC_REPLAY_LONG_PAUSE_SEC &&
                    out->insight_count < SC_REPLAY_MAX_INSIGHTS) {
                    sc_replay_insight_t *ins = &out->insights[out->insight_count];
                    ins->observation = sc_strndup(alloc, "Long pause before their reply", 28);
                    ins->observation_len = ins->observation ? 28 : 0;
                    ins->recommendation = sc_strndup(alloc, "Consider shorter or more engaging messages", 41);
                    ins->recommendation_len = ins->recommendation ? 41 : 0;
                    ins->score_delta = -5;
                    if (ins->observation && ins->recommendation)
                        out->insight_count++;
                    else {
                        if (ins->observation)
                            alloc->free(alloc->ctx, ins->observation, 29);
                        if (ins->recommendation)
                            alloc->free(alloc->ctx, ins->recommendation, 42);
                    }
                }
            }
        }

        /* Our response too long relative to theirs (previous message) */
        if (i > 0 && !entries[i - 1].from_me) {
            size_t their_len = strlen(entries[i - 1].text);
            if (their_len > 0 && my_len > their_len * 3 && my_len > 150 &&
                out->insight_count < SC_REPLAY_MAX_INSIGHTS) {
                sc_replay_insight_t *ins = &out->insights[out->insight_count];
                ins->observation = sc_strndup(alloc, "Response was much longer than theirs", 35);
                ins->observation_len = ins->observation ? 35 : 0;
                ins->recommendation = sc_strndup(alloc, "Mirror their message length more closely", 38);
                ins->recommendation_len = ins->recommendation ? 38 : 0;
                ins->score_delta = -8;
                if (ins->observation && ins->recommendation)
                    out->insight_count++;
                else {
                    if (ins->observation)
                        alloc->free(alloc->ctx, ins->observation, 36);
                    if (ins->recommendation)
                        alloc->free(alloc->ctx, ins->recommendation, 39);
                }
            }
        }
    }

    /* Overall score: average quality of our responses */
    if (our_response_count > 0)
        out->overall_score = total_quality / (int)our_response_count;
    if (out->overall_score > 100)
        out->overall_score = 100;
    if (out->overall_score < 0)
        out->overall_score = 0;

    /* Build summary */
    {
        char buf[256];
        int n;
        if (positive_count > 0 && negative_count == 0)
            n = snprintf(buf, sizeof(buf), "Conversation was natural; %d response(s) got positive engagement", positive_count);
        else if (negative_count > 0 && positive_count == 0)
            n = snprintf(buf, sizeof(buf), "Conversation had %d disengaged or robotic moment(s)", negative_count);
        else if (positive_count > 0 && negative_count > 0)
            n = snprintf(buf, sizeof(buf), "Conversation mixed; %d positive, %d negative signal(s)", positive_count, negative_count);
        else
            n = snprintf(buf, sizeof(buf), "Conversation quality score %d/100", out->overall_score);
        if (n > 0 && (size_t)n < sizeof(buf)) {
            out->summary = sc_strndup(alloc, buf, (size_t)n);
            out->summary_len = out->summary ? (size_t)n : 0;
        }
    }

    return SC_OK;
}

void sc_replay_result_deinit(sc_replay_result_t *result, sc_allocator_t *alloc) {
    if (!result || !alloc)
        return;
    for (size_t i = 0; i < result->insight_count; i++) {
        sc_replay_insight_t *ins = &result->insights[i];
        if (ins->observation) {
            alloc->free(alloc->ctx, ins->observation, ins->observation_len + 1);
            ins->observation = NULL;
        }
        if (ins->recommendation) {
            alloc->free(alloc->ctx, ins->recommendation, ins->recommendation_len + 1);
            ins->recommendation = NULL;
        }
    }
    result->insight_count = 0;
    if (result->summary) {
        alloc->free(alloc->ctx, result->summary, result->summary_len + 1);
        result->summary = NULL;
    }
    result->summary_len = 0;
}

char *sc_replay_build_context(sc_allocator_t *alloc, const sc_replay_result_t *result,
                              size_t *out_len) {
    if (!alloc || !result || !out_len)
        return NULL;
    *out_len = 0;
    if (result->insight_count == 0)
        return NULL;

    size_t cap = 512;
    for (size_t i = 0; i < result->insight_count; i++) {
        cap += (result->insights[i].observation_len + result->insights[i].recommendation_len + 16);
    }
    if (result->summary)
        cap += result->summary_len + 32;

    char *buf = (char *)alloc->alloc(alloc->ctx, cap);
    if (!buf)
        return NULL;

    size_t pos = 0;
    int w;

    w = snprintf(buf, cap, "### Session Replay Insights\n");
    if (w > 0 && (size_t)w < cap) pos += (size_t)w;

    if (pos < cap) {
        w = snprintf(buf + pos, cap - pos, "Overall quality: %d/100\n", result->overall_score);
        if (w > 0 && pos + (size_t)w < cap) pos += (size_t)w;
    }
    if (result->summary && pos < cap) {
        w = snprintf(buf + pos, cap - pos, "%s\n", result->summary);
        if (w > 0 && pos + (size_t)w < cap) pos += (size_t)w;
    }

    bool has_positive = false;
    bool has_negative = false;
    for (size_t i = 0; i < result->insight_count; i++) {
        if (result->insights[i].score_delta > 0)
            has_positive = true;
        else
            has_negative = true;
    }

    if (has_positive && pos < cap) {
        w = snprintf(buf + pos, cap - pos, "What worked:\n");
        if (w > 0 && pos + (size_t)w < cap) pos += (size_t)w;
        for (size_t i = 0; i < result->insight_count && pos < cap; i++) {
            if (result->insights[i].score_delta > 0 && result->insights[i].observation) {
                w = snprintf(buf + pos, cap - pos, "- %s\n", result->insights[i].observation);
                if (w > 0 && pos + (size_t)w < cap) pos += (size_t)w;
            }
        }
    }
    if (has_negative && pos < cap) {
        w = snprintf(buf + pos, cap - pos, "What to improve:\n");
        if (w > 0 && pos + (size_t)w < cap) pos += (size_t)w;
        for (size_t i = 0; i < result->insight_count && pos < cap; i++) {
            if (result->insights[i].score_delta <= 0 && result->insights[i].observation) {
                w = snprintf(buf + pos, cap - pos, "- %s\n", result->insights[i].observation);
                if (w > 0 && pos + (size_t)w < cap) pos += (size_t)w;
            }
        }
    }

    if (pos == 0 || pos >= cap) {
        alloc->free(alloc->ctx, buf, cap);
        return NULL;
    }
    buf[pos] = '\0';
    char *out = sc_strndup(alloc, buf, pos);
    alloc->free(alloc->ctx, buf, cap);
    if (out)
        *out_len = (size_t)pos;
    return out;
}

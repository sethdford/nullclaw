#include "seaclaw/core/allocator.h"
#include "seaclaw/core/error.h"
#include "seaclaw/core/json.h"
#include "seaclaw/core/string.h"
#include "seaclaw/persona.h"
#include <stdio.h>
#include <string.h>

static sc_error_t parse_string_array_from_json(sc_allocator_t *a, const sc_json_value_t *arr,
                                               char ***out, size_t *out_count) {
    if (!arr || arr->type != SC_JSON_ARRAY || !arr->data.array.items)
        return SC_OK;
    size_t n = arr->data.array.len;
    if (n == 0)
        return SC_OK;
    char **buf = (char **)a->alloc(a->ctx, n * sizeof(char *));
    if (!buf)
        return SC_ERR_OUT_OF_MEMORY;
    size_t count = 0;
    for (size_t i = 0; i < n; i++) {
        const sc_json_value_t *item = arr->data.array.items[i];
        if (!item || item->type != SC_JSON_STRING || !item->data.string.ptr)
            continue;
        char *dup = sc_strndup(a, item->data.string.ptr, item->data.string.len);
        if (!dup) {
            for (size_t j = 0; j < count; j++)
                a->free(a->ctx, buf[j], strlen(buf[j]) + 1);
            a->free(a->ctx, buf, n * sizeof(char *));
            return SC_ERR_OUT_OF_MEMORY;
        }
        buf[count++] = dup;
    }
    *out = buf;
    *out_count = count;
    return SC_OK;
}

sc_error_t sc_persona_analyzer_build_prompt(const char **messages, size_t msg_count,
                                            const char *channel, char *buf, size_t cap,
                                            size_t *out_len) {
    if (!messages || !buf || !out_len || cap < 128)
        return SC_ERR_INVALID_ARGUMENT;

    size_t n = 0;
    n += (size_t)snprintf(
        buf + n, cap - n,
        "Analyze these %zu message samples from channel \"%s\" and extract a deep "
        "personality profile.\n\n"
        "Return valid JSON with ALL of these fields:\n"
        "- identity (string: one-sentence description of who this person is)\n"
        "- traits (array of strings)\n"
        "- vocabulary {preferred (array), avoided (array), slang (array)}\n"
        "- communication_rules (array of strings)\n"
        "- values (array of strings)\n"
        "- decision_style (string)\n"
        "- formality (string), avg_length (string), emoji_usage (string)\n"
        "- humor {type (string), frequency (string), targets (array), "
        "boundaries (array), timing (string)}\n"
        "- conflict_style {pushback_response, confrontation_comfort, apology_style, "
        "boundary_assertion, repair_behavior} (all strings)\n"
        "- emotional_range {ceiling, floor (strings), escalation_triggers (array), "
        "de_escalation (array), withdrawal_conditions, recovery_style (strings)}\n"
        "- voice_rhythm {sentence_pattern, paragraph_cadence, response_tempo, "
        "emphasis_style, pause_behavior} (all strings)\n"
        "- motivation {primary_drive, protecting, avoiding, wanting} (all strings)\n"
        "- intellectual {thinking_style (string), expertise (array), curiosity_areas (array), "
        "metaphor_sources (string)}\n"
        "- sensory {dominant_sense (string), metaphor_vocabulary (array), "
        "grounding_patterns (string)}\n"
        "- relational {bid_response_style (string), emotional_bids (array), "
        "attachment_style (string: secure/anxious/avoidant/mixed), "
        "attachment_awareness (string), dunbar_awareness (string)}\n"
        "- listening {default_response_type (string: support or shift), "
        "reflective_techniques (array: e.g. open_questions, affirmations, "
        "reflective_listening, summary_reflections), "
        "nvc_style (string), validation_style (string)}\n"
        "- repair {rupture_detection (string), repair_approach (string), "
        "face_saving_style (string), repair_phrases (array)}\n"
        "- mirroring {mirroring_level (string: none/subtle/moderate/strong), "
        "adapts_to (array: e.g. message_length, formality, emoji, pacing), "
        "convergence_speed (string), power_dynamic (string)}\n"
        "- social {default_ego_state (string: adult/nurturing_parent/free_child), "
        "phatic_style (string), bonding_behaviors (array), anti_patterns (array)}\n"
        "\nMessages:\n",
        msg_count, channel ? channel : "unknown");
    if ((size_t)n >= cap)
        return SC_ERR_INVALID_ARGUMENT;

    for (size_t i = 0; i < msg_count && n < cap; i++) {
        if (messages[i]) {
            size_t len = strlen(messages[i]);
            if (n + len + 4 > cap)
                break;
            n += (size_t)snprintf(buf + n, cap - n, "%zu. %s\n", i + 1, messages[i]);
        }
    }
    *out_len = n;
    return SC_OK;
}

sc_error_t sc_persona_analyzer_parse_response(sc_allocator_t *alloc, const char *response,
                                              size_t resp_len, const char *channel,
                                              size_t channel_len, sc_persona_t *out) {
    if (!alloc || !response || !out)
        return SC_ERR_INVALID_ARGUMENT;
    memset(out, 0, sizeof(*out));

    sc_json_value_t *root = NULL;
    sc_error_t err = sc_json_parse(alloc, response, resp_len, &root);
    if (err != SC_OK || !root || root->type != SC_JSON_OBJECT) {
        if (root)
            sc_json_free(alloc, root);
        return err != SC_OK ? err : SC_ERR_JSON_PARSE;
    }

    sc_json_value_t *traits = sc_json_object_get(root, "traits");
    if (traits) {
        err = parse_string_array_from_json(alloc, traits, &out->traits, &out->traits_count);
        if (err != SC_OK) {
            sc_json_free(alloc, root);
            return err;
        }
    }

    sc_json_value_t *vocab = sc_json_object_get(root, "vocabulary");
    if (vocab && vocab->type == SC_JSON_OBJECT) {
        sc_json_value_t *pref = sc_json_object_get(vocab, "preferred");
        if (pref) {
            err = parse_string_array_from_json(alloc, pref, &out->preferred_vocab,
                                               &out->preferred_vocab_count);
            if (err != SC_OK) {
                sc_persona_deinit(alloc, out);
                sc_json_free(alloc, root);
                return err;
            }
        }
        sc_json_value_t *avoid = sc_json_object_get(vocab, "avoided");
        if (avoid) {
            err = parse_string_array_from_json(alloc, avoid, &out->avoided_vocab,
                                               &out->avoided_vocab_count);
            if (err != SC_OK) {
                sc_persona_deinit(alloc, out);
                sc_json_free(alloc, root);
                return err;
            }
        }
        sc_json_value_t *sl = sc_json_object_get(vocab, "slang");
        if (sl) {
            err = parse_string_array_from_json(alloc, sl, &out->slang, &out->slang_count);
            if (err != SC_OK) {
                sc_persona_deinit(alloc, out);
                sc_json_free(alloc, root);
                return err;
            }
        }
    }

    sc_json_value_t *rules = sc_json_object_get(root, "communication_rules");
    if (rules) {
        err = parse_string_array_from_json(alloc, rules, &out->communication_rules,
                                           &out->communication_rules_count);
        if (err != SC_OK) {
            sc_persona_deinit(alloc, out);
            sc_json_free(alloc, root);
            return err;
        }
    }

    const char *formality = sc_json_get_string(root, "formality");
    const char *avg_length = sc_json_get_string(root, "avg_length");
    const char *emoji_usage = sc_json_get_string(root, "emoji_usage");
    if (channel && channel_len > 0 && (formality || avg_length || emoji_usage)) {
        out->overlays = alloc->alloc(alloc->ctx, sizeof(sc_persona_overlay_t));
        if (!out->overlays) {
            sc_persona_deinit(alloc, out);
            sc_json_free(alloc, root);
            return SC_ERR_OUT_OF_MEMORY;
        }
        memset(out->overlays, 0, sizeof(sc_persona_overlay_t));
        out->overlays_count = 1;
        out->overlays[0].channel = sc_strndup(alloc, channel, channel_len);
        if (!out->overlays[0].channel) {
            sc_persona_deinit(alloc, out);
            sc_json_free(alloc, root);
            return SC_ERR_OUT_OF_MEMORY;
        }
        if (formality) {
            out->overlays[0].formality = sc_strdup(alloc, formality);
            if (!out->overlays[0].formality) {
                sc_persona_deinit(alloc, out);
                sc_json_free(alloc, root);
                return SC_ERR_OUT_OF_MEMORY;
            }
        }
        if (avg_length) {
            out->overlays[0].avg_length = sc_strdup(alloc, avg_length);
            if (!out->overlays[0].avg_length) {
                sc_persona_deinit(alloc, out);
                sc_json_free(alloc, root);
                return SC_ERR_OUT_OF_MEMORY;
            }
        }
        if (emoji_usage) {
            out->overlays[0].emoji_usage = sc_strdup(alloc, emoji_usage);
            if (!out->overlays[0].emoji_usage) {
                sc_persona_deinit(alloc, out);
                sc_json_free(alloc, root);
                return SC_ERR_OUT_OF_MEMORY;
            }
        }
    }

    const char *identity = sc_json_get_string(root, "identity");
    if (identity) {
        out->identity = sc_strdup(alloc, identity);
        if (!out->identity) {
            sc_persona_deinit(alloc, out);
            sc_json_free(alloc, root);
            return SC_ERR_OUT_OF_MEMORY;
        }
    }

    const char *decision_style = sc_json_get_string(root, "decision_style");
    if (decision_style) {
        out->decision_style = sc_strdup(alloc, decision_style);
        if (!out->decision_style) {
            sc_persona_deinit(alloc, out);
            sc_json_free(alloc, root);
            return SC_ERR_OUT_OF_MEMORY;
        }
    }

    sc_json_value_t *values = sc_json_object_get(root, "values");
    if (values) {
        err = parse_string_array_from_json(alloc, values, &out->values, &out->values_count);
        if (err != SC_OK) {
            sc_persona_deinit(alloc, out);
            sc_json_free(alloc, root);
            return err;
        }
    }

    /* Humor profile */
    sc_json_value_t *humor = sc_json_object_get(root, "humor");
    if (humor && humor->type == SC_JSON_OBJECT) {
        const char *ht = sc_json_get_string(humor, "type");
        if (ht)
            out->humor.type = sc_strdup(alloc, ht);
        const char *hf = sc_json_get_string(humor, "frequency");
        if (hf)
            out->humor.frequency = sc_strdup(alloc, hf);
        const char *htm = sc_json_get_string(humor, "timing");
        if (htm)
            out->humor.timing = sc_strdup(alloc, htm);
        sc_json_value_t *ht_arr = sc_json_object_get(humor, "targets");
        if (ht_arr)
            (void)parse_string_array_from_json(alloc, ht_arr, &out->humor.targets,
                                               &out->humor.targets_count);
        sc_json_value_t *hb_arr = sc_json_object_get(humor, "boundaries");
        if (hb_arr)
            (void)parse_string_array_from_json(alloc, hb_arr, &out->humor.boundaries,
                                               &out->humor.boundaries_count);
    }

    /* Conflict style */
    sc_json_value_t *conflict = sc_json_object_get(root, "conflict_style");
    if (conflict && conflict->type == SC_JSON_OBJECT) {
        const char *s;
        s = sc_json_get_string(conflict, "pushback_response");
        if (s)
            out->conflict_style.pushback_response = sc_strdup(alloc, s);
        s = sc_json_get_string(conflict, "confrontation_comfort");
        if (s)
            out->conflict_style.confrontation_comfort = sc_strdup(alloc, s);
        s = sc_json_get_string(conflict, "apology_style");
        if (s)
            out->conflict_style.apology_style = sc_strdup(alloc, s);
        s = sc_json_get_string(conflict, "boundary_assertion");
        if (s)
            out->conflict_style.boundary_assertion = sc_strdup(alloc, s);
        s = sc_json_get_string(conflict, "repair_behavior");
        if (s)
            out->conflict_style.repair_behavior = sc_strdup(alloc, s);
    }

    /* Emotional range */
    sc_json_value_t *emo = sc_json_object_get(root, "emotional_range");
    if (emo && emo->type == SC_JSON_OBJECT) {
        const char *s;
        s = sc_json_get_string(emo, "ceiling");
        if (s)
            out->emotional_range.ceiling = sc_strdup(alloc, s);
        s = sc_json_get_string(emo, "floor");
        if (s)
            out->emotional_range.floor = sc_strdup(alloc, s);
        s = sc_json_get_string(emo, "withdrawal_conditions");
        if (s)
            out->emotional_range.withdrawal_conditions = sc_strdup(alloc, s);
        s = sc_json_get_string(emo, "recovery_style");
        if (s)
            out->emotional_range.recovery_style = sc_strdup(alloc, s);
        sc_json_value_t *esc = sc_json_object_get(emo, "escalation_triggers");
        if (esc)
            (void)parse_string_array_from_json(alloc, esc,
                                               &out->emotional_range.escalation_triggers,
                                               &out->emotional_range.escalation_triggers_count);
        sc_json_value_t *de = sc_json_object_get(emo, "de_escalation");
        if (de)
            (void)parse_string_array_from_json(alloc, de, &out->emotional_range.de_escalation,
                                               &out->emotional_range.de_escalation_count);
    }

    /* Voice rhythm */
    sc_json_value_t *voice = sc_json_object_get(root, "voice_rhythm");
    if (voice && voice->type == SC_JSON_OBJECT) {
        const char *s;
        s = sc_json_get_string(voice, "sentence_pattern");
        if (s)
            out->voice_rhythm.sentence_pattern = sc_strdup(alloc, s);
        s = sc_json_get_string(voice, "paragraph_cadence");
        if (s)
            out->voice_rhythm.paragraph_cadence = sc_strdup(alloc, s);
        s = sc_json_get_string(voice, "response_tempo");
        if (s)
            out->voice_rhythm.response_tempo = sc_strdup(alloc, s);
        s = sc_json_get_string(voice, "emphasis_style");
        if (s)
            out->voice_rhythm.emphasis_style = sc_strdup(alloc, s);
        s = sc_json_get_string(voice, "pause_behavior");
        if (s)
            out->voice_rhythm.pause_behavior = sc_strdup(alloc, s);
    }

    /* Motivation */
    sc_json_value_t *mot = sc_json_object_get(root, "motivation");
    if (mot && mot->type == SC_JSON_OBJECT) {
        const char *s;
        s = sc_json_get_string(mot, "primary_drive");
        if (s)
            out->motivation.primary_drive = sc_strdup(alloc, s);
        s = sc_json_get_string(mot, "protecting");
        if (s)
            out->motivation.protecting = sc_strdup(alloc, s);
        s = sc_json_get_string(mot, "avoiding");
        if (s)
            out->motivation.avoiding = sc_strdup(alloc, s);
        s = sc_json_get_string(mot, "wanting");
        if (s)
            out->motivation.wanting = sc_strdup(alloc, s);
    }

    /* Intellectual profile */
    sc_json_value_t *intel = sc_json_object_get(root, "intellectual");
    if (intel && intel->type == SC_JSON_OBJECT) {
        const char *s = sc_json_get_string(intel, "thinking_style");
        if (s)
            out->intellectual.thinking_style = sc_strdup(alloc, s);
        sc_json_value_t *a = sc_json_object_get(intel, "expertise");
        if (a)
            (void)parse_string_array_from_json(alloc, a, &out->intellectual.expertise,
                                               &out->intellectual.expertise_count);
        a = sc_json_object_get(intel, "curiosity_areas");
        if (a)
            (void)parse_string_array_from_json(alloc, a, &out->intellectual.curiosity_areas,
                                               &out->intellectual.curiosity_areas_count);
        s = sc_json_get_string(intel, "metaphor_sources");
        if (s)
            out->intellectual.metaphor_sources = sc_strdup(alloc, s);
    }

    /* Sensory preferences */
    sc_json_value_t *sens = sc_json_object_get(root, "sensory");
    if (sens && sens->type == SC_JSON_OBJECT) {
        const char *s = sc_json_get_string(sens, "dominant_sense");
        if (s)
            out->sensory.dominant_sense = sc_strdup(alloc, s);
        sc_json_value_t *a = sc_json_object_get(sens, "metaphor_vocabulary");
        if (a)
            (void)parse_string_array_from_json(alloc, a, &out->sensory.metaphor_vocabulary,
                                               &out->sensory.metaphor_vocabulary_count);
        s = sc_json_get_string(sens, "grounding_patterns");
        if (s)
            out->sensory.grounding_patterns = sc_strdup(alloc, s);
    }

    /* Relational intelligence */
    sc_json_value_t *rel = sc_json_object_get(root, "relational");
    if (rel && rel->type == SC_JSON_OBJECT) {
        const char *s;
        s = sc_json_get_string(rel, "bid_response_style");
        if (s)
            out->relational.bid_response_style = sc_strdup(alloc, s);
        s = sc_json_get_string(rel, "attachment_style");
        if (s)
            out->relational.attachment_style = sc_strdup(alloc, s);
        s = sc_json_get_string(rel, "attachment_awareness");
        if (s)
            out->relational.attachment_awareness = sc_strdup(alloc, s);
        s = sc_json_get_string(rel, "dunbar_awareness");
        if (s)
            out->relational.dunbar_awareness = sc_strdup(alloc, s);
        sc_json_value_t *a = sc_json_object_get(rel, "emotional_bids");
        if (a)
            (void)parse_string_array_from_json(alloc, a, &out->relational.emotional_bids,
                                               &out->relational.emotional_bids_count);
    }

    /* Listening protocol */
    sc_json_value_t *lis = sc_json_object_get(root, "listening");
    if (lis && lis->type == SC_JSON_OBJECT) {
        const char *s;
        s = sc_json_get_string(lis, "default_response_type");
        if (s)
            out->listening.default_response_type = sc_strdup(alloc, s);
        s = sc_json_get_string(lis, "nvc_style");
        if (s)
            out->listening.nvc_style = sc_strdup(alloc, s);
        s = sc_json_get_string(lis, "validation_style");
        if (s)
            out->listening.validation_style = sc_strdup(alloc, s);
        sc_json_value_t *a = sc_json_object_get(lis, "reflective_techniques");
        if (a)
            (void)parse_string_array_from_json(alloc, a, &out->listening.reflective_techniques,
                                               &out->listening.reflective_techniques_count);
    }

    /* Repair protocol */
    sc_json_value_t *rep = sc_json_object_get(root, "repair");
    if (rep && rep->type == SC_JSON_OBJECT) {
        const char *s;
        s = sc_json_get_string(rep, "rupture_detection");
        if (s)
            out->repair.rupture_detection = sc_strdup(alloc, s);
        s = sc_json_get_string(rep, "repair_approach");
        if (s)
            out->repair.repair_approach = sc_strdup(alloc, s);
        s = sc_json_get_string(rep, "face_saving_style");
        if (s)
            out->repair.face_saving_style = sc_strdup(alloc, s);
        sc_json_value_t *a = sc_json_object_get(rep, "repair_phrases");
        if (a)
            (void)parse_string_array_from_json(alloc, a, &out->repair.repair_phrases,
                                               &out->repair.repair_phrases_count);
    }

    /* Linguistic mirroring */
    sc_json_value_t *mir = sc_json_object_get(root, "mirroring");
    if (mir && mir->type == SC_JSON_OBJECT) {
        const char *s;
        s = sc_json_get_string(mir, "mirroring_level");
        if (s)
            out->mirroring.mirroring_level = sc_strdup(alloc, s);
        s = sc_json_get_string(mir, "convergence_speed");
        if (s)
            out->mirroring.convergence_speed = sc_strdup(alloc, s);
        s = sc_json_get_string(mir, "power_dynamic");
        if (s)
            out->mirroring.power_dynamic = sc_strdup(alloc, s);
        sc_json_value_t *a = sc_json_object_get(mir, "adapts_to");
        if (a)
            (void)parse_string_array_from_json(alloc, a, &out->mirroring.adapts_to,
                                               &out->mirroring.adapts_to_count);
    }

    /* Social dynamics */
    sc_json_value_t *soc = sc_json_object_get(root, "social");
    if (soc && soc->type == SC_JSON_OBJECT) {
        const char *s;
        s = sc_json_get_string(soc, "default_ego_state");
        if (s)
            out->social.default_ego_state = sc_strdup(alloc, s);
        s = sc_json_get_string(soc, "phatic_style");
        if (s)
            out->social.phatic_style = sc_strdup(alloc, s);
        sc_json_value_t *a = sc_json_object_get(soc, "bonding_behaviors");
        if (a)
            (void)parse_string_array_from_json(alloc, a, &out->social.bonding_behaviors,
                                               &out->social.bonding_behaviors_count);
        a = sc_json_object_get(soc, "anti_patterns");
        if (a)
            (void)parse_string_array_from_json(alloc, a, &out->social.anti_patterns,
                                               &out->social.anti_patterns_count);
    }

    sc_json_free(alloc, root);
    return SC_OK;
}

/* SPDX-License-Identifier: MIT */
/* Runner report writer (D-19, D-30, plan 04-18). The report is flat `key value` lines, ASCII and
 * LF, every key of the closed key table below in its fixed order (all keys always present, `-`
 * when a value does not apply): `ivsr 2` first and `report.end <n>` last, n being the number of
 * lines before it, so a truncated report cannot read as complete. The tables are the whole key
 * set; the writer prints nothing else. Never printed: wall time, host paths, the environment,
 * image bytes. Values echoed from the scenario (id, profile, a relative content path in a
 * detail) are charset-restricted by the parser, so they hold no blank line or control byte.
 * Key order (the pinned names that plans 04-21 and 04-23 read):
 *   ivsr, scenario.id, scenario.profile, api.version, outcome, outcome.detail, exit.code,
 *   slot.<exec|grom|cart>.<loaded|length_bytes|sha256|status|reason>,
 *   exec: (label), grom: (label),
 *   per step k: step.<k>.<target_ticks|now_ticks|overshoot_ticks|stop_reason|expected_stop>,
 *     then step.<k>.assert.<j> for that step's asserts,
 *   cpu.run_state, cpu.fault_reason, input.events, input.consumed, input.controller.<0..3>,
 *   digest.<cpu|mem|images|instance|composite|video|audio|input|psglog>, output.video.bytes,
 *   output.audio.bytes, output.audio.samples, output.audio.changes, output.psglog.records,
 *   output.psglog.bytes, audio.sample_ticks, audio.chain.check, host.calls, report.end.
 * `ivsr 2` (plan 05-09, D-20, the one bump of Phase 5) adds the keys from digest.psglog on; the
 * refpcm.* keys of the reference-PCM option join the table in plan 05-10 only when it is asked for.
 * Named-fault sites (data/expected/faults/runner.ids) sit in this TU. */
#include <stdint.h>
#include <stdio.h>
#include <string.h>

#include "ivs/ivs.h"
#include "ivs_runner.h"

#define RP_LINE_MAX 640u

/* Field ids of the key tables. */
#define RF_IVSR 1u
#define RF_SCN_ID 2u
#define RF_PROFILE 3u
#define RF_API 4u
#define RF_OUTCOME 5u
#define RF_DETAIL 6u
#define RF_EXIT 7u
#define RF_SLOT_LOADED 10u
#define RF_SLOT_LENGTH 11u
#define RF_SLOT_SHA 12u
#define RF_SLOT_STATUS 13u
#define RF_SLOT_REASON 14u
#define RF_LABEL_EXEC 20u
#define RF_LABEL_GROM 21u
#define RF_STEP_TARGET 30u
#define RF_STEP_NOW 31u
#define RF_STEP_OVERSHOOT 32u
#define RF_STEP_STOP 33u
#define RF_STEP_EXPECTED 34u
#define RF_CPU_STATE 40u
#define RF_CPU_FAULT 41u
#define RF_INPUT_EVENTS 42u
#define RF_INPUT_CONSUMED 43u
#define RF_INPUT_CTRL 44u /* + controller index */
#define RF_DIGEST 50u     /* + digest row index (record order) */
#define RF_OUT_VIDEO 60u
#define RF_OUT_AUDIO 61u
#define RF_HOST_CALLS 62u
#define RF_OUT_AUDIO_SAMPLES 63u
#define RF_OUT_AUDIO_CHANGES 64u
#define RF_OUT_PSG_RECORDS 65u
#define RF_OUT_PSG_BYTES 66u
#define RF_AUDIO_SAMPLE_TICKS 67u
#define RF_AUDIO_CHAIN 68u

typedef struct rp_key {
  const char *key;
  uint32_t field;
} rp_key;

static const rp_key rp_head[] = {
    {"ivsr", RF_IVSR},          {"scenario.id", RF_SCN_ID}, {"scenario.profile", RF_PROFILE},
    {"api.version", RF_API},    {"outcome", RF_OUTCOME},    {"outcome.detail", RF_DETAIL},
    {"exit.code", RF_EXIT},
};

/* slot.<role>.<key> for each role in the order exec, grom, cart. */
static const char *const rp_roles[IVS_SCN_ROLES] = {"exec", "grom", "cart"};
static const rp_key rp_slot[] = {
    {"loaded", RF_SLOT_LOADED}, {"length_bytes", RF_SLOT_LENGTH}, {"sha256", RF_SLOT_SHA},
    {"status", RF_SLOT_STATUS}, {"reason", RF_SLOT_REASON},
};

static const rp_key rp_label[] = {{"exec:", RF_LABEL_EXEC}, {"grom:", RF_LABEL_GROM}};

/* step.<k>.<key>, then step.<k>.assert.<j> for the step's asserts. */
static const rp_key rp_step[] = {
    {"target_ticks", RF_STEP_TARGET}, {"now_ticks", RF_STEP_NOW},           {"overshoot_ticks", RF_STEP_OVERSHOOT},
    {"stop_reason", RF_STEP_STOP},    {"expected_stop", RF_STEP_EXPECTED},
};

/* The digest rows of the record (ivs_runner_record.digest) are in this order too. */
static const rp_key rp_tail[] = {
    {"cpu.run_state", RF_CPU_STATE},
    {"cpu.fault_reason", RF_CPU_FAULT},
    {"input.events", RF_INPUT_EVENTS},
    {"input.consumed", RF_INPUT_CONSUMED},
    {"input.controller.0", RF_INPUT_CTRL + 0u},
    {"input.controller.1", RF_INPUT_CTRL + 1u},
    {"input.controller.2", RF_INPUT_CTRL + 2u},
    {"input.controller.3", RF_INPUT_CTRL + 3u},
#ifdef IVS_FAULT_RUNNER_KEY_ORDER_SWAPPED
    /* Fault (plan 04-18, D-19, D-23): two keys swapped, so the order is no longer the pinned one. */
    {"digest.mem", RF_DIGEST + 1u},
    {"digest.cpu", RF_DIGEST + 0u},
#else
    {"digest.cpu", RF_DIGEST + 0u},
    {"digest.mem", RF_DIGEST + 1u},
#endif
    {"digest.images", RF_DIGEST + 2u},
    {"digest.instance", RF_DIGEST + 3u},
    {"digest.composite", RF_DIGEST + 4u},
    {"digest.video", RF_DIGEST + 5u},
    {"digest.audio", RF_DIGEST + 6u},
    {"digest.input", RF_DIGEST + 7u},
    {"digest.psglog", RF_DIGEST + 8u},
    {"output.video.bytes", RF_OUT_VIDEO},
    {"output.audio.bytes", RF_OUT_AUDIO},
    {"output.audio.samples", RF_OUT_AUDIO_SAMPLES},
    {"output.audio.changes", RF_OUT_AUDIO_CHANGES},
    {"output.psglog.records", RF_OUT_PSG_RECORDS},
    {"output.psglog.bytes", RF_OUT_PSG_BYTES},
    {"audio.sample_ticks", RF_AUDIO_SAMPLE_TICKS},
    {"audio.chain.check", RF_AUDIO_CHAIN},
    {"host.calls", RF_HOST_CALLS},
};

#define RP_N(a) (sizeof(a) / sizeof((a)[0]))

const char *ivs_runner_stop_token(uint32_t stop_reason)
{
  switch (stop_reason) {
  case IVS_STOP_TARGET:
    return "target";
  case IVS_STOP_CPU_HALTED:
    return "halted";
  case IVS_STOP_GUEST_FAULT:
    return "faulted";
  case IVS_STOP_INTERNAL:
    return "internal";
  default:
    return "unknown";
  }
}

const char *ivs_runner_expected_stop_token(uint32_t expected_stop)
{
  switch (expected_stop) {
  case IVS_SCN_STOP_TARGET:
    return "target";
  case IVS_SCN_STOP_HALTED:
    return "halted";
  case IVS_SCN_STOP_FAULTED:
    return "faulted";
  default:
    return "unknown";
  }
}

const char *ivs_runner_domain_token(uint32_t domain)
{
  switch (domain) {
  case IVS_DIGEST_COMPOSITE:
    return "composite";
  case IVS_DIGEST_CPU:
    return "cpu";
  case IVS_DIGEST_MEM:
    return "mem";
  case IVS_DIGEST_IMAGES:
    return "images";
  case IVS_DIGEST_INSTANCE:
    return "instance";
  case IVS_DIGEST_VIDEO:
    return "video";
  case IVS_DIGEST_AUDIO:
    return "audio";
  case IVS_DIGEST_INPUT:
    return "input";
  case IVS_DIGEST_PSGLOG:
    return "psglog";
  default:
    return "unknown";
  }
}

static const char *rp_run_state(uint32_t s)
{
  switch (s) {
  case IVS_CPU_STATE_UNRESET:
    return "unreset";
  case IVS_CPU_STATE_RESET_PENDING:
    return "reset-pending";
  case IVS_CPU_STATE_RUNNING:
    return "running";
  case IVS_CPU_STATE_HALTED:
    return "halted";
  case IVS_CPU_STATE_FAULTED:
    return "faulted";
  default:
    return "unknown";
  }
}

static const char *rp_fault(uint32_t f)
{
  switch (f) {
  case IVS_CPU_FAULT_NONE:
    return "none";
  case IVS_CPU_FAULT_UNSUPPORTED_SDBD_TARGET:
    return "unsupported-sdbd-target";
  case IVS_CPU_FAULT_UNSUPPORTED_MVO_SSS_EQ_MMM:
    return "unsupported-mvo-sss-eq-mmm";
  case IVS_CPU_FAULT_UNSUPPORTED_JUMP_DE:
    return "unsupported-jump-de";
  case IVS_CPU_FAULT_BUS_WAIT_LIMIT:
    return "bus-wait-limit";
  default:
    return "unknown";
  }
}

/* 64 lower-case hex digits and a NUL into out (65 bytes). */
static void rp_hex(const uint8_t sha[32], char *out)
{
  static const char digits[] = "0123456789abcdef";
  for (size_t i = 0u; i < 32u; i++) {
    out[2u * i] = digits[sha[i] >> 4];
    out[2u * i + 1u] = digits[sha[i] & 0x0Fu];
  }
  out[64] = '\0';
}

typedef struct rp_out {
  const ivs_runner_host *host;
  uint64_t lines;
  int failed;
} rp_out;

/* Emits one `key value` line. */
static void rp_emit(rp_out *o, const char *key, const char *value)
{
  char line[RP_LINE_MAX];
  int n;
  if (o->failed) {
    return;
  }
  n = snprintf(line, sizeof line, "%s %s\n", key, value);
  if (n < 0 || (size_t)n >= sizeof line) {
    o->failed = 1;
    return;
  }
  if (o->host->write_report(o->host->ctx, line, (size_t)n) != 0) {
    o->failed = 1;
    return;
  }
  o->lines++;
}

static void rp_u64(char *v, size_t cap, uint64_t x) { (void)snprintf(v, cap, "%llu", (unsigned long long)x); }

/* The value of a head, label or tail field. */
static void rp_value(const ivs_runner_record *r, uint32_t field, char *v, size_t cap)
{
  const ivs_scenario *s = r->scn;
  uint32_t api;
  (void)snprintf(v, cap, "-");
  switch (field) {
  case RF_IVSR:
    (void)snprintf(v, cap, "2");
    break;
  case RF_SCN_ID:
    (void)snprintf(v, cap, "%s", s->id);
    break;
  case RF_PROFILE:
    (void)snprintf(v, cap, "%s", s->profile);
    break;
  case RF_API:
    api = ivs_api_version();
    (void)snprintf(v, cap, "%u.%u.%u", (unsigned)((api >> 16) & 0xFFu), (unsigned)((api >> 8) & 0xFFu),
                   (unsigned)(api & 0xFFu));
    break;
  case RF_OUTCOME:
    (void)snprintf(v, cap, "%s", ivs_runner_outcome_name(r->outcome));
    break;
  case RF_DETAIL:
    (void)snprintf(v, cap, "%s", r->detail[0] != '\0' ? r->detail : "-");
    break;
  case RF_EXIT:
    (void)snprintf(v, cap, "%u", (unsigned)ivs_runner_exit_code(r->outcome));
    break;
  case RF_LABEL_EXEC:
    /* D-30: from the symbolic content name only, never from the bytes or a table of hashes. */
    if (s->content[IVS_SCN_ROLE_EXEC].kind == IVS_SCN_CONTENT_BOOT_STUB) {
      char hex[65];
      const ivs_runner_slot_record *e = &r->slot[IVS_SCN_ROLE_EXEC];
      if (e->loaded != 0u) {
        rp_hex(e->sha256, hex);
      } else {
        (void)snprintf(hex, sizeof hex, "-");
      }
      (void)snprintf(v, cap, "project boot stub, not an EXEC sha256=%s", hex);
    }
    break;
  case RF_LABEL_GROM:
    if (s->content[IVS_SCN_ROLE_GROM].kind == IVS_SCN_CONTENT_ZERO_GROM) {
      (void)snprintf(v, cap, "zero image, not a GROM");
    }
    break;
  case RF_CPU_STATE:
    if (r->have_machine) {
      (void)snprintf(v, cap, "%s", rp_run_state(r->cpu_run_state));
    }
    break;
  case RF_CPU_FAULT:
    if (r->have_machine) {
      (void)snprintf(v, cap, "%s", rp_fault(r->cpu_fault_reason));
    }
    break;
  case RF_INPUT_EVENTS:
    rp_u64(v, cap, s->n_inputs);
    break;
  case RF_INPUT_CONSUMED:
    rp_u64(v, cap, r->input_consumed);
    break;
  case RF_OUT_VIDEO:
    rp_u64(v, cap, r->video_bytes);
    break;
  case RF_OUT_AUDIO:
    rp_u64(v, cap, r->audio_bytes);
    break;
  case RF_OUT_AUDIO_SAMPLES:
    rp_u64(v, cap, r->audio_samples);
    break;
  case RF_OUT_AUDIO_CHANGES:
    rp_u64(v, cap, r->audio_changes);
    break;
  case RF_OUT_PSG_RECORDS:
    rp_u64(v, cap, r->psglog_records);
    break;
  case RF_OUT_PSG_BYTES:
    rp_u64(v, cap, r->psglog_bytes);
    break;
  case RF_AUDIO_SAMPLE_TICKS:
    if (r->audio_sample_ticks != 0u) {
      rp_u64(v, cap, r->audio_sample_ticks);
    }
    break;
  case RF_AUDIO_CHAIN:
    (void)snprintf(v, cap, "%s", r->chain_check == IVS_RUNNER_CHAIN_PASS   ? "pass"
                                 : r->chain_check == IVS_RUNNER_CHAIN_FAIL ? "fail"
                                                                           : "-");
    break;
  case RF_HOST_CALLS:
    rp_u64(v, cap, r->host_calls);
    break;
  default:
    if (field >= RF_INPUT_CTRL && field < RF_INPUT_CTRL + 4u) {
      if (r->have_input) {
        (void)snprintf(v, cap, "%08X", (unsigned)r->controller[field - RF_INPUT_CTRL]);
      }
    } else if (field >= RF_DIGEST && field < RF_DIGEST + 9u) {
      if (r->have_machine) {
        rp_hex(r->digest[field - RF_DIGEST], v);
      }
    }
    break;
  }
}

static void rp_slot_value(const ivs_runner_slot_record *sl, uint32_t field, char *v, size_t cap)
{
  (void)snprintf(v, cap, "-");
  switch (field) {
  case RF_SLOT_LOADED:
    rp_u64(v, cap, sl->loaded);
    break;
  case RF_SLOT_LENGTH:
    rp_u64(v, cap, sl->loaded ? sl->length_bytes : 0u);
    break;
  case RF_SLOT_SHA:
    if (sl->loaded) {
      rp_hex(sl->sha256, v);
    }
    break;
  case RF_SLOT_STATUS:
    if (sl->attempted) {
      (void)snprintf(v, cap, "%s", sl->status == IVS_OK ? "ok" : ivs_status_name(sl->status));
    }
    break;
  case RF_SLOT_REASON:
    if (sl->attempted) {
      rp_u64(v, cap, sl->reason);
    }
    break;
  default:
    break;
  }
}

/* The step value; a cpu-cycles or frames target keeps its scenario form after the ticks. */
static void rp_step_value(const ivs_scenario_step *st, const ivs_runner_step_record *sr, uint32_t field, char *v,
                          size_t cap)
{
  (void)snprintf(v, cap, "-");
  switch (field) {
  case RF_STEP_TARGET:
    if (!sr->converted) {
      break;
    }
    if (st->unit == IVS_SCN_UNIT_CPU_CYCLES) {
      (void)snprintf(v, cap, "%llu from=%llu cpu-cycles", (unsigned long long)sr->target_ticks,
                     (unsigned long long)st->target);
    } else if (st->unit == IVS_SCN_UNIT_FRAMES) {
      (void)snprintf(v, cap, "%llu from=%llu frames nominal, not frame events", (unsigned long long)sr->target_ticks,
                     (unsigned long long)st->target);
    } else {
      rp_u64(v, cap, sr->target_ticks);
    }
    break;
  case RF_STEP_NOW:
    if (sr->ran) {
      rp_u64(v, cap, sr->now_ticks);
    }
    break;
  case RF_STEP_OVERSHOOT:
    if (sr->ran) {
#ifdef IVS_FAULT_RUNNER_OVERSHOOT_NOT_REPORTED
      /* Fault (plan 04-18, D-03, D-19, D-23): the overshoot is printed as 0. */
      rp_u64(v, cap, 0u);
#else
      rp_u64(v, cap, sr->overshoot_ticks);
#endif
    }
    break;
  case RF_STEP_STOP:
    if (sr->ran) {
      (void)snprintf(v, cap, "%s", ivs_runner_stop_token(sr->stop_reason));
    }
    break;
  case RF_STEP_EXPECTED:
    (void)snprintf(v, cap, "%s", ivs_runner_expected_stop_token(st->stop));
    break;
  default:
    break;
  }
}

/* `<state> <subject> <op> <expected> got <actual>` of one assert. */
static void rp_assert_value(const ivs_scenario_assert *a, const ivs_runner_assert_record *ar, char *v, size_t cap)
{
  const char *state = ar->state == IVS_RUNNER_ASSERT_PASS   ? "pass"
                      : ar->state == IVS_RUNNER_ASSERT_FAIL ? "fail"
                                                            : "not-run";
  const char *op = a->op == IVS_SCN_OP_NE ? "!=" : "==";
  const char *got = ar->state == IVS_RUNNER_ASSERT_NOT_RUN || ar->got[0] == '\0' ? "-" : ar->got;
  char hex[65];
  switch (a->subject) {
  case IVS_SCN_SUBJ_HALTED:
    (void)snprintf(v, cap, "%s halted %s %s got %s", state, op, a->value != 0u ? "yes" : "no", got);
    break;
  case IVS_SCN_SUBJ_REG:
    (void)snprintf(v, cap, "%s reg R%u %s %04X got %s", state, (unsigned)a->index, op, (unsigned)a->value, got);
    break;
  case IVS_SCN_SUBJ_MEM:
    (void)snprintf(v, cap, "%s mem %04X %s %04X got %s", state, (unsigned)a->addr_lo, op, (unsigned)a->value, got);
    break;
  case IVS_SCN_SUBJ_MEM_RANGE:
    (void)snprintf(v, cap, "%s mem %04X..%04X %s %04X got %s", state, (unsigned)a->addr_lo, (unsigned)a->addr_hi, op,
                   (unsigned)a->value, got);
    break;
  case IVS_SCN_SUBJ_DIGEST:
    rp_hex(a->sha256, hex);
    (void)snprintf(v, cap, "%s digest %s %s %s got %s", state, ivs_runner_domain_token(a->index), op, hex, got);
    break;
  default:
    (void)snprintf(v, cap, "%s unknown", state);
    break;
  }
}

int ivs_runner_report_write(const ivs_runner_record *rec, const ivs_runner_host *host, uint64_t *lines)
{
  rp_out o;
  char key[64];
  char v[RP_LINE_MAX - 80u];
  const ivs_scenario *s;
  if (rec == NULL || rec->scn == NULL || host == NULL || host->write_report == NULL) {
    return -1;
  }
  s = rec->scn;
  o.host = host;
  o.lines = 0u;
  o.failed = 0;
  for (size_t i = 0u; i < RP_N(rp_head); i++) {
    rp_value(rec, rp_head[i].field, v, sizeof v);
    rp_emit(&o, rp_head[i].key, v);
  }
  for (uint32_t role = 0u; role < IVS_SCN_ROLES; role++) {
    for (size_t i = 0u; i < RP_N(rp_slot); i++) {
      (void)snprintf(key, sizeof key, "slot.%s.%s", rp_roles[role], rp_slot[i].key);
      rp_slot_value(&rec->slot[role], rp_slot[i].field, v, sizeof v);
      rp_emit(&o, key, v);
    }
  }
  for (size_t i = 0u; i < RP_N(rp_label); i++) {
    rp_value(rec, rp_label[i].field, v, sizeof v);
    rp_emit(&o, rp_label[i].key, v);
  }
  for (uint32_t k = 0u; k < s->n_steps && k < IVS_SCN_MAX_STEPS; k++) {
    for (size_t i = 0u; i < RP_N(rp_step); i++) {
      (void)snprintf(key, sizeof key, "step.%u.%s", (unsigned)(k + 1u), rp_step[i].key);
      rp_step_value(&s->steps[k], &rec->step[k], rp_step[i].field, v, sizeof v);
      rp_emit(&o, key, v);
    }
    for (uint32_t j = 0u; j < s->n_asserts && j < IVS_SCN_MAX_ASSERTS; j++) {
      if (s->asserts[j].step != k) {
        continue;
      }
      (void)snprintf(key, sizeof key, "step.%u.assert.%u", (unsigned)(k + 1u), (unsigned)(j + 1u));
      rp_assert_value(&s->asserts[j], &rec->assert_rec[j], v, sizeof v);
      rp_emit(&o, key, v);
    }
  }
  for (size_t i = 0u; i < RP_N(rp_tail); i++) {
    rp_value(rec, rp_tail[i].field, v, sizeof v);
    rp_emit(&o, rp_tail[i].key, v);
  }
  if (lines != NULL) {
    *lines = o.lines;
  }
  rp_u64(v, sizeof v, o.lines);
  rp_emit(&o, "report.end", v);
  return o.failed ? -1 : 0;
}

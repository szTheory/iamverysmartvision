/* SPDX-License-Identifier: MIT */
/* ivs-inspect's trace pass (Phase 18, plan 18-02, TOOL-02). The frozen contract is
 * docs/inspection.md: this file implements its `## The trace file`, its `## The reconciliation` and
 * the `--report` sentence of its `## Command surface`. Where this file's prose and that page
 * disagree, the page wins.
 *
 * Two passes, and this is the sentence that says why they cannot be one. Pass A is the library's: the
 * caller has already parsed the scenario and this file is handed the report text the library produced
 * through the host's `write_report`, so the report is not this tool's to write and the nine `digest.*`
 * values in it are ivs-run's own for the same scenario and the same options. Pass B is this file's:
 * the executor steps through the public run call, which carries no access log, and its state struct is
 * static inside runner/exec.c, so its live instance is not reachable from a second `main`. Pass B
 * therefore creates its own instance, loads the same bytes the report pass loaded, resets it, and
 * drives it one unit at a time through ivs_machine_step_unit_trace_internal - the seam the `psg`,
 * `stic` and `machine` suites already drive - with the core's own input floor (ivs_input_apply_due)
 * and the core's own halt and fault breaks.
 *
 * The record that carries `digest[9][32]` and `step[k].now_ticks` is declared library-internal by
 * runner/ivs_runner.h, built as a local inside runner/exec.c and written there through
 * ivs_runner_report_write, which takes that record; ivs_runner_execute does not return it, so this
 * tool never holds one and never calls that writer. The oracle is therefore the report TEXT: the nine
 * `digest.*` values and every `step.<k>.now_ticks` are read out of the bytes the library handed the
 * host, which is the stronger reading of the frozen page - the report's own words are what the trace
 * must agree with, not a struct the library could reshape without changing a byte of what it prints.
 *
 * `--report <file>` substitutes a stored report for the run's own as the oracle, and the stored report
 * must agree with the text this run captured: the nine digests and every step's `now_ticks` are
 * compared, so a stored report that disagrees is refused. The library's report is still what the
 * caller prints on stdout, because that is the tool's output and not its oracle; the flag adds no
 * second oracle, it replaces the run's captured numbers with a file's.
 *
 * The loop mirrors the core's own ladder (core/src/machine/machine_bound.c): the limit is the
 * converted target (the executor never sets IVS_RUN_F_STOP_ON_FRAME, so the frame branch of the core's
 * limit rule is unreachable here and the limit is the target, capped by the executor's default machine
 * ceiling); then, per unit, a faulted CPU ends the run, a halted CPU ends it under the stop-on-halt
 * flag the executor always sets, the due input events are applied before the unit begins, and a
 * refused step ends the run rather than becoming an error - exactly as the core's loop treats it. A
 * loop that stepped to the target instead would print the target for a run that stopped below it.
 *
 * The trace is built in memory and written only after all five equalities hold, so a refusal cannot
 * leave a partial file or a printed trace. The tick of an access record is its cycle stamp,
 * unit_start + cycle * ticks_per_cpu_cycle, with the multiplier read from ivs_machine_info and never
 * written as a literal; the end record's tick is the last step record's end, which is the report's own
 * step.<k>.now_ticks.
 *
 * The one cross-file prototype this program needs is declared at both ends - here and in
 * tools/inspect/inspect.c - because plan 18-02 pins the tool to two sources and a header would be a
 * third file. Contract: 0 the trace is written and reconciled, 1 the reconciliation refused (detail
 * carries the reason; the caller prints the named refusal), 2 a file or usage error (detail carries
 * the message; the caller prints it with no token). */

#include <inttypes.h>
#include <stdarg.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "cpu/cp1610.h"
#include "ivs/ivs_machine.h"
#include "ivs/ivs_run.h"
#include "ivs/ivs_status.h"
#include "ivs_runner.h"
#include "machine/machine.h"
#include "psg/psg.h"

#define IXT_OK 0
#define IXT_REFUSED 1
#define IXT_ERROR 2

#define IXT_DETAIL_MAX 320u
#define IXT_TRACE_MAX (64u * 1024u * 1024u)
#define IXT_DOMAINS 9

/* The interface tools/inspect/inspect.c calls. See the file header for the contract. */
int ivs_inspect_trace_run(const ivs_scenario *scn, uint8_t *const role_bytes[IVS_SCN_ROLES],
                          const size_t role_len[IVS_SCN_ROLES], const char *report_text, size_t report_len,
                          const char *stored_text, size_t stored_len, FILE *out, char *detail, size_t detail_cap);

/* The nine domains in the report's own order, and their key names. */
static const uint32_t ixt_domain[IXT_DOMAINS] = {IVS_DIGEST_CPU,      IVS_DIGEST_MEM,   IVS_DIGEST_IMAGES,
                                                 IVS_DIGEST_INSTANCE, IVS_DIGEST_COMPOSITE, IVS_DIGEST_VIDEO,
                                                 IVS_DIGEST_AUDIO,    IVS_DIGEST_INPUT, IVS_DIGEST_PSGLOG};
static const char *const ixt_domain_name[IXT_DOMAINS] = {"cpu",   "mem",   "images", "instance", "composite",
                                                        "video", "audio", "input",  "psglog"};

/* The nine access kinds of core/src/cpu/cp1610.h, lower-cased with the two strobes hyphenated. */
static const char *const ixt_kind_name[10] = {"none",  "fetch", "read",   "write",     "adar",
                                              "intak", "iab",   "dw",     "strobe-tci", "strobe-pcit"};

/* The oracle: the nine digests and every step's end tick, read out of a report's text. */
typedef struct ixt_oracle {
  char digest[IXT_DOMAINS][65];
  int have_digest[IXT_DOMAINS];
  uint64_t step_now[IVS_SCN_MAX_STEPS];
  int have_step[IVS_SCN_MAX_STEPS];
} ixt_oracle;

/* A growable text buffer for the trace. */
typedef struct ixt_buf {
  char *p;
  size_t len;
  size_t cap;
  int bad;
} ixt_buf;

static void ixt_hex32(const uint8_t d[32], char out[65])
{
  static const char digits[] = "0123456789abcdef";
  int i;
  for (i = 0; i < 32; i++) {
    out[2 * i] = digits[d[i] >> 4];
    out[2 * i + 1] = digits[d[i] & 0x0Fu];
  }
  out[64] = '\0';
}

/* 64 lower-case hex digits and nothing else. */
static int ixt_is_hex64(const char *s, size_t n)
{
  size_t i;
  if (n != 64u) {
    return 0;
  }
  for (i = 0u; i < n; i++) {
    char c = s[i];
    if (!((c >= '0' && c <= '9') || (c >= 'a' && c <= 'f'))) {
      return 0;
    }
  }
  return 1;
}

/* Decimal digits only, at least one, no wrap. */
static int ixt_is_dec(const char *s, size_t n, uint64_t *out)
{
  uint64_t v = 0u;
  size_t i;
  if (n == 0u) {
    return 0;
  }
  for (i = 0u; i < n; i++) {
    uint64_t d;
    if (s[i] < '0' || s[i] > '9') {
      return 0;
    }
    d = (uint64_t)(s[i] - '0');
    if (v > (UINT64_MAX - d) / 10u) {
      return 0;
    }
    v = v * 10u + d;
  }
  *out = v;
  return 1;
}

/* Reads one report line into the oracle. A line that is not one of the oracle's two shapes is
 * ignored, so the rest of the report's closed key table costs nothing here. */
static void ixt_take_line(const char *line, size_t n, uint32_t n_steps, ixt_oracle *o)
{
  int i;
  for (i = 0; i < IXT_DOMAINS; i++) {
    size_t nlen = strlen(ixt_domain_name[i]);
    size_t head = 7u + nlen + 1u; /* "digest." + name + ' ' */
    if (n == head + 64u && memcmp(line, "digest.", 7u) == 0 &&
        memcmp(line + 7u, ixt_domain_name[i], nlen) == 0 && line[7u + nlen] == ' ') {
      if (ixt_is_hex64(line + head, 64u)) {
        (void)memcpy(o->digest[i], line + head, 64u);
        o->digest[i][64] = '\0';
        o->have_digest[i] = 1;
      }
      return;
    }
  }
  if (n > 17u && memcmp(line, "step.", 5u) == 0) {
    size_t p = 5u;
    uint64_t k = 0u;
    int digits = 0;
    uint64_t v;
    while (p < n && line[p] >= '0' && line[p] <= '9') {
      k = k * 10u + (uint64_t)(line[p] - '0');
      if (k > IVS_SCN_MAX_STEPS) {
        return;
      }
      p++;
      digits++;
    }
    if (digits == 0 || p + 11u >= n) {
      return;
    }
    if (memcmp(line + p, ".now_ticks ", 11u) != 0) {
      return;
    }
    p += 11u;
    if (!ixt_is_dec(line + p, n - p, &v)) {
      return;
    }
    if (k >= 1u && k <= (uint64_t)n_steps) {
      o->step_now[(size_t)(k - 1u)] = v;
      o->have_step[(size_t)(k - 1u)] = 1;
    }
  }
}

/* Fills o from a report's text. -1 when any of the nine digests or any step's end tick is absent or
 * malformed: an empty extraction would make a later comparison silent for the wrong reason. */
static int ixt_parse_oracle(const char *text, size_t len, uint32_t n_steps, ixt_oracle *o, char *detail,
                            size_t cap)
{
  size_t at = 0u;
  int i;
  uint32_t k;
  memset(o, 0, sizeof *o);
  while (at < len) {
    size_t end = at;
    while (end < len && text[end] != '\n') {
      end++;
    }
    ixt_take_line(text + at, end - at, n_steps, o);
    at = end + 1u;
  }
  for (i = 0; i < IXT_DOMAINS; i++) {
    if (o->have_digest[i] == 0) {
      (void)snprintf(detail, cap, "the report holds no digest.%s line, so there is nothing to reconcile against",
                     ixt_domain_name[i]);
      return -1;
    }
  }
  for (k = 0u; k < n_steps; k++) {
    if (o->have_step[k] == 0) {
      (void)snprintf(detail, cap, "the report holds no step.%" PRIu32 ".now_ticks line", k + 1u);
      return -1;
    }
  }
  return 0;
}

/* Appends formatted text, growing the buffer. bad is set on an allocation failure or when the trace
 * would pass IXT_TRACE_MAX. */
static void ixt_add(ixt_buf *b, const char *fmt, ...)
{
  va_list ap;
  if (b->bad) {
    return;
  }
  if (b->p == NULL) {
    b->cap = 65536u;
    b->p = malloc(b->cap);
    if (b->p == NULL) {
      b->bad = 1;
      return;
    }
  }
  for (;;) {
    size_t room = b->cap - b->len;
    int n;
    va_start(ap, fmt);
    n = vsnprintf(b->p + b->len, room, fmt, ap);
    va_end(ap);
    if (n < 0) {
      b->bad = 1;
      return;
    }
    if ((size_t)n < room) {
      b->len += (size_t)n;
      return;
    }
    {
      size_t want = b->cap;
      char *q;
      while (want < b->len + (size_t)n + 1u) {
        if (want > IXT_TRACE_MAX / 2u) {
          b->bad = 1;
          return;
        }
        want *= 2u;
      }
      q = realloc(b->p, want);
      if (q == NULL) {
        b->bad = 1;
        return;
      }
      b->p = q;
      b->cap = want;
    }
  }
}

/* The stop word of one run-to, classified exactly as the core classifies it: fault, then halt under
 * the stop-on-halt flag, then the target. */
static const char *ixt_stop_name(uint32_t run_state)
{
  if (run_state == IVS_CP1610_RS_FAULTED) {
    return "faulted";
  }
  if (run_state == IVS_CP1610_RS_HALTED) {
    return "halted";
  }
  return "target";
}

int ivs_inspect_trace_run(const ivs_scenario *scn, uint8_t *const role_bytes[IVS_SCN_ROLES],
                          const size_t role_len[IVS_SCN_ROLES], const char *report_text, size_t report_len,
                          const char *stored_text, size_t stored_len, FILE *out, char *detail, size_t detail_cap)
{
  static const uint32_t role_kind[IVS_SCN_ROLES] = {IVS_KIND_EXEC, IVS_KIND_GROM, IVS_KIND_CARTRIDGE};
  static const char *const role_name[IVS_SCN_ROLES] = {"exec", "grom", "cart"};
  ixt_oracle run_oracle;
  ixt_oracle oracle;
  ivs_storage_requirements req;
  ivs_create_options co;
  ivs_info info;
  ivs_input_event sched[IVS_SCN_MAX_INPUTS];
  ixt_buf buf;
  void *storage = NULL;
  ivs_machine *m = NULL;
  uint64_t now;
  uint64_t step_mult;
  uint64_t input_mult;
  uint64_t records = 0u;
  uint64_t units = 0u;
  uint64_t steps = 0u;
  uint64_t last_step_end = 0u;
  uint64_t prev_tick = 0u;
  uint32_t consumed = 0u;
  uint32_t n_sched = 0u;
  uint32_t role;
  uint32_t k;
  uint32_t i;
  int have_prev_tick = 0;
  int ticks_move = 0;
  int have_last = 0;
  int rc = IXT_ERROR;

  detail[0] = '\0';
  memset(&buf, 0, sizeof buf);
  memset(sched, 0, sizeof sched);

  if (scn->n_steps == 0u) {
    (void)snprintf(detail, detail_cap, "the scenario has no run-to step, so there is no run to trace");
    return IXT_ERROR;
  }

  /* The oracle: the report this run printed, and - with --report - a stored one that must agree. */
  if (ixt_parse_oracle(report_text, report_len, scn->n_steps, &run_oracle, detail, detail_cap) != 0) {
    return IXT_REFUSED;
  }
  oracle = run_oracle;
  if (stored_text != NULL) {
    ixt_oracle stored;
    if (ixt_parse_oracle(stored_text, stored_len, scn->n_steps, &stored, detail, detail_cap) != 0) {
      return IXT_REFUSED;
    }
    for (i = 0u; i < IXT_DOMAINS; i++) {
      if (strcmp(stored.digest[i], run_oracle.digest[i]) != 0) {
        (void)snprintf(detail, detail_cap,
                       "the stored report's digest.%s is not the one this run printed, so the two documents "
                       "disagree",
                       ixt_domain_name[i]);
        return IXT_REFUSED;
      }
    }
    for (k = 0u; k < scn->n_steps; k++) {
      if (stored.step_now[k] != run_oracle.step_now[k]) {
        (void)snprintf(detail, detail_cap,
                       "the stored report's step.%" PRIu32
                       ".now_ticks is not the one this run printed, so the two documents disagree",
                       k + 1u);
        return IXT_REFUSED;
      }
    }
    oracle = stored;
  }

  /* Pass B's instance: the profile's own size and alignment, then each role's bytes. */
  memset(&req, 0, sizeof req);
  req.struct_size = (uint32_t)sizeof req;
  {
    ivs_status st = ivs_machine_storage_requirements(scn->profile, &req);
    if (st != IVS_OK) {
      (void)snprintf(detail, detail_cap, "the scenario's profile %s is not registered (status %s)", scn->profile,
                     ivs_status_name(st));
      return IXT_ERROR;
    }
  }
  if (req.align_bytes == 0u || (req.align_bytes & (req.align_bytes - 1u)) != 0u) {
    (void)snprintf(detail, detail_cap, "the profile publishes no usable instance alignment");
    return IXT_ERROR;
  }
  storage = malloc((size_t)req.size_bytes + (size_t)req.align_bytes);
  if (storage == NULL) {
    (void)snprintf(detail, detail_cap, "cannot allocate the traced instance's storage");
    return IXT_ERROR;
  }
  {
    uintptr_t base = (uintptr_t)storage;
    uintptr_t aligned = (base + (uintptr_t)(req.align_bytes - 1u)) & ~(uintptr_t)(req.align_bytes - 1u);
    memset(&co, 0, sizeof co);
    co.struct_size = (uint32_t)sizeof co;
    co.flags = 0u;
    co.profile_id = scn->profile;
    {
      ivs_status st = ivs_machine_create(&co, (void *)aligned, (size_t)req.size_bytes, &m);
      if (st != IVS_OK) {
        (void)snprintf(detail, detail_cap, "the traced instance could not be created (status %s)",
                       ivs_status_name(st));
        goto done;
      }
    }
  }
  for (role = 0u; role < IVS_SCN_ROLES; role++) {
    const ivs_scenario_expect *e = &scn->expect[role];
    ivs_load_options lo;
    ivs_load_report lr;
    ivs_status st;
    if (scn->content[role].kind == IVS_SCN_CONTENT_NONE) {
      continue;
    }
    if (role_bytes[role] == NULL) {
      (void)snprintf(detail, detail_cap, "role %s has no bytes to load", role_name[role]);
      goto done;
    }
    memset(&lo, 0, sizeof lo);
    lo.struct_size = (uint32_t)sizeof lo;
    if (e->sha_set != 0u) {
      lo.flags = IVS_LOAD_F_EXPECT_SHA256;
      (void)memcpy(lo.expect_sha256, e->sha256, sizeof lo.expect_sha256);
    }
    memset(&lr, 0, sizeof lr);
    lr.struct_size = (uint32_t)sizeof lr;
    lr.image.struct_size = (uint32_t)sizeof lr.image;
    st = ivs_machine_load(m, role_kind[role], role_bytes[role], role_len[role], &lo, &lr);
    if (st != IVS_OK) {
      (void)snprintf(detail, detail_cap, "the trace pass could not load role %s (status %s, reason %" PRIu32 ")",
                     role_name[role], ivs_status_name(st), lr.reason);
      goto done;
    }
  }
  {
    ivs_status st = ivs_machine_reset(m, IVS_RESET_POWER_ON);
    if (st != IVS_OK) {
      (void)snprintf(detail, detail_cap, "the traced instance refused the power-on reset (status %s)",
                     ivs_status_name(st));
      goto done;
    }
  }
  memset(&info, 0, sizeof info);
  info.struct_size = (uint32_t)sizeof info;
  {
    ivs_status st = ivs_machine_info(m, &info);
    if (st != IVS_OK || info.ticks_per_cpu_cycle == 0u || info.nominal_frame_ticks == 0u) {
      (void)snprintf(detail, detail_cap, "the traced instance publishes no time base");
      goto done;
    }
  }

  /* The input schedule, in master ticks, with the executor's own unit conversion. */
  input_mult = scn->input_unit == IVS_SCN_INPUT_AT_CYCLE ? (uint64_t)info.ticks_per_cpu_cycle : 1u;
  for (i = 0u; i < scn->n_inputs && i < IVS_SCN_MAX_INPUTS; i++) {
    uint64_t at;
    if (input_mult == 0u || scn->inputs[i].at > IVS_RUN_TARGET_MAX / input_mult) {
      (void)snprintf(detail, detail_cap, "input-out-of-range line=%" PRIu32, scn->inputs[i].line);
      goto done;
    }
    at = scn->inputs[i].at * input_mult;
    if (at >= IVS_RUN_TARGET_MAX) {
      (void)snprintf(detail, detail_cap, "input-out-of-range line=%" PRIu32, scn->inputs[i].line);
      goto done;
    }
    sched[i].at_tick = at;
    sched[i].flags = 0u;
    sched[i].reserved = 0u;
    sched[i].controller[0] = scn->inputs[i].controller[0];
    sched[i].controller[1] = scn->inputs[i].controller[1];
    n_sched++;
  }

  step_mult = 1u;
  if (scn->step_unit == IVS_SCN_UNIT_CPU_CYCLES) {
    step_mult = (uint64_t)info.ticks_per_cpu_cycle;
  } else if (scn->step_unit == IVS_SCN_UNIT_FRAMES) {
    step_mult = info.nominal_frame_ticks;
  }

  /* Pass B: one unit at a time, with the core's own ladder. */
  now = info.time_ticks;
  for (k = 0u; k < scn->n_steps && k < IVS_SCN_MAX_STEPS; k++) {
    uint64_t target_ticks;
    uint64_t limit;
    uint64_t entry;
    ivs_psg_sink sink;
    uint32_t run_state = 0u;
    if (step_mult == 0u || scn->steps[k].target > IVS_RUN_TARGET_MAX / step_mult) {
      (void)snprintf(detail, detail_cap, "bound-too-large step=%" PRIu32, k + 1u);
      goto done;
    }
    target_ticks = scn->steps[k].target * step_mult;
    limit = target_ticks < IVS_RUNNER_MAX_TICKS_DEFAULT ? target_ticks : IVS_RUNNER_MAX_TICKS_DEFAULT;
    entry = now;
    /* One sink per run-to, the shape the core gives one run call: the counting channel for all three
     * observation domains, so the machine state, the log and the samples do not depend on a buffer. */
    ivs_psg_sink_init(&sink, NULL, 0u);
    for (;;) {
      ivs_cpu_state_report cs;
      ivs_cp1610_access trace[IVS_CP1610_MAX_ACCESSES];
      ivs_cp1610_result res;
      uint64_t unit_end;
      ivs_status st;
      uint32_t a;
      if (now >= limit) {
        break;
      }
      memset(&cs, 0, sizeof cs);
      cs.struct_size = (uint32_t)sizeof cs;
      st = ivs_machine_cpu_state(m, &cs);
      if (st != IVS_OK) {
        (void)snprintf(detail, detail_cap, "the traced instance refused its CPU state (status %s)",
                       ivs_status_name(st));
        goto done;
      }
      run_state = cs.run_state;
      if (run_state == IVS_CP1610_RS_FAULTED) {
        break; /* a faulted CPU is never stepped, as in the core's loop */
      }
      if (run_state == IVS_CP1610_RS_HALTED) {
        break; /* the executor always sets IVS_RUN_F_STOP_ON_HALT */
      }
      consumed = ivs_input_apply_due(m, n_sched == 0u ? NULL : sched, n_sched, consumed, now, entry);
      memset(trace, 0, sizeof trace);
      memset(&res, 0, sizeof res);
      st = ivs_machine_step_unit_trace_internal(m, &res, &sink, trace, IVS_CP1610_MAX_ACCESSES);
      if (st != IVS_OK) {
        break; /* a refused step ends the run; the core reports it as an internal stop, not an error */
      }
      unit_end = now + (uint64_t)res.elapsed_cpu_cycles * (uint64_t)info.ticks_per_cpu_cycle;
      for (a = 0u; a < (uint32_t)res.n_accesses && a < IVS_CP1610_MAX_ACCESSES; a++) {
        const ivs_cp1610_access *ac = &trace[a];
        uint64_t tick = now + (uint64_t)ac->offset_cpu_cycles * (uint64_t)info.ticks_per_cpu_cycle;
        if (ac->kind == 0u || ac->kind > 9u) {
          (void)snprintf(detail, detail_cap, "the core reported an access kind this tool does not know");
          goto done;
        }
        ixt_add(&buf,
                "tick=%" PRIu64 " kind=%s addr=%04X value=%04X wait=%u cycle=%u unit_start=%" PRIu64
                " unit_end=%" PRIu64 "\n",
                tick, ixt_kind_name[ac->kind], (unsigned)ac->addr, (unsigned)ac->data,
                (unsigned)ac->wait_cpu_cycles, (unsigned)ac->offset_cpu_cycles, now, unit_end);
        records++;
        if (have_prev_tick != 0 && tick < prev_tick) {
          (void)snprintf(detail, detail_cap,
                         "access tick %" PRIu64 " follows %" PRIu64 ", so the trace is not cycle-monotonic", tick,
                         prev_tick);
          goto done;
        }
        if (have_prev_tick != 0 && tick != prev_tick) {
          ticks_move = 1;
        }
        prev_tick = tick;
        have_prev_tick = 1;
      }
      if (unit_end == now) {
        (void)snprintf(detail, detail_cap, "a unit of the traced run advanced no machine time");
        goto done;
      }
      now = unit_end;
      units++;
    }
    steps++;
    ixt_add(&buf, "step=%" PRIu64 " target=%" PRIu64 " end=%" PRIu64 " stop=%s\n", steps, target_ticks, now,
            ixt_stop_name(run_state));
    if (now != oracle.step_now[k]) {
      (void)snprintf(detail, detail_cap,
                     "step %" PRIu64 " ends at %" PRIu64 " and the report's step.%" PRIu32
                     ".now_ticks is %" PRIu64 ", so the trace is not the run the report describes",
                     steps, now, k + 1u, oracle.step_now[k]);
      goto done;
    }
    last_step_end = now;
    have_last = 1;
  }
  if (have_last == 0) {
    (void)snprintf(detail, detail_cap, "the traced run produced no step record");
    goto done;
  }
  if (buf.bad != 0) {
    (void)snprintf(detail, detail_cap, "the trace is larger than %u bytes, or could not be built",
                   (unsigned)IXT_TRACE_MAX);
    goto done;
  }
  /* The fifth equality: non-decreasing (checked per record above) AND not all equal. A trace whose
   * every access carries one tick is non-decreasing and asserts nothing, so it is refused here. */
  if (records == 0u || ticks_move == 0) {
    (void)snprintf(detail, detail_cap,
                   "the trace holds %" PRIu64
                   " access records and one distinct tick value at most, so it asserts nothing",
                   records);
    goto done;
  }
  /* The end record, last in the file and the only record that is not an observation. */
  ixt_add(&buf,
          "end tick=%" PRIu64 " records=%" PRIu64 " units=%" PRIu64 " steps=%" PRIu64
          " break_pc=none break_hits=0 other_pc=none other_hits=0\n",
          last_step_end, records, units, steps);
  if (buf.bad != 0) {
    (void)snprintf(detail, detail_cap, "the trace is larger than %u bytes, or could not be built",
                   (unsigned)IXT_TRACE_MAX);
    goto done;
  }
  /* The second equality: the traced instance's nine digests, byte for byte against the report's own. */
  for (i = 0u; i < IXT_DOMAINS; i++) {
    uint8_t d[32];
    char hex[65];
    ivs_status st = ivs_machine_state_digest(m, ixt_domain[i], d);
    if (st != IVS_OK) {
      (void)snprintf(detail, detail_cap, "the traced instance refused digest.%s (status %s)",
                     ixt_domain_name[i], ivs_status_name(st));
      goto done;
    }
    ixt_hex32(d, hex);
    if (strcmp(hex, oracle.digest[i]) != 0) {
      (void)snprintf(detail, detail_cap,
                     "the traced instance's digest.%s is %s and the report printed %s, so the two sweeps are not "
                     "the same run",
                     ixt_domain_name[i], hex, oracle.digest[i]);
      goto done;
    }
  }
  /* Every equality holds: only now does the trace reach the output. */
  if (fwrite(buf.p, 1u, buf.len, out) != buf.len) {
    (void)snprintf(detail, detail_cap, "a write error on the trace's output");
    goto done;
  }
  if (fflush(out) != 0) {
    (void)snprintf(detail, detail_cap, "a write error on the trace's output");
    goto done;
  }
  rc = IXT_OK;

done:
  if (m != NULL) {
    (void)ivs_machine_destroy(m);
  }
  free(storage);
  free(buf.p);
  return rc;
}

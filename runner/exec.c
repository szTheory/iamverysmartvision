/* SPDX-License-Identifier: MIT */
/* Runner scenario executor (D-08, D-09, D-17, D-20, D-22, D-30, plan 04-18). One scenario on one
 * fresh instance the executor creates in the host's storage buffer, aligned by hand; the public
 * API only (ivs/ivs.h); no globals and no allocation (the host owns every buffer). Each run-to is
 * one ivs_machine_run call in this plan (slices, budget ceiling and wall cap: plan 04-19), always
 * with IVS_RUN_F_STOP_ON_HALT so a halt is a stop the scenario must expect. The outcome is the
 * pure precedence of ivs_runner_outcome_resolve over what the run observed (outcome.c), except a
 * load refusal that matches expect-load, which the executor decides as pass before that call.
 * C rule (hosted-only hazard): one call per statement when one argument reads state another
 * argument's call changes. Named-fault sites (data/expected/faults/runner.ids) sit in this TU. */
#include <stddef.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>

#include "ivs/ivs.h"
#include "ivs_runner.h"

/* Observation slots for the first detail of each kind (index = bit number of IVS_RUNNER_OBS_*). */
#define EX_OBS_BITS 6u
#define EX_READ_BUF (IVS_RUNNER_CAP_CART + 1u)

typedef struct ex_state {
  const ivs_scenario *scn;
  const ivs_runner_host *host;
  ivs_runner_record *rec;
  ivs_machine *m;
  uint8_t *readbuf; /* EX_READ_BUF bytes of host storage after the instance */
  uint32_t observed;
  char detail[EX_OBS_BITS][IVS_RUNNER_DETAIL_MAX];
} ex_state;

static const uint32_t ex_role_kind[IVS_SCN_ROLES] = {IVS_KIND_EXEC, IVS_KIND_GROM, IVS_KIND_CARTRIDGE};
static const char *const ex_role_name[IVS_SCN_ROLES] = {"exec", "grom", "cart"};
static const size_t ex_role_cap[IVS_SCN_ROLES] = {IVS_RUNNER_CAP_EXEC, IVS_RUNNER_CAP_GROM, IVS_RUNNER_CAP_CART};

/* Digest rows in record order (report.c prints them in this order). */
static const uint32_t ex_domains[8] = {IVS_DIGEST_CPU,       IVS_DIGEST_MEM,   IVS_DIGEST_IMAGES, IVS_DIGEST_INSTANCE,
                                       IVS_DIGEST_COMPOSITE, IVS_DIGEST_VIDEO, IVS_DIGEST_AUDIO,  IVS_DIGEST_INPUT};

/* Records an observation; the first detail of each kind is kept. */
static void ex_observe(ex_state *x, uint32_t bit, const char *detail)
{
  for (uint32_t i = 0u; i < EX_OBS_BITS; i++) {
    if (bit == (1u << i)) {
      if ((x->observed & bit) == 0u) {
        (void)snprintf(x->detail[i], sizeof x->detail[i], "%s", detail);
      }
      break;
    }
  }
  x->observed |= bit;
}

static const char *ex_status_token(ivs_status st) { return st == IVS_OK ? "ok" : ivs_status_name(st); }

uint32_t ivs_runner_stop_observation(uint32_t stop_reason, uint32_t expected_stop)
{
  if (expected_stop != IVS_SCN_STOP_TARGET && expected_stop != IVS_SCN_STOP_HALTED &&
      expected_stop != IVS_SCN_STOP_FAULTED) {
    return IVS_RUNNER_OBS_HARNESS_FAIL;
  }
  switch (stop_reason) {
  case IVS_STOP_TARGET:
    return expected_stop == IVS_SCN_STOP_TARGET ? 0u : IVS_RUNNER_OBS_TIMEOUT_BUDGET;
  case IVS_STOP_CPU_HALTED:
    return expected_stop == IVS_SCN_STOP_HALTED ? 0u : IVS_RUNNER_OBS_GUEST_FAIL;
  case IVS_STOP_GUEST_FAULT:
    return expected_stop == IVS_SCN_STOP_FAULTED ? 0u : IVS_RUNNER_OBS_GUEST_FAIL;
  default: /* IVS_STOP_INTERNAL (D-04: a harness failure) or not a stop reason */
    return IVS_RUNNER_OBS_HARNESS_FAIL;
  }
}

/* ---- host calls, each counted for host.calls ---- */

static int ex_cancelled(ex_state *x)
{
  if (x->host->cancel_requested == NULL) {
    return 0;
  }
  x->rec->host_calls++;
  return x->host->cancel_requested(x->host->ctx) != 0;
}

/* Reads the bytes of one role into x->readbuf; returns an IVS_RUNNER_READ_* result and the
 * length in *len (zero-grom is made here, without a host call). */
static int ex_read_role(ex_state *x, uint32_t role, size_t *len)
{
  const ivs_scenario_content *c = &x->scn->content[role];
  size_t cap = ex_role_cap[role];
  int r;
  *len = 0u;
  switch (c->kind) {
  case IVS_SCN_CONTENT_ZERO_GROM:
    memset(x->readbuf, 0, IVS_RUNNER_CAP_GROM);
    *len = IVS_RUNNER_CAP_GROM;
    return IVS_RUNNER_READ_OK;
  case IVS_SCN_CONTENT_BOOT_STUB:
    x->rec->host_calls++;
    r = x->host->read_file(x->host->ctx, IVS_RUNNER_BOOT_STUB_FILE, cap, x->readbuf, len);
    break;
  case IVS_SCN_CONTENT_PATH:
    x->rec->host_calls++;
    r = x->host->read_file(x->host->ctx, c->path, cap, x->readbuf, len);
    break;
  case IVS_SCN_CONTENT_USER:
    if (x->host->read_user == NULL) {
      return IVS_RUNNER_READ_MISSING;
    }
    x->rec->host_calls++;
    r = x->host->read_user(x->host->ctx, role, cap, x->readbuf, len);
    break;
  default:
    return IVS_RUNNER_READ_IO_ERROR;
  }
  if (*len > cap + 1u) {
    return IVS_RUNNER_READ_IO_ERROR; /* the host broke its contract */
  }
  if (r == IVS_RUNNER_READ_TOO_LARGE && *len != cap + 1u) {
    return IVS_RUNNER_READ_IO_ERROR;
  }
  if (r == IVS_RUNNER_READ_OK && *len > cap) {
    return IVS_RUNNER_READ_TOO_LARGE;
  }
  return r;
}

/* The name a detail gives a role's content: path=<relpath>, path=boot-stub.exec or user. */
static void ex_content_name(const ex_state *x, uint32_t role, char *out, size_t cap)
{
  const ivs_scenario_content *c = &x->scn->content[role];
  if (c->kind == IVS_SCN_CONTENT_USER) {
    (void)snprintf(out, cap, "user");
  } else if (c->kind == IVS_SCN_CONTENT_BOOT_STUB) {
    (void)snprintf(out, cap, "path=%s", IVS_RUNNER_BOOT_STUB_FILE);
  } else if (c->kind == IVS_SCN_CONTENT_ZERO_GROM) {
    (void)snprintf(out, cap, "zero-grom");
  } else {
    (void)snprintf(out, cap, "path=%s", c->path);
  }
}

/* Loads every role with a content line. Returns 1 to go on, 0 when the scenario ends here (an
 * observation was made, or a refusal matched expect-load: pass, with a detail). */
static int ex_load_all(ex_state *x)
{
  char what[IVS_SCN_MAX_PATH + 16u];
  char d[IVS_RUNNER_DETAIL_MAX];
  for (uint32_t role = 0u; role < IVS_SCN_ROLES; role++) {
    const ivs_scenario_expect *e = &x->scn->expect[role];
    ivs_runner_slot_record *sl = &x->rec->slot[role];
    ivs_load_options lo;
    ivs_load_report lr;
    ivs_status st;
    size_t len = 0u;
    int r;
    int32_t want;
    if (x->scn->content[role].kind == IVS_SCN_CONTENT_NONE) {
      continue;
    }
    r = ex_read_role(x, role, &len);
    ex_content_name(x, role, what, sizeof what);
    if (r == IVS_RUNNER_READ_MISSING || r == IVS_RUNNER_READ_NOT_REGULAR) {
      (void)snprintf(d, sizeof d, "%s role=%s %s", r == IVS_RUNNER_READ_MISSING ? "missing" : "not-regular",
                     ex_role_name[role], what);
      ex_observe(x, IVS_RUNNER_OBS_MISSING_FIXTURE, d);
      return 0;
    }
    if (r != IVS_RUNNER_READ_OK && r != IVS_RUNNER_READ_TOO_LARGE) {
      (void)snprintf(d, sizeof d, "io-error role=%s %s", ex_role_name[role], what);
      ex_observe(x, IVS_RUNNER_OBS_HARNESS_FAIL, d);
      return 0;
    }
    memset(&lo, 0, sizeof lo);
    lo.struct_size = (uint32_t)sizeof lo;
    if (e->sha_set) {
      lo.flags = IVS_LOAD_F_EXPECT_SHA256;
      memcpy(lo.expect_sha256, e->sha256, sizeof lo.expect_sha256);
    }
    memset(&lr, 0, sizeof lr);
    lr.struct_size = (uint32_t)sizeof lr;
    lr.image.struct_size = (uint32_t)sizeof lr.image;
    st = ivs_machine_load(x->m, ex_role_kind[role], x->readbuf, len, &lo, &lr);
    sl->attempted = 1u;
    sl->status = (int32_t)st;
    sl->reason = st == IVS_OK ? IVS_REASON_NONE : lr.reason;
    if (st == IVS_OK) {
      sl->loaded = lr.image.loaded;
      sl->length_bytes = lr.image.length_bytes;
      memcpy(sl->sha256, lr.image.sha256, sizeof sl->sha256);
    }
    want = e->load_set ? e->load_status : IVS_OK;
    if (st == IVS_OK && want == IVS_OK) {
      continue;
    }
    if (st != IVS_OK && st == want && (e->load_reason == 0u || e->load_reason == lr.reason)) {
      /* D-20: a refusal the scenario expects ends it as pass. */
      (void)snprintf(x->rec->detail, sizeof x->rec->detail, "load-refused-as-expected role=%s status=%s reason=%u",
                     ex_role_name[role], ivs_status_name(st), (unsigned)lr.reason);
      return 0;
    }
    if (st == IVS_OK) {
      (void)snprintf(d, sizeof d, "load-not-refused role=%s expected=%s", ex_role_name[role], ivs_status_name(want));
    } else {
      (void)snprintf(d, sizeof d, "load-refused role=%s status=%s reason=%u", ex_role_name[role], ex_status_token(st),
                     (unsigned)lr.reason);
    }
    ex_observe(x, IVS_RUNNER_OBS_HARNESS_FAIL, d);
    return 0;
  }
  return 1;
}

/* n * mult as a tick count; 0 when it exceeds IVS_RUN_TARGET_MAX or mult is 0. */
static int ex_ticks(uint64_t n, uint64_t mult, uint64_t *out)
{
  if (mult == 0u || n > IVS_RUN_TARGET_MAX / mult) {
    return 0;
  }
  *out = n * mult;
  return 1;
}

/* ---- asserts (D-08: observation through cpu_state, peek and state_digest only) ---- */

/* Evaluates assert a into ar. Returns 1 when it held, 0 when it failed, -1 when an observation
 * call was refused (a harness failure). */
static int ex_assert_eval(ex_state *x, const ivs_scenario_assert *a, ivs_runner_assert_record *ar)
{
  int held = 0;
  ivs_status st;
  switch (a->subject) {
  case IVS_SCN_SUBJ_HALTED:
  case IVS_SCN_SUBJ_REG: {
    ivs_cpu_state_report c;
    memset(&c, 0, sizeof c);
    c.struct_size = (uint32_t)sizeof c;
    st = ivs_machine_cpu_state(x->m, &c);
    if (st != IVS_OK) {
      return -1;
    }
    if (a->subject == IVS_SCN_SUBJ_HALTED) {
      uint16_t halted = (uint16_t)(c.run_state == IVS_CPU_STATE_HALTED ? 1u : 0u);
      held = halted == a->value;
      (void)snprintf(ar->got, sizeof ar->got, "%s", halted ? "yes" : "no");
    } else {
      if (a->index > 7u) {
        return -1;
      }
      held = c.r[a->index] == a->value;
      (void)snprintf(ar->got, sizeof ar->got, "%04X", (unsigned)c.r[a->index]);
    }
    break;
  }
  case IVS_SCN_SUBJ_MEM:
  case IVS_SCN_SUBJ_MEM_RANGE: {
    /* Every word of [lo, hi] equals the value; got is the common value or the first word that
     * differs, as AAAA=HHHH. */
    uint16_t words[64];
    uint32_t addr = a->addr_lo;
    uint32_t hi = a->addr_hi;
    int all = 1;
    if (hi < addr) {
      return -1;
    }
    (void)snprintf(ar->got, sizeof ar->got, "%04X", 0u);
    while (addr <= hi && all) {
      uint32_t n = hi - addr + 1u;
      if (n > 64u) {
        n = 64u;
      }
      st = ivs_machine_peek(x->m, (uint16_t)addr, n, words);
      if (st != IVS_OK) {
        return -1;
      }
      for (uint32_t i = 0u; i < n; i++) {
        if (words[i] != a->value) {
          all = 0;
          if (a->subject == IVS_SCN_SUBJ_MEM) {
            (void)snprintf(ar->got, sizeof ar->got, "%04X", (unsigned)words[i]);
          } else {
            (void)snprintf(ar->got, sizeof ar->got, "%04X=%04X", (unsigned)(addr + i), (unsigned)words[i]);
          }
          break;
        }
      }
      addr += n;
    }
    if (all) {
      (void)snprintf(ar->got, sizeof ar->got, "%04X", (unsigned)a->value);
    }
    held = all;
    break;
  }
  case IVS_SCN_SUBJ_DIGEST: {
    static const char digits[] = "0123456789abcdef";
    uint8_t sha[32];
    st = ivs_machine_state_digest(x->m, a->index, sha);
    if (st != IVS_OK) {
      return -1;
    }
    held = memcmp(sha, a->sha256, sizeof sha) == 0;
    for (size_t i = 0u; i < 32u; i++) {
      ar->got[2u * i] = digits[sha[i] >> 4];
      ar->got[2u * i + 1u] = digits[sha[i] & 0x0Fu];
    }
    ar->got[64] = '\0';
    break;
  }
  default:
    return -1;
  }
  if (a->op == IVS_SCN_OP_NE) {
    held = !held;
  } else if (a->op != IVS_SCN_OP_EQ) {
    return -1;
  }
  return held ? 1 : 0;
}

/* The clause of an assert as the scenario states it: `<subject> <op> <expected>`. */
static void ex_assert_text(const ivs_scenario_assert *a, char *out, size_t cap)
{
  static const char digits[] = "0123456789abcdef";
  const char *op = a->op == IVS_SCN_OP_NE ? "!=" : "==";
  char hex[65];
  switch (a->subject) {
  case IVS_SCN_SUBJ_HALTED:
    (void)snprintf(out, cap, "halted %s %s", op, a->value != 0u ? "yes" : "no");
    break;
  case IVS_SCN_SUBJ_REG:
    (void)snprintf(out, cap, "reg R%u %s %04X", (unsigned)a->index, op, (unsigned)a->value);
    break;
  case IVS_SCN_SUBJ_MEM:
    (void)snprintf(out, cap, "mem %04X %s %04X", (unsigned)a->addr_lo, op, (unsigned)a->value);
    break;
  case IVS_SCN_SUBJ_MEM_RANGE:
    (void)snprintf(out, cap, "mem %04X..%04X %s %04X", (unsigned)a->addr_lo, (unsigned)a->addr_hi, op,
                   (unsigned)a->value);
    break;
  case IVS_SCN_SUBJ_DIGEST:
    for (size_t i = 0u; i < 32u; i++) {
      hex[2u * i] = digits[a->sha256[i] >> 4];
      hex[2u * i + 1u] = digits[a->sha256[i] & 0x0Fu];
    }
    hex[64] = '\0';
    (void)snprintf(out, cap, "digest %s %s %s", ivs_runner_domain_token(a->index), op, hex);
    break;
  default:
    (void)snprintf(out, cap, "unknown");
    break;
  }
}

/* Evaluates the asserts of step k; a failure is a guest-fail naming the first failing one. */
static void ex_step_asserts(ex_state *x, uint32_t k)
{
  for (uint32_t j = 0u; j < x->scn->n_asserts; j++) {
    const ivs_scenario_assert *a = &x->scn->asserts[j];
    ivs_runner_assert_record *ar = &x->rec->assert_rec[j];
    char d[IVS_RUNNER_DETAIL_MAX];
    int r;
    if (a->step != k) {
      continue;
    }
    r = ex_assert_eval(x, a, ar);
    if (r < 0) {
      ar->state = IVS_RUNNER_ASSERT_NOT_RUN;
      (void)snprintf(d, sizeof d, "observe-refused assert=%u line=%u", (unsigned)(j + 1u), (unsigned)a->line);
      ex_observe(x, IVS_RUNNER_OBS_HARNESS_FAIL, d);
      return;
    }
    ar->state = r == 1 ? IVS_RUNNER_ASSERT_PASS : IVS_RUNNER_ASSERT_FAIL;
    if (ar->state == IVS_RUNNER_ASSERT_FAIL) {
      char clause[IVS_RUNNER_DETAIL_MAX / 2u];
      ex_assert_text(a, clause, sizeof clause);
      (void)snprintf(d, sizeof d, "assert=%u line=%u %s got %s", (unsigned)(j + 1u), (unsigned)a->line, clause,
                     ar->got);
      ex_observe(x, IVS_RUNNER_OBS_GUEST_FAIL, d);
    }
  }
}

/* ---- steps ---- */

static void ex_run_steps(ex_state *x, const ivs_info *info)
{
  const ivs_scenario *s = x->scn;
  ivs_input_event events[IVS_SCN_MAX_INPUTS];
  uint32_t consumed = 0u;
  char d[IVS_RUNNER_DETAIL_MAX];
  uint64_t step_mult = 1u;
  uint64_t input_mult = s->input_unit == IVS_SCN_INPUT_AT_CYCLE ? info->ticks_per_cpu_cycle : 1u;
  if (s->step_unit == IVS_SCN_UNIT_CPU_CYCLES) {
    step_mult = info->ticks_per_cpu_cycle;
  } else if (s->step_unit == IVS_SCN_UNIT_FRAMES) {
    step_mult = info->nominal_frame_ticks;
  }
  /* The input schedule in master ticks (D-09, D-13); words 2 and 3 stay 0. */
  memset(events, 0, sizeof events);
  for (uint32_t i = 0u; i < s->n_inputs && i < IVS_SCN_MAX_INPUTS; i++) {
    if (!ex_ticks(s->inputs[i].at, input_mult, &events[i].at_tick) || events[i].at_tick >= IVS_RUN_TARGET_MAX) {
      (void)snprintf(d, sizeof d, "input-out-of-range line=%u", (unsigned)s->inputs[i].line);
      ex_observe(x, IVS_RUNNER_OBS_HARNESS_FAIL, d);
      return;
    }
    events[i].controller[0] = s->inputs[i].controller[0];
    events[i].controller[1] = s->inputs[i].controller[1];
  }
  for (uint32_t k = 0u; k < s->n_steps && k < IVS_SCN_MAX_STEPS; k++) {
    const ivs_scenario_step *st = &s->steps[k];
    ivs_runner_step_record *sr = &x->rec->step[k];
    ivs_run_request q;
    ivs_run_report rep;
    ivs_status rs;
    uint32_t sobs;
    if (!ex_ticks(st->target, step_mult, &sr->target_ticks)) {
      (void)snprintf(d, sizeof d, "bound-too-large step=%u line=%u", (unsigned)(k + 1u), (unsigned)st->line);
      ex_observe(x, IVS_RUNNER_OBS_HARNESS_FAIL, d);
      return;
    }
    sr->converted = 1u;
    if (ex_cancelled(x)) {
      (void)snprintf(d, sizeof d, "signal step=%u", (unsigned)(k + 1u));
      ex_observe(x, IVS_RUNNER_OBS_CANCELLED, d);
      return;
    }
    memset(&q, 0, sizeof q);
    q.struct_size = (uint32_t)sizeof q;
    q.flags = IVS_RUN_F_STOP_ON_HALT;
    q.target_ticks = sr->target_ticks;
    if (info->video_bytes_per_frame != 0u || info->audio_bytes_per_frame != 0u) {
      size_t half = x->host->buffer_bytes > IVS_RUNNER_BUFFERS_MAX ? IVS_RUNNER_BUFFERS_MAX / 2u
                                                                   : x->host->buffer_bytes / 2u;
      if (x->host->buffers != NULL && half > 0u) {
        q.video = x->host->buffers;
        q.video_capacity = half;
        q.audio = x->host->buffers + half;
        q.audio_capacity = half;
      }
    }
    q.events = consumed < s->n_inputs ? &events[consumed] : NULL;
    q.event_count = s->n_inputs - consumed;
    memset(&rep, 0, sizeof rep);
    rep.struct_size = (uint32_t)sizeof rep;
    rs = ivs_machine_run(x->m, &q, &rep);
    if (rs != IVS_OK) {
      (void)snprintf(d, sizeof d, "run-refused step=%u status=%s reason=%u", (unsigned)(k + 1u), ivs_status_name(rs),
                     (unsigned)rep.refusal_reason);
      ex_observe(x, IVS_RUNNER_OBS_HARNESS_FAIL, d);
      return;
    }
    sr->ran = 1u;
    sr->now_ticks = rep.now_ticks;
    sr->overshoot_ticks = rep.overshoot_ticks;
    sr->stop_reason = rep.stop_reason;
    consumed += rep.input_consumed;
    x->rec->have_input = 1u;
    x->rec->input_consumed = consumed;
    memcpy(x->rec->controller, rep.controller_now, sizeof x->rec->controller);
    x->rec->video_bytes += rep.video_bytes_written;
    x->rec->audio_bytes += rep.audio_bytes_written;
    sobs = ivs_runner_stop_observation(rep.stop_reason, st->stop);
    if ((sobs & IVS_RUNNER_OBS_HARNESS_FAIL) != 0u) {
      (void)snprintf(d, sizeof d, "stop-internal step=%u line=%u got=%s", (unsigned)(k + 1u), (unsigned)st->line,
                     ivs_runner_stop_token(rep.stop_reason));
      ex_observe(x, IVS_RUNNER_OBS_HARNESS_FAIL, d);
      return;
    }
    if (sobs != 0u) {
      (void)snprintf(d, sizeof d, "%s step=%u line=%u expected=%s got=%s",
                     sobs == IVS_RUNNER_OBS_TIMEOUT_BUDGET ? "budget" : "stop", (unsigned)(k + 1u),
                     (unsigned)st->line, ivs_runner_expected_stop_token(st->stop),
                     ivs_runner_stop_token(rep.stop_reason));
      ex_observe(x, sobs, d);
    }
    ex_step_asserts(x, k);
    if (x->observed != 0u) {
      return;
    }
  }
}

/* Final observation for the report: CPU state and every digest domain. */
static void ex_final(ex_state *x)
{
  ivs_cpu_state_report c;
  memset(&c, 0, sizeof c);
  c.struct_size = (uint32_t)sizeof c;
  if (ivs_machine_cpu_state(x->m, &c) != IVS_OK) {
    ex_observe(x, IVS_RUNNER_OBS_HARNESS_FAIL, "observe-refused final");
    return;
  }
  x->rec->cpu_run_state = c.run_state;
  x->rec->cpu_fault_reason = c.fault_reason;
  for (size_t i = 0u; i < 8u; i++) {
    if (ivs_machine_state_digest(x->m, ex_domains[i], x->rec->digest[i]) != IVS_OK) {
      ex_observe(x, IVS_RUNNER_OBS_HARNESS_FAIL, "observe-refused final");
      return;
    }
  }
  x->rec->have_machine = 1u;
}

/* Creates the instance in host storage, aligned by hand (D-17, D-22). */
static int ex_create(ex_state *x)
{
  ivs_storage_requirements req;
  ivs_create_options co;
  ivs_status st;
  uintptr_t base;
  uintptr_t aligned;
  size_t usable;
  size_t pad;
  char d[IVS_RUNNER_DETAIL_MAX];
  memset(&req, 0, sizeof req);
  req.struct_size = (uint32_t)sizeof req;
  st = ivs_machine_storage_requirements(x->scn->profile, &req);
  if (st != IVS_OK) {
    (void)snprintf(d, sizeof d, "profile-refused status=%s", ivs_status_name(st));
    ex_observe(x, IVS_RUNNER_OBS_HARNESS_FAIL, d);
    return 0;
  }
  usable = x->host->storage_bytes > IVS_RUNNER_STORAGE_MAX ? IVS_RUNNER_STORAGE_MAX : x->host->storage_bytes;
  if (x->host->storage == NULL || req.align_bytes == 0u || (req.align_bytes & (req.align_bytes - 1u)) != 0u ||
      req.size_bytes > usable) {
    ex_observe(x, IVS_RUNNER_OBS_HARNESS_FAIL, "storage-too-small");
    return 0;
  }
  base = (uintptr_t)x->host->storage;
  aligned = (base + (uintptr_t)(req.align_bytes - 1u)) & ~(uintptr_t)(req.align_bytes - 1u);
  pad = (size_t)(aligned - base);
  if (pad > usable || usable - pad < req.size_bytes || usable - pad - (size_t)req.size_bytes < EX_READ_BUF) {
    ex_observe(x, IVS_RUNNER_OBS_HARNESS_FAIL, "storage-too-small");
    return 0;
  }
  memset(&co, 0, sizeof co);
  co.struct_size = (uint32_t)sizeof co;
  co.profile_id = x->scn->profile;
  st = ivs_machine_create(&co, (void *)aligned, (size_t)req.size_bytes, &x->m);
  if (st != IVS_OK) {
    x->m = NULL;
    (void)snprintf(d, sizeof d, "create-refused status=%s", ivs_status_name(st));
    ex_observe(x, IVS_RUNNER_OBS_HARNESS_FAIL, d);
    return 0;
  }
  x->readbuf = (uint8_t *)aligned + (size_t)req.size_bytes;
  return 1;
}

uint32_t ivs_runner_execute(const ivs_scenario *scn, const ivs_runner_host *host, const ivs_runner_options *opts,
                            ivs_runner_result *result)
{
  ivs_runner_record rec;
  ex_state x;
  ivs_info info;
  uint64_t lines = 0u;
  uint32_t outcome;
  int wrote;
  if (result != NULL) {
    memset(result, 0, sizeof *result);
    result->outcome = IVS_RUNNER_OUTCOME_HARNESS_FAIL;
    result->exit_code = ivs_runner_exit_code(IVS_RUNNER_OUTCOME_HARNESS_FAIL);
  }
  if (scn == NULL || host == NULL || host->read_file == NULL || host->write_report == NULL ||
      (opts != NULL && (opts->flags != 0u || opts->reserved != 0u))) {
    return IVS_RUNNER_OUTCOME_HARNESS_FAIL;
  }
  memset(&rec, 0, sizeof rec);
  memset(&x, 0, sizeof x);
  rec.scn = scn;
  x.scn = scn;
  x.host = host;
  x.rec = &rec;
  if (ex_create(&x) && ex_load_all(&x)) {
    ivs_status st = ivs_machine_reset(x.m, IVS_RESET_POWER_ON);
    if (st != IVS_OK) {
      char d[IVS_RUNNER_DETAIL_MAX];
      (void)snprintf(d, sizeof d, "reset-refused status=%s", ivs_status_name(st));
      ex_observe(&x, IVS_RUNNER_OBS_HARNESS_FAIL, d);
    } else {
      memset(&info, 0, sizeof info);
      info.struct_size = (uint32_t)sizeof info;
      st = ivs_machine_info(x.m, &info);
      if (st != IVS_OK || info.ticks_per_cpu_cycle == 0u || info.nominal_frame_ticks == 0u) {
        ex_observe(&x, IVS_RUNNER_OBS_HARNESS_FAIL, "no-timebase");
      } else {
        ex_run_steps(&x, &info);
      }
    }
  }
  if (x.m != NULL) {
    ex_final(&x);
  }
  outcome = ivs_runner_outcome_resolve(x.observed);
  if (x.observed != 0u) {
    /* The detail of the winning observation (the first of its kind). */
    for (uint32_t i = 0u; i < EX_OBS_BITS; i++) {
      uint32_t bit = 1u << i;
      if ((x.observed & bit) != 0u && ivs_runner_outcome_resolve(bit) == outcome) {
        (void)snprintf(rec.detail, sizeof rec.detail, "%s", x.detail[i]);
        break;
      }
    }
  }
  rec.outcome = outcome;
  if (x.m != NULL) {
    (void)ivs_machine_destroy(x.m);
    x.m = NULL;
  }
  wrote = ivs_runner_report_write(&rec, host, &lines);
  if (wrote != 0) {
    outcome = IVS_RUNNER_OUTCOME_HARNESS_FAIL;
  }
  if (result != NULL) {
    result->outcome = outcome;
    result->exit_code = ivs_runner_exit_code(outcome);
    result->host_calls = rec.host_calls;
    result->report_lines = lines;
    result->report_write_failed = wrote != 0 ? 1u : 0u;
  }
  return outcome;
}

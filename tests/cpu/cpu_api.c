/* SPDX-License-Identifier: MIT */
/* Suite cpu-api: the internal CPU contract (D-02, D-03, D-06, D-12). State and access sizes,
 * state validation, refusal without mutation, access-log rules, determinism and instance
 * isolation, the no-progress guard, sticky faults, HALTED steps and wait-shifted offsets. The
 * shared fixture programs use CLRC ($006) and SETC ($007); the plan 02-15 cases build their own
 * small programs (NOP, MVI, a refused MVO@). */
#include <stdint.h>
#include <stdio.h>
#include <string.h>

#include "cpu/cp1610.h"
#include "ivs_test.h"
#include "ivs_testbus.h"

/* Fails the case with a diagnostic naming the line and the failed expression. */
#define IVS_CHECK(cond)                                                                                \
  do {                                                                                                 \
    if (!(cond)) {                                                                                     \
      ivs_test_diag("check failed at line %d: %s", __LINE__, #cond);                                   \
      return IVS_TEST_FAIL;                                                                            \
    }                                                                                                  \
  } while (0)

/* ---- sizes (D-02, D-03) ---- */

static int api_state_size_32(void)
{
  IVS_CHECK(sizeof(ivs_cp1610) == 32u);
  IVS_CHECK(ivs_cp1610_state_size() == 32u);
  return IVS_TEST_PASS;
}

static int api_access_size_12(void)
{
  IVS_CHECK(sizeof(ivs_cp1610_access) == 12u);
  IVS_CHECK(IVS_CP1610_MAX_ACCESSES == 8u);
  return IVS_TEST_PASS;
}

/* ---- validation (D-03) ---- */

static int api_validate_zeroed_state(void)
{
  ivs_cp1610 s;
  memset(&s, 0, sizeof s);
  IVS_CHECK(s.run_state == IVS_CP1610_RS_UNRESET);
  IVS_CHECK(ivs_cp1610_state_validate(&s) == IVS_OK);
  IVS_CHECK(ivs_cp1610_state_validate(NULL) == IVS_E_BAD_ARG);
  return IVS_TEST_PASS;
}

static int api_validate_rejects_bad_run_state(void)
{
  ivs_cp1610 s;
  unsigned v;
  for (v = 0u; v <= 255u; v++) {
    memset(&s, 0, sizeof s);
    s.run_state = (uint8_t)v;
    if (v <= IVS_CP1610_RS_FAULTED) {
      IVS_CHECK(ivs_cp1610_state_validate(&s) == IVS_OK);
    } else {
      IVS_CHECK(ivs_cp1610_state_validate(&s) == IVS_E_BAD_STATE);
    }
  }
  return IVS_TEST_PASS;
}

static int api_validate_rejects_bad_fault_reason(void)
{
  ivs_cp1610 s;
  unsigned v;
  for (v = 0u; v <= 255u; v++) {
    memset(&s, 0, sizeof s);
    s.fault_reason = (uint8_t)v;
    if (v <= IVS_CP1610_FAULT_LAST) {
      IVS_CHECK(ivs_cp1610_state_validate(&s) == IVS_OK);
    } else {
      IVS_CHECK(ivs_cp1610_state_validate(&s) == IVS_E_BAD_STATE);
    }
  }
  return IVS_TEST_PASS;
}

static int api_validate_rejects_reserved_nonzero(void)
{
  ivs_cp1610 s;
  size_t i;
  for (i = 0u; i < sizeof s.reserved; i++) {
    memset(&s, 0, sizeof s);
    s.reserved[i] = 0x01u;
    IVS_CHECK(ivs_cp1610_state_validate(&s) == IVS_E_BAD_STATE);
    s.reserved[i] = 0x80u;
    IVS_CHECK(ivs_cp1610_state_validate(&s) == IVS_E_BAD_STATE);
  }
  return IVS_TEST_PASS;
}

static int api_validate_rejects_status_low_bits(void)
{
  ivs_cp1610 s;
  unsigned bit;
  for (bit = 0u; bit < 4u; bit++) {
    memset(&s, 0, sizeof s);
    s.status = (uint8_t)(1u << bit);
    IVS_CHECK(ivs_cp1610_state_validate(&s) == IVS_E_BAD_STATE);
  }
  /* The four flag bits S Z OV C (pdf 45 GSWD layout) are valid in any combination. */
  memset(&s, 0, sizeof s);
  s.status = IVS_CP1610_STATUS_MASK;
  IVS_CHECK(ivs_cp1610_state_validate(&s) == IVS_OK);
  return IVS_TEST_PASS;
}

/* ---- fixtures ---- */

/* A run is the reset entry plus one step per program word. */
#define API_PROG_LEN 6u
#define API_STEPS (1u + API_PROG_LEN)
#define API_BASE_A 0x1000u
#define API_BASE_B 0x2000u

static const uint16_t api_prog_a[API_PROG_LEN] = {0x0006u, 0x0007u, 0x0006u, 0x0007u, 0x0007u, 0x0006u};
static const uint16_t api_prog_b[API_PROG_LEN] = {0x0007u, 0x0006u, 0x0006u, 0x0007u, 0x0006u, 0x0007u};

static ivs_testbus api_bus_a;
static ivs_testbus api_bus_b;

typedef struct api_run {
  ivs_status st[API_STEPS];
  ivs_cp1610_result res[API_STEPS];
  ivs_cp1610_access log[API_STEPS][IVS_CP1610_MAX_ACCESSES];
  ivs_cp1610 final;
} api_run;

typedef struct api_calls {
  ivs_cp1610_access c[IVS_TESTBUS_CALL_CAP];
  size_t n;
} api_calls;

static api_calls api_calls_a;
static api_calls api_calls_b;

/* Machine fixture: a flat bus holding prog at base, the reset vector (IAB) answering base,
 * a wait of `wait` cycles answered at base + 1 when nonzero, and a freshly reset state. */
static void api_setup(ivs_testbus *b, ivs_cp1610 *s, uint16_t base, const uint16_t *prog, uint16_t wait)
{
  uint32_t i;
  ivs_testbus_init(b);
  for (i = 0u; i < API_PROG_LEN; i++) {
    ivs_testbus_poke(b, IVS_U16((uint32_t)base + i), prog[i]);
  }
  ivs_testbus_set_iab(b, base);
  if (wait != 0u) {
    (void)ivs_testbus_add_wait(b, IVS_U16((uint32_t)base + 1u), wait);
  }
  (void)ivs_cp1610_reset(s);
}

/* One step with the profile default config, all lines low and the bus port of b. */
static ivs_status api_step(ivs_cp1610 *s, ivs_testbus *b, ivs_cp1610_access *log, size_t cap,
                           ivs_cp1610_result *out)
{
  ivs_cp1610_config cfg;
  ivs_cp1610_lines lines;
  ivs_cp1610_port port;
  ivs_cp1610_config_default(&cfg);
  memset(&lines, 0, sizeof lines);
  ivs_testbus_port(b, &port, 0);
  return ivs_cp1610_step(s, &cfg, &lines, &port, log, cap, out);
}

/* Step k of a run; with_log 0 passes log NULL with the given cap. */
static void api_run_step(api_run *r, size_t k, ivs_cp1610 *s, ivs_testbus *b, int with_log, size_t cap)
{
  r->st[k] = api_step(s, b, with_log ? r->log[k] : NULL, cap, &r->res[k]);
  r->final = *s;
}

static void api_run_all(api_run *r, ivs_cp1610 *s, ivs_testbus *b, int with_log, size_t cap)
{
  size_t k;
  memset(r, 0, sizeof *r);
  for (k = 0u; k < API_STEPS; k++) {
    api_run_step(r, k, s, b, with_log, cap);
  }
}

/* Statuses, results, final state and (when with_log) logs are byte-identical. */
static int api_runs_equal(const api_run *x, const api_run *y, int with_log)
{
  return memcmp(x->st, y->st, sizeof x->st) == 0 && memcmp(x->res, y->res, sizeof x->res) == 0 &&
         memcmp(&x->final, &y->final, sizeof x->final) == 0 &&
         (!with_log || memcmp(x->log, y->log, sizeof x->log) == 0);
}

/* The run did what the program says, so equality checks are not vacuous: every step OK, a
 * reset entry, then one single-access instruction unit per program word in order, ending
 * RUNNING past the program with C as left by the last word (SETC sets, CLRC clears). */
static int api_run_sane(const api_run *r, uint16_t base, const uint16_t *prog)
{
  size_t k;
  for (k = 0u; k < API_STEPS; k++) {
    if (r->st[k] != IVS_OK) {
      return 0;
    }
  }
  if (r->res[0].unit_kind != IVS_CP1610_UNIT_RESET_ENTRY || r->res[0].n_accesses != 1u) {
    return 0;
  }
  for (k = 1u; k < API_STEPS; k++) {
    if (r->res[k].unit_kind != IVS_CP1610_UNIT_INSTR || r->res[k].opcode != prog[k - 1u] ||
        r->res[k].pc_before != IVS_U16((uint32_t)base + (uint32_t)(k - 1u)) || r->res[k].n_accesses != 1u) {
      return 0;
    }
  }
  return r->final.run_state == IVS_CP1610_RS_RUNNING && r->final.r[7] == IVS_U16((uint32_t)base + API_PROG_LEN) &&
         r->final.status == (prog[API_PROG_LEN - 1u] == 0x0007u ? IVS_CP1610_C : 0u);
}

static void api_calls_save(api_calls *d, const ivs_testbus *b)
{
  memcpy(d->c, b->calls, sizeof d->c);
  d->n = b->n_calls;
}

static int api_calls_equal(const api_calls *d, const ivs_testbus *b)
{
  return b->n_dropped == 0u && d->n == b->n_calls && memcmp(d->c, b->calls, d->n * sizeof d->c[0]) == 0;
}

/* ---- refusals (D-06): status code, no state write, no bus call, no log or result write ---- */

static ivs_cp1610_access api_log_fill[IVS_CP1610_MAX_ACCESSES];
static ivs_cp1610_access api_log_seen[IVS_CP1610_MAX_ACCESSES];
static ivs_cp1610_result api_out_fill;
static ivs_cp1610_result api_out_seen;

static void api_fill_outputs(void)
{
  memset(api_log_fill, 0xA5, sizeof api_log_fill);
  memcpy(api_log_seen, api_log_fill, sizeof api_log_seen);
  memset(&api_out_fill, 0xA5, sizeof api_out_fill);
  api_out_seen = api_out_fill;
}

static int api_untouched(const ivs_cp1610 *s, const ivs_cp1610 *before, const ivs_testbus *b)
{
  return memcmp(s, before, sizeof *s) == 0 && b->n_calls == 0u &&
         memcmp(api_log_seen, api_log_fill, sizeof api_log_seen) == 0 &&
         memcmp(&api_out_seen, &api_out_fill, sizeof api_out_seen) == 0;
}

static int api_refuse_null_args(void)
{
  ivs_cp1610 s;
  ivs_cp1610 before;
  ivs_cp1610_config cfg;
  ivs_cp1610_lines lines;
  ivs_cp1610_port port;
  ivs_cp1610_port noport;
  const size_t cap = IVS_CP1610_MAX_ACCESSES;
  unsigned which;
  api_setup(&api_bus_a, &s, API_BASE_A, api_prog_a, 0u);
  ivs_cp1610_config_default(&cfg);
  memset(&lines, 0, sizeof lines);
  ivs_testbus_port(&api_bus_a, &port, 0);
  noport = port;
  noport.bus = NULL;
  before = s;
  for (which = 0u; which < 6u; which++) {
    ivs_status st;
    api_fill_outputs();
    switch (which) {
    case 0u:
      st = ivs_cp1610_step(NULL, &cfg, &lines, &port, api_log_seen, cap, &api_out_seen);
      break;
    case 1u:
      st = ivs_cp1610_step(&s, NULL, &lines, &port, api_log_seen, cap, &api_out_seen);
      break;
    case 2u:
      st = ivs_cp1610_step(&s, &cfg, NULL, &port, api_log_seen, cap, &api_out_seen);
      break;
    case 3u:
      st = ivs_cp1610_step(&s, &cfg, &lines, NULL, api_log_seen, cap, &api_out_seen);
      break;
    case 4u:
      st = ivs_cp1610_step(&s, &cfg, &lines, &noport, api_log_seen, cap, &api_out_seen);
      break;
    default:
      st = ivs_cp1610_step(&s, &cfg, &lines, &port, api_log_seen, cap, NULL);
      break;
    }
    if (st != IVS_E_BAD_ARG || !api_untouched(&s, &before, &api_bus_a)) {
      ivs_test_diag("NULL argument %u: status %d, or state, log, result or bus touched", which, st);
      return IVS_TEST_FAIL;
    }
  }
  IVS_CHECK(ivs_cp1610_reset(NULL) == IVS_E_BAD_ARG);
  ivs_cp1610_config_default(NULL);
  /* Control: the same arguments without a NULL are accepted (the reset entry runs). */
  api_fill_outputs();
  IVS_CHECK(ivs_cp1610_step(&s, &cfg, &lines, &port, api_log_seen, cap, &api_out_seen) == IVS_OK);
  IVS_CHECK(api_out_seen.unit_kind == IVS_CP1610_UNIT_RESET_ENTRY);
  return IVS_TEST_PASS;
}

static int api_refuse_small_log(void)
{
  ivs_cp1610 s;
  ivs_cp1610 before;
  size_t cap;
  api_setup(&api_bus_a, &s, API_BASE_A, api_prog_a, 0u);
  before = s;
  for (cap = 0u; cap < IVS_CP1610_MAX_ACCESSES; cap++) {
    ivs_status st;
    api_fill_outputs();
    st = api_step(&s, &api_bus_a, api_log_seen, cap, &api_out_seen);
    if (st != IVS_E_BAD_ARG || !api_untouched(&s, &before, &api_bus_a)) {
      ivs_test_diag("log_cap %zu: status %d, or state, log, result or bus touched", cap, st);
      return IVS_TEST_FAIL;
    }
  }
  /* Control: log_cap IVS_CP1610_MAX_ACCESSES is accepted. */
  api_fill_outputs();
  IVS_CHECK(api_step(&s, &api_bus_a, api_log_seen, IVS_CP1610_MAX_ACCESSES, &api_out_seen) == IVS_OK);
  IVS_CHECK(api_out_seen.unit_kind == IVS_CP1610_UNIT_RESET_ENTRY);
  return IVS_TEST_PASS;
}

static int api_refuse_invalid_state(void)
{
  ivs_cp1610 s;
  ivs_cp1610 before;
  unsigned which;
  for (which = 0u; which < 8u; which++) {
    ivs_status st;
    api_setup(&api_bus_a, &s, API_BASE_A, api_prog_a, 0u);
    switch (which) {
    case 0u:
      s.run_state = (uint8_t)(IVS_CP1610_RS_FAULTED + 1u);
      break;
    case 1u:
      s.fault_reason = (uint8_t)(IVS_CP1610_FAULT_LAST + 1u);
      break;
    case 2u:
      s.reserved[3] = 1u;
      break;
    case 3u:
      s.status = 0x01u;
      break;
    case 4u:
      s.intff = 2u;
      break;
    case 5u:
      s.dbd = 2u;
      break;
    case 6u:
      s.halt_pending = 0xFFu;
      break;
    default:
      s.busak = 2u;
      break;
    }
    before = s;
    api_fill_outputs();
    st = api_step(&s, &api_bus_a, api_log_seen, IVS_CP1610_MAX_ACCESSES, &api_out_seen);
    if (st != IVS_E_BAD_STATE || !api_untouched(&s, &before, &api_bus_a)) {
      ivs_test_diag("corruption %u: status %d, or state, log, result or bus touched", which, st);
      return IVS_TEST_FAIL;
    }
  }
  return IVS_TEST_PASS;
}

/* ---- access log (D-02, CPU-05) ---- */

/* log == NULL (with any log_cap) gives the same statuses, results, final state and bus calls
 * as a full log. A wait at base + 1 makes the timing nontrivial. */
static int api_null_log_equivalent(void)
{
  static const size_t caps[] = {0u, 1u, IVS_CP1610_MAX_ACCESSES - 1u, IVS_CP1610_MAX_ACCESSES, SIZE_MAX};
  api_run full;
  api_run bare;
  ivs_cp1610 s;
  size_t i;
  api_setup(&api_bus_a, &s, API_BASE_A, api_prog_a, 3u);
  api_run_all(&full, &s, &api_bus_a, 1, IVS_CP1610_MAX_ACCESSES);
  IVS_CHECK(api_run_sane(&full, API_BASE_A, api_prog_a));
  api_calls_save(&api_calls_a, &api_bus_a);
  for (i = 0u; i < sizeof caps / sizeof caps[0]; i++) {
    api_setup(&api_bus_a, &s, API_BASE_A, api_prog_a, 3u);
    api_run_all(&bare, &s, &api_bus_a, 0, caps[i]);
    if (!api_runs_equal(&full, &bare, 0) || !api_calls_equal(&api_calls_a, &api_bus_a)) {
      ivs_test_diag("log NULL with log_cap %zu differs from the full-log run", caps[i]);
      return IVS_TEST_FAIL;
    }
  }
  return IVS_TEST_PASS;
}

/* The port is called once per log record, in log order, with the same kind, flags, address
 * and offset. The port sees the request (wait 0, read data 0); the log carries the
 * transferred value and the applied wait (plan 02-02 decision). Data is compared directly
 * only for WRITE and DW. */
static int api_bus_order_matches_log(void)
{
  const uint16_t wait = 3u;
  const uint16_t wait_addr = IVS_U16(API_BASE_A + 1u);
  api_run r;
  ivs_cp1610 s;
  size_t k;
  size_t i;
  size_t total = 0u;
  const ivs_cp1610_row *row;
  api_setup(&api_bus_a, &s, API_BASE_A, api_prog_a, wait);
  memset(&r, 0, sizeof r);
  for (k = 0u; k < API_STEPS; k++) {
    size_t first = api_bus_a.n_calls;
    api_run_step(&r, k, &s, &api_bus_a, 1, IVS_CP1610_MAX_ACCESSES);
    IVS_CHECK(r.st[k] == IVS_OK);
    IVS_CHECK(api_bus_a.n_calls - first == (size_t)r.res[k].n_accesses);
    for (i = 0u; i < r.res[k].n_accesses; i++) {
      const ivs_cp1610_access *p = &api_bus_a.calls[first + i];
      const ivs_cp1610_access *l = &r.log[k][i];
      IVS_CHECK(p->kind == l->kind && p->flags == l->flags);
      IVS_CHECK(p->addr == l->addr && p->offset_cpu_cycles == l->offset_cpu_cycles);
      IVS_CHECK(p->wait_cpu_cycles == 0u);
      if (l->kind == IVS_ACC_WRITE || l->kind == IVS_ACC_DW) {
        IVS_CHECK(p->data == l->data);
      } else {
        IVS_CHECK(p->data == 0u);
      }
      if (l->kind == IVS_ACC_FETCH) {
        IVS_CHECK(l->data == api_bus_a.mem[l->addr]);
        IVS_CHECK(l->wait_cpu_cycles == (l->addr == wait_addr ? wait : 0u));
      } else if (l->kind == IVS_ACC_IAB) {
        IVS_CHECK(l->data == API_BASE_A && l->wait_cpu_cycles == 0u);
      }
    }
    total += r.res[k].n_accesses;
  }
  IVS_CHECK(api_run_sane(&r, API_BASE_A, api_prog_a));
  IVS_CHECK(total == API_STEPS && api_bus_a.n_calls == total);
  /* Reset entry: IAB at offset 1 (pdf 27). Instructions: FETCH at offset 0 (pdf 28). */
  IVS_CHECK(r.log[0][0].kind == IVS_ACC_IAB && r.log[0][0].offset_cpu_cycles == 1u);
  IVS_CHECK(r.log[1][0].kind == IVS_ACC_FETCH && r.log[1][0].offset_cpu_cycles == 0u);
  /* The waited fetch lengthens its unit by the wait. */
  row = ivs_cp1610_decode(api_prog_a[1]);
  IVS_CHECK(row != NULL);
  IVS_CHECK(r.res[2].elapsed_cpu_cycles == (uint32_t)row->base_cycles + wait);
  return IVS_TEST_PASS;
}

/* ---- units (D-12, CPU-04) ---- */

/* A never-reset (zeroed) state reports NOT_RESET: 0 cycles, no access, no event, no state
 * change, however often it is stepped (POL-RST-01). */
static int api_not_reset_unit(void)
{
  ivs_cp1610 s;
  ivs_cp1610 zero;
  unsigned k;
  ivs_testbus_init(&api_bus_a);
  memset(&s, 0, sizeof s);
  memset(&zero, 0, sizeof zero);
  for (k = 0u; k < 3u; k++) {
    api_fill_outputs();
    IVS_CHECK(api_step(&s, &api_bus_a, api_log_seen, IVS_CP1610_MAX_ACCESSES, &api_out_seen) == IVS_OK);
    IVS_CHECK(api_out_seen.unit_kind == IVS_CP1610_UNIT_NOT_RESET);
    IVS_CHECK(api_out_seen.elapsed_cpu_cycles == 0u && api_out_seen.n_accesses == 0u);
    IVS_CHECK(api_out_seen.events == 0u && api_out_seen.pc_before == 0u);
    IVS_CHECK(memcmp(&s, &zero, sizeof s) == 0);
    IVS_CHECK(api_bus_a.n_calls == 0u);
    IVS_CHECK(memcmp(api_log_seen, api_log_fill, sizeof api_log_seen) == 0);
  }
  return IVS_TEST_PASS;
}

/* ---- no-progress guard, sticky faults, HALTED and waits (D-12, plan 02-15) ---- */

/* One step with the profile default config, the given lines and the bus port of b. */
static ivs_status api_step_lines(ivs_cp1610 *s, ivs_testbus *b, const ivs_cp1610_lines *lines, ivs_cp1610_access *log,
                                 ivs_cp1610_result *out)
{
  ivs_cp1610_config cfg;
  ivs_cp1610_port port;
  ivs_cp1610_config_default(&cfg);
  ivs_testbus_port(b, &port, 0);
  return ivs_cp1610_step(s, &cfg, lines, &port, log, IVS_CP1610_MAX_ACCESSES, out);
}

/* Lines from a 4-bit pattern: bit 0 INTR, 1 INTRM, 2 BUSRQ, 3 STPST. */
static ivs_cp1610_lines api_lines(unsigned bits)
{
  ivs_cp1610_lines l;
  l.intr = (uint8_t)(bits & 1u);
  l.intrm = (uint8_t)((bits >> 1) & 1u);
  l.busrq = (uint8_t)((bits >> 2) & 1u);
  l.stpst = (uint8_t)((bits >> 3) & 1u);
  return l;
}

/* A bus holding the refused decle $264 (MVO@ with SSS = MMM = 4, POL-MVO-01) at API_BASE_A, the
 * reset vector answering API_BASE_A, and a reset state stepped through the reset entry and the
 * refused fetch. The faulting step itself counts its fetch: a FAULT unit with elapsed above 0. */
static int api_make_faulted(ivs_cp1610 *s)
{
  ivs_cp1610_lines low = api_lines(0u);
  ivs_cp1610_access log[IVS_CP1610_MAX_ACCESSES];
  ivs_cp1610_result r;
  ivs_testbus_init(&api_bus_a);
  ivs_testbus_poke(&api_bus_a, API_BASE_A, 0x0264u);
  ivs_testbus_set_iab(&api_bus_a, API_BASE_A);
  IVS_CHECK(ivs_cp1610_reset(s) == IVS_OK);
  IVS_CHECK(api_step_lines(s, &api_bus_a, &low, log, &r) == IVS_OK);
  IVS_CHECK(r.unit_kind == IVS_CP1610_UNIT_RESET_ENTRY);
  IVS_CHECK(api_step_lines(s, &api_bus_a, &low, log, &r) == IVS_OK);
  IVS_CHECK(r.unit_kind == IVS_CP1610_UNIT_FAULT && r.elapsed_cpu_cycles > 0u && r.n_accesses == 1u);
  IVS_CHECK((r.events & IVS_CP1610_EV_FAULT) != 0u);
  IVS_CHECK(s->run_state == IVS_CP1610_RS_FAULTED);
  IVS_CHECK(s->fault_reason == IVS_CP1610_FAULT_UNSUPPORTED_MVO_SSS_EQ_MMM);
  return IVS_TEST_PASS;
}

/* Steps that start in UNRESET or FAULTED take 0 cycles, make no access and no bus call, leave
 * the state as it was and repeat byte-identically under any lines (T-02-32). The step that
 * faults is not one of them: it reports the cycles it spent (api_make_faulted). */
static int api_no_progress_units(void)
{
  static const uint8_t starts[2] = {IVS_CP1610_RS_UNRESET, IVS_CP1610_RS_FAULTED};
  ivs_cp1610 s;
  ivs_cp1610 before;
  ivs_cp1610_result first;
  ivs_cp1610_result r;
  ivs_cp1610_access log[IVS_CP1610_MAX_ACCESSES];
  unsigned which;
  unsigned bits;
  memset(&first, 0, sizeof first);
  for (which = 0u; which < 2u; which++) {
    if (starts[which] == IVS_CP1610_RS_UNRESET) {
      ivs_testbus_init(&api_bus_a);
      memset(&s, 0, sizeof s);
    } else {
      IVS_CHECK(api_make_faulted(&s) == IVS_TEST_PASS);
    }
    before = s;
    for (bits = 0u; bits < 16u; bits++) {
      ivs_cp1610_lines l = api_lines(bits);
      size_t calls = api_bus_a.n_calls;
      IVS_CHECK(api_step_lines(&s, &api_bus_a, &l, log, &r) == IVS_OK);
      IVS_CHECK(r.elapsed_cpu_cycles == 0u && r.n_accesses == 0u && r.events == 0u);
      IVS_CHECK(r.unit_kind == (starts[which] == IVS_CP1610_RS_UNRESET ? IVS_CP1610_UNIT_NOT_RESET : IVS_CP1610_UNIT_FAULT));
      IVS_CHECK(api_bus_a.n_calls == calls);
      IVS_CHECK(memcmp(&s, &before, sizeof s) == 0);
      if (bits == 0u) {
        first = r;
      }
      IVS_CHECK(memcmp(&r, &first, sizeof r) == 0);
    }
  }
  return IVS_TEST_PASS;
}

/* A fault stays until reset: no line combination moves a FAULTED state; reset clears the
 * fault reason and the next step is the reset entry (D-12, POL-RST-01). */
static int api_fault_sticky_until_reset(void)
{
  ivs_cp1610 s;
  ivs_cp1610 before;
  ivs_cp1610_result r;
  ivs_cp1610_access log[IVS_CP1610_MAX_ACCESSES];
  ivs_cp1610_lines low = api_lines(0u);
  unsigned k;
  IVS_CHECK(api_make_faulted(&s) == IVS_TEST_PASS);
  before = s;
  for (k = 0u; k < 64u; k++) {
    ivs_cp1610_lines l = api_lines(k & 15u);
    IVS_CHECK(api_step_lines(&s, &api_bus_a, &l, log, &r) == IVS_OK);
    IVS_CHECK(r.unit_kind == IVS_CP1610_UNIT_FAULT && r.elapsed_cpu_cycles == 0u);
    IVS_CHECK(memcmp(&s, &before, sizeof s) == 0);
  }
  IVS_CHECK(ivs_cp1610_reset(&s) == IVS_OK);
  IVS_CHECK(s.run_state == IVS_CP1610_RS_RESET_PENDING && s.fault_reason == IVS_CP1610_FAULT_NONE);
  IVS_CHECK(api_step_lines(&s, &api_bus_a, &low, log, &r) == IVS_OK);
  IVS_CHECK(r.unit_kind == IVS_CP1610_UNIT_RESET_ENTRY && r.elapsed_cpu_cycles > 0u);
  IVS_CHECK(s.run_state == IVS_CP1610_RS_RUNNING && s.r[7] == API_BASE_A);
  return IVS_TEST_PASS;
}

/* Every step in HALTED advances time: idle units of 1 cycle whatever INTR and INTRM do, grant
 * and hold units of 1 cycle, a release of the configured latency, and the CPU stays HALTED
 * until a STPST edge (POL-HLT-02, C45; the D-12 no-progress guard). */
static int api_halted_step_progresses(void)
{
  static const uint16_t prog[2] = {0x0034u, 0x0034u};
  ivs_cp1610 s;
  ivs_cp1610_config cfg;
  ivs_cp1610_result r;
  ivs_cp1610_access log[IVS_CP1610_MAX_ACCESSES];
  ivs_cp1610_lines l;
  uint32_t total = 0u;
  unsigned k;
  ivs_cp1610_config_default(&cfg);
  ivs_testbus_init(&api_bus_a);
  ivs_testbus_poke(&api_bus_a, API_BASE_A, prog[0]);
  ivs_testbus_poke(&api_bus_a, IVS_U16(API_BASE_A + 1u), prog[1]);
  ivs_testbus_set_iab(&api_bus_a, API_BASE_A);
  IVS_CHECK(ivs_cp1610_reset(&s) == IVS_OK);
  l = api_lines(0u);
  IVS_CHECK(api_step_lines(&s, &api_bus_a, &l, log, &r) == IVS_OK); /* reset entry */
  IVS_CHECK(api_step_lines(&s, &api_bus_a, &l, log, &r) == IVS_OK); /* NOP: interruptible */
  l = api_lines(8u);
  IVS_CHECK(api_step_lines(&s, &api_bus_a, &l, log, &r) == IVS_OK); /* STPST edge: stop */
  IVS_CHECK(r.unit_kind == IVS_CP1610_UNIT_HALTED && r.elapsed_cpu_cycles == 1u);
  IVS_CHECK((r.events & IVS_CP1610_EV_STOPPED) != 0u && s.run_state == IVS_CP1610_RS_HALTED);
  for (k = 0u; k < 100u; k++) {
    l = api_lines(k & 3u); /* INTR and INTRM in every combination, never accepted */
    IVS_CHECK(api_step_lines(&s, &api_bus_a, &l, log, &r) == IVS_OK);
    IVS_CHECK(r.unit_kind == IVS_CP1610_UNIT_HALTED && r.elapsed_cpu_cycles == 1u && r.n_accesses == 0u);
    IVS_CHECK(s.run_state == IVS_CP1610_RS_HALTED);
    total += r.elapsed_cpu_cycles;
  }
  IVS_CHECK(total == 100u);
  l = api_lines(4u);
  for (k = 0u; k < 3u; k++) {
    IVS_CHECK(api_step_lines(&s, &api_bus_a, &l, log, &r) == IVS_OK);
    IVS_CHECK(r.unit_kind == IVS_CP1610_UNIT_BUS_GRANT && r.elapsed_cpu_cycles == 1u && r.n_accesses == 0u);
    IVS_CHECK(s.busak == 1u && s.run_state == IVS_CP1610_RS_HALTED);
  }
  l = api_lines(0u);
  IVS_CHECK(api_step_lines(&s, &api_bus_a, &l, log, &r) == IVS_OK);
  IVS_CHECK(r.unit_kind == IVS_CP1610_UNIT_BUS_GRANT && r.elapsed_cpu_cycles == cfg.release_latency_cpu_cycles);
  IVS_CHECK((r.events & IVS_CP1610_EV_BUS_RELEASED) != 0u && s.busak == 0u && s.run_state == IVS_CP1610_RS_HALTED);
  l = api_lines(8u);
  IVS_CHECK(api_step_lines(&s, &api_bus_a, &l, log, &r) == IVS_OK); /* STPST edge: resume, fetch */
  IVS_CHECK(r.unit_kind == IVS_CP1610_UNIT_INSTR && s.run_state == IVS_CP1610_RS_RUNNING);
  IVS_CHECK(s.r[7] == IVS_U16(API_BASE_A + 2u));
  return IVS_TEST_PASS;
}

/* Each wait taken by a FETCH, READ or ADAR shifts every later access of the unit and the unit
 * length (POL-WAIT-01, pdf 37, 40, 43). MVI $0200, R1 with waits 1 on the fetch, 2 on the
 * address word and 3 on the operand: nominal offsets 0, 4, 6 (pdf 30) become 0, 5, 9, and the
 * printed 10 cycles become 16. The port sees the same shifted offsets. */
static int api_wait_shifts_offsets(void)
{
  ivs_cp1610 s;
  ivs_cp1610_result r;
  ivs_cp1610_access log[IVS_CP1610_MAX_ACCESSES];
  ivs_cp1610_lines low = api_lines(0u);
  size_t first;
  size_t i;
  ivs_testbus_init(&api_bus_a);
  ivs_testbus_poke(&api_bus_a, API_BASE_A, 0x0281u);
  ivs_testbus_poke(&api_bus_a, IVS_U16(API_BASE_A + 1u), 0x0200u);
  ivs_testbus_poke(&api_bus_a, 0x0200u, 0x5A3Cu);
  ivs_testbus_set_iab(&api_bus_a, API_BASE_A);
  IVS_CHECK(ivs_testbus_add_wait(&api_bus_a, API_BASE_A, 1u) == 0);
  IVS_CHECK(ivs_testbus_add_wait(&api_bus_a, IVS_U16(API_BASE_A + 1u), 2u) == 0);
  IVS_CHECK(ivs_testbus_add_wait(&api_bus_a, 0x0200u, 3u) == 0);
  IVS_CHECK(ivs_cp1610_reset(&s) == IVS_OK);
  IVS_CHECK(api_step_lines(&s, &api_bus_a, &low, log, &r) == IVS_OK);
  IVS_CHECK(r.unit_kind == IVS_CP1610_UNIT_RESET_ENTRY);
  first = api_bus_a.n_calls;
  IVS_CHECK(api_step_lines(&s, &api_bus_a, &low, log, &r) == IVS_OK);
  IVS_CHECK(r.unit_kind == IVS_CP1610_UNIT_INSTR && r.n_accesses == 3u);
  IVS_CHECK(log[0].kind == IVS_ACC_FETCH && log[0].offset_cpu_cycles == 0u && log[0].wait_cpu_cycles == 1u);
  IVS_CHECK(log[1].kind == IVS_ACC_READ && log[1].offset_cpu_cycles == 5u && log[1].wait_cpu_cycles == 2u);
  IVS_CHECK(log[2].kind == IVS_ACC_ADAR && log[2].offset_cpu_cycles == 9u && log[2].wait_cpu_cycles == 3u);
  IVS_CHECK(log[2].addr == 0x0200u && log[2].data == 0x5A3Cu);
  IVS_CHECK(r.elapsed_cpu_cycles == 16u);
  IVS_CHECK(s.r[1] == 0x5A3Cu && s.r[7] == IVS_U16(API_BASE_A + 2u));
  for (i = 0u; i < 3u; i++) {
    IVS_CHECK(api_bus_a.calls[first + i].offset_cpu_cycles == log[i].offset_cpu_cycles);
  }
  return IVS_TEST_PASS;
}

/* reset twice equals reset once; reset ignores prior contents (zeroed, byte-filled, or a
 * state that has run), since POL-RST-01 zero-fills every field. */
static int api_reset_idempotent(void)
{
  ivs_cp1610 once;
  ivs_cp1610 twice;
  ivs_cp1610 filled;
  ivs_cp1610 ran;
  api_run r;
  memset(&once, 0, sizeof once);
  IVS_CHECK(ivs_cp1610_reset(&once) == IVS_OK);
  IVS_CHECK(once.run_state == IVS_CP1610_RS_RESET_PENDING);
  IVS_CHECK(ivs_cp1610_state_validate(&once) == IVS_OK);
  twice = once;
  IVS_CHECK(ivs_cp1610_reset(&twice) == IVS_OK);
  IVS_CHECK(memcmp(&once, &twice, sizeof once) == 0);
  memset(&filled, 0xA5, sizeof filled);
  IVS_CHECK(ivs_cp1610_reset(&filled) == IVS_OK);
  IVS_CHECK(memcmp(&once, &filled, sizeof once) == 0);
  api_setup(&api_bus_a, &ran, API_BASE_A, api_prog_a, 0u);
  api_run_all(&r, &ran, &api_bus_a, 1, IVS_CP1610_MAX_ACCESSES);
  IVS_CHECK(api_run_sane(&r, API_BASE_A, api_prog_a));
  IVS_CHECK(ivs_cp1610_reset(&ran) == IVS_OK);
  IVS_CHECK(memcmp(&once, &ran, sizeof once) == 0);
  return IVS_TEST_PASS;
}

/* ---- determinism and isolation (D-05, CPU-01) ---- */

/* The same initial state on two identical bus images, stepped one after the other: identical
 * statuses, results, logs, final states, bus calls and memory images. */
static int api_deterministic_repeat(void)
{
  api_run r1;
  api_run r2;
  ivs_cp1610 s1;
  ivs_cp1610 s2;
  api_setup(&api_bus_a, &s1, API_BASE_A, api_prog_a, 2u);
  api_setup(&api_bus_b, &s2, API_BASE_A, api_prog_a, 2u);
  IVS_CHECK(memcmp(&s1, &s2, sizeof s1) == 0);
  api_run_all(&r1, &s1, &api_bus_a, 1, IVS_CP1610_MAX_ACCESSES);
  api_run_all(&r2, &s2, &api_bus_b, 1, IVS_CP1610_MAX_ACCESSES);
  IVS_CHECK(api_run_sane(&r1, API_BASE_A, api_prog_a));
  IVS_CHECK(api_runs_equal(&r1, &r2, 1));
  api_calls_save(&api_calls_a, &api_bus_a);
  IVS_CHECK(api_calls_equal(&api_calls_a, &api_bus_b));
  IVS_CHECK(memcmp(api_bus_a.mem, api_bus_b.mem, sizeof api_bus_a.mem) == 0);
  IVS_CHECK(memcmp(api_bus_a.valid, api_bus_b.valid, sizeof api_bus_a.valid) == 0);
  IVS_CHECK(api_bus_a.uninit_reads == 0u && api_bus_b.uninit_reads == 0u);
  return IVS_TEST_PASS;
}

/* Two instances with different programs, bases and waits, stepped interleaved (A, B, A, B,
 * ...), give the same results, logs, final states and bus calls as each stepped alone. */
static int api_two_instances_interleaved(void)
{
  api_run a_alone;
  api_run b_alone;
  api_run a_mixed;
  api_run b_mixed;
  ivs_cp1610 sa;
  ivs_cp1610 sb;
  size_t k;
  api_setup(&api_bus_a, &sa, API_BASE_A, api_prog_a, 2u);
  api_run_all(&a_alone, &sa, &api_bus_a, 1, IVS_CP1610_MAX_ACCESSES);
  api_calls_save(&api_calls_a, &api_bus_a);
  api_setup(&api_bus_b, &sb, API_BASE_B, api_prog_b, 5u);
  api_run_all(&b_alone, &sb, &api_bus_b, 1, IVS_CP1610_MAX_ACCESSES);
  api_calls_save(&api_calls_b, &api_bus_b);
  IVS_CHECK(api_run_sane(&a_alone, API_BASE_A, api_prog_a));
  IVS_CHECK(api_run_sane(&b_alone, API_BASE_B, api_prog_b));
  IVS_CHECK(!api_runs_equal(&a_alone, &b_alone, 0));

  api_setup(&api_bus_a, &sa, API_BASE_A, api_prog_a, 2u);
  api_setup(&api_bus_b, &sb, API_BASE_B, api_prog_b, 5u);
  memset(&a_mixed, 0, sizeof a_mixed);
  memset(&b_mixed, 0, sizeof b_mixed);
  for (k = 0u; k < API_STEPS; k++) {
    api_run_step(&a_mixed, k, &sa, &api_bus_a, 1, IVS_CP1610_MAX_ACCESSES);
    api_run_step(&b_mixed, k, &sb, &api_bus_b, 1, IVS_CP1610_MAX_ACCESSES);
  }
  IVS_CHECK(api_runs_equal(&a_alone, &a_mixed, 1));
  IVS_CHECK(api_runs_equal(&b_alone, &b_mixed, 1));
  IVS_CHECK(api_calls_equal(&api_calls_a, &api_bus_a));
  IVS_CHECK(api_calls_equal(&api_calls_b, &api_bus_b));
  return IVS_TEST_PASS;
}

static const ivs_test_case api_cases[] = {
    {"cpu-api.access-size-12", api_access_size_12, NULL},
    {"cpu-api.bus-order-matches-log", api_bus_order_matches_log, NULL},
    {"cpu-api.deterministic-repeat", api_deterministic_repeat, NULL},
    {"cpu-api.fault-sticky-until-reset", api_fault_sticky_until_reset, NULL},
    {"cpu-api.halted-step-progresses", api_halted_step_progresses, NULL},
    {"cpu-api.no-progress-units", api_no_progress_units, NULL},
    {"cpu-api.not-reset-unit", api_not_reset_unit, NULL},
    {"cpu-api.null-log-equivalent", api_null_log_equivalent, NULL},
    {"cpu-api.refuse-invalid-state", api_refuse_invalid_state, NULL},
    {"cpu-api.refuse-null-args", api_refuse_null_args, NULL},
    {"cpu-api.refuse-small-log", api_refuse_small_log, NULL},
    {"cpu-api.reset-idempotent", api_reset_idempotent, NULL},
    {"cpu-api.two-instances-interleaved", api_two_instances_interleaved, NULL},
    {"cpu-api.state-size-32", api_state_size_32, NULL},
    {"cpu-api.validate-rejects-bad-fault-reason", api_validate_rejects_bad_fault_reason, NULL},
    {"cpu-api.validate-rejects-bad-run-state", api_validate_rejects_bad_run_state, NULL},
    {"cpu-api.validate-rejects-reserved-nonzero", api_validate_rejects_reserved_nonzero, NULL},
    {"cpu-api.validate-rejects-status-low-bits", api_validate_rejects_status_low_bits, NULL},
    {"cpu-api.validate-zeroed-state", api_validate_zeroed_state, NULL},
    {"cpu-api.wait-shifts-offsets", api_wait_shifts_offsets, NULL},
};

int main(int argc, char **argv)
{
  ivs_test_suite suite;
  ivs_test_suite_from_cases(&suite, "cpu-api", api_cases, sizeof api_cases / sizeof api_cases[0]);
  return ivs_test_main(argc, argv, &suite);
}

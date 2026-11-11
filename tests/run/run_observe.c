/* SPDX-License-Identifier: MIT */
/* Suite run, family observe (D-08, A-01, plan 04-06): read-only guest observation through the
 * public header ivs/ivs_run.h.
 *   run.cpu-state.*   the CPU snapshot OFF, after a power-on reset and after stepped units (the
 *                     expected registers are literals worked out from the stand-in program A of
 *                     tests/machine/machine_standin.c, never read back from the code under test),
 *                     the flag mapping and the out-struct size rule.
 *   run.refuse.*      every refusal class with the SP4 oracle: the status, the whole instance
 *                     storage memcmp-equal to a copy taken just before the call, the composite and
 *                     seven domain digests and info's time_ticks equal (valid handle), and an out
 *                     parameter poisoned with 0xA5 that keeps every byte.
 * Instances live in exact-size heap storage (ivs_ms_new); an out parameter aimed into the
 * instance gets its struct_size written into the cartridge backing words above the stand-in
 * segment before the snapshot, so the refused call alone is judged. */
#include <stddef.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>

#include "cpu/cp1610.h"
#include "ivs/ivs.h"
#include "ivs_test.h"
#include "machine/machine.h"
#include "run_cases.h"

/* Fails the case with a diagnostic naming the line and the failed expression. */
#define IVS_CHECK(cond)                                                                                \
  do {                                                                                                 \
    if (!(cond)) {                                                                                     \
      ivs_test_diag("check failed at line %d: %s", __LINE__, #cond);                                   \
      return IVS_TEST_FAIL;                                                                            \
    }                                                                                                  \
  } while (0)

/* Like IVS_CHECK, inside a case that must release its resources (rc, label done). */
#define RO_CHECK(cond)                                                                                 \
  do {                                                                                                 \
    if (!(cond)) {                                                                                     \
      ivs_test_diag("check failed at line %d: %s", __LINE__, #cond);                                   \
      goto done;                                                                                       \
    }                                                                                                  \
  } while (0)

#define RO_POISON 0xA5u
#define RO_REPORT_BYTES ((uint32_t)sizeof(ivs_cpu_state_report))

static const uint32_t ro_domains[IVS_RUN_SNAP_DIGESTS] = {
    IVS_DIGEST_COMPOSITE, IVS_DIGEST_CPU,   IVS_DIGEST_MEM,   IVS_DIGEST_IMAGES,
    IVS_DIGEST_INSTANCE,  IVS_DIGEST_VIDEO, IVS_DIGEST_AUDIO, IVS_DIGEST_INPUT};

/* ---- the refusal and side-effect oracle (run_cases.h) ---- */

int ivs_run_snap_take(ivs_run_snap *s, const ivs_ms_inst *x, int live)
{
  uint32_t i;
  memset(s, 0, sizeof *s);
  s->copy = malloc(IVS_MACHINE_BYTES);
  if (s->copy == NULL) {
    ivs_test_diag("snapshot: no memory");
    return 0;
  }
  memcpy(s->copy, x->storage, IVS_MACHINE_BYTES);
  s->live = live;
  if (live) {
    ivs_info info;
    for (i = 0u; i < IVS_RUN_SNAP_DIGESTS; i++) {
      if (ivs_machine_state_digest(x->m, ro_domains[i], s->digest[i]) != IVS_OK) {
        ivs_test_diag("snapshot: digest of domain %u refused", (unsigned)ro_domains[i]);
        return 0;
      }
    }
    memset(&info, 0, sizeof info);
    info.struct_size = (uint32_t)sizeof info;
    if (ivs_machine_info(x->m, &info) != IVS_OK) {
      ivs_test_diag("snapshot: info refused");
      return 0;
    }
    s->time_ticks = info.time_ticks;
  }
  return 1;
}

int ivs_run_snap_same(const ivs_run_snap *s, const ivs_ms_inst *x)
{
  uint32_t i;
  if (memcmp(s->copy, x->storage, IVS_MACHINE_BYTES) != 0) {
    for (i = 0u; i < IVS_MACHINE_BYTES; i++) {
      if (s->copy[i] != x->storage[i]) {
        break;
      }
    }
    ivs_test_diag("instance storage changed (first byte %u)", (unsigned)i);
    return 0;
  }
  if (s->live) {
    ivs_info info;
    uint8_t d[32];
    for (i = 0u; i < IVS_RUN_SNAP_DIGESTS; i++) {
      if (ivs_machine_state_digest(x->m, ro_domains[i], d) != IVS_OK || memcmp(d, s->digest[i], 32u) != 0) {
        ivs_test_diag("digest of domain %u changed", (unsigned)ro_domains[i]);
        return 0;
      }
    }
    memset(&info, 0, sizeof info);
    info.struct_size = (uint32_t)sizeof info;
    if (ivs_machine_info(x->m, &info) != IVS_OK || info.time_ticks != s->time_ticks) {
      ivs_test_diag("machine time changed");
      return 0;
    }
  }
  return 1;
}

void ivs_run_snap_free(ivs_run_snap *s)
{
  free(s->copy);
  s->copy = NULL;
}

/* ---- helpers ---- */

/* The internal instance behind x (test seam: tests see core/src). */
static struct ivs_machine *ro_inst(const ivs_ms_inst *x)
{
  return (struct ivs_machine *)(void *)x->storage;
}

/* An offset inside the instance, 16-aligned, in cartridge backing words that no stand-in
 * segment uses (content A writes $5000..$500F, B $6000..$6017). */
static size_t ro_inside_off(void)
{
  size_t off = offsetof(struct ivs_machine, mem) + offsetof(ivs_memsys, cart) + 2u * 0x6000u;
  return (off + 15u) & ~(size_t)15u;
}

/* A heap report of n bytes poisoned with RO_POISON, struct_size set to size. */
static ivs_cpu_state_report *ro_report(size_t n, uint32_t size)
{
  ivs_cpu_state_report *r = malloc(n);
  if (r != NULL) {
    memset(r, RO_POISON, n);
    r->struct_size = size;
  }
  return r;
}

/* 1 when every byte of p equals the RO_POISON byte except a leading struct_size of value size. */
static int ro_report_kept(const ivs_cpu_state_report *r, size_t n, uint32_t size)
{
  const uint8_t *p = (const uint8_t *)(const void *)r;
  size_t i;
  if (r->struct_size != size) {
    ivs_test_diag("struct_size changed");
    return 0;
  }
  for (i = sizeof r->struct_size; i < n; i++) {
    if (p[i] != RO_POISON) {
      ivs_test_diag("out byte %u written by a refused call", (unsigned)i);
      return 0;
    }
  }
  return 1;
}

/* Expected registers of a report: all of r[] equal want_r, flags all zero, reserved zero. */
static int ro_regs(const ivs_cpu_state_report *r, uint32_t run_state, const uint16_t want_r[8])
{
  uint32_t i;
  if (r->struct_size != RO_REPORT_BYTES || r->run_state != run_state || r->fault_reason != IVS_CPU_FAULT_NONE) {
    ivs_test_diag("size %u run_state %u fault %u, want %u %u 0", (unsigned)r->struct_size, (unsigned)r->run_state,
                  (unsigned)r->fault_reason, (unsigned)RO_REPORT_BYTES, (unsigned)run_state);
    return 0;
  }
  for (i = 0u; i < 8u; i++) {
    if (r->r[i] != want_r[i]) {
      ivs_test_diag("R%u = $%04X, want $%04X", (unsigned)i, (unsigned)r->r[i], (unsigned)want_r[i]);
      return 0;
    }
  }
  if (r->reserved[0] != 0u || r->reserved[1] != 0u || r->reserved[2] != 0u) {
    ivs_test_diag("reserved bytes not zero");
    return 0;
  }
  return 1;
}

/* ---- cpu_state (D-08) ---- */

/* OFF: a fresh instance and one with content loaded report UNRESET with every register zero. */
static int ro_cpu_off(void)
{
  static const uint16_t zero[8] = {0u, 0u, 0u, 0u, 0u, 0u, 0u, 0u};
  ivs_ms_inst x;
  ivs_cpu_state_report r;
  int rc = IVS_TEST_FAIL;
  IVS_CHECK(ivs_ms_new(&x, RO_POISON));
  memset(&r, RO_POISON, sizeof r);
  r.struct_size = RO_REPORT_BYTES;
  RO_CHECK(ivs_machine_cpu_state(x.m, &r) == IVS_OK);
  RO_CHECK(ro_regs(&r, IVS_CPU_STATE_UNRESET, zero));
  RO_CHECK(r.s == 0u && r.z == 0u && r.ov == 0u && r.c == 0u && r.intr_enabled == 0u);
  RO_CHECK(ivs_ms_content(x.m, IVS_MS_CONTENT_A));
  memset(&r, RO_POISON, sizeof r);
  r.struct_size = RO_REPORT_BYTES;
  RO_CHECK(ivs_machine_cpu_state(x.m, &r) == IVS_OK);
  RO_CHECK(ro_regs(&r, IVS_CPU_STATE_UNRESET, zero));
  rc = IVS_TEST_PASS;
done:
  ivs_ms_free(&x);
  return rc;
}

/* After a power-on reset the CPU is armed for its reset entry: RESET_PENDING, registers zero,
 * and the report equals the CPU's own fields. */
static int ro_cpu_after_reset(void)
{
  static const uint16_t zero[8] = {0u, 0u, 0u, 0u, 0u, 0u, 0u, 0u};
  ivs_ms_inst x;
  ivs_cpu_state_report r;
  const struct ivs_machine *im;
  uint32_t i;
  int rc = IVS_TEST_FAIL;
  IVS_CHECK(ivs_ms_powered(&x, IVS_MS_CONTENT_A, RO_POISON));
  im = ro_inst(&x);
  memset(&r, RO_POISON, sizeof r);
  r.struct_size = RO_REPORT_BYTES;
  RO_CHECK(ivs_machine_cpu_state(x.m, &r) == IVS_OK);
  RO_CHECK(ro_regs(&r, IVS_CPU_STATE_RESET_PENDING, zero));
  RO_CHECK(r.s == 0u && r.z == 0u && r.ov == 0u && r.c == 0u && r.intr_enabled == 0u);
  RO_CHECK(r.run_state == im->cpu.run_state);
  for (i = 0u; i < 8u; i++) {
    RO_CHECK(r.r[i] == im->cpu.r[i]);
  }
  rc = IVS_TEST_PASS;
done:
  ivs_ms_free(&x);
  return rc;
}

/* Program A (machine_standin.c): reset entry -> R7 $1000; MVII #$155, R1 -> R1 $155, R7 $1002;
 * MVO R1, $0200 -> R7 $1004; INCR R1 -> R1 $156, R7 $1005 (S and Z clear). */
static int ro_cpu_after_steps(void)
{
  static const uint16_t entry[8] = {0u, 0u, 0u, 0u, 0u, 0u, 0u, 0x1000u};
  static const uint16_t mvii[8] = {0u, 0x0155u, 0u, 0u, 0u, 0u, 0u, 0x1002u};
  static const uint16_t incr[8] = {0u, 0x0156u, 0u, 0u, 0u, 0u, 0u, 0x1005u};
  ivs_ms_inst x;
  ivs_cpu_state_report r;
  int rc = IVS_TEST_FAIL;
  IVS_CHECK(ivs_ms_powered(&x, IVS_MS_CONTENT_A, RO_POISON));
  r.struct_size = RO_REPORT_BYTES;
  RO_CHECK(ivs_ms_step(x.m, 1u, NULL));
  RO_CHECK(ivs_machine_cpu_state(x.m, &r) == IVS_OK);
  RO_CHECK(ro_regs(&r, IVS_CPU_STATE_RUNNING, entry));
  RO_CHECK(ivs_ms_step(x.m, 1u, NULL));
  RO_CHECK(ivs_machine_cpu_state(x.m, &r) == IVS_OK);
  RO_CHECK(ro_regs(&r, IVS_CPU_STATE_RUNNING, mvii));
  RO_CHECK(ivs_ms_step(x.m, 2u, NULL));
  RO_CHECK(ivs_machine_cpu_state(x.m, &r) == IVS_OK);
  RO_CHECK(ro_regs(&r, IVS_CPU_STATE_RUNNING, incr));
  RO_CHECK(r.s == 0u && r.z == 0u);
  rc = IVS_TEST_PASS;
done:
  ivs_ms_free(&x);
  return rc;
}

/* Flag mapping: the status byte's S ($80), Z ($40), OV ($20) and C ($10) bits and the interrupt
 * enable reach their own report fields as 0 or 1 (set through the internal seam). */
static int ro_cpu_flag_bits(void)
{
  ivs_ms_inst x;
  ivs_cpu_state_report r;
  struct ivs_machine *im;
  int rc = IVS_TEST_FAIL;
  IVS_CHECK(ivs_ms_powered(&x, IVS_MS_CONTENT_A, RO_POISON));
  im = ro_inst(&x);
  r.struct_size = RO_REPORT_BYTES;
  im->cpu.status = 0xA0u; /* S and OV */
  im->cpu.intff = 1u;
  RO_CHECK(ivs_machine_cpu_state(x.m, &r) == IVS_OK);
  RO_CHECK(r.s == 1u && r.z == 0u && r.ov == 1u && r.c == 0u && r.intr_enabled == 1u);
  im->cpu.status = 0x50u; /* Z and C */
  im->cpu.intff = 0u;
  RO_CHECK(ivs_machine_cpu_state(x.m, &r) == IVS_OK);
  RO_CHECK(r.s == 0u && r.z == 1u && r.ov == 0u && r.c == 1u && r.intr_enabled == 0u);
  rc = IVS_TEST_PASS;
done:
  ivs_ms_free(&x);
  return rc;
}

/* A larger caller struct (64 bytes, and the IVS_STRUCT_SIZE_MAX bound itself) is accepted: the
 * core writes its 36 bytes, sets struct_size to 36 and leaves the caller's tail untouched. */
static int ro_cpu_larger_struct(void)
{
  static const size_t sizes[2] = {64u, IVS_STRUCT_SIZE_MAX};
  ivs_ms_inst x;
  ivs_cpu_state_report *r = NULL;
  int rc = IVS_TEST_FAIL;
  size_t k;
  size_t i;
  IVS_CHECK(ivs_ms_powered(&x, IVS_MS_CONTENT_A, RO_POISON));
  for (k = 0u; k < 2u; k++) {
    const uint8_t *p;
    r = ro_report(sizes[k], (uint32_t)sizes[k]);
    RO_CHECK(r != NULL);
    RO_CHECK(ivs_machine_cpu_state(x.m, r) == IVS_OK);
    RO_CHECK(r->struct_size == RO_REPORT_BYTES && r->run_state == IVS_CPU_STATE_RESET_PENDING);
    p = (const uint8_t *)(const void *)r;
    for (i = RO_REPORT_BYTES; i < sizes[k]; i++) {
      RO_CHECK(p[i] == RO_POISON);
    }
    free(r);
    r = NULL;
  }
  rc = IVS_TEST_PASS;
done:
  free(r);
  ivs_ms_free(&x);
  return rc;
}

/* ---- cpu_state refusals (D-08, SP4) ---- */

/* BAD_ARG: out NULL, with a valid and with a NULL handle (BAD_ARG is checked first). */
static int ro_refuse_cpu_null(void)
{
  ivs_ms_inst x;
  ivs_run_snap s;
  int rc = IVS_TEST_FAIL;
  IVS_CHECK(ivs_ms_powered(&x, IVS_MS_CONTENT_A, RO_POISON));
  s.copy = NULL;
  RO_CHECK(ivs_run_snap_take(&s, &x, 1));
  RO_CHECK(ivs_machine_cpu_state(x.m, NULL) == IVS_E_BAD_ARG);
  RO_CHECK(ivs_machine_cpu_state(NULL, NULL) == IVS_E_BAD_ARG);
  RO_CHECK(ivs_run_snap_same(&s, &x));
  rc = IVS_TEST_PASS;
done:
  ivs_run_snap_free(&s);
  ivs_ms_free(&x);
  return rc;
}

/* BAD_ARG: struct_size 0, 35 (one below v1), IVS_STRUCT_SIZE_MAX + 1 and the poison itself; also
 * with a NULL handle (BAD_ARG first). The out struct keeps every byte. */
static int ro_refuse_cpu_struct_size(void)
{
  static const uint32_t bad[4] = {0u, RO_REPORT_BYTES - 1u, IVS_STRUCT_SIZE_MAX + 1u, 0xA5A5A5A5u};
  ivs_ms_inst x;
  ivs_run_snap s;
  ivs_cpu_state_report *r = NULL;
  int rc = IVS_TEST_FAIL;
  size_t k;
  IVS_CHECK(ivs_ms_powered(&x, IVS_MS_CONTENT_A, RO_POISON));
  s.copy = NULL;
  RO_CHECK(ivs_run_snap_take(&s, &x, 1));
  r = ro_report(sizeof *r, 0u);
  RO_CHECK(r != NULL);
  for (k = 0u; k < 4u; k++) {
    r->struct_size = bad[k];
    RO_CHECK(ivs_machine_cpu_state(x.m, r) == IVS_E_BAD_ARG);
    RO_CHECK(ro_report_kept(r, sizeof *r, bad[k]));
    RO_CHECK(ivs_machine_cpu_state(NULL, r) == IVS_E_BAD_ARG);
    RO_CHECK(ro_report_kept(r, sizeof *r, bad[k]));
  }
  RO_CHECK(ivs_run_snap_same(&s, &x));
  rc = IVS_TEST_PASS;
done:
  free(r);
  ivs_run_snap_free(&s);
  ivs_ms_free(&x);
  return rc;
}

/* BAD_HANDLE: a NULL handle, a destroyed instance, and a destroyed instance with out inside its
 * storage (no valid instance, so no overlap class); out and storage keep every byte. */
static int ro_refuse_cpu_bad_handle(void)
{
  ivs_ms_inst x;
  ivs_run_snap s;
  ivs_cpu_state_report *r = NULL;
  ivs_cpu_state_report *in;
  int rc = IVS_TEST_FAIL;
  IVS_CHECK(ivs_ms_powered(&x, IVS_MS_CONTENT_A, RO_POISON));
  s.copy = NULL;
  r = ro_report(sizeof *r, RO_REPORT_BYTES);
  RO_CHECK(r != NULL);
  RO_CHECK(ivs_machine_cpu_state(NULL, r) == IVS_E_BAD_HANDLE);
  RO_CHECK(ro_report_kept(r, sizeof *r, RO_REPORT_BYTES));
  RO_CHECK(ivs_machine_destroy(x.m) == IVS_OK);
  in = (ivs_cpu_state_report *)(void *)(x.storage + ro_inside_off());
  in->struct_size = RO_REPORT_BYTES;
  RO_CHECK(ivs_run_snap_take(&s, &x, 0));
  RO_CHECK(ivs_machine_cpu_state(x.m, r) == IVS_E_BAD_HANDLE);
  RO_CHECK(ro_report_kept(r, sizeof *r, RO_REPORT_BYTES));
  RO_CHECK(ivs_machine_cpu_state(x.m, in) == IVS_E_BAD_HANDLE);
  RO_CHECK(ivs_run_snap_same(&s, &x));
  rc = IVS_TEST_PASS;
done:
  free(r);
  ivs_run_snap_free(&s);
  ivs_ms_free(&x);
  return rc;
}

/* BAD_ARG: out inside the instance storage, and out straddling its end (16 bytes inside, the
 * rest past it: the core reads only struct_size, inside, before it refuses). A positive control
 * places out just after the instance bytes of a longer host allocation (accepted). */
static int ro_refuse_cpu_out_inside(void)
{
  ivs_ms_inst x;
  ivs_run_snap s;
  ivs_cpu_state_report *in;
  ivs_cpu_state_report *tail;
  ivs_cpu_state_report *after;
  ivs_create_options o;
  uint8_t *big = NULL;
  ivs_machine *bm = NULL;
  int rc = IVS_TEST_FAIL;
  IVS_CHECK(ivs_ms_powered(&x, IVS_MS_CONTENT_A, RO_POISON));
  s.copy = NULL;
  in = (ivs_cpu_state_report *)(void *)(x.storage + ro_inside_off());
  in->struct_size = RO_REPORT_BYTES;
  tail = (ivs_cpu_state_report *)(void *)(x.storage + IVS_MACHINE_BYTES - 16u);
  tail->struct_size = RO_REPORT_BYTES;
  RO_CHECK(ivs_run_snap_take(&s, &x, 1));
  RO_CHECK(ivs_machine_cpu_state(x.m, in) == IVS_E_BAD_ARG);
  RO_CHECK(ivs_machine_cpu_state(x.m, tail) == IVS_E_BAD_ARG);
  RO_CHECK(ivs_run_snap_same(&s, &x));
  memset(&o, 0, sizeof o);
  o.struct_size = (uint32_t)sizeof o;
  o.profile_id = IVS_MS_PROFILE;
  big = aligned_alloc(IVS_MACHINE_ALIGN, IVS_MACHINE_BYTES + 64u);
  RO_CHECK(big != NULL);
  memset(big, RO_POISON, IVS_MACHINE_BYTES + 64u);
  RO_CHECK(ivs_machine_create(&o, big, IVS_MACHINE_BYTES + 64u, &bm) == IVS_OK);
  after = (ivs_cpu_state_report *)(void *)(big + IVS_MACHINE_BYTES);
  after->struct_size = RO_REPORT_BYTES;
  RO_CHECK(ivs_machine_cpu_state(bm, after) == IVS_OK);
  RO_CHECK(after->struct_size == RO_REPORT_BYTES && after->run_state == IVS_CPU_STATE_UNRESET);
  rc = IVS_TEST_PASS;
done:
  free(big);
  ivs_run_snap_free(&s);
  ivs_ms_free(&x);
  return rc;
}

const ivs_test_case ivs_run_observe_cases[] = {
    {"run.cpu-state.off", ro_cpu_off, NULL},
    {"run.cpu-state.after-reset", ro_cpu_after_reset, NULL},
    {"run.cpu-state.after-steps", ro_cpu_after_steps, NULL},
    {"run.cpu-state.flag-bits", ro_cpu_flag_bits, NULL},
    {"run.cpu-state.larger-struct", ro_cpu_larger_struct, NULL},
    {"run.refuse.cpu-state.null", ro_refuse_cpu_null, NULL},
    {"run.refuse.cpu-state.struct-size", ro_refuse_cpu_struct_size, NULL},
    {"run.refuse.cpu-state.bad-handle", ro_refuse_cpu_bad_handle, NULL},
    {"run.refuse.cpu-state.out-inside-instance", ro_refuse_cpu_out_inside, NULL},
};
const size_t ivs_run_observe_count = sizeof ivs_run_observe_cases / sizeof ivs_run_observe_cases[0];

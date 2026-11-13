/* SPDX-License-Identifier: MIT */
/* Suite stic, family state (plan 06-07; D-04, VID-06): the public inspection call ivs_machine_stic_state, the stored view of the
 * STIC. Expected values are relations to the instance's stored members and to the generated parameters (frame start = phase + frame
 * index x F), and the offsets of the report are literals taken from the plan's layout, never read back from the core.
 *   stic.state.refusal-ladder   every refusal class, in order, with no byte of out written and the instance unchanged
 *   stic.state.fields           after a guest that strobes, writes registers and crosses the register window, every field
 *                               equals the stored value and the gate bits equal the memory system's gates
 *   stic.state.no-write         whole-instance equality (storage, nine digests, machine time) around the call
 *   stic.state.v1-size-caller   a caller with exactly 208 bytes, and a larger caller whose tail stays untouched
 *   stic.state.struct-pins      the size and every offset of the report as literals, and the reserved words written 0
 * The snapshot oracle (ss_snap_*) is shared with the peek family (stic_peek.c). */
#include <stddef.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>

#include "ivs/ivs.h"
#include "machine/machine.h"
#include "stic_cases.h"

#define SS_POISON 0xA5u

static const uint32_t ss_domains[SS_DIGESTS] = {IVS_DIGEST_COMPOSITE, IVS_DIGEST_CPU,   IVS_DIGEST_MEM,   IVS_DIGEST_IMAGES,
                                                IVS_DIGEST_INSTANCE,  IVS_DIGEST_VIDEO, IVS_DIGEST_AUDIO, IVS_DIGEST_INPUT,
                                                IVS_DIGEST_PSGLOG};

int ss_snap_take(ss_snap *s, const ivs_ms_inst *x)
{
  uint32_t i;
  memset(s, 0, sizeof *s);
  s->copy = malloc(IVS_MACHINE_BYTES);
  if (s->copy == NULL) {
    return 0;
  }
  memcpy(s->copy, x->storage, IVS_MACHINE_BYTES);
  for (i = 0u; i < SS_DIGESTS; i++) {
    if (ivs_machine_state_digest(x->m, ss_domains[i], s->digest[i]) != IVS_OK) {
      ivs_test_diag("snapshot: digest of domain %u refused", (unsigned)ss_domains[i]);
      free(s->copy);
      s->copy = NULL;
      return 0;
    }
  }
  s->time_ticks = ivs_machine_from_handle(x->m)->time_ticks;
  return 1;
}

int ss_snap_same(const ss_snap *s, const ivs_ms_inst *x)
{
  uint32_t i;
  if (s->copy == NULL) {
    return 0;
  }
  if (memcmp(s->copy, x->storage, IVS_MACHINE_BYTES) != 0) {
    for (i = 0u; i < IVS_MACHINE_BYTES; i++) {
      if (s->copy[i] != x->storage[i]) {
        break;
      }
    }
    ivs_test_diag("instance storage changed (first byte %u)", (unsigned)i);
    return 0;
  }
  for (i = 0u; i < SS_DIGESTS; i++) {
    uint8_t d[32];
    if (ivs_machine_state_digest(x->m, ss_domains[i], d) != IVS_OK || memcmp(d, s->digest[i], 32u) != 0) {
      ivs_test_diag("digest of domain %u changed", (unsigned)ss_domains[i]);
      return 0;
    }
  }
  if (ivs_machine_from_handle(x->m)->time_ticks != s->time_ticks) {
    ivs_test_diag("machine time changed");
    return 0;
  }
  return 1;
}

void ss_snap_free(ss_snap *s)
{
  free(s->copy);
  s->copy = NULL;
}

/* A heap report of bytes bytes with struct_size set to size_field and every other byte the poison. */
static ivs_stic_report *ss_report(size_t bytes, uint32_t size_field)
{
  ivs_stic_report *r = malloc(bytes);
  if (r == NULL) {
    return NULL;
  }
  memset(r, SS_POISON, bytes);
  r->struct_size = size_field;
  return r;
}

/* 1 when every byte of r past the first four is the poison and the first four are size_field (nothing was written). */
static int ss_report_kept(const ivs_stic_report *r, size_t bytes, uint32_t size_field)
{
  const uint8_t *p = (const uint8_t *)(const void *)r;
  size_t i;
  if (r->struct_size != size_field) {
    return 0;
  }
  for (i = sizeof r->struct_size; i < bytes; i++) {
    if (p[i] != SS_POISON) {
      return 0;
    }
  }
  return 1;
}

#define SS_CHECK(cond)                                                                                 \
  do {                                                                                                 \
    if (!(cond)) {                                                                                     \
      ivs_test_diag("check failed at line %d: %s", __LINE__, #cond);                                   \
      goto done;                                                                                       \
    }                                                                                                  \
  } while (0)

/* An offset inside the instance storage where a whole report fits (past the header words, 16 aligned). */
#define SS_INSIDE_OFF 1024u

/* ---- the refusal ladder ---- */

static int ss_refusal_ladder(void)
{
  static const uint32_t bad[4] = {0u, IVS_STIC_REPORT_V1_SIZE - 1u, IVS_STRUCT_SIZE_MAX + 1u, 0xA5A5A5A5u};
  ivs_ms_inst x;
  ss_snap s;
  ivs_stic_report *r = NULL;
  ivs_stic_report *in;
  ivs_stic_report *tail;
  ivs_stic_report *after;
  ivs_create_options o;
  uint8_t *big = NULL;
  ivs_machine *bm = NULL;
  int rc = IVS_TEST_FAIL;
  size_t k;
  s.copy = NULL;
  SC_CHECK(ivs_ms_powered(&x, IVS_MS_CONTENT_A, SS_POISON));
  r = ss_report(sizeof *r, IVS_STIC_REPORT_V1_SIZE);
  SS_CHECK(r != NULL);
  SS_CHECK(ss_snap_take(&s, &x));
  /* 1: out NULL is BAD_ARG, with a valid handle and with a NULL handle (BAD_ARG first) */
  SS_CHECK(ivs_machine_stic_state(x.m, NULL) == IVS_E_BAD_ARG);
  SS_CHECK(ivs_machine_stic_state(NULL, NULL) == IVS_E_BAD_ARG);
  /* 2: a struct_size below the v1 size or above the bound, also with a NULL handle; the out struct keeps every byte */
  for (k = 0u; k < 4u; k++) {
    r->struct_size = bad[k];
    SS_CHECK(ivs_machine_stic_state(x.m, r) == IVS_E_BAD_ARG);
    SS_CHECK(ss_report_kept(r, sizeof *r, bad[k]));
    SS_CHECK(ivs_machine_stic_state(NULL, r) == IVS_E_BAD_ARG);
    SS_CHECK(ss_report_kept(r, sizeof *r, bad[k]));
  }
  r->struct_size = IVS_STIC_REPORT_V1_SIZE;
  /* 3: a NULL handle with a good out is BAD_HANDLE, nothing written */
  SS_CHECK(ivs_machine_stic_state(NULL, r) == IVS_E_BAD_HANDLE);
  SS_CHECK(ss_report_kept(r, sizeof *r, IVS_STIC_REPORT_V1_SIZE));
  /* 4: out inside the instance storage, and straddling its end, is BAD_ARG; the instance keeps every byte */
  in = (ivs_stic_report *)(void *)(x.storage + SS_INSIDE_OFF);
  in->struct_size = IVS_STIC_REPORT_V1_SIZE;
  tail = (ivs_stic_report *)(void *)(x.storage + IVS_MACHINE_BYTES - 16u);
  tail->struct_size = IVS_STIC_REPORT_V1_SIZE;
  ss_snap_free(&s);
  SS_CHECK(ss_snap_take(&s, &x));
  SS_CHECK(ivs_machine_stic_state(x.m, in) == IVS_E_BAD_ARG);
  SS_CHECK(ivs_machine_stic_state(x.m, tail) == IVS_E_BAD_ARG);
  SS_CHECK(ss_snap_same(&s, &x));
  /* 5: a refused call leaves the machine equal; a valid call from the same state still works (the ladder is not sticky) */
  SS_CHECK(ivs_machine_stic_state(x.m, r) == IVS_OK);
  SS_CHECK(r->struct_size == IVS_STIC_REPORT_V1_SIZE);
  SS_CHECK(ss_snap_same(&s, &x));
  /* 6: a destroyed instance is BAD_HANDLE (no valid instance, so no overlap class), with out outside and inside the storage */
  r->struct_size = IVS_STIC_REPORT_V1_SIZE;
  memset(&r->mode, SS_POISON, sizeof *r - sizeof r->struct_size);
  SS_CHECK(ivs_machine_destroy(x.m) == IVS_OK);
  in->struct_size = IVS_STIC_REPORT_V1_SIZE;
  SS_CHECK(ivs_machine_stic_state(x.m, r) == IVS_E_BAD_HANDLE);
  SS_CHECK(ss_report_kept(r, sizeof *r, IVS_STIC_REPORT_V1_SIZE));
  SS_CHECK(ivs_machine_stic_state(x.m, in) == IVS_E_BAD_HANDLE);
  /* 7: positive control for the overlap test: out just after the instance bytes of a longer host allocation is accepted */
  memset(&o, 0, sizeof o);
  o.struct_size = (uint32_t)sizeof o;
  o.profile_id = IVS_MS_PROFILE;
  big = aligned_alloc(IVS_MACHINE_ALIGN, IVS_MACHINE_BYTES + 256u);
  SS_CHECK(big != NULL);
  memset(big, SS_POISON, IVS_MACHINE_BYTES + 256u);
  SS_CHECK(ivs_machine_create(&o, big, IVS_MACHINE_BYTES + 256u, &bm) == IVS_OK);
  after = (ivs_stic_report *)(void *)(big + IVS_MACHINE_BYTES);
  after->struct_size = IVS_STIC_REPORT_V1_SIZE;
  SS_CHECK(ivs_machine_stic_state(bm, after) == IVS_OK);
  SS_CHECK(after->struct_size == IVS_STIC_REPORT_V1_SIZE);
  rc = IVS_TEST_PASS;
done:
  free(big);
  free(r);
  ss_snap_free(&s);
  ivs_ms_free(&x);
  return rc;
}

/* ---- the fields ---- */

/* Builds the guest of the fields case: strobe the display enable, store two registers, cross the register window, then one
 * blocked read and one blocked write. Words: 2 + 2 + 2 + 2 + 5 + 2 + 2 + 2 = 19. */
static size_t ss_fields_guest(uint16_t *w)
{
  size_t n = 0u;
  n = sc_w_mvii(w, n, 0u, 0x0355u);
  n = sc_w_mvo(w, n, 0u, 0x0020u); /* display-enable strobe */
  n = sc_w_mvo(w, n, 0u, 0x0000u); /* register 0 keeps 07FF */
  n = sc_w_mvo(w, n, 0u, 0x0018u); /* collision register 0 keeps 03FF */
  n = sc_w_delay(w, n, SC_DELAY_PAST_REG);
  n = sc_w_mvi(w, n, 0x0018u, 1u); /* a blocked read */
  n = sc_w_mvo(w, n, 0u, 0x0001u); /* a blocked write */
  n = sc_w_loop(w, n);
  return n;
}

/* 1 when every field of r equals the instance's stored value, read here from the instance itself. */
static int ss_fields_equal(const ivs_stic_report *r, const struct ivs_machine *im)
{
  const ivs_stic_params *p = ivs_stic_params_get(im->profile);
  uint32_t i;
  if (r->struct_size != IVS_STIC_REPORT_V1_SIZE || r->mode != im->stic.mode || r->display_latch != im->stic.display_latch ||
      r->windows_active != im->stic.windows_active || r->cs_ptr != im->stic.cs_ptr ||
      r->event_index != im->stic.event_index || r->frame_index != im->stic.frame_index ||
      r->blocked_reads != im->stic.blocked_reads || r->blocked_writes != im->stic.blocked_writes ||
      r->late_grants != im->stic.late_grants) {
    return 0;
  }
  if (r->regs_window_open != (uint32_t)im->mem.gate_open[IVS_MEMSYS_GATE_STIC_REGS] ||
      r->gmem_window_open != (uint32_t)im->mem.gate_open[IVS_MEMSYS_GATE_GMEM]) {
    return 0;
  }
  if (r->frame_start_tick != (uint64_t)p->frame_phase_ticks + im->stic.frame_index * (uint64_t)p->frame_ticks) {
    return 0;
  }
  for (i = 0u; i < 64u; i++) {
    if (r->regs[i] != im->mem.stic_reg[i]) {
      return 0;
    }
  }
  return r->reserved0 == 0u && r->reserved[0] == 0u && r->reserved[1] == 0u && r->reserved[2] == 0u && r->reserved[3] == 0u;
}

static int ss_fields(void)
{
  uint16_t w[SC_PROG_MAX];
  size_t n = ss_fields_guest(w);
  ivs_ms_inst x;
  ivs_stic_report r;
  const struct ivs_machine *im;
  int rc = IVS_TEST_FAIL;
  uint32_t reserved_zero = 1u;
  uint32_t i;
  SC_CHECK(n <= SC_PROG_MAX);
  SC_CHECK(sc_guest(&x, w, n, 0u));
  im = ivs_machine_from_handle(x.m);
  /* early, inside both windows: the gate bits read 1 and equal the memory system's; the instance just reset */
  SS_CHECK(sc_run_until(&x, 50u));
  memset(&r, SS_POISON, sizeof r);
  r.struct_size = IVS_STIC_REPORT_V1_SIZE;
  SS_CHECK(ivs_machine_stic_state(x.m, &r) == IVS_OK);
  SS_CHECK(ss_fields_equal(&r, im));
  SS_CHECK(r.regs_window_open == 1u && r.gmem_window_open == 1u);
  /* after the guest crossed the register window: latch set, a register stored, one blocked read and one blocked write */
  SS_CHECK(sc_run_until(&x, 40000u));
  memset(&r, SS_POISON, sizeof r);
  r.struct_size = IVS_STIC_REPORT_V1_SIZE;
  SS_CHECK(ivs_machine_stic_state(x.m, &r) == IVS_OK);
  SS_CHECK(ss_fields_equal(&r, im));
  SS_CHECK(r.display_latch == 1u && r.mode == 0u);
  SS_CHECK(r.regs[0] == 0x0355u && r.regs[0x18] == 0x0355u);
  SS_CHECK(r.blocked_reads == 1u && r.blocked_writes == 1u);
  SS_CHECK(r.regs_window_open == 0u); /* the register gate closed before the guest's blocked accesses */
  SS_CHECK(r.frame_index == 0u && r.frame_start_tick == (uint64_t)ivs_stic_params_get(im->profile)->frame_phase_ticks);
  /* the stored words, not the CPU view: a register that reads back with bits forced is reported as stored */
  SS_CHECK(r.regs[0x18] == im->mem.stic_reg[0x18]);
  /* the strobe and reserved registers store nothing in a normal run (the independent register literal names 24 of them:
   * 2 strobes and 22 reserved) */
  {
    uint32_t reserved_regs = 0u;
    const sl_reg_row *rows = sl_regs();
    for (i = 0u; i < 64u; i++) {
      if (rows[i].cls == 1u || rows[i].cls == 3u) {
        reserved_regs++;
        if (r.regs[i] != 0u) {
          reserved_zero = 0u;
        }
      }
    }
    SS_CHECK(reserved_regs == 24u && reserved_zero == 1u);
  }
  /* a stored word that no CPU read can return still shows: a raw poke into a write-only register is visible */
  {
    struct ivs_machine *mut = ivs_machine_from_handle_mut(x.m);
    mut->mem.stic_reg[0x01] = 0x04D2u;
    mut->stic.cs_ptr = 3u;
    mut->stic.late_grants = 7u;
  }
  memset(&r, SS_POISON, sizeof r);
  r.struct_size = IVS_STIC_REPORT_V1_SIZE;
  SS_CHECK(ivs_machine_stic_state(x.m, &r) == IVS_OK);
  SS_CHECK(r.regs[1] == 0x04D2u && r.cs_ptr == 3u && r.late_grants == 7u);
  SS_CHECK(ss_fields_equal(&r, im));
  rc = IVS_TEST_PASS;
done:
  ivs_ms_free(&x);
  return rc;
}

/* ---- no write ---- */

static int ss_no_write(void)
{
  uint16_t w[SC_PROG_MAX];
  size_t n = ss_fields_guest(w);
  ivs_ms_inst x;
  ss_snap s;
  ivs_stic_report r;
  int rc = IVS_TEST_FAIL;
  uint32_t k;
  s.copy = NULL;
  SC_CHECK(n <= SC_PROG_MAX);
  SC_CHECK(sc_guest(&x, w, n, 0u));
  /* three states: just reset, inside the window, past the window; each time the call changes nothing and a repeat is equal */
  for (k = 0u; k < 3u; k++) {
    ivs_stic_report r2;
    SS_CHECK(k == 0u || sc_run_until(&x, k == 1u ? 400u : 40000u));
    SS_CHECK(ss_snap_take(&s, &x));
    memset(&r, 0, sizeof r);
    r.struct_size = IVS_STIC_REPORT_V1_SIZE;
    SS_CHECK(ivs_machine_stic_state(x.m, &r) == IVS_OK);
    SS_CHECK(ss_snap_same(&s, &x));
    memset(&r2, 0, sizeof r2);
    r2.struct_size = IVS_STIC_REPORT_V1_SIZE;
    SS_CHECK(ivs_machine_stic_state(x.m, &r2) == IVS_OK);
    SS_CHECK(ss_snap_same(&s, &x));
    SS_CHECK(memcmp(&r, &r2, sizeof r) == 0);
    ss_snap_free(&s);
  }
  /* also valid while OFF: a created, never reset instance */
  ivs_ms_free(&x);
  SS_CHECK(ivs_ms_new(&x, SS_POISON));
  SS_CHECK(ss_snap_take(&s, &x));
  memset(&r, 0, sizeof r);
  r.struct_size = IVS_STIC_REPORT_V1_SIZE;
  SS_CHECK(ivs_machine_stic_state(x.m, &r) == IVS_OK);
  SS_CHECK(ss_snap_same(&s, &x));
  rc = IVS_TEST_PASS;
done:
  ss_snap_free(&s);
  ivs_ms_free(&x);
  return rc;
}

/* ---- the caller sizes ---- */

static int ss_v1_size_caller(void)
{
  static const size_t sizes[2] = {256u, IVS_STRUCT_SIZE_MAX};
  uint16_t w[SC_PROG_MAX];
  size_t n = ss_fields_guest(w);
  ivs_ms_inst x;
  ivs_stic_report *r = NULL;
  ivs_stic_report ref;
  int rc = IVS_TEST_FAIL;
  size_t k;
  size_t i;
  SC_CHECK(n <= SC_PROG_MAX);
  SC_CHECK(sc_guest(&x, w, n, 0u));
  SS_CHECK(sc_run_until(&x, 40000u));
  /* exactly 208 bytes: the heap block has no byte beyond the report, so a write past it is a sanitizer finding */
  r = ss_report(IVS_STIC_REPORT_V1_SIZE, IVS_STIC_REPORT_V1_SIZE);
  SS_CHECK(r != NULL);
  SS_CHECK(ivs_machine_stic_state(x.m, r) == IVS_OK);
  SS_CHECK(r->struct_size == IVS_STIC_REPORT_V1_SIZE);
  SS_CHECK(ss_fields_equal(r, ivs_machine_from_handle(x.m)));
  ref = *r;
  free(r);
  r = NULL;
  /* a larger caller struct (256 bytes and the bound itself): the core writes its 208 bytes, sets struct_size to 208 and
   * leaves the caller's tail untouched */
  for (k = 0u; k < 2u; k++) {
    const uint8_t *p;
    r = ss_report(sizes[k], (uint32_t)sizes[k]);
    SS_CHECK(r != NULL);
    SS_CHECK(ivs_machine_stic_state(x.m, r) == IVS_OK);
    SS_CHECK(r->struct_size == IVS_STIC_REPORT_V1_SIZE);
    SS_CHECK(memcmp(r, &ref, IVS_STIC_REPORT_V1_SIZE) == 0);
    p = (const uint8_t *)(const void *)r;
    for (i = IVS_STIC_REPORT_V1_SIZE; i < sizes[k]; i++) {
      SS_CHECK(p[i] == SS_POISON);
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

/* ---- the layout pins ---- */

static int ss_struct_pins(void)
{
  ivs_ms_inst x;
  ivs_stic_report r;
  int rc = IVS_TEST_FAIL;
  uint8_t *b = (uint8_t *)(void *)&r;
  uint32_t i;
  SS_CHECK(IVS_STIC_REPORT_V1_SIZE == 208u && sizeof(ivs_stic_report) == 208u);
  SS_CHECK(offsetof(ivs_stic_report, struct_size) == 0u && offsetof(ivs_stic_report, mode) == 4u);
  SS_CHECK(offsetof(ivs_stic_report, display_latch) == 8u && offsetof(ivs_stic_report, windows_active) == 12u);
  SS_CHECK(offsetof(ivs_stic_report, cs_ptr) == 16u && offsetof(ivs_stic_report, regs_window_open) == 20u);
  SS_CHECK(offsetof(ivs_stic_report, gmem_window_open) == 24u && offsetof(ivs_stic_report, event_index) == 28u);
  SS_CHECK(offsetof(ivs_stic_report, frame_index) == 32u && offsetof(ivs_stic_report, frame_start_tick) == 40u);
  SS_CHECK(offsetof(ivs_stic_report, blocked_reads) == 48u && offsetof(ivs_stic_report, blocked_writes) == 52u);
  SS_CHECK(offsetof(ivs_stic_report, late_grants) == 56u && offsetof(ivs_stic_report, reserved0) == 60u);
  SS_CHECK(offsetof(ivs_stic_report, regs) == 64u && offsetof(ivs_stic_report, reserved) == 192u);
  SC_CHECK(ivs_ms_powered(&x, IVS_MS_CONTENT_A, SS_POISON));
  memset(&r, SS_POISON, sizeof r);
  r.struct_size = IVS_STIC_REPORT_V1_SIZE;
  SS_CHECK(ivs_machine_stic_state(x.m, &r) == IVS_OK);
  /* a freshly reset machine: the mode and latch from the profile's power-on record (zero), registers 0, reserved words 0 and
   * no poison byte left anywhere (the report is built whole, never copied in pieces) */
  SS_CHECK(r.mode == 0u && r.display_latch == 0u && r.windows_active == 0u && r.cs_ptr == 0u);
  SS_CHECK(r.blocked_reads == 0u && r.blocked_writes == 0u && r.late_grants == 0u && r.frame_index == 0u);
  for (i = 0u; i < 64u; i++) {
    SS_CHECK(r.regs[i] == 0u);
  }
  SS_CHECK(r.reserved0 == 0u && r.reserved[0] == 0u && r.reserved[1] == 0u && r.reserved[2] == 0u && r.reserved[3] == 0u);
  SS_CHECK(memchr(&b[4], SS_POISON, IVS_STIC_REPORT_V1_SIZE - 4u) == NULL);
  rc = IVS_TEST_PASS;
done:
  ivs_ms_free(&x);
  return rc;
}

/* Plan 06-11 (VID-06): the report's cs_ptr after a rendered frame equals the final pointer a frame with a known number of advances leaves. The
 * BACKTAB holds N Color Stack cards with bit 13 set (the word 2000: graphics ROM card 0, foreground 0, advance), each advance moving the pointer by one
 * and the pointer wrapping at 4: 3 advances end at 3, 4 advances at 0, 5 at 1. The guest reads $0021 (Color Stack), strobes and loops; the machine
 * runs past the first frame event. A Foreground/Background run of the same BACKTAB leaves the pointer 0 (bit 13 is a background bit there). */
static int ss_cs_ptr_run(uint32_t advances, uint32_t cs_mode, uint32_t *ptr)
{
  uint16_t w[SC_PROG_MAX];
  size_t n = 0u;
  ivs_ms_inst x;
  ivs_stic_report r;
  struct ivs_machine *im;
  uint32_t k;
  int ok = 0;
  if (cs_mode != 0u) {
    n = sc_w_mvi(w, n, 0x0021u, 1u);
  }
  n = sc_w_mvo(w, n, 0u, 0x0020u);
  n = sc_w_loop(w, n);
  SC_CHECK(sc_guest(&x, w, n, 0u));
  im = ivs_machine_from_handle_mut(x.m);
  for (k = 0u; k < advances; k++) {
    ivs_memsys_poke_raw_internal(&im->mem, (uint16_t)(0x0200u + 7u + 13u * k), 0x2000u);
  }
  if (sc_run_until(&x, 60000u)) {
    memset(&r, 0, sizeof r);
    r.struct_size = IVS_STIC_REPORT_V1_SIZE;
    ok = ivs_machine_stic_state(x.m, &r) == IVS_OK && im->obs.video.count == 1u && r.mode == cs_mode && r.frame_index == 1u;
    *ptr = r.cs_ptr;
  }
  ivs_ms_free(&x);
  return ok;
}

static int ss_cs_ptr_after_render(void)
{
  uint32_t p = 99u;
  SC_CHECK(ss_cs_ptr_run(0u, 1u, &p) && p == 0u);
  SC_CHECK(ss_cs_ptr_run(3u, 1u, &p) && p == 3u);
  SC_CHECK(ss_cs_ptr_run(4u, 1u, &p) && p == 0u);
  SC_CHECK(ss_cs_ptr_run(5u, 1u, &p) && p == 1u);
  SC_CHECK(ss_cs_ptr_run(3u, 0u, &p) && p == 0u);
  return IVS_TEST_PASS;
}

const ivs_test_case ivs_stic_state_cases[] = {
    {"stic.state.refusal-ladder", ss_refusal_ladder, NULL},
    {"stic.state.fields", ss_fields, NULL},
    {"stic.state.no-write", ss_no_write, NULL},
    {"stic.state.v1-size-caller", ss_v1_size_caller, NULL},
    {"stic.state.struct-pins", ss_struct_pins, NULL},
    {"stic.state.cs-ptr-after-render", ss_cs_ptr_after_render, NULL},
};
const size_t ivs_stic_state_count = sizeof ivs_stic_state_cases / sizeof ivs_stic_state_cases[0];

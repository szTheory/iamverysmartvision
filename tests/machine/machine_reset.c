/* SPDX-License-Identifier: MIT */
/* Suite machine, family reset (D-05, D-14, D-20, A-08 d, e, plan 03-12): the power-on reset
 * through the public header and one CPU unit at a time through the internal step seam
 * ivs_machine_step_unit_internal (machine/machine.h, never public).
 *
 * The tracer case runs a project-authored stand-in program (program A of machine_standin.c, its
 * decles hand-assembled from the project's Phase 2 vector rows) on the profile-driven bus,
 * entered through the profile's reset vector, and sees its RAM writes. Expected values are the
 * program's own arithmetic (R1 starts at $155 and is stored, then incremented, each pass), the
 * profile's reset.r7 ($1000) and ticks_per_cpu_cycle (4, docs/time-base.md) as literals, and the
 * sum of the cycles the seam reports; none is read back from the state under test except the
 * values being checked. */
#include <stddef.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>

#include "cpu/cp1610.h"
#include "ivs/ivs.h"
#include "ivs_test.h"
#include "machine/machine.h"
#include "machine_cases.h"
#include "memmap/memsys.h"

#define IVS_CHECK(cond)                                                                                \
  do {                                                                                                 \
    if (!(cond)) {                                                                                     \
      ivs_test_diag("check failed at line %d: %s", __LINE__, #cond);                                   \
      return IVS_TEST_FAIL;                                                                            \
    }                                                                                                  \
  } while (0)

#define MR_RESET_R7 0x1000u      /* profile reset.r7 */
#define MR_TICKS_PER_CYCLE 4u    /* profile ticks_per_cpu_cycle (D2) */
#define MR_MAX_UNITS 64u

/* The instance behind a test handle (the tests may read internals through machine.h). */
static const struct ivs_machine *mr_im(const ivs_machine *m)
{
  return ivs_machine_from_handle(m);
}

/* Tracer: load the stand-ins, reset, step program A until its store has run twice. */
static int mr_step_program(void)
{
  ivs_ms_inst x;
  const struct ivs_machine *im;
  ivs_info info;
  uint64_t cycles = 0u;
  uint32_t units = 0u;
  uint32_t stores = 0u;
  int rc = IVS_TEST_FAIL;
  if (!ivs_ms_new(&x, 0x5Au)) {
    return IVS_TEST_FAIL;
  }
  if (!ivs_ms_load(x.m, IVS_KIND_EXEC, ivs_ms_exec(IVS_MS_PROG_A), IVS_MS_EXEC_BYTES) ||
      !ivs_ms_load(x.m, IVS_KIND_GROM, ivs_ms_grom(0u), IVS_MS_GROM_BYTES)) {
    goto done;
  }
  if (ivs_machine_reset(x.m, IVS_RESET_POWER_ON) != IVS_OK) {
    ivs_test_diag("reset refused");
    goto done;
  }
  memset(&info, 0, sizeof info);
  info.struct_size = (uint32_t)sizeof info;
  if (ivs_machine_info(x.m, &info) != IVS_OK || info.phase != IVS_PHASE_POWERED || info.time_ticks != 0u) {
    ivs_test_diag("after reset: phase %u time %llu", (unsigned)info.phase, (unsigned long long)info.time_ticks);
    goto done;
  }
  im = mr_im(x.m);
  while (stores < 2u && units < MR_MAX_UNITS) {
    ivs_cp1610_result r;
    if (ivs_machine_step_unit_internal(x.m, &r) != IVS_OK) {
      ivs_test_diag("step %u refused", (unsigned)units);
      goto done;
    }
    units++;
    cycles += r.elapsed_cpu_cycles;
    if (r.unit_kind == IVS_CP1610_UNIT_INSTR && r.pc_before == 0x1002u) {
      stores++;
    }
  }
  if (stores != 2u) {
    ivs_test_diag("the store ran %u times in %u units", (unsigned)stores, (unsigned)units);
    goto done;
  }
  /* Pass 1 stored $155, pass 2 stored $156 (one INCR between them). */
  if (ivs_memsys_read(&im->mem, 0x0200u) != 0x0156u) {
    ivs_test_diag("[$0200] = $%04X, want $0156", (unsigned)ivs_memsys_read(&im->mem, 0x0200u));
    goto done;
  }
  if (im->cpu.r[1] != 0x0156u || im->cpu.r[7] < 0x1002u || im->cpu.r[7] > 0x1006u) {
    ivs_test_diag("R1 $%04X PC $%04X", (unsigned)im->cpu.r[1], (unsigned)im->cpu.r[7]);
    goto done;
  }
  if (ivs_machine_info(x.m, &info) != IVS_OK || info.time_ticks != cycles * MR_TICKS_PER_CYCLE ||
      cycles == 0u) {
    ivs_test_diag("time %llu ticks, want %llu cycles x %u", (unsigned long long)info.time_ticks,
                  (unsigned long long)cycles, MR_TICKS_PER_CYCLE);
    goto done;
  }
  rc = IVS_TEST_PASS;
done:
  ivs_ms_free(&x);
  return rc;
}

/* The first unit after reset is the reset entry, whose IAB the machine bus answers with the
 * profile's reset.r7; the next unit fetches program A's first decle there. */
static int mr_iab_reset_vector(void)
{
  ivs_ms_inst x;
  ivs_cp1610_result r;
  const struct ivs_machine *im;
  int rc = IVS_TEST_FAIL;
  if (!ivs_ms_powered(&x, IVS_MS_CONTENT_A, 0xA5u)) {
    return IVS_TEST_FAIL;
  }
  im = mr_im(x.m);
  if (im->cpu.run_state != IVS_CP1610_RS_RESET_PENDING) {
    ivs_test_diag("run state %u after reset", (unsigned)im->cpu.run_state);
  } else if (ivs_machine_step_unit_internal(x.m, &r) != IVS_OK || r.unit_kind != IVS_CP1610_UNIT_RESET_ENTRY ||
             im->cpu.r[7] != MR_RESET_R7) {
    ivs_test_diag("first unit kind %u, R7 $%04X", (unsigned)r.unit_kind, (unsigned)im->cpu.r[7]);
  } else if (ivs_profile_get(im->profile) == NULL || ivs_profile_get(im->profile)->reset_r7 != MR_RESET_R7 ||
             ivs_profile_get(im->profile)->ticks_per_cpu_cycle != MR_TICKS_PER_CYCLE) {
    ivs_test_diag("registry reset.r7 or ticks_per_cpu_cycle differs from the profile literal");
  } else if (ivs_machine_step_unit_internal(x.m, &r) != IVS_OK || r.unit_kind != IVS_CP1610_UNIT_INSTR ||
             r.pc_before != MR_RESET_R7 || r.opcode != 0x02B9u) {
    ivs_test_diag("second unit kind %u at $%04X opcode $%04X", (unsigned)r.unit_kind, (unsigned)r.pc_before,
                  (unsigned)r.opcode);
  } else {
    rc = IVS_TEST_PASS;
  }
  ivs_ms_free(&x);
  return rc;
}

/* EXEC and GROM only (no cartridge): the reset succeeds, the slots and the images digest are
 * untouched, and the instance is POWERED at time 0. */
static int mr_no_cart_ok(void)
{
  ivs_ms_inst x;
  ivs_info info;
  uint8_t img0[32];
  uint8_t img1[32];
  int rc = IVS_TEST_FAIL;
  if (!ivs_ms_new(&x, 0xA5u)) {
    return IVS_TEST_FAIL;
  }
  memset(&info, 0, sizeof info);
  info.struct_size = (uint32_t)sizeof info;
  if (!ivs_ms_load(x.m, IVS_KIND_EXEC, ivs_ms_exec(IVS_MS_PROG_A), IVS_MS_EXEC_BYTES) ||
      !ivs_ms_load(x.m, IVS_KIND_GROM, ivs_ms_grom(0u), IVS_MS_GROM_BYTES) ||
      ivs_machine_state_digest(x.m, IVS_DIGEST_IMAGES, img0) != IVS_OK) {
    ivs_test_diag("content or digest failed");
  } else if (ivs_machine_reset(x.m, IVS_RESET_POWER_ON) != IVS_OK) {
    ivs_test_diag("reset with EXEC and GROM only refused");
  } else if (ivs_machine_info(x.m, &info) != IVS_OK || info.phase != IVS_PHASE_POWERED || info.time_ticks != 0u ||
             info.slots[0].loaded != 1u || info.slots[1].loaded != 1u || info.slots[2].loaded != 0u) {
    ivs_test_diag("info: phase %u time %llu loaded %u %u %u", (unsigned)info.phase,
                  (unsigned long long)info.time_ticks, (unsigned)info.slots[0].loaded,
                  (unsigned)info.slots[1].loaded, (unsigned)info.slots[2].loaded);
  } else if (ivs_machine_state_digest(x.m, IVS_DIGEST_IMAGES, img1) != IVS_OK || memcmp(img0, img1, 32u) != 0) {
    ivs_test_diag("the reset changed the images digest");
  } else {
    rc = IVS_TEST_PASS;
  }
  ivs_ms_free(&x);
  return rc;
}

/* A power cycle after stepping gives the state of a fresh instance with the same content after
 * one reset: same composite digest and the same instance bytes (D-14, A-08 d). Program A wrote
 * $0200 and moved the CPU, so a reset that kept RAM or CPU state would differ. */
static int mr_equivalence(void)
{
  ivs_ms_inst x;
  ivs_ms_inst y;
  uint8_t dx[32];
  uint8_t dy[32];
  int rc = IVS_TEST_FAIL;
  if (!ivs_ms_powered(&x, IVS_MS_CONTENT_A, 0xA5u)) {
    return IVS_TEST_FAIL;
  }
  if (!ivs_ms_powered(&y, IVS_MS_CONTENT_A, 0x5Au)) {
    ivs_ms_free(&x);
    return IVS_TEST_FAIL;
  }
  if (!ivs_ms_step(x.m, 40u, NULL)) {
    ivs_test_diag("stepping failed");
  } else if (ivs_memsys_read(&ivs_machine_from_handle(x.m)->mem, 0x0200u) == 0x0000u) {
    ivs_test_diag("the program did not write $0200 before the power cycle");
  } else if (ivs_machine_reset(x.m, IVS_RESET_POWER_ON) != IVS_OK) {
    ivs_test_diag("power cycle refused");
  } else if (ivs_machine_state_digest(x.m, IVS_DIGEST_COMPOSITE, dx) != IVS_OK ||
             ivs_machine_state_digest(y.m, IVS_DIGEST_COMPOSITE, dy) != IVS_OK || memcmp(dx, dy, 32u) != 0) {
    ivs_test_diag("composite digest after a power cycle differs from one reset of the same content");
  } else if (memcmp(x.storage, y.storage, IVS_MACHINE_BYTES) != 0) {
    ivs_test_diag("instance bytes after a power cycle differ from one reset of the same content");
  } else {
    rc = IVS_TEST_PASS;
  }
  ivs_ms_free(&y);
  ivs_ms_free(&x);
  return rc;
}

/* After the first reset nothing returns the instance to OFF (A-08 d): a second reset is OK and
 * POWERED, and every load and unload of every kind is BAD_STATE with the instance unchanged. */
static int mr_power_cycle_one_way(void)
{
  ivs_ms_inst x;
  ivs_info info;
  uint8_t *snap = NULL;
  uint8_t *img = NULL;
  uint32_t k;
  int rc = IVS_TEST_FAIL;
  if (!ivs_ms_powered(&x, IVS_MS_CONTENT_A, 0xA5u)) {
    return IVS_TEST_FAIL;
  }
  memset(&info, 0, sizeof info);
  info.struct_size = (uint32_t)sizeof info;
  snap = malloc(IVS_MACHINE_BYTES);
  img = ivs_ms_exec(IVS_MS_PROG_B);
  if (snap == NULL || img == NULL || !ivs_ms_step(x.m, 5u, NULL)) {
    goto done;
  }
  if (ivs_machine_reset(x.m, IVS_RESET_POWER_ON) != IVS_OK || ivs_machine_info(x.m, &info) != IVS_OK ||
      info.phase != IVS_PHASE_POWERED) {
    ivs_test_diag("second reset: phase %u", (unsigned)info.phase);
    goto done;
  }
  memcpy(snap, x.storage, IVS_MACHINE_BYTES);
  for (k = IVS_KIND_EXEC; k <= IVS_KIND_CARTRIDGE; k++) {
    ivs_status sl = ivs_machine_load(x.m, k, img, k == IVS_KIND_GROM ? IVS_MS_GROM_BYTES : IVS_MS_EXEC_BYTES, NULL,
                                     NULL);
    ivs_status su = ivs_machine_unload(x.m, k);
    if (sl != IVS_E_BAD_STATE || su != IVS_E_BAD_STATE) {
      ivs_test_diag("kind %u: load %s, unload %s, want BAD_STATE", (unsigned)k, ivs_status_name(sl),
                    ivs_status_name(su));
      goto done;
    }
  }
  if (memcmp(snap, x.storage, IVS_MACHINE_BYTES) != 0) {
    ivs_test_diag("a refused load or unload changed the instance");
    goto done;
  }
  rc = IVS_TEST_PASS;
done:
  free(img);
  free(snap);
  ivs_ms_free(&x);
  return rc;
}

/* The seam refuses: out NULL is BAD_ARG, a NULL or destroyed handle BAD_HANDLE, and an instance
 * that is OFF (fresh, or with every image loaded but never reset) BAD_STATE; each refusal leaves
 * the instance bytes unchanged. */
static int mr_step_refusals(void)
{
  ivs_ms_inst x;
  ivs_cp1610_result r;
  uint8_t *snap = malloc(IVS_MACHINE_BYTES);
  int rc = IVS_TEST_FAIL;
  if (snap == NULL) {
    return IVS_TEST_FAIL;
  }
  if (!ivs_ms_new(&x, 0xA5u)) {
    free(snap);
    return IVS_TEST_FAIL;
  }
  memcpy(snap, x.storage, IVS_MACHINE_BYTES);
  if (ivs_machine_step_unit_internal(x.m, &r) != IVS_E_BAD_STATE || memcmp(snap, x.storage, IVS_MACHINE_BYTES) != 0) {
    ivs_test_diag("fresh instance: stepping while OFF was not BAD_STATE or changed the instance");
    goto done;
  }
  if (!ivs_ms_content(x.m, IVS_MS_CONTENT_A)) {
    goto done;
  }
  memcpy(snap, x.storage, IVS_MACHINE_BYTES);
  if (ivs_machine_step_unit_internal(x.m, &r) != IVS_E_BAD_STATE || memcmp(snap, x.storage, IVS_MACHINE_BYTES) != 0) {
    ivs_test_diag("loaded instance: stepping while OFF was not BAD_STATE or changed the instance");
    goto done;
  }
  if (ivs_machine_reset(x.m, IVS_RESET_POWER_ON) != IVS_OK) {
    goto done;
  }
  memcpy(snap, x.storage, IVS_MACHINE_BYTES);
  if (ivs_machine_step_unit_internal(x.m, NULL) != IVS_E_BAD_ARG ||
      ivs_machine_step_unit_internal(NULL, &r) != IVS_E_BAD_HANDLE || memcmp(snap, x.storage, IVS_MACHINE_BYTES) != 0) {
    ivs_test_diag("out NULL or handle NULL not refused, or the instance changed");
    goto done;
  }
  if (ivs_machine_destroy(x.m) != IVS_OK || ivs_machine_step_unit_internal(x.m, &r) != IVS_E_BAD_HANDLE) {
    ivs_test_diag("destroyed instance: stepping was not BAD_HANDLE");
    goto done;
  }
  rc = IVS_TEST_PASS;
done:
  free(snap);
  ivs_ms_free(&x);
  return rc;
}

/* ---- damaged profile index (review WR-03) ----
 * A loaded instance (EXEC and GROM, so a reset would succeed) is relocated by memcpy into a second
 * exact-size buffer, which is a valid handle (info OK), then its stored profile words are damaged
 * there: every call that takes the handle is BAD_HANDLE, the load report says HANDLE, and the
 * copy's bytes are unchanged. The calls run in order and the case stops at the first wrong
 * status, so no call reaches a profile table with the damaged index. */
#define MR_DAMAGE_PROFILE 0u    /* profile = registry count; memsys profile intact */
#define MR_DAMAGE_MEM_PROFILE 1u /* memsys profile = registry count; profile intact */
#define MR_DAMAGE_BOTH 2u       /* both 0xFFFFFFFF (consistent, but unregistered) */

static int mr_damaged_calls(ivs_machine *h)
{
  ivs_info info;
  ivs_load_report rep;
  ivs_cp1610_result r;
  uint8_t dig[32];
  uint8_t *cart;
  size_t cart_len = 0u;
  ivs_status st;
  memset(&info, 0, sizeof info);
  info.struct_size = (uint32_t)sizeof info;
  IVS_CHECK(ivs_machine_info(h, &info) == IVS_E_BAD_HANDLE);
  IVS_CHECK(ivs_machine_state_digest(h, IVS_DIGEST_COMPOSITE, dig) == IVS_E_BAD_HANDLE);
  IVS_CHECK(ivs_machine_state_digest(h, IVS_DIGEST_MEM, dig) == IVS_E_BAD_HANDLE);
  IVS_CHECK(ivs_machine_unload(h, IVS_KIND_EXEC) == IVS_E_BAD_HANDLE);
  IVS_CHECK(ivs_machine_reset(h, IVS_RESET_POWER_ON) == IVS_E_BAD_HANDLE);
  IVS_CHECK(ivs_machine_step_unit_internal(h, &r) == IVS_E_BAD_HANDLE);
  cart = ivs_ms_cart(0x5000u, 16u, 0u, &cart_len);
  IVS_CHECK(cart != NULL);
  memset(&rep, 0, sizeof rep);
  rep.struct_size = (uint32_t)sizeof rep;
  st = ivs_machine_load(h, IVS_KIND_CARTRIDGE, cart, cart_len, NULL, &rep);
  free(cart);
  IVS_CHECK(st == IVS_E_BAD_HANDLE && rep.reason == IVS_REASON_HANDLE);
  IVS_CHECK(ivs_machine_destroy(h) == IVS_E_BAD_HANDLE);
  return IVS_TEST_PASS;
}

static int mr_damaged_profile(uint32_t damage)
{
  ivs_ms_inst x;
  struct ivs_machine *cm;
  ivs_info info;
  uint8_t *copy = NULL;
  uint8_t *snap = NULL;
  int rc = IVS_TEST_FAIL;
  if (!ivs_ms_new(&x, 0x5Au)) {
    return IVS_TEST_FAIL;
  }
  copy = aligned_alloc(IVS_MACHINE_ALIGN, IVS_MACHINE_BYTES);
  snap = malloc(IVS_MACHINE_BYTES);
  if (copy == NULL || snap == NULL || !ivs_ms_load(x.m, IVS_KIND_EXEC, ivs_ms_exec(IVS_MS_PROG_A), IVS_MS_EXEC_BYTES) ||
      !ivs_ms_load(x.m, IVS_KIND_GROM, ivs_ms_grom(0u), IVS_MS_GROM_BYTES)) {
    goto done;
  }
  memcpy(copy, x.storage, IVS_MACHINE_BYTES);
  memset(&info, 0, sizeof info);
  info.struct_size = (uint32_t)sizeof info;
  if (ivs_machine_info((ivs_machine *)(void *)copy, &info) != IVS_OK) {
    ivs_test_diag("the relocated copy is not a valid handle before the damage");
    goto done;
  }
  cm = (struct ivs_machine *)(void *)copy;
  if (damage == MR_DAMAGE_PROFILE) {
    cm->profile = ivs_profile_count();
  } else if (damage == MR_DAMAGE_MEM_PROFILE) {
    cm->mem.profile = ivs_profile_count();
  } else {
    cm->profile = 0xFFFFFFFFu;
    cm->mem.profile = 0xFFFFFFFFu;
  }
  memcpy(snap, copy, IVS_MACHINE_BYTES);
  if (mr_damaged_calls((ivs_machine *)(void *)copy) != IVS_TEST_PASS) {
    goto done;
  }
  if (memcmp(snap, copy, IVS_MACHINE_BYTES) != 0) {
    ivs_test_diag("a refused call changed the damaged instance");
    goto done;
  }
  rc = IVS_TEST_PASS;
done:
  free(snap);
  free(copy);
  ivs_ms_free(&x);
  return rc;
}

static int mr_damaged_profile_index(void)
{
  return mr_damaged_profile(MR_DAMAGE_PROFILE);
}

static int mr_damaged_mem_profile(void)
{
  return mr_damaged_profile(MR_DAMAGE_MEM_PROFILE);
}

static int mr_damaged_both(void)
{
  return mr_damaged_profile(MR_DAMAGE_BOTH);
}

/* Plan 05-03 (D-04, D-16): a power-on reset clears the PSG storage. Two instances of content A
 * are powered; one then has the PSG registers, the pull-down masks, every PSG state member and
 * every observation row (the audio level rows and the PSGLOG rows included) dirtied, and is reset
 * again. The whole storage, byte for byte, and every digest then equal the untouched reference's
 * (the reset adds nothing and leaves nothing behind). The two pad bytes of the memory system are
 * never written by anything and stay zero, so they are not dirtied. */
static int mr_psg_state_cleared(void)
{
  ivs_ms_inst a;
  ivs_ms_inst r;
  struct ivs_machine *im;
  uint32_t d;
  uint8_t da[32];
  uint8_t dr[32];
  int rc = IVS_TEST_FAIL;
  if (!ivs_ms_powered(&r, IVS_MS_CONTENT_A, 0x5Au)) {
    return IVS_TEST_FAIL;
  }
  if (!ivs_ms_powered(&a, IVS_MS_CONTENT_A, 0x5Au)) {
    ivs_ms_free(&r);
    return IVS_TEST_FAIL;
  }
  im = ivs_machine_from_handle_mut(a.m);
  if (im == NULL) {
    goto done;
  }
  memset(im->mem.psg_reg, 0xA5, sizeof im->mem.psg_reg);
  memset(im->mem.port_pulldown, 0xA5, sizeof im->mem.port_pulldown);
  memset(&im->psg, 0xA5, sizeof im->psg);
  memset(&im->obs, 0xA5, sizeof im->obs);
  if (ivs_machine_reset(a.m, IVS_RESET_POWER_ON) != IVS_OK) {
    goto done;
  }
  if (memcmp(a.storage, r.storage, IVS_MACHINE_BYTES) != 0) {
    ivs_test_diag("storage after the reset differs from the reference");
    goto done;
  }
  for (d = 0u; d <= 19u; d++) {
    if (d > 4u && d < 16u) {
      continue;
    }
    if (ivs_machine_state_digest(a.m, d, da) != IVS_OK || ivs_machine_state_digest(r.m, d, dr) != IVS_OK ||
        memcmp(da, dr, 32u) != 0) {
      ivs_test_diag("digest of domain %u differs", (unsigned)d);
      goto done;
    }
  }
  rc = IVS_TEST_PASS;
done:
  ivs_ms_free(&a);
  ivs_ms_free(&r);
  return rc;
}

/* Plan 06-05 (D-04, D-06, D-16): a power-on reset restores the STIC storage. The profile's power-on record
 * (stic.power_on, C85) is zero registers, Foreground/Background mode and a clear display latch, so the
 * reset state of ivs_stic_state is 528 zero bytes (frame_index 0 and event_index 0: FRAME_START of frame 0
 * is still to be processed) and stic_reg is 128 zero bytes. A powered reference and a freshly created
 * instance agree on both (the twin oracle: create and reset start the same state); an instance whose
 * STIC registers and state were dirtied through the raw seam is reset again and then equals the reference
 * byte for byte over the whole storage. The dirtied state is refused by ivs_stic_state_validate and the
 * reset one is accepted. Named fault STIC_STATE_NOT_RESET_ON_POWER_ON skips the reset call. */
static int mr_stic_state(void)
{
  static const uint8_t zero[sizeof(ivs_stic_state)] = {0};
  ivs_ms_inst a;
  ivs_ms_inst r;
  ivs_ms_inst f;
  struct ivs_machine *im;
  struct ivs_machine *ref;
  struct ivs_machine *fresh;
  ivs_stic_state bad;
  int rc = IVS_TEST_FAIL;
  if (!ivs_ms_powered(&r, IVS_MS_CONTENT_A, 0x5Au)) {
    return IVS_TEST_FAIL;
  }
  if (!ivs_ms_powered(&a, IVS_MS_CONTENT_A, 0x5Au)) {
    ivs_ms_free(&r);
    return IVS_TEST_FAIL;
  }
  if (!ivs_ms_new(&f, 0x00u)) {
    ivs_ms_free(&a);
    ivs_ms_free(&r);
    return IVS_TEST_FAIL;
  }
  im = ivs_machine_from_handle_mut(a.m);
  ref = ivs_machine_from_handle_mut(r.m);
  fresh = ivs_machine_from_handle_mut(f.m);
  if (im == NULL || ref == NULL || fresh == NULL) {
    goto done;
  }
  /* The reset record, as literals. */
  if (memcmp(&ref->stic, zero, sizeof zero) != 0 || ref->stic.mode != 0u || ref->stic.display_latch != 0u ||
      ref->stic.event_index != 0u || ref->stic.frame_index != 0u) {
    ivs_test_diag("reset STIC state is not the power-on record");
    goto done;
  }
  for (uint32_t i = 0u; i < IVS_MEMSYS_STIC_REGS; i++) {
    if (ref->mem.stic_reg[i] != 0u) {
      ivs_test_diag("reset stic_reg[%u] is not 0", (unsigned)i);
      goto done;
    }
  }
  /* Create and reset start the same state (the twin oracle over the two ranges). */
  if (memcmp(&fresh->stic, &ref->stic, sizeof ref->stic) != 0 ||
      memcmp(fresh->mem.stic_reg, ref->mem.stic_reg, sizeof ref->mem.stic_reg) != 0) {
    ivs_test_diag("a fresh instance and a reset instance differ in the STIC storage");
    goto done;
  }
  if (ivs_stic_state_validate(&ref->stic) != IVS_OK || ivs_stic_state_validate(NULL) != IVS_E_BAD_ARG) {
    ivs_test_diag("validate refuses the reset state or accepts NULL");
    goto done;
  }
  /* The out-of-range members are refused one at a time. */
  for (uint32_t k = 0u; k < 6u; k++) {
    bad = ref->stic;
    switch (k) {
    case 0u:
      bad.event_index = IVS_STIC_EVENT_MAX;
      break;
    case 1u:
      bad.display_latch = 2u;
      break;
    case 2u:
      bad.windows_active = 2u;
      break;
    case 3u:
      bad.mode = 2u;
      break;
    case 4u:
      bad.cs_ptr = 4u;
      break;
    default:
      bad.reserved0 = 1u;
      break;
    }
    if (ivs_stic_state_validate(&bad) != IVS_E_BAD_STATE) {
      ivs_test_diag("validate accepts out-of-range member %u", (unsigned)k);
      goto done;
    }
  }
  /* Dirty both stores through the raw seam, then reset. */
  memset(im->mem.stic_reg, 0xA5, sizeof im->mem.stic_reg);
  memset(&im->stic, 0xA5, sizeof im->stic);
  if (ivs_stic_state_validate(&im->stic) != IVS_E_BAD_STATE) {
    ivs_test_diag("the dirtied STIC state is accepted");
    goto done;
  }
  if (ivs_machine_reset(a.m, IVS_RESET_POWER_ON) != IVS_OK) {
    goto done;
  }
  if (memcmp(&im->stic, &ref->stic, sizeof im->stic) != 0 ||
      memcmp(im->mem.stic_reg, ref->mem.stic_reg, sizeof im->mem.stic_reg) != 0) {
    ivs_test_diag("the STIC storage after the reset differs from the reference");
    goto done;
  }
  if (memcmp(a.storage, r.storage, IVS_MACHINE_BYTES) != 0) {
    ivs_test_diag("storage after the reset differs from the reference");
    goto done;
  }
  rc = IVS_TEST_PASS;
done:
  ivs_ms_free(&a);
  ivs_ms_free(&r);
  ivs_ms_free(&f);
  return rc;
}

const ivs_test_case ivs_machine_reset_cases[] = {
    {"machine.refuse.handle.mem-profile-mismatch", mr_damaged_mem_profile, NULL},
    {"machine.refuse.handle.profile-garbage", mr_damaged_both, NULL},
    {"machine.refuse.handle.profile-out-of-range", mr_damaged_profile_index, NULL},
    {"machine.reset.equivalence", mr_equivalence, NULL},
    {"machine.reset.iab-reset-vector", mr_iab_reset_vector, NULL},
    {"machine.reset.no-cart-ok", mr_no_cart_ok, NULL},
    {"machine.reset.power-cycle-one-way", mr_power_cycle_one_way, NULL},
    {"machine.reset.psg-state-cleared", mr_psg_state_cleared, NULL},
    {"machine.reset.step-program", mr_step_program, NULL},
    {"machine.reset.step-refusals", mr_step_refusals, NULL},
    {"machine.reset.stic-state", mr_stic_state, NULL},
};
const size_t ivs_machine_reset_count = sizeof ivs_machine_reset_cases / sizeof ivs_machine_reset_cases[0];

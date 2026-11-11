/* SPDX-License-Identifier: MIT */
/* Suite machine, family iso (D-07, D-36, API-01, ROADMAP success criterion 5, plan 03-12): two
 * instances with different content step without affecting each other. No threads: the gate
 * shows isolation by behaviour on one thread (D-36), next to the c-purity writable-data rule,
 * which is the structural proof that the core has no mutable static (plan 03-03).
 *
 * Content A and B (machine_standin.c) differ in every slot: EXEC program A or B, a GROM pattern
 * and a one-segment cartridge at a different address. Every instance lives in exact-size heap
 * storage. Checkpoints are the composite and CPU state digests after every 8th unit an instance
 * has stepped; a solo run of the same content, in its own storage, is the reference. */
#include <stddef.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>

#include "ivs/ivs.h"
#include "ivs_test.h"
#include "machine/machine.h"
#include "machine_cases.h"

#define MI_UNITS 64u
#define MI_EVERY 8u
#define MI_CHECKS (MI_UNITS / MI_EVERY)

/* Checkpoint digests of one run: [k] after unit (k + 1) * MI_EVERY. */
typedef struct mi_trace {
  uint8_t comp[MI_CHECKS][32];
  uint8_t cpu[MI_CHECKS][32];
} mi_trace;

/* A solo run of content from reset: MI_UNITS units with a checkpoint every MI_EVERY. */
static int mi_solo(uint32_t content, mi_trace *t)
{
  ivs_ms_inst x;
  uint32_t k;
  int ok = 1;
  if (!ivs_ms_powered(&x, content, 0x5Au)) {
    return 0;
  }
  for (k = 0u; k < MI_CHECKS && ok; k++) {
    ok = ivs_ms_step(x.m, MI_EVERY, NULL) && ivs_ms_digests(x.m, t->comp[k], t->cpu[k]);
  }
  ivs_ms_free(&x);
  return ok;
}

/* Steps n units of an instance that has stepped *done units, checking every checkpoint it
 * crosses against the reference trace. */
static int mi_chunk(const char *name, ivs_machine *m, uint32_t n, uint32_t *done, const mi_trace *ref)
{
  uint32_t i;
  for (i = 0u; i < n && *done < MI_UNITS; i++) {
    uint8_t comp[32];
    uint8_t cpu[32];
    if (!ivs_ms_step(m, 1u, NULL)) {
      return 0;
    }
    (*done)++;
    if (*done % MI_EVERY == 0u) {
      uint32_t k = *done / MI_EVERY - 1u;
      if (!ivs_ms_digests(m, comp, cpu)) {
        return 0;
      }
      if (memcmp(comp, ref->comp[k], 32u) != 0 || memcmp(cpu, ref->cpu[k], 32u) != 0) {
        ivs_test_diag("%s: checkpoint after unit %u differs from its solo run (composite %s, cpu %s)", name,
                      (unsigned)*done, memcmp(comp, ref->comp[k], 32u) != 0 ? "differs" : "equal",
                      memcmp(cpu, ref->cpu[k], 32u) != 0 ? "differs" : "equal");
        return 0;
      }
    }
  }
  return 1;
}

/* Interleaved equals sequential: A and B alone, then a fresh pair (B created first) stepped by a
 * fixed schedule of chunks 1..7 (A takes 1, 2, ..., 7, 1, ...; B takes 7, 6, ..., 1, 7, ...,
 * and B goes first). Every checkpoint of both equals its solo run. */
static int mi_interleaved(void)
{
  mi_trace *ta = malloc(sizeof *ta);
  mi_trace *tb = malloc(sizeof *tb);
  ivs_ms_inst a;
  ivs_ms_inst b;
  uint32_t da = 0u;
  uint32_t db = 0u;
  uint32_t round = 0u;
  int ok = 0;
  memset(&a, 0, sizeof a);
  memset(&b, 0, sizeof b);
  if (ta == NULL || tb == NULL || !mi_solo(IVS_MS_CONTENT_A, ta) || !mi_solo(IVS_MS_CONTENT_B, tb)) {
    goto done;
  }
  if (memcmp(ta->cpu[MI_CHECKS - 1u], tb->cpu[MI_CHECKS - 1u], 32u) == 0) {
    ivs_test_diag("contents A and B reach the same CPU state: the case would prove nothing");
    goto done;
  }
  if (!ivs_ms_powered(&b, IVS_MS_CONTENT_B, 0xA5u) || !ivs_ms_powered(&a, IVS_MS_CONTENT_A, 0xA5u)) {
    goto done;
  }
  ok = 1;
  while (ok && (da < MI_UNITS || db < MI_UNITS)) {
    ok = mi_chunk("B", b.m, 7u - round % 7u, &db, tb) && mi_chunk("A", a.m, round % 7u + 1u, &da, ta);
    round++;
  }
done:
  ivs_ms_free(&a);
  ivs_ms_free(&b);
  free(ta);
  free(tb);
  return ok ? IVS_TEST_PASS : IVS_TEST_FAIL;
}

/* Storage poisoned 0xA5 and 0x5A before create, the same content and the same steps: the two
 * instances end byte-identical (create writes every instance byte, D-02). */
static int mi_poisoned(void)
{
  ivs_ms_inst x;
  ivs_ms_inst y;
  int ok = 0;
  memset(&y, 0, sizeof y);
  if (!ivs_ms_powered(&x, IVS_MS_CONTENT_B, 0xA5u)) {
    return IVS_TEST_FAIL;
  }
  if (ivs_ms_powered(&y, IVS_MS_CONTENT_B, 0x5Au) && ivs_ms_step(x.m, 32u, NULL) && ivs_ms_step(y.m, 32u, NULL)) {
    ok = memcmp(x.storage, y.storage, IVS_MACHINE_BYTES) == 0;
    if (!ok) {
      ivs_test_diag("instances from differently poisoned storage differ after the same steps");
    }
  }
  ivs_ms_free(&y);
  ivs_ms_free(&x);
  return ok ? IVS_TEST_PASS : IVS_TEST_FAIL;
}

/* An instance copied with memcpy to new storage is a working instance there (the struct holds no
 * pointer, D-02): both copies step identically and stay byte-identical. */
static int mi_memcpy_relocation(void)
{
  ivs_ms_inst x;
  uint8_t *moved = aligned_alloc(IVS_MACHINE_ALIGN, IVS_MACHINE_BYTES);
  ivs_machine *m2;
  uint8_t d1[32];
  uint8_t d2[32];
  uint8_t c1[32];
  uint8_t c2[32];
  int ok = 0;
  if (moved == NULL) {
    return IVS_TEST_FAIL;
  }
  if (!ivs_ms_powered(&x, IVS_MS_CONTENT_A, 0xA5u)) {
    free(moved);
    return IVS_TEST_FAIL;
  }
  if (((uintptr_t)moved % 16u) != 0u) {
    ivs_test_diag("the heap gave storage below 16-byte alignment");
  } else if (ivs_ms_step(x.m, 20u, NULL)) {
    memcpy(moved, x.storage, IVS_MACHINE_BYTES);
    m2 = (ivs_machine *)(void *)moved;
    if (ivs_ms_step(x.m, 30u, NULL) && ivs_ms_step(m2, 30u, NULL) && ivs_ms_digests(x.m, d1, c1) &&
        ivs_ms_digests(m2, d2, c2)) {
      ok = memcmp(d1, d2, 32u) == 0 && memcmp(c1, c2, 32u) == 0 &&
           memcmp(x.storage, moved, IVS_MACHINE_BYTES) == 0;
      if (!ok) {
        ivs_test_diag("the relocated copy diverged from the original");
      }
    }
  }
  free(moved);
  ivs_ms_free(&x);
  return ok ? IVS_TEST_PASS : IVS_TEST_FAIL;
}

/* Destroying A halfway leaves B on its solo trace to the end. */
static int mi_destroy_mid_run(void)
{
  mi_trace *tb = malloc(sizeof *tb);
  ivs_ms_inst a;
  ivs_ms_inst b;
  uint32_t da = 0u;
  uint32_t db = 0u;
  int ok = 0;
  memset(&a, 0, sizeof a);
  memset(&b, 0, sizeof b);
  if (tb == NULL || !mi_solo(IVS_MS_CONTENT_B, tb)) {
    goto done;
  }
  if (!ivs_ms_powered(&a, IVS_MS_CONTENT_A, 0xA5u) || !ivs_ms_powered(&b, IVS_MS_CONTENT_B, 0xA5u)) {
    goto done;
  }
  ok = 1;
  while (ok && db < MI_UNITS) {
    if (a.m != NULL && da >= MI_UNITS / 2u) {
      ok = ivs_machine_destroy(a.m) == IVS_OK;
      a.m = NULL;
      if (!ok) {
        ivs_test_diag("destroy refused");
        break;
      }
    }
    if (a.m != NULL) {
      ok = ivs_ms_step(a.m, 3u, NULL);
      da += 3u;
    }
    ok = ok && mi_chunk("B", b.m, 5u, &db, tb);
  }
  if (ok && a.m != NULL) {
    ivs_test_diag("A was never destroyed");
    ok = 0;
  }
done:
  ivs_ms_free(&a);
  ivs_ms_free(&b);
  free(tb);
  return ok ? IVS_TEST_PASS : IVS_TEST_FAIL;
}

/* A successful load and refused loads (DUPLICATE_LOAD, OVERSIZED, MALFORMED) into B leave A's
 * digest and bytes unchanged, and A then steps on. */
static int mi_cross_load(void)
{
  ivs_ms_inst a;
  ivs_ms_inst b;
  uint8_t *snap = malloc(IVS_MACHINE_BYTES);
  uint8_t *exec = ivs_ms_exec(IVS_MS_PROG_B);
  uint8_t da0[32];
  uint8_t da1[32];
  uint8_t ca[32];
  int ok = 0;
  memset(&b, 0, sizeof b);
  if (snap == NULL || exec == NULL || !ivs_ms_powered(&a, IVS_MS_CONTENT_A, 0xA5u)) {
    free(snap);
    free(exec);
    return IVS_TEST_FAIL;
  }
  if (!ivs_ms_step(a.m, 12u, NULL) || !ivs_ms_digests(a.m, da0, ca) || !ivs_ms_new(&b, 0x5Au)) {
    goto done;
  }
  memcpy(snap, a.storage, IVS_MACHINE_BYTES);
  if (ivs_machine_load(b.m, IVS_KIND_EXEC, exec, IVS_MS_EXEC_BYTES, NULL, NULL) != IVS_OK ||
      ivs_machine_load(b.m, IVS_KIND_EXEC, exec, IVS_MS_EXEC_BYTES, NULL, NULL) != IVS_E_DUPLICATE_LOAD ||
      ivs_machine_load(b.m, IVS_KIND_GROM, exec, IVS_MS_EXEC_BYTES, NULL, NULL) != IVS_E_OVERSIZED ||
      ivs_machine_load(b.m, IVS_KIND_CARTRIDGE, exec, 66u, NULL, NULL) != IVS_E_MALFORMED) {
    ivs_test_diag("B: the load outcomes were not OK, DUPLICATE_LOAD, OVERSIZED, MALFORMED");
    goto done;
  }
  if (ivs_machine_state_digest(a.m, IVS_DIGEST_COMPOSITE, da1) != IVS_OK || memcmp(da0, da1, 32u) != 0 ||
      memcmp(snap, a.storage, IVS_MACHINE_BYTES) != 0) {
    ivs_test_diag("loads into B changed A");
    goto done;
  }
  ok = ivs_ms_step(a.m, 4u, NULL);
done:
  ivs_ms_free(&b);
  ivs_ms_free(&a);
  free(exec);
  free(snap);
  return ok ? IVS_TEST_PASS : IVS_TEST_FAIL;
}

const ivs_test_case ivs_machine_iso_cases[] = {
    {"machine.iso.cross-load", mi_cross_load, NULL},
    {"machine.iso.destroy-mid-run", mi_destroy_mid_run, NULL},
    {"machine.iso.interleaved-equals-sequential", mi_interleaved, NULL},
    {"machine.iso.memcpy-relocation", mi_memcpy_relocation, NULL},
    {"machine.iso.poisoned-storage-identical", mi_poisoned, NULL},
};
const size_t ivs_machine_iso_count = sizeof ivs_machine_iso_cases / sizeof ivs_machine_iso_cases[0];

/* SPDX-License-Identifier: MIT */
/* Suite machine, family sweep (D-12, D-34, plan 03-12): oracle-free cartridge sweeps. This is a
 * robustness check, not an accuracy check: it asserts no expected status or reason, only that a
 * mutated container never leaves partial state.
 *
 * Bases are the positive IVSC fixtures of at most 256 bytes whose segments lie in the profile's
 * cartridge span (the generated fixture table of data/cart/, plan 03-07). One case per base,
 * machine.sweep.<fixture id without "ivsc.">; machine.sweep.bases-complete checks that the list
 * below names exactly those fixtures. For each base:
 *   - every truncation length 0..len (len itself is the unmutated base, which must load OK);
 *   - every single-byte change to 0x00 and to 0xFF (a byte that already holds the value is no
 *     change and is skipped);
 * each loaded as a cartridge into an instance whose bytes equal a fresh instance's. An accepted
 * load must unload back to the fresh composite digest (and the fresh bytes); a refusal must be an
 * IVSC refusal status (MALFORMED, OVERSIZED, OVERLAP, OUT_OF_RANGE, UNSUPPORTED or
 * DIGEST_MISMATCH) with the instance byte-identical (memcmp only, no digest, for speed). One
 * instance serves a whole base: it is equal to a fresh instance before every load, which the two
 * checks above establish, so reusing it is the same as loading into a new one. Every input is a
 * heap buffer of exactly its length, so ASan sees any read past it. */
#include <stddef.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>

#include "content/ivsc.h"
#include "ivs/ivs.h"
#include "ivs_test.h"
#include "machine/machine.h"
#include "machine_cases.h"

#include "../ivsc/ivsc_table.h"

#include "ivs_cart_table.inc"

#define MS_MAX_BASE_BYTES 256u

/* The sweep bases (fixture ids), in case-table order. */
static const char *const ms_bases[] = {
    "ivsc.B01-segs-16", "ivsc.P01-min", "ivsc.P02-two-segments", "ivsc.P03-word-patterns", "ivsc.P04-adjacent",
    "ivsc.P06-partial",
};
#define MS_N_BASES (sizeof ms_bases / sizeof ms_bases[0])

static int ms_is_base(const ivs_cart_fixture *f)
{
  return f->kind == IVS_CART_POSITIVE && f->bytes != NULL && f->n_bytes <= MS_MAX_BASE_BYTES && f->in_span != 0u &&
         f->status == IVS_OK;
}

static const ivs_cart_fixture *ms_fixture(const char *id)
{
  size_t i;
  for (i = 0u; i < ivs_cart_count; i++) {
    if (strcmp(ivs_cart_table[i].id, id) == 0) {
      return &ivs_cart_table[i];
    }
  }
  return NULL;
}

static int ms_ivsc_refusal(ivs_status st)
{
  return st == IVS_E_MALFORMED || st == IVS_E_OVERSIZED || st == IVS_E_OVERLAP || st == IVS_E_OUT_OF_RANGE ||
         st == IVS_E_UNSUPPORTED || st == IVS_E_DIGEST_MISMATCH;
}

typedef struct ms_run {
  ivs_ms_inst x;
  uint8_t *fresh;    /* the fresh instance's bytes */
  uint8_t d0[32];    /* the fresh composite digest */
  uint32_t loads;
  uint32_t accepted;
} ms_run;

/* One load of len bytes copied from src into an exact-size heap buffer. */
static int ms_one(ms_run *r, const uint8_t *src, size_t len, const char *what, size_t at)
{
  uint8_t *b = malloc(len != 0u ? len : 1u);
  ivs_status st;
  int ok = 1;
  if (b == NULL) {
    return 0;
  }
  memcpy(b, src, len);
  st = ivs_machine_load(r->x.m, IVS_KIND_CARTRIDGE, b, len, NULL, NULL);
  free(b);
  r->loads++;
  if (st == IVS_OK) {
    uint8_t d1[32];
    r->accepted++;
    if (ivs_machine_unload(r->x.m, IVS_KIND_CARTRIDGE) != IVS_OK ||
        ivs_machine_state_digest(r->x.m, IVS_DIGEST_COMPOSITE, d1) != IVS_OK || memcmp(d1, r->d0, 32u) != 0 ||
        memcmp(r->x.storage, r->fresh, IVS_MACHINE_BYTES) != 0) {
      ivs_test_diag("%s %zu: accepted, but unload did not return the fresh state", what, at);
      ok = 0;
    }
  } else if (!ms_ivsc_refusal(st)) {
    ivs_test_diag("%s %zu: %s is not an IVSC refusal status", what, at, ivs_status_name(st));
    ok = 0;
  } else if (memcmp(r->x.storage, r->fresh, IVS_MACHINE_BYTES) != 0) {
    ivs_test_diag("%s %zu: refused (%s) with the instance changed", what, at, ivs_status_name(st));
    ok = 0;
  }
  return ok;
}

static int ms_sweep(size_t base)
{
  const ivs_cart_fixture *f = ms_fixture(ms_bases[base]);
  ms_run r;
  uint8_t *mut = NULL;
  size_t len;
  size_t i;
  int ok = 0;
  memset(&r, 0, sizeof r);
  if (f == NULL || !ms_is_base(f)) {
    ivs_test_diag("%s is not a sweep base of the fixture table", ms_bases[base]);
    return IVS_TEST_FAIL;
  }
  len = f->n_bytes;
  r.fresh = malloc(IVS_MACHINE_BYTES);
  mut = malloc(len);
  if (r.fresh == NULL || mut == NULL || !ivs_ms_new(&r.x, 0xA5u)) {
    goto done;
  }
  memcpy(r.fresh, r.x.storage, IVS_MACHINE_BYTES);
  if (ivs_machine_state_digest(r.x.m, IVS_DIGEST_COMPOSITE, r.d0) != IVS_OK) {
    goto done;
  }
  /* The unmutated base must load: a sweep of a base the core refuses would be vacuous. */
  if (ivs_machine_load(r.x.m, IVS_KIND_CARTRIDGE, f->bytes, len, NULL, NULL) != IVS_OK ||
      ivs_machine_unload(r.x.m, IVS_KIND_CARTRIDGE) != IVS_OK) {
    ivs_test_diag("the unmutated base was refused");
    goto done;
  }
  ok = 1;
  for (i = 0u; i <= len && ok; i++) {
    ok = ms_one(&r, f->bytes, i, "truncated to", i);
  }
  for (i = 0u; i < len && ok; i++) {
    static const uint8_t values[2] = {0x00u, 0xFFu};
    size_t v;
    for (v = 0u; v < 2u && ok; v++) {
      if (f->bytes[i] == values[v]) {
        continue;
      }
      memcpy(mut, f->bytes, len);
      mut[i] = values[v];
      ok = ms_one(&r, mut, len, values[v] == 0u ? "byte to 0x00 at" : "byte to 0xFF at", i);
    }
  }
  if (ok && r.accepted == 0u) {
    ivs_test_diag("no sweep load was accepted (the full-length truncation is the base)");
    ok = 0;
  }
  if (ok) {
    ivs_test_diag("%s: %u loads, %u accepted", ms_bases[base], (unsigned)r.loads, (unsigned)r.accepted);
  }
done:
  free(mut);
  free(r.fresh);
  ivs_ms_free(&r.x);
  return ok ? IVS_TEST_PASS : IVS_TEST_FAIL;
}

static int ms_b01(void)
{
  return ms_sweep(0u);
}

static int ms_p01(void)
{
  return ms_sweep(1u);
}

static int ms_p02(void)
{
  return ms_sweep(2u);
}

static int ms_p03(void)
{
  return ms_sweep(3u);
}

static int ms_p04(void)
{
  return ms_sweep(4u);
}

static int ms_p06(void)
{
  return ms_sweep(5u);
}

/* The base list names exactly the sweep bases of the fixture table. */
static int ms_bases_complete(void)
{
  size_t i;
  size_t n = 0u;
  for (i = 0u; i < ivs_cart_count; i++) {
    if (ms_is_base(&ivs_cart_table[i])) {
      size_t k;
      int listed = 0;
      n++;
      for (k = 0u; k < MS_N_BASES; k++) {
        listed |= strcmp(ms_bases[k], ivs_cart_table[i].id) == 0;
      }
      if (!listed) {
        ivs_test_diag("%s is a sweep base but has no sweep case", ivs_cart_table[i].id);
        return IVS_TEST_FAIL;
      }
    }
  }
  if (n != MS_N_BASES) {
    ivs_test_diag("%zu sweep bases in the table, %zu listed", n, (size_t)MS_N_BASES);
    return IVS_TEST_FAIL;
  }
  return IVS_TEST_PASS;
}

const ivs_test_case ivs_machine_sweep_cases[] = {
    {"machine.sweep.B01-segs-16", ms_b01, NULL},
    {"machine.sweep.P01-min", ms_p01, NULL},
    {"machine.sweep.P02-two-segments", ms_p02, NULL},
    {"machine.sweep.P03-word-patterns", ms_p03, NULL},
    {"machine.sweep.P04-adjacent", ms_p04, NULL},
    {"machine.sweep.P06-partial", ms_p06, NULL},
    {"machine.sweep.bases-complete", ms_bases_complete, NULL},
};
const size_t ivs_machine_sweep_count = sizeof ivs_machine_sweep_cases / sizeof ivs_machine_sweep_cases[0];

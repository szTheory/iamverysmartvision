/* SPDX-License-Identifier: MIT */
/* Suite state, family save (plan 08-11, D-12, D-13, D-14, D-16): the size of a snapshot and the refusal ladder of the two public calls
 * ivs_machine_snapshot_size and ivs_machine_snapshot_save (ivs/ivs_state.h). Expected values come from the literals of plan 08-09
 * (data/expected/ivsn-literals.ids) and from the contract text of the header, never from the writer: the size is checked against the
 * 4,299 bytes derived on paper, each refusal against the exact status the header documents, and every refusal against the effects the
 * api-effects table claims for it (the whole instance byte-identical, the output parameter and the buffer untouched). Task 2 of the plan
 * adds state.size.derived-and-constant and state.save.refusal-ladder; the rights, canonical-stream and read-only cases of Task 3 follow
 * in this file. */
#include <stdint.h>
#include <stdlib.h>
#include <string.h>

#include "common/ivs_sha256.h"
#include "ivs/ivs.h"
#include "machine/machine.h"
#include "state_cases.h"

/* A powered instance with the EXEC and GROM images only (no cartridge): the smallest content a power-on reset accepts. */
static int st_power_no_cart(ivs_ms_inst *x)
{
  if (!ivs_ms_new(x, 0x5Au)) {
    return 0;
  }
  if (!ivs_ms_load(x->m, IVS_KIND_EXEC, ivs_ms_exec(IVS_MS_PROG_A), IVS_MS_EXEC_BYTES) ||
      !ivs_ms_load(x->m, IVS_KIND_GROM, ivs_ms_grom(0u), IVS_MS_GROM_BYTES) ||
      ivs_machine_reset(x->m, IVS_RESET_POWER_ON) != IVS_OK) {
    ivs_ms_free(x);
    return 0;
  }
  return 1;
}

/* The size of one powered instance after units units (content A or B, or content A without its cartridge when content is 2):
 * ivs_machine_snapshot_size equals the total-bytes literal; save writes exactly that many bytes; and the seven state-only section lengths
 * of the machine seam equal the section-bytes literals and sum to sections-total-bytes, with header and trailer to total-bytes. */
static int st_size_state(uint32_t content, uint32_t units, uint64_t *seen)
{
  ivs_ms_inst x;
  uint8_t snap[IVS_SNAPSHOT_MAX_BYTES];
  uint32_t total_lit = 0u;
  uint32_t sections_lit = 0u;
  uint32_t header_lit = 0u;
  uint32_t trailer_lit = 0u;
  uint64_t size = 0u;
  uint64_t w = 0u;
  uint64_t sum = 0u;
  uint32_t i;
  int ok = 1;
  ST_REQ(ok, ivsn_lit_u32("total-bytes", &total_lit));
  ST_REQ(ok, ivsn_lit_u32("sections-total-bytes", &sections_lit));
  ST_REQ(ok, ivsn_lit_u32("header-bytes", &header_lit));
  ST_REQ(ok, ivsn_lit_u32("trailer-bytes", &trailer_lit));
  if (!ok) {
    return 0;
  }
  if (content == 2u ? !st_power_no_cart(&x) : !ivs_ms_powered(&x, content, 0x5Au)) {
    return 0;
  }
  if (units > 0u) {
    ST_REQ(ok, ivs_ms_step(x.m, units, NULL));
  }
  ST_REQ(ok, ivs_machine_snapshot_size(x.m, &size) == IVS_OK);
  ST_REQ(ok, size == total_lit);
  ST_REQ(ok, ivs_machine_snapshot_save(x.m, snap, sizeof snap, &w) == IVS_OK);
  ST_REQ(ok, w == size);
  for (i = 0u; i < ST_SECTIONS; i++) {
    char key[48];
    uint32_t lit = 0u;
    size_t len = ivs_machine_state_preimage(x.m, st_section_domain[i], NULL, 0u);
    (void)snprintf(key, sizeof key, "section-bytes.%s", st_section_name[i]);
    ST_REQ(ok, ivsn_lit_u32(key, &lit));
    if (len != lit) {
      ivs_test_diag("section %s: the state-only walk is %u bytes, the literal %u", st_section_name[i], (unsigned)len, (unsigned)lit);
    }
    ST_REQ(ok, len == lit);
    sum += len;
  }
  ST_REQ(ok, sum == sections_lit);
  ST_REQ(ok, (uint64_t)header_lit + sum + trailer_lit == w);
  *seen = size;
  ivs_ms_free(&x);
  return ok;
}

/* state.size.derived-and-constant (Task 2, D-13, D-16): the size of a snapshot is the 4,299 bytes derived on paper (612 + 3,655 + 32), the
 * same for a reset instance, one stepped 100 units, one cut at an arbitrary mid-run point, one run past several frames, a second
 * content and an instance without a cartridge; the literal file is consistent with itself (the sum rule and the section offsets chain
 * from the header size to the trailer); and a save writes as many bytes as the size call says. The sizes come from the literals, so a
 * walk that writes a different number of bytes for any section (a reserved word serialized, an image row admitted) fails here. */
static int st_size_derived(void)
{
  static const uint32_t units[4] = {0u, 100u, 3333u, 9999u};
  uint32_t total_lit = 0u;
  uint32_t header_lit = 0u;
  uint32_t trailer_lit = 0u;
  uint32_t sections_lit = 0u;
  uint32_t off = 0u;
  uint32_t sum = 0u;
  uint32_t i;
  uint64_t seen = 0u;
  int ok = 1;
  ST_CHECK(ivsn_lit_u32("total-bytes", &total_lit));
  ST_CHECK(ivsn_lit_u32("header-bytes", &header_lit));
  ST_CHECK(ivsn_lit_u32("trailer-bytes", &trailer_lit));
  ST_CHECK(ivsn_lit_u32("sections-total-bytes", &sections_lit));
  /* the literal file agrees with itself: offsets chain from the header to the trailer, the sizes sum, 4,299 is the figure */
  off = header_lit;
  for (i = 0u; i < ST_SECTIONS; i++) {
    uint32_t o = 0u;
    uint32_t b = 0u;
    char key[48];
    (void)snprintf(key, sizeof key, "section-offset.%s", st_section_name[i]);
    ST_CHECK(ivsn_lit_u32(key, &o));
    (void)snprintf(key, sizeof key, "section-bytes.%s", st_section_name[i]);
    ST_CHECK(ivsn_lit_u32(key, &b));
    ST_CHECK(o == off);
    off += b;
    sum += b;
  }
  ST_CHECK(sum == sections_lit);
  ST_CHECK(header_lit + sections_lit + trailer_lit == total_lit);
  ST_CHECK(off == total_lit - trailer_lit);
  {
    uint32_t t = 0u;
    ST_CHECK(ivsn_lit_u32("trailer-offset", &t));
    ST_CHECK(t == off);
  }
  for (i = 0u; i < 4u; i++) {
    ST_REQ(ok, st_size_state(IVS_MS_CONTENT_A, units[i], &seen));
    ST_REQ(ok, seen == total_lit);
  }
  ST_REQ(ok, st_size_state(IVS_MS_CONTENT_B, 777u, &seen));
  ST_REQ(ok, seen == total_lit);
  ST_REQ(ok, st_size_state(2u, 500u, &seen));
  ST_REQ(ok, seen == total_lit);
  return ok ? IVS_TEST_PASS : IVS_TEST_FAIL;
}

/* ---- the refusal ladder ---- */

/* An instance with room on both sides in host memory, so that buffers can lie just before it, just after it and across its edges. */
#define ST_GAP 8192u

typedef struct st_arena {
  uint8_t *base;
  size_t bytes;
  uint8_t *inst;  /* the storage of the instance: base + ST_GAP, IVS_MACHINE_BYTES long */
  uint8_t *ref;   /* a copy of the instance bytes, to prove that no call changed them */
  ivs_machine *m;
} st_arena;

static void st_arena_free(st_arena *a)
{
  free(a->base);
  free(a->ref);
  memset(a, 0, sizeof *a);
}

/* power: 1 powers the instance on, 0 leaves it OFF with the images loaded. 1 on success. */
static int st_arena_new(st_arena *a, int power)
{
  ivs_create_options o;
  memset(a, 0, sizeof *a);
  a->bytes = (size_t)ST_GAP + IVS_MACHINE_BYTES + ST_GAP;
  a->base = (uint8_t *)aligned_alloc(IVS_MACHINE_ALIGN, a->bytes);
  a->ref = (uint8_t *)malloc(IVS_MACHINE_BYTES);
  if (a->base == NULL || a->ref == NULL) {
    st_arena_free(a);
    return 0;
  }
  memset(a->base, 0x5A, a->bytes);
  a->inst = a->base + ST_GAP;
  memset(&o, 0, sizeof o);
  o.struct_size = (uint32_t)sizeof o;
  o.profile_id = IVS_MS_PROFILE;
  if (ivs_machine_create(&o, a->inst, IVS_MACHINE_BYTES, &a->m) != IVS_OK || !ivs_ms_content(a->m, IVS_MS_CONTENT_A) ||
      (power && ivs_machine_reset(a->m, IVS_RESET_POWER_ON) != IVS_OK)) {
    st_arena_free(a);
    return 0;
  }
  if (power && !ivs_ms_step(a->m, 250u, NULL)) {
    st_arena_free(a);
    return 0;
  }
  memcpy(a->ref, a->inst, IVS_MACHINE_BYTES);
  return 1;
}

/* One row of the size call: the status, that the instance bytes are as they were and that *out is (or is not) changed to expect. */
static int st_size_row(const char *label, const st_arena *a, const ivs_machine *h, uint64_t *out, ivs_status want, int out_set,
                       uint64_t out_expect)
{
  uint64_t before = out != NULL ? *out : 0u;
  ivs_status st = ivs_machine_snapshot_size(h, out);
  int ok = st == want;
  if (out != NULL) {
    ok = ok && (out_set ? *out == out_expect : *out == before);
  }
  ok = ok && memcmp(a->inst, a->ref, IVS_MACHINE_BYTES) == 0;
  if (!ok) {
    ivs_test_diag("size row '%s': status %s (want %s)", label, ivs_status_name(st), ivs_status_name(want));
  }
  return ok;
}

/* One row of the save call. guard is the number of bytes at buf that must be as they were afterwards (0 when buf has no readable extent
 * of that size); the instance bytes of a must be as they were; *out is expected to hold out_expect afterwards when out_set, else its old
 * value. */
static int st_save_row(const char *label, const st_arena *a, const ivs_machine *h, uint8_t *buf, uint64_t cap, uint64_t *out,
                       size_t guard, ivs_status want, int out_set, uint64_t out_expect)
{
  uint64_t before = out != NULL ? *out : 0u;
  uint8_t *keep = NULL;
  ivs_status st;
  int ok;
  if (guard > 0u) {
    keep = (uint8_t *)malloc(guard);
    if (keep == NULL) {
      return 0;
    }
    memcpy(keep, buf, guard);
  }
  st = ivs_machine_snapshot_save(h, buf, cap, out);
  ok = st == want;
  if (out != NULL) {
    ok = ok && (out_set ? *out == out_expect : *out == before);
  }
  ok = ok && memcmp(a->inst, a->ref, IVS_MACHINE_BYTES) == 0;
  if (keep != NULL) {
    ok = ok && memcmp(keep, buf, guard) == 0;
    free(keep);
  }
  if (!ok) {
    ivs_test_diag("save row '%s': status %s (want %s)", label, ivs_status_name(st), ivs_status_name(want));
  }
  return ok;
}

/* The rows of ivs_machine_snapshot_size. */
static int st_size_ladder(const st_arena *a, const st_arena *off, uint64_t total, const ivs_machine *destroyed)
{
  const uint64_t seed = 0x1122334455667788ull;
  uint64_t out = seed;
  uint64_t *inside = (uint64_t *)(void *)(a->inst + 64u);
  int ok = 1;
  ST_REQ(ok, st_size_row("out NULL", a, a->m, NULL, IVS_E_BAD_ARG, 0, 0u));
  ST_REQ(ok, st_size_row("NULL handle", a, NULL, &out, IVS_E_BAD_HANDLE, 0, 0u));
  ST_REQ(ok, st_size_row("destroyed handle", a, destroyed, &out, IVS_E_BAD_HANDLE, 0, 0u));
  ST_REQ(ok, st_size_row("out inside the instance", a, a->m, inside, IVS_E_BAD_ARG, 0, 0u));
  ST_REQ(ok, st_size_row("OFF", off, off->m, &out, IVS_E_BAD_STATE, 0, 0u));
  /* the order: arguments before the handle, the overlap before the phase */
  ST_REQ(ok, st_size_row("NULL handle with out NULL", a, NULL, NULL, IVS_E_BAD_ARG, 0, 0u));
  ST_REQ(ok, st_size_row("OFF with out inside the instance", off, off->m, (uint64_t *)(void *)(off->inst + 64u), IVS_E_BAD_ARG, 0, 0u));
  /* the answer */
  ST_REQ(ok, st_size_row("powered", a, a->m, &out, IVS_OK, 1, total));
  return ok;
}

/* The rows of ivs_machine_snapshot_save. */
static int st_save_ladder(const st_arena *a, const st_arena *off, uint64_t total, const ivs_machine *destroyed)
{
  const uint64_t seed = 0x1122334455667788ull;
  const size_t n = (size_t)total;
  uint64_t out = seed;
  uint8_t *buf = (uint8_t *)aligned_alloc(16u, 16384u + 64u);
  uint8_t *ref = (uint8_t *)malloc(16384u + 64u);
  uint8_t *twin = (uint8_t *)malloc(16384u + 64u);
  uint64_t *inside = (uint64_t *)(void *)(a->inst + 64u);
  int ok = 1;
  if (buf == NULL || ref == NULL || twin == NULL) {
    free(buf);
    free(ref);
    free(twin);
    return 0;
  }
  memset(buf, 0xC3, 16384u + 64u);

  /* argument pointers: BAD_ARG, before the handle */
  ST_REQ(ok, st_save_row("out_written NULL", a, a->m, buf, 4299u, NULL, 4299u, IVS_E_BAD_ARG, 0, 0u));
  ST_REQ(ok, st_save_row("buf NULL with cap", a, a->m, NULL, 10u, &out, 0u, IVS_E_BAD_ARG, 0, 0u));
  ST_REQ(ok, st_save_row("NULL handle and buf NULL with cap", a, NULL, NULL, 10u, &out, 0u, IVS_E_BAD_ARG, 0, 0u));
  ST_REQ(ok, st_save_row("out_written inside the buffer", a, a->m, buf, 4304u, (uint64_t *)(void *)(buf + 8u), 4304u, IVS_E_BAD_ARG, 0, 0u));
  ST_REQ(ok, st_save_row("out_written at the buffer start", a, a->m, buf, 4304u, (uint64_t *)(void *)buf, 4304u, IVS_E_BAD_ARG, 0, 0u));
  /* the handle */
  ST_REQ(ok, st_save_row("NULL handle", a, NULL, buf, 4299u, &out, 4299u, IVS_E_BAD_HANDLE, 0, 0u));
  ST_REQ(ok, st_save_row("destroyed handle", a, destroyed, buf, 4299u, &out, 4299u, IVS_E_BAD_HANDLE, 0, 0u));
  /* outputs and buffers against the instance: BAD_ARG, nothing written into the instance */
  ST_REQ(ok, st_save_row("out_written inside the instance", a, a->m, buf, 4299u, inside, 4299u, IVS_E_BAD_ARG, 0, 0u));
  ST_REQ(ok, st_save_row("buffer is the instance", a, a->m, a->inst, 4299u, &out, 0u, IVS_E_BAD_ARG, 0, 0u));
  ST_REQ(ok, st_save_row("buffer starts inside the instance", a, a->m, a->inst + 1000u, 4299u, &out, 0u, IVS_E_BAD_ARG, 0, 0u));
  ST_REQ(ok, st_save_row("buffer holds the last instance byte", a, a->m, a->inst + IVS_MACHINE_BYTES - 1u, 2u, &out, 0u, IVS_E_BAD_ARG, 0, 0u));
  ST_REQ(ok, st_save_row("buffer reaches into the instance", a, a->m, a->inst - 100u, 101u, &out, 100u, IVS_E_BAD_ARG, 0, 0u));
  ST_REQ(ok, st_save_row("buffer reaches into the instance, cap 4300", a, a->m, a->inst - 4299u, 4300u, &out, 4299u, IVS_E_BAD_ARG, 0, 0u));
  ST_REQ(ok, st_save_row("capacity runs past the address space", a, a->m, buf, UINT64_MAX, &out, 0u, IVS_E_BAD_ARG, 0, 0u));
  /* the phase: OFF, after the overlap rules and before the capacity */
  ST_REQ(ok, st_save_row("OFF", off, off->m, buf, 4299u, &out, 4299u, IVS_E_BAD_STATE, 0, 0u));
  ST_REQ(ok, st_save_row("OFF with a small capacity", off, off->m, buf, 1u, &out, 1u, IVS_E_BAD_STATE, 0, 0u));
  ST_REQ(ok, st_save_row("OFF with cap 0", off, off->m, NULL, 0u, &out, 0u, IVS_E_BAD_STATE, 0, 0u));
  ST_REQ(ok, st_save_row("OFF with out_written inside the instance", off, off->m, buf, 4299u, (uint64_t *)(void *)(off->inst + 64u), 4299u,
                         IVS_E_BAD_ARG, 0, 0u));
  /* the capacity: BUFFER_TOO_SMALL reports the exact size and never writes the buffer */
  ST_REQ(ok, st_save_row("cap one below the size", a, a->m, buf, total - 1u, &out, n, IVS_E_BUFFER_TOO_SMALL, 1, total));
  out = seed;
  ST_REQ(ok, st_save_row("cap 1", a, a->m, buf, 1u, &out, 16u, IVS_E_BUFFER_TOO_SMALL, 1, total));
  out = seed;
  ST_REQ(ok, st_save_row("cap 0 with a buffer", a, a->m, buf, 0u, &out, 16u, IVS_E_BUFFER_TOO_SMALL, 1, total));
  out = seed;
  ST_REQ(ok, st_save_row("cap 0 with no buffer", a, a->m, NULL, 0u, &out, 0u, IVS_E_BUFFER_TOO_SMALL, 1, total));
  /* a buffer that ends exactly where the instance begins does not overlap it: the capacity decides */
  out = seed;
  ST_REQ(ok, st_save_row("buffer ends at the instance, cap below the size", a, a->m, a->inst - 100u, 100u, &out, 100u,
                         IVS_E_BUFFER_TOO_SMALL, 1, total));

  /* success: exactly the size, no byte past it, no instance change, the same bytes every time and in every place */
  memset(buf, 0xC3, 16384u + 64u);
  out = seed;
  ST_REQ(ok, ivs_machine_snapshot_save(a->m, buf, total, &out) == IVS_OK);
  ST_REQ(ok, out == total);
  ST_REQ(ok, memcmp(a->inst, a->ref, IVS_MACHINE_BYTES) == 0);
  {
    size_t i;
    int canary = 1;
    for (i = n; i < 16384u + 64u; i++) {
      canary = canary && buf[i] == 0xC3u;
    }
    ST_REQ(ok, canary);
  }
  memcpy(ref, buf, n);
  memset(twin, 0x3C, 16384u + 64u);
  out = seed;
  ST_REQ(ok, ivs_machine_snapshot_save(a->m, twin, 16384u, &out) == IVS_OK);
  ST_REQ(ok, out == total);
  ST_REQ(ok, memcmp(twin, ref, n) == 0);
  {
    size_t i;
    int canary = 1;
    for (i = n; i < 16384u + 64u; i++) {
      canary = canary && twin[i] == 0x3Cu;
    }
    ST_REQ(ok, canary);
  }
  /* an output just past the buffer is outside it; a buffer ending where the instance begins, and one beginning where it ends, are
   * outside the instance */
  memset(buf, 0xC3, 16384u + 64u);
  out = seed;
  ST_REQ(ok, ivs_machine_snapshot_save(a->m, buf, 4304u, (uint64_t *)(void *)(buf + 4304u)) == IVS_OK);
  ST_REQ(ok, *(uint64_t *)(void *)(buf + 4304u) == total);
  ST_REQ(ok, memcmp(buf, ref, n) == 0);
  memset(a->inst - 4299u, 0xC3, 4299u);
  ST_REQ(ok, ivs_machine_snapshot_save(a->m, a->inst - 4299u, 4299u, &out) == IVS_OK);
  ST_REQ(ok, out == total);
  ST_REQ(ok, memcmp(a->inst - 4299u, ref, n) == 0);
  ST_REQ(ok, memcmp(a->inst, a->ref, IVS_MACHINE_BYTES) == 0);
  memset(a->inst + IVS_MACHINE_BYTES, 0xC3, 4299u);
  out = seed;
  ST_REQ(ok, ivs_machine_snapshot_save(a->m, a->inst + IVS_MACHINE_BYTES, 4299u, &out) == IVS_OK);
  ST_REQ(ok, out == total);
  ST_REQ(ok, memcmp(a->inst + IVS_MACHINE_BYTES, ref, n) == 0);
  ST_REQ(ok, memcmp(a->inst, a->ref, IVS_MACHINE_BYTES) == 0);
  free(buf);
  free(ref);
  free(twin);
  return ok;
}

/* state.save.refusal-ladder (Task 2, D-14, D-16): every refusal row of the two calls with the exact status, the exact out effect (the
 * required size or untouched) and, for each, the whole-instance memcmp and the buffer bytes unchanged. The order of the header text is
 * held: arguments, handle, overlap, OFF, capacity. */
static int st_refusal_ladder(void)
{
  st_arena a;
  st_arena off;
  st_arena dead;
  uint32_t total_lit = 0u;
  int ok = 1;
  ST_CHECK(ivsn_lit_u32("total-bytes", &total_lit));
  ST_CHECK(st_arena_new(&a, 1));
  if (!st_arena_new(&off, 0)) {
    st_arena_free(&a);
    return IVS_TEST_FAIL;
  }
  if (!st_arena_new(&dead, 1)) {
    st_arena_free(&a);
    st_arena_free(&off);
    return IVS_TEST_FAIL;
  }
  /* the handle of a destroyed instance: its storage is scrubbed, so the handle is invalid from then on */
  ST_REQ(ok, ivs_machine_destroy(dead.m) == IVS_OK);
  memcpy(dead.ref, dead.inst, IVS_MACHINE_BYTES);
  ST_REQ(ok, st_size_ladder(&a, &off, total_lit, dead.m));
  ST_REQ(ok, st_save_ladder(&a, &off, total_lit, dead.m));
  st_arena_free(&a);
  st_arena_free(&off);
  st_arena_free(&dead);
  return ok ? IVS_TEST_PASS : IVS_TEST_FAIL;
}

const ivs_test_case ivs_state_save_cases[] = {
    {"state.size.derived-and-constant", st_size_derived, NULL},
    {"state.save.refusal-ladder", st_refusal_ladder, NULL},
};
const size_t ivs_state_save_count = sizeof ivs_state_save_cases / sizeof ivs_state_save_cases[0];

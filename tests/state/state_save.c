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

/* ---- the rights property of the container (Task 3, D-19, T-08-27) ----
 * A recognisable image: word i of image k (0 EXEC, 1 GROM, 2 cartridge) of pattern set s is i * mul[s] + add[s] * (k + 1) modulo 2^16.
 * The multiplier is odd, so the word value names its index (a bijection of the 16-bit values) and no run of eight consecutive words is
 * all zero or repeats one value: a 16-byte window of an image is found in a byte stream by reading its first word, inverting it to an
 * index and comparing the other seven words, in time linear in the stream. The serialized form of a row is big-endian (D-12), so that
 * is the form searched for. Two sets, so that two instances differ in every word of every image. */
static const uint32_t st_pat_mul[2] = {0x9E37u, 0x7A5Bu};
static const uint32_t st_pat_add[2] = {0x1357u, 0x2B6Du};

static uint16_t st_pat(uint32_t set, uint32_t k, uint32_t i)
{
  return (uint16_t)(i * st_pat_mul[set] + st_pat_add[set] * (k + 1u));
}

static uint32_t st_pat_inverse(uint32_t set)
{
  uint32_t x = 1u;
  while (((x * st_pat_mul[set]) & 0xFFFFu) != 1u) {
    x += 2u;
  }
  return x;
}

/* The words of image k of an instance. */
static uint32_t st_image_words(uint32_t k)
{
  return k == 0u ? IVS_MEMSYS_EXEC_WORDS : (k == 1u ? IVS_MEMSYS_GROM_WORDS : IVS_MEMSYS_CART_WORDS);
}

/* Overwrites every word of the three image arrays of m with pattern set set (the slot records are left as they are). */
static void st_fill_images(struct ivs_machine *m, uint32_t set)
{
  uint32_t i;
  for (i = 0u; i < IVS_MEMSYS_EXEC_WORDS; i++) {
    m->mem.exec[i] = st_pat(set, 0u, i);
  }
  for (i = 0u; i < IVS_MEMSYS_GROM_WORDS; i++) {
    m->mem.grom[i] = st_pat(set, 1u, i);
  }
  for (i = 0u; i < IVS_MEMSYS_CART_WORDS; i++) {
    m->mem.cart[i] = st_pat(set, 2u, i);
  }
}

/* 1 when the n bytes at s contain eight consecutive big-endian words of one of the three images of pattern set set. */
static int st_has_window(const uint8_t *s, size_t n, uint32_t set)
{
  uint32_t inv = st_pat_inverse(set);
  size_t p;
  uint32_t k;
  uint32_t j;
  if (n < 16u) {
    return 0;
  }
  for (p = 0u; p + 16u <= n; p++) {
    uint32_t v = ((uint32_t)s[p] << 8) | (uint32_t)s[p + 1u];
    for (k = 0u; k < 3u; k++) {
      uint32_t i = (((v - st_pat_add[set] * (k + 1u)) & 0xFFFFu) * inv) & 0xFFFFu;
      int same = i + 8u <= st_image_words(k);
      for (j = 0u; same && j < 8u; j++) {
        uint32_t w = ((uint32_t)s[p + 2u * j] << 8) | (uint32_t)s[p + 2u * j + 1u];
        same = w == (uint32_t)st_pat(set, k, i + j);
      }
      if (same) {
        return 1;
      }
    }
  }
  return 0;
}

/* The number of places in the n bytes at s where the plen bytes at pat occur, and the first such offset in *first. */
static uint32_t st_occurrences(const uint8_t *s, size_t n, const uint8_t *pat, size_t plen, size_t *first)
{
  uint32_t count = 0u;
  size_t p;
  for (p = 0u; p + plen <= n; p++) {
    if (memcmp(s + p, pat, plen) == 0) {
      if (count == 0u) {
        *first = p;
      }
      count++;
    }
  }
  return count;
}

/* The bytes of a snapshot of m in a heap buffer of exactly the size the size call reports (so that a snapshot of any size is scanned in
 * full: the property is about the content, not about the 4,299 bytes), and its length. NULL on a refusal. */
static uint8_t *st_save_heap(const ivs_machine *m, uint64_t *n)
{
  uint64_t sz = 0u;
  uint8_t *b;
  *n = 0u;
  if (ivs_machine_snapshot_size(m, &sz) != IVS_OK || sz == 0u || sz > (uint64_t)(8u * 1024u * 1024u)) {
    return NULL;
  }
  b = (uint8_t *)malloc((size_t)sz);
  if (b == NULL) {
    return NULL;
  }
  if (ivs_machine_snapshot_save(m, b, sz, n) != IVS_OK || *n != sz) {
    free(b);
    *n = 0u;
    return NULL;
  }
  return b;
}

/* 1 when byte offset off of a snapshot of len bytes lies in a field that legitimately depends on the images: the three slot records, the
 * composite digest, the digest of each section table entry, the trailer. */
static int st_image_dependent(size_t off, size_t len)
{
  uint32_t i;
  if (off >= 152u && off < 304u) {
    return 1; /* three slot records (152 to 271) and the composite digest (272 to 303) */
  }
  for (i = 0u; i < ST_SECTIONS; i++) {
    size_t d = 304u + 44u * i + 12u;
    if (off >= d && off < d + 32u) {
      return 1;
    }
  }
  return off + 32u >= len;
}

/* The detectors must be able to find what they look for: a window of each image in a buffer of zeros is found, a window with one word
 * wrong is not, and the same for a slot sha256. Without this a scan of a clean snapshot would prove nothing. */
static int st_detectors_work(void)
{
  uint8_t buf[96];
  uint32_t set;
  uint32_t k;
  uint32_t j;
  int ok = 1;
  for (set = 0u; set < 2u; set++) {
    for (k = 0u; k < 3u; k++) {
      memset(buf, 0, sizeof buf);
      for (j = 0u; j < 8u; j++) {
        uint16_t w = st_pat(set, k, 100u + j);
        buf[11u + 2u * j] = (uint8_t)(w >> 8);
        buf[12u + 2u * j] = (uint8_t)w;
      }
      ST_REQ(ok, st_has_window(buf, sizeof buf, set) == 1);
      ST_REQ(ok, st_has_window(buf, sizeof buf, 1u - set) == 0);
      buf[12u + 2u * 5u] ^= 1u;
      ST_REQ(ok, st_has_window(buf, sizeof buf, set) == 0);
    }
  }
  {
    uint8_t pat[32];
    size_t first = 0u;
    for (j = 0u; j < 32u; j++) {
      pat[j] = (uint8_t)(0x91u + 29u * j);
    }
    memset(buf, 0, sizeof buf);
    memcpy(buf + 40u, pat, 32u);
    ST_REQ(ok, st_occurrences(buf, sizeof buf, pat, 32u, &first) == 1u && first == 40u);
    memcpy(buf + 3u, pat, 32u);
    ST_REQ(ok, st_occurrences(buf, sizeof buf, pat, 32u, &first) == 2u && first == 3u);
  }
  return ok;
}

/* state.save.no-image-bytes (Task 3, D-19, T-08-27): the rights property at the container level. Two instances with different content,
 * every word of each image array overwritten by a recognisable non-repeating pattern (a different pattern for each instance): (1) the
 * detectors can find a window and a slot digest; (2) the snapshot of neither contains a 16-byte window (eight consecutive words) of any
 * image array of either pattern; (3) each slot sha256 occurs in the snapshot exactly once, at its slot record; (4) the size is the
 * literal, because an embedded image would change it; (5) the bytes of the two snapshots are identical outside the fields that
 * describe the images (the three slot records, the composite and section digests, the trailer), which differ: the state bytes do not
 * depend on the image arrays. The snapshot buffer has the size the size call reports, so an embedded image is scanned in full. */
static int st_no_image_bytes(void)
{
  ivs_ms_inst a;
  ivs_ms_inst b;
  uint8_t *sa;
  uint8_t *sb;
  uint64_t na = 0u;
  uint64_t nb = 0u;
  uint32_t total_lit = 0u;
  uint32_t i;
  size_t j;
  int ok = 1;
  ST_CHECK(ivsn_lit_u32("total-bytes", &total_lit));
  ST_CHECK(st_detectors_work());
  ST_CHECK(ivs_ms_powered(&a, IVS_MS_CONTENT_A, 0x5Au));
  if (!ivs_ms_powered(&b, IVS_MS_CONTENT_B, 0x5Au)) {
    ivs_ms_free(&a);
    return IVS_TEST_FAIL;
  }
  st_fill_images(a.m, 0u);
  st_fill_images(b.m, 1u);
  sa = st_save_heap(a.m, &na);
  sb = st_save_heap(b.m, &nb);
  ST_REQ(ok, sa != NULL && sb != NULL);
  if (sa != NULL && sb != NULL) {
    /* (4) the size */
    ST_REQ(ok, na == total_lit);
    ST_REQ(ok, nb == total_lit);
    /* (2) no window of either pattern in either snapshot */
    for (i = 0u; i < 2u; i++) {
      if (st_has_window(sa, (size_t)na, i) != 0 || st_has_window(sb, (size_t)nb, i) != 0) {
        ivs_test_diag("a snapshot holds a 16-byte window of an image array of pattern %u", (unsigned)i);
        ok = 0;
      }
    }
    /* (3) each slot sha256 once, at its record */
    for (i = 0u; i < IVS_SLOT_COUNT; i++) {
      size_t first = 0u;
      const struct ivs_machine *ma = a.m;
      const struct ivs_machine *mb = b.m;
      ST_REQ(ok, st_occurrences(sa, (size_t)na, ma->slot[i].sha256, 32u, &first) == 1u && first == 152u + 40u * i + 4u);
      ST_REQ(ok, st_occurrences(sb, (size_t)nb, mb->slot[i].sha256, 32u, &first) == 1u && first == 152u + 40u * i + 4u);
    }
    /* (5) the two snapshots differ only where the images are described */
    if (na == nb) {
      size_t differing = 0u;
      size_t masked_differing = 0u;
      for (j = 0u; j < (size_t)na; j++) {
        if (sa[j] != sb[j]) {
          if (st_image_dependent(j, (size_t)na)) {
            masked_differing++;
          } else {
            differing++;
            if (differing <= 8u) {
              ivs_test_diag("state byte %u differs between two instances that differ only in their images", (unsigned)j);
            }
          }
        }
      }
      ST_REQ(ok, differing == 0u);
      ST_REQ(ok, masked_differing > 0u); /* the images did differ: the slot records, the MEM digest, the composite and the trailer */
      ST_REQ(ok, memcmp(sa + 272u, sb + 272u, 32u) != 0);
    }
  }
  free(sa);
  free(sb);
  ivs_ms_free(&a);
  ivs_ms_free(&b);
  return ok ? IVS_TEST_PASS : IVS_TEST_FAIL;
}

/* ---- the canonical stream (Task 3, D-12, D-16) ---- */

/* The stand-in boot program of this plan at $1000, built only from decles of the existing stand-in programs of machine_standin.c (each
 * derived there from the printed vector rows): the four PSG writes of program TONE (mixer R7 = $3E, amplitude A = $0F, tone A fine
 * period 1 and coarse 0, at $1000 to $100F), then the loop of program A (R1 = $0155, then forever [$0200] = R1 and R1 += 1, at $1010
 * to $1016; the back branch has the same displacement 4 as in program A, because it is relative). It touches the PSG register file, the
 * PSG device state and the audio chain (the first tone edge is at chip step 4096), the write log (four records), system RAM and the
 * CPU registers, and the frame events of the STIC run beneath it. */
static const uint16_t st_boot_words[] = {0x02B8u, 0x003Eu, 0x0240u, 0x01F8u, 0x02B8u, 0x000Fu, 0x0240u, 0x01FBu,
                                         0x02B8u, 0x0001u, 0x0240u, 0x01F0u, 0x02B8u, 0x0000u, 0x0240u, 0x01F4u,
                                         0x02B9u, 0x0155u, 0x0241u, 0x0200u, 0x0009u, 0x0220u, 0x0004u};

/* The number of units of the stepped boot run, chosen by arithmetic. The setup is 9 units (the four PSG writes with their immediates and
 * the load of R1); each pass of the loop is 3 units of 26 cycles (MVO direct 11, INCR 6, a taken branch 9), that is 104 master ticks at 4
 * ticks per cycle. The first tone edge needs chip step 4096 of 16 ticks, 65,536 ticks; a frame is 59,736 ticks, so the first two frames end
 * at 119,472 and the third at 179,208. 5000 units are about 1,664 passes: about 43,000 cycles and 173,000 master ticks (measured once, when
 * the literal was pinned: 43,338 cycles, 173,352 ticks, 2.9 frames), which is past the first tone edge and past two frame ends and short
 * of the third: a cut in the middle of the third frame, after a write to every array the program touches. The test asserts these facts
 * about the state it reached instead of trusting the arithmetic. */
#define ST_BOOT_UNITS 5000u

/* The stepped boot instance, and 1 when the facts the arithmetic promised hold: the PSG registers written, system RAM written, the audio
 * chain moved, four write-log records, two frames completed (the video count and the STIC frame index), and time strictly inside a frame. */
static int st_boot_instance(ivs_ms_inst *x, uint32_t units)
{
  ivs_info info;
  const struct ivs_machine *im;
  uint16_t v = 0u;
  int ok = 1;
  if (!ivs_ms_powered_words(x, st_boot_words, sizeof st_boot_words / sizeof st_boot_words[0], 0x5Au)) {
    return 0;
  }
  if (!ivs_ms_step(x->m, units, NULL)) {
    ivs_ms_free(x);
    return 0;
  }
  im = x->m;
  memset(&info, 0, sizeof info);
  info.struct_size = (uint32_t)sizeof info;
  ST_REQ(ok, ivs_machine_info(x->m, &info) == IVS_OK);
  if (units == ST_BOOT_UNITS) {
    ST_REQ(ok, ivs_machine_peek(x->m, 0x01F8u, 1u, &v) == IVS_OK && v == 0x003Eu);
    ST_REQ(ok, ivs_machine_peek(x->m, 0x0200u, 1u, &v) == IVS_OK && v >= 0x0155u);
    ST_REQ(ok, im->obs.psglog.count == 4u);
    ST_REQ(ok, im->obs.audio.count >= 1u);
    ST_REQ(ok, im->stic.frame_index == 2u && im->obs.video.count == 2u);
    ST_REQ(ok, info.nominal_frame_ticks != 0u && im->time_ticks % info.nominal_frame_ticks != 0u);
    ST_REQ(ok, im->time_ticks > 2u * info.nominal_frame_ticks && im->time_ticks < 3u * info.nominal_frame_ticks);
  }
  if (!ok) {
    ivs_ms_free(x);
    return 0;
  }
  return 1;
}

/* A snapshot of the stepped boot instance (ST_BOOT_UNITS units) into snap; the instance stays alive in x. */
static int st_snapshot_of_boot(ivs_ms_inst *x, uint8_t *snap, size_t cap, size_t *n)
{
  uint64_t w = 0u;
  if (!st_boot_instance(x, ST_BOOT_UNITS)) {
    return 0;
  }
  if (ivs_machine_snapshot_save(x->m, snap, cap, &w) != IVS_OK) {
    ivs_ms_free(x);
    return 0;
  }
  *n = (size_t)w;
  return 1;
}

static void st_hex(const uint8_t *d, char out[65])
{
  static const char h[] = "0123456789abcdef";
  uint32_t i;
  for (i = 0u; i < 32u; i++) {
    out[2u * i] = h[d[i] >> 4];
    out[2u * i + 1u] = h[d[i] & 0xFu];
  }
  out[64] = '\0';
}

/* state.save.canonical-stream-literal (Task 3, D-12, D-16): the snapshot of the stepped stand-in boot program (ST_BOOT_UNITS units, a cut
 * in the middle of the third frame, see above) has the SHA-256 pinned as stepped-snapshot-sha256 in data/expected/ivsn-literals.ids. The
 * pin is a self-recorded value that proves repeatability only, never that the state is right; because the same literal passes in the dev
 * and the release preset, the snapshot bytes are identical across the two builds. A second snapshot of the same instance is byte-identical
 * to the first (save is a pure function of the instance), also after a size call, and the instance is unchanged. */
static int st_canonical_stream(void)
{
  ivs_ms_inst x;
  uint8_t snap[IVS_SNAPSHOT_MAX_BYTES];
  uint8_t again[IVS_SNAPSHOT_MAX_BYTES];
  uint8_t want[32];
  uint8_t got[32];
  uint8_t *before;
  char hex[65];
  uint64_t n = 0u;
  uint64_t n2 = 0u;
  uint64_t sz = 0u;
  int ok = 1;
  ST_CHECK(ivsn_lit_hex("stepped-snapshot-sha256", want, 32u));
  ST_CHECK(st_boot_instance(&x, ST_BOOT_UNITS));
  before = (uint8_t *)malloc(IVS_MACHINE_BYTES);
  if (before == NULL) {
    ivs_ms_free(&x);
    return IVS_TEST_FAIL;
  }
  memcpy(before, x.storage, IVS_MACHINE_BYTES);
  ST_REQ(ok, ivs_machine_snapshot_save(x.m, snap, sizeof snap, &n) == IVS_OK);
  ST_REQ(ok, ivs_machine_snapshot_size(x.m, &sz) == IVS_OK && sz == n);
  ST_REQ(ok, ivs_machine_snapshot_save(x.m, again, sizeof again, &n2) == IVS_OK);
  ST_REQ(ok, n2 == n && n <= sizeof snap && memcmp(snap, again, (size_t)n) == 0);
  ST_REQ(ok, memcmp(before, x.storage, IVS_MACHINE_BYTES) == 0);
  if (n <= sizeof snap) {
    ivs_sha256_digest(snap, (size_t)n, got);
    st_hex(got, hex);
    if (memcmp(got, want, 32u) != 0) {
      ivs_test_diag("the stepped snapshot sha256 is %s", hex);
    }
    ST_REQ(ok, memcmp(got, want, 32u) == 0);
  }
  free(before);
  ivs_ms_free(&x);
  return ok ? IVS_TEST_PASS : IVS_TEST_FAIL;
}

/* ---- read-only and fixed size (Task 3, D-14) ---- */

/* The nine digests of the instance (the composite and the eight domains), 9 x 32 bytes. */
static int st_nine_digests(const ivs_machine *m, uint8_t out[9][32])
{
  static const uint32_t doms[9] = {IVS_DIGEST_COMPOSITE, IVS_DIGEST_CPU,   IVS_DIGEST_MEM,   IVS_DIGEST_IMAGES, IVS_DIGEST_INSTANCE,
                                   IVS_DIGEST_VIDEO,     IVS_DIGEST_AUDIO, IVS_DIGEST_INPUT, IVS_DIGEST_PSGLOG};
  uint32_t i;
  for (i = 0u; i < 9u; i++) {
    if (ivs_machine_state_digest(m, doms[i], out[i]) != IVS_OK) {
      return 0;
    }
  }
  return 1;
}

/* A run through the public call to target ticks with a video buffer of the capacity the layout requires; 1 when it ended OK. */
static int st_run_with_video(ivs_ms_inst *x, uint64_t target, uint64_t *frames)
{
  ivs_info info;
  ivs_run_request q;
  ivs_run_report r;
  uint8_t *video;
  uint64_t cap;
  ivs_status st;
  memset(&info, 0, sizeof info);
  info.struct_size = (uint32_t)sizeof info;
  if (ivs_machine_info(x->m, &info) != IVS_OK || info.nominal_frame_ticks == 0u) {
    return 0;
  }
  cap = (uint64_t)info.video_bytes_per_frame * (target / info.nominal_frame_ticks + 1u + info.video_slack_frames);
  video = (uint8_t *)malloc((size_t)cap);
  if (video == NULL) {
    return 0;
  }
  memset(video, 0xAB, (size_t)cap);
  memset(&q, 0, sizeof q);
  q.struct_size = (uint32_t)sizeof q;
  q.target_ticks = target;
  q.video = video;
  q.video_capacity = cap;
  memset(&r, 0, sizeof r);
  r.struct_size = (uint32_t)sizeof r;
  st = ivs_machine_run(x->m, &q, &r);
  free(video);
  *frames = r.video_frames_completed;
  return st == IVS_OK;
}

/* The checks of one instance state: size and save change no byte of the instance and none of the nine digests, the size is the literal,
 * and two saves are byte-identical. */
static int st_read_only_state(ivs_ms_inst *x, uint32_t total_lit, const char *label)
{
  uint8_t snap[IVS_SNAPSHOT_MAX_BYTES];
  uint8_t again[IVS_SNAPSHOT_MAX_BYTES];
  uint8_t d0[9][32];
  uint8_t d1[9][32];
  uint8_t *before = (uint8_t *)malloc(IVS_MACHINE_BYTES);
  uint64_t sz = 0u;
  uint64_t n = 0u;
  uint64_t n2 = 0u;
  int ok = 1;
  if (before == NULL) {
    return 0;
  }
  memcpy(before, x->storage, IVS_MACHINE_BYTES);
  ST_REQ(ok, st_nine_digests(x->m, d0));
  ST_REQ(ok, ivs_machine_snapshot_size(x->m, &sz) == IVS_OK && sz == total_lit);
  ST_REQ(ok, ivs_machine_snapshot_save(x->m, snap, sizeof snap, &n) == IVS_OK && n == total_lit);
  ST_REQ(ok, ivs_machine_snapshot_save(x->m, again, sizeof again, &n2) == IVS_OK && n2 == n && n <= sizeof snap);
  ST_REQ(ok, n <= sizeof snap && memcmp(snap, again, (size_t)n) == 0);
  ST_REQ(ok, memcmp(before, x->storage, IVS_MACHINE_BYTES) == 0);
  ST_REQ(ok, st_nine_digests(x->m, d1));
  ST_REQ(ok, memcmp(d0, d1, sizeof d0) == 0);
  if (!ok) {
    ivs_test_diag("read-only check failed in state '%s'", label);
  }
  free(before);
  return ok;
}

/* state.save.read-only-fixed-size (Task 3, D-14): save and size do not mutate the instance (the whole instance byte-identical, the nine
 * digests unchanged) in the reset state, after a stepped run and after a run through the public call that completed several frames
 * with a video buffer; and the size is the same literal in all those states, for the second content and for an instance whose cartridge
 * slot is empty (the plan's "no image loaded" is not reachable while powered: a power-on reset refuses an instance without its EXEC and
 * GROM images, so the smallest powered content is the two system images alone). */
static int st_read_only_fixed_size(void)
{
  ivs_ms_inst x;
  uint32_t total_lit = 0u;
  uint64_t frames = 0u;
  int ok = 1;
  ST_CHECK(ivsn_lit_u32("total-bytes", &total_lit));
  /* reset state, then a stepped run, then three frames through the public call with a video buffer */
  ST_CHECK(ivs_ms_powered_words(&x, st_boot_words, sizeof st_boot_words / sizeof st_boot_words[0], 0x5Au));
  ST_REQ(ok, st_read_only_state(&x, total_lit, "reset"));
  ST_REQ(ok, ivs_ms_step(x.m, 777u, NULL));
  ST_REQ(ok, st_read_only_state(&x, total_lit, "stepped"));
  ST_REQ(ok, st_run_with_video(&x, 3u * 59736u + 4000u, &frames) && frames >= 2u);
  ST_REQ(ok, st_read_only_state(&x, total_lit, "after several frames with a video buffer"));
  ivs_ms_free(&x);
  /* the second content and the instance without a cartridge */
  ST_CHECK(ivs_ms_powered(&x, IVS_MS_CONTENT_B, 0x5Au));
  ST_REQ(ok, ivs_ms_step(x.m, 300u, NULL));
  ST_REQ(ok, st_read_only_state(&x, total_lit, "content B"));
  ivs_ms_free(&x);
  ST_CHECK(st_power_no_cart(&x));
  ST_REQ(ok, ivs_ms_step(x.m, 300u, NULL));
  ST_REQ(ok, st_read_only_state(&x, total_lit, "no cartridge"));
  ivs_ms_free(&x);
  return ok ? IVS_TEST_PASS : IVS_TEST_FAIL;
}

/* ---- the judge: the structure comparison can fail (Task 3) ---- */

/* state.judge.header-literal-corrupt (Task 3, D-16): the comparison of state.literal.header-structure is a function of the bytes it is
 * given, so it is run against damaged copies of a good snapshot: one header byte flipped, one section table byte flipped and the trailer
 * flipped must each be reported as a difference, by the comparison that names that field (header prefix 2, section entry 3, trailer
 * 10); so must a one-byte-short copy (1), a flipped schema digest byte (4), a flipped profile id byte (5), a flipped parameter digest byte
 * (6), a flipped slot record byte (7) and a flipped composite digest byte (8). The id passes only when every damaged copy is caught
 * and the undamaged one is not. */
static int st_judge_header_corrupt(void)
{
  static const struct {
    size_t off;      /* byte to flip, or (size_t)-1 for the length row */
    int want;        /* the comparison that must report it */
    const char *name;
  } rows[] = {
      {9u, 2, "a header byte (the format version)"},
      {304u + 44u + 5u, 3, "a section table byte (the MEM section offset)"},
      {24u + 7u, 4, "a schema digest byte"},
      {56u + 3u, 5, "a profile id byte"},
      {120u + 11u, 6, "a parameter digest byte"},
      {152u + 40u + 9u, 7, "a slot record byte (the GROM sha256)"},
      {272u + 5u, 8, "a composite digest byte"},
      {304u + 44u * 2u + 20u, 9, "a section digest byte"},
      {4299u - 1u, 10, "the last trailer byte"},
      {4299u - 32u, 10, "the first trailer byte"},
      {(size_t)-1, 1, "the length (one byte short)"},
  };
  ivs_ms_inst x;
  uint8_t snap[IVS_SNAPSHOT_MAX_BYTES];
  uint8_t bad[IVS_SNAPSHOT_MAX_BYTES];
  size_t n = 0u;
  const char *what = "";
  size_t r;
  int ok = 1;
  ST_CHECK(st_snapshot_of_boot(&x, snap, sizeof snap, &n));
  ST_REQ(ok, st_structure_diff(snap, n, x.m, &what) == 0);
  for (r = 0u; r < sizeof rows / sizeof rows[0]; r++) {
    int d;
    memcpy(bad, snap, n);
    if (rows[r].off == (size_t)-1) {
      d = st_structure_diff(bad, n - 1u, x.m, &what);
    } else {
      bad[rows[r].off] ^= 0x01u;
      d = st_structure_diff(bad, n, x.m, &what);
    }
    if (d != rows[r].want) {
      ivs_test_diag("damaged copy (%s): comparison %d reported, comparison %d expected", rows[r].name, d, rows[r].want);
      ok = 0;
    }
  }
  ivs_ms_free(&x);
  return ok ? IVS_TEST_PASS : IVS_TEST_FAIL;
}

const ivs_test_case ivs_state_save_cases[] = {
    {"state.size.derived-and-constant", st_size_derived, NULL},
    {"state.save.refusal-ladder", st_refusal_ladder, NULL},
    {"state.save.no-image-bytes", st_no_image_bytes, NULL},
    {"state.save.canonical-stream-literal", st_canonical_stream, NULL},
    {"state.save.read-only-fixed-size", st_read_only_fixed_size, NULL},
    {"state.judge.header-literal-corrupt", st_judge_header_corrupt, NULL},
};
const size_t ivs_state_save_count = sizeof ivs_state_save_cases / sizeof ivs_state_save_cases[0];

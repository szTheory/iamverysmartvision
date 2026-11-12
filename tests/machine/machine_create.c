/* SPDX-License-Identifier: MIT */
/* Suite machine, family create (D-01..D-04, D-08, plan 03-09): storage requirements, create,
 * info and destroy through the public header only (ivs/ivs_machine.h); machine/machine.h is read
 * for the instance size and the magic. Instances live in exact-size heap storage, so ASan sees
 * any byte the core touches outside them; never on the stack (about 113 KB). */
#include <stdint.h>
#include <stdlib.h>
#include <string.h>

#include "ivs/ivs.h"
#include "ivs/ivs_machine.h"
#include "ivs_test.h"
#include "machine/machine.h"
#include "machine_cases.h"

/* Fails the case with a diagnostic naming the line and the failed expression. */
#define IVS_CHECK(cond)                                                                                \
  do {                                                                                                 \
    if (!(cond)) {                                                                                     \
      ivs_test_diag("check failed at line %d: %s", __LINE__, #cond);                                   \
      return IVS_TEST_FAIL;                                                                            \
    }                                                                                                  \
  } while (0)

#define MC_PROFILE "intv-2609-ntsc-family-p1"

static ivs_create_options mc_opts(const char *id)
{
  ivs_create_options o;
  memset(&o, 0, sizeof o);
  o.struct_size = (uint32_t)sizeof o;
  o.flags = 0u;
  o.profile_id = id;
  return o;
}

/* ---- tracer ---- */

/* The requirements call reports the internal instance size and 16-byte alignment. */
static int mc_requirements(void)
{
  ivs_storage_requirements r;
  memset(&r, 0xA5, sizeof r);
  r.struct_size = (uint32_t)sizeof r;
  IVS_CHECK(ivs_machine_storage_requirements(MC_PROFILE, &r) == IVS_OK);
  IVS_CHECK(r.struct_size == (uint32_t)sizeof r);
  IVS_CHECK(r.reserved == 0u);
  IVS_CHECK(r.size_bytes == (uint64_t)sizeof(struct ivs_machine));
  IVS_CHECK(r.size_bytes == (uint64_t)IVS_MACHINE_BYTES);
  IVS_CHECK(r.align_bytes == 16u);
  return IVS_TEST_PASS;
}

/* Create in exact-size heap storage, info shows phase OFF and three empty slots, destroy, and
 * info on the destroyed handle is BAD_HANDLE. */
static int mc_ok_info_destroy(void)
{
  static const uint8_t zero32[32] = {0};
  ivs_create_options o = mc_opts(MC_PROFILE);
  ivs_machine *m = NULL;
  ivs_info info;
  uint32_t i;
  int rc = IVS_TEST_FAIL;
  uint8_t *st = aligned_alloc(IVS_MACHINE_ALIGN, IVS_MACHINE_BYTES);
  IVS_CHECK(st != NULL);
  memset(st, 0xA5, IVS_MACHINE_BYTES);
  if (ivs_machine_create(&o, st, IVS_MACHINE_BYTES, &m) != IVS_OK || m != (ivs_machine *)(void *)st) {
    ivs_test_diag("create failed");
    goto done;
  }
  memset(&info, 0xA5, sizeof info);
  info.struct_size = (uint32_t)sizeof info;
  if (ivs_machine_info(m, &info) != IVS_OK || info.struct_size != (uint32_t)sizeof info ||
      info.phase != IVS_PHASE_OFF || info.time_ticks != 0u || info.profile_index != 0u || info.reserved != 0u) {
    ivs_test_diag("info header wrong");
    goto done;
  }
  for (i = 0u; i < IVS_SLOT_COUNT; i++) {
    const ivs_image_report *s = &info.slots[i];
    if (s->struct_size != (uint32_t)sizeof *s || s->kind != i + 1u || s->loaded != 0u || s->length_bytes != 0u ||
        s->length_units != 0u || s->unit_bits != 0u || s->base_addr != 0u || s->reserved != 0u ||
        memcmp(s->sha256, zero32, sizeof zero32) != 0) {
      ivs_test_diag("slot %u not empty", (unsigned)i);
      goto done;
    }
  }
  if (ivs_machine_destroy(m) != IVS_OK) {
    ivs_test_diag("destroy failed");
    goto done;
  }
  if (ivs_machine_info(m, &info) != IVS_E_BAD_HANDLE) {
    ivs_test_diag("info after destroy is not BAD_HANDLE");
    goto done;
  }
  rc = IVS_TEST_PASS;
done:
  free(st);
  return rc;
}

/* ---- storage helpers ---- */

/* Over-allocated storage: raw holds n + MC_PAD bytes of poison and p is raw aligned to 16 plus
 * misalign. A refusal must leave every raw byte poisoned (also bytes past the storage). */
#define MC_PAD 32u

typedef struct mc_store {
  uint8_t *raw;
  uint8_t *p;
  size_t raw_bytes;
  uint8_t poison;
} mc_store;

static int mc_store_new(mc_store *s, size_t n, size_t misalign, uint8_t poison)
{
  uintptr_t a;
  s->raw_bytes = n + MC_PAD;
  s->raw = malloc(s->raw_bytes);
  s->poison = poison;
  if (s->raw == NULL) {
    return 0;
  }
  memset(s->raw, poison, s->raw_bytes);
  a = (uintptr_t)s->raw;
  s->p = s->raw + ((16u - (a % 16u)) % 16u) + misalign;
  return 1;
}

static int mc_store_intact(const mc_store *s)
{
  size_t i;
  for (i = 0; i < s->raw_bytes; i++) {
    if (s->raw[i] != s->poison) {
      ivs_test_diag("raw byte %zu written (0x%02x)", i, (unsigned)s->raw[i]);
      return 0;
    }
  }
  return 1;
}

/* Exact-size 16-aligned storage of one instance, poisoned. */
static uint8_t *mc_exact(uint8_t poison)
{
  uint8_t *p = aligned_alloc(16u, IVS_MACHINE_BYTES);
  if (p != NULL) {
    memset(p, poison, IVS_MACHINE_BYTES);
  }
  return p;
}

/* create with o into poisoned storage aligned + misalign, passing storage_bytes; NULL storage or
 * NULL out when asked. Expects status want, *out NULL and every storage byte untouched. */
static int mc_create_refused(const ivs_create_options *o, int null_storage, int null_out, size_t misalign,
                             size_t storage_bytes, ivs_status want)
{
  static uint8_t sentinel_storage[16];
  mc_store s;
  ivs_machine *m = (ivs_machine *)(void *)sentinel_storage;
  ivs_status st;
  int ok;
  if (!mc_store_new(&s, IVS_MACHINE_BYTES, misalign, 0xA5u)) {
    ivs_test_diag("malloc failed");
    return IVS_TEST_FAIL;
  }
  st = ivs_machine_create(o, null_storage ? NULL : s.p, storage_bytes, null_out ? NULL : &m);
  ok = mc_store_intact(&s);
  free(s.raw);
  if (st != want) {
    ivs_test_diag("status %d, want %d", st, want);
    return IVS_TEST_FAIL;
  }
  if (!null_out && m != NULL) {
    ivs_test_diag("*out not NULL after a refusal");
    return IVS_TEST_FAIL;
  }
  return ok ? IVS_TEST_PASS : IVS_TEST_FAIL;
}

/* ---- create refusals (precedence BAD_ARG, UNSUPPORTED, UNKNOWN_PROFILE, BUFFER_TOO_SMALL) ---- */

static int mc_refuse_null_opts(void)
{
  return mc_create_refused(NULL, 0, 0, 0u, IVS_MACHINE_BYTES, IVS_E_BAD_ARG);
}

static int mc_refuse_null_storage(void)
{
  ivs_create_options o = mc_opts(MC_PROFILE);
  return mc_create_refused(&o, 1, 0, 0u, IVS_MACHINE_BYTES, IVS_E_BAD_ARG);
}

static int mc_refuse_null_out(void)
{
  ivs_create_options o = mc_opts(MC_PROFILE);
  return mc_create_refused(&o, 0, 1, 0u, IVS_MACHINE_BYTES, IVS_E_BAD_ARG);
}

static int mc_refuse_size_0(void)
{
  ivs_create_options o = mc_opts(MC_PROFILE);
  o.struct_size = 0u;
  return mc_create_refused(&o, 0, 0, 0u, IVS_MACHINE_BYTES, IVS_E_BAD_ARG);
}

static int mc_refuse_size_4(void)
{
  ivs_create_options o = mc_opts(MC_PROFILE);
  o.struct_size = 4u;
  return mc_create_refused(&o, 0, 0, 0u, IVS_MACHINE_BYTES, IVS_E_BAD_ARG);
}

static int mc_refuse_id_null(void)
{
  ivs_create_options o = mc_opts(NULL);
  return mc_create_refused(&o, 0, 0, 0u, IVS_MACHINE_BYTES, IVS_E_BAD_ARG);
}

/* 64 bytes with no NUL, in an exact-size heap buffer: the scan must stop at the bound. */
static int mc_refuse_id_unterminated(void)
{
  ivs_create_options o;
  int rc;
  char *id = malloc(IVS_PROFILE_ID_MAX_BYTES);
  IVS_CHECK(id != NULL);
  memset(id, 'x', IVS_PROFILE_ID_MAX_BYTES);
  o = mc_opts(id);
  rc = mc_create_refused(&o, 0, 0, 0u, IVS_MACHINE_BYTES, IVS_E_BAD_ARG);
  free(id);
  return rc;
}

/* 63 bytes and a NUL (the longest legal id) is not BAD_ARG: it is a registry miss. */
static int mc_refuse_id_63_unknown(void)
{
  ivs_create_options o;
  int rc;
  char *id = malloc(IVS_PROFILE_ID_MAX_BYTES);
  IVS_CHECK(id != NULL);
  memset(id, 'x', IVS_PROFILE_ID_MAX_BYTES - 1u);
  id[IVS_PROFILE_ID_MAX_BYTES - 1u] = '\0';
  o = mc_opts(id);
  rc = mc_create_refused(&o, 0, 0, 0u, IVS_MACHINE_BYTES, IVS_E_UNKNOWN_PROFILE);
  free(id);
  return rc;
}

static int mc_refuse_misaligned_1(void)
{
  ivs_create_options o = mc_opts(MC_PROFILE);
  return mc_create_refused(&o, 0, 0, 1u, IVS_MACHINE_BYTES, IVS_E_BAD_ARG);
}

static int mc_refuse_misaligned_8(void)
{
  ivs_create_options o = mc_opts(MC_PROFILE);
  return mc_create_refused(&o, 0, 0, 8u, IVS_MACHINE_BYTES, IVS_E_BAD_ARG);
}

static int mc_refuse_flags_1(void)
{
  ivs_create_options o = mc_opts(MC_PROFILE);
  o.flags = 1u;
  return mc_create_refused(&o, 0, 0, 0u, IVS_MACHINE_BYTES, IVS_E_UNSUPPORTED);
}

/* A larger caller struct (a future version) whose unknown tail is not zero. */
typedef struct mc_big_opts {
  ivs_create_options o;
  uint8_t tail[16];
} mc_big_opts;

static mc_big_opts mc_big(const char *id)
{
  mc_big_opts b;
  memset(&b, 0, sizeof b);
  b.o.struct_size = (uint32_t)sizeof b;
  b.o.profile_id = id;
  return b;
}

static int mc_refuse_tail_nonzero(void)
{
  mc_big_opts b = mc_big(MC_PROFILE);
  b.tail[15] = 1u;
  return mc_create_refused(&b.o, 0, 0, 0u, IVS_MACHINE_BYTES, IVS_E_UNSUPPORTED);
}

/* Options in an exact-size heap buffer of bytes bytes (at least the v1 size), zero past the v1
 * struct, with struct_size set: ASan sees any byte the core reads past the buffer (review WR-02).
 * The caller frees the buffer. */
static uint8_t *mc_heap_opts(size_t bytes, uint32_t struct_size)
{
  ivs_create_options o = mc_opts(MC_PROFILE);
  uint8_t *b = malloc(bytes);
  if (b != NULL) {
    memset(b, 0, bytes);
    o.struct_size = struct_size;
    memcpy(b, &o, sizeof o);
  }
  return b;
}

static int mc_heap_opts_refused(size_t bytes, uint32_t struct_size, size_t nonzero_at, ivs_status want)
{
  uint8_t *b = mc_heap_opts(bytes, struct_size);
  int r;
  IVS_CHECK(b != NULL);
  if (nonzero_at < bytes) {
    b[nonzero_at] = 0xFFu;
  }
  r = mc_create_refused((const ivs_create_options *)(void *)b, 0, 0, 0u, IVS_MACHINE_BYTES, want);
  free(b);
  return r;
}

/* struct_size one above IVS_STRUCT_SIZE_MAX, with an all-zero tail: BAD_ARG, not a scan. */
static int mc_refuse_size_over_max(void)
{
  return mc_heap_opts_refused(IVS_STRUCT_SIZE_MAX + 1u, IVS_STRUCT_SIZE_MAX + 1u, (size_t)-1, IVS_E_BAD_ARG);
}

/* A garbage struct_size (an uninitialised host struct) on a small struct whose first byte past
 * the v1 size is nonzero: BAD_ARG; an uncapped scan would stop there with UNSUPPORTED. */
static int mc_refuse_size_garbage(void)
{
  return mc_heap_opts_refused(sizeof(ivs_create_options) + 8u, 0x7FFFFFF0u, sizeof(ivs_create_options),
                              IVS_E_BAD_ARG);
}

/* struct_size exactly IVS_STRUCT_SIZE_MAX keeps D-04: its last unknown byte nonzero is UNSUPPORTED. */
static int mc_refuse_size_at_max_tail(void)
{
  return mc_heap_opts_refused(IVS_STRUCT_SIZE_MAX, IVS_STRUCT_SIZE_MAX, IVS_STRUCT_SIZE_MAX - 1u, IVS_E_UNSUPPORTED);
}

static int mc_refuse_unknown_profile(void)
{
  ivs_create_options o = mc_opts("no-such-profile");
  return mc_create_refused(&o, 0, 0, 0u, IVS_MACHINE_BYTES, IVS_E_UNKNOWN_PROFILE);
}

static int mc_refuse_near_miss(void)
{
  ivs_create_options o = mc_opts("intv-2609-ntsc-family-p2");
  return mc_create_refused(&o, 0, 0, 0u, IVS_MACHINE_BYTES, IVS_E_UNKNOWN_PROFILE);
}

static int mc_refuse_trailing_space(void)
{
  ivs_create_options o = mc_opts(MC_PROFILE " ");
  return mc_create_refused(&o, 0, 0, 0u, IVS_MACHINE_BYTES, IVS_E_UNKNOWN_PROFILE);
}

static int mc_refuse_short_storage(void)
{
  ivs_create_options o = mc_opts(MC_PROFILE);
  return mc_create_refused(&o, 0, 0, 0u, IVS_MACHINE_BYTES - 1u, IVS_E_BUFFER_TOO_SMALL);
}

/* Two refusal classes at once: the earlier class wins (also pins the order the SUMMARY records). */
static int mc_refuse_misaligned_over_flags(void)
{
  ivs_create_options o = mc_opts(MC_PROFILE);
  o.flags = 1u;
  return mc_create_refused(&o, 0, 0, 1u, IVS_MACHINE_BYTES, IVS_E_BAD_ARG);
}

static int mc_refuse_flags_over_unknown(void)
{
  ivs_create_options o = mc_opts("no-such-profile");
  o.flags = 1u;
  return mc_create_refused(&o, 0, 0, 0u, IVS_MACHINE_BYTES, IVS_E_UNSUPPORTED);
}

static int mc_refuse_unknown_over_short(void)
{
  ivs_create_options o = mc_opts("no-such-profile");
  return mc_create_refused(&o, 0, 0, 0u, IVS_MACHINE_BYTES - 1u, IVS_E_UNKNOWN_PROFILE);
}

/* ---- create positives ---- */

/* Review WR-04: the profile registry, the PSG parameter table and the control-line table are three
 * generated includes in three translation units; a profile must have a row in each. The three counts are
 * equal, every registered profile has PSG parameters (and none exists past the end), and a machine of
 * every registered profile can be created, so no profile index reads a table past its end and no machine
 * runs with its PSG silently absent. The named faults PSG_TABLE_SHORT and CTRL_TABLE_SHORT shorten one
 * table by a profile. */
static int mc_tables_cover_registry(void)
{
  uint32_t n = ivs_profile_count();
  uint32_t i;
  IVS_CHECK(n > 0u);
  IVS_CHECK(ivs_psg_params_count() == n);
  IVS_CHECK(ivs_input_profile_count() == n);
  IVS_CHECK(ivs_psg_params_get(n) == NULL);
  for (i = 0u; i < n; i++) {
    const ivs_profile_desc *d = ivs_profile_get(i);
    ivs_create_options o;
    ivs_machine *m = NULL;
    uint8_t *st;
    int ok;
    IVS_CHECK(d != NULL);
    IVS_CHECK(ivs_psg_params_get(i) != NULL);
    o = mc_opts(d->id);
    st = mc_exact(0xA5u);
    IVS_CHECK(st != NULL);
    ok = ivs_machine_create(&o, st, IVS_MACHINE_BYTES, &m) == IVS_OK && m != NULL && ivs_machine_destroy(m) == IVS_OK;
    free(st);
    IVS_CHECK(ok);
  }
  return IVS_TEST_PASS;
}

/* A larger caller struct with a zero tail is accepted. */
static int mc_tail_zero_ok(void)
{
  mc_big_opts b = mc_big(MC_PROFILE);
  ivs_machine *m = NULL;
  int ok;
  uint8_t *st = mc_exact(0xA5u);
  IVS_CHECK(st != NULL);
  ok = ivs_machine_create(&b.o, st, IVS_MACHINE_BYTES, &m) == IVS_OK && m != NULL &&
       ivs_machine_destroy(m) == IVS_OK;
  free(st);
  IVS_CHECK(ok);
  return IVS_TEST_PASS;
}

/* struct_size exactly IVS_STRUCT_SIZE_MAX with an all-zero tail is accepted (review WR-02). */
static int mc_size_at_max_ok(void)
{
  ivs_machine *m = NULL;
  int ok;
  uint8_t *b = mc_heap_opts(IVS_STRUCT_SIZE_MAX, IVS_STRUCT_SIZE_MAX);
  uint8_t *st = mc_exact(0xA5u);
  ok = b != NULL && st != NULL &&
       ivs_machine_create((const ivs_create_options *)(void *)b, st, IVS_MACHINE_BYTES, &m) == IVS_OK && m != NULL &&
       ivs_machine_destroy(m) == IVS_OK;
  free(b);
  free(st);
  IVS_CHECK(ok);
  return IVS_TEST_PASS;
}

/* Create covers every instance byte: storage poisoned 0xA5 and 0x5A ends byte-identical. */
static int mc_poisoned_storage(void)
{
  ivs_create_options o = mc_opts(MC_PROFILE);
  ivs_machine *a = NULL;
  ivs_machine *b = NULL;
  int ok;
  uint8_t *sa = mc_exact(0xA5u);
  uint8_t *sb = mc_exact(0x5Au);
  ok = sa != NULL && sb != NULL && ivs_machine_create(&o, sa, IVS_MACHINE_BYTES, &a) == IVS_OK &&
       ivs_machine_create(&o, sb, IVS_MACHINE_BYTES, &b) == IVS_OK && memcmp(sa, sb, IVS_MACHINE_BYTES) == 0;
  free(sa);
  free(sb);
  IVS_CHECK(ok);
  return IVS_TEST_PASS;
}

/* Storage longer than the instance: the bytes past the instance are the host's and stay as they
 * were; storage_bytes equal to the size is enough. */
static int mc_larger_storage(void)
{
  ivs_create_options o = mc_opts(MC_PROFILE);
  ivs_machine *m = NULL;
  mc_store s;
  size_t i;
  int ok;
  IVS_CHECK(mc_store_new(&s, IVS_MACHINE_BYTES + 64u, 0u, 0xA5u));
  ok = ivs_machine_create(&o, s.p, IVS_MACHINE_BYTES + 64u, &m) == IVS_OK;
  for (i = 0; ok && i < 64u; i++) {
    ok = s.p[IVS_MACHINE_BYTES + i] == 0xA5u;
  }
  free(s.raw);
  IVS_CHECK(ok);
  return IVS_TEST_PASS;
}

/* One case in a 16-aligned static buffer, so the release -O2 run covers static storage too. */
static _Alignas(16) uint8_t mc_static_storage[IVS_MACHINE_BYTES];

static int mc_static_storage_case(void)
{
  ivs_create_options o = mc_opts(MC_PROFILE);
  ivs_machine *m = NULL;
  ivs_info info;
  size_t i;
  memset(mc_static_storage, 0x5A, sizeof mc_static_storage);
  IVS_CHECK(ivs_machine_create(&o, mc_static_storage, sizeof mc_static_storage, &m) == IVS_OK);
  memset(&info, 0, sizeof info);
  info.struct_size = (uint32_t)sizeof info;
  IVS_CHECK(ivs_machine_info(m, &info) == IVS_OK);
  IVS_CHECK(info.phase == IVS_PHASE_OFF);
  IVS_CHECK(ivs_machine_destroy(m) == IVS_OK);
  for (i = 0; i < sizeof mc_static_storage; i++) {
    IVS_CHECK(mc_static_storage[i] == 0u);
  }
  return IVS_TEST_PASS;
}

/* ---- destroy ---- */

static int mc_refuse_destroy_null(void)
{
  IVS_CHECK(ivs_machine_destroy(NULL) == IVS_E_BAD_HANDLE);
  return IVS_TEST_PASS;
}

/* Storage that never held an instance (no magic): BAD_HANDLE and no write. */
static int mc_refuse_destroy_wrong_magic(void)
{
  mc_store s;
  ivs_status st;
  int ok;
  IVS_CHECK(mc_store_new(&s, IVS_MACHINE_BYTES, 0u, 0xA5u));
  st = ivs_machine_destroy((ivs_machine *)(void *)s.p);
  ok = mc_store_intact(&s);
  free(s.raw);
  IVS_CHECK(st == IVS_E_BAD_HANDLE);
  IVS_CHECK(ok);
  return IVS_TEST_PASS;
}

/* Every instance byte is zero after destroy. */
static int mc_destroy_scrubs(void)
{
  ivs_create_options o = mc_opts(MC_PROFILE);
  ivs_machine *m = NULL;
  size_t i;
  int ok;
  uint8_t *st = mc_exact(0xA5u);
  IVS_CHECK(st != NULL);
  ok = ivs_machine_create(&o, st, IVS_MACHINE_BYTES, &m) == IVS_OK && ivs_machine_destroy(m) == IVS_OK;
  for (i = 0; ok && i < IVS_MACHINE_BYTES; i++) {
    ok = st[i] == 0u;
  }
  free(st);
  IVS_CHECK(ok);
  return IVS_TEST_PASS;
}

static int mc_refuse_destroy_twice(void)
{
  ivs_create_options o = mc_opts(MC_PROFILE);
  ivs_machine *m = NULL;
  int ok;
  uint8_t *st = mc_exact(0xA5u);
  IVS_CHECK(st != NULL);
  ok = ivs_machine_create(&o, st, IVS_MACHINE_BYTES, &m) == IVS_OK && ivs_machine_destroy(m) == IVS_OK &&
       ivs_machine_destroy(m) == IVS_E_BAD_HANDLE;
  free(st);
  IVS_CHECK(ok);
  return IVS_TEST_PASS;
}

/* ---- info ---- */

/* A fresh instance in exact-size storage; the caller frees *st. */
static int mc_fresh(uint8_t **st, ivs_machine **m)
{
  ivs_create_options o = mc_opts(MC_PROFILE);
  *m = NULL;
  *st = mc_exact(0xA5u);
  return *st != NULL && ivs_machine_create(&o, *st, IVS_MACHINE_BYTES, m) == IVS_OK;
}

/* info(m, out) with a poisoned out of out_bytes whose struct_size is size: expects want, out
 * untouched and the instance unchanged. */
static int mc_info_refused(int null_handle, int destroyed, uint32_t size, ivs_status want)
{
  ivs_machine *m = NULL;
  uint8_t *st = NULL;
  uint8_t *snap;
  ivs_info out;
  ivs_info poison;
  ivs_status got;
  int ok;
  if (!mc_fresh(&st, &m)) {
    free(st);
    ivs_test_diag("create failed");
    return IVS_TEST_FAIL;
  }
  if (destroyed && ivs_machine_destroy(m) != IVS_OK) {
    free(st);
    return IVS_TEST_FAIL;
  }
  snap = malloc(IVS_MACHINE_BYTES);
  if (snap == NULL) {
    free(st);
    return IVS_TEST_FAIL;
  }
  memcpy(snap, st, IVS_MACHINE_BYTES);
  memset(&out, 0xA5, sizeof out);
  out.struct_size = size;
  poison = out;
  got = ivs_machine_info(null_handle ? NULL : m, &out);
  ok = got == want && memcmp(&out, &poison, sizeof out) == 0 && memcmp(snap, st, IVS_MACHINE_BYTES) == 0;
  if (got != want) {
    ivs_test_diag("status %d, want %d", got, want);
  }
  free(snap);
  free(st);
  return ok ? IVS_TEST_PASS : IVS_TEST_FAIL;
}

static int mc_refuse_info_null_out(void)
{
  ivs_machine *m = NULL;
  uint8_t *st = NULL;
  int ok = mc_fresh(&st, &m) && ivs_machine_info(m, NULL) == IVS_E_BAD_ARG;
  free(st);
  IVS_CHECK(ok);
  return IVS_TEST_PASS;
}

/* Both a NULL out and a NULL handle: the argument class comes first. */
static int mc_refuse_info_null_both(void)
{
  IVS_CHECK(ivs_machine_info(NULL, NULL) == IVS_E_BAD_ARG);
  return IVS_TEST_PASS;
}

static int mc_refuse_info_size_0(void)
{
  return mc_info_refused(0, 0, 0u, IVS_E_BAD_ARG);
}

/* Below the v1 size (A-02, plan 04-09: a size between the v1 size and sizeof(ivs_info) is a valid
 * older caller, so the short case sits one byte below the v1 size). */
static int mc_refuse_info_short(void)
{
  return mc_info_refused(0, 0, IVS_INFO_V1_SIZE - 1u, IVS_E_BAD_ARG);
}

static int mc_refuse_info_null_handle(void)
{
  return mc_info_refused(1, 0, (uint32_t)sizeof(ivs_info), IVS_E_BAD_HANDLE);
}

static int mc_refuse_info_destroyed(void)
{
  return mc_info_refused(0, 1, (uint32_t)sizeof(ivs_info), IVS_E_BAD_HANDLE);
}

/* A larger out struct gets only the known bytes; struct_size says how many were written. */
typedef struct mc_big_info {
  ivs_info info;
  uint8_t tail[24];
} mc_big_info;

static int mc_info_larger_out(void)
{
  ivs_machine *m = NULL;
  uint8_t *st = NULL;
  mc_big_info b;
  size_t i;
  int ok;
  memset(&b, 0xA5, sizeof b);
  b.info.struct_size = (uint32_t)sizeof b;
  ok = mc_fresh(&st, &m) && ivs_machine_info(m, &b.info) == IVS_OK;
  free(st);
  IVS_CHECK(ok);
  IVS_CHECK(b.info.struct_size == (uint32_t)sizeof(ivs_info));
  IVS_CHECK(b.info.phase == IVS_PHASE_OFF);
  for (i = 0; i < sizeof b.tail; i++) {
    IVS_CHECK(b.tail[i] == 0xA5u);
  }
  return IVS_TEST_PASS;
}

/* ---- storage requirements ---- */

static int mc_req_refused(const char *id, uint32_t size, ivs_status want)
{
  ivs_storage_requirements r;
  ivs_storage_requirements poison;
  memset(&r, 0xA5, sizeof r);
  r.struct_size = size;
  poison = r;
  IVS_CHECK(ivs_machine_storage_requirements(id, &r) == want);
  IVS_CHECK(memcmp(&r, &poison, sizeof r) == 0);
  return IVS_TEST_PASS;
}

static int mc_refuse_req_null_out(void)
{
  IVS_CHECK(ivs_machine_storage_requirements(MC_PROFILE, NULL) == IVS_E_BAD_ARG);
  return IVS_TEST_PASS;
}

static int mc_refuse_req_size_0(void)
{
  return mc_req_refused(MC_PROFILE, 0u, IVS_E_BAD_ARG);
}

static int mc_refuse_req_short(void)
{
  return mc_req_refused(MC_PROFILE, (uint32_t)sizeof(ivs_storage_requirements) - 1u, IVS_E_BAD_ARG);
}

static int mc_refuse_req_null_id(void)
{
  return mc_req_refused(NULL, (uint32_t)sizeof(ivs_storage_requirements), IVS_E_BAD_ARG);
}

static int mc_refuse_req_id_unterminated(void)
{
  int rc;
  char *id = malloc(IVS_PROFILE_ID_MAX_BYTES);
  IVS_CHECK(id != NULL);
  memset(id, 'x', IVS_PROFILE_ID_MAX_BYTES);
  rc = mc_req_refused(id, (uint32_t)sizeof(ivs_storage_requirements), IVS_E_BAD_ARG);
  free(id);
  return rc;
}

static int mc_refuse_req_unknown(void)
{
  return mc_req_refused("no-such-profile", (uint32_t)sizeof(ivs_storage_requirements), IVS_E_UNKNOWN_PROFILE);
}

typedef struct mc_big_req {
  ivs_storage_requirements r;
  uint8_t tail[16];
} mc_big_req;

static int mc_req_larger_out(void)
{
  mc_big_req b;
  size_t i;
  memset(&b, 0xA5, sizeof b);
  b.r.struct_size = (uint32_t)sizeof b;
  IVS_CHECK(ivs_machine_storage_requirements(MC_PROFILE, &b.r) == IVS_OK);
  IVS_CHECK(b.r.struct_size == (uint32_t)sizeof(ivs_storage_requirements));
  IVS_CHECK(b.r.size_bytes == (uint64_t)IVS_MACHINE_BYTES);
  for (i = 0; i < sizeof b.tail; i++) {
    IVS_CHECK(b.tail[i] == 0xA5u);
  }
  return IVS_TEST_PASS;
}

/* ---- status names and API version ---- */

#define MC_ST(x) {x, #x}

typedef struct mc_status_name {
  ivs_status value;
  const char *name;
} mc_status_name;

/* Every code of ivs/ivs_status.h; the pin data/expected/api-statuses.ids lists the same set. */
static const mc_status_name mc_statuses[] = {
    MC_ST(IVS_OK),
    MC_ST(IVS_E_BAD_ARG),
    MC_ST(IVS_E_BAD_STATE),
    MC_ST(IVS_E_BAD_HANDLE),
    MC_ST(IVS_E_UNSUPPORTED),
    MC_ST(IVS_E_BUFFER_TOO_SMALL),
    MC_ST(IVS_E_UNKNOWN_PROFILE),
    MC_ST(IVS_E_MALFORMED),
    MC_ST(IVS_E_OVERSIZED),
    MC_ST(IVS_E_OVERLAP),
    MC_ST(IVS_E_OUT_OF_RANGE),
    MC_ST(IVS_E_MISSING_SYSTEM_IMAGE),
    MC_ST(IVS_E_DUPLICATE_LOAD),
    MC_ST(IVS_E_DIGEST_MISMATCH),
};

static int mc_status_names(void)
{
  static const ivs_status unknown[] = {-6, -15, -24, -31, 1, -1000};
  size_t i;
  IVS_CHECK(sizeof mc_statuses / sizeof mc_statuses[0] == 14u);
  for (i = 0; i < sizeof mc_statuses / sizeof mc_statuses[0]; i++) {
    const char *got = ivs_status_name(mc_statuses[i].value);
    if (got == NULL || strcmp(got, mc_statuses[i].name) != 0) {
      ivs_test_diag("status %d: name %s, want %s", mc_statuses[i].value, got == NULL ? "(null)" : got,
                    mc_statuses[i].name);
      return IVS_TEST_FAIL;
    }
  }
  for (i = 0; i < sizeof unknown / sizeof unknown[0]; i++) {
    const char *got = ivs_status_name(unknown[i]);
    IVS_CHECK(got != NULL && strcmp(got, "IVS_E_UNKNOWN") == 0);
  }
  return IVS_TEST_PASS;
}

static int mc_api_version(void)
{
  IVS_CHECK(IVS_API_VERSION_MAJOR == 0u);
  IVS_CHECK(IVS_API_VERSION_MINOR == 1u);
  IVS_CHECK(IVS_API_VERSION_PATCH == 0u);
  IVS_CHECK(IVS_API_VERSION == ((IVS_API_VERSION_MAJOR << 16) | (IVS_API_VERSION_MINOR << 8) | IVS_API_VERSION_PATCH));
  IVS_CHECK(ivs_api_version() == IVS_API_VERSION);
  return IVS_TEST_PASS;
}

const ivs_test_case ivs_machine_create_cases[] = {
    {"machine.api.version", mc_api_version, NULL},
    {"machine.create.info-larger-out", mc_info_larger_out, NULL},
    {"machine.create.larger-storage", mc_larger_storage, NULL},
    {"machine.create.ok-info-destroy", mc_ok_info_destroy, NULL},
    {"machine.create.poisoned-storage", mc_poisoned_storage, NULL},
    {"machine.create.requirements", mc_requirements, NULL},
    {"machine.create.requirements-larger-out", mc_req_larger_out, NULL},
    {"machine.create.size-at-max-ok", mc_size_at_max_ok, NULL},
    {"machine.create.static-storage", mc_static_storage_case, NULL},
    {"machine.create.tables-cover-registry", mc_tables_cover_registry, NULL},
    {"machine.create.tail-zero-ok", mc_tail_zero_ok, NULL},
    {"machine.destroy.scrubs", mc_destroy_scrubs, NULL},
    {"machine.refuse.create.flags-1", mc_refuse_flags_1, NULL},
    {"machine.refuse.create.flags-over-unknown-profile", mc_refuse_flags_over_unknown, NULL},
    {"machine.refuse.create.id-63-unknown", mc_refuse_id_63_unknown, NULL},
    {"machine.refuse.create.id-null", mc_refuse_id_null, NULL},
    {"machine.refuse.create.id-unterminated", mc_refuse_id_unterminated, NULL},
    {"machine.refuse.create.misaligned-1", mc_refuse_misaligned_1, NULL},
    {"machine.refuse.create.misaligned-8", mc_refuse_misaligned_8, NULL},
    {"machine.refuse.create.misaligned-over-flags", mc_refuse_misaligned_over_flags, NULL},
    {"machine.refuse.create.near-miss", mc_refuse_near_miss, NULL},
    {"machine.refuse.create.null-opts", mc_refuse_null_opts, NULL},
    {"machine.refuse.create.null-out", mc_refuse_null_out, NULL},
    {"machine.refuse.create.null-storage", mc_refuse_null_storage, NULL},
    {"machine.refuse.create.short-storage", mc_refuse_short_storage, NULL},
    {"machine.refuse.create.size-0", mc_refuse_size_0, NULL},
    {"machine.refuse.create.size-4", mc_refuse_size_4, NULL},
    {"machine.refuse.create.size-at-max-tail-nonzero", mc_refuse_size_at_max_tail, NULL},
    {"machine.refuse.create.size-garbage", mc_refuse_size_garbage, NULL},
    {"machine.refuse.create.size-over-max", mc_refuse_size_over_max, NULL},
    {"machine.refuse.create.tail-nonzero", mc_refuse_tail_nonzero, NULL},
    {"machine.refuse.create.trailing-space", mc_refuse_trailing_space, NULL},
    {"machine.refuse.create.unknown-over-short-storage", mc_refuse_unknown_over_short, NULL},
    {"machine.refuse.create.unknown-profile", mc_refuse_unknown_profile, NULL},
    {"machine.refuse.destroy.null", mc_refuse_destroy_null, NULL},
    {"machine.refuse.destroy.twice", mc_refuse_destroy_twice, NULL},
    {"machine.refuse.destroy.wrong-magic", mc_refuse_destroy_wrong_magic, NULL},
    {"machine.refuse.info.destroyed", mc_refuse_info_destroyed, NULL},
    {"machine.refuse.info.null-both", mc_refuse_info_null_both, NULL},
    {"machine.refuse.info.null-handle", mc_refuse_info_null_handle, NULL},
    {"machine.refuse.info.null-out", mc_refuse_info_null_out, NULL},
    {"machine.refuse.info.short", mc_refuse_info_short, NULL},
    {"machine.refuse.info.size-0", mc_refuse_info_size_0, NULL},
    {"machine.refuse.requirements.id-unterminated", mc_refuse_req_id_unterminated, NULL},
    {"machine.refuse.requirements.null-id", mc_refuse_req_null_id, NULL},
    {"machine.refuse.requirements.null-out", mc_refuse_req_null_out, NULL},
    {"machine.refuse.requirements.short", mc_refuse_req_short, NULL},
    {"machine.refuse.requirements.size-0", mc_refuse_req_size_0, NULL},
    {"machine.refuse.requirements.unknown-profile", mc_refuse_req_unknown, NULL},
    {"machine.status.names", mc_status_names, NULL},
};
const size_t ivs_machine_create_count = sizeof ivs_machine_create_cases / sizeof ivs_machine_create_cases[0];

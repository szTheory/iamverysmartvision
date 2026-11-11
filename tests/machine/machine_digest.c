/* SPDX-License-Identifier: MIT */
/* Suite machine, family digest (D-35, plan 03-09): the canonical state digest.
 * - Literal streams: the INSTANCE and CPU preimages of a fresh instance, and the CPU preimage of a
 *   synthetic CPU state, equal byte sequences written out by hand (big-endian); their SHA-256
 *   values were computed at authoring time with `cmake -E sha256sum` over those bytes, so the
 *   code under test never produced them.
 * - Composite framing: "IVSSTATE" || u16 version || u8 id || digest for domains 1..4, rebuilt here.
 * - Coverage: every byte of struct ivs_machine is in exactly one field-table row.
 * - Sensitivity: flipping the first and the last byte of each serialized row changes that row's
 *   domain digest and the composite and leaves the other domains' preimages identical; flipping a
 *   declared padding byte changes nothing.
 * - Refusals: bad domain and NULL out (BAD_ARG), NULL or destroyed handle (BAD_HANDLE), with no
 *   write to the instance or out.
 * - Stepped literals (plan 04-05): the composite and domain 1..4 digests after content A, reset and
 *   100 units, recorded from the Phase 3 code before the observation rows were added.
 * The fresh composite and stepped literals are labelled repeatability only: they pin the value,
 * not its truth. */
#include <stdint.h>
#include <stdlib.h>
#include <string.h>

#include "common/ivs_sha256.h"
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

#define MD_PROFILE "intv-2609-ntsc-family-p1"
#define MD_PRE_MAX 120000u /* larger than every domain preimage (MEM is about 113 KB) */

/* A fresh instance in exact-size 16-aligned heap storage (NULL on failure). */
static struct ivs_machine *md_fresh(void)
{
  ivs_create_options o;
  ivs_machine *m = NULL;
  uint8_t *st = aligned_alloc(16u, IVS_MACHINE_BYTES);
  if (st == NULL) {
    return NULL;
  }
  memset(&o, 0, sizeof o);
  o.struct_size = (uint32_t)sizeof o;
  o.profile_id = MD_PROFILE;
  if (ivs_machine_create(&o, st, IVS_MACHINE_BYTES, &m) != IVS_OK) {
    free(st);
    return NULL;
  }
  return m;
}

/* ---- literal streams ---- */

/* INSTANCE preimage of a fresh instance: tag, domain 4, version 1, the profile id (u32 length 24
 * and its ASCII), phase OFF (u32 0), machine time 0 (u64). */
static const uint8_t md_inst_stream[47] = {
    0x49, 0x56, 0x53, 0x44, 0x04, 0x00, 0x01, 0x00, 0x00, 0x00, 0x18, 'i',  'n',  't',  'v',  '-',
    '2',  '6',  '0',  '9',  '-',  'n',  't',  's',  'c',  '-',  'f',  'a',  'm',  'i',  'l',  'y',
    '-',  'p',  '1',  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
};
static const uint8_t md_inst_sha[32] = {
    0x20, 0x3d, 0xc1, 0x62, 0x45, 0x98, 0x69, 0x87, 0xed, 0xf9, 0x2e, 0x32, 0x1a, 0x73, 0x1a, 0x5e,
    0xcd, 0x0b, 0xf4, 0x1e, 0x8a, 0xf5, 0x96, 0xaa, 0x9a, 0xe5, 0x55, 0x3c, 0x6c, 0xd5, 0xc2, 0x69,
};

/* CPU preimage of a fresh instance: tag, domain 1, version 1, u32 count 8 and eight zero
 * registers, then the eight zero status and flag bytes (the reserved bytes are not serialized). */
static const uint8_t md_cpu_stream[35] = {
    0x49, 0x56, 0x53, 0x44, 0x01, 0x00, 0x01, 0x00, 0x00, 0x00, 0x08, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00,
};
static const uint8_t md_cpu_sha[32] = {
    0x22, 0xeb, 0xa1, 0x26, 0x6d, 0xd6, 0x2a, 0x7e, 0x81, 0xcf, 0x15, 0x19, 0x62, 0x05, 0xb3, 0xa1,
    0x99, 0x9d, 0x85, 0x42, 0x2e, 0xb0, 0x5e, 0x69, 0xe5, 0xcf, 0xce, 0xdd, 0x91, 0x6f, 0xdf, 0xcd,
};

/* CPU preimage of a synthetic state: R0..R7 = A000, A111, ..., A777, status C3, then 11, 22, 33,
 * 44, 55, 66, 77 for intff, dbd, prev_interruptible, run_state, halt_pending, fault_reason, busak;
 * reserved bytes EE (not serialized). Every register byte pair differs, so a byte-order slip shows. */
static const uint8_t md_cpus_stream[35] = {
    0x49, 0x56, 0x53, 0x44, 0x01, 0x00, 0x01, 0x00, 0x00, 0x00, 0x08, 0xa0, 0x00, 0xa1, 0x11, 0xa2,
    0x22, 0xa3, 0x33, 0xa4, 0x44, 0xa5, 0x55, 0xa6, 0x66, 0xa7, 0x77, 0xc3, 0x11, 0x22, 0x33, 0x44,
    0x55, 0x66, 0x77,
};
static const uint8_t md_cpus_sha[32] = {
    0x9c, 0x07, 0x7f, 0x62, 0x68, 0x04, 0x96, 0xd2, 0x7c, 0x88, 0x37, 0x9d, 0x9a, 0x9f, 0x5d, 0x89,
    0xec, 0xc3, 0x50, 0x0f, 0xfb, 0x19, 0x47, 0x09, 0xe7, 0xfd, 0x58, 0x3a, 0xff, 0xdd, 0xe4, 0xb1,
};

/* Preimage of domain equals stream, and the public domain digest equals sha. */
static int md_stream_check(const struct ivs_machine *m, uint32_t domain, const uint8_t *stream, size_t n,
                           const uint8_t sha[32])
{
  uint8_t buf[64];
  uint8_t got[32];
  size_t len = ivs_machine_digest_preimage(m, domain, buf, sizeof buf);
  IVS_CHECK(len == n);
  IVS_CHECK(memcmp(buf, stream, n) == 0);
  IVS_CHECK(ivs_machine_state_digest(m, domain, got) == IVS_OK);
  IVS_CHECK(memcmp(got, sha, 32u) == 0);
  return IVS_TEST_PASS;
}

static int md_stream_instance(void)
{
  int rc;
  struct ivs_machine *m = md_fresh();
  IVS_CHECK(m != NULL);
  rc = md_stream_check(m, IVS_DIGEST_INSTANCE, md_inst_stream, sizeof md_inst_stream, md_inst_sha);
  free(m);
  return rc;
}

static int md_stream_cpu(void)
{
  int rc;
  struct ivs_machine *m = md_fresh();
  IVS_CHECK(m != NULL);
  rc = md_stream_check(m, IVS_DIGEST_CPU, md_cpu_stream, sizeof md_cpu_stream, md_cpu_sha);
  free(m);
  return rc;
}

static int md_stream_cpu_synthetic(void)
{
  int rc;
  uint32_t i;
  struct ivs_machine *m = md_fresh();
  IVS_CHECK(m != NULL);
  for (i = 0u; i < 8u; i++) {
    m->cpu.r[i] = (uint16_t)(0xA000u + 0x0111u * i);
  }
  m->cpu.status = 0xC3u;
  m->cpu.intff = 0x11u;
  m->cpu.dbd = 0x22u;
  m->cpu.prev_interruptible = 0x33u;
  m->cpu.run_state = 0x44u;
  m->cpu.halt_pending = 0x55u;
  m->cpu.fault_reason = 0x66u;
  m->cpu.busak = 0x77u;
  memset(m->cpu.reserved, 0xEE, sizeof m->cpu.reserved);
  rc = md_stream_check(m, IVS_DIGEST_CPU, md_cpus_stream, sizeof md_cpus_stream, md_cpus_sha);
  free(m);
  return rc;
}

/* The SHA-256 of each domain preimage (hashed here) equals the public domain digest, so the
 * preimage seam and the hash path serialize the same bytes. */
static int md_preimage_hash(void)
{
  uint32_t d;
  int ok = 1;
  struct ivs_machine *m = md_fresh();
  uint8_t *buf = malloc(MD_PRE_MAX);
  if (m == NULL || buf == NULL) {
    free(m);
    free(buf);
    return IVS_TEST_FAIL;
  }
  for (d = IVS_DIGEST_CPU; ok && d <= IVS_DIGEST_INSTANCE; d++) {
    uint8_t a[32];
    uint8_t b[32];
    size_t len = ivs_machine_digest_preimage(m, d, buf, MD_PRE_MAX);
    ok = len > 7u && len <= MD_PRE_MAX && ivs_machine_state_digest(m, d, a) == IVS_OK;
    if (ok) {
      ivs_sha256_digest(buf, len, b);
      ok = memcmp(a, b, 32u) == 0;
    }
    if (!ok) {
      ivs_test_diag("domain %u", (unsigned)d);
    }
  }
  free(buf);
  free(m);
  return ok ? IVS_TEST_PASS : IVS_TEST_FAIL;
}

/* ---- composite ---- */

/* Rebuild the composite from the four domain digests. */
static int md_composite_framing(void)
{
  static const uint8_t tag[8] = {'I', 'V', 'S', 'S', 'T', 'A', 'T', 'E'};
  uint8_t pre[8 + 2 + 4 * 33];
  uint8_t want[32];
  uint8_t got[32];
  uint32_t d;
  int ok = 1;
  struct ivs_machine *m = md_fresh();
  IVS_CHECK(m != NULL);
  memcpy(pre, tag, 8u);
  pre[8] = 0x00;
  pre[9] = 0x01;
  for (d = 1u; ok && d <= 4u; d++) {
    pre[10u + 33u * (d - 1u)] = (uint8_t)d;
    ok = ivs_machine_state_digest(m, d, &pre[11u + 33u * (d - 1u)]) == IVS_OK;
  }
  ok = ok && ivs_machine_state_digest(m, IVS_DIGEST_COMPOSITE, got) == IVS_OK;
  free(m);
  IVS_CHECK(ok);
  ivs_sha256_digest(pre, sizeof pre, want);
  IVS_CHECK(memcmp(got, want, 32u) == 0);
  return IVS_TEST_PASS;
}

/* Repeatability only (not an accuracy claim): the composite of a fresh instance of the profile.
 * At authoring time an independent rendering of the framing (all four preimages built from the
 * format text and hashed by another SHA-256 implementation) gave the same value. */
static const uint8_t md_fresh_composite[32] = {
    0xac, 0x79, 0x48, 0x21, 0xfa, 0x80, 0xa0, 0x23, 0x0d, 0xb1, 0xd2, 0x1a, 0xaf, 0x86, 0xa7, 0xec,
    0xdc, 0x69, 0x5c, 0xa5, 0x51, 0x5a, 0xa4, 0x2c, 0xeb, 0x8d, 0xc3, 0xe8, 0x5e, 0xa5, 0xd1, 0x37,
};

static int md_fresh_composite_case(void)
{
  uint8_t got[32];
  size_t i;
  int ok;
  struct ivs_machine *m = md_fresh();
  IVS_CHECK(m != NULL);
  ok = ivs_machine_state_digest(m, IVS_DIGEST_COMPOSITE, got) == IVS_OK;
  free(m);
  IVS_CHECK(ok);
  if (memcmp(got, md_fresh_composite, 32u) != 0) {
    char hex[65];
    for (i = 0; i < 32u; i++) {
      static const char xd[] = "0123456789abcdef";
      hex[2u * i] = xd[got[i] >> 4];
      hex[2u * i + 1u] = xd[got[i] & 15u];
    }
    hex[64] = '\0';
    ivs_test_diag("fresh composite %s", hex);
    return IVS_TEST_FAIL;
  }
  return IVS_TEST_PASS;
}

/* Repeatability pin recorded from the Phase 3 code at commit dae4339 (plan 04-05, A-03); tier
 * deterministic-repeatability; not an independent oracle. Content A loaded, power-on reset, 100
 * units through the step seam: the composite (index 0) and the domain 1..4 digests. Recorded
 * before the observation rows (domains 16..18) were added, and unchanged after: the composite
 * stays version 1 over domains 1..4. */
static const uint8_t md_stepped[5][32] = {
    {
        0x90, 0xa2, 0x3b, 0x92, 0x2f, 0x2c, 0x4a, 0xb3, 0xaa, 0x56, 0x85, 0x49, 0x8f, 0x54, 0x0a, 0x6b,
        0xd3, 0x97, 0x0c, 0x1a, 0xeb, 0x7d, 0x9f, 0x1a, 0x9e, 0x3e, 0x5a, 0x7c, 0x01, 0xc1, 0x9e, 0x7f,
    },
    {
        0x88, 0x10, 0x55, 0xb7, 0x67, 0x26, 0x5a, 0x9e, 0xc9, 0x3b, 0x94, 0x3e, 0xd1, 0x9e, 0x54, 0x7f,
        0x37, 0x0c, 0x0d, 0xef, 0x76, 0x47, 0x12, 0xfa, 0x32, 0xa6, 0xe0, 0x6e, 0x68, 0xce, 0x82, 0xab,
    },
    {
        0xea, 0x7b, 0xa6, 0x2a, 0x4f, 0x38, 0xae, 0x70, 0x0e, 0xca, 0x1c, 0xd5, 0x99, 0x86, 0xbf, 0xbd,
        0xe4, 0x0e, 0x27, 0x81, 0x16, 0xbf, 0x20, 0x9e, 0x06, 0x3a, 0x29, 0xbb, 0xe1, 0x80, 0x1f, 0x95,
    },
    {
        0x3f, 0x42, 0x08, 0x2e, 0xef, 0x76, 0xbb, 0xfb, 0xea, 0x05, 0xc9, 0x64, 0xfc, 0xf4, 0x7a, 0x2d,
        0xd0, 0x89, 0xd9, 0x05, 0x0e, 0xe8, 0x5c, 0x2f, 0xc3, 0x28, 0xb0, 0xca, 0x83, 0x99, 0x5d, 0xfe,
    },
    {
        0xfc, 0xa8, 0xe6, 0x2d, 0xd7, 0xd0, 0x5d, 0xed, 0xdd, 0x29, 0xc9, 0x3f, 0x51, 0xec, 0xfa, 0x25,
        0xf5, 0x5d, 0x34, 0xbd, 0x72, 0x70, 0xbe, 0x8a, 0xcc, 0x27, 0x7f, 0xf9, 0x84, 0xeb, 0x3c, 0xcb,
    },
};

static int md_stepped_literals(void)
{
  static const char xd[] = "0123456789abcdef";
  ivs_ms_inst x;
  uint8_t got[32];
  uint32_t d;
  size_t i;
  int ok = 1;
  IVS_CHECK(ivs_ms_powered(&x, IVS_MS_CONTENT_A, 0x5Au));
  if (!ivs_ms_step(x.m, 100u, NULL)) {
    ivs_ms_free(&x);
    return IVS_TEST_FAIL;
  }
  for (d = 0u; d <= 4u; d++) {
    if (ivs_machine_state_digest(x.m, d, got) != IVS_OK) {
      ivs_test_diag("domain %u refused", (unsigned)d);
      ok = 0;
      continue;
    }
    if (memcmp(got, md_stepped[d], 32u) != 0) {
      char hex[65];
      for (i = 0; i < 32u; i++) {
        hex[2u * i] = xd[got[i] >> 4];
        hex[2u * i + 1u] = xd[got[i] & 15u];
      }
      hex[64] = '\0';
      ivs_test_diag("stepped domain %u %s", (unsigned)d, hex);
      ok = 0;
    }
  }
  ivs_ms_free(&x);
  return ok ? IVS_TEST_PASS : IVS_TEST_FAIL;
}

/* ---- coverage ---- */

static int md_coverage(void)
{
  size_t n = ivs_machine_digest_field_count();
  size_t r;
  size_t i;
  size_t total = 0;
  uint32_t per_domain[5] = {0u, 0u, 0u, 0u, 0u};
  uint8_t *seen = calloc(IVS_MACHINE_BYTES, 1u);
  IVS_CHECK(seen != NULL);
  for (r = 0; r < n; r++) {
    const ivs_digest_field *f = ivs_machine_digest_field(r);
    size_t len;
    if (f == NULL || f->name == NULL || f->domain < 1u || f->domain > 4u || f->count == 0u ||
        (f->kind != IVS_DIGEST_KIND_UINT && f->kind != IVS_DIGEST_KIND_PROFILE && f->kind != IVS_DIGEST_KIND_MAGIC &&
         f->kind != IVS_DIGEST_KIND_PAD) ||
        (f->kind == IVS_DIGEST_KIND_UINT && f->width != 1u && f->width != 2u && f->width != 4u && f->width != 8u) ||
        (f->kind == IVS_DIGEST_KIND_PROFILE && (f->width != 4u || f->count != 1u))) {
      ivs_test_diag("row %zu malformed", r);
      free(seen);
      return IVS_TEST_FAIL;
    }
    len = (size_t)f->width * f->count;
    if ((size_t)f->offset + len > IVS_MACHINE_BYTES) {
      ivs_test_diag("row %s past the end", f->name);
      free(seen);
      return IVS_TEST_FAIL;
    }
    for (i = 0; i < len; i++) {
      seen[f->offset + i]++;
    }
    total += len;
    if (f->kind == IVS_DIGEST_KIND_UINT || f->kind == IVS_DIGEST_KIND_PROFILE) {
      per_domain[f->domain]++;
    }
  }
  for (i = 0; i < IVS_MACHINE_BYTES; i++) {
    if (seen[i] != 1u) {
      ivs_test_diag("byte %zu covered %u times", i, (unsigned)seen[i]);
      free(seen);
      return IVS_TEST_FAIL;
    }
  }
  free(seen);
  IVS_CHECK(total == sizeof(struct ivs_machine));
  for (i = 1u; i <= 4u; i++) {
    IVS_CHECK(per_domain[i] > 0u);
  }
  return IVS_TEST_PASS;
}

/* ---- sensitivity ---- */

typedef struct md_base {
  struct ivs_machine *m; /* the instance under change (a copy of a fresh one) */
  uint8_t *pre[5];       /* baseline preimages of domains 1..4 */
  size_t len[5];
  uint8_t dig[5][32]; /* baseline digests, [0] the composite */
  uint8_t *scratch;
} md_base;

static void md_base_free(md_base *b)
{
  uint32_t d;
  for (d = 1u; d <= 4u; d++) {
    free(b->pre[d]);
  }
  free(b->scratch);
  free(b->m);
}

static int md_base_init(md_base *b)
{
  uint32_t d;
  memset(b, 0, sizeof *b);
  b->m = md_fresh();
  b->scratch = malloc(MD_PRE_MAX);
  if (b->m == NULL || b->scratch == NULL) {
    return 0;
  }
  for (d = 0u; d <= 4u; d++) {
    if (ivs_machine_state_digest(b->m, d, b->dig[d]) != IVS_OK) {
      return 0;
    }
    if (d == 0u) {
      continue;
    }
    b->len[d] = ivs_machine_digest_preimage(b->m, d, NULL, 0u);
    b->pre[d] = malloc(b->len[d]);
    if (b->pre[d] == NULL || ivs_machine_digest_preimage(b->m, d, b->pre[d], b->len[d]) != b->len[d]) {
      return 0;
    }
  }
  return 1;
}

/* After a change at instance byte pos: own's digest differs (and the composite when check_comp),
 * every other domain's preimage is identical to the baseline. A changed profile word (seam != 0)
 * damages the handle (review WR-03): the public digest is then BAD_HANDLE, and own's change is
 * judged on its preimage through the seam, which takes no handle. */
static int md_changed_only(md_base *b, uint32_t own, size_t pos, int check_comp, int seam)
{
  uint8_t got[32];
  uint32_t d;
  if (seam) {
    size_t len = ivs_machine_digest_preimage(b->m, own, b->scratch, MD_PRE_MAX);
    if (ivs_machine_state_digest(b->m, own, got) != IVS_E_BAD_HANDLE) {
      ivs_test_diag("byte %zu: a damaged profile word was not BAD_HANDLE", pos);
      return 0;
    }
    if (len == b->len[own] && memcmp(b->scratch, b->pre[own], len) == 0) {
      ivs_test_diag("byte %zu: domain %u preimage unchanged", pos, (unsigned)own);
      return 0;
    }
    check_comp = 0;
  } else if (ivs_machine_state_digest(b->m, own, got) != IVS_OK || memcmp(got, b->dig[own], 32u) == 0) {
    ivs_test_diag("byte %zu: domain %u digest unchanged", pos, (unsigned)own);
    return 0;
  }
  for (d = 1u; d <= 4u; d++) {
    size_t len;
    if (d == own) {
      continue;
    }
    len = ivs_machine_digest_preimage(b->m, d, b->scratch, MD_PRE_MAX);
    if (len != b->len[d] || memcmp(b->scratch, b->pre[d], len) != 0) {
      ivs_test_diag("byte %zu: domain %u changed too", pos, (unsigned)d);
      return 0;
    }
  }
  if (check_comp &&
      (ivs_machine_state_digest(b->m, IVS_DIGEST_COMPOSITE, got) != IVS_OK || memcmp(got, b->dig[0], 32u) == 0)) {
    ivs_test_diag("byte %zu: composite unchanged", pos);
    return 0;
  }
  return 1;
}

/* Flip the first and the last byte of every serialized row of domain. */
static int md_sensitivity(uint32_t domain)
{
  md_base b;
  size_t n = ivs_machine_digest_field_count();
  size_t r;
  uint32_t rows = 0u;
  int ok;
  int seam;
  uint8_t *bytes;
  ok = md_base_init(&b);
  bytes = (uint8_t *)b.m;
  for (r = 0; ok && r < n; r++) {
    const ivs_digest_field *f = ivs_machine_digest_field(r);
    size_t first;
    size_t last;
    if (f->domain != domain || (f->kind != IVS_DIGEST_KIND_UINT && f->kind != IVS_DIGEST_KIND_PROFILE)) {
      continue;
    }
    first = f->offset;
    last = (size_t)f->offset + (size_t)f->width * f->count - 1u;
    seam = f->kind == IVS_DIGEST_KIND_PROFILE;
    bytes[first] ^= 0xFFu;
    ok = md_changed_only(&b, domain, first, 1, seam);
    bytes[first] ^= 0xFFu;
    if (ok && last != first) {
      bytes[last] ^= 0xFFu;
      ok = md_changed_only(&b, domain, last, 0, seam);
      bytes[last] ^= 0xFFu;
    }
    if (!ok) {
      ivs_test_diag("row %s", f->name);
    }
    rows++;
  }
  md_base_free(&b);
  IVS_CHECK(ok);
  IVS_CHECK(rows > 0u);
  return IVS_TEST_PASS;
}

static int md_sensitivity_cpu(void)
{
  return md_sensitivity(IVS_DIGEST_CPU);
}

static int md_sensitivity_mem(void)
{
  return md_sensitivity(IVS_DIGEST_MEM);
}

static int md_sensitivity_images(void)
{
  return md_sensitivity(IVS_DIGEST_IMAGES);
}

static int md_sensitivity_instance(void)
{
  return md_sensitivity(IVS_DIGEST_INSTANCE);
}

/* Flipping a declared padding byte leaves its domain's preimage identical; all padding flipped at
 * once leaves every domain digest and the composite unchanged. */
static int md_padding_ignored(void)
{
  md_base b;
  size_t n = ivs_machine_digest_field_count();
  size_t r;
  size_t i;
  uint32_t d;
  uint32_t pads = 0u;
  int ok;
  uint8_t *bytes;
  ok = md_base_init(&b);
  bytes = (uint8_t *)b.m;
  for (r = 0; ok && r < n; r++) {
    const ivs_digest_field *f = ivs_machine_digest_field(r);
    if (f->kind != IVS_DIGEST_KIND_PAD) {
      continue;
    }
    for (i = 0; ok && i < (size_t)f->width * f->count; i++) {
      size_t pos = f->offset + i;
      size_t len;
      bytes[pos] ^= 0xFFu;
      len = ivs_machine_digest_preimage(b.m, f->domain, b.scratch, MD_PRE_MAX);
      ok = len == b.len[f->domain] && memcmp(b.scratch, b.pre[f->domain], len) == 0;
      if (!ok) {
        ivs_test_diag("padding byte %zu (%s) changed domain %u", pos, f->name, (unsigned)f->domain);
      }
      pads++;
    }
  }
  /* Now every padding byte is flipped at once. */
  for (d = 0u; ok && d <= 4u; d++) {
    uint8_t got[32];
    ok = ivs_machine_state_digest(b.m, d, got) == IVS_OK && memcmp(got, b.dig[d], 32u) == 0;
    if (!ok) {
      ivs_test_diag("domain %u changed with all padding flipped", (unsigned)d);
    }
  }
  md_base_free(&b);
  IVS_CHECK(ok);
  IVS_CHECK(pads > 0u);
  return IVS_TEST_PASS;
}

/* ---- refusals ---- */

/* digest(handle, domain, out) expecting want, with out and the instance untouched. */
static int md_refused(int null_handle, int destroyed, uint32_t domain, int null_out, ivs_status want)
{
  struct ivs_machine *m = md_fresh();
  uint8_t *snap = malloc(IVS_MACHINE_BYTES);
  uint8_t out[32];
  uint8_t poison[32];
  ivs_status got;
  int ok;
  if (m == NULL || snap == NULL) {
    free(m);
    free(snap);
    return IVS_TEST_FAIL;
  }
  if (destroyed && ivs_machine_destroy(m) != IVS_OK) {
    free(m);
    free(snap);
    return IVS_TEST_FAIL;
  }
  memcpy(snap, m, IVS_MACHINE_BYTES);
  memset(out, 0xA5, sizeof out);
  memcpy(poison, out, sizeof out);
  got = ivs_machine_state_digest(null_handle ? NULL : m, domain, null_out ? NULL : out);
  ok = got == want && memcmp(out, poison, sizeof out) == 0 && memcmp(snap, m, IVS_MACHINE_BYTES) == 0;
  if (got != want) {
    ivs_test_diag("status %d, want %d", got, want);
  }
  free(snap);
  free(m);
  return ok ? IVS_TEST_PASS : IVS_TEST_FAIL;
}

static int md_refuse_domain_5(void)
{
  return md_refused(0, 0, 5u, 0, IVS_E_BAD_ARG);
}

static int md_refuse_domain_16(void)
{
  return md_refused(0, 0, IVS_DIGEST_RESERVED_VIDEO, 0, IVS_E_BAD_ARG);
}

static int md_refuse_domain_99(void)
{
  return md_refused(0, 0, 99u, 0, IVS_E_BAD_ARG);
}

static int md_refuse_null_out(void)
{
  return md_refused(0, 0, IVS_DIGEST_COMPOSITE, 1, IVS_E_BAD_ARG);
}

static int md_refuse_null_handle(void)
{
  return md_refused(1, 0, IVS_DIGEST_COMPOSITE, 0, IVS_E_BAD_HANDLE);
}

static int md_refuse_destroyed(void)
{
  return md_refused(0, 1, IVS_DIGEST_CPU, 0, IVS_E_BAD_HANDLE);
}

/* A bad domain and a NULL handle: the argument class comes first. */
static int md_refuse_domain_over_handle(void)
{
  return md_refused(1, 0, 99u, 0, IVS_E_BAD_ARG);
}

const ivs_test_case ivs_machine_digest_cases[] = {
    {"machine.digest.composite-framing", md_composite_framing, NULL},
    {"machine.digest.coverage", md_coverage, NULL},
    {"machine.digest.fresh-composite", md_fresh_composite_case, NULL},
    {"machine.digest.padding-ignored", md_padding_ignored, NULL},
    {"machine.digest.preimage-hash", md_preimage_hash, NULL},
    {"machine.digest.sensitivity-cpu", md_sensitivity_cpu, NULL},
    {"machine.digest.sensitivity-images", md_sensitivity_images, NULL},
    {"machine.digest.sensitivity-instance", md_sensitivity_instance, NULL},
    {"machine.digest.sensitivity-mem", md_sensitivity_mem, NULL},
    {"machine.digest.stepped-literals", md_stepped_literals, NULL},
    {"machine.digest.stream-cpu", md_stream_cpu, NULL},
    {"machine.digest.stream-cpu-synthetic", md_stream_cpu_synthetic, NULL},
    {"machine.digest.stream-instance", md_stream_instance, NULL},
    {"machine.refuse.digest.destroyed", md_refuse_destroyed, NULL},
    {"machine.refuse.digest.domain-16", md_refuse_domain_16, NULL},
    {"machine.refuse.digest.domain-5", md_refuse_domain_5, NULL},
    {"machine.refuse.digest.domain-99", md_refuse_domain_99, NULL},
    {"machine.refuse.digest.domain-over-handle", md_refuse_domain_over_handle, NULL},
    {"machine.refuse.digest.null-handle", md_refuse_null_handle, NULL},
    {"machine.refuse.digest.null-out", md_refuse_null_out, NULL},
};
const size_t ivs_machine_digest_count = sizeof ivs_machine_digest_cases / sizeof ivs_machine_digest_cases[0];

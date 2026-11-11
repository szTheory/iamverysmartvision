/* SPDX-License-Identifier: MIT */
/* Suite ivsc, family fuzz (D-34, plan 03-08; threats T-03-25, T-03-26): a seeded robustness run
 * of the IVSC v1 layer-1 parser beyond the fixed fixtures. Each case ivsc.fuzz.seed-NN is one
 * fixed 32-bit seed driving an in-repo xorshift32 generator; nothing reads a host source of
 * randomness or time, so every run is identical.
 *
 * Per seed FUZZ_ITERS iterations each pick a mutation base (an accepted positive record of the
 * generated fixture table, all project-authored) and apply exactly one mutation:
 *   bit flip    one bit of one byte;
 *   truncate    to a random shorter length (0 included);
 *   grow        1..FUZZ_GROW_MAX random bytes inserted at a random offset (the end included);
 *   field edit  one prefix field (version, flags, required_features, total_bytes,
 *               segment_count, reserved) or one field of one entry (addr, len_words, width,
 *               attr, reserved) set to an edge value: 0, 1, all ones, the original plus or minus
 *               one, one bit flipped, a random value, or a field-specific value (the length plus
 *               or minus one and the maximum plus one for total_bytes, 17 and the counts for
 *               segment_count, another entry's start and the wrap edge for addr).
 * Every second iteration then reseals the trailer (the last 32 bytes become the core SHA-256 of
 * every byte before them), so mutations reach the checks after the digest. The mutant is copied
 * into a heap buffer of exactly its size (NULL for 0 bytes; ASan in the dev preset sees any read
 * past it) and parsed into a poisoned view and report.
 *
 * Invariants checked on every input:
 *   - the status is IVS_OK or one of the IVSC refusal statuses, and a refusal's reason is a
 *     defined reason whose status is the one the spec's reason table maps it to;
 *   - a refusal leaves the view as poisoned, an acceptance leaves the report as poisoned, and
 *     the input bytes are never written;
 *   - a refusal's segment index is IVS_IVSC_NO_SEGMENT or below 16, and its byte offset is at
 *     most the input length (at most 131,256 for too-large);
 *   - parsing the same bytes again gives the identical status, view and report (determinism);
 *   - an acceptance's total_bytes is the input length, and re-encoding the view canonically
 *     reproduces the input exactly.
 * ivsc.fuzz.reasons-covered replays every seed and requires every refusal reason of the spec
 * (1..18) and at least one acceptance to have been observed, so the run is not vacuous. */
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "common/ivs_sha256.h"
#include "content/ivsc.h"
#include "ivs_test.h"
#include "ivsc_fuzz.h"

#define FUZZ_SEEDS 8u
#define FUZZ_ITERS 1500u
#define FUZZ_GROW_MAX 8u
#define FUZZ_BASE_MAX 512u /* longest base taken; longer positive records are skipped */

/* Fails the case with a diagnostic naming the seed, the iteration and the failed expression. */
#define FUZZ_CHECK(f, cond)                                                                            \
  do {                                                                                                 \
    if (!(cond)) {                                                                                     \
      ivs_test_diag("seed=0x%08X iter=%u: check failed at line %d: %s", (unsigned)(f)->seed,          \
                    (unsigned)(f)->iter, __LINE__, #cond);                                             \
      return IVS_TEST_FAIL;                                                                            \
    }                                                                                                  \
  } while (0)

static const uint32_t fuzz_seeds[FUZZ_SEEDS] = {0x6B43A9B5u, 0x1F83D9ABu, 0xA0761D65u, 0x3C6EF372u,
                                                0xE7037ED1u, 0x47B6137Bu, 0x8EBC6AF1u, 0x2D358DCCu};

static const char *const fuzz_ids[IVSC_FUZZ_CASES] = {
    "ivsc.fuzz.seed-01", "ivsc.fuzz.seed-02", "ivsc.fuzz.seed-03", "ivsc.fuzz.seed-04", "ivsc.fuzz.seed-05",
    "ivsc.fuzz.seed-06", "ivsc.fuzz.seed-07", "ivsc.fuzz.seed-08", "ivsc.fuzz.reasons-covered"};

/* The status each reason maps to (spec, "Refusal reasons"); index 0 (none) is unused. */
static const ivs_status fuzz_reason_status[IVS_IVSC_REASON_COUNT] = {
    IVS_OK,                /* none */
    IVS_E_MALFORMED,       /* truncated */
    IVS_E_OVERSIZED,       /* too-large */
    IVS_E_MALFORMED,       /* magic */
    IVS_E_MALFORMED,       /* version */
    IVS_E_MALFORMED,       /* flags */
    IVS_E_MALFORMED,       /* reserved */
    IVS_E_MALFORMED,       /* trailing */
    IVS_E_MALFORMED,       /* segment-count */
    IVS_E_MALFORMED,       /* size-mismatch */
    IVS_E_MALFORMED,       /* segment-length */
    IVS_E_MALFORMED,       /* segment-width */
    IVS_E_MALFORMED,       /* segment-attr */
    IVS_E_MALFORMED,       /* segment-reserved */
    IVS_E_OUT_OF_RANGE,    /* addr-wrap */
    IVS_E_MALFORMED,       /* order */
    IVS_E_OVERLAP,         /* overlap */
    IVS_E_DIGEST_MISMATCH, /* digest */
    IVS_E_MALFORMED,       /* word-range */
};

typedef struct fuzz_ctx {
  uint32_t seed;
  uint32_t x; /* xorshift32 state, never 0 */
  uint32_t iter;
  uint32_t accepted;
  uint32_t seen[IVS_IVSC_REASON_COUNT]; /* refusals per reason */
} fuzz_ctx;

/* xorshift32 (shifts 13, 17, 5): a full-period generator over the nonzero 32-bit values. */
static uint32_t fuzz_next(fuzz_ctx *f)
{
  uint32_t x = f->x;
  x ^= x << 13;
  x ^= x >> 17;
  x ^= x << 5;
  f->x = x;
  return x;
}

/* A value in 0..n-1 (n > 0); the slight modulo bias is irrelevant here. */
static uint32_t fuzz_below(fuzz_ctx *f, uint32_t n)
{
  return fuzz_next(f) % n;
}

static uint32_t fuzz_get(const uint8_t *p, uint32_t w)
{
  uint32_t v = 0u;
  uint32_t k;
  for (k = 0u; k < w; k++) {
    v = (v << 8) | (uint32_t)p[k];
  }
  return v;
}

static void fuzz_put(uint8_t *p, uint32_t w, uint32_t v)
{
  uint32_t k;
  for (k = 0u; k < w; k++) {
    p[w - 1u - k] = (uint8_t)((v >> (8u * k)) & 0xFFu);
  }
}

/* Field kinds of the edit mutation. */
#define FK_PLAIN 0u
#define FK_TOTAL 1u
#define FK_COUNT 2u
#define FK_ADDR 3u

/* An edge value for a field of w bytes holding orig. */
static uint32_t fuzz_value(fuzz_ctx *f, uint32_t w, uint32_t orig, uint32_t kind, const uint8_t *m, size_t len,
                           uint32_t n_ent, uint32_t len_words)
{
  uint32_t ones = w >= 4u ? 0xFFFFFFFFu : ((1u << (8u * w)) - 1u);
  uint32_t pick = fuzz_below(f, kind == FK_PLAIN ? 7u : 9u);
  switch (pick) {
  case 0u:
    return 0u;
  case 1u:
    return 1u;
  case 2u:
    return ones;
  case 3u:
    return (orig + 1u) & ones;
  case 4u:
    return (orig - 1u) & ones;
  case 5u:
    return orig ^ (1u << fuzz_below(f, 8u * w));
  case 6u:
    return fuzz_next(f) & ones;
  default:
    break;
  }
  if (kind == FK_TOTAL) {
    static const int32_t delta[3] = {-1, 1, 0};
    uint32_t d = fuzz_below(f, 3u);
    return d == 2u ? IVS_IVSC_MAX_BYTES + 1u : (uint32_t)((int32_t)len + delta[d]);
  }
  if (kind == FK_COUNT) {
    return pick == 7u ? IVS_IVSC_MAX_SEGMENTS + 1u : fuzz_below(f, IVS_IVSC_MAX_SEGMENTS + 1u);
  }
  /* FK_ADDR: another entry's start (plus 0..2 words), or the wrap edge of this entry. */
  if (pick == 7u && n_ent > 0u) {
    const uint8_t *e = m + IVS_IVSC_PREFIX_BYTES + IVS_IVSC_ENTRY_BYTES * fuzz_below(f, n_ent);
    return (fuzz_get(e, 2u) + fuzz_below(f, 3u)) & 0xFFFFu;
  }
  return (IVS_IVSC_ADDRESS_SPACE_WORDS - len_words + fuzz_below(f, 2u)) & 0xFFFFu;
}

/* One field edit in place: a prefix field, or a field of an entry the buffer holds. */
static void fuzz_edit(fuzz_ctx *f, uint8_t *m, size_t len)
{
  static const uint8_t pre_off[6] = {8u, 10u, 12u, 16u, 20u, 22u};
  static const uint8_t pre_w[6] = {2u, 2u, 4u, 4u, 2u, 2u};
  static const uint8_t ent_off[5] = {0u, 2u, 4u, 5u, 6u};
  static const uint8_t ent_w[5] = {2u, 2u, 1u, 1u, 2u};
  uint32_t n_ent = 0u;
  uint32_t at;
  uint32_t w;
  uint32_t kind = FK_PLAIN;
  uint32_t len_words = 1u;
  if (len >= IVS_IVSC_PREFIX_BYTES) {
    n_ent = fuzz_get(m + 20, 2u);
    if (n_ent > IVS_IVSC_MAX_SEGMENTS) {
      n_ent = IVS_IVSC_MAX_SEGMENTS;
    }
    while (n_ent > 0u && IVS_IVSC_PREFIX_BYTES + IVS_IVSC_ENTRY_BYTES * n_ent > len) {
      n_ent--;
    }
  }
  if (n_ent > 0u && fuzz_below(f, 2u) == 0u) {
    uint32_t e = IVS_IVSC_PREFIX_BYTES + IVS_IVSC_ENTRY_BYTES * fuzz_below(f, n_ent);
    uint32_t k = fuzz_below(f, 5u);
    at = e + ent_off[k];
    w = ent_w[k];
    len_words = fuzz_get(m + e + 2u, 2u);
    kind = k == 0u ? FK_ADDR : FK_PLAIN;
  } else {
    uint32_t k = fuzz_below(f, 6u);
    at = pre_off[k];
    w = pre_w[k];
    kind = k == 3u ? FK_TOTAL : (k == 4u ? FK_COUNT : FK_PLAIN);
  }
  if ((size_t)at + w > len) {
    return; /* the field is not in this (truncated) buffer */
  }
  fuzz_put(m + at, w, fuzz_value(f, w, fuzz_get(m + at, w), kind, m, len, n_ent, len_words));
}

/* One mutation of base into m (capacity FUZZ_BASE_MAX + FUZZ_GROW_MAX); returns the length. */
static size_t fuzz_mutate(fuzz_ctx *f, const uint8_t *base, size_t blen, uint8_t *m)
{
  size_t len = blen;
  memcpy(m, base, blen);
  switch (fuzz_below(f, 4u)) {
  case 0u:
    m[fuzz_below(f, (uint32_t)len)] ^= (uint8_t)(1u << fuzz_below(f, 8u));
    break;
  case 1u:
    len = fuzz_below(f, (uint32_t)len);
    break;
  case 2u: {
    size_t n = 1u + fuzz_below(f, FUZZ_GROW_MAX);
    size_t at = fuzz_below(f, (uint32_t)len + 1u);
    size_t k;
    memmove(m + at + n, m + at, len - at);
    for (k = 0u; k < n; k++) {
      m[at + k] = (uint8_t)(fuzz_next(f) & 0xFFu);
    }
    len += n;
    break;
  }
  default:
    fuzz_edit(f, m, len);
    break;
  }
  return len;
}

/* Parse len bytes at b into a poisoned view and report. */
static ivs_status fuzz_parse(const uint8_t *b, size_t len, ivs_ivsc_view *v, ivs_ivsc_report *r)
{
  memset(v, 0xA5, sizeof *v);
  memset(r, 0x5A, sizeof *r);
  return ivs_ivsc_parse(b, len, v, r);
}

/* Parse one mutant and check every invariant. */
static int fuzz_check(fuzz_ctx *f, const uint8_t *m, size_t len)
{
  uint8_t *b = NULL;
  ivs_ivsc_view v;
  ivs_ivsc_view v2;
  ivs_ivsc_view poison_v;
  ivs_ivsc_report r;
  ivs_ivsc_report r2;
  ivs_ivsc_report poison_r;
  ivs_status st;
  ivs_status st2;
  int ok = 1;

  memset(&poison_v, 0xA5, sizeof poison_v);
  memset(&poison_r, 0x5A, sizeof poison_r);
  if (len > 0u) {
    b = malloc(len);
    FUZZ_CHECK(f, b != NULL);
    memcpy(b, m, len);
  }
  st = fuzz_parse(b, len, &v, &r);
  st2 = fuzz_parse(b, len, &v2, &r2);
  if (st != st2 || memcmp(&v, &v2, sizeof v) != 0 || memcmp(&r, &r2, sizeof r) != 0) {
    ivs_test_diag("not deterministic: status %d then %d", st, st2);
    ok = 0;
  }
  if (len > 0u && memcmp(b, m, len) != 0) {
    ivs_test_diag("the parser wrote its input");
    ok = 0;
  }
  if (st == IVS_OK) {
    f->accepted++;
    if (memcmp(&r, &poison_r, sizeof r) != 0) {
      ivs_test_diag("an acceptance wrote the report");
      ok = 0;
    }
    if (v.total_bytes != len || !ivsc_test_reencode_equal(b, len, &v)) {
      ivs_test_diag("an accepted %zu-byte container does not re-encode to itself", len);
      ok = 0;
    }
  } else if (r.reason == 0u || r.reason >= IVS_IVSC_REASON_COUNT || fuzz_reason_status[r.reason] != st) {
    ivs_test_diag("status %d with reason %u", st, (unsigned)r.reason);
    ok = 0;
  } else {
    f->seen[r.reason]++;
    if (memcmp(&v, &poison_v, sizeof v) != 0) {
      ivs_test_diag("a refusal (reason %u) wrote the view", (unsigned)r.reason);
      ok = 0;
    }
    if (r.segment_index != IVS_IVSC_NO_SEGMENT && r.segment_index >= IVS_IVSC_MAX_SEGMENTS) {
      ivs_test_diag("reason %u: segment index %u", (unsigned)r.reason, (unsigned)r.segment_index);
      ok = 0;
    }
    if (r.byte_offset > (r.reason == IVS_IVSC_R_TOO_LARGE ? (size_t)IVS_IVSC_MAX_BYTES : len)) {
      ivs_test_diag("reason %u: byte offset %u past the %zu input bytes", (unsigned)r.reason,
                    (unsigned)r.byte_offset, len);
      ok = 0;
    }
  }
  free(b);
  FUZZ_CHECK(f, ok);
  return IVS_TEST_PASS;
}

/* Run one seed; the observations accumulate in *f. */
static int fuzz_seed(fuzz_ctx *f, uint32_t seed)
{
  uint8_t m[FUZZ_BASE_MAX + FUZZ_GROW_MAX];
  size_t n_bases = ivsc_test_base_count();
  uint32_t k;
  f->seed = seed;
  f->x = seed != 0u ? seed : 1u;
  for (k = 0u; k < FUZZ_ITERS; k++) {
    const uint8_t *base = NULL;
    size_t blen = 0u;
    size_t len;
    f->iter = k;
    FUZZ_CHECK(f, n_bases > 0u && ivsc_test_base(fuzz_below(f, (uint32_t)n_bases), &base, &blen));
    FUZZ_CHECK(f, blen >= IVS_IVSC_MIN_BYTES && blen <= FUZZ_BASE_MAX);
    len = fuzz_mutate(f, base, blen, m);
    if ((k & 1u) == 1u && len >= IVS_IVSC_TRAILER_BYTES) {
      ivs_sha256_digest(m, len - IVS_IVSC_TRAILER_BYTES, m + len - IVS_IVSC_TRAILER_BYTES);
    }
    if (fuzz_check(f, m, len) != IVS_TEST_PASS) {
      return IVS_TEST_FAIL;
    }
  }
  return IVS_TEST_PASS;
}

static int fuzz_reasons_covered(void)
{
  fuzz_ctx f;
  uint32_t i;
  uint32_t r;
  int ok = 1;
  memset(&f, 0, sizeof f);
  for (i = 0u; i < FUZZ_SEEDS; i++) {
    if (fuzz_seed(&f, fuzz_seeds[i]) != IVS_TEST_PASS) {
      return IVS_TEST_FAIL;
    }
  }
  for (r = 1u; r < IVS_IVSC_REASON_COUNT; r++) {
    if (f.seen[r] == 0u) {
      ivs_test_diag("reason %u was never reached by the %u seeds", (unsigned)r, (unsigned)FUZZ_SEEDS);
      ok = 0;
    }
  }
  if (f.accepted == 0u) {
    ivs_test_diag("no mutant was accepted, so the round-trip invariant never ran");
    ok = 0;
  }
  return ok ? IVS_TEST_PASS : IVS_TEST_FAIL;
}

const char *ivsc_fuzz_case_id(size_t i)
{
  return i < IVSC_FUZZ_CASES ? fuzz_ids[i] : NULL;
}

int ivsc_fuzz_case_run(size_t i)
{
  fuzz_ctx f;
  if (i < FUZZ_SEEDS) {
    memset(&f, 0, sizeof f);
    return fuzz_seed(&f, fuzz_seeds[i]);
  }
  return i == FUZZ_SEEDS ? fuzz_reasons_covered() : IVS_TEST_FAIL;
}

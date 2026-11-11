/* SPDX-License-Identifier: MIT */
/* Suite machine, family sha256 (D-28, A-04, plan 03-02): the core SHA-256 against expected
 * digests that the code under test never produces. Every row of data/sha256/vectors.txt is a
 * case; its literal digest was typed in from `cmake -E sha256sum` and cmake/IvsShaVectors.cmake
 * re-derives it with CMake's string(SHA256) whenever ivs_sha_vectors.inc is generated, failing
 * the build on a mismatch. The invariance cases need no expected digest: splitting the input at
 * any offset, or snapshotting the context by assignment, must not change a result. ctx-plain
 * pins the plain, pointer-free context that a snapshot copy relies on, and init-resets shows
 * that init forgets a context's earlier input. */
#include <stdint.h>
#include <string.h>

#include "common/ivs_sha256.h"
#include "ivs_test.h"
#include "machine_cases.h"

/* Fails the case with a diagnostic naming the line and the failed expression. */
#define IVS_CHECK(cond)                                                                                \
  do {                                                                                                 \
    if (!(cond)) {                                                                                     \
      ivs_test_diag("check failed at line %d: %s", __LINE__, #cond);                                   \
      return IVS_TEST_FAIL;                                                                            \
    }                                                                                                  \
  } while (0)

#define IVS_SHA_SPEC_LIT 1u
#define IVS_SHA_SPEC_PAT 2u
#define IVS_SHA_SPEC_REP 3u

/* The largest pat: message expanded in memory; rep: messages are streamed instead. */
#define SHA_MAX_PAT 8192u
#define SHA_REP_CHUNK 4096u
/* Invariance message length: crosses two block boundaries and the 56-byte padding boundary. */
#define SHA_INV_LEN 130u

typedef struct ivs_sha_vec {
  const char *id;
  uint32_t kind;
  size_t length;
  const char *text; /* lit: bytes, else NULL */
  uint8_t rep;      /* rep: byte, else 0 */
  uint8_t expect[32];
} ivs_sha_vec;

static int ivs_sha_vec_run(size_t i);

#include "ivs_sha_vectors.inc"

/* The pat: alphabet of data/sha256/vectors.txt: byte i is character (i mod 62). */
static const char sha_alphabet[] = "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789";

static uint8_t sha_buf[SHA_MAX_PAT];

/* Fill sha_buf with the first n bytes of the pattern message. */
static void sha_pattern(size_t n)
{
  size_t i;
  for (i = 0; i < n; i++) {
    sha_buf[i] = (uint8_t)sha_alphabet[i % 62u];
  }
}

/* One-shot digest of the first n bytes of the pattern message. */
static void sha_pattern_digest(size_t n, uint8_t out[32])
{
  sha_pattern(n);
  ivs_sha256_digest(sha_buf, n, out);
}

static void sha_ctx_zero(ivs_sha256 *c)
{
  memset(c, 0, sizeof *c);
}

static int ivs_sha_vec_run(size_t i)
{
  const ivs_sha_vec *v = &ivs_sha_vectors[i];
  uint8_t out[32];
  if (v->kind == IVS_SHA_SPEC_LIT) {
    IVS_CHECK(strlen(v->text) == v->length);
    ivs_sha256_digest((const uint8_t *)v->text, v->length, out);
  } else if (v->kind == IVS_SHA_SPEC_PAT) {
    IVS_CHECK(v->length <= SHA_MAX_PAT);
    sha_pattern_digest(v->length, out);
  } else {
    ivs_sha256 c;
    size_t left = v->length;
    IVS_CHECK(v->kind == IVS_SHA_SPEC_REP);
    memset(sha_buf, v->rep, SHA_REP_CHUNK);
    sha_ctx_zero(&c);
    ivs_sha256_init(&c);
    while (left > 0u) {
      size_t n = left < SHA_REP_CHUNK ? left : SHA_REP_CHUNK;
      ivs_sha256_update(&c, sha_buf, n);
      left -= n;
    }
    ivs_sha256_final(&c, out);
  }
  IVS_CHECK(memcmp(out, v->expect, sizeof out) == 0);
  return IVS_TEST_PASS;
}

/* Two update calls split at every offset 0..130 give the one-call digest. */
static int sha_split_every_offset(void)
{
  uint8_t ref[32];
  uint8_t out[32];
  size_t k;
  sha_pattern_digest(SHA_INV_LEN, ref);
  for (k = 0; k <= SHA_INV_LEN; k++) {
    ivs_sha256 c;
    sha_ctx_zero(&c);
    ivs_sha256_init(&c);
    ivs_sha256_update(&c, sha_buf, k);
    ivs_sha256_update(&c, sha_buf + k, SHA_INV_LEN - k);
    ivs_sha256_final(&c, out);
    if (memcmp(out, ref, sizeof out) != 0) {
      ivs_test_diag("split at offset %zu differs from the one-call digest", k);
      return IVS_TEST_FAIL;
    }
  }
  return IVS_TEST_PASS;
}

/* A context copied after k bytes finishes to the digest of those k bytes, and the original then
 * finishes to the digest of the whole message, for every k in 0..130. */
static int sha_ctx_copy_snapshot(void)
{
  uint8_t ref[32];
  uint8_t prefix[32];
  uint8_t out[32];
  size_t k;
  sha_pattern_digest(SHA_INV_LEN, ref);
  for (k = 0; k <= SHA_INV_LEN; k++) {
    ivs_sha256 a;
    ivs_sha256 b;
    sha_pattern_digest(k, prefix); /* rewrites the first k bytes of sha_buf with the same bytes */
    sha_ctx_zero(&a);
    ivs_sha256_init(&a);
    ivs_sha256_update(&a, sha_buf, k);
    b = a;
    IVS_CHECK(memcmp(&a, &b, sizeof a) == 0);
    ivs_sha256_final(&b, out);
    if (memcmp(out, prefix, sizeof out) != 0) {
      ivs_test_diag("snapshot after %zu bytes differs from the digest of those bytes", k);
      return IVS_TEST_FAIL;
    }
    ivs_sha256_update(&a, sha_buf + k, SHA_INV_LEN - k);
    ivs_sha256_final(&a, out);
    if (memcmp(out, ref, sizeof out) != 0) {
      ivs_test_diag("original after a snapshot at %zu bytes differs from the one-call digest", k);
      return IVS_TEST_FAIL;
    }
  }
  return IVS_TEST_PASS;
}

/* The context is plain data: its size is the documented field sum (no implicit padding) and an
 * assignment copies every field. Computes no digest. */
static int sha_ctx_plain(void)
{
  ivs_sha256 a;
  ivs_sha256 b;
  uint32_t i;
  IVS_CHECK(sizeof(ivs_sha256) == IVS_SHA256_CTX_BYTES);
  IVS_CHECK(sizeof a.h + sizeof a.total_bytes + sizeof a.block + sizeof a.used + sizeof a.reserved ==
            IVS_SHA256_CTX_BYTES);
  memset(&a, 0x5A, sizeof a);
  memset(&b, 0x00, sizeof b);
  for (i = 0; i < 8u; i++) {
    a.h[i] = 0x01020304u * (i + 1u);
  }
  a.total_bytes = 0x0102030405060708u;
  a.used = 17u;
  a.reserved = 0u;
  b = a;
  IVS_CHECK(memcmp(&a, &b, sizeof a) == 0);
  return IVS_TEST_PASS;
}

/* init on a context that already took input (with and without a final) starts a fresh
 * message: "abc" then gives the published FIPS 180-4 digest. */
static int sha_init_resets(void)
{
  static const uint8_t abc[3] = {0x61, 0x62, 0x63};
  static const uint8_t abc_digest[32] = {
      0xba, 0x78, 0x16, 0xbf, 0x8f, 0x01, 0xcf, 0xea, 0x41, 0x41, 0x40, 0xde, 0x5d, 0xae, 0x22, 0x23,
      0xb0, 0x03, 0x61, 0xa3, 0x96, 0x17, 0x7a, 0x9c, 0xb4, 0x10, 0xff, 0x61, 0xf2, 0x00, 0x15, 0xad,
  };
  ivs_sha256 c;
  uint8_t out[32];
  sha_ctx_zero(&c);
  ivs_sha256_init(&c);
  ivs_sha256_update(&c, abc, sizeof abc);
  ivs_sha256_init(&c);
  ivs_sha256_update(&c, abc, sizeof abc);
  ivs_sha256_final(&c, out);
  IVS_CHECK(memcmp(out, abc_digest, sizeof out) == 0);
  ivs_sha256_init(&c);
  ivs_sha256_update(&c, abc, sizeof abc);
  ivs_sha256_final(&c, out);
  IVS_CHECK(memcmp(out, abc_digest, sizeof out) == 0);
  return IVS_TEST_PASS;
}

const ivs_test_case ivs_machine_sha256_cases[] = {
    IVS_SHA_VEC_CASES
    {"machine.sha256.ctx-copy-snapshot", sha_ctx_copy_snapshot, NULL},
    {"machine.sha256.ctx-plain", sha_ctx_plain, NULL},
    {"machine.sha256.init-resets", sha_init_resets, NULL},
    {"machine.sha256.split-every-offset", sha_split_every_offset, NULL},
};
const size_t ivs_machine_sha256_count = sizeof ivs_machine_sha256_cases / sizeof ivs_machine_sha256_cases[0];
const ivs_test_source *const ivs_machine_sha256_sources = ivs_sha_sources;
const size_t ivs_machine_sha256_source_count = sizeof ivs_sha_sources / sizeof ivs_sha_sources[0];

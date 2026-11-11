/* SPDX-License-Identifier: MIT */
/* Suite machine, family sha256 (D-28, A-04, plan 03-02): the core SHA-256 against expected
 * digests that the code under test never produces. The FIPS 180-4 "abc" digest is the
 * published value (FIPS 180-4 examples; also what `cmake -E sha256sum` gives for the three
 * bytes "abc"). ctx-plain pins the plain, pointer-free context that a snapshot copy relies on. */
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

/* FIPS 180-4 published digest of "abc". */
static const uint8_t sha_abc_digest[32] = {
    0xba, 0x78, 0x16, 0xbf, 0x8f, 0x01, 0xcf, 0xea, 0x41, 0x41, 0x40, 0xde, 0x5d, 0xae, 0x22, 0x23,
    0xb0, 0x03, 0x61, 0xa3, 0x96, 0x17, 0x7a, 0x9c, 0xb4, 0x10, 0xff, 0x61, 0xf2, 0x00, 0x15, 0xad,
};

static int sha_fips_abc(void)
{
  static const uint8_t msg[3] = {0x61, 0x62, 0x63};
  uint8_t out[32];
  ivs_sha256_digest(msg, sizeof msg, out);
  IVS_CHECK(memcmp(out, sha_abc_digest, sizeof out) == 0);
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

const ivs_test_case ivs_machine_sha256_cases[] = {
    {"machine.sha256.ctx-plain", sha_ctx_plain, NULL},
    {"machine.sha256.fips-abc", sha_fips_abc, NULL},
};
const size_t ivs_machine_sha256_count = sizeof ivs_machine_sha256_cases / sizeof ivs_machine_sha256_cases[0];
const ivs_test_source ivs_machine_sha256_sources[1] = {{NULL, NULL}};
const size_t ivs_machine_sha256_source_count = 0u;

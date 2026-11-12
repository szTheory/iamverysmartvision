/* SPDX-License-Identifier: MIT */
/* Suite psg, family chain (AUD-02, plan 05-06, D-18, T-05-17): the AUDIO digest (domain 17) is a chain over the
 * LEVEL-CHANGE records of the sample stream, indexed by the change count so that it does not depend on call
 * boundaries (the Phase 4 per-record index || len || record framing of per-call data is not reused), with the
 * emitted count and the previous levels bound into the digest preimage so that a longer silent run changes it.
 *   chain' = SHA-256('I' 'V' 'S' 'R' || 17 || chain || be64(count) || be32(12) || be64(k) || A || B || C || 0)
 * where k is the step number of the sample whose three levels differ from the previous sample's (the first sample
 * compares with all-zero levels) and the initial chain is 32 zero bytes.
 *
 * Authoring recipe of the literals (POSIX sh or dash, in a mktemp -d directory; nothing is committed; the same
 * values were cross-checked with a second, independent hash implementation):
 *   oct() { h=$1; while [ -n "$h" ]; do b=${h%"${h#??}"}; h=${h#??}; printf '\\%03o' "0x$b"; done; }
 *   sha() { cmake -E sha256sum "$1" | cut -c1-64; }
 *   Z=0000000000000000000000000000000000000000000000000000000000000000
 *   printf "$(oct "4956535211${Z}0000000000000000""0000000c""0000000000000001""0f000000")" > c1.in; C1=$(sha c1.in)
 *   printf "$(oct "4956535211${C1}0000000000000001""0000000c""0000000000000002""0f030000")" > c2.in; C2=$(sha c2.in)
 * Each 61-byte preimage is "IVSR" (49 56 53 52), the domain byte 17 (0x11), the 32-byte chain, be64 of the count
 * so far, be32 12 and the 12-byte record. The scenario: R7 = $3F (every channel a constant 1) and amplitude A = 15
 * written at stamp 0 give the first change at sample 1 (levels 15, 0, 0), amplitude B = 3 written at stamp 16 gives
 * the second at sample 2 (levels 15, 3, 0).
 *   C1 = after the first record     d5ba7529...1649c291
 *   C2 = after the second record    3e6c4709...f6bcdd
 * These literals check the chain's framing and order, not hardware behaviour.
 *   psg.chain.audio-framing          the chain, count, emitted and previous levels after each of the two records
 *   psg.chain.audio-slicing-invariant  one call and many slices of the TONE guest give equal chain, count, emitted,
 *                                    previous levels and domain-17 digest
 *   psg.chain.emitted-bound          a longer silent run changes the AUDIO digest through the emitted count
 *   psg.chain.reset-clears           a power-on reset returns the row to 32 zero bytes and zero counts */
#include "psg_util.h"

#include "psg_cases.h"

#define CH_CHECK(cond)                                                                                 \
  do {                                                                                                 \
    if (!(cond)) {                                                                                     \
      ivs_test_diag("check failed at line %d: %s", __LINE__, #cond);                                   \
      goto done;                                                                                       \
    }                                                                                                  \
  } while (0)

static const uint8_t ch_c1[32] = {0xd5, 0xba, 0x75, 0x29, 0x8d, 0x0f, 0x35, 0x84, 0xc6, 0xc8, 0x5c,
                                  0x22, 0x5e, 0x7d, 0x7d, 0x44, 0x20, 0xb2, 0xbd, 0xb9, 0x52, 0xe6,
                                  0x42, 0x4e, 0x3a, 0x89, 0x56, 0x65, 0x16, 0x49, 0xc2, 0x91};
static const uint8_t ch_c2[32] = {0x3e, 0x6c, 0x47, 0x09, 0xaa, 0xf9, 0x8a, 0x99, 0xc7, 0x2d, 0x05,
                                  0x48, 0x0b, 0xeb, 0x6e, 0x1d, 0xe7, 0x61, 0x09, 0xa5, 0x28, 0x12,
                                  0xd3, 0x44, 0x01, 0x85, 0xc9, 0x37, 0x84, 0xf6, 0xbc, 0xdd};

static int ch_audio_framing(void)
{
  pu_rig r;
  ivs_psg_sink k;
  static const uint8_t zero[32] = {0};
  if (!pu_init(&r)) {
    return IVS_TEST_FAIL;
  }
  ivs_psg_sink_init(&k, NULL, 0u);
  if (memcmp(r.obs.chain, zero, 32u) != 0 || r.obs.count != 0u) {
    return IVS_TEST_FAIL;
  }
  pu_write(&r, &k, 0u, 8u, 0x3Fu);  /* R7 = $3F: every channel a constant 1 */
  pu_write(&r, &k, 0u, 11u, 15u);   /* amplitude A = 15 */
  pu_run(&r, &k, 16u);              /* sample 1: levels 15, 0, 0 */
  if (r.obs.count != 1u || r.obs.emitted != 1u || r.obs.prev_levels != 0x0000000Fu ||
      memcmp(r.obs.chain, ch_c1, 32u) != 0) {
    ivs_test_diag("after sample 1: count %llu, emitted %llu, chain differs from the literal",
                  (unsigned long long)r.obs.count, (unsigned long long)r.obs.emitted);
    return IVS_TEST_FAIL;
  }
  pu_write(&r, &k, 16u, 12u, 3u);   /* amplitude B = 3, stamped at tick 16: seen from sample 2 */
  pu_run(&r, &k, 48u);              /* samples 2 and 3: levels 15, 3, 0 both times, one change */
  if (r.obs.count != 2u || r.obs.emitted != 3u || r.obs.prev_levels != 0x0000030Fu ||
      memcmp(r.obs.chain, ch_c2, 32u) != 0) {
    ivs_test_diag("after sample 3: count %llu, emitted %llu, chain differs from the literal",
                  (unsigned long long)r.obs.count, (unsigned long long)r.obs.emitted);
    return IVS_TEST_FAIL;
  }
  return IVS_TEST_PASS;
}

static int ch_row_equal(const struct ivs_machine *a, const struct ivs_machine *b)
{
  return memcmp(&a->obs.audio, &b->obs.audio, sizeof a->obs.audio) == 0;
}

static int ch_slicing_invariant(void)
{
  static const uint64_t pats[3][3] = {{1u, 0u, 0u}, {13u, 0u, 0u}, {700u, 7u, 1u}};
  static const uint32_t npat[3] = {1u, 1u, 3u};
  ivs_ms_inst one;
  uint8_t *tmp = malloc(4u * 4500u);
  uint64_t n = 0u;
  uint8_t d1[32];
  uint32_t p;
  int rc = IVS_TEST_FAIL;
  one.storage = NULL;
  one.m = NULL;
  CH_CHECK(tmp != NULL && ivs_ms_powered_prog(&one, IVS_MS_PROG_TONE, 0x5Au));
  CH_CHECK(pu_run_sliced(one.m, 70000u, NULL, 0u, tmp, 4u * 4500u, &n));
  CH_CHECK(((struct ivs_machine *)one.m)->obs.audio.count > 100u); /* many changes: the chain is exercised */
  CH_CHECK(ivs_machine_state_digest(one.m, IVS_DIGEST_AUDIO, d1) == IVS_OK);
  for (p = 0u; p < 3u; p++) {
    ivs_ms_inst x;
    uint8_t dx[32];
    int ok;
    x.storage = NULL;
    x.m = NULL;
    CH_CHECK(ivs_ms_powered_prog(&x, IVS_MS_PROG_TONE, 0x5Au));
    ok = pu_run_sliced(x.m, 70000u, pats[p], npat[p], tmp, 4u * 4500u, &n);
    ok = ok && ch_row_equal((struct ivs_machine *)x.m, (struct ivs_machine *)one.m) &&
         ivs_machine_state_digest(x.m, IVS_DIGEST_AUDIO, dx) == IVS_OK && memcmp(dx, d1, 32u) == 0;
    ivs_ms_free(&x);
    if (!ok) {
      ivs_test_diag("slice pattern %u gives another chain", (unsigned)p);
      goto done;
    }
  }
  rc = IVS_TEST_PASS;
done:
  free(tmp);
  if (one.storage != NULL) {
    ivs_ms_free(&one);
  }
  return rc;
}

static int ch_emitted_bound(void)
{
  ivs_ms_inst a;
  ivs_ms_inst b;
  uint8_t *tmp = malloc(4u * 4500u);
  uint64_t n = 0u;
  uint8_t da[32];
  uint8_t db[32];
  int rc = IVS_TEST_FAIL;
  a.storage = b.storage = NULL;
  a.m = b.m = NULL;
  CH_CHECK(tmp != NULL);
  CH_CHECK(ivs_ms_powered(&a, IVS_MS_CONTENT_A, 0x5Au)); /* program A never touches the PSG: silence */
  CH_CHECK(ivs_ms_powered(&b, IVS_MS_CONTENT_A, 0x5Au));
  CH_CHECK(pu_run_sliced(a.m, 1000u, NULL, 0u, tmp, 4u * 4500u, &n));
  CH_CHECK(pu_run_sliced(b.m, 2000u, NULL, 0u, tmp, 4u * 4500u, &n));
  CH_CHECK(((struct ivs_machine *)a.m)->obs.audio.count == 0u && ((struct ivs_machine *)b.m)->obs.audio.count == 0u);
  CH_CHECK(((struct ivs_machine *)a.m)->obs.audio.emitted < ((struct ivs_machine *)b.m)->obs.audio.emitted);
  CH_CHECK(ivs_machine_state_digest(a.m, IVS_DIGEST_AUDIO, da) == IVS_OK);
  CH_CHECK(ivs_machine_state_digest(b.m, IVS_DIGEST_AUDIO, db) == IVS_OK);
  CH_CHECK(memcmp(da, db, 32u) != 0);
  rc = IVS_TEST_PASS;
done:
  free(tmp);
  if (a.storage != NULL) {
    ivs_ms_free(&a);
  }
  if (b.storage != NULL) {
    ivs_ms_free(&b);
  }
  return rc;
}

static int ch_reset_clears(void)
{
  ivs_ms_inst x;
  ivs_ms_inst fresh;
  uint8_t *tmp = malloc(4u * 4500u);
  uint64_t n = 0u;
  uint8_t dx[32];
  uint8_t df[32];
  static const uint8_t zero[56] = {0};
  int rc = IVS_TEST_FAIL;
  x.storage = fresh.storage = NULL;
  x.m = fresh.m = NULL;
  CH_CHECK(tmp != NULL && ivs_ms_powered_prog(&x, IVS_MS_PROG_TONE, 0x5Au));
  CH_CHECK(ivs_ms_powered_prog(&fresh, IVS_MS_PROG_TONE, 0x5Au));
  CH_CHECK(pu_run_sliced(x.m, 70000u, NULL, 0u, tmp, 4u * 4500u, &n));
  CH_CHECK(((struct ivs_machine *)x.m)->obs.audio.count > 0u);
  CH_CHECK(ivs_machine_reset(x.m, IVS_RESET_POWER_ON) == IVS_OK);
  CH_CHECK(memcmp(&((struct ivs_machine *)x.m)->obs.audio, zero, sizeof zero) == 0);
  CH_CHECK(ivs_machine_state_digest(x.m, IVS_DIGEST_AUDIO, dx) == IVS_OK);
  CH_CHECK(ivs_machine_state_digest(fresh.m, IVS_DIGEST_AUDIO, df) == IVS_OK && memcmp(dx, df, 32u) == 0);
  rc = IVS_TEST_PASS;
done:
  free(tmp);
  if (x.storage != NULL) {
    ivs_ms_free(&x);
  }
  if (fresh.storage != NULL) {
    ivs_ms_free(&fresh);
  }
  return rc;
}

const ivs_test_case ivs_psg_chain_cases[] = {
    {"psg.chain.audio-framing", ch_audio_framing, NULL},
    {"psg.chain.audio-slicing-invariant", ch_slicing_invariant, NULL},
    {"psg.chain.emitted-bound", ch_emitted_bound, NULL},
    {"psg.chain.reset-clears", ch_reset_clears, NULL},
};
const size_t ivs_psg_chain_count = sizeof ivs_psg_chain_cases / sizeof ivs_psg_chain_cases[0];

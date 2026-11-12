/* SPDX-License-Identifier: MIT */
/* Suite psg, family iso and reset (AUD-02, plan 05-08, D-07, D-13, D-18, T-05-20): the write log and its chain are
 * per-instance state (no file-scope carry) and a power-on reset clears them.
 *   psg.iso.interleaved-instances   two instances with different random programs advanced in turn, calls of 700 ticks
 *                                   alternating, equal their sequential runs: delivered log bytes, delivered samples,
 *                                   whole storage and the AUDIO and PSGLOG digests (a file-scope static in the log
 *                                   writer breaks the chains)
 *   psg.reset.log-rows-cleared      a power-on reset after activity returns the PSGLOG row to 40 zero bytes and
 *                                   whole storage and every digest to those of a fresh instance
 *   psg.reset.chains-cleared        both chains (domains 17 and 19) are non-zero after activity and 32 zero bytes with
 *                                   zero counts after a power-on reset */
#include "psg_util.h"

#include "psg_cases.h"

#define IS_CHECK(cond)                                                                                 \
  do {                                                                                                 \
    if (!(cond)) {                                                                                     \
      ivs_test_diag("check failed at line %d: %s", __LINE__, #cond);                                   \
      goto done;                                                                                       \
    }                                                                                                  \
  } while (0)

#define IS_POISON 0x5Au
#define IS_CALL 700u
#define IS_CALLS 60u
#define IS_A_CAP (4u * 3000u)
#define IS_L_CAP (16u * 6000u)

static int is_random(ivs_ms_inst *x, uint32_t seed)
{
  uint16_t w[5u * 160u + 4u];
  size_t n = pu_prog_random(w, seed, 160u);
  return ivs_ms_powered_words(x, w, n, IS_POISON);
}

/* One call of IS_CALL ticks with exact-capacity buffers, appending the delivered bytes. 1 on success. */
static int is_call(ivs_machine *m, uint8_t *a, uint64_t *an, uint8_t *l, uint64_t *ln)
{
  uint8_t sa[3u * 15060u];
  uint8_t sl[3u * 34432u];
  ivs_run_request q;
  ivs_run_report rep;
  uint64_t now = pu_now(m);
  memset(&q, 0, sizeof q);
  q.struct_size = (uint32_t)sizeof q;
  q.target_ticks = now + IS_CALL;
  q.audio = sa;
  q.audio_capacity = pu_need(m, now, q.target_ticks);
  q.psg_log = sl;
  q.psg_log_capacity = pu_log_need(m, now, q.target_ticks);
  memset(&rep, 0, sizeof rep);
  rep.struct_size = (uint32_t)sizeof rep;
  if (ivs_machine_run(m, &q, &rep) != IVS_OK || rep.stop_reason != IVS_STOP_TARGET || *an + rep.audio_bytes_written > IS_A_CAP ||
      *ln + rep.psg_log_bytes_written > IS_L_CAP) {
    return 0;
  }
  memcpy(a + *an, sa, (size_t)rep.audio_bytes_written);
  memcpy(l + *ln, sl, (size_t)rep.psg_log_bytes_written);
  *an += rep.audio_bytes_written;
  *ln += rep.psg_log_bytes_written;
  return 1;
}

static int is_interleaved(void)
{
  ivs_ms_inst a;
  ivs_ms_inst b;
  ivs_ms_inst sa;
  ivs_ms_inst sb;
  uint8_t *buf = malloc(4u * (IS_A_CAP + IS_L_CAP));
  uint8_t *aa;
  uint8_t *al;
  uint8_t *ba;
  uint8_t *bl;
  uint8_t *sa_a;
  uint8_t *sa_l;
  uint8_t *sb_a;
  uint8_t *sb_l;
  uint64_t n[8] = {0u, 0u, 0u, 0u, 0u, 0u, 0u, 0u};
  uint32_t i;
  uint8_t d1[32];
  uint8_t d2[32];
  int rc = IVS_TEST_FAIL;
  a.storage = b.storage = sa.storage = sb.storage = NULL;
  a.m = b.m = sa.m = sb.m = NULL;
  IS_CHECK(buf != NULL);
  aa = buf;
  al = aa + IS_A_CAP;
  ba = al + IS_L_CAP;
  bl = ba + IS_A_CAP;
  sa_a = bl + IS_L_CAP;
  sa_l = sa_a + IS_A_CAP;
  sb_a = sa_l + IS_L_CAP;
  sb_l = sb_a + IS_A_CAP;
  IS_CHECK(is_random(&a, 0x94D049BBu) && is_random(&b, 0x5851F42Du) && is_random(&sa, 0x94D049BBu) &&
           is_random(&sb, 0x5851F42Du));
  /* the two instances in turn */
  for (i = 0u; i < IS_CALLS; i++) {
    IS_CHECK(is_call(a.m, aa, &n[0], al, &n[1]));
    IS_CHECK(is_call(b.m, ba, &n[2], bl, &n[3]));
  }
  /* each alone */
  for (i = 0u; i < IS_CALLS; i++) {
    IS_CHECK(is_call(sa.m, sa_a, &n[4], sa_l, &n[5]));
  }
  for (i = 0u; i < IS_CALLS; i++) {
    IS_CHECK(is_call(sb.m, sb_a, &n[6], sb_l, &n[7]));
  }
  IS_CHECK(n[1] >= 16u * 100u && n[3] >= 16u * 100u && n[1] != n[3]); /* both log, with different programs */
  IS_CHECK(n[0] == n[4] && n[1] == n[5] && n[2] == n[6] && n[3] == n[7]);
  IS_CHECK(memcmp(aa, sa_a, (size_t)n[0]) == 0 && memcmp(al, sa_l, (size_t)n[1]) == 0);
  IS_CHECK(memcmp(ba, sb_a, (size_t)n[2]) == 0 && memcmp(bl, sb_l, (size_t)n[3]) == 0);
  IS_CHECK(memcmp(a.storage, sa.storage, IVS_MACHINE_BYTES) == 0 && memcmp(b.storage, sb.storage, IVS_MACHINE_BYTES) == 0);
  IS_CHECK(ivs_machine_state_digest(a.m, IVS_DIGEST_PSGLOG, d1) == IVS_OK &&
           ivs_machine_state_digest(sa.m, IVS_DIGEST_PSGLOG, d2) == IVS_OK && memcmp(d1, d2, 32u) == 0);
  IS_CHECK(ivs_machine_state_digest(b.m, IVS_DIGEST_PSGLOG, d1) == IVS_OK &&
           ivs_machine_state_digest(sb.m, IVS_DIGEST_PSGLOG, d2) == IVS_OK && memcmp(d1, d2, 32u) == 0);
  IS_CHECK(ivs_machine_state_digest(a.m, IVS_DIGEST_AUDIO, d1) == IVS_OK &&
           ivs_machine_state_digest(sa.m, IVS_DIGEST_AUDIO, d2) == IVS_OK && memcmp(d1, d2, 32u) == 0);
  rc = IVS_TEST_PASS;
done:
  free(buf);
  if (a.storage != NULL) {
    ivs_ms_free(&a);
  }
  if (b.storage != NULL) {
    ivs_ms_free(&b);
  }
  if (sa.storage != NULL) {
    ivs_ms_free(&sa);
  }
  if (sb.storage != NULL) {
    ivs_ms_free(&sb);
  }
  return rc;
}

/* An instance with activity (the random program run for 20,000 ticks), and a fresh twin. 1 on success. */
static int is_active(ivs_ms_inst *x, ivs_ms_inst *fresh)
{
  uint8_t *a = malloc(IS_A_CAP);
  uint8_t *l = malloc(IS_L_CAP);
  uint64_t an = 0u;
  uint64_t ln = 0u;
  uint32_t i;
  int ok = a != NULL && l != NULL && is_random(x, 0x27D4EB2Fu) && is_random(fresh, 0x27D4EB2Fu);
  for (i = 0u; ok && i < 20000u / IS_CALL; i++) {
    ok = is_call(x->m, a, &an, l, &ln);
  }
  free(a);
  free(l);
  return ok && ln >= 16u * 20u;
}

static int is_reset_log_rows(void)
{
  static const uint8_t zero[40] = {0};
  ivs_ms_inst x;
  ivs_ms_inst fresh;
  uint8_t dx[32];
  uint8_t df[32];
  int rc = IVS_TEST_FAIL;
  x.storage = fresh.storage = NULL;
  x.m = fresh.m = NULL;
  IS_CHECK(is_active(&x, &fresh));
  IS_CHECK(((struct ivs_machine *)x.m)->obs.psglog.count > 0u);
  IS_CHECK(ivs_machine_reset(x.m, IVS_RESET_POWER_ON) == IVS_OK);
  IS_CHECK(memcmp(&((struct ivs_machine *)x.m)->obs.psglog, zero, sizeof zero) == 0);
  IS_CHECK(ivs_machine_state_digest(x.m, IVS_DIGEST_PSGLOG, dx) == IVS_OK &&
           ivs_machine_state_digest(fresh.m, IVS_DIGEST_PSGLOG, df) == IVS_OK && memcmp(dx, df, 32u) == 0);
  IS_CHECK(memcmp(x.storage, fresh.storage, IVS_MACHINE_BYTES) == 0);
  rc = IVS_TEST_PASS;
done:
  if (x.storage != NULL) {
    ivs_ms_free(&x);
  }
  if (fresh.storage != NULL) {
    ivs_ms_free(&fresh);
  }
  return rc;
}

static int is_reset_chains(void)
{
  static const uint8_t zero32[32] = {0};
  ivs_ms_inst x;
  ivs_ms_inst fresh;
  struct ivs_machine *im;
  uint8_t dx[32];
  uint8_t df[32];
  int rc = IVS_TEST_FAIL;
  x.storage = fresh.storage = NULL;
  x.m = fresh.m = NULL;
  IS_CHECK(is_active(&x, &fresh));
  im = (struct ivs_machine *)x.m;
  IS_CHECK(im->obs.psglog.count > 0u && memcmp(im->obs.psglog.chain, zero32, 32u) != 0);
  IS_CHECK(im->obs.audio.count > 0u && memcmp(im->obs.audio.chain, zero32, 32u) != 0);
  IS_CHECK(ivs_machine_reset(x.m, IVS_RESET_POWER_ON) == IVS_OK);
  IS_CHECK(im->obs.psglog.count == 0u && memcmp(im->obs.psglog.chain, zero32, 32u) == 0);
  IS_CHECK(im->obs.audio.count == 0u && memcmp(im->obs.audio.chain, zero32, 32u) == 0 && im->obs.audio.emitted == 0u);
  IS_CHECK(ivs_machine_state_digest(x.m, IVS_DIGEST_AUDIO, dx) == IVS_OK &&
           ivs_machine_state_digest(fresh.m, IVS_DIGEST_AUDIO, df) == IVS_OK && memcmp(dx, df, 32u) == 0);
  IS_CHECK(ivs_machine_state_digest(x.m, IVS_DIGEST_PSGLOG, dx) == IVS_OK &&
           ivs_machine_state_digest(fresh.m, IVS_DIGEST_PSGLOG, df) == IVS_OK && memcmp(dx, df, 32u) == 0);
  rc = IVS_TEST_PASS;
done:
  if (x.storage != NULL) {
    ivs_ms_free(&x);
  }
  if (fresh.storage != NULL) {
    ivs_ms_free(&fresh);
  }
  return rc;
}

const ivs_test_case ivs_psg_iso_cases[] = {
    {"psg.iso.interleaved-instances", is_interleaved, NULL},
    {"psg.reset.log-rows-cleared", is_reset_log_rows, NULL},
    {"psg.reset.chains-cleared", is_reset_chains, NULL},
};
const size_t ivs_psg_iso_count = sizeof ivs_psg_iso_cases / sizeof ivs_psg_iso_cases[0];

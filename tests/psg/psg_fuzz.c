/* SPDX-License-Identifier: MIT */
/* Suite psg, family fuzz (AUD-02, API-03 for audio and log, plan 05-08, D-13, D-16, D-25, T-05-20, T-05-21): eight
 * seeded cases of random write programs against random slicing, judged against a one-unit-per-call reference.
 * Each case is one fixed 32-bit seed of the in-repo xorshift32 generator (the seeds of run.fuzz.seed-01..08):
 *   1. the seed builds a project-authored random program (psg_util.h pu_prog_random: 160 tokens of direct and
 *      indirect writes to random PSG registers, fillers and a branch back; no write ever gives the noise period 1, so
 *      a sample is not a level change at every step);
 *   2. instance T runs it to 100,000 ticks in calls whose lengths the generator draws (1 .. 4,000 ticks, one in four
 *      of exactly 1 tick), delivering audio and write log into exact-capacity buffers;
 *   3. instance R runs the same program one unit per call (target now + 1) up to every boundary T reached;
 *   4. at each such boundary: equal time, equal delivered sample bytes, equal delivered log bytes, each call's
 *      audio_first_index and psg_log_first_index the counts delivered before it, equal AUDIO and PSGLOG digests, then
 *      equal whole storage.
 *   psg.fuzz.seed01 .. psg.fuzz.seed08   one seed each
 *   psg.fuzz.cases-covered               a replay of every seed shows that a write lay inside an overshoot window (a
 *                                        record stamped at or after a call's target) and that the log was delivered
 *                                        in many calls, so the cases are not vacuous */
#include "psg_util.h"

#include "psg_cases.h"

#define FZ_TICKS 100000u
#define FZ_TOKENS 160u
#define FZ_A_CAP (4u * (FZ_TICKS / 16u + 256u))
#define FZ_L_CAP (16u * (FZ_TICKS / 28u + 128u))
#define FZ_POISON 0xA5u

static const uint32_t fz_seeds[8] = {0x2545F491u, 0x9E3779B9u, 0x7F4A7C15u, 0xD1B54A32u,
                                     0x94D049BBu, 0x5851F42Du, 0xC2B2AE35u, 0x27D4EB2Fu};

typedef struct fz_cover {
  uint32_t in_window;  /* records stamped at or after the target of the call that delivered them */
  uint32_t log_calls;  /* calls of the slicing instance that delivered at least one record */
  uint32_t records;    /* records delivered in all */
  uint32_t changes;    /* AUDIO level-change records chained (a non-silent run) */
} fz_cover;

/* One call to target with exact-capacity buffers; the delivered bytes are appended to a and l. The checks of the
 * first-index fields are made here. 1 on success. */
typedef struct fz_stream {
  uint8_t *a;
  uint8_t *l;
  uint64_t an;
  uint64_t ln;
  uint8_t *sa; /* scratch, three frame slots */
  uint8_t *sl;
} fz_stream;

static int fz_call(ivs_machine *m, uint64_t target, fz_stream *s, ivs_run_report *rep)
{
  ivs_run_request q;
  uint64_t now = pu_now(m);
  memset(&q, 0, sizeof q);
  q.struct_size = (uint32_t)sizeof q;
  q.target_ticks = target;
  q.audio = s->sa;
  q.audio_capacity = pu_need(m, now, target);
  q.psg_log = s->sl;
  q.psg_log_capacity = pu_log_need(m, now, target);
  if (q.audio_capacity > 3u * 15060u || q.psg_log_capacity > 3u * 34432u) {
    return 0;
  }
  memset(rep, 0, sizeof *rep);
  rep->struct_size = (uint32_t)sizeof *rep;
  if (ivs_machine_run(m, &q, rep) != IVS_OK || rep->stop_reason != IVS_STOP_TARGET ||
      rep->audio_first_index != s->an / 4u || rep->psg_log_first_index != s->ln / 16u ||
      s->an + rep->audio_bytes_written > FZ_A_CAP || s->ln + rep->psg_log_bytes_written > FZ_L_CAP) {
    return 0;
  }
  memcpy(s->a + s->an, s->sa, (size_t)rep->audio_bytes_written);
  memcpy(s->l + s->ln, s->sl, (size_t)rep->psg_log_bytes_written);
  s->an += rep->audio_bytes_written;
  s->ln += rep->psg_log_bytes_written;
  return 1;
}

static int fz_digests_equal(const ivs_machine *a, const ivs_machine *b)
{
  uint8_t da[32];
  uint8_t db[32];
  static const uint32_t doms[2] = {IVS_DIGEST_AUDIO, IVS_DIGEST_PSGLOG};
  uint32_t i;
  for (i = 0u; i < 2u; i++) {
    if (ivs_machine_state_digest(a, doms[i], da) != IVS_OK || ivs_machine_state_digest(b, doms[i], db) != IVS_OK ||
        memcmp(da, db, 32u) != 0) {
      return 0;
    }
  }
  return 1;
}

static int fz_seed(uint32_t seed, fz_cover *cov)
{
  ivs_ms_inst t;
  ivs_ms_inst r;
  fz_stream st;
  fz_stream sr;
  uint16_t w[5u * FZ_TOKENS + 4u];
  size_t nw = pu_prog_random(w, seed, FZ_TOKENS);
  uint32_t g = seed;
  uint32_t calls = 0u;
  int rc = IVS_TEST_FAIL;
  memset(&st, 0, sizeof st);
  memset(&sr, 0, sizeof sr);
  t.storage = r.storage = NULL;
  t.m = r.m = NULL;
  st.a = malloc(FZ_A_CAP);
  st.l = malloc(FZ_L_CAP);
  st.sa = malloc(3u * 15060u);
  st.sl = malloc(3u * 34432u);
  sr.a = malloc(FZ_A_CAP);
  sr.l = malloc(FZ_L_CAP);
  sr.sa = malloc(3u * 15060u);
  sr.sl = malloc(3u * 34432u);
  if (st.a == NULL || st.l == NULL || st.sa == NULL || st.sl == NULL || sr.a == NULL || sr.l == NULL || sr.sa == NULL ||
      sr.sl == NULL || !ivs_ms_powered_words(&t, w, nw, FZ_POISON) || !ivs_ms_powered_words(&r, w, nw, FZ_POISON)) {
    ivs_test_diag("seed=0x%08X: setup failed", (unsigned)seed);
    goto done;
  }
  while (pu_now(t.m) < FZ_TICKS) {
    ivs_run_report rep;
    uint64_t now = pu_now(t.m);
    uint32_t pick = pu_xs(&g);
    uint64_t len = (pick & 3u) == 0u ? 1u : 1u + (pick >> 2) % 4000u;
    uint64_t target = now + len < FZ_TICKS ? now + len : FZ_TICKS;
    uint64_t before_l = st.ln;
    uint64_t i;
    if (!fz_call(t.m, target, &st, &rep)) {
      ivs_test_diag("seed=0x%08X: call %u to %llu failed", (unsigned)seed, (unsigned)calls, (unsigned long long)target);
      goto done;
    }
    calls++;
    if (rep.psg_log_bytes_written != 0u) {
      cov->log_calls++;
    }
    for (i = before_l / 16u; i < st.ln / 16u; i++) {
      if (pu_get_be64(st.l + 16u * i) >= target) {
        cov->in_window++;
      }
    }
    /* the reference: one unit per call up to the same boundary */
    while (pu_now(r.m) < pu_now(t.m)) {
      ivs_run_report rr;
      if (!fz_call(r.m, pu_now(r.m) + 1u, &sr, &rr)) {
        ivs_test_diag("seed=0x%08X: reference call failed", (unsigned)seed);
        goto done;
      }
    }
    if (pu_now(r.m) != pu_now(t.m) || sr.an != st.an || sr.ln != st.ln || memcmp(sr.a, st.a, (size_t)st.an) != 0 ||
        memcmp(sr.l, st.l, (size_t)st.ln) != 0 || !fz_digests_equal(r.m, t.m) ||
        memcmp(r.storage, t.storage, IVS_MACHINE_BYTES) != 0) {
      ivs_test_diag("seed=0x%08X: the sliced run differs from the one-unit-per-call reference at tick %llu (call %u)",
                    (unsigned)seed, (unsigned long long)pu_now(t.m), (unsigned)calls);
      goto done;
    }
  }
  cov->records += (uint32_t)(st.ln / 16u);
  cov->changes += (uint32_t)((struct ivs_machine *)t.m)->obs.audio.count;
  rc = IVS_TEST_PASS;
done:
  free(st.a);
  free(st.l);
  free(st.sa);
  free(st.sl);
  free(sr.a);
  free(sr.l);
  free(sr.sa);
  free(sr.sl);
  if (t.storage != NULL) {
    ivs_ms_free(&t);
  }
  if (r.storage != NULL) {
    ivs_ms_free(&r);
  }
  return rc;
}

static int fz_one(uint32_t i)
{
  fz_cover cov;
  memset(&cov, 0, sizeof cov);
  return fz_seed(fz_seeds[i], &cov);
}

static int fz_seed_01(void) { return fz_one(0u); }
static int fz_seed_02(void) { return fz_one(1u); }
static int fz_seed_03(void) { return fz_one(2u); }
static int fz_seed_04(void) { return fz_one(3u); }
static int fz_seed_05(void) { return fz_one(4u); }
static int fz_seed_06(void) { return fz_one(5u); }
static int fz_seed_07(void) { return fz_one(6u); }
static int fz_seed_08(void) { return fz_one(7u); }

static int fz_cases_covered(void)
{
  fz_cover cov;
  uint32_t i;
  memset(&cov, 0, sizeof cov);
  for (i = 0u; i < 8u; i++) {
    if (fz_seed(fz_seeds[i], &cov) != IVS_TEST_PASS) {
      return IVS_TEST_FAIL;
    }
  }
  ivs_test_diag("covered: %u records, %u calls with a log, %u in an overshoot window, %u audio level changes",
                (unsigned)cov.records, (unsigned)cov.log_calls, (unsigned)cov.in_window, (unsigned)cov.changes);
  if (cov.in_window == 0u || cov.log_calls < 100u || cov.records < 8u * 500u || cov.changes < 8u * 50u) {
    ivs_test_diag("the seeds do not cover a write inside an overshoot window, many log calls or an audible run");
    return IVS_TEST_FAIL;
  }
  return IVS_TEST_PASS;
}

const ivs_test_case ivs_psg_fuzz_cases[] = {
    {"psg.fuzz.seed01", fz_seed_01, NULL}, {"psg.fuzz.seed02", fz_seed_02, NULL},
    {"psg.fuzz.seed03", fz_seed_03, NULL}, {"psg.fuzz.seed04", fz_seed_04, NULL},
    {"psg.fuzz.seed05", fz_seed_05, NULL}, {"psg.fuzz.seed06", fz_seed_06, NULL},
    {"psg.fuzz.seed07", fz_seed_07, NULL}, {"psg.fuzz.seed08", fz_seed_08, NULL},
    {"psg.fuzz.cases-covered", fz_cases_covered, NULL},
};
const size_t ivs_psg_fuzz_count = sizeof ivs_psg_fuzz_cases / sizeof ivs_psg_fuzz_cases[0];

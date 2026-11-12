/* SPDX-License-Identifier: MIT */
/* Suite run, family fuzz (D-12..D-16, A-02, A-04, plan 04-13): a seeded input/slicing fuzz of
 * ivs_machine_run and the v1 sizes of the Phase 4 public structs.
 *
 * run.fuzz.seed-01..08: each case is one fixed 32-bit seed of an in-repo xorshift32 generator
 * (nothing reads a host source of randomness or time, so every run is identical). Over stand-in
 * content A (tests/machine/machine_standin.c, a forever loop; no rights-bearing byte):
 *   1. Boundaries b[0..FZ_UNITS] come from a twin stepped one unit at a time through the internal
 *      step seam, never through the call under test (input is guest-invisible in Phase 4, so the
 *      boundaries do not depend on the schedule).
 *   2. A schedule of 1..32 events, nondecreasing by at_tick: ticks on a boundary, strictly inside
 *      a unit (an overshoot window), anywhere up to past the horizon (those stay unconsumed),
 *      then some ticks set equal to their predecessor; random words with bit 31 clear on the two
 *      connected controllers, words 2 and 3 zero.
 *   3. Reference: a fresh instance run unit by unit (run(now + 1), the unconsumed suffix
 *      resubmitted each call). At every boundary k it must hold the independent expectation (from
 *      b[] and the schedule alone, A-04): consumed = events with at_tick <= b[k - 1] (0 at k = 0),
 *      controller_now = the words of the last of them (0 when none). It records the INPUT and CPU
 *      domain digests there.
 *   4. Slicing: a second fresh instance replays the same schedule with random targets (now + 1 to
 *      now + 48, the tick of one of the next three pending events, a tick strictly inside one of
 *      the next four units, one of the next four boundaries exactly, or now itself), the last
 *      call targeting E inside the last unit, the suffix from input_consumed resubmitted each
 *      call. Every call is accepted, ends on a
 *      reference boundary k, and there equals the reference: total consumed, controller_now, the
 *      INPUT and CPU domain digests, and input_floor_tick = b[k - 1] + 1. The final storage equals
 *      the reference's byte for byte. Calls are capped, so a defect that stops progress fails.
 * run.fuzz.cases-covered replays every seed and requires that at least one event lay inside the
 * overshoot window of a call (target < at_tick < the call's end), one pair of events had equal
 * ticks, one event sat exactly on a call target, some event was applied and some event was still
 * pending at the end (its at_tick past the last unit start; non-vacuity).
 *
 * run.struct.v1-sizes: from plan 04-13 the v1 sizes of the Phase 4 public structs are frozen and
 * the A-02 growth rule applies to them (a new field goes after the v1 size and the minimum
 * struct_size stays the v1 size). Pinned here by offsetof and sizeof literals: the pointer-free
 * ivs_input_event (32), ivs_cpu_state_report (36), ivs_run_report (112) and ivs_info (v1 216,
 * whole 256), the pointer-free prefix of ivs_run_request (video at 16), and every size within
 * IVS_STRUCT_SIZE_MAX (Pitfall 13). run.struct.v1-sizes-lp64: the pointer-bearing ivs_run_request
 * is 64 bytes with events at 48 where pointers are 64-bit; elsewhere a visible skip. */
#include <stddef.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>

#include "ivs/ivs.h"
#include "ivs_test.h"
#include "machine/machine.h"
#include "run_cases.h"

#define FZ_SEEDS 8u
#define FZ_UNITS 96u      /* reference horizon in units */
#define FZ_EVENTS_MAX 32u /* events per schedule at most */
#define FZ_CALLS_MAX 4096u
#define FZ_POISON 0xA5u

/* Fails the seed with a diagnostic naming the seed and the failed expression. */
#define FZ_CHECK(cond)                                                                                 \
  do {                                                                                                 \
    if (!(cond)) {                                                                                     \
      ivs_test_diag("seed=0x%08X: check failed at line %d: %s", (unsigned)seed, __LINE__, #cond);     \
      goto done;                                                                                       \
    }                                                                                                  \
  } while (0)

#define FZ_PIN(cond)                                                                                   \
  do {                                                                                                 \
    if (!(cond)) {                                                                                     \
      ivs_test_diag("v1 size pin failed at line %d: %s", __LINE__, #cond);                             \
      return IVS_TEST_FAIL;                                                                            \
    }                                                                                                  \
  } while (0)

static const uint32_t fz_seeds[FZ_SEEDS] = {0x2545F491u, 0x9E3779B9u, 0x7F4A7C15u, 0xD1B54A32u,
                                            0x94D049BBu, 0x5851F42Du, 0xC2B2AE35u, 0x27D4EB2Fu};

typedef struct fz_cover {
  uint32_t in_window;   /* events with target < at_tick < end of a call that overshot */
  uint32_t equal_ticks; /* adjacent event pairs with one tick */
  uint32_t at_target;   /* pending events whose at_tick was a call's target */
  uint32_t applied;     /* events applied */
  uint32_t left;        /* events still pending after the last call (at_tick past the last unit start) */
} fz_cover;

/* The reference at every boundary (step 3 of the header). */
typedef struct fz_ref {
  uint64_t b[FZ_UNITS + 1u];
  uint32_t consumed[FZ_UNITS + 1u];
  uint32_t words[FZ_UNITS + 1u][4];
  uint8_t input[FZ_UNITS + 1u][32];
  uint8_t cpu[FZ_UNITS + 1u][32];
} fz_ref;

static uint32_t fz_next(uint32_t *s)
{
  uint32_t x = *s;
  x ^= x << 13;
  x ^= x >> 17;
  x ^= x << 5;
  *s = x;
  return x;
}

/* Machine time (info), or UINT64_MAX when refused. */
static uint64_t fz_now(const ivs_machine *m)
{
  ivs_info info;
  memset(&info, 0, sizeof info);
  info.struct_size = (uint32_t)sizeof info;
  if (ivs_machine_info(m, &info) != IVS_OK) {
    return UINT64_MAX;
  }
  return info.time_ticks;
}

/* The INPUT and CPU domain digests of m. 1 on success. */
static int fz_digests(const ivs_machine *m, uint8_t input[32], uint8_t cpu[32])
{
  if (ivs_machine_state_digest(m, IVS_DIGEST_INPUT, input) != IVS_OK) {
    return 0;
  }
  return ivs_machine_state_digest(m, IVS_DIGEST_CPU, cpu) == IVS_OK;
}

/* One accepted run call from ev[0..n) to target; the report is poisoned first. 1 when the call is
 * accepted with no refusal, no failing index and at most n consumed. */
static int fz_run(ivs_machine *m, uint64_t target, const ivs_input_event *ev, uint32_t n, ivs_run_report *rep)
{
  ivs_run_request q;
  ivs_status st;
  memset(&q, 0, sizeof q);
  q.struct_size = (uint32_t)sizeof q;
  q.target_ticks = target;
  q.events = n > 0u ? ev : NULL;
  q.event_count = n;
  memset(rep, FZ_POISON, sizeof *rep);
  rep->struct_size = (uint32_t)sizeof *rep;
  st = ivs_machine_run(m, &q, rep);
  if (st != IVS_OK || rep->refusal_reason != IVS_REASON_NONE || rep->input_failing_index != IVS_INPUT_INDEX_NONE ||
      rep->input_consumed > n) {
    ivs_test_diag("run to %llu with %u events: status %d reason %u index %u consumed %u", (unsigned long long)target,
                  (unsigned)n, (int)st, (unsigned)rep->refusal_reason, (unsigned)rep->input_failing_index,
                  (unsigned)rep->input_consumed);
    return 0;
  }
  return 1;
}

/* Boundaries of content A from a seam-stepped twin. 1 on success. */
static int fz_bounds(uint64_t b[FZ_UNITS + 1u])
{
  ivs_ms_inst y;
  uint32_t i;
  int ok = 0;
  if (!ivs_ms_powered(&y, IVS_MS_CONTENT_A, FZ_POISON)) {
    return 0;
  }
  b[0] = fz_now(y.m);
  for (i = 1u; i <= FZ_UNITS; i++) {
    if (!ivs_ms_step(y.m, 1u, NULL)) {
      goto done;
    }
    b[i] = fz_now(y.m);
    if (b[i] <= b[i - 1u] || b[i] == UINT64_MAX) {
      goto done;
    }
  }
  ok = b[0] == 0u;
done:
  ivs_ms_free(&y);
  return ok;
}

/* A tick strictly inside unit k (b[k] < t < b[k + 1]), or b[k] when the unit is one tick long. */
static uint64_t fz_inside(const uint64_t *b, uint32_t k, uint32_t r)
{
  uint64_t span = b[k + 1u] - b[k];
  return span > 1u ? b[k] + 1u + (uint64_t)r % (span - 1u) : b[k];
}

/* The schedule of step 2. Returns the event count. */
static uint32_t fz_schedule(uint32_t *s, const uint64_t *b, ivs_input_event *ev)
{
  uint32_t n = 1u + fz_next(s) % FZ_EVENTS_MAX;
  uint32_t i;
  for (i = 0u; i < n; i++) {
    uint32_t mode = fz_next(s) % 3u;
    uint32_t k = fz_next(s) % FZ_UNITS;
    uint64_t t;
    uint32_t w0;
    uint32_t w1;
    if (mode == 0u) {
      t = b[k];
    } else if (mode == 1u) {
      t = fz_inside(b, k, fz_next(s));
    } else {
      t = (uint64_t)fz_next(s) % (b[FZ_UNITS] + 512u);
    }
    w0 = fz_next(s) & ~IVS_CTRL_RESERVED_MASK;
    w1 = fz_next(s) & ~IVS_CTRL_RESERVED_MASK;
    memset(&ev[i], 0, sizeof ev[i]);
    ev[i].at_tick = t;
    ev[i].controller[0] = w0;
    ev[i].controller[1] = w1;
  }
  /* Insertion sort by tick (stable), then some ticks equal to their predecessor. */
  for (i = 1u; i < n; i++) {
    ivs_input_event e = ev[i];
    uint32_t j = i;
    while (j > 0u && ev[j - 1u].at_tick > e.at_tick) {
      ev[j] = ev[j - 1u];
      j--;
    }
    ev[j] = e;
  }
  for (i = 1u; i < n; i++) {
    if (fz_next(s) % 4u == 0u) {
      ev[i].at_tick = ev[i - 1u].at_tick;
    }
  }
  return n;
}

/* Events of ev[0..n) with at_tick <= t. */
static uint32_t fz_due(const ivs_input_event *ev, uint32_t n, uint64_t t)
{
  uint32_t c = 0u;
  while (c < n && ev[c].at_tick <= t) {
    c++;
  }
  return c;
}

/* The slicing target of step 4 for a call at boundary index k (b[k] = now). */
static uint64_t fz_target(uint32_t *s, const uint64_t *b, uint32_t k, const ivs_input_event *ev, uint32_t next,
                          uint32_t n, uint64_t end)
{
  uint32_t mode = fz_next(s) % 5u;
  uint32_t ahead = k + 1u + fz_next(s) % 4u;
  uint64_t t;
  if (ahead > FZ_UNITS) {
    ahead = FZ_UNITS;
  }
  if (mode == 0u) {
    t = b[k] + 1u + fz_next(s) % 48u;
  } else if (mode == 1u && next < n) {
    uint32_t pick = n - next < 3u ? n - next : 3u;
    t = ev[next + fz_next(s) % pick].at_tick;
  } else if (mode == 2u && ahead < FZ_UNITS) {
    t = fz_inside(b, ahead, fz_next(s));
  } else if (mode == 3u) {
    t = b[ahead];
  } else {
    t = b[k];
  }
  return t > end ? end : t;
}

/* One seed (header steps 1-4), adding what it covered to *cov. IVS_TEST_PASS or IVS_TEST_FAIL. */
static int fz_seed(uint32_t seed, fz_cover *cov)
{
  fz_ref *ref = NULL;
  ivs_input_event ev[FZ_EVENTS_MAX];
  ivs_ms_inst x;
  ivs_ms_inst y;
  ivs_run_report r;
  uint32_t s = seed;
  uint32_t n;
  uint32_t next;
  uint32_t k;
  uint32_t calls;
  uint64_t end;
  int have_x = 0;
  int have_y = 0;
  int rc = IVS_TEST_FAIL;
  ref = (fz_ref *)calloc(1u, sizeof *ref);
  FZ_CHECK(ref != NULL);
  FZ_CHECK(fz_bounds(ref->b));
  n = fz_schedule(&s, ref->b, ev);
  for (k = 1u; k < n; k++) {
    if (ev[k].at_tick == ev[k - 1u].at_tick) {
      cov->equal_ticks++;
    }
  }
  /* E inside the last unit (or on its end): every call sequence ends at b[FZ_UNITS]. */
  end = ref->b[FZ_UNITS - 1u] + 1u + (uint64_t)fz_next(&s) % (ref->b[FZ_UNITS] - ref->b[FZ_UNITS - 1u]);

  /* Reference, unit by unit, against the independent expectation. */
  FZ_CHECK(ivs_ms_powered(&y, IVS_MS_CONTENT_A, FZ_POISON));
  have_y = 1;
  next = 0u;
  FZ_CHECK(fz_digests(y.m, ref->input[0], ref->cpu[0]));
  for (k = 1u; k <= FZ_UNITS; k++) {
    uint32_t want = fz_due(ev, n, ref->b[k - 1u]);
    uint32_t i;
    FZ_CHECK(fz_run(y.m, ref->b[k - 1u] + 1u, ev + next, n - next, &r));
    next += r.input_consumed;
    FZ_CHECK(r.now_ticks == ref->b[k] && r.units_executed == 1u);
    FZ_CHECK(next == want && r.input_floor_tick == ref->b[k - 1u] + 1u);
    for (i = 0u; i < 4u; i++) {
      uint32_t w = want > 0u ? ev[want - 1u].controller[i] : 0u;
      FZ_CHECK(r.controller_now[i] == w);
    }
    ref->consumed[k] = next;
    memcpy(ref->words[k], r.controller_now, sizeof ref->words[k]);
    FZ_CHECK(fz_digests(y.m, ref->input[k], ref->cpu[k]));
  }

  /* Random slicing against the reference. */
  FZ_CHECK(ivs_ms_powered(&x, IVS_MS_CONTENT_A, FZ_POISON));
  have_x = 1;
  next = 0u;
  k = 0u;
  for (calls = 0u; ref->b[k] < end; calls++) {
    uint64_t target = fz_target(&s, ref->b, k, ev, next, n, end);
    uint8_t input[32];
    uint8_t cpu[32];
    uint32_t i;
    FZ_CHECK(calls < FZ_CALLS_MAX);
    for (i = next; i < n; i++) {
      if (ev[i].at_tick == target) {
        cov->at_target++;
      }
    }
    FZ_CHECK(fz_run(x.m, target, ev + next, n - next, &r));
    next += r.input_consumed;
    cov->applied += r.input_consumed;
    for (i = 0u; i < n; i++) {
      if (r.now_ticks > target && ev[i].at_tick > target && ev[i].at_tick < r.now_ticks) {
        cov->in_window++;
      }
    }
    while (k < FZ_UNITS && ref->b[k] < r.now_ticks) {
      k++;
    }
    FZ_CHECK(ref->b[k] == r.now_ticks);
    FZ_CHECK(next == ref->consumed[k]);
    FZ_CHECK(memcmp(r.controller_now, ref->words[k], sizeof r.controller_now) == 0);
    FZ_CHECK(r.input_floor_tick == (k > 0u ? ref->b[k - 1u] + 1u : 0u));
    FZ_CHECK(fz_digests(x.m, input, cpu));
    FZ_CHECK(memcmp(input, ref->input[k], sizeof input) == 0);
    FZ_CHECK(memcmp(cpu, ref->cpu[k], sizeof cpu) == 0);
  }
  FZ_CHECK(k == FZ_UNITS && next == ref->consumed[FZ_UNITS]);
  cov->left += n - next;
  FZ_CHECK(memcmp(x.storage, y.storage, IVS_MACHINE_BYTES) == 0);
  rc = IVS_TEST_PASS;
done:
  if (have_x) {
    ivs_ms_free(&x);
  }
  if (have_y) {
    ivs_ms_free(&y);
  }
  free(ref);
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
  for (i = 0u; i < FZ_SEEDS; i++) {
    if (fz_seed(fz_seeds[i], &cov) != IVS_TEST_PASS) {
      return IVS_TEST_FAIL;
    }
  }
  if (cov.in_window == 0u || cov.equal_ticks == 0u || cov.at_target == 0u || cov.applied == 0u || cov.left == 0u) {
    ivs_test_diag("vacuous: in-window %u equal-ticks %u at-target %u applied %u left %u", (unsigned)cov.in_window,
                  (unsigned)cov.equal_ticks, (unsigned)cov.at_target, (unsigned)cov.applied, (unsigned)cov.left);
    return IVS_TEST_FAIL;
  }
  return IVS_TEST_PASS;
}

/* ---- v1 struct sizes (A-02, Pitfall 13) ---- */

static int fz_struct_v1(void)
{
  FZ_PIN(sizeof(ivs_input_event) == 32u);
  FZ_PIN(offsetof(ivs_input_event, flags) == 8u && offsetof(ivs_input_event, controller) == 16u);

  FZ_PIN(sizeof(ivs_cpu_state_report) == 36u);
  FZ_PIN(offsetof(ivs_cpu_state_report, r) == 12u && offsetof(ivs_cpu_state_report, s) == 28u);
  FZ_PIN(offsetof(ivs_cpu_state_report, intr_enabled) == 32u && offsetof(ivs_cpu_state_report, reserved) == 33u);

  /* The v1 report is the pinned constant, a prefix of the grown (v2, 136-byte) struct (plan 05-02). */
  FZ_PIN(IVS_RUN_REPORT_V1_SIZE == 112u && offsetof(ivs_run_report, audio_first_index) == IVS_RUN_REPORT_V1_SIZE);
  FZ_PIN(sizeof(ivs_run_report) == 136u);
  FZ_PIN(offsetof(ivs_run_report, reserved) == 20u && offsetof(ivs_run_report, now_ticks) == 24u);
  FZ_PIN(offsetof(ivs_run_report, units_executed) == 48u && offsetof(ivs_run_report, audio_bytes_written) == 72u);
  FZ_PIN(offsetof(ivs_run_report, input_consumed) == 80u && offsetof(ivs_run_report, input_failing_index) == 84u);
  FZ_PIN(offsetof(ivs_run_report, input_floor_tick) == 88u && offsetof(ivs_run_report, controller_now) == 96u);

  FZ_PIN(offsetof(ivs_info, ticks_per_cpu_cycle) == 216u && offsetof(ivs_info, nominal_frame_ticks) == 232u);
  FZ_PIN(sizeof(ivs_info) == 256u);

  /* The pointer-free prefix of the request. */
  FZ_PIN(offsetof(ivs_run_request, flags) == 4u && offsetof(ivs_run_request, target_ticks) == 8u);
  FZ_PIN(offsetof(ivs_run_request, video) == 16u);

  FZ_PIN(sizeof(ivs_run_request) <= IVS_STRUCT_SIZE_MAX && sizeof(ivs_run_report) <= IVS_STRUCT_SIZE_MAX);
  FZ_PIN(sizeof(ivs_cpu_state_report) <= IVS_STRUCT_SIZE_MAX && sizeof(ivs_info) <= IVS_STRUCT_SIZE_MAX);
  return IVS_TEST_PASS;
}

#if UINTPTR_MAX == 0xFFFFFFFFFFFFFFFFu
#define FZ_LP64_SKIP NULL
#else
#define FZ_LP64_SKIP "ivs_run_request holds pointers: its v1 size is pinned for 64-bit pointers only"
#endif

static int fz_struct_v1_lp64(void)
{
  FZ_PIN(sizeof(void *) == 8u);
  FZ_PIN(offsetof(ivs_run_request, video_capacity) == 24u && offsetof(ivs_run_request, audio) == 32u);
  FZ_PIN(offsetof(ivs_run_request, audio_capacity) == 40u && offsetof(ivs_run_request, events) == 48u);
  FZ_PIN(offsetof(ivs_run_request, event_count) == 56u && offsetof(ivs_run_request, reserved_in) == 60u);
  /* The v1 request is the pinned constant 64 (the offset of the first appended field); the grown v2
   * struct is 80 bytes (plan 05-02). */
  FZ_PIN(IVS_RUN_REQUEST_V1_SIZE == 64u && offsetof(ivs_run_request, psg_log) == IVS_RUN_REQUEST_V1_SIZE);
  FZ_PIN(sizeof(ivs_run_request) == 80u);
  return IVS_TEST_PASS;
}

const ivs_test_case ivs_run_fuzz_cases[] = {
    {"run.fuzz.seed-01", fz_seed_01, NULL},
    {"run.fuzz.seed-02", fz_seed_02, NULL},
    {"run.fuzz.seed-03", fz_seed_03, NULL},
    {"run.fuzz.seed-04", fz_seed_04, NULL},
    {"run.fuzz.seed-05", fz_seed_05, NULL},
    {"run.fuzz.seed-06", fz_seed_06, NULL},
    {"run.fuzz.seed-07", fz_seed_07, NULL},
    {"run.fuzz.seed-08", fz_seed_08, NULL},
    {"run.fuzz.cases-covered", fz_cases_covered, NULL},
    {"run.struct.v1-sizes", fz_struct_v1, NULL},
    {"run.struct.v1-sizes-lp64", fz_struct_v1_lp64, FZ_LP64_SKIP},
};

const size_t ivs_run_fuzz_count = sizeof ivs_run_fuzz_cases / sizeof ivs_run_fuzz_cases[0];

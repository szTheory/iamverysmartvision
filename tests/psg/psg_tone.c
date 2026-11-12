/* SPDX-License-Identifier: MIT */
/* Suite psg, family tone (AUD-01, plan 05-06, D-09, D-10, rules PSG-R05..R08): the tone generators of
 * the PSG, and the shared judge of the engine vectors (families tone, mix, amp).
 *
 * Expected values come from the sourced vector files only (data/psg/vectors/tone.txt, mix.txt, amp.txt,
 * turned into C includes by cmake/IvsPsgVectors.cmake, which reads neither the profile nor the core),
 * never from the generated parameter tables or a core output. An engine vector runs at module level, with
 * no guest program: its writes go through ivs_psg_write at the stamp step x 16 master ticks (16 is the
 * literal of the vector grammar), the engine then runs steps chip steps by catching up to steps x 16, and
 * the delivered samples (4 bytes: levels A, B, C, 0) are judged: the number of samples must equal steps (a
 * step of another length fails every vector), edge spacing, toggle count and level sequences are read off
 * the level of the named channel with the level before step 1 taken as 0, and the gate and index vectors
 * evaluate the pure functions.
 *
 * Fixed cases (hand-written ids):
 *   psg.tone.guest-late-period  the tracer's guest check (plan 05-06 Task 1): a stand-in program writes the
 *                               mixer, amplitude A and a period of 1 within the first 300 master ticks,
 *                               after the first chip step reloaded the unprogrammed period 0 as 4096, so by
 *                               the stated timing rule (a period write takes effect at the next reload) the
 *                               first edge is at step 4096 and the edges then follow every step. The units
 *                               are stepped through the internal seam with a sink, the run lasts
 *                               4096 + 2 + 8 steps = 65,536 + 32 + 128 master ticks and more.
 *   psg.judge.tone-row-changed  the predicate rejects an engine vector whose expected spacing was changed
 *                               (and accepts the original) */
#include <stdint.h>
#include <stdlib.h>
#include <string.h>

#include "cpu/cp1610.h"
#include "ivs/ivs.h"
#include "ivs_test.h"
#include "machine/machine.h"
#include "machine_cases.h"
#include "memmap/memsys.h"
#include "psg/psg.h"
#include "psg_cases.h"

#include "ivs_psg_vectors_tone.inc"

#define PT_TICKS_PER_STEP 16u /* the literal of the vector grammar: a write at step N is stamped N x 16 */

static const ivs_psg_params *pt_params(void)
{
  uint32_t p;
  for (p = 0u; p < ivs_profile_count(); p++) {
    if (strcmp(ivs_profile_get(p)->id, IVS_MS_PROFILE) == 0) {
      return ivs_psg_params_get(p);
    }
  }
  return NULL;
}

/* Level of channel c at step k (1-based) in a sample buffer. */
static uint32_t pt_level(const uint8_t *buf, uint32_t k, uint32_t c)
{
  return buf[4u * (k - 1u) + c];
}

int ivs_pe_eval(const ivs_pe *v, int quiet)
{
  const ivs_psg_params *p = pt_params();
  ivs_psg_state s;
  ivs_psg_obs obs;
  ivs_psg_sink sink;
  uint16_t regs[IVS_PSG_REG_COUNT];
  uint8_t *buf = NULL;
  uint32_t k;
  uint32_t changes = 0u;
  uint32_t prev = 0u;
  int ok = 0;
  if (p == NULL) {
    return 0;
  }
  if (v->kind == IVS_PE_GATE) {
    uint32_t got = ivs_psg_mix_channel(p, v->b, v->c, v->a, v->chan);
    if (got != v->n && !quiet) {
      ivs_test_diag("%s: gate is %u, vector expects %u", v->id, (unsigned)got, (unsigned)v->n);
    }
    return got == v->n;
  }
  if (v->kind == IVS_PE_INDEX) {
    uint32_t got = ivs_psg_amp_index(p, v->a, v->b);
    if (got != v->n && !quiet) {
      ivs_test_diag("%s: index is %u, vector expects %u", v->id, (unsigned)got, (unsigned)v->n);
    }
    return got == v->n;
  }
  memset(regs, 0, sizeof regs);
  memset(&obs, 0, sizeof obs);
  ivs_psg_state_reset(&s);
  buf = malloc((size_t)4u * (size_t)v->steps + 4u);
  if (buf == NULL) {
    return 0;
  }
  ivs_psg_sink_init(&sink, buf, (uint64_t)4u * v->steps);
  for (k = 0u; k < v->n_writes; k++) {
    ivs_psg_write(p, &s, regs, &obs, &sink, (uint64_t)v->writes[k].step * PT_TICKS_PER_STEP,
                  (uint32_t)v->writes[k].addr - 0x01F0u, v->writes[k].value);
  }
  ivs_psg_catch_up(p, &s, regs, &obs, (uint64_t)v->steps * PT_TICKS_PER_STEP, &sink);
  if (sink.samples != v->steps || sink.written != (uint64_t)4u * v->steps || sink.overflow != 0u) {
    if (!quiet) {
      ivs_test_diag("%s: %llu samples (%llu bytes) for %u steps", v->id, (unsigned long long)sink.samples,
                    (unsigned long long)sink.written, (unsigned)v->steps);
    }
    goto done;
  }
  if (v->kind == IVS_PE_LEVEL_SEQ) {
    for (k = 1u; k <= v->n_seq; k++) {
      if (pt_level(buf, k, v->chan) != v->seq[k - 1u]) {
        if (!quiet) {
          ivs_test_diag("%s: level at step %u is %u, vector expects %u", v->id, (unsigned)k,
                        (unsigned)pt_level(buf, k, v->chan), (unsigned)v->seq[k - 1u]);
        }
        goto done;
      }
    }
    ok = 1;
    goto done;
  }
  for (k = 1u; k <= v->steps; k++) {
    uint32_t now = pt_level(buf, k, v->chan);
    uint32_t changed = now != prev ? 1u : 0u;
    if (v->kind == IVS_PE_EDGE_SPACING) {
      /* a change exactly at the multiples of n steps, the first at step n */
      uint32_t want = k % v->n == 0u ? 1u : 0u;
      if (changed != want) {
        if (!quiet) {
          ivs_test_diag("%s: level %s at step %u (spacing %u)", v->id, changed != 0u ? "changes" : "holds",
                        (unsigned)k, (unsigned)v->n);
        }
        goto done;
      }
    } else if (k <= v->window) {
      changes += changed;
    }
    prev = now;
  }
  if (v->kind == IVS_PE_TOGGLES && changes != v->n) {
    if (!quiet) {
      ivs_test_diag("%s: %u level changes in %u steps, vector expects %u", v->id, (unsigned)changes,
                    (unsigned)v->window, (unsigned)v->n);
    }
    goto done;
  }
  ok = 1;
done:
  free(buf);
  return ok;
}

/* ---- the guest tracer ---- */

/* Runs program TONE unit by unit through the seam with a sink until machine time reaches min_ticks; the sink
 * holds cap bytes. Returns 1 on success; *samples gets the sink's sample count. */
static int pt_guest_run(uint8_t *buf, uint64_t cap, uint64_t min_ticks, uint64_t *samples)
{
  ivs_ms_inst x;
  ivs_psg_sink sink;
  ivs_cp1610_result r;
  ivs_info info;
  int ok = 0;
  if (!ivs_ms_powered_prog(&x, IVS_MS_PROG_TONE, 0x5Au)) {
    return 0;
  }
  ivs_psg_sink_init(&sink, buf, cap);
  memset(&info, 0, sizeof info);
  info.struct_size = (uint32_t)sizeof info;
  do {
    if (ivs_machine_step_unit_sink_internal(x.m, &r, &sink) != IVS_OK || ivs_machine_info(x.m, &info) != IVS_OK) {
      goto done;
    }
  } while (info.time_ticks < min_ticks);
  *samples = sink.samples;
  ok = sink.overflow == 0u && sink.written == (uint64_t)4u * sink.samples;
done:
  ivs_ms_free(&x);
  return ok;
}

static int pt_guest_late_period(void)
{
  enum { STEPS = 4096 + 2 + 8, CAP_STEPS = 4400 };
  uint8_t *buf = malloc((size_t)4u * CAP_STEPS);
  uint64_t samples = 0u;
  uint32_t k;
  int rc = IVS_TEST_FAIL;
  if (buf == NULL) {
    return IVS_TEST_FAIL;
  }
  memset(buf, 0xA5, (size_t)4u * CAP_STEPS);
  /* 65,536 + 32 P + 128 ticks with P = 1: a little more, so the last unit ends past it. */
  if (!pt_guest_run(buf, (uint64_t)4u * CAP_STEPS, (uint64_t)STEPS * PT_TICKS_PER_STEP, &samples)) {
    ivs_test_diag("the guest run failed or the sink overflowed");
    goto done;
  }
  if (samples < STEPS || samples > CAP_STEPS) {
    ivs_test_diag("%llu samples, want at least %d", (unsigned long long)samples, STEPS);
    goto done;
  }
  /* The registers were 0 at the first reload: the counter held 4096 steps, so channel A is silent until step
   * 4096, where it first reaches level 15; from the reload at step 4097 the period of 1 gives an edge every step. */
  for (k = 1u; k < 4096u; k++) {
    if (pt_level(buf, k, 0u) != 0u) {
      ivs_test_diag("level %u at step %u before the first edge", (unsigned)pt_level(buf, k, 0u), (unsigned)k);
      goto done;
    }
  }
  for (k = 4096u; k <= (uint32_t)STEPS; k++) {
    uint32_t want = ((k - 4096u) % 2u == 0u) ? 15u : 0u;
    if (pt_level(buf, k, 0u) != want) {
      ivs_test_diag("level %u at step %u, want %u", (unsigned)pt_level(buf, k, 0u), (unsigned)k, (unsigned)want);
      goto done;
    }
  }
  for (k = 1u; k <= (uint32_t)STEPS; k++) {
    if (pt_level(buf, k, 1u) != 0u || pt_level(buf, k, 2u) != 0u || buf[4u * (k - 1u) + 3u] != 0u) {
      ivs_test_diag("channels B, C or the reserved byte are not 0 at step %u", (unsigned)k);
      goto done;
    }
  }
  rc = IVS_TEST_PASS;
done:
  free(buf);
  return rc;
}

static int pt_judge_row_changed(void)
{
  ivs_pe v;
  const ivs_pe *src = &ivs_pe_tone[0];
  if (src->kind != IVS_PE_EDGE_SPACING) {
    return IVS_TEST_FAIL;
  }
  if (!ivs_pe_eval(src, 0)) {
    ivs_test_diag("the predicate rejects the real vector %s", src->id);
    return IVS_TEST_FAIL;
  }
  v = *src;
  v.n = src->n + 1u;
  if (ivs_pe_eval(&v, 1)) {
    ivs_test_diag("the predicate accepts %s with its spacing changed", src->id);
    return IVS_TEST_FAIL;
  }
  return IVS_TEST_PASS;
}

const ivs_test_case ivs_psg_tone_cases[] = {
    {"psg.tone.guest-late-period", pt_guest_late_period, NULL},
    {"psg.judge.tone-row-changed", pt_judge_row_changed, NULL},
};
const size_t ivs_psg_tone_count = sizeof ivs_psg_tone_cases / sizeof ivs_psg_tone_cases[0];

size_t ivs_psg_tone_vector_count(void)
{
  return (size_t)IVS_PV_tone_COUNT;
}

const char *ivs_psg_tone_vector_id(size_t i)
{
  return ivs_pe_tone[i].id;
}

int ivs_psg_tone_vector_run(size_t i)
{
  return ivs_pe_eval(&ivs_pe_tone[i], 0) ? IVS_TEST_PASS : IVS_TEST_FAIL;
}

const ivs_test_source *ivs_psg_tone_sources(size_t *n)
{
  *n = 1u;
  return &ivs_pv_sources_tone[0];
}

/* SPDX-License-Identifier: MIT */
/* Suite psg, families env, ampenv and reseng (AUD-01, plan 05-07, D-10, rules PSG-R11..R13, PSG-R17, PSG-R18,
 * PSG-R21): the envelope generator of the PSG (16 shapes, restart on a shape write, the 2 x EP state rate), its
 * use as the amplitude of a channel, and the engine state after a reset.
 *
 * Expected values come from the sourced vector files data/psg/vectors/env.txt (ids psg.env.*, 22 rows),
 * ampenv.txt (psg.ampenv.*, 4) and reset-engine.txt (psg.reseng.*, 2), turned into C includes by
 * cmake/IvsPsgVectors.cmake (which reads neither the profile nor the core), never from the engine. The judge is
 * ivs_pe_eval (psg_tone.c) on the sample stream of a fresh PSG.
 * Phase convention (policy POL-PSG-02, second read docs/psg/second-read-record-env.md): a shape write restarts
 * the generator with an empty step counter, so the sample of the next step shows the first level and the first
 * level change comes 2 x EP steps later (the level of step 1 is 15 for a decay, 0 for an attack; level j of a
 * level-seq is the sample at step 2 x EP x j). That phase is a convention, not a page statement; the restart
 * property and the 2 x EP step rate are what the vectors and the blind reader both pin.
 *
 * Fixed cases (hand-written ids; the sequences are derived by hand from the same shape table as the vectors):
 *   psg.env.period-write-no-restart  a period write after step 6 of a decay at period 1 does not restart it
 *                                    (levels 15,14,14,13,13,12 continue 12,11,11)
 *   psg.env.reset-holds-zero         a generator that never saw a shape write holds level 0 (selected by a channel
 *                                    with both generators disabled it stays silent) and a first shape write after
 *                                    step 10 starts it at step 11
 *   psg.env.shapes-hold-levels       property guard: every shape reaches its documented end state after a long
 *                                    run: continue 0 holds 0, shapes 1001 and 1111 hold 0, shapes 1011 and 1101
 *                                    hold 15, the repeating shapes 1000, 1010, 1100 and 1110 do not hold
 *   psg.env.guest-shape-write-starts a guest program (stand-in ENV) writes the shape register through the machine's
 *                                    bus wrapper: the envelope starts at the write (silence before, then 15, 14,
 *                                    14, 13, ...), so the wrapper and the module-level write share the restart
 *   psg.amp.envelope-drives-level    amplitude selects 3, 2 and 1 of channels A, B and C follow the envelope level
 *                                    as is, shifted right by 1 and by 2 (plan amp.txt index rows)
 *   psg.reseng.reset-state           after a reset the noise register holds the seed (1), the envelope is held at
 *                                    level 0 and every counter and tone output is 0 (the digest-visible state)
 *   psg.judge.env-row-changed        the predicate rejects an envelope sequence and an edge-spacing row whose
 *                                    expected value was changed (and accepts the originals) */
#include <stdint.h>
#include <stdlib.h>
#include <string.h>

#include "ivs/ivs.h"
#include "ivs_test.h"
#include "machine/machine.h"
#include "machine_cases.h"
#include "memmap/memsys.h"
#include "psg/psg.h"
#include "psg_cases.h"

#include "ivs_psg_vectors_ampenv.inc"
#include "ivs_psg_vectors_env.inc"
#include "ivs_psg_vectors_reseng.inc"

#define PE_TICKS_PER_STEP 16u

static const ivs_psg_params *pe_params(void)
{
  uint32_t p;
  for (p = 0u; p < ivs_profile_count(); p++) {
    if (strcmp(ivs_profile_get(p)->id, IVS_MS_PROFILE) == 0) {
      return ivs_psg_params_get(p);
    }
  }
  return NULL;
}

/* A hand-built engine row: writes at steps, a level sequence of one channel. */
static ivs_pe pe_seq_row(const char *id, const ivs_pv_write *w, uint32_t nw, uint32_t steps, uint32_t chan,
                         const uint8_t *seq, uint32_t nseq)
{
  ivs_pe v;
  memset(&v, 0, sizeof v);
  v.id = id;
  v.rule = "PSG-R12";
  v.source = 0u;
  v.writes = w;
  v.n_writes = nw;
  v.steps = steps;
  v.kind = IVS_PE_LEVEL_SEQ;
  v.chan = chan;
  v.seq = seq;
  v.n_seq = nseq;
  return v;
}

static int pe_period_write_no_restart(void)
{
  static const ivs_pv_write w[] = {{0x01F8u, 0x003Fu, 0u}, {0x01FBu, 0x0030u, 0u}, {0x01F3u, 0x0001u, 0u},
                                   {0x01F7u, 0x0000u, 0u}, {0x01FAu, 0x0008u, 0u}, {0x01F3u, 0x0001u, 6u}};
  static const uint8_t seq[] = {15u, 14u, 14u, 13u, 13u, 12u, 12u, 11u, 11u};
  ivs_pe v = pe_seq_row("psg.env.period-write-no-restart", w, 6u, 9u, 0u, seq, 9u);
  return ivs_pe_eval(&v, 0) ? IVS_TEST_PASS : IVS_TEST_FAIL;
}

static int pe_reset_holds_zero(void)
{
  /* 10 steps at level 0 (select 3, envelope held at 0), then a shape write after step 10: step 11 is level 15 */
  static const ivs_pv_write w[] = {{0x01F8u, 0x003Fu, 0u}, {0x01FBu, 0x0030u, 0u}, {0x01F3u, 0x0001u, 0u},
                                   {0x01FAu, 0x0008u, 10u}};
  static const uint8_t seq[] = {0u, 0u, 0u, 0u, 0u, 0u, 0u, 0u, 0u, 0u, 15u, 14u, 14u, 13u};
  ivs_pe v = pe_seq_row("psg.env.reset-holds-zero", w, 4u, 14u, 0u, seq, 14u);
  return ivs_pe_eval(&v, 0) ? IVS_TEST_PASS : IVS_TEST_FAIL;
}

/* Shape value, whether it holds and the level it holds, by hand from the shape table (bits continue, attack,
 * alternate, hold): 00xx level 0; 01xx level 0; 1000 repeats; 1001 holds 0; 1010 repeats; 1011 holds 15; 1100
 * repeats; 1101 holds 15; 1110 repeats; 1111 holds 0. */
static int pe_shapes_hold_levels(void)
{
  static const int8_t hold[16] = {0, 0, 0, 0, 0, 0, 0, 0, -1, 0, -1, 15, -1, 15, -1, 0};
  const ivs_psg_params *p = pe_params();
  uint32_t shape;
  if (p == NULL) {
    return IVS_TEST_FAIL;
  }
  for (shape = 0u; shape < 16u; shape++) {
    ivs_psg_state s;
    ivs_psg_obs obs;
    ivs_psg_sink sink;
    uint16_t regs[IVS_PSG_REG_COUNT];
    memset(regs, 0, sizeof regs);
    memset(&obs, 0, sizeof obs);
    ivs_psg_state_reset(p, &s);
    ivs_psg_sink_init(&sink, NULL, 0u);
    ivs_psg_write(p, &s, regs, &obs, &sink, 0u, 3u, 1u);
    ivs_psg_write(p, &s, regs, &obs, &sink, 0u, 10u, shape);
    ivs_psg_catch_up(p, &s, regs, &obs, 200u * PE_TICKS_PER_STEP, &sink); /* more than five 32-step cycles */
    if (hold[shape] < 0) {
      if (s.env_held != 0u) {
        ivs_test_diag("shape %u holds but is a repeating shape", (unsigned)shape);
        return IVS_TEST_FAIL;
      }
    } else if (s.env_held != 1u || s.env_level != (uint32_t)hold[shape]) {
      ivs_test_diag("shape %u: held %u level %u, documented hold level %d", (unsigned)shape, (unsigned)s.env_held,
                    (unsigned)s.env_level, (int)hold[shape]);
      return IVS_TEST_FAIL;
    }
  }
  return IVS_TEST_PASS;
}

static int pe_guest_shape_write(void)
{
  enum { CAP_STEPS = 400, RUN_STEPS = 160 };
  ivs_ms_inst x;
  uint8_t *buf = malloc((size_t)4u * CAP_STEPS);
  ivs_psg_sink sink;
  ivs_cp1610_result r;
  ivs_info info;
  uint32_t k;
  uint32_t first = 0u;
  int rc = IVS_TEST_FAIL;
  if (buf == NULL) {
    return IVS_TEST_FAIL;
  }
  if (!ivs_ms_powered_prog(&x, IVS_MS_PROG_ENV, 0x5Au)) {
    free(buf);
    return IVS_TEST_FAIL;
  }
  ivs_psg_sink_init(&sink, buf, (uint64_t)4u * CAP_STEPS);
  memset(&info, 0, sizeof info);
  info.struct_size = (uint32_t)sizeof info;
  do {
    if (ivs_machine_step_unit_sink_internal(x.m, &r, &sink) != IVS_OK || ivs_machine_info(x.m, &info) != IVS_OK) {
      goto done;
    }
  } while (info.time_ticks < (uint64_t)RUN_STEPS * PE_TICKS_PER_STEP);
  if (sink.samples < (uint64_t)RUN_STEPS || sink.overflow != 0u) {
    goto done;
  }
  for (k = 1u; k <= RUN_STEPS; k++) {
    if (buf[4u * (k - 1u)] != 0u) {
      first = k;
      break;
    }
  }
  /* the shape write lands within the first 300 ticks (about 19 steps); level 15 is the first sample after it */
  if (first == 0u || first > 30u || buf[4u * (first - 1u)] != 15u) {
    ivs_test_diag("first nonzero level at step %u (%u)", (unsigned)first, (unsigned)(first != 0u ? buf[4u * (first - 1u)] : 0u));
    goto done;
  }
  for (k = first; k < first + 34u; k++) {
    uint32_t r2 = k - first + 1u; /* steps since the restart, 1-based */
    uint32_t want = 15u - r2 / 2u;
    if (k > (uint32_t)RUN_STEPS) {
      goto done;
    }
    if ((uint32_t)buf[4u * (k - 1u)] != want) {
      ivs_test_diag("step %u (%u after the restart): level %u, want %u", (unsigned)k, (unsigned)r2,
                    (unsigned)buf[4u * (k - 1u)], (unsigned)want);
      goto done;
    }
    if (want == 0u) {
      break;
    }
  }
  rc = IVS_TEST_PASS;
done:
  ivs_ms_free(&x);
  free(buf);
  return rc;
}

static int pe_amp_envelope_drives_level(void)
{
  /* shape 1000 at period 1: envelope levels 15,14,14,13,13,12,12,11,11 for steps 1..9; channel A select 3 is the
   * level, B select 2 the level >> 1 (7,7,7,6,6,6,6,5,5), C select 1 the level >> 2 (3,3,3,3,3,3,3,2,2) */
  static const ivs_pv_write w[] = {{0x01F8u, 0x003Fu, 0u}, {0x01FBu, 0x0030u, 0u}, {0x01FCu, 0x0020u, 0u},
                                   {0x01FDu, 0x0010u, 0u}, {0x01F3u, 0x0001u, 0u}, {0x01FAu, 0x0008u, 0u}};
  static const uint8_t a[] = {15u, 14u, 14u, 13u, 13u, 12u, 12u, 11u, 11u};
  static const uint8_t b[] = {7u, 7u, 7u, 6u, 6u, 6u, 6u, 5u, 5u};
  static const uint8_t c[] = {3u, 3u, 3u, 3u, 3u, 3u, 3u, 2u, 2u};
  ivs_pe va = pe_seq_row("psg.amp.envelope-drives-level", w, 6u, 9u, 0u, a, 9u);
  ivs_pe vb = pe_seq_row("psg.amp.envelope-drives-level", w, 6u, 9u, 1u, b, 9u);
  ivs_pe vc = pe_seq_row("psg.amp.envelope-drives-level", w, 6u, 9u, 2u, c, 9u);
  return ivs_pe_eval(&va, 0) && ivs_pe_eval(&vb, 0) && ivs_pe_eval(&vc, 0) ? IVS_TEST_PASS : IVS_TEST_FAIL;
}

static int pe_reseng_reset_state(void)
{
  const ivs_psg_params *p = pe_params();
  ivs_psg_state s;
  if (p == NULL) {
    return IVS_TEST_FAIL;
  }
  memset(&s, 0xA5, sizeof s);
  ivs_psg_state_reset(p, &s);
  /* the seed 1 of the noise vector notes (family notes of noise.txt: "seed 1") */
  if (s.noise_lfsr != 1u || s.noise_counter != 0u || s.env_held != 1u || s.env_level != 0u || s.env_counter != 0u ||
      s.env_cycles != 0u || s.env_dir != 0u || s.tone_out[0] != 0u || s.tone_out[1] != 0u || s.tone_out[2] != 0u ||
      s.tone_counter[0] != 0u || s.tone_counter[1] != 0u || s.tone_counter[2] != 0u || s.next_sample != 1u ||
      s.reserved[0] != 0u || s.reserved[1] != 0u || s.reserved[2] != 0u) {
    ivs_test_diag("the reset state is not the documented one");
    return IVS_TEST_FAIL;
  }
  return IVS_TEST_PASS;
}

static int pe_judge_row_changed(void)
{
  const ivs_pe *seq = NULL;
  const ivs_pe *edge = NULL;
  ivs_pe a;
  ivs_pe b;
  uint8_t changed[64];
  size_t i;
  for (i = 0u; i < (size_t)IVS_PV_env_COUNT; i++) {
    if (strcmp(ivs_pe_env[i].id, "psg.env.shape-0") == 0) {
      seq = &ivs_pe_env[i];
    } else if (strcmp(ivs_pe_env[i].id, "psg.env.rate-ep1") == 0) {
      edge = &ivs_pe_env[i];
    }
  }
  if (seq == NULL || edge == NULL || seq->kind != IVS_PE_LEVEL_SEQ || edge->kind != IVS_PE_EDGE_SPACING ||
      seq->n_seq > sizeof changed) {
    return IVS_TEST_FAIL;
  }
  if (!ivs_pe_eval(seq, 0) || !ivs_pe_eval(edge, 0)) {
    ivs_test_diag("the predicate rejects a real vector");
    return IVS_TEST_FAIL;
  }
  memcpy(changed, seq->seq, seq->n_seq);
  changed[3] = (uint8_t)(changed[3] ^ 1u);
  a = *seq;
  a.seq = changed;
  b = *edge;
  b.n += 1u;
  if (ivs_pe_eval(&a, 1) || ivs_pe_eval(&b, 1)) {
    ivs_test_diag("the predicate accepts a vector with its expectation changed");
    return IVS_TEST_FAIL;
  }
  return IVS_TEST_PASS;
}

const ivs_test_case ivs_psg_env_cases[] = {
    {"psg.env.period-write-no-restart", pe_period_write_no_restart, NULL},
    {"psg.env.reset-holds-zero", pe_reset_holds_zero, NULL},
    {"psg.env.shapes-hold-levels", pe_shapes_hold_levels, NULL},
    {"psg.env.guest-shape-write-starts", pe_guest_shape_write, NULL},
    {"psg.amp.envelope-drives-level", pe_amp_envelope_drives_level, NULL},
    {"psg.judge.env-row-changed", pe_judge_row_changed, NULL},
};
const size_t ivs_psg_env_count = sizeof ivs_psg_env_cases / sizeof ivs_psg_env_cases[0];

size_t ivs_psg_env_vector_count(void)
{
  return (size_t)IVS_PV_env_COUNT;
}

const char *ivs_psg_env_vector_id(size_t i)
{
  return ivs_pe_env[i].id;
}

int ivs_psg_env_vector_run(size_t i)
{
  return ivs_pe_eval(&ivs_pe_env[i], 0) ? IVS_TEST_PASS : IVS_TEST_FAIL;
}

const ivs_test_source *ivs_psg_env_sources(size_t *n)
{
  *n = 1u;
  return &ivs_pv_sources_env[0];
}

/* Family ampenv: only vectors. */
const ivs_test_case ivs_psg_ampenv_cases[] = {{"", NULL, NULL}};
const size_t ivs_psg_ampenv_count = 0u;

size_t ivs_psg_ampenv_vector_count(void)
{
  return (size_t)IVS_PV_ampenv_COUNT;
}

const char *ivs_psg_ampenv_vector_id(size_t i)
{
  return ivs_pe_ampenv[i].id;
}

int ivs_psg_ampenv_vector_run(size_t i)
{
  return ivs_pe_eval(&ivs_pe_ampenv[i], 0) ? IVS_TEST_PASS : IVS_TEST_FAIL;
}

const ivs_test_source *ivs_psg_ampenv_sources(size_t *n)
{
  *n = 1u;
  return &ivs_pv_sources_ampenv[0];
}

/* Family reseng: the vectors of reset-engine.txt and the reset-state case. */
const ivs_test_case ivs_psg_reseng_cases[] = {
    {"psg.reseng.reset-state", pe_reseng_reset_state, NULL},
};
const size_t ivs_psg_reseng_count = sizeof ivs_psg_reseng_cases / sizeof ivs_psg_reseng_cases[0];

size_t ivs_psg_reseng_vector_count(void)
{
  return (size_t)IVS_PV_reseng_COUNT;
}

const char *ivs_psg_reseng_vector_id(size_t i)
{
  return ivs_pe_reseng[i].id;
}

int ivs_psg_reseng_vector_run(size_t i)
{
  return ivs_pe_eval(&ivs_pe_reseng[i], 0) ? IVS_TEST_PASS : IVS_TEST_FAIL;
}

const ivs_test_source *ivs_psg_reseng_sources(size_t *n)
{
  *n = 1u;
  return &ivs_pv_sources_reseng[0];
}

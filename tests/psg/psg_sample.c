/* SPDX-License-Identifier: MIT */
/* Suite psg, family sample (AUD-02, plan 05-06, D-15, D-16, C78): the chip-rate sample stream. One sample per chip
 * step, stamped k x 16 master ticks on an absolute grid from reset, 4 bytes A, B, C, 0 (the effective amplitude
 * index of each channel, IVS_OUTPUT_FORMAT_PSG_LEVELS_V1). The module-level cases drive the engine through
 * ivs_psg_write and ivs_psg_catch_up; a constant level needs no tone, so they set R7 = $3F (tone and noise of every
 * channel disabled: the channel gate is a constant 1) and a fixed amplitude. Expected values are literals written
 * here from the rules of D-15 and C78, never produced by the engine.
 *   psg.sample.count-formula        a run of T ticks from entry E emits floor((E + T) / 16) - floor(E / 16) samples
 *   psg.sample.grid-stamp           sample k is made when the clock passes k x 16: none at 15 ticks, one at 16
 *   psg.sample.byte-order           the record of levels 15, 10, 5 is 0F 0A 05 00, in that order
 *   psg.sample.record-layout        samples are contiguous 4-byte records, 12 bytes for 3 samples
 *   psg.sample.levels-0-15          every fixed level 0..15 appears as the byte of its channel
 *   psg.sample.reserved-byte-zero   the fourth byte is 0 in every sample
 *   psg.sample.first-sample-index   the first sample is step 1 (stamp 16): emitted 1, next step 2
 *   psg.sample.silent-after-reset   a power-on state with no write makes silence and no level-change record
 *   psg.sample.absolute-grid-from-reset  any split of the same ticks gives the same samples and the same state
 *   psg.sample.format-id            the layout format is 1 (PSG_LEVELS_V1) with 4-byte samples every 16 ticks
 *   psg.sample.write-effect-boundary  a write stamped k x 16 is seen from sample k + 1 on, one stamped k x 16 - 1
 *                                   from sample k on (step first, then write, C78)
 *   psg.sample.wrapper-stamp        a guest write is stamped unit start + offset x 4 ticks: the first sample that
 *                                   sees the amplitude write of stand-in program CONST is the first at a tick above
 *                                   the stamp (the offset 6 of a direct MVO is the one the bounds probe measured) */
#include "psg_util.h"

#include "psg_cases.h"

#define PS_CHECK(cond)                                                                                 \
  do {                                                                                                 \
    if (!(cond)) {                                                                                     \
      ivs_test_diag("check failed at line %d: %s", __LINE__, #cond);                                   \
      goto done;                                                                                       \
    }                                                                                                  \
  } while (0)

#define PS_MIXER 8u
#define PS_AMP_A 11u
#define PS_AMP_B 12u
#define PS_AMP_C 13u
#define PS_CONST_ON 0x3Fu

static int ps_count_formula(void)
{
  static const uint64_t entries[4] = {0u, 1u, 15u, 16u};
  static const uint64_t spans[6] = {0u, 1u, 15u, 16u, 17u, 1000u};
  uint32_t e;
  uint32_t s;
  for (e = 0u; e < 4u; e++) {
    for (s = 0u; s < 6u; s++) {
      pu_rig r;
      ivs_psg_sink k;
      uint64_t from = 1000u + entries[e];
      uint64_t want = (from + spans[s]) / 16u - from / 16u;
      if (!pu_init(&r)) {
        return IVS_TEST_FAIL;
      }
      ivs_psg_sink_init(&k, NULL, 0u);
      pu_run(&r, &k, from);
      k.samples = 0u;
      pu_run(&r, &k, from + spans[s]);
      if (k.samples != want) {
        ivs_test_diag("entry %llu span %llu: %llu samples, want %llu", (unsigned long long)from,
                      (unsigned long long)spans[s], (unsigned long long)k.samples, (unsigned long long)want);
        return IVS_TEST_FAIL;
      }
    }
  }
  return IVS_TEST_PASS;
}

static int ps_grid_stamp(void)
{
  pu_rig r;
  ivs_psg_sink k;
  uint8_t buf[64];
  if (!pu_init(&r)) {
    return IVS_TEST_FAIL;
  }
  ivs_psg_sink_init(&k, buf, sizeof buf);
  pu_run(&r, &k, 15u);
  if (k.samples != 0u || r.obs.emitted != 0u) {
    ivs_test_diag("a sample before tick 16");
    return IVS_TEST_FAIL;
  }
  pu_run(&r, &k, 16u);
  if (k.samples != 1u || r.obs.emitted != 1u) {
    ivs_test_diag("no sample at tick 16");
    return IVS_TEST_FAIL;
  }
  pu_run(&r, &k, 31u);
  if (k.samples != 1u) {
    return IVS_TEST_FAIL;
  }
  pu_run(&r, &k, 32u);
  return k.samples == 2u ? IVS_TEST_PASS : IVS_TEST_FAIL;
}

/* Programs three constant levels: R7 = $3F and the three amplitudes. */
static void ps_levels(pu_rig *r, ivs_psg_sink *k, uint32_t a, uint32_t b, uint32_t c)
{
  pu_write(r, k, 0u, PS_MIXER, PS_CONST_ON);
  pu_write(r, k, 0u, PS_AMP_A, a);
  pu_write(r, k, 0u, PS_AMP_B, b);
  pu_write(r, k, 0u, PS_AMP_C, c);
}

static int ps_byte_order(void)
{
  pu_rig r;
  ivs_psg_sink k;
  uint8_t buf[16];
  static const uint8_t want[4] = {0x0Fu, 0x0Au, 0x05u, 0x00u};
  if (!pu_init(&r)) {
    return IVS_TEST_FAIL;
  }
  memset(buf, 0xA5, sizeof buf);
  ivs_psg_sink_init(&k, buf, sizeof buf);
  ps_levels(&r, &k, 15u, 10u, 5u);
  pu_run(&r, &k, 16u);
  return k.written == 4u && memcmp(buf, want, 4u) == 0 && buf[4] == 0xA5u ? IVS_TEST_PASS : IVS_TEST_FAIL;
}

static int ps_record_layout(void)
{
  pu_rig r;
  ivs_psg_sink k;
  uint8_t buf[16];
  static const uint8_t want[12] = {1, 2, 3, 0, 1, 2, 3, 0, 1, 2, 3, 0};
  if (!pu_init(&r)) {
    return IVS_TEST_FAIL;
  }
  memset(buf, 0xA5, sizeof buf);
  ivs_psg_sink_init(&k, buf, sizeof buf);
  ps_levels(&r, &k, 1u, 2u, 3u);
  pu_run(&r, &k, 48u);
  return k.written == 12u && k.samples == 3u && memcmp(buf, want, 12u) == 0 && buf[12] == 0xA5u ? IVS_TEST_PASS
                                                                                                  : IVS_TEST_FAIL;
}

static int ps_levels_0_15(void)
{
  uint32_t lv;
  uint32_t ch;
  for (ch = 0u; ch < 3u; ch++) {
    for (lv = 0u; lv < 16u; lv++) {
      pu_rig r;
      ivs_psg_sink k;
      uint8_t buf[8];
      uint32_t want[3];
      want[0] = ch == 0u ? lv : 0u;
      want[1] = ch == 1u ? lv : 0u;
      want[2] = ch == 2u ? lv : 0u;
      if (!pu_init(&r)) {
        return IVS_TEST_FAIL;
      }
      memset(buf, 0xA5, sizeof buf);
      ivs_psg_sink_init(&k, buf, sizeof buf);
      pu_write(&r, &k, 0u, PS_MIXER, PS_CONST_ON);
      pu_write(&r, &k, 0u, PS_AMP_A + ch, lv);
      pu_run(&r, &k, 16u);
      if (buf[0] != want[0] || buf[1] != want[1] || buf[2] != want[2] || buf[3] != 0u) {
        ivs_test_diag("channel %u level %u: record %02X %02X %02X %02X", (unsigned)ch, (unsigned)lv, buf[0], buf[1],
                      buf[2], buf[3]);
        return IVS_TEST_FAIL;
      }
    }
  }
  return IVS_TEST_PASS;
}

static int ps_reserved_byte_zero(void)
{
  pu_rig r;
  ivs_psg_sink k;
  uint8_t buf[400];
  uint32_t i;
  if (!pu_init(&r)) {
    return IVS_TEST_FAIL;
  }
  memset(buf, 0xA5, sizeof buf);
  ivs_psg_sink_init(&k, buf, sizeof buf);
  ps_levels(&r, &k, 0x3Fu, 0x2Fu, 0x1Fu); /* every select value and a changing fixed part */
  pu_write(&r, &k, 0u, 0u, 1u);           /* tone A period 1 (not enabled: constant gate) */
  pu_run(&r, &k, 100u * 16u);
  if (k.samples != 100u) {
    return IVS_TEST_FAIL;
  }
  for (i = 0u; i < 100u; i++) {
    if (buf[4u * i + 3u] != 0u) {
      ivs_test_diag("reserved byte of sample %u is %02X", (unsigned)(i + 1u), buf[4u * i + 3u]);
      return IVS_TEST_FAIL;
    }
  }
  return IVS_TEST_PASS;
}

static int ps_first_sample_index(void)
{
  pu_rig r;
  ivs_psg_sink k;
  if (!pu_init(&r)) {
    return IVS_TEST_FAIL;
  }
  ivs_psg_sink_init(&k, NULL, 0u);
  if (r.s.next_sample != 1u || r.obs.emitted != 0u) {
    return IVS_TEST_FAIL;
  }
  pu_run(&r, &k, 16u);
  return r.s.next_sample == 2u && r.obs.emitted == 1u ? IVS_TEST_PASS : IVS_TEST_FAIL;
}

static int ps_silent_after_reset(void)
{
  pu_rig r;
  ivs_psg_sink k;
  uint8_t buf[400];
  uint32_t i;
  if (!pu_init(&r)) {
    return IVS_TEST_FAIL;
  }
  memset(buf, 0xA5, sizeof buf);
  ivs_psg_sink_init(&k, buf, sizeof buf);
  pu_run(&r, &k, 100u * 16u);
  for (i = 0u; i < 400u; i++) {
    if (buf[i] != 0u) {
      return IVS_TEST_FAIL;
    }
  }
  return k.samples == 100u && r.obs.count == 0u && r.obs.prev_levels == 0u ? IVS_TEST_PASS : IVS_TEST_FAIL;
}

static int ps_absolute_grid(void)
{
  static const uint64_t cuts[3][6] = {{1u, 17u, 33u, 5000u, 9000u, 12000u},
                                      {15u, 16u, 31u, 32u, 4999u, 12000u},
                                      {12000u, 12000u, 12000u, 12000u, 12000u, 12000u}};
  pu_rig base;
  ivs_psg_sink kb;
  uint8_t *bb = malloc(4u * 800u);
  uint32_t c;
  int rc = IVS_TEST_FAIL;
  if (bb == NULL || !pu_init(&base)) {
    free(bb);
    return IVS_TEST_FAIL;
  }
  ivs_psg_sink_init(&kb, bb, 4u * 800u);
  pu_write(&base, &kb, 0u, PS_MIXER, 0x3Eu);
  pu_write(&base, &kb, 0u, PS_AMP_A, 15u);
  pu_write(&base, &kb, 0u, 0u, 3u); /* tone A fine period 3 */
  pu_run(&base, &kb, 12000u);
  for (c = 0u; c < 3u; c++) {
    pu_rig r;
    ivs_psg_sink k;
    uint8_t *b = malloc(4u * 800u);
    uint32_t i;
    if (b == NULL || !pu_init(&r)) {
      free(b);
      goto done;
    }
    ivs_psg_sink_init(&k, b, 4u * 800u);
    pu_write(&r, &k, 0u, PS_MIXER, 0x3Eu);
    pu_write(&r, &k, 0u, PS_AMP_A, 15u);
    pu_write(&r, &k, 0u, 0u, 3u);
    for (i = 0u; i < 6u; i++) {
      pu_run(&r, &k, cuts[c][i]);
    }
    if (k.written != kb.written || memcmp(b, bb, (size_t)kb.written) != 0 ||
        memcmp(&r.s, &base.s, sizeof r.s) != 0 || memcmp(&r.obs, &base.obs, sizeof r.obs) != 0) {
      ivs_test_diag("split %u differs from the single catch-up", (unsigned)c);
      free(b);
      goto done;
    }
    free(b);
  }
  rc = IVS_TEST_PASS;
done:
  free(bb);
  return rc;
}

static int ps_format_id(void)
{
  ivs_ms_inst x;
  ivs_info info;
  int ok;
  if (!ivs_ms_powered(&x, IVS_MS_CONTENT_A, 0x5Au)) {
    return IVS_TEST_FAIL;
  }
  memset(&info, 0, sizeof info);
  info.struct_size = (uint32_t)sizeof info;
  ok = ivs_machine_info(x.m, &info) == IVS_OK && info.audio_format == 1u && info.audio_sample_bytes == 4u &&
       info.audio_sample_ticks == 16u;
  ivs_ms_free(&x);
  return ok ? IVS_TEST_PASS : IVS_TEST_FAIL;
}

/* Level of channel A at sample k (1-based) in buf. */
static uint32_t ps_a(const uint8_t *buf, uint32_t k)
{
  return buf[4u * (k - 1u)];
}

static int ps_write_effect_boundary(void)
{
  uint32_t variant;
  for (variant = 0u; variant < 2u; variant++) {
    pu_rig r;
    ivs_psg_sink k;
    uint8_t buf[80];
    /* variant 0: a write stamped exactly 5 x 16 = 80: sample 5 (tick 80) does not see it, sample 6 does;
     * variant 1: a write stamped 79: sample 5 sees it */
    uint64_t stamp = variant == 0u ? 80u : 79u;
    uint32_t first_seen = variant == 0u ? 6u : 5u;
    uint32_t j;
    if (!pu_init(&r)) {
      return IVS_TEST_FAIL;
    }
    memset(buf, 0xA5, sizeof buf);
    ivs_psg_sink_init(&k, buf, sizeof buf);
    pu_write(&r, &k, 0u, PS_MIXER, PS_CONST_ON);
    pu_write(&r, &k, stamp, PS_AMP_A, 15u);
    pu_run(&r, &k, 8u * 16u);
    if (k.samples != 8u) {
      return IVS_TEST_FAIL;
    }
    for (j = 1u; j <= 8u; j++) {
      uint32_t want = j >= first_seen ? 15u : 0u;
      if (ps_a(buf, j) != want) {
        ivs_test_diag("write at %llu: sample %u is %u, want %u", (unsigned long long)stamp, (unsigned)j,
                      (unsigned)ps_a(buf, j), (unsigned)want);
        return IVS_TEST_FAIL;
      }
    }
  }
  return IVS_TEST_PASS;
}

static int ps_wrapper_stamp(void)
{
  enum { SAMPLES = 64 };
  ivs_ms_inst x;
  ivs_psg_sink k;
  ivs_cp1610_result res;
  uint8_t buf[4 * SAMPLES];
  uint64_t start = 0u;
  uint64_t stamp = 0u;
  uint32_t first_nonzero = 0u;
  uint32_t j;
  uint32_t guard;
  int rc = IVS_TEST_FAIL;
  if (!ivs_ms_powered_prog(&x, IVS_MS_PROG_CONST, 0x5Au)) {
    return IVS_TEST_FAIL;
  }
  memset(buf, 0xA5, sizeof buf);
  ivs_psg_sink_init(&k, buf, sizeof buf);
  /* Step until the amplitude register of channel A changes: that unit's start tick is the one the write is
   * stamped from; a direct MVO writes at CPU cycle 6 of its unit (psg.bounds.write-gap-min measured it). */
  for (guard = 0u; guard < 20u; guard++) {
    uint64_t before = pu_now(x.m);
    PS_CHECK(ivs_machine_step_unit_sink_internal(x.m, &res, &k) == IVS_OK);
    if (ivs_memsys_psg_reg_internal(&((struct ivs_machine *)x.m)->mem, 11u) == 15u) {
      start = before;
      break;
    }
  }
  PS_CHECK(guard < 20u);
  stamp = start + 6u * 4u;
  /* run on until sample SAMPLES is due */
  for (guard = 0u; guard < 100u && k.samples < SAMPLES; guard++) {
    PS_CHECK(ivs_machine_step_unit_sink_internal(x.m, &res, &k) == IVS_OK);
  }
  PS_CHECK(k.samples >= 40u);
  for (j = 1u; j <= 40u; j++) {
    if (buf[4u * (j - 1u)] != 0u) {
      first_nonzero = j;
      break;
    }
  }
  /* the first sample at a tick above the stamp: 16 k > stamp */
  if (first_nonzero != (uint32_t)(stamp / 16u) + 1u) {
    ivs_test_diag("first level 15 at sample %u, stamp %llu wants sample %u", (unsigned)first_nonzero,
                  (unsigned long long)stamp, (unsigned)(stamp / 16u + 1u));
    goto done;
  }
  rc = IVS_TEST_PASS;
done:
  ivs_ms_free(&x);
  return rc;
}

const ivs_test_case ivs_psg_sample_cases[] = {
    {"psg.sample.count-formula", ps_count_formula, NULL},
    {"psg.sample.grid-stamp", ps_grid_stamp, NULL},
    {"psg.sample.byte-order", ps_byte_order, NULL},
    {"psg.sample.record-layout", ps_record_layout, NULL},
    {"psg.sample.levels-0-15", ps_levels_0_15, NULL},
    {"psg.sample.reserved-byte-zero", ps_reserved_byte_zero, NULL},
    {"psg.sample.first-sample-index", ps_first_sample_index, NULL},
    {"psg.sample.silent-after-reset", ps_silent_after_reset, NULL},
    {"psg.sample.absolute-grid-from-reset", ps_absolute_grid, NULL},
    {"psg.sample.format-id", ps_format_id, NULL},
    {"psg.sample.write-effect-boundary", ps_write_effect_boundary, NULL},
    {"psg.sample.wrapper-stamp", ps_wrapper_stamp, NULL},
};
const size_t ivs_psg_sample_count = sizeof ivs_psg_sample_cases / sizeof ivs_psg_sample_cases[0];

/* SPDX-License-Identifier: MIT */
/* Reference PCM converter (D-21, plan 05-10): see ivs_refpcm.h for the algorithm, the label and the
 * placement rules. Pure integer arithmetic; no I/O, no allocation, no globals. */
#include "ivs_refpcm.h"

#include <stddef.h>
#include <stdint.h>

#define REFPCM_DC_NUM 32763
#define REFPCM_DC_DEN 32768
#define REFPCM_SCALE 10000u /* maps the 300,000,000 maximum of a mixed level to 30,000 */

/* A copy of the profile field psg.dac.levels (GI Fig. 9 labels, levels 8..1 extrapolated, tag inference,
 * conflict C12): units of 1e-8 of full scale. The test runner.refpcm.dac-table-matches-profile keeps it
 * honest. */
static const uint32_t refpcm_dac[IVS_REFPCM_DAC_LEVELS] = {
    0u,        781250u,   946875u,   1562500u,  1893750u,  3125000u,  3787500u,  6250000u,
    7575000u,  12500000u, 15150000u, 25000000u, 30300000u, 50000000u, 70700000u, 100000000u};

int64_t ivs_refpcm_floor_div(int64_t a, int64_t b)
{
  int64_t q = a / b;
  if (a % b < 0) {
    q -= 1;
  }
  return q;
}

uint32_t ivs_refpcm_dac_level(uint32_t i) { return i < IVS_REFPCM_DAC_LEVELS ? refpcm_dac[i] : 0u; }

int ivs_refpcm_init(ivs_refpcm_state *st, uint64_t crystal_num, uint64_t crystal_den, uint32_t sample_period_ticks,
                    uint32_t rate)
{
  if (rate != IVS_REFPCM_RATE_44100 && rate != IVS_REFPCM_RATE_48000) {
    return IVS_REFPCM_E_ARG;
  }
  if (crystal_num == 0u || crystal_num > UINT64_C(0x100000000) || crystal_den == 0u ||
      crystal_den > UINT64_C(0x100000) || sample_period_ticks == 0u || sample_period_ticks > 0x10000u) {
    return IVS_REFPCM_E_ARG;
  }
  st->cell_units = (uint64_t)sample_period_ticks * crystal_den * (uint64_t)rate;
  st->win_units = crystal_num;
  st->win_rem = crystal_num;
  st->acc = 0u;
  st->frames = 0u;
  st->samples = 0u;
  st->prev_x = 0;
  st->prev_y = 0;
  st->rate = rate;
  st->reserved = 0u;
  return IVS_REFPCM_OK;
}

size_t ivs_refpcm_out_bound(const ivs_refpcm_state *st, size_t nsamples)
{
  /* nsamples * cell_units is at most 2^64 / 2^20 * 2^52 only for absurd nsamples; the callers feed at most
   * a few thousand samples, and the product is computed in 64 bits with the quotient first. */
  uint64_t whole = ((uint64_t)nsamples * st->cell_units) / st->win_units;
  return (size_t)whole + 1u;
}

/* One finished window: the box output, then the DC block, then the saturating store. */
static int16_t refpcm_emit(ivs_refpcm_state *st)
{
  uint64_t d = st->win_units * (uint64_t)REFPCM_SCALE;
  int32_t x = (int32_t)((2u * st->acc + d) / (2u * d)); /* round half up */
  int64_t y = (int64_t)x - (int64_t)st->prev_x + ivs_refpcm_floor_div((int64_t)st->prev_y * REFPCM_DC_NUM, REFPCM_DC_DEN);
  st->prev_x = x;
  st->prev_y = (int32_t)y;
  st->frames++;
  if (y > 32767) {
    return 32767;
  }
  if (y < -32768) {
    return -32768;
  }
  return (int16_t)y;
}

int ivs_refpcm_feed(ivs_refpcm_state *st, const uint8_t *levels, size_t nsamples, int16_t *out, size_t out_capacity,
                    size_t *produced)
{
  size_t n = 0u;
  *produced = 0u;
  if (nsamples == 0u) {
    return IVS_REFPCM_OK;
  }
  if (out_capacity < ivs_refpcm_out_bound(st, nsamples)) {
    return IVS_REFPCM_E_SPACE;
  }
  for (size_t i = 0u; i < nsamples; i++) {
    if (levels[4u * i] > 15u || levels[4u * i + 1u] > 15u || levels[4u * i + 2u] > 15u || levels[4u * i + 3u] != 0u) {
      return IVS_REFPCM_E_LEVEL;
    }
  }
  for (size_t i = 0u; i < nsamples; i++) {
    uint64_t x = (uint64_t)refpcm_dac[levels[4u * i]] + (uint64_t)refpcm_dac[levels[4u * i + 1u]] +
                 (uint64_t)refpcm_dac[levels[4u * i + 2u]];
    uint64_t cell = st->cell_units;
    while (cell != 0u) {
      uint64_t t = cell < st->win_rem ? cell : st->win_rem;
      st->acc += t * x;
      cell -= t;
      st->win_rem -= t;
      if (st->win_rem == 0u) {
        out[n++] = refpcm_emit(st);
        st->acc = 0u;
        st->win_rem = st->win_units;
      }
    }
    st->samples++;
  }
  *produced = n;
  return IVS_REFPCM_OK;
}

static void refpcm_put32(uint8_t *p, uint32_t v)
{
  p[0] = (uint8_t)(v & 0xFFu);
  p[1] = (uint8_t)((v >> 8) & 0xFFu);
  p[2] = (uint8_t)((v >> 16) & 0xFFu);
  p[3] = (uint8_t)((v >> 24) & 0xFFu);
}

static void refpcm_put16(uint8_t *p, uint32_t v)
{
  p[0] = (uint8_t)(v & 0xFFu);
  p[1] = (uint8_t)((v >> 8) & 0xFFu);
}

int ivs_refpcm_wav_header(uint8_t out[44], uint32_t rate, uint64_t nframes)
{
  uint32_t data;
  if (rate == 0u || nframes > (UINT64_C(0xFFFFFFFF) - 36u) / 2u) {
    return IVS_REFPCM_E_ARG;
  }
  data = (uint32_t)(2u * nframes);
  out[0] = 'R';
  out[1] = 'I';
  out[2] = 'F';
  out[3] = 'F';
  refpcm_put32(out + 4, 36u + data);
  out[8] = 'W';
  out[9] = 'A';
  out[10] = 'V';
  out[11] = 'E';
  out[12] = 'f';
  out[13] = 'm';
  out[14] = 't';
  out[15] = ' ';
  refpcm_put32(out + 16, 16u);
  refpcm_put16(out + 20, 1u);
  refpcm_put16(out + 22, 1u);
  refpcm_put32(out + 24, rate);
  refpcm_put32(out + 28, 2u * rate);
  refpcm_put16(out + 32, 2u);
  refpcm_put16(out + 34, 16u);
  out[36] = 'd';
  out[37] = 'a';
  out[38] = 't';
  out[39] = 'a';
  refpcm_put32(out + 40, data);
  return IVS_REFPCM_OK;
}

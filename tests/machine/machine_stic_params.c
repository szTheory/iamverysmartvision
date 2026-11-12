/* SPDX-License-Identifier: MIT */
/* Suite machine, family stic.params (plan 06-04, D-02, D-05, D-07, D-09, D-12, D-13, D-15, A-11): module-level
 * relation tests of the generated STIC parameter table ivs_stic_params, over every registry profile.
 *
 * The tests assert RELATIONS between the generated struct and the registry (the profile's ticks per CPU
 * cycle, cycles per scanline and scanlines per frame, read through ivs_profile_get, a path independent of the
 * STIC generator) and between the members of the struct. They never compare a policy number (a window length,
 * a hold, a pitch) with a literal copy of itself: whether 2,000 or 3,780 is the right figure is the business of
 * the discrepancy log and the D10 measurements, and a test of the number would only prove the profile equals
 * itself (D-02). The table is data only in plan 06-04; the device (06-06) and the later plans consume it. */
#include <stddef.h>
#include <stdint.h>

#include "ivs/ivs.h"
#include "ivs/ivs_machine.h"
#include "ivs_test.h"
#include "machine_cases.h"
#include "memmap/memsys.h"
#include "stic/stic_params.h"

#define IVS_CHECK(cond)                                                                                \
  do {                                                                                                 \
    if (!(cond)) {                                                                                     \
      ivs_test_diag("check failed at line %d: %s", __LINE__, #cond);                                   \
      return IVS_TEST_FAIL;                                                                            \
    }                                                                                                  \
  } while (0)

/* Row i of the registry and of the STIC table; both exist for every registered profile (the registry tie of
 * ivs_machine_profile_covered and machine.create.tables-cover-registry). */
static int msp_row(uint32_t i, const ivs_profile_desc **d, const ivs_stic_params **p)
{
  *d = ivs_profile_get(i);
  *p = ivs_stic_params_get(i);
  return *d != NULL && *p != NULL;
}

/* Every tick field is its cycle field times the registry's ticks per CPU cycle, and the frame length is the
 * product of the three registry time-base fields (D-07). The table is covered for every registry profile. */
static int msp_unit_conversion(void)
{
  uint32_t n = ivs_profile_count();
  uint32_t i;
  IVS_CHECK(n > 0u);
  IVS_CHECK(ivs_stic_params_count() == n);
  for (i = 0u; i < n; i++) {
    const ivs_profile_desc *d;
    const ivs_stic_params *p;
    uint64_t tpc;
    uint64_t frame;
    IVS_CHECK(msp_row(i, &d, &p));
    tpc = d->ticks_per_cpu_cycle;
    IVS_CHECK(tpc > 0u);
    IVS_CHECK(p->ticks_per_cpu_cycle == tpc);
    frame = tpc * (uint64_t)d->cpu_cycles_per_scanline * (uint64_t)d->scanlines_per_frame;
    IVS_CHECK(frame > 0u && frame <= 0xFFFFFFFFu);
    IVS_CHECK(p->frame_ticks == frame);
    IVS_CHECK(p->reg_window_ticks == (uint64_t)p->reg_window_cycles * tpc);
    IVS_CHECK(p->gmem_window_ticks == (uint64_t)p->gmem_window_cycles * tpc);
    IVS_CHECK(p->irq_low_ticks == (uint64_t)p->irq_low_cycles * tpc);
    IVS_CHECK(p->line_sample_lag_ticks == (uint64_t)p->line_sample_lag_cycles * tpc);
    IVS_CHECK(p->bus_first_start_ticks == (uint64_t)p->bus_first_start_cycles * tpc);
    IVS_CHECK(p->bus_pitch_ticks == (uint64_t)p->bus_pitch_cycles * tpc);
    IVS_CHECK(p->bus_hold_ticks == (uint64_t)p->bus_hold_cycles * tpc);
    IVS_CHECK(p->bus_fetch_offset_ticks == (uint64_t)p->bus_fetch_offset_cycles * tpc);
    IVS_CHECK(p->frame_phase_ticks < p->frame_ticks);
  }
  return IVS_TEST_PASS;
}

/* The per-frame event table (D-09): a frame start at 0, the register window close, the graphics-memory window
 * close, one row fetch per bus row at first_start + i x pitch + fetch_offset, and the frame end at F; ticks
 * ascend and the kinds come in the documented order. */
static int msp_event_table(void)
{
  uint32_t n = ivs_profile_count();
  uint32_t i;
  for (i = 0u; i < n; i++) {
    const ivs_profile_desc *d;
    const ivs_stic_params *p;
    uint32_t k;
    IVS_CHECK(msp_row(i, &d, &p));
    IVS_CHECK(p->bus_rows >= 1u && p->bus_rows <= p->bus_count);
    IVS_CHECK(p->bus_rows == p->video_card_rows);
    IVS_CHECK(p->event_count == 4u + p->bus_rows);
    IVS_CHECK(p->event_count <= IVS_STIC_EVENT_MAX);
    IVS_CHECK(p->event_kind[0] == IVS_STIC_EV_FRAME_START && p->event_tick[0] == 0u);
    IVS_CHECK(p->event_kind[1] == IVS_STIC_EV_REG_CLOSE && p->event_tick[1] == p->reg_window_ticks);
    IVS_CHECK(p->event_kind[2] == IVS_STIC_EV_GMEM_CLOSE && p->event_tick[2] == p->gmem_window_ticks);
    for (k = 0u; k < p->bus_rows; k++) {
      uint64_t want = (uint64_t)p->bus_first_start_ticks + (uint64_t)k * p->bus_pitch_ticks + p->bus_fetch_offset_ticks;
      IVS_CHECK(p->event_kind[3u + k] == IVS_STIC_EV_ROW_FETCH);
      IVS_CHECK(p->event_arg[3u + k] == k);
      IVS_CHECK(p->event_tick[3u + k] == want);
    }
    IVS_CHECK(p->event_kind[p->event_count - 1u] == IVS_STIC_EV_FRAME_END);
    IVS_CHECK(p->event_tick[p->event_count - 1u] == p->frame_ticks);
    for (k = 1u; k < p->event_count; k++) {
      IVS_CHECK(p->event_tick[k - 1u] <= p->event_tick[k]);
    }
    IVS_CHECK(d->scanlines_per_frame > 0u);
  }
  return IVS_TEST_PASS;
}

/* The access windows close in order and before the first bus window (D-05): reg <= gmem <= first_start, and the
 * interrupt level ends inside the frame. */
static int msp_window_order(void)
{
  uint32_t n = ivs_profile_count();
  uint32_t i;
  for (i = 0u; i < n; i++) {
    const ivs_profile_desc *d;
    const ivs_stic_params *p;
    IVS_CHECK(msp_row(i, &d, &p));
    IVS_CHECK(p->reg_window_ticks <= p->gmem_window_ticks);
    IVS_CHECK(p->gmem_window_ticks <= p->bus_first_start_ticks);
    IVS_CHECK(p->irq_low_ticks <= p->frame_ticks);
    IVS_CHECK(p->access_strobe_ticks < p->frame_ticks);
    IVS_CHECK(d->ticks_per_cpu_cycle > 0u);
  }
  return IVS_TEST_PASS;
}

/* The bus-window train tiles inside the frame (D-09): the last window ends no later than F, windows do not
 * overlap (pitch >= hold) and each row latch falls inside its own window (offset <= hold). */
static int msp_tiling_within_frame(void)
{
  uint32_t n = ivs_profile_count();
  uint32_t i;
  for (i = 0u; i < n; i++) {
    const ivs_profile_desc *d;
    const ivs_stic_params *p;
    uint64_t last_end;
    uint32_t k;
    IVS_CHECK(msp_row(i, &d, &p));
    IVS_CHECK(p->bus_count >= 1u);
    IVS_CHECK(p->bus_pitch_ticks >= p->bus_hold_ticks);
    IVS_CHECK(p->bus_fetch_offset_ticks <= p->bus_hold_ticks);
    last_end = (uint64_t)p->bus_first_start_ticks + (uint64_t)(p->bus_count - 1u) * p->bus_pitch_ticks + p->bus_hold_ticks;
    IVS_CHECK(last_end <= p->frame_ticks);
    for (k = 0u; k < p->bus_rows; k++) {
      uint64_t start = (uint64_t)p->bus_first_start_ticks + (uint64_t)k * p->bus_pitch_ticks;
      IVS_CHECK((uint64_t)p->event_tick[3u + k] >= start);
      IVS_CHECK((uint64_t)p->event_tick[3u + k] <= start + p->bus_hold_ticks);
    }
    IVS_CHECK(d->cpu_cycles_per_scanline > 0u);
  }
  return IVS_TEST_PASS;
}

/* The derived published surface (D-12, D-13, D-15): width = (card_columns - 1) x card_w + last_card_column_px,
 * height = card_rows x card_h, rows tightly packed, slot = header + plane with the header size the public
 * header names, the surface inside the object grid, and the slack the capacity rule needs for the registry's
 * maximum unit (U < F gives one extra frame). */
static int msp_geometry_derived(void)
{
  uint32_t n = ivs_profile_count();
  uint32_t i;
  for (i = 0u; i < n; i++) {
    const ivs_profile_desc *d;
    const ivs_stic_params *p;
    uint64_t u;
    uint64_t f;
    IVS_CHECK(msp_row(i, &d, &p));
    IVS_CHECK(p->video_last_column_px >= 1u && p->video_last_column_px <= p->video_card_w);
    IVS_CHECK(p->video_width == (p->video_card_columns - 1u) * p->video_card_w + p->video_last_column_px);
    IVS_CHECK(p->video_height == p->video_card_rows * p->video_card_h);
    IVS_CHECK(p->video_stride == p->video_width);
    IVS_CHECK(p->video_plane_bytes == p->video_width * p->video_height);
    IVS_CHECK(p->video_header_bytes == IVS_FRAME_HEADER_BYTES);
    IVS_CHECK(p->video_slot_bytes == p->video_header_bytes + p->video_plane_bytes);
    IVS_CHECK(p->video_width <= p->video_grid_w && p->video_height <= p->video_grid_h);
    IVS_CHECK(p->video_blank_value >= p->video_colors && p->video_blank_value == IVS_PIXEL_BLANK);
    IVS_CHECK(p->video_colors == IVS_PIXEL_COLOR_MAX + 1u);
    IVS_CHECK(p->video_row_lines >= 1u);
    IVS_CHECK(p->video_display_off_fill == IVS_STIC_FILL_BORDER);
    u = d->psg_max_unit_ticks;
    f = p->frame_ticks;
    IVS_CHECK(u > 0u && 2u * u < f);
    IVS_CHECK(p->video_slack_frames == (u + f - 1u) / f);
  }
  return IVS_TEST_PASS;
}

/* The register file shape (D-04): 64 rows; the classes split as the register literal splits them (32 stored,
 * 8 collision, 2 strobe, 22 reserved); the STIC has 14 data pins so no mask has bit 15 or 14; a reserved or
 * strobe register stores nothing; every mode-select address decodes to a strobe-class register (its low six
 * bits); every side-effect row names a register of the file. */
static int msp_regmap_shape(void)
{
  uint32_t n = ivs_profile_count();
  uint32_t i;
  for (i = 0u; i < n; i++) {
    const ivs_profile_desc *d;
    const ivs_stic_params *p;
    uint32_t count[4] = {0u, 0u, 0u, 0u};
    uint32_t r;
    IVS_CHECK(msp_row(i, &d, &p));
    IVS_CHECK(IVS_STIC_REG_COUNT == 64u);
    for (r = 0u; r < IVS_STIC_REG_COUNT; r++) {
      IVS_CHECK(p->reg_class[r] <= IVS_STIC_CLASS_RESERVED);
      count[p->reg_class[r]]++;
      IVS_CHECK((p->reg_wmask[r] & 0xC000u) == 0u && (p->reg_rmask[r] & 0xC000u) == 0u);
      if (p->reg_class[r] == IVS_STIC_CLASS_RESERVED || p->reg_class[r] == IVS_STIC_CLASS_STROBE) {
        IVS_CHECK(p->reg_wmask[r] == 0u);
      }
    }
    IVS_CHECK(count[IVS_STIC_CLASS_STORED] == 32u);
    IVS_CHECK(count[IVS_STIC_CLASS_COLLISION] == 8u);
    IVS_CHECK(count[IVS_STIC_CLASS_STROBE] == 2u);
    IVS_CHECK(count[IVS_STIC_CLASS_RESERVED] == 22u);
    IVS_CHECK(p->mode_addr_count >= 1u && p->mode_addr_count <= IVS_STIC_MODE_ADDR_MAX);
    for (r = 0u; r < p->mode_addr_count; r++) {
      IVS_CHECK(p->reg_class[p->mode_addr[r] & 0x3Fu] == IVS_STIC_CLASS_STROBE);
    }
    IVS_CHECK(p->effect_count >= 1u && p->effect_count <= IVS_STIC_EFFECT_MAX);
    for (r = 0u; r < p->effect_count; r++) {
      IVS_CHECK(p->effect_addr[r] < IVS_STIC_REG_COUNT);
      IVS_CHECK(p->effect_access[r] <= IVS_STIC_ACCESS_WRITE);
      IVS_CHECK(p->effect_code[r] <= IVS_STIC_EFFECT_MODE_CS);
    }
    IVS_CHECK(p->alias_read_sensitive <= 1u);
  }
  return IVS_TEST_PASS;
}

/* The power-on record and the background span: the mode is one of the two modes, the latch a flag, the span
 * holds one word per card and lies wholly inside the 16-bit address space. */
static int msp_power_on(void)
{
  uint32_t n = ivs_profile_count();
  uint32_t i;
  for (i = 0u; i < n; i++) {
    const ivs_profile_desc *d;
    const ivs_stic_params *p;
    IVS_CHECK(msp_row(i, &d, &p));
    IVS_CHECK(p->power_on_mode == IVS_STIC_MODE_FGBG || p->power_on_mode == IVS_STIC_MODE_COLOR_STACK);
    IVS_CHECK(p->power_on_display_latch <= 1u);
    IVS_CHECK(p->backtab_words == p->video_card_columns * p->video_card_rows);
    IVS_CHECK(p->backtab_lo + p->backtab_words <= 0x10000u);
    IVS_CHECK(p->frame_phase_ticks < p->frame_ticks);
    IVS_CHECK(d->id[0] != '\0');
  }
  return IVS_TEST_PASS;
}

const ivs_test_case ivs_machine_stic_params_cases[] = {
    {"machine.stic.params.event-table", msp_event_table, NULL},
    {"machine.stic.params.geometry-derived", msp_geometry_derived, NULL},
    {"machine.stic.params.power-on", msp_power_on, NULL},
    {"machine.stic.params.regmap-shape", msp_regmap_shape, NULL},
    {"machine.stic.params.tiling-within-frame", msp_tiling_within_frame, NULL},
    {"machine.stic.params.unit-conversion", msp_unit_conversion, NULL},
    {"machine.stic.params.window-order", msp_window_order, NULL},
};
const size_t ivs_machine_stic_params_count = sizeof ivs_machine_stic_params_cases / sizeof ivs_machine_stic_params_cases[0];

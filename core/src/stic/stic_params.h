/* SPDX-License-Identifier: MIT */
/* STIC parameters (plan 06-04, D-03, D-05, D-07, D-08, D-09, D-12, D-13, D-15, A-11, Pattern A1): the
 * explicit parameter struct every STIC function takes. Internal; it lives in core/src, which only the
 * core and the tests (ivs_core_internal) see.
 *
 * The values are generated from the profile by cmake/IvsSticTable.cmake (fail closed) into
 * ivs_stic_params.inc, which only stic_params.c includes; the core holds no profile constant (D-02).
 * Every CPU-cycle policy field is converted to master ticks once, by the generator, and both forms are
 * kept so a test can prove the conversion against the registry's ticks per CPU cycle. Nothing consumes
 * the table in plan 06-04 (data only): the device (06-06) and the later plans read it.
 *
 * Time origin: every tick in the event table counts from the frame start S_k = frame_phase_ticks +
 * k x frame_ticks. A nonzero phase means the interval before S_0 schedules nothing: the access gates stay
 * open and no line is driven. */
#ifndef IVS_STIC_PARAMS_H
#define IVS_STIC_PARAMS_H

#include <stdint.h>

#define IVS_STIC_REG_COUNT 64u    /* registers $0000..$003F (stic.regmap) */
#define IVS_STIC_EVENT_MAX 16u    /* FRAME_START, REG_CLOSE, GMEM_CLOSE, ROW_FETCH x rows, FRAME_END */
#define IVS_STIC_MODE_ADDR_MAX 8u /* stic.mode_select_addrs */
#define IVS_STIC_EFFECT_MAX 8u    /* stic.access_effects rows */

/* Frame event kinds, in the order the table lists them within one tick (FRAME_END before the next frame's
 * FRAME_START is the machine's rule; the table itself is one frame, ascending). */
#define IVS_STIC_EV_FRAME_START 0u
#define IVS_STIC_EV_REG_CLOSE 1u
#define IVS_STIC_EV_GMEM_CLOSE 2u
#define IVS_STIC_EV_ROW_FETCH 3u /* event_arg is the card row 0..bus_rows-1 */
#define IVS_STIC_EV_FRAME_END 4u

/* Register classes (stic.regmap class). */
#define IVS_STIC_CLASS_STORED 0u
#define IVS_STIC_CLASS_STROBE 1u
#define IVS_STIC_CLASS_COLLISION 2u
#define IVS_STIC_CLASS_RESERVED 3u

/* Access kinds and side effects of stic.access_effects. A read is READ or FETCH, a write is WRITE or DW. */
#define IVS_STIC_ACCESS_READ 0u
#define IVS_STIC_ACCESS_WRITE 1u
#define IVS_STIC_EFFECT_NONE 0u
#define IVS_STIC_EFFECT_ARM 1u
#define IVS_STIC_EFFECT_MODE_FGBG 2u
#define IVS_STIC_EFFECT_MODE_CS 3u

/* Display modes (stic.power_on mode) and the display-off fill kind (video.display_off_fill). */
#define IVS_STIC_MODE_FGBG 0u
#define IVS_STIC_MODE_COLOR_STACK 1u
#define IVS_STIC_FILL_BORDER 0u

typedef struct ivs_stic_params {
  /* Frame (D-07): F = ticks_per_cpu_cycle x cpu_cycles_per_scanline x scanlines_per_frame. */
  uint32_t ticks_per_cpu_cycle;
  uint32_t frame_ticks;
  uint32_t frame_phase_ticks;
  /* Access windows, interrupt level, sampling lag (D-05, D-08): CPU cycles as profiled and master ticks. */
  uint32_t reg_window_cycles;
  uint32_t reg_window_ticks;
  uint32_t gmem_window_cycles;
  uint32_t gmem_window_ticks;
  uint32_t irq_low_cycles;
  uint32_t irq_low_ticks;
  uint32_t line_sample_lag_cycles;
  uint32_t line_sample_lag_ticks;
  uint32_t access_strobe_ticks; /* master ticks added to an access start to give its window stamp */
  /* Bus windows (D-09). */
  uint32_t bus_count;
  uint32_t bus_rows;
  uint32_t bus_first_start_cycles;
  uint32_t bus_first_start_ticks;
  uint32_t bus_pitch_cycles;
  uint32_t bus_pitch_ticks;
  uint32_t bus_hold_cycles;
  uint32_t bus_hold_ticks;
  uint32_t bus_fetch_offset_cycles;
  uint32_t bus_fetch_offset_ticks;
  /* The per-frame event table: event_count entries in ascending tick order (ticks from the frame start). */
  uint32_t event_count;
  uint32_t event_tick[IVS_STIC_EVENT_MAX];
  uint8_t event_kind[IVS_STIC_EVENT_MAX];
  uint8_t event_arg[IVS_STIC_EVENT_MAX];
  /* Register file (stic.regmap): a store keeps value & wmask, a read gives (store & rmask) | (fill & ~rmask). */
  uint16_t reg_wmask[IVS_STIC_REG_COUNT];
  uint16_t reg_rmask[IVS_STIC_REG_COUNT];
  uint8_t reg_class[IVS_STIC_REG_COUNT];
  /* Mode-select decode (stic.mode_select_addrs, stic.alias_read_sensitive) and side-effect rows. */
  uint32_t mode_addr_count;
  uint16_t mode_addr[IVS_STIC_MODE_ADDR_MAX];
  uint32_t effect_count;
  uint16_t effect_addr[IVS_STIC_EFFECT_MAX];
  uint8_t effect_access[IVS_STIC_EFFECT_MAX];
  uint8_t effect_code[IVS_STIC_EFFECT_MAX];
  uint32_t alias_read_sensitive;
  /* Background memory span (stic.backtab) and the power-on record (stic.power_on; registers are zero). */
  uint32_t backtab_lo;
  uint32_t backtab_words;
  uint32_t power_on_mode;
  uint32_t power_on_display_latch;
  /* Video geometry (D-12, D-13): the card matrix, the object grid, and the DERIVED published surface. */
  uint32_t video_card_columns;
  uint32_t video_card_rows;
  uint32_t video_card_w;
  uint32_t video_card_h;
  uint32_t video_grid_w;
  uint32_t video_grid_h;
  uint32_t video_colors;
  uint32_t video_width;       /* (card_columns - 1) x card_w + last_card_column_px */
  uint32_t video_height;      /* card_rows x card_h */
  uint32_t video_stride;      /* equals the width: rows are tightly packed */
  uint32_t video_plane_bytes; /* width x height */
  uint32_t video_header_bytes;
  uint32_t video_slot_bytes;  /* header + plane */
  uint32_t video_slack_frames;
  uint32_t video_row_lines;
  uint32_t video_last_column_px;
  uint32_t video_blank_value;
  /* Moving objects (plan 08-04, D-02, D-03; rules STIC-R37, R43, R50; stic_mob.c): the generator reads them from video.mob_field_origin,
   * video.mob_line_sample and video.mob_prio_hides (fail closed). origin_x and origin_y (0..15) are the plane positions between the
   * object field and the published plane: a MOB whose X coordinate is X puts its first pattern column on plane column X - origin_x
   * (tier4-community, the pair 8 and 8). line_sample (0 or 1, below video_row_lines) is the television line of a plane row that the
   * line-resolved model publishes: plane row y shows line video_row_lines x y + line_sample (default 0, the even line,
   * unknown-default-chosen, C108). prio_hides (0 or 1) says what hides a MOB whose priority bit is set: 0 the card on pixels only, 1
   * every card pixel (default 0, unknown-default-chosen, C107). */
  uint32_t video_mob_origin_x;
  uint32_t video_mob_origin_y;
  uint32_t video_mob_line_sample;
  uint32_t video_mob_prio_hides;
  /* Collision (plan 08-05, D-02, rules STIC-R44 and R45; stic_mob.c, stic_frame.c): the four pinned selectors, each 0 or 1 (the generator reads
   * video.collision_x0_interacts, video.collision_border_bit, video.collision_offscreen and video.collision_display_off and fails closed), all
   * unknown-default-chosen (C87): x0_interacts says whether a MOB with X coordinate 0 takes part in collision (default 0, no); border_bit whether a set
   * pixel of an interacting MOB inside the object grid and outside the plane sets bit 9 (default 1); offscreen whether MOB against MOB is evaluated
   * over the whole object grid and not only over the plane (default 1); display_off whether a frame with the display off evaluates collision at all
   * (default 0, no). */
  uint32_t video_collision_x0_interacts;
  uint32_t video_collision_border_bit;
  uint32_t video_collision_offscreen;
  uint32_t video_collision_display_off;
  /* Border extension (plan 08-06, D-02, rules STIC-R48 and R49; stic_frame.c): the generator reads video.border_ext_strip {cols, rows} (each 1..15,
   * unknown-default-chosen, C109): the width in plane columns of the left strip (extension register bit 0) and the height in plane rows of the top
   * strip (bit 1). The left width of 8 is the derivable core; the top height (default 8, the pinned unit PD-8) is the pinned part. */
  uint32_t video_border_ext_cols;
  uint32_t video_border_ext_rows;
  uint32_t video_display_off_fill;
} ivs_stic_params;

/* The number of profiles the generated parameter table covers. The registry (ivs_profile_get), the PSG
 * table, the control-line table and this table are four generated includes over the same sorted profile
 * files; the machine uses a registry index only below the smallest of the four counts
 * (ivs_machine_profile_covered, review WR-04), and test machine.create.tables-cover-registry requires all
 * four to equal the registry. */
uint32_t ivs_stic_params_count(void);

/* The parameters of registry profile index profile, or NULL for an index at or above
 * ivs_stic_params_count(). A live machine never holds such an index (the handle check refuses it), so the
 * machine code does not test for NULL: a missing row is a refusal at create, never a STIC that is quietly
 * absent. */
const ivs_stic_params *ivs_stic_params_get(uint32_t profile);

#endif

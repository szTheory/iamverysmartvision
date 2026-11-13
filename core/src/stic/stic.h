/* SPDX-License-Identifier: MIT */
/* STIC device state (plan 06-05, D-04, D-06, D-16, A-05, A-11): the O(1) state of the STIC, a flat,
 * pointer-free struct inside the machine instance (digest domain 4, INSTANCE). Internal; it lives in
 * core/src, which only the core and the tests (ivs_core_internal) see. The guest-visible register file
 * (64 words) is not here: it lives in ivs_memsys (stic_reg), beside the PSG registers, and is a MEM row.
 *
 * Plan 06-05 adds storage only: nothing reads or writes this state yet (the device plan 06-06 and the
 * timeline plan 06-08 do); the reset and validate functions below are the whole behaviour.
 *
 * Layout derivation (bytes, no implicit padding; machine.h states the instance arithmetic):
 *   frame_index      u64  at   0   index k of the frame in progress (never derived from time by modulo, D-07)
 *   event_index      u32  at   8   cursor into the per-frame event table, 0 .. IVS_STIC_EVENT_MAX - 1;
 *                                  0 means FRAME_START of frame_index is still to be processed
 *   display_latch    u32  at  12   0 or 1, the display-enable handshake of the current frame (D-06)
 *   windows_active   u32  at  16   0 or 1, decided at the first window of the frame from the latch
 *   mode             u32  at  20   0 Foreground/Background, 1 Color Stack (persistent, D-06)
 *   cs_ptr           u32  at  24   color-stack pointer 0..3: the final pointer of the last display-on frame rendered (each render starts at 0)
 *   blocked_reads    u32  at  28   reads refused by a closed gate (D-06), saturating
 *   blocked_writes   u32  at  32   writes dropped by a closed gate (D-06), saturating
 *   late_grants      u32  at  36   bus windows that ended without a grant (T-6)
 *   grants_in_frame  u32  at  40   grants of the frame in progress
 *   reserved0        u32  at  44   0, a PAD row of the digest table (never serialized)
 *   row_latch   u16[12][20] at 48  the BACKTAB row latches, 12 rows x 20 words = 480 bytes (A-05: window 12
 *                                  fetches nothing, so 12 rows and not 13), ending at 528
 * Total 528 = 8 + 10 x 4 + 480, a multiple of 8 (the u64 member makes the struct 8-aligned). */
#ifndef IVS_STIC_H
#define IVS_STIC_H

#include <stdint.h>

#include "ivs/ivs_machine.h"
#include "ivs/ivs_status.h"
#include "memmap/memsys.h"
#include "psg/psg.h"
#include "stic/stic_params.h"

#define IVS_STIC_LATCH_ROWS 12u  /* card rows with a fetch window (window 12 fetches nothing, A-05) */
#define IVS_STIC_ROW_WORDS 20u   /* BACKTAB words per card row */

typedef struct ivs_stic_state {
  uint64_t frame_index;
  uint32_t event_index;
  uint32_t display_latch;
  uint32_t windows_active;
  uint32_t mode;
  uint32_t cs_ptr;
  uint32_t blocked_reads;
  uint32_t blocked_writes;
  uint32_t late_grants;
  uint32_t grants_in_frame;
  uint32_t reserved0; /* 0 */
  uint16_t row_latch[IVS_STIC_LATCH_ROWS][IVS_STIC_ROW_WORDS];
} ivs_stic_state;

_Static_assert(sizeof(ivs_stic_state) == 528u, "ivs_stic_state must be 528 bytes (machine.h layout)");

/* Sets s to the power-on state (D-06): every member 0 (frame_index 0 and event_index 0, so FRAME_START of
 * frame 0 is still to be processed; windows_active 0; cs_ptr 0; the counters and the row latches 0),
 * except mode and display_latch, which come from the profile's stic.power_on record. */
void ivs_stic_state_reset(ivs_stic_state *s, const ivs_stic_params *p);

/* IVS_OK when every member is in range (the cursor below IVS_STIC_EVENT_MAX, display_latch and
 * windows_active 0 or 1, mode 0 or 1, cs_ptr below 4, reserved0 0; frame_index, the counters and the
 * row latches are any value), IVS_E_BAD_STATE otherwise, IVS_E_BAD_ARG for NULL. In the style of
 * ivs_cp1610_state_validate: a Phase 8 snapshot reload refuses a state this function does not accept. */
ivs_status ivs_stic_state_validate(const ivs_stic_state *s);

/* The frame sink of the STIC (plan 06-09, D-13, D-16, A-03): the per-call delivery target of every observation channel is the
 * psg-named struct ivs_psg_sink, whose video channel (video, video_cap, video_written, video_frames, video_overflow) takes the
 * slots a catch-up completes. NULL is accepted and means a counting video channel that is not tallied. */
typedef ivs_psg_sink ivs_stic_sink;

/* The VIDEO row of the instance (digest domain 16, plan 06-09, D-16): the per-frame chain and the count of frames chained. The same
 * 40 bytes as the earlier row; the machine's ivs_machine_obs_av is this type. The chain is h0 = SHA-256 of the 16 ASCII bytes
 * IVS-VIDEO-LOG-V1 and h(n+1) = SHA-256(h(n) || slot n) over the exact delivered slot bytes (header then plane rows). */
typedef struct ivs_stic_video_row {
  uint8_t chain[32];
  uint64_t count;
} ivs_stic_video_row;

/* Sets row to its create and power-on state: count 0 and chain h0 (stic_frame.c). Both call sites, create and reset, use it. */
void ivs_video_chain_init(ivs_stic_video_row *row);

/* The published surface derived from the PRIMARY geometry inputs of the parameters (card columns, last card column pixels, card
 * width, card rows, card height) and IVS_FRAME_HEADER_BYTES (stic_frame.c): width = (columns - 1) x card_w + last_px, height =
 * rows x card_h, stride = width (rows tightly packed), plane = width x height, slot = header + plane. Pure. A test compares the
 * generated derived members of ivs_stic_params with this function. */
typedef struct ivs_stic_geom {
  uint32_t width;
  uint32_t height;
  uint32_t stride;
  uint32_t plane_bytes;
  uint32_t header_bytes;
  uint32_t slot_bytes;
} ivs_stic_geom;
ivs_stic_geom ivs_stic_geom_derive(const ivs_stic_params *p);

/* The tick at which the frame in progress ends: frame_phase_ticks + (frame_index + 1) x frame_ticks, from the state (plan 06-09, D-14, A-06;
 * never derived from a time by division). The effective target of a run with IVS_RUN_F_STOP_ON_FRAME is min(target, this). */
uint64_t ivs_stic_next_frame_end(const ivs_stic_params *p, const ivs_stic_state *s);

/* The working plane the renderer fills: card_columns x card_w by card_rows x card_h index bytes (160 x 96 = 15,360 for the first
 * profile); the published rows are the first width bytes of each row. The core never holds the whole plane on its stack (a 15 KB frame
 * pulls in the platform stack probe, a forbidden import): it renders one card row at a time, card_h pixel rows of the working width,
 * at most IVS_STIC_CARDROW_MAX bytes (8 x 160 = 1,280 for the first profile). */
#define IVS_STIC_PLANE_MAX 15360u
#define IVS_STIC_CARDROW_MAX 2048u

/* The inputs of the card renderer (plan 06-10, D-18, D-19): a value copy of the card mode (IVS_STIC_MODE_FGBG or IVS_STIC_MODE_COLOR_STACK),
 * the border color (0..15) and the four color-stack values (0..15, registers $28..$2B), each already masked by the caller. Nothing else
 * of the machine reaches the renderer. */
typedef struct ivs_stic_cardin {
  uint32_t mode;
  uint32_t border;
  uint32_t stack[4];
} ivs_stic_cardin;

/* Pure card renderer (stic_cards.c; no static or global data, no allocation, no state kept between calls). Row form: renders the 20
 * BACKTAB words row_words[0..19] of ONE card row into out, the 8 pixel rows of the 160-wide working plane (1,280 bytes, stride 160).
 * W = word and $3FFF (bits 15..14 ignored). Foreground/Background word: sel = bit 11 (0 graphics ROM, 1 graphics RAM), card = bits
 * 8..3, fg = bits 2..0, bg = 8 x bit 12 + 4 x bit 13 + 2 x bit 10 + bit 9; the pixel of graphics byte y at x is fg when bit 7 - x is
 * set, else bg. Color Stack word, by the pair of bits 12 and 11: 00 graphics ROM card (card = bits 10..3, fg = bits 2..0), 01 graphics
 * RAM card (card = bits 8..3, fg = bits 2..0), 11 graphics RAM card with fg = 8 + bits 2..0, 10 Colored Squares (four 4 x 4 quadrants,
 * colors bits 2..0, 5..3, 8..6 and 4 x b13 + 2 x b10 + b9, color 7 the current stack entry); a card layout has bit 13 advance the
 * pointer, pos = (pos + 1) mod 4, before the card is drawn, and draws its background from in->stack[pos]. cs_pos carries the pointer
 * in and out (the caller starts a frame at 0 and passes the same variable for the 12 rows; the Foreground/Background mode leaves it
 * unchanged). In column 19 the pixel x = 7 is IVS_PIXEL_BLANK (the cut-off column, clip side right). The graphics bytes come only from
 * ivs_memsys_stic_gfx_byte_internal. Returns the bytes written (1,280). */
uint32_t ivs_stic_render_cards_row(const ivs_stic_cardin *in, const uint16_t *row_words, const ivs_memsys *mem, uint32_t *cs_pos, uint8_t *out);

/* Plane form: the 240 BACKTAB words (cell r, c is word 20 r + c) into the 160 x 96 working plane plane (15,360 bytes, caller-owned),
 * the color-stack pointer starting at 0. When cs_final is not NULL it receives the final pointer 0..3. Returns the bytes written. The
 * unit-path tests use it; the machine renders one card row at a time (no 15 KB stack frame). */
uint32_t ivs_stic_render_cards(const ivs_stic_cardin *in, const uint16_t *words, const ivs_memsys *mem, uint32_t *cs_final, uint8_t *plane);

/* Fills out with the pixel rows of card row cr (0 .. card_rows - 1), row stride card_columns x card_w: a display-off frame (displayed
 * 0) with the border color index (stic_reg 0x2C & 0xF, video.display_off_fill border, C99); a display-on Foreground/Background frame
 * through ivs_stic_render_cards_row from s->row_latch and the registers, in the mode of s; a display-on Color Stack frame carries the
 * pointer cs_pos (in and out) across the rows. Returns the bytes filled (card_h x working width), 0 when one card row does not fit
 * IVS_STIC_CARDROW_MAX. */
uint32_t ivs_stic_render_card_row(const ivs_stic_params *p, const ivs_stic_state *s, const ivs_memsys *mem, uint32_t displayed, uint32_t cr,
                                  uint32_t *cs_pos, uint8_t *out);

/* The whole working plane, card row by card row, into a caller-owned buffer of at least IVS_STIC_PLANE_MAX bytes (a test and review
 * entry point; the frame writer below streams card rows instead). Returns the bytes filled, 0 when the plane does not fit. */
uint32_t ivs_stic_render_plane(const ivs_stic_params *p, const ivs_stic_state *s, const ivs_memsys *mem, uint32_t displayed, uint8_t *plane);
/* Same as ivs_stic_render_plane, and also stores the final color-stack pointer in *cs_final when it is not NULL. */
uint32_t ivs_stic_render_plane_cs(const ivs_stic_params *p, const ivs_stic_state *s, const ivs_memsys *mem, uint32_t displayed, uint8_t *plane,
                                  uint32_t *cs_final);

/* Completes the frame that ends at end_tick: renders it from the state and the register file (a display-on frame leaves the final
 * color-stack pointer of its render in s->cs_ptr, plan 06-11; a display-off frame leaves it), frames the 32-byte header (be64 tick,
 * be64 index = row->count, be32 width, be32 height, be32 flags with bit 0 set when s->windows_active, be32 0), advances the chain
 * and the count of row whether or not a buffer is present, and copies the slot to sink->video when it has room (else sets
 * video_overflow). sink NULL is the counting channel. Called by the FRAME_END handler of ivs_stic_catch_up at the event. */
void ivs_stic_frame_deliver(const ivs_stic_params *p, ivs_stic_state *s, const ivs_memsys *mem, ivs_stic_video_row *row,
                            ivs_stic_sink *sink, uint64_t end_tick);

/* Moves the timeline cursor of s to machine tick to_tick: applies, in order, every event of the per-frame table whose
 * tick is at or below to_tick (the next tick is phase + frame_index x F + event_tick[event_index], from the state),
 * opening and closing the register and graphics-memory gates of mem and deciding windows_active (stic_events.c, D-05,
 * D-07, D-10, A-06). Idempotent for one to_tick. At each FRAME_END event (plan 06-09) the frame is completed through
 * ivs_stic_frame_deliver into row and sink when row is not NULL (a NULL row delivers nothing: only the module-level tests pass it). */
void ivs_stic_catch_up(const ivs_stic_params *p, ivs_stic_state *s, ivs_memsys *mem, uint64_t to_tick, ivs_stic_sink *sink,
                       ivs_stic_video_row *row);

/* The two CPU line levels the STIC drives (stic_lines.c, plan 06-08, D-08, D-09): INTRM (the VBLANK interrupt request) and BUSRQ
 * (the bus request of the 13 fetch windows), each 0 or 1. */
typedef struct ivs_stic_line_levels {
  uint32_t intrm;
  uint32_t busrq;
} ivs_stic_line_levels;

/* The levels at machine tick `tick` of the frame in progress: S_k = frame_phase_ticks + frame_index x F from the state, rel = tick - S_k.
 * A tick before S_k is in the previous frame, where both levels are 0 (INTRM ended and the last window closed before the frame end).
 * INTRM is 0 <= rel < irq_low_ticks, independent of the display latch; BUSRQ needs windows_active and window index
 * (rel - first_start) / pitch below bus_count with (rel - first_start) % pitch < hold. Pure: it reads the state and never advances it
 * (the event cursor is the only clock, D-07), so the machine seam may call it at any unit start whatever the host's slicing. */
ivs_stic_line_levels ivs_stic_lines(const ivs_stic_params *p, const ivs_stic_state *s, uint64_t tick);

/* Access effects (stic_regs.c, plan 06-06; pure functions of the parameters, no address constant). access is
 * IVS_STIC_ACCESS_READ (a READ or FETCH) or IVS_STIC_ACCESS_WRITE (a WRITE or DW); the result is an IVS_STIC_EFFECT_*.
 * A read has an effect only when its address is in stic.mode_select_addrs (decided on the address before decode, A-07). */
uint32_t ivs_stic_is_mode_select_addr(const ivs_stic_params *p, uint32_t addr);
uint32_t ivs_stic_effect(const ivs_stic_params *p, uint32_t access, uint32_t addr);
/* Applies an effect to the display latch or the mode of s; any other value does nothing. */
void ivs_stic_apply_effect(ivs_stic_state *s, uint32_t effect);
/* Counts one refused access (blocked_reads when is_read is nonzero, else blocked_writes), saturating at 0xFFFFFFFF. */
void ivs_stic_count_blocked(ivs_stic_state *s, uint32_t is_read);

#endif

/* SPDX-License-Identifier: MIT */
/* STIC raw frame delivery (plan 06-09; D-12, D-13, D-16, A-03, A-06): the geometry derived at run time from the primary inputs, the
 * render entry point, the slot writer and the domain-16 chain.
 *
 * One slot per completed frame, written at the FRAME_END event (stamp E_k = S_(k+1)): a frame contains the accesses stamped strictly
 * below E_k, because the machine catches the STIC up to an access stamp before the access acts. The slot is a 32-byte header written
 * byte by byte, big-endian (be64 tick E_k, be64 index = the frames completed before this one, be32 width, be32 height, be32 flags with
 * bit 0 IVS_FRAME_F_DISPLAYED, be32 reserved 0) followed by `height` rows of `width` index bytes (the working plane is card_columns x
 * card_w wide, the published rows are its first `width` bytes: the cut-off 20th column, D-12). No struct is ever hashed by memory.
 *
 * The chain (domain 16) is h0 = SHA-256 of the 16 ASCII bytes IVS-VIDEO-LOG-V1 and h(n+1) = SHA-256(h(n) || header || rows), streamed
 * through the incremental SHA-256 over exactly the delivered slot bytes. It is advanced for every completed frame whether or not a
 * buffer is present, so the machine state never depends on a buffer and a sliced run equals one call (the chain is per frame, not per
 * call).
 *
 * Render (plans 06-10 and 06-11): a display-off frame is filled with the border color index (stic_reg 0x2C and 0xF,
 * video.display_off_fill border, C99); a display-on frame, Foreground/Background or Color Stack, is decoded card row by card row from
 * the row latches and the registers by the pure renderer of stic_cards.c, the color-stack pointer carried across the rows from 0; the
 * final pointer of a display-on frame is stored in s->cs_ptr (observable through the plan 06-07 report, not cleared at frame start).
 * Plan 08-04: each card row goes through ivs_stic_compose_card_row (this file), which renders the card row and overlays the moving objects
 * of stic_mob.c on its plane rows; the streamed slot and the whole plane (ivs_stic_render_plane_cs) share that one path, so they are equal
 * by construction. With every MOB register at 0 the composition returns the card row untouched.
 *
 * Named faults (plan 06-09): VIDEO_STATE_DEPENDS_ON_BUFFER (with no buffer the chain and the count do not advance),
 * VIDEO_CHAIN_FRAMING_STRUCT_HASH (the chain hashes an image of a struct instead of the framed bytes). The file holds no static or
 * global state. */
#include "common/ivs_prelude.h"

#include "common/ivs_sha256.h"
#include "stic/stic.h"

/* The border color register of the STIC register file (stic.regmap, $2C). */
#define IVS_STIC_REG_BORDER 0x2Cu
/* The first of the four color-stack registers ($28..$2B). */
#define IVS_STIC_REG_STACK0 0x28u

uint64_t ivs_stic_next_frame_end(const ivs_stic_params *p, const ivs_stic_state *s)
{
  /* E_next = phase + (frame_index + 1) x F, from the state: the frame index is a count of FRAME_END events, never now / F (D-07). */
  return (uint64_t)p->frame_phase_ticks + (s->frame_index + 1u) * (uint64_t)p->frame_ticks;
}

ivs_stic_geom ivs_stic_geom_derive(const ivs_stic_params *p)
{
  ivs_stic_geom g;
  g.width = (p->video_card_columns - 1u) * p->video_card_w + p->video_last_column_px;
  g.height = p->video_card_rows * p->video_card_h;
  g.stride = g.width;
  g.plane_bytes = g.width * g.height;
  g.header_bytes = IVS_FRAME_HEADER_BYTES;
  g.slot_bytes = g.header_bytes + g.plane_bytes;
  return g;
}

void ivs_video_chain_init(ivs_stic_video_row *row)
{
  static const char tag[] = "IVS-VIDEO-LOG-V1";
  memset(row, 0, sizeof *row);
  ivs_sha256_digest((const uint8_t *)tag, sizeof tag - 1u, row->chain);
}

/* 1 when the card decoder draws card row cr under the geometry of p (20 x 12 cards of 8 x 8 positions): the one condition under which a
 * display-on row is decoded from the row latches and moving objects are overlaid. */
static uint32_t ivs_stic_cards_drawable(const ivs_stic_params *p, uint32_t cr)
{
  return (p->video_card_columns == IVS_STIC_ROW_WORDS && p->video_card_w == 8u && p->video_card_h == 8u && cr < IVS_STIC_LATCH_ROWS) ? 1u : 0u;
}

/* The inputs of the card renderer from the mode of s and the border and color-stack registers of mem, each masked to 4 bits. */
static void ivs_stic_cardin_get(const ivs_stic_state *s, const ivs_memsys *mem, ivs_stic_cardin *in)
{
  uint32_t k;
  in->mode = s->mode;
  in->border = (uint32_t)mem->stic_reg[IVS_STIC_REG_BORDER] & 0xFu;
  for (k = 0u; k < 4u; k++) {
    in->stack[k] = (uint32_t)mem->stic_reg[IVS_STIC_REG_STACK0 + k] & 0xFu;
  }
}

uint32_t ivs_stic_render_card_row(const ivs_stic_params *p, const ivs_stic_state *s, const ivs_memsys *mem, uint32_t displayed, uint32_t cr,
                                  uint32_t *cs_pos, uint8_t *out)
{
  uint32_t work = p->video_card_columns * p->video_card_w * p->video_card_h;
  uint8_t fill;
  if (work > IVS_STIC_CARDROW_MAX) {
    return 0u;
  }
  if (displayed != 0u) {
    if (ivs_stic_cards_drawable(p, cr) != 0u) {
      ivs_stic_cardin in;
      ivs_stic_cardin_get(s, mem, &in);
      return ivs_stic_render_cards_row(&in, s->row_latch[cr], mem, cs_pos, out);
    }
    fill = (uint8_t)p->video_blank_value; /* a geometry the card decoder does not draw: every position not displayed */
  } else {
    fill = (uint8_t)(mem->stic_reg[IVS_STIC_REG_BORDER] & 0xFu); /* display off: the border color index */
  }
  memset(out, fill, work);
  return work;
}

uint32_t ivs_stic_compose_card_row(const ivs_stic_params *p, const ivs_stic_state *s, const ivs_memsys *mem, uint32_t displayed, uint32_t cr,
                                   uint32_t *cs_pos, uint8_t *out)
{
  ivs_stic_mob mobs[IVS_STIC_MOBS];
  uint8_t on_mask[IVS_STIC_ROW_WORDS * 8u]; /* the on pixels of the card layer of this card row, 20 bytes per plane row */
  uint32_t work_w = p->video_card_columns * p->video_card_w;
  uint32_t n = ivs_stic_render_card_row(p, s, mem, displayed, cr, cs_pos, out);
  uint32_t k;
  if (n == 0u || displayed == 0u || ivs_stic_cards_drawable(p, cr) == 0u) {
    return n; /* a display-off row is the border fill and draws no MOB */
  }
  if (ivs_stic_mob_decode(p, mem, mobs) == 0u) {
    return n; /* no MOB is on: the row is exactly the card layer (every MOB register at its reset value draws nothing) */
  }
  {
    ivs_stic_cardin in;
    ivs_stic_cardin_get(s, mem, &in);
    (void)ivs_stic_cards_on_row(&in, s->row_latch[cr], mem, on_mask);
  }
  for (k = 0u; k < p->video_card_h; k++) {
    ivs_stic_overlay_mobs_row(p, mem, mobs, cr * p->video_card_h + k, on_mask + (size_t)k * IVS_STIC_ROW_WORDS, out + (size_t)k * work_w);
  }
  return n;
}

uint32_t ivs_stic_render_plane_cs(const ivs_stic_params *p, const ivs_stic_state *s, const ivs_memsys *mem, uint32_t displayed, uint8_t *plane,
                                  uint32_t *cs_final)
{
  uint32_t row_bytes = p->video_card_columns * p->video_card_w * p->video_card_h;
  uint32_t pos = 0u;
  uint32_t cr;
  if (row_bytes * p->video_card_rows > IVS_STIC_PLANE_MAX) {
    return 0u;
  }
  for (cr = 0u; cr < p->video_card_rows; cr++) {
    if (ivs_stic_compose_card_row(p, s, mem, displayed, cr, &pos, plane + (size_t)cr * row_bytes) == 0u) {
      return 0u;
    }
  }
  if (cs_final != NULL) {
    *cs_final = pos;
  }
  return row_bytes * p->video_card_rows;
}

uint32_t ivs_stic_render_plane(const ivs_stic_params *p, const ivs_stic_state *s, const ivs_memsys *mem, uint32_t displayed, uint8_t *plane)
{
  return ivs_stic_render_plane_cs(p, s, mem, displayed, plane, NULL);
}

/* Writes v big-endian into n bytes at out, one byte at a time. */
static void ivs_frame_be(uint8_t *out, uint32_t n, uint64_t v)
{
  uint32_t i;
  for (i = 0u; i < n; i++) {
    out[i] = (uint8_t)((v >> (8u * (n - 1u - i))) & 0xFFu);
  }
}

void ivs_stic_frame_deliver(const ivs_stic_params *p, ivs_stic_state *s, const ivs_memsys *mem, ivs_stic_video_row *row,
                            ivs_stic_sink *sink, uint64_t end_tick)
{
  uint8_t cardrow[IVS_STIC_CARDROW_MAX];
  uint8_t header[IVS_FRAME_HEADER_BYTES];
  ivs_sha256 ctx;
  ivs_stic_geom g = ivs_stic_geom_derive(p);
  uint32_t work_w = p->video_card_columns * p->video_card_w;
  uint32_t displayed = s->windows_active != 0u ? 1u : 0u;
  uint32_t cr;
  uint32_t py;
  uint32_t y = 0u;
  uint32_t cs_pos = 0u; /* the color-stack pointer restarts at 0 for every frame render (D-06) */
  uint32_t deliver;
  uint8_t *out = NULL;
  if (g.width > work_w || g.header_bytes != IVS_FRAME_HEADER_BYTES || p->video_card_h * work_w > IVS_STIC_CARDROW_MAX) {
    /* A geometry the card-row working buffer cannot hold: no frame is produced. The generator refuses such a profile
     * (cmake/IvsSticTable.cmake), so this is never reached; if it ever is, the run ends INTERNAL through video_overflow
     * instead of letting frame_index advance with no frame (Phase 6 review WR-03). */
#ifndef IVS_FAULT_VIDEO_GEOMETRY_SKIP_SILENT
    if (sink != NULL) {
      sink->video_overflow = 1u;
    }
#endif
    return;
  }
  memset(header, 0, sizeof header);
  ivs_frame_be(header + 0u, 8u, end_tick);
  ivs_frame_be(header + 8u, 8u, row->count);
  ivs_frame_be(header + 16u, 4u, g.width);
  ivs_frame_be(header + 20u, 4u, g.height);
  ivs_frame_be(header + 24u, 4u, displayed != 0u ? IVS_FRAME_F_DISPLAYED : 0u);
  ivs_frame_be(header + 28u, 4u, 0u);
#ifdef IVS_FAULT_VIDEO_STATE_DEPENDS_ON_BUFFER
  if (sink == NULL || sink->video == NULL) {
    return; /* named fault: with no buffer the chain and the count do not advance */
  }
#endif
  deliver = 0u;
  if (sink != NULL && sink->video != NULL) {
    if (sink->video_cap - sink->video_written >= (uint64_t)g.slot_bytes) {
      deliver = 1u;
      out = sink->video + sink->video_written;
      memcpy(out, header, sizeof header);
    } else {
      sink->video_overflow = 1u; /* never expected: the capacity rule of the run call proves the room */
    }
  }
  ivs_sha256_init(&ctx);
  ivs_sha256_update(&ctx, row->chain, 32u);
#ifdef IVS_FAULT_VIDEO_CHAIN_FRAMING_STRUCT_HASH
  {
    /* named fault: the chain hashes an image of a struct (host byte order and padding) instead of the framed header */
    struct {
      uint32_t flags;
      uint64_t tick;
      uint32_t width;
      uint64_t index;
      uint32_t height;
    } image;
    memset(&image, 0, sizeof image);
    image.flags = displayed != 0u ? IVS_FRAME_F_DISPLAYED : 0u;
    image.tick = end_tick;
    image.width = g.width;
    image.index = row->count;
    image.height = g.height;
    ivs_sha256_update(&ctx, (const uint8_t *)&image, sizeof image);
  }
#else
  ivs_sha256_update(&ctx, header, sizeof header);
#endif
  for (cr = 0u; cr < p->video_card_rows; cr++) {
    if (ivs_stic_compose_card_row(p, s, mem, displayed, cr, &cs_pos, cardrow) == 0u) {
      if (sink != NULL) {
        sink->video_overflow = 1u; /* unreachable for a generated profile (see above): a short frame is an INTERNAL stop, never a quiet one */
      }
      break;
    }
    for (py = 0u; py < p->video_card_h && y < g.height; py++, y++) {
      const uint8_t *src = cardrow + (size_t)py * work_w;
      ivs_sha256_update(&ctx, src, g.width);
      if (deliver != 0u) {
        memcpy(out + g.header_bytes + (size_t)y * g.stride, src, g.width);
      }
    }
  }
  ivs_sha256_final(&ctx, row->chain);
  row->count += 1u;
  if (displayed != 0u) {
    s->cs_ptr = cs_pos & 3u; /* the final pointer of the frame just rendered; a display-off frame leaves it as it was */
  }
  if (sink != NULL) {
    sink->video_frames += 1u;
    if (deliver != 0u) {
      sink->video_written += (uint64_t)g.slot_bytes;
    }
  }
}

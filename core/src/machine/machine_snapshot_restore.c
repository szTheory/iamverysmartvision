/* SPDX-License-Identifier: MIT */
/* The snapshot restore (plan 08-13, D-14, D-15, D-41, docs/formats/ivsn-v1.md "Check order", "Refusal reasons" and "Derived invariants"):
 * puts the state of an IVSN v1 container back into an instance. Restore sits on the reader of plan 08-12 (machine_snapshot_read.c, the
 * layer that judges the container with no instance) and adds the layers that judge what the snapshot means for one instance: the
 * identity (profile, parameter digest, images) and the content (the rows, the device validators, the derived invariants).
 *
 * Validate everything, then commit (D-15). Nothing is written to the instance until every check has passed: the sections are decoded
 * into a probe (machine.h, a few hundred bytes of the small device structs and the scalars the invariants read) and judged there, and
 * only then does a commit that has no refusal path copy the decoded rows into the instance. A refusal therefore leaves every byte of the
 * instance as it was and writes only the report. The decoder is the digest field table itself (machine_digest.c, the single
 * serializer, D-11): one walk over the rows of a section, big-endian, a u32 count before an array row, in two modes, validate (into the
 * probe) and commit (straight into the instance). The class column of the table says what a restore does with a row: STATE rows are
 * decoded and stored, PAD rows are zeroed, BOUND and IMAGE rows are never written (the handle magic, the profile index, the committed
 * image structure, the three slot records and the image arrays stay what the target holds).
 *
 * Check order (the first failing class wins; the number is the reason and the step is the step of the specification):
 *    1  arguments  an unusable report struct (nothing written); a NULL buffer with a length above 0 (58); the report over the buffer (59);
 *                  an invalid handle (BAD_HANDLE); the buffer or the report over the instance (81)
 *    2-6 container the reader of plan 08-12, unchanged: length, header, section table, trailer, form of the profile id (60 to 71)
 *    6  profile    the id registered (72, UNKNOWN_PROFILE), equal to the instance's (73), the parameter digest equal to this build's (74)
 *    7  images     per slot in the order EXEC, GROM, cartridge: the loaded flag at most 1 and an empty record empty (78), a slot loaded in
 *                  the snapshot and not in the instance (75, MISSING_SYSTEM_IMAGE), a slot loaded only in the instance or loaded on both
 *                  sides with another SHA-256 or length (76)
 *    8  content    every section length equal to its derived length (80); every array row count equal to the table's (77); every frame,
 *                  every device state through its validator, the phase and the clock range (78); the derived invariants (79)
 *    9  commit     table-driven stores that cannot fail, the phase becomes the snapshot's POWERED
 * The images step has no check of the recorded IMAGES domain digest: a snapshot has no IMAGES section (the three slot records are its whole
 * image structure), so there is nothing recorded to compare with the live structure, and the slot identities are what is compared.
 * The eight digest fields of the header (composite and the seven section digests) are informational (D-41) and never checked: the trailer is
 * integrity, not authenticity, and a hostile snapshot with a recomputed trailer is refused by the validators and the invariants.
 *
 * Reasons and statuses (docs/formats/ivsn-v1.md): 72 UNKNOWN_PROFILE; 73, 74, 76 STATE_MISMATCH; 75 MISSING_SYSTEM_IMAGE; 77 to 80 MALFORMED;
 * 81 BAD_ARG. Offsets reported: the offset of the id field (72, 73), of the parameter digest field (74), of the field of the slot record
 * that failed (75, 76, and 78 for a slot record), of the length field of the section table entry (80), of the array count (77), and for
 * 78 and 79 the offset in the container of the first row of the struct or scalar that failed (the section index says which section).
 *
 * The invariants (INV-1 to INV-6) are functions of the probe, shared with the live-instance probe builder so that a test can prove each
 * one true on every unit boundary of real programs before it is allowed to refuse a snapshot (state.restore.invariants-hold-reachable).
 * Named faults (plan 08-13, suite state), one site each below: RESTORE_PROFILE_UNCHECKED (the profile id, parameter digest and registry
 * checks are skipped), RESTORE_IMAGE_DIGEST_UNCHECKED (the SHA-256 and length comparison of a slot loaded on both sides is skipped) and
 * RESTORE_MUTATES_BEFORE_VALIDATE (the CPU rows are committed before the STIC validators run).
 * This file holds no static or global writable state, allocates nothing, reads no clock and opens no file; its frame is well under 4 KB. */
#include "common/ivs_prelude.h"

#include "ivs/ivs.h"
#include "machine/machine.h"

#define IVS_SNAP_TIME_MAX IVS_RUN_TARGET_MAX /* 2^63: the clock range of every machine (a run target is at most this) */
#define IVS_SNAP_NO_OFFSET 0xFFFFFFFFu

/* ---- the probe regions: which instance bytes the probe holds, and where ----
 * A STATE row whose instance bytes lie inside one of these regions is decoded into the probe at the same offset inside the member of the
 * probe; every other STATE row (the RAM words, the register file, the chains, the counts the invariants do not read) is read, counted and
 * stepped over in validate mode. The offsets inside a member are those of the instance because the types are the same. */
typedef struct ivs_snap_region {
  uint32_t inst_off;
  uint32_t size;
  uint32_t probe_off;
} ivs_snap_region;

#define IVS_SNAP_REG(imem, pmem)                                                                                  \
  {(uint32_t)offsetof(struct ivs_machine, imem), (uint32_t)sizeof(((ivs_snap_probe *)0)->pmem),                    \
   (uint32_t)offsetof(ivs_snap_probe, pmem)}

#define IVS_SNAP_R_TIME 0u
#define IVS_SNAP_R_PHASE 1u
#define IVS_SNAP_R_CPU 2u
#define IVS_SNAP_R_GATE 3u
#define IVS_SNAP_R_PSGREG 4u
#define IVS_SNAP_R_PULLDOWN 5u
#define IVS_SNAP_R_PSG 6u
#define IVS_SNAP_R_STIC 7u
#define IVS_SNAP_R_VIDEO 8u
#define IVS_SNAP_R_EMITTED 9u
#define IVS_SNAP_R_PREV 10u
#define IVS_SNAP_R_INPUT 11u
#define IVS_SNAP_REGIONS 12u

static const ivs_snap_region ivs_snap_regions[IVS_SNAP_REGIONS] = {
    IVS_SNAP_REG(time_ticks, time_ticks),
    IVS_SNAP_REG(phase, phase),
    IVS_SNAP_REG(cpu, cpu),
    IVS_SNAP_REG(mem.gate_open, gate_open),
    IVS_SNAP_REG(mem.psg_reg, psg_reg),
    IVS_SNAP_REG(mem.port_pulldown, port_pulldown),
    IVS_SNAP_REG(psg, psg),
    IVS_SNAP_REG(stic, stic),
    IVS_SNAP_REG(obs.video.count, video_count),
    IVS_SNAP_REG(obs.audio.emitted, audio_emitted),
    IVS_SNAP_REG(obs.audio.prev_levels, audio_prev_levels),
    IVS_SNAP_REG(obs.input, input),
};

/* Where in the container the first row of a region was read, and in which section. */
typedef struct ivs_snap_loc {
  uint32_t off;
  uint32_t section;
} ivs_snap_loc;

/* A refusal under construction: the status, the reason, the offset and the section index the report will say, and the slot kind of a
 * refusal about an image slot (0 otherwise). */
typedef struct ivs_snap_refusal {
  ivs_status st;
  uint32_t reason;
  uint64_t off;
  uint32_t section;
  uint32_t kind;
} ivs_snap_refusal;

static ivs_status ivs_snap_refuse(ivs_snap_refusal *r, ivs_status st, uint32_t reason, uint64_t off, uint32_t section, uint32_t kind)
{
  r->st = st;
  r->reason = reason;
  r->off = off;
  r->section = section;
  r->kind = kind;
  return st;
}

/* ---- reading and storing one element ---- */

/* The big-endian value of width bytes (1, 2, 4 or 8) at p. */
static uint64_t ivs_snap_get(const uint8_t *p, uint32_t width)
{
  uint64_t v = 0u;
  uint32_t i;
  for (i = 0u; i < width; i++) {
    v = (v << 8) | (uint64_t)p[i];
  }
  return v;
}

/* Stores v as a native value of width bytes at dst (one memcpy of one scalar, alias safe), so the instance holds the value whatever the
 * byte order of the host. */
static void ivs_snap_store(uint8_t *dst, uint64_t v, uint32_t width)
{
  uint8_t u8v;
  uint16_t u16v;
  uint32_t u32v;
  switch (width) {
  case 1u:
    u8v = (uint8_t)v;
    memcpy(dst, &u8v, 1u);
    break;
  case 2u:
    u16v = (uint16_t)v;
    memcpy(dst, &u16v, 2u);
    break;
  case 4u:
    u32v = (uint32_t)v;
    memcpy(dst, &u32v, 4u);
    break;
  default:
    memcpy(dst, &v, 8u);
    break;
  }
}

/* 1 when row f is a row of the snapshot of section domain: a STATE row of that domain (the class says STATE only for UINT rows). */
static uint32_t ivs_snap_state_row(const ivs_digest_field *f, uint32_t domain)
{
  return (uint32_t)(f->domain == domain && f->snap == IVS_SNAP_STATE && f->kind == IVS_DIGEST_KIND_UINT);
}

/* The probe region that holds the instance bytes of row f, or IVS_SNAP_REGIONS for none. */
static uint32_t ivs_snap_region_of(const ivs_digest_field *f)
{
  uint32_t k;
  uint64_t end = (uint64_t)f->offset + (uint64_t)f->width * f->count;
  for (k = 0u; k < IVS_SNAP_REGIONS; k++) {
    if (f->offset >= ivs_snap_regions[k].inst_off && end <= (uint64_t)ivs_snap_regions[k].inst_off + ivs_snap_regions[k].size) {
      return k;
    }
  }
  return IVS_SNAP_REGIONS;
}

/* ---- the three passes over the sections (all read only except the commit) ---- */

/* Step 8, reason 80: every section length equals the length derived from the table of this build. The reader ties the sum and the
 * adjacency, so this is reached only by changes that chain and keep the sum. */
static ivs_status ivs_snap_check_lengths(const ivs_snap_view *v, ivs_snap_refusal *r)
{
  uint32_t i;
  for (i = 0u; i < IVS_SNAP_SECTIONS; i++) {
    if (v->section[i].length != ivs_snapshot_section_derived_length(i)) {
      return ivs_snap_refuse(r, IVS_E_MALFORMED, IVS_REASON_SNAP_SECTION_LENGTH,
                             (uint64_t)IVS_SNAP_OFF_TABLE + (uint64_t)IVS_SNAP_ENTRY_BYTES * i + 8u, i, 0u);
    }
  }
  return IVS_OK;
}

/* Step 8, reason 77: every array row announces the count of the table. The position of each row inside a section is a function of the
 * table alone (7 bytes of frame, then per STATE row its width times its count plus 4 bytes for an array), and the section length equals
 * the sum of those, so every count read here, and every element read by the decoder, lies inside its section. */
static ivs_status ivs_snap_check_counts(const uint8_t *buf, const ivs_snap_view *v, ivs_snap_refusal *r)
{
  size_t rows = ivs_machine_digest_field_count();
  uint32_t i;
  for (i = 0u; i < IVS_SNAP_SECTIONS; i++) {
    uint32_t pos = IVS_SNAP_FRAME_BYTES;
    size_t k;
    for (k = 0u; k < rows; k++) {
      const ivs_digest_field *f = ivs_machine_digest_field(k);
      if (f == NULL || ivs_snap_state_row(f, v->section[i].domain) == 0u) {
        continue;
      }
      if (f->count > 1u) {
        uint64_t got = ivs_snap_get(buf + v->section[i].offset + pos, 4u);
        if (got != f->count) {
          return ivs_snap_refuse(r, IVS_E_MALFORMED, IVS_REASON_SNAP_ROW_COUNT, (uint64_t)v->section[i].offset + pos, i, 0u);
        }
        pos += 4u;
      }
      pos += f->width * f->count;
    }
  }
  return IVS_OK;
}

/* Step 8, reason 78 (a value out of range): the frame of a section is the bytes IVSD, the domain of the table entry and the domain
 * version 1; anything else is a value no writer produces. */
static ivs_status ivs_snap_check_frames(const uint8_t *buf, const ivs_snap_view *v, ivs_snap_refusal *r)
{
  static const uint8_t tag[4] = {0x49u, 0x56u, 0x53u, 0x44u}; /* IVSD */
  uint32_t i;
  for (i = 0u; i < IVS_SNAP_SECTIONS; i++) {
    const uint8_t *d = buf + v->section[i].offset;
    uint32_t k;
    for (k = 0u; k < IVS_SNAP_FRAME_BYTES; k++) {
      uint32_t want;
      if (k < 4u) {
        want = tag[k];
      } else if (k == 4u) {
        want = v->section[i].domain;
      } else {
        want = k == 5u ? (IVS_DIGEST_DOMAIN_VERSION >> 8) : (IVS_DIGEST_DOMAIN_VERSION & 0xFFu);
      }
      if ((uint32_t)d[k] != want) {
        return ivs_snap_refuse(r, IVS_E_MALFORMED, IVS_REASON_SNAP_FIELD_RANGE, (uint64_t)v->section[i].offset + k, i, 0u);
      }
    }
  }
  return IVS_OK;
}

/* The decoder of one section in validate mode: every STATE row of the section's domain in table order, each element read big-endian and
 * stored into the probe when a region of the probe holds the row. loc records where the first row of each region lies. */
static void ivs_snap_decode_probe(const uint8_t *buf, const ivs_snap_view *v, uint32_t i, ivs_snap_probe *pr, ivs_snap_loc loc[IVS_SNAP_REGIONS])
{
  const uint8_t *d = buf + v->section[i].offset;
  size_t rows = ivs_machine_digest_field_count();
  uint32_t pos = IVS_SNAP_FRAME_BYTES;
  size_t k;
  for (k = 0u; k < rows; k++) {
    const ivs_digest_field *f = ivs_machine_digest_field(k);
    uint32_t reg;
    uint32_t e;
    if (f == NULL || ivs_snap_state_row(f, v->section[i].domain) == 0u) {
      continue;
    }
    if (f->count > 1u) {
      pos += 4u; /* the count, already compared with the table */
    }
    reg = ivs_snap_region_of(f);
    if (reg < IVS_SNAP_REGIONS && loc[reg].off == IVS_SNAP_NO_OFFSET) {
      loc[reg].off = v->section[i].offset + pos;
      loc[reg].section = i;
    }
    for (e = 0u; e < f->count; e++) {
      if (reg < IVS_SNAP_REGIONS) {
        uint8_t *dst = (uint8_t *)pr + ivs_snap_regions[reg].probe_off + (f->offset - ivs_snap_regions[reg].inst_off) + (size_t)e * f->width;
        ivs_snap_store(dst, ivs_snap_get(d + pos, f->width), f->width);
      }
      pos += f->width;
    }
  }
}

/* Step 9, the commit: copy only, no check and no early return. Every STATE row is decoded straight into the instance, every PAD row is
 * zeroed, no BOUND or IMAGE row is touched (the magic, the profile index, the committed image structure, the slot records and the three
 * image arrays stay), and the phase row, which every accepted snapshot holds as POWERED, makes the machine POWERED. */
static void ivs_snap_commit(struct ivs_machine *im, const uint8_t *buf, const ivs_snap_view *v)
{
  uint8_t *base = (uint8_t *)im;
  size_t rows = ivs_machine_digest_field_count();
  uint32_t i;
  size_t k;
  for (k = 0u; k < rows; k++) {
    const ivs_digest_field *f = ivs_machine_digest_field(k);
    if (f != NULL && f->snap == IVS_SNAP_PAD) {
      memset(base + f->offset, 0, (size_t)f->width * f->count);
    }
  }
  for (i = 0u; i < IVS_SNAP_SECTIONS; i++) {
    const uint8_t *d = buf + v->section[i].offset;
    uint32_t pos = IVS_SNAP_FRAME_BYTES;
    for (k = 0u; k < rows; k++) {
      const ivs_digest_field *f = ivs_machine_digest_field(k);
      uint32_t e;
      if (f == NULL || ivs_snap_state_row(f, v->section[i].domain) == 0u) {
        continue;
      }
      if (f->count > 1u) {
        pos += 4u;
      }
      for (e = 0u; e < f->count; e++) {
        ivs_snap_store(base + f->offset + (size_t)e * f->width, ivs_snap_get(d + pos, f->width), f->width);
        pos += f->width;
      }
    }
  }
}

/* ---- the identity ---- */

uint32_t ivs_snapshot_profile_id_matches(const ivs_snap_view *v, const ivs_profile_desc *d)
{
  uint32_t i;
  if (v == NULL || d == NULL || v->profile_id_length >= IVS_PROFILE_ID_MAX_BYTES) {
    return 0u;
  }
  for (i = 0u; i < v->profile_id_length; i++) {
    if (v->profile_id[i] != (uint8_t)d->id[i]) {
      return 0u;
    }
  }
  return (uint32_t)(d->id[v->profile_id_length] == '\0');
}

/* Step 6, reasons 72 to 74 (named fault RESTORE_PROFILE_UNCHECKED skips all three). */
static ivs_status ivs_snap_check_profile(const struct ivs_machine *im, const ivs_snap_view *v, ivs_snap_refusal *r)
{
#ifndef IVS_FAULT_RESTORE_PROFILE_UNCHECKED
  uint8_t want[32];
  uint8_t diff = 0u;
  uint32_t index = 0u;
  uint32_t i;
  if (ivs_machine_profile_lookup((const char *)v->profile_id, &index) != IVS_OK) {
    return ivs_snap_refuse(r, IVS_E_UNKNOWN_PROFILE, IVS_REASON_SNAP_UNKNOWN_PROFILE, IVS_SNAP_OFF_PROFILE_ID, IVS_SEGMENT_NONE, 0u);
  }
  if (ivs_snapshot_profile_id_matches(v, ivs_profile_get(im->profile)) == 0u) {
    return ivs_snap_refuse(r, IVS_E_STATE_MISMATCH, IVS_REASON_SNAP_PROFILE_MISMATCH, IVS_SNAP_OFF_PROFILE_ID, IVS_SEGMENT_NONE, 0u);
  }
  if (ivs_snapshot_params_digest(im->profile, want) != IVS_OK) {
    return ivs_snap_refuse(r, IVS_E_STATE_MISMATCH, IVS_REASON_SNAP_PROFILE_PARAMS, IVS_SNAP_OFF_PARAMS, IVS_SEGMENT_NONE, 0u);
  }
  for (i = 0u; i < 32u; i++) {
    diff = (uint8_t)(diff | (v->params_sha256[i] ^ want[i]));
  }
  if (diff != 0u) {
    return ivs_snap_refuse(r, IVS_E_STATE_MISMATCH, IVS_REASON_SNAP_PROFILE_PARAMS, IVS_SNAP_OFF_PARAMS, IVS_SEGMENT_NONE, 0u);
  }
#else
  (void)im;
  (void)v;
  (void)r;
#endif
  return IVS_OK;
}

/* Step 7, reasons 75, 76 and 78 for a slot record: per slot in the order EXEC, GROM, cartridge. The snapshot's record must be a record a
 * writer makes (loaded 0 or 1, an empty slot with no length and no digest: the slot of an instance is zeroed when it is empty), and is
 * then compared with the live slot. */
static ivs_status ivs_snap_check_slots(const struct ivs_machine *im, const ivs_snap_view *v, ivs_snap_refusal *r)
{
  uint32_t i;
  for (i = 0u; i < IVS_SLOT_COUNT; i++) {
    const ivs_snap_slot_view *s = &v->slot[i];
    const ivs_machine_slot *l = &im->slot[i];
    uint64_t rec = (uint64_t)IVS_SNAP_OFF_SLOTS + (uint64_t)IVS_SNAP_SLOT_BYTES * i;
    uint32_t kind = i + 1u;
    uint32_t sha_zero = 0u;
    uint32_t sha_diff = 0u;
    uint32_t j;
    for (j = 0u; j < 32u; j++) {
      sha_zero |= (uint32_t)s->sha256[j];
      sha_diff |= (uint32_t)(s->sha256[j] ^ l->sha256[j]);
    }
    if (s->loaded > 1u) {
      return ivs_snap_refuse(r, IVS_E_MALFORMED, IVS_REASON_SNAP_FIELD_RANGE, rec, IVS_SEGMENT_NONE, kind);
    }
    if (s->loaded == 0u && s->length_bytes != 0u) {
      return ivs_snap_refuse(r, IVS_E_MALFORMED, IVS_REASON_SNAP_FIELD_RANGE, rec + 36u, IVS_SEGMENT_NONE, kind);
    }
    if (s->loaded == 0u && sha_zero != 0u) {
      return ivs_snap_refuse(r, IVS_E_MALFORMED, IVS_REASON_SNAP_FIELD_RANGE, rec + 4u, IVS_SEGMENT_NONE, kind);
    }
    if (s->loaded == 1u && l->loaded == 0u) {
      return ivs_snap_refuse(r, IVS_E_MISSING_SYSTEM_IMAGE, IVS_REASON_SNAP_SLOT_MISSING, rec, IVS_SEGMENT_NONE, kind);
    }
    if (s->loaded == 0u && l->loaded != 0u) {
      return ivs_snap_refuse(r, IVS_E_STATE_MISMATCH, IVS_REASON_SNAP_SLOT_MISMATCH, rec, IVS_SEGMENT_NONE, kind);
    }
    {
      uint32_t compare = 1u;
#ifdef IVS_FAULT_RESTORE_IMAGE_DIGEST_UNCHECKED
      /* Named fault RESTORE_IMAGE_DIGEST_UNCHECKED (plan 08-13): the identity of a slot loaded on both sides is not compared, so a snapshot
       * taken with other images is accepted into an instance that holds these. */
      compare = 0u;
#endif
      if (compare != 0u && s->loaded == 1u && sha_diff != 0u) {
        return ivs_snap_refuse(r, IVS_E_STATE_MISMATCH, IVS_REASON_SNAP_SLOT_MISMATCH, rec + 4u, IVS_SEGMENT_NONE, kind);
      }
      if (compare != 0u && s->loaded == 1u && s->length_bytes != l->length_bytes) {
        return ivs_snap_refuse(r, IVS_E_STATE_MISMATCH, IVS_REASON_SNAP_SLOT_MISMATCH, rec + 36u, IVS_SEGMENT_NONE, kind);
      }
    }
  }
  return IVS_OK;
}

/* ---- the derived invariants of docs/formats/ivsn-v1.md, each a function of the probe and the profile parameters ---- */

/* INV-1, the device validators and the event cursor; INV-5, the phase and the clock range. Restore judges these as range rules (78), one
 * function each so the refusal can name where the struct or scalar lies; ivs_snapshot_invariants judges the same functions. */
static uint32_t ivs_snap_inv1_cpu(const ivs_snap_probe *p)
{
  return (uint32_t)(ivs_cp1610_state_validate(&p->cpu) == IVS_OK);
}

static uint32_t ivs_snap_inv1_psg(const ivs_snap_probe *p, const ivs_psg_params *pp)
{
  return (uint32_t)(pp != NULL && ivs_psg_state_validate(pp, &p->psg) == IVS_OK);
}

static uint32_t ivs_snap_inv1_stic(const ivs_snap_probe *p, const ivs_stic_params *sp)
{
  return (uint32_t)(sp != NULL && ivs_stic_state_validate(&p->stic) == IVS_OK && p->stic.event_index < sp->event_count);
}

static uint32_t ivs_snap_inv5_phase(const ivs_snap_probe *p)
{
  return (uint32_t)(p->phase == IVS_PHASE_POWERED);
}

static uint32_t ivs_snap_inv5_time(const ivs_snap_probe *p)
{
  return (uint32_t)(p->time_ticks <= IVS_SNAP_TIME_MAX);
}

/* The tick of event ei of frame fi, phase + fi x F + the event's tick, into *out; 0 when the sum does not fit 64 bits. */
static uint32_t ivs_snap_event_at(const ivs_stic_params *sp, uint64_t fi, uint32_t ei, uint64_t *out)
{
  uint64_t add = (uint64_t)sp->frame_phase_ticks + (uint64_t)sp->event_tick[ei];
  uint64_t f = sp->frame_ticks;
  if (f != 0u && fi > (UINT64_MAX - add) / f) {
    return 0u;
  }
  *out = add + fi * f;
  return 1u;
}

/* INV-2, the STIC timeline: the cursor names the next event, whose tick is above machine time, and the event before it (the one before
 * the cursor in this frame, or the last of the frame before) is at or below it; the gate flags are what replaying the events below the
 * cursor from the open state gives (FRAME_START opens both gates, REG_CLOSE closes the registers, GMEM_CLOSE the graphics memory; gate 0
 * is always open and gate 3 always closed). Requires the cursor below the event count (INV-1). */
static uint32_t ivs_snap_inv2(const ivs_snap_probe *p, const ivs_stic_params *sp)
{
  uint64_t fi = p->stic.frame_index;
  uint32_t ei = p->stic.event_index;
  uint64_t at = 0u;
  uint32_t regs = 1u;
  uint32_t gmem = 1u;
  uint32_t e;
  if (ei >= sp->event_count) {
    return 0u;
  }
  /* The machine that has run no unit (time 0, the cursor at the very start) is the one state where the first event may be due: the
   * timeline is caught up at the end of every unit, never at the reset, so at the reset the event at tick 0 has not been applied yet. */
  if ((p->time_ticks != 0u || fi != 0u || ei != 0u) && ivs_snap_event_at(sp, fi, ei, &at) != 0u && at <= p->time_ticks) {
    return 0u; /* the next event is already due: the timeline was not caught up to machine time */
  }
  if (ei > 0u) {
    if (ivs_snap_event_at(sp, fi, ei - 1u, &at) == 0u || at > p->time_ticks) {
      return 0u;
    }
  } else if (fi > 0u) {
    if (ivs_snap_event_at(sp, fi - 1u, sp->event_count - 1u, &at) == 0u || at > p->time_ticks) {
      return 0u;
    }
  }
  for (e = 0u; e < ei; e++) {
    switch (sp->event_kind[e]) {
    case IVS_STIC_EV_FRAME_START:
      regs = 1u;
      gmem = 1u;
      break;
    case IVS_STIC_EV_REG_CLOSE:
      regs = 0u;
      break;
    case IVS_STIC_EV_GMEM_CLOSE:
      gmem = 0u;
      break;
    default:
      break;
    }
  }
  return (uint32_t)(p->gate_open[IVS_MEMSYS_GATE_NONE] == 1u && p->gate_open[IVS_MEMSYS_GATE_STIC_REGS] == regs &&
                    p->gate_open[IVS_MEMSYS_GATE_GMEM] == gmem && p->gate_open[IVS_MEMSYS_GATES] == 0u);
}

/* INV-3, the PSG: next_sample is the index of the chip step due after machine time (one step every sample period from step 1), the
 * emitted count of the audio row is the steps made, the previous levels are three four-bit levels in the low three bytes and a fourth
 * byte of 0 (the sample is A, B, C, 0, psg_sample.c; the specification says "the bytes are at most 15", a weaker form that this
 * stricter one implies: a correction by derivation, see the appended notes), and every stored PSG register
 * is a value its write mask lets through. The sample period of 0 (a profile that publishes no audio) leaves the step index free. */
static uint32_t ivs_snap_inv3(const ivs_snap_probe *p, const ivs_psg_params *pp)
{
  uint32_t i;
  if (pp->sample_period_ticks != 0u && p->psg.next_sample != p->time_ticks / pp->sample_period_ticks + 1u) {
    return 0u;
  }
  if (p->audio_emitted + 1u != p->psg.next_sample) {
    return 0u;
  }
  if ((p->audio_prev_levels & 0xFFF0F0F0u) != 0u) {
    return 0u;
  }
  for (i = 0u; i < IVS_PSG_REG_COUNT; i++) {
    if ((uint32_t)(p->psg_reg[i] & pp->reg_wmask[i]) != (uint32_t)p->psg_reg[i]) {
      return 0u;
    }
  }
  return 1u;
}

/* INV-4, the input: no control word has the reserved bit, a word of a controller the profile does not connect is 0, the two pull-down
 * masks are what the words derive, and the input floor never lies above machine time. */
static uint32_t ivs_snap_inv4(const ivs_snap_probe *p, uint32_t profile_index, uint32_t controllers)
{
  uint8_t derived[2];
  uint32_t i;
  for (i = 0u; i < 4u; i++) {
    if ((p->input.words[i] & IVS_CTRL_RESERVED_MASK) != 0u || (i >= controllers && p->input.words[i] != 0u)) {
      return 0u;
    }
  }
  ivs_input_derive(profile_index, p->input.words, derived);
  if (derived[0] != p->port_pulldown[0] || derived[1] != p->port_pulldown[1]) {
    return 0u;
  }
  return (uint32_t)(p->input.floor_tick <= p->time_ticks);
}

/* INV-6, the video row counts the frames the STIC completed (the machine path delivers every frame through the video row). */
static uint32_t ivs_snap_inv6(const ivs_snap_probe *p)
{
  return (uint32_t)(p->video_count == p->stic.frame_index);
}

uint32_t ivs_snapshot_invariants(const ivs_snap_probe *p, uint32_t profile_index)
{
  const ivs_profile_desc *pd = ivs_profile_get(profile_index);
  const ivs_psg_params *pp = ivs_psg_params_get(profile_index);
  const ivs_stic_params *sp = ivs_stic_params_get(profile_index);
  if (p == NULL || pd == NULL || pp == NULL || sp == NULL) {
    return 1u;
  }
  if (ivs_snap_inv1_cpu(p) == 0u || ivs_snap_inv1_psg(p, pp) == 0u || ivs_snap_inv1_stic(p, sp) == 0u) {
    return 1u;
  }
  if (ivs_snap_inv2(p, sp) == 0u) {
    return 2u;
  }
  if (ivs_snap_inv3(p, pp) == 0u) {
    return 3u;
  }
  if (ivs_snap_inv4(p, profile_index, pd->input_controllers) == 0u) {
    return 4u;
  }
  if (ivs_snap_inv5_phase(p) == 0u || ivs_snap_inv5_time(p) == 0u) {
    return 5u;
  }
  if (ivs_snap_inv6(p) == 0u) {
    return 6u;
  }
  return 0u;
}

void ivs_snapshot_probe_from_instance(const struct ivs_machine *m, ivs_snap_probe *p)
{
  memset(p, 0, sizeof *p);
  p->time_ticks = m->time_ticks;
  p->phase = m->phase;
  p->cpu = m->cpu;
  p->psg = m->psg;
  p->stic = m->stic;
  p->input = m->obs.input;
  memcpy(p->psg_reg, m->mem.psg_reg, sizeof p->psg_reg);
  memcpy(p->gate_open, m->mem.gate_open, sizeof p->gate_open);
  memcpy(p->port_pulldown, m->mem.port_pulldown, sizeof p->port_pulldown);
  p->video_count = m->obs.video.count;
  p->audio_emitted = m->obs.audio.emitted;
  p->audio_prev_levels = m->obs.audio.prev_levels;
}

/* Step 8, reasons 78 and 79 on the decoded probe. The range rules (INV-1 and INV-5) first, each naming where its struct or scalar was
 * read, then the derived rules (INV-2, INV-3, INV-4 and INV-6). im gives the registry profile; it is not const only because the named fault below commits into it. */
static ivs_status ivs_snap_check_probe(struct ivs_machine *im, const ivs_snap_probe *pr, const ivs_snap_loc loc[IVS_SNAP_REGIONS],
                                       ivs_snap_refusal *r)
{
  const ivs_profile_desc *pd = ivs_profile_get(im->profile);
  const ivs_psg_params *pp = ivs_psg_params_get(im->profile);
  const ivs_stic_params *sp = ivs_stic_params_get(im->profile);
  if (ivs_snap_inv1_cpu(pr) == 0u) {
    return ivs_snap_refuse(r, IVS_E_MALFORMED, IVS_REASON_SNAP_FIELD_RANGE, loc[IVS_SNAP_R_CPU].off, loc[IVS_SNAP_R_CPU].section, 0u);
  }
  if (ivs_snap_inv5_phase(pr) == 0u) {
    return ivs_snap_refuse(r, IVS_E_MALFORMED, IVS_REASON_SNAP_FIELD_RANGE, loc[IVS_SNAP_R_PHASE].off, loc[IVS_SNAP_R_PHASE].section, 0u);
  }
  if (ivs_snap_inv5_time(pr) == 0u) {
    return ivs_snap_refuse(r, IVS_E_MALFORMED, IVS_REASON_SNAP_FIELD_RANGE, loc[IVS_SNAP_R_TIME].off, loc[IVS_SNAP_R_TIME].section, 0u);
  }
  if (ivs_snap_inv1_psg(pr, pp) == 0u) {
    return ivs_snap_refuse(r, IVS_E_MALFORMED, IVS_REASON_SNAP_FIELD_RANGE, loc[IVS_SNAP_R_PSG].off, loc[IVS_SNAP_R_PSG].section, 0u);
  }
#ifdef IVS_FAULT_RESTORE_MUTATES_BEFORE_VALIDATE
  /* Named fault RESTORE_MUTATES_BEFORE_VALIDATE (plan 08-13): the CPU rows are committed before the STIC validators run, so a snapshot that
   * a STIC validator refuses has already changed the instance. */
  im->cpu = pr->cpu;
#endif
  if (ivs_snap_inv1_stic(pr, sp) == 0u) {
    return ivs_snap_refuse(r, IVS_E_MALFORMED, IVS_REASON_SNAP_FIELD_RANGE, loc[IVS_SNAP_R_STIC].off, loc[IVS_SNAP_R_STIC].section, 0u);
  }
  if (ivs_snap_inv2(pr, sp) == 0u) {
    return ivs_snap_refuse(r, IVS_E_MALFORMED, IVS_REASON_SNAP_INVARIANT, loc[IVS_SNAP_R_STIC].off, loc[IVS_SNAP_R_STIC].section, 0u);
  }
  if (ivs_snap_inv3(pr, pp) == 0u) {
    return ivs_snap_refuse(r, IVS_E_MALFORMED, IVS_REASON_SNAP_INVARIANT, loc[IVS_SNAP_R_PSG].off, loc[IVS_SNAP_R_PSG].section, 0u);
  }
  if (ivs_snap_inv4(pr, im->profile, pd->input_controllers) == 0u) {
    return ivs_snap_refuse(r, IVS_E_MALFORMED, IVS_REASON_SNAP_INVARIANT, loc[IVS_SNAP_R_INPUT].off, loc[IVS_SNAP_R_INPUT].section, 0u);
  }
  if (ivs_snap_inv6(pr) == 0u) {
    return ivs_snap_refuse(r, IVS_E_MALFORMED, IVS_REASON_SNAP_INVARIANT, loc[IVS_SNAP_R_VIDEO].off, loc[IVS_SNAP_R_VIDEO].section, 0u);
  }
  return IVS_OK;
}

/* A refusal into the report (nothing when rep is NULL): the view carries the reason, offset and section index. */
static void ivs_snap_write_refusal(ivs_snapshot_report *rep, ivs_snap_view *v, const ivs_snap_refusal *r)
{
  if (rep == NULL) {
    return;
  }
  v->status = r->st;
  v->reason = r->reason;
  v->byte_offset = r->off;
  v->section_index = r->section;
  ivs_snap_report_refusal(rep, v);
  if (r->kind != 0u) {
    rep->kind = r->kind;
  }
}

/* A refusal of the argument classes and of the handle: only the reason (the offset 0, no section), and only where the report may be
 * written (it exists and does not lie inside a valid instance, which a refusal must leave byte-identical). */
static void ivs_snap_write_arg_refusal(ivs_snapshot_report *rep, uint32_t may_write, uint32_t reason)
{
  ivs_snap_view v;
  if (rep == NULL || may_write == 0u) {
    return;
  }
  memset(&v, 0, sizeof v);
  v.reason = reason;
  v.section_index = IVS_SEGMENT_NONE;
  ivs_snap_report_refusal(rep, &v);
}

ivs_status ivs_machine_snapshot_restore(ivs_machine *m, const uint8_t *buf, uint64_t len, ivs_snapshot_report *rep)
{
  ivs_snap_view v;
  ivs_snap_refusal r;
  ivs_snap_probe pr;
  ivs_snap_loc loc[IVS_SNAP_REGIONS];
  struct ivs_machine *im;
  uint64_t span;
  uint32_t reason;
  uint32_t rep_in_instance;
  uint32_t k;
  ivs_status st;
  if (rep != NULL && (rep->struct_size < IVS_SNAPSHOT_REPORT_V1_SIZE || rep->struct_size > IVS_STRUCT_SIZE_MAX)) {
    return IVS_E_BAD_ARG; /* an unusable report: nothing is written */
  }
  im = ivs_machine_from_handle_mut(m);
  /* Only a valid instance has bytes a report can overlap (a NULL or invalid handle is BAD_HANDLE wherever the report lies). */
  rep_in_instance = (uint32_t)(rep != NULL && im != NULL && ivs_machine_span_overlaps(rep, IVS_SNAPSHOT_REPORT_V1_SIZE, im, sizeof *im) != 0u);

  /* Step 1: the argument classes of the reader, then the handle, then the overlap with the instance. */
  reason = ivs_snapshot_arg_reason(buf, len, rep, rep != NULL ? IVS_SNAPSHOT_REPORT_V1_SIZE : 0u);
  if (reason != 0u) {
    ivs_snap_write_arg_refusal(rep, (uint32_t)(rep_in_instance == 0u), reason);
    return IVS_E_BAD_ARG;
  }
  if (im == NULL) {
    ivs_snap_write_arg_refusal(rep, 1u, IVS_REASON_HANDLE);
    return IVS_E_BAD_HANDLE;
  }
  span = len > IVS_SNAPSHOT_MAX_BYTES ? (uint64_t)IVS_SNAPSHOT_MAX_BYTES : len;
  if (ivs_machine_span_overlaps(buf, (size_t)span, im, sizeof *im) != 0u || rep_in_instance != 0u) {
    ivs_snap_write_arg_refusal(rep, (uint32_t)(rep_in_instance == 0u), IVS_REASON_SNAP_ARG_OVERLAP_INSTANCE);
    return IVS_E_BAD_ARG;
  }
  /* From here on the report, when there is one, lies outside the instance and outside the buffer. */

  /* Steps 2 to 6a: the container, exactly as describe judges it. */
  st = ivs_snapshot_read(buf, len, &v);
  if (st != IVS_OK) {
    if (rep != NULL) {
      ivs_snap_report_refusal(rep, &v);
    }
    return st;
  }

  /* Steps 6 and 7: the identity. */
  memset(&r, 0, sizeof r);
  if (ivs_snap_check_profile(im, &v, &r) != IVS_OK || ivs_snap_check_slots(im, &v, &r) != IVS_OK) {
    ivs_snap_write_refusal(rep, &v, &r);
    return r.st;
  }

  /* Step 8: the content, into the probe; nothing is written to the instance. */
  memset(&pr, 0, sizeof pr);
  for (k = 0u; k < IVS_SNAP_REGIONS; k++) {
    loc[k].off = IVS_SNAP_NO_OFFSET;
    loc[k].section = IVS_SEGMENT_NONE;
  }
  if (ivs_snap_check_lengths(&v, &r) != IVS_OK || ivs_snap_check_counts(buf, &v, &r) != IVS_OK ||
      ivs_snap_check_frames(buf, &v, &r) != IVS_OK) {
    ivs_snap_write_refusal(rep, &v, &r);
    return r.st;
  }
  for (k = 0u; k < IVS_SNAP_SECTIONS; k++) {
    ivs_snap_decode_probe(buf, &v, k, &pr, loc);
  }
  if (ivs_snap_check_probe(im, &pr, loc, &r) != IVS_OK) {
    ivs_snap_write_refusal(rep, &v, &r);
    return r.st;
  }

  /* Step 9: the commit. */
  ivs_snap_commit(im, buf, &v);
  if (rep != NULL) {
    ivs_snap_report_success(rep, &v);
  }
  return IVS_OK;
}

/* SPDX-License-Identifier: MIT */
/* Suite state, family restore (plan 08-13, D-14, D-15, D-16, D-41): ivs_machine_snapshot_restore (ivs/ivs_state.h) and the rules it
 * applies before it writes the first byte of an instance. The refusal matrices are written from the specification
 * (docs/formats/ivsn-v1.md: Layout, Check order, Refusal reasons, Derived invariants) and from the contract text of the header, never from
 * the restore code: every row names the mutation of a valid snapshot, the reason it must give (the status follows from the reason by
 * the table of the specification) and the byte offset, section index and slot kind it must report, and the position of every member it
 * changes is found from the digest field table read as data (su_row_pos), not from the restore code. A snapshot is built once by the
 * writer (held to the literals by the other families of the suite: it is the stepped boot snapshot of the pinned literal) and each row
 * changes bytes of a copy of it and, where the row is about the content, recomputes the trailer: a resealed snapshot reaches the
 * validators and the invariants, because the trailer is a checksum, never the defence.
 *
 * What every row checks (su_run_rows, state_util.c): the status; the whole report, 400 bytes of a caller struct larger than the v1 size,
 * filled with two different poison bytes in turn, against the report the contract allows (a refusal writes only reason, byte_offset,
 * section_index, the header facts the length let the reader see and, for an image slot, kind); the supplied bytes unchanged; and the whole
 * target instance byte-identical to what it was before the call. The input is a heap copy of exactly the supplied length (the dev preset's
 * sanitizer finds any read past it); a fault build adds a zeroed tail so that a reader that trusts a claimed length reads defined bytes. */
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "common/ivs_sha256.h"
#include "ivs/ivs.h"
#include "machine/machine.h"
#include "state_cases.h"
#include "state_util.h"

#define RS_NONE 0xFFFFFFFFu
#define RS_ALL_REASONS 0x00FFFFFFu /* the bits of the reasons 58 to 81 (bit reason - 58) */
#define RS_BIT(reason) (1u << ((reason)-58u))

/* ---- the base snapshot: the stepped stand-in boot program, the snapshot that stepped-snapshot-sha256 pins ---- */

static uint8_t rs_snap[IVS_SNAPSHOT_MAX_BYTES];
static size_t rs_snap_n;

static int rs_base_get(void)
{
  ivs_ms_inst x;
  uint32_t total = 0u;
  size_t n = 0u;
  int ok;
  if (rs_snap_n != 0u) {
    return 1;
  }
  if (!ivsn_lit_u32("total-bytes", &total) || !su_boot_stepped(&x, SU_BOOT_UNITS, 0x5Au)) {
    return 0;
  }
  ok = su_snapshot(x.m, rs_snap, sizeof rs_snap, &n) && n == total;
  ivs_ms_free(&x);
  if (!ok) {
    ivs_test_diag("the base snapshot could not be built");
    return 0;
  }
  rs_snap_n = n;
  return 1;
}

/* The position of a STATE row, or a mark on the table that it does not exist. */
typedef struct rs_where {
  uint32_t sec;
  uint32_t off;  /* the first byte of the first element */
  uint32_t cnt;  /* the count prefix of an array row, else 0 */
} rs_where;

static int rs_where_of(su_tab *t, const char *row, rs_where *w)
{
  if (!su_row_pos(row, &w->sec, &w->off, &w->cnt)) {
    ivs_test_diag("no STATE row '%s' in the field table", row);
    t->bad = 1;
    memset(w, 0, sizeof *w);
    return 0;
  }
  return 1;
}

/* The big-endian value of an element of a row in the base snapshot. */
static uint64_t rs_base_val(su_tab *t, const char *row, uint32_t elem, uint32_t width)
{
  rs_where w;
  if (!rs_where_of(t, row, &w)) {
    return 0u;
  }
  return su_get(rs_snap + w.off + elem * width, width);
}

/* One member of a row: row name, element index, width and the value to put there. */
typedef struct rs_mem {
  const char *row;
  uint32_t elem;
  uint32_t width;
  uint64_t v;
} rs_mem;

static void rs_put_mem(su_tab *t, su_row *r, const rs_mem *m)
{
  rs_where w;
  if (m->row != NULL && rs_where_of(t, m->row, &w)) {
    su_setn(t, r, w.off + m->elem * m->width, m->width, m->v);
  }
}

/* ---- family: the container structure (the reader's reasons 60 to 71, through restore) ---- */

static void rs_build_structure(su_tab *t)
{
  su_row *r;
  size_t n = rs_snap_n;
  uint32_t entry0 = 304u;
  /* 60 to 71: one representative row each; the offsets are the ones of the specification and of the header of ivs_state.h */
  r = su_add(t, "60 length claimed above the bound, one real byte");
  r->big_len = 16385u;
  su_expect(r, 60u, 16384u, RS_NONE, 0u);
  r = su_add(t, "61 one byte short");
  r->trunc = (uint32_t)(n - 1u);
  su_expect(r, 61u, n - 1u, RS_NONE, 0u);
  r = su_add(t, "61 shorter than a header");
  r->trunc = 400u;
  su_expect(r, 61u, 400u, RS_NONE, 0u);
  r = su_add(t, "62 magic, first byte");
  su_xor(t, r, 0u, 0x01u);
  su_expect(r, 62u, 0u, RS_NONE, 0u);
  r = su_add(t, "62 magic, last byte");
  su_xor(t, r, 7u, 0x80u);
  su_expect(r, 62u, 7u, RS_NONE, 0u);
  r = su_add(t, "63 format version 2");
  su_set(t, r, 9u, 2u);
  su_expect(r, 63u, 8u, RS_NONE, 0u);
  r = su_add(t, "64 flags 1");
  su_set(t, r, 11u, 1u);
  su_expect(r, 64u, 10u, RS_NONE, 0u);
  r = su_add(t, "65 schema version 2");
  su_set(t, r, 19u, 2u);
  su_expect(r, 65u, 16u, RS_NONE, 0u);
  r = su_add(t, "66 schema digest, first byte");
  su_xor(t, r, 24u, 0x01u);
  su_expect(r, 66u, 24u, RS_NONE, 0u);
  r = su_add(t, "67 total below the length");
  su_setn(t, r, 12u, 4u, n - 1u);
  su_expect(r, 67u, 12u, RS_NONE, 0u);
  r = su_add(t, "68 section count 6");
  su_set(t, r, 23u, 6u);
  su_expect(r, 68u, 20u, RS_NONE, 0u);
  r = su_add(t, "69 section table, the domain of entry 0");
  su_set(t, r, entry0, 3u);
  su_expect(r, 69u, entry0, 0u, 0u);
  r = su_add(t, "70 trailer, last byte");
  su_xor(t, r, (uint32_t)(n - 1u), 0x01u);
  su_expect(r, 70u, n - 32u, RS_NONE, 0u);
  r = su_add(t, "71 profile id with a control byte, resealed");
  su_set(t, r, 56u + 5u, 0x01u);
  r->reseal = 1u;
  su_expect(r, 71u, 56u + 5u, RS_NONE, 0u);
  /* the order of the classes: structure before the trailer before the identity before the content */
  r = su_add(t, "70 an unregistered id with a stale trailer is the trailer");
  su_set(t, r, 56u, (uint8_t)'z');
  su_expect(r, 70u, n - 32u, RS_NONE, 0u);
  r = su_add(t, "62 a bad magic with a bad id and a stale trailer is the magic");
  su_xor(t, r, 2u, 0x40u);
  su_set(t, r, 56u, (uint8_t)'z');
  su_expect(r, 62u, 2u, RS_NONE, 0u);
  r = su_add(t, "71 a control byte in an unregistered id is the form, not the registry");
  su_set(t, r, 56u, 0x02u);
  r->reseal = 1u;
  su_expect(r, 71u, 56u, RS_NONE, 0u);
}

/* ---- family: the identity of the profile (72, 74; 73 is the seam test) ---- */

/* Sets the 64 byte id field to s (NUL padded): every byte of it that differs from the base id, then a NUL. */
static void rs_set_id(su_tab *t, su_row *r, const char *s)
{
  size_t have = strlen(IVS_MS_PROFILE);
  size_t want = strlen(s);
  size_t last = (have > want ? have : want) + 1u;
  size_t i;
  for (i = 0u; i < last && i < IVS_PROFILE_ID_MAX_BYTES; i++) {
    su_set(t, r, 56u + (uint32_t)i, i < want ? (uint8_t)s[i] : 0u);
  }
}

static void rs_build_profile(su_tab *t)
{
  static const char *const bad_ids[] = {"x", "", "intv-2609-ntsc-family-p", "intv-2609-ntsc-family-p2", "INTV-2609-ntsc-family-p1",
                                        "intv-2609-ntsc-family-p1x", "intv-2609-ntsc-family-p1-2"};
  size_t i;
  su_row *r;
  char longer[80];
  for (i = 0u; i < sizeof bad_ids / sizeof bad_ids[0]; i++) {
    r = su_add(t, "72 id '%s'", bad_ids[i]);
    rs_set_id(t, r, bad_ids[i]);
    r->reseal = 1u;
    su_expect(r, 72u, 56u, RS_NONE, 0u);
  }
  (void)snprintf(longer, sizeof longer, "%s%s", IVS_MS_PROFILE, "-xxxxxxxxxxxxxxxx");
  r = su_add(t, "72 a longer id");
  rs_set_id(t, r, longer);
  r->reseal = 1u;
  su_expect(r, 72u, 56u, RS_NONE, 0u);
  /* the parameter digest: any byte of its 32 */
  {
    static const uint32_t at[] = {0u, 1u, 15u, 30u, 31u};
    for (i = 0u; i < sizeof at / sizeof at[0]; i++) {
      r = su_add(t, "74 parameter digest byte %u", (unsigned)at[i]);
      su_xor(t, r, 120u + at[i], 0x01u);
      r->reseal = 1u;
      su_expect(r, 74u, 120u, RS_NONE, 0u);
    }
    r = su_add(t, "74 parameter digest, every bit of byte 7");
    su_xor(t, r, 120u + 7u, 0xFFu);
    r->reseal = 1u;
    su_expect(r, 74u, 120u, RS_NONE, 0u);
  }
  /* the order inside the class and against its neighbours */
  r = su_add(t, "72 an unregistered id before a flipped parameter digest");
  rs_set_id(t, r, "x");
  su_xor(t, r, 120u, 0x01u);
  r->reseal = 1u;
  su_expect(r, 72u, 56u, RS_NONE, 0u);
  r = su_add(t, "74 a flipped parameter digest before a slot mismatch");
  su_xor(t, r, 120u, 0x01u);
  su_xor(t, r, 152u + 4u, 0x01u);
  r->reseal = 1u;
  su_expect(r, 74u, 120u, RS_NONE, 0u);
  r = su_add(t, "74 a flipped parameter digest before a bad phase");
  {
    rs_where w;
    su_xor(t, r, 120u, 0x01u);
    if (rs_where_of(t, "phase", &w)) {
      su_setn(t, r, w.off, 4u, 0u);
    }
  }
  r->reseal = 1u;
  su_expect(r, 74u, 120u, RS_NONE, 0u);
  r = su_add(t, "72 an unregistered id with a stale trailer is the trailer, not the registry");
  rs_set_id(t, r, "x");
  su_expect(r, 70u, rs_snap_n - 32u, RS_NONE, 0u);
}

/* ---- family: the identity of the three image slots (75, 76 and the slot records' range, 78) ---- */

static void rs_build_slots(su_tab *t)
{
  static const char *const slot_name[3] = {"EXEC", "GROM", "cartridge"};
  static const uint32_t target_without[3] = {SU_T_NOEXEC, SU_T_NOGROM, SU_T_NOCART};
  uint32_t s;
  su_row *r;
  for (s = 0u; s < 3u; s++) {
    uint32_t rec = 152u + 40u * s;
    uint32_t kind = s + 1u;
    uint32_t j;
    const char *nm = slot_name[s];
    r = su_add(t, "78 %s loaded flag 2", nm);
    su_setn(t, r, rec, 4u, 2u);
    r->reseal = 1u;
    su_expect(r, 78u, rec, RS_NONE, kind);
    r = su_add(t, "78 %s loaded flag 0xFFFFFFFF", nm);
    su_setn(t, r, rec, 4u, 0xFFFFFFFFu);
    r->reseal = 1u;
    su_expect(r, 78u, rec, RS_NONE, kind);
    r = su_add(t, "78 %s empty record that holds a length", nm);
    su_setn(t, r, rec, 4u, 0u);
    r->reseal = 1u;
    su_expect(r, 78u, rec + 36u, RS_NONE, kind);
    r = su_add(t, "78 %s empty record that holds a digest", nm);
    su_setn(t, r, rec, 4u, 0u);
    su_setn(t, r, rec + 36u, 4u, 0u);
    r->reseal = 1u;
    su_expect(r, 78u, rec + 4u, RS_NONE, kind);
    r = su_add(t, "76 %s empty in the snapshot, loaded in the instance", nm);
    su_setn(t, r, rec, 4u, 0u);
    su_setn(t, r, rec + 36u, 4u, 0u);
    for (j = 0u; j < 32u; j++) {
      su_set(t, r, rec + 4u + j, 0u);
    }
    r->reseal = 1u;
    su_expect(r, 76u, rec, RS_NONE, kind);
    r = su_add(t, "75 %s loaded in the snapshot, missing in the instance", nm);
    r->tsel = (uint8_t)target_without[s];
    su_expect(r, 75u, rec, RS_NONE, kind); /* the snapshot is the base, unchanged: the target is what differs */
    r = su_add(t, "76 %s digest, first byte", nm);
    su_xor(t, r, rec + 4u, 0x01u);
    r->reseal = 1u;
    su_expect(r, 76u, rec + 4u, RS_NONE, kind);
    r = su_add(t, "76 %s digest, last byte", nm);
    su_xor(t, r, rec + 4u + 31u, 0x80u);
    r->reseal = 1u;
    su_expect(r, 76u, rec + 4u, RS_NONE, kind);
    r = su_add(t, "76 %s length one more", nm);
    su_xor(t, r, rec + 39u, 0x01u);
    r->reseal = 1u;
    su_expect(r, 76u, rec + 36u, RS_NONE, kind);
    r = su_add(t, "76 %s length, the high byte", nm);
    su_xor(t, r, rec + 36u, 0x01u);
    r->reseal = 1u;
    su_expect(r, 76u, rec + 36u, RS_NONE, kind);
    r = su_add(t, "76 %s digest and length both differ: the digest is reported", nm);
    su_xor(t, r, rec + 4u + 9u, 0x10u);
    su_xor(t, r, rec + 39u, 0x02u);
    r->reseal = 1u;
    su_expect(r, 76u, rec + 4u, RS_NONE, kind);
  }
  /* the order: slot after slot, and the images before the content */
  r = su_add(t, "76 slot 0 mismatch before a range error of slot 2");
  su_xor(t, r, 152u + 4u, 0x01u);
  su_setn(t, r, 152u + 80u, 4u, 2u);
  r->reseal = 1u;
  su_expect(r, 76u, 152u + 4u, RS_NONE, 1u);
  r = su_add(t, "78 slot 1 range error before a mismatch of slot 2");
  su_setn(t, r, 152u + 40u, 4u, 2u);
  su_xor(t, r, 152u + 80u + 4u, 0x01u);
  r->reseal = 1u;
  su_expect(r, 78u, 152u + 40u, RS_NONE, 2u);
  r = su_add(t, "76 a mismatched EXEC digest before a bad phase");
  {
    rs_where w;
    su_xor(t, r, 152u + 4u, 0x01u);
    if (rs_where_of(t, "phase", &w)) {
      su_setn(t, r, w.off, 4u, 0u);
    }
  }
  r->reseal = 1u;
  su_expect(r, 76u, 152u + 4u, RS_NONE, 1u);
  r = su_add(t, "75 a missing cartridge before a section length error");
  r->tsel = SU_T_NOCART;
  r->reseal = 1u;
  su_setn(t, r, 304u + 8u, 4u, 36u);
  su_setn(t, r, 304u + 44u + 4u, 4u, 648u);
  su_setn(t, r, 304u + 44u + 8u, 4u, 2760u);
  su_expect(r, 75u, 152u + 80u, RS_NONE, 3u);
}

/* ---- family: the content, the range rules (77, 78, 80) ---- */

/* Where each device struct and scalar of the probe starts in the container: the offset restore reports for a refusal about it. */
typedef struct rs_region {
  rs_where cpu;
  rs_where phase;
  rs_where time;
  rs_where psg;
  rs_where stic;
  rs_where input;
  rs_where video;
} rs_region;

static void rs_regions(su_tab *t, rs_region *g)
{
  (void)rs_where_of(t, "cpu.r", &g->cpu);
  (void)rs_where_of(t, "phase", &g->phase);
  (void)rs_where_of(t, "time_ticks", &g->time);
  (void)rs_where_of(t, "psg.next_sample", &g->psg);
  (void)rs_where_of(t, "stic.frame_index", &g->stic);
  (void)rs_where_of(t, "obs.input.words", &g->input);
  (void)rs_where_of(t, "obs.video.count", &g->video);
}

/* A row that changes up to three members and must be refused as reason 78 or 79 at where. */
static void rs_member_row(su_tab *t, const char *label, uint32_t reason, const rs_where *at, const rs_mem *a, const rs_mem *b, const rs_mem *c)
{
  su_row *r = su_add(t, "%u %s", (unsigned)reason, label);
  if (a != NULL) {
    rs_put_mem(t, r, a);
  }
  if (b != NULL) {
    rs_put_mem(t, r, b);
  }
  if (c != NULL) {
    rs_put_mem(t, r, c);
  }
  r->reseal = 1u;
  su_expect(r, reason, at->off, at->sec, 0u);
}

static void rs_build_validators(su_tab *t)
{
  rs_region g;
  rs_where w;
  su_row *r;
  uint32_t i;
  rs_regions(t, &g);
  /* the CPU validator: each rule, each member (cpu.c ivs_cp1610_state_validate and its cross-field rules) */
  {
    static const struct {
      const char *label;
      rs_mem a, b, c;
    } cpu_rows[] = {
        {"cpu.status 0x01, a bit outside the mask", {"cpu.status", 0u, 1u, 0x01u}, {NULL, 0u, 0u, 0u}, {NULL, 0u, 0u, 0u}},
        {"cpu.status 0x08", {"cpu.status", 0u, 1u, 0x08u}, {NULL, 0u, 0u, 0u}, {NULL, 0u, 0u, 0u}},
        {"cpu.run_state 5", {"cpu.run_state", 0u, 1u, 5u}, {NULL, 0u, 0u, 0u}, {NULL, 0u, 0u, 0u}},
        {"cpu.run_state 255", {"cpu.run_state", 0u, 1u, 255u}, {NULL, 0u, 0u, 0u}, {NULL, 0u, 0u, 0u}},
        {"cpu.fault_reason 5", {"cpu.fault_reason", 0u, 1u, 5u}, {NULL, 0u, 0u, 0u}, {NULL, 0u, 0u, 0u}},
        {"cpu.intff 2", {"cpu.intff", 0u, 1u, 2u}, {NULL, 0u, 0u, 0u}, {NULL, 0u, 0u, 0u}},
        {"cpu.dbd 2", {"cpu.dbd", 0u, 1u, 2u}, {NULL, 0u, 0u, 0u}, {NULL, 0u, 0u, 0u}},
        {"cpu.prev_interruptible 2", {"cpu.prev_interruptible", 0u, 1u, 2u}, {NULL, 0u, 0u, 0u}, {NULL, 0u, 0u, 0u}},
        {"cpu.halt_pending 2", {"cpu.halt_pending", 0u, 1u, 2u}, {NULL, 0u, 0u, 0u}, {NULL, 0u, 0u, 0u}},
        {"cpu.busak 2", {"cpu.busak", 0u, 1u, 2u}, {NULL, 0u, 0u, 0u}, {NULL, 0u, 0u, 0u}},
        {"FAULTED without a fault reason", {"cpu.run_state", 0u, 1u, 4u}, {NULL, 0u, 0u, 0u}, {NULL, 0u, 0u, 0u}},
        {"a fault reason while RUNNING", {"cpu.fault_reason", 0u, 1u, 1u}, {NULL, 0u, 0u, 0u}, {NULL, 0u, 0u, 0u}},
        {"RESET_PENDING with INTFF set", {"cpu.run_state", 0u, 1u, 1u}, {"cpu.intff", 0u, 1u, 1u}, {NULL, 0u, 0u, 0u}},
        {"UNRESET with a bus grant", {"cpu.run_state", 0u, 1u, 0u}, {"cpu.busak", 0u, 1u, 1u}, {NULL, 0u, 0u, 0u}},
        {"a bus grant while FAULTED", {"cpu.run_state", 0u, 1u, 4u}, {"cpu.fault_reason", 0u, 1u, 1u}, {"cpu.busak", 0u, 1u, 1u}},
        {"DBD with prev_interruptible set", {"cpu.dbd", 0u, 1u, 1u}, {"cpu.prev_interruptible", 0u, 1u, 1u}, {NULL, 0u, 0u, 0u}},
        {"DBD while HALTED", {"cpu.run_state", 0u, 1u, 3u}, {"cpu.dbd", 0u, 1u, 1u}, {NULL, 0u, 0u, 0u}},
        {"a stop latch while HALTED", {"cpu.run_state", 0u, 1u, 3u}, {"cpu.halt_pending", 0u, 1u, 1u}, {NULL, 0u, 0u, 0u}},
    };
    for (i = 0u; i < sizeof cpu_rows / sizeof cpu_rows[0]; i++) {
      rs_member_row(t, cpu_rows[i].label, 78u, &g.cpu, &cpu_rows[i].a, cpu_rows[i].b.row != NULL ? &cpu_rows[i].b : NULL,
                    cpu_rows[i].c.row != NULL ? &cpu_rows[i].c : NULL);
    }
  }
  /* the scalar rules: the phase and the clock range (INV-5). The clock bound is what a run call can produce, IVS_RUN_TARGET_MAX plus the
   * length of one unit (U = psg.max_unit_ticks: a run to 2^63 ends at the first unit boundary at or after it), so the first refused value is
   * one above that bound, derived from the profile here rather than guessed (docs/formats/ivsn-v1.md, the appended correction note). */
  {
    const ivs_psg_params *pp = ivs_psg_params_get(st_profile_index());
    rs_mem phase0 = {"phase", 0u, 4u, 0u};
    rs_mem phase2 = {"phase", 0u, 4u, 2u};
    rs_mem time_hi = {"time_ticks", 0u, 8u, 0u};
    rs_mem time_max = {"time_ticks", 0u, 8u, 0xFFFFFFFFFFFFFFFFull};
    if (pp == NULL) {
      ivs_test_diag("no PSG parameters for the registered profile");
      t->bad = 1;
      return;
    }
    time_hi.v = 0x8000000000000000ull + (uint64_t)pp->max_unit_ticks + 1u;
    rs_member_row(t, "phase 0 (OFF): a snapshot is never of a machine that is OFF", 78u, &g.phase, &phase0, NULL, NULL);
    rs_member_row(t, "phase 2", 78u, &g.phase, &phase2, NULL, NULL);
    rs_member_row(t, "time_ticks one above the bound 2^63 + U + 1", 78u, &g.time, &time_hi, NULL, NULL);
    rs_member_row(t, "time_ticks 2^64 - 1", 78u, &g.time, &time_max, NULL, NULL);
  }
  /* the PSG validator (psg_state.c, psg.h): each bound at its first refused value */
  {
    static const struct {
      const char *label;
      rs_mem a, b;
    } psg_rows[] = {
        {"psg.next_sample 0", {"psg.next_sample", 0u, 8u, 0u}, {NULL, 0u, 0u, 0u}},
        {"psg.tone_counter[0] 4096 (the reload bound)", {"psg.tone_counter", 0u, 4u, 4096u}, {NULL, 0u, 0u, 0u}},
        {"psg.tone_counter[2] 0xFFFFFFFF", {"psg.tone_counter", 2u, 4u, 0xFFFFFFFFu}, {NULL, 0u, 0u, 0u}},
        {"psg.tone_out[1] 2", {"psg.tone_out", 1u, 4u, 2u}, {NULL, 0u, 0u, 0u}},
        {"psg.noise_counter 64 (steps per period unit times the larger period)", {"psg.noise_counter", 0u, 4u, 64u}, {NULL, 0u, 0u, 0u}},
        {"psg.noise_lfsr 2^17 (wider than the Galois mask)", {"psg.noise_lfsr", 0u, 4u, 131072u}, {NULL, 0u, 0u, 0u}},
        {"psg.noise_lfsr 0 (the lock-up value)", {"psg.noise_lfsr", 0u, 4u, 0u}, {NULL, 0u, 0u, 0u}},
        {"psg.env_counter 131072", {"psg.env_counter", 0u, 4u, 131072u}, {NULL, 0u, 0u, 0u}},
        {"psg.env_level 16", {"psg.env_level", 0u, 4u, 16u}, {NULL, 0u, 0u, 0u}},
        {"psg.env_cycles 16", {"psg.env_cycles", 0u, 4u, 16u}, {NULL, 0u, 0u, 0u}},
        {"psg.env_dir 2", {"psg.env_dir", 0u, 4u, 2u}, {NULL, 0u, 0u, 0u}},
        {"psg.env_held 2", {"psg.env_held", 0u, 4u, 2u}, {NULL, 0u, 0u, 0u}},
        {"a running envelope whose level is not its cycle", {"psg.env_held", 0u, 4u, 0u}, {"psg.env_level", 0u, 4u, 7u}},
        {"a held envelope with a running counter", {"psg.env_counter", 0u, 4u, 5u}, {NULL, 0u, 0u, 0u}},
    };
    for (i = 0u; i < sizeof psg_rows / sizeof psg_rows[0]; i++) {
      rs_member_row(t, psg_rows[i].label, 78u, &g.psg, &psg_rows[i].a, psg_rows[i].b.row != NULL ? &psg_rows[i].b : NULL, NULL);
    }
  }
  /* the STIC validator (stic_state.c): each member at its first refused value */
  {
    static const rs_mem rows[] = {
        {"stic.event_index", 0u, 4u, 16u},  {"stic.event_index", 0u, 4u, 0xFFFFFFFFu}, {"stic.display_latch", 0u, 4u, 2u},
        {"stic.windows_active", 0u, 4u, 2u}, {"stic.mode", 0u, 4u, 2u},               {"stic.cs_ptr", 0u, 4u, 4u},
    };
    for (i = 0u; i < sizeof rows / sizeof rows[0]; i++) {
      char label[64];
      (void)snprintf(label, sizeof label, "%s %llu", rows[i].row, (unsigned long long)rows[i].v);
      rs_member_row(t, label, 78u, &g.stic, &rows[i], NULL, NULL);
    }
  }
  /* the frame of a section: IVSD, the domain and the domain version 1 */
  for (i = 0u; i < 7u; i++) {
    r = su_add(t, "78 frame byte %u of the CPU section", (unsigned)i);
    su_xor(t, r, su_section_offset(0u) + i, 0x04u);
    r->reseal = 1u;
    su_expect(r, 78u, su_section_offset(0u) + i, 0u, 0u);
  }
  for (i = 1u; i < 7u; i++) {
    r = su_add(t, "78 frame byte %u of section %u", (unsigned)i, (unsigned)i);
    su_xor(t, r, su_section_offset(i) + i, 0x10u);
    r->reseal = 1u;
    su_expect(r, 78u, su_section_offset(i) + i, i, 0u);
  }
  /* 77: the count prefix of the first array row of every section */
  {
    static const char *const first[7] = {"cpu.r", "mem.gate_open", "psg.tone_counter", "obs.video.chain", "obs.audio.chain",
                                         "obs.input.words", "obs.psglog.chain"};
    for (i = 0u; i < 7u; i++) {
      uint32_t table_count = 0u;
      size_t k;
      for (k = 0u; k < ivs_machine_digest_field_count(); k++) {
        const ivs_digest_field *f = ivs_machine_digest_field(k);
        if (f != NULL && strcmp(f->name, first[i]) == 0) {
          table_count = f->count;
        }
      }
      if (!rs_where_of(t, first[i], &w)) {
        continue;
      }
      r = su_add(t, "77 %s count %u, the table says %u", first[i], (unsigned)table_count + 1u, (unsigned)table_count);
      su_setn(t, r, w.cnt, 4u, (uint64_t)table_count + 1u);
      r->reseal = 1u;
      su_expect(r, 77u, w.cnt, w.sec, 0u);
      r = su_add(t, "77 %s count 0", first[i]);
      su_setn(t, r, w.cnt, 4u, 0u);
      r->reseal = 1u;
      su_expect(r, 77u, w.cnt, w.sec, 0u);
    }
    if (rs_where_of(t, "cpu.r", &w)) {
      r = su_add(t, "77 cpu.r count 0xFFFFFFFF");
      su_setn(t, r, w.cnt, 4u, 0xFFFFFFFFu);
      r->reseal = 1u;
      su_expect(r, 77u, w.cnt, w.sec, 0u);
      r = su_add(t, "77 cpu.r count 1");
      su_setn(t, r, w.cnt, 4u, 1u);
      r->reseal = 1u;
      su_expect(r, 77u, w.cnt, w.sec, 0u);
    }
    if (rs_where_of(t, "mem.sysram", &w)) {
      rs_where w2;
      r = su_add(t, "77 two counts wrong: the first in section order is reported");
      su_setn(t, r, w.cnt, 4u, 513u);
      if (rs_where_of(t, "obs.audio.chain", &w2)) {
        su_setn(t, r, w2.cnt, 4u, 31u);
      }
      r->reseal = 1u;
      su_expect(r, 77u, w.cnt, w.sec, 0u);
    }
  }
  /* 80: lengths that chain and keep the sum, so the reader accepts the table and restore finds the length that is not the derived one */
  {
    uint32_t len[7];
    uint32_t off[7];
    char key[48];
    uint32_t e;
    for (e = 0u; e < 7u; e++) {
      uint32_t o = 0u;
      uint32_t b = 0u;
      (void)snprintf(key, sizeof key, "section-offset.%s", st_section_name[e]);
      (void)ivsn_lit_u32(key, &o);
      (void)snprintf(key, sizeof key, "section-bytes.%s", st_section_name[e]);
      (void)ivsn_lit_u32(key, &b);
      off[e] = o;
      len[e] = b;
    }
    for (e = 0u; e < 6u; e++) {
      int dir;
      for (dir = 0; dir < 2; dir++) {
        if (dir == 1 && e != 0u && e != 3u && e != 5u) {
          continue;
        }
        r = su_add(t, "80 section %u %s, section %u %s", (unsigned)e, dir == 0 ? "one byte long" : "one byte short", (unsigned)e + 1u,
                   dir == 0 ? "one byte short" : "one byte long");
        su_setn(t, r, 304u + 44u * e + 8u, 4u, dir == 0 ? len[e] + 1u : len[e] - 1u);
        su_setn(t, r, 304u + 44u * (e + 1u) + 4u, 4u, dir == 0 ? off[e + 1u] + 1u : off[e + 1u] - 1u);
        su_setn(t, r, 304u + 44u * (e + 1u) + 8u, 4u, dir == 0 ? len[e + 1u] - 1u : len[e + 1u] + 1u);
        r->reseal = 1u;
        su_expect(r, 80u, 304u + 44u * e + 8u, e, 0u);
      }
    }
  }
}

/* ---- family: the derived invariants (79) ---- */

static void rs_build_invariants(su_tab *t)
{
  rs_region g;
  uint64_t time_ticks = rs_base_val(t, "time_ticks", 0u, 8u);
  uint64_t ei = rs_base_val(t, "stic.event_index", 0u, 4u);
  uint64_t fi = rs_base_val(t, "stic.frame_index", 0u, 8u);
  uint64_t ns = rs_base_val(t, "psg.next_sample", 0u, 8u);
  uint64_t em = rs_base_val(t, "obs.audio.emitted", 0u, 8u);
  uint64_t vc = rs_base_val(t, "obs.video.count", 0u, 8u);
  rs_regions(t, &g);
  /* INV-2, the STIC timeline and the gate flags */
  {
    rs_mem m1 = {"mem.gate_open", 1u, 1u, 0u};
    rs_mem m2 = {"mem.gate_open", 2u, 1u, 0u};
    rs_mem m0 = {"mem.gate_open", 0u, 1u, 0u};
    rs_mem m3 = {"mem.gate_open", 3u, 1u, 1u};
    rs_mem fr = {"stic.frame_index", 0u, 8u, fi + 1u};
    rs_mem vc1 = {"obs.video.count", 0u, 8u, vc + 1u};
    rs_mem ei1 = {"stic.event_index", 0u, 4u, ei + 1u};
    rs_mem ei0 = {"stic.event_index", 0u, 4u, ei - 1u};
    /* the base is mid-frame, so the register gate is closed or open as the replay says: flip each */
    m1.v = (rs_base_val(t, "mem.gate_open", 1u, 1u) ^ 1u);
    m2.v = (rs_base_val(t, "mem.gate_open", 2u, 1u) ^ 1u);
    rs_member_row(t, "INV-2 the register gate flag flipped", 79u, &g.stic, &m1, NULL, NULL);
    rs_member_row(t, "INV-2 the graphics gate flag flipped", 79u, &g.stic, &m2, NULL, NULL);
    rs_member_row(t, "INV-2 gate 0 is always open", 79u, &g.stic, &m0, NULL, NULL);
    rs_member_row(t, "INV-2 gate 3 is always closed", 79u, &g.stic, &m3, NULL, NULL);
    rs_member_row(t, "INV-2 frame 3 with a video count of 3 (the next event is in the future)", 79u, &g.stic, &fr, &vc1, NULL);
    if (ei + 1u < 16u) {
      rs_member_row(t, "INV-2 the cursor one event ahead", 79u, &g.stic, &ei1, NULL, NULL);
    }
    if (ei >= 1u) {
      rs_member_row(t, "INV-2 the cursor one event behind", 79u, &g.stic, &ei0, NULL, NULL);
    }
  }
  /* INV-3, the PSG sample index, the audio row and the register masks */
  {
    rs_mem n_hi = {"psg.next_sample", 0u, 8u, ns + 1u};
    rs_mem n_lo = {"psg.next_sample", 0u, 8u, ns - 1u};
    rs_mem e_hi = {"obs.audio.emitted", 0u, 8u, em + 1u};
    rs_mem pl = {"obs.audio.prev_levels", 0u, 4u, 0x10u};
    rs_mem pt = {"obs.audio.prev_levels", 0u, 4u, 0x01000000u};
    rs_mem pt2 = {"obs.audio.prev_levels", 0u, 4u, 0x10000000u};
    rs_mem r0 = {"mem.psg_reg", 0u, 2u, 0x0101u};
    rs_mem r4 = {"mem.psg_reg", 4u, 2u, 0x0010u};
    rs_mem r10 = {"mem.psg_reg", 10u, 2u, 0x0010u};
    rs_mem e_up = {"obs.audio.emitted", 0u, 8u, em + 1u};
    rs_member_row(t, "INV-3 next_sample one more than the clock says", 79u, &g.psg, &n_hi, NULL, NULL);
    rs_member_row(t, "INV-3 next_sample and emitted both one more: only the clock disagrees", 79u, &g.psg, &n_hi, &e_up, NULL);
    rs_member_row(t, "INV-3 next_sample one less than the clock says", 79u, &g.psg, &n_lo, NULL, NULL);
    rs_member_row(t, "INV-3 emitted is not next_sample - 1", 79u, &g.psg, &e_hi, NULL, NULL);
    rs_member_row(t, "INV-3 a previous level of 16", 79u, &g.psg, &pl, NULL, NULL);
    rs_member_row(t, "INV-3 the fourth byte of the previous levels is not 0", 79u, &g.psg, &pt, NULL, NULL);
    rs_member_row(t, "INV-3 a fourth byte of the previous levels of 16", 79u, &g.psg, &pt2, NULL, NULL);
    rs_member_row(t, "INV-3 register 0 holds a bit its mask drops", 79u, &g.psg, &r0, NULL, NULL);
    rs_member_row(t, "INV-3 register 4 holds a bit its mask drops", 79u, &g.psg, &r4, NULL, NULL);
    rs_member_row(t, "INV-3 register 10 holds a bit its mask drops", 79u, &g.psg, &r10, NULL, NULL);
  }
  /* INV-4, the controller words, the pull-downs and the input floor */
  {
    rs_mem w0 = {"obs.input.words", 0u, 4u, 0x80000000u};
    rs_mem w2 = {"obs.input.words", 2u, 4u, 1u};
    rs_mem w3 = {"obs.input.words", 3u, 4u, 0x40000000u};
    rs_mem w0k = {"obs.input.words", 0u, 4u, 1u};
    rs_mem p0 = {"mem.port_pulldown", 0u, 1u, 1u};
    rs_mem p1 = {"mem.port_pulldown", 1u, 1u, 0x80u};
    rs_mem fl = {"obs.input.floor_tick", 0u, 8u, time_ticks + 1u};
    rs_member_row(t, "INV-4 the reserved bit of a controller word", 79u, &g.input, &w0, NULL, NULL);
    rs_member_row(t, "INV-4 a word of a controller the profile does not connect", 79u, &g.input, &w2, NULL, NULL);
    rs_member_row(t, "INV-4 a word of the fourth controller", 79u, &g.input, &w3, NULL, NULL);
    rs_member_row(t, "INV-4 a key held with the pull-downs of no key", 79u, &g.input, &w0k, NULL, NULL);
    rs_member_row(t, "INV-4 a pull-down of port 0 with no key held", 79u, &g.input, &p0, NULL, NULL);
    rs_member_row(t, "INV-4 a pull-down of port 1 with no key held", 79u, &g.input, &p1, NULL, NULL);
    rs_member_row(t, "INV-4 the input floor above machine time", 79u, &g.input, &fl, NULL, NULL);
  }
  /* INV-6, the video row counts the frames the STIC completed */
  {
    rs_mem v_hi = {"obs.video.count", 0u, 8u, vc + 1u};
    rs_mem v_lo = {"obs.video.count", 0u, 8u, vc - 1u};
    rs_mem v_big = {"obs.video.count", 0u, 8u, 0xFFFFFFFFFFFFFFFFull};
    rs_member_row(t, "INV-6 one frame more in the video row", 79u, &g.video, &v_hi, NULL, NULL);
    if (vc >= 1u) {
      rs_member_row(t, "INV-6 one frame less in the video row", 79u, &g.video, &v_lo, NULL, NULL);
    }
    rs_member_row(t, "INV-6 a video count of 2^64 - 1", 79u, &g.video, &v_big, NULL, NULL);
  }
}

/* ---- the families as data, and one runner ---- */

typedef struct rs_fam {
  const char *name;
  void (*build)(su_tab *);
  size_t cap;
} rs_fam;

static const rs_fam rs_families[] = {
    {"structure", rs_build_structure, 64u},
    {"identity-profile", rs_build_profile, 64u},
    {"identity-slots", rs_build_slots, 128u},
    {"content-validators", rs_build_validators, 256u},
    {"content-invariants", rs_build_invariants, 64u},
};
#define RS_FAMILIES (sizeof rs_families / sizeof rs_families[0])

/* Builds and runs one family against a fresh run: the number of failing rows. poisoned is the run's target mode and digest_mode its digest
 * mode: 0 none, 1 the nine digests of every row, 2 the nine digests of row 0, every 8th row and the last row (plan 09-02, trim (c)). */
static uint32_t rs_run_family(const rs_fam *f, int poisoned, int digest_mode, uint32_t *produced)
{
  su_tab t;
  su_run u;
  uint32_t bad;
  if (!rs_base_get() || !su_tab_init(&t, f->cap)) {
    return 1u;
  }
  f->build(&t);
  su_run_init(&u, rs_snap, rs_snap_n, poisoned);
  u.digests = digest_mode == 1;
  u.digest_sampled = digest_mode == 2;
  bad = su_run_rows(&u, &t);
  if (produced != NULL) {
    *produced |= u.produced;
  }
  if (bad != 0u) {
    ivs_test_diag("family %s: %u of %u rows failed", f->name, (unsigned)bad, (unsigned)t.n);
  }
  su_run_free(&u);
  su_tab_free(&t);
  return bad;
}

static int rs_family_case(const char *name)
{
  size_t i;
  for (i = 0u; i < RS_FAMILIES; i++) {
    if (strcmp(rs_families[i].name, name) == 0) {
      return rs_run_family(&rs_families[i], 0, 0, NULL) == 0u ? IVS_TEST_PASS : IVS_TEST_FAIL;
    }
  }
  return IVS_TEST_FAIL;
}

static int rs_refuse_structure(void)
{
  return rs_family_case("structure");
}

static int rs_refuse_identity_profile(void)
{
  return rs_family_case("identity-profile");
}

static int rs_refuse_identity_slots(void)
{
  return rs_family_case("identity-slots");
}

static int rs_refuse_content_validators(void)
{
  return rs_family_case("content-validators");
}

static int rs_refuse_content_invariants(void)
{
  return rs_family_case("content-invariants");
}

/* ---- the argument classes of restore (rows written as code) ---- */

/* Stores struct_size of a report that the test places in memory it chose (a native u32, as a caller's struct holds it). */
static void rs_set_struct_size(uint8_t *at)
{
  uint32_t v = SU_CALLER;
  memcpy(at + offsetof(ivs_snapshot_report, struct_size), &v, sizeof v);
}

/* An instance with room on both sides in host memory, so that a buffer or a report can lie before it, after it and across its edges. */
#define RS_GAP 8192u

typedef struct rs_arena {
  uint8_t *base;
  uint8_t *inst; /* the storage of the instance: base + RS_GAP, IVS_MACHINE_BYTES long */
  uint8_t *orig; /* the instance as it was built */
  uint8_t *ref;  /* the instance before the call of a row */
  ivs_machine *m;
} rs_arena;

static void rs_arena_free(rs_arena *a)
{
  free(a->base);
  free(a->orig);
  free(a->ref);
  memset(a, 0, sizeof *a);
}

static int rs_arena_new(rs_arena *a, int poisoned)
{
  memset(a, 0, sizeof *a);
  a->base = (uint8_t *)aligned_alloc(IVS_MACHINE_ALIGN, (size_t)RS_GAP + IVS_MACHINE_BYTES + RS_GAP);
  a->orig = (uint8_t *)malloc(IVS_MACHINE_BYTES);
  a->ref = (uint8_t *)malloc(IVS_MACHINE_BYTES);
  if (a->base == NULL || a->orig == NULL || a->ref == NULL) {
    rs_arena_free(a);
    return 0;
  }
  memset(a->base, 0x5A, (size_t)RS_GAP + IVS_MACHINE_BYTES + RS_GAP);
  a->inst = a->base + RS_GAP;
  if (!su_boot_at(a->inst, SU_IMG_ALL, 1, &a->m) || !ivs_ms_step(a->m, 300u, NULL)) {
    rs_arena_free(a);
    return 0;
  }
  if (poisoned) {
    state_poison(a->m);
  }
  memcpy(a->orig, a->inst, IVS_MACHINE_BYTES);
  return 1;
}

/* One call: the status, the 400 bytes at rep against the report the contract allows (written = 0: nothing may change; else a refusal of
 * reason, offset and section index, with readable bytes of src behind the header facts), and the instance against ref. The memory at rep
 * is the caller's, possibly inside the buffer. */
static int rs_arg_call(const char *name, rs_arena *a, ivs_machine *handle, const uint8_t *buf, uint64_t len, uint8_t *rep_mem, int written,
                       ivs_status want, uint32_t reason, uint64_t off, size_t readable, const uint8_t *src, uint32_t *produced)
{
  uint8_t exp[SU_CALLER];
  ivs_status st;
  int ok = 1;
  if (rep_mem != NULL) {
    memcpy(exp, rep_mem, sizeof exp);
    if (written) {
      su_expect_report(exp, src, readable, reason, off, RS_NONE, 0u);
    }
  }
  memcpy(a->ref, a->inst, IVS_MACHINE_BYTES);
  st = ivs_machine_snapshot_restore(handle, buf, len, rep_mem != NULL ? (ivs_snapshot_report *)(void *)rep_mem : NULL);
  if (st != want) {
    ivs_test_diag("row '%s': status %s (want %s)", name, ivs_status_name(st), ivs_status_name(want));
    ok = 0;
  }
  if (rep_mem != NULL && memcmp(rep_mem, exp, sizeof exp) != 0) {
    size_t i = 0u;
    while (rep_mem[i] == exp[i]) {
      i++;
    }
    ivs_test_diag("row '%s': report byte %u is %02X, the contract says %02X", name, (unsigned)i, (unsigned)rep_mem[i], (unsigned)exp[i]);
    ok = 0;
  }
  if (memcmp(a->inst, a->ref, IVS_MACHINE_BYTES) != 0) {
    ivs_test_diag("row '%s': the instance bytes changed by a refusal", name);
    ok = 0;
  }
  if (ok && produced != NULL && reason >= 58u && reason <= 81u) {
    *produced |= RS_BIT(reason);
  }
  return ok;
}

/* The argument rows against the arena instance (also run by the atomicity sweep of the second task on poisoned instances). */
static int rs_args(uint32_t *produced, int poisoned)
{
  rs_arena a;
  su_rep rep;
  uint8_t *good = NULL;
  uint8_t *bad_magic = NULL;
  uint8_t *big = NULL;
  size_t n;
  int ok = 1;
  if (!rs_base_get() || !rs_arena_new(&a, poisoned)) {
    return 0;
  }
  n = rs_snap_n;
  good = su_heap_copy(rs_snap, n);
  bad_magic = su_heap_copy(rs_snap, n);
  big = (uint8_t *)malloc(n + 2u * SU_CALLER);
  if (good == NULL || bad_magic == NULL || big == NULL) {
    free(good);
    free(bad_magic);
    free(big);
    rs_arena_free(&a);
    return 0;
  }
  bad_magic[0] ^= 0x01u;

  /* a NULL buffer with a length (58), and an empty one (the reader's truncation) */
  memset(rep.b, 0xA5, sizeof rep.b);
  rep.r.struct_size = SU_CALLER;
  ok &= rs_arg_call("58 NULL buffer with a length", &a, a.m, NULL, 100u, rep.b, 1, IVS_E_BAD_ARG, 58u, 0u, 0u, rs_snap, produced);
  memset(rep.b, 0xA5, sizeof rep.b);
  rep.r.struct_size = SU_CALLER;
  ok &= rs_arg_call("61 NULL buffer, length 0", &a, a.m, NULL, 0u, rep.b, 1, IVS_E_MALFORMED, 61u, 0u, 0u, rs_snap, produced);

  /* an unusable report: nothing is written, whatever else is wrong */
  memset(rep.b, 0xA5, sizeof rep.b);
  rep.r.struct_size = IVS_SNAPSHOT_REPORT_V1_SIZE - 1u;
  ok &= rs_arg_call("a report struct_size below the v1 size", &a, a.m, good, n, rep.b, 0, IVS_E_BAD_ARG, 0u, 0u, 0u, rs_snap, NULL);
  memset(rep.b, 0xA5, sizeof rep.b);
  rep.r.struct_size = IVS_STRUCT_SIZE_MAX + 1u;
  ok &= rs_arg_call("a report struct_size above the maximum", &a, a.m, good, n, rep.b, 0, IVS_E_BAD_ARG, 0u, 0u, 0u, rs_snap, NULL);
  memset(rep.b, 0xA5, sizeof rep.b);
  rep.r.struct_size = 0u;
  ok &= rs_arg_call("a report struct_size of 0, a NULL buffer and a NULL handle", &a, NULL, NULL, 100u, rep.b, 0, IVS_E_BAD_ARG, 0u, 0u, 0u,
                    rs_snap, NULL);

  /* a NULL report is allowed: a refusal says the status only */
  ok &= rs_arg_call("62 a NULL report with a bad magic", &a, a.m, bad_magic, n, NULL, 0, IVS_E_MALFORMED, 62u, 0u, 0u, rs_snap, NULL);
  ok &= rs_arg_call("58 a NULL report with a NULL buffer", &a, a.m, NULL, 7u, NULL, 0, IVS_E_BAD_ARG, 58u, 0u, 0u, rs_snap, NULL);

  /* a report that overlaps the buffer (59): inside it, across its last bytes; and one right after it that does not (all 8-byte aligned) */
  memcpy(big, rs_snap, n);
  memset(big + n, 0xC3, 2u * SU_CALLER);
  rs_set_struct_size(big + 2000u);
  ok &= rs_arg_call("59 the report inside the buffer", &a, a.m, big, n, big + 2000u, 1, IVS_E_BAD_ARG, 59u, 0u, 0u, rs_snap, produced);
  memcpy(big, rs_snap, n);
  memset(big + n, 0xC3, 2u * SU_CALLER);
  rs_set_struct_size(big + (n & ~(size_t)7u));
  ok &= rs_arg_call("59 the report across the last bytes of the buffer", &a, a.m, big, n, big + (n & ~(size_t)7u), 1, IVS_E_BAD_ARG, 59u, 0u, 0u,
                    rs_snap, produced);
  memcpy(big, bad_magic, n);
  memset(big + n, 0xC3, 2u * SU_CALLER);
  rs_set_struct_size(big + ((n + 7u) & ~(size_t)7u));
  ok &= rs_arg_call("62 the report right after the buffer is no overlap", &a, a.m, big, n, big + ((n + 7u) & ~(size_t)7u), 1, IVS_E_MALFORMED, 62u,
                    0u, n, bad_magic, produced);

  /* a buffer that overlaps the instance (81): placed inside the instance, across its start and across its end. The buffer is never read
   * (the refusal comes first), so its bytes are whatever the instance and its gaps hold. */
  {
    static const struct {
      const char *label;
      long delta; /* where the buffer starts, relative to the instance start */
    } rows[] = {
        {"81 the buffer inside the instance", 3000},
        {"81 the buffer across the start of the instance", -100},
        {"81 the buffer across the end of the instance", (long)IVS_MACHINE_BYTES - 100},
        {"81 the buffer starting on the last byte of the instance", (long)IVS_MACHINE_BYTES - 1},
        {"81 the buffer ending on the first byte of the instance", -4298},
    };
    size_t i;
    for (i = 0u; i < sizeof rows / sizeof rows[0]; i++) {
      memset(rep.b, 0xA5, sizeof rep.b);
      rep.r.struct_size = SU_CALLER;
      ok &= rs_arg_call(rows[i].label, &a, a.m, a.inst + rows[i].delta, n, rep.b, 1, IVS_E_BAD_ARG, 81u, 0u, 0u, rs_snap, produced);
    }
    /* a buffer that ends where the instance begins and one that begins where it ends touch it without overlapping it */
    memcpy(a.inst - n, bad_magic, n);
    memset(rep.b, 0xA5, sizeof rep.b);
    rep.r.struct_size = SU_CALLER;
    ok &= rs_arg_call("62 the buffer ending where the instance begins", &a, a.m, a.inst - n, n, rep.b, 1, IVS_E_MALFORMED, 62u, 0u, n,
                      bad_magic, produced);
    memcpy(a.inst + IVS_MACHINE_BYTES, bad_magic, n);
    memset(rep.b, 0xA5, sizeof rep.b);
    rep.r.struct_size = SU_CALLER;
    ok &= rs_arg_call("62 the buffer beginning where the instance ends", &a, a.m, a.inst + IVS_MACHINE_BYTES, n, rep.b, 1, IVS_E_MALFORMED, 62u, 0u,
                      n, bad_magic, produced);
    /* an absurd length for a buffer inside the instance is judged over the bound: an overlap, not OVERSIZED */
    memset(rep.b, 0xA5, sizeof rep.b);
    rep.r.struct_size = SU_CALLER;
    ok &= rs_arg_call("81 a length of 2^40 for a buffer inside the instance", &a, a.m, a.inst + 3000u, (uint64_t)1u << 40, rep.b, 1,
                      IVS_E_BAD_ARG, 81u, 0u, 0u, rs_snap, produced);
  }

  /* a report that overlaps the instance: nothing is written (the instance must stay byte-identical), the status is the same BAD_ARG */
  {
    uint8_t *at = a.inst + ((offsetof(struct ivs_machine, mem.sysram) + 64u + 7u) & ~(size_t)7u);
    memcpy(a.inst, a.orig, IVS_MACHINE_BYTES);
    rs_set_struct_size(at);
    ok &= rs_arg_call("81 the report inside the instance: nothing written", &a, a.m, good, n, at, 0, IVS_E_BAD_ARG, 81u, 0u, 0u, rs_snap, NULL);
    memcpy(a.inst, a.orig, IVS_MACHINE_BYTES);
    rs_set_struct_size(at);
    ok &= rs_arg_call("81 a NULL buffer and the report inside the instance: nothing written", &a, a.m, NULL, 100u, at, 0, IVS_E_BAD_ARG, 58u, 0u,
                      0u, rs_snap, NULL);
    memcpy(a.inst, a.orig, IVS_MACHINE_BYTES);
  }

  /* the handle: NULL, a damaged magic, a destroyed instance; the report says the handle reason; arguments come first */
  memset(rep.b, 0xA5, sizeof rep.b);
  rep.r.struct_size = SU_CALLER;
  ok &= rs_arg_call("a NULL handle", &a, NULL, good, n, rep.b, 1, IVS_E_BAD_HANDLE, IVS_REASON_HANDLE, 0u, 0u, rs_snap, NULL);
  memset(rep.b, 0xA5, sizeof rep.b);
  rep.r.struct_size = SU_CALLER;
  ok &= rs_arg_call("58 a NULL handle and a NULL buffer: the argument first", &a, NULL, NULL, 10u, rep.b, 1, IVS_E_BAD_ARG, 58u, 0u, 0u,
                    rs_snap, produced);
  {
    ivs_ms_inst dead;
    if (su_boot_new(&dead, SU_IMG_ALL, 1, 0x5Au)) {
      ok &= ivs_machine_destroy(dead.m) == IVS_OK;
      memset(rep.b, 0xA5, sizeof rep.b);
      rep.r.struct_size = SU_CALLER;
      /* the handle of a destroyed instance is refused; the live arena is compared as the unchanged instance */
      ok &= rs_arg_call("a destroyed handle", &a, dead.m, good, n, rep.b, 1, IVS_E_BAD_HANDLE, IVS_REASON_HANDLE, 0u, 0u, rs_snap, NULL);
      ivs_ms_free(&dead);
    } else {
      ok = 0;
    }
    if (su_boot_new(&dead, SU_IMG_ALL, 1, 0x5Au)) {
      dead.storage[0] ^= 0x01u; /* the magic */
      memset(rep.b, 0xA5, sizeof rep.b);
      rep.r.struct_size = SU_CALLER;
      ok &= rs_arg_call("a handle whose magic is damaged", &a, dead.m, good, n, rep.b, 1, IVS_E_BAD_HANDLE, IVS_REASON_HANDLE, 0u, 0u, rs_snap,
                        NULL);
      ivs_ms_free(&dead);
    } else {
      ok = 0;
    }
  }
  free(good);
  free(bad_magic);
  free(big);
  rs_arena_free(&a);
  return ok;
}

static int rs_refuse_args(void)
{
  return rs_args(NULL, 0) ? IVS_TEST_PASS : IVS_TEST_FAIL;
}

/* ---- the seam: the comparison of the profile id (reason 73) ---- */

/* A view with the given id, as the reader fills it after a success. */
static void rs_view(ivs_snap_view *v, const char *id)
{
  size_t n = strlen(id);
  memset(v, 0, sizeof *v);
  memcpy(v->profile_id, id, n < IVS_PROFILE_ID_MAX_BYTES ? n : IVS_PROFILE_ID_MAX_BYTES);
  v->profile_id_length = (uint32_t)n;
}

static void rs_desc(ivs_profile_desc *d, const char *id)
{
  memset(d, 0, sizeof *d);
  memcpy(d->id, id, strlen(id) < sizeof d->id ? strlen(id) : sizeof d->id - 1u);
}

/* 1 when the seam judges every row as the specification says: used by state.restore.profile-id-seam and by the coverage of reason 73. */
static int rs_seam_rows(int say)
{
  static const struct {
    const char *snap;
    const char *inst;
    int equal;
    const char *what;
  } rows[] = {
      {"intv-2609-ntsc-family-p1", "intv-2609-ntsc-family-p1", 1, "equal"},
      {"intv-2609-ntsc-family-p1", "intv-2609-ntsc-family-p2", 0, "different in the last byte"},
      {"xntv-2609-ntsc-family-p1", "intv-2609-ntsc-family-p1", 0, "different in the first byte"},
      {"intv-2609", "intv-2609-ntsc-family-p1", 0, "the snapshot id is a prefix of the instance id"},
      {"intv-2609-ntsc-family-p1", "intv-2609", 0, "the snapshot id is longer than the instance id"},
      {"", "intv-2609-ntsc-family-p1", 0, "an empty snapshot id"},
      {"intv-2609-ntsc-family-p1", "", 0, "an empty instance id"},
      {"", "", 1, "two empty ids"},
      {"a", "a", 1, "one byte"},
      {"a", "b", 0, "one byte, different"},
      {"abcdefghijklmnopqrstuvwxyz0123456789abcdefghijklmnopqrstuvwxyz0", "abcdefghijklmnopqrstuvwxyz0123456789abcdefghijklmnopqrstuvwxyz0", 1,
       "63 bytes, equal"},
      {"abcdefghijklmnopqrstuvwxyz0123456789abcdefghijklmnopqrstuvwxyz0", "abcdefghijklmnopqrstuvwxyz0123456789abcdefghijklmnopqrstuvwxyz1", 0,
       "63 bytes, different in the last"},
      {"intv-2609-ntsc-family-p1", "INTV-2609-ntsc-family-p1", 0, "case matters"},
  };
  size_t i;
  int ok = 1;
  for (i = 0u; i < sizeof rows / sizeof rows[0]; i++) {
    ivs_snap_view v;
    ivs_profile_desc d;
    uint32_t got;
    rs_view(&v, rows[i].snap);
    rs_desc(&d, rows[i].inst);
    got = ivs_snapshot_profile_id_matches(&v, &d);
    if (got != (uint32_t)rows[i].equal) {
      if (say) {
        ivs_test_diag("seam row '%s': the comparison says %u, the specification says %d", rows[i].what, (unsigned)got, rows[i].equal);
      }
      ok = 0;
    }
  }
  /* NULL arguments and an id length that no reader produces are no match */
  {
    ivs_snap_view v;
    ivs_profile_desc d;
    rs_view(&v, "a");
    rs_desc(&d, "a");
    ok = ok && ivs_snapshot_profile_id_matches(NULL, &d) == 0u && ivs_snapshot_profile_id_matches(&v, NULL) == 0u;
    v.profile_id_length = IVS_PROFILE_ID_MAX_BYTES;
    ok = ok && ivs_snapshot_profile_id_matches(&v, &d) == 0u;
  }
  /* the registered descriptor matches the id of the base snapshot as the reader reports it */
  {
    ivs_snap_view v;
    ivs_status st;
    uint8_t *h;
    ok = ok && rs_base_get();
    h = ok ? su_heap_copy(rs_snap, rs_snap_n) : NULL;
    ok = ok && h != NULL;
    if (ok) {
      st = ivs_snapshot_read(h, rs_snap_n, &v);
      ok = st == IVS_OK && ivs_snapshot_profile_id_matches(&v, ivs_profile_get(st_profile_index())) == 1u;
    }
    free(h);
  }
  return ok;
}

/* state.restore.profile-id-seam: a SEAM test, not an API test. With one registered profile a second registered id cannot be built through
 * the public API, so the comparison that gives reason 73 (the id is registered and differs from the instance's) is held as a pure function
 * with a synthetic descriptor: equal, different, a prefix of, longer than, empty, 63 bytes, case. The API rows of the family reach reason
 * 72 (an unregistered id) and 74 (a parameter digest); reason 73 is reached only here. */
static int rs_profile_id_seam(void)
{
  return rs_seam_rows(1) ? IVS_TEST_PASS : IVS_TEST_FAIL;
}

/* ---- every reason is produced ---- */

/* state.restore.reasons-covered: every one of the reasons 58 to 81 that restore can give is produced by some row, exactly as expected. 73
 * is the one reason no API row can produce (one registered profile): it is covered by the seam rows, which this case runs again. */
static int rs_reasons_covered(void)
{
  uint32_t produced = 0u;
  uint32_t bad = 0u;
  size_t i;
  int ok = 1;
  for (i = 0u; i < RS_FAMILIES; i++) {
    bad += rs_run_family(&rs_families[i], 0, 0, &produced);
  }
  ok &= rs_args(&produced, 0);
  ST_CHECK(bad == 0u);
  if (rs_seam_rows(1)) {
    produced |= RS_BIT(73u); /* the seam judges a registered id that differs from the instance's: the comparison behind reason 73 */
  }
  if (produced != RS_ALL_REASONS) {
    uint32_t r;
    for (r = 58u; r <= 81u; r++) {
      if ((produced & RS_BIT(r)) == 0u) {
        ivs_test_diag("reason %u is produced by no row", (unsigned)r);
      }
    }
    ok = 0;
  }
  return ok ? IVS_TEST_PASS : IVS_TEST_FAIL;
}

/* ---- the round trip ---- */

/* The nine digests of two instances equal, and the whole instances byte-identical. */
static int rs_same_instance(const char *label, const ivs_ms_inst *a, const ivs_ms_inst *b)
{
  uint8_t da[9][32];
  uint8_t db[9][32];
  int ok = 1;
  ST_REQ(ok, su_nine_digests(a->m, da) && su_nine_digests(b->m, db));
  ST_REQ(ok, memcmp(da, db, sizeof da) == 0);
  ST_REQ(ok, memcmp(a->storage, b->storage, IVS_MACHINE_BYTES) == 0);
  if (!ok) {
    ivs_test_diag("%s: the instances differ", label);
  }
  return ok;
}

/* Puts the big-endian value v into an element of a STATE row of a snapshot buffer, at the position the digest field table gives (su_row_pos
 * read as data). 1, or 0 with a diagnostic when the row does not exist. */
static int rs_snap_put(uint8_t *snap, const char *row, uint32_t elem, uint32_t width, uint64_t v)
{
  uint32_t sec = 0u;
  uint32_t off = 0u;
  uint32_t cnt = 0u;
  if (!su_row_pos(row, &sec, &off, &cnt)) {
    ivs_test_diag("no STATE row '%s' in the field table", row);
    return 0;
  }
  su_put(snap + off + elem * width, v, width);
  return 1;
}

/* The top of the clock (WR-01 of the Phase 8 review): a snapshot restored at 2^63 - 1, ONE public run call to IVS_RUN_TARGET_MAX, which ends
 * at the first unit boundary at or after 2^63, a save of that state and a restore of those bytes. The second restore must be ACCEPTED: a
 * snapshot this library wrote is one it reads back, also at the top of the clock. INV-5's bound is what a run call can produce, IVS_RUN_TARGET_MAX
 * plus one unit (U = psg.max_unit_ticks); when it was IVS_RUN_TARGET_MAX alone the second restore was refused MALFORMED, reason 78 at the
 * time_ticks row, which is the defect this scenario locks in (it fails against that bound; the review widened the bound and this case proves
 * the widening). The state at 2^63 - 1 is a real stepped state moved forward by whole frames, with the event cursor and the gate flags at the
 * position 2^63 - 1 falls in, so it is a state the machine itself could hold; every value is derived from the profile's own tick tables. */
static int rs_roundtrip_top_of_clock(void)
{
  const ivs_stic_params *sp = ivs_stic_params_get(st_profile_index());
  const ivs_psg_params *pp = ivs_psg_params_get(st_profile_index());
  ivs_ms_inst src;
  ivs_ms_inst mid;
  ivs_ms_inst end;
  static uint8_t snap[IVS_SNAPSHOT_MAX_BYTES];
  static uint8_t snap2[IVS_SNAPSHOT_MAX_BYTES];
  size_t n1 = 0u;
  size_t n2 = 0u;
  size_t n3 = 0u;
  const uint64_t t1 = ((uint64_t)1u << 63) - 1u;
  uint64_t fi;
  uint64_t rem;
  uint64_t clock;
  uint32_t ei;
  uint32_t regs = 1u;
  uint32_t gmem = 1u;
  uint32_t consumed = 0u;
  int ok = 1;
  ST_CHECK(sp != NULL && pp != NULL && pp->sample_period_ticks != 0u);
  ST_CHECK(su_boot_stepped(&src, SU_BOOT_UNITS, 0x5Au) && su_snapshot(src.m, snap, sizeof snap, &n1));
  ivs_ms_free(&src);
  /* The position of 2^63 - 1 on the timeline: the frame it falls in, the cursor at the first event above that offset (the next frame's
   * first event when no event of this frame is above it), and the gate flags the replay of the events below the cursor gives. */
  rem = t1 - (uint64_t)sp->frame_phase_ticks;
  fi = rem / sp->frame_ticks;
  rem %= sp->frame_ticks;
  for (ei = 0u; ei < sp->event_count; ei++) {
    if ((uint64_t)sp->event_tick[ei] > rem) {
      break;
    }
  }
  if (ei == sp->event_count) {
    fi += 1u;
    ei = 0u;
  }
  for (uint32_t e = 0u; e < ei; e++) {
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
  /* The rows the other invariants tie to machine time move with it: the frame cursor and the video count (INV-2, INV-6), the PSG sample
   * index and the emitted audio count (INV-3), the gate flags (INV-2). */
  ST_REQ(ok, rs_snap_put(snap, "time_ticks", 0u, 8u, t1));
  ST_REQ(ok, rs_snap_put(snap, "stic.frame_index", 0u, 8u, fi));
  ST_REQ(ok, rs_snap_put(snap, "stic.event_index", 0u, 4u, (uint64_t)ei));
  ST_REQ(ok, rs_snap_put(snap, "obs.video.count", 0u, 8u, fi));
  ST_REQ(ok, rs_snap_put(snap, "psg.next_sample", 0u, 8u, t1 / pp->sample_period_ticks + 1u));
  ST_REQ(ok, rs_snap_put(snap, "obs.audio.emitted", 0u, 8u, t1 / pp->sample_period_ticks));
  ST_REQ(ok, rs_snap_put(snap, "mem.gate_open", 0u, 1u, 1u));
  ST_REQ(ok, rs_snap_put(snap, "mem.gate_open", 1u, 1u, regs));
  ST_REQ(ok, rs_snap_put(snap, "mem.gate_open", 2u, 1u, gmem));
  ST_REQ(ok, rs_snap_put(snap, "mem.gate_open", 3u, 1u, 0u));
  su_reseal(snap, n1);
  /* The first restore: a snapshot at 2^63 - 1 is inside the bound. Everything after it needs an instance that really holds that state; on a
   * refused restore (a fault build of the writer, say) the case stops here instead of running a reset machine towards 2^63. */
  ST_REQ(ok, su_boot_new(&mid, SU_IMG_ALL, 1, 0x33u));
  if (ok) {
    ST_REQ(ok, ivs_machine_snapshot_restore(mid.m, snap, n1, NULL) == IVS_OK);
  }
  if (!ok) {
    ivs_test_diag("the snapshot at 2^63 - 1 was not accepted; the rest of the scenario needs that state");
    ivs_ms_free(&mid);
    return ok;
  }
  ST_REQ(ok, ((const struct ivs_machine *)mid.m)->time_ticks == t1);
  if (!ok) {
    /* the run below is only bounded when the clock really is at 2^63 - 1 */
    ivs_ms_free(&mid);
    return 0;
  }
  /* one run call to the last tick a host may ask for: it ends past 2^63 (at the first unit boundary at or after it) */
  ST_REQ(ok, su_run_to(mid.m, IVS_RUN_TARGET_MAX, NULL, 0u, &consumed));
  clock = ((const struct ivs_machine *)mid.m)->time_ticks;
  ST_REQ(ok, clock > IVS_RUN_TARGET_MAX && clock <= IVS_RUN_TARGET_MAX + (uint64_t)pp->max_unit_ticks);
  /* the save of that state and its restore: the second restore must be accepted, and it re-encodes to the same bytes */
  ST_REQ(ok, su_snapshot(mid.m, snap2, sizeof snap2, &n2));
  if (ok) {
    ST_REQ(ok, su_boot_new(&end, SU_IMG_ALL, 1, 0x5Au));
  }
  if (!ok) {
    ivs_ms_free(&mid);
    return 0;
  }
  if (!(ivs_machine_snapshot_restore(end.m, snap2, n2, NULL) == IVS_OK)) {
    ivs_test_diag("a snapshot saved at %llu (above 2^63) is refused by its own restore", (unsigned long long)clock);
    ok = 0;
  }
  ST_REQ(ok, ((const struct ivs_machine *)end.m)->time_ticks == clock);
  ST_REQ(ok, memcmp(end.storage, mid.storage, IVS_MACHINE_BYTES) == 0);
  ST_REQ(ok, su_snapshot(end.m, snap, sizeof snap, &n3) && n3 == n2 && memcmp(snap, snap2, n2) == 0);
  ivs_ms_free(&mid);
  ivs_ms_free(&end);
  return ok;
}

/* state.restore.roundtrip-fixed-point (D-14, D-16): at several cuts of the stand-in boot run, save, restore into a second instance of the
 * same images that is at another point of its run, and save again: the second snapshot is byte-identical to the first (snapshot, restore,
 * snapshot is a fixed point), the nine digests of the two instances are equal and so are their bytes, the report of a success is the report
 * that describe gives, and restoring the same snapshot again changes nothing. The cut of 5000 units is the one the literal
 * stepped-snapshot-sha256 pins, so the snapshot restored here is the pinned one. */
static int rs_roundtrip_fixed_point(void)
{
  static const uint32_t cuts[] = {0u, 1u, 100u, 3333u, SU_BOOT_UNITS, 9999u};
  uint8_t want[32];
  size_t c;
  int ok = 1;
  ST_CHECK(ivsn_lit_hex("stepped-snapshot-sha256", want, 32u));
  for (c = 0u; c < sizeof cuts / sizeof cuts[0]; c++) {
    ivs_ms_inst src;
    ivs_ms_inst dst;
    uint8_t s1[IVS_SNAPSHOT_MAX_BYTES];
    uint8_t s2[IVS_SNAPSHOT_MAX_BYTES];
    uint8_t got[32];
    size_t n1 = 0u;
    size_t n2 = 0u;
    su_rep rep;
    su_rep want_rep;
    if (!su_boot_stepped(&src, cuts[c], 0x5Au)) {
      return IVS_TEST_FAIL;
    }
    if (!su_boot_stepped(&dst, 250u + cuts[c] % 7u, 0x33u) || !su_snapshot(src.m, s1, sizeof s1, &n1)) {
      ivs_ms_free(&src);
      return IVS_TEST_FAIL;
    }
    if (cuts[c] == SU_BOOT_UNITS) {
      ivs_sha256_digest(s1, n1, got);
      ST_REQ(ok, memcmp(got, want, 32u) == 0);
    }
    memset(rep.b, 0xA5, sizeof rep.b);
    rep.r.struct_size = SU_CALLER;
    memset(want_rep.b, 0xA5, sizeof want_rep.b);
    want_rep.r.struct_size = SU_CALLER;
    ST_REQ(ok, ivs_snapshot_describe(s1, n1, &want_rep.r) == IVS_OK);
    ST_REQ(ok, ivs_machine_snapshot_restore(dst.m, s1, n1, &rep.r) == IVS_OK);
    ST_REQ(ok, memcmp(rep.b, want_rep.b, sizeof rep.b) == 0 && rep.r.struct_size == IVS_SNAPSHOT_REPORT_V1_SIZE);
    ST_REQ(ok, su_snapshot(dst.m, s2, sizeof s2, &n2) && n2 == n1 && memcmp(s1, s2, n1) == 0);
    ST_REQ(ok, rs_same_instance("after the restore", &src, &dst));
    /* again: restoring the snapshot a second time changes nothing */
    ST_REQ(ok, ivs_machine_snapshot_restore(dst.m, s1, n1, NULL) == IVS_OK);
    ST_REQ(ok, rs_same_instance("after the second restore", &src, &dst));
    /* the restored machine runs on as the source does */
    ST_REQ(ok, ivs_ms_step(src.m, 400u, NULL) && ivs_ms_step(dst.m, 400u, NULL));
    ST_REQ(ok, rs_same_instance("after 400 more units", &src, &dst));
    ivs_ms_free(&src);
    ivs_ms_free(&dst);
  }
  /* and at the top of the clock, where a run call can end past 2^63 (WR-01 of the Phase 8 review) */
  ST_REQ(ok, rs_roundtrip_top_of_clock());
  return ok ? IVS_TEST_PASS : IVS_TEST_FAIL;
}

/* ---- restore into a poisoned target equals the source ---- */

/* The whole instances byte-identical and the nine digests equal; label names the situation in the diagnostic. */
static int rs_equal_instances(const char *label, const ivs_ms_inst *want, const ivs_ms_inst *got)
{
  uint8_t dw[9][32];
  uint8_t dg[9][32];
  int ok = 1;
  ST_REQ(ok, su_nine_digests(want->m, dw) && su_nine_digests(got->m, dg));
  ST_REQ(ok, memcmp(dw, dg, sizeof dw) == 0);
  if (memcmp(want->storage, got->storage, IVS_MACHINE_BYTES) != 0) {
    size_t i = 0u;
    size_t rows = ivs_machine_digest_field_count();
    while (((const uint8_t *)want->storage)[i] == ((const uint8_t *)got->storage)[i]) {
      i++;
    }
    for (size_t r = 0u; r < rows; r++) {
      const ivs_digest_field *f = ivs_machine_digest_field(r);
      if (f != NULL && i >= f->offset && i < f->offset + (size_t)f->width * f->count) {
        ivs_test_diag("%s: the instances differ at byte %u, row %s", label, (unsigned)i, f->name);
        break;
      }
    }
    ok = 0;
  }
  return ok;
}

/* The cuts of state.restore.poisoned-target-equals-source: a scenario (or the stand-in boot program, scenario SU_SC_COUNT) run through the
 * public call to a tick, so each is a unit boundary reached the way a host reaches it. */
typedef struct rs_cut {
  uint32_t scenario;
  uint64_t ticks;
} rs_cut;

static int rs_cut_source(ivs_ms_inst *x, uint32_t scenario, uint64_t ticks, uint32_t *consumed)
{
  const ivs_input_event *ev;
  uint32_t n = 0u;
  *consumed = 0u;
  if (scenario == SU_SC_COUNT ? !su_boot_new(x, SU_IMG_ALL, 1, 0x5Au) : !su_scenario_new(x, scenario, 1)) {
    return 0;
  }
  ev = scenario == SU_SC_COUNT ? NULL : su_scenario_events(scenario, &n);
  if (ticks > 0u && !su_run_to(x->m, ticks, ev, n, consumed)) {
    ivs_ms_free(x);
    return 0;
  }
  return 1;
}

/* state.restore.poisoned-target-equals-source (D-15, D-16, the table-free oracle): at each cut, a snapshot of the source is restored into a
 * target of the same images whose STATE and PAD rows were filled with a poison byte through the offset, width and count of the digest field
 * table: afterwards the WHOLE target instance is byte-identical to the source (the pad bytes too, because restore zeroes them), the nine
 * digests are equal, and a snapshot of the target is the snapshot again. A restore that misses any row leaves poison behind; the test runs
 * with two different poison bytes, so a row whose source bytes happen to equal one of them is still seen with the other, and a target
 * that is all zero (where a skipped row would look restored) is never used. */
static int rs_poisoned_target(void)
{
  static const rs_cut cuts[] = {
      {SU_SC_COUNT, 1u},        {SU_SC_COUNT, 59735u},   {SU_SC_COUNT, 59736u},    {SU_SC_COUNT, 70000u},  {SU_SC_COUNT, 119472u},
      {SU_SC_COUNT, 173352u},   {SU_SC_LAMP_IDLE, 100000u}, {SU_SC_LAMP_IDLE, 200000u}, {SU_SC_LAMP_IDLE, 400000u},
      {SU_SC_LAMP_PASS, 250000u}, {SU_SC_LAMP_PASS, 350000u}, {SU_SC_STIC2_MOB, 70000u}, {SU_SC_STIC2_COLLISION, 130000u},
  };
  static const uint8_t poison[2] = {0xA5u, 0x5Au};
  size_t c;
  int ok = 1;
  for (c = 0u; c < sizeof cuts / sizeof cuts[0]; c++) {
    ivs_ms_inst src;
    uint8_t snap[IVS_SNAPSHOT_MAX_BYTES];
    uint8_t again[IVS_SNAPSHOT_MAX_BYTES];
    size_t n = 0u;
    size_t n2 = 0u;
    uint32_t consumed = 0u;
    uint32_t k;
    char label[64];
    if (!rs_cut_source(&src, cuts[c].scenario, cuts[c].ticks, &consumed) || !su_snapshot(src.m, snap, sizeof snap, &n)) {
      return IVS_TEST_FAIL;
    }
    for (k = 0u; k < 2u; k++) {
      ivs_ms_inst dst;
      ivs_status st;
      int built = cuts[c].scenario == SU_SC_COUNT ? su_boot_new(&dst, SU_IMG_ALL, 1, 0x33u) : su_scenario_new(&dst, cuts[c].scenario, 1);
      (void)snprintf(label, sizeof label, "cut %u (%s at %llu), poison %02X", (unsigned)c,
                     cuts[c].scenario == SU_SC_COUNT ? "boot" : su_scenario_name(cuts[c].scenario), (unsigned long long)cuts[c].ticks,
                     (unsigned)poison[k]);
      if (!built) {
        ivs_ms_free(&src);
        return IVS_TEST_FAIL;
      }
      su_poison(dst.m, poison[k]);
      ST_REQ(ok, memcmp(src.storage, dst.storage, IVS_MACHINE_BYTES) != 0);
      st = ivs_machine_snapshot_restore(dst.m, snap, n, NULL);
      ST_REQ(ok, st == IVS_OK);
      ST_REQ(ok, rs_equal_instances(label, &src, &dst));
      ST_REQ(ok, su_snapshot(dst.m, again, sizeof again, &n2) && n2 == n && memcmp(snap, again, n) == 0);
      ivs_ms_free(&dst);
    }
    ivs_ms_free(&src);
  }
  return ok ? IVS_TEST_PASS : IVS_TEST_FAIL;
}

/* ---- any instance of the same profile and images ---- */

/* Restores a snapshot of src into dst and requires the whole instance to equal src afterwards, the machine POWERED. */
static int rs_restore_into(const char *label, const ivs_ms_inst *src, ivs_ms_inst *dst)
{
  uint8_t snap[IVS_SNAPSHOT_MAX_BYTES];
  size_t n = 0u;
  ivs_info info;
  int ok = 1;
  if (!su_snapshot(src->m, snap, sizeof snap, &n)) {
    ivs_test_diag("%s: no snapshot of the source", label);
    return 0;
  }
  ST_REQ(ok, ivs_machine_snapshot_restore(dst->m, snap, n, NULL) == IVS_OK);
  memset(&info, 0, sizeof info);
  info.struct_size = (uint32_t)sizeof info;
  ST_REQ(ok, ivs_machine_info(dst->m, &info) == IVS_OK && info.phase == IVS_PHASE_POWERED);
  ST_REQ(ok, rs_equal_instances(label, src, dst));
  return ok;
}

/* The loop-then-HLT and loop-then-fault programs of tests/run/run_slice.c (derived there from the vector rows). */
static const uint16_t rs_prog_halt[] = {0x02B9u, 0x0008u, 0x0011u, 0x022Cu, 0x0002u, 0x0000u, 0x0089u};
static const uint16_t rs_prog_fault[] = {0x02B9u, 0x0008u, 0x0011u, 0x022Cu, 0x0002u, 0x0264u};

/* A powered instance of the words, run by the seam for units units. */
static int rs_words_at(ivs_ms_inst *x, const uint16_t *words, size_t n, uint32_t units)
{
  if (!ivs_ms_powered_words(x, words, n, 0x5Au)) {
    return 0;
  }
  if (units > 0u && !ivs_ms_step(x->m, units, NULL)) {
    ivs_ms_free(x);
    return 0;
  }
  return 1;
}

/* state.restore.same-profile-any-instance (D-14): a snapshot goes into any instance of the profile whose slots match, whatever that instance
 * was doing: an OFF instance with the images loaded (the fresh target), a powered one at its reset, one at another tick, one far on, a
 * poisoned one, one whose CPU is halted and one whose CPU has faulted; and a halted or faulted source goes into a running target. After each
 * restore the whole target equals the source byte for byte and the machine is POWERED. */
static int rs_same_profile_any_instance(void)
{
  ivs_ms_inst src;
  int ok = 1;
  uint32_t k;
  /* the boot program at 5000 units into targets in other states */
  ST_CHECK(su_boot_stepped(&src, SU_BOOT_UNITS, 0x5Au));
  for (k = 0u; k < 6u; k++) {
    static const char *const names[6] = {"an OFF instance with the images", "a powered instance at its reset", "a powered instance at 777 units",
                                         "a powered instance far on", "a poisoned OFF instance", "a poisoned powered instance"};
    ivs_ms_inst dst;
    int built = k == 0u || k == 4u ? su_boot_new(&dst, SU_IMG_ALL, 0, 0x33u)
                                   : su_boot_stepped(&dst, k == 1u ? 0u : (k == 2u ? 777u : (k == 3u ? 30000u : 400u)), 0x33u);
    if (!built) {
      ivs_ms_free(&src);
      return IVS_TEST_FAIL;
    }
    if (k >= 4u) {
      state_poison(dst.m);
    }
    ST_REQ(ok, rs_restore_into(names[k], &src, &dst));
    ivs_ms_free(&dst);
  }
  ivs_ms_free(&src);
  /* the halting program: a running source into a halted target, and the halted source into a running target */
  {
    ivs_ms_inst run;
    ivs_ms_inst halted;
    size_t n = sizeof rs_prog_halt / sizeof rs_prog_halt[0];
    ST_CHECK(rs_words_at(&run, rs_prog_halt, n, 3u));
    ST_CHECK(rs_words_at(&halted, rs_prog_halt, n, 60u));
    ST_REQ(ok, ((const struct ivs_machine *)halted.m)->cpu.run_state == IVS_CP1610_RS_HALTED);
    ST_REQ(ok, rs_restore_into("a running source into a halted target", &run, &halted));
    ST_REQ(ok, ((const struct ivs_machine *)halted.m)->cpu.run_state != IVS_CP1610_RS_HALTED);
    ivs_ms_free(&run);
    ivs_ms_free(&halted);
    ST_CHECK(rs_words_at(&run, rs_prog_halt, n, 3u));
    ST_CHECK(rs_words_at(&halted, rs_prog_halt, n, 60u));
    ST_REQ(ok, rs_restore_into("a halted source into a running target", &halted, &run));
    ST_REQ(ok, ((const struct ivs_machine *)run.m)->cpu.run_state == IVS_CP1610_RS_HALTED);
    ivs_ms_free(&run);
    ivs_ms_free(&halted);
  }
  /* the faulting program: a source at the fault boundary into a running target, and a running source into the faulted target */
  {
    ivs_ms_inst run;
    ivs_ms_inst faulted;
    size_t n = sizeof rs_prog_fault / sizeof rs_prog_fault[0];
    uint32_t units = 0u;
    ST_CHECK(rs_words_at(&faulted, rs_prog_fault, n, 0u));
    while (((const struct ivs_machine *)faulted.m)->cpu.run_state != IVS_CP1610_RS_FAULTED && units < 200u) {
      ST_CHECK(ivs_ms_step(faulted.m, 1u, NULL));
      units++;
    }
    ST_REQ(ok, ((const struct ivs_machine *)faulted.m)->cpu.run_state == IVS_CP1610_RS_FAULTED);
    ST_CHECK(rs_words_at(&run, rs_prog_fault, n, 3u));
    ST_REQ(ok, rs_restore_into("a faulted source into a running target", &faulted, &run));
    ST_REQ(ok, ((const struct ivs_machine *)run.m)->cpu.run_state == IVS_CP1610_RS_FAULTED);
    ivs_ms_free(&run);
    ST_CHECK(rs_words_at(&run, rs_prog_fault, n, 3u));
    ST_REQ(ok, rs_restore_into("a running source into a faulted target", &run, &faulted));
    ST_REQ(ok, ((const struct ivs_machine *)faulted.m)->cpu.run_state != IVS_CP1610_RS_FAULTED);
    ivs_ms_free(&run);
    ivs_ms_free(&faulted);
  }
  return ok ? IVS_TEST_PASS : IVS_TEST_FAIL;
}

/* ---- restore sets the phase and writes nothing that identifies the instance ---- */

/* state.restore.sets-powered (D-14, D-11): the target is OFF with its images loaded; after the restore it is POWERED, and every row of class
 * BOUND (the magic, the profile indices, the committed image structure, the three slot records) and of class IMAGE (the three image arrays)
 * is byte-identical to what it was before (read from the digest field table as data), while the STATE rows now equal the source's. The
 * restored machine then runs through the public call. A snapshot is also refused by an instance whose images are missing and by one whose
 * slots differ, and that refusal leaves the phase alone. */
static int rs_sets_powered(void)
{
  ivs_ms_inst src;
  ivs_ms_inst dst;
  ivs_info info;
  uint8_t snap[IVS_SNAPSHOT_MAX_BYTES];
  uint8_t *before;
  size_t n = 0u;
  size_t rows = ivs_machine_digest_field_count();
  size_t r;
  uint32_t classes = 0u;
  uint32_t consumed = 0u;
  int ok = 1;
  ST_CHECK(su_boot_stepped(&src, SU_BOOT_UNITS, 0x5Au) && su_snapshot(src.m, snap, sizeof snap, &n));
  ST_CHECK(su_boot_new(&dst, SU_IMG_ALL, 0, 0x33u));
  before = (uint8_t *)malloc(IVS_MACHINE_BYTES);
  ST_CHECK(before != NULL);
  memcpy(before, dst.storage, IVS_MACHINE_BYTES);
  memset(&info, 0, sizeof info);
  info.struct_size = (uint32_t)sizeof info;
  ST_REQ(ok, ivs_machine_info(dst.m, &info) == IVS_OK && info.phase == IVS_PHASE_OFF);
  ST_REQ(ok, ivs_machine_snapshot_restore(dst.m, snap, n, NULL) == IVS_OK);
  ST_REQ(ok, ivs_machine_info(dst.m, &info) == IVS_OK && info.phase == IVS_PHASE_POWERED);
  for (r = 0u; r < rows; r++) {
    const ivs_digest_field *f = ivs_machine_digest_field(r);
    size_t bytes;
    if (f == NULL || (f->snap != IVS_SNAP_BOUND && f->snap != IVS_SNAP_IMAGE)) {
      continue;
    }
    bytes = (size_t)f->width * f->count;
    classes++;
    if (memcmp(before + f->offset, (const uint8_t *)dst.storage + f->offset, bytes) != 0) {
      ivs_test_diag("the row %s (class %s) was changed by the restore", f->name, f->snap == IVS_SNAP_BOUND ? "BOUND" : "IMAGE");
      ok = 0;
    }
  }
  ST_REQ(ok, classes == 31u); /* 28 BOUND rows and 3 IMAGE rows */
  ST_REQ(ok, rs_equal_instances("the OFF target after the restore", &src, &dst));
  ST_REQ(ok, su_run_to(dst.m, ((const struct ivs_machine *)dst.m)->time_ticks + 5000u, NULL, 0u, &consumed));
  ivs_ms_free(&dst);
  /* a target without its EXEC image is refused and stays OFF */
  ST_CHECK(su_boot_new(&dst, SU_IMG_GROM | SU_IMG_CART, 0, 0x33u));
  memcpy(before, dst.storage, IVS_MACHINE_BYTES);
  ST_REQ(ok, ivs_machine_snapshot_restore(dst.m, snap, n, NULL) == IVS_E_MISSING_SYSTEM_IMAGE);
  ST_REQ(ok, memcmp(before, dst.storage, IVS_MACHINE_BYTES) == 0);
  ST_REQ(ok, ivs_machine_info(dst.m, &info) == IVS_OK && info.phase == IVS_PHASE_OFF);
  free(before);
  ivs_ms_free(&dst);
  ivs_ms_free(&src);
  return ok ? IVS_TEST_PASS : IVS_TEST_FAIL;
}

/* ---- every refusal leaves the whole instance untouched ---- */

/* state.restore.atomic-every-refusal (D-15, D-16, T-08-31): the refusal matrices of this file, every row of every family and the argument
 * rows, are run against POISONED non-trivial targets (their STATE and PAD rows hold 0xA5, so a write of any kind is seen at once): after
 * every refusal the whole target instance is byte-identical to what it was and the nine digests are unchanged. This is the id that the
 * fault RESTORE_MUTATES_BEFORE_VALIDATE must fail: the matrices hold rows that only a STIC validator refuses, and a restore that commits
 * the CPU rows first has changed the target when it gets there.
 * Plan 09-02, trim (c): the nine digests are computed on row 0, every 8th row and the last row of each family (digest mode 2), and the two
 * assertions that make the property hold are untouched by that sampling - the whole-instance comparison of every row at
 * tests/state/state_util.c:479 (`memcmp(x->storage, u->ref[w->tsel], IVS_MACHINE_BYTES)`, lines 476-480 before this plan's comments) and the
 * `produced` liveness accumulation at tests/state/state_util.c:496 (`u->produced |= 1u << (w->reason - 58u)`, line 491 before), both
 * outside the digest branch. */
static int rs_atomic_every_refusal(void)
{
  uint32_t bad = 0u;
  size_t i;
  uint32_t produced = 0u;
  for (i = 0u; i < RS_FAMILIES; i++) {
    bad += rs_run_family(&rs_families[i], 1, 2, &produced);
  }
  if (!rs_args(&produced, 1)) {
    bad++;
  }
  return bad == 0u ? IVS_TEST_PASS : IVS_TEST_FAIL;
}

/* ---- the digest fields of the header are informational ---- */

/* state.restore.digest-fields-informational (D-41): the eight digest fields of the header (the composite and the seven section digests) are
 * not authenticated by restore. A snapshot whose eight fields were flipped and whose trailer was recomputed is accepted; afterwards the
 * target's real digests equal the SAVED instance's real digests, not the flipped ones the header holds; and a snapshot of the target is the
 * original snapshot, which differs from the accepted bytes in exactly those eight fields and the trailer (the one exception of the
 * canonical re-encode rule of the corruption family). The same holds with a single field flipped. */
static int rs_digest_fields_informational(void)
{
  ivs_ms_inst src;
  ivs_ms_inst dst;
  uint8_t snap[IVS_SNAPSHOT_MAX_BYTES];
  uint8_t bent[IVS_SNAPSHOT_MAX_BYTES];
  uint8_t again[IVS_SNAPSHOT_MAX_BYTES];
  uint8_t real[9][32];
  uint8_t got[9][32];
  size_t n = 0u;
  size_t n2 = 0u;
  size_t i;
  uint32_t e;
  int ok = 1;
  ST_CHECK(su_boot_stepped(&src, SU_BOOT_UNITS, 0x5Au) && su_snapshot(src.m, snap, sizeof snap, &n));
  ST_CHECK(su_nine_digests(src.m, real));
  for (e = 0u; e < 2u; e++) {
    ST_CHECK(su_boot_stepped(&dst, 250u, 0x33u));
    memcpy(bent, snap, n);
    if (e == 0u) {
      for (i = 0u; i < 32u; i++) {
        bent[272u + i] ^= 0xFFu; /* the composite */
      }
      for (i = 0u; i < ST_SECTIONS; i++) {
        uint32_t j;
        for (j = 0u; j < 32u; j++) {
          bent[304u + 44u * (uint32_t)i + 12u + j] ^= 0xFFu; /* the digest of section i */
        }
      }
    } else {
      bent[304u + 44u * 1u + 12u + 7u] ^= 0x01u; /* one byte of the MEM section digest only */
    }
    su_reseal(bent, n);
    ST_REQ(ok, ivs_machine_snapshot_restore(dst.m, bent, n, NULL) == IVS_OK);
    ST_REQ(ok, su_nine_digests(dst.m, got));
    ST_REQ(ok, memcmp(got, real, sizeof got) == 0); /* the real digests of the source */
    ST_REQ(ok, memcmp(got[0], bent + 272u, 32u) != 0 || e == 1u); /* and not the composite the header says */
    for (i = 0u; i < ST_SECTIONS && e == 0u; i++) {
      uint32_t dom = st_section_domain[i];
      ST_REQ(ok, memcmp(got[dom <= 4u ? dom : (dom == 16u ? 5u : (dom == 17u ? 6u : (dom == 18u ? 7u : 8u)))], bent + 304u + 44u * i + 12u, 32u) != 0);
    }
    ST_REQ(ok, su_snapshot(dst.m, again, sizeof again, &n2) && n2 == n && memcmp(again, snap, n) == 0);
    /* the accepted bytes and the re-encode differ in the digest fields and the trailer only */
    for (i = 0u; i < n; i++) {
      int in_digest = (i >= 272u && i < 304u);
      uint32_t j;
      for (j = 0u; j < ST_SECTIONS; j++) {
        if (i >= 304u + 44u * j + 12u && i < 304u + 44u * j + 44u) {
          in_digest = 1;
        }
      }
      if (bent[i] != again[i] && !in_digest && i < n - 32u) {
        ivs_test_diag("byte %u differs outside the digest fields", (unsigned)i);
        ok = 0;
        break;
      }
    }
    ivs_ms_free(&dst);
  }
  ivs_ms_free(&src);
  return ok ? IVS_TEST_PASS : IVS_TEST_FAIL;
}

/* ---- the judge: the matrix runner and the atomicity oracle can fail ---- */

/* state.judge.refusal-expectation-corrupt: the runner of the restore matrices can fail. For every family and for three rows of it (the
 * first, a middle one and the last), one expectation is corrupted in a copy of the row in turn (the byte offset, the section index, the
 * reason to one with another status) and the runner must report exactly that row: an unchanged row passes, a corrupted one fails. And the
 * atomicity oracle itself can fail: a byte of the target instance is changed behind the runner's back and the next row must report it. */
static int rs_judge_refusal(void)
{
  size_t fi;
  int ok = 1;
  if (!rs_base_get()) {
    return IVS_TEST_FAIL;
  }
  for (fi = 0u; fi < RS_FAMILIES; fi++) {
    su_tab t;
    uint32_t pick;
    if (!su_tab_init(&t, rs_families[fi].cap)) {
      return IVS_TEST_FAIL;
    }
    rs_families[fi].build(&t);
    for (pick = 0u; pick < 3u; pick++) {
      size_t k = pick == 0u ? 0u : (pick == 1u ? t.n / 2u : t.n - 1u);
      uint32_t kind;
      for (kind = 0u; kind < 4u; kind++) {
        su_tab one;
        su_run u;
        uint32_t bad;
        if (!su_tab_init(&one, 1u)) {
          su_tab_free(&t);
          return IVS_TEST_FAIL;
        }
        one.row[0] = t.row[k];
        one.n = 1u;
        switch (kind) {
        case 1u:
          one.row[0].off += 1u;
          break;
        case 2u:
          one.row[0].sec ^= 1u;
          break;
        case 3u:
          one.row[0].reason = one.row[0].reason == 62u ? 63u : 62u;
          break;
        default:
          break;
        }
        su_run_init(&u, rs_snap, rs_snap_n, 0);
        u.digests = 0;
        u.quiet = 1;
        bad = su_run_rows(&u, &one);
        if ((kind == 0u && bad != 0u) || (kind != 0u && bad != 1u)) {
          ivs_test_diag("family %s row %u (%s), corruption %u: the runner reports %u failing rows", rs_families[fi].name, (unsigned)k,
                        t.row[k].name, (unsigned)kind, (unsigned)bad);
          ok = 0;
        }
        su_run_free(&u);
        su_tab_free(&one);
      }
    }
    su_tab_free(&t);
  }
  /* the atomicity oracle: change a byte of the target between two rows */
  {
    su_tab full;
    su_tab one;
    su_run u;
    uint32_t bad;
    ST_CHECK(su_tab_init(&full, 64u) && su_tab_init(&one, 1u));
    rs_build_structure(&full);
    one.row[0] = full.row[0];
    one.n = 1u;
    su_run_init(&u, rs_snap, rs_snap_n, 0);
    u.digests = 1;
    u.quiet = 1;
    bad = su_run_rows(&u, &one);
    ST_REQ(ok, bad == 0u);
    ((uint8_t *)u.target[one.row[0].tsel].storage)[IVS_MACHINE_BYTES / 2u] ^= 0xFFu;
    bad = su_run_rows(&u, &one);
    ST_REQ(ok, bad == 1u);
    su_run_free(&u);
    su_tab_free(&one);
    su_tab_free(&full);
  }
  return ok ? IVS_TEST_PASS : IVS_TEST_FAIL;
}

const ivs_test_case ivs_state_restore_cases[] = {
    {"state.restore.roundtrip-fixed-point", rs_roundtrip_fixed_point, NULL},
    {"state.restore.refuse-args", rs_refuse_args, NULL},
    {"state.restore.refuse-structure", rs_refuse_structure, NULL},
    {"state.restore.refuse-identity-profile", rs_refuse_identity_profile, NULL},
    {"state.restore.refuse-identity-slots", rs_refuse_identity_slots, NULL},
    {"state.restore.refuse-content-validators", rs_refuse_content_validators, NULL},
    {"state.restore.refuse-content-invariants", rs_refuse_content_invariants, NULL},
    {"state.restore.profile-id-seam", rs_profile_id_seam, NULL},
    {"state.restore.reasons-covered", rs_reasons_covered, NULL},
    {"state.restore.poisoned-target-equals-source", rs_poisoned_target, NULL},
    {"state.restore.same-profile-any-instance", rs_same_profile_any_instance, NULL},
    {"state.restore.sets-powered", rs_sets_powered, NULL},
    {"state.restore.atomic-every-refusal", rs_atomic_every_refusal, NULL},
    {"state.restore.digest-fields-informational", rs_digest_fields_informational, NULL},
    {"state.judge.refusal-expectation-corrupt", rs_judge_refusal, NULL},
};
const size_t ivs_state_restore_count = sizeof ivs_state_restore_cases / sizeof ivs_state_restore_cases[0];

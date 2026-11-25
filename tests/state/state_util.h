/* SPDX-License-Identifier: MIT */
/* Suite state, shared helpers (plan 08-13, D-16): the instances, the poison, the oracles and the row machinery that the restore, cut,
 * sweep and fuzz families of the suite share. Nothing here calls the restore code to find out what it should do: the expected values come
 * from the specification (docs/formats/ivsn-v1.md), from the literals of data/expected/ivsn-literals.ids and from the digest field table
 * read as data (the position of a row in a section is the table's own order, an independent walk from the one the restore code makes). */
#ifndef IVS_STATE_UTIL_H
#define IVS_STATE_UTIL_H

#include <stddef.h>
#include <stdint.h>

#include "state_cases.h"

/* ---- instances of the stand-in boot program ---- */

/* The stand-in boot program (the same 23 decles that state_save.c derives: the four PSG writes of program TONE, then the RAM loop of
 * program A at $1000), the images it is run with (EXEC from these words, the GROM of seed 0 and the cartridge of 16 words at $5000) and
 * the number of units of the stepped run that the stepped-snapshot-sha256 literal pins. */
#define SU_BOOT_WORDS 23u
#define SU_BOOT_UNITS 5000u
extern const uint16_t su_boot_words[SU_BOOT_WORDS];

/* The image slots a target holds, as bits: EXEC 1, GROM 2, cartridge 4. */
#define SU_IMG_EXEC 1u
#define SU_IMG_GROM 2u
#define SU_IMG_CART 4u
#define SU_IMG_ALL 7u

/* A new instance, its storage poisoned with poison before create, with the images named by images loaded (the images of the boot
 * program), and a power-on reset when power is not 0 (which needs EXEC and GROM). 1 on success; the instance is freed on failure. */
int su_boot_new(ivs_ms_inst *x, uint32_t images, int power, uint8_t poison);

/* su_boot_new with all three images, powered, and then units units stepped through the seam. */
int su_boot_stepped(ivs_ms_inst *x, uint32_t units, uint8_t poison);

/* ---- the scenario programs (the build-tree images of tools/pack, ivs_state_images.inc) ----
 * A powered instance for the programs the cut and sweep families run: the project boot stub (contract 2, or contract 1 for the first
 * STIC diagnostic) as the EXEC image, the zero graphics ROM the scenarios use and the program's own cartridge, with the input events of the
 * scenario's `input` lines. Nothing here is committed: the images are the bytes tools/pack wrote into the build tree. */
#define SU_SC_LAMP_IDLE 0u   /* Lamp Tone, no input */
#define SU_SC_LAMP_PASS 1u   /* Lamp Tone with the six events of demo-lamp-tone-pass */
#define SU_SC_STUB2 2u       /* the boot stub contract 2 alone, no cartridge */
#define SU_SC_STIC1_FGBG 3u  /* the first STIC diagnostic, no input */
#define SU_SC_STIC1_CSTACK 4u /* the first STIC diagnostic, KEY_1 from tick 0 */
#define SU_SC_STIC2_IDLE 5u  /* the second STIC diagnostic, no selector */
#define SU_SC_STIC2_MOB 6u   /* the second STIC diagnostic, selector KEY_1 .. KEY_5 (borders, delays, collision) */
#define SU_SC_STIC2_BORDER 7u
#define SU_SC_STIC2_HDELAY 8u
#define SU_SC_STIC2_VDELAY 9u
#define SU_SC_STIC2_COLLISION 10u
#define SU_SC_COUNT 11u

const char *su_scenario_name(uint32_t sc);
/* The events of the scenario (NULL and 0 for none). */
const ivs_input_event *su_scenario_events(uint32_t sc, uint32_t *count);
/* A new instance with the scenario's images loaded, and a power-on reset when power is not 0. 1 on success; freed on failure. */
int su_scenario_new(ivs_ms_inst *x, uint32_t sc, int power);
/* Runs one unit boundary through the public run call (target one tick past machine time, buffers not requested), passing the events
 * from index *consumed on and advancing *consumed by the number the call applied: the way a host that resubmits its events from the
 * applied-event count steps a machine. 1 when the call returned OK. */
int su_run_unit(ivs_machine *m, const ivs_input_event *ev, uint32_t n, uint32_t *consumed);
/* Runs the public call to target ticks (no buffers) with the events from *consumed on; 1 when it returned OK. */
int su_run_to(ivs_machine *m, uint64_t target, const ivs_input_event *ev, uint32_t n, uint32_t *consumed);

/* ---- oracles ---- */

/* The nine digests of an instance (the composite and the eight domains), 9 x 32 bytes, by the public call. 1 on success. */
int su_nine_digests(const ivs_machine *m, uint8_t out[9][32]);

/* Fills every row of the instance that a restore writes or zeroes, the STATE rows and the PAD rows, with the byte 0xA5, using only the
 * offset, width and count of the digest field table (the table is data here, never the restore code). The BOUND rows (the magic, the
 * profile indices, the committed image structure, the slot records) and the IMAGE rows (the three image arrays) are left as they are, so
 * the instance stays a valid handle of the same images. A restore that misses any row leaves 0xA5 behind. */
void state_poison(ivs_machine *m);
/* The same with another byte (two different poisons expose a row whose source bytes happen to equal one of them). */
void su_poison(ivs_machine *m, uint8_t byte);

/* The snapshot of m into buf (cap bytes), its length in *n. 1 when the save returned OK. */
int su_snapshot(const ivs_machine *m, uint8_t *buf, size_t cap, size_t *n);

/* Recomputes the SHA-256 trailer of the n bytes of a (possibly mutated) snapshot in place. */
void su_reseal(uint8_t *snap, size_t n);

/* Big-endian read and write of width bytes (1, 2, 4 or 8). */
uint64_t su_get(const uint8_t *p, uint32_t width);
void su_put(uint8_t *p, uint64_t v, uint32_t width);

/* A heap copy of exactly len bytes of src (a fault build adds a zeroed tail of 4,400 bytes so that a reader that trusts a claimed
 * length reads defined bytes and fails by its status); at least one byte is allocated, so the pointer is never NULL. NULL when malloc
 * fails. The caller frees it. */
uint8_t *su_heap_copy(const uint8_t *src, size_t len);

/* Where a row of the digest field table lies in a snapshot of the first profile, found by walking the table's STATE rows in order (7 bytes
 * of section frame, then per row a 4 byte count when its count is above 1, then width times count) from the section offsets of the
 * literals: the section index, the offset in the container of the first byte of the row's first element (*value_off) and of its count
 * prefix (*count_off, 0 for a scalar row). 1 when the row exists and is a STATE row. */
int su_row_pos(const char *row, uint32_t *section, uint32_t *value_off, uint32_t *count_off);

/* The container offset of section i's first byte (the frame), from the literals. */
uint32_t su_section_offset(uint32_t i);

/* ---- the refusal rows of the restore family ---- */

/* The status each reason of 58 to 81 travels with (the table of "Refusal reasons" of the specification); IVS_OK for any other number. */
ivs_status su_status_of(uint32_t reason);

/* A row is a mutation of a copy of a base snapshot and the outcome restore must give: patches apply in order (op 1 flips the bits of v,
 * op 2 sets the byte to v); reseal recomputes the trailer afterwards so that the row reaches the check it is about; trunc supplies only
 * that many bytes; big_len claims that length for a one-byte buffer (the reader must refuse before it reads); tsel picks the target the
 * restore is aimed at (SU_T_*). reason 0 would be a success and is not used by the refusal tables. The expected report is the one the
 * contract of ivs/ivs_state.h allows: a refusal writes reason, byte_offset, section_index and the header facts the supplied length let
 * the reader see (kind too for a slot reason), and keeps every other byte of the caller's struct. */
#define SU_PATCHES 48u
typedef struct su_patch {
  uint16_t off;
  uint8_t op;
  uint8_t v;
} su_patch;

#define SU_T_FULL 0u    /* powered, EXEC, GROM and cartridge loaded */
#define SU_T_NOCART 1u  /* powered, EXEC and GROM loaded */
#define SU_T_NOEXEC 2u  /* OFF, GROM and cartridge loaded */
#define SU_T_NOGROM 3u  /* OFF, EXEC and cartridge loaded */
#define SU_T_COUNT 4u

typedef struct su_row {
  char name[80];
  uint8_t reseal;
  uint8_t np;
  uint8_t tsel;
  uint32_t trunc;
  uint64_t big_len;
  uint32_t reason;
  uint64_t off;
  uint32_t sec;
  uint32_t kind;
  su_patch p[SU_PATCHES];
} su_row;

typedef struct su_tab {
  su_row *row;
  size_t n;
  size_t cap;
  int bad;
} su_tab;

int su_tab_init(su_tab *t, size_t cap);
void su_tab_free(su_tab *t);
/* Adds a row (printf style name); the section index starts at IVS_SEGMENT_NONE. */
su_row *su_add(su_tab *t, const char *fmt, ...);
void su_xor(su_tab *t, su_row *r, uint32_t off, uint8_t m);
void su_set(su_tab *t, su_row *r, uint32_t off, uint8_t v);
/* Sets width bytes at off to the big-endian value v. */
void su_setn(su_tab *t, su_row *r, uint32_t off, uint32_t width, uint64_t v);
/* The outcome of the row. */
void su_expect(su_row *r, uint32_t reason, uint64_t off, uint32_t sec, uint32_t kind);

/* A caller struct larger than the v1 size, so that the tail past 360 bytes shows a write past the v1 report, and the report the contract allows
 * after a refusal: exp (SU_CALLER bytes, the report as the caller gave it) is edited so that only reason, byte_offset, section_index and
 * kind (when not 0) change, plus the header facts that readable bytes of src let the reader see (the format version from 12 bytes, the
 * total from 16, the schema version from 24). */
#define SU_CALLER 400u
typedef union su_rep {
  ivs_snapshot_report r;
  uint64_t align;
  uint8_t b[SU_CALLER];
} su_rep;
void su_expect_report(uint8_t *exp, const uint8_t *src, size_t readable, uint32_t reason, uint64_t off, uint32_t sec, uint32_t kind);

/* A new instance in the storage the caller gives (IVS_MACHINE_BYTES, 16 aligned), with the images named by images loaded and a
 * power-on reset when power is not 0, as su_boot_new makes it. 1 on success. */
int su_boot_at(void *storage, uint32_t images, int power, ivs_machine **m);

/* The state of a family run: the base snapshot the rows mutate, the four targets (built on first use, in the state the matrices expect:
 * poisoned or not), and the counters. */
typedef struct su_run {
  const uint8_t *base;
  size_t base_n;
  int poisoned; /* 1: every target gets its STATE and PAD rows poisoned (state_poison) before its first use */
  int quiet;    /* 1: no diagnostics (the judge case runs corrupted expectations) */
  int digests;  /* 1: the nine digests of the target are compared before and after each row, besides the whole instance */
  int digest_sampled; /* 1: the nine digests are compared on row 0, every 8th row and the last row of the table, and nowhere else
                       * (plan 09-02, trim (c)).  This is a second flag and not a third value of digests so that every existing
                       * setter keeps the every-row meaning.  The whole-instance comparison of every row is outside both flags and
                       * therefore cannot be switched off by either: see the memcmp in su_call_row, which is not inside the branch. */
  ivs_ms_inst target[SU_T_COUNT];
  uint8_t *ref[SU_T_COUNT]; /* the bytes of each target before the first row, to prove none was changed */
  uint8_t dig[SU_T_COUNT][9][32];
  int have[SU_T_COUNT];
  uint32_t produced; /* bit (reason - 58) of every reason a row produced exactly as expected */
} su_run;

/* Builds the run's base reference; the targets are built lazily. su_run_free releases everything. */
void su_run_init(su_run *u, const uint8_t *base, size_t base_n, int poisoned);
void su_run_free(su_run *u);

/* Runs every row of t: for each row, two calls of ivs_machine_snapshot_restore into a report poisoned 0xA5 and then 0x5A (a caller
 * struct of 400 bytes) against the target of the row; the status, the whole report, the supplied bytes and the whole target instance
 * (and, with u->digests, its nine digests on every row; with u->digest_sampled, on row 0, every 8th row and the last row) are compared
 * with what the contract allows. Returns the number of rows that failed. */
uint32_t su_run_rows(su_run *u, const su_tab *t);


/* ---- plan 08-14, D-16, D-17: the hard-point cuts, their witnesses and the comparison oracle ---- */

#define SC_PENDING UINT64_C(18446744073709551615) /* the pinned tick of a cut that is not pinned yet */
#define SC_NO_SCENARIO 0xFFFFFFFFu
#define SC_FRAME 59736u /* one frame of the profile in master ticks (14,934 CPU cycles of 4 ticks) */

/* A program of the cuts (state_progs.c): the `program` column of data/expected/state-cuts.ids. A stand-in program is a list of decles at
 * $1000 (docs/state/hard-points.md); a scenario run is one of the SU_SC_* builders with its input events. */
typedef struct sc_run {
  const char *id;
  uint32_t scenario; /* SU_SC_* or SC_NO_SCENARIO */
  const uint16_t *words;
  size_t nwords;
} sc_run;
const sc_run *sc_run_find(const char *id);
/* A powered instance of the run, stepped no unit (1 on success; freed on failure). power must be nonzero for a stand-in program. */
int sc_new(ivs_ms_inst *x, const sc_run *r, int power);
/* The input events of the run (NULL and 0 for none). */
const ivs_input_event *sc_events(const sc_run *r, uint32_t *count);

/* One line of data/expected/state-cuts.ids through the generated include; the tick is SC_PENDING until it is pinned. */
typedef struct sc_cut {
  const char *id;
  const char *program;
  uint64_t tick;
  const char *family;
} sc_cut;
size_t sc_cut_count(void);
const sc_cut *sc_cut_at(size_t i);
const sc_cut *sc_cut_find(const char *id);
/* The end tick E of the uninterrupted reference of a cut (the arithmetic is in state_progs.c). */
uint64_t sc_cut_end(const sc_cut *c);
/* The occurrence rule of a cut in words, for the failure texts (state_witness.c). */
const char *sc_cut_rule(const sc_cut *c);

/* The witness scan (state_witness.c, H17): steps a powered instance of the cut's program one unit at a time (through the internal seam
 * when the program has no input events, through the public run call with the events resubmitted otherwise) and finds the first unit
 * boundary at or after the occurrence rule of the cut at which the predicate of docs/state/hard-points.md holds. Returns 1 and the tick
 * and the number of units stepped, and (when storage is not NULL) copies the IVS_MACHINE_BYTES of the instance at that boundary there;
 * 0 when the predicate never holds before the end tick of the cut. */
int sc_witness_find(const sc_cut *c, uint64_t *tick, uint64_t *units, uint8_t *storage);
/* The part of the predicate that is a function of the instance alone (the whole predicate when it needs nothing from the unit just run):
 * 1 holds, 0 does not hold. */
int sc_witness_state_holds(const sc_cut *c, const ivs_machine *m);

/* One leg of a run: a call of the public run API with fresh video, audio and PSG log buffers sized from ivs_info, and what it reported. */
typedef struct sc_leg {
  uint8_t *video;
  uint8_t *audio;
  uint8_t *log;
  uint64_t vbytes, abytes, lbytes; /* bytes delivered */
  uint64_t vfirst, afirst, lfirst; /* the first-index fields of the report */
  uint64_t frames;                 /* video frames completed */
  uint64_t now, units, elapsed;
  uint32_t stop;
  uint32_t consumed; /* events applied by the call */
  uint32_t ctrl[4];  /* controller_now of the report: the words in force at exit */
  uint64_t floor;    /* input_floor_tick of the report */
  ivs_status st;
} sc_leg;
/* Runs m to target ticks with the events from index consumed_in on; the buffers are allocated here (the capacity rule of the run call). 1 when
 * the call returned OK (the leg is filled either way); sc_leg_free releases the buffers. */
int sc_run_leg(ivs_machine *m, uint64_t target, const ivs_input_event *ev, uint32_t nev, uint32_t consumed_in, sc_leg *out);
void sc_leg_free(sc_leg *l);

/* The three public reports of an instance (CPU state, STIC state, info), each with its struct_size set. 1 when all three calls returned OK. */
int sc_reports(const ivs_machine *m, ivs_cpu_state_report *cpu, ivs_stic_report *stic, ivs_info *info);

/* The uninterrupted reference of a run to an end tick (kept in a small cache; sc_ref_free releases it). */
typedef struct sc_ref {
  const sc_run *run;
  uint64_t end;
  ivs_ms_inst x;
  sc_leg leg;
  uint8_t dig[9][32];
  ivs_cpu_state_report cpu;
  ivs_stic_report stic;
  ivs_info info;
} sc_ref;
const sc_ref *sc_ref_get(const sc_run *run, uint64_t end);
void sc_ref_free(void);

/* The end tick a test of the cut may use: E, or, under IVS_STATE_FAULT_RUN for an id that is not a must-fail id (full == 0), the end of the
 * frame the cut lies in but never below the cut plus one tick (a one-frame window, D-40). */
uint64_t sc_window_end(uint64_t tick, uint64_t end, int full);

/* The comparison oracle of a cut (D-16, 08-DISCUSS-state 5.3): REF is the uninterrupted run of the program from a fresh instance to the end
 * tick; SUB runs to the cut, saves, restores into a target that was poisoned (every STATE and PAD row 0xA5, state_poison) and compares at
 * once the whole target instance and all nine digests with the source; it continues to the end tick with fresh buffers and the events
 * resubmitted from the applied count, and compares the concatenated video, audio and PSG log streams, the first-index fields, the final
 * instance bytes, the nine digests and the public reports (CPU state, STIC state, info) with REF. The result is a bit mask of what differed
 * (0 is a pass); tamper changes the target on purpose, so that a test can show the comparisons can fail (SC_T_*). */
#define SC_F_IMM_MEM 0x001u  /* the whole target differs from the source right after the restore */
#define SC_F_IMM_DIG 0x002u  /* a digest of the target differs from the source right after the restore */
#define SC_F_VIDEO 0x004u    /* the concatenated video stream differs from REF */
#define SC_F_AUDIO 0x008u
#define SC_F_LOG 0x010u
#define SC_F_FIRST 0x020u    /* the first-index fields of leg 2 do not continue leg 1 */
#define SC_F_FINAL_MEM 0x040u
#define SC_F_FINAL_DIG 0x080u
#define SC_F_REPORTS 0x100u
#define SC_F_RUN 0x200u      /* a call refused, or the run report (time, units, stop reason, events applied) differs from REF */
#define SC_F_SETUP 0x400u    /* the comparison could not be set up (allocation, snapshot, restore refused, a leg that missed the cut) */
#define SC_T_NONE 0u
#define SC_T_ROW 1u    /* the target: one byte of the STATE row time_ticks changed after the restore */
#define SC_T_STREAM 2u /* leg 2: one byte of the video stream flipped */
typedef struct sc_cmp_info {
  uint64_t tick, end;
  uint64_t units1, units2;
  uint64_t vbytes1, vbytes2, abytes1, abytes2, lbytes1, lbytes2;
  uint32_t consumed1;
  uint32_t slot_bytes;     /* video_slot_bytes of ivs_info */
  uint32_t mob, coll;      /* the OR of the STIC register words 0x00 to 0x17 and 0x18 to 0x1F of the source at the cut */
  uint32_t pulldown;       /* the OR of the two port pull-down masks of the source at the cut */
  uint8_t snap_sha[32];
} sc_cmp_info;
/* label names the case in the diagnostics; quiet suppresses them (the judge runs corrupted comparisons). */
uint32_t sc_cut_compare(const char *label, const sc_run *run, uint64_t tick, uint64_t end, uint32_t tamper, int quiet, sc_cmp_info *info);

#endif

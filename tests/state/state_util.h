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
 * (and, with u->digests, its nine digests) are compared with what the contract allows. Returns the number of rows that failed. */
uint32_t su_run_rows(su_run *u, const su_tab *t);

#endif

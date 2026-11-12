/* SPDX-License-Identifier: MIT */
/* Machine instance (D-01, D-02, D-08, D-35, plan 03-09): internal layout and seams. Not
 * installed and not an ABI: it lives in core/src, which only the core and the tests
 * (ivs_core_internal) see. The public handle type ivs_machine (ivs/ivs_machine.h) is this struct.
 *
 * One flat struct with no pointers: magic, profile index, run phase, a reserved word, machine
 * time, the embedded 32-byte CPU state, the memory system (gates, loaded flags, the committed
 * cartridge segment table, every backing array at the profile's worst-case capacity and the PSG
 * registers and port pull-down masks), the PSG device state (plan 05-03, D-16), the observation
 * rows (plan 04-05, D-10: the video, audio and input digest domains 16..18; plan 05-03, D-18:
 * the PSG write log, domain 19) and one record per image slot. Every field is fixed width; the
 * only bytes that are not fields are the reserved words and pads (the instance's, the CPU's, the
 * memory system's, the PSG state's and the audio row's), and all of them stay zero. The instance
 * holds no last-error or timestamp (D-08); the observation counts are machine state of their
 * domains.
 *
 * Layout derivation (bytes; offsets from the start of the instance):
 *   header   24   magic, profile, phase, reserved (4 each) and time_ticks (8)         0 ..    24
 *   cpu      32                                                                      24 ..    56
 *   mem  113,264  Phase 4: 113,228; plus psg_reg[16] u16 (32), port_pulldown[2] u8 (2)
 *                 and psg_reserved[2] (2): +36                                       56 .. 113,320
 *   psg       72  ivs_psg_state: next_sample u64 (8), 13 u32 state words (52) and
 *                 reserved[3] u32 (12)                                          113,320 .. 113,392
 *   obs      200  video 40 (chain 32, count 8), audio 56 (chain 32, count 8,
 *                 emitted 8, prev_levels 4, reserved 4), input 64, psglog 40
 *                 (chain 32, count 8)                                             113,392 .. 113,592
 *   slots    168  3 x 56                                                         113,592 .. 113,760
 * Total 113,760, a multiple of 16 (7,110 x 16), so there is no tail padding. mem now ends at
 * 56 + 113,264 = 113,320, which is 0 mod 8: the Phase 4 obs_pad word is gone and obs (8-aligned)
 * follows the PSG state directly (the 72-byte PSG state is what brings the total back to a
 * multiple of 16: with 64 bytes it would end at 113,752, 8 mod 16). The previous layout (Phase 4)
 * was 113,600; Phase 3 (no obs) was 113,456 with 4 bytes of tail padding. */
#ifndef IVS_MACHINE_INTERNAL_H
#define IVS_MACHINE_INTERNAL_H

#include <stddef.h>
#include <stdint.h>

#include "content/ivsc.h"
#include "cpu/cp1610.h"
#include "ivs/ivs_machine.h"
#include "ivs/ivs_run.h"
#include "memmap/memsys.h"
#include "psg/psg.h"

#define IVS_MACHINE_MAGIC 0x4956534Du /* "IVSM" */
#define IVS_MACHINE_ALIGN 16u
#define IVS_MACHINE_BYTES 113760u

/* IVS_INFO_V1_SIZE, the smallest struct_size info accepts, is public (ivs/ivs_machine.h, plan
 * 05-02): the installed consumer pins it from the installed headers. */

/* One image slot (kind fixed at create: slot i holds kind i + 1). */
typedef struct ivs_machine_slot {
  uint32_t kind;
  uint32_t loaded; /* 0 or 1 */
  uint32_t length_bytes;
  uint32_t length_units;
  uint32_t unit_bits;
  uint32_t base_addr;
  uint8_t sha256[32];
} ivs_machine_slot;

/* Observation rows (plan 04-05, D-10, A-03; plan 05-03, D-18): a chain[32] and a u64 record count
 * for VIDEO (digest domain 16) and PSGLOG (19); AUDIO (17) adds the emitted-sample count and the
 * levels of the last emitted sample (the chain is over level-change records, plan 05-06); INPUT
 * (18) holds the controller words in force, the applied event count, the input floor tick and
 * the input chain (plan 04-11, D-15). Every row is zero at create and after a reset except the
 * INPUT chain, which then holds its start value h0 (ivs_input_init). Domain version 1; not part
 * of the composite, which is version 2 over domains 1..4. */
typedef struct ivs_machine_obs_av {
  uint8_t chain[32];
  uint64_t count;
} ivs_machine_obs_av;

typedef struct ivs_machine_obs_audio {
  uint8_t chain[32];
  uint64_t count;      /* level-change records chained */
  uint64_t emitted;    /* samples emitted (with or without a buffer) */
  uint32_t prev_levels; /* levels of the last emitted sample, channels A, B, C in bytes 0..2 */
  uint32_t reserved;    /* 0 (PAD row) */
} ivs_machine_obs_audio;

typedef struct ivs_machine_obs_input {
  uint32_t words[4];
  uint64_t count;
  uint64_t floor_tick;
  uint8_t chain[32];
} ivs_machine_obs_input;

typedef struct ivs_machine_obs {
  ivs_machine_obs_av video;
  ivs_machine_obs_audio audio;
  ivs_machine_obs_input input;
  ivs_machine_obs_av psglog;
} ivs_machine_obs;

struct ivs_machine {
  _Alignas(16) uint32_t magic; /* IVS_MACHINE_MAGIC while valid; 0 after destroy */
  uint32_t profile;            /* registry index */
  uint32_t phase;              /* IVS_PHASE_* */
  uint32_t reserved;           /* 0 */
  uint64_t time_ticks;         /* machine time in master ticks */
  ivs_cp1610 cpu;
  ivs_memsys mem;
  ivs_psg_state psg;   /* before obs and slot[] (plan 05-03, D-16): O(1) PSG device state */
  ivs_machine_obs obs; /* before slot[]: IVS_FAULT_DIGEST_SKIPS_IMAGE_TABLE skips rows from slot on */
  ivs_machine_slot slot[IVS_SLOT_COUNT];
};

_Static_assert(sizeof(ivs_machine_slot) == 56u, "ivs_machine_slot must be 56 bytes");
_Static_assert(sizeof(ivs_machine_obs_audio) == 56u, "ivs_machine_obs_audio must be 56 bytes");
_Static_assert(sizeof(ivs_machine_obs) == 200u, "ivs_machine_obs must be 200 bytes");
_Static_assert(offsetof(struct ivs_machine, mem) == 56u, "mem starts at 56");
_Static_assert(offsetof(struct ivs_machine, psg) == 113320u, "psg state starts at 113,320");
_Static_assert(offsetof(struct ivs_machine, obs) == 113392u, "obs follows the 72-byte PSG state with no hole");
_Static_assert(offsetof(struct ivs_machine, slot) == 113592u, "slot[] follows the 200-byte obs rows with no hole");
_Static_assert(_Alignof(struct ivs_machine) == IVS_MACHINE_ALIGN, "instance alignment is 16");
_Static_assert(sizeof(struct ivs_machine) == IVS_MACHINE_BYTES,
               "instance size changed: review the digest field table (machine_digest.c)");

/* ---- canonical state digest (D-35, machine_digest.c) ----
 * One field table covers every instance byte exactly once. A UINT row is count elements of width
 * bytes (1, 2, 4 or 8), each read natively and written big-endian by shifts; a row with count > 1
 * is preceded by its u32 count. A PROFILE row is a u32 registry index written as the profile id:
 * u32 length then the ASCII bytes (an index outside the registry writes u32 0xFFFFFFFF then the
 * index). MAGIC and PAD rows (the magic, the reserved word, the CPU's reserved bytes and the
 * observation pad word) are declared but never serialized; their domain names the neighbour they
 * sit with. Domain preimage: "IVSD" || u8 domain || u16 IVS_DIGEST_DOMAIN_VERSION || the domain's
 * rows in table order. Composite: "IVSSTATE" || u16 IVS_DIGEST_COMPOSITE_VERSION || for domains
 * 1..4: u8 domain || its 32-byte SHA-256. Domains 16..19 (VIDEO, AUDIO, INPUT, PSGLOG) have their
 * own digests and are not in the composite (D-10, D-18). Composite version 2 (plan 05-03,
 * deliberate, D-18): the PSG registers and pull-downs (MEM) and the PSG device state (INSTANCE)
 * joined domains 2 and 4, so guest-visible PSG state can never differ with equal composites;
 * version 1 was the Phase 4 framing over the same four domains. */
#define IVS_DIGEST_KIND_UINT 1u
#define IVS_DIGEST_KIND_PROFILE 2u
#define IVS_DIGEST_KIND_MAGIC 3u
#define IVS_DIGEST_KIND_PAD 4u
#define IVS_DIGEST_DOMAIN_VERSION 1u
#define IVS_DIGEST_COMPOSITE_VERSION 2u

typedef struct ivs_digest_field {
  const char *name;
  uint32_t offset; /* offsetof(struct ivs_machine, ...) */
  uint32_t width;  /* bytes per element */
  uint32_t count;  /* elements */
  uint8_t domain;  /* IVS_DIGEST_CPU..IVS_DIGEST_INSTANCE or IVS_DIGEST_VIDEO..IVS_DIGEST_PSGLOG */
  uint8_t kind;    /* IVS_DIGEST_KIND_* */
  uint16_t reserved;
} ivs_digest_field;

/* The field table (coverage seam for the tests). */
size_t ivs_machine_digest_field_count(void);
const ivs_digest_field *ivs_machine_digest_field(size_t i);

/* Writes the preimage of domain 1..4 or 16..19 of m into buf (at most cap bytes; buf may be NULL when cap
 * is 0) and returns its full length; 0 for NULL m or another domain. No handle check: the tests
 * pass modified copies. */
size_t ivs_machine_digest_preimage(const struct ivs_machine *m, uint32_t domain, uint8_t *buf, size_t cap);

/* Writes the public report of one slot record (shared by ivs_machine_info and the load report,
 * A-08 f). */
void ivs_machine_slot_report(const ivs_machine_slot *s, ivs_image_report *o);

/* Layer 2 span rule of a cartridge load (D-33, A-01, machine_load.c): the index of the first
 * segment of v (table order) that does not lie wholly inside the words [lo, hi], or
 * IVS_SEGMENT_NONE. A test seam as well: the registered profile's span ends at $FFFF, where the
 * parser's address-wrap check already refuses a segment that runs past it, so the span's upper
 * edge is tested here with a synthetic span. */
uint32_t ivs_machine_cart_span_check(uint32_t lo, uint32_t hi, const ivs_ivsc_view *v);

/* 1 when the n bytes at p overlap the inst_bytes bytes at inst (A-06, A-08 a). Differences of
 * uintptr_t values only, so nothing wraps whatever the pointers are; n or inst_bytes 0 is 0; a
 * span that runs past the top of the address space is 1 (review IN-02: it names no object).
 * Used for input spans (only within the kind's bound) and for every out parameter that points
 * into caller memory: an out struct or buffer that overlaps the instance storage is refused
 * BAD_ARG with nothing written, so a host cannot aim a report into its own instance. */
uint32_t ivs_machine_span_overlaps(const void *p, size_t n, const void *inst, size_t inst_bytes);

/* The out-parameter rule of create, info and state_digest (plan 03-11): 1 when the n bytes at out
 * overlap the inst_bytes bytes at inst (ivs_machine_span_overlaps), so the call refuses BAD_ARG
 * with nothing written. info, state_digest and the load report test it only against a validated
 * instance, after BAD_HANDLE (review WR-01): a NULL handle has no bytes to overlap. One function so the named fault OUT_OVERLAP_UNCHECKED (plan 03-12)
 * removes the rule from all three calls at once; the load report keeps its own check and fault
 * (REPORT_INSIDE_UNCHECKED). */
uint32_t ivs_machine_out_overlaps(const void *out, size_t n, const void *inst, size_t inst_bytes);

/* ---- machine-level bus wrapper (plan 05-04, A-01, machine_bus.c) ----
 * The CPU port of every unit the machine steps: ctx is an ivs_machine_bus_ctx built per unit in the
 * step seam (the run call steps through the seam). In plan 05-04 the wrapper forwards EVERY access
 * unchanged to ivs_memsys_bus on the instance's own memory system, which keeps the masked store of
 * the PSG region (ivs_psg_reg_store); it only reserves the place where plans 05-06 and 05-08 add the
 * catch-up of the PSG to the write's tick and the write log BEFORE delegating the store. The context
 * carries the unit start tick (machine time before the step) and the master ticks per CPU cycle
 * of the profile, from which those plans stamp a write; nothing reads them yet. ivs_memsys_bus
 * itself answers wait 0 for every kind (U and g depend on it, A-03). The wrapper never touches
 * the memory system without going through ivs_memsys_bus. */
typedef struct ivs_machine_bus_ctx {
  struct ivs_machine *m;
  uint64_t unit_start_tick;
  uint32_t ticks_per_cpu_cycle;
  uint32_t reserved; /* 0 */
} ivs_machine_bus_ctx;

ivs_bus_reply ivs_machine_bus(void *ctx, const ivs_cp1610_access *a);

/* ---- internal step seam (D-05, D-20, plan 03-12, machine_run.c) ----
 * Steps one CPU unit of a POWERED instance through its memory system: the CPU configuration
 * comes from the profile registry (cpu.interrupt_accept_clears_intff,
 * bus.release_latency_cpu_cycles, bus.wait_limit_cpu_cycles), every line is inactive (no device
 * drives INTR, INTRM, BUSRQ or STPST in Phase 3), and the port's bus is ivs_machine_bus (plan
 * 05-04), which forwards to ivs_memsys_bus on the instance's own memory system, so the IAB of the reset entry answers the profile's reset.r7.
 * No access log is kept. Machine time advances by the unit's elapsed CPU cycles times the
 * profile's ticks_per_cpu_cycle. BAD_ARG: out NULL; BAD_HANDLE; BAD_STATE: the instance is OFF;
 * else the CPU step's status (a CPU refusal leaves time unchanged). *out is written only on
 * IVS_OK. Never public: it is declared here and nowhere under core/include; Phase 4's bounded
 * run API is built on it. */
ivs_status ivs_machine_step_unit_internal(ivs_machine *m, ivs_cp1610_result *out);

/* ---- internal run env seam (D-06, plan 04-10, machine_bound.c) ----
 * The output layout a run is checked against: per channel a format (IVS_OUTPUT_FORMAT_*) and the
 * bytes one frame needs, and the nominal frame length in master ticks. The capacity a run needs
 * per channel is bytes_per_frame * (max(target - now, 0) / nominal_frame_ticks + 1); a channel
 * with 0 bytes per frame needs nothing, and nominal_frame_ticks 0 with bytes above 0 can never be
 * met (BUFFER_TOO_SMALL). */
typedef struct ivs_run_env {
  uint32_t video_format;
  uint32_t video_bytes_per_frame;
  uint32_t audio_format;
  uint32_t audio_bytes_per_frame;
  uint64_t nominal_frame_ticks;
  /* Plan 05-02 (D-14, D-19): the write-log channel, appended. The log needs
   * psg_log_bytes_per_frame * (max(target - now, 0) / nominal_frame_ticks + 1) bytes like the
   * other two channels; 0 (every profile until plan 05-08) needs nothing. The seam is an internal
   * plain struct without a struct_size: callers build it with memset, so the appended field is
   * 0 for a caller that does not know it. */
  uint32_t psg_log_bytes_per_frame;
  uint32_t reserved_env; /* 0 */
} ivs_run_env;

/* The real output layout of profile p (shared by ivs_machine_info and the public run call):
 * Phase 4 profiles produce nothing (formats NONE, 0 bytes per frame), and the nominal frame is
 * ticks_per_cpu_cycle * cpu_cycles_per_scanline * scanlines_per_frame (D-09). */
void ivs_machine_profile_env(const ivs_profile_desc *p, ivs_run_env *env);

/* ivs_machine_run against the layout env instead of the profile's (the public call passes the
 * profile's own, ivs_machine_profile_env). Same ladder, loop and report as ivs_machine_run
 * (ivs/ivs_run.h); env NULL is BAD_ARG (RUN_NULL_ARG) in the first stage. It lets the tests prove
 * the capacity contract with a synthetic layout while the real layout is zero-length. Never
 * public: it is declared here and nowhere under core/include. */
ivs_status ivs_machine_run_env_internal(ivs_machine *m, const ivs_run_request *req, ivs_run_report *rep,
                                        const ivs_run_env *env);

/* ---- input (D-12..D-15, plan 04-11, machine_input.c) ----
 * The INPUT row (obs.input): the controller words in force, the applied event count, the input
 * floor (start of the last begun unit + 1, set by the step seam when a unit begins) and the chain
 * h0 = SHA-256("IVS-INPUT-LOG-V1"), h(n+1) = SHA-256(h(n) || be64(at_tick) || be32(words[0..3])). */

/* Sets in to its create and reset state: words, count and floor 0, chain h0. */
void ivs_input_init(ivs_machine_obs_input *in);

/* The per-event stages of the run ladder (D-05, D-14), after BUFFER_TOO_SMALL: IVS_REASON_NONE
 * with *st IVS_OK and *index IVS_INPUT_INDEX_NONE, or the reason with *st and the failing index.
 * BAD_ARG pass over the whole array first (at_tick at or above 2^63, below its predecessor, below
 * in->floor_tick, in that order per event), then the UNSUPPORTED pass (nonzero flags, nonzero
 * reserved word, a controller word with IVS_CTRL_RESERVED_MASK set, a nonzero word at an index at
 * or above controllers, the profile's input.controllers). Reads ev[0..count) only. */
uint32_t ivs_input_check(const ivs_machine_obs_input *in, const ivs_input_event *ev, uint32_t count,
                         uint32_t controllers, ivs_status *st, uint32_t *index);

/* Applies, in array order, every event ev[next..count) whose at_tick is at or below start (the
 * start tick of the unit about to begin): words in force, count + 1, chain. Returns the index of
 * the first event not applied. entry is machine time at the call's entry (used only by a named
 * fault). Cannot refuse: the run validated the whole array first. */
uint32_t ivs_input_apply_due(struct ivs_machine *im, const ivs_input_event *ev, uint32_t count, uint32_t next,
                             uint64_t start, uint64_t entry);

/* The instance behind a handle, or NULL for NULL, a wrong magic, or a stored profile index that
 * is outside the registry or differs from the memory system's (review WR-03): every call that
 * takes a handle refuses such an instance BAD_HANDLE, and nothing behind it indexes a profile
 * table with an unchecked index. */
const struct ivs_machine *ivs_machine_from_handle(const ivs_machine *m);
struct ivs_machine *ivs_machine_from_handle_mut(ivs_machine *m);

/* Length of a profile id (bytes before its NUL), bounded by IVS_PROFILE_ID_MAX_BYTES; BAD_ARG for
 * NULL or no NUL in the first 64 bytes. Hand-written bounded loop, no string functions (A-06). */
ivs_status ivs_machine_profile_id_length(const char *id, uint32_t *len);

/* Registry index of a profile id: BAD_ARG as above, UNKNOWN_PROFILE for a registry miss. */
ivs_status ivs_machine_profile_lookup(const char *id, uint32_t *index);

#endif

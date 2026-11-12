/* SPDX-License-Identifier: MIT */
/* Public run, observation and input API (D-08, A-01, plan 04-06). Observation types come first:
 * a host, the runner and the installed consumer read guest state (CPU registers and memory)
 * through these calls, never through core internals.
 *
 * Observation calls are side-effect-free by contract (D-08): they take a const instance, advance
 * no machine time, cause no gate or stub side effect and read memory through the non-mutating
 * view of the memory map, so the instance storage and every state digest are the same before
 * and after the call. Both are valid while the machine is OFF and while it is POWERED.
 *
 * The struct rule of ivs/ivs_machine.h applies (D-04, A-02): an out struct starts with uint32_t
 * struct_size; a size below the v1 size, or above IVS_STRUCT_SIZE_MAX, is BAD_ARG with nothing
 * written; otherwise the core writes its own (v1) size and sets struct_size to it, and bytes of a
 * larger caller struct past that size are untouched. An out struct or buffer that overlaps the
 * instance storage is BAD_ARG with nothing written; that test runs after the handle check (a
 * NULL or invalid handle is BAD_HANDLE wherever the out parameter lies). Every refusal happens
 * before any write to the out parameter.
 * Standard headers only; constants are #defines (no enums, bool or bitfields). */
#ifndef IVS_RUN_H
#define IVS_RUN_H

#include <stdint.h>

#include "ivs/ivs_machine.h"
#include "ivs/ivs_status.h"

#ifdef __cplusplus
extern "C" {
#endif

/* CPU run states (ivs_cpu_state_report.run_state). A new instance is UNRESET; a power-on reset
 * leaves RESET_PENDING (the CPU takes its reset entry on its next unit), then RUNNING. HALTED
 * after HLT; FAULTED after a guest fault, whose reason is in fault_reason. */
#define IVS_CPU_STATE_UNRESET 0u
#define IVS_CPU_STATE_RESET_PENDING 1u
#define IVS_CPU_STATE_RUNNING 2u
#define IVS_CPU_STATE_HALTED 3u
#define IVS_CPU_STATE_FAULTED 4u

/* Sticky guest fault reasons (ivs_cpu_state_report.fault_reason): nonzero only while FAULTED. */
#define IVS_CPU_FAULT_NONE 0u
#define IVS_CPU_FAULT_UNSUPPORTED_SDBD_TARGET 1u
#define IVS_CPU_FAULT_UNSUPPORTED_MVO_SSS_EQ_MMM 2u
#define IVS_CPU_FAULT_UNSUPPORTED_JUMP_DE 3u
#define IVS_CPU_FAULT_BUS_WAIT_LIMIT 4u

/* Out: a CPU snapshot. r[6] is the stack pointer and r[7] the program counter. s, z, ov and c are
 * the sign, zero, overflow and carry flags and intr_enabled the interrupt enable, each 0 or 1.
 * v1 size 36 bytes; reserved is written 0. */
typedef struct ivs_cpu_state_report {
  uint32_t struct_size;
  uint32_t run_state;    /* IVS_CPU_STATE_* */
  uint32_t fault_reason; /* IVS_CPU_FAULT_* */
  uint16_t r[8];
  uint8_t s;
  uint8_t z;
  uint8_t ov;
  uint8_t c;
  uint8_t intr_enabled;
  uint8_t reserved[3];
} ivs_cpu_state_report;

/* Copies the CPU state into *out; the instance is not written. Refusal order: BAD_ARG (out NULL,
 * or out->struct_size below the v1 size or above IVS_STRUCT_SIZE_MAX); BAD_HANDLE; BAD_ARG (out
 * overlapping the instance storage). Nothing is written on a refusal. */
IVS_API ivs_status ivs_machine_cpu_state(const ivs_machine *m, ivs_cpu_state_report *out);

/* Reads count 16-bit words at addr, addr + 1, ... into out[0..count-1]; the caller supplies at
 * least count words at out (there is no capacity argument). Each word is what a CPU read at that
 * address returns now: decode, width mask, closed gates and undriven space included (a closed
 * gate, a stub, an empty image slot and an address no device drives read the profile's undriven
 * value). The instance is not written and machine time does not advance (D-08); valid OFF and
 * POWERED. Refusal order: BAD_ARG (out NULL or count 0); BAD_HANDLE; OUT_OF_RANGE (addr + count
 * above 0x10000; addr + count == 0x10000 is accepted); BAD_ARG (the count * 2 bytes at out
 * overlap the instance storage). Nothing is written on a refusal. */
IVS_API ivs_status ivs_machine_peek(const ivs_machine *m, uint16_t addr, uint32_t count, uint16_t *out);

/* ---- bounded execution (D-01..D-05, D-09, plan 04-07) ----
 * ivs_machine_run runs a POWERED machine to an absolute target in MASTER TICKS (docs/time-base.md,
 * D2): while machine time is below the target it steps one CPU unit, so it ends at the first
 * unit boundary at or after the target. A unit is never split, so the run may end past the
 * target by part of one unit (the overshoot, reported, never carried). The call keeps no state
 * between calls (no budget, deficit or pointer): the whole instance is the resumable state, and
 * any slicing of one target into several calls gives the same machine. run(now + 1) runs exactly
 * one unit (the single-step idiom). CPU-cycle and frame bounds are host sugar over the tick
 * target (D-09): a host converts them; the core knows only ticks.
 * Stops are not refusals: a run that executes ends with IVS_OK and a stop reason, a guest fault
 * included (a faulted CPU is never stepped again; every later call stops GUEST_FAULT at once). */

/* Run flags (ivs_run_request.flags). Any other bit is UNSUPPORTED. */
#define IVS_RUN_F_STOP_ON_HALT 1u   /* stop at the first boundary where the CPU is HALTED */
#define IVS_RUN_F_STOP_ON_FRAME 2u  /* stop at the end of a frame; accepted only once the machine publishes frame
                                     * events, UNSUPPORTED as an unknown flag until then (D-14) */

/* Largest accepted target: 2^63 master ticks; a target above it is OUT_OF_RANGE. */
#define IVS_RUN_TARGET_MAX 0x8000000000000000u

/* Stop reasons (ivs_run_report.stop_reason), append-only; 0 is not a stop reason (a refused
 * call does not write stop_reason). The reason is a pure function of the exit state, in this order: INTERNAL (a unit
 * step was refused or made no progress: a harness failure, the state is the last whole
 * boundary; for a refused step the INPUT row included, since the events applied for that unit
 * are taken back and not counted in input_consumed), GUEST_FAULT (the CPU is FAULTED), CPU_HALTED (only with IVS_RUN_F_STOP_ON_HALT: the
 * CPU is HALTED), FRAME (only with IVS_RUN_F_STOP_ON_FRAME: a frame event was reached at the
 * boundary the call ends at), TARGET (machine time reached the target). The exit-state order is
 * INTERNAL, GUEST_FAULT, CPU_HALTED (flag), FRAME (flag), TARGET. IVS_STOP_FRAME is defined from
 * plan 06-03 and is not produced by any run until the frame stop is implemented. */
#define IVS_STOP_TARGET 1u
#define IVS_STOP_CPU_HALTED 2u
#define IVS_STOP_GUEST_FAULT 3u
#define IVS_STOP_INTERNAL 4u
#define IVS_STOP_FRAME 5u

/* Output buffers (D-02, D-03, D-06, plan 04-10). Video and audio go to caller-owned buffers named
 * in the request; the core keeps no pointer after the call. The profile's output layout (ivs_info:
 * video_format, video_bytes_per_frame, audio_format, audio_bytes_per_frame, nominal_frame_ticks)
 * fixes the capacity a run needs, per channel, checked before the first unit:
 *   required = bytes_per_frame * (max(target_ticks - now, 0) / nominal_frame_ticks + 1)
 * an upper bound on the frames the run can complete, the overshoot of its last unit included. A
 * capacity below it is BUFFER_TOO_SMALL with nothing changed; a buffer may be NULL only with
 * capacity 0, and that pair means "channel not requested": no check, no byte, no state change
 * whatever the layout requires (D-17, plan 05-02; a non-NULL buffer with capacity 0 against a layout
 * that needs bytes is still BUFFER_TOO_SMALL). A third channel, the PSG register-write log
 * (psg_log, psg_log_capacity; request v2), follows video and audio with
 * psg_log_bytes_per_frame from the profile's layout. A layout with 0 bytes per frame needs nothing
 * and gets nothing (video, until Phase 6, is such a channel: no frame is fabricated). From plan 05-06 the audio layout is real: format IVS_OUTPUT_FORMAT_PSG_LEVELS_V1,
 * one 4-byte sample (level_A, level_B, level_C, 0) per PSG chip step, stamped k * audio_sample_ticks
 * master ticks on an absolute grid from reset, delivered into the audio buffer in order. The core
 * catches the chip up to the end of every unit it runs (never to the target), so a call delivers
 * the samples due up to the boundary it ends at; audio_bytes_written counts the bytes delivered and
 * audio_first_index is the number of samples emitted before the call (the delivered streams of
 * consecutive calls concatenate to the stream of one call). A NULL buffer with capacity 0 runs the
 * same code against a counting sink: nothing is written and the machine state is byte-identical to a
 * run with a buffer. The per-frame size includes a slack of ceil(U / P) + 1 samples, U the proven
 * maximum ticks of one CPU unit and P the sample period, which covers the samples of the last
 * unit's overshoot. */

/* Input (D-12..D-15, plan 04-11). Core input is LOGICAL switch state: one uint32_t per controller,
 * indexed by PSG port (0 = R14/$01FE, 1 = R15/$01FF), never by player or side. A word holds the
 * switches that are closed; bit 31 (IVS_CTRL_RESERVED_MASK) is reserved and must be 0. The
 * words are guest-visible machine input (plan 05-05): the guest reads the two PSG I/O ports at
 * $01FE (word 0) and $01FF (word 1), indexed by PSG port, and a port in input mode returns the
 * electrical active-low byte, derived by the core from the words and the profile's control table
 * (FF XOR the OR of the line masks of every closed switch, upper byte 0; a port the guest set to
 * output returns its stored latch); there is no raw-byte override and no electrical input path.
 * Words at an index at or above the profile's connected controller count (input.controllers; 2
 * on the Phase 4 profile, so words 2 and 3, reserved for expansion ports) must be 0 (UNSUPPORTED,
 * INPUT_EXTRA_CONTROLLER, plan 04-12).
 *
 * Input arrives as a caller-owned array of level samples in the run request, nondecreasing by
 * at_tick (master ticks, the unit of target_ticks). The core keeps no pointer, no queue and has no
 * set-input call. An event takes effect at the first CPU-unit boundary whose start tick is at or
 * after its at_tick: a run tests its exit conditions first and, only when a unit will begin,
 * applies every not yet applied event with at_tick <= the unit's start, in array order (equal
 * ticks keep it; the last one applied is the state in force). Nothing is applied when a call
 * ends. The report returns input_consumed (events applied by this call; the host resubmits the
 * suffix from there), input_floor_tick (start of the last begun unit + 1, 0 before any unit) and
 * controller_now (the words in force). The floor is instance state: an event below it is refused,
 * so an applied event can never be followed by an earlier one. Create and reset set the words to
 * 0, the count and floor to 0 and the input log (digest domain IVS_DIGEST_INPUT) to its start.
 * The log is a chain over the applied events with their absolute at_tick:
 *   h0 = SHA-256("IVS-INPUT-LOG-V1"), h(n+1) = SHA-256(h(n) || be64(at_tick) || be32(controller[0..3]))
 * The array must not change during the call (undefined behaviour otherwise): the whole array is
 * validated before any write, then each event is read once for application. */
#define IVS_INPUT_CONTROLLER_WORDS 4u   /* words per event and in controller_now */
#define IVS_CTRL_RESERVED_MASK 0x80000000u /* reserved control bit: nonzero is UNSUPPORTED */
#define IVS_INPUT_EVENTS_MAX 65536u     /* largest event_count */
#define IVS_INPUT_INDEX_NONE 0xFFFFFFFFu /* input_failing_index of a report that names no event */

/* Control-surface bits of one controller word (D-12, D-16, plan 04-13): each name is one closed
 * switch, as LOGICAL state; the word is indexed by PSG port, and the electrical byte a guest reads
 * is a derived Phase 5 observation, not this bit layout. Keypad bits 0..11; disc positions bits
 * 12..27, sixteen 22.5-degree steps clockwise from north; side codes bits 28..30, where
 * IVS_CTRL_SIDE_TOP is the two top side buttons as ONE code (a host maps either to it; C15);
 * bit 31 is IVS_CTRL_RESERVED_MASK. Several bits may be set at once. The bit indices are pinned
 * by data/expected/input-bits.ids (append-only): a moved bit would change every recorded input. */
#define IVS_CTRL_KEY_1 (1u << 0)
#define IVS_CTRL_KEY_2 (1u << 1)
#define IVS_CTRL_KEY_3 (1u << 2)
#define IVS_CTRL_KEY_4 (1u << 3)
#define IVS_CTRL_KEY_5 (1u << 4)
#define IVS_CTRL_KEY_6 (1u << 5)
#define IVS_CTRL_KEY_7 (1u << 6)
#define IVS_CTRL_KEY_8 (1u << 7)
#define IVS_CTRL_KEY_9 (1u << 8)
#define IVS_CTRL_KEY_CLEAR (1u << 9)
#define IVS_CTRL_KEY_0 (1u << 10)
#define IVS_CTRL_KEY_ENTER (1u << 11)
#define IVS_CTRL_DISC_N (1u << 12)
#define IVS_CTRL_DISC_NNE (1u << 13)
#define IVS_CTRL_DISC_NE (1u << 14)
#define IVS_CTRL_DISC_ENE (1u << 15)
#define IVS_CTRL_DISC_E (1u << 16)
#define IVS_CTRL_DISC_ESE (1u << 17)
#define IVS_CTRL_DISC_SE (1u << 18)
#define IVS_CTRL_DISC_SSE (1u << 19)
#define IVS_CTRL_DISC_S (1u << 20)
#define IVS_CTRL_DISC_SSW (1u << 21)
#define IVS_CTRL_DISC_SW (1u << 22)
#define IVS_CTRL_DISC_WSW (1u << 23)
#define IVS_CTRL_DISC_W (1u << 24)
#define IVS_CTRL_DISC_WNW (1u << 25)
#define IVS_CTRL_DISC_NW (1u << 26)
#define IVS_CTRL_DISC_NNW (1u << 27)
#define IVS_CTRL_SIDE_TOP (1u << 28)
#define IVS_CTRL_SIDE_LOWER_LEFT (1u << 29)
#define IVS_CTRL_SIDE_LOWER_RIGHT (1u << 30)

/* One input event: 32 bytes, a fixed v1 record (no struct_size; growth is a new request field).
 * controller[i] is the full logical state of the controller on PSG port i from at_tick on
 * (IVS_INPUT_CONTROLLER_WORDS words). */
typedef struct ivs_input_event {
  uint64_t at_tick;       /* master ticks, below 2^63 */
  uint32_t flags;         /* must be 0 (UNSUPPORTED otherwise) */
  uint32_t reserved;      /* must be 0 (UNSUPPORTED otherwise) */
  uint32_t controller[4]; /* index = PSG port (0 = R14/$01FE, 1 = R15/$01FF), not player */
} ivs_input_event;

/* In: one run. The v1 size is IVS_RUN_REQUEST_V1_SIZE (64 bytes with 64-bit pointers; events at
 * offset 48), frozen by plan 04-13 (test run.struct.*) and pinned as a constant by plan 05-02
 * (D-19, A-04): the minimum accepted struct_size is that constant, never sizeof of the grown
 * struct, so a caller built against the Phase 4 header keeps working. The v2 fields psg_log and
 * psg_log_capacity (plan 05-02, offsets 64 and 72, sizeof 80 with 64-bit pointers) are read only
 * when struct_size covers them (at least 80); a v1-sized caller reads as psg_log NULL, capacity
 * 0. A struct_size above the core's own size (up to IVS_STRUCT_SIZE_MAX) is accepted only when
 * every byte past the core's size is zero. New fields go after the last one (A-02). */
#define IVS_RUN_REQUEST_V1_SIZE 64u /* the 64-bit-pointer statement; other pointer widths do not pin it */
typedef struct ivs_run_request {
  uint32_t struct_size;
  uint32_t flags;          /* IVS_RUN_F_* */
  uint64_t target_ticks;   /* absolute machine time in master ticks, at most IVS_RUN_TARGET_MAX */
  uint8_t *video;          /* caller-owned video bytes, or NULL with video_capacity 0 */
  uint64_t video_capacity; /* bytes at video */
  uint8_t *audio;          /* caller-owned audio bytes, or NULL with audio_capacity 0 */
  uint64_t audio_capacity; /* bytes at audio */
  const ivs_input_event *events; /* caller-owned input schedule, or NULL with event_count 0 */
  uint32_t event_count;          /* events at events, at most IVS_INPUT_EVENTS_MAX */
  uint32_t reserved_in;          /* must be 0 (UNSUPPORTED otherwise) */
  /* ---- end of the v1 size (64) ---- */
  uint8_t *psg_log;            /* v2: caller-owned write-log bytes, or NULL with psg_log_capacity 0 */
  uint64_t psg_log_capacity;   /* v2: bytes at psg_log */
} ivs_run_request;

#define IVS_RUN_REPORT_V1_SIZE 112u

/* Out: what one run did (v1 size IVS_RUN_REPORT_V1_SIZE = 112 bytes on every ABI, frozen by plan 04-13 and pinned as a
 * constant by plan 05-02; A-02 from there). On IVS_OK
 * every field is written: the stop reason,
 * refusal_reason IVS_REASON_NONE, the CPU run state and fault reason at exit (IVS_CPU_STATE_*,
 * IVS_CPU_FAULT_*), machine time at exit, overshoot_ticks = max(now_ticks - target_ticks, 0)
 * (derived, never state), elapsed_ticks = now_ticks minus machine time at entry and the units
 * executed by this call, the output counts (frames completed, bytes written to each buffer;
 * all 0 in Phase 4), and the input fields (input_consumed, input_failing_index
 * IVS_INPUT_INDEX_NONE, input_floor_tick, controller_now; above). On a refusal only struct_size
 * and refusal_reason (IVS_REASON_*) are written, plus input_failing_index for a refusal of one
 * event (the IVS_REASON_INPUT_* reasons of the event stages below), and nothing at all when the
 * report itself is the cause (NULL, size, overlap).
 * Growth (plan 05-02, D-19, A-04): the minimum accepted rep->struct_size is IVS_RUN_REPORT_V1_SIZE;
 * the core writes min(rep->struct_size, sizeof(ivs_run_report)) bytes, sets the written
 * struct_size to that count, and leaves every byte of a larger caller struct past it untouched.
 * Refusal writes (struct_size, refusal_reason, input_failing_index) stay inside the v1 prefix. */
typedef struct ivs_run_report {
  uint32_t struct_size;
  uint32_t stop_reason;      /* IVS_STOP_* */
  uint32_t refusal_reason;   /* IVS_REASON_* */
  uint32_t cpu_run_state;    /* IVS_CPU_STATE_* */
  uint32_t cpu_fault_reason; /* IVS_CPU_FAULT_* */
  uint32_t reserved;         /* written 0 */
  uint64_t now_ticks;
  uint64_t overshoot_ticks;
  uint64_t elapsed_ticks;
  uint64_t units_executed;
  uint64_t video_frames_completed; /* 0 in Phase 4 */
  uint64_t video_bytes_written;    /* 0 in Phase 4 */
  uint64_t audio_bytes_written;    /* audio bytes delivered into the caller buffer (plan 05-06) */
  uint32_t input_consumed;         /* events of the request applied by this call */
  uint32_t input_failing_index;    /* the refused event, else IVS_INPUT_INDEX_NONE */
  uint64_t input_floor_tick;       /* start of the last begun unit + 1 (0 before any unit) */
  uint32_t controller_now[4];      /* the controller words in force at exit */
  /* ---- end of the v1 size (112) ---- */
  uint64_t audio_first_index;      /* v2, offset 112: samples emitted before this call (plan 05-06) */
  uint64_t psg_log_bytes_written;  /* v2, offset 120: write-log bytes delivered into psg_log by this call (plan 05-08) */
  uint64_t psg_log_first_index;    /* v2, offset 128: records logged before this call (plan 05-08); sizeof 136 */
} ivs_run_report;

/* Runs m to req->target_ticks (above). Refusal order (D-05), first failing class wins, every
 * refusal before the first unit so a refused call changes nothing in the instance and writes no
 * byte into either output buffer:
 *   BAD_ARG       req NULL (IVS_REASON_RUN_NULL_ARG); req->struct_size below
 *                 IVS_RUN_REQUEST_V1_SIZE or above IVS_STRUCT_SIZE_MAX (RUN_STRUCT_SIZE); rep NULL,
 *                 or rep->struct_size below IVS_RUN_REPORT_V1_SIZE or above IVS_STRUCT_SIZE_MAX
 *                 (nothing written);
 *   BAD_HANDLE    NULL or invalid handle (IVS_REASON_HANDLE);
 *   BAD_ARG       rep overlaps the instance storage (nothing written);
 *   UNSUPPORTED   any flag bit other than IVS_RUN_F_STOP_ON_HALT (RUN_FLAGS; IVS_RUN_F_STOP_ON_FRAME
 *                 is refused as unknown until the frame stop is implemented), then a nonzero byte past the core's own
 *                 sizeof of a larger request (RUN_TAIL), then a nonzero reserved_in
 *                 (INPUT_RESERVED_IN);
 *   BAD_ARG       output buffers: a NULL buffer with a nonzero capacity (RUN_BUFFER_NULL), then a
 *                 buffer (capacity bytes) running past the top of the address space or
 *                 overlapping the instance storage, the other buffer or the report
 *                 (RUN_BUFFER_OVERLAP; uintptr_t arithmetic, 64-bit lengths); then the write log
 *                 (plan 05-02): psg_log NULL with a nonzero psg_log_capacity (RUN_PSGLOG_NULL),
 *                 psg_log running past the top of the address space or overlapping the instance,
 *                 the report, video, audio or the event array (RUN_PSGLOG_OVERLAP);
 *   BAD_ARG       event arguments: events NULL with event_count above 0 (INPUT_EVENTS_NULL),
 *                 event_count above IVS_INPUT_EVENTS_MAX (INPUT_EVENT_COUNT), then the
 *                 event_count * 32 bytes at events running past the top of the address space
 *                 or overlapping the instance storage, the report,
 *                 the video buffer or the audio buffer (INPUT_EVENTS_OVERLAP); no event is read;
 *   BAD_STATE     the machine is OFF (RUN_STATE_OFF);
 *   OUT_OF_RANGE  target_ticks above IVS_RUN_TARGET_MAX (RUN_TARGET_RANGE);
 *   BUFFER_TOO_SMALL  a capacity below the output layout's requirement (above; RUN_BUFFER_SMALL for
 *                 video or audio, RUN_PSGLOG_SMALL for the write log);
 *   BAD_ARG       the events, whole array, lowest failing index: an at_tick at or above 2^63
 *                 (INPUT_TICK_RANGE), below the previous event's (INPUT_UNSORTED) or below the
 *                 input floor (INPUT_BEFORE_FLOOR);
 *   UNSUPPORTED   the events, whole array, lowest failing index: nonzero flags
 *                 (INPUT_EVENT_FLAGS), a nonzero reserved word (INPUT_EVENT_RESERVED), a controller
 *                 word with IVS_CTRL_RESERVED_MASK set (INPUT_CONTROL_RESERVED), a nonzero word at
 *                 an index at or above input.controllers (INPUT_EXTRA_CONTROLLER).
 * A refusal writes only struct_size and refusal_reason (and, for the last two stages,
 * input_failing_index), and only into a report that is not the cause, does not overlap the bytes
 * the handle names and does not overlap an output buffer or the event array (the event_count * 32
 * bytes at events, considered only for an event_count within IVS_INPUT_EVENTS_MAX); it writes no
 * byte into either buffer or the array. */
IVS_API ivs_status ivs_machine_run(ivs_machine *m, const ivs_run_request *req, ivs_run_report *rep);

/* The static, constant name of a stop reason ("IVS_STOP_TARGET", ...) or "IVS_STOP_UNKNOWN" for
 * a value that is not one (0 included). Cannot refuse and touches no instance. */
IVS_API const char *ivs_run_stop_name(uint32_t stop);

#ifdef __cplusplus
}
#endif

#endif

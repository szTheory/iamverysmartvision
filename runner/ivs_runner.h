/* SPDX-License-Identifier: MIT */
/* Runner library interface (D-17, D-20, plan 04-03). Outcome taxonomy: an outcome value is its
 * process exit code (pass 0, guest-fail 10, timeout 11, cancelled 12, missing-fixture 13,
 * harness-fail 14; 1 is AddressSanitizer's default, 77 the ctest skip code and 126 and above
 * belong to the shell). A timeout carries a detail flag: IVS_RUNNER_OUTCOME_F_WALL marks the
 * host watchdog (host-dependent, never golden); without it the machine-time budget ended
 * (deterministic). Pure functions: no I/O, no state. */
#ifndef IVS_RUNNER_H
#define IVS_RUNNER_H

#include <stddef.h>
#include <stdint.h>

#define IVS_RUNNER_OUTCOME_PASS 0u
#define IVS_RUNNER_OUTCOME_GUEST_FAIL 10u
#define IVS_RUNNER_OUTCOME_TIMEOUT 11u
#define IVS_RUNNER_OUTCOME_CANCELLED 12u
#define IVS_RUNNER_OUTCOME_MISSING_FIXTURE 13u
#define IVS_RUNNER_OUTCOME_HARNESS_FAIL 14u

/* Timeout detail flag and the two timeout outcomes. */
#define IVS_RUNNER_OUTCOME_F_WALL 0x100u
#define IVS_RUNNER_OUTCOME_TIMEOUT_BUDGET IVS_RUNNER_OUTCOME_TIMEOUT
#define IVS_RUNNER_OUTCOME_TIMEOUT_WALL (IVS_RUNNER_OUTCOME_TIMEOUT | IVS_RUNNER_OUTCOME_F_WALL)

/* Observation bits for ivs_runner_outcome_resolve: what the run saw. */
#define IVS_RUNNER_OBS_GUEST_FAIL 0x01u
#define IVS_RUNNER_OBS_TIMEOUT_BUDGET 0x02u
#define IVS_RUNNER_OBS_TIMEOUT_WALL 0x04u
#define IVS_RUNNER_OBS_CANCELLED 0x08u
#define IVS_RUNNER_OBS_MISSING_FIXTURE 0x10u
#define IVS_RUNNER_OBS_HARNESS_FAIL 0x20u
#define IVS_RUNNER_OBS_ALL 0x3Fu

/* Exit code of an outcome. A value that is not one of the seven outcomes above is an internal
 * error and maps to the harness-fail code. */
uint32_t ivs_runner_exit_code(uint32_t outcome);

/* Report name of an outcome: pass, guest-fail, timeout (both kinds), cancelled,
 * missing-fixture, harness-fail; a value that is not an outcome is named harness-fail. */
const char *ivs_runner_outcome_name(uint32_t outcome);

/* The highest-precedence outcome present in observed_mask: harness-fail > missing-fixture >
 * cancelled > timeout (wall) > timeout (budget) > guest-fail > pass. An empty mask is pass; a
 * bit outside IVS_RUNNER_OBS_ALL is harness-fail. A load refusal that matches the scenario's
 * expect-load is decided by the executor before this call. */
uint32_t ivs_runner_outcome_resolve(uint32_t observed_mask);

/* ---- IVSS v1 scenario parser (D-18, D-22, plan 04-17) ----
 * A scenario is untrusted inert data. ivs_scenario_parse is a pure function from bytes to a
 * struct: no I/O, no allocation, no state; every array has a fixed size at the D-22 caps.
 * Grammar (ASCII, LF line ends, one directive per line, tokens split on spaces and tabs, no
 * quoting; a line whose first byte is `#` is a comment, `#` anywhere else is an ordinary byte;
 * blank lines are skipped). The first directive is `ivss 1`; `id` and `profile` are required.
 *   ivss 1
 *   id <[a-z0-9-]{1,48}>
 *   profile <[a-z0-9-]{1,64}>
 *   content <exec|grom|cart> <relpath>|user|boot-stub|zero-grom   (boot-stub: exec only,
 *       zero-grom: grom only; relpath: [A-Za-z0-9._/-]{1,200}, no `..`, no leading `/`, no
 *       empty segment; resolved later by the executor under --content-dir only)
 *   expect-load <role> ok|<IVS_E_NAME> [reason <n>]   (after that role's content line)
 *   expect-sha256 <role> <64 lower-case hex>           (after that role's content line)
 *   wall-timeout-ms <n>                                (1..4294967295; only lowers the cap)
 *   run-to <n> ticks|cpu-cycles|frames [stop target|halted|faulted]
 *   assert halted ==|!= yes|no
 *   assert reg R<0-7> ==|!= <HHHH>
 *   assert mem <AAAA> ==|!= <HHHH>
 *   assert mem <AAAA>..<BBBB> ==|!= <HHHH>     (every word of the inclusive range equals HHHH)
 *   assert digest <composite|cpu|mem|images|instance|video|audio|input> ==|!= <64 hex>
 *   input at_tick=<n>|at_cycle=<n> c0=<names|0xHEX> c1=<names|0xHEX>
 * Header directives (id, profile, content, expect-load, expect-sha256, wall-timeout-ms) come
 * before the first run-to; singletons (per role for content and expect-*) appear once. Steps
 * share one unit and their targets strictly increase (absolute machine time, at most 2^62);
 * an assert belongs to the run-to before it (a checkpoint). Input lines may appear anywhere
 * after `ivss 1`, share one unit and are nondecreasing in time; a controller word is either
 * 0x plus 1..8 hex digits or names joined by `+`, each name an IVS_CTRL_* define of
 * ivs/ivs_run.h minus the IVS_CTRL_ prefix (the names pinned by data/expected/input-bits.ids);
 * the reserved bit 31 is refused. `at_frame=` is refused until Phase 6. Numbers are decimal
 * digits only (no sign); HHHH and AAAA are four upper-case hex digits. A scenario names no
 * output file and no system image path. Caps: 64 KiB, 512 lines, 256 bytes per line (LF not
 * counted), 64 steps, 64 asserts, 64 input lines. */
#define IVS_SCN_MAX_BYTES 65536u
#define IVS_SCN_MAX_LINES 512u
#define IVS_SCN_MAX_LINE_BYTES 256u
#define IVS_SCN_MAX_STEPS 64u
#define IVS_SCN_MAX_ASSERTS 64u
#define IVS_SCN_MAX_INPUTS 64u
#define IVS_SCN_MAX_PATH 200u
#define IVS_SCN_MAX_ID 48u
#define IVS_SCN_MAX_PROFILE 64u
#define IVS_SCN_TIME_MAX UINT64_C(0x4000000000000000) /* 2^62: largest run-to or input time */

/* Content roles (the index of ivs_scenario.content and .expect). */
#define IVS_SCN_ROLE_EXEC 0u
#define IVS_SCN_ROLE_GROM 1u
#define IVS_SCN_ROLE_CART 2u
#define IVS_SCN_ROLES 3u

/* Content kinds; NONE: the role has no content line. */
#define IVS_SCN_CONTENT_NONE 0u
#define IVS_SCN_CONTENT_PATH 1u
#define IVS_SCN_CONTENT_USER 2u
#define IVS_SCN_CONTENT_BOOT_STUB 3u
#define IVS_SCN_CONTENT_ZERO_GROM 4u

/* run-to units and stop expectations (a run-to without `stop` expects target). */
#define IVS_SCN_UNIT_TICKS 1u
#define IVS_SCN_UNIT_CPU_CYCLES 2u
#define IVS_SCN_UNIT_FRAMES 3u
#define IVS_SCN_STOP_TARGET 1u
#define IVS_SCN_STOP_HALTED 2u
#define IVS_SCN_STOP_FAULTED 3u

/* Input time units. */
#define IVS_SCN_INPUT_AT_TICK 1u
#define IVS_SCN_INPUT_AT_CYCLE 2u

/* Assert subjects and operators. */
#define IVS_SCN_SUBJ_HALTED 1u
#define IVS_SCN_SUBJ_REG 2u
#define IVS_SCN_SUBJ_MEM 3u
#define IVS_SCN_SUBJ_MEM_RANGE 4u
#define IVS_SCN_SUBJ_DIGEST 5u
#define IVS_SCN_OP_EQ 1u
#define IVS_SCN_OP_NE 2u

/* Refusal reasons: each has one report token (ivs_scenario_reason_name). Line 0 names the whole
 * file (file-too-large, and missing-header or missing-directive found at the end). */
#define IVS_SCN_R_NONE 0u
#define IVS_SCN_R_FILE_TOO_LARGE 1u
#define IVS_SCN_R_TOO_MANY_LINES 2u
#define IVS_SCN_R_LINE_TOO_LONG 3u
#define IVS_SCN_R_CR 4u
#define IVS_SCN_R_NUL 5u
#define IVS_SCN_R_NON_ASCII 6u
#define IVS_SCN_R_CONTROL_BYTE 7u
#define IVS_SCN_R_MISSING_HEADER 8u
#define IVS_SCN_R_LATE_HEADER 9u
#define IVS_SCN_R_BAD_VERSION 10u
#define IVS_SCN_R_UNKNOWN_DIRECTIVE 11u
#define IVS_SCN_R_DUPLICATE_DIRECTIVE 12u
#define IVS_SCN_R_HEADER_AFTER_STEP 13u
#define IVS_SCN_R_MISSING_DIRECTIVE 14u
#define IVS_SCN_R_BAD_ARITY 15u
#define IVS_SCN_R_BAD_TOKEN 16u
#define IVS_SCN_R_BAD_NUMBER 17u
#define IVS_SCN_R_OUT_OF_RANGE 18u
#define IVS_SCN_R_BAD_ID 19u
#define IVS_SCN_R_BAD_PROFILE 20u
#define IVS_SCN_R_PATH_DOTDOT 21u
#define IVS_SCN_R_PATH_ABSOLUTE 22u
#define IVS_SCN_R_PATH_BYTE 23u
#define IVS_SCN_R_PATH_EMPTY_SEGMENT 24u
#define IVS_SCN_R_PATH_TOO_LONG 25u
#define IVS_SCN_R_BAD_CONTENT_KIND 26u
#define IVS_SCN_R_EXPECT_WITHOUT_CONTENT 27u
#define IVS_SCN_R_BAD_STATUS 28u
#define IVS_SCN_R_BAD_SHA256 29u
#define IVS_SCN_R_STEPS_NOT_INCREASING 30u
#define IVS_SCN_R_MIXED_STEP_UNIT 31u
#define IVS_SCN_R_TOO_MANY_STEPS 32u
#define IVS_SCN_R_TOO_MANY_ASSERTS 33u
#define IVS_SCN_R_ASSERT_BEFORE_STEP 34u
#define IVS_SCN_R_BAD_VALUE 35u
#define IVS_SCN_R_UNKNOWN_INPUT_NAME 36u
#define IVS_SCN_R_INPUT_RESERVED_BIT 37u
#define IVS_SCN_R_AT_FRAME 38u
#define IVS_SCN_R_INPUT_UNSORTED 39u
#define IVS_SCN_R_TOO_MANY_INPUTS 40u
#define IVS_SCN_R_MIXED_INPUT_UNIT 41u
#define IVS_SCN_R_BAD_INPUT_VALUE 42u
#define IVS_SCN_R_COUNT 43u

typedef struct ivs_scenario_content {
  uint32_t kind; /* IVS_SCN_CONTENT_* */
  uint32_t line;
  char path[IVS_SCN_MAX_PATH + 1u]; /* NUL-terminated relative path, kind PATH only */
} ivs_scenario_content;

typedef struct ivs_scenario_expect {
  uint32_t load_set;    /* 1: an expect-load line was given (else the executor expects ok) */
  uint32_t load_line;
  int32_t load_status;  /* 0 (ok) or an IVS_E_* status */
  uint32_t load_reason; /* 0: any reason */
  uint32_t sha_set;     /* 1: an expect-sha256 line was given */
  uint32_t sha_line;
  uint8_t sha256[32];
} ivs_scenario_expect;

typedef struct ivs_scenario_step {
  uint64_t target; /* absolute, in unit */
  uint32_t unit;   /* IVS_SCN_UNIT_* */
  uint32_t stop;   /* IVS_SCN_STOP_* */
  uint32_t line;
} ivs_scenario_step;

typedef struct ivs_scenario_assert {
  uint32_t line;
  uint32_t step;      /* index of the run-to this checkpoint follows */
  uint32_t subject;   /* IVS_SCN_SUBJ_* */
  uint32_t op;        /* IVS_SCN_OP_* */
  uint32_t index;     /* register number (REG) or IVS_DIGEST_* domain (DIGEST) */
  uint16_t addr_lo;   /* MEM, MEM_RANGE */
  uint16_t addr_hi;   /* MEM_RANGE; equals addr_lo for MEM */
  uint16_t value;     /* HALTED (1 yes, 0 no), REG, MEM, MEM_RANGE */
  uint8_t sha256[32]; /* DIGEST */
} ivs_scenario_assert;

typedef struct ivs_scenario_input {
  uint64_t at; /* in the scenario's input_unit */
  uint32_t controller[2];
  uint32_t line;
} ivs_scenario_input;

typedef struct ivs_scenario {
  char id[IVS_SCN_MAX_ID + 1u];
  char profile[IVS_SCN_MAX_PROFILE + 1u];
  uint32_t wall_timeout_ms; /* 0: not given */
  ivs_scenario_content content[IVS_SCN_ROLES];
  ivs_scenario_expect expect[IVS_SCN_ROLES];
  uint32_t step_unit; /* unit of every step, 0 when there is none */
  uint32_t n_steps;
  ivs_scenario_step steps[IVS_SCN_MAX_STEPS];
  uint32_t n_asserts;
  ivs_scenario_assert asserts[IVS_SCN_MAX_ASSERTS];
  uint32_t input_unit; /* IVS_SCN_INPUT_AT_*, 0 when there is no input line */
  uint32_t n_inputs;
  ivs_scenario_input inputs[IVS_SCN_MAX_INPUTS];
} ivs_scenario;

typedef struct ivs_scenario_error {
  uint32_t line;   /* 1-based physical line; 0 names the whole file */
  uint32_t reason; /* IVS_SCN_R_*; IVS_SCN_R_NONE after a successful parse */
} ivs_scenario_error;

/* Parses len bytes. Returns 0 and fills *out on success; returns the nonzero reason, fills *err
 * and leaves *out all zero on refusal; returns -1 without touching anything when out or err is
 * NULL, or bytes is NULL with a nonzero len. The first failing check in file order wins. */
int ivs_scenario_parse(const uint8_t *bytes, size_t len, ivs_scenario *out, ivs_scenario_error *err);

/* Report token of a reason ("file-too-large", ...), or "unknown" for a value that is not one. */
const char *ivs_scenario_reason_name(uint32_t reason);

/* Writes "scenario-syntax line=<n> reason=<token>" with a NUL into buf. Returns 0, or -1 when
 * buf is NULL or cap is too small (buf then holds an empty string when cap > 0). */
int ivs_scenario_error_format(const ivs_scenario_error *err, char *buf, size_t cap);

/* The input-name table: count, and the name (without IVS_CTRL_) and bit mask of entry i
 * (NULL and mask 0 when i is out of range). */
uint32_t ivs_scenario_input_name_count(void);
const char *ivs_scenario_input_name(uint32_t i, uint32_t *mask);

/* ---- executor and report (D-17, D-19, D-20, D-22, D-30, plan 04-18) ----
 * ivs_runner_execute runs one parsed scenario on one fresh instance it creates in the host's
 * storage buffer (aligned by hand; the library allocates nothing and has no globals), through
 * the public API only: create, load each role in the order exec, grom, cart, reset, then each
 * run-to as slices of ivs_machine_run (always with IVS_RUN_F_STOP_ON_HALT, so a halt is observed
 * as a stop; slicing, budget and wall cap: ivs_runner_options), evaluating the asserts of a step through ivs_machine_cpu_state, ivs_machine_peek and
 * ivs_machine_state_digest at that step's stop. Execution ends at the first step with a failed
 * stop expectation or assert. It then writes the report through host->write_report and
 * returns the outcome (an IVS_RUNNER_OUTCOME_* value).
 * Content: `boot-stub` reads the reserved file boot-stub.exec under the content dir (the label
 * comes from the symbolic name only, never from the bytes or a hash table, D-30); `zero-grom`
 * is 2048 zero bytes made in the runner; a relpath goes to host->read_file; `user` goes to
 * host->read_user (missing-fixture when that is NULL or has no operator image).
 * Units: cpu-cycles and frames bounds and at_cycle input times are converted with the
 * timebase of ivs_info (n * ticks_per_cpu_cycle, n * nominal_frame_ticks; frames are nominal,
 * not frame events, D-09); a converted value above IVS_RUN_TARGET_MAX is harness-fail.
 * Report numbering: steps and asserts are numbered from 1 in scenario order; an assert keeps
 * its scenario-wide number (step.<k>.assert.<j>, outcome detail assert=<j>). */

/* Bounds of a role's read (the core's own bounds, ivs/ivs_machine.h); the read buffer holds
 * cap + 1 bytes so an oversized file reaches the core's own OVERSIZED refusal. */
#define IVS_RUNNER_CAP_EXEC 8192u
#define IVS_RUNNER_CAP_GROM 2048u
#define IVS_RUNNER_CAP_CART 131256u
#define IVS_RUNNER_STORAGE_MAX (64u * 1024u * 1024u) /* D-22: storage cap */
#define IVS_RUNNER_BUFFERS_MAX (8u * 1024u * 1024u)  /* D-22: output buffer cap */
#define IVS_RUNNER_BOOT_STUB_FILE "boot-stub.exec"   /* reserved name under the content dir */

/* Host read results. MISSING and NOT_REGULAR are missing-fixture; TOO_LARGE means the file has
 * more than cap bytes (the host returns the first cap + 1 in buf, which the executor hands to
 * the core for its OVERSIZED refusal); IO_ERROR (and any other value) is harness-fail. */
#define IVS_RUNNER_READ_OK 0
#define IVS_RUNNER_READ_MISSING 1
#define IVS_RUNNER_READ_NOT_REGULAR 2
#define IVS_RUNNER_READ_TOO_LARGE 3
#define IVS_RUNNER_READ_IO_ERROR 4

/* Host services (D-17, D-21, D-22). read_file reads relpath (already validated by the parser,
 * resolved by the host under its content dir only) into buf, at most cap + 1 bytes, and sets
 * *len. read_user (may be NULL) reads the operator's image for a `user` role (role is
 * IVS_SCN_ROLE_*). cancel_requested (may be NULL) returns nonzero once cancellation was asked
 * for; now_ms (may be NULL: no wall cap) is the host's monotonic time in milliseconds; both are
 * called once before every slice (ivs_runner_options above).
 * write_report receives the report one line at a time (LF included) and returns 0, or nonzero
 * on a write error. storage: storage_bytes bytes the executor owns during the call (instance,
 * aligned by hand, then the read buffer); at most IVS_RUNNER_STORAGE_MAX of it is used.
 * buffers: buffer_bytes for video and audio output (may be NULL/0 while the profile's output
 * layout is NONE); at most IVS_RUNNER_BUFFERS_MAX is used. */
typedef struct ivs_runner_host {
  void *ctx;
  int (*read_file)(void *ctx, const char *relpath, size_t cap, uint8_t *buf, size_t *len);
  int (*read_user)(void *ctx, uint32_t role, size_t cap, uint8_t *buf, size_t *len);
  uint64_t (*now_ms)(void *ctx);
  int (*cancel_requested)(void *ctx);
  int (*write_report)(void *ctx, const char *bytes, size_t len);
  void *storage;
  size_t storage_bytes;
  uint8_t *buffers;
  size_t buffer_bytes;
} ivs_runner_host;

/* Slicing, budget and wall cap (D-21, plan 04-19). Every run-to is executed as slices of at most
 * slice_ticks master ticks: ivs_machine_run(target = min(limit, now + slice_ticks)) with the
 * unconsumed input suffix, until the CPU stops or machine time reaches the limit (the core's
 * slicing invariance, API-02, makes the result equal to one call). Before every slice (so also
 * between slices) the executor calls cancel_requested exactly once, then now_ms once: a
 * cancellation is outcome cancelled (`signal step=<k>`); now_ms minus its value at the start of
 * the first step at or above the wall cap is timeout with detail `wall step=<k> cap-ms=<n>`
 * (host-dependent, never golden). Both at once resolve to cancelled (precedence). The
 * machine-time budget is separate and deterministic: the run-to bound reached where `stop
 * halted|faulted` was expected (`budget step=...`), or the max_ticks ceiling cut a run-to short
 * (`budget step=<k> line=<n> max-ticks=<m>`), are timeout without the wall flag. A scenario's
 * wall-timeout-ms only lowers the operator's cap. */
#define IVS_RUNNER_SLICE_TICKS_MAX UINT64_C(4194304)          /* 1 << 22, the default slice */
#define IVS_RUNNER_MAX_TICKS_DEFAULT UINT64_C(1099511627776)  /* 1 << 40 master ticks */
#define IVS_RUNNER_WALL_MS_DEFAULT 30000u                    /* D-22: default wall cap */

/* Executor options (may be NULL: every default). A value outside its range is harness-fail
 * without a report. */
typedef struct ivs_runner_options {
  uint32_t flags;           /* must be 0 */
  uint32_t wall_timeout_ms; /* the operator's wall cap; 0: IVS_RUNNER_WALL_MS_DEFAULT */
  uint64_t slice_ticks;     /* 1..IVS_RUNNER_SLICE_TICKS_MAX; 0: IVS_RUNNER_SLICE_TICKS_MAX */
  uint64_t max_ticks;       /* machine-time ceiling, 1..IVS_RUN_TARGET_MAX; 0: the default */
} ivs_runner_options;

/* What a call returned, besides the outcome. host_calls counts read_file, read_user, now_ms and
 * cancel_requested calls (the one host-dependent report key); report_lines counts the lines
 * before report.end; report_write_failed is 1 when write_report failed (the outcome is then
 * harness-fail whatever the report's outcome line says). */
typedef struct ivs_runner_result {
  uint32_t outcome;
  uint32_t exit_code;
  uint64_t host_calls;
  uint64_t report_lines;
  uint32_t report_write_failed;
  uint32_t reserved;
} ivs_runner_result;

/* Runs scn (a successful ivs_scenario_parse result) and writes its report. Returns the outcome
 * (also in *result when result is not NULL); NULL scn or host, a NULL write_report or read_file,
 * or nonzero options are harness-fail without a report. */
uint32_t ivs_runner_execute(const ivs_scenario *scn, const ivs_runner_host *host, const ivs_runner_options *opts,
                            ivs_runner_result *result);

/* Stop classification (D-04, D-20), a pure function of the core's stop reason and the
 * scenario's expected stop (IVS_SCN_STOP_*). Returns 0 when they match (TARGET with target,
 * CPU_HALTED with halted, GUEST_FAULT with faulted); IVS_RUNNER_OBS_TIMEOUT_BUDGET when the
 * bound was reached (TARGET) while halted or faulted was expected; IVS_RUNNER_OBS_HARNESS_FAIL for
 * IVS_STOP_INTERNAL, a value that is not a stop reason or an unknown expectation;
 * IVS_RUNNER_OBS_GUEST_FAIL for any other pair (the guest halted or faulted unexpectedly). */
uint32_t ivs_runner_stop_observation(uint32_t stop_reason, uint32_t expected_stop);

/* ---- report record: library-internal, filled by exec.c, written by report.c ---- */
#define IVS_RUNNER_DETAIL_MAX 320u
#define IVS_RUNNER_GOT_MAX 80u
#define IVS_RUNNER_ASSERT_NOT_RUN 0u
#define IVS_RUNNER_ASSERT_PASS 1u
#define IVS_RUNNER_ASSERT_FAIL 2u

typedef struct ivs_runner_slot_record {
  uint32_t attempted; /* a load call was made */
  int32_t status;     /* its status */
  uint32_t reason;    /* its load report reason */
  uint32_t loaded;
  uint32_t length_bytes;
  uint8_t sha256[32];
} ivs_runner_slot_record;

typedef struct ivs_runner_step_record {
  uint32_t converted; /* target_ticks is known */
  uint32_t ran;       /* the run call returned IVS_OK */
  uint64_t target_ticks;
  uint64_t now_ticks;
  uint64_t overshoot_ticks;
  uint32_t stop_reason;
  uint32_t reserved;
} ivs_runner_step_record;

typedef struct ivs_runner_assert_record {
  uint32_t state;                  /* IVS_RUNNER_ASSERT_* */
  char got[IVS_RUNNER_GOT_MAX];    /* actual value as printed, NUL-terminated */
} ivs_runner_assert_record;

typedef struct ivs_runner_record {
  const ivs_scenario *scn;
  uint32_t outcome;
  uint32_t have_machine; /* cpu and digest fields are valid */
  char detail[IVS_RUNNER_DETAIL_MAX];
  ivs_runner_slot_record slot[IVS_SCN_ROLES];
  ivs_runner_step_record step[IVS_SCN_MAX_STEPS];
  ivs_runner_assert_record assert_rec[IVS_SCN_MAX_ASSERTS];
  uint32_t cpu_run_state;
  uint32_t cpu_fault_reason;
  uint8_t digest[8][32]; /* cpu, mem, images, instance, composite, video, audio, input */
  uint32_t have_input;   /* a run call returned IVS_OK: controller words are valid */
  uint32_t input_consumed;
  uint32_t controller[4];
  uint64_t video_bytes;
  uint64_t audio_bytes;
  uint64_t host_calls;
} ivs_runner_record;

/* Writes the report of rec through host->write_report, every key of the closed key table in
 * its fixed order: `ivsr 1` first, `report.end <n>` last (n = lines before it). Returns 0, or
 * -1 on a write error; *lines gets the count of lines before report.end. */
int ivs_runner_report_write(const ivs_runner_record *rec, const ivs_runner_host *host, uint64_t *lines);

/* Report tokens: the stop reason (target, halted, faulted, internal, unknown), an expected stop
 * (target, halted, faulted, unknown) and a digest domain (cpu, ..., or unknown). */
const char *ivs_runner_stop_token(uint32_t stop_reason);
const char *ivs_runner_expected_stop_token(uint32_t expected_stop);
const char *ivs_runner_domain_token(uint32_t domain);

#endif

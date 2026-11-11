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

#endif

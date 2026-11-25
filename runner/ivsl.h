/* SPDX-License-Identifier: MIT */
/* IVSL v1, the input log of one run (TOOL-01, D-15, plan 11-02). The C surface of the format specified by
 * docs/formats/ivsl-v1.md: the constants, the parsed log, the nine refusal reasons and the three calls. This is a
 * pure library translation unit: the writer fills a caller-supplied buffer and the parser reads a caller-supplied
 * buffer, so a log is only ever memory here and the file is the operator layer's business (A-11-04). It is in
 * IVS_RUNNER_SOURCES rather than IVS_RUN_SOURCES because tests/faults.cmake refuses a fault whose owner TU is not in
 * a module of its suite, and the two faults of this format must have sites in a library TU.
 *
 * The header has no include of ivs_runner.h: ivs_runner.h includes this one, so the constants are defined here
 * rather than borrowed. IVS_IVSL_MAX_EVENTS is IVS_SCN_MAX_INPUTS (64): the recorder can never produce more events
 * than the scenario supplied, so it is not a new number. IVS_IVSL_MAX_BYTES is 4096: the largest header (154) plus
 * 64 event lines of the widest shape (52 each, 3328) is 3482, leaving 614 bytes of slack.
 */
#ifndef IVS_IVSL_H
#define IVS_IVSL_H

#include <stddef.h>
#include <stdint.h>

#define IVS_IVSL_VERSION 1u
#define IVS_IVSL_MAX_BYTES 4096u
#define IVS_IVSL_MAX_EVENTS 64u /* IVS_SCN_MAX_INPUTS */
#define IVS_IVSL_MAX_ID 48u
#define IVS_IVSL_MAX_PROFILE 64u
#define IVS_IVSL_CONTENT_PROJECT 0u
#define IVS_IVSL_CONTENT_USER 1u
#define IVS_IVSL_TICK_MAX UINT64_C(0x4000000000000000) /* 2^62: the largest at_tick is one below it */

/* One event of the log: the master tick it is declared at and the two control words the replay submits, index =
 * PSG port (0 = R14/$01FE, 1 = R15/$01FF). Words 2 and 3 of the core's ivs_input_event stay 0: IVSL v1 carries the
 * two ports the profile connects (docs/formats/ivsl-v1.md, "Profile scope"). */
typedef struct ivs_ivsl_event {
  uint64_t at_tick;
  uint32_t controller[2];
} ivs_ivsl_event;

/* One parsed log. scenario and profile are informational in v1 (nothing compares them with the scenario being
 * replayed); content is IVS_IVSL_CONTENT_*; n_events is at most IVS_IVSL_MAX_EVENTS. */
typedef struct ivs_ivsl_log {
  uint32_t version;
  char scenario[IVS_IVSL_MAX_ID + 1u];
  char profile[IVS_IVSL_MAX_PROFILE + 1u];
  uint32_t content;
  uint32_t n_events;
  ivs_ivsl_event events[IVS_IVSL_MAX_EVENTS];
} ivs_ivsl_log;

/* The nine refusal reasons of the document's "Refusals" table, in its row order, and the success value. */
#define IVS_IVSL_R_OK 0u
#define IVS_IVSL_R_UNKNOWN_DIRECTIVE 1u
#define IVS_IVSL_R_VERSION 2u
#define IVS_IVSL_R_ARITY 3u
#define IVS_IVSL_R_HEX 4u
#define IVS_IVSL_R_TRAILING 5u
#define IVS_IVSL_R_TICK_ORDER 6u
#define IVS_IVSL_R_TICK_BOUND 7u
#define IVS_IVSL_R_EVENT_CAP 8u
#define IVS_IVSL_R_CONTROLLER_BIT 9u

/* A refusal names the reason and the 1-based line it judged (0 when the file has no line to name). */
typedef struct ivs_ivsl_error {
  uint32_t reason;
  uint32_t line;
} ivs_ivsl_error;

/* The reason's token exactly as the document spells it ("unknown-directive", ...), or "unknown" for a value that is
 * not one of the nine. */
const char *ivs_ivsl_reason_name(uint32_t reason);

/* Writes log as IVSL v1 bytes into out (cap bytes). Returns 0 and sets *written when every byte fits; returns 1 and
 * writes nothing when it does not, when log is not formattable (a name outside the grammar's slug charset, a name
 * above its width, more than IVS_IVSL_MAX_EVENTS events) or when an argument is NULL. It never touches a file. */
int ivs_ivsl_format(const ivs_ivsl_log *log, uint8_t *out, uint64_t cap, uint64_t *written);

/* Parses len bytes. Returns 0 and fills *out on success. Returns 1 on a refusal, setting err->reason and err->line
 * and leaving *out untouched. Returns -1 without touching anything when an argument is NULL or len is above
 * IVS_IVSL_MAX_BYTES (the file cap belongs to the caller, which reads the file). The first failing check in file
 * order wins; a header directive that is missing, duplicated or out of order is IVS_IVSL_R_ARITY named at the line
 * that broke the order (or at the last line when a header is missing at the end). */
int ivs_ivsl_parse(const uint8_t *bytes, size_t len, ivs_ivsl_log *out, ivs_ivsl_error *err);

#endif /* IVS_IVSL_H */

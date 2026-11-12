/* SPDX-License-Identifier: MIT */
/* Suite runner, family parse (D-18, D-22, D-23, plan 04-17): the IVSS v1 scenario parser of the
 * runner library.
 *   - runner.parse.diag-boot-pass feeds the scenario text of diag-boot-pass, whose 18 assert
 *     lines come verbatim from data/golden/diag-cpu-1.expect (diag_cpu_1_expect_text.inc), and
 *     compares the parsed asserts with the independent cmake reading of the same lines
 *     (diag_cpu_1_expect.inc, the X-macro form suite run reads).
 *   - runner.parse.accept.<directive>: one accepted scenario per directive, every field checked
 *     (each has a run-to, review WR-03); runner.parse.accept.refusal-last-role: an expected
 *     refusal with nothing after it, accepted without a run-to (review WR-02);
 *     runner.parse.accept.caps: every cap exactly at its limit is accepted.
 *   - runner.parse.refuse.<token>: one refused scenario per reason token, with the line it must
 *     name; the output must be all zero and a second parse identical.
 *     runner.parse.reasons-named requires one refuse case per reason, so a new reason without a
 *     case fails. runner.parse.edge.*: further refusals of an already covered token.
 *   - runner.parse.input-names-complete: the parser's input-name table against the pinned
 *     names of data/expected/input-bits.ids (ivs_input_bits.inc), both ways.
 * Each scenario is copied into a heap block of exactly its length, so the sanitizers of the dev
 * preset see any read past the end. */
#include <stdint.h>
#include <stdlib.h>
#include <string.h>

#include "ivs/ivs.h"
#include "ivs_runner.h"
#include "ivs_test.h"
#include "runner_cases.h"

#define BUILD_CAP 70000u

/* Parses len bytes from a heap copy of exactly len bytes. Returns the parser's result. */
static int parse_exact(const char *text, size_t len, ivs_scenario *s, ivs_scenario_error *e)
{
  uint8_t *copy = NULL;
  int r;
  if (len > 0u) {
    copy = (uint8_t *)malloc(len);
    if (copy == NULL) {
      ivs_test_diag("out of memory");
      return -2;
    }
    memcpy(copy, text, len);
  }
  r = ivs_scenario_parse(copy, len, s, e);
  free(copy);
  return r;
}

/* Parses and requires acceptance. */
static int parse_ok_n(const char *text, size_t len, ivs_scenario *s)
{
  ivs_scenario_error e;
  int r = parse_exact(text, len, s, &e);
  if (r != 0 || e.reason != IVS_SCN_R_NONE || e.line != 0u) {
    ivs_test_diag("refused: r=%d line=%u reason=%s", r, (unsigned)e.line, ivs_scenario_reason_name(e.reason));
    return 0;
  }
  return 1;
}

static int parse_ok(const char *text, ivs_scenario *s)
{
  return parse_ok_n(text, strlen(text), s);
}

static int all_zero(const void *p, size_t n)
{
  const uint8_t *b = (const uint8_t *)p;
  size_t i;
  for (i = 0u; i < n; i++) {
    if (b[i] != 0u) {
      return 0;
    }
  }
  return 1;
}

/* Parses and requires refusal with reason at line; the output is all zero (it was poisoned
 * before the call) and a second parse gives the identical result. */
static int parse_refused(const char *text, size_t len, uint32_t reason, uint32_t line)
{
  ivs_scenario *s = (ivs_scenario *)malloc(sizeof *s);
  ivs_scenario_error e;
  ivs_scenario_error e2;
  int ok = 1;
  int r;
  int r2;
  if (s == NULL) {
    ivs_test_diag("out of memory");
    return 0;
  }
  memset(s, 0xA5, sizeof *s);
  r = parse_exact(text, len, s, &e);
  if (r != (int)reason || e.reason != reason || e.line != line) {
    ivs_test_diag("got r=%d line=%u reason=%s, want line=%u reason=%s", r, (unsigned)e.line,
                  ivs_scenario_reason_name(e.reason), (unsigned)line, ivs_scenario_reason_name(reason));
    ok = 0;
  } else if (!all_zero(s, sizeof *s)) {
    ivs_test_diag("refusal left a nonzero output");
    ok = 0;
  } else {
    r2 = parse_exact(text, len, s, &e2);
    if (r2 != r || e2.reason != e.reason || e2.line != e.line) {
      ivs_test_diag("second parse differs");
      ok = 0;
    }
  }
  free(s);
  return ok;
}

/* ---- builders for scenarios too long to write as literals (into a test buffer) ---- */

static char build_buf[BUILD_CAP];

typedef struct sb {
  size_t n;
  int overflow;
} sb;

static void sb_add(sb *b, const char *s)
{
  size_t k = strlen(s);
  if (b->overflow || k > BUILD_CAP - b->n) {
    b->overflow = 1;
    return;
  }
  memcpy(build_buf + b->n, s, k);
  b->n += k;
}

static void sb_rep(sb *b, char c, size_t k)
{
  if (b->overflow || k > BUILD_CAP - b->n) {
    b->overflow = 1;
    return;
  }
  memset(build_buf + b->n, c, k);
  b->n += k;
}

static void sb_num(sb *b, uint64_t v)
{
  char d[24];
  size_t k = 0u;
  char out[24];
  size_t i;
  do {
    d[k++] = (char)('0' + (int)(v % 10u));
    v /= 10u;
  } while (v != 0u && k < sizeof d);
  for (i = 0u; i < k; i++) {
    out[i] = d[k - 1u - i];
  }
  out[k] = '\0';
  sb_add(b, out);
}

#define HDR "ivss 1\nid t\nprofile p\n"
/* A run-to every accepted scenario without an expected refusal needs (review WR-03). */
#define RUN0 "run-to 0 ticks\n"

/* Header and a run-to, then comment lines until the file is exactly total bytes (total >= 45). */
static size_t build_size(size_t total)
{
  sb b = {0u, 0};
  sb_add(&b, HDR RUN0);
  while (!b.overflow && total - b.n > 200u) {
    sb_add(&b, "#");
    sb_rep(&b, 'c', 198u);
    sb_add(&b, "\n");
  }
  {
    size_t k = total - b.n - 2u; /* the last line: `#`, k bytes, LF */
    sb_add(&b, "#");
    sb_rep(&b, 'c', k);
    sb_add(&b, "\n");
  }
  return b.overflow ? 0u : b.n;
}

static size_t build_file_too_large(void)
{
  return build_size(IVS_SCN_MAX_BYTES + 1u);
}

/* Header (3 lines), a run-to and comment lines up to n lines in total. */
static size_t build_lines(uint32_t n)
{
  sb b = {0u, 0};
  uint32_t i;
  sb_add(&b, HDR RUN0);
  for (i = 4u; i < n; i++) {
    sb_add(&b, "#\n");
  }
  return b.overflow ? 0u : b.n;
}

static size_t build_too_many_lines(void)
{
  return build_lines(IVS_SCN_MAX_LINES + 1u);
}

/* Line 4 is a comment of exactly k bytes, followed by a step. */
static size_t build_line_of(size_t k)
{
  sb b = {0u, 0};
  sb_add(&b, HDR);
  sb_add(&b, "#");
  sb_rep(&b, 'x', k - 1u);
  sb_add(&b, "\nrun-to 1 ticks\n");
  return b.overflow ? 0u : b.n;
}

static size_t build_line_too_long(void)
{
  return build_line_of(IVS_SCN_MAX_LINE_BYTES + 1u);
}

static size_t build_path_of(size_t k)
{
  sb b = {0u, 0};
  sb_add(&b, HDR);
  sb_add(&b, "content cart ");
  sb_rep(&b, 'a', k);
  sb_add(&b, "\n" RUN0);
  return b.overflow ? 0u : b.n;
}

static size_t build_path_too_long(void)
{
  return build_path_of(IVS_SCN_MAX_PATH + 1u);
}

static size_t build_steps(uint32_t n)
{
  sb b = {0u, 0};
  uint32_t i;
  sb_add(&b, HDR);
  for (i = 1u; i <= n; i++) {
    sb_add(&b, "run-to ");
    sb_num(&b, i);
    sb_add(&b, " ticks\n");
  }
  return b.overflow ? 0u : b.n;
}

static size_t build_too_many_steps(void)
{
  return build_steps(IVS_SCN_MAX_STEPS + 1u);
}

static size_t build_asserts(uint32_t n)
{
  sb b = {0u, 0};
  uint32_t i;
  sb_add(&b, HDR "run-to 1 ticks\n");
  for (i = 0u; i < n; i++) {
    sb_add(&b, "assert halted == no\n");
  }
  return b.overflow ? 0u : b.n;
}

static size_t build_too_many_asserts(void)
{
  return build_asserts(IVS_SCN_MAX_ASSERTS + 1u);
}

static size_t build_inputs(uint32_t n)
{
  sb b = {0u, 0};
  uint32_t i;
  sb_add(&b, HDR);
  for (i = 0u; i < n; i++) {
    sb_add(&b, "input at_tick=");
    sb_num(&b, i);
    sb_add(&b, " c0=KEY_1 c1=0x0\n");
  }
  sb_add(&b, RUN0);
  return b.overflow ? 0u : b.n;
}

static size_t build_too_many_inputs(void)
{
  return build_inputs(IVS_SCN_MAX_INPUTS + 1u);
}

/* ---- refusal table: one entry per reason token ---- */

typedef struct refuse_case {
  const char *id;
  uint32_t reason;
  uint32_t line;
  const char *text; /* NULL: build() writes build_buf */
  size_t len;       /* text length (NUL bytes allowed inside) */
  size_t (*build)(void);
} refuse_case;

#define TXT(s) s, sizeof(s) - 1u, NULL
#define BLD(f) NULL, 0u, f

static const refuse_case refuse_cases[] = {
    {"runner.parse.refuse.file-too-large", IVS_SCN_R_FILE_TOO_LARGE, 0u, BLD(build_file_too_large)},
    {"runner.parse.refuse.too-many-lines", IVS_SCN_R_TOO_MANY_LINES, 513u, BLD(build_too_many_lines)},
    {"runner.parse.refuse.line-too-long", IVS_SCN_R_LINE_TOO_LONG, 4u, BLD(build_line_too_long)},
    {"runner.parse.refuse.cr", IVS_SCN_R_CR, 4u, TXT(HDR "run-to 1 ticks\r\n")},
    {"runner.parse.refuse.nul", IVS_SCN_R_NUL, 4u, TXT(HDR "# a\0b\nrun-to 1 ticks\n")},
    {"runner.parse.refuse.non-ascii", IVS_SCN_R_NON_ASCII, 4u, TXT(HDR "# caf\xC3\xA9\n")},
    {"runner.parse.refuse.control-byte", IVS_SCN_R_CONTROL_BYTE, 4u, TXT(HDR "# a\x01"
                                                                             "b\n")},
    {"runner.parse.refuse.missing-header", IVS_SCN_R_MISSING_HEADER, 2u, TXT("# first\nid t\nivss 1\n")},
    {"runner.parse.refuse.late-header", IVS_SCN_R_LATE_HEADER, 4u, TXT(HDR "ivss 1\n")},
    {"runner.parse.refuse.bad-version", IVS_SCN_R_BAD_VERSION, 1u, TXT("ivss 2\nid t\nprofile p\n")},
    {"runner.parse.refuse.unknown-directive", IVS_SCN_R_UNKNOWN_DIRECTIVE, 4u, TXT(HDR "output report.txt\n")},
    {"runner.parse.refuse.duplicate-directive", IVS_SCN_R_DUPLICATE_DIRECTIVE, 4u, TXT(HDR "id u\n")},
    {"runner.parse.refuse.header-after-step", IVS_SCN_R_HEADER_AFTER_STEP, 5u,
     TXT(HDR "run-to 1 ticks\ncontent cart a.ivsc\n")},
    {"runner.parse.refuse.missing-directive", IVS_SCN_R_MISSING_DIRECTIVE, 0u, TXT("ivss 1\nid t\nrun-to 1 ticks\n")},
    {"runner.parse.refuse.bad-arity", IVS_SCN_R_BAD_ARITY, 4u, TXT(HDR "run-to 1 ticks stop\n")},
    {"runner.parse.refuse.bad-token", IVS_SCN_R_BAD_TOKEN, 4u, TXT(HDR "run-to 1 seconds\n")},
    {"runner.parse.refuse.bad-number", IVS_SCN_R_BAD_NUMBER, 4u, TXT(HDR "run-to -1 ticks\n")},
    {"runner.parse.refuse.out-of-range", IVS_SCN_R_OUT_OF_RANGE, 4u, TXT(HDR "run-to 4611686018427387905 ticks\n")},
    {"runner.parse.refuse.bad-id", IVS_SCN_R_BAD_ID, 2u, TXT("ivss 1\nid Diag\nprofile p\n")},
    {"runner.parse.refuse.bad-profile", IVS_SCN_R_BAD_PROFILE, 3u, TXT("ivss 1\nid t\nprofile p_1\n")},
    {"runner.parse.refuse.path-dotdot", IVS_SCN_R_PATH_DOTDOT, 4u, TXT(HDR "content cart ../secret.ivsc\n")},
    {"runner.parse.refuse.path-absolute", IVS_SCN_R_PATH_ABSOLUTE, 4u, TXT(HDR "content cart /etc/x.ivsc\n")},
    {"runner.parse.refuse.path-byte", IVS_SCN_R_PATH_BYTE, 4u, TXT(HDR "content cart a~b.ivsc\n")},
    {"runner.parse.refuse.path-empty-segment", IVS_SCN_R_PATH_EMPTY_SEGMENT, 4u, TXT(HDR "content cart a//b.ivsc\n")},
    {"runner.parse.refuse.path-too-long", IVS_SCN_R_PATH_TOO_LONG, 4u, BLD(build_path_too_long)},
    {"runner.parse.refuse.bad-content-kind", IVS_SCN_R_BAD_CONTENT_KIND, 4u, TXT(HDR "content cart boot-stub\n")},
    {"runner.parse.refuse.expect-without-content", IVS_SCN_R_EXPECT_WITHOUT_CONTENT, 4u,
     TXT(HDR "expect-load cart ok\ncontent cart a.ivsc\n")},
    {"runner.parse.refuse.bad-status", IVS_SCN_R_BAD_STATUS, 5u,
     TXT(HDR "content cart a.ivsc\nexpect-load cart IVS_E_UNKNOWN\n")},
    {"runner.parse.refuse.bad-sha256", IVS_SCN_R_BAD_SHA256, 5u,
     TXT(HDR "content cart a.ivsc\n"
             "expect-sha256 cart BA7816BF8F01CFEA414140DE5DAE2223B00361A396177A9CB410FF61F20015AD\n")},
    {"runner.parse.refuse.steps-not-increasing", IVS_SCN_R_STEPS_NOT_INCREASING, 5u,
     TXT(HDR "run-to 5 ticks\nrun-to 5 ticks\n")},
    {"runner.parse.refuse.mixed-step-unit", IVS_SCN_R_MIXED_STEP_UNIT, 5u, TXT(HDR "run-to 5 ticks\nrun-to 6 frames\n")},
    {"runner.parse.refuse.too-many-steps", IVS_SCN_R_TOO_MANY_STEPS, 68u, BLD(build_too_many_steps)},
    {"runner.parse.refuse.too-many-asserts", IVS_SCN_R_TOO_MANY_ASSERTS, 69u, BLD(build_too_many_asserts)},
    {"runner.parse.refuse.assert-before-step", IVS_SCN_R_ASSERT_BEFORE_STEP, 4u, TXT(HDR "assert halted == yes\n")},
    {"runner.parse.refuse.bad-value", IVS_SCN_R_BAD_VALUE, 5u, TXT(HDR "run-to 1 ticks\nassert reg R0 == 03c5\n")},
    {"runner.parse.refuse.unknown-input-name", IVS_SCN_R_UNKNOWN_INPUT_NAME, 4u,
     TXT(HDR "input at_tick=0 c0=KEY_1+KEY_X c1=0x0\n")},
    {"runner.parse.refuse.input-reserved-bit", IVS_SCN_R_INPUT_RESERVED_BIT, 4u,
     TXT(HDR "input at_tick=0 c0=0x0 c1=0x80000001\n")},
    {"runner.parse.refuse.at-frame", IVS_SCN_R_AT_FRAME, 4u, TXT(HDR "input at_frame=1 c0=0x0 c1=0x0\n")},
    {"runner.parse.refuse.input-unsorted", IVS_SCN_R_INPUT_UNSORTED, 5u,
     TXT(HDR "input at_tick=5 c0=0x0 c1=0x0\ninput at_tick=4 c0=0x0 c1=0x0\n")},
    {"runner.parse.refuse.too-many-inputs", IVS_SCN_R_TOO_MANY_INPUTS, 68u, BLD(build_too_many_inputs)},
    {"runner.parse.refuse.mixed-input-unit", IVS_SCN_R_MIXED_INPUT_UNIT, 5u,
     TXT(HDR "input at_tick=5 c0=0x0 c1=0x0\ninput at_cycle=6 c0=0x0 c1=0x0\n")},
    {"runner.parse.refuse.bad-input-value", IVS_SCN_R_BAD_INPUT_VALUE, 4u,
     TXT(HDR "input at_tick=0 c0=KEY_1++KEY_2 c1=0x0\n")},
    /* Review WR-02: the first line that can never run after an expected refusal (here the later
     * role's content line, before the run-to). Review WR-03: no run-to and no expected refusal. */
    {"runner.parse.refuse.dead-after-refusal", IVS_SCN_R_DEAD_AFTER_REFUSAL, 5u,
     TXT(HDR "content exec boot-stub\ncontent cart a.ivsc\nexpect-load exec IVS_E_MALFORMED\nrun-to 1 ticks\n")},
    {"runner.parse.refuse.no-run-to", IVS_SCN_R_NO_RUN_TO, 0u, TXT(HDR "content exec boot-stub\n")},
    /* Plan 05-09: capture and the stream asserts (one case per new reason). */
    {"runner.parse.refuse.bad-capture", IVS_SCN_R_BAD_CAPTURE, 4u, TXT(HDR "capture video\n" RUN0)},
    {"runner.parse.refuse.duplicate-capture", IVS_SCN_R_DUPLICATE_CAPTURE, 5u,
     TXT(HDR "capture psglog\ncapture psglog\n" RUN0)},
    {"runner.parse.refuse.bad-psglog-assert", IVS_SCN_R_BAD_PSGLOG_ASSERT, 6u,
     TXT(HDR "capture psglog\n" RUN0 "assert psglog count == x\n")},
    {"runner.parse.refuse.bad-audio-assert", IVS_SCN_R_BAD_AUDIO_ASSERT, 6u,
     TXT(HDR "capture audio\n" RUN0 "assert audio at 16 A=1 B=2\n")},
    {"runner.parse.refuse.assert-without-capture", IVS_SCN_R_ASSERT_WITHOUT_CAPTURE, 5u,
     TXT(HDR RUN0 "assert psglog 0 addr=1\n")},
    /* Further refusals of covered tokens. */
    {"runner.parse.edge.capture-arity", IVS_SCN_R_BAD_ARITY, 4u, TXT(HDR "capture\n" RUN0)},
    {"runner.parse.edge.capture-extra-token", IVS_SCN_R_BAD_ARITY, 5u, TXT(HDR RUN0 "capture psglog extra\n")},
    {"runner.parse.edge.psglog-no-field", IVS_SCN_R_BAD_PSGLOG_ASSERT, 6u,
     TXT(HDR "capture psglog\n" RUN0 "assert psglog 0\n")},
    {"runner.parse.edge.psglog-field-twice", IVS_SCN_R_BAD_PSGLOG_ASSERT, 6u,
     TXT(HDR "capture psglog\n" RUN0 "assert psglog 0 addr=1 addr=2\n")},
    {"runner.parse.edge.psglog-addr-16", IVS_SCN_R_BAD_PSGLOG_ASSERT, 6u,
     TXT(HDR "capture psglog\n" RUN0 "assert psglog 0 addr=16\n")},
    {"runner.parse.edge.psglog-value-lower", IVS_SCN_R_BAD_PSGLOG_ASSERT, 6u,
     TXT(HDR "capture psglog\n" RUN0 "assert psglog 0 value=00ff\n")},
    {"runner.parse.edge.psglog-bad-source", IVS_SCN_R_BAD_PSGLOG_ASSERT, 6u,
     TXT(HDR "capture psglog\n" RUN0 "assert psglog 0 source=cpu\n")},
    {"runner.parse.edge.psglog-index-cap", IVS_SCN_R_BAD_PSGLOG_ASSERT, 6u,
     TXT(HDR "capture psglog\n" RUN0 "assert psglog 4096 addr=1\n")},
    {"runner.parse.edge.psglog-eight-tokens", IVS_SCN_R_BAD_PSGLOG_ASSERT, 6u,
     TXT(HDR "capture psglog\n" RUN0 "assert psglog 0 tick=1 addr=1 value=0001 source=cpu-dw extra\n")},
    {"runner.parse.edge.audio-at-missing-level", IVS_SCN_R_BAD_AUDIO_ASSERT, 6u,
     TXT(HDR "capture audio\n" RUN0 "assert audio at 16 A=1 C=2 B=3\n")},
    {"runner.parse.edge.audio-level-256", IVS_SCN_R_BAD_AUDIO_ASSERT, 6u,
     TXT(HDR "capture audio\n" RUN0 "assert audio at 16 A=256 B=0 C=0\n")},
    {"runner.parse.edge.audio-emitted-no-op", IVS_SCN_R_BAD_AUDIO_ASSERT, 6u,
     TXT(HDR "capture audio\n" RUN0 "assert audio emitted 3\n")},
    {"runner.parse.edge.audio-at-without-capture", IVS_SCN_R_ASSERT_WITHOUT_CAPTURE, 5u,
     TXT(HDR RUN0 "assert audio at 16 A=0 B=0 C=0\n")},
    {"runner.parse.edge.capture-after-assert-ok-order", IVS_SCN_R_ASSERT_WITHOUT_CAPTURE, 5u,
     TXT(HDR RUN0 "assert psglog 3 value=0001\ncapture audio\n")},
    {"runner.parse.edge.dead-run-to", IVS_SCN_R_DEAD_AFTER_REFUSAL, 6u,
     TXT(HDR "content cart a.ivsc\nexpect-load cart IVS_E_MALFORMED\nrun-to 1 ticks\nassert halted == yes\n")},
    {"runner.parse.edge.dead-input", IVS_SCN_R_DEAD_AFTER_REFUSAL, 4u,
     TXT(HDR "input at_tick=0 c0=0x0 c1=0x0\ncontent cart a.ivsc\nexpect-load cart IVS_E_MALFORMED\n")},
    {"runner.parse.edge.dead-second-refusal", IVS_SCN_R_DEAD_AFTER_REFUSAL, 6u,
     TXT(HDR "content exec boot-stub\nexpect-load exec IVS_E_MALFORMED\ncontent grom zero-grom\n"
             "expect-load grom IVS_E_MALFORMED\n")},
    {"runner.parse.edge.no-run-to-expect-ok", IVS_SCN_R_NO_RUN_TO, 0u,
     TXT(HDR "content cart a.ivsc\nexpect-load cart ok\n")},
    {"runner.parse.edge.no-run-to-inputs-only", IVS_SCN_R_NO_RUN_TO, 0u, TXT(HDR "input at_tick=0 c0=0x0 c1=0x0\n")},
    {"runner.parse.edge.empty-file", IVS_SCN_R_MISSING_HEADER, 0u, TXT("")},
    {"runner.parse.edge.comments-only", IVS_SCN_R_MISSING_HEADER, 0u, TXT("# a\n\n#b")},
    {"runner.parse.edge.number-overflow", IVS_SCN_R_BAD_NUMBER, 4u, TXT(HDR "run-to 18446744073709551616 ticks\n")},
    {"runner.parse.edge.number-empty", IVS_SCN_R_BAD_NUMBER, 4u, TXT(HDR "input at_tick= c0=0x0 c1=0x0\n")},
    {"runner.parse.edge.number-plus-sign", IVS_SCN_R_BAD_NUMBER, 4u, TXT(HDR "wall-timeout-ms +5\n")},
    {"runner.parse.edge.indented-hash", IVS_SCN_R_UNKNOWN_DIRECTIVE, 4u, TXT(HDR "  # not a comment\n")},
    {"runner.parse.edge.inline-hash", IVS_SCN_R_BAD_TOKEN, 4u, TXT(HDR "run-to 1 ticks # note\n")},
    {"runner.parse.edge.dotdot-inside", IVS_SCN_R_PATH_DOTDOT, 4u, TXT(HDR "content cart a/../../b.ivsc\n")},
    {"runner.parse.edge.trailing-slash", IVS_SCN_R_PATH_EMPTY_SEGMENT, 4u, TXT(HDR "content cart dir/\n")},
    {"runner.parse.edge.zero-grom-as-cart", IVS_SCN_R_BAD_CONTENT_KIND, 4u, TXT(HDR "content cart zero-grom\n")},
    {"runner.parse.edge.reason-with-ok", IVS_SCN_R_BAD_TOKEN, 5u, TXT(HDR "content cart a.ivsc\nexpect-load cart ok reason 1\n")},
    {"runner.parse.edge.reason-zero", IVS_SCN_R_OUT_OF_RANGE, 5u,
     TXT(HDR "content cart a.ivsc\nexpect-load cart IVS_E_MALFORMED reason 0\n")},
    {"runner.parse.edge.wall-zero", IVS_SCN_R_OUT_OF_RANGE, 4u, TXT(HDR "wall-timeout-ms 0\n")},
    {"runner.parse.edge.wall-too-big", IVS_SCN_R_OUT_OF_RANGE, 4u, TXT(HDR "wall-timeout-ms 4294967296\n")},
    {"runner.parse.edge.step-decreasing", IVS_SCN_R_STEPS_NOT_INCREASING, 5u, TXT(HDR "run-to 9 ticks\nrun-to 2 ticks\n")},
    {"runner.parse.edge.mem-range-reversed", IVS_SCN_R_OUT_OF_RANGE, 5u,
     TXT(HDR "run-to 1 ticks\nassert mem 0201..0200 == 0000\n")},
    {"runner.parse.edge.reg-r8", IVS_SCN_R_BAD_TOKEN, 5u, TXT(HDR "run-to 1 ticks\nassert reg R8 == 0000\n")},
    {"runner.parse.edge.input-hex-long", IVS_SCN_R_BAD_INPUT_VALUE, 4u, TXT(HDR "input at_tick=0 c0=0x000000001 c1=0x0\n")},
    {"runner.parse.edge.input-empty-word", IVS_SCN_R_BAD_INPUT_VALUE, 4u, TXT(HDR "input at_tick=0 c0= c1=0x0\n")},
    {"runner.parse.edge.input-prefix-name", IVS_SCN_R_UNKNOWN_INPUT_NAME, 4u,
     TXT(HDR "input at_tick=0 c0=IVS_CTRL_KEY_1 c1=0x0\n")},
    {"runner.parse.edge.time-max-plus-one-input", IVS_SCN_R_OUT_OF_RANGE, 4u,
     TXT(HDR "input at_tick=4611686018427387905 c0=0x0 c1=0x0\n")},
    {"runner.parse.edge.cr-at-header", IVS_SCN_R_CR, 1u, TXT("ivss 1\r\nid t\nprofile p\n")},
    {"runner.parse.edge.del-byte", IVS_SCN_R_CONTROL_BYTE, 2u, TXT("ivss 1\nid t\x7F\n")},
};

#define REFUSE_CASES (sizeof refuse_cases / sizeof refuse_cases[0])

static int run_refuse(const refuse_case *c)
{
  const char *text = c->text;
  size_t len = c->len;
  if (c->build != NULL) {
    len = c->build();
    text = build_buf;
    if (len == 0u) {
      ivs_test_diag("builder overflow");
      return IVS_TEST_FAIL;
    }
  }
  return parse_refused(text, len, c->reason, c->line) ? IVS_TEST_PASS : IVS_TEST_FAIL;
}

/* ---- the diagnostic scenario (plan 04-16 predicate, plan 04-19 writes the .ivss file) ---- */

static const char diag_boot_pass_text[] = "ivss 1\n"
                                          "id diag-boot-pass\n"
                                          "profile intv-2609-ntsc-family-p1\n"
                                          "content exec boot-stub\n"
                                          "content grom zero-grom\n"
                                          "content cart diag-cpu-1.ivsc\n"
                                          "run-to 4000 cpu-cycles stop halted\n"
#include "diag_cpu_1_expect_text.inc"
    ;

#define PD_HALTED 1u
#define PD_REG 2u
#define PD_MEM 3u
typedef struct pd_clause {
  uint32_t kind;
  uint16_t where; /* register number or address */
  uint16_t value;
} pd_clause;

#define IVS_EXPECT_HALTED(y) {PD_HALTED, 0u, (uint16_t)(y)},
#define IVS_EXPECT_REG(r, v) {PD_REG, (uint16_t)(r), (uint16_t)(v)},
#define IVS_EXPECT_MEM(a, v) {PD_MEM, (uint16_t)(a), (uint16_t)(v)},
static const pd_clause diag_clauses[] = {
#include "diag_cpu_1_expect.inc"
};
#undef IVS_EXPECT_HALTED
#undef IVS_EXPECT_REG
#undef IVS_EXPECT_MEM

static int parse_diag_boot_pass(void)
{
  ivs_scenario s;
  size_t n = sizeof diag_clauses / sizeof diag_clauses[0];
  size_t i;
  IVS_CHECK(parse_ok(diag_boot_pass_text, &s));
  IVS_CHECK(strcmp(s.id, "diag-boot-pass") == 0);
  IVS_CHECK(strcmp(s.profile, "intv-2609-ntsc-family-p1") == 0);
  IVS_CHECK(s.content[IVS_SCN_ROLE_EXEC].kind == IVS_SCN_CONTENT_BOOT_STUB);
  IVS_CHECK(s.content[IVS_SCN_ROLE_GROM].kind == IVS_SCN_CONTENT_ZERO_GROM);
  IVS_CHECK(s.content[IVS_SCN_ROLE_CART].kind == IVS_SCN_CONTENT_PATH);
  IVS_CHECK(strcmp(s.content[IVS_SCN_ROLE_CART].path, "diag-cpu-1.ivsc") == 0);
  IVS_CHECK(s.content[IVS_SCN_ROLE_CART].line == 6u);
  for (i = 0u; i < IVS_SCN_ROLES; i++) {
    IVS_CHECK(s.expect[i].load_set == 0u && s.expect[i].sha_set == 0u);
  }
  IVS_CHECK(s.wall_timeout_ms == 0u);
  IVS_CHECK(s.n_steps == 1u && s.step_unit == IVS_SCN_UNIT_CPU_CYCLES);
  IVS_CHECK(s.steps[0].target == 4000u && s.steps[0].unit == IVS_SCN_UNIT_CPU_CYCLES);
  IVS_CHECK(s.steps[0].stop == IVS_SCN_STOP_HALTED && s.steps[0].line == 7u);
  IVS_CHECK(s.n_inputs == 0u && s.input_unit == 0u);
  /* 18 clauses, each read the same way by the parser and by the cmake X-macro generator. */
  IVS_CHECK(n == 18u);
  IVS_CHECK(s.n_asserts == n);
  for (i = 0u; i < n; i++) {
    const ivs_scenario_assert *a = &s.asserts[i];
    const pd_clause *c = &diag_clauses[i];
    IVS_CHECK(a->step == 0u && a->op == IVS_SCN_OP_EQ && a->line == 8u + (uint32_t)i);
    if (c->kind == PD_HALTED) {
      IVS_CHECK(a->subject == IVS_SCN_SUBJ_HALTED && a->value == c->value);
    } else if (c->kind == PD_REG) {
      IVS_CHECK(a->subject == IVS_SCN_SUBJ_REG && a->index == c->where && a->value == c->value);
    } else {
      IVS_CHECK(a->subject == IVS_SCN_SUBJ_MEM && a->addr_lo == c->where && a->addr_hi == c->where);
      IVS_CHECK(a->value == c->value);
    }
  }
  return IVS_TEST_PASS;
}

/* ---- accepted scenarios, one per directive ---- */

static const char accept_ivss_text[] = "# comment before the header\n"
                                       "\n"
                                       "  \t \n"
                                       "ivss 1\n"
                                       "id t\n"
                                       "profile p\n"
                                       "run-to 0 ticks"; /* no final LF */

static int parse_accept_ivss(void)
{
  ivs_scenario s;
  uint32_t i;
  IVS_CHECK(parse_ok(accept_ivss_text, &s));
  IVS_CHECK(strcmp(s.id, "t") == 0 && strcmp(s.profile, "p") == 0);
  IVS_CHECK(s.n_steps == 1u && s.steps[0].target == 0u && s.steps[0].line == 7u);
  IVS_CHECK(s.n_asserts == 0u && s.n_inputs == 0u && s.wall_timeout_ms == 0u);
  for (i = 0u; i < IVS_SCN_ROLES; i++) {
    IVS_CHECK(s.content[i].kind == IVS_SCN_CONTENT_NONE);
  }
  return IVS_TEST_PASS;
}

static int parse_accept_id(void)
{
  ivs_scenario s;
  IVS_CHECK(parse_ok("ivss 1\nprofile p\n\tid   abcdefghijklmnopqrstuvwxyz-0123456789-abcdefghij  \n" RUN0, &s));
  IVS_CHECK(strlen(s.id) == IVS_SCN_MAX_ID);
  IVS_CHECK(strcmp(s.id, "abcdefghijklmnopqrstuvwxyz-0123456789-abcdefghij") == 0);
  return IVS_TEST_PASS;
}

static int parse_accept_profile(void)
{
  ivs_scenario s;
  IVS_CHECK(parse_ok("ivss 1\nid t\nprofile 0123456789-0123456789-0123456789-0123456789-0123456789-012345678\n" RUN0, &s));
  IVS_CHECK(strlen(s.profile) == IVS_SCN_MAX_PROFILE);
  return IVS_TEST_PASS;
}

static int parse_accept_content(void)
{
  ivs_scenario s;
  IVS_CHECK(parse_ok(HDR "content cart sub_1/x-y.Z9/diag.ivsc\ncontent exec user\ncontent grom user\n" RUN0, &s));
  IVS_CHECK(s.content[IVS_SCN_ROLE_CART].kind == IVS_SCN_CONTENT_PATH);
  IVS_CHECK(strcmp(s.content[IVS_SCN_ROLE_CART].path, "sub_1/x-y.Z9/diag.ivsc") == 0);
  IVS_CHECK(s.content[IVS_SCN_ROLE_CART].line == 4u);
  IVS_CHECK(s.content[IVS_SCN_ROLE_EXEC].kind == IVS_SCN_CONTENT_USER && s.content[IVS_SCN_ROLE_EXEC].line == 5u);
  IVS_CHECK(s.content[IVS_SCN_ROLE_GROM].kind == IVS_SCN_CONTENT_USER && s.content[IVS_SCN_ROLE_GROM].line == 6u);
  IVS_CHECK(s.content[IVS_SCN_ROLE_EXEC].path[0] == '\0');
  IVS_CHECK(parse_ok(HDR "content exec boot-stub\ncontent grom zero-grom\ncontent cart ./a.b\n" RUN0, &s));
  IVS_CHECK(s.content[IVS_SCN_ROLE_EXEC].kind == IVS_SCN_CONTENT_BOOT_STUB);
  IVS_CHECK(s.content[IVS_SCN_ROLE_GROM].kind == IVS_SCN_CONTENT_ZERO_GROM);
  IVS_CHECK(strcmp(s.content[IVS_SCN_ROLE_CART].path, "./a.b") == 0);
  return IVS_TEST_PASS;
}

static int parse_accept_expect_load(void)
{
  ivs_scenario s;
  IVS_CHECK(parse_ok(HDR "content cart a.ivsc\ncontent exec user\nexpect-load cart IVS_E_MALFORMED reason 7\n"
                         "expect-load exec ok\n",
                     &s));
  IVS_CHECK(s.expect[IVS_SCN_ROLE_CART].load_set == 1u && s.expect[IVS_SCN_ROLE_CART].load_line == 6u);
  IVS_CHECK(s.expect[IVS_SCN_ROLE_CART].load_status == IVS_E_MALFORMED);
  IVS_CHECK(s.expect[IVS_SCN_ROLE_CART].load_reason == 7u);
  IVS_CHECK(s.expect[IVS_SCN_ROLE_EXEC].load_set == 1u && s.expect[IVS_SCN_ROLE_EXEC].load_status == IVS_OK);
  IVS_CHECK(s.expect[IVS_SCN_ROLE_EXEC].load_reason == 0u);
  IVS_CHECK(s.expect[IVS_SCN_ROLE_GROM].load_set == 0u);
  IVS_CHECK(parse_ok(HDR "content cart a.ivsc\nexpect-load cart IVS_E_DIGEST_MISMATCH\n", &s));
  IVS_CHECK(s.expect[IVS_SCN_ROLE_CART].load_status == IVS_E_DIGEST_MISMATCH);
  IVS_CHECK(s.expect[IVS_SCN_ROLE_CART].load_reason == 0u);
  return IVS_TEST_PASS;
}

/* Review WR-02: an expected refusal of the last loaded role, with the earlier roles loaded (one
 * expecting ok), an expect-sha256 on the refused role and a wall cap: every line can take effect,
 * accepted without a run-to. */
static int parse_accept_refusal_last_role(void)
{
  ivs_scenario s;
  IVS_CHECK(parse_ok(HDR "wall-timeout-ms 5\ncontent exec boot-stub\ncontent grom zero-grom\nexpect-load exec ok\n"
                         "content cart a.ivsc\n"
                         "expect-sha256 cart ba7816bf8f01cfea414140de5dae2223b00361a396177a9cb410ff61f20015ad\n"
                         "expect-load cart IVS_E_DIGEST_MISMATCH\n",
                     &s));
  IVS_CHECK(s.n_steps == 0u && s.n_inputs == 0u && s.wall_timeout_ms == 5u);
  IVS_CHECK(s.expect[IVS_SCN_ROLE_CART].load_status == IVS_E_DIGEST_MISMATCH && s.expect[IVS_SCN_ROLE_CART].sha_set);
  /* The refused role first in load order, the later roles absent. */
  IVS_CHECK(parse_ok(HDR "content exec a.bin\nexpect-load exec IVS_E_MALFORMED reason 2\n", &s));
  IVS_CHECK(s.n_steps == 0u && s.content[IVS_SCN_ROLE_GROM].kind == IVS_SCN_CONTENT_NONE);
  return IVS_TEST_PASS;
}

static int parse_accept_expect_sha256(void)
{
  static const uint8_t want[32] = {0xba, 0x78, 0x16, 0xbf, 0x8f, 0x01, 0xcf, 0xea, 0x41, 0x41, 0x40,
                                   0xde, 0x5d, 0xae, 0x22, 0x23, 0xb0, 0x03, 0x61, 0xa3, 0x96, 0x17,
                                   0x7a, 0x9c, 0xb4, 0x10, 0xff, 0x61, 0xf2, 0x00, 0x15, 0xad};
  ivs_scenario s;
  IVS_CHECK(parse_ok(HDR "content grom zero-grom\n"
                         "expect-sha256 grom ba7816bf8f01cfea414140de5dae2223b00361a396177a9cb410ff61f20015ad\n" RUN0,
                     &s));
  IVS_CHECK(s.expect[IVS_SCN_ROLE_GROM].sha_set == 1u && s.expect[IVS_SCN_ROLE_GROM].sha_line == 5u);
  IVS_CHECK(memcmp(s.expect[IVS_SCN_ROLE_GROM].sha256, want, 32u) == 0);
  return IVS_TEST_PASS;
}

static int parse_accept_wall_timeout(void)
{
  ivs_scenario s;
  IVS_CHECK(parse_ok(HDR "wall-timeout-ms 4294967295\n" RUN0, &s));
  IVS_CHECK(s.wall_timeout_ms == 4294967295u);
  IVS_CHECK(parse_ok(HDR "wall-timeout-ms 0001\n" RUN0, &s));
  IVS_CHECK(s.wall_timeout_ms == 1u);
  return IVS_TEST_PASS;
}

static int parse_accept_run_to(void)
{
  ivs_scenario s;
  IVS_CHECK(parse_ok(HDR "run-to 0 frames\nrun-to 1 frames stop target\nrun-to 2 frames stop halted\n"
                         "run-to 4611686018427387904 frames stop faulted\n",
                     &s));
  IVS_CHECK(s.n_steps == 4u && s.step_unit == IVS_SCN_UNIT_FRAMES);
  IVS_CHECK(s.steps[0].target == 0u && s.steps[0].stop == IVS_SCN_STOP_TARGET && s.steps[0].line == 4u);
  IVS_CHECK(s.steps[1].target == 1u && s.steps[1].stop == IVS_SCN_STOP_TARGET);
  IVS_CHECK(s.steps[2].stop == IVS_SCN_STOP_HALTED);
  IVS_CHECK(s.steps[3].target == IVS_SCN_TIME_MAX && s.steps[3].stop == IVS_SCN_STOP_FAULTED);
  IVS_CHECK(parse_ok(HDR "run-to 10 ticks\n", &s));
  IVS_CHECK(s.steps[0].unit == IVS_SCN_UNIT_TICKS);
  return IVS_TEST_PASS;
}

static int parse_accept_assert(void)
{
  ivs_scenario s;
  const ivs_scenario_assert *a;
  IVS_CHECK(parse_ok(HDR "run-to 100 ticks\n"
                         "assert halted != no\n"
                         "assert reg R7 != FFFF\n"
                         "run-to 200 ticks\n"
                         "assert mem 0200..020F == 0000\n"
                         "assert mem 0300..0300 != 1234\n"
                         "assert digest input == e3b0c44298fc1c149afbf4c8996fb92427ae41e4649b934ca495991b7852b855\n"
                         "assert digest composite != 0000000000000000000000000000000000000000000000000000000000000000\n",
                     &s));
  IVS_CHECK(s.n_asserts == 6u);
  a = &s.asserts[0];
  IVS_CHECK(a->step == 0u && a->subject == IVS_SCN_SUBJ_HALTED && a->op == IVS_SCN_OP_NE && a->value == 0u);
  IVS_CHECK(a->line == 5u);
  a = &s.asserts[1];
  IVS_CHECK(a->step == 0u && a->subject == IVS_SCN_SUBJ_REG && a->index == 7u && a->value == 0xFFFFu);
  a = &s.asserts[2];
  IVS_CHECK(a->step == 1u && a->subject == IVS_SCN_SUBJ_MEM_RANGE && a->op == IVS_SCN_OP_EQ);
  IVS_CHECK(a->addr_lo == 0x0200u && a->addr_hi == 0x020Fu && a->value == 0u);
  a = &s.asserts[3];
  IVS_CHECK(a->subject == IVS_SCN_SUBJ_MEM_RANGE && a->addr_lo == 0x0300u && a->addr_hi == 0x0300u);
  IVS_CHECK(a->value == 0x1234u && a->op == IVS_SCN_OP_NE);
  a = &s.asserts[4];
  IVS_CHECK(a->subject == IVS_SCN_SUBJ_DIGEST && a->index == IVS_DIGEST_INPUT && a->sha256[0] == 0xe3u);
  IVS_CHECK(a->sha256[31] == 0x55u);
  a = &s.asserts[5];
  IVS_CHECK(a->subject == IVS_SCN_SUBJ_DIGEST && a->index == IVS_DIGEST_COMPOSITE && a->op == IVS_SCN_OP_NE);
  IVS_CHECK(a->line == 11u);
  return IVS_TEST_PASS;
}

/* Plan 05-09: the capture directive and the stream asserts, every field checked. The capture
 * directive may stand anywhere after the header, also after the assert that reads it. */
static int parse_accept_capture_forms(void)
{
  ivs_scenario s;
  const ivs_scenario_assert *a;
  IVS_CHECK(parse_ok(HDR "run-to 100 ticks\n"
                         "assert psglog count == 4\n"
                         "assert psglog count != 0\n"
                         "assert psglog 3 tick=18446744073709551615 addr=15 value=FFFF source=cpu-dw\n"
                         "assert psglog 0 source=cpu-write\n"
                         "assert audio emitted != 7\n"
                         "assert audio at 160 A=15 B=0 C=255\n"
                         "capture audio\n"
                         "assert digest psglog == e3b0c44298fc1c149afbf4c8996fb92427ae41e4649b934ca495991b7852b855\n"
                         "capture psglog\n",
                     &s));
  IVS_CHECK(s.capture == (IVS_SCN_CAPTURE_PSGLOG | IVS_SCN_CAPTURE_AUDIO));
  IVS_CHECK(s.capture_line[1] == 11u && s.capture_line[0] == 13u);
  IVS_CHECK(s.n_asserts == 7u);
  a = &s.asserts[0];
  IVS_CHECK(a->subject == IVS_SCN_SUBJ_PSGLOG && a->form == IVS_SCN_FORM_COUNT && a->op == IVS_SCN_OP_EQ && a->num == 4u);
  a = &s.asserts[1];
  IVS_CHECK(a->form == IVS_SCN_FORM_COUNT && a->op == IVS_SCN_OP_NE && a->num == 0u);
  a = &s.asserts[2];
  IVS_CHECK(a->form == IVS_SCN_FORM_RECORD && a->index == 3u && a->num == UINT64_MAX && a->addr == 15u);
  IVS_CHECK(a->value == 0xFFFFu && a->source == IVS_PSGLOG_SRC_CPU_DW);
  IVS_CHECK(a->fields == (IVS_SCN_FIELD_TICK | IVS_SCN_FIELD_ADDR | IVS_SCN_FIELD_VALUE | IVS_SCN_FIELD_SOURCE));
  a = &s.asserts[3];
  IVS_CHECK(a->index == 0u && a->fields == IVS_SCN_FIELD_SOURCE && a->source == IVS_PSGLOG_SRC_CPU_WRITE);
  a = &s.asserts[4];
  IVS_CHECK(a->subject == IVS_SCN_SUBJ_AUDIO && a->form == IVS_SCN_FORM_EMITTED && a->op == IVS_SCN_OP_NE &&
            a->num == 7u);
  a = &s.asserts[5];
  IVS_CHECK(a->form == IVS_SCN_FORM_AT && a->num == 160u && a->level[0] == 15u && a->level[1] == 0u &&
            a->level[2] == 255u && a->fields == 0x7u);
  a = &s.asserts[6];
  IVS_CHECK(a->subject == IVS_SCN_SUBJ_DIGEST && a->index == IVS_DIGEST_PSGLOG);
  /* Field order is free; the count and emitted forms need no capture. */
  IVS_CHECK(parse_ok(HDR RUN0 "assert psglog 5 source=cpu-dw value=0001 tick=9\ncapture psglog\n", &s));
  IVS_CHECK(s.asserts[0].fields == (IVS_SCN_FIELD_TICK | IVS_SCN_FIELD_VALUE | IVS_SCN_FIELD_SOURCE));
  IVS_CHECK(parse_ok(HDR RUN0 "assert psglog count == 0\nassert audio emitted == 0\n", &s));
  IVS_CHECK(s.capture == 0u && s.capture_line[0] == 0u && s.capture_line[1] == 0u);
  return IVS_TEST_PASS;
}

static int parse_accept_input(void)
{
  ivs_scenario s;
  IVS_CHECK(parse_ok(HDR "input at_cycle=0 c0=KEY_1+DISC_N+SIDE_TOP c1=0x0\n"
                         "run-to 5000 cpu-cycles\n"
                         "input at_cycle=10 c0=0x7fffFFFF c1=KEY_ENTER\n"
                         "input at_cycle=10 c0=0x0 c1=SIDE_LOWER_RIGHT+SIDE_LOWER_RIGHT\n",
                     &s));
  IVS_CHECK(s.n_inputs == 3u && s.input_unit == IVS_SCN_INPUT_AT_CYCLE);
  IVS_CHECK(s.inputs[0].at == 0u && s.inputs[0].line == 4u);
  IVS_CHECK(s.inputs[0].controller[0] == (IVS_CTRL_KEY_1 | IVS_CTRL_DISC_N | IVS_CTRL_SIDE_TOP));
  IVS_CHECK(s.inputs[0].controller[1] == 0u);
  IVS_CHECK(s.inputs[1].at == 10u && s.inputs[1].controller[0] == 0x7FFFFFFFu);
  IVS_CHECK(s.inputs[1].controller[1] == IVS_CTRL_KEY_ENTER);
  IVS_CHECK(s.inputs[2].controller[1] == IVS_CTRL_SIDE_LOWER_RIGHT);
  IVS_CHECK(parse_ok(HDR "input at_tick=4611686018427387904 c0=0xA c1=0x0\n" RUN0, &s));
  IVS_CHECK(s.input_unit == IVS_SCN_INPUT_AT_TICK && s.inputs[0].at == IVS_SCN_TIME_MAX);
  IVS_CHECK(s.inputs[0].controller[0] == 0xAu);
  return IVS_TEST_PASS;
}

/* Every cap exactly at its limit is accepted (the refuse cases hold limit + 1). */
static int parse_accept_caps(void)
{
  ivs_scenario s;
  size_t n;
  n = build_size(IVS_SCN_MAX_BYTES);
  IVS_CHECK(n == IVS_SCN_MAX_BYTES && parse_ok_n(build_buf, n, &s));
  n = build_lines(IVS_SCN_MAX_LINES);
  IVS_CHECK(n > 0u && parse_ok_n(build_buf, n, &s));
  n = build_line_of(IVS_SCN_MAX_LINE_BYTES);
  IVS_CHECK(n > 0u && parse_ok_n(build_buf, n, &s));
  n = build_path_of(IVS_SCN_MAX_PATH);
  IVS_CHECK(n > 0u && parse_ok_n(build_buf, n, &s));
  IVS_CHECK(strlen(s.content[IVS_SCN_ROLE_CART].path) == IVS_SCN_MAX_PATH);
  n = build_steps(IVS_SCN_MAX_STEPS);
  IVS_CHECK(n > 0u && parse_ok_n(build_buf, n, &s) && s.n_steps == IVS_SCN_MAX_STEPS);
  IVS_CHECK(s.steps[IVS_SCN_MAX_STEPS - 1u].target == IVS_SCN_MAX_STEPS);
  n = build_asserts(IVS_SCN_MAX_ASSERTS);
  IVS_CHECK(n > 0u && parse_ok_n(build_buf, n, &s) && s.n_asserts == IVS_SCN_MAX_ASSERTS);
  n = build_inputs(IVS_SCN_MAX_INPUTS);
  IVS_CHECK(n > 0u && parse_ok_n(build_buf, n, &s) && s.n_inputs == IVS_SCN_MAX_INPUTS);
  return IVS_TEST_PASS;
}

/* ---- input names against the pin file ---- */

typedef struct pin_name {
  const char *name;
  uint32_t bit;
  uint32_t header_mask; /* the IVS_CTRL_ define of the public header for the same name */
} pin_name;

#define IVS_INPUT_BIT(n, b) {#n, b, IVS_CTRL_##n},
static const pin_name pinned_names[] = {
#include "ivs_input_bits.inc"
};
#undef IVS_INPUT_BIT

static int parse_input_names_complete(void)
{
  size_t n = sizeof pinned_names / sizeof pinned_names[0];
  uint32_t count = ivs_scenario_input_name_count();
  size_t i;
  uint32_t k;
  IVS_CHECK(n == 31u);
  IVS_CHECK(count == (uint32_t)n);
  /* Each pinned name parses to its pinned bit, through a scenario. */
  for (i = 0u; i < n; i++) {
    ivs_scenario s;
    sb b = {0u, 0};
    IVS_CHECK(pinned_names[i].header_mask == (1u << pinned_names[i].bit));
    sb_add(&b, HDR "input at_tick=0 c0=0x0 c1=");
    sb_add(&b, pinned_names[i].name);
    sb_add(&b, "\n" RUN0);
    IVS_CHECK(!b.overflow && parse_ok_n(build_buf, b.n, &s));
    if (s.inputs[0].controller[1] != (1u << pinned_names[i].bit)) {
      ivs_test_diag("name %s parses to 0x%08X", pinned_names[i].name, (unsigned)s.inputs[0].controller[1]);
      return IVS_TEST_FAIL;
    }
  }
  /* Each table entry is a pinned name with the same bit, and no entry repeats. */
  for (k = 0u; k < count; k++) {
    uint32_t mask = 0u;
    const char *nm = ivs_scenario_input_name(k, &mask);
    uint32_t hits = 0u;
    IVS_CHECK(nm != NULL);
    for (i = 0u; i < n; i++) {
      if (strcmp(nm, pinned_names[i].name) == 0) {
        IVS_CHECK(mask == (1u << pinned_names[i].bit));
        hits++;
      }
    }
    if (hits != 1u) {
      ivs_test_diag("table name %s matches %u pinned names", nm, (unsigned)hits);
      return IVS_TEST_FAIL;
    }
    for (i = 0u; i < k; i++) {
      IVS_CHECK(strcmp(ivs_scenario_input_name((uint32_t)i, NULL), nm) != 0);
    }
  }
  {
    uint32_t mask = 1u;
    IVS_CHECK(ivs_scenario_input_name(count, &mask) == NULL && mask == 0u);
  }
  return IVS_TEST_PASS;
}

/* ---- reason names, error format, NULL arguments ---- */

static int parse_reasons_named(void)
{
  uint32_t r;
  size_t i;
  for (r = 1u; r < IVS_SCN_R_COUNT; r++) {
    const char *nm = ivs_scenario_reason_name(r);
    const char *c;
    uint32_t q;
    uint32_t covered = 0u;
    IVS_CHECK(nm != NULL && nm[0] != '\0' && strcmp(nm, "unknown") != 0 && strcmp(nm, "none") != 0);
    for (c = nm; *c != '\0'; c++) {
      IVS_CHECK((*c >= 'a' && *c <= 'z') || (*c >= '0' && *c <= '9') || *c == '-');
    }
    for (q = 1u; q < r; q++) {
      IVS_CHECK(strcmp(ivs_scenario_reason_name(q), nm) != 0);
    }
    /* The refuse case named after the token exists and refuses with this reason. */
    for (i = 0u; i < REFUSE_CASES; i++) {
      const char *id = refuse_cases[i].id;
      size_t pre = strlen("runner.parse.refuse.");
      if (strncmp(id, "runner.parse.refuse.", pre) == 0 && strcmp(id + pre, nm) == 0) {
        IVS_CHECK(refuse_cases[i].reason == r);
        covered++;
      }
    }
    if (covered != 1u) {
      ivs_test_diag("reason %s has %u refuse cases", nm, (unsigned)covered);
      return IVS_TEST_FAIL;
    }
  }
  IVS_CHECK(strcmp(ivs_scenario_reason_name(IVS_SCN_R_NONE), "none") == 0);
  IVS_CHECK(strcmp(ivs_scenario_reason_name(IVS_SCN_R_COUNT), "unknown") == 0);
  IVS_CHECK(strcmp(ivs_scenario_reason_name(0xFFFFFFFFu), "unknown") == 0);
  return IVS_TEST_PASS;
}

static int parse_error_format(void)
{
  ivs_scenario s;
  ivs_scenario_error e;
  char buf[64];
  const char *t = HDR "content cart ../x.ivsc\n";
  IVS_CHECK(parse_exact(t, strlen(t), &s, &e) == (int)IVS_SCN_R_PATH_DOTDOT);
  IVS_CHECK(ivs_scenario_error_format(&e, buf, sizeof buf) == 0);
  IVS_CHECK(strcmp(buf, "scenario-syntax line=4 reason=path-dotdot") == 0);
  IVS_CHECK(ivs_scenario_error_format(&e, buf, 10u) == -1 && buf[0] == '\0');
  IVS_CHECK(ivs_scenario_error_format(NULL, buf, sizeof buf) == -1 && buf[0] == '\0');
  IVS_CHECK(ivs_scenario_error_format(&e, NULL, 8u) == -1);
  return IVS_TEST_PASS;
}

static int parse_null_args(void)
{
  ivs_scenario s;
  ivs_scenario_error e;
  const uint8_t one = (uint8_t)'i';
  memset(&s, 0x5A, sizeof s);
  e.line = 7u;
  e.reason = 7u;
  IVS_CHECK(ivs_scenario_parse(&one, 1u, NULL, &e) == -1 && e.line == 7u && e.reason == 7u);
  IVS_CHECK(ivs_scenario_parse(&one, 1u, &s, NULL) == -1);
  IVS_CHECK(ivs_scenario_parse(NULL, 1u, &s, &e) == -1);
  IVS_CHECK(ivs_scenario_parse(NULL, 0u, &s, &e) == (int)IVS_SCN_R_MISSING_HEADER && e.line == 0u);
  return IVS_TEST_PASS;
}

/* ---- case table ---- */

typedef struct fn_case {
  const char *id;
  int (*fn)(void);
} fn_case;

static const fn_case fn_cases[] = {
    {"runner.parse.diag-boot-pass", parse_diag_boot_pass},
    {"runner.parse.accept.ivss", parse_accept_ivss},
    {"runner.parse.accept.id", parse_accept_id},
    {"runner.parse.accept.profile", parse_accept_profile},
    {"runner.parse.accept.content", parse_accept_content},
    {"runner.parse.accept.expect-load", parse_accept_expect_load},
    {"runner.parse.accept.expect-sha256", parse_accept_expect_sha256},
    {"runner.parse.accept.wall-timeout-ms", parse_accept_wall_timeout},
    {"runner.parse.accept.run-to", parse_accept_run_to},
    {"runner.parse.accept.assert", parse_accept_assert},
    {"runner.parse.accept.input", parse_accept_input},
    {"runner.parse.accept.capture-forms", parse_accept_capture_forms},
    {"runner.parse.accept.caps", parse_accept_caps},
    {"runner.parse.accept.refusal-last-role", parse_accept_refusal_last_role},
    {"runner.parse.input-names-complete", parse_input_names_complete},
    {"runner.parse.reasons-named", parse_reasons_named},
    {"runner.parse.error-format", parse_error_format},
    {"runner.parse.null-args", parse_null_args},
};

#define FN_CASES (sizeof fn_cases / sizeof fn_cases[0])

size_t runner_parse_count(void)
{
  return FN_CASES + REFUSE_CASES;
}

const char *runner_parse_id(size_t i)
{
  if (i < FN_CASES) {
    return fn_cases[i].id;
  }
  i -= FN_CASES;
  return i < REFUSE_CASES ? refuse_cases[i].id : NULL;
}

int runner_parse_run(size_t i)
{
  if (i < FN_CASES) {
    return fn_cases[i].fn();
  }
  i -= FN_CASES;
  return i < REFUSE_CASES ? run_refuse(&refuse_cases[i]) : IVS_TEST_FAIL;
}

/* Mutation bases for the fuzz family: accepted scenarios covering every directive. */
static const char fuzz_base_rich[] =
    "# rich base\n"
    "ivss 1\n"
    "id rich-1\n"
    "profile intv-2609-ntsc-family-p1\n"
    "content exec boot-stub\n"
    "content grom zero-grom\n"
    "content cart dir/sub_1/diag-cpu-1.ivsc\n"
    "expect-load cart IVS_E_MALFORMED reason 3\n"
    "expect-sha256 cart ba7816bf8f01cfea414140de5dae2223b00361a396177a9cb410ff61f20015ad\n"
    "wall-timeout-ms 30000\n"
    "input at_tick=0 c0=KEY_1+DISC_NE c1=0x0\n"
    "run-to 1000 ticks\n"
    "assert halted == no\n"
    "assert reg R3 != 0037\n"
    "input at_tick=1500 c0=0x1000 c1=SIDE_TOP\n"
    "run-to 4000 ticks stop halted\n"
    "assert mem 0200..020F == 0000\n"
    "assert mem 0200 == 7FFF\n"
    "assert digest input == e3b0c44298fc1c149afbf4c8996fb92427ae41e4649b934ca495991b7852b855\n";

/* Plan 05-09: a base with both captures and every new assert form. */
static const char fuzz_base_capture[] =
    "# capture base\n"
    "ivss 1\n"
    "id capture-1\n"
    "profile intv-2609-ntsc-family-p1\n"
    "content exec boot-stub\n"
    "content grom zero-grom\n"
    "capture psglog\n"
    "capture audio\n"
    "run-to 3000 ticks\n"
    "assert psglog count == 4\n"
    "assert psglog 0 tick=100 addr=8 value=003F source=cpu-write\n"
    "assert psglog 3 addr=11 value=0007\n"
    "assert audio emitted != 0\n"
    "assert audio at 160 A=15 B=0 C=0\n"
    "assert digest psglog != e3b0c44298fc1c149afbf4c8996fb92427ae41e4649b934ca495991b7852b855\n"
    "assert digest audio == e3b0c44298fc1c149afbf4c8996fb92427ae41e4649b934ca495991b7852b855\n";

int runner_parse_base(size_t i, const char **text, size_t *len)
{
  switch (i) {
  case 0u:
    *text = diag_boot_pass_text;
    *len = sizeof diag_boot_pass_text - 1u;
    return 1;
  case 1u:
    *text = fuzz_base_rich;
    *len = sizeof fuzz_base_rich - 1u;
    return 1;
  case 2u:
    *text = accept_ivss_text;
    *len = sizeof accept_ivss_text - 1u;
    return 1;
  case 3u:
    *text = fuzz_base_capture;
    *len = sizeof fuzz_base_capture - 1u;
    return 1;
  default:
    return 0;
  }
}

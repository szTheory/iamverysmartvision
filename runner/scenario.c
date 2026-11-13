/* SPDX-License-Identifier: MIT */
/* IVSS v1 scenario parser (D-18, D-22, plan 04-17); the grammar is in ivs_runner.h. A scenario
 * is untrusted inert data, so this TU is a pure bytes-in, struct-out function: the length cap is
 * checked first, every read is bounded by the line it belongs to, the first failing check in
 * file order wins, and a refusal leaves the output all zero. Integers and ASCII classes are
 * hand-written (no strtol, sscanf or ctype: locale and overflow hazards). No allocation, no
 * state: tokens are slices of the input, and only length-checked values are copied.
 * Named-fault sites (data/expected/faults/runner.ids, plan 04-17, D-22): the `..` path rule
 * (threat T-04-49) and the 256-byte line cap (T-04-50), each removable by one fault id. */
#include <stddef.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>

#include "ivs/ivs.h"
#include "ivs_runner.h"

#define SCN_TOKS 9u /* longest directive has 8 tokens (assert frame pixel); a 9th marks "too many" */

typedef struct scn_tok {
  const uint8_t *p;
  size_t n;
} scn_tok;

typedef struct scn_line {
  scn_tok t[SCN_TOKS];
  uint32_t n; /* tokens found, capped at SCN_TOKS */
} scn_line;

/* ---- small lexical helpers ---- */

static int tok_is(scn_tok t, const char *s)
{
  size_t n = strlen(s);
  return t.n == n && memcmp(t.p, s, n) == 0;
}

static int tok_starts(scn_tok t, const char *s)
{
  size_t n = strlen(s);
  return t.n >= n && memcmp(t.p, s, n) == 0;
}

static scn_tok tok_tail(scn_tok t, size_t k)
{
  scn_tok r;
  r.p = t.p + k;
  r.n = t.n - k;
  return r;
}

static int is_digit(uint8_t c)
{
  return c >= (uint8_t)'0' && c <= (uint8_t)'9';
}

static int is_lower(uint8_t c)
{
  return c >= (uint8_t)'a' && c <= (uint8_t)'z';
}

static int is_upper(uint8_t c)
{
  return c >= (uint8_t)'A' && c <= (uint8_t)'Z';
}

/* Hex digit value of c, or -1; upper-case letters always, lower-case only when lower is set. */
static int hex_val(uint8_t c, int upper, int lower)
{
  if (is_digit(c)) {
    return (int)(c - (uint8_t)'0');
  }
  if (upper && c >= (uint8_t)'A' && c <= (uint8_t)'F') {
    return (int)(c - (uint8_t)'A') + 10;
  }
  if (lower && c >= (uint8_t)'a' && c <= (uint8_t)'f') {
    return (int)(c - (uint8_t)'a') + 10;
  }
  return -1;
}

/* Decimal digits only: no sign, no space, at least one digit, no uint64 overflow. */
static uint32_t parse_dec(scn_tok t, uint64_t *v)
{
  uint64_t x = 0u;
  size_t i;
  if (t.n == 0u) {
    return IVS_SCN_R_BAD_NUMBER;
  }
  for (i = 0u; i < t.n; i++) {
    uint64_t d;
    if (!is_digit(t.p[i])) {
      return IVS_SCN_R_BAD_NUMBER;
    }
    d = (uint64_t)(t.p[i] - (uint8_t)'0');
    if (x > (UINT64_MAX - d) / 10u) {
      return IVS_SCN_R_BAD_NUMBER;
    }
    x = x * 10u + d;
  }
  *v = x;
  return IVS_SCN_R_NONE;
}

/* Exactly four upper-case hex digits. */
static int parse_hex4(scn_tok t, uint16_t *v)
{
  uint32_t x = 0u;
  size_t i;
  if (t.n != 4u) {
    return 0;
  }
  for (i = 0u; i < 4u; i++) {
    int d = hex_val(t.p[i], 1, 0);
    if (d < 0) {
      return 0;
    }
    x = (x << 4) | (uint32_t)d;
  }
  *v = (uint16_t)x;
  return 1;
}

/* Exactly 64 lower-case hex digits into 32 bytes. */
static int parse_sha256(scn_tok t, uint8_t out[32])
{
  size_t i;
  if (t.n != 64u) {
    return 0;
  }
  for (i = 0u; i < 32u; i++) {
    int hi = hex_val(t.p[2u * i], 0, 1);
    int lo = hex_val(t.p[2u * i + 1u], 0, 1);
    if (hi < 0 || lo < 0) {
      return 0;
    }
    out[i] = (uint8_t)((hi << 4) | lo);
  }
  return 1;
}

/* A slug [a-z0-9-]{1,max} copied with a NUL into dst (max + 1 bytes). */
static int copy_slug(scn_tok t, char *dst, size_t max)
{
  size_t i;
  if (t.n == 0u || t.n > max) {
    return 0;
  }
  for (i = 0u; i < t.n; i++) {
    uint8_t c = t.p[i];
    if (!is_lower(c) && !is_digit(c) && c != (uint8_t)'-') {
      return 0;
    }
  }
  memcpy(dst, t.p, t.n);
  dst[t.n] = '\0';
  return 1;
}

/* The relative content path rule (D-22): [A-Za-z0-9._/-]{1,200}, no leading `/`, no `..`
 * anywhere, no empty segment (`//` or a trailing `/`). Returns a reason or NONE. */
static uint32_t check_path(scn_tok t)
{
  size_t i;
  if (t.n > IVS_SCN_MAX_PATH) {
    return IVS_SCN_R_PATH_TOO_LONG;
  }
  if (t.p[0] == (uint8_t)'/') {
    return IVS_SCN_R_PATH_ABSOLUTE;
  }
  for (i = 0u; i < t.n; i++) {
    uint8_t c = t.p[i];
    if (!is_lower(c) && !is_upper(c) && !is_digit(c) && c != (uint8_t)'.' && c != (uint8_t)'_' &&
        c != (uint8_t)'/' && c != (uint8_t)'-') {
      return IVS_SCN_R_PATH_BYTE;
    }
  }
#ifndef IVS_FAULT_RUNNER_DOTDOT_ACCEPTED
  for (i = 0u; i + 1u < t.n; i++) {
    if (t.p[i] == (uint8_t)'.' && t.p[i + 1u] == (uint8_t)'.') {
      return IVS_SCN_R_PATH_DOTDOT;
    }
  }
#endif
  for (i = 0u; i < t.n; i++) {
    if (t.p[i] == (uint8_t)'/' && (i + 1u == t.n || t.p[i + 1u] == (uint8_t)'/')) {
      return IVS_SCN_R_PATH_EMPTY_SEGMENT;
    }
  }
  return IVS_SCN_R_NONE;
}

static uint32_t parse_role(scn_tok t, uint32_t *role)
{
  if (tok_is(t, "exec")) {
    *role = IVS_SCN_ROLE_EXEC;
  } else if (tok_is(t, "grom")) {
    *role = IVS_SCN_ROLE_GROM;
  } else if (tok_is(t, "cart")) {
    *role = IVS_SCN_ROLE_CART;
  } else {
    return IVS_SCN_R_BAD_TOKEN;
  }
  return IVS_SCN_R_NONE;
}

/* ---- input names: the IVS_CTRL_* defines of ivs/ivs_run.h minus the prefix ---- */

#define SCN_INPUT_NAMES 31u
#define SCN_NAME(i, n)                                                                                 \
  case i:                                                                                              \
    *mask = IVS_CTRL_##n;                                                                              \
    return #n;

const char *ivs_scenario_input_name(uint32_t i, uint32_t *mask)
{
  uint32_t dummy = 0u;
  if (mask == NULL) {
    mask = &dummy;
  }
  switch (i) {
    SCN_NAME(0u, KEY_1)
    SCN_NAME(1u, KEY_2)
    SCN_NAME(2u, KEY_3)
    SCN_NAME(3u, KEY_4)
    SCN_NAME(4u, KEY_5)
    SCN_NAME(5u, KEY_6)
    SCN_NAME(6u, KEY_7)
    SCN_NAME(7u, KEY_8)
    SCN_NAME(8u, KEY_9)
    SCN_NAME(9u, KEY_CLEAR)
    SCN_NAME(10u, KEY_0)
    SCN_NAME(11u, KEY_ENTER)
    SCN_NAME(12u, DISC_N)
    SCN_NAME(13u, DISC_NNE)
    SCN_NAME(14u, DISC_NE)
    SCN_NAME(15u, DISC_ENE)
    SCN_NAME(16u, DISC_E)
    SCN_NAME(17u, DISC_ESE)
    SCN_NAME(18u, DISC_SE)
    SCN_NAME(19u, DISC_SSE)
    SCN_NAME(20u, DISC_S)
    SCN_NAME(21u, DISC_SSW)
    SCN_NAME(22u, DISC_SW)
    SCN_NAME(23u, DISC_WSW)
    SCN_NAME(24u, DISC_W)
    SCN_NAME(25u, DISC_WNW)
    SCN_NAME(26u, DISC_NW)
    SCN_NAME(27u, DISC_NNW)
    SCN_NAME(28u, SIDE_TOP)
    SCN_NAME(29u, SIDE_LOWER_LEFT)
    SCN_NAME(30u, SIDE_LOWER_RIGHT)
  default:
    break;
  }
  *mask = 0u;
  return NULL;
}

uint32_t ivs_scenario_input_name_count(void)
{
  return SCN_INPUT_NAMES;
}

/* One controller word: 0x plus 1..8 hex digits (either case), or names joined by `+`. */
static uint32_t parse_controller(scn_tok t, uint32_t *word)
{
  uint32_t v = 0u;
  size_t i;
  if (t.n >= 2u && t.p[0] == (uint8_t)'0' && t.p[1] == (uint8_t)'x') {
    if (t.n < 3u || t.n > 10u) {
      return IVS_SCN_R_BAD_INPUT_VALUE;
    }
    for (i = 2u; i < t.n; i++) {
      int d = hex_val(t.p[i], 1, 1);
      if (d < 0) {
        return IVS_SCN_R_BAD_INPUT_VALUE;
      }
      v = (v << 4) | (uint32_t)d;
    }
    if ((v & IVS_CTRL_RESERVED_MASK) != 0u) {
      return IVS_SCN_R_INPUT_RESERVED_BIT;
    }
    *word = v;
    return IVS_SCN_R_NONE;
  }
  if (t.n == 0u) {
    return IVS_SCN_R_BAD_INPUT_VALUE;
  }
  i = 0u;
  while (i <= t.n) {
    size_t j = i;
    scn_tok name;
    uint32_t k;
    uint32_t found = 0u;
    while (j < t.n && t.p[j] != (uint8_t)'+') {
      j++;
    }
    name.p = t.p + i;
    name.n = j - i;
    if (name.n == 0u) {
      return IVS_SCN_R_BAD_INPUT_VALUE;
    }
    for (k = 0u; k < SCN_INPUT_NAMES; k++) {
      uint32_t mask = 0u;
      const char *nm = ivs_scenario_input_name(k, &mask);
      if (nm != NULL && tok_is(name, nm)) {
        v |= mask;
        found = 1u;
        break;
      }
    }
    if (!found) {
      return IVS_SCN_R_UNKNOWN_INPUT_NAME;
    }
    i = j + 1u;
  }
  *word = v;
  return IVS_SCN_R_NONE;
}

/* ---- directives ---- */

static uint32_t dir_content(ivs_scenario *s, const scn_line *l, uint32_t line)
{
  uint32_t role = 0u;
  uint32_t r;
  ivs_scenario_content *c;
  scn_tok v;
  if (l->n != 3u) {
    return IVS_SCN_R_BAD_ARITY;
  }
  r = parse_role(l->t[1], &role);
  if (r != IVS_SCN_R_NONE) {
    return r;
  }
  c = &s->content[role];
  if (c->kind != IVS_SCN_CONTENT_NONE) {
    return IVS_SCN_R_DUPLICATE_DIRECTIVE;
  }
  v = l->t[2];
  if (tok_is(v, "user")) {
    c->kind = IVS_SCN_CONTENT_USER;
  } else if (tok_is(v, "boot-stub")) {
    if (role != IVS_SCN_ROLE_EXEC) {
      return IVS_SCN_R_BAD_CONTENT_KIND;
    }
    c->kind = IVS_SCN_CONTENT_BOOT_STUB;
  } else if (tok_is(v, "zero-grom")) {
    if (role != IVS_SCN_ROLE_GROM) {
      return IVS_SCN_R_BAD_CONTENT_KIND;
    }
    c->kind = IVS_SCN_CONTENT_ZERO_GROM;
  } else {
    r = check_path(v);
    if (r != IVS_SCN_R_NONE) {
      return r;
    }
    memcpy(c->path, v.p, v.n);
    c->path[v.n] = '\0';
    c->kind = IVS_SCN_CONTENT_PATH;
  }
  c->line = line;
  return IVS_SCN_R_NONE;
}

/* A status name of ivs_status_name other than IVS_OK and IVS_E_UNKNOWN. */
static int parse_status(scn_tok t, int32_t *status)
{
  int v;
  for (v = -31; v <= -1; v++) {
    const char *nm = ivs_status_name(v);
    if (nm != NULL && strcmp(nm, "IVS_E_UNKNOWN") != 0 && tok_is(t, nm)) {
      *status = (int32_t)v;
      return 1;
    }
  }
  return 0;
}

static uint32_t dir_expect_load(ivs_scenario *s, const scn_line *l, uint32_t line)
{
  uint32_t role = 0u;
  uint32_t r;
  ivs_scenario_expect *e;
  int32_t status = 0;
  uint64_t reason = 0u;
  if (l->n != 3u && l->n != 5u) {
    return IVS_SCN_R_BAD_ARITY;
  }
  r = parse_role(l->t[1], &role);
  if (r != IVS_SCN_R_NONE) {
    return r;
  }
  e = &s->expect[role];
  if (e->load_set != 0u) {
    return IVS_SCN_R_DUPLICATE_DIRECTIVE;
  }
  if (s->content[role].kind == IVS_SCN_CONTENT_NONE) {
    return IVS_SCN_R_EXPECT_WITHOUT_CONTENT;
  }
  if (!tok_is(l->t[2], "ok") && !parse_status(l->t[2], &status)) {
    return IVS_SCN_R_BAD_STATUS;
  }
  if (l->n == 5u) {
    if (status == 0 || !tok_is(l->t[3], "reason")) {
      return IVS_SCN_R_BAD_TOKEN;
    }
    r = parse_dec(l->t[4], &reason);
    if (r != IVS_SCN_R_NONE) {
      return r;
    }
    if (reason == 0u || reason > UINT32_MAX) {
      return IVS_SCN_R_OUT_OF_RANGE;
    }
  }
  e->load_set = 1u;
  e->load_line = line;
  e->load_status = status;
  e->load_reason = (uint32_t)reason;
  return IVS_SCN_R_NONE;
}

static uint32_t dir_expect_sha256(ivs_scenario *s, const scn_line *l, uint32_t line)
{
  uint32_t role = 0u;
  uint32_t r;
  ivs_scenario_expect *e;
  uint8_t sha[32];
  if (l->n != 3u) {
    return IVS_SCN_R_BAD_ARITY;
  }
  r = parse_role(l->t[1], &role);
  if (r != IVS_SCN_R_NONE) {
    return r;
  }
  e = &s->expect[role];
  if (e->sha_set != 0u) {
    return IVS_SCN_R_DUPLICATE_DIRECTIVE;
  }
  if (s->content[role].kind == IVS_SCN_CONTENT_NONE) {
    return IVS_SCN_R_EXPECT_WITHOUT_CONTENT;
  }
  if (!parse_sha256(l->t[2], sha)) {
    return IVS_SCN_R_BAD_SHA256;
  }
  e->sha_set = 1u;
  e->sha_line = line;
  memcpy(e->sha256, sha, sizeof sha);
  return IVS_SCN_R_NONE;
}

static uint32_t dir_run_to(ivs_scenario *s, const scn_line *l, uint32_t line)
{
  uint64_t n = 0u;
  uint32_t unit;
  uint32_t stop = IVS_SCN_STOP_TARGET;
  uint32_t r;
  ivs_scenario_step *st;
  if (l->n != 3u && l->n != 5u) {
    return IVS_SCN_R_BAD_ARITY;
  }
  if (s->n_steps >= IVS_SCN_MAX_STEPS) {
    return IVS_SCN_R_TOO_MANY_STEPS;
  }
  r = parse_dec(l->t[1], &n);
  if (r != IVS_SCN_R_NONE) {
    return r;
  }
  if (n > IVS_SCN_TIME_MAX) {
    return IVS_SCN_R_OUT_OF_RANGE;
  }
  if (tok_is(l->t[2], "ticks")) {
    unit = IVS_SCN_UNIT_TICKS;
  } else if (tok_is(l->t[2], "cpu-cycles")) {
    unit = IVS_SCN_UNIT_CPU_CYCLES;
  } else if (tok_is(l->t[2], "frames")) {
    unit = IVS_SCN_UNIT_FRAMES;
  } else {
    return IVS_SCN_R_BAD_TOKEN;
  }
  if (l->n == 5u) {
    if (!tok_is(l->t[3], "stop")) {
      return IVS_SCN_R_BAD_TOKEN;
    }
    if (tok_is(l->t[4], "target")) {
      stop = IVS_SCN_STOP_TARGET;
    } else if (tok_is(l->t[4], "halted")) {
      stop = IVS_SCN_STOP_HALTED;
    } else if (tok_is(l->t[4], "faulted")) {
      stop = IVS_SCN_STOP_FAULTED;
    } else {
      return IVS_SCN_R_BAD_TOKEN;
    }
  }
  if (s->n_steps > 0u) {
    if (unit != s->step_unit) {
      return IVS_SCN_R_MIXED_STEP_UNIT;
    }
    if (n <= s->steps[s->n_steps - 1u].target) {
      return IVS_SCN_R_STEPS_NOT_INCREASING;
    }
  }
  s->step_unit = unit;
  st = &s->steps[s->n_steps];
  st->target = n;
  st->unit = unit;
  st->stop = stop;
  st->line = line;
  s->n_steps++;
  return IVS_SCN_R_NONE;
}

static uint32_t parse_digest_domain(scn_tok t, uint32_t *domain)
{
  if (tok_is(t, "composite")) {
    *domain = IVS_DIGEST_COMPOSITE;
  } else if (tok_is(t, "cpu")) {
    *domain = IVS_DIGEST_CPU;
  } else if (tok_is(t, "mem")) {
    *domain = IVS_DIGEST_MEM;
  } else if (tok_is(t, "images")) {
    *domain = IVS_DIGEST_IMAGES;
  } else if (tok_is(t, "instance")) {
    *domain = IVS_DIGEST_INSTANCE;
  } else if (tok_is(t, "video")) {
    *domain = IVS_DIGEST_VIDEO;
  } else if (tok_is(t, "audio")) {
    *domain = IVS_DIGEST_AUDIO;
  } else if (tok_is(t, "input")) {
    *domain = IVS_DIGEST_INPUT;
  } else if (tok_is(t, "psglog")) {
    *domain = IVS_DIGEST_PSGLOG;
  } else {
    return IVS_SCN_R_BAD_TOKEN;
  }
  return IVS_SCN_R_NONE;
}

static uint32_t parse_op(scn_tok t, uint32_t *op)
{
  if (tok_is(t, "==")) {
    *op = IVS_SCN_OP_EQ;
  } else if (tok_is(t, "!=")) {
    *op = IVS_SCN_OP_NE;
  } else {
    return IVS_SCN_R_BAD_TOKEN;
  }
  return IVS_SCN_R_NONE;
}

/* `name=<decimal>`: 1 and *v when t is `name=` plus a decimal number (hand-written, no overflow). */
static int field_dec(scn_tok t, const char *name, uint64_t *v)
{
  size_t k = strlen(name);
  if (!tok_starts(t, name) || t.n <= k) {
    return 0;
  }
  return parse_dec(tok_tail(t, k), v) == IVS_SCN_R_NONE;
}

/* `assert psglog count ==|!= <n>` and `assert psglog <i> [tick=<n>] [addr=<0..15>] [value=<HHHH>]
 * [source=<cpu-write|cpu-dw>]` (plan 05-09). Any malformation is bad-psglog-assert; a record form
 * names each field at most once and at least one. */
static uint32_t assert_psglog(ivs_scenario_assert *a, const scn_line *l)
{
  uint64_t n = 0u;
  a->subject = IVS_SCN_SUBJ_PSGLOG;
  if (tok_is(l->t[2], "count")) {
    if (l->n != 5u || parse_op(l->t[3], &a->op) != IVS_SCN_R_NONE || parse_dec(l->t[4], &n) != IVS_SCN_R_NONE) {
      return IVS_SCN_R_BAD_PSGLOG_ASSERT;
    }
    a->form = IVS_SCN_FORM_COUNT;
    a->num = n;
    return IVS_SCN_R_NONE;
  }
  if (l->n < 4u || l->n > 7u || parse_dec(l->t[2], &n) != IVS_SCN_R_NONE || n >= IVS_SCN_CAPTURE_MAX) {
    return IVS_SCN_R_BAD_PSGLOG_ASSERT;
  }
  a->form = IVS_SCN_FORM_RECORD;
  a->op = IVS_SCN_OP_EQ;
  a->index = (uint32_t)n;
  for (uint32_t k = 3u; k < l->n; k++) {
    uint64_t v = 0u;
    scn_tok t = l->t[k];
    if (tok_starts(t, "tick=")) {
      if ((a->fields & IVS_SCN_FIELD_TICK) != 0u || !field_dec(t, "tick=", &v)) {
        return IVS_SCN_R_BAD_PSGLOG_ASSERT;
      }
      a->fields |= IVS_SCN_FIELD_TICK;
      a->num = v;
    } else if (tok_starts(t, "addr=")) {
      if ((a->fields & IVS_SCN_FIELD_ADDR) != 0u || !field_dec(t, "addr=", &v) || v > 15u) {
        return IVS_SCN_R_BAD_PSGLOG_ASSERT;
      }
      a->fields |= IVS_SCN_FIELD_ADDR;
      a->addr = (uint32_t)v;
    } else if (tok_starts(t, "value=")) {
      if ((a->fields & IVS_SCN_FIELD_VALUE) != 0u || !parse_hex4(tok_tail(t, 6u), &a->value)) {
        return IVS_SCN_R_BAD_PSGLOG_ASSERT;
      }
      a->fields |= IVS_SCN_FIELD_VALUE;
    } else if (tok_starts(t, "source=")) {
      scn_tok src = tok_tail(t, 7u);
      if ((a->fields & IVS_SCN_FIELD_SOURCE) != 0u) {
        return IVS_SCN_R_BAD_PSGLOG_ASSERT;
      }
      if (tok_is(src, "cpu-write")) {
        a->source = IVS_PSGLOG_SRC_CPU_WRITE;
      } else if (tok_is(src, "cpu-dw")) {
        a->source = IVS_PSGLOG_SRC_CPU_DW;
      } else {
        return IVS_SCN_R_BAD_PSGLOG_ASSERT;
      }
      a->fields |= IVS_SCN_FIELD_SOURCE;
    } else {
      return IVS_SCN_R_BAD_PSGLOG_ASSERT;
    }
  }
  return a->fields != 0u ? IVS_SCN_R_NONE : IVS_SCN_R_BAD_PSGLOG_ASSERT;
}

/* `assert audio emitted ==|!= <n>` and `assert audio at <tick> A=<n> B=<n> C=<n>` (plan 05-09). */
static uint32_t assert_audio(ivs_scenario_assert *a, const scn_line *l)
{
  uint64_t n = 0u;
  static const char *const name[3] = {"A=", "B=", "C="};
  a->subject = IVS_SCN_SUBJ_AUDIO;
  if (tok_is(l->t[2], "emitted")) {
    if (l->n != 5u || parse_op(l->t[3], &a->op) != IVS_SCN_R_NONE || parse_dec(l->t[4], &n) != IVS_SCN_R_NONE) {
      return IVS_SCN_R_BAD_AUDIO_ASSERT;
    }
    a->form = IVS_SCN_FORM_EMITTED;
    a->num = n;
    return IVS_SCN_R_NONE;
  }
  if (!tok_is(l->t[2], "at") || l->n != 7u || parse_dec(l->t[3], &n) != IVS_SCN_R_NONE || n > IVS_SCN_TIME_MAX) {
    return IVS_SCN_R_BAD_AUDIO_ASSERT;
  }
  a->form = IVS_SCN_FORM_AT;
  a->op = IVS_SCN_OP_EQ;
  a->num = n;
  for (uint32_t k = 0u; k < 3u; k++) {
    uint64_t v = 0u;
    if (!field_dec(l->t[4u + k], name[k], &v) || v > 255u) {
      return IVS_SCN_R_BAD_AUDIO_ASSERT;
    }
    a->level[k] = (uint32_t)v;
  }
  a->fields = 0x7u;
  return IVS_SCN_R_NONE;
}

/* `assert frame <n> pixel <x> <y> ==|!= <0..15|blank>`, `assert frame <n> hash ==|!= <64 hex>` and
 * `assert frame <n> displayed ==|!= yes|no` (plan 06-12). Any malformation is bad-frame-assert; the
 * frame number is the slot's index (at most 2^31 - 1, so it is also a valid `capture video` first). */
static uint32_t assert_frame(ivs_scenario_assert *a, const scn_line *l)
{
  uint64_t n = 0u;
  uint64_t x = 0u;
  uint64_t y = 0u;
  uint64_t v = 0u;
  a->subject = IVS_SCN_SUBJ_FRAME;
  if (l->n < 4u || parse_dec(l->t[2], &n) != IVS_SCN_R_NONE || n > 0x7FFFFFFFu) {
    return IVS_SCN_R_BAD_FRAME_ASSERT;
  }
  a->num = n;
  if (tok_is(l->t[3], "pixel")) {
    if (l->n != 8u || parse_dec(l->t[4], &x) != IVS_SCN_R_NONE || x > 65535u || parse_dec(l->t[5], &y) != IVS_SCN_R_NONE ||
        y > 65535u || parse_op(l->t[6], &a->op) != IVS_SCN_R_NONE) {
      return IVS_SCN_R_BAD_FRAME_ASSERT;
    }
    if (tok_is(l->t[7], "blank")) {
      v = IVS_SCN_PIXEL_BLANK;
    } else if (parse_dec(l->t[7], &v) != IVS_SCN_R_NONE || v > 15u) {
      return IVS_SCN_R_BAD_FRAME_ASSERT;
    }
    a->form = IVS_SCN_FORM_PIXEL;
    a->index = (uint32_t)x;
    a->addr = (uint32_t)y;
    a->level[0] = (uint32_t)v;
    return IVS_SCN_R_NONE;
  }
  if (tok_is(l->t[3], "hash")) {
    if (l->n != 6u || parse_op(l->t[4], &a->op) != IVS_SCN_R_NONE || !parse_sha256(l->t[5], a->sha256)) {
      return IVS_SCN_R_BAD_FRAME_ASSERT;
    }
    a->form = IVS_SCN_FORM_HASH;
    return IVS_SCN_R_NONE;
  }
  if (tok_is(l->t[3], "displayed")) {
    if (l->n != 6u || parse_op(l->t[4], &a->op) != IVS_SCN_R_NONE) {
      return IVS_SCN_R_BAD_FRAME_ASSERT;
    }
    if (tok_is(l->t[5], "yes")) {
      a->value = 1u;
    } else if (!tok_is(l->t[5], "no")) {
      return IVS_SCN_R_BAD_FRAME_ASSERT;
    }
    a->form = IVS_SCN_FORM_DISPLAYED;
    return IVS_SCN_R_NONE;
  }
  return IVS_SCN_R_BAD_FRAME_ASSERT;
}

/* `assert video count ==|!= <n>` and `assert video chain ==|!= <64 hex>` (plan 06-12). */
static uint32_t assert_video(ivs_scenario_assert *a, const scn_line *l)
{
  uint64_t n = 0u;
  a->subject = IVS_SCN_SUBJ_VIDEO;
  if (l->n != 5u || parse_op(l->t[3], &a->op) != IVS_SCN_R_NONE) {
    return IVS_SCN_R_BAD_VIDEO_ASSERT;
  }
  if (tok_is(l->t[2], "count")) {
    if (parse_dec(l->t[4], &n) != IVS_SCN_R_NONE) {
      return IVS_SCN_R_BAD_VIDEO_ASSERT;
    }
    a->form = IVS_SCN_FORM_COUNT;
    a->num = n;
    return IVS_SCN_R_NONE;
  }
  if (tok_is(l->t[2], "chain") && parse_sha256(l->t[4], a->sha256)) {
    a->form = IVS_SCN_FORM_CHAIN;
    return IVS_SCN_R_NONE;
  }
  return IVS_SCN_R_BAD_VIDEO_ASSERT;
}

/* `capture video <first> <count>` (plan 06-12): the frames with index first .. first + count - 1 are kept, count 1 to
 * IVS_SCN_CAPTURE_VIDEO_MAX; once, anywhere after the header. */
static uint32_t dir_capture_video(ivs_scenario *s, const scn_line *l, uint32_t line)
{
  uint64_t first = 0u;
  uint64_t count = 0u;
  if (l->n != 4u) {
    return IVS_SCN_R_BAD_ARITY;
  }
  if (parse_dec(l->t[2], &first) != IVS_SCN_R_NONE || parse_dec(l->t[3], &count) != IVS_SCN_R_NONE || first > 0x7FFFFFFFu ||
      count == 0u || count > IVS_SCN_CAPTURE_VIDEO_MAX) {
    return IVS_SCN_R_BAD_CAPTURE_VIDEO;
  }
  if ((s->capture & IVS_SCN_CAPTURE_VIDEO) != 0u) {
    return IVS_SCN_R_DUPLICATE_CAPTURE;
  }
  s->capture |= IVS_SCN_CAPTURE_VIDEO;
  s->capture_line[2] = line;
  s->video_first = (uint32_t)first;
  s->video_count = (uint32_t)count;
  return IVS_SCN_R_NONE;
}

/* `capture psglog|audio`: once per target, anywhere after the header (plan 05-09). */
static uint32_t dir_capture(ivs_scenario *s, const scn_line *l, uint32_t line)
{
  uint32_t bit;
  uint32_t idx;
  if (l->n >= 2u && tok_is(l->t[1], "video")) {
    return dir_capture_video(s, l, line);
  }
  if (l->n != 2u) {
    return IVS_SCN_R_BAD_ARITY;
  }
  if (tok_is(l->t[1], "psglog")) {
    bit = IVS_SCN_CAPTURE_PSGLOG;
    idx = 0u;
  } else if (tok_is(l->t[1], "audio")) {
    bit = IVS_SCN_CAPTURE_AUDIO;
    idx = 1u;
  } else {
    return IVS_SCN_R_BAD_CAPTURE;
  }
  if ((s->capture & bit) != 0u) {
    return IVS_SCN_R_DUPLICATE_CAPTURE;
  }
  s->capture |= bit;
  s->capture_line[idx] = line;
  return IVS_SCN_R_NONE;
}

static uint32_t dir_assert(ivs_scenario *s, const scn_line *l, uint32_t line)
{
  ivs_scenario_assert a;
  uint32_t r;
  if (s->n_steps == 0u) {
    return IVS_SCN_R_ASSERT_BEFORE_STEP;
  }
  if (s->n_asserts >= IVS_SCN_MAX_ASSERTS) {
    return IVS_SCN_R_TOO_MANY_ASSERTS;
  }
  if (l->n < 2u) {
    return IVS_SCN_R_BAD_ARITY;
  }
  memset(&a, 0, sizeof a);
  a.line = line;
  a.step = s->n_steps - 1u;
  if (tok_is(l->t[1], "halted")) {
    if (l->n != 4u) {
      return IVS_SCN_R_BAD_ARITY;
    }
    r = parse_op(l->t[2], &a.op);
    if (r != IVS_SCN_R_NONE) {
      return r;
    }
    if (tok_is(l->t[3], "yes")) {
      a.value = 1u;
    } else if (!tok_is(l->t[3], "no")) {
      return IVS_SCN_R_BAD_VALUE;
    }
    a.subject = IVS_SCN_SUBJ_HALTED;
  } else if (tok_is(l->t[1], "reg") || tok_is(l->t[1], "mem") || tok_is(l->t[1], "digest")) {
    scn_tok o;
    if (l->n != 5u) {
      return IVS_SCN_R_BAD_ARITY;
    }
    o = l->t[2];
    if (tok_is(l->t[1], "reg")) {
      if (o.n != 2u || o.p[0] != (uint8_t)'R' || o.p[1] < (uint8_t)'0' || o.p[1] > (uint8_t)'7') {
        return IVS_SCN_R_BAD_TOKEN;
      }
      a.subject = IVS_SCN_SUBJ_REG;
      a.index = (uint32_t)(o.p[1] - (uint8_t)'0');
    } else if (tok_is(l->t[1], "mem")) {
      if (o.n == 10u && o.p[4] == (uint8_t)'.' && o.p[5] == (uint8_t)'.') {
        scn_tok lo = o;
        scn_tok hi = tok_tail(o, 6u);
        lo.n = 4u;
        if (!parse_hex4(lo, &a.addr_lo) || !parse_hex4(hi, &a.addr_hi)) {
          return IVS_SCN_R_BAD_TOKEN;
        }
        if (a.addr_hi < a.addr_lo) {
          return IVS_SCN_R_OUT_OF_RANGE;
        }
        a.subject = IVS_SCN_SUBJ_MEM_RANGE;
      } else {
        if (!parse_hex4(o, &a.addr_lo)) {
          return IVS_SCN_R_BAD_TOKEN;
        }
        a.addr_hi = a.addr_lo;
        a.subject = IVS_SCN_SUBJ_MEM;
      }
    } else {
      r = parse_digest_domain(o, &a.index);
      if (r != IVS_SCN_R_NONE) {
        return r;
      }
      a.subject = IVS_SCN_SUBJ_DIGEST;
    }
    r = parse_op(l->t[3], &a.op);
    if (r != IVS_SCN_R_NONE) {
      return r;
    }
    if (a.subject == IVS_SCN_SUBJ_DIGEST) {
      if (!parse_sha256(l->t[4], a.sha256)) {
        return IVS_SCN_R_BAD_VALUE;
      }
    } else if (!parse_hex4(l->t[4], &a.value)) {
      return IVS_SCN_R_BAD_VALUE;
    }
  } else if (tok_is(l->t[1], "psglog") || tok_is(l->t[1], "audio") || tok_is(l->t[1], "frame") ||
             tok_is(l->t[1], "video")) {
    if (l->n < 3u) {
      return IVS_SCN_R_BAD_ARITY;
    }
    if (tok_is(l->t[1], "psglog")) {
      r = assert_psglog(&a, l);
    } else if (tok_is(l->t[1], "audio")) {
      r = assert_audio(&a, l);
    } else if (tok_is(l->t[1], "frame")) {
      r = assert_frame(&a, l);
    } else {
      r = assert_video(&a, l);
    }
    if (r != IVS_SCN_R_NONE) {
      return r;
    }
  } else {
    return IVS_SCN_R_BAD_TOKEN;
  }
  s->asserts[s->n_asserts] = a;
  s->n_asserts++;
  return IVS_SCN_R_NONE;
}

static uint32_t dir_input(ivs_scenario *s, const scn_line *l, uint32_t line)
{
  ivs_scenario_input in;
  uint32_t unit;
  uint32_t r;
  scn_tok num;
  if (l->n != 4u) {
    return IVS_SCN_R_BAD_ARITY;
  }
  if (s->n_inputs >= IVS_SCN_MAX_INPUTS) {
    return IVS_SCN_R_TOO_MANY_INPUTS;
  }
  memset(&in, 0, sizeof in);
  if (tok_starts(l->t[1], "at_tick=")) {
    unit = IVS_SCN_INPUT_AT_TICK;
    num = tok_tail(l->t[1], 8u);
  } else if (tok_starts(l->t[1], "at_cycle=")) {
    unit = IVS_SCN_INPUT_AT_CYCLE;
    num = tok_tail(l->t[1], 9u);
  } else if (tok_starts(l->t[1], "at_frame=")) {
    return IVS_SCN_R_AT_FRAME;
  } else {
    return IVS_SCN_R_BAD_TOKEN;
  }
  r = parse_dec(num, &in.at);
  if (r != IVS_SCN_R_NONE) {
    return r;
  }
  if (in.at > IVS_SCN_TIME_MAX) {
    return IVS_SCN_R_OUT_OF_RANGE;
  }
  if (s->n_inputs > 0u) {
    if (unit != s->input_unit) {
      return IVS_SCN_R_MIXED_INPUT_UNIT;
    }
    if (in.at < s->inputs[s->n_inputs - 1u].at) {
      return IVS_SCN_R_INPUT_UNSORTED;
    }
  }
  if (!tok_starts(l->t[2], "c0=") || !tok_starts(l->t[3], "c1=")) {
    return IVS_SCN_R_BAD_TOKEN;
  }
  r = parse_controller(tok_tail(l->t[2], 3u), &in.controller[0]);
  if (r != IVS_SCN_R_NONE) {
    return r;
  }
  r = parse_controller(tok_tail(l->t[3], 3u), &in.controller[1]);
  if (r != IVS_SCN_R_NONE) {
    return r;
  }
  in.line = line;
  s->input_unit = unit;
  s->inputs[s->n_inputs] = in;
  s->n_inputs++;
  return IVS_SCN_R_NONE;
}

/* One directive line (tokens already split). have_header is the parser's only flag. */
static uint32_t parse_directive(ivs_scenario *s, const scn_line *l, uint32_t line, uint32_t *have_header)
{
  scn_tok d = l->t[0];
  uint64_t n = 0u;
  uint32_t r;
  if (*have_header == 0u) {
    if (!tok_is(d, "ivss")) {
      return IVS_SCN_R_MISSING_HEADER;
    }
    if (l->n != 2u) {
      return IVS_SCN_R_BAD_ARITY;
    }
    if (!tok_is(l->t[1], "1")) {
      return IVS_SCN_R_BAD_VERSION;
    }
    *have_header = 1u;
    return IVS_SCN_R_NONE;
  }
  if (tok_is(d, "ivss")) {
    return IVS_SCN_R_LATE_HEADER;
  }
  if (tok_is(d, "run-to")) {
    return dir_run_to(s, l, line);
  }
  if (tok_is(d, "assert")) {
    return dir_assert(s, l, line);
  }
  if (tok_is(d, "input")) {
    return dir_input(s, l, line);
  }
  if (tok_is(d, "capture")) {
    return dir_capture(s, l, line);
  }
  if (!tok_is(d, "id") && !tok_is(d, "profile") && !tok_is(d, "content") && !tok_is(d, "expect-load") &&
      !tok_is(d, "expect-sha256") && !tok_is(d, "wall-timeout-ms")) {
    return IVS_SCN_R_UNKNOWN_DIRECTIVE;
  }
  /* Header directives. */
  if (s->n_steps > 0u) {
    return IVS_SCN_R_HEADER_AFTER_STEP;
  }
  if (tok_is(d, "content")) {
    return dir_content(s, l, line);
  }
  if (tok_is(d, "expect-load")) {
    return dir_expect_load(s, l, line);
  }
  if (tok_is(d, "expect-sha256")) {
    return dir_expect_sha256(s, l, line);
  }
  if (tok_is(d, "id")) {
    if (s->id[0] != '\0') {
      return IVS_SCN_R_DUPLICATE_DIRECTIVE;
    }
    if (l->n != 2u) {
      return IVS_SCN_R_BAD_ARITY;
    }
    return copy_slug(l->t[1], s->id, IVS_SCN_MAX_ID) ? IVS_SCN_R_NONE : IVS_SCN_R_BAD_ID;
  }
  if (tok_is(d, "profile")) {
    if (s->profile[0] != '\0') {
      return IVS_SCN_R_DUPLICATE_DIRECTIVE;
    }
    if (l->n != 2u) {
      return IVS_SCN_R_BAD_ARITY;
    }
    return copy_slug(l->t[1], s->profile, IVS_SCN_MAX_PROFILE) ? IVS_SCN_R_NONE : IVS_SCN_R_BAD_PROFILE;
  }
  /* wall-timeout-ms */
  if (s->wall_timeout_ms != 0u) {
    return IVS_SCN_R_DUPLICATE_DIRECTIVE;
  }
  if (l->n != 2u) {
    return IVS_SCN_R_BAD_ARITY;
  }
  r = parse_dec(l->t[1], &n);
  if (r != IVS_SCN_R_NONE) {
    return r;
  }
  if (n == 0u || n > UINT32_MAX) {
    return IVS_SCN_R_OUT_OF_RANGE;
  }
  s->wall_timeout_ms = (uint32_t)n;
  return IVS_SCN_R_NONE;
}

/* The smaller nonzero line of a and b (0: none yet). */
static uint32_t scn_first_line(uint32_t a, uint32_t b)
{
  return (a == 0u || (b != 0u && b < a)) ? b : a;
}

/* Every directive of an accepted scenario can take effect (review WR-02, WR-03). The executor
 * loads the roles in role order (exec, grom, cart) and a refusal that matches expect-load ends
 * the scenario as pass, so with an expect-load naming a refusal nothing after that load ever
 * runs: a content line of a later role (and its expect lines), any run-to (and so any assert)
 * and any input line are refused as dead-after-refusal, at the first such line. Without an
 * expected refusal a scenario needs at least one run-to (no-run-to, line 0): steps and asserts
 * are what a pass vouches for. */
static uint32_t scn_check_capture(const ivs_scenario *s, uint32_t *line)
{
  /* A record form of `assert psglog` and `assert audio at` read a capture: the directive may stand
   * anywhere in the file, so this is decided at the end, at the first assert that lacks its capture. */
  for (uint32_t i = 0u; i < s->n_asserts; i++) {
    const ivs_scenario_assert *a = &s->asserts[i];
    if ((a->form == IVS_SCN_FORM_RECORD && (s->capture & IVS_SCN_CAPTURE_PSGLOG) == 0u) ||
        (a->form == IVS_SCN_FORM_AT && (s->capture & IVS_SCN_CAPTURE_AUDIO) == 0u) ||
        (a->subject == IVS_SCN_SUBJ_FRAME && (s->capture & IVS_SCN_CAPTURE_VIDEO) == 0u)) {
      *line = a->line;
      return IVS_SCN_R_ASSERT_WITHOUT_CAPTURE;
    }
  }
  *line = 0u;
  return IVS_SCN_R_NONE;
}

static uint32_t scn_check_reach(const ivs_scenario *s, uint32_t *line)
{
  uint32_t refused = IVS_SCN_ROLES;
  uint32_t dead = 0u;
  uint32_t role;
  for (role = 0u; role < IVS_SCN_ROLES; role++) {
    if (s->expect[role].load_set != 0u && s->expect[role].load_status != 0) {
      refused = role;
      break;
    }
  }
  if (refused == IVS_SCN_ROLES) {
    *line = 0u;
    return s->n_steps == 0u ? IVS_SCN_R_NO_RUN_TO : IVS_SCN_R_NONE;
  }
  /* A role's expect lines follow its content line, so the content line is the role's first. */
  for (role = refused + 1u; role < IVS_SCN_ROLES; role++) {
    if (s->content[role].kind != IVS_SCN_CONTENT_NONE) {
      dead = scn_first_line(dead, s->content[role].line);
    }
  }
  /* Steps and inputs are stored in file order; an assert follows a run-to. */
  if (s->n_steps > 0u) {
    dead = scn_first_line(dead, s->steps[0].line);
  }
  if (s->n_inputs > 0u) {
    dead = scn_first_line(dead, s->inputs[0].line);
  }
  *line = dead;
  return dead != 0u ? IVS_SCN_R_DEAD_AFTER_REFUSAL : IVS_SCN_R_NONE;
}

static int scn_refuse(ivs_scenario *out, ivs_scenario_error *err, uint32_t line, uint32_t reason)
{
  memset(out, 0, sizeof *out);
  err->line = line;
  err->reason = reason;
  return (int)reason;
}

int ivs_scenario_parse(const uint8_t *bytes, size_t len, ivs_scenario *out, ivs_scenario_error *err)
{
  size_t pos = 0u;
  uint32_t line = 0u;
  uint32_t have_header = 0u;
  if (out == NULL || err == NULL || (bytes == NULL && len != 0u)) {
    return -1;
  }
  memset(out, 0, sizeof *out);
  err->line = 0u;
  err->reason = IVS_SCN_R_NONE;
  if (len > IVS_SCN_MAX_BYTES) {
    return scn_refuse(out, err, 0u, IVS_SCN_R_FILE_TOO_LARGE);
  }
  while (pos < len) {
    size_t start = pos;
    size_t end;
    size_t i;
    scn_line l;
    uint32_t r;
    line++;
    if (line > IVS_SCN_MAX_LINES) {
      return scn_refuse(out, err, line, IVS_SCN_R_TOO_MANY_LINES);
    }
    /* Byte classes and the line cap, in byte order (the first failing byte wins). */
    while (pos < len && bytes[pos] != (uint8_t)'\n') {
      uint8_t c = bytes[pos];
#ifndef IVS_FAULT_RUNNER_LINE_CAP_UNCHECKED
      if (pos - start >= IVS_SCN_MAX_LINE_BYTES) {
        return scn_refuse(out, err, line, IVS_SCN_R_LINE_TOO_LONG);
      }
#endif
      if (c == 0u) {
        return scn_refuse(out, err, line, IVS_SCN_R_NUL);
      }
      if (c == (uint8_t)'\r') {
        return scn_refuse(out, err, line, IVS_SCN_R_CR);
      }
      if (c >= 0x80u) {
        return scn_refuse(out, err, line, IVS_SCN_R_NON_ASCII);
      }
      if ((c < 0x20u && c != (uint8_t)'\t') || c == 0x7Fu) {
        return scn_refuse(out, err, line, IVS_SCN_R_CONTROL_BYTE);
      }
      pos++;
    }
    end = pos;
    if (pos < len) {
      pos++; /* the LF */
    }
    if (end > start && bytes[start] == (uint8_t)'#') {
      continue; /* comment line */
    }
    /* Split on spaces and tabs. */
    memset(&l, 0, sizeof l);
    i = start;
    while (i < end) {
      size_t j;
      while (i < end && (bytes[i] == (uint8_t)' ' || bytes[i] == (uint8_t)'\t')) {
        i++;
      }
      if (i == end) {
        break;
      }
      j = i;
      while (j < end && bytes[j] != (uint8_t)' ' && bytes[j] != (uint8_t)'\t') {
        j++;
      }
      if (l.n < SCN_TOKS) {
        l.t[l.n].p = bytes + i;
        l.t[l.n].n = j - i;
        l.n++;
      }
      i = j;
    }
    if (l.n == 0u) {
      continue; /* blank line */
    }
    r = parse_directive(out, &l, line, &have_header);
    if (r != IVS_SCN_R_NONE) {
      return scn_refuse(out, err, line, r);
    }
  }
  if (have_header == 0u) {
    return scn_refuse(out, err, 0u, IVS_SCN_R_MISSING_HEADER);
  }
  if (out->id[0] == '\0' || out->profile[0] == '\0') {
    return scn_refuse(out, err, 0u, IVS_SCN_R_MISSING_DIRECTIVE);
  }
  {
    uint32_t at = 0u;
    uint32_t r = scn_check_capture(out, &at);
    if (r == IVS_SCN_R_NONE) {
      r = scn_check_reach(out, &at);
    }
    if (r != IVS_SCN_R_NONE) {
      return scn_refuse(out, err, at, r);
    }
  }
  return 0;
}

const char *ivs_scenario_reason_name(uint32_t reason)
{
  switch (reason) {
  case IVS_SCN_R_NONE:
    return "none";
  case IVS_SCN_R_FILE_TOO_LARGE:
    return "file-too-large";
  case IVS_SCN_R_TOO_MANY_LINES:
    return "too-many-lines";
  case IVS_SCN_R_LINE_TOO_LONG:
    return "line-too-long";
  case IVS_SCN_R_CR:
    return "cr";
  case IVS_SCN_R_NUL:
    return "nul";
  case IVS_SCN_R_NON_ASCII:
    return "non-ascii";
  case IVS_SCN_R_CONTROL_BYTE:
    return "control-byte";
  case IVS_SCN_R_MISSING_HEADER:
    return "missing-header";
  case IVS_SCN_R_LATE_HEADER:
    return "late-header";
  case IVS_SCN_R_BAD_VERSION:
    return "bad-version";
  case IVS_SCN_R_UNKNOWN_DIRECTIVE:
    return "unknown-directive";
  case IVS_SCN_R_DUPLICATE_DIRECTIVE:
    return "duplicate-directive";
  case IVS_SCN_R_HEADER_AFTER_STEP:
    return "header-after-step";
  case IVS_SCN_R_MISSING_DIRECTIVE:
    return "missing-directive";
  case IVS_SCN_R_BAD_ARITY:
    return "bad-arity";
  case IVS_SCN_R_BAD_TOKEN:
    return "bad-token";
  case IVS_SCN_R_BAD_NUMBER:
    return "bad-number";
  case IVS_SCN_R_OUT_OF_RANGE:
    return "out-of-range";
  case IVS_SCN_R_BAD_ID:
    return "bad-id";
  case IVS_SCN_R_BAD_PROFILE:
    return "bad-profile";
  case IVS_SCN_R_PATH_DOTDOT:
    return "path-dotdot";
  case IVS_SCN_R_PATH_ABSOLUTE:
    return "path-absolute";
  case IVS_SCN_R_PATH_BYTE:
    return "path-byte";
  case IVS_SCN_R_PATH_EMPTY_SEGMENT:
    return "path-empty-segment";
  case IVS_SCN_R_PATH_TOO_LONG:
    return "path-too-long";
  case IVS_SCN_R_BAD_CONTENT_KIND:
    return "bad-content-kind";
  case IVS_SCN_R_EXPECT_WITHOUT_CONTENT:
    return "expect-without-content";
  case IVS_SCN_R_BAD_STATUS:
    return "bad-status";
  case IVS_SCN_R_BAD_SHA256:
    return "bad-sha256";
  case IVS_SCN_R_STEPS_NOT_INCREASING:
    return "steps-not-increasing";
  case IVS_SCN_R_MIXED_STEP_UNIT:
    return "mixed-step-unit";
  case IVS_SCN_R_TOO_MANY_STEPS:
    return "too-many-steps";
  case IVS_SCN_R_TOO_MANY_ASSERTS:
    return "too-many-asserts";
  case IVS_SCN_R_ASSERT_BEFORE_STEP:
    return "assert-before-step";
  case IVS_SCN_R_BAD_VALUE:
    return "bad-value";
  case IVS_SCN_R_UNKNOWN_INPUT_NAME:
    return "unknown-input-name";
  case IVS_SCN_R_INPUT_RESERVED_BIT:
    return "input-reserved-bit";
  case IVS_SCN_R_AT_FRAME:
    return "at-frame";
  case IVS_SCN_R_INPUT_UNSORTED:
    return "input-unsorted";
  case IVS_SCN_R_TOO_MANY_INPUTS:
    return "too-many-inputs";
  case IVS_SCN_R_MIXED_INPUT_UNIT:
    return "mixed-input-unit";
  case IVS_SCN_R_BAD_INPUT_VALUE:
    return "bad-input-value";
  case IVS_SCN_R_DEAD_AFTER_REFUSAL:
    return "dead-after-refusal";
  case IVS_SCN_R_NO_RUN_TO:
    return "no-run-to";
  case IVS_SCN_R_BAD_CAPTURE:
    return "bad-capture";
  case IVS_SCN_R_DUPLICATE_CAPTURE:
    return "duplicate-capture";
  case IVS_SCN_R_BAD_PSGLOG_ASSERT:
    return "bad-psglog-assert";
  case IVS_SCN_R_BAD_AUDIO_ASSERT:
    return "bad-audio-assert";
  case IVS_SCN_R_ASSERT_WITHOUT_CAPTURE:
    return "assert-without-capture";
  case IVS_SCN_R_BAD_CAPTURE_VIDEO:
    return "bad-capture-video";
  case IVS_SCN_R_BAD_FRAME_ASSERT:
    return "bad-frame-assert";
  case IVS_SCN_R_BAD_VIDEO_ASSERT:
    return "bad-video-assert";
  default:
    return "unknown";
  }
}

int ivs_scenario_error_format(const ivs_scenario_error *err, char *buf, size_t cap)
{
  int n;
  if (buf == NULL || cap == 0u) {
    return -1;
  }
  buf[0] = '\0';
  if (err == NULL) {
    return -1;
  }
  n = snprintf(buf, cap, "scenario-syntax line=%lu reason=%s", (unsigned long)err->line,
               ivs_scenario_reason_name(err->reason));
  if (n < 0 || (size_t)n >= cap) {
    buf[0] = '\0';
    return -1;
  }
  return 0;
}

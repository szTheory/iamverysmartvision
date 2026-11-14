/* SPDX-License-Identifier: MIT */
/* Suite runner, family fuzz (D-22, D-23, plan 04-17; threats T-04-49, T-04-50): a seeded
 * robustness run of the IVSS v1 scenario parser. Each case runner.fuzz.seed-NN is one fixed
 * 32-bit seed driving an in-repo xorshift32 generator; nothing reads a host source of randomness
 * or time, so every run is identical.
 *
 * Per seed FUZZ_ITERS iterations each pick a mutation base (an accepted scenario of
 * runner_parse.c, all project-authored) and apply one to three stacked mutations:
 *   byte       one byte flipped in one bit, or set to a byte of a dictionary (space, tab, LF,
 *              CR, NUL, `#`, `.`, `/`, `=`, `+`, digits, hex letters, 0x80, DEL, 0x01);
 *   insert     a dictionary token (`..`, `/`, directive names, huge numbers, `at_frame=`,
 *              status names, `0x80000000`, ...) at a random offset;
 *   cut        truncate, or delete a short range;
 *   lines      duplicate or swap whole lines;
 *   long line  200..300 bytes inserted into one line;
 *   grow       one line repeated many times, or comment padding to the 64 KiB edge (rare,
 *              to reach the line, step, assert, input and size caps).
 * The mutant is copied into a heap block of exactly its size (NULL for 0 bytes; ASan in the dev
 * preset sees any read past it) and parsed into a poisoned struct.
 *
 * Invariants on every input:
 *   - the result is 0 (with error line 0 and reason none) or a reason in 1..IVS_SCN_R_COUNT-1
 *     equal to the error's reason, whose name is not "unknown"; never -1;
 *   - a refusal leaves the output all zero; its line is at most the input's line count, and
 *     line 0 only for the whole-file reasons (file-too-large, missing-header,
 *     missing-directive, no-run-to);
 *   - an acceptance came from an input within the byte, line and line-length caps, and
 *     satisfies every structural rule of the grammar (charsets, path rule, caps, increasing
 *     steps, asserts tied to existing steps, sorted inputs, no reserved bit, a run-to unless an
 *     expected refusal ends the scenario, nothing after such a refusal);
 *   - parsing the same bytes again gives the identical result, struct and error (determinism),
 *     and the input block is never written.
 * runner.fuzz.reasons-covered replays every seed and requires at least 8 distinct reasons and
 * one acceptance, so the run is not vacuous. */
#include <stdint.h>
#include <stdlib.h>
#include <string.h>

#include "ivs/ivs.h"
#include "ivs_runner.h"
#include "ivs_test.h"
#include "runner_cases.h"

#define FUZZ_SEEDS 8u
#define FUZZ_ITERS 1500u
#define FUZZ_CAP 70000u
#define FUZZ_MIN_REASONS 8u

#define FUZZ_CHECK(f, cond)                                                                            \
  do {                                                                                                 \
    if (!(cond)) {                                                                                     \
      ivs_test_diag("seed=0x%08X iter=%u: check failed at line %d: %s", (unsigned)(f)->seed,          \
                    (unsigned)(f)->iter, __LINE__, #cond);                                             \
      return IVS_TEST_FAIL;                                                                            \
    }                                                                                                  \
  } while (0)

static const uint32_t fuzz_seeds[FUZZ_SEEDS] = {0x9E3779B9u, 0x7F4A7C15u, 0x85EBCA6Bu, 0xC2B2AE35u,
                                                0x27D4EB2Fu, 0x165667B1u, 0xD3A2646Cu, 0xFD7046C5u};

static const char *const fuzz_ids[FUZZ_SEEDS + 1u] = {
    "runner.fuzz.seed-01", "runner.fuzz.seed-02", "runner.fuzz.seed-03", "runner.fuzz.seed-04", "runner.fuzz.seed-05",
    "runner.fuzz.seed-06", "runner.fuzz.seed-07", "runner.fuzz.seed-08", "runner.fuzz.reasons-covered"};

static const uint8_t fuzz_bytes[] = {' ', '\t', '\n', '\r', 0u,  '#', '.', '/', '=',  '+',  '-', '0',
                                     '9', 'x',  'A',  'f',  'R', '_', 'z', 0x80u, 0x7Fu, 0x01u, '!', '~'};

static const char *const fuzz_tokens[] = {"..",
                                          "/",
                                          " ",
                                          "\n",
                                          "#",
                                          "ivss 1\n",
                                          "ivss 2\n",
                                          "id x\n",
                                          "profile q\n",
                                          "run-to ",
                                          "assert ",
                                          "capture psglog\n",
                                          "capture audio\n",
                                          "capture video 0 16\n",
                                          "capture video 2147483647 1\n",
                                          "assert frame 0 pixel 158 95 == blank\n",
                                          "assert frame 1 hash != 0123456789abcdef0123456789abcdef0123456789abcdef0123456789abcdef\n",
                                          "assert frame 0 displayed == no\n",
                                          "assert video count == 2\n",
                                          "assert video chain == 0123456789abcdef0123456789abcdef0123456789abcdef0123456789abcdef\n",
                                          "pixel ", "hash ", "displayed ", "chain ", "blank", "frame ", "video ",
                                          "assert psglog count == 3\n",
                                          "assert psglog 0 tick=100 addr=11 value=000F source=cpu-write\n",
                                          "assert audio emitted != 0\n",
                                          "assert audio at 160 A=15 B=0 C=0\n",
                                          "tick=", "addr=", "value=", "source=cpu-dw", "count", "emitted",
                                          "input ",
                                          "content cart ",
                                          "content exec boot-stub\n",
                                          "content exec boot-stub-2\n",
                                          "expect-load cart IVS_E_OVERSIZED reason 0\n",
                                          "expect-sha256 cart ",
                                          "wall-timeout-ms 0\n",
                                          "99999999999999999999",
                                          "4611686018427387905",
                                          "18446744073709551615",
                                          "0x80000000",
                                          "0x",
                                          "at_frame=1",
                                          "at_cycle=7",
                                          "KEY_1+",
                                          "+",
                                          "==",
                                          "!=",
                                          "stop halted",
                                          "frames",
                                          "..0000",
                                          "digest video",
                                          "reg R9",
                                          "\r\n"};

typedef struct fuzz_ctx {
  uint32_t seed;
  uint32_t x; /* xorshift32 state, never 0 */
  uint32_t iter;
  uint32_t accepted;
  uint32_t seen[IVS_SCN_R_COUNT];
  uint8_t buf[FUZZ_CAP];
  size_t n;
  uint8_t tmp[FUZZ_CAP];
} fuzz_ctx;

/* xorshift32 (shifts 13, 17, 5): a full-period generator over the nonzero 32-bit values. */
static uint32_t fuzz_next(fuzz_ctx *f)
{
  uint32_t x = f->x;
  x ^= x << 13;
  x ^= x >> 17;
  x ^= x << 5;
  f->x = x;
  return x;
}

static uint32_t fuzz_below(fuzz_ctx *f, uint32_t n)
{
  return fuzz_next(f) % n;
}

static size_t fuzz_below_sz(fuzz_ctx *f, size_t n)
{
  return n == 0u ? 0u : (size_t)(fuzz_next(f) % (uint32_t)n);
}

/* Inserts k bytes at off (clipped to the buffer cap). */
static void fuzz_insert(fuzz_ctx *f, size_t off, const uint8_t *src, size_t k)
{
  if (k > FUZZ_CAP - f->n) {
    k = FUZZ_CAP - f->n;
  }
  memmove(f->buf + off + k, f->buf + off, f->n - off);
  memcpy(f->buf + off, src, k);
  f->n += k;
}

/* Start and end (exclusive of the LF) of the line holding off. */
static void fuzz_line_at(const fuzz_ctx *f, size_t off, size_t *s, size_t *e)
{
  size_t a = off;
  size_t b = off;
  while (a > 0u && f->buf[a - 1u] != (uint8_t)'\n') {
    a--;
  }
  while (b < f->n && f->buf[b] != (uint8_t)'\n') {
    b++;
  }
  *s = a;
  *e = b;
}

static void fuzz_mutate(fuzz_ctx *f)
{
  uint32_t pick = fuzz_below(f, 40u);
  size_t off = fuzz_below_sz(f, f->n + 1u);
  if (pick < 6u) { /* byte flip */
    if (f->n > 0u) {
      size_t i = fuzz_below_sz(f, f->n);
      f->buf[i] ^= (uint8_t)(1u << fuzz_below(f, 8u));
    }
  } else if (pick < 13u) { /* byte set from the dictionary */
    if (f->n > 0u) {
      size_t i = fuzz_below_sz(f, f->n);
      f->buf[i] = fuzz_bytes[fuzz_below(f, (uint32_t)sizeof fuzz_bytes)];
    }
  } else if (pick < 22u) { /* token insert */
    const char *t = fuzz_tokens[fuzz_below(f, (uint32_t)(sizeof fuzz_tokens / sizeof fuzz_tokens[0]))];
    fuzz_insert(f, off, (const uint8_t *)t, strlen(t));
  } else if (pick < 25u) { /* truncate */
    f->n = fuzz_below_sz(f, f->n + 1u);
  } else if (pick < 29u) { /* delete a short range */
    size_t k = 1u + fuzz_below_sz(f, 16u);
    if (off < f->n) {
      if (k > f->n - off) {
        k = f->n - off;
      }
      memmove(f->buf + off, f->buf + off + k, f->n - off - k);
      f->n -= k;
    }
  } else if (pick < 33u) { /* duplicate a line elsewhere */
    size_t s;
    size_t e;
    size_t k;
    size_t dst;
    if (f->n == 0u) {
      return;
    }
    fuzz_line_at(f, fuzz_below_sz(f, f->n), &s, &e);
    k = e - s + (e < f->n ? 1u : 0u);
    memcpy(f->tmp, f->buf + s, k);
    dst = fuzz_below_sz(f, f->n + 1u);
    fuzz_insert(f, dst, f->tmp, k);
  } else if (pick < 35u) { /* swap a line with the next one */
    size_t s1;
    size_t e1;
    size_t s2;
    size_t e2;
    if (f->n == 0u) {
      return;
    }
    fuzz_line_at(f, fuzz_below_sz(f, f->n), &s1, &e1);
    if (e1 + 1u >= f->n) {
      return;
    }
    fuzz_line_at(f, e1 + 1u, &s2, &e2);
    memcpy(f->tmp, f->buf + s2, e2 - s2);
    f->tmp[e2 - s2] = (uint8_t)'\n';
    memcpy(f->tmp + (e2 - s2) + 1u, f->buf + s1, e1 - s1);
    memcpy(f->buf + s1, f->tmp, e2 - s1);
  } else if (pick < 37u) { /* long line */
    size_t k = 200u + fuzz_below_sz(f, 101u);
    memset(f->tmp, 'a' + (int)fuzz_below(f, 3u), k);
    fuzz_insert(f, off, f->tmp, k);
  } else if (pick < 39u) { /* repeat one line many times */
    size_t s;
    size_t e;
    size_t k;
    uint32_t times = 30u + fuzz_below(f, 600u);
    uint32_t t;
    if (f->n == 0u) {
      return;
    }
    fuzz_line_at(f, fuzz_below_sz(f, f->n), &s, &e);
    k = e - s;
    if (k + 1u > 300u) {
      return;
    }
    memcpy(f->tmp, f->buf + s, k);
    f->tmp[k] = (uint8_t)'\n';
    for (t = 0u; t < times && f->n + k + 1u <= FUZZ_CAP; t++) {
      fuzz_insert(f, e < f->n ? e + 1u : f->n, f->tmp, k + 1u);
    }
  } else { /* pad with comment lines to near the 64 KiB edge */
    size_t target = IVS_SCN_MAX_BYTES - 8u + fuzz_below_sz(f, 17u);
    while (f->n + 2u <= target) {
      size_t k = target - f->n - 1u;
      if (k > 120u) {
        k = 120u;
      }
      f->tmp[0] = (uint8_t)'#';
      memset(f->tmp + 1u, 'p', k - 1u);
      f->tmp[k] = (uint8_t)'\n';
      fuzz_insert(f, f->n, f->tmp, k + 1u);
    }
  }
}

static int slug_ok(const char *s, size_t max)
{
  size_t n = 0u;
  while (n <= max && s[n] != '\0') {
    char c = s[n];
    if (!((c >= 'a' && c <= 'z') || (c >= '0' && c <= '9') || c == '-')) {
      return 0;
    }
    n++;
  }
  return n >= 1u && n <= max;
}

static int path_ok(const char *p)
{
  size_t n = 0u;
  while (n <= IVS_SCN_MAX_PATH && p[n] != '\0') {
    char c = p[n];
    if (!((c >= 'a' && c <= 'z') || (c >= 'A' && c <= 'Z') || (c >= '0' && c <= '9') || c == '.' || c == '_' ||
          c == '/' || c == '-')) {
      return 0;
    }
    if (n > 0u && c == '.' && p[n - 1u] == '.') {
      return 0;
    }
    if (c == '/' && (p[n + 1u] == '/' || p[n + 1u] == '\0')) {
      return 0;
    }
    n++;
  }
  return n >= 1u && n <= IVS_SCN_MAX_PATH && p[0] != '/';
}

static int digest_domain_ok(uint32_t d)
{
  return d == IVS_DIGEST_COMPOSITE || d == IVS_DIGEST_CPU || d == IVS_DIGEST_MEM || d == IVS_DIGEST_IMAGES ||
         d == IVS_DIGEST_INSTANCE || d == IVS_DIGEST_VIDEO || d == IVS_DIGEST_AUDIO || d == IVS_DIGEST_INPUT ||
         d == IVS_DIGEST_PSGLOG;
}

/* Structural rules of an accepted scenario. */
static int fuzz_valid(fuzz_ctx *f, const ivs_scenario *s)
{
  uint32_t i;
  FUZZ_CHECK(f, slug_ok(s->id, IVS_SCN_MAX_ID) && slug_ok(s->profile, IVS_SCN_MAX_PROFILE));
  for (i = 0u; i < IVS_SCN_ROLES; i++) {
    const ivs_scenario_content *c = &s->content[i];
    const ivs_scenario_expect *e = &s->expect[i];
    FUZZ_CHECK(f, c->kind <= IVS_SCN_CONTENT_BOOT_STUB_2);
    FUZZ_CHECK(f, c->kind != IVS_SCN_CONTENT_PATH || path_ok(c->path));
    FUZZ_CHECK(f, c->kind == IVS_SCN_CONTENT_PATH || c->path[0] == '\0');
    FUZZ_CHECK(f, c->kind != IVS_SCN_CONTENT_BOOT_STUB || i == IVS_SCN_ROLE_EXEC);
    FUZZ_CHECK(f, c->kind != IVS_SCN_CONTENT_BOOT_STUB_2 || i == IVS_SCN_ROLE_EXEC);
    FUZZ_CHECK(f, c->kind != IVS_SCN_CONTENT_ZERO_GROM || i == IVS_SCN_ROLE_GROM);
    FUZZ_CHECK(f, c->kind != IVS_SCN_CONTENT_NONE || (e->load_set == 0u && e->sha_set == 0u));
    FUZZ_CHECK(f, e->load_status <= 0 && (e->load_status == 0 || strcmp(ivs_status_name(e->load_status),
                                                                         "IVS_E_UNKNOWN") != 0));
    FUZZ_CHECK(f, e->load_reason == 0u || e->load_status != 0);
  }
  FUZZ_CHECK(f, s->n_steps <= IVS_SCN_MAX_STEPS && s->n_asserts <= IVS_SCN_MAX_ASSERTS);
  FUZZ_CHECK(f, s->n_inputs <= IVS_SCN_MAX_INPUTS);
  FUZZ_CHECK(f, (s->n_steps == 0u) == (s->step_unit == 0u));
  for (i = 0u; i < s->n_steps; i++) {
    const ivs_scenario_step *st = &s->steps[i];
    FUZZ_CHECK(f, st->target <= IVS_SCN_TIME_MAX && st->unit == s->step_unit);
    FUZZ_CHECK(f, st->stop >= IVS_SCN_STOP_TARGET && st->stop <= IVS_SCN_STOP_FAULTED);
    FUZZ_CHECK(f, i == 0u || st->target > s->steps[i - 1u].target);
  }
  FUZZ_CHECK(f, s->n_asserts == 0u || s->n_steps > 0u);
  /* Reachability (review WR-02, WR-03): without an expected refusal at least one run-to; with
   * one, no run-to, no input and no content for a role loaded after the first refused role. */
  for (i = 0u; i < IVS_SCN_ROLES; i++) {
    if (s->expect[i].load_set != 0u && s->expect[i].load_status != 0) {
      break;
    }
  }
  FUZZ_CHECK(f, i < IVS_SCN_ROLES || s->n_steps > 0u);
  if (i < IVS_SCN_ROLES) {
    FUZZ_CHECK(f, s->n_steps == 0u && s->n_inputs == 0u);
    for (i = i + 1u; i < IVS_SCN_ROLES; i++) {
      FUZZ_CHECK(f, s->content[i].kind == IVS_SCN_CONTENT_NONE);
    }
  }
  for (i = 0u; i < s->n_asserts; i++) {
    const ivs_scenario_assert *a = &s->asserts[i];
    FUZZ_CHECK(f, a->step < s->n_steps && (i == 0u || a->step >= s->asserts[i - 1u].step));
    FUZZ_CHECK(f, a->op == IVS_SCN_OP_EQ || a->op == IVS_SCN_OP_NE);
    FUZZ_CHECK(f, a->subject >= IVS_SCN_SUBJ_HALTED && a->subject <= IVS_SCN_SUBJ_VIDEO);
    FUZZ_CHECK(f, a->subject != IVS_SCN_SUBJ_HALTED || a->value <= 1u);
    FUZZ_CHECK(f, a->subject != IVS_SCN_SUBJ_REG || a->index <= 7u);
    FUZZ_CHECK(f, a->addr_lo <= a->addr_hi);
    FUZZ_CHECK(f, a->subject != IVS_SCN_SUBJ_DIGEST || digest_domain_ok(a->index));
    /* Plan 05-09: the stream asserts name a form, record forms name fields, and a record or an
     * `at` form has its capture directive somewhere in the file. */
    if (a->subject == IVS_SCN_SUBJ_PSGLOG) {
      FUZZ_CHECK(f, a->form == IVS_SCN_FORM_COUNT || a->form == IVS_SCN_FORM_RECORD);
      FUZZ_CHECK(f, a->form != IVS_SCN_FORM_RECORD || (a->index < IVS_SCN_CAPTURE_MAX && a->fields != 0u &&
                                                         a->fields <= 0xFu && a->addr <= 15u &&
                                                         (s->capture & IVS_SCN_CAPTURE_PSGLOG) != 0u));
      FUZZ_CHECK(f, a->form != IVS_SCN_FORM_RECORD ||
                        ((a->fields & IVS_SCN_FIELD_SOURCE) == 0u || a->source == 1u || a->source == 2u));
    } else if (a->subject == IVS_SCN_SUBJ_FRAME) {
      /* Plan 06-12: a frame assert names a form, its capture directive stands somewhere in the file. */
      FUZZ_CHECK(f, (s->capture & IVS_SCN_CAPTURE_VIDEO) != 0u && a->num <= 0x7FFFFFFFu);
      FUZZ_CHECK(f, a->form == IVS_SCN_FORM_PIXEL || a->form == IVS_SCN_FORM_HASH || a->form == IVS_SCN_FORM_DISPLAYED);
      FUZZ_CHECK(f, a->form != IVS_SCN_FORM_PIXEL || (a->index <= 65535u && a->addr <= 65535u &&
                                                       (a->level[0] <= 15u || a->level[0] == IVS_SCN_PIXEL_BLANK)));
      FUZZ_CHECK(f, a->form != IVS_SCN_FORM_DISPLAYED || a->value <= 1u);
    } else if (a->subject == IVS_SCN_SUBJ_VIDEO) {
      FUZZ_CHECK(f, a->form == IVS_SCN_FORM_COUNT || a->form == IVS_SCN_FORM_CHAIN);
    } else if (a->subject == IVS_SCN_SUBJ_AUDIO) {
      FUZZ_CHECK(f, a->form == IVS_SCN_FORM_EMITTED || a->form == IVS_SCN_FORM_AT);
      FUZZ_CHECK(f, a->form != IVS_SCN_FORM_AT || ((s->capture & IVS_SCN_CAPTURE_AUDIO) != 0u && a->level[0] <= 255u &&
                                                    a->level[1] <= 255u && a->level[2] <= 255u));
    }
  }
  FUZZ_CHECK(f, (s->capture & ~(IVS_SCN_CAPTURE_PSGLOG | IVS_SCN_CAPTURE_AUDIO | IVS_SCN_CAPTURE_VIDEO)) == 0u);
  FUZZ_CHECK(f, ((s->capture & IVS_SCN_CAPTURE_VIDEO) != 0u) == (s->capture_line[2] != 0u));
  FUZZ_CHECK(f, ((s->capture & IVS_SCN_CAPTURE_VIDEO) != 0u) == (s->video_count >= 1u && s->video_count <= IVS_SCN_CAPTURE_VIDEO_MAX));
  FUZZ_CHECK(f, ((s->capture & IVS_SCN_CAPTURE_PSGLOG) != 0u) == (s->capture_line[0] != 0u));
  FUZZ_CHECK(f, ((s->capture & IVS_SCN_CAPTURE_AUDIO) != 0u) == (s->capture_line[1] != 0u));
  FUZZ_CHECK(f, (s->n_inputs == 0u) == (s->input_unit == 0u));
  for (i = 0u; i < s->n_inputs; i++) {
    const ivs_scenario_input *in = &s->inputs[i];
    FUZZ_CHECK(f, in->at <= IVS_SCN_TIME_MAX && (i == 0u || in->at >= s->inputs[i - 1u].at));
    FUZZ_CHECK(f, ((in->controller[0] | in->controller[1]) & IVS_CTRL_RESERVED_MASK) == 0u);
  }
  return IVS_TEST_PASS;
}

/* Longest line in bytes, LF not counted. */
static size_t fuzz_longest_line(const uint8_t *b, size_t n)
{
  size_t best = 0u;
  size_t cur = 0u;
  size_t i;
  for (i = 0u; i < n; i++) {
    if (b[i] == (uint8_t)'\n') {
      cur = 0u;
    } else {
      cur++;
      if (cur > best) {
        best = cur;
      }
    }
  }
  return best;
}

static uint32_t fuzz_line_count(const uint8_t *b, size_t n)
{
  uint32_t lines = 0u;
  size_t i;
  for (i = 0u; i < n; i++) {
    if (b[i] == (uint8_t)'\n') {
      lines++;
    }
  }
  if (n > 0u && b[n - 1u] != (uint8_t)'\n') {
    lines++;
  }
  return lines;
}

static int fuzz_check_one(fuzz_ctx *f, ivs_scenario *s1, ivs_scenario *s2)
{
  uint8_t *copy = NULL;
  ivs_scenario_error e1;
  ivs_scenario_error e2;
  int r1;
  int r2;
  if (f->n > 0u) {
    copy = (uint8_t *)malloc(f->n);
    FUZZ_CHECK(f, copy != NULL);
    memcpy(copy, f->buf, f->n);
  }
  memset(s1, 0xA5, sizeof *s1);
  memset(s2, 0x5A, sizeof *s2);
  r1 = ivs_scenario_parse(copy, f->n, s1, &e1);
  r2 = ivs_scenario_parse(copy, f->n, s2, &e2);
  if (f->n > 0u && memcmp(copy, f->buf, f->n) != 0) {
    free(copy);
    ivs_test_diag("seed=0x%08X iter=%u: the parser wrote its input", (unsigned)f->seed, (unsigned)f->iter);
    return IVS_TEST_FAIL;
  }
  free(copy);
  FUZZ_CHECK(f, r1 == r2 && e1.line == e2.line && e1.reason == e2.reason);
  FUZZ_CHECK(f, memcmp(s1, s2, sizeof *s1) == 0);
  if (r1 == 0) {
    FUZZ_CHECK(f, e1.reason == IVS_SCN_R_NONE && e1.line == 0u);
    FUZZ_CHECK(f, f->n <= IVS_SCN_MAX_BYTES && fuzz_line_count(f->buf, f->n) <= IVS_SCN_MAX_LINES);
    FUZZ_CHECK(f, fuzz_longest_line(f->buf, f->n) <= IVS_SCN_MAX_LINE_BYTES);
    f->accepted++;
    return fuzz_valid(f, s1);
  }
  FUZZ_CHECK(f, r1 > 0 && (uint32_t)r1 < IVS_SCN_R_COUNT && (uint32_t)r1 == e1.reason);
  FUZZ_CHECK(f, strcmp(ivs_scenario_reason_name(e1.reason), "unknown") != 0);
  {
    const uint8_t *b = (const uint8_t *)s1;
    size_t i;
    for (i = 0u; i < sizeof *s1; i++) {
      FUZZ_CHECK(f, b[i] == 0u);
    }
  }
  if (e1.line == 0u) {
    FUZZ_CHECK(f, e1.reason == IVS_SCN_R_FILE_TOO_LARGE || e1.reason == IVS_SCN_R_MISSING_HEADER ||
                      e1.reason == IVS_SCN_R_MISSING_DIRECTIVE || e1.reason == IVS_SCN_R_NO_RUN_TO);
  } else {
    FUZZ_CHECK(f, e1.line <= fuzz_line_count(f->buf, f->n));
  }
  f->seen[e1.reason]++;
  return IVS_TEST_PASS;
}

static int fuzz_seed(fuzz_ctx *f, uint32_t seed)
{
  ivs_scenario *s1 = (ivs_scenario *)malloc(sizeof *s1);
  ivs_scenario *s2 = (ivs_scenario *)malloc(sizeof *s2);
  size_t n_bases = 0u;
  int rc = IVS_TEST_PASS;
  const char *text;
  size_t len;
  while (runner_parse_base(n_bases, &text, &len)) {
    n_bases++;
  }
  f->seed = seed;
  f->x = seed;
  if (s1 == NULL || s2 == NULL || n_bases == 0u) {
    free(s1);
    free(s2);
    ivs_test_diag("setup failed");
    return IVS_TEST_FAIL;
  }
  for (f->iter = 0u; f->iter < FUZZ_ITERS && rc == IVS_TEST_PASS; f->iter++) {
    uint32_t k;
    uint32_t muts = 1u + fuzz_below(f, 3u);
    (void)runner_parse_base(fuzz_below_sz(f, n_bases), &text, &len);
    memcpy(f->buf, text, len);
    f->n = len;
    for (k = 0u; k < muts; k++) {
      fuzz_mutate(f);
    }
    rc = fuzz_check_one(f, s1, s2);
  }
  free(s1);
  free(s2);
  return rc;
}

static int fuzz_run_seed(uint32_t index)
{
  fuzz_ctx *f = (fuzz_ctx *)calloc(1u, sizeof *f);
  int rc;
  if (f == NULL) {
    ivs_test_diag("out of memory");
    return IVS_TEST_FAIL;
  }
  rc = fuzz_seed(f, fuzz_seeds[index]);
  free(f);
  return rc;
}

static int fuzz_reasons_covered(void)
{
  fuzz_ctx *f = (fuzz_ctx *)calloc(1u, sizeof *f);
  uint32_t i;
  uint32_t distinct = 0u;
  if (f == NULL) {
    ivs_test_diag("out of memory");
    return IVS_TEST_FAIL;
  }
  for (i = 0u; i < FUZZ_SEEDS; i++) {
    if (fuzz_seed(f, fuzz_seeds[i]) != IVS_TEST_PASS) {
      free(f);
      return IVS_TEST_FAIL;
    }
  }
  for (i = 1u; i < IVS_SCN_R_COUNT; i++) {
    if (f->seen[i] != 0u) {
      distinct++;
    }
  }
  ivs_test_diag("accepted=%u distinct-reasons=%u", (unsigned)f->accepted, (unsigned)distinct);
  i = (f->accepted > 0u && distinct >= FUZZ_MIN_REASONS) ? 1u : 0u;
  free(f);
  return i != 0u ? IVS_TEST_PASS : IVS_TEST_FAIL;
}

size_t runner_fuzz_count(void)
{
  return FUZZ_SEEDS + 1u;
}

const char *runner_fuzz_id(size_t i)
{
  return i < FUZZ_SEEDS + 1u ? fuzz_ids[i] : NULL;
}

int runner_fuzz_run(size_t i)
{
  if (i < FUZZ_SEEDS) {
    return fuzz_run_seed((uint32_t)i);
  }
  return i == FUZZ_SEEDS ? fuzz_reasons_covered() : IVS_TEST_FAIL;
}

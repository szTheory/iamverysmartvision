/* SPDX-License-Identifier: MIT */
/* IVSL v1 writer and parser (TOOL-01, D-15, plan 11-02). The implementation of docs/formats/ivsl-v1.md: the four
 * header directives and the at_tick event lines, the seven fields of the document's "Fields" table, and the nine
 * refusal reasons of its "Refusals" table. Pure: no file function, no path, no feature macro, no allocation. The
 * writer fills a caller-supplied buffer and the parser reads one, so the log is memory here and the operator layer
 * (runner/main.c) is the only thing that opens a file.
 *
 * The writer emits only the seven fields of the table, which is what makes the absence of a content digest, an image
 * name, a length, a byte or a path a property of the bytes produced rather than a promise (D-15, A-11-05).
 */
#include <stddef.h>
#include <stdint.h>
#include <string.h>

#include "ivs/ivs.h"

#include "ivsl.h"

#define IVS_IVSL_LINE_MAX 128u /* the longest legal line is 73 bytes; a longer line is a line-shape fault */
#define IVS_IVSL_TOKENS 4      /* at_tick <n> c0=<HEX> c1=<HEX> is the longest directive */

static const char *const ivs_ivsl_reasons[] = {"ok",           "unknown-directive", "version", "arity", "hex",
                                               "trailing",     "tick-order",        "tick-bound", "event-cap",
                                               "controller-bit"};

const char *ivs_ivsl_reason_name(uint32_t reason)
{
  if (reason >= sizeof ivs_ivsl_reasons / sizeof ivs_ivsl_reasons[0]) {
    return "unknown";
  }
  return ivs_ivsl_reasons[reason];
}

/* 1 when s is 1..max bytes of [a-z0-9-] and nothing else (the document's name charset). */
static int ivs_ivsl_slug(const char *s, size_t max)
{
  size_t n = strlen(s);
  if (n == 0u || n > max) {
    return 0;
  }
  for (size_t i = 0u; i < n; i++) {
    char c = s[i];
    if (!((c >= 'a' && c <= 'z') || (c >= '0' && c <= '9') || c == '-')) {
      return 0;
    }
  }
  return 1;
}

/* 1 when s is 1..19 decimal digits with no sign, no 0x and no leading zero beyond the single digit 0. */
static int ivs_ivsl_dec(const char *s, uint64_t *v)
{
  size_t n = strlen(s);
  uint64_t x = 0u;
  if (n == 0u || n > 19u) {
    return 0;
  }
  if (n > 1u && s[0] == '0') {
    return 0;
  }
  for (size_t i = 0u; i < n; i++) {
    if (s[i] < '0' || s[i] > '9') {
      return 0;
    }
    x = x * 10u + (uint64_t)(s[i] - '0');
  }
  *v = x;
  return 1;
}

/* 1 when s is exactly eight upper-case hex digits. A lower-case digit is refused, never folded. */
static int ivs_ivsl_hex8(const char *s, uint32_t *v)
{
  uint32_t x = 0u;
  for (size_t i = 0u; i < 8u; i++) {
    char c = s[i];
    uint32_t d;
    if (c >= '0' && c <= '9') {
      d = (uint32_t)(c - '0');
    } else if (c >= 'A' && c <= 'F') {
      d = (uint32_t)(c - 'A') + 10u;
    } else {
      return 0;
    }
    x = (x << 4) | d;
  }
  if (s[8] != '\0') {
    return 0;
  }
  *v = x;
  return 1;
}

/* ---- the writer ---- */

static size_t ivs_ivsl_digits(uint64_t v)
{
  size_t n = 1u;
  while (v >= 10u) {
    v /= 10u;
    n++;
  }
  return n;
}

static char *ivs_ivsl_put(char *p, const char *s)
{
  size_t n = strlen(s);
  memcpy(p, s, n);
  return p + n;
}

static char *ivs_ivsl_put_u64(char *p, uint64_t v)
{
  size_t n = ivs_ivsl_digits(v);
  size_t i = n;
  while (i != 0u) {
    i--;
    p[i] = (char)('0' + (char)(v % 10u));
    v /= 10u;
  }
  return p + n;
}

static char *ivs_ivsl_put_hex8(char *p, uint32_t v)
{
  static const char digits[] = "0123456789ABCDEF";
  for (size_t i = 0u; i < 8u; i++) {
    p[i] = digits[(v >> (4u * (7u - i))) & 0xFu];
  }
  return p + 8u;
}

int ivs_ivsl_format(const ivs_ivsl_log *log, uint8_t *out, uint64_t cap, uint64_t *written)
{
  const char *content;
  size_t need;
  uint32_t n_lines;
  char *p;
  if (log == NULL || out == NULL || written == NULL) {
    return 1;
  }
  if (log->n_events > IVS_IVSL_MAX_EVENTS || !ivs_ivsl_slug(log->scenario, IVS_IVSL_MAX_ID) ||
      !ivs_ivsl_slug(log->profile, IVS_IVSL_MAX_PROFILE) ||
      (log->content != IVS_IVSL_CONTENT_PROJECT && log->content != IVS_IVSL_CONTENT_USER)) {
    return 1;
  }
  content = log->content == IVS_IVSL_CONTENT_USER ? "user" : "project";
  /* IVS_FAULT_RECORD_DROPS_LAST_EVENT (plan 11-05, TOOL-01): the writer emits one event line fewer than the log
   * holds, so a produced log claims one event fewer than the run applied. Both loops read n_lines, so the byte
   * count and the emitted lines stay consistent with each other: the log still parses, and it is short by one
   * event. What this must break is the recorded-and-replayed equality: the replayed applied-event chain and
   * input.events both differ from the recording run's. It must not be aimed at the format round trip, which
   * compares a log to a log. */
#ifdef IVS_FAULT_RECORD_DROPS_LAST_EVENT
  n_lines = log->n_events > 0u ? log->n_events - 1u : 0u;
#else
  n_lines = log->n_events;
#endif
  /* The exact byte count first: a buffer that is too small takes nothing, never a truncation. */
  need = strlen("ivsl 1\n") + strlen("scenario ") + strlen(log->scenario) + 1u + strlen("profile ") +
         strlen(log->profile) + 1u + strlen("content ") + strlen(content) + 1u;
  for (uint32_t i = 0u; i < n_lines; i++) {
    need += strlen("at_tick ") + ivs_ivsl_digits(log->events[i].at_tick) + strlen(" c0=") + 8u + strlen(" c1=") + 8u +
            1u;
  }
  if ((uint64_t)need > cap || need > IVS_IVSL_MAX_BYTES) {
    return 1;
  }
  p = (char *)out;
  p = ivs_ivsl_put(p, "ivsl 1\n");
  p = ivs_ivsl_put(p, "scenario ");
  p = ivs_ivsl_put(p, log->scenario);
  p = ivs_ivsl_put(p, "\nprofile ");
  p = ivs_ivsl_put(p, log->profile);
  p = ivs_ivsl_put(p, "\ncontent ");
  p = ivs_ivsl_put(p, content);
  *p++ = '\n';
  for (uint32_t i = 0u; i < n_lines; i++) {
    p = ivs_ivsl_put(p, "at_tick ");
    p = ivs_ivsl_put_u64(p, log->events[i].at_tick);
    p = ivs_ivsl_put(p, " c0=");
    p = ivs_ivsl_put_hex8(p, log->events[i].controller[0]);
    p = ivs_ivsl_put(p, " c1=");
    p = ivs_ivsl_put_hex8(p, log->events[i].controller[1]);
    *p++ = '\n';
  }
  *written = (uint64_t)need;
  return 0;
}

/* ---- the parser ---- */

typedef struct ivs_ivsl_state {
  ivs_ivsl_log log;
  uint32_t stage;    /* 0: ivsl next, 1: scenario, 2: profile, 3: content, 4: event lines */
  uint64_t prev_tick;
  uint32_t have_prev;
} ivs_ivsl_state;

/* Splits one line in place on exactly one space between tokens. Returns the token count, or -1 for a doubled,
 * leading or trailing space, or for more than max tokens (a line-shape fault either way). */
static int ivs_ivsl_split(char *l, char *tok[], int max)
{
  int n = 0;
  char *p = l;
  if (*p == '\0') {
    return 0;
  }
  for (;;) {
    char *sp = strchr(p, ' ');
    if (n == max) {
      return -1;
    }
    tok[n++] = p;
    if (sp == NULL) {
      break;
    }
    *sp = '\0';
    p = sp + 1;
    if (*p == '\0' || *p == ' ') {
      return -1;
    }
  }
  return n;
}

/* Judges one line (ll bytes, no LF). Returns 1 to go on, 0 after a refusal was set. */
static int ivs_ivsl_line(ivs_ivsl_state *st, const char *l, size_t ll, uint32_t line, ivs_ivsl_error *err)
{
  char buf[IVS_IVSL_LINE_MAX];
  char *tok[IVS_IVSL_TOKENS];
  int n;
  if (ll == 0u || ll >= sizeof buf) {
    /* An empty line, or a line longer than any directive of the grammar can be. */
    err->reason = IVS_IVSL_R_ARITY;
    err->line = line;
    return 0;
  }
  memcpy(buf, l, ll);
  buf[ll] = '\0';
  n = ivs_ivsl_split(buf, tok, IVS_IVSL_TOKENS);
  if (n < 1) {
    err->reason = IVS_IVSL_R_ARITY;
    err->line = line;
    return 0;
  }
  if (strcmp(tok[0], "ivsl") == 0) {
    if (st->stage != 0u) {
      err->reason = IVS_IVSL_R_ARITY; /* duplicated or out of order */
      err->line = line;
      return 0;
    }
    if (n != 2) {
      err->reason = IVS_IVSL_R_ARITY;
      err->line = line;
      return 0;
    }
    if (strcmp(tok[1], "1") != 0) {
      err->reason = IVS_IVSL_R_VERSION;
      err->line = line;
      return 0;
    }
    st->stage = 1u;
  } else if (strcmp(tok[0], "scenario") == 0) {
    if (st->stage != 1u || n != 2 || !ivs_ivsl_slug(tok[1], IVS_IVSL_MAX_ID)) {
      err->reason = IVS_IVSL_R_ARITY;
      err->line = line;
      return 0;
    }
    (void)memcpy(st->log.scenario, tok[1], strlen(tok[1]) + 1u);
    st->stage = 2u;
  } else if (strcmp(tok[0], "profile") == 0) {
    if (st->stage != 2u || n != 2 || !ivs_ivsl_slug(tok[1], IVS_IVSL_MAX_PROFILE)) {
      err->reason = IVS_IVSL_R_ARITY;
      err->line = line;
      return 0;
    }
    (void)memcpy(st->log.profile, tok[1], strlen(tok[1]) + 1u);
    st->stage = 3u;
  } else if (strcmp(tok[0], "content") == 0) {
    if (st->stage != 3u || n != 2) {
      err->reason = IVS_IVSL_R_ARITY;
      err->line = line;
      return 0;
    }
    if (strcmp(tok[1], "project") == 0) {
      st->log.content = IVS_IVSL_CONTENT_PROJECT;
    } else if (strcmp(tok[1], "user") == 0) {
      st->log.content = IVS_IVSL_CONTENT_USER;
    } else {
      err->reason = IVS_IVSL_R_ARITY;
      err->line = line;
      return 0;
    }
    st->stage = 4u;
  } else if (strcmp(tok[0], "at_tick") == 0) {
    uint64_t tick = 0u;
    uint32_t c0 = 0u;
    uint32_t c1 = 0u;
    if (st->stage != 4u || n != 4 || strncmp(tok[2], "c0=", 3u) != 0 || strncmp(tok[3], "c1=", 3u) != 0 ||
        !ivs_ivsl_dec(tok[1], &tick)) {
      err->reason = IVS_IVSL_R_ARITY;
      err->line = line;
      return 0;
    }
    if (!ivs_ivsl_hex8(tok[2] + 3, &c0) || !ivs_ivsl_hex8(tok[3] + 3, &c1)) {
      err->reason = IVS_IVSL_R_HEX;
      err->line = line;
      return 0;
    }
    if (st->log.n_events >= IVS_IVSL_MAX_EVENTS) {
      err->reason = IVS_IVSL_R_EVENT_CAP;
      err->line = line;
      return 0;
    }
    if (tick >= IVS_IVSL_TICK_MAX) {
      err->reason = IVS_IVSL_R_TICK_BOUND;
      err->line = line;
      return 0;
    }
    if (st->have_prev != 0u && tick < st->prev_tick) {
      err->reason = IVS_IVSL_R_TICK_ORDER;
      err->line = line;
      return 0;
    }
    if ((c0 & IVS_CTRL_RESERVED_MASK) != 0u || (c1 & IVS_CTRL_RESERVED_MASK) != 0u) {
      err->reason = IVS_IVSL_R_CONTROLLER_BIT;
      err->line = line;
      return 0;
    }
    st->log.events[st->log.n_events].at_tick = tick;
    st->log.events[st->log.n_events].controller[0] = c0;
    st->log.events[st->log.n_events].controller[1] = c1;
    st->log.n_events++;
    st->prev_tick = tick;
    st->have_prev = 1u;
  } else {
    err->reason = IVS_IVSL_R_UNKNOWN_DIRECTIVE;
    err->line = line;
    return 0;
  }
  return 1;
}

int ivs_ivsl_parse(const uint8_t *bytes, size_t len, ivs_ivsl_log *out, ivs_ivsl_error *err)
{
  ivs_ivsl_state st;
  size_t i;
  uint32_t line = 1u;
  if (bytes == NULL || out == NULL || err == NULL) {
    return -1;
  }
  if (len > (size_t)IVS_IVSL_MAX_BYTES) {
    return -1;
  }
  err->reason = IVS_IVSL_R_OK;
  err->line = 0u;
  memset(&st, 0, sizeof st);
  st.log.version = IVS_IVSL_VERSION;
  /* The file's shape first: printable ASCII and LF only. A byte outside that set is a trailing fault, named at the
   * line the byte sits on. */
  for (i = 0u; i < len; i++) {
    uint8_t c = bytes[i];
    if (c == (uint8_t)'\n') {
      line++;
    } else if (c < 0x20u || c > 0x7Eu) {
      err->reason = IVS_IVSL_R_TRAILING;
      err->line = line;
      return 1;
    }
  }
  if (len == 0u) {
    /* No first line to judge: the file cannot be a log. */
    err->reason = IVS_IVSL_R_ARITY;
    err->line = 0u;
    return 1;
  }
  line = 1u;
  i = 0u;
  while (i < len) {
    const uint8_t *nl = (const uint8_t *)memchr(bytes + i, '\n', len - i);
    size_t ll;
    if (nl == NULL) {
      /* The last line does not end in LF. */
      err->reason = IVS_IVSL_R_TRAILING;
      err->line = line;
      return 1;
    }
    ll = (size_t)(nl - (bytes + i));
    if (!ivs_ivsl_line(&st, (const char *)(bytes + i), ll, line, err)) {
      return 1;
    }
    i += ll + 1u;
    line++;
  }
  if (st.stage != 4u) {
    /* A header directive is missing: the last line is the last one the file has. */
    err->reason = IVS_IVSL_R_ARITY;
    err->line = line - 1u;
    return 1;
  }
  *out = st.log;
  return 0;
}

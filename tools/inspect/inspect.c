/* SPDX-License-Identifier: MIT */
/* ivs-inspect: the project's build-only inspection instrument (Phase 18, plan 18-02, TOOL-02 and
 * TOOL-09). One binary, four subcommands, no window, never installed, driven by a script only. The
 * frozen contract is docs/inspection.md: this file implements its `## Command surface` (the `trace`,
 * `break` and `step` grammars, `--help`, the three exit statuses), its output policy (the two output
 * refusals of `## Refusal tokens` and the `refs/` exception of `## Rights`), the `break` report of
 * `## Breakpoints` and its host. Where this file's prose and that page disagree, the page wins.
 *
 * `break` steps no machine: it reads a trace this tool wrote and counts, so the non-interference the
 * frozen page asserts is structural there - and it is still asserted by a case, because a later
 * implementation that re-ran the scenario to count would break it silently. Its count is derived
 * from the file while the trace's own end record carries the in-run count, so the two are two
 * independent computations of the same number and the case reconciles them rather than trusting
 * either. `step` creates its own instance through inspect_trace.c's own setup, so its unit count is
 * the trace's own `units` when it is given that number.
 *
 * What this file owns, and what it deliberately does not. The report is the library's: pass A parses
 * the scenario with ivs_scenario_parse and runs it with ivs_runner_execute, so the report's bytes are
 * the ones ivs-run prints for the same scenario and the same options, by construction. The record
 * that carries `digest[9][32]` and `step[k].now_ticks` is declared library-internal by
 * runner/ivs_runner.h, built as a local inside runner/exec.c and written there through
 * ivs_runner_report_write, so this tool can never hold one and never calls that writer. Its only
 * view of the report is the text the library hands `write_report`, and that text is therefore the
 * oracle the trace is reconciled against (tools/inspect/inspect_trace.c). Nothing here is a second
 * report writer: a second one would make the `digest.*` equality unfalsifiable.
 *
 * The report reaches stdout only after every check has passed. `write_report` captures the library's
 * bytes into this file's own buffer and prints nothing, and the buffer is written to stdout at the
 * end of a successful run. That is what the frozen page's exit status 1 requires - "exactly one
 * stderr line ... nothing on stdout; and no output file left behind" - so a refusal that happens
 * after the report pass (a failed reconciliation, a failed write) cannot leave a report on stdout.
 *
 * The host is this tool's own, and it is deliberately narrow. read_file resolves a relative name
 * under --content-dir only, refusing a `..` segment, a leading `/` and an empty segment, and serves
 * the two reserved boot-stub names the same way the runner does, because they are ordinary file
 * names under the content directory. read_user reports MISSING: the tool takes no operator image.
 * now_ms and cancel_requested are NULL, so the executor has no wall cap and no cancellation, which is
 * right for a tool a script drives to completion. The content of every role is read once, before
 * either pass, into this file's cache, and read_file serves that cache: the two passes are then given
 * the same bytes by construction, and the report's own `slot.<role>.sha256` lines are the oracle that
 * both loaded them.
 *
 * The output policy is decided before anything is created, in the order the frozen page states: the
 * physical parent of the output is compared with the physical work-tree root (IVS_INSPECT_TREE_ROOT,
 * resolved at run time) and refused when it is inside and not under refs/; then an existing path is
 * refused, a dangling symlink included; only then is the file created exclusively. The core never
 * sees a path: the tool opens and writes its own file, and no core file is edited by this phase.
 *
 * Exit statuses, and only three: 0 the work is done (the output file exists and the report is
 * written), 1 a named refusal (one stderr line `ivs-inspect: <token>: <detail>`, nothing on stdout, no
 * output file left), 2 a usage or file error (one stderr line, no token). Each refusal token is a
 * standalone double-quoted literal in this file or in inspect_trace.c, because the coverage id
 * extracts them from every source in this directory with `grep -h -o -E '"inspect-[a-z-]+"'`, and a
 * token assembled from fragments would read as an unreachable refusal.
 *
 * Naming discipline (GUARDRAILS-v0.4 W3, tools/check/rights-lib.cmake's naming limb): this directory
 * is inside the naming limb's scope, so the spellings the inspection-output ledger row refuses never
 * appear here, in tools/check/binary-ext.txt or in the operator pages this phase edits. The artifact
 * is named "the inspection trace" or "the disassembly dump" in prose; the row and docs/inspection.md
 * carry the refused spellings.
 *
 * The cross-file prototypes this program needs are declared at both ends - here and at the top of
 * inspect_trace.c and inspect_dis.c - because plan 18-02 pinned the command surface to its own file and
 * a header would be one more file. inspect_trace.c's contract: 0 the trace is written and reconciled, 1
 * the reconciliation refused (detail carries the reason and the caller prints the named refusal), 2 a
 * file or usage error (detail carries the message and the caller prints it with no token).
 * inspect_dis.c's contract: 0 the emitted source is in the buffer the caller frees, 1 a row refused
 * (`inspect-dis-row`, detail carries the detail), 2 a listing or bound error with no token. */

#define _POSIX_C_SOURCE 200809L

#include <errno.h>
#include <fcntl.h>
#include <inttypes.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <unistd.h>

#include "ivs/ivs_machine.h"
#include "ivs/ivs_run.h"
#include "ivs/ivs_status.h"
#include "ivs_runner.h"

/* The tool's own bounds. The path bound matches tools/flatwrap's; the report bound is the tool's, not
 * the library's: a report of a scenario at the parser's caps is a few hundred kilobytes. */
#define IX_PATH_MAX 4096
#define IX_REPORT_MAX (16u * 1024u * 1024u)
/* The trace the `break` surface reads back: the same 64 MiB bound the trace pass writes under. */
#define IX_TRACE_MAX (64u * 1024u * 1024u)
/* The listing `dis` reads: a listing is a few hundred lines at the listing grammar's caps. */
#define IX_LISTING_MAX (4u * 1024u * 1024u)
#define IX_DETAIL_MAX 320u

/* Exit statuses of the frozen page. */
#define IX_EXIT_OK 0
#define IX_EXIT_REFUSED 1
#define IX_EXIT_ERROR 2

/* Read results, the tool's own; the host maps them to IVS_RUNNER_READ_*. */
#define IX_READ_OK 0
#define IX_READ_MISSING 1
#define IX_READ_NOT_REGULAR 2
#define IX_READ_TOO_LARGE 3
#define IX_READ_ERROR 4

/* The output policy's two refusal tokens and `break`'s own, each one standalone double-quoted
 * literal: the coverage case extracts the tool's token set from every source in this directory with
 * `grep -h -o -E '"inspect-[a-z-]+"'`, so a token assembled from fragments would read as a page line
 * the tool cannot print. `inspect-reconcile` is written the same way where it is raised. */
static const char ix_tok_output_tree[] = "inspect-output-tree";
static const char ix_tok_output_exists[] = "inspect-output-exists";
static const char ix_tok_break_empty[] = "inspect-break-empty";
static const char ix_tok_dis_row[] = "inspect-dis-row";

/* The outcome of the output policy, in the frozen order: the tree, then existence, then the exclusive
 * create. IX_OPEN_REFUSED_TREE and IX_OPEN_REFUSED_EXISTS carry the caller's token; IX_OPEN_ERROR is a
 * file error with no token. */
#define IX_OPEN_OK 0
#define IX_OPEN_REFUSED_TREE 1
#define IX_OPEN_REFUSED_EXISTS 2
#define IX_OPEN_ERROR 3

/* One role's content, read once and handed to both passes. */
typedef struct ix_content {
  uint8_t *bytes[IVS_SCN_ROLES];
  size_t len[IVS_SCN_ROLES];
  int present[IVS_SCN_ROLES];
} ix_content;

/* The tool's host context: the content directory, the cache read_file serves, and the captured
 * report text. */
typedef struct ix_host {
  const char *content_dir;
  const ivs_scenario *scn;
  const ix_content *content;
  char *report;
  size_t report_len;
  size_t report_cap;
} ix_host;

/* The interface tools/inspect/inspect_dis.c implements. See that file's header for the contract: 0 the
 * emitted source is in *out_text (malloc'd, *out_len bytes, the caller owns it); 1 a row was refused
 * (detail carries the refusal's detail); 2 a bound, allocation or listing-grammar error (detail carries
 * the message, which carries no token). */
int ivs_inspect_dis_emit(const uint8_t *bytes, size_t len, char **out_text, size_t *out_len, char *detail,
                         size_t detail_cap);

/* The interface tools/inspect/inspect_trace.c implements. See the file header for the contract. */
int ivs_inspect_trace_run(const ivs_scenario *scn, uint8_t *const role_bytes[IVS_SCN_ROLES],
                          const size_t role_len[IVS_SCN_ROLES], const char *report_text, size_t report_len,
                          const char *stored_text, size_t stored_len, int have_break, uint16_t break_pc,
                          uint16_t other_pc, FILE *out, char *detail, size_t detail_cap);
int ivs_inspect_step_run(const ivs_scenario *scn, uint8_t *const role_bytes[IVS_SCN_ROLES],
                         const size_t role_len[IVS_SCN_ROLES], uint64_t units, int have_peek,
                         uint16_t peek_addr, uint32_t peek_count, FILE *out, char *detail, size_t detail_cap);

/* ---- the command surface ---- */

static const char *const ix_help_lines[] = {
  "ivs-inspect: the project's build-only inspection instrument.\n",
  "usage:\n",
  ("  ivs-inspect trace <scenario> --content-dir <dir> --out <file.ivst> [--report <file>] [--break <HHHH> "
   "--other <HHHH>]\n"),
  "  ivs-inspect break --pc <HHHH> --other <HHHH> --trace <file.ivst> --out <file>\n",
  "  ivs-inspect dis <listing> --out <file.asm>\n",
  "  ivs-inspect step <scenario> --content-dir <dir> --units <n> [--peek <HHHH> <count>]\n",
  "exit: 0 the work is done, 1 a named refusal, 2 a usage or file error.\n",
};

static int ix_usage(void)
{
  size_t i;
  (void)fprintf(stderr, "ivs-inspect: usage error\n");
  for (i = 0u; i < sizeof ix_help_lines / sizeof ix_help_lines[0]; i++) {
    (void)fputs(ix_help_lines[i], stderr);
  }
  return IX_EXIT_ERROR;
}

static int ix_help(void)
{
  size_t i;
  for (i = 0u; i < sizeof ix_help_lines / sizeof ix_help_lines[0]; i++) {
    (void)fputs(ix_help_lines[i], stdout);
  }
  return IX_EXIT_OK;
}

/* One stderr line, prefixed and carrying the token; nothing on stdout. */
static int ix_refuse(const char *token, const char *detail)
{
  (void)fprintf(stderr, "ivs-inspect: %s: %s\n", token, detail);
  return IX_EXIT_REFUSED;
}

/* Four hex digits, no prefix, upper or lower case. */
static int ix_hex4(const char *a, uint16_t *out)
{
  uint32_t v = 0u;
  int i;
  for (i = 0; i < 4; i++) {
    char c = a[i];
    uint32_t d;
    if (c >= '0' && c <= '9') {
      d = (uint32_t)(c - '0');
    } else if (c >= 'a' && c <= 'f') {
      d = (uint32_t)(c - 'a') + 10u;
    } else if (c >= 'A' && c <= 'F') {
      d = (uint32_t)(c - 'A') + 10u;
    } else {
      return 0;
    }
    v = (v << 4) | d;
  }
  if (a[4] != '\0') {
    return 0;
  }
  *out = (uint16_t)v;
  return 1;
}

/* ---- files ---- */

/* Reads at most cap + 1 bytes of a regular file, without blocking on a FIFO (the technique
 * runner/hostio.c and tools/flatwrap use for the same reason: a refusal that never returns is worse
 * than a wrong token). *len is the bytes read; IX_READ_TOO_LARGE means there were more than cap. */
static int ix_read_regular(const char *path, size_t cap, uint8_t *buf, size_t *len)
{
  struct stat sb;
  size_t got = 0u;
  int fd;
  *len = 0u;
  fd = open(path, O_RDONLY | O_NOCTTY | O_NONBLOCK);
  if (fd < 0) {
    return errno == ENOENT ? IX_READ_MISSING : IX_READ_ERROR;
  }
  if (fstat(fd, &sb) != 0) {
    (void)close(fd);
    return IX_READ_ERROR;
  }
  if (!S_ISREG(sb.st_mode)) {
    (void)close(fd);
    return IX_READ_NOT_REGULAR;
  }
  while (got < cap + 1u) {
    ssize_t n = read(fd, buf + got, cap + 1u - got);
    if (n < 0) {
      if (errno == EINTR) {
        continue;
      }
      (void)close(fd);
      return IX_READ_ERROR;
    }
    if (n == 0) {
      break;
    }
    got += (size_t)n;
  }
  (void)close(fd);
  *len = got;
  return got > cap ? IX_READ_TOO_LARGE : IX_READ_OK;
}

/* A scenario's relative name: no empty segment, no `..` segment, no leading `/`. The parser already
 * refuses these for a `content <role> <path>` line; the host checks again because the two reserved
 * boot-stub names never pass through the parser. */
static int ix_relpath_ok(const char *rel)
{
  const char *p = rel;
  if (rel[0] == '\0' || rel[0] == '/') {
    return 0;
  }
  while (*p != '\0') {
    const char *seg = p;
    while (*p != '\0' && *p != '/') {
      p++;
    }
    if (p == seg) {
      return 0;
    }
    if (p - seg == 2 && seg[0] == '.' && seg[1] == '.') {
      return 0;
    }
    if (*p == '/') {
      p++;
    }
  }
  return 1;
}

/* 1 when the physical parent directory of out lies inside the work tree and is not under refs/, 0 when
 * it is outside or under refs/, -1 when the question cannot be answered (detail is then filled). The
 * parent is resolved physically, so a spelling through `..` or through a symlink to a directory inside
 * the tree gets the same answer as the literal path: the technique tools/flatwrap uses for its own
 * output rule, and the one the frozen page's three spellings require. */
static int ix_inside_tree(const char *out, char *detail, size_t cap)
{
  char root[IX_PATH_MAX];
  char dir[IX_PATH_MAX];
  char phys[IX_PATH_MAX];
  char *slash;
  size_t rootlen;

  if (realpath(IVS_INSPECT_TREE_ROOT, root) == NULL) {
    (void)snprintf(detail, cap, "cannot resolve the work-tree root %s", IVS_INSPECT_TREE_ROOT);
    return -1;
  }
  if (strlen(out) >= sizeof dir) {
    (void)snprintf(detail, cap, "the output path is too long");
    return -1;
  }
  (void)memcpy(dir, out, strlen(out) + 1u);
  slash = strrchr(dir, '/');
  if (slash == NULL) {
    (void)memcpy(dir, ".", 2u);
  } else if (slash == dir) {
    dir[1] = '\0';
  } else {
    *slash = '\0';
  }
  if (realpath(dir, phys) == NULL) {
    (void)snprintf(detail, cap, "cannot resolve the output directory %s", dir);
    return -1;
  }
  rootlen = strlen(root);
  if (strcmp(phys, root) != 0 && !(strncmp(phys, root, rootlen) == 0 && phys[rootlen] == '/')) {
    return 0;
  }
  if (strcmp(phys + rootlen, "/refs") == 0 || strncmp(phys + rootlen, "/refs/", 6u) == 0) {
    return 0;
  }
  return 1;
}

/* The frozen page's output policy, decided before anything is created, in its own order: the
 * physical tree check, then existence (a dangling symlink included), then the exclusive create.
 * *created is set once the path is this run's to remove. Every subcommand's --out goes through this,
 * because the page states the rule for "an output" and not for one subcommand's output. */
static int ix_open_output(const char *out, FILE **f, int *created, char *detail, size_t cap)
{
  struct stat sb;
  int inside = ix_inside_tree(out, detail, cap);

  *f = NULL;
  *created = 0;
  if (inside < 0) {
    return IX_OPEN_ERROR;
  }
  if (inside != 0) {
    (void)snprintf(detail, cap, "the output %s is inside the work tree outside refs/", out);
    return IX_OPEN_REFUSED_TREE;
  }
  /* lstat, not stat: a dangling symlink is a path that exists, and the page refuses it. */
  if (lstat(out, &sb) == 0) {
    (void)snprintf(detail, cap, "the output %s already exists", out);
    return IX_OPEN_REFUSED_EXISTS;
  }
  *f = fopen(out, "wbx");
  if (*f != NULL) {
    *created = 1; /* from here on the path is this run's to remove */
    return IX_OPEN_OK;
  }
  if (errno == EEXIST) {
    (void)snprintf(detail, cap, "the output %s already exists", out);
    return IX_OPEN_REFUSED_EXISTS;
  }
  (void)snprintf(detail, cap, "cannot create %s", out);
  return IX_OPEN_ERROR;
}

/* ---- the host ---- */

/* The role whose content name is relpath, or -1. The three spellings are the executor's own
 * (runner/exec.c's ex_content_name): the two reserved boot-stub names and the scenario's path. */
static int ix_role_of(const ix_host *h, const char *relpath)
{
  uint32_t role;
  for (role = 0u; role < IVS_SCN_ROLES; role++) {
    const ivs_scenario_content *c;
    const char *name;
    if (h->content->present[role] == 0) {
      continue;
    }
    c = &h->scn->content[role];
    if (c->kind == IVS_SCN_CONTENT_BOOT_STUB) {
      name = IVS_RUNNER_BOOT_STUB_FILE;
    } else if (c->kind == IVS_SCN_CONTENT_BOOT_STUB_2) {
      name = IVS_RUNNER_BOOT_STUB_2_FILE;
    } else if (c->kind == IVS_SCN_CONTENT_PATH) {
      name = c->path;
    } else {
      continue; /* zero-grom is made, never read */
    }
    if (strcmp(name, relpath) == 0) {
      return (int)role;
    }
  }
  return -1;
}

/* Serves the cached bytes, at most cap + 1 of them, with the executor's contract: *len above cap means
 * the file was larger than the role's bound. */
static int ix_host_read(void *ctx, const char *relpath, size_t cap, uint8_t *buf, size_t *len)
{
  ix_host *h = ctx;
  char path[IX_PATH_MAX];
  int role;
  int r;

  *len = 0u;
  role = ix_role_of(h, relpath);
  if (role >= 0) {
    size_t n = h->content->len[(uint32_t)role];
    size_t take = n > cap + 1u ? cap + 1u : n;
    (void)memcpy(buf, h->content->bytes[(uint32_t)role], take);
    *len = take;
    return n > cap ? IVS_RUNNER_READ_TOO_LARGE : IVS_RUNNER_READ_OK;
  }
  if (!ix_relpath_ok(relpath)) {
    return IVS_RUNNER_READ_MISSING;
  }
  if (strlen(h->content_dir) + strlen(relpath) + 2u > sizeof path) {
    return IVS_RUNNER_READ_IO_ERROR;
  }
  (void)snprintf(path, sizeof path, "%s/%s", h->content_dir, relpath);
  r = ix_read_regular(path, cap, buf, len);
  switch (r) {
  case IX_READ_OK:
    return IVS_RUNNER_READ_OK;
  case IX_READ_MISSING:
    return IVS_RUNNER_READ_MISSING;
  case IX_READ_NOT_REGULAR:
    return IVS_RUNNER_READ_NOT_REGULAR;
  case IX_READ_TOO_LARGE:
    return IVS_RUNNER_READ_TOO_LARGE;
  default:
    return IVS_RUNNER_READ_IO_ERROR;
  }
}

static int ix_host_read_user(void *ctx, uint32_t role, size_t cap, uint8_t *buf, size_t *len)
{
  (void)ctx;
  (void)role;
  (void)cap;
  (void)buf;
  *len = 0u;
  return IVS_RUNNER_READ_MISSING;
}

/* Captures the library's bytes and nothing else: the report reaches stdout after every check has
 * passed (see the file header). A failed capture is a write error, which the executor turns into a
 * harness failure. */
static int ix_host_write_report(void *ctx, const char *bytes, size_t len)
{
  ix_host *h = ctx;
  size_t need = h->report_len + len;
  if (need + 1u > IX_REPORT_MAX) {
    return -1;
  }
  if (need + 1u > h->report_cap) {
    size_t want = h->report_cap == 0u ? 65536u : h->report_cap;
    char *p;
    while (want < need + 1u) {
      want *= 2u;
    }
    if (want > IX_REPORT_MAX) {
      want = IX_REPORT_MAX;
    }
    p = realloc(h->report, want);
    if (p == NULL) {
      return -1;
    }
    h->report = p;
    h->report_cap = want;
  }
  (void)memcpy(h->report + h->report_len, bytes, len);
  h->report_len = need;
  h->report[need] = '\0';
  return 0;
}

/* ---- content ---- */

/* Reads every role with a content line once, so both passes get the same bytes. zero-grom is made
 * here without a host call, exactly as runner/exec.c makes it. */
static int ix_load_content(const ivs_scenario *scn, const char *content_dir, ix_content *c, char *detail,
                           size_t cap)
{
  static const char *const role_name[IVS_SCN_ROLES] = {"exec", "grom", "cart"};
  static const size_t role_cap[IVS_SCN_ROLES] = {IVS_RUNNER_CAP_EXEC, IVS_RUNNER_CAP_GROM, IVS_RUNNER_CAP_CART};
  uint32_t role;

  memset(c, 0, sizeof *c);
  for (role = 0u; role < IVS_SCN_ROLES; role++) {
    const ivs_scenario_content *s = &scn->content[role];
    char path[IX_PATH_MAX];
    const char *name;
    size_t len = 0u;
    int r;
    if (s->kind == IVS_SCN_CONTENT_NONE) {
      continue;
    }
    if (s->kind == IVS_SCN_CONTENT_USER) {
      (void)snprintf(detail, cap, "role %s names an operator image, and this tool takes no operator image",
                     role_name[role]);
      return IX_EXIT_ERROR;
    }
    c->bytes[role] = malloc(role_cap[role] + 1u);
    if (c->bytes[role] == NULL) {
      (void)snprintf(detail, cap, "cannot allocate the content buffer of role %s", role_name[role]);
      return IX_EXIT_ERROR;
    }
    c->present[role] = 1;
    if (s->kind == IVS_SCN_CONTENT_ZERO_GROM) {
      memset(c->bytes[role], 0, IVS_RUNNER_CAP_GROM);
      c->len[role] = IVS_RUNNER_CAP_GROM;
      continue;
    }
    if (s->kind == IVS_SCN_CONTENT_BOOT_STUB) {
      name = IVS_RUNNER_BOOT_STUB_FILE;
    } else if (s->kind == IVS_SCN_CONTENT_BOOT_STUB_2) {
      name = IVS_RUNNER_BOOT_STUB_2_FILE;
    } else {
      name = s->path;
    }
    if (!ix_relpath_ok(name) || strlen(content_dir) + strlen(name) + 2u > sizeof path) {
      (void)snprintf(detail, cap, "role %s names a path this tool does not read", role_name[role]);
      return IX_EXIT_ERROR;
    }
    (void)snprintf(path, sizeof path, "%s/%s", content_dir, name);
    r = ix_read_regular(path, role_cap[role], c->bytes[role], &len);
    if (r != IX_READ_OK) {
      (void)snprintf(detail, cap, "role %s: %s (%s)", role_name[role],
                     r == IX_READ_MISSING       ? "missing"
                     : r == IX_READ_NOT_REGULAR ? "not a regular file"
                     : r == IX_READ_TOO_LARGE   ? "larger than the role's bound"
                                                : "cannot be read",
                     path);
      return IX_EXIT_ERROR;
    }
    c->len[role] = len;
  }
  return IX_EXIT_OK;
}

static void ix_free_content(ix_content *c)
{
  uint32_t role;
  for (role = 0u; role < IVS_SCN_ROLES; role++) {
    free(c->bytes[role]);
    c->bytes[role] = NULL;
  }
}

/* ---- the trace subcommand ---- */

/* One `key value` line of the captured report, copied into out (NUL-terminated). 1 when the key is
 * present exactly once. Used only to make a failure message say why the library stopped. */
static int ix_report_value(const char *text, const char *key, char *out, size_t cap)
{
  size_t klen = strlen(key);
  const char *p = text;
  int found = 0;
  while (*p != '\0') {
    const char *end = strchr(p, '\n');
    size_t n = end == NULL ? strlen(p) : (size_t)(end - p);
    if (n > klen && strncmp(p, key, klen) == 0 && p[klen] == ' ') {
      size_t vlen = n - klen - 1u;
      if (vlen >= cap) {
        vlen = cap - 1u;
      }
      (void)memcpy(out, p + klen + 1u, vlen);
      out[vlen] = '\0';
      found++;
    }
    if (end == NULL) {
      break;
    }
    p = end + 1;
  }
  if (found != 1 && cap > 0u) {
    out[0] = '\0';
  }
  return found == 1;
}

static int ix_cmd_trace(int argc, char **argv)
{
  const char *scenario_path;
  const char *content_dir = NULL;
  const char *out = NULL;
  const char *report_in = NULL;
  uint16_t break_pc = 0u;
  uint16_t other_pc = 0u;
  int have_break = 0;
  int have_other = 0;
  uint8_t *scn_bytes = NULL;
  size_t scn_len = 0u;
  ivs_scenario scn;
  ivs_scenario_error serr;
  ix_content content;
  ix_host h;
  ivs_runner_host host;
  ivs_runner_result res;
  void *storage = NULL;
  char *stored = NULL;
  size_t stored_len = 0u;
  char detail[IX_DETAIL_MAX];
  char outcome_detail[IX_DETAIL_MAX];
  FILE *f = NULL;
  int created = 0;
  int rc;
  int i;

  detail[0] = '\0';
  memset(&content, 0, sizeof content);
  memset(&h, 0, sizeof h);
  memset(&host, 0, sizeof host);

  if (argc < 3) {
    return ix_usage();
  }
  scenario_path = argv[2];
  for (i = 3; i < argc; i++) {
    if (strcmp(argv[i], "--content-dir") == 0) {
      if (content_dir != NULL || i + 1 >= argc) {
        return ix_usage();
      }
      content_dir = argv[++i];
    } else if (strcmp(argv[i], "--out") == 0) {
      if (out != NULL || i + 1 >= argc) {
        return ix_usage();
      }
      out = argv[++i];
    } else if (strcmp(argv[i], "--report") == 0) {
      if (report_in != NULL || i + 1 >= argc) {
        return ix_usage();
      }
      report_in = argv[++i];
    } else if (strcmp(argv[i], "--break") == 0) {
      if (have_break || i + 1 >= argc || !ix_hex4(argv[i + 1], &break_pc)) {
        return ix_usage();
      }
      have_break = 1;
      i++;
    } else if (strcmp(argv[i], "--other") == 0) {
      if (have_other || i + 1 >= argc || !ix_hex4(argv[i + 1], &other_pc)) {
        return ix_usage();
      }
      have_other = 1;
      i++;
    } else {
      return ix_usage();
    }
  }
  if (content_dir == NULL || out == NULL || have_break != have_other) {
    return ix_usage();
  }

  /* The scenario, read and parsed before anything is created. */
  scn_bytes = malloc((size_t)IVS_SCN_MAX_BYTES + 1u);
  if (scn_bytes == NULL) {
    (void)fprintf(stderr, "ivs-inspect: cannot allocate the scenario buffer\n");
    rc = IX_EXIT_ERROR;
    goto done;
  }
  if (ix_read_regular(scenario_path, (size_t)IVS_SCN_MAX_BYTES, scn_bytes, &scn_len) != IX_READ_OK) {
    (void)fprintf(stderr, "ivs-inspect: cannot read the scenario %s\n", scenario_path);
    rc = IX_EXIT_ERROR;
    goto done;
  }
  memset(&scn, 0, sizeof scn);
  memset(&serr, 0, sizeof serr);
  if (ivs_scenario_parse(scn_bytes, scn_len, &scn, &serr) != 0) {
    char text[IX_DETAIL_MAX];
    if (ivs_scenario_error_format(&serr, text, sizeof text) != 0) {
      (void)snprintf(text, sizeof text, "scenario-syntax line=%u", (unsigned)serr.line);
    }
    (void)fprintf(stderr, "ivs-inspect: %s\n", text);
    rc = IX_EXIT_ERROR;
    goto done;
  }
  free(scn_bytes);
  scn_bytes = NULL;

  /* The content, once per role, and the stored report when the flag names one. */
  rc = ix_load_content(&scn, content_dir, &content, detail, sizeof detail);
  if (rc != IX_EXIT_OK) {
    (void)fprintf(stderr, "ivs-inspect: %s\n", detail);
    goto done;
  }
  if (report_in != NULL) {
    stored = malloc(IX_REPORT_MAX);
    if (stored == NULL) {
      (void)fprintf(stderr, "ivs-inspect: cannot allocate the stored report buffer\n");
      rc = IX_EXIT_ERROR;
      goto done;
    }
    if (ix_read_regular(report_in, IX_REPORT_MAX - 1u, (uint8_t *)stored, &stored_len) != IX_READ_OK) {
      (void)fprintf(stderr, "ivs-inspect: cannot read the stored report %s\n", report_in);
      rc = IX_EXIT_ERROR;
      goto done;
    }
    stored[stored_len] = '\0';
  }

  /* The output policy, in the frozen order: the tree, then existence, then the exclusive create. */
  switch (ix_open_output(out, &f, &created, detail, sizeof detail)) {
  case IX_OPEN_OK:
    break;
  case IX_OPEN_REFUSED_TREE:
    rc = ix_refuse(ix_tok_output_tree, detail);
    goto done;
  case IX_OPEN_REFUSED_EXISTS:
    rc = ix_refuse(ix_tok_output_exists, detail);
    goto done;
  default:
    (void)fprintf(stderr, "ivs-inspect: %s\n", detail);
    rc = IX_EXIT_ERROR;
    goto done;
  }

  /* Pass A: the report, from the library, through this tool's host. */
  h.content_dir = content_dir;
  h.scn = &scn;
  h.content = &content;
  host.ctx = &h;
  host.read_file = ix_host_read;
  host.read_user = ix_host_read_user;
  host.now_ms = NULL;
  host.cancel_requested = NULL;
  host.write_report = ix_host_write_report;
  host.storage = malloc((size_t)IVS_RUNNER_STORAGE_MAX);
  if (host.storage == NULL) {
    (void)fprintf(stderr, "ivs-inspect: cannot allocate the report pass's storage\n");
    rc = IX_EXIT_ERROR;
    goto done;
  }
  storage = host.storage;
  host.storage_bytes = (size_t)IVS_RUNNER_STORAGE_MAX;
  host.buffers = NULL;
  host.buffer_bytes = 0u;
  memset(&res, 0, sizeof res);
  (void)ivs_runner_execute(&scn, &host, NULL, &res);
  free(storage);
  storage = NULL;
  host.storage = NULL;
  if (h.report == NULL || h.report_len == 0u) {
    (void)fprintf(stderr, "ivs-inspect: the report pass wrote no report\n");
    rc = IX_EXIT_ERROR;
    goto done;
  }
  if (res.outcome == IVS_RUNNER_OUTCOME_HARNESS_FAIL) {
    (void)ix_report_value(h.report, "outcome.detail", outcome_detail, sizeof outcome_detail);
    (void)fprintf(stderr, "ivs-inspect: the report pass ended harness-fail: %s\n",
                  outcome_detail[0] == '\0' ? "-" : outcome_detail);
    rc = IX_EXIT_ERROR;
    goto done;
  }

  /* Pass B and the reconciliation, in inspect_trace.c. */
  rc = ivs_inspect_trace_run(&scn, content.bytes, content.len, h.report, h.report_len, stored, stored_len,
                             have_break, break_pc, other_pc, f, detail, sizeof detail);
  if (fclose(f) != 0 && rc == IX_EXIT_OK) {
    (void)snprintf(detail, sizeof detail, "a write error on the output %s", out);
    rc = IX_EXIT_ERROR;
  }
  f = NULL;
  if (rc == IX_EXIT_REFUSED) {
    rc = ix_refuse("inspect-reconcile", detail);
  } else if (rc != IX_EXIT_OK) {
    (void)fprintf(stderr, "ivs-inspect: %s\n", detail);
    rc = IX_EXIT_ERROR;
  }

done:
  if (f != NULL) {
    (void)fclose(f);
  }
  if (rc != IX_EXIT_OK) {
    /* No output file is left behind when this run created one. A path that already existed is never
     * removed: that file is what the exists refusal exists to protect. */
    if (created != 0) {
      (void)remove(out);
    }
  } else {
    /* The report, printed only now that every check has passed. */
    (void)fwrite(h.report, 1u, h.report_len, stdout);
  }
  free(storage);
  free(stored);
  free(h.report);
  free(scn_bytes);
  ix_free_content(&content);
  return rc;
}

/* ---- the break subcommand ---- */

/* The value of `key` in one record line: 1 when the line holds exactly one `key=value` field, with
 * *val and *vlen naming it. The trace's grammar separates fields with single spaces and nothing else,
 * so a field is a space-delimited token whose text starts `key=`. */
static int ix_field(const char *line, size_t n, const char *key, const char **val, size_t *vlen)
{
  size_t klen = strlen(key);
  size_t at = 0u;
  int found = 0;

  while (at < n) {
    size_t end = at;
    while (end < n && line[end] != ' ') {
      end++;
    }
    if (end - at > klen && memcmp(line + at, key, klen) == 0 && line[at + klen] == '=') {
      *val = line + at + klen + 1u;
      *vlen = end - at - klen - 1u;
      found++;
    }
    at = end + 1u;
  }
  return found == 1;
}

/* A decimal field, at least one digit, no wrap. */
static int ix_field_dec(const char *line, size_t n, const char *key, uint64_t *out)
{
  const char *v;
  size_t vlen;
  uint64_t x = 0u;
  size_t i;

  if (!ix_field(line, n, key, &v, &vlen) || vlen == 0u) {
    return 0;
  }
  for (i = 0u; i < vlen; i++) {
    uint64_t d;
    if (v[i] < '0' || v[i] > '9') {
      return 0;
    }
    d = (uint64_t)(v[i] - '0');
    if (x > (UINT64_MAX - d) / 10u) {
      return 0;
    }
    x = x * 10u + d;
  }
  *out = x;
  return 1;
}

/* A four-hex-digit field. */
static int ix_field_hex4(const char *line, size_t n, const char *key, uint16_t *out)
{
  const char *v;
  size_t vlen;
  char buf[5];

  if (!ix_field(line, n, key, &v, &vlen) || vlen != 4u) {
    return 0;
  }
  (void)memcpy(buf, v, 4u);
  buf[4] = '\0';
  return ix_hex4(buf, out);
}

/* The frozen page's hit rule: `fetch` or `iab`. Both must count - the reset entry is an IAB carrying
 * the reset flag, not a fetch - so a rule that named only `fetch` would read the entry as never
 * executed. */
static int ix_kind_is_hit(const char *kind, size_t n)
{
  return (n == 5u && memcmp(kind, "fetch", 5u) == 0) || (n == 3u && memcmp(kind, "iab", 3u) == 0);
}

/* `break`: the file-derived count at each of the two addresses, over a trace this tool wrote. It steps
 * no machine, so the breakpoint cannot have moved anything - and the report's last three keys are
 * copied from the trace's own end record so the two documents can be compared rather than trusted. */
static int ix_cmd_break(int argc, char **argv)
{
  const char *trace_path = NULL;
  const char *out = NULL;
  uint16_t pc = 0u;
  uint16_t other = 0u;
  int have_pc = 0;
  int have_other = 0;
  uint8_t *text = NULL;
  size_t len = 0u;
  uint64_t hits = 0u;
  uint64_t other_hits = 0u;
  uint64_t end_tick = 0u;
  uint64_t end_records = 0u;
  uint64_t end_steps = 0u;
  int have_end = 0;
  size_t at = 0u;
  FILE *f = NULL;
  int created = 0;
  char detail[IX_DETAIL_MAX];
  int rc = IX_EXIT_ERROR;
  int i;

  detail[0] = '\0';
  if (argc < 3) {
    return ix_usage();
  }
  for (i = 2; i < argc; i++) {
    if (strcmp(argv[i], "--pc") == 0) {
      if (have_pc || i + 1 >= argc || !ix_hex4(argv[i + 1], &pc)) {
        return ix_usage();
      }
      have_pc = 1;
      i++;
    } else if (strcmp(argv[i], "--other") == 0) {
      if (have_other || i + 1 >= argc || !ix_hex4(argv[i + 1], &other)) {
        return ix_usage();
      }
      have_other = 1;
      i++;
    } else if (strcmp(argv[i], "--trace") == 0) {
      if (trace_path != NULL || i + 1 >= argc) {
        return ix_usage();
      }
      trace_path = argv[++i];
    } else if (strcmp(argv[i], "--out") == 0) {
      if (out != NULL || i + 1 >= argc) {
        return ix_usage();
      }
      out = argv[++i];
    } else {
      return ix_usage();
    }
  }
  if (have_pc == 0 || have_other == 0 || trace_path == NULL || out == NULL) {
    return ix_usage();
  }

  /* The trace, read whole before anything is created: an unreadable or malformed trace is a file
   * error (exit 2, no token) and must not leave an output behind. */
  text = malloc(IX_TRACE_MAX);
  if (text == NULL) {
    (void)fprintf(stderr, "ivs-inspect: cannot allocate the trace buffer\n");
    goto done;
  }
  if (ix_read_regular(trace_path, IX_TRACE_MAX - 1u, text, &len) != IX_READ_OK) {
    (void)fprintf(stderr, "ivs-inspect: cannot read the trace %s\n", trace_path);
    goto done;
  }
  while (at < len) {
    size_t end = at;
    const char *line = (const char *)text + at;
    size_t n;
    while (end < len && text[end] != '\n') {
      end++;
    }
    n = end - at;
    if (n > 5u && memcmp(line, "tick=", 5u) == 0) {
      const char *kind = NULL;
      size_t kind_len = 0u;
      uint16_t addr = 0u;
      if (!ix_field(line, n, "kind", &kind, &kind_len) || !ix_field_hex4(line, n, "addr", &addr)) {
        (void)fprintf(stderr, "ivs-inspect: the trace's access record is malformed\n");
        goto done;
      }
      if (ix_kind_is_hit(kind, kind_len) != 0) {
        if (addr == pc) {
          hits++;
        }
        if (addr == other) {
          other_hits++;
        }
      }
    } else if (n > 4u && memcmp(line, "end ", 4u) == 0) {
      if (!ix_field_dec(line, n, "tick", &end_tick) || !ix_field_dec(line, n, "records", &end_records) ||
          !ix_field_dec(line, n, "steps", &end_steps)) {
        (void)fprintf(stderr, "ivs-inspect: the trace's end record is malformed\n");
        goto done;
      }
      have_end = 1;
    }
    at = end + 1u;
  }
  if (have_end == 0) {
    (void)fprintf(stderr, "ivs-inspect: the trace %s holds no end record\n", trace_path);
    goto done;
  }
  /* `inspect-break-empty`: a trace with no access record has nothing to count a breakpoint against.
   * A trace with one access record yields 1 or 0 and is not refused. */
  if (end_records == 0u) {
    (void)snprintf(detail, sizeof detail, "the trace %s holds no access record to count against", trace_path);
    rc = ix_refuse(ix_tok_break_empty, detail);
    goto done;
  }

  switch (ix_open_output(out, &f, &created, detail, sizeof detail)) {
  case IX_OPEN_OK:
    break;
  case IX_OPEN_REFUSED_TREE:
    rc = ix_refuse(ix_tok_output_tree, detail);
    goto done;
  case IX_OPEN_REFUSED_EXISTS:
    rc = ix_refuse(ix_tok_output_exists, detail);
    goto done;
  default:
    (void)fprintf(stderr, "ivs-inspect: %s\n", detail);
    goto done;
  }

  /* The frozen page's break report, one `key value` line each and in this order. */
  if (fprintf(f, "break.pc %04X\n", (unsigned)pc) < 0 || fprintf(f, "break.hits %" PRIu64 "\n", hits) < 0 ||
      fprintf(f, "break.other.pc %04X\n", (unsigned)other) < 0 ||
      fprintf(f, "break.other.hits %" PRIu64 "\n", other_hits) < 0 ||
      fprintf(f, "break.records %" PRIu64 "\n", end_records) < 0 ||
      fprintf(f, "break.end_tick %" PRIu64 "\n", end_tick) < 0 ||
      fprintf(f, "break.steps %" PRIu64 "\n", end_steps) < 0) {
    (void)snprintf(detail, sizeof detail, "a write error on the break report");
    (void)fprintf(stderr, "ivs-inspect: %s\n", detail);
    goto done;
  }
  if (fclose(f) != 0) {
    f = NULL;
    (void)fprintf(stderr, "ivs-inspect: a write error on the break report\n");
    goto done;
  }
  f = NULL;
  rc = IX_EXIT_OK;

 done:
  if (f != NULL) {
    (void)fclose(f);
  }
  if (rc != IX_EXIT_OK && created != 0) {
    (void)remove(out);
  }
  free(text);
  return rc;
}

/* ---- the dis subcommand ---- */

/* `dis`: the frozen page's fourth subcommand. The listing is read whole and turned into source before
 * anything is created, because the row refusal is fail-closed and must leave no output behind; only
 * then is the output opened through the same ladder `trace` and `break` use, and a short write removes
 * what this run created. The transform itself, and the only knowledge of the listing's grammar, is
 * inspect_dis.c. */
static int ix_cmd_dis(int argc, char **argv)
{
  const char *listing_path;
  const char *out = NULL;
  uint8_t *bytes = NULL;
  size_t len = 0u;
  char *text = NULL;
  size_t text_len = 0u;
  char detail[IX_DETAIL_MAX];
  FILE *f = NULL;
  int created = 0;
  int rc = IX_EXIT_ERROR;
  int r;
  int i;

  detail[0] = '\0';
  if (argc < 3) {
    return ix_usage();
  }
  listing_path = argv[2];
  for (i = 3; i < argc; i++) {
    if (strcmp(argv[i], "--out") == 0) {
      if (out != NULL || i + 1 >= argc) {
        return ix_usage();
      }
      out = argv[++i];
    } else {
      return ix_usage();
    }
  }
  if (out == NULL) {
    return ix_usage();
  }

  bytes = malloc(IX_LISTING_MAX + 1u);
  if (bytes == NULL) {
    (void)fprintf(stderr, "ivs-inspect: cannot allocate the listing buffer\n");
    goto done;
  }
  if (ix_read_regular(listing_path, IX_LISTING_MAX, bytes, &len) != IX_READ_OK) {
    (void)fprintf(stderr, "ivs-inspect: cannot read the listing %s\n", listing_path);
    goto done;
  }
  r = ivs_inspect_dis_emit(bytes, len, &text, &text_len, detail, sizeof detail);
  if (r == 1) {
    rc = ix_refuse(ix_tok_dis_row, detail);
    goto done;
  }
  if (r != 0) {
    (void)fprintf(stderr, "ivs-inspect: %s\n", detail);
    goto done;
  }

  switch (ix_open_output(out, &f, &created, detail, sizeof detail)) {
  case IX_OPEN_OK:
    break;
  case IX_OPEN_REFUSED_TREE:
    rc = ix_refuse(ix_tok_output_tree, detail);
    goto done;
  case IX_OPEN_REFUSED_EXISTS:
    rc = ix_refuse(ix_tok_output_exists, detail);
    goto done;
  default:
    (void)fprintf(stderr, "ivs-inspect: %s\n", detail);
    goto done;
  }

  if (fwrite(text, 1u, text_len, f) != text_len) {
    (void)fclose(f);
    f = NULL;
    (void)fprintf(stderr, "ivs-inspect: a write error on %s\n", out);
    goto done;
  }
  if (fclose(f) != 0) {
    f = NULL;
    (void)fprintf(stderr, "ivs-inspect: a write error on %s\n", out);
    goto done;
  }
  f = NULL;
  rc = IX_EXIT_OK;

 done:
  if (f != NULL) {
    (void)fclose(f);
  }
  if (rc != IX_EXIT_OK && created != 0) {
    (void)remove(out);
  }
  free(text);
  free(bytes);
  return rc;
}

/* ---- the step subcommand ---- */

/* A whole argument as a decimal count. */
static int ix_dec(const char *a, uint64_t *out)
{
  uint64_t v = 0u;
  size_t i = 0u;

  if (a[0] == '\0') {
    return 0;
  }
  for (i = 0u; a[i] != '\0'; i++) {
    uint64_t d;
    if (a[i] < '0' || a[i] > '9') {
      return 0;
    }
    d = (uint64_t)(a[i] - '0');
    if (v > (UINT64_MAX - d) / 10u) {
      return 0;
    }
    v = v * 10u + d;
  }
  *out = v;
  return 1;
}

/* `step`: one instance, `--units` units of the same loop the trace pass runs, then the frozen page's
 * key order. Its witnesses are read through ivs_machine_cpu_state and ivs_machine_peek only, both
 * public and side-effect-free, so the report advances no time and changes no digest. */
static int ix_cmd_step(int argc, char **argv)
{
  const char *scenario_path;
  const char *content_dir = NULL;
  uint64_t units = 0u;
  int have_units = 0;
  int have_peek = 0;
  uint16_t peek_addr = 0u;
  uint32_t peek_count = 0u;
  uint8_t *scn_bytes = NULL;
  size_t scn_len = 0u;
  ivs_scenario scn;
  ivs_scenario_error serr;
  ix_content content;
  char detail[IX_DETAIL_MAX];
  int rc;
  int i;

  detail[0] = '\0';
  memset(&content, 0, sizeof content);
  if (argc < 3) {
    return ix_usage();
  }
  scenario_path = argv[2];
  for (i = 3; i < argc; i++) {
    if (strcmp(argv[i], "--content-dir") == 0) {
      if (content_dir != NULL || i + 1 >= argc) {
        return ix_usage();
      }
      content_dir = argv[++i];
    } else if (strcmp(argv[i], "--units") == 0) {
      if (have_units || i + 1 >= argc || !ix_dec(argv[i + 1], &units)) {
        return ix_usage();
      }
      have_units = 1;
      i++;
    } else if (strcmp(argv[i], "--peek") == 0) {
      uint64_t count = 0u;
      if (have_peek || i + 2 >= argc || !ix_hex4(argv[i + 1], &peek_addr) || !ix_dec(argv[i + 2], &count) ||
          count == 0u || count > 0xFFFFu) {
        return ix_usage();
      }
      peek_count = (uint32_t)count;
      have_peek = 1;
      i += 2;
    } else {
      return ix_usage();
    }
  }
  if (content_dir == NULL || have_units == 0 || units == 0u) {
    return ix_usage();
  }

  scn_bytes = malloc((size_t)IVS_SCN_MAX_BYTES + 1u);
  if (scn_bytes == NULL) {
    (void)fprintf(stderr, "ivs-inspect: cannot allocate the scenario buffer\n");
    rc = IX_EXIT_ERROR;
    goto done;
  }
  if (ix_read_regular(scenario_path, (size_t)IVS_SCN_MAX_BYTES, scn_bytes, &scn_len) != IX_READ_OK) {
    (void)fprintf(stderr, "ivs-inspect: cannot read the scenario %s\n", scenario_path);
    rc = IX_EXIT_ERROR;
    goto done;
  }
  memset(&scn, 0, sizeof scn);
  memset(&serr, 0, sizeof serr);
  if (ivs_scenario_parse(scn_bytes, scn_len, &scn, &serr) != 0) {
    char text[IX_DETAIL_MAX];
    if (ivs_scenario_error_format(&serr, text, sizeof text) != 0) {
      (void)snprintf(text, sizeof text, "scenario-syntax line=%u", (unsigned)serr.line);
    }
    (void)fprintf(stderr, "ivs-inspect: %s\n", text);
    rc = IX_EXIT_ERROR;
    goto done;
  }
  free(scn_bytes);
  scn_bytes = NULL;

  rc = ix_load_content(&scn, content_dir, &content, detail, sizeof detail);
  if (rc != IX_EXIT_OK) {
    (void)fprintf(stderr, "ivs-inspect: %s\n", detail);
    goto done;
  }
  rc = ivs_inspect_step_run(&scn, content.bytes, content.len, units, have_peek, peek_addr, peek_count, stdout,
                            detail, sizeof detail);
  if (rc != IX_EXIT_OK) {
    (void)fprintf(stderr, "ivs-inspect: %s\n", detail);
    rc = IX_EXIT_ERROR;
  }

 done:
  free(scn_bytes);
  ix_free_content(&content);
  return rc;
}

int main(int argc, char **argv)
{
  if (argc < 2) {
    return ix_usage();
  }
  if (strcmp(argv[1], "--help") == 0) {
    return ix_help();
  }
  if (strcmp(argv[1], "trace") == 0) {
    return ix_cmd_trace(argc, argv);
  }
  if (strcmp(argv[1], "break") == 0) {
    return ix_cmd_break(argc, argv);
  }
  if (strcmp(argv[1], "dis") == 0) {
    return ix_cmd_dis(argc, argv);
  }
  if (strcmp(argv[1], "step") == 0) {
    return ix_cmd_step(argc, argv);
  }
  /* A fifth name is a usage error (exit 2, no token), never a quiet alias: the frozen page's command
   * surface is four subcommands and no fifth. */
  return ix_usage();
}

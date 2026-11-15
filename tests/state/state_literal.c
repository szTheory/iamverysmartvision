/* SPDX-License-Identifier: MIT */
/* Suite state, family literal (plan 08-11, D-13, D-16): the hand-derived literals of the IVSN v1 container (plan 08-09) read through the
 * generated includes ivsn_literals.inc and ivsn_schema.inc (tests/ivsn-literals-inc.cmake), and the first cases that hold the code to
 * them. Nothing here is copied from the code under test: a number or a text is looked up by key in the data file the owner reviewed.
 * Task 1 of the plan: state.literal.params-digest, the profile parameter digest against its literal and against an independent
 * rebuild of its preimage from the four table body digests. The structure and schema cases of Task 2 are added to this file. */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "common/ivs_sha256.h"
#include "ivs/ivs.h"
#include "machine/machine.h"
#include "state_cases.h"

/* The literals: one row per non-comment line of data/expected/ivsn-literals.ids. */
typedef struct ivsn_lit_row {
  const char *key;
  const char *val;
} ivsn_lit_row;

#define IVSN_SOURCE(p, h)
#define IVSN_LIT(k, v) {k, v},
static const ivsn_lit_row ivsn_lits[] = {
#include "ivsn_literals.inc"
    {NULL, NULL}};
#undef IVSN_LIT
#undef IVSN_SOURCE

/* The schema literal: one string per line of data/state/schema.literal.txt. */
#define IVSN_SOURCE(p, h)
#define IVSN_SCHEMA_LINE(s) s,
static const char *const ivsn_schema[] = {
#include "ivsn_schema.inc"
};
#undef IVSN_SCHEMA_LINE
#undef IVSN_SOURCE

/* The data files read, for the `# source` lines of the result file. */
#define IVSN_LIT(k, v)
#define IVSN_SCHEMA_LINE(s)
#define STATE_CUT(c, p, t, f)
#define IVSN_SOURCE(p, h) {p, h},
static const ivs_test_source ivsn_sources[] = {
#include "ivsn_literals.inc"
#include "ivsn_schema.inc"
#include "state_cuts.inc" /* plan 08-14: the pinned ticks of the hard-point cuts are an oracle file the suite reads too */
};
#undef IVSN_SOURCE
#undef STATE_CUT
#undef IVSN_SCHEMA_LINE
#undef IVSN_LIT

const ivs_test_source *ivs_state_sources(size_t *n)
{
  *n = sizeof ivsn_sources / sizeof ivsn_sources[0];
  return ivsn_sources;
}

size_t ivsn_schema_line_count(void)
{
  return sizeof ivsn_schema / sizeof ivsn_schema[0];
}

const char *ivsn_schema_line(size_t i)
{
  return i < ivsn_schema_line_count() ? ivsn_schema[i] : NULL;
}

const char *ivsn_lit(const char *key)
{
  size_t i;
  for (i = 0u; ivsn_lits[i].key != NULL; i++) {
    if (strcmp(ivsn_lits[i].key, key) == 0) {
      return ivsn_lits[i].val;
    }
  }
  ivs_test_diag("no literal '%s' in data/expected/ivsn-literals.ids", key);
  return NULL;
}

int ivsn_lit_u32(const char *key, uint32_t *out)
{
  const char *v = ivsn_lit(key);
  uint64_t acc = 0u;
  size_t i;
  if (v == NULL || v[0] == '\0') {
    return 0;
  }
  for (i = 0u; v[i] != '\0'; i++) {
    if (v[i] < '0' || v[i] > '9') {
      ivs_test_diag("literal '%s' is not a decimal number: %s", key, v);
      return 0;
    }
    acc = acc * 10u + (uint64_t)(v[i] - '0');
    if (acc > 0xFFFFFFFFu) {
      ivs_test_diag("literal '%s' does not fit 32 bits: %s", key, v);
      return 0;
    }
  }
  *out = (uint32_t)acc;
  return 1;
}

static int st_hex_digit(char c)
{
  if (c >= '0' && c <= '9') {
    return c - '0';
  }
  if (c >= 'a' && c <= 'f') {
    return c - 'a' + 10;
  }
  if (c >= 'A' && c <= 'F') {
    return c - 'A' + 10;
  }
  return -1;
}

int ivsn_lit_hex(const char *key, uint8_t *out, size_t n)
{
  const char *v = ivsn_lit(key);
  size_t i;
  if (v == NULL) {
    return 0;
  }
  if (strlen(v) != 2u * n) {
    ivs_test_diag("literal '%s' is not %u hex bytes: %s", key, (unsigned)n, v);
    return 0;
  }
  for (i = 0u; i < n; i++) {
    int hi = st_hex_digit(v[2u * i]);
    int lo = st_hex_digit(v[2u * i + 1u]);
    if (hi < 0 || lo < 0) {
      ivs_test_diag("literal '%s' has a non-hex digit: %s", key, v);
      return 0;
    }
    out[i] = (uint8_t)(hi * 16 + lo);
  }
  return 1;
}

/* 1 when the 32 bytes are all zero. */
static uint32_t st_is_zero32(const uint8_t *p)
{
  uint32_t i;
  for (i = 0u; i < 32u; i++) {
    if (p[i] != 0u) {
      return 0u;
    }
  }
  return 1u;
}

/* state.literal.params-digest (Task 1, D-13): the profile parameter digest of the registered profile (a) equals the literal pinned in
 * data/expected/ivsn-literals.ids (a repeatability pin of the current profile files, never evidence of correctness), (b) equals the
 * SHA-256 of a preimage this test rebuilds from the specification alone (the ASCII text IVSP, a big-endian u16 1 and the four body
 * digests in the order memory map, PSG, STIC, controller), so the order and the framing are held by a second reader, and (c) refuses
 * what it must: an index one past the registry, a NULL out, with the out bytes untouched. The four accessors each give a non-NULL,
 * non-zero, pairwise different 32-byte value for the registered profile and NULL one past the registry. */
static int st_params_digest(void)
{
  uint32_t p = st_profile_index();
  uint32_t n = ivs_profile_count();
  uint8_t want[32];
  uint8_t got[32];
  uint8_t rebuilt[32];
  uint8_t pre[4u + 2u + 4u * 32u];
  uint8_t poison[32];
  const uint8_t *d[4];
  uint32_t i;
  uint32_t j;
  ST_CHECK(p < n);
  ST_CHECK(ivsn_lit_hex("params-digest-sha256", want, 32u));
  ST_CHECK(ivs_snapshot_params_digest(p, got) == IVS_OK);
  ST_CHECK(memcmp(got, want, 32u) == 0);

  d[0] = ivs_memmap_table_digest(p);
  d[1] = ivs_psg_table_digest(p);
  d[2] = ivs_stic_table_digest(p);
  d[3] = ivs_ctrl_table_digest(p);
  for (i = 0u; i < 4u; i++) {
    ST_CHECK(d[i] != NULL);
    ST_CHECK(st_is_zero32(d[i]) == 0u);
    for (j = 0u; j < i; j++) {
      ST_CHECK(memcmp(d[i], d[j], 32u) != 0);
    }
  }

  /* the specification's preimage, written byte by byte */
  pre[0] = (uint8_t)'I';
  pre[1] = (uint8_t)'V';
  pre[2] = (uint8_t)'S';
  pre[3] = (uint8_t)'P';
  pre[4] = 0x00u;
  pre[5] = 0x01u;
  for (i = 0u; i < 4u; i++) {
    memcpy(pre + 6u + 32u * i, d[i], 32u);
  }
  ivs_sha256_digest(pre, sizeof pre, rebuilt);
  ST_CHECK(memcmp(rebuilt, got, 32u) == 0);

  /* one past the registry: every accessor gives NULL, the digest refuses and leaves its out bytes alone */
  ST_CHECK(ivs_memmap_table_digest(n) == NULL);
  ST_CHECK(ivs_psg_table_digest(n) == NULL);
  ST_CHECK(ivs_stic_table_digest(n) == NULL);
  ST_CHECK(ivs_ctrl_table_digest(n) == NULL);
  ST_CHECK(ivs_memmap_table_digest(0xFFFFFFFFu) == NULL);
  memset(poison, 0xA5, sizeof poison);
  memcpy(rebuilt, poison, sizeof poison);
  ST_CHECK(ivs_snapshot_params_digest(n, rebuilt) == IVS_E_BAD_ARG);
  ST_CHECK(memcmp(rebuilt, poison, 32u) == 0);
  ST_CHECK(ivs_snapshot_params_digest(0xFFFFFFFFu, rebuilt) == IVS_E_BAD_ARG);
  ST_CHECK(memcmp(rebuilt, poison, 32u) == 0);
  ST_CHECK(ivs_snapshot_params_digest(p, NULL) == IVS_E_BAD_ARG);
  return IVS_TEST_PASS;
}

/* ---- the structure of a snapshot against the literals (Task 2) ---- */

const uint8_t st_section_domain[ST_SECTIONS] = {IVS_DIGEST_CPU,   IVS_DIGEST_MEM,   IVS_DIGEST_INSTANCE, IVS_DIGEST_VIDEO,
                                                IVS_DIGEST_AUDIO, IVS_DIGEST_INPUT, IVS_DIGEST_PSGLOG};
const char *const st_section_name[ST_SECTIONS] = {"cpu", "mem", "instance", "video", "audio", "input", "psglog"};

static uint64_t st_be(const uint8_t *p, uint32_t width)
{
  uint64_t v = 0u;
  uint32_t i;
  for (i = 0u; i < width; i++) {
    v = (v << 8) | p[i];
  }
  return v;
}

/* The value of the literal <prefix><section name> as a u32, or 0 with *ok cleared. */
static uint32_t st_sec_lit(const char *prefix, uint32_t i, int *ok)
{
  char key[48];
  uint32_t v = 0u;
  (void)snprintf(key, sizeof key, "%s%s", prefix, st_section_name[i]);
  if (!ivsn_lit_u32(key, &v)) {
    *ok = 0;
  }
  return v;
}

/* Every comparison of a snapshot with the literals and with the live instance it was taken from; see state_cases.h. The first failing
 * comparison is reported, so a test can name it. The expected numbers are read from data/expected/ivsn-literals.ids by key. */
int st_structure_diff(const uint8_t *snap, size_t len, const ivs_machine *m, const char **what)
{
  const struct ivs_machine *im = (const struct ivs_machine *)m;
  const ivs_profile_desc *pd;
  uint32_t total_lit = 0u;
  uint32_t trailer_lit = 0u;
  uint8_t pre[24];
  uint8_t want[32];
  uint8_t got[32];
  uint8_t idfield[IVS_PROFILE_ID_MAX_BYTES];
  int ok = 1;
  uint32_t i;
  size_t idlen;
  *what = "literals missing";
  if (!ivsn_lit_u32("total-bytes", &total_lit) || !ivsn_lit_u32("trailer-bytes", &trailer_lit)) {
    return 99;
  }
  *what = "the length of the snapshot differs from total-bytes";
  if (len != (size_t)total_lit) {
    return 1;
  }
  *what = "header bytes 0 to 23 differ from header-prefix-hex";
  if (!ivsn_lit_hex("header-prefix-hex", pre, sizeof pre)) {
    return 99;
  }
  if (memcmp(snap, pre, sizeof pre) != 0) {
    return 2;
  }
  *what = "a section table entry differs from the domain, section-offset or section-bytes literals";
  for (i = 0u; i < ST_SECTIONS; i++) {
    const uint8_t *e = snap + 304u + 44u * i;
    uint32_t off = st_sec_lit("section-offset.", i, &ok);
    uint32_t sz = st_sec_lit("section-bytes.", i, &ok);
    if (!ok) {
      return 99;
    }
    if (e[0] != st_section_domain[i] || e[1] != 0u || e[2] != 0u || e[3] != 0u || st_be(e + 4u, 4u) != off ||
        st_be(e + 8u, 4u) != sz) {
      return 3;
    }
  }
  *what = "the schema digest differs from schema-digest-sha256";
  if (!ivsn_lit_hex("schema-digest-sha256", want, 32u)) {
    return 99;
  }
  if (memcmp(snap + 24u, want, 32u) != 0) {
    return 4;
  }
  *what = "the profile id field is not the registered id NUL padded";
  pd = ivs_profile_get(im->profile);
  if (pd == NULL) {
    return 5;
  }
  memset(idfield, 0, sizeof idfield);
  idlen = strlen(pd->id);
  if (idlen >= sizeof idfield) {
    return 5;
  }
  memcpy(idfield, pd->id, idlen);
  if (memcmp(snap + 56u, idfield, sizeof idfield) != 0) {
    return 5;
  }
  *what = "the parameter digest differs from params-digest-sha256";
  if (!ivsn_lit_hex("params-digest-sha256", want, 32u)) {
    return 99;
  }
  if (memcmp(snap + 120u, want, 32u) != 0) {
    return 6;
  }
  *what = "a slot record differs from the live slot";
  for (i = 0u; i < IVS_SLOT_COUNT; i++) {
    const uint8_t *r = snap + 152u + 40u * i;
    if (st_be(r, 4u) != im->slot[i].loaded || memcmp(r + 4u, im->slot[i].sha256, 32u) != 0 ||
        st_be(r + 36u, 4u) != im->slot[i].length_bytes) {
      return 7;
    }
  }
  *what = "the composite digest differs from ivs_machine_state_digest";
  if (ivs_machine_state_digest(m, IVS_DIGEST_COMPOSITE, got) != IVS_OK || memcmp(snap + 272u, got, 32u) != 0) {
    return 8;
  }
  *what = "a section digest differs from ivs_machine_state_digest";
  for (i = 0u; i < ST_SECTIONS; i++) {
    if (ivs_machine_state_digest(m, st_section_domain[i], got) != IVS_OK || memcmp(snap + 304u + 44u * i + 12u, got, 32u) != 0) {
      return 9;
    }
  }
  *what = "the trailer differs from the SHA-256 of every earlier byte";
  ivs_sha256_digest(snap, (size_t)(total_lit - trailer_lit), got);
  if (memcmp(snap + (total_lit - trailer_lit), got, 32u) != 0) {
    return 10;
  }
  *what = "the bytes of a section differ from the state-only walk of its domain";
  for (i = 0u; i < ST_SECTIONS; i++) {
    uint32_t off = st_sec_lit("section-offset.", i, &ok);
    uint32_t sz = st_sec_lit("section-bytes.", i, &ok);
    uint8_t *tmp;
    size_t n;
    int same;
    if (!ok || (size_t)off + sz > len) {
      return 11;
    }
    tmp = (uint8_t *)malloc(sz);
    if (tmp == NULL) {
      return 99;
    }
    n = ivs_machine_state_preimage(im, st_section_domain[i], tmp, sz);
    same = n == sz && memcmp(snap + off, tmp, sz) == 0;
    free(tmp);
    if (!same) {
      return 11;
    }
  }
  *what = "";
  return 0;
}

/* The fixture of the structure cases: content A on a powered instance, stepped units units. NULL and a diagnostic on failure. */
static int st_snapshot_of(uint32_t content, uint32_t units, uint8_t *snap, size_t cap, size_t *n, ivs_ms_inst *x)
{
  uint64_t w = 0u;
  if (!ivs_ms_powered(x, content, 0x5Au)) {
    return 0;
  }
  if (units > 0u && !ivs_ms_step(x->m, units, NULL)) {
    ivs_ms_free(x);
    return 0;
  }
  if (ivs_machine_snapshot_save(x->m, snap, cap, &w) != IVS_OK) {
    ivs_test_diag("save refused");
    ivs_ms_free(x);
    return 0;
  }
  *n = (size_t)w;
  return 1;
}

/* state.literal.header-structure (Task 2, D-13, D-16): the container of a reset instance and of a stepped one equals the structure
 * derived on paper in plan 08-09: the first 24 header bytes, the seven section entries (domain, offset, length), the schema digest,
 * the registered profile id NUL padded, the parameter digest, the three slot records of the live slots, the composite and the seven
 * section digests of ivs_machine_state_digest and the trailer. Every number is a literal read by key; nothing is copied from the
 * writer. The comparison itself is proven able to fail by state.judge.header-literal-corrupt (plan 08-11, Task 3). */
static int st_header_structure(void)
{
  static const uint32_t units[3] = {0u, 100u, 3333u};
  uint8_t snap[IVS_SNAPSHOT_MAX_BYTES];
  uint32_t k;
  for (k = 0u; k < 3u; k++) {
    ivs_ms_inst x;
    size_t n = 0u;
    const char *what = "";
    int d;
    ST_CHECK(st_snapshot_of(IVS_MS_CONTENT_A, units[k], snap, sizeof snap, &n, &x));
    d = st_structure_diff(snap, n, x.m, &what);
    if (d != 0) {
      ivs_test_diag("structure comparison %d failed after %u units: %s", d, (unsigned)units[k], what);
    }
    ivs_ms_free(&x);
    ST_CHECK(d == 0);
  }
  return IVS_TEST_PASS;
}

/* state.literal.schema-digest (Task 2, D-13): the schema text the writer builds from the live table descriptors equals the hand-derived
 * data/state/schema.literal.txt line by line, and its SHA-256 equals schema-digest-sha256, which is also the schema digest field of a
 * snapshot. The lines are compared first so that a disagreement names the first differing descriptor line index: such a disagreement is
 * a finding about the hand derivation or the code, to be reported, never a reason to retarget the literal. */
static int st_schema_digest(void)
{
  uint8_t want[32];
  uint8_t got[32];
  char line[160];
  size_t i;
  size_t n = ivs_snapshot_schema_line_count();
  size_t first_bad = (size_t)-1;
  ST_CHECK(ivsn_lit_hex("schema-digest-sha256", want, 32u));
  if (n != ivsn_schema_line_count()) {
    ivs_test_diag("the table has %u descriptor lines, the literal %u", (unsigned)n, (unsigned)ivsn_schema_line_count());
    first_bad = n < ivsn_schema_line_count() ? n : ivsn_schema_line_count();
  }
  for (i = 0u; i < n && i < ivsn_schema_line_count(); i++) {
    size_t len = ivs_snapshot_schema_line(i, line, sizeof line);
    if (len != strlen(ivsn_schema_line(i)) || memcmp(line, ivsn_schema_line(i), len) != 0) {
      ivs_test_diag("descriptor line %u differs: code '%s', literal '%s'", (unsigned)i, line, ivsn_schema_line(i));
      first_bad = i < first_bad ? i : first_bad;
      break;
    }
  }
  ST_CHECK(first_bad == (size_t)-1);
  ivs_snapshot_schema_digest(got);
  ST_CHECK(memcmp(got, want, 32u) == 0);
  /* the digest field of a snapshot is that digest */
  {
    ivs_ms_inst x;
    uint8_t snap[IVS_SNAPSHOT_MAX_BYTES];
    uint64_t w = 0u;
    ST_CHECK(ivs_ms_powered(&x, IVS_MS_CONTENT_A, 0x5Au));
    ST_CHECK(ivs_machine_snapshot_save(x.m, snap, sizeof snap, &w) == IVS_OK);
    ivs_ms_free(&x);
    ST_CHECK(w > 56u && memcmp(snap + 24u, want, 32u) == 0);
  }
  /* a line past the last and a NULL or empty buffer give 0 */
  ST_CHECK(ivs_snapshot_schema_line(n, line, sizeof line) == 0u);
  ST_CHECK(ivs_snapshot_schema_line(0u, NULL, sizeof line) == 0u);
  ST_CHECK(ivs_snapshot_schema_line(0u, line, 0u) == 0u);
  return IVS_TEST_PASS;
}

const ivs_test_case ivs_state_literal_cases[] = {
    {"state.literal.params-digest", st_params_digest, NULL},
    {"state.literal.header-structure", st_header_structure, NULL},
    {"state.literal.schema-digest", st_schema_digest, NULL},
};
const size_t ivs_state_literal_count = sizeof ivs_state_literal_cases / sizeof ivs_state_literal_cases[0];

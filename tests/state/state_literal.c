/* SPDX-License-Identifier: MIT */
/* Suite state, family literal (plan 08-11, D-13, D-16): the hand-derived literals of the IVSN v1 container (plan 08-09) read through the
 * generated includes ivsn_literals.inc and ivsn_schema.inc (tests/ivsn-literals-inc.cmake), and the first cases that hold the code to
 * them. Nothing here is copied from the code under test: a number or a text is looked up by key in the data file the owner reviewed.
 * Task 1 of the plan: state.literal.params-digest, the profile parameter digest against its literal and against an independent
 * rebuild of its preimage from the four table body digests. The structure and schema cases of Task 2 are added to this file. */
#include <stdio.h>
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
#define IVSN_SOURCE(p, h) {p, h},
static const ivs_test_source ivsn_sources[] = {
#include "ivsn_literals.inc"
#include "ivsn_schema.inc"
};
#undef IVSN_SOURCE
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

const ivs_test_case ivs_state_literal_cases[] = {
    {"state.literal.params-digest", st_params_digest, NULL},
};
const size_t ivs_state_literal_count = sizeof ivs_state_literal_cases / sizeof ivs_state_literal_cases[0];

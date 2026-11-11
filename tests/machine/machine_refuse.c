/* SPDX-License-Identifier: MIT */
/* Suite machine, families refuse and prec (D-08, D-10..D-13, D-26, D-32, D-33, A-06, A-08,
 * plan 03-11): the load refusal matrix. Every row is one call against a prior instance, with the
 * full oracle of D-12:
 *   - the status the row names (its own typed status, never a neighbour's);
 *   - the whole host allocation (instance storage, plus the lead bytes of a lead row) byte for
 *     byte as it was before the call (memcmp of a copy taken after the prior and the input were
 *     built);
 *   - the composite state digest equal before and after (a destroyed instance has none);
 *   - the load report, poisoned with 0xA5 before the call, written only in its detail fields:
 *     struct_size, reason, segment_index, byte_offset and missing_features as the row expects,
 *     and an all-zero image; a row whose report is refused (short, or inside the instance) leaves
 *     every report byte poisoned.
 * Ids: machine.refuse.<op>.<kind>.<slug>[.<prior>], one id per row and prior, and
 * machine.prec.<slug> for the precedence rows (one fixed prior, no suffix). Priors:
 *   f  fresh;
 *   x  every kind except the one under test loaded (GROM and cartridge for an EXEC row, EXEC
 *      and cartridge for a GROM row, EXEC and GROM for a cartridge row);
 *   s  EXEC and GROM loaded;
 *   c  EXEC, GROM and cartridge loaded;
 *   w  EXEC and GROM loaded, then a power-on reset: the machine is POWERED (plan 03-12).
 * Rows decided before the run phase and slot occupancy (argument BAD_ARG, UNSUPPORTED) run on f,
 * s, c and w with the same status; rows decided after them (OVERSIZED, input-overlap BAD_ARG,
 * content, DIGEST_MISMATCH) run on f and x, where the slot under test is empty; DUPLICATE_LOAD
 * rows run only on an occupied slot (s and c, or c for the cartridge); BAD_STATE rows (every load
 * kind and unload while POWERED, D-13, A-08 d) run on w; handle rows carry no prior.
 * Reset rows (plan 03-12, D-14, A-08 e): machine.refuse.reset.<slug>, one fixed prior each, check
 * BAD_ARG for an unknown reset kind, BAD_HANDLE, and MISSING_SYSTEM_IMAGE with EXEC, GROM or both
 * empty, with the same storage and digest oracle (a reset has no report). Every lying length (a claimed length above the bound with a small buffer) runs
 * in two placements, a 16-byte heap buffer (`-heap`) and a pointer into the middle of the
 * instance storage (`-inside`): both are OVERSIZED, since a refusal status is a function of the
 * argument values, never of where the host placed its buffer (A-08 a).
 * A few positive controls sit in the matrix (`-ok` rows): an input that ends exactly where the
 * instance begins, adjacent cartridge segments, and a create whose out pointer follows the
 * instance bytes; they must load (or create) OK.
 * Out parameters aimed into the instance (plan 03-11 addition): the load report, the info out
 * struct, the digest out buffer and create's out pointer are refused BAD_ARG with nothing
 * written when they overlap the instance bytes.
 *
 * Every instance and input lives in a heap buffer of exactly its size (never on the stack), so
 * ASan sees any byte the core touches outside them. Inputs are generated stand-ins (D-24): EXEC
 * word i = (37 * i + 11) & 0x3FF high byte first, GROM byte i = (7 * i + 3) & 0xFF (the stand-ins
 * of family load), IVSC containers from the fixture records of data/cart/ (the prior cartridge is
 * ivsc.P02-two-segments, sealed at authoring time with `cmake -E sha256sum`), and containers
 * built here from the registry's cartridge span and sealed with the core SHA-256 (layer-2 rows,
 * so the parser's trailer check passes). Expected reasons, segments and offsets are literals
 * taken from the fixture records (data/cart/ivsc-parse.json) or worked out from the container
 * layout of docs/formats/ivsc-v1.md. No binary file is read or committed. */
#include <stddef.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>

#include "common/ivs_sha256.h"
#include "content/ivsc.h"
#include "ivs/ivs.h"
#include "ivs/ivs_machine.h"
#include "ivs_test.h"
#include "machine/machine.h"
#include "machine_cases.h"

#include "../ivsc/ivsc_table.h"

#include "ivs_cart_table.inc"

#define RF_PROFILE "intv-2609-ntsc-family-p1"
#define RF_EXEC_BYTES 8192u
#define RF_GROM_BYTES 2048u
#define RF_FAR_BYTES ((size_t)1u << 20)
#define RF_LEAD_BYTES 16384u

/* 2^32 + 8192: narrowed to 32 bits it would be a valid EXEC length (64-bit size_t only). */
#if SIZE_MAX > 0xFFFFFFFFu
#define RF_WRAP_LEN ((((size_t)1u) << 32) + (size_t)RF_EXEC_BYTES)
#define RF_SIZE_32 0
#else
#define RF_WRAP_LEN ((size_t)RF_EXEC_BYTES + 1u) /* never used: the rows are skipped */
#define RF_SIZE_32 1
#endif

/* Priors (bit mask of a row). */
#define RF_P_F 0x1u
#define RF_P_X 0x2u
#define RF_P_S 0x4u
#define RF_P_C 0x8u
#define RF_P_W 0x10u
#define RF_P_FSCW (RF_P_F | RF_P_S | RF_P_C | RF_P_W)
#define RF_P_FX (RF_P_F | RF_P_X)
#define RF_P_SC (RF_P_S | RF_P_C)

/* Operations. */
#define RF_OP_LOAD 1u
#define RF_OP_UNLOAD 2u
#define RF_OP_INFO 3u
#define RF_OP_DIGEST 4u
#define RF_OP_CREATE 5u
#define RF_OP_RESET 6u

/* Row flags. */
#define RF_REPORT_KEPT 0x1u /* the report is refused: every report byte stays poisoned */
#define RF_NO_SUFFIX 0x2u   /* one prior, id without a prior suffix (prec, handle and out rows) */
#define RF_EXPECT_OK 0x4u   /* a positive control: the call must succeed */
#define RF_LEAD 0x8u        /* the host allocation has RF_LEAD_BYTES before the instance */
#define RF_DESTROYED 0x10u  /* the instance is destroyed before the call (no digest) */
#define RF_SKIP32 0x20u     /* skipped where size_t is 32 bits */

#define RF_NONE IVS_SEGMENT_NONE

typedef struct rf_ctx rf_ctx;
typedef int (*rf_build_fn)(rf_ctx *c);

typedef struct rf_row {
  const char *id; /* without the prior suffix */
  uint32_t op;
  uint32_t kind;   /* the kind argument of the call */
  uint32_t priors; /* RF_P_* mask */
  rf_build_fn build;
  const char *fixture; /* the IVSC fixture record a fixture builder reads, or NULL */
  ivs_status want;
  uint32_t reason;
  uint32_t seg;
  uint64_t off;
  uint32_t missing;
  uint32_t flags;
} rf_row;

/* One row run: the host allocation, the instance in it and the call's arguments. */
struct rf_ctx {
  const rf_row *row;
  uint8_t *alloc;     /* the host allocation: lead bytes, then the instance */
  size_t alloc_bytes; /* lead + IVS_MACHINE_BYTES */
  uint8_t *inst;      /* instance storage */
  ivs_machine *m;
  ivs_machine *handle; /* the handle passed (default m) */
  uint8_t *buf;        /* heap input, freed after the row */
  const uint8_t *bytes;
  size_t len;
  uint8_t *optbuf; /* heap options, freed after the row */
  const ivs_load_options *opts;
  ivs_load_report *report; /* the report passed (default the runner's poisoned one) */
  uint32_t report_size;    /* when nonzero, the struct_size written into the runner's report */
  ivs_info *info_out;      /* RF_OP_INFO */
  uint8_t *digest_out;     /* RF_OP_DIGEST */
};

/* ---- stand-in images ---- */

static uint16_t rf_exec_word(uint32_t i)
{
  return (uint16_t)((37u * i + 11u) & 0x3FFu);
}

/* n bytes (at least 1 allocated) holding the EXEC stand-in pattern, zero past 8192. */
static uint8_t *rf_exec_bytes(size_t n)
{
  size_t cap = n != 0u ? n : 1u;
  uint8_t *b = malloc(cap);
  size_t i;
  if (b == NULL) {
    return NULL;
  }
  memset(b, 0, cap);
  for (i = 0u; i < n && i < RF_EXEC_BYTES; i += 2u) {
    uint16_t w = rf_exec_word((uint32_t)(i / 2u));
    b[i] = (uint8_t)(w >> 8);
    if (i + 1u < n) {
      b[i + 1u] = (uint8_t)(w & 0xFFu);
    }
  }
  return b;
}

/* n bytes (at least 1 allocated) holding the GROM stand-in pattern. */
static uint8_t *rf_grom_bytes(size_t n)
{
  size_t cap = n != 0u ? n : 1u;
  uint8_t *b = malloc(cap);
  size_t i;
  if (b == NULL) {
    return NULL;
  }
  memset(b, 0, cap);
  for (i = 0u; i < n; i++) {
    b[i] = (uint8_t)((7u * i + 3u) & 0xFFu);
  }
  return b;
}

/* The fixture record id of the generated table, or NULL. */
static const ivs_cart_fixture *rf_fixture(const char *id)
{
  size_t i;
  for (i = 0u; id != NULL && i < ivs_cart_count; i++) {
    if (strcmp(ivs_cart_table[i].id, id) == 0) {
      return &ivs_cart_table[i];
    }
  }
  return NULL;
}

/* A fixture record's literal bytes (positive or patch) in a fresh heap buffer of exactly their
 * size, resealed with the core SHA-256 when reseal is set and the record asks for it. */
static uint8_t *rf_fixture_copy(const char *id, int reseal, size_t *len)
{
  const ivs_cart_fixture *f = rf_fixture(id);
  uint8_t *b;
  if (f == NULL || f->bytes == NULL || f->n_bytes == 0u) {
    return NULL;
  }
  b = malloc(f->n_bytes);
  if (b == NULL) {
    return NULL;
  }
  memcpy(b, f->bytes, f->n_bytes);
  if (reseal && f->reseal != 0u && f->n_bytes >= IVS_IVSC_TRAILER_BYTES) {
    ivs_sha256_digest(b, f->n_bytes - IVS_IVSC_TRAILER_BYTES, b + f->n_bytes - IVS_IVSC_TRAILER_BYTES);
  }
  *len = f->n_bytes;
  return b;
}

/* The registry record of the test profile. */
static const ivs_profile_desc *rf_profile(void)
{
  uint32_t index = 0u;
  if (ivs_machine_profile_lookup(RF_PROFILE, &index) != IVS_OK) {
    return NULL;
  }
  return ivs_profile_get(index);
}

static void rf_put16(uint8_t *p, uint32_t v)
{
  p[0] = (uint8_t)((v >> 8) & 0xFFu);
  p[1] = (uint8_t)(v & 0xFFu);
}

static void rf_reseal(uint8_t *b, size_t len)
{
  ivs_sha256_digest(b, len - IVS_IVSC_TRAILER_BYTES, b + len - IVS_IVSC_TRAILER_BYTES);
}

typedef struct rf_seg {
  uint32_t addr;
  uint32_t len_words;
} rf_seg;

/* An IVSC v1 container of the given segments (word k of segment i is (97 i + 13 k + 5) & 0x3FF)
 * and required_features, sealed with the core SHA-256, in a heap buffer of exactly its size. */
static uint8_t *rf_cart_build(const rf_seg *segs, uint32_t n, uint32_t features, size_t *out_len)
{
  static const uint8_t magic[8] = {0x89u, 0x49u, 0x56u, 0x53u, 0x43u, 0x0Du, 0x0Au, 0x1Au};
  size_t words = 0u;
  size_t total;
  size_t at;
  uint32_t i;
  uint32_t k;
  uint8_t *b;
  for (i = 0u; i < n; i++) {
    words += segs[i].len_words;
  }
  total = IVS_IVSC_PREFIX_BYTES + IVS_IVSC_ENTRY_BYTES * (size_t)n + 2u * words + IVS_IVSC_TRAILER_BYTES;
  b = malloc(total);
  if (b == NULL) {
    return NULL;
  }
  memset(b, 0, total);
  memcpy(b, magic, sizeof magic);
  rf_put16(b + 8, 1u);
  rf_put16(b + 12, features >> 16);
  rf_put16(b + 14, features & 0xFFFFu);
  rf_put16(b + 16, (uint32_t)(total >> 16));
  rf_put16(b + 18, (uint32_t)(total & 0xFFFFu));
  rf_put16(b + 20, n);
  at = IVS_IVSC_PREFIX_BYTES + IVS_IVSC_ENTRY_BYTES * (size_t)n;
  for (i = 0u; i < n; i++) {
    uint8_t *e = b + IVS_IVSC_PREFIX_BYTES + IVS_IVSC_ENTRY_BYTES * i;
    rf_put16(e, segs[i].addr);
    rf_put16(e + 2, segs[i].len_words);
    e[4] = (uint8_t)IVS_IVSC_WIDTH_BITS;
    for (k = 0u; k < segs[i].len_words; k++) {
      rf_put16(b + at, (97u * i + 13u * k + 5u) & 0x3FFu);
      at += 2u;
    }
  }
  rf_reseal(b, total);
  *out_len = total;
  return b;
}

/* ---- priors ---- */

static int rf_load_one(ivs_machine *m, uint32_t kind)
{
  uint8_t *b = NULL;
  size_t len = 0u;
  ivs_status st;
  switch (kind) {
  case IVS_KIND_EXEC:
    len = RF_EXEC_BYTES;
    b = rf_exec_bytes(len);
    break;
  case IVS_KIND_GROM:
    len = RF_GROM_BYTES;
    b = rf_grom_bytes(len);
    break;
  default:
    b = rf_fixture_copy("ivsc.P02-two-segments", 1, &len);
    break;
  }
  if (b == NULL) {
    return 0;
  }
  st = ivs_machine_load(m, kind, b, len, NULL, NULL);
  free(b);
  if (st != IVS_OK) {
    ivs_test_diag("prior: load of kind %u refused (%s)", (unsigned)kind, ivs_status_name(st));
    return 0;
  }
  return 1;
}

/* Loads the kinds of a prior. kind is the row's kind (the one prior x leaves empty). */
static int rf_prior(ivs_machine *m, uint32_t prior, uint32_t kind)
{
  uint32_t k;
  for (k = IVS_KIND_EXEC; k <= IVS_KIND_CARTRIDGE; k++) {
    int want;
    switch (prior) {
    case RF_P_X:
      want = k != kind;
      break;
    case RF_P_S:
      want = k != IVS_KIND_CARTRIDGE;
      break;
    case RF_P_C:
      want = 1;
      break;
    case RF_P_W:
      want = k != IVS_KIND_CARTRIDGE;
      break;
    default:
      want = 0;
      break;
    }
    if (want && !rf_load_one(m, k)) {
      return 0;
    }
  }
  if (prior == RF_P_W && ivs_machine_reset(m, IVS_RESET_POWER_ON) != IVS_OK) {
    ivs_test_diag("prior w: power-on reset refused");
    return 0;
  }
  return 1;
}

/* A 16-byte-aligned offset inside the instance's cartridge words at $A000, a region no prior
 * loads: report and out rows aim their pointer here. */
static size_t rf_inside_off(void)
{
  size_t off = offsetof(struct ivs_machine, mem) + offsetof(ivs_memsys, cart) + 2u * 0x6000u;
  return (off + 15u) & ~(size_t)15u;
}

/* ---- builders: inputs ---- */

static int rf_take(rf_ctx *c, uint8_t *b, size_t len)
{
  c->buf = b;
  c->bytes = b;
  c->len = len;
  return b != NULL;
}

/* The exact size of a kind's valid input (a cartridge: the 66-byte minimum). */
static size_t rf_kind_len(uint32_t kind)
{
  switch (kind) {
  case IVS_KIND_GROM:
    return RF_GROM_BYTES;
  case IVS_KIND_CARTRIDGE:
    return IVS_IVSC_MIN_BYTES;
  default:
    return RF_EXEC_BYTES;
  }
}

/* A valid input of the row's kind: the EXEC or GROM stand-in, or ivsc.P01-min (EXEC for an
 * unknown kind). */
static int rf_b_valid(rf_ctx *c)
{
  size_t n = 0u;
  switch (c->row->kind) {
  case IVS_KIND_GROM:
    return rf_take(c, rf_grom_bytes(RF_GROM_BYTES), RF_GROM_BYTES);
  case IVS_KIND_CARTRIDGE: {
    uint8_t *b = rf_fixture_copy("ivsc.P01-min", 1, &n);
    return rf_take(c, b, n);
  }
  default:
    return rf_take(c, rf_exec_bytes(RF_EXEC_BYTES), RF_EXEC_BYTES);
  }
}

static int rf_b_none(rf_ctx *c)
{
  (void)c;
  return 1;
}

static int rf_b_null_8(rf_ctx *c)
{
  c->bytes = NULL;
  c->len = 8u;
  return 1;
}

static int rf_b_null_machine_only(rf_ctx *c)
{
  c->handle = NULL;
  return 1;
}

static int rf_b_null_machine(rf_ctx *c)
{
  c->handle = NULL;
  return rf_b_valid(c);
}

/* Heap options of bytes bytes, zero except struct_size and flags. */
static int rf_opts(rf_ctx *c, size_t bytes, uint32_t struct_size, uint32_t flags)
{
  ivs_load_options *o;
  c->optbuf = malloc(bytes);
  if (c->optbuf == NULL) {
    return 0;
  }
  memset(c->optbuf, 0, bytes);
  o = (ivs_load_options *)(void *)c->optbuf;
  o->struct_size = struct_size;
  o->flags = flags;
  c->opts = o;
  return 1;
}

static int rf_b_opts_short(rf_ctx *c)
{
  return rf_b_valid(c) && rf_opts(c, sizeof(ivs_load_options), (uint32_t)sizeof(ivs_load_options) - 1u, 0u);
}

static int rf_b_opts_size0(rf_ctx *c)
{
  return rf_b_valid(c) && rf_opts(c, sizeof(ivs_load_options), 0u, 0u);
}

static int rf_b_flags_bit1(rf_ctx *c)
{
  return rf_b_valid(c) && rf_opts(c, sizeof(ivs_load_options), (uint32_t)sizeof(ivs_load_options), 0x2u);
}

static int rf_b_flags_bit31(rf_ctx *c)
{
  return rf_b_valid(c) && rf_opts(c, sizeof(ivs_load_options), (uint32_t)sizeof(ivs_load_options), 0x80000000u);
}

static int rf_b_null_flags_bit1(rf_ctx *c)
{
  c->handle = NULL;
  return rf_b_flags_bit1(c);
}

/* A larger options struct (48 bytes) whose unknown tail holds one nonzero byte. */
static int rf_b_opts_tail(rf_ctx *c)
{
  size_t n = sizeof(ivs_load_options) + 8u;
  if (!rf_b_valid(c) || !rf_opts(c, n, (uint32_t)n, 0u)) {
    return 0;
  }
  c->optbuf[sizeof(ivs_load_options) + 4u] = 1u;
  return 1;
}

/* expect_sha256 set to the input's digest with its last bit flipped. */
static int rf_expect_bad(rf_ctx *c)
{
  ivs_load_options *o;
  if (!rf_opts(c, sizeof(ivs_load_options), (uint32_t)sizeof(ivs_load_options), IVS_LOAD_F_EXPECT_SHA256)) {
    return 0;
  }
  o = (ivs_load_options *)(void *)c->optbuf;
  ivs_sha256_digest(c->bytes, c->len, o->expect_sha256);
  o->expect_sha256[31] ^= 0x01u;
  return 1;
}

static int rf_b_expect_mismatch(rf_ctx *c)
{
  return rf_b_valid(c) && rf_expect_bad(c);
}

static int rf_b_report_short(rf_ctx *c)
{
  c->report_size = (uint32_t)sizeof(ivs_load_report) - 1u;
  return rf_b_valid(c);
}

/* The report points into the instance, with a well-formed struct_size there, so only the overlap
 * refuses it (the poke happens before the snapshot). */
static int rf_b_report_inside(rf_ctx *c)
{
  ivs_load_report *r = (ivs_load_report *)(void *)(c->inst + rf_inside_off());
  r->struct_size = (uint32_t)sizeof *r;
  c->report = r;
  return rf_b_valid(c);
}

/* A valid-length input whose pointer lies inside the instance. */
static int rf_b_input_inside(rf_ctx *c)
{
  c->bytes = c->inst + rf_inside_off();
  c->len = rf_kind_len(c->row->kind);
  return 1;
}

/* An 8192-byte input starting 4096 bytes before the instance (lead row). */
static int rf_b_input_straddle_below(rf_ctx *c)
{
  c->bytes = c->inst - RF_EXEC_BYTES / 2u;
  c->len = RF_EXEC_BYTES;
  return 1;
}

/* The EXEC stand-in in the 8192 lead bytes ending exactly where the instance begins. */
static int rf_b_input_adjacent_below(rf_ctx *c)
{
  uint8_t *img = rf_exec_bytes(RF_EXEC_BYTES);
  if (img == NULL) {
    return 0;
  }
  memcpy(c->inst - RF_EXEC_BYTES, img, RF_EXEC_BYTES);
  free(img);
  c->bytes = c->inst - RF_EXEC_BYTES;
  c->len = RF_EXEC_BYTES;
  return 1;
}

static int rf_b_exec_8193(rf_ctx *c)
{
  return rf_take(c, rf_exec_bytes(RF_EXEC_BYTES + 1u), RF_EXEC_BYTES + 1u);
}

static int rf_b_grom_2049(rf_ctx *c)
{
  return rf_take(c, rf_grom_bytes(RF_GROM_BYTES + 1u), RF_GROM_BYTES + 1u);
}

/* A lying length: a small heap buffer, or a pointer into the middle of the instance. */
static int rf_lying_heap(rf_ctx *c, size_t len)
{
  uint8_t *b = malloc(16u);
  if (b != NULL) {
    memset(b, 0, 16u);
  }
  return rf_take(c, b, len);
}

static int rf_lying_inside(rf_ctx *c, size_t len)
{
  c->bytes = c->inst + IVS_MACHINE_BYTES / 2u;
  c->len = len;
  return 1;
}

static int rf_b_far_heap(rf_ctx *c)
{
  return rf_lying_heap(c, RF_FAR_BYTES);
}

static int rf_b_far_inside(rf_ctx *c)
{
  return rf_lying_inside(c, RF_FAR_BYTES);
}

static int rf_b_cart_over_heap(rf_ctx *c)
{
  return rf_lying_heap(c, (size_t)IVS_IVSC_MAX_BYTES + 1u);
}

static int rf_b_cart_over_inside(rf_ctx *c)
{
  return rf_lying_inside(c, (size_t)IVS_IVSC_MAX_BYTES + 1u);
}

/* 2^32 + 8192 over a whole EXEC stand-in, so a length narrowed to 32 bits would load it. */
static int rf_b_wrap_heap(rf_ctx *c)
{
  return rf_take(c, rf_exec_bytes(RF_EXEC_BYTES), RF_WRAP_LEN);
}

static int rf_b_wrap_inside(rf_ctx *c)
{
  return rf_lying_inside(c, RF_WRAP_LEN);
}

static int rf_b_zero_ptr(rf_ctx *c)
{
  return rf_take(c, rf_exec_bytes(0u), 0u);
}

static int rf_b_zero_null(rf_ctx *c)
{
  c->bytes = NULL;
  c->len = 0u;
  return 1;
}

static int rf_b_exec_8191(rf_ctx *c)
{
  return rf_take(c, rf_exec_bytes(RF_EXEC_BYTES - 1u), RF_EXEC_BYTES - 1u);
}

static int rf_b_exec_4097(rf_ctx *c)
{
  return rf_take(c, rf_exec_bytes(4097u), 4097u);
}

static int rf_b_exec_1(rf_ctx *c)
{
  return rf_take(c, rf_exec_bytes(1u), 1u);
}

static int rf_b_grom_2047(rf_ctx *c)
{
  return rf_take(c, rf_grom_bytes(RF_GROM_BYTES - 1u), RF_GROM_BYTES - 1u);
}

static int rf_b_wide_first(rf_ctx *c)
{
  if (!rf_take(c, rf_exec_bytes(RF_EXEC_BYTES), RF_EXEC_BYTES)) {
    return 0;
  }
  c->buf[0] |= 0x04u;
  return 1;
}

static int rf_b_wide_last(rf_ctx *c)
{
  if (!rf_take(c, rf_exec_bytes(RF_EXEC_BYTES), RF_EXEC_BYTES)) {
    return 0;
  }
  c->buf[RF_EXEC_BYTES - 2u] |= 0x80u;
  return 1;
}

/* The row's fixture record (resealed as the record asks). */
static int rf_b_fixture(rf_ctx *c)
{
  size_t n = 0u;
  uint8_t *b = rf_fixture_copy(c->row->fixture, 1, &n);
  return rf_take(c, b, n);
}

/* The row's fixture record as patched, never resealed. */
static int rf_b_fixture_raw(rf_ctx *c)
{
  size_t n = 0u;
  uint8_t *b = rf_fixture_copy(c->row->fixture, 0, &n);
  return rf_take(c, b, n);
}

/* ---- builders: containers from the registry span ---- */

static int rf_cart1(rf_ctx *c, uint32_t addr, uint32_t len_words, uint32_t features)
{
  rf_seg seg;
  size_t n = 0u;
  seg.addr = addr;
  seg.len_words = len_words;
  return rf_take(c, rf_cart_build(&seg, 1u, features, &n), n);
}

static int rf_span_lo(uint32_t *lo)
{
  const ivs_profile_desc *p = rf_profile();
  if (p == NULL || p->has_cart_span == 0u || p->cart_lo == 0u) {
    return 0;
  }
  *lo = p->cart_lo;
  return 1;
}

static int rf_b_span_below_one(rf_ctx *c)
{
  uint32_t lo = 0u;
  return rf_span_lo(&lo) && rf_cart1(c, lo - 1u, 1u, 0u);
}

static int rf_b_span_straddle(rf_ctx *c)
{
  uint32_t lo = 0u;
  return rf_span_lo(&lo) && rf_cart1(c, lo - 1u, 2u, 0u);
}

static int rf_b_cart_exec_region(rf_ctx *c)
{
  return rf_cart1(c, 0x1000u, 1u, 0u);
}

static int rf_b_feature_0(rf_ctx *c)
{
  uint32_t lo = 0u;
  return rf_span_lo(&lo) && rf_cart1(c, lo, 4u, 0x1u);
}

static int rf_b_feature_31(rf_ctx *c)
{
  uint32_t lo = 0u;
  return rf_span_lo(&lo) && rf_cart1(c, lo, 4u, 0x80000000u);
}

/* prec: an entry of width 8 at $FFFF of two words (it would also wrap). */
static int rf_b_width_and_wrap(rf_ctx *c)
{
  if (!rf_cart1(c, 0xFFFFu, 2u, 0u)) {
    return 0;
  }
  c->buf[IVS_IVSC_PREFIX_BYTES + 4u] = 8u;
  rf_reseal(c->buf, c->len);
  return 1;
}

/* prec: entry 0 wraps ($FFF0, 32 words) and entry 1 ($FFF8) overlaps it. */
static int rf_b_wrap_and_overlap(rf_ctx *c)
{
  static const rf_seg segs[2] = {{0xFFF0u, 0x20u}, {0xFFF8u, 1u}};
  size_t n = 0u;
  return rf_take(c, rf_cart_build(segs, 2u, 0u, &n), n);
}

/* prec: overlapping segments below the span. */
static int rf_b_overlap_below_span(rf_ctx *c)
{
  static const rf_seg segs[2] = {{0x0100u, 4u}, {0x0102u, 1u}};
  size_t n = 0u;
  return rf_take(c, rf_cart_build(segs, 2u, 0u, &n), n);
}

/* prec: a segment below the span with a wrong trailer. */
static int rf_b_trailer_below_span(rf_ctx *c)
{
  if (!rf_cart1(c, 0x0100u, 1u, 0u)) {
    return 0;
  }
  c->buf[c->len - 1u] ^= 0x01u;
  return 1;
}

/* Sets the first data word (one entry) to 0x0400 and reseals. */
static void rf_first_word_400(rf_ctx *c)
{
  rf_put16(c->buf + IVS_IVSC_PREFIX_BYTES + IVS_IVSC_ENTRY_BYTES, 0x0400u);
  rf_reseal(c->buf, c->len);
}

/* prec: a word above 0x3FF in a container that also needs feature bit 0. */
static int rf_b_word_and_feature(rf_ctx *c)
{
  uint32_t lo = 0u;
  if (!rf_span_lo(&lo) || !rf_cart1(c, lo, 2u, 0x1u)) {
    return 0;
  }
  rf_first_word_400(c);
  return 1;
}

/* prec: a word above 0x3FF in a segment below the span. */
static int rf_b_word_below_span(rf_ctx *c)
{
  if (!rf_cart1(c, 0x0100u, 1u, 0u)) {
    return 0;
  }
  rf_first_word_400(c);
  return 1;
}

/* prec: a segment below the span in a container that also needs feature bit 0. */
static int rf_b_span_and_feature(rf_ctx *c)
{
  uint32_t lo = 0u;
  return rf_span_lo(&lo) && rf_cart1(c, lo - 1u, 1u, 0x1u);
}

/* prec: feature bit 0 with a wrong expect_sha256. */
static int rf_b_feature_and_expect(rf_ctx *c)
{
  uint32_t lo = 0u;
  return rf_span_lo(&lo) && rf_cart1(c, lo, 1u, 0x1u) && rf_expect_bad(c);
}

/* prec: a length bound broken (8193 bytes) and a wide first word. */
static int rf_b_8193_wide(rf_ctx *c)
{
  if (!rf_b_exec_8193(c)) {
    return 0;
  }
  c->buf[0] |= 0x04u;
  return 1;
}

/* prec: an input inside the instance whose length (8191) is also short. */
static int rf_b_inside_8191(rf_ctx *c)
{
  c->bytes = c->inst + rf_inside_off();
  c->len = RF_EXEC_BYTES - 1u;
  return 1;
}

/* prec: a short EXEC (8191) with a wrong expect_sha256. */
static int rf_b_8191_expect(rf_ctx *c)
{
  return rf_b_exec_8191(c) && rf_expect_bad(c);
}

/* ---- builders: run phase and reset (plan 03-12) ---- */

/* A second power-on reset (a power cycle), then a valid input: the machine stays POWERED. */
static int rf_b_power_cycle_valid(rf_ctx *c)
{
  if (ivs_machine_reset(c->m, IVS_RESET_POWER_ON) != IVS_OK) {
    ivs_test_diag("power cycle refused");
    return 0;
  }
  return rf_b_valid(c);
}

static int rf_b_power_cycle(rf_ctx *c)
{
  if (ivs_machine_reset(c->m, IVS_RESET_POWER_ON) != IVS_OK) {
    ivs_test_diag("power cycle refused");
    return 0;
  }
  return 1;
}

/* Loads one system image into the fresh prior before the snapshot. */
static int rf_b_with_grom(rf_ctx *c)
{
  return rf_load_one(c->m, IVS_KIND_GROM);
}

static int rf_b_with_exec(rf_ctx *c)
{
  return rf_load_one(c->m, IVS_KIND_EXEC);
}

/* ---- builders: out parameters ---- */

static int rf_b_info_inside(rf_ctx *c)
{
  ivs_info *o = (ivs_info *)(void *)(c->inst + rf_inside_off());
  o->struct_size = (uint32_t)sizeof *o;
  c->info_out = o;
  return 1;
}

static int rf_b_digest_inside(rf_ctx *c)
{
  c->digest_out = c->inst + rf_inside_off();
  return 1;
}

/* ---- rows ---- */

#define RF_LOAD(id, kind, priors, build, want, reason, seg, off, missing, flags)                                      \
  {id, RF_OP_LOAD, kind, priors, build, NULL, want, reason, seg, off, missing, flags}
#define RF_FIX(id, priors, fixture, want, reason, seg, off)                                                          \
  {id, RF_OP_LOAD, IVS_KIND_CARTRIDGE, priors, rf_b_fixture, fixture, want, reason, seg, off, 0u, 0u}
#define RF_PREC(id, kind, prior, build, want, reason, seg, off, missing)                                             \
  {id, RF_OP_LOAD, kind, prior, build, NULL, want, reason, seg, off, missing, RF_NO_SUFFIX}
#define RF_OTHER(id, op, kind, priors, build, want, flags) {id, op, kind, priors, build, NULL, want, 0u, 0u, 0u, 0u, flags}

#define RF_E IVS_KIND_EXEC
#define RF_G IVS_KIND_GROM
#define RF_K IVS_KIND_CARTRIDGE

static const rf_row rf_rows[] = {
    /* BAD_ARG, argument values (decided before occupancy): f, s, c. */
    RF_LOAD("machine.refuse.load.arg.kind-0", 0u, RF_P_FSCW, rf_b_valid, IVS_E_BAD_ARG, IVS_REASON_ARG_KIND, RF_NONE, 0u,
            0u, 0u),
    RF_LOAD("machine.refuse.load.arg.kind-4", 4u, RF_P_FSCW, rf_b_valid, IVS_E_BAD_ARG, IVS_REASON_ARG_KIND, RF_NONE, 0u,
            0u, 0u),
    RF_LOAD("machine.refuse.load.arg.kind-255", 255u, RF_P_FSCW, rf_b_valid, IVS_E_BAD_ARG, IVS_REASON_ARG_KIND, RF_NONE,
            0u, 0u, 0u),
    RF_LOAD("machine.refuse.load.exec.null-bytes-len-8", RF_E, RF_P_FSCW, rf_b_null_8, IVS_E_BAD_ARG,
            IVS_REASON_ARG_NULL_BYTES, RF_NONE, 0u, 0u, 0u),
    RF_LOAD("machine.refuse.load.grom.null-bytes-len-8", RF_G, RF_P_FSCW, rf_b_null_8, IVS_E_BAD_ARG,
            IVS_REASON_ARG_NULL_BYTES, RF_NONE, 0u, 0u, 0u),
    RF_LOAD("machine.refuse.load.cart.null-bytes-len-8", RF_K, RF_P_FSCW, rf_b_null_8, IVS_E_BAD_ARG,
            IVS_REASON_ARG_NULL_BYTES, RF_NONE, 0u, 0u, 0u),
    RF_LOAD("machine.refuse.load.exec.opts-short", RF_E, RF_P_FSCW, rf_b_opts_short, IVS_E_BAD_ARG,
            IVS_REASON_ARG_STRUCT_SIZE, RF_NONE, 0u, 0u, 0u),
    RF_LOAD("machine.refuse.load.cart.opts-size-0", RF_K, RF_P_FSCW, rf_b_opts_size0, IVS_E_BAD_ARG,
            IVS_REASON_ARG_STRUCT_SIZE, RF_NONE, 0u, 0u, 0u),
    RF_LOAD("machine.refuse.load.exec.report-short", RF_E, RF_P_FSCW, rf_b_report_short, IVS_E_BAD_ARG, 0u, 0u, 0u, 0u,
            RF_REPORT_KEPT),
    RF_LOAD("machine.refuse.load.exec.report-inside", RF_E, RF_P_FSCW, rf_b_report_inside, IVS_E_BAD_ARG, 0u, 0u, 0u, 0u,
            RF_REPORT_KEPT),
    RF_LOAD("machine.refuse.load.cart.report-inside", RF_K, RF_P_FSCW, rf_b_report_inside, IVS_E_BAD_ARG, 0u, 0u, 0u, 0u,
            RF_REPORT_KEPT),

    /* UNSUPPORTED, option flags and tails (decided before occupancy): f, s, c. */
    RF_LOAD("machine.refuse.load.exec.flags-bit-1", RF_E, RF_P_FSCW, rf_b_flags_bit1, IVS_E_UNSUPPORTED,
            IVS_REASON_OPTION_FLAGS, RF_NONE, 0u, 0u, 0u),
    RF_LOAD("machine.refuse.load.cart.flags-bit-31", RF_K, RF_P_FSCW, rf_b_flags_bit31, IVS_E_UNSUPPORTED,
            IVS_REASON_OPTION_FLAGS, RF_NONE, 0u, 0u, 0u),
    RF_LOAD("machine.refuse.load.exec.opts-tail-nonzero", RF_E, RF_P_FSCW, rf_b_opts_tail, IVS_E_UNSUPPORTED,
            IVS_REASON_OPTION_TAIL, RF_NONE, 0u, 0u, 0u),
    RF_LOAD("machine.refuse.load.grom.opts-tail-nonzero", RF_G, RF_P_FSCW, rf_b_opts_tail, IVS_E_UNSUPPORTED,
            IVS_REASON_OPTION_TAIL, RF_NONE, 0u, 0u, 0u),

    /* DUPLICATE_LOAD: only on an occupied slot. */
    RF_LOAD("machine.refuse.load.exec.duplicate-load", RF_E, RF_P_SC, rf_b_valid, IVS_E_DUPLICATE_LOAD,
            IVS_REASON_SLOT_OCCUPIED, RF_NONE, 0u, 0u, 0u),
    RF_LOAD("machine.refuse.load.grom.duplicate-load", RF_G, RF_P_SC, rf_b_valid, IVS_E_DUPLICATE_LOAD,
            IVS_REASON_SLOT_OCCUPIED, RF_NONE, 0u, 0u, 0u),
    RF_LOAD("machine.refuse.load.cart.duplicate-load", RF_K, RF_P_C, rf_b_valid, IVS_E_DUPLICATE_LOAD,
            IVS_REASON_SLOT_OCCUPIED, RF_NONE, 0u, 0u, 0u),

    /* OVERSIZED (after occupancy): f, x. Lying lengths in two placements. */
    RF_LOAD("machine.refuse.load.exec.oversized-by-one", RF_E, RF_P_FX, rf_b_exec_8193, IVS_E_OVERSIZED,
            IVS_REASON_IMAGE_LONG, RF_NONE, RF_EXEC_BYTES, 0u, 0u),
    RF_LOAD("machine.refuse.load.exec.oversized-far-heap", RF_E, RF_P_FX, rf_b_far_heap, IVS_E_OVERSIZED,
            IVS_REASON_IMAGE_LONG, RF_NONE, RF_EXEC_BYTES, 0u, 0u),
    RF_LOAD("machine.refuse.load.exec.oversized-far-inside", RF_E, RF_P_FX, rf_b_far_inside, IVS_E_OVERSIZED,
            IVS_REASON_IMAGE_LONG, RF_NONE, RF_EXEC_BYTES, 0u, 0u),
    RF_LOAD("machine.refuse.load.exec.len-wrap-heap", RF_E, RF_P_FX, rf_b_wrap_heap, IVS_E_OVERSIZED,
            IVS_REASON_IMAGE_LONG, RF_NONE, RF_EXEC_BYTES, 0u, RF_SKIP32),
    RF_LOAD("machine.refuse.load.exec.len-wrap-inside", RF_E, RF_P_FX, rf_b_wrap_inside, IVS_E_OVERSIZED,
            IVS_REASON_IMAGE_LONG, RF_NONE, RF_EXEC_BYTES, 0u, RF_SKIP32),
    RF_LOAD("machine.refuse.load.grom.oversized-by-one", RF_G, RF_P_FX, rf_b_grom_2049, IVS_E_OVERSIZED,
            IVS_REASON_IMAGE_LONG, RF_NONE, RF_GROM_BYTES, 0u, 0u),
    RF_LOAD("machine.refuse.load.grom.oversized-far-heap", RF_G, RF_P_FX, rf_b_far_heap, IVS_E_OVERSIZED,
            IVS_REASON_IMAGE_LONG, RF_NONE, RF_GROM_BYTES, 0u, 0u),
    RF_LOAD("machine.refuse.load.grom.oversized-far-inside", RF_G, RF_P_FX, rf_b_far_inside, IVS_E_OVERSIZED,
            IVS_REASON_IMAGE_LONG, RF_NONE, RF_GROM_BYTES, 0u, 0u),
    RF_LOAD("machine.refuse.load.cart.oversized-by-one-heap", RF_K, RF_P_FX, rf_b_cart_over_heap, IVS_E_OVERSIZED,
            IVS_REASON_TOO_LARGE, RF_NONE, IVS_IVSC_MAX_BYTES, 0u, 0u),
    RF_LOAD("machine.refuse.load.cart.oversized-by-one-inside", RF_K, RF_P_FX, rf_b_cart_over_inside, IVS_E_OVERSIZED,
            IVS_REASON_TOO_LARGE, RF_NONE, IVS_IVSC_MAX_BYTES, 0u, 0u),
    RF_FIX("machine.refuse.load.cart.total-over-max", RF_P_FX, "ivsc.B09-total-over-max", IVS_E_OVERSIZED,
           IVS_REASON_TOO_LARGE, RF_NONE, 16u),

    /* BAD_ARG, the bounded input span overlapping the instance (after occupancy): f, x. */
    RF_LOAD("machine.refuse.load.exec.input-inside", RF_E, RF_P_FX, rf_b_input_inside, IVS_E_BAD_ARG,
            IVS_REASON_ARG_INPUT_OVERLAP, RF_NONE, 0u, 0u, 0u),
    RF_LOAD("machine.refuse.load.grom.input-inside", RF_G, RF_P_FX, rf_b_input_inside, IVS_E_BAD_ARG,
            IVS_REASON_ARG_INPUT_OVERLAP, RF_NONE, 0u, 0u, 0u),
    RF_LOAD("machine.refuse.load.cart.input-inside", RF_K, RF_P_FX, rf_b_input_inside, IVS_E_BAD_ARG,
            IVS_REASON_ARG_INPUT_OVERLAP, RF_NONE, 0u, 0u, 0u),
    RF_LOAD("machine.refuse.load.exec.input-straddle-below", RF_E, RF_P_FX, rf_b_input_straddle_below, IVS_E_BAD_ARG,
            IVS_REASON_ARG_INPUT_OVERLAP, RF_NONE, 0u, 0u, RF_LEAD),
    RF_LOAD("machine.refuse.load.exec.input-adjacent-below-ok", RF_E, RF_P_FX, rf_b_input_adjacent_below, IVS_OK,
            IVS_REASON_NONE, RF_NONE, 0u, 0u, RF_LEAD | RF_EXPECT_OK),

    /* MALFORMED (after occupancy): f, x. */
    RF_LOAD("machine.refuse.load.exec.zero-length-ptr", RF_E, RF_P_FX, rf_b_zero_ptr, IVS_E_MALFORMED,
            IVS_REASON_IMAGE_SHORT, RF_NONE, 0u, 0u, 0u),
    RF_LOAD("machine.refuse.load.exec.zero-length-null", RF_E, RF_P_FX, rf_b_zero_null, IVS_E_MALFORMED,
            IVS_REASON_IMAGE_SHORT, RF_NONE, 0u, 0u, 0u),
    RF_LOAD("machine.refuse.load.grom.zero-length-ptr", RF_G, RF_P_FX, rf_b_zero_ptr, IVS_E_MALFORMED,
            IVS_REASON_IMAGE_SHORT, RF_NONE, 0u, 0u, 0u),
    RF_LOAD("machine.refuse.load.grom.zero-length-null", RF_G, RF_P_FX, rf_b_zero_null, IVS_E_MALFORMED,
            IVS_REASON_IMAGE_SHORT, RF_NONE, 0u, 0u, 0u),
    RF_LOAD("machine.refuse.load.cart.zero-length-ptr", RF_K, RF_P_FX, rf_b_zero_ptr, IVS_E_MALFORMED,
            IVS_REASON_TRUNCATED, RF_NONE, 0u, 0u, 0u),
    RF_LOAD("machine.refuse.load.cart.zero-length-null", RF_K, RF_P_FX, rf_b_zero_null, IVS_E_MALFORMED,
            IVS_REASON_TRUNCATED, RF_NONE, 0u, 0u, 0u),
    RF_LOAD("machine.refuse.load.exec.short-by-one", RF_E, RF_P_FX, rf_b_exec_8191, IVS_E_MALFORMED,
            IVS_REASON_IMAGE_SHORT, RF_NONE, RF_EXEC_BYTES - 1u, 0u, 0u),
    RF_LOAD("machine.refuse.load.exec.odd-length-4097", RF_E, RF_P_FX, rf_b_exec_4097, IVS_E_MALFORMED,
            IVS_REASON_IMAGE_SHORT, RF_NONE, 4097u, 0u, 0u),
    RF_LOAD("machine.refuse.load.exec.odd-length-1", RF_E, RF_P_FX, rf_b_exec_1, IVS_E_MALFORMED, IVS_REASON_IMAGE_SHORT,
            RF_NONE, 1u, 0u, 0u),
    RF_LOAD("machine.refuse.load.grom.short-by-one", RF_G, RF_P_FX, rf_b_grom_2047, IVS_E_MALFORMED,
            IVS_REASON_IMAGE_SHORT, RF_NONE, RF_GROM_BYTES - 1u, 0u, 0u),
    RF_LOAD("machine.refuse.load.exec.wide-word-first", RF_E, RF_P_FX, rf_b_wide_first, IVS_E_MALFORMED,
            IVS_REASON_IMAGE_WORD, RF_NONE, 0u, 0u, 0u),
    RF_LOAD("machine.refuse.load.exec.wide-word-last", RF_E, RF_P_FX, rf_b_wide_last, IVS_E_MALFORMED,
            IVS_REASON_IMAGE_WORD, RF_NONE, RF_EXEC_BYTES - 2u, 0u, 0u),
    /* Every IVSC v1 parse class through the load path (reasons and offsets of the records). */
    RF_FIX("machine.refuse.load.cart.parse-truncated", RF_P_FX, "ivsc.N02-len-65", IVS_E_MALFORMED,
           IVS_REASON_TRUNCATED, RF_NONE, 65u),
    RF_FIX("machine.refuse.load.cart.parse-magic", RF_P_FX, "ivsc.N03-magic-bit", IVS_E_MALFORMED, IVS_REASON_MAGIC,
           RF_NONE, 1u),
    RF_FIX("machine.refuse.load.cart.parse-version", RF_P_FX, "ivsc.N05-version-0", IVS_E_MALFORMED,
           IVS_REASON_VERSION, RF_NONE, 8u),
    RF_FIX("machine.refuse.load.cart.parse-flags", RF_P_FX, "ivsc.N07-flags", IVS_E_MALFORMED, IVS_REASON_FLAGS, RF_NONE,
           10u),
    RF_FIX("machine.refuse.load.cart.parse-reserved", RF_P_FX, "ivsc.N08-prefix-reserved", IVS_E_MALFORMED,
           IVS_REASON_RESERVED, RF_NONE, 22u),
    RF_FIX("machine.refuse.load.cart.parse-trailing", RF_P_FX, "ivsc.N09-trailing-byte", IVS_E_MALFORMED,
           IVS_REASON_TRAILING, RF_NONE, 66u),
    RF_FIX("machine.refuse.load.cart.parse-segment-count", RF_P_FX, "ivsc.N12-count-0", IVS_E_MALFORMED,
           IVS_REASON_SEGMENT_COUNT, RF_NONE, 20u),
    RF_FIX("machine.refuse.load.cart.parse-size-mismatch", RF_P_FX, "ivsc.N11-table-size-mismatch", IVS_E_MALFORMED,
           IVS_REASON_SIZE_MISMATCH, RF_NONE, 16u),
    RF_FIX("machine.refuse.load.cart.parse-segment-length", RF_P_FX, "ivsc.N13-segment-length-0", IVS_E_MALFORMED,
           IVS_REASON_SEGMENT_LENGTH, 0u, 26u),
    RF_FIX("machine.refuse.load.cart.parse-segment-width", RF_P_FX, "ivsc.N14-width-8", IVS_E_MALFORMED,
           IVS_REASON_SEGMENT_WIDTH, 0u, 28u),
    RF_FIX("machine.refuse.load.cart.parse-segment-attr", RF_P_FX, "ivsc.N17-attr-1", IVS_E_MALFORMED,
           IVS_REASON_SEGMENT_ATTR, 0u, 29u),
    RF_FIX("machine.refuse.load.cart.parse-segment-reserved", RF_P_FX, "ivsc.N19-segment-reserved", IVS_E_MALFORMED,
           IVS_REASON_SEGMENT_RESERVED, 0u, 30u),
    RF_FIX("machine.refuse.load.cart.parse-order", RF_P_FX, "ivsc.N23-descending", IVS_E_MALFORMED, IVS_REASON_ORDER,
           1u, 32u),
    RF_FIX("machine.refuse.load.cart.parse-word-range", RF_P_FX, "ivsc.N24-word-400-first", IVS_E_MALFORMED,
           IVS_REASON_WORD_RANGE, 0u, 40u),

    /* OUT_OF_RANGE: f, x. */
    RF_FIX("machine.refuse.load.cart.addr-wrap", RF_P_FX, "ivsc.B06-addr-wrap", IVS_E_OUT_OF_RANGE,
           IVS_REASON_ADDR_WRAP, 0u, 24u),
    RF_LOAD("machine.refuse.load.cart.span-below-one", RF_K, RF_P_FX, rf_b_span_below_one, IVS_E_OUT_OF_RANGE,
            IVS_REASON_OUT_OF_SPAN, 0u, 24u, 0u, 0u),
    RF_LOAD("machine.refuse.load.cart.span-straddle-start", RF_K, RF_P_FX, rf_b_span_straddle, IVS_E_OUT_OF_RANGE,
            IVS_REASON_OUT_OF_SPAN, 0u, 24u, 0u, 0u),
    RF_LOAD("machine.refuse.load.cart.exec-region", RF_K, RF_P_FX, rf_b_cart_exec_region, IVS_E_OUT_OF_RANGE,
            IVS_REASON_OUT_OF_SPAN, 0u, 24u, 0u, 0u),

    /* OVERLAP: f, x; adjacent segments are the positive control. */
    RF_FIX("machine.refuse.load.cart.overlap-exact", RF_P_FX, "ivsc.N21-duplicate-segment", IVS_E_OVERLAP,
           IVS_REASON_OVERLAP, 1u, 32u),
    RF_FIX("machine.refuse.load.cart.overlap-partial", RF_P_FX, "ivsc.N20-overlap-one-word", IVS_E_OVERLAP,
           IVS_REASON_OVERLAP, 1u, 32u),
    {"machine.refuse.load.cart.overlap-adjacent-ok", RF_OP_LOAD, RF_K, RF_P_FX, rf_b_fixture, "ivsc.P04-adjacent", IVS_OK,
     IVS_REASON_NONE, RF_NONE, 0u, 0u, RF_EXPECT_OK},

    /* DIGEST_MISMATCH: f, x. */
    RF_FIX("machine.refuse.load.cart.trailer-bit-flip", RF_P_FX, "ivsc.N28-trailer-bit-flip", IVS_E_DIGEST_MISMATCH,
           IVS_REASON_DIGEST, RF_NONE, 50u),
    RF_LOAD("machine.refuse.load.exec.expect-mismatch", RF_E, RF_P_FX, rf_b_expect_mismatch, IVS_E_DIGEST_MISMATCH,
            IVS_REASON_EXPECT_DIGEST, RF_NONE, 0u, 0u, 0u),
    RF_LOAD("machine.refuse.load.grom.expect-mismatch", RF_G, RF_P_FX, rf_b_expect_mismatch, IVS_E_DIGEST_MISMATCH,
            IVS_REASON_EXPECT_DIGEST, RF_NONE, 0u, 0u, 0u),
    RF_LOAD("machine.refuse.load.cart.expect-mismatch", RF_K, RF_P_FX, rf_b_expect_mismatch, IVS_E_DIGEST_MISMATCH,
            IVS_REASON_EXPECT_DIGEST, RF_NONE, 0u, 0u, 0u),

    /* UNSUPPORTED, layer 2 required_features: f, x. */
    RF_LOAD("machine.refuse.load.cart.feature-bit-0", RF_K, RF_P_FX, rf_b_feature_0, IVS_E_UNSUPPORTED,
            IVS_REASON_FEATURES, RF_NONE, IVS_IVSC_FEATURES_OFFSET, 0x1u, 0u),
    RF_LOAD("machine.refuse.load.cart.feature-bit-31", RF_K, RF_P_FX, rf_b_feature_31, IVS_E_UNSUPPORTED,
            IVS_REASON_FEATURES, RF_NONE, IVS_IVSC_FEATURES_OFFSET, 0x80000000u, 0u),

    /* BAD_HANDLE: no prior. */
    RF_LOAD("machine.refuse.load.exec.null-machine", RF_E, RF_P_F, rf_b_null_machine, IVS_E_BAD_HANDLE,
            IVS_REASON_HANDLE, RF_NONE, 0u, 0u, RF_NO_SUFFIX),
    RF_LOAD("machine.refuse.load.cart.null-machine", RF_K, RF_P_F, rf_b_null_machine, IVS_E_BAD_HANDLE,
            IVS_REASON_HANDLE, RF_NONE, 0u, 0u, RF_NO_SUFFIX),
    RF_LOAD("machine.refuse.load.exec.destroyed", RF_E, RF_P_F, rf_b_valid, IVS_E_BAD_HANDLE, IVS_REASON_HANDLE,
            RF_NONE, 0u, 0u, RF_NO_SUFFIX | RF_DESTROYED),
    RF_OTHER("machine.refuse.unload.exec.null-machine", RF_OP_UNLOAD, RF_E, RF_P_F, rf_b_null_machine,
             IVS_E_BAD_HANDLE, RF_NO_SUFFIX),
    RF_OTHER("machine.refuse.unload.exec.destroyed", RF_OP_UNLOAD, RF_E, RF_P_F, rf_b_none, IVS_E_BAD_HANDLE,
             RF_NO_SUFFIX | RF_DESTROYED),
    /* Unload BAD_ARG (unknown enumerant, A-08 e): f, s, c. */
    RF_OTHER("machine.refuse.unload.arg.kind-0", RF_OP_UNLOAD, 0u, RF_P_FSCW, rf_b_none, IVS_E_BAD_ARG, 0u),
    RF_OTHER("machine.refuse.unload.arg.kind-4", RF_OP_UNLOAD, 4u, RF_P_FSCW, rf_b_none, IVS_E_BAD_ARG, 0u),

    /* BAD_STATE: every load kind and unload while POWERED (D-13), w only; after a power cycle the
     * machine is still POWERED (A-08 d). The EXEC and GROM slots are occupied in w, so these rows
     * also show BAD_STATE winning over DUPLICATE_LOAD; the cartridge slot is empty. */
    RF_LOAD("machine.refuse.load.exec.powered", RF_E, RF_P_W, rf_b_valid, IVS_E_BAD_STATE, IVS_REASON_STATE_POWERED,
            RF_NONE, 0u, 0u, 0u),
    RF_LOAD("machine.refuse.load.grom.powered", RF_G, RF_P_W, rf_b_valid, IVS_E_BAD_STATE, IVS_REASON_STATE_POWERED,
            RF_NONE, 0u, 0u, 0u),
    RF_LOAD("machine.refuse.load.cart.powered", RF_K, RF_P_W, rf_b_valid, IVS_E_BAD_STATE, IVS_REASON_STATE_POWERED,
            RF_NONE, 0u, 0u, 0u),
    RF_LOAD("machine.refuse.load.exec.after-power-cycle", RF_E, RF_P_W, rf_b_power_cycle_valid, IVS_E_BAD_STATE,
            IVS_REASON_STATE_POWERED, RF_NONE, 0u, 0u, 0u),
    RF_OTHER("machine.refuse.unload.exec.powered", RF_OP_UNLOAD, RF_E, RF_P_W, rf_b_none, IVS_E_BAD_STATE, 0u),
    RF_OTHER("machine.refuse.unload.grom.powered", RF_OP_UNLOAD, RF_G, RF_P_W, rf_b_none, IVS_E_BAD_STATE, 0u),
    RF_OTHER("machine.refuse.unload.cart.powered", RF_OP_UNLOAD, RF_K, RF_P_W, rf_b_none, IVS_E_BAD_STATE, 0u),
    RF_OTHER("machine.refuse.unload.grom.after-power-cycle", RF_OP_UNLOAD, RF_G, RF_P_W, rf_b_power_cycle,
             IVS_E_BAD_STATE, 0u),

    /* Reset (D-14, A-08 e): kind is the reset kind argument. MISSING_SYSTEM_IMAGE on a fresh
     * instance with GROM only, EXEC only and neither; BAD_ARG for kinds 0 and 2 on an instance that
     * a valid reset would power (s) and on a POWERED one (w); BAD_HANDLE. */
    RF_OTHER("machine.refuse.reset.no-exec", RF_OP_RESET, IVS_RESET_POWER_ON, RF_P_F, rf_b_with_grom,
             IVS_E_MISSING_SYSTEM_IMAGE, RF_NO_SUFFIX),
    RF_OTHER("machine.refuse.reset.no-grom", RF_OP_RESET, IVS_RESET_POWER_ON, RF_P_F, rf_b_with_exec,
             IVS_E_MISSING_SYSTEM_IMAGE, RF_NO_SUFFIX),
    RF_OTHER("machine.refuse.reset.neither", RF_OP_RESET, IVS_RESET_POWER_ON, RF_P_F, rf_b_none,
             IVS_E_MISSING_SYSTEM_IMAGE, RF_NO_SUFFIX),
    RF_OTHER("machine.refuse.reset.no-exec-with-cart", RF_OP_RESET, IVS_RESET_POWER_ON, RF_P_X, rf_b_none,
             IVS_E_MISSING_SYSTEM_IMAGE, RF_NO_SUFFIX),
    RF_OTHER("machine.refuse.reset.kind-0", RF_OP_RESET, 0u, RF_P_S, rf_b_none, IVS_E_BAD_ARG, RF_NO_SUFFIX),
    RF_OTHER("machine.refuse.reset.kind-2", RF_OP_RESET, 2u, RF_P_S, rf_b_none, IVS_E_BAD_ARG, RF_NO_SUFFIX),
    RF_OTHER("machine.refuse.reset.kind-2-powered", RF_OP_RESET, 2u, RF_P_W, rf_b_none, IVS_E_BAD_ARG, RF_NO_SUFFIX),
    RF_OTHER("machine.refuse.reset.null", RF_OP_RESET, IVS_RESET_POWER_ON, RF_P_S, rf_b_null_machine_only,
             IVS_E_BAD_HANDLE, RF_NO_SUFFIX),
    RF_OTHER("machine.refuse.reset.destroyed", RF_OP_RESET, IVS_RESET_POWER_ON, RF_P_S, rf_b_none, IVS_E_BAD_HANDLE,
             RF_NO_SUFFIX | RF_DESTROYED),

    /* Out parameters aimed into the instance (plan 03-11 addition). */
    RF_OTHER("machine.refuse.info.out-inside", RF_OP_INFO, RF_E, RF_P_F, rf_b_info_inside, IVS_E_BAD_ARG, RF_NO_SUFFIX),
    RF_OTHER("machine.refuse.digest.out-inside", RF_OP_DIGEST, RF_E, RF_P_F, rf_b_digest_inside, IVS_E_BAD_ARG,
             RF_NO_SUFFIX),
    RF_OTHER("machine.refuse.create.out-inside-storage", RF_OP_CREATE, RF_E, RF_P_F, rf_b_none, IVS_E_BAD_ARG,
             RF_NO_SUFFIX),
    RF_OTHER("machine.refuse.create.out-after-instance-ok", RF_OP_CREATE, RF_E, RF_P_F, rf_b_none, IVS_OK,
             RF_NO_SUFFIX | RF_EXPECT_OK),

    /* Precedence: each adjacent pair of the load ladder (D-11, A-08 a, b). */
    RF_PREC("machine.prec.arg-over-handle", 0u, RF_P_F, rf_b_null_machine, IVS_E_BAD_ARG, IVS_REASON_ARG_KIND, RF_NONE,
            0u, 0u),
    RF_PREC("machine.prec.handle-over-unsupported", RF_E, RF_P_F, rf_b_null_flags_bit1, IVS_E_BAD_HANDLE,
            IVS_REASON_HANDLE, RF_NONE, 0u, 0u),
    RF_PREC("machine.prec.unsupported-over-duplicate", RF_E, RF_P_S, rf_b_flags_bit1, IVS_E_UNSUPPORTED,
            IVS_REASON_OPTION_FLAGS, RF_NONE, 0u, 0u),
    RF_PREC("machine.prec.duplicate-over-oversized", RF_E, RF_P_S, rf_b_exec_8193, IVS_E_DUPLICATE_LOAD,
            IVS_REASON_SLOT_OCCUPIED, RF_NONE, 0u, 0u),
    RF_PREC("machine.prec.duplicate-over-overlap", RF_E, RF_P_S, rf_b_input_inside, IVS_E_DUPLICATE_LOAD,
            IVS_REASON_SLOT_OCCUPIED, RF_NONE, 0u, 0u),
    RF_PREC("machine.prec.duplicate-over-malformed", RF_E, RF_P_S, rf_b_zero_ptr, IVS_E_DUPLICATE_LOAD,
            IVS_REASON_SLOT_OCCUPIED, RF_NONE, 0u, 0u),
    {"machine.prec.duplicate-over-digest", RF_OP_LOAD, RF_K, RF_P_C, rf_b_fixture, "ivsc.N28-trailer-bit-flip",
     IVS_E_DUPLICATE_LOAD, IVS_REASON_SLOT_OCCUPIED, RF_NONE, 0u, 0u, RF_NO_SUFFIX},
    RF_PREC("machine.prec.oversized-over-overlap", RF_E, RF_P_F, rf_b_far_inside, IVS_E_OVERSIZED, IVS_REASON_IMAGE_LONG,
            RF_NONE, RF_EXEC_BYTES, 0u),
    RF_PREC("machine.prec.overlap-over-malformed", RF_E, RF_P_F, rf_b_inside_8191, IVS_E_BAD_ARG,
            IVS_REASON_ARG_INPUT_OVERLAP, RF_NONE, 0u, 0u),
    RF_PREC("machine.prec.oversized-over-malformed", RF_E, RF_P_F, rf_b_8193_wide, IVS_E_OVERSIZED,
            IVS_REASON_IMAGE_LONG, RF_NONE, RF_EXEC_BYTES, 0u),
    RF_PREC("machine.prec.malformed-over-out-of-range", RF_K, RF_P_F, rf_b_width_and_wrap, IVS_E_MALFORMED,
            IVS_REASON_SEGMENT_WIDTH, 0u, 28u, 0u),
    RF_PREC("machine.prec.wrap-over-overlap", RF_K, RF_P_F, rf_b_wrap_and_overlap, IVS_E_OUT_OF_RANGE,
            IVS_REASON_ADDR_WRAP, 0u, 24u, 0u),
    {"machine.prec.overlap-over-digest", RF_OP_LOAD, RF_K, RF_P_F, rf_b_fixture_raw, "ivsc.N20-overlap-one-word",
     IVS_E_OVERLAP, IVS_REASON_OVERLAP, 1u, 32u, 0u, RF_NO_SUFFIX},
    RF_PREC("machine.prec.malformed-over-expect-digest", RF_E, RF_P_F, rf_b_8191_expect, IVS_E_MALFORMED,
            IVS_REASON_IMAGE_SHORT, RF_NONE, RF_EXEC_BYTES - 1u, 0u),
    RF_PREC("machine.prec.cart-overlap-before-span", RF_K, RF_P_F, rf_b_overlap_below_span, IVS_E_OVERLAP,
            IVS_REASON_OVERLAP, 1u, 32u, 0u),
    RF_PREC("machine.prec.cart-trailer-before-span", RF_K, RF_P_F, rf_b_trailer_below_span, IVS_E_DIGEST_MISMATCH,
            IVS_REASON_DIGEST, RF_NONE, 34u, 0u),
    RF_PREC("machine.prec.cart-malformed-before-span", RF_K, RF_P_F, rf_b_word_below_span, IVS_E_MALFORMED,
            IVS_REASON_WORD_RANGE, 0u, 32u, 0u),
    RF_PREC("machine.prec.cart-malformed-before-feature", RF_K, RF_P_F, rf_b_word_and_feature, IVS_E_MALFORMED,
            IVS_REASON_WORD_RANGE, 0u, 32u, 0u),
    RF_PREC("machine.prec.cart-span-before-feature", RF_K, RF_P_F, rf_b_span_and_feature, IVS_E_OUT_OF_RANGE,
            IVS_REASON_OUT_OF_SPAN, 0u, 24u, 0u),
    RF_PREC("machine.prec.cart-feature-before-expect", RF_K, RF_P_F, rf_b_feature_and_expect, IVS_E_UNSUPPORTED,
            IVS_REASON_FEATURES, RF_NONE, IVS_IVSC_FEATURES_OFFSET, 0x1u),
    /* The run phase in the load ladder (plan 03-12): UNSUPPORTED options win over BAD_STATE, and
     * BAD_STATE wins over DUPLICATE_LOAD (the EXEC slot is occupied in w). */
    RF_PREC("machine.prec.unsupported-over-bad-state", RF_E, RF_P_W, rf_b_flags_bit1, IVS_E_UNSUPPORTED,
            IVS_REASON_OPTION_FLAGS, RF_NONE, 0u, 0u),
    RF_PREC("machine.prec.bad-state-over-duplicate", RF_E, RF_P_W, rf_b_valid, IVS_E_BAD_STATE,
            IVS_REASON_STATE_POWERED, RF_NONE, 0u, 0u),
    /* The reset ladder: BAD_ARG, then BAD_HANDLE, then MISSING_SYSTEM_IMAGE. */
    RF_OTHER("machine.prec.reset-arg-over-handle", RF_OP_RESET, 2u, RF_P_F, rf_b_null_machine_only, IVS_E_BAD_ARG,
             RF_NO_SUFFIX),
    RF_OTHER("machine.prec.reset-arg-over-missing", RF_OP_RESET, 0u, RF_P_F, rf_b_none, IVS_E_BAD_ARG, RF_NO_SUFFIX),
    RF_OTHER("machine.prec.reset-handle-over-missing", RF_OP_RESET, IVS_RESET_POWER_ON, RF_P_F, rf_b_none,
             IVS_E_BAD_HANDLE, RF_NO_SUFFIX | RF_DESTROYED),
};
#define RF_ROWS (sizeof rf_rows / sizeof rf_rows[0])

/* ---- the (row, prior) index ---- */

#define RF_MAX_CASES 320u
#define RF_ID_BYTES 96u

typedef struct rf_case {
  uint16_t row;
  uint16_t prior; /* one RF_P_* bit */
} rf_case;

static rf_case rf_cases[RF_MAX_CASES];
static char rf_ids[RF_MAX_CASES][RF_ID_BYTES];
static size_t rf_n_cases;
static int rf_ready;

static char rf_prior_letter(uint32_t prior)
{
  switch (prior) {
  case RF_P_F:
    return 'f';
  case RF_P_X:
    return 'x';
  case RF_P_S:
    return 's';
  case RF_P_W:
    return 'w';
  default:
    return 'c';
  }
}

/* Expands every row into one case per prior (in the order f, x, s, c, w). An id that would not fit
 * is left empty and a row past RF_MAX_CASES is dropped; both make the harness or the inventory
 * fail, never pass silently. */
static void rf_index(void)
{
  static const uint32_t order[5] = {RF_P_F, RF_P_X, RF_P_S, RF_P_C, RF_P_W};
  size_t r;
  uint32_t k;
  if (rf_ready) {
    return;
  }
  rf_ready = 1;
  for (r = 0u; r < RF_ROWS; r++) {
    for (k = 0u; k < 5u; k++) {
      size_t n = strlen(rf_rows[r].id);
      char *id;
      if ((rf_rows[r].priors & order[k]) == 0u || rf_n_cases >= RF_MAX_CASES) {
        continue;
      }
      id = rf_ids[rf_n_cases];
      if (n + 3u <= RF_ID_BYTES) {
        memcpy(id, rf_rows[r].id, n);
        if ((rf_rows[r].flags & RF_NO_SUFFIX) == 0u) {
          id[n++] = '.';
          id[n++] = rf_prior_letter(order[k]);
        }
        id[n] = '\0';
      }
      rf_cases[rf_n_cases].row = (uint16_t)r;
      rf_cases[rf_n_cases].prior = (uint16_t)order[k];
      rf_n_cases++;
    }
  }
}

size_t ivs_machine_refuse_count(void)
{
  rf_index();
  return rf_n_cases;
}

const char *ivs_machine_refuse_id(size_t i)
{
  rf_index();
  return i < rf_n_cases ? rf_ids[i] : "";
}

const char *ivs_machine_refuse_skip(size_t i)
{
  rf_index();
  if (i < rf_n_cases && (rf_rows[rf_cases[i].row].flags & RF_SKIP32) != 0u && RF_SIZE_32) {
    return "size_t is 32 bits: no length above 2^32";
  }
  return NULL;
}

/* ---- the runner ---- */

static ivs_load_report rf_poisoned_report(void)
{
  ivs_load_report r;
  memset(&r, 0xA5, sizeof r);
  r.struct_size = (uint32_t)sizeof r;
  return r;
}

static ivs_status rf_call(rf_ctx *c, const rf_row *row)
{
  switch (row->op) {
  case RF_OP_UNLOAD:
    return ivs_machine_unload(c->handle, row->kind);
  case RF_OP_INFO:
    return ivs_machine_info(c->handle, c->info_out);
  case RF_OP_DIGEST:
    return ivs_machine_state_digest(c->handle, IVS_DIGEST_COMPOSITE, c->digest_out);
  case RF_OP_RESET:
    return ivs_machine_reset(c->handle, row->kind);
  default:
    return ivs_machine_load(c->handle, row->kind, c->bytes, c->len, c->opts, c->report);
  }
}

/* A positive control: OK, the slot reports the image, and the state digest moved. */
static int rf_check_ok(const rf_ctx *c, const rf_row *row, ivs_status st, const ivs_load_report *rep,
                       const uint8_t d0[32])
{
  uint8_t d1[32];
  if (st != IVS_OK) {
    ivs_test_diag("status %s, want IVS_OK (reason %u)", ivs_status_name(st), (unsigned)rep->reason);
    return 0;
  }
  if (rep->reason != IVS_REASON_NONE || rep->image.loaded != 1u || rep->image.kind != row->kind ||
      rep->image.length_bytes != (uint32_t)c->len) {
    ivs_test_diag("report: reason %u loaded %u kind %u length %u", (unsigned)rep->reason,
                  (unsigned)rep->image.loaded, (unsigned)rep->image.kind, (unsigned)rep->image.length_bytes);
    return 0;
  }
  if (ivs_machine_state_digest(c->m, IVS_DIGEST_COMPOSITE, d1) != IVS_OK || memcmp(d0, d1, 32u) == 0) {
    ivs_test_diag("the state digest did not move");
    return 0;
  }
  return 1;
}

static int rf_call_and_check(rf_ctx *c, const rf_row *row)
{
  static const uint8_t zero[sizeof(ivs_image_report)] = {0};
  ivs_load_report rep = rf_poisoned_report();
  ivs_load_report rep0;
  uint8_t d0[32];
  uint8_t d1[32];
  uint8_t *snap;
  ivs_status st;
  int destroyed = (row->flags & RF_DESTROYED) != 0u;
  int ok = 1;
  if (c->report_size != 0u) {
    rep.struct_size = c->report_size;
  }
  rep0 = rep;
  if (c->report == NULL) {
    c->report = &rep;
  }
  if (!destroyed && ivs_machine_state_digest(c->m, IVS_DIGEST_COMPOSITE, d0) != IVS_OK) {
    ivs_test_diag("digest before the call refused");
    return 0;
  }
  snap = malloc(c->alloc_bytes);
  if (snap == NULL) {
    return 0;
  }
  memcpy(snap, c->alloc, c->alloc_bytes);
  st = rf_call(c, row);
  if ((row->flags & RF_EXPECT_OK) != 0u) {
    free(snap);
    return rf_check_ok(c, row, st, &rep, d0);
  }
  if (st != row->want) {
    ivs_test_diag("status %s, want %s (reason %u)", ivs_status_name(st), ivs_status_name(row->want),
                  (unsigned)rep.reason);
    ok = 0;
  }
  if (row->op == RF_OP_LOAD && (row->flags & RF_REPORT_KEPT) != 0u) {
    if (memcmp(&rep, &rep0, sizeof rep) != 0) {
      ivs_test_diag("a refused report was written");
      ok = 0;
    }
  } else if (row->op == RF_OP_LOAD) {
    if (rep.struct_size != (uint32_t)sizeof rep || rep.reason != row->reason || rep.segment_index != row->seg ||
        rep.byte_offset != row->off || rep.missing_features != row->missing) {
      ivs_test_diag("detail: size %u reason %u seg %u off %llu missing 0x%X, want reason %u seg %u off %llu missing 0x%X",
                    (unsigned)rep.struct_size, (unsigned)rep.reason, (unsigned)rep.segment_index,
                    (unsigned long long)rep.byte_offset, (unsigned)rep.missing_features, (unsigned)row->reason,
                    (unsigned)row->seg, (unsigned long long)row->off, (unsigned)row->missing);
      ok = 0;
    }
    if (memcmp(&rep.image, zero, sizeof zero) != 0) {
      ivs_test_diag("a refused report carries an image");
      ok = 0;
    }
  }
  if (memcmp(snap, c->alloc, c->alloc_bytes) != 0) {
    ivs_test_diag("the refused call changed the host allocation");
    ok = 0;
  }
  if (!destroyed &&
      (ivs_machine_state_digest(c->m, IVS_DIGEST_COMPOSITE, d1) != IVS_OK || memcmp(d0, d1, sizeof d0) != 0)) {
    ivs_test_diag("the state digest changed");
    ok = 0;
  }
  free(snap);
  return ok;
}

/* create with its out pointer inside the storage's instance bytes (BAD_ARG, storage untouched)
 * or just past them in a larger storage (OK, *out is the storage). */
static int rf_run_create(const rf_row *row)
{
  ivs_create_options o;
  size_t bytes = IVS_MACHINE_BYTES + 16u;
  uint8_t *st = malloc(bytes);
  uint8_t *snap = malloc(bytes);
  ivs_machine **out;
  ivs_status got;
  int ok = 0;
  if (st == NULL || snap == NULL) {
    free(st);
    free(snap);
    return 0;
  }
  memset(st, 0xA5, bytes);
  memset(&o, 0, sizeof o);
  o.struct_size = (uint32_t)sizeof o;
  o.profile_id = RF_PROFILE;
  out = (ivs_machine **)(void *)(st + ((row->flags & RF_EXPECT_OK) != 0u ? IVS_MACHINE_BYTES : 64u));
  memcpy(snap, st, bytes);
  got = ivs_machine_create(&o, st, bytes, out);
  if ((row->flags & RF_EXPECT_OK) != 0u) {
    ok = got == IVS_OK && *out == (ivs_machine *)(void *)st;
    if (!ok) {
      ivs_test_diag("create with out past the instance: %s", ivs_status_name(got));
    }
  } else {
    ok = got == row->want && memcmp(snap, st, bytes) == 0;
    if (!ok) {
      ivs_test_diag("create with out inside the storage: %s, storage %s", ivs_status_name(got),
                    memcmp(snap, st, bytes) == 0 ? "intact" : "changed");
    }
  }
  free(snap);
  free(st);
  return ok;
}

int ivs_machine_refuse_run(size_t i)
{
  const rf_row *row;
  rf_ctx c;
  ivs_create_options o;
  size_t lead;
  int ok = 0;
  rf_index();
  if (i >= rf_n_cases) {
    return IVS_TEST_FAIL;
  }
  row = &rf_rows[rf_cases[i].row];
  if (row->op == RF_OP_CREATE) {
    return rf_run_create(row) ? IVS_TEST_PASS : IVS_TEST_FAIL;
  }
  memset(&c, 0, sizeof c);
  c.row = row;
  lead = (row->flags & RF_LEAD) != 0u ? RF_LEAD_BYTES : 0u;
  c.alloc_bytes = lead + IVS_MACHINE_BYTES;
  c.alloc = malloc(c.alloc_bytes);
  if (c.alloc == NULL) {
    return IVS_TEST_FAIL;
  }
  memset(c.alloc, 0x5A, c.alloc_bytes);
  c.inst = c.alloc + lead;
  memset(&o, 0, sizeof o);
  o.struct_size = (uint32_t)sizeof o;
  o.profile_id = RF_PROFILE;
  if (ivs_machine_create(&o, c.inst, IVS_MACHINE_BYTES, &c.m) != IVS_OK) {
    ivs_test_diag("create refused");
  } else if (!rf_prior(c.m, rf_cases[i].prior, row->kind)) {
    ivs_test_diag("prior %c could not be built", rf_prior_letter(rf_cases[i].prior));
  } else if ((row->flags & RF_DESTROYED) != 0u && ivs_machine_destroy(c.m) != IVS_OK) {
    ivs_test_diag("destroy refused");
  } else {
    c.handle = c.m;
    if (!row->build(&c)) {
      ivs_test_diag("input could not be built");
    } else {
      ok = rf_call_and_check(&c, row);
    }
  }
  free(c.buf);
  free(c.optbuf);
  free(c.alloc);
  return ok ? IVS_TEST_PASS : IVS_TEST_FAIL;
}

/* SPDX-License-Identifier: MIT */
/* Suite machine, families refuse and prec (D-08, D-10..D-13, D-26, D-32, D-33, A-06, A-08,
 * plan 03-11): the load refusal matrix. Every row is one call against a prior instance, with the
 * full oracle of D-12:
 *   - the status the row names (its own typed status, never a neighbour's);
 *   - the whole host allocation (instance storage, plus the lead bytes of a lead row) byte for
 *     byte as it was before the call (memcmp of a copy taken after the prior was built);
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
 *   c  EXEC, GROM and cartridge loaded.
 * Rows decided before slot occupancy (argument BAD_ARG, UNSUPPORTED) run on f, s and c with the
 * same status; rows decided after it (OVERSIZED, input-overlap BAD_ARG, content, DIGEST_MISMATCH)
 * run on f and x, where the slot under test is empty; DUPLICATE_LOAD rows run only on an occupied
 * slot (s and c, or c for the cartridge); handle rows carry no prior. The powered prior w joins
 * in plan 03-12.
 *
 * Every instance and input lives in a heap buffer of exactly its size (never on the stack), so
 * ASan sees any byte the core touches outside them. Inputs are generated stand-ins (D-24): EXEC
 * word i = (37 * i + 11) & 0x3FF high byte first, GROM byte i = (7 * i + 3) & 0xFF (the stand-ins
 * of family load), and IVSC containers from the fixture records of data/cart/ (the prior
 * cartridge is ivsc.P02-two-segments, sealed at authoring time with `cmake -E sha256sum`). No
 * binary file is read or committed. */
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

/* Priors (bit mask of a row). */
#define RF_P_F 0x1u
#define RF_P_X 0x2u
#define RF_P_S 0x4u
#define RF_P_C 0x8u
#define RF_P_FSC (RF_P_F | RF_P_S | RF_P_C)
#define RF_P_FX (RF_P_F | RF_P_X)

/* Operations. */
#define RF_OP_LOAD 1u
#define RF_OP_UNLOAD 2u

/* Row flags. */
#define RF_REPORT_KEPT 0x1u /* the report is refused: every report byte stays poisoned */
#define RF_NO_SUFFIX 0x2u   /* one prior, id without a prior suffix (prec and handle rows) */

typedef struct rf_ctx rf_ctx;
typedef int (*rf_build_fn)(rf_ctx *c);

typedef struct rf_row {
  const char *id; /* without the prior suffix */
  uint32_t op;
  uint32_t kind;   /* the kind argument of the call */
  uint32_t priors; /* RF_P_* mask */
  rf_build_fn build;
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
};

/* ---- stand-in images ---- */

static uint16_t rf_exec_word(uint32_t i)
{
  return (uint16_t)((37u * i + 11u) & 0x3FFu);
}

/* n bytes (at least 1 allocated) holding the EXEC stand-in pattern, zero past 8192. */
static uint8_t *rf_exec_bytes(size_t n)
{
  uint8_t *b = malloc(n != 0u ? n : 1u);
  size_t i;
  if (b == NULL) {
    return NULL;
  }
  memset(b, 0, n != 0u ? n : 1u);
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
  uint8_t *b = malloc(n != 0u ? n : 1u);
  size_t i;
  if (b == NULL) {
    return NULL;
  }
  memset(b, 0, n != 0u ? n : 1u);
  for (i = 0u; i < n; i++) {
    b[i] = (uint8_t)((7u * i + 3u) & 0xFFu);
  }
  return b;
}

/* The fixture record id of the generated table, or NULL. */
static const ivs_cart_fixture *rf_fixture(const char *id)
{
  size_t i;
  for (i = 0u; i < ivs_cart_count; i++) {
    if (strcmp(ivs_cart_table[i].id, id) == 0) {
      return &ivs_cart_table[i];
    }
  }
  return NULL;
}

/* A fixture record's literal bytes (positive or patch) in a fresh heap buffer of exactly their
 * size, resealed with the core SHA-256 when the record asks for it. */
static uint8_t *rf_fixture_bytes(const char *id, size_t *len)
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
  if (f->reseal != 0u && f->n_bytes >= IVS_IVSC_TRAILER_BYTES) {
    ivs_sha256_digest(b, f->n_bytes - IVS_IVSC_TRAILER_BYTES, b + f->n_bytes - IVS_IVSC_TRAILER_BYTES);
  }
  *len = f->n_bytes;
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
    b = rf_fixture_bytes("ivsc.P02-two-segments", &len);
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
    default:
      want = 0;
      break;
    }
    if (want && !rf_load_one(m, k)) {
      return 0;
    }
  }
  return 1;
}

/* ---- builders ---- */

/* An EXEC stand-in one byte too long (8193 bytes, the extra byte zero). */
static int rf_b_exec_8193(rf_ctx *c)
{
  c->buf = rf_exec_bytes(RF_EXEC_BYTES + 1u);
  c->bytes = c->buf;
  c->len = RF_EXEC_BYTES + 1u;
  return c->buf != NULL;
}

/* ---- rows ---- */

static const rf_row rf_rows[] = {
    /* OVERSIZED, decided after occupancy: f and x. */
    {"machine.refuse.load.exec.oversized-by-one", RF_OP_LOAD, IVS_KIND_EXEC, RF_P_FX, rf_b_exec_8193, IVS_E_OVERSIZED,
     IVS_REASON_IMAGE_LONG, IVS_SEGMENT_NONE, RF_EXEC_BYTES, 0u, 0u},
};
#define RF_ROWS (sizeof rf_rows / sizeof rf_rows[0])

/* ---- the (row, prior) index ---- */

#define RF_MAX_CASES 256u
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
  default:
    return 'c';
  }
}

/* Expands every row into one case per prior (in the order f, x, s, c). Ids that would not fit
 * are left empty, which the harness rejects as a duplicate or empty id. */
static void rf_index(void)
{
  static const uint32_t order[4] = {RF_P_F, RF_P_X, RF_P_S, RF_P_C};
  size_t r;
  uint32_t k;
  if (rf_ready) {
    return;
  }
  rf_ready = 1;
  for (r = 0u; r < RF_ROWS; r++) {
    for (k = 0u; k < 4u; k++) {
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
  (void)i;
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

static int rf_call_and_check(rf_ctx *c, const rf_row *row)
{
  static const uint8_t zero[sizeof(ivs_image_report)] = {0};
  ivs_load_report rep = rf_poisoned_report();
  ivs_load_report rep0 = rep;
  uint8_t d0[32];
  uint8_t d1[32];
  uint8_t *snap;
  ivs_status st;
  int ok = 1;
  if (c->report_size != 0u) {
    rep.struct_size = c->report_size;
    rep0 = rep;
  }
  if (c->report == NULL) {
    c->report = &rep;
  }
  snap = malloc(c->alloc_bytes);
  if (snap == NULL) {
    return 0;
  }
  memcpy(snap, c->alloc, c->alloc_bytes);
  if (ivs_machine_state_digest(c->m, IVS_DIGEST_COMPOSITE, d0) != IVS_OK) {
    ivs_test_diag("digest before the call refused");
    free(snap);
    return 0;
  }
  if (row->op == RF_OP_UNLOAD) {
    st = ivs_machine_unload(c->handle, row->kind);
  } else {
    st = ivs_machine_load(c->handle, row->kind, c->bytes, c->len, c->opts, c->report);
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
  if (ivs_machine_state_digest(c->m, IVS_DIGEST_COMPOSITE, d1) != IVS_OK || memcmp(d0, d1, sizeof d0) != 0) {
    ivs_test_diag("the state digest changed");
    ok = 0;
  }
  free(snap);
  return ok;
}

int ivs_machine_refuse_run(size_t i)
{
  const rf_row *row;
  rf_ctx c;
  ivs_create_options o;
  int ok = 0;
  rf_index();
  if (i >= rf_n_cases) {
    return IVS_TEST_FAIL;
  }
  row = &rf_rows[rf_cases[i].row];
  memset(&c, 0, sizeof c);
  c.row = row;
  c.alloc_bytes = IVS_MACHINE_BYTES;
  c.alloc = malloc(c.alloc_bytes);
  if (c.alloc == NULL) {
    return IVS_TEST_FAIL;
  }
  memset(c.alloc, 0x5A, c.alloc_bytes);
  c.inst = c.alloc;
  memset(&o, 0, sizeof o);
  o.struct_size = (uint32_t)sizeof o;
  o.profile_id = RF_PROFILE;
  if (ivs_machine_create(&o, c.inst, IVS_MACHINE_BYTES, &c.m) != IVS_OK) {
    ivs_test_diag("create refused");
  } else if (!rf_prior(c.m, rf_cases[i].prior, row->kind)) {
    ivs_test_diag("prior %c could not be built", rf_prior_letter(rf_cases[i].prior));
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

/* SPDX-License-Identifier: MIT */
/* Suite machine, family load (D-10, D-13, D-24..D-27, D-32, D-33, A-04, plan 03-10): typed loads
 * of EXEC, GROM and IVSC cartridges through the public header, their reports, the bus view of the
 * loaded words, unload back to the fresh state, layer 2 and the load options.
 *
 * Every image here is a generated stand-in (D-24), never a real system image:
 *   EXEC  word i = (37 * i + 11) & 0x3FF, high byte first, 8192 bytes;
 *   GROM  byte i = (7 * i + 3) & 0xFF, 2048 bytes.
 * Their literal digests were computed at authoring time with `cmake -E sha256sum` over scratch
 * files holding the same bytes (written by a throwaway script, not committed), so the core
 * SHA-256 meets an independent value. Instances and images live in exact-size heap buffers, so
 * ASan sees any byte the core touches outside them. machine/machine.h and memmap/memsys.h are
 * read only for the instance size and the bus view of the loaded words. */
#include <stdint.h>
#include <stdlib.h>
#include <string.h>

#include "ivs/ivs.h"
#include "ivs/ivs_machine.h"
#include "ivs_test.h"
#include "machine/machine.h"
#include "machine_cases.h"
#include "memmap/memsys.h"

#define IVS_CHECK(cond)                                                                                \
  do {                                                                                                 \
    if (!(cond)) {                                                                                     \
      ivs_test_diag("check failed at line %d: %s", __LINE__, #cond);                                   \
      return IVS_TEST_FAIL;                                                                            \
    }                                                                                                  \
  } while (0)

#define ML_PROFILE "intv-2609-ntsc-family-p1"
#define ML_EXEC_BYTES 8192u
#define ML_GROM_BYTES 2048u

/* `cmake -E sha256sum exec.bin` over the EXEC stand-in. */
static const uint8_t ml_exec_sha[32] = {0xe1u, 0xa0u, 0xeeu, 0xd9u, 0x6du, 0xa8u, 0x9au, 0xf0u, 0xf8u, 0x5au, 0xe8u,
                                        0x02u, 0x7bu, 0x46u, 0x16u, 0x69u, 0x32u, 0x0bu, 0x22u, 0xf9u, 0xa4u, 0xc1u,
                                        0xc7u, 0xffu, 0x20u, 0xe6u, 0x27u, 0xe6u, 0xd2u, 0xd2u, 0xeeu, 0xe2u};

/* One instance in exact-size heap storage. */
typedef struct ml_inst {
  uint8_t *storage;
  ivs_machine *m;
} ml_inst;

static int ml_new(ml_inst *x)
{
  ivs_create_options o;
  memset(&o, 0, sizeof o);
  o.struct_size = (uint32_t)sizeof o;
  o.profile_id = ML_PROFILE;
  x->m = NULL;
  x->storage = malloc(IVS_MACHINE_BYTES);
  if (x->storage == NULL) {
    return 0;
  }
  memset(x->storage, 0xA5, IVS_MACHINE_BYTES);
  if (ivs_machine_create(&o, x->storage, IVS_MACHINE_BYTES, &x->m) != IVS_OK) {
    free(x->storage);
    x->storage = NULL;
    return 0;
  }
  return 1;
}

static void ml_free(ml_inst *x)
{
  free(x->storage);
  x->storage = NULL;
  x->m = NULL;
}

static uint16_t ml_exec_word(uint32_t i)
{
  return (uint16_t)((37u * i + 11u) & 0x3FFu);
}

/* The EXEC stand-in in a fresh heap buffer of exactly 8192 bytes. */
static uint8_t *ml_exec_image(void)
{
  uint8_t *b = malloc(ML_EXEC_BYTES);
  uint32_t i;
  if (b == NULL) {
    return NULL;
  }
  for (i = 0u; i < ML_EXEC_BYTES / 2u; i++) {
    b[2u * i] = (uint8_t)(ml_exec_word(i) >> 8);
    b[2u * i + 1u] = (uint8_t)(ml_exec_word(i) & 0xFFu);
  }
  return b;
}

static ivs_load_report ml_report(void)
{
  ivs_load_report r;
  memset(&r, 0xA5, sizeof r);
  r.struct_size = (uint32_t)sizeof r;
  return r;
}

static uint16_t ml_bus(const ml_inst *x, uint16_t addr)
{
  return ivs_memsys_read(&((const struct ivs_machine *)(const void *)x->m)->mem, addr);
}

static int ml_same(const uint8_t *a, const uint8_t *b, size_t n)
{
  return memcmp(a, b, n) == 0;
}

/* The fixed fields of a successful report. */
static int ml_report_ok(const ivs_load_report *r, uint32_t kind, uint32_t len, uint32_t units, uint32_t bits,
                        uint32_t base, const uint8_t sha[32])
{
  const ivs_image_report *s = &r->image;
  IVS_CHECK(r->struct_size == (uint32_t)sizeof *r);
  IVS_CHECK(r->reason == IVS_REASON_NONE);
  IVS_CHECK(r->segment_index == IVS_SEGMENT_NONE);
  IVS_CHECK(r->missing_features == 0u);
  IVS_CHECK(r->byte_offset == 0u);
  IVS_CHECK(s->struct_size == (uint32_t)sizeof *s);
  IVS_CHECK(s->kind == kind);
  IVS_CHECK(s->loaded == 1u);
  IVS_CHECK(s->length_bytes == len);
  IVS_CHECK(s->length_units == units);
  IVS_CHECK(s->unit_bits == bits);
  IVS_CHECK(s->base_addr == base);
  IVS_CHECK(s->reserved == 0u);
  IVS_CHECK(ml_same(s->sha256, sha, 32u));
  return IVS_TEST_PASS;
}

/* ---- tracer: EXEC ---- */

/* A stand-in EXEC loads OK: the report carries its length, units, width, base and the literal
 * digest; info reports the same slot; the bus reads its words at $1000..$1FFF. */
static int ml_exec_ok(void)
{
  ml_inst x;
  ivs_load_report r = ml_report();
  ivs_info info;
  int rc = IVS_TEST_FAIL;
  uint8_t *img = ml_exec_image();
  IVS_CHECK(img != NULL);
  if (!ml_new(&x)) {
    free(img);
    return IVS_TEST_FAIL;
  }
  memset(&info, 0, sizeof info);
  info.struct_size = (uint32_t)sizeof info;
  if (ivs_machine_load(x.m, IVS_KIND_EXEC, img, ML_EXEC_BYTES, NULL, &r) != IVS_OK) {
    ivs_test_diag("load refused, reason %u", (unsigned)r.reason);
  } else if (ml_report_ok(&r, IVS_KIND_EXEC, 8192u, 4096u, 10u, 0x1000u, ml_exec_sha) != IVS_TEST_PASS) {
    ivs_test_diag("report wrong");
  } else if (ivs_machine_info(x.m, &info) != IVS_OK ||
             !ml_same((const uint8_t *)&info.slots[0], (const uint8_t *)&r.image, sizeof r.image)) {
    ivs_test_diag("info slot differs from the report");
  } else if (ml_bus(&x, 0x1000u) != ml_exec_word(0u) || ml_bus(&x, 0x1001u) != ml_exec_word(1u) ||
             ml_bus(&x, 0x1FFFu) != ml_exec_word(4095u)) {
    ivs_test_diag("bus reads %04x at $1000, want %04x", (unsigned)ml_bus(&x, 0x1000u), (unsigned)ml_exec_word(0u));
  } else if (info.slots[1].loaded != 0u || info.slots[2].loaded != 0u) {
    ivs_test_diag("another slot reports loaded");
  } else {
    rc = IVS_TEST_PASS;
  }
  ml_free(&x);
  free(img);
  return rc;
}

/* Load then unload: the composite digest and every instance byte equal a fresh instance's; a
 * second unload of the empty slot is OK. */
static int ml_exec_unload_fresh(void)
{
  ml_inst x;
  ml_inst fresh;
  uint8_t d0[32];
  uint8_t d1[32];
  uint8_t d2[32];
  int rc = IVS_TEST_FAIL;
  uint8_t *img = ml_exec_image();
  IVS_CHECK(img != NULL);
  if (!ml_new(&x)) {
    free(img);
    return IVS_TEST_FAIL;
  }
  if (!ml_new(&fresh)) {
    ml_free(&x);
    free(img);
    return IVS_TEST_FAIL;
  }
  if (ivs_machine_state_digest(x.m, IVS_DIGEST_COMPOSITE, d0) != IVS_OK ||
      ivs_machine_load(x.m, IVS_KIND_EXEC, img, ML_EXEC_BYTES, NULL, NULL) != IVS_OK ||
      ivs_machine_state_digest(x.m, IVS_DIGEST_COMPOSITE, d1) != IVS_OK) {
    ivs_test_diag("load or digest failed");
  } else if (ml_same(d0, d1, 32u)) {
    ivs_test_diag("the load did not change the digest");
  } else if (ivs_machine_unload(x.m, IVS_KIND_EXEC) != IVS_OK ||
             ivs_machine_state_digest(x.m, IVS_DIGEST_COMPOSITE, d2) != IVS_OK) {
    ivs_test_diag("unload or digest failed");
  } else if (!ml_same(d0, d2, 32u)) {
    ivs_test_diag("digest after unload differs from the fresh digest");
  } else if (!ml_same(x.storage, fresh.storage, IVS_MACHINE_BYTES)) {
    ivs_test_diag("instance bytes after unload differ from a fresh instance");
  } else if (ivs_machine_unload(x.m, IVS_KIND_EXEC) != IVS_OK) {
    ivs_test_diag("unload of an empty slot refused");
  } else if (ml_bus(&x, 0x1000u) != ml_bus(&fresh, 0x1000u)) {
    ivs_test_diag("bus still reads the image after unload");
  } else {
    rc = IVS_TEST_PASS;
  }
  ml_free(&fresh);
  ml_free(&x);
  free(img);
  return rc;
}

const ivs_test_case ivs_machine_load_cases[] = {
    {"machine.load.exec-ok", ml_exec_ok, NULL},
    {"machine.load.exec-unload-fresh", ml_exec_unload_fresh, NULL},
};
const size_t ivs_machine_load_count = sizeof ivs_machine_load_cases / sizeof ivs_machine_load_cases[0];

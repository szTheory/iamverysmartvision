/* SPDX-License-Identifier: MIT */
/* Vector table types filled by the cmake -P generator (cmake/IvsVectorTable.cmake) from the
 * JSON vectors under data/cp1610/vectors (D-14). The generator merges each vector's init
 * with its expect delta, so the harness compares full states. Nothing here is committed
 * generated output: ivs_vectors.inc lives in the build tree only. */
#ifndef IVS_VEC_H
#define IVS_VEC_H

#include <stddef.h>
#include <stdint.h>

#include "cpu/cp1610.h"
#include "ivs_test.h"

typedef struct ivs_vec_word {
  uint16_t addr;
  uint16_t word;
} ivs_vec_word;

typedef struct ivs_vec_bus {
  uint32_t cycle; /* counted from vector start across steps */
  uint16_t addr;
  uint16_t data;
  uint8_t kind;
} ivs_vec_bus;

typedef struct ivs_vec_wait {
  uint16_t addr;
  uint16_t wait_cpu_cycles;
} ivs_vec_wait;

typedef struct ivs_vec {
  const char *id;
  const char *row;
  const char *kind;   /* "source" or "policy" */
  const char *policy; /* POL-<AREA>-<NN> or NULL */
  const char *cite;
  const char *derivation;
  const char *skip; /* discrepancy id from cp1610-skips.ids, or NULL */
  uint32_t steps;
  uint8_t reset;
  uint8_t has_iab;
  uint16_t iab;
  uint8_t config_mask;       /* bit 0: interrupt_accept_clears_intff, 1: release latency, 2: wait limit */
  ivs_cp1610_config config; /* overrides of ivs_cp1610_config_default, per config_mask */
  ivs_cp1610 init;
  ivs_cp1610 expect;
  ivs_cp1610_lines lines;             /* init.lines, used for every step without step_lines */
  const ivs_cp1610_lines *step_lines; /* one per step, or NULL */
  size_t n_step_lines;
  const ivs_vec_word *mem; /* program words and init.mem, in file order */
  size_t n_mem;
  const ivs_vec_word *expect_mem; /* expect.mem overrides */
  size_t n_expect_mem;
  uint8_t cycles_asserted;
  uint32_t cycles;
  uint8_t has_bus;
  uint8_t bus_cycles_asserted; /* bus_basis figure: 1, inference: 0 */
  const ivs_vec_bus *bus;
  size_t n_bus;
  const ivs_vec_wait *waits;
  size_t n_waits;
  const uint8_t *ebci; /* codes the provider answers true for */
  size_t n_ebci;
  uint8_t has_ebci; /* 0: NULL provider (key absent); 1: provider present, even with no codes */
  const uint8_t *units;
  size_t n_units;
  uint8_t has_events;
  uint8_t events;
} ivs_vec;

#endif

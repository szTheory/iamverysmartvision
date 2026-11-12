/* SPDX-License-Identifier: MIT */
/* Suite psg (AUD-01, plan 05-04): the PSG register file, then (plans 05-05..05-08) the ports, tone,
 * mixer, noise, envelope, sample stream and write log. One executable; each family file defines its
 * case table here declared and psg_main.c concatenates them into one suite. Expected values come
 * from the sourced data files only: the vector files data/psg/vectors (one text file per family) and the register-address
 * literal data/psg/<profile>.regmap.literal.txt, turned into C includes by cmake/IvsPsgVectors.cmake
 * (which reads neither the profile nor the core), never from the generated parameter tables or from
 * a core output. */
#ifndef IVS_PSG_CASES_H
#define IVS_PSG_CASES_H

#include <stddef.h>
#include <stdint.h>

#include "ivs_test.h"

/* One vector (field order of the generator's initializers, cmake/IvsPsgVectors.cmake): writes are
 * applied in order through the memory-system bus, then every read is compared with the register
 * store of its address. steps is the run length of the engine families (0 here). */
typedef struct ivs_pv_write {
  uint16_t addr;
  uint16_t value;
  uint32_t step;
} ivs_pv_write;

typedef struct ivs_pv_read {
  uint16_t addr;
  uint16_t value;
} ivs_pv_read;

typedef struct ivs_pv {
  const char *id;
  const char *rule;
  uint32_t source; /* 1: kind source, 0: kind policy */
  const ivs_pv_write *writes;
  uint32_t n_writes;
  const ivs_pv_read *reads;
  uint32_t n_reads;
  uint32_t steps;
} ivs_pv;

/* One row of the register-address literal: address, write mask, read mask. */
typedef struct ivs_pv_lit {
  uint16_t addr;
  uint16_t wmask;
  uint16_t rmask;
} ivs_pv_lit;

/* Family reg (plan 05-04, psg_reg.c): the register file, the reset and the bus wrapper. The
 * fixed cases (hand-written ids) and the vector cases (one per vector of regs.txt and reset.txt). */
extern const ivs_test_case ivs_psg_reg_cases[];
extern const size_t ivs_psg_reg_count;
size_t ivs_psg_reg_vector_count(void);
const char *ivs_psg_reg_vector_id(size_t i);
int ivs_psg_reg_vector_run(size_t i);
/* The data files the family derives from, with their SHA-256 (re-checked by tests-inventory). */
const ivs_test_source *ivs_psg_reg_sources(size_t *n);

#endif

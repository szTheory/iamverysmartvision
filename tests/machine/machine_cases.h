/* SPDX-License-Identifier: MIT */
/* Suite machine (D-15, plans 03-02..03-12): one executable, several case families. Each family
 * file defines its case table and count here declared; machine_main.c concatenates them into
 * one suite. A family that reads committed data also exports its source list, so the result
 * file carries a `# source <path> <sha256>` line that tests-inventory re-hashes. */
#ifndef IVS_MACHINE_CASES_H
#define IVS_MACHINE_CASES_H

#include <stddef.h>
#include <stdint.h>

#include "ivs/ivs.h"
#include "ivs_test.h"

/* Family sha256 (plan 03-02): core SHA-256 against literal digests (machine_sha256.c). */
extern const ivs_test_case ivs_machine_sha256_cases[];
extern const size_t ivs_machine_sha256_count;
extern const ivs_test_source *const ivs_machine_sha256_sources;
extern const size_t ivs_machine_sha256_source_count;

/* Family create (plan 03-09): requirements, create, info, destroy, status names, API version
 * (machine_create.c). */
extern const ivs_test_case ivs_machine_create_cases[];
extern const size_t ivs_machine_create_count;

/* Family digest (plan 03-09): canonical state digest, literal streams, coverage, sensitivity
 * and refusals (machine_digest.c). */
extern const ivs_test_case ivs_machine_digest_cases[];
extern const size_t ivs_machine_digest_count;

/* Family load (plan 03-10): typed EXEC, GROM and cartridge loads, reports, layer 2, options and
 * unload (machine_load.c). */
extern const ivs_test_case ivs_machine_load_cases[];
extern const size_t ivs_machine_load_count;
extern const ivs_test_source *const ivs_machine_load_sources;
extern const size_t ivs_machine_load_source_count;

/* Families refuse and prec (plan 03-11): the load refusal matrix, one case per (row, prior),
 * and the precedence rows (machine_refuse.c). Indexed rather than a case table: the ids are
 * built from the row table and the priors at first use. */
size_t ivs_machine_refuse_count(void);
const char *ivs_machine_refuse_id(size_t i);
const char *ivs_machine_refuse_skip(size_t i);
int ivs_machine_refuse_run(size_t i);

/* Family reset (plan 03-12): power-on reset, the internal step seam and the reset refusal
 * outcomes that are not matrix rows (machine_reset.c). */
extern const ivs_test_case ivs_machine_reset_cases[];
extern const size_t ivs_machine_reset_count;

/* ---- shared stand-in content (plan 03-12, machine_standin.c) ----
 * Instances in exact-size heap storage and generated images (D-24): IVS_MS_PROG_A and _B are
 * two hand-assembled EXEC programs at $1000 (see machine_standin.c for their decles and the
 * vector rows the encodings come from); content A and B pair a program with its own GROM pattern
 * and one cartridge segment (A: 16 words at $5000, B: 24 words at $6000). */
#define IVS_MS_PROFILE "intv-2609-ntsc-family-p1"
#define IVS_MS_EXEC_BYTES 8192u
#define IVS_MS_GROM_BYTES 2048u
#define IVS_MS_PROG_A 0u
#define IVS_MS_PROG_B 1u
#define IVS_MS_CONTENT_A 0u
#define IVS_MS_CONTENT_B 1u

typedef struct ivs_ms_inst {
  uint8_t *storage; /* IVS_MACHINE_BYTES of heap */
  ivs_machine *m;
} ivs_ms_inst;

/* Heap images (the caller frees; NULL when malloc fails). */
uint8_t *ivs_ms_exec(uint32_t program);
uint8_t *ivs_ms_grom(uint32_t seed);
/* One-segment IVSC v1 container sealed with the core SHA-256. */
uint8_t *ivs_ms_cart(uint32_t addr, uint32_t len_words, uint32_t seed, size_t *out_len);

/* A new instance in heap storage poisoned with poison before create. 1 on success. */
int ivs_ms_new(ivs_ms_inst *x, uint8_t poison);
void ivs_ms_free(ivs_ms_inst *x);
/* Loads bytes (then frees them); 1 on IVS_OK. */
int ivs_ms_load(ivs_machine *m, uint32_t kind, uint8_t *bytes, size_t len);
/* Loads content A or B (EXEC, GROM, cartridge). */
int ivs_ms_content(ivs_machine *m, uint32_t content);
/* ivs_ms_new, ivs_ms_content, then a power-on reset. */
int ivs_ms_powered(ivs_ms_inst *x, uint32_t content, uint8_t poison);
/* Steps units units through the seam, adding their CPU cycles to *cycles when not NULL. */
int ivs_ms_step(ivs_machine *m, uint32_t units, uint64_t *cycles);
/* The composite and CPU digests. */
int ivs_ms_digests(const ivs_machine *m, uint8_t composite[32], uint8_t cpu[32]);

#endif

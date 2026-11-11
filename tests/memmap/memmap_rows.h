/* SPDX-License-Identifier: MIT */
/* Suite memmap, literal-map family (plan 03-06): cases generated from the sourced literal maps
 * data/memmap/<profile>.literal.txt (cmake/IvsMemmapProbes.cmake). memmap_main.c appends them to its
 * case table and reports the literal files as the suite's sources. */
#ifndef IVS_MEMMAP_ROWS_H
#define IVS_MEMMAP_ROWS_H

#include <stddef.h>

#include "ivs_test.h"

/* Builds the case ids; returns 0 when an id does not fit. Call once before the other functions. */
int mm_rows_init(void);
size_t mm_rows_count(void);
const char *mm_rows_id(size_t i);
int mm_rows_run(size_t i);
/* The literal files the cases derive from, with their SHA256 (re-checked by tests-inventory). */
const ivs_test_source *mm_rows_sources(size_t *n);

#endif

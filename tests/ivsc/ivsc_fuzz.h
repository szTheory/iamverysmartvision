/* SPDX-License-Identifier: MIT */
/* Suite ivsc, seeded fuzz family (D-34, plan 03-08): the interface between ivsc_main.c, which
 * owns the generated fixture table and the canonical re-encoder, and ivsc_fuzz.c, which owns the
 * ivsc.fuzz.* cases. Test code only. */
#ifndef IVSC_FUZZ_H
#define IVSC_FUZZ_H

#include <stddef.h>
#include <stdint.h>

#include "content/ivsc.h"

/* ivsc.fuzz.seed-01 .. seed-08 and ivsc.fuzz.reasons-covered. */
#define IVSC_FUZZ_CASES 9u

/* Provided by ivsc_main.c. The mutation bases are the accepted positive records whose
 * container is literal hex in the fixture file (the long build records are left out to keep
 * the family fast); 0 when i is past the last base. */
size_t ivsc_test_base_count(void);
int ivsc_test_base(size_t i, const uint8_t **bytes, size_t *len);
/* 1 when re-encoding the accepted view v of the len bytes at b reproduces b exactly. */
int ivsc_test_reencode_equal(const uint8_t *b, size_t len, const ivs_ivsc_view *v);

/* Provided by ivsc_fuzz.c. */
const char *ivsc_fuzz_case_id(size_t i);
int ivsc_fuzz_case_run(size_t i);

#endif

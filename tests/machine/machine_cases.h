/* SPDX-License-Identifier: MIT */
/* Suite machine (D-15, plans 03-02..03-12): one executable, several case families. Each family
 * file defines its case table and count here declared; machine_main.c concatenates them into
 * one suite. A family that reads committed data also exports its source list, so the result
 * file carries a `# source <path> <sha256>` line that tests-inventory re-hashes. */
#ifndef IVS_MACHINE_CASES_H
#define IVS_MACHINE_CASES_H

#include <stddef.h>

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

#endif

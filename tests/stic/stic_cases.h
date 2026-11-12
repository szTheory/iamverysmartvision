/* SPDX-License-Identifier: MIT */
/* Suite stic (VID-01, plan 06-06): the STIC register file, the access windows, the display-enable handshake and the
 * frame timeline. One executable; each family file defines its case table here declared and stic_main.c
 * concatenates them into one suite. Module-level cases perturb a stack copy of the generated ivs_stic_params; the
 * machine-path cases drive stand-in guests (decles derived in comments from data/cp1610/opcode-families.json).
 * Expected values are relations to the parameters or values from the independent literals, never a policy number
 * (D-02) and never a core table. */
#ifndef IVS_STIC_CASES_H
#define IVS_STIC_CASES_H

#include <stddef.h>
#include <stdint.h>

#include "ivs_test.h"

#define SC_CHECK(cond)                                                                                 \
  do {                                                                                                 \
    if (!(cond)) {                                                                                     \
      ivs_test_diag("check failed at line %d: %s", __LINE__, #cond);                                   \
      return IVS_TEST_FAIL;                                                                            \
    }                                                                                                  \
  } while (0)

/* The registry index of the only profile of the first slice. */
#define SC_PROFILE 0u

extern const ivs_test_case ivs_stic_events_cases[];
extern const size_t ivs_stic_events_count;

#endif

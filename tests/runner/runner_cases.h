/* SPDX-License-Identifier: MIT */
/* Suite runner: the interface between runner_main.c, which owns the suite and the outcome
 * family, and the family files runner_parse.c (runner.parse.*, plan 04-17) and runner_fuzz.c
 * (runner.fuzz.*, plan 04-17). Each family exposes its case table by index. Test code only. */
#ifndef RUNNER_CASES_H
#define RUNNER_CASES_H

#include <stddef.h>

#include "ivs_test.h"

/* Fails the case with a diagnostic naming the line and the failed expression. */
#define IVS_CHECK(cond)                                                                                \
  do {                                                                                                 \
    if (!(cond)) {                                                                                     \
      ivs_test_diag("check failed at line %d: %s", __LINE__, #cond);                                   \
      return IVS_TEST_FAIL;                                                                            \
    }                                                                                                  \
  } while (0)

size_t runner_parse_case_count(void);
const ivs_test_case *runner_parse_case(size_t i);

size_t runner_fuzz_case_count(void);
const ivs_test_case *runner_fuzz_case(size_t i);

#endif

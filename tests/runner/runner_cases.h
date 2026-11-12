/* SPDX-License-Identifier: MIT */
/* Suite runner: the interface between runner_main.c, which owns the suite and the outcome
 * family, and the family files runner_parse.c (runner.parse.*, plan 04-17), runner_fuzz.c
 * (runner.fuzz.*, plan 04-17), runner_exec.c (runner.exec.*, plan 04-18) and runner_time.c
 * (runner.time.*, plan 04-19). Each family exposes its cases by index. Test code only. */
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

/* Family parse (runner_parse.c): case count, id and run by index (id NULL past the end). */
size_t runner_parse_count(void);
const char *runner_parse_id(size_t i);
int runner_parse_run(size_t i);

/* Accepted scenario texts of runner_parse.c, the mutation bases of the fuzz family: 0 when i is
 * past the last base. */
int runner_parse_base(size_t i, const char **text, size_t *len);

/* Family fuzz (runner_fuzz.c). */
size_t runner_fuzz_count(void);
const char *runner_fuzz_id(size_t i);
int runner_fuzz_run(size_t i);

/* Family exec (runner_exec.c, plan 04-18): the executor and report writer with a fake host. */
size_t runner_exec_count(void);
const char *runner_exec_id(size_t i);
int runner_exec_run(size_t i);

/* Family time (runner_time.c, plan 04-19): slicing, budget, wall cap and cancellation with a fake
 * clock and a fake cancel flag. */
size_t runner_time_count(void);
const char *runner_time_id(size_t i);
int runner_time_run(size_t i);

/* Family refpcm (runner_refpcm.c, plan 05-10): the reference PCM converter of tools/pcm and its
 * judge self-controls. */
size_t runner_refpcm_count(void);
const char *runner_refpcm_id(size_t i);
int runner_refpcm_run(size_t i);

#endif

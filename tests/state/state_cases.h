/* SPDX-License-Identifier: MIT */
/* Suite state (STA-01, STA-02, plans 08-11 to 08-15, D-16): the IVSN v1 snapshot container and the machine state it carries. One
 * executable; each family file defines its case table here declared and state_main.c concatenates them into one suite. Expected values
 * come from the hand-derived literals of plan 08-09 (data/expected/ivsn-literals.ids and data/state/schema.literal.txt, read through
 * the generated includes of tests/ivsn-literals-inc.cmake, so no number is retyped) and from the specification
 * docs/formats/ivsn-v1.md, never from the code under test. The family prefixes of the ids are state.size., state.literal.,
 * state.save., state.read., state.restore., state.witness., state.cut., state.sweep., state.fuzz. and state.judge. */
#ifndef IVS_STATE_CASES_H
#define IVS_STATE_CASES_H

#include <stddef.h>
#include <stdint.h>

#include "ivs_test.h"
#include "machine_cases.h"
#include "memmap/memsys.h"

#define ST_CHECK(cond)                                                                                 \
  do {                                                                                                 \
    if (!(cond)) {                                                                                     \
      ivs_test_diag("check failed at line %d: %s", __LINE__, #cond);                                   \
      return IVS_TEST_FAIL;                                                                            \
    }                                                                                                  \
  } while (0)

/* Records a failed condition in an ok flag, so that a case can free what it holds before it returns. The condition is always evaluated
 * (it may hold a call), and every failure is reported, not only the first. */
#define ST_REQ(okv, cond)                                                                              \
  do {                                                                                                 \
    if (!(cond)) {                                                                                     \
      ivs_test_diag("check failed at line %d: %s", __LINE__, #cond);                                   \
      (okv) = 0;                                                                                       \
    }                                                                                                  \
  } while (0)

/* Fault-run cost rule (D-40, D-37, plan 08-11): the fault executables of this suite are compiled with IVS_STATE_FAULT_RUN (the
 * FAULT_DEFINITIONS of ivs_fault_suite(state ...)); the normal executable never is. Every named fault re-runs the whole suite
 * unsanitized, so a heavy id that is not the must-fail id of any fault runs a reduced point set under it (every 16th offset or row, a
 * one-frame window), never a different check; the must-fail ids and the normal builds always run the full set. ST_FAULT_STRIDE is
 * the stride a heavy loop takes: 1 in a normal build, 16 in a fault run. */
#ifdef IVS_STATE_FAULT_RUN
#define ST_FAULT_STRIDE 16u
#else
#define ST_FAULT_STRIDE 1u
#endif

/* The registry index of profile IVS_MS_PROFILE, or ivs_profile_count() when it is not registered. */
static inline uint32_t st_profile_index(void)
{
  uint32_t p;
  for (p = 0u; p < ivs_profile_count(); p++) {
    const char *id = ivs_profile_get(p)->id;
    const char *want = IVS_MS_PROFILE;
    size_t i = 0u;
    while (id[i] != '\0' && id[i] == want[i]) {
      i++;
    }
    if (id[i] == '\0' && want[i] == '\0') {
      return p;
    }
  }
  return ivs_profile_count();
}

/* The literals of data/expected/ivsn-literals.ids (state_literal.c). ivsn_lit returns the value text of a key, or NULL after a
 * diagnostic for a key that is not there: a test fails on NULL, never on a made-up value. ivsn_lit_u32 parses a decimal literal;
 * ivsn_lit_hex decodes a hex literal of exactly n bytes. Both return 1 on success and 0 (with a diagnostic) otherwise. */
const char *ivsn_lit(const char *key);
int ivsn_lit_u32(const char *key, uint32_t *out);
int ivsn_lit_hex(const char *key, uint8_t *out, size_t n);

/* The lines of data/state/schema.literal.txt without their newlines: line 0 is `IVSSCHEMA 1`, line i the descriptor of table row i - 1. */
size_t ivsn_schema_line_count(void);
const char *ivsn_schema_line(size_t i);

/* The seven sections of a snapshot in table order: the domain ids and the names of the keys of data/expected/ivsn-literals.ids
 * (section-bytes.<name>, section-offset.<name>). */
#define ST_SECTIONS 7u
extern const uint8_t st_section_domain[ST_SECTIONS];
extern const char *const st_section_name[ST_SECTIONS];

/* The structure comparison of a snapshot (state_literal.c, plan 08-11): every number it expects comes from the literals of
 * data/expected/ivsn-literals.ids or from the live instance m the snapshot was taken from, never from the writer. Returns 0 when
 * every comparison holds; otherwise the number of the first comparison that fails (1 the length, 2 the header prefix, 3 a section
 * table entry, 4 the schema digest, 5 the profile id, 6 the parameter digest, 7 a slot record, 8 the composite digest, 9 a section
 * digest, 10 the trailer, 11 the bytes of a section) and sets *what to a static text that says which. The comparison is a function of
 * the bytes it is given, so a test can hand it a damaged copy and require a nonzero result (state.judge.header-literal-corrupt). */
int st_structure_diff(const uint8_t *snap, size_t len, const ivs_machine *m, const char **what);

/* The data files this suite read, for the `# source` lines of the result file (the gate step tests-inventory re-hashes them). */
const ivs_test_source *ivs_state_sources(size_t *n);

extern const ivs_test_case ivs_state_literal_cases[];
extern const size_t ivs_state_literal_count;
extern const ivs_test_case ivs_state_save_cases[];
extern const size_t ivs_state_save_count;

#endif

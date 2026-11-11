/* SPDX-License-Identifier: MIT */
/* Suite runner, family parse (D-18, D-22, D-23, plan 04-17): the IVSS v1 scenario parser of the
 * runner library. runner.parse.diag-boot-pass feeds the scenario text of diag-boot-pass, whose
 * 18 assert lines come verbatim from data/golden/diag-cpu-1.expect (diag_cpu_1_expect_text.inc),
 * and compares the parsed asserts with the independent cmake reading of the same lines
 * (diag_cpu_1_expect.inc, the X-macro form suite run reads). */
#include <stdint.h>
#include <string.h>

#include "ivs/ivs.h"
#include "ivs_runner.h"
#include "ivs_test.h"
#include "runner_cases.h"

/* Parses text (without its NUL) and requires acceptance. */
static int parse_ok(const char *text, ivs_scenario *s)
{
  ivs_scenario_error e;
  int r = ivs_scenario_parse((const uint8_t *)text, strlen(text), s, &e);
  if (r != 0) {
    ivs_test_diag("refused: line=%u reason=%s", (unsigned)e.line, ivs_scenario_reason_name(e.reason));
    return 0;
  }
  return 1;
}

/* ---- the diagnostic scenario (plan 04-16 predicate, plan 04-19 writes the .ivss file) ---- */

static const char diag_boot_pass_text[] = "ivss 1\n"
                                          "id diag-boot-pass\n"
                                          "profile intv-2609-ntsc-family-p1\n"
                                          "content exec boot-stub\n"
                                          "content grom zero-grom\n"
                                          "content cart diag-cpu-1.ivsc\n"
                                          "run-to 4000 cpu-cycles stop halted\n"
#include "diag_cpu_1_expect_text.inc"
    ;

#define PD_HALTED 1u
#define PD_REG 2u
#define PD_MEM 3u
typedef struct pd_clause {
  uint32_t kind;
  uint16_t where; /* register number or address */
  uint16_t value;
} pd_clause;

#define IVS_EXPECT_HALTED(y) {PD_HALTED, 0u, (uint16_t)(y)},
#define IVS_EXPECT_REG(r, v) {PD_REG, (uint16_t)(r), (uint16_t)(v)},
#define IVS_EXPECT_MEM(a, v) {PD_MEM, (uint16_t)(a), (uint16_t)(v)},
static const pd_clause diag_clauses[] = {
#include "diag_cpu_1_expect.inc"
};
#undef IVS_EXPECT_HALTED
#undef IVS_EXPECT_REG
#undef IVS_EXPECT_MEM

static int parse_diag_boot_pass(void)
{
  ivs_scenario s;
  size_t n = sizeof diag_clauses / sizeof diag_clauses[0];
  size_t i;
  IVS_CHECK(parse_ok(diag_boot_pass_text, &s));
  IVS_CHECK(strcmp(s.id, "diag-boot-pass") == 0);
  IVS_CHECK(strcmp(s.profile, "intv-2609-ntsc-family-p1") == 0);
  IVS_CHECK(s.content[IVS_SCN_ROLE_EXEC].kind == IVS_SCN_CONTENT_BOOT_STUB);
  IVS_CHECK(s.content[IVS_SCN_ROLE_GROM].kind == IVS_SCN_CONTENT_ZERO_GROM);
  IVS_CHECK(s.content[IVS_SCN_ROLE_CART].kind == IVS_SCN_CONTENT_PATH);
  IVS_CHECK(strcmp(s.content[IVS_SCN_ROLE_CART].path, "diag-cpu-1.ivsc") == 0);
  IVS_CHECK(s.content[IVS_SCN_ROLE_CART].line == 6u);
  for (i = 0u; i < IVS_SCN_ROLES; i++) {
    IVS_CHECK(s.expect[i].load_set == 0u && s.expect[i].sha_set == 0u);
  }
  IVS_CHECK(s.wall_timeout_ms == 0u);
  IVS_CHECK(s.n_steps == 1u && s.step_unit == IVS_SCN_UNIT_CPU_CYCLES);
  IVS_CHECK(s.steps[0].target == 4000u && s.steps[0].unit == IVS_SCN_UNIT_CPU_CYCLES);
  IVS_CHECK(s.steps[0].stop == IVS_SCN_STOP_HALTED && s.steps[0].line == 7u);
  IVS_CHECK(s.n_inputs == 0u && s.input_unit == 0u);
  /* 18 clauses, each read the same way by the parser and by the cmake X-macro generator. */
  IVS_CHECK(n == 18u);
  IVS_CHECK(s.n_asserts == n);
  for (i = 0u; i < n; i++) {
    const ivs_scenario_assert *a = &s.asserts[i];
    const pd_clause *c = &diag_clauses[i];
    IVS_CHECK(a->step == 0u && a->op == IVS_SCN_OP_EQ && a->line == 8u + (uint32_t)i);
    if (c->kind == PD_HALTED) {
      IVS_CHECK(a->subject == IVS_SCN_SUBJ_HALTED && a->value == c->value);
    } else if (c->kind == PD_REG) {
      IVS_CHECK(a->subject == IVS_SCN_SUBJ_REG && a->index == c->where && a->value == c->value);
    } else {
      IVS_CHECK(a->subject == IVS_SCN_SUBJ_MEM && a->addr_lo == c->where && a->addr_hi == c->where);
      IVS_CHECK(a->value == c->value);
    }
  }
  return IVS_TEST_PASS;
}

static const ivs_test_case parse_cases[] = {
    {"runner.parse.diag-boot-pass", parse_diag_boot_pass, NULL},
};

size_t runner_parse_case_count(void)
{
  return sizeof parse_cases / sizeof parse_cases[0];
}

const ivs_test_case *runner_parse_case(size_t i)
{
  return i < runner_parse_case_count() ? &parse_cases[i] : NULL;
}

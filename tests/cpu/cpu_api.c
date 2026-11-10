/* SPDX-License-Identifier: MIT */
/* Suite cpu-api: the internal CPU contract (D-02, D-03, D-06, D-12). State and access sizes
 * and the state validation rules. Programs use CLRC and SETC only. */
#include <stdint.h>
#include <stdio.h>
#include <string.h>

#include "cpu/cp1610.h"
#include "ivs_test.h"

/* Fails the case with a diagnostic naming the line and the failed expression. */
#define IVS_CHECK(cond)                                                                                \
  do {                                                                                                 \
    if (!(cond)) {                                                                                     \
      ivs_test_diag("check failed at line %d: %s", __LINE__, #cond);                                   \
      return IVS_TEST_FAIL;                                                                            \
    }                                                                                                  \
  } while (0)

/* ---- sizes (D-02, D-03) ---- */

static int api_state_size_32(void)
{
  IVS_CHECK(sizeof(ivs_cp1610) == 32u);
  IVS_CHECK(ivs_cp1610_state_size() == 32u);
  return IVS_TEST_PASS;
}

static int api_access_size_12(void)
{
  IVS_CHECK(sizeof(ivs_cp1610_access) == 12u);
  IVS_CHECK(IVS_CP1610_MAX_ACCESSES == 8u);
  return IVS_TEST_PASS;
}

/* ---- validation (D-03) ---- */

static int api_validate_zeroed_state(void)
{
  ivs_cp1610 s;
  memset(&s, 0, sizeof s);
  IVS_CHECK(s.run_state == IVS_CP1610_RS_UNRESET);
  IVS_CHECK(ivs_cp1610_state_validate(&s) == IVS_OK);
  IVS_CHECK(ivs_cp1610_state_validate(NULL) == IVS_E_BAD_ARG);
  return IVS_TEST_PASS;
}

static int api_validate_rejects_bad_run_state(void)
{
  ivs_cp1610 s;
  unsigned v;
  for (v = 0u; v <= 255u; v++) {
    memset(&s, 0, sizeof s);
    s.run_state = (uint8_t)v;
    if (v <= IVS_CP1610_RS_FAULTED) {
      IVS_CHECK(ivs_cp1610_state_validate(&s) == IVS_OK);
    } else {
      IVS_CHECK(ivs_cp1610_state_validate(&s) == IVS_E_BAD_STATE);
    }
  }
  return IVS_TEST_PASS;
}

static int api_validate_rejects_bad_fault_reason(void)
{
  ivs_cp1610 s;
  unsigned v;
  for (v = 0u; v <= 255u; v++) {
    memset(&s, 0, sizeof s);
    s.fault_reason = (uint8_t)v;
    if (v <= IVS_CP1610_FAULT_UNIMPLEMENTED) {
      IVS_CHECK(ivs_cp1610_state_validate(&s) == IVS_OK);
    } else {
      IVS_CHECK(ivs_cp1610_state_validate(&s) == IVS_E_BAD_STATE);
    }
  }
  return IVS_TEST_PASS;
}

static int api_validate_rejects_reserved_nonzero(void)
{
  ivs_cp1610 s;
  size_t i;
  for (i = 0u; i < sizeof s.reserved; i++) {
    memset(&s, 0, sizeof s);
    s.reserved[i] = 0x01u;
    IVS_CHECK(ivs_cp1610_state_validate(&s) == IVS_E_BAD_STATE);
    s.reserved[i] = 0x80u;
    IVS_CHECK(ivs_cp1610_state_validate(&s) == IVS_E_BAD_STATE);
  }
  return IVS_TEST_PASS;
}

static int api_validate_rejects_status_low_bits(void)
{
  ivs_cp1610 s;
  unsigned bit;
  for (bit = 0u; bit < 4u; bit++) {
    memset(&s, 0, sizeof s);
    s.status = (uint8_t)(1u << bit);
    IVS_CHECK(ivs_cp1610_state_validate(&s) == IVS_E_BAD_STATE);
  }
  /* The four flag bits S Z OV C (pdf 45 GSWD layout) are valid in any combination. */
  memset(&s, 0, sizeof s);
  s.status = IVS_CP1610_STATUS_MASK;
  IVS_CHECK(ivs_cp1610_state_validate(&s) == IVS_OK);
  return IVS_TEST_PASS;
}

static const ivs_test_case api_cases[] = {
    {"cpu-api.access-size-12", api_access_size_12, NULL},
    {"cpu-api.state-size-32", api_state_size_32, NULL},
    {"cpu-api.validate-rejects-bad-fault-reason", api_validate_rejects_bad_fault_reason, NULL},
    {"cpu-api.validate-rejects-bad-run-state", api_validate_rejects_bad_run_state, NULL},
    {"cpu-api.validate-rejects-reserved-nonzero", api_validate_rejects_reserved_nonzero, NULL},
    {"cpu-api.validate-rejects-status-low-bits", api_validate_rejects_status_low_bits, NULL},
    {"cpu-api.validate-zeroed-state", api_validate_zeroed_state, NULL},
};

int main(int argc, char **argv)
{
  ivs_test_suite suite;
  ivs_test_suite_from_cases(&suite, "cpu-api", api_cases, sizeof api_cases / sizeof api_cases[0]);
  return ivs_test_main(argc, argv, &suite);
}

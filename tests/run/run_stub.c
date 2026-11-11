/* SPDX-License-Identifier: MIT */
/* Suite run, family stub (plan 04-14, D-28..D-30, RUN-06): the project boot stub, a 22-word
 * hand-assembled listing (fixtures/FX-boot-stub/boot-stub.listing.txt), packed by tools/pack into
 * the build-tree EXEC image whose bytes ivs_images.inc carries, loaded with a 2048-byte zero GROM
 * (a project stand-in, not a GROM) and driven from a power-on reset through the public API.
 *   run.stub.path   one run call per unit (target now + 1): after the reset entry unit, R7 at the
 *                   start of each following unit is 1000, 1010, 1011, 1013, 5000 (B $1010, DIS,
 *                   MVII #$0340,R6, J $5000), hand-derived from the listing; the case stops at the
 *                   first unit at 5000, since the cartridge window is empty.
 * The expected values are literals worked out from the listing and the manual pages it cites,
 * never read back from the code under test. No image byte is committed (D-30). */
#include <stddef.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>

#include "ivs/ivs.h"
#include "ivs_test.h"
#include "run_cases.h"

#include "ivs_images.inc"

#define IVS_CHECK(cond)                                                                                \
  do {                                                                                                 \
    if (!(cond)) {                                                                                     \
      ivs_test_diag("check failed at line %d: %s", __LINE__, #cond);                                   \
      return IVS_TEST_FAIL;                                                                            \
    }                                                                                                  \
  } while (0)

/* Like IVS_CHECK, inside a case that must release its instance (label done). */
#define RS_CHECK(cond)                                                                                 \
  do {                                                                                                 \
    if (!(cond)) {                                                                                     \
      ivs_test_diag("check failed at line %d: %s", __LINE__, #cond);                                   \
      goto done;                                                                                       \
    }                                                                                                  \
  } while (0)

#define RS_POISON 0x5Au
#define RS_ZERO_GROM_BYTES 2048u

/* R7 at the start of the units after the reset entry (listing lines 1000, 1010, 1011, 1013, then
 * the cartridge entry 5000). */
static const uint16_t rs_path_r7[] = {0x1000u, 0x1010u, 0x1011u, 0x1013u, 0x5000u};

/* ---- helpers ---- */

/* A heap copy of the build-tree stub image (ivs_ms_load frees it), or NULL. */
static uint8_t *rs_stub_image(void)
{
  uint8_t *b = malloc(IVS_IMG_BOOT_STUB_BYTES);
  if (b != NULL) {
    memcpy(b, ivs_img_boot_stub, IVS_IMG_BOOT_STUB_BYTES);
  }
  return b;
}

/* A powered instance: stub EXEC from bytes (consumed), 2048 zero bytes as GROM, no cartridge, a
 * power-on reset. 1 on success. */
static int rs_powered(ivs_ms_inst *x, uint8_t *exec)
{
  ivs_status st;
  if (!ivs_ms_new(x, RS_POISON)) {
    free(exec);
    return 0;
  }
  if (!ivs_ms_load(x->m, IVS_KIND_EXEC, exec, IVS_IMG_BOOT_STUB_BYTES) ||
      !ivs_ms_load(x->m, IVS_KIND_GROM, calloc(RS_ZERO_GROM_BYTES, 1u), RS_ZERO_GROM_BYTES)) {
    ivs_ms_free(x);
    return 0;
  }
  st = ivs_machine_reset(x->m, IVS_RESET_POWER_ON);
  if (st != IVS_OK) {
    ivs_test_diag("reset refused (%s)", ivs_status_name(st));
    ivs_ms_free(x);
    return 0;
  }
  return 1;
}

static int rs_cpu(const ivs_machine *m, ivs_cpu_state_report *r)
{
  memset(r, 0, sizeof *r);
  r->struct_size = (uint32_t)sizeof *r;
  if (ivs_machine_cpu_state(m, r) != IVS_OK) {
    ivs_test_diag("cpu_state refused");
    return 0;
  }
  return 1;
}

/* One run call to now + 1 (exactly one unit); 1 when it ran one unit and stopped at the target. */
static int rs_one_unit(ivs_machine *m)
{
  ivs_info info;
  ivs_run_request q;
  ivs_run_report rep;
  ivs_status st;
  memset(&info, 0, sizeof info);
  info.struct_size = (uint32_t)sizeof info;
  if (ivs_machine_info(m, &info) != IVS_OK) {
    ivs_test_diag("info refused");
    return 0;
  }
  memset(&q, 0, sizeof q);
  q.struct_size = (uint32_t)sizeof q;
  q.target_ticks = info.time_ticks + 1u;
  memset(&rep, 0, sizeof rep);
  rep.struct_size = (uint32_t)sizeof rep;
  st = ivs_machine_run(m, &q, &rep);
  if (st != IVS_OK || rep.stop_reason != IVS_STOP_TARGET || rep.units_executed != 1u) {
    ivs_test_diag("run to now + 1: status %s stop %u units %llu", ivs_status_name(st), (unsigned)rep.stop_reason,
                  (unsigned long long)rep.units_executed);
    return 0;
  }
  return 1;
}

/* Drives a powered stub instance from reset to the first unit at the cartridge entry 5000, checking
 * the path; 1 when every check holds. */
static int rs_to_entry(ivs_ms_inst *x)
{
  ivs_cpu_state_report r;
  size_t k;
  if (!rs_cpu(x->m, &r)) {
    return 0;
  }
  if (r.run_state != IVS_CPU_STATE_RESET_PENDING) {
    ivs_test_diag("after reset: run_state %u, want RESET_PENDING", (unsigned)r.run_state);
    return 0;
  }
  for (k = 0u; k < sizeof rs_path_r7 / sizeof rs_path_r7[0]; k++) {
    if (!rs_one_unit(x->m) || !rs_cpu(x->m, &r)) {
      return 0;
    }
    if (r.run_state != IVS_CPU_STATE_RUNNING || r.r[7] != rs_path_r7[k]) {
      ivs_test_diag("unit %u: run_state %u R7 %04X, want RUNNING and R7 %04X", (unsigned)(k + 1u),
                    (unsigned)r.run_state, (unsigned)r.r[7], (unsigned)rs_path_r7[k]);
      return 0;
    }
    /* After DIS (R7 = 1011) the interrupt system is off; after MVII #$0340,R6 (R7 = 1013) R6 is the
     * project stack start. */
    if ((rs_path_r7[k] == 0x1011u && r.intr_enabled != 0u) || (rs_path_r7[k] == 0x1013u && r.r[6] != 0x0340u)) {
      ivs_test_diag("unit %u: intr_enabled %u R6 %04X", (unsigned)(k + 1u), (unsigned)r.intr_enabled,
                    (unsigned)r.r[6]);
      return 0;
    }
  }
  return 1;
}

/* ---- cases ---- */

static int rs_path(void)
{
  ivs_ms_inst x;
  int rc = IVS_TEST_FAIL;
  IVS_CHECK(IVS_IMG_BOOT_STUB_BYTES == 8192u);
  IVS_CHECK(rs_powered(&x, rs_stub_image()));
  RS_CHECK(rs_to_entry(&x));
  rc = IVS_TEST_PASS;
done:
  ivs_ms_free(&x);
  return rc;
}

const ivs_test_case ivs_run_stub_cases[] = {
    {"run.stub.path", rs_path, NULL},
};

const size_t ivs_run_stub_count = sizeof ivs_run_stub_cases / sizeof ivs_run_stub_cases[0];

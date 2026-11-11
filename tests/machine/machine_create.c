/* SPDX-License-Identifier: MIT */
/* Suite machine, family create (D-01..D-04, D-08, plan 03-09): storage requirements, create,
 * info and destroy through the public header only (ivs/ivs_machine.h); machine/machine.h is read
 * for the instance size and the magic. Instances live in exact-size heap storage, so ASan sees
 * any byte the core touches outside them; never on the stack (about 113 KB). */
#include <stdint.h>
#include <stdlib.h>
#include <string.h>

#include "ivs/ivs_machine.h"
#include "ivs_test.h"
#include "machine/machine.h"
#include "machine_cases.h"

/* Fails the case with a diagnostic naming the line and the failed expression. */
#define IVS_CHECK(cond)                                                                                \
  do {                                                                                                 \
    if (!(cond)) {                                                                                     \
      ivs_test_diag("check failed at line %d: %s", __LINE__, #cond);                                   \
      return IVS_TEST_FAIL;                                                                            \
    }                                                                                                  \
  } while (0)

#define MC_PROFILE "intv-2609-ntsc-family-p1"

static ivs_create_options mc_opts(const char *id)
{
  ivs_create_options o;
  memset(&o, 0, sizeof o);
  o.struct_size = (uint32_t)sizeof o;
  o.flags = 0u;
  o.profile_id = id;
  return o;
}

/* ---- tracer ---- */

/* The requirements call reports the internal instance size and 16-byte alignment. */
static int mc_requirements(void)
{
  ivs_storage_requirements r;
  memset(&r, 0xA5, sizeof r);
  r.struct_size = (uint32_t)sizeof r;
  IVS_CHECK(ivs_machine_storage_requirements(MC_PROFILE, &r) == IVS_OK);
  IVS_CHECK(r.struct_size == (uint32_t)sizeof r);
  IVS_CHECK(r.reserved == 0u);
  IVS_CHECK(r.size_bytes == (uint64_t)sizeof(struct ivs_machine));
  IVS_CHECK(r.size_bytes == (uint64_t)IVS_MACHINE_BYTES);
  IVS_CHECK(r.align_bytes == 16u);
  return IVS_TEST_PASS;
}

/* Create in exact-size heap storage, info shows phase OFF and three empty slots, destroy, and
 * info on the destroyed handle is BAD_HANDLE. */
static int mc_ok_info_destroy(void)
{
  static const uint8_t zero32[32] = {0};
  ivs_create_options o = mc_opts(MC_PROFILE);
  ivs_machine *m = NULL;
  ivs_info info;
  uint32_t i;
  int rc = IVS_TEST_FAIL;
  uint8_t *st = malloc(IVS_MACHINE_BYTES);
  IVS_CHECK(st != NULL);
  memset(st, 0xA5, IVS_MACHINE_BYTES);
  if (ivs_machine_create(&o, st, IVS_MACHINE_BYTES, &m) != IVS_OK || m != (ivs_machine *)(void *)st) {
    ivs_test_diag("create failed");
    goto done;
  }
  memset(&info, 0xA5, sizeof info);
  info.struct_size = (uint32_t)sizeof info;
  if (ivs_machine_info(m, &info) != IVS_OK || info.struct_size != (uint32_t)sizeof info ||
      info.phase != IVS_PHASE_OFF || info.time_ticks != 0u || info.profile_index != 0u || info.reserved != 0u) {
    ivs_test_diag("info header wrong");
    goto done;
  }
  for (i = 0u; i < IVS_SLOT_COUNT; i++) {
    const ivs_image_report *s = &info.slots[i];
    if (s->struct_size != (uint32_t)sizeof *s || s->kind != i + 1u || s->loaded != 0u || s->length_bytes != 0u ||
        s->length_units != 0u || s->unit_bits != 0u || s->base_addr != 0u || s->reserved != 0u ||
        memcmp(s->sha256, zero32, sizeof zero32) != 0) {
      ivs_test_diag("slot %u not empty", (unsigned)i);
      goto done;
    }
  }
  if (ivs_machine_destroy(m) != IVS_OK) {
    ivs_test_diag("destroy failed");
    goto done;
  }
  if (ivs_machine_info(m, &info) != IVS_E_BAD_HANDLE) {
    ivs_test_diag("info after destroy is not BAD_HANDLE");
    goto done;
  }
  rc = IVS_TEST_PASS;
done:
  free(st);
  return rc;
}

const ivs_test_case ivs_machine_create_cases[] = {
    {"machine.create.ok-info-destroy", mc_ok_info_destroy, NULL},
    {"machine.create.requirements", mc_requirements, NULL},
};
const size_t ivs_machine_create_count = sizeof ivs_machine_create_cases / sizeof ivs_machine_create_cases[0];

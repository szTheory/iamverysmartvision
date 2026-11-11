/* SPDX-License-Identifier: MIT */
/* Suite run, family info (D-06, D-09, A-02, plan 04-09): the timebase and output layout that
 * ivs_machine_info publishes after its Phase 3 fields, and the A-02 growth rule.
 *   run.info.timebase        ticks_per_cpu_cycle, cpu_cycles_per_scanline and scanlines_per_frame
 *                            equal the profile registry record, and nominal_frame_ticks equals the
 *                            literal 59736 = 262 x 57 x 4 derived in docs/time-base.md (an
 *                            independent figure, not the product the core computes).
 *   run.info.layout-none     the real profile's layout: both formats IVS_OUTPUT_FORMAT_NONE, both
 *                            sizes and reserved2 0, written over a poisoned struct.
 *   run.info.v1-size-caller  a caller sized for v1 (the Phase 3 size) with poisoned bytes after it
 *                            is accepted, gets struct_size = v1 and the same v1 bytes as a full
 *                            caller, and every byte past its size is untouched; a mid-size caller
 *                            (through video_format) likewise.
 *   run.info.short-refused   one byte below the v1 size: BAD_ARG, nothing written, the instance
 *                            storage unchanged. */
#include <stddef.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>

#include "ivs/ivs.h"
#include "ivs_test.h"
#include "machine/machine.h"
#include "run_cases.h"

#define IVS_CHECK(cond)                                                                                \
  do {                                                                                                 \
    if (!(cond)) {                                                                                     \
      ivs_test_diag("check failed at line %d: %s", __LINE__, #cond);                                   \
      return IVS_TEST_FAIL;                                                                            \
    }                                                                                                  \
  } while (0)

#define RI_CHECK(cond)                                                                                 \
  do {                                                                                                 \
    if (!(cond)) {                                                                                     \
      ivs_test_diag("check failed at line %d: %s", __LINE__, #cond);                                   \
      goto done;                                                                                       \
    }                                                                                                  \
  } while (0)

#define RI_POISON 0xA5u
/* docs/time-base.md "Nominal NTSC numbers": 262 x 57 = 14,934 CPU cycles, x 4 = 59,736 ticks. */
#define RI_NOMINAL_FRAME_TICKS 59736u

/* A caller buffer larger than ivs_info, so bytes past any caller size can be checked. */
typedef struct ri_buf {
  ivs_info info;
  uint8_t tail[32];
} ri_buf;

/* info into a fully sized struct over poison; 1 on IVS_OK. */
static int ri_full(const ivs_machine *m, ivs_info *out)
{
  memset(out, RI_POISON, sizeof *out);
  out->struct_size = (uint32_t)sizeof *out;
  return ivs_machine_info(m, out) == IVS_OK;
}

static int ri_timebase(void)
{
  ivs_ms_inst x;
  ivs_info a;
  const ivs_profile_desc *p;
  int rc = IVS_TEST_FAIL;
  IVS_CHECK(ivs_ms_powered(&x, IVS_MS_CONTENT_A, RI_POISON));
  RI_CHECK(ri_full(x.m, &a));
  RI_CHECK(a.struct_size == (uint32_t)sizeof(ivs_info));
  p = ivs_profile_get(a.profile_index);
  RI_CHECK(p != NULL);
  RI_CHECK(a.ticks_per_cpu_cycle == p->ticks_per_cpu_cycle);
  RI_CHECK(a.cpu_cycles_per_scanline == p->cpu_cycles_per_scanline);
  RI_CHECK(a.scanlines_per_frame == p->scanlines_per_frame);
  RI_CHECK(a.ticks_per_cpu_cycle == 4u && a.cpu_cycles_per_scanline == 57u && a.scanlines_per_frame == 262u);
  RI_CHECK(a.nominal_frame_ticks == (uint64_t)RI_NOMINAL_FRAME_TICKS);
  rc = IVS_TEST_PASS;
done:
  ivs_ms_free(&x);
  return rc;
}

static int ri_layout_none(void)
{
  ivs_ms_inst x;
  ivs_info a;
  int rc = IVS_TEST_FAIL;
  IVS_CHECK(ivs_ms_new(&x, RI_POISON));
  RI_CHECK(ri_full(x.m, &a));
  RI_CHECK(a.video_format == IVS_OUTPUT_FORMAT_NONE && a.video_bytes_per_frame == 0u);
  RI_CHECK(a.audio_format == IVS_OUTPUT_FORMAT_NONE && a.audio_bytes_per_frame == 0u);
  RI_CHECK(a.reserved == 0u && a.reserved2 == 0u);
  rc = IVS_TEST_PASS;
done:
  ivs_ms_free(&x);
  return rc;
}

/* A caller of size bytes: OK, struct_size = size, its bytes equal the full answer's (after the
 * size word), every byte from size on still poison. */
static int ri_sized_caller(const ivs_machine *m, const ivs_info *full, uint32_t size)
{
  ri_buf b;
  const uint8_t *got = (const uint8_t *)&b;
  const uint8_t *want = (const uint8_t *)full;
  size_t i;
  memset(&b, RI_POISON, sizeof b);
  b.info.struct_size = size;
  if (ivs_machine_info(m, &b.info) != IVS_OK) {
    ivs_test_diag("size %u refused", (unsigned)size);
    return 0;
  }
  if (b.info.struct_size != size) {
    ivs_test_diag("size %u: struct_size %u written", (unsigned)size, (unsigned)b.info.struct_size);
    return 0;
  }
  for (i = sizeof b.info.struct_size; i < (size_t)size; i++) {
    if (got[i] != want[i]) {
      ivs_test_diag("size %u: byte %u differs from a full caller", (unsigned)size, (unsigned)i);
      return 0;
    }
  }
  for (i = (size_t)size; i < sizeof b; i++) {
    if (got[i] != RI_POISON) {
      ivs_test_diag("size %u: byte %u past the caller's size written", (unsigned)size, (unsigned)i);
      return 0;
    }
  }
  return 1;
}

static int ri_v1_size_caller(void)
{
  ivs_ms_inst x;
  ivs_info full;
  int rc = IVS_TEST_FAIL;
  IVS_CHECK(ivs_ms_powered(&x, IVS_MS_CONTENT_A, RI_POISON));
  RI_CHECK(ri_full(x.m, &full));
  RI_CHECK(ri_sized_caller(x.m, &full, IVS_INFO_V1_SIZE));
  RI_CHECK(ri_sized_caller(x.m, &full, (uint32_t)offsetof(ivs_info, nominal_frame_ticks)));
  rc = IVS_TEST_PASS;
done:
  ivs_ms_free(&x);
  return rc;
}

static int ri_short_refused(void)
{
  ivs_ms_inst x;
  ri_buf b;
  ri_buf poison;
  ivs_run_snap s;
  ivs_status st;
  int rc = IVS_TEST_FAIL;
  IVS_CHECK(ivs_ms_powered(&x, IVS_MS_CONTENT_A, RI_POISON));
  memset(&s, 0, sizeof s);
  RI_CHECK(ivs_run_snap_take(&s, &x, 1));
  memset(&b, RI_POISON, sizeof b);
  b.info.struct_size = IVS_INFO_V1_SIZE - 1u;
  poison = b;
  st = ivs_machine_info(x.m, &b.info);
  RI_CHECK(st == IVS_E_BAD_ARG);
  RI_CHECK(memcmp(&b, &poison, sizeof b) == 0);
  RI_CHECK(ivs_run_snap_same(&s, &x));
  rc = IVS_TEST_PASS;
done:
  ivs_run_snap_free(&s);
  ivs_ms_free(&x);
  return rc;
}

const ivs_test_case ivs_run_info_cases[] = {
    {"run.info.timebase", ri_timebase, NULL},
    {"run.info.layout-none", ri_layout_none, NULL},
    {"run.info.v1-size-caller", ri_v1_size_caller, NULL},
    {"run.info.short-refused", ri_short_refused, NULL},
};
const size_t ivs_run_info_count = sizeof ivs_run_info_cases / sizeof ivs_run_info_cases[0];

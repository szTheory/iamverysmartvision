/* SPDX-License-Identifier: MIT */
/* Suite run, family info (D-06, D-09, A-02, plan 04-09): the timebase and output layout that
 * ivs_machine_info publishes after its Phase 3 fields, and the A-02 growth rule.
 *   run.info.timebase        ticks_per_cpu_cycle, cpu_cycles_per_scanline and scanlines_per_frame
 *                            equal the profile registry record, and nominal_frame_ticks equals the
 *                            literal 59736 = 262 x 57 x 4 derived in docs/time-base.md (an
 *                            independent figure, not the product the core computes).
 *   run.info.layout-real     the real profile's layout (flipped in plan 05-06 from layout-none, which
 *                            asserted both formats NONE and both sizes 0): video IVS_OUTPUT_FORMAT_NONE
 *                            with 0 bytes per frame, audio IVS_OUTPUT_FORMAT_PSG_LEVELS_V1 with a
 *                            nonzero size, reserved and reserved2 0, written over a poisoned struct.
 *   run.info.v1-size-caller  a caller sized for v1 (the Phase 3 size) with poisoned bytes after it
 *                            is accepted, gets struct_size = v1 and the same v1 bytes as a full
 *                            caller, and every byte past its size is untouched; a mid-size caller
 *                            (through video_format) likewise.
 *   run.info.v2-size-caller  a caller sized for v2 (256 bytes, plan 05-02) gets exactly 256 bytes and
 *                            struct_size 256; the poisoned byte at offset 256 and every byte past
 *                            it are untouched.
 *   run.info.v3-fields       a full caller (sizeof(ivs_info), 320 since plan 06-03) gets struct_size 320, the
 *                            Phase 5 tail at the pinned offsets 256, 260, 264, 268, 272, 280 and its
 *                            values: audio_sample_ticks 16, audio_sample_bytes 4, the crystal
 *                            passthrough 3579545 / 1 (plan 05-06; it replaces run.info.v3-fields-zero of
 *                            plan 05-02, which asserted every tail field 0), and (plan 05-08) the write-log
 *                            fields psg_log_record_bytes 16 and psg_log_records_per_frame 2,152.
 *   run.info.v4-fields       the Phase 6 tail (plan 06-03): offsets 288, 296, 300, 304, 308, 312, 316, sizeof
 *                            320, every field 0 while video_format is NONE.
 *   run.info.v3-size-caller  a v3-sized caller (288 bytes) gets exactly 288 bytes; the v4 tail is untouched.
 *   run.info.audio-fields    the audio layout is the literal one: format 1 (PSG_LEVELS_V1), sample
 *                            period 16 ticks, sample record 4 bytes and audio_bytes_per_frame 15,060 =
 *                            4 x (ceil(59,736 / 16) + ceil(468 / 16) + 1) = 4 x (3,734 + 30 + 1).
 *   run.info.short-refused   one byte below the v1 size: BAD_ARG, nothing written, the instance
 *                            storage unchanged.
 *   run.info.oversize-refused  one byte above IVS_STRUCT_SIZE_MAX: BAD_ARG, nothing written, the
 *                            instance storage unchanged (review IN-03); exactly the cap is
 *                            accepted. */
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

static int ri_layout_real(void)
{
  ivs_ms_inst x;
  ivs_info a;
  int rc = IVS_TEST_FAIL;
  IVS_CHECK(ivs_ms_new(&x, RI_POISON));
  RI_CHECK(ri_full(x.m, &a));
  /* Plan 06-09 (a reviewed edit; plan 06-03 pinned NONE and a zero tail): the video layout is real, one raw slot per frame. */
  RI_CHECK(a.video_format == IVS_OUTPUT_FORMAT_INDEX8_FRAMES_V1 && a.video_bytes_per_frame == 15296u);
  RI_CHECK(a.audio_format == IVS_OUTPUT_FORMAT_PSG_LEVELS_V1 && a.audio_bytes_per_frame != 0u);
  RI_CHECK(a.reserved == 0u && a.reserved2 == 0u);
  RI_CHECK(a.frame_phase_ticks == 0u && a.video_width == 159u && a.video_height == 96u);
  RI_CHECK(a.video_slot_bytes == 15296u && a.video_header_bytes == 32u && a.video_slack_frames == 1u);
  RI_CHECK(a.video_row_lines == 2u);
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

static int ri_v2_size_caller(void)
{
  ivs_ms_inst x;
  ivs_info full;
  ri_buf b;
  int rc = IVS_TEST_FAIL;
  IVS_CHECK(ivs_ms_powered(&x, IVS_MS_CONTENT_A, RI_POISON));
  RI_CHECK(ri_full(x.m, &full));
  RI_CHECK(ri_sized_caller(x.m, &full, 256u));
  /* The byte at 256 is the first of the tail: poison stays (ri_sized_caller checks every byte from
   * the caller's size on; this names the boundary). */
  memset(&b, RI_POISON, sizeof b);
  b.info.struct_size = 256u;
  RI_CHECK(ivs_machine_info(x.m, &b.info) == IVS_OK && b.info.struct_size == 256u);
  RI_CHECK(b.info.audio_sample_ticks == 0xA5A5A5A5u && b.info.crystal_hz_den == 0xA5A5A5A5A5A5A5A5u);
  rc = IVS_TEST_PASS;
done:
  ivs_ms_free(&x);
  return rc;
}

static int ri_v3_fields(void)
{
  ivs_ms_inst x;
  ivs_info a;
  int rc = IVS_TEST_FAIL;
  IVS_CHECK(offsetof(ivs_info, audio_sample_ticks) == 256u && offsetof(ivs_info, audio_sample_bytes) == 260u);
  IVS_CHECK(offsetof(ivs_info, psg_log_record_bytes) == 264u && offsetof(ivs_info, psg_log_records_per_frame) == 268u);
  IVS_CHECK(offsetof(ivs_info, crystal_hz_num) == 272u && offsetof(ivs_info, crystal_hz_den) == 280u);
  IVS_CHECK(sizeof(ivs_info) == 320u && IVS_INFO_V1_SIZE == 216u);
  IVS_CHECK(ivs_ms_powered(&x, IVS_MS_CONTENT_A, RI_POISON));
  RI_CHECK(ri_full(x.m, &a));
  RI_CHECK(a.struct_size == 320u); /* sizeof(ivs_info) is the v4 size from plan 06-03; the v3 caller is run.info.v3-size-caller */
  /* Plan 05-06: the audio fields and the crystal passthrough (literals of the data sheet's crystal and
   * of docs/time-base.md: one PSG step is 8 chip clocks of 2 master ticks). */
  RI_CHECK(a.audio_sample_ticks == 16u && a.audio_sample_bytes == 4u);
  RI_CHECK(a.crystal_hz_num == 3579545u && a.crystal_hz_den == 1u);
  /* Plan 05-08: the write-log layout is real: 16-byte records, 2,152 per frame slot (literals of the derivation
   * W_log = ceil(59,736 / 28) + ceil(468 / 28) + 1 = 2,134 + 17 + 1; psg.bounds.log-capacity-formula). */
  RI_CHECK(a.psg_log_record_bytes == 16u && a.psg_log_records_per_frame == 2152u);
  rc = IVS_TEST_PASS;
done:
  ivs_ms_free(&x);
  return rc;
}

/* run.info.v4-fields (plan 06-03, A-10): the pinned offsets and size of the Phase 6 tail; plan 06-09 (reviewed edit)
 * the real values, whatever the struct held before. */
static int ri_v4_fields(void)
{
  ivs_ms_inst x;
  ivs_info a;
  int rc = IVS_TEST_FAIL;
  IVS_CHECK(offsetof(ivs_info, frame_phase_ticks) == 288u && offsetof(ivs_info, video_width) == 296u);
  IVS_CHECK(offsetof(ivs_info, video_height) == 300u && offsetof(ivs_info, video_slot_bytes) == 304u);
  IVS_CHECK(offsetof(ivs_info, video_header_bytes) == 308u && offsetof(ivs_info, video_slack_frames) == 312u);
  IVS_CHECK(offsetof(ivs_info, video_row_lines) == 316u && sizeof(ivs_info) == 320u);
  IVS_CHECK(IVS_INFO_V1_SIZE == 216u);
  IVS_CHECK(ivs_ms_powered(&x, IVS_MS_CONTENT_A, RI_POISON));
  RI_CHECK(ri_full(x.m, &a));
  /* Plan 06-09 (a reviewed edit): the real values; the literals are the numbers of data/video/geometry.literal.txt (159 x 96, header
   * 32, slot 15,296, 2 TV lines per display row) and the owner's slack of one frame. */
  RI_CHECK(a.struct_size == 320u && a.video_format == IVS_OUTPUT_FORMAT_INDEX8_FRAMES_V1);
  RI_CHECK(a.frame_phase_ticks == 0u && a.video_width == 159u && a.video_height == 96u);
  RI_CHECK(a.video_slot_bytes == 15296u && a.video_header_bytes == 32u && a.video_slack_frames == 1u);
  RI_CHECK(a.video_row_lines == 2u);
  RI_CHECK(ri_sized_caller(x.m, &a, 320u));
  rc = IVS_TEST_PASS;
done:
  ivs_ms_free(&x);
  return rc;
}

/* run.info.v3-size-caller (plan 06-03, A-02): a caller sized for v3 (288 bytes) gets exactly 288 bytes
 * and struct_size 288; the poisoned bytes from 288 on (the v4 tail) are untouched. */
static int ri_v3_size_caller(void)
{
  ivs_ms_inst x;
  ivs_info full;
  ri_buf b;
  int rc = IVS_TEST_FAIL;
  IVS_CHECK(ivs_ms_powered(&x, IVS_MS_CONTENT_A, RI_POISON));
  RI_CHECK(ri_full(x.m, &full));
  RI_CHECK(ri_sized_caller(x.m, &full, 288u));
  memset(&b, RI_POISON, sizeof b);
  b.info.struct_size = 288u;
  RI_CHECK(ivs_machine_info(x.m, &b.info) == IVS_OK && b.info.struct_size == 288u);
  RI_CHECK(b.info.crystal_hz_num == 3579545u && b.info.crystal_hz_den == 1u);
  RI_CHECK(b.info.frame_phase_ticks == 0xA5A5A5A5A5A5A5A5u && b.info.video_width == 0xA5A5A5A5u);
  RI_CHECK(b.info.video_row_lines == 0xA5A5A5A5u);
  rc = IVS_TEST_PASS;
done:
  ivs_ms_free(&x);
  return rc;
}

static int ri_audio_fields(void)
{
  ivs_ms_inst x;
  ivs_info a;
  int rc = IVS_TEST_FAIL;
  IVS_CHECK(ivs_ms_powered(&x, IVS_MS_CONTENT_A, RI_POISON));
  RI_CHECK(ri_full(x.m, &a));
  RI_CHECK(a.audio_format == IVS_OUTPUT_FORMAT_PSG_LEVELS_V1);
  RI_CHECK(a.audio_sample_ticks == 16u && a.audio_sample_bytes == 4u);
  /* 4 x (3,734 samples of a nominal frame + 31 slack) with U = 468 ticks: see the header of this file. */
  RI_CHECK(a.audio_bytes_per_frame == 15060u);
  RI_CHECK(a.nominal_frame_ticks == RI_NOMINAL_FRAME_TICKS);
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

static int ri_oversize_refused(void)
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
  b.info.struct_size = IVS_STRUCT_SIZE_MAX + 1u;
  poison = b;
  st = ivs_machine_info(x.m, &b.info);
  RI_CHECK(st == IVS_E_BAD_ARG);
  RI_CHECK(memcmp(&b, &poison, sizeof b) == 0);
  RI_CHECK(ivs_run_snap_same(&s, &x));
  /* Exactly the cap: accepted, the core's own size written. */
  b.info.struct_size = IVS_STRUCT_SIZE_MAX;
  RI_CHECK(ivs_machine_info(x.m, &b.info) == IVS_OK && b.info.struct_size == (uint32_t)sizeof(ivs_info));
  rc = IVS_TEST_PASS;
done:
  ivs_run_snap_free(&s);
  ivs_ms_free(&x);
  return rc;
}

const ivs_test_case ivs_run_info_cases[] = {
    {"run.info.timebase", ri_timebase, NULL},
    {"run.info.layout-real", ri_layout_real, NULL},
    {"run.info.v1-size-caller", ri_v1_size_caller, NULL},
    {"run.info.v2-size-caller", ri_v2_size_caller, NULL},
    {"run.info.v3-fields", ri_v3_fields, NULL},
    {"run.info.v4-fields", ri_v4_fields, NULL},
    {"run.info.v3-size-caller", ri_v3_size_caller, NULL},
    {"run.info.audio-fields", ri_audio_fields, NULL},
    {"run.info.short-refused", ri_short_refused, NULL},
    {"run.info.oversize-refused", ri_oversize_refused, NULL},
};
const size_t ivs_run_info_count = sizeof ivs_run_info_cases / sizeof ivs_run_info_cases[0];

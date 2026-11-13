/* SPDX-License-Identifier: MIT */
/* Suite run, family struct (D-19, A-02, A-04, plan 05-02): the pinned v1 minimums of the grown
 * run request (v2 appends psg_log and psg_log_capacity) and run report (v2 appends
 * audio_first_index, psg_log_bytes_written, psg_log_first_index).
 *   run.struct.request-v1-caller  a request whose struct_size is IVS_RUN_REQUEST_V1_SIZE inside a
 *                                 larger poisoned (0xA5) buffer, with a report of struct_size 112
 *                                 inside a poisoned larger buffer, runs as the v2-sized form of
 *                                 the same call: the same status, report prefix and storage; the
 *                                 report bytes past offset 112 are untouched. A core that read the
 *                                 appended request fields would see the poison as a pointer and a
 *                                 capacity.
 *   run.struct.report-v1-caller   the report half, accepted and refused: success writes exactly 112
 *                                 bytes and sets struct_size 112, and a BAD_ARG refusal writes only
 *                                 struct_size and refusal_reason inside the 112-byte prefix.
 *   run.struct.request-v2-fields  sizeof and offsetof of the appended request fields (64-bit
 *                                 pointers only: visible skip elsewhere), and a v2-sized request
 *                                 with a NULL psg_log and capacity 0 equals the v1 call.
 *   run.struct.report-v2-fields   sizeof and offsetof of the appended report fields (pointer-free,
 *                                 every ABI), and a full caller gets struct_size 160 (the v3 size since plan 06-03) with the
 *                                 appended fields written.
 *   run.struct.report-v3-offsets  offsetof 136, 144, 152 and sizeof 160 of the v3 report (plan 06-03).
 *   run.struct.report-v2-caller   a 136-byte caller gets the v2 fields only; every byte from 136 on stays
 *                                 poison, on success and on a refusal.
 *   run.struct.report-v3-caller   a 160-byte caller gets the v3 tail (0); a 144-byte caller only the first
 *                                 appended field.
 *   run.struct.env-size           sizeof(ivs_run_env) is 32 with video_slack_frames at offset 28.
 *   run.struct.tail-nonzero       a caller larger than the core's own size with a nonzero tail is
 *                                 refused RUN_TAIL, with a zero tail accepted; a caller between v1
 *                                 and v2 sizes is valid; one byte below either v1 minimum is
 *                                 refused RUN_STRUCT_SIZE.
 * Stand-in content A of tests/machine/machine_standin.c (no rights-bearing byte). */
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

#define RS_CHECK(cond)                                                                                 \
  do {                                                                                                 \
    if (!(cond)) {                                                                                     \
      ivs_test_diag("check failed at line %d: %s", __LINE__, #cond);                                   \
      goto done;                                                                                       \
    }                                                                                                  \
  } while (0)

#define RS_POISON 0xA5u
#define RS_SPAN 5000u
#define RS_BUF 256u /* caller buffers larger than either struct */

/* A request in a poisoned buffer: the first size bytes are the caller's struct (a copy of the
 * prefix of t), the rest stays poison. */
typedef union rs_req_buf {
  ivs_run_request q;
  uint8_t b[RS_BUF];
} rs_req_buf;

typedef union rs_rep_buf {
  ivs_run_report r;
  uint8_t b[RS_BUF];
} rs_rep_buf;

/* A request caller of size bytes holding the fields of a zeroed v2 request with target target
 * and, when the size covers them, a NULL psg_log with capacity 0; every byte past size poison. */
static void rs_req(rs_req_buf *u, uint32_t size, uint64_t target)
{
  ivs_run_request t;
  memset(&t, 0, sizeof t);
  t.struct_size = size;
  t.target_ticks = target;
  memset(u, RS_POISON, sizeof *u);
  memcpy(u->b, &t, size < (uint32_t)sizeof t ? size : (uint32_t)sizeof t);
}

static void rs_rep(rs_rep_buf *u, uint32_t size)
{
  memset(u, RS_POISON, sizeof *u);
  u->r.struct_size = size;
}

/* Machine time of m, or UINT64_MAX when info refuses. */
static uint64_t rs_now(const ivs_machine *m)
{
  ivs_info info;
  memset(&info, 0, sizeof info);
  info.struct_size = IVS_INFO_V1_SIZE;
  if (ivs_machine_info(m, &info) != IVS_OK) {
    return UINT64_MAX;
  }
  return info.time_ticks;
}

/* 1 when x and y hold equal storage and equal composite and CPU digests. */
static int rs_same_machine(const ivs_ms_inst *x, const ivs_ms_inst *y)
{
  uint8_t cx[32];
  uint8_t px[32];
  uint8_t cy[32];
  uint8_t py[32];
  if (memcmp(x->storage, y->storage, IVS_MACHINE_BYTES) != 0) {
    ivs_test_diag("instance storage differs");
    return 0;
  }
  if (!ivs_ms_digests(x->m, cx, px) || !ivs_ms_digests(y->m, cy, py)) {
    return 0;
  }
  if (memcmp(cx, cy, 32u) != 0 || memcmp(px, py, 32u) != 0) {
    ivs_test_diag("digests differ");
    return 0;
  }
  return 1;
}

/* Two powered twins and their now (equal). */
static int rs_twins(ivs_ms_inst *x, ivs_ms_inst *y, uint64_t *now)
{
  x->storage = NULL;
  x->m = NULL;
  y->storage = NULL;
  y->m = NULL;
  if (!ivs_ms_powered(x, IVS_MS_CONTENT_A, RS_POISON) || !ivs_ms_powered(y, IVS_MS_CONTENT_A, RS_POISON)) {
    return 0;
  }
  *now = rs_now(x->m);
  return *now != UINT64_MAX && *now == rs_now(y->m);
}

/* ---- run.struct.request-v1-caller ---- */

static int rs_request_v1_caller(void)
{
  ivs_ms_inst x;
  ivs_ms_inst y;
  rs_req_buf qx;
  rs_req_buf qy;
  rs_rep_buf rx;
  rs_rep_buf ry;
  uint64_t now;
  uint32_t i;
  size_t k;
  int rc = IVS_TEST_FAIL;
  RS_CHECK(rs_twins(&x, &y, &now));
  for (i = 0u; i < 2u; i++) {
    ivs_status sx;
    ivs_status sy;
    /* x: the v1-sized caller (the appended fields are poison in its buffer); y: the v2-sized form. */
    rs_req(&qx, IVS_RUN_REQUEST_V1_SIZE, now + RS_SPAN);
    rs_req(&qy, (uint32_t)sizeof(ivs_run_request), now + RS_SPAN);
    rs_rep(&rx, IVS_RUN_REPORT_V1_SIZE);
    rs_rep(&ry, (uint32_t)sizeof(ivs_run_report));
    sx = ivs_machine_run(x.m, &qx.q, &rx.r);
    sy = ivs_machine_run(y.m, &qy.q, &ry.r);
    if (sx != IVS_OK || sy != IVS_OK) {
      ivs_test_diag("status v1 %s v2 %s (reason %u)", ivs_status_name(sx), ivs_status_name(sy),
                    (unsigned)rx.r.refusal_reason);
      goto done;
    }
    RS_CHECK(rx.r.struct_size == IVS_RUN_REPORT_V1_SIZE && rx.r.stop_reason == IVS_STOP_TARGET);
    RS_CHECK(rx.r.stop_reason == ry.r.stop_reason && rx.r.units_executed > 0u);
    RS_CHECK(memcmp(rx.b + 4u, ry.b + 4u, IVS_RUN_REPORT_V1_SIZE - 4u) == 0);
    for (k = IVS_RUN_REPORT_V1_SIZE; k < RS_BUF; k++) {
      if (rx.b[k] != RS_POISON) {
        ivs_test_diag("v1 report byte %u written", (unsigned)k);
        goto done;
      }
    }
    RS_CHECK(rs_same_machine(&x, &y));
    now = rx.r.now_ticks;
  }
  rc = IVS_TEST_PASS;
done:
  ivs_ms_free(&y);
  ivs_ms_free(&x);
  return rc;
}

/* ---- run.struct.report-v1-caller ---- */

static int rs_report_v1_caller(void)
{
  ivs_ms_inst x;
  ivs_ms_inst y;
  rs_req_buf q;
  rs_rep_buf rx;
  rs_rep_buf ry;
  ivs_status st;
  uint64_t now;
  size_t i;
  int rc = IVS_TEST_FAIL;
  RS_CHECK(rs_twins(&x, &y, &now));
  /* Success: the core writes exactly the 112-byte prefix and says so. */
  rs_req(&q, (uint32_t)sizeof(ivs_run_request), now + RS_SPAN);
  rs_rep(&rx, IVS_RUN_REPORT_V1_SIZE);
  rs_rep(&ry, (uint32_t)sizeof(ivs_run_report));
  st = ivs_machine_run(x.m, &q.q, &rx.r);
  RS_CHECK(st == IVS_OK);
  st = ivs_machine_run(y.m, &q.q, &ry.r);
  RS_CHECK(st == IVS_OK);
  RS_CHECK(rx.r.struct_size == IVS_RUN_REPORT_V1_SIZE && ry.r.struct_size == (uint32_t)sizeof(ivs_run_report));
  RS_CHECK(memcmp(rx.b + 4u, ry.b + 4u, IVS_RUN_REPORT_V1_SIZE - 4u) == 0);
  for (i = IVS_RUN_REPORT_V1_SIZE; i < RS_BUF; i++) {
    if (rx.b[i] != RS_POISON) {
      ivs_test_diag("v1 report byte %u written by a successful run", (unsigned)i);
      goto done;
    }
  }
  /* Refusal (BAD_ARG, a NULL video buffer with a capacity): only struct_size and refusal_reason
   * are written, inside the 112-byte prefix, and the instance is unchanged. */
  rs_req(&q, (uint32_t)sizeof(ivs_run_request), now + RS_SPAN);
  q.q.video = NULL;
  q.q.video_capacity = 8u;
  rs_rep(&rx, IVS_RUN_REPORT_V1_SIZE);
  {
    ivs_run_snap s;
    RS_CHECK(ivs_run_snap_take(&s, &x, 1));
    st = ivs_machine_run(x.m, &q.q, &rx.r);
    if (st != IVS_E_BAD_ARG || rx.r.refusal_reason != IVS_REASON_RUN_BUFFER_NULL ||
        rx.r.struct_size != IVS_RUN_REPORT_V1_SIZE || !ivs_run_snap_same(&s, &x)) {
      ivs_test_diag("refusal: status %s reason %u struct_size %u", ivs_status_name(st),
                    (unsigned)rx.r.refusal_reason, (unsigned)rx.r.struct_size);
      ivs_run_snap_free(&s);
      goto done;
    }
    ivs_run_snap_free(&s);
  }
  for (i = 4u; i < RS_BUF; i++) {
    if (i >= offsetof(ivs_run_report, refusal_reason) && i < offsetof(ivs_run_report, refusal_reason) + 4u) {
      continue;
    }
    if (rx.b[i] != RS_POISON) {
      ivs_test_diag("report byte %u written by a refusal", (unsigned)i);
      goto done;
    }
  }
  rc = IVS_TEST_PASS;
done:
  ivs_ms_free(&y);
  ivs_ms_free(&x);
  return rc;
}

/* ---- run.struct.request-v2-fields ---- */

#if UINTPTR_MAX == 0xFFFFFFFFFFFFFFFFu
#define RS_LP64_SKIP NULL
#else
#define RS_LP64_SKIP "ivs_run_request holds pointers: its v2 layout is pinned for 64-bit pointers only"
#endif

static int rs_request_v2_fields(void)
{
  ivs_ms_inst x;
  ivs_ms_inst y;
  rs_req_buf qx;
  rs_req_buf qy;
  rs_rep_buf rx;
  rs_rep_buf ry;
  uint64_t now;
  int rc = IVS_TEST_FAIL;
  IVS_CHECK(sizeof(void *) == 8u);
  IVS_CHECK(IVS_RUN_REQUEST_V1_SIZE == 64u && sizeof(ivs_run_request) == 80u);
  IVS_CHECK(offsetof(ivs_run_request, reserved_in) == 60u);
  IVS_CHECK(offsetof(ivs_run_request, psg_log) == 64u && offsetof(ivs_run_request, psg_log_capacity) == 72u);
  RS_CHECK(rs_twins(&x, &y, &now));
  /* A v2-sized request with no log channel equals the v1-sized call. */
  rs_req(&qx, IVS_RUN_REQUEST_V1_SIZE, now + RS_SPAN);
  rs_req(&qy, (uint32_t)sizeof(ivs_run_request), now + RS_SPAN);
  RS_CHECK(qy.q.psg_log == NULL && qy.q.psg_log_capacity == 0u);
  rs_rep(&rx, (uint32_t)sizeof(ivs_run_report));
  rs_rep(&ry, (uint32_t)sizeof(ivs_run_report));
  RS_CHECK(ivs_machine_run(x.m, &qx.q, &rx.r) == IVS_OK);
  RS_CHECK(ivs_machine_run(y.m, &qy.q, &ry.r) == IVS_OK);
  RS_CHECK(memcmp(&rx.r, &ry.r, sizeof(ivs_run_report)) == 0);
  RS_CHECK(rs_same_machine(&x, &y));
  rc = IVS_TEST_PASS;
done:
  ivs_ms_free(&y);
  ivs_ms_free(&x);
  return rc;
}

/* ---- run.struct.report-v2-fields ---- */

static int rs_report_v2_fields(void)
{
  ivs_ms_inst x;
  rs_req_buf q;
  rs_rep_buf r;
  uint64_t now;
  int rc = IVS_TEST_FAIL;
  IVS_CHECK(IVS_RUN_REPORT_V1_SIZE == 112u && offsetof(ivs_run_report, video_first_index) == 136u);
  IVS_CHECK(sizeof(ivs_run_report) == 160u); /* v3 from plan 06-03; the v2 end is the offset of video_first_index */
  IVS_CHECK(offsetof(ivs_run_report, controller_now) == 96u);
  IVS_CHECK(offsetof(ivs_run_report, audio_first_index) == 112u);
  IVS_CHECK(offsetof(ivs_run_report, psg_log_bytes_written) == 120u);
  IVS_CHECK(offsetof(ivs_run_report, psg_log_first_index) == 128u);
  IVS_CHECK(IVS_RUN_REPORT_V1_SIZE + 24u == offsetof(ivs_run_report, video_first_index));
  IVS_CHECK(ivs_ms_powered(&x, IVS_MS_CONTENT_A, RS_POISON));
  now = rs_now(x.m);
  RS_CHECK(now != UINT64_MAX);
  rs_req(&q, (uint32_t)sizeof(ivs_run_request), now + RS_SPAN);
  rs_rep(&r, (uint32_t)sizeof(ivs_run_report));
  RS_CHECK(ivs_machine_run(x.m, &q.q, &r.r) == IVS_OK);
  RS_CHECK(r.r.struct_size == (uint32_t)sizeof(ivs_run_report));
  /* Plan 05-06: the first call starts at sample 0 (audio_first_index, the samples emitted before the
   * call), and the request has no audio buffer, so nothing is delivered (audio_bytes_written 0). Plan 05-08:
   * the request has no log buffer and stand-in program A never writes the PSG, so no record exists
   * (psg_log_bytes_written 0 and psg_log_first_index 0, the records before the call). Plan 05-02 asserted all
   * three appended fields 0; the case below (RS_LOG) gives them real values. */
  RS_CHECK(r.r.audio_first_index == 0u && r.r.audio_bytes_written == 0u);
  RS_CHECK(r.r.psg_log_bytes_written == 0u && r.r.psg_log_first_index == 0u);
  /* A report between the v1 size and its own: only the covered bytes are written. The second call
   * enters at the first call's end, so its audio_first_index is the literal floor(now / 16) (one sample
   * per 16 ticks, absolute grid from reset). */
  now = r.r.now_ticks;
  RS_CHECK(now >= RS_SPAN);
  rs_req(&q, (uint32_t)sizeof(ivs_run_request), now + RS_SPAN);
  rs_rep(&r, 120u);
  RS_CHECK(ivs_machine_run(x.m, &q.q, &r.r) == IVS_OK);
  RS_CHECK(r.r.struct_size == 120u);
  RS_CHECK(r.r.audio_first_index == now / 16u);
  RS_CHECK(r.b[120] == RS_POISON && r.b[135] == RS_POISON);
  /* RS_LOG (plan 05-08): stand-in program C writes $01F4 once (a record of 16 bytes). A first call that ends after
   * the write reports psg_log_bytes_written 16 and psg_log_first_index 0 (no record before the call); the next call,
   * which makes no further write, reports 0 bytes and psg_log_first_index 1 (one record before it). */
  {
    ivs_ms_inst c;
    ivs_info info;
    uint8_t *log;
    uint64_t cap;
    c.storage = NULL;
    c.m = NULL;
    memset(&info, 0, sizeof info);
    info.struct_size = (uint32_t)sizeof info;
    RS_CHECK(ivs_ms_powered_prog(&c, IVS_MS_PROG_C, RS_POISON));
    RS_CHECK(ivs_machine_info(c.m, &info) == IVS_OK && info.psg_log_record_bytes == 16u);
    cap = (uint64_t)info.psg_log_record_bytes * info.psg_log_records_per_frame * 2u;
    log = malloc((size_t)cap);
    if (log == NULL) {
      ivs_ms_free(&c);
      goto done;
    }
    rs_req(&q, (uint32_t)sizeof(ivs_run_request), 200u);
    q.q.psg_log = log;
    q.q.psg_log_capacity = cap;
    rs_rep(&r, (uint32_t)sizeof(ivs_run_report));
    if (ivs_machine_run(c.m, &q.q, &r.r) != IVS_OK || r.r.psg_log_bytes_written != 16u ||
        r.r.psg_log_first_index != 0u) {
      ivs_test_diag("first call: log bytes %llu, first index %llu", (unsigned long long)r.r.psg_log_bytes_written,
                    (unsigned long long)r.r.psg_log_first_index);
      free(log);
      ivs_ms_free(&c);
      goto done;
    }
    rs_req(&q, (uint32_t)sizeof(ivs_run_request), 400u);
    q.q.psg_log = log;
    q.q.psg_log_capacity = cap;
    rs_rep(&r, (uint32_t)sizeof(ivs_run_report));
    if (ivs_machine_run(c.m, &q.q, &r.r) != IVS_OK || r.r.psg_log_bytes_written != 0u ||
        r.r.psg_log_first_index != 1u) {
      ivs_test_diag("second call: log bytes %llu, first index %llu", (unsigned long long)r.r.psg_log_bytes_written,
                    (unsigned long long)r.r.psg_log_first_index);
      free(log);
      ivs_ms_free(&c);
      goto done;
    }
    free(log);
    ivs_ms_free(&c);
  }
  rc = IVS_TEST_PASS;
done:
  ivs_ms_free(&x);
  return rc;
}

/* ---- run.struct.tail-nonzero ---- */

static int rs_tail_nonzero(void)
{
  ivs_ms_inst x;
  ivs_ms_inst y;
  rs_req_buf q;
  rs_rep_buf r;
  rs_rep_buf r0;
  ivs_run_snap s;
  ivs_status st;
  uint64_t now;
  uint32_t big = (uint32_t)sizeof(ivs_run_request) + 8u;
  int rc = IVS_TEST_FAIL;
  s.copy = NULL;
  RS_CHECK(rs_twins(&x, &y, &now));
  /* Larger than the core's size with a nonzero tail: UNSUPPORTED RUN_TAIL, nothing changes. */
  rs_req(&q, big, now + RS_SPAN);
  memset(q.b + sizeof(ivs_run_request), 0, 8u);
  q.b[sizeof(ivs_run_request) + 3u] = 1u;
  rs_rep(&r, (uint32_t)sizeof(ivs_run_report));
  RS_CHECK(ivs_run_snap_take(&s, &x, 1));
  st = ivs_machine_run(x.m, &q.q, &r.r);
  if (st != IVS_E_UNSUPPORTED || r.r.refusal_reason != IVS_REASON_RUN_TAIL || !ivs_run_snap_same(&s, &x)) {
    ivs_test_diag("nonzero tail: status %s reason %u", ivs_status_name(st), (unsigned)r.r.refusal_reason);
    goto done;
  }
  /* The same size with a zero tail is accepted. */
  q.b[sizeof(ivs_run_request) + 3u] = 0u;
  rs_rep(&r, (uint32_t)sizeof(ivs_run_report));
  RS_CHECK(ivs_machine_run(x.m, &q.q, &r.r) == IVS_OK && r.r.stop_reason == IVS_STOP_TARGET);
  /* Between the v1 and v2 sizes: valid, the partial appended bytes are not read (y is the v1 call). */
  rs_req(&q, 72u, rs_now(y.m) + RS_SPAN);
  rs_rep(&r0, IVS_RUN_REPORT_V1_SIZE);
  RS_CHECK(ivs_machine_run(y.m, &q.q, &r0.r) == IVS_OK && r0.r.stop_reason == IVS_STOP_TARGET);
  RS_CHECK(r0.r.now_ticks == r.r.now_ticks);
  /* One byte below either v1 minimum: RUN_STRUCT_SIZE; a short report: nothing written. */
  ivs_run_snap_free(&s);
  RS_CHECK(ivs_run_snap_take(&s, &y, 1));
  rs_req(&q, IVS_RUN_REQUEST_V1_SIZE - 1u, rs_now(y.m) + RS_SPAN);
  rs_rep(&r, IVS_RUN_REPORT_V1_SIZE);
  st = ivs_machine_run(y.m, &q.q, &r.r);
  if (st != IVS_E_BAD_ARG || r.r.refusal_reason != IVS_REASON_RUN_STRUCT_SIZE || !ivs_run_snap_same(&s, &y)) {
    ivs_test_diag("short request: status %s reason %u", ivs_status_name(st), (unsigned)r.r.refusal_reason);
    goto done;
  }
  rs_req(&q, IVS_RUN_REQUEST_V1_SIZE, rs_now(y.m) + RS_SPAN);
  rs_rep(&r, IVS_RUN_REPORT_V1_SIZE - 1u);
  st = ivs_machine_run(y.m, &q.q, &r.r);
  RS_CHECK(st == IVS_E_BAD_ARG && r.r.struct_size == IVS_RUN_REPORT_V1_SIZE - 1u);
  RS_CHECK(r.r.refusal_reason == RS_POISON * 0x01010101u && r.b[RS_BUF - 1u] == RS_POISON);
  RS_CHECK(ivs_run_snap_same(&s, &y));
  rc = IVS_TEST_PASS;
done:
  ivs_run_snap_free(&s);
  ivs_ms_free(&y);
  ivs_ms_free(&x);
  return rc;
}

/* ---- run.struct.report-v3-offsets ---- */

static int rs_report_v3_offsets(void)
{
  IVS_CHECK(offsetof(ivs_run_report, psg_log_first_index) == 128u);
  IVS_CHECK(offsetof(ivs_run_report, video_first_index) == 136u);
  IVS_CHECK(offsetof(ivs_run_report, last_frame_end_ticks) == 144u);
  IVS_CHECK(offsetof(ivs_run_report, next_frame_end_ticks) == 152u);
  IVS_CHECK(sizeof(ivs_run_report) == 160u && IVS_RUN_REPORT_V1_SIZE == 112u);
  IVS_CHECK(sizeof(ivs_run_report) % 8u == 0u && sizeof(ivs_run_report) <= IVS_STRUCT_SIZE_MAX);
  return IVS_TEST_PASS;
}

/* ---- run.struct.report-v2-caller ---- */

/* A caller whose struct_size is 136 (the v2 size) in a poisoned larger buffer gets the v2 fields exactly
 * as a v3-sized caller does, and every byte from 136 on stays poison, on success and on a refusal. */
static int rs_report_v2_caller(void)
{
  ivs_ms_inst x;
  ivs_ms_inst y;
  rs_req_buf q;
  rs_rep_buf rx;
  rs_rep_buf ry;
  ivs_run_snap s;
  ivs_status st;
  uint64_t now;
  size_t i;
  int rc = IVS_TEST_FAIL;
  s.copy = NULL;
  RS_CHECK(rs_twins(&x, &y, &now));
  rs_req(&q, (uint32_t)sizeof(ivs_run_request), now + RS_SPAN);
  rs_rep(&rx, 136u);
  rs_rep(&ry, (uint32_t)sizeof(ivs_run_report));
  RS_CHECK(ivs_machine_run(x.m, &q.q, &rx.r) == IVS_OK);
  RS_CHECK(ivs_machine_run(y.m, &q.q, &ry.r) == IVS_OK);
  RS_CHECK(rx.r.struct_size == 136u && ry.r.struct_size == (uint32_t)sizeof(ivs_run_report));
  RS_CHECK(rx.r.stop_reason == IVS_STOP_TARGET && rx.r.units_executed > 0u);
  RS_CHECK(memcmp(rx.b + 4u, ry.b + 4u, 136u - 4u) == 0);
  for (i = 136u; i < RS_BUF; i++) {
    if (rx.b[i] != RS_POISON) {
      ivs_test_diag("v2 report byte %u written by a successful run", (unsigned)i);
      goto done;
    }
  }
  /* A refusal (a NULL video buffer with a capacity) writes only struct_size and refusal_reason. */
  rs_req(&q, (uint32_t)sizeof(ivs_run_request), now + RS_SPAN);
  q.q.video = NULL;
  q.q.video_capacity = 8u;
  rs_rep(&rx, 136u);
  RS_CHECK(ivs_run_snap_take(&s, &x, 1));
  st = ivs_machine_run(x.m, &q.q, &rx.r);
  RS_CHECK(st == IVS_E_BAD_ARG && rx.r.refusal_reason == IVS_REASON_RUN_BUFFER_NULL && rx.r.struct_size == 136u);
  RS_CHECK(ivs_run_snap_same(&s, &x));
  for (i = 136u; i < RS_BUF; i++) {
    if (rx.b[i] != RS_POISON) {
      ivs_test_diag("v2 report byte %u written by a refusal", (unsigned)i);
      goto done;
    }
  }
  rc = IVS_TEST_PASS;
done:
  ivs_run_snap_free(&s);
  ivs_ms_free(&y);
  ivs_ms_free(&x);
  return rc;
}

/* ---- run.struct.report-v3-caller ---- */

/* A v3-sized caller gets struct_size 160 and the three appended fields written (the values of a live
 * timeline since plan 06-09; they were poison before the call); a caller between 136 and 160 gets only the fields its size
 * covers; every byte past the caller's size stays poison. */
static int rs_report_v3_caller(void)
{
  ivs_ms_inst x;
  rs_req_buf q;
  rs_rep_buf r;
  uint64_t now;
  size_t i;
  int rc = IVS_TEST_FAIL;
  IVS_CHECK(ivs_ms_powered(&x, IVS_MS_CONTENT_A, RS_POISON));
  now = rs_now(x.m);
  RS_CHECK(now != UINT64_MAX);
  rs_req(&q, (uint32_t)sizeof(ivs_run_request), now + RS_SPAN);
  rs_rep(&r, 160u);
  RS_CHECK(ivs_machine_run(x.m, &q.q, &r.r) == IVS_OK);
  RS_CHECK(r.r.struct_size == 160u && r.r.stop_reason == IVS_STOP_TARGET);
  /* Plan 06-09 (a reviewed edit; the tail was 0 until the frame events were published): no frame ended in this call, so the first index and
   * the last frame end are 0 and the end of the frame in progress is 59,736. */
  RS_CHECK(r.r.video_first_index == 0u && r.r.last_frame_end_ticks == 0u && r.r.next_frame_end_ticks == 59736u);
  for (i = 160u; i < RS_BUF; i++) {
    if (r.b[i] != RS_POISON) {
      ivs_test_diag("v3 report byte %u past the caller's size written", (unsigned)i);
      goto done;
    }
  }
  /* 144 bytes: video_first_index (offset 136) only; last_frame_end_ticks onward stay poison. */
  now = r.r.now_ticks;
  rs_req(&q, (uint32_t)sizeof(ivs_run_request), now + RS_SPAN);
  rs_rep(&r, 144u);
  RS_CHECK(ivs_machine_run(x.m, &q.q, &r.r) == IVS_OK);
  RS_CHECK(r.r.struct_size == 144u && r.r.video_first_index == 0u);
  RS_CHECK(r.b[144] == RS_POISON && r.b[159] == RS_POISON);
  rc = IVS_TEST_PASS;
done:
  ivs_ms_free(&x);
  return rc;
}

/* ---- run.struct.env-size ---- */

/* The internal env seam keeps 32 bytes while video_slack_frames takes the former reserved word (plan
 * 06-03); the profile's own env leaves the word 0 (no video layout yet). */
static int rs_env_size(void)
{
  ivs_run_env env;
  const ivs_profile_desc *p = ivs_profile_get(0u);
  IVS_CHECK(sizeof(ivs_run_env) == 32u);
  IVS_CHECK(offsetof(ivs_run_env, nominal_frame_ticks) == 16u && offsetof(ivs_run_env, psg_log_bytes_per_frame) == 24u);
  IVS_CHECK(offsetof(ivs_run_env, video_slack_frames) == 28u);
  IVS_CHECK(p != NULL);
  memset(&env, 0xA5, sizeof env);
  ivs_machine_profile_env(p, &env);
  IVS_CHECK(env.video_slack_frames == 1u); /* plan 06-09 (a reviewed edit): the profile's env carries the owner's slack of one frame */
  return IVS_TEST_PASS;
}

/* run.struct.report-v3-live (plan 06-09): the three v3 fields at their pinned offsets 136, 144 and 152 carry the live values after a flagged
 * run to the first frame end (index 0, last end 59,736, next end 119,472), read as raw bytes at the offsets, and a v2-sized caller gets none
 * of them (its bytes from 136 stay poison). */
static int rs_report_v3_live(void)
{
  ivs_ms_inst x;
  ivs_run_request q;
  rs_rep_buf r;
  uint64_t v[3];
  int rc = IVS_TEST_FAIL;
  IVS_CHECK(ivs_ms_powered(&x, IVS_MS_CONTENT_A, RS_POISON));
  memset(&q, 0, sizeof q);
  q.struct_size = (uint32_t)sizeof q;
  q.flags = IVS_RUN_F_STOP_ON_FRAME;
  q.target_ticks = IVS_RUN_TARGET_MAX;
  rs_rep(&r, 136u); /* a v2-sized caller */
  RS_CHECK(ivs_machine_run(x.m, &q, &r.r) == IVS_OK && r.r.stop_reason == IVS_STOP_FRAME);
  RS_CHECK(r.b[136] == RS_POISON && r.b[152] == RS_POISON && r.b[159] == RS_POISON);
  q.target_ticks = IVS_RUN_TARGET_MAX;
  rs_rep(&r, 160u);
  RS_CHECK(ivs_machine_run(x.m, &q, &r.r) == IVS_OK && r.r.stop_reason == IVS_STOP_FRAME);
  memcpy(&v[0], r.b + 136u, 8u);
  memcpy(&v[1], r.b + 144u, 8u);
  memcpy(&v[2], r.b + 152u, 8u);
  RS_CHECK(v[0] == 1u && v[1] == 2u * 59736u && v[2] == 3u * 59736u); /* the first call completed frame 0, this one frame 1 */
  rc = IVS_TEST_PASS;
done:
  ivs_ms_free(&x);
  return rc;
}

const ivs_test_case ivs_run_struct_cases[] = {
    {"run.struct.request-v1-caller", rs_request_v1_caller, NULL},
    {"run.struct.report-v1-caller", rs_report_v1_caller, NULL},
    {"run.struct.request-v2-fields", rs_request_v2_fields, RS_LP64_SKIP},
    {"run.struct.report-v2-fields", rs_report_v2_fields, NULL},
    {"run.struct.tail-nonzero", rs_tail_nonzero, NULL},
    {"run.struct.report-v3-offsets", rs_report_v3_offsets, NULL},
    {"run.struct.report-v2-caller", rs_report_v2_caller, NULL},
    {"run.struct.report-v3-caller", rs_report_v3_caller, NULL},
    {"run.struct.env-size", rs_env_size, NULL},
    {"run.struct.report-v3-live", rs_report_v3_live, NULL},
};

const size_t ivs_run_struct_count = sizeof ivs_run_struct_cases / sizeof ivs_run_struct_cases[0];

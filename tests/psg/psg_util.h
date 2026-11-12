/* SPDX-License-Identifier: MIT */
/* Shared helpers of the psg suite's audio families (plan 05-06): the module-level rig (a PSG state, its audio
 * row and a register file driven through ivs_psg_write and ivs_psg_catch_up with a test sink), a bounded-run
 * wrapper over the public run call, and the profile lookup. Static inline: each test file includes it. */
#ifndef IVS_PSG_UTIL_H
#define IVS_PSG_UTIL_H

#include <stdint.h>
#include <stdlib.h>
#include <string.h>

#include "ivs/ivs.h"
#include "ivs/ivs_run.h"
#include "ivs_test.h"
#include "machine/machine.h"
#include "machine_cases.h"
#include "memmap/memsys.h"
#include "psg/psg.h"

typedef struct pu_rig {
  ivs_psg_state s;
  ivs_psg_obs obs;
  uint16_t regs[IVS_PSG_REG_COUNT];
  const ivs_psg_params *p;
} pu_rig;

static inline const ivs_psg_params *pu_params(void)
{
  uint32_t p;
  for (p = 0u; p < ivs_profile_count(); p++) {
    if (strcmp(ivs_profile_get(p)->id, IVS_MS_PROFILE) == 0) {
      return ivs_psg_params_get(p);
    }
  }
  return NULL;
}

/* A rig in the power-on state: zero registers and audio row, step 1 next. 0 when the profile is missing. */
static inline int pu_init(pu_rig *r)
{
  memset(r, 0, sizeof *r);
  r->p = pu_params();
  ivs_psg_state_reset(r->p, &r->s);
  return r->p != NULL;
}

static inline void pu_write(pu_rig *r, ivs_psg_sink *k, uint64_t stamp, uint32_t reg, uint32_t value)
{
  ivs_psg_write(r->p, &r->s, r->regs, &r->obs, k, stamp, reg, value);
}

static inline void pu_run(pu_rig *r, ivs_psg_sink *k, uint64_t to)
{
  ivs_psg_catch_up(r->p, &r->s, r->regs, &r->obs, to, k);
}

/* Machine time of m (info), or UINT64_MAX when info refuses. */
static inline uint64_t pu_now(const ivs_machine *m)
{
  ivs_info info;
  memset(&info, 0, sizeof info);
  info.struct_size = (uint32_t)sizeof info;
  return ivs_machine_info(m, &info) == IVS_OK ? info.time_ticks : UINT64_MAX;
}

/* The audio bytes the run call needs from now to target (the layout of ivs_info). */
static inline uint64_t pu_need(const ivs_machine *m, uint64_t now, uint64_t target)
{
  ivs_info info;
  memset(&info, 0, sizeof info);
  info.struct_size = (uint32_t)sizeof info;
  if (ivs_machine_info(m, &info) != IVS_OK || info.nominal_frame_ticks == 0u) {
    return 0u;
  }
  return (uint64_t)info.audio_bytes_per_frame * ((target > now ? target - now : 0u) / info.nominal_frame_ticks + 1u);
}

/* One run call to target with an audio buffer of exactly the needed capacity (buf holds at least that many
 * bytes); the report is written to rep. 1 on IVS_OK. */
static inline int pu_run_to(ivs_machine *m, uint64_t target, uint8_t *buf, ivs_run_report *rep)
{
  ivs_run_request q;
  memset(&q, 0, sizeof q);
  q.struct_size = (uint32_t)sizeof q;
  q.target_ticks = target;
  q.audio = buf;
  q.audio_capacity = buf != NULL ? pu_need(m, pu_now(m), target) : 0u;
  memset(rep, 0, sizeof *rep);
  rep->struct_size = (uint32_t)sizeof *rep;
  return ivs_machine_run(m, &q, rep) == IVS_OK;
}

/* Runs m up to target in calls whose targets advance by the slice sizes in turn (slices[i % n]; n 0: one call to
 * target), the last call ending at target, appending every delivered audio byte to out (cap out_cap) and checking
 * that audio_first_index of each call is the number of samples delivered before it. *delivered gets the bytes.
 * 1 on success. */
static inline int pu_run_sliced(ivs_machine *m, uint64_t target, const uint64_t *slices, uint32_t n, uint8_t *out,
                                uint64_t out_cap, uint64_t *delivered)
{
  ivs_info info;
  uint8_t *scratch;
  uint64_t scratch_bytes;
  uint64_t got = 0u;
  uint32_t i = 0u;
  int ok = 0;
  memset(&info, 0, sizeof info);
  info.struct_size = (uint32_t)sizeof info;
  if (ivs_machine_info(m, &info) != IVS_OK) {
    return 0;
  }
  scratch_bytes = (uint64_t)info.audio_bytes_per_frame * 3u;
  scratch = malloc((size_t)scratch_bytes);
  if (scratch == NULL) {
    return 0;
  }
  while (pu_now(m) < target) {
    ivs_run_request q;
    ivs_run_report rep;
    uint64_t now = pu_now(m);
    uint64_t t = target;
    if (n != 0u && slices[i % n] < target - now) {
      t = now + slices[i % n];
    }
    i++;
    memset(&q, 0, sizeof q);
    q.struct_size = (uint32_t)sizeof q;
    q.target_ticks = t;
    q.audio = scratch;
    q.audio_capacity = pu_need(m, now, t);
    if (q.audio_capacity > scratch_bytes) {
      goto done;
    }
    memset(&rep, 0, sizeof rep);
    rep.struct_size = (uint32_t)sizeof rep;
    if (ivs_machine_run(m, &q, &rep) != IVS_OK || rep.audio_first_index != got / 4u ||
        got + rep.audio_bytes_written > out_cap || rep.stop_reason != IVS_STOP_TARGET) {
      goto done;
    }
    memcpy(out + got, scratch, (size_t)rep.audio_bytes_written);
    got += rep.audio_bytes_written;
  }
  *delivered = got;
  ok = 1;
done:
  free(scratch);
  return ok;
}

/* ---- write log helpers (plan 05-08) ---- */

/* The write-log bytes the run call needs from now to target (the layout of ivs_info). */
static inline uint64_t pu_log_need(const ivs_machine *m, uint64_t now, uint64_t target)
{
  ivs_info info;
  memset(&info, 0, sizeof info);
  info.struct_size = (uint32_t)sizeof info;
  if (ivs_machine_info(m, &info) != IVS_OK || info.nominal_frame_ticks == 0u) {
    return 0u;
  }
  return (uint64_t)info.psg_log_record_bytes * (uint64_t)info.psg_log_records_per_frame *
         ((target > now ? target - now : 0u) / info.nominal_frame_ticks + 1u);
}

/* One run call to target with a write-log buffer (and an audio buffer when audio is not NULL) of exactly the
 * needed capacities; the report is written to rep. 1 on IVS_OK. */
static inline int pu_run_to_log(ivs_machine *m, uint64_t target, uint8_t *audio, uint8_t *log, ivs_run_report *rep)
{
  ivs_run_request q;
  uint64_t now = pu_now(m);
  memset(&q, 0, sizeof q);
  q.struct_size = (uint32_t)sizeof q;
  q.target_ticks = target;
  q.audio = audio;
  q.audio_capacity = audio != NULL ? pu_need(m, now, target) : 0u;
  q.psg_log = log;
  q.psg_log_capacity = log != NULL ? pu_log_need(m, now, target) : 0u;
  memset(rep, 0, sizeof *rep);
  rep->struct_size = (uint32_t)sizeof *rep;
  return ivs_machine_run(m, &q, rep) == IVS_OK;
}

/* Big-endian stores, written separately from the core. */
static inline void pu_be64(uint8_t *d, uint64_t v)
{
  uint32_t i;
  for (i = 0u; i < 8u; i++) {
    d[i] = (uint8_t)(v >> (56u - 8u * i));
  }
}

static inline uint64_t pu_get_be64(const uint8_t *d)
{
  uint64_t v = 0u;
  uint32_t i;
  for (i = 0u; i < 8u; i++) {
    v = (v << 8) | d[i];
  }
  return v;
}

/* Stand-in programs of the write-log tests, as words at $1000 (encodings as in machine_standin.c: MVII is 02B8 +
 * destination, MVO Rs, a is 0240 + source, MVO@ Rs, Rm is 0240 + 8 x m + s, B back is 0220 with d, target =
 * address of the B when d is 1). The word count is returned.
 * LOGMIX: MVII #$0F, R0; MVO R0, $01FB (direct); MVII #$01FC, R3; MVO@ R0, R3; then B to itself. The direct MVO
 * writes at CPU cycle 6 of its 11-cycle unit, the MVO@ at cycle 4 of its 9-cycle unit.
 * WRITER n: MVII #$01FB, R3; MVII #$0F, R0; then n straight-line MVO@ R0, R3 (gap 9 CPU cycles = 36 ticks between
 * writes, the instruction-only density), then B to itself. w holds at least n + 6 words. */
static inline size_t pu_prog_logmix(uint16_t *w)
{
  static const uint16_t p[] = {0x02B8u, 0x000Fu, 0x0240u, 0x01FBu, 0x02BBu, 0x01FCu, 0x0258u, 0x0220u, 0x0001u};
  memcpy(w, p, sizeof p);
  return sizeof p / sizeof p[0];
}

static inline size_t pu_prog_writer(uint16_t *w, uint32_t n)
{
  uint32_t i;
  size_t k = 0u;
  w[k++] = 0x02BBu;
  w[k++] = 0x01FBu;
  w[k++] = 0x02B8u;
  w[k++] = 0x000Fu;
  for (i = 0u; i < n; i++) {
    w[k++] = 0x0258u;
  }
  w[k++] = 0x0220u;
  w[k++] = 0x0001u;
  return k;
}

/* Runs m to target in calls whose targets advance by the slice sizes in turn (as pu_run_sliced), delivering the
 * audio into out_a (cap a_cap, *a_n bytes) and the write log into out_l (cap l_cap, *l_n bytes), and checking that
 * audio_first_index and psg_log_first_index of each call are the counts delivered before it. 1 on success. */
static inline int pu_run_sliced_log(ivs_machine *m, uint64_t target, const uint64_t *slices, uint32_t n,
                                    uint8_t *out_a, uint64_t a_cap, uint64_t *a_n, uint8_t *out_l, uint64_t l_cap,
                                    uint64_t *l_n)
{
  ivs_info info;
  uint8_t *sa;
  uint8_t *sl;
  uint64_t got_a = 0u;
  uint64_t got_l = 0u;
  uint32_t i = 0u;
  int ok = 0;
  memset(&info, 0, sizeof info);
  info.struct_size = (uint32_t)sizeof info;
  if (ivs_machine_info(m, &info) != IVS_OK) {
    return 0;
  }
  sa = malloc((size_t)info.audio_bytes_per_frame * 3u);
  sl = malloc((size_t)info.psg_log_record_bytes * info.psg_log_records_per_frame * 3u);
  if (sa == NULL || sl == NULL) {
    free(sa);
    free(sl);
    return 0;
  }
  while (pu_now(m) < target) {
    ivs_run_request q;
    ivs_run_report rep;
    uint64_t now = pu_now(m);
    uint64_t t = target;
    if (n != 0u && slices[i % n] < target - now) {
      t = now + slices[i % n];
    }
    i++;
    memset(&q, 0, sizeof q);
    q.struct_size = (uint32_t)sizeof q;
    q.target_ticks = t;
    q.audio = sa;
    q.audio_capacity = pu_need(m, now, t);
    q.psg_log = sl;
    q.psg_log_capacity = pu_log_need(m, now, t);
    if (q.audio_capacity > (uint64_t)info.audio_bytes_per_frame * 3u ||
        q.psg_log_capacity > (uint64_t)info.psg_log_record_bytes * info.psg_log_records_per_frame * 3u) {
      goto done;
    }
    memset(&rep, 0, sizeof rep);
    rep.struct_size = (uint32_t)sizeof rep;
    if (ivs_machine_run(m, &q, &rep) != IVS_OK || rep.audio_first_index != got_a / 4u ||
        rep.psg_log_first_index != got_l / 16u || got_a + rep.audio_bytes_written > a_cap ||
        got_l + rep.psg_log_bytes_written > l_cap || rep.stop_reason != IVS_STOP_TARGET) {
      goto done;
    }
    memcpy(out_a + got_a, sa, (size_t)rep.audio_bytes_written);
    memcpy(out_l + got_l, sl, (size_t)rep.psg_log_bytes_written);
    got_a += rep.audio_bytes_written;
    got_l += rep.psg_log_bytes_written;
  }
  *a_n = got_a;
  *l_n = got_l;
  ok = 1;
done:
  free(sa);
  free(sl);
  return ok;
}

#endif

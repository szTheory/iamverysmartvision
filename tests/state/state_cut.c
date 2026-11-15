/* SPDX-License-Identifier: MIT */
/* Suite state, family cut (plan 08-14, D-16, D-17, 08-DISCUSS-state 5.3 and 7.3): snapshot, restore and run equals the uninterrupted run at
 * the named cuts of data/expected/state-cuts.ids, compared by oracles that do not depend on the shared digest field table.
 * REF is an uninterrupted run of the program from a fresh instance to the end tick E of the cut through the public run call (video, audio
 * and PSG log buffers sized from ivs_info). SUB runs a second fresh instance to the cut, saves, and restores into a target that was
 * poisoned (every STATE and PAD row filled with 0xA5 through the offset, width and count of the field table, state_poison): a fresh target
 * is zero and every field that is zero at the cut would pass. At once the whole target instance is compared with the source (memcmp) and
 * all nine digests. The target then continues to E with fresh stream buffers and the input events resubmitted from the applied count: the
 * video, audio and PSG log streams of the two legs concatenated equal REF's byte for byte, each leg's first-index fields continue the
 * previous leg, and the final instance bytes, the nine digests and the public reports (CPU state, STIC state, info) equal REF's.
 *   state.cut.cpu-points     the eleven cpu cuts (plan 08-14 task 1)
 *   state.judge.cut-compare-detects-divergence   the comparison can fail: a byte of a STATE row changed after the restore is seen by the
 *                            immediate memcmp, the digests, the final memcmp and the final digests; a stream byte flipped in leg 2 by the
 *                            concatenation comparison */
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "common/ivs_sha256.h"
#include "ivs/ivs.h"
#include "machine/machine.h"
#include "state_cases.h"
#include "state_util.h"

/* ---- the legs ---- */

void sc_leg_free(sc_leg *l)
{
  free(l->video);
  free(l->audio);
  free(l->log);
  memset(l, 0, sizeof *l);
}

static uint8_t *sc_buf(uint64_t cap)
{
  uint8_t *b;
  if (cap == 0u) {
    return NULL;
  }
  b = (uint8_t *)malloc((size_t)cap);
  if (b != NULL) {
    memset(b, 0xCCu, (size_t)cap); /* a byte the run did not write stays recognisable */
  }
  return b;
}

int sc_run_leg(ivs_machine *m, uint64_t target, const ivs_input_event *ev, uint32_t nev, uint32_t consumed_in, sc_leg *out)
{
  ivs_info info;
  ivs_run_request q;
  ivs_run_report r;
  uint64_t d;
  uint64_t frames;
  uint64_t vcap;
  uint64_t acap;
  uint64_t lcap;
  memset(out, 0, sizeof *out);
  out->st = IVS_E_BAD_ARG;
  memset(&info, 0, sizeof info);
  info.struct_size = (uint32_t)sizeof info;
  if (ivs_machine_info(m, &info) != IVS_OK || info.nominal_frame_ticks == 0u) {
    return 0;
  }
  /* the capacity rule of the run call: bytes per frame times (floor(D / F) + 1), the video slack slots added */
  d = target > info.time_ticks ? target - info.time_ticks : 0u;
  frames = d / info.nominal_frame_ticks + 1u;
  vcap = (uint64_t)info.video_slot_bytes * (frames + (uint64_t)info.video_slack_frames);
  acap = (uint64_t)info.audio_bytes_per_frame * frames;
  lcap = (uint64_t)info.psg_log_record_bytes * (uint64_t)info.psg_log_records_per_frame * frames;
  out->video = sc_buf(vcap);
  out->audio = sc_buf(acap);
  out->log = sc_buf(lcap);
  if ((vcap != 0u && out->video == NULL) || (acap != 0u && out->audio == NULL) || (lcap != 0u && out->log == NULL)) {
    sc_leg_free(out);
    return 0;
  }
  memset(&q, 0, sizeof q);
  q.struct_size = (uint32_t)sizeof q;
  q.target_ticks = target;
  q.video = out->video;
  q.video_capacity = vcap;
  q.audio = out->audio;
  q.audio_capacity = acap;
  q.psg_log = out->log;
  q.psg_log_capacity = lcap;
  q.events = nev > consumed_in ? ev + consumed_in : NULL;
  q.event_count = nev > consumed_in ? nev - consumed_in : 0u;
  memset(&r, 0, sizeof r);
  r.struct_size = (uint32_t)sizeof r;
  out->st = ivs_machine_run(m, &q, &r);
  if (out->st != IVS_OK) {
    return 0;
  }
  out->vbytes = r.video_bytes_written;
  out->abytes = r.audio_bytes_written;
  out->lbytes = r.psg_log_bytes_written;
  out->vfirst = r.video_first_index;
  out->afirst = r.audio_first_index;
  out->lfirst = r.psg_log_first_index;
  out->frames = r.video_frames_completed;
  out->now = r.now_ticks;
  out->units = r.units_executed;
  out->elapsed = r.elapsed_ticks;
  out->stop = r.stop_reason;
  out->consumed = r.input_consumed;
  memcpy(out->ctrl, r.controller_now, sizeof out->ctrl);
  out->floor = r.input_floor_tick;
  return 1;
}

/* ---- the reference ---- */

#define SC_REF_MAX 24u
static sc_ref sc_refs[SC_REF_MAX];
static uint32_t sc_ref_n;

static void sc_ref_release(sc_ref *r)
{
  sc_leg_free(&r->leg);
  ivs_ms_free(&r->x);
  memset(r, 0, sizeof *r);
}

void sc_ref_free(void)
{
  uint32_t i;
  for (i = 0u; i < sc_ref_n; i++) {
    sc_ref_release(&sc_refs[i]);
  }
  sc_ref_n = 0u;
}

int sc_reports(const ivs_machine *m, ivs_cpu_state_report *cpu, ivs_stic_report *stic, ivs_info *info)
{
  memset(cpu, 0, sizeof *cpu);
  cpu->struct_size = (uint32_t)sizeof *cpu;
  memset(stic, 0, sizeof *stic);
  stic->struct_size = (uint32_t)sizeof *stic;
  memset(info, 0, sizeof *info);
  info->struct_size = (uint32_t)sizeof *info;
  return ivs_machine_cpu_state(m, cpu) == IVS_OK && ivs_machine_stic_state(m, stic) == IVS_OK && ivs_machine_info(m, info) == IVS_OK;
}

const sc_ref *sc_ref_get(const sc_run *run, uint64_t end)
{
  uint32_t i;
  sc_ref *r;
  const ivs_input_event *ev;
  uint32_t nev = 0u;
  for (i = 0u; i < sc_ref_n; i++) {
    if (sc_refs[i].run == run && sc_refs[i].end == end) {
      return &sc_refs[i];
    }
  }
  if (sc_ref_n >= SC_REF_MAX) {
    sc_ref_free();
  }
  r = &sc_refs[sc_ref_n];
  memset(r, 0, sizeof *r);
  r->run = run;
  r->end = end;
  if (!sc_new(&r->x, run, 1)) {
    return NULL;
  }
  ev = sc_events(run, &nev);
  if (!sc_run_leg(r->x.m, end, ev, nev, 0u, &r->leg) || !su_nine_digests(r->x.m, r->dig) || !sc_reports(r->x.m, &r->cpu, &r->stic, &r->info)) {
    ivs_test_diag("the reference of %s to tick %llu could not be made", run->id, (unsigned long long)end);
    sc_ref_release(r);
    return NULL;
  }
  sc_ref_n++;
  return r;
}

uint64_t sc_window_end(uint64_t tick, uint64_t end, int full)
{
#ifdef IVS_STATE_FAULT_RUN
  if (!full) {
    uint64_t frame_end = (tick / SC_FRAME + 1u) * SC_FRAME;
    return frame_end < end ? frame_end : end;
  }
#else
  (void)tick;
  (void)full;
#endif
  return end;
}

/* ---- the comparison ---- */

/* The first differing byte of two buffers of n bytes, or n. */
static size_t sc_first_diff(const uint8_t *a, const uint8_t *b, size_t n)
{
  size_t i = 0u;
  while (i < n && a[i] == b[i]) {
    i++;
  }
  return i;
}

/* leg1 ++ leg2 against the reference stream (n1 and n2 bytes of the legs): 1 when equal. */
static int sc_stream_equal(const uint8_t *a, uint64_t n1, const uint8_t *b, uint64_t n2, const uint8_t *ref, uint64_t nref)
{
  if (n1 + n2 != nref) {
    return 0;
  }
  return (n1 == 0u || memcmp(a, ref, (size_t)n1) == 0) && (n2 == 0u || memcmp(b, ref + n1, (size_t)n2) == 0);
}

#define SC_NOTE(q, ...)                                                                                \
  do {                                                                                                 \
    if (!(q)) {                                                                                        \
      ivs_test_diag(__VA_ARGS__);                                                                      \
    }                                                                                                  \
  } while (0)

uint32_t sc_cut_compare(const char *label, const sc_run *run, uint64_t tick, uint64_t end, uint32_t tamper, int quiet, sc_cmp_info *info)
{
  uint32_t f = 0u;
  const sc_ref *ref = sc_ref_get(run, end);
  ivs_ms_inst src;
  ivs_ms_inst tgt;
  sc_leg l1;
  sc_leg l2;
  uint8_t *snap = (uint8_t *)malloc(IVS_SNAPSHOT_MAX_BYTES);
  size_t n = 0u;
  const ivs_input_event *ev;
  uint32_t nev = 0u;
  uint8_t d_src[9][32];
  uint8_t d_tgt[9][32];
  ivs_cpu_state_report cpu;
  ivs_stic_report stic;
  ivs_info inf;
  ivs_info sinfo;
  int have_src = 0;
  int have_tgt = 0;
  memset(&l1, 0, sizeof l1);
  memset(&l2, 0, sizeof l2);
  if (info != NULL) {
    memset(info, 0, sizeof *info);
    info->tick = tick;
    info->end = end;
  }
  if (ref == NULL || snap == NULL) {
    free(snap);
    return SC_F_SETUP;
  }
  ev = sc_events(run, &nev);
  if (!sc_new(&src, run, 1)) {
    free(snap);
    return SC_F_SETUP;
  }
  have_src = 1;
  /* leg 1: the source runs to the cut and must end exactly on it */
  if (tick > 0u) {
    if (!sc_run_leg(src.m, tick, ev, nev, 0u, &l1)) {
      SC_NOTE(quiet, "%s: leg 1 refused (%s)", label, ivs_status_name(l1.st));
      f |= SC_F_RUN | SC_F_SETUP;
    } else if (l1.now != tick) {
      SC_NOTE(quiet, "%s: leg 1 ended at tick %llu, not on the cut %llu", label, (unsigned long long)l1.now, (unsigned long long)tick);
      f |= SC_F_SETUP;
    }
  }
  if (f != 0u || !su_snapshot(src.m, snap, IVS_SNAPSHOT_MAX_BYTES, &n)) {
    SC_NOTE(quiet, "%s: no snapshot of the source", label);
    f |= SC_F_SETUP;
    goto done;
  }
  if (info != NULL) {
    const struct ivs_machine *sm = src.m;
    uint32_t w;
    for (w = 0u; w < 0x18u; w++) {
      info->mob |= sm->mem.stic_reg[w];
    }
    for (w = 0x18u; w < 0x20u; w++) {
      info->coll |= sm->mem.stic_reg[w];
    }
    info->pulldown = (uint32_t)sm->mem.port_pulldown[0] | (uint32_t)sm->mem.port_pulldown[1];
    info->slot_bytes = ref->info.video_slot_bytes;
    ivs_sha256_digest(snap, n, info->snap_sha);
    info->units1 = l1.units;
    info->vbytes1 = l1.vbytes;
    info->abytes1 = l1.abytes;
    info->lbytes1 = l1.lbytes;
    info->consumed1 = l1.consumed;
  }
  /* the poisoned target: the same images, every STATE and PAD row 0xA5 */
  if (!sc_new(&tgt, run, 1)) {
    f |= SC_F_SETUP;
    goto done;
  }
  have_tgt = 1;
  state_poison(tgt.m);
  if (memcmp(src.storage, tgt.storage, IVS_MACHINE_BYTES) == 0) {
    SC_NOTE(quiet, "%s: the poisoned target equals the source before the restore: the poison is not doing its job", label);
    f |= SC_F_SETUP;
  }
  if (ivs_machine_snapshot_restore(tgt.m, snap, n, NULL) != IVS_OK) {
    SC_NOTE(quiet, "%s: the restore refused the snapshot of the cut", label);
    f |= SC_F_SETUP;
    goto done;
  }
  if (tamper == SC_T_ROW) {
    ((struct ivs_machine *)tgt.m)->time_ticks ^= 1u; /* one byte of the STATE row time_ticks, after the restore */
  }
  /* at once: the whole target instance and the nine digests against the source */
  if (memcmp(src.storage, tgt.storage, IVS_MACHINE_BYTES) != 0) {
    size_t at = sc_first_diff(src.storage, tgt.storage, IVS_MACHINE_BYTES);
    SC_NOTE(quiet, "%s: right after the restore the target differs from the source at byte %u (source %02X, target %02X)", label, (unsigned)at,
            (unsigned)src.storage[at], (unsigned)tgt.storage[at]);
    f |= SC_F_IMM_MEM;
  }
  if (!su_nine_digests(src.m, d_src) || !su_nine_digests(tgt.m, d_tgt)) {
    f |= SC_F_SETUP;
    goto done;
  }
  if (memcmp(d_src, d_tgt, sizeof d_src) != 0) {
    uint32_t k = 0u;
    while (k < 9u && memcmp(d_src[k], d_tgt[k], 32u) == 0) {
      k++;
    }
    SC_NOTE(quiet, "%s: right after the restore the digest %u of the target differs from the source", label, (unsigned)k);
    f |= SC_F_IMM_DIG;
  }
  /* leg 2: fresh buffers, the events resubmitted from the applied count the first leg reported */
  if (!sc_run_leg(tgt.m, end, ev, nev, l1.consumed, &l2)) {
    SC_NOTE(quiet, "%s: leg 2 refused (%s)", label, ivs_status_name(l2.st));
    f |= SC_F_RUN;
    goto done;
  }
  if (info != NULL) {
    info->units2 = l2.units;
    info->vbytes2 = l2.vbytes;
    info->abytes2 = l2.abytes;
    info->lbytes2 = l2.lbytes;
  }
  if (tamper == SC_T_STREAM && l2.vbytes > 0u) {
    l2.video[l2.vbytes / 2u] ^= 0x01u;
  }
  if (!sc_stream_equal(l1.video, l1.vbytes, l2.video, l2.vbytes, ref->leg.video, ref->leg.vbytes)) {
    SC_NOTE(quiet, "%s: the video stream of the two legs (%llu + %llu bytes) is not the reference stream (%llu bytes)", label,
            (unsigned long long)l1.vbytes, (unsigned long long)l2.vbytes, (unsigned long long)ref->leg.vbytes);
    f |= SC_F_VIDEO;
  }
  if (!sc_stream_equal(l1.audio, l1.abytes, l2.audio, l2.abytes, ref->leg.audio, ref->leg.abytes)) {
    SC_NOTE(quiet, "%s: the audio stream of the two legs (%llu + %llu bytes) is not the reference stream (%llu bytes)", label,
            (unsigned long long)l1.abytes, (unsigned long long)l2.abytes, (unsigned long long)ref->leg.abytes);
    f |= SC_F_AUDIO;
  }
  if (!sc_stream_equal(l1.log, l1.lbytes, l2.log, l2.lbytes, ref->leg.log, ref->leg.lbytes)) {
    SC_NOTE(quiet, "%s: the PSG log of the two legs (%llu + %llu bytes) is not the reference log (%llu bytes)", label, (unsigned long long)l1.lbytes,
            (unsigned long long)l2.lbytes, (unsigned long long)ref->leg.lbytes);
    f |= SC_F_LOG;
  }
  /* each leg's first index is the count the previous legs delivered */
  inf = ref->info;
  if (l2.vfirst != l1.vfirst + l1.frames || l2.afirst != l1.afirst + l1.abytes / inf.audio_sample_bytes ||
      l2.lfirst != l1.lfirst + l1.lbytes / inf.psg_log_record_bytes || l1.vfirst != 0u || l1.afirst != 0u || l1.lfirst != 0u) {
    SC_NOTE(quiet, "%s: the first-index fields do not continue: video %llu after %llu frames, audio %llu after %llu samples, log %llu after %llu records",
            label, (unsigned long long)l2.vfirst, (unsigned long long)l1.frames, (unsigned long long)l2.afirst,
            (unsigned long long)(l1.abytes / inf.audio_sample_bytes), (unsigned long long)l2.lfirst,
            (unsigned long long)(l1.lbytes / inf.psg_log_record_bytes));
    f |= SC_F_FIRST;
  }
  /* the run reports: time, units, stop reason and the events applied equal the uninterrupted run */
  if (l2.now != ref->leg.now || l1.units + l2.units != ref->leg.units || l2.stop != ref->leg.stop || l1.consumed + l2.consumed != ref->leg.consumed ||
      l2.floor != ref->leg.floor || memcmp(l2.ctrl, ref->leg.ctrl, sizeof l2.ctrl) != 0) {
    SC_NOTE(quiet, "%s: the run reports differ: now %llu (reference %llu), units %llu + %llu (reference %llu), stop %u (reference %u), events %u + %u (reference %u)",
            label, (unsigned long long)l2.now, (unsigned long long)ref->leg.now, (unsigned long long)l1.units, (unsigned long long)l2.units,
            (unsigned long long)ref->leg.units, (unsigned)l2.stop, (unsigned)ref->leg.stop, (unsigned)l1.consumed, (unsigned)l2.consumed,
            (unsigned)ref->leg.consumed);
    f |= SC_F_RUN;
  }
  /* the end: the whole instance, the nine digests and the public reports against the reference */
  if (memcmp(tgt.storage, ref->x.storage, IVS_MACHINE_BYTES) != 0) {
    size_t at = sc_first_diff(tgt.storage, ref->x.storage, IVS_MACHINE_BYTES);
    SC_NOTE(quiet, "%s: the final instance differs from the reference at byte %u (target %02X, reference %02X)", label, (unsigned)at,
            (unsigned)tgt.storage[at], (unsigned)ref->x.storage[at]);
    f |= SC_F_FINAL_MEM;
  }
  if (!su_nine_digests(tgt.m, d_tgt) || memcmp(d_tgt, ref->dig, sizeof d_tgt) != 0) {
    SC_NOTE(quiet, "%s: a final digest differs from the reference", label);
    f |= SC_F_FINAL_DIG;
  }
  if (!sc_reports(tgt.m, &cpu, &stic, &sinfo) || memcmp(&cpu, &ref->cpu, sizeof cpu) != 0 || memcmp(&stic, &ref->stic, sizeof stic) != 0 ||
      memcmp(&sinfo, &ref->info, sizeof sinfo) != 0) {
    SC_NOTE(quiet, "%s: a public report (CPU state, STIC state or info) differs from the reference", label);
    f |= SC_F_REPORTS;
  }
done:
  sc_leg_free(&l1);
  sc_leg_free(&l2);
  if (have_src) {
    ivs_ms_free(&src);
  }
  if (have_tgt) {
    ivs_ms_free(&tgt);
  }
  free(snap);
  return f;
}

/* ---- the ids ---- */

/* What a cut must show besides equality (the plan's per-cut checks): that the state of the cut is the one its predicate names, so that equality
 * is not the equality of two empty things. 1 when it holds. */
static int sc_cut_extra(const sc_cut *c, const sc_cmp_info *i)
{
  const char *id = c->id;
  if (strcmp(id, "HP-FRAME-MID") == 0 || strcmp(id, "HP-LATCH") == 0) {
    return i->vbytes2 >= i->slot_bytes; /* a video slot completed after the restore, compared byte for byte with the reference's */
  }
  if (strcmp(id, "HP-MOB") == 0) {
    return i->mob != 0u && i->coll != 0u && i->vbytes2 >= i->slot_bytes; /* MOB registers and a sticky collision bit, and the slot */
  }
  if (strcmp(id, "HP-INPUT") == 0) {
    return i->consumed1 == 1u && i->pulldown != 0u; /* one event applied, a pull-down in force; the other events are resubmitted from the count */
  }
  if (strcmp(id, "HP-LOG") == 0) {
    return i->lbytes1 > 0u; /* the log has records before the cut: the record index continues */
  }
  if (strcmp(id, "HP-PSG-MID") == 0 || strcmp(id, "HP-ENV-MID") == 0) {
    return i->abytes1 > 0u && i->abytes2 > 0u; /* audio samples on both sides of the cut */
  }
  return 1;
}

/* The cuts of a family through the comparison. full: the must-fail id of a fault runs every cut with the whole window in a fault run. */
static int sc_cut_family(const char *family, int full)
{
  size_t i;
  size_t n = 0u;
  int ok = 1;
  for (i = 0u; i < sc_cut_count(); i++) {
    const sc_cut *c = sc_cut_at(i);
    const sc_run *run;
    sc_cmp_info info;
    uint64_t end;
    uint32_t f;
    if (strcmp(c->family, family) != 0) {
      continue;
    }
    n++;
    /* the fault-run reduced point set: every fourth cut (the must-fail id of no fault, D-40) */
    if (!full && ST_FAULT_STRIDE > 1u && (n - 1u) % 4u != 0u) {
      continue;
    }
    run = sc_run_find(c->program);
    if (run == NULL || c->tick == SC_PENDING) {
      ivs_test_diag("cut %s: no program %s or a pending tick", c->id, c->program);
      ok = 0;
      continue;
    }
    end = sc_window_end(c->tick, sc_cut_end(c), full);
    f = sc_cut_compare(c->id, run, c->tick, end, SC_T_NONE, 0, &info);
    ivs_test_diag("cut %s: tick %llu, end %llu, %llu + %llu units, video %llu + %llu bytes, audio %llu + %llu, log %llu + %llu, events %u before the cut%s", c->id,
                  (unsigned long long)info.tick, (unsigned long long)info.end, (unsigned long long)info.units1, (unsigned long long)info.units2,
                  (unsigned long long)info.vbytes1, (unsigned long long)info.vbytes2, (unsigned long long)info.abytes1,
                  (unsigned long long)info.abytes2, (unsigned long long)info.lbytes1, (unsigned long long)info.lbytes2, (unsigned)info.consumed1,
                  f == 0u ? "" : " FAILED");
    ST_REQ(ok, f == 0u);
    if (f == 0u && !sc_cut_extra(c, &info)) {
      ivs_test_diag("cut %s: the state of the cut is not the state its predicate names (MOB %04X, collision %04X, pull-downs %02X, events %u)", c->id,
                    (unsigned)info.mob, (unsigned)info.coll, (unsigned)info.pulldown, (unsigned)info.consumed1);
      ok = 0;
    }
  }
  sc_ref_free();
  ST_REQ(ok, n > 0u);
  return ok ? IVS_TEST_PASS : IVS_TEST_FAIL;
}

static int sc_cut_cpu(void)
{
  return sc_cut_family("cpu-points", 0);
}

static int sc_cut_stic(void)
{
  return sc_cut_family("stic-points", 1); /* the must-fail id of SNAPSHOT_STIC_ROWS_OMITTED: always the full set */
}

static int sc_cut_device(void)
{
  return sc_cut_family("device-points", 0);
}

/* state.cut.latch-not-recomputed (D-17, HP-LATCH): P-LATCH stores a changing value into a word of an already latched row after its fetch, in a
 * Color Stack frame, so the latched copy stic.row_latch differs from the current BACKTAB word at the cut. The snapshot is restored twice into
 * poisoned targets: the real restore equals the reference (state.cut.stic-points shows that); the variant target is the real one with
 * stic.row_latch recomputed from the BACKTAB words (what a restore that derives the latch would leave), and its continuation must differ from
 * the reference in the first video slot completed after the cut, which renders row 0 from the latch. If the variant happened to equal the
 * reference the program would not discriminate a recomputing restore, and the test fails. */
static int sc_latch_not_recomputed(void)
{
  const sc_cut *c = sc_cut_find("HP-LATCH");
  const sc_run *run = c != NULL ? sc_run_find(c->program) : NULL;
  const sc_ref *ref;
  uint64_t end;
  ivs_ms_inst src;
  ivs_ms_inst real;
  ivs_ms_inst var;
  sc_leg l1;
  sc_leg lr;
  sc_leg lv;
  uint8_t *snap;
  size_t n = 0u;
  uint32_t slot;
  uint32_t differing = 0u;
  uint32_t r;
  uint32_t k;
  int ok = 1;
  ST_CHECK(run != NULL && c->tick != SC_PENDING);
  end = sc_window_end(c->tick, sc_cut_end(c), 0);
  ref = sc_ref_get(run, end);
  ST_CHECK(ref != NULL);
  slot = ref->info.video_slot_bytes;
  snap = (uint8_t *)malloc(IVS_SNAPSHOT_MAX_BYTES);
  ST_CHECK(snap != NULL);
  memset(&l1, 0, sizeof l1);
  memset(&lr, 0, sizeof lr);
  memset(&lv, 0, sizeof lv);
  if (!sc_new(&src, run, 1) || !sc_run_leg(src.m, c->tick, NULL, 0u, 0u, &l1) || ((const struct ivs_machine *)src.m)->time_ticks != c->tick ||
      !su_snapshot(src.m, snap, IVS_SNAPSHOT_MAX_BYTES, &n) || !sc_new(&real, run, 1) || !sc_new(&var, run, 1)) {
    ivs_test_diag("the HP-LATCH run could not be set up");
    free(snap);
    return IVS_TEST_FAIL;
  }
  state_poison(real.m);
  state_poison(var.m);
  ST_REQ(ok, ivs_machine_snapshot_restore(real.m, snap, n, NULL) == IVS_OK);
  ST_REQ(ok, ivs_machine_snapshot_restore(var.m, snap, n, NULL) == IVS_OK);
  {
    struct ivs_machine *v = (struct ivs_machine *)var.m;
    for (r = 0u; r < 12u; r++) {
      for (k = 0u; k < 20u; k++) {
        v->stic.row_latch[r][k] = v->mem.sysram[r * 20u + k]; /* the latch rebuilt from the BACKTAB words, not restored */
      }
    }
  }
  ST_REQ(ok, memcmp(real.storage, src.storage, IVS_MACHINE_BYTES) == 0);
  ST_REQ(ok, memcmp(var.storage, src.storage, IVS_MACHINE_BYTES) != 0); /* the cut has a latch that BACKTAB cannot give back */
  ST_REQ(ok, sc_run_leg(real.m, end, NULL, 0u, 0u, &lr) && sc_run_leg(var.m, end, NULL, 0u, 0u, &lv));
  if (ok) {
    const uint8_t *want = ref->leg.video + l1.vbytes;
    ST_REQ(ok, lr.vbytes >= slot && lv.vbytes >= slot && lr.vbytes == ref->leg.vbytes - l1.vbytes && lv.vbytes == lr.vbytes);
    if (lr.vbytes >= slot && lv.vbytes >= slot) {
      for (k = 0u; k < slot; k++) {
        differing += (uint32_t)(lv.video[k] != want[k]);
      }
      ST_REQ(ok, memcmp(lr.video, want, (size_t)lr.vbytes) == 0);              /* the real restore: the slots are the reference's */
      ST_REQ(ok, differing > 0u);                                              /* the recomputing variant: the first slot is not */
      ivs_test_diag("the first video slot after the cut: the real restore equals the reference, the recomputed latch differs in %u of %u bytes", (unsigned)differing,
                    (unsigned)slot);
    }
  }
  sc_leg_free(&l1);
  sc_leg_free(&lr);
  sc_leg_free(&lv);
  ivs_ms_free(&src);
  ivs_ms_free(&real);
  ivs_ms_free(&var);
  free(snap);
  sc_ref_free();
  return ok ? IVS_TEST_PASS : IVS_TEST_FAIL;
}

/* state.judge.cut-compare-detects-divergence: the comparison is run with a target that was changed on purpose after the restore (one byte of
 * the STATE row time_ticks) and with a stream byte flipped in leg 2, at a cut with video; every comparison that should see the change must
 * report it, and the untouched comparison reports nothing. */
static int sc_judge_cut_compare(void)
{
  const sc_cut *c = sc_cut_find("HP-INT-ENTRY");
  const sc_run *run = c != NULL ? sc_run_find(c->program) : NULL;
  uint64_t end;
  uint32_t clean;
  uint32_t row;
  uint32_t stream;
  int ok = 1;
  ST_CHECK(run != NULL && c->tick != SC_PENDING);
  end = sc_window_end(c->tick, sc_cut_end(c), 0);
  clean = sc_cut_compare("judge/none", run, c->tick, end, SC_T_NONE, 1, NULL);
  row = sc_cut_compare("judge/row", run, c->tick, end, SC_T_ROW, 1, NULL);
  stream = sc_cut_compare("judge/stream", run, c->tick, end, SC_T_STREAM, 1, NULL);
  sc_ref_free();
  ivs_test_diag("untouched mask %03X, a changed STATE byte %03X, a flipped stream byte %03X", (unsigned)clean, (unsigned)row, (unsigned)stream);
  ST_REQ(ok, clean == 0u);
  ST_REQ(ok, (row & SC_F_IMM_MEM) != 0u);
  ST_REQ(ok, (row & SC_F_IMM_DIG) != 0u);
  ST_REQ(ok, (row & SC_F_FINAL_MEM) != 0u);
  ST_REQ(ok, (row & SC_F_FINAL_DIG) != 0u);
  ST_REQ(ok, (stream & SC_F_VIDEO) != 0u);
  ST_REQ(ok, (stream & (SC_F_IMM_MEM | SC_F_IMM_DIG | SC_F_FINAL_MEM | SC_F_FINAL_DIG)) == 0u);
  return ok ? IVS_TEST_PASS : IVS_TEST_FAIL;
}

/* ---- the canonical form of the snapshot at the cuts ---- */

/* Runs m to the cut tick through the public call in slices of slice ticks (every call's target is the smaller of now + slice and the cut, so a
 * slice that would overshoot the cut stops on it), or in slices of pseudo-random length 1 to 2,000 from a fixed seed (a deterministic LCG, no host
 * time) when seed is not 0. Every call ends on the first unit boundary at or after its target and the cut tick is a unit boundary (the witness id
 * proves that), so the run ends exactly on the cut. 1 when it did. */
static int sc_run_sliced(ivs_machine *m, const ivs_input_event *ev, uint32_t nev, uint64_t tick, uint64_t slice, uint64_t seed, uint32_t *consumed)
{
  uint64_t rng = seed;
  uint64_t calls = 0u;
  *consumed = 0u;
  while (((const struct ivs_machine *)m)->time_ticks < tick) {
    uint64_t now = ((const struct ivs_machine *)m)->time_ticks;
    uint64_t len = slice;
    uint64_t target;
    if (seed != 0u) {
      rng = rng * UINT64_C(6364136223846793005) + UINT64_C(1442695040888963407);
      len = 1u + (rng >> 33) % 2000u;
    }
    target = now + len > tick ? tick : now + len;
    if (!su_run_to(m, target, ev, nev, consumed) || ++calls > 400000u) {
      return 0;
    }
  }
  return ((const struct ivs_machine *)m)->time_ticks == tick;
}

/* The key of the snapshot literal of a cut in data/expected/ivsn-literals.ids: cut-snapshot-sha256. and the cut id in lower case. */
static const char *sc_literal_key(const sc_cut *c, char *buf, size_t cap)
{
  size_t i = 0u;
  (void)snprintf(buf, cap, "cut-snapshot-sha256.");
  i = strlen(buf);
  for (const char *p = c->id; *p != '\0' && i + 1u < cap; p++) {
    buf[i++] = (char)((*p >= 'A' && *p <= 'Z') ? *p - 'A' + 'a' : *p);
  }
  buf[i] = '\0';
  return buf;
}

/* The three cuts whose snapshot bytes are pinned: the mid-frame cut, the designed latch cut and the MOB cut. */
static int sc_literal_cut(const sc_cut *c)
{
  return strcmp(c->id, "HP-FRAME-MID") == 0 || strcmp(c->id, "HP-LATCH") == 0 || strcmp(c->id, "HP-MOB") == 0;
}

/* state.cut.fixed-point-and-canonical (D-12, 08-DISCUSS-state 7.3 item 7): at every cut, snapshot, restore into a poisoned target and snapshot again
 * is a fixed point (the two byte strings are identical); the snapshot of a second, independently built source (run to the cut in slices of 468 ticks)
 * is byte-identical to the first; and the SHA-256 of the snapshot bytes at HP-FRAME-MID, HP-LATCH and HP-MOB equals the literal pinned in
 * data/expected/ivsn-literals.ids (self-recorded, repeatability only, never evidence of correctness), which the dev and release presets and every
 * host build have to match: the canonical form does not depend on the preset. A fault run takes every fourth cut (the id is the must-fail id of no fault). */
static int sc_fixed_point(void)
{
  size_t i;
  size_t n = 0u;
  int ok = 1;
  uint8_t *s1 = (uint8_t *)malloc(IVS_SNAPSHOT_MAX_BYTES);
  uint8_t *s2 = (uint8_t *)malloc(IVS_SNAPSHOT_MAX_BYTES);
  uint8_t *s3 = (uint8_t *)malloc(IVS_SNAPSHOT_MAX_BYTES);
  if (s1 == NULL || s2 == NULL || s3 == NULL) {
    free(s1);
    free(s2);
    free(s3);
    return IVS_TEST_FAIL;
  }
  for (i = 0u; i < sc_cut_count(); i++) {
    const sc_cut *c = sc_cut_at(i);
    const sc_run *run = sc_run_find(c->program);
    const ivs_input_event *ev;
    uint32_t nev = 0u;
    uint32_t consumed = 0u;
    ivs_ms_inst a;
    ivs_ms_inst b;
    ivs_ms_inst t;
    size_t n1 = 0u;
    size_t n2 = 0u;
    size_t n3 = 0u;
    n++;
    if (ST_FAULT_STRIDE > 1u && (n - 1u) % 4u != 0u) {
      continue;
    }
    if (run == NULL || c->tick == SC_PENDING) {
      ivs_test_diag("cut %s: no program or a pending tick", c->id);
      ok = 0;
      continue;
    }
    ev = sc_events(run, &nev);
    if (!sc_new(&a, run, 1) || !sc_new(&b, run, 1) || !sc_new(&t, run, 1)) {
      ok = 0;
      continue;
    }
    ST_REQ(ok, c->tick == 0u || su_run_to(a.m, c->tick, ev, nev, &consumed));
    ST_REQ(ok, su_snapshot(a.m, s1, IVS_SNAPSHOT_MAX_BYTES, &n1));
    /* save, restore into a poisoned target, save: the fixed point */
    state_poison(t.m);
    ST_REQ(ok, ivs_machine_snapshot_restore(t.m, s1, n1, NULL) == IVS_OK);
    ST_REQ(ok, su_snapshot(t.m, s2, IVS_SNAPSHOT_MAX_BYTES, &n2) && n2 == n1 && memcmp(s1, s2, n1) == 0);
    /* a second, independently built source, run to the same cut in other slices */
    ST_REQ(ok, sc_run_sliced(b.m, ev, nev, c->tick, 468u, 0u, &consumed));
    ST_REQ(ok, su_snapshot(b.m, s3, IVS_SNAPSHOT_MAX_BYTES, &n3) && n3 == n1 && memcmp(s1, s3, n1) == 0);
    if (n1 == n2 && n1 == n3 && memcmp(s1, s2, n1) == 0 && memcmp(s1, s3, n1) == 0) {
      /* nothing to say beyond the comparison */
    } else {
      ivs_test_diag("cut %s: the snapshots differ (%u, %u and %u bytes)", c->id, (unsigned)n1, (unsigned)n2, (unsigned)n3);
    }
    if (sc_literal_cut(c)) {
      char key[64];
      uint8_t want[32];
      uint8_t got[32];
      char hex[65];
      uint32_t k;
      ivs_sha256_digest(s1, n1, got);
      for (k = 0u; k < 32u; k++) {
        (void)snprintf(hex + 2u * k, 3u, "%02x", (unsigned)got[k]);
      }
      sc_literal_key(c, key, sizeof key);
      if (!ivsn_lit_hex(key, want, 32u)) {
        ivs_test_diag("cut %s: no literal %s; the snapshot of %u bytes has SHA-256 %s", c->id, key, (unsigned)n1, hex);
        ok = 0;
      } else if (memcmp(got, want, 32u) != 0) {
        ivs_test_diag("cut %s: the snapshot SHA-256 %s is not the pinned literal %s", c->id, hex, key);
        ok = 0;
      } else {
        ivs_test_diag("cut %s: %u bytes, SHA-256 %s equals the pinned literal", c->id, (unsigned)n1, hex);
      }
    }
    ivs_ms_free(&a);
    ivs_ms_free(&b);
    ivs_ms_free(&t);
  }
  free(s1);
  free(s2);
  free(s3);
  return ok ? IVS_TEST_PASS : IVS_TEST_FAIL;
}

/* state.cut.slicing-independent (D-12, 08-DISCUSS-state 7.3 item 7): the instance at a cut tick, and so its snapshot, does not depend on how the run
 * to that tick was sliced into calls: the run to the cut is made in calls of 1, 7, 468 and 4,194,304 ticks and in calls of pseudo-random length from a fixed
 * seed (the slice suite's rule, no host time), and each result equals the run made in one call byte for byte, whole instance and snapshot. A call ends on the
 * first unit boundary at or after its target; the last target of every sliced run is the cut tick itself, which is a unit boundary, so every sliced run ends
 * exactly on it. A fault run takes the first cut only. */
static int sc_slicing_independent(void)
{
  static const char *const cuts[] = {"HP-FRAME-MID", "HP-LATCH", "HP-MOB", "HP-INPUT"};
  static const uint64_t slices[] = {1u, 7u, 468u, 4194304u, 0u};
  size_t ci;
  int ok = 1;
  uint8_t *s1 = (uint8_t *)malloc(IVS_SNAPSHOT_MAX_BYTES);
  uint8_t *s2 = (uint8_t *)malloc(IVS_SNAPSHOT_MAX_BYTES);
  if (s1 == NULL || s2 == NULL) {
    free(s1);
    free(s2);
    return IVS_TEST_FAIL;
  }
  for (ci = 0u; ci < sizeof cuts / sizeof cuts[0]; ci++) {
    const sc_cut *c = sc_cut_find(cuts[ci]);
    const sc_run *run = c != NULL ? sc_run_find(c->program) : NULL;
    const ivs_input_event *ev;
    uint32_t nev = 0u;
    uint32_t consumed = 0u;
    ivs_ms_inst ref;
    size_t n1 = 0u;
    size_t k;
    if (ST_FAULT_STRIDE > 1u && ci > 0u) {
      continue;
    }
    if (run == NULL || c->tick == SC_PENDING) {
      ok = 0;
      continue;
    }
    ev = sc_events(run, &nev);
    if (!sc_new(&ref, run, 1) || !su_run_to(ref.m, c->tick, ev, nev, &consumed) || !su_snapshot(ref.m, s1, IVS_SNAPSHOT_MAX_BYTES, &n1)) {
      ok = 0;
      continue;
    }
    for (k = 0u; k < sizeof slices / sizeof slices[0]; k++) {
      ivs_ms_inst x;
      uint32_t used = 0u;
      size_t n2 = 0u;
      int same;
      if (!sc_new(&x, run, 1)) {
        ok = 0;
        continue;
      }
      /* the last slice entry is the seeded run: pseudo-random lengths from the seed 0x2545F4914F6CDD1D */
      ST_REQ(ok, sc_run_sliced(x.m, ev, nev, c->tick, slices[k] != 0u ? slices[k] : 1u, slices[k] != 0u ? 0u : UINT64_C(0x2545F4914F6CDD1D), &used));
      same = memcmp(x.storage, ref.storage, IVS_MACHINE_BYTES) == 0 && used == consumed && su_snapshot(x.m, s2, IVS_SNAPSHOT_MAX_BYTES, &n2) && n2 == n1 &&
             memcmp(s1, s2, n1) == 0;
      if (!same) {
        ivs_test_diag("cut %s: the run in slices of %llu ticks (0: seeded) differs from the run in one call", c->id, (unsigned long long)slices[k]);
      }
      ST_REQ(ok, same);
      ivs_ms_free(&x);
    }
    if (ok) {
      ivs_test_diag("cut %s: tick %llu, %u events applied, the runs in calls of 1, 7, 468 and 4,194,304 ticks and the seeded one equal the run in one call", c->id,
                    (unsigned long long)c->tick, (unsigned)consumed);
    }
    ivs_ms_free(&ref);
  }
  free(s1);
  free(s2);
  return ok ? IVS_TEST_PASS : IVS_TEST_FAIL;
}

const ivs_test_case ivs_state_cut_cases[] = {
    {"state.cut.cpu-points", sc_cut_cpu, NULL},
    {"state.cut.stic-points", sc_cut_stic, NULL},
    {"state.cut.device-points", sc_cut_device, NULL},
    {"state.cut.latch-not-recomputed", sc_latch_not_recomputed, NULL},
    {"state.cut.fixed-point-and-canonical", sc_fixed_point, NULL},
    {"state.cut.slicing-independent", sc_slicing_independent, NULL},
    {"state.judge.cut-compare-detects-divergence", sc_judge_cut_compare, NULL},
};
const size_t ivs_state_cut_count = sizeof ivs_state_cut_cases / sizeof ivs_state_cut_cases[0];

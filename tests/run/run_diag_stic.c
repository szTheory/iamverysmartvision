/* SPDX-License-Identifier: MIT */
/* Suite run, family diag-stic (plan 06-13, D-19, D-20, D-14, A-04; VID-01..VID-04): the third project-authored diagnostic,
 * ivs-diag-stic-1 (fixtures/FX-diag-stic-1/diag-stic-1.listing.txt), packed by tools/pack into the build-tree IVSC container whose
 * bytes ivs_images.inc carries, entered from a power-on reset through the project boot stub (contract 1) with the zero graphics ROM
 * (D-19: no GROM) and driven through the public API only, with a video buffer sized from ivs_info (video_slot_bytes, nominal_frame_ticks
 * and video_slack_frames, never a hard-coded size). One image, two modes: no input leaves Foreground/Background mode, KEY_1 closed from
 * tick 0 selects Color Stack mode (the guest reads 01FE once at the start).
 * The expected values are not in this file: diag_stic_fgbg_pass_expect.inc and diag_stic_cstack_pass_expect.inc are generated at build
 * time from the assert lines of data/golden/diag-stic-fgbg-pass.expect and diag-stic-cstack-pass.expect (tests/expect-inc.cmake), where
 * they were derived by hand (docs/video/diag-stic-1-derivation.md and the frame literals, plan 06-01) before the program existed, and the
 * two expected planes are the literals F-D1 and F-D2 through ivs_diag_frames.inc (cmake/IvsVideoFrames.cmake, FRAMESET=diag). No
 * expected value is read from a core output. Machine time is asserted only as bounds: the stop lies between E_0 + 4,000 and E_1 - 4,000
 * ticks (the margins of the cycle budget of the derivation, section (c)); frame 0 ends at E_0 = 59,736 ticks and frame 1 at E_1 = 119,472.
 *   run.diag.stic.fgbg             no input: the guest halts in the hand-derived state, one frame is delivered, its plane equals F-D1
 *                                  byte for byte and its text-form hash equals the sha256-text key of F-D1
 *   run.diag.stic.cstack           KEY_1 at tick 0 (the SAME image): the Color Stack state and frame F-D2, different from F-D1
 *   run.diag.stic.slice-invariant  slices of 1, 7 and 468 ticks give the same instance bytes, frames and digests as one call
 *   run.diag.stic.frame-stop       a call sequence with IVS_RUN_F_STOP_ON_FRAME (one slot per call) delivers the same frame stream
 *                                  and ends in the same machine; the stops are FRAME then CPU_HALTED
 *   run.diag.stic.nodisplay        the display-enable store patched to the reserved register 0022: frame 0 is delivered not
 *                                  displayed, all border color 0, the guest still halts in the same state inside the margins */
#include <stddef.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>

#include "common/ivs_sha256.h"
#include "ivs/ivs.h"
#include "ivs_test.h"
#include "machine/machine.h"
#include "run_cases.h"

#include "ivs_images.inc"

typedef struct ds_frame_lit {
  const char *name;
  const char *const *plane; /* 96 lines of 160 symbols; the 160th is the cut-off mark */
  const char *sha;          /* the sha256-text key */
} ds_frame_lit;
#include "ivs_diag_frames.inc"

#define DS_CHECK(cond)                                                                                 \
  do {                                                                                                 \
    if (!(cond)) {                                                                                     \
      ivs_test_diag("check failed at line %d: %s", __LINE__, #cond);                                   \
      goto done;                                                                                       \
    }                                                                                                  \
  } while (0)

#define DS_POISON 0x3Cu
#define DS_ZERO_GROM_BYTES 2048u
#define DS_TARGET 150000u    /* the run-to bound of the scenarios */
#define DS_E0 59736u         /* the end of frame 0 (hand-derived: 262 lines x 57 cycles x 4 ticks, docs/video) */
#define DS_E1 119472u        /* the end of frame 1 */
#define DS_MARGIN 4000u      /* the margin of the cycle budget, both sides */
#define DS_MAX_CALLS 140000u /* the slice-1 run takes about 88,500 calls; a faulted core must not loop forever */
#define DS_MAX_FRAMES 4u
#define DS_SLOT_MAX 16384u   /* an upper bound of one slot, a sanity check of ivs_info (the real slot is 15,296) */
#define DS_RAM_BASE 0x0200u
#define DS_RAM_WORDS 0x0160u /* 0200-035F, the system RAM of the profile */
#define DS_DOMAINS 9u
#define DS_W 159u
#define DS_H 96u
#define DS_STROBE_ADDR 0x500Du /* the address word of MVO R0, $0020 in the listing (the display-enable strobe) */
#define DS_RESERVED_REG 0x0022u

/* ---- the predicates, from the two .expect files ---- */
#define DS_HALTED 0u
#define DS_MEM 1u
#define DS_FDISP 2u
#define DS_FPIXEL 3u
#define DS_FHASH 4u
#define DS_VCOUNT 5u

typedef struct ds_clause {
  uint32_t kind;
  uint32_t a; /* address, frame index or count */
  uint32_t b; /* expected value, x */
  uint32_t c; /* y */
  uint32_t d; /* pixel value */
  const char *hash;
} ds_clause;

#define IVS_EXPECT_HALTED(y) {DS_HALTED, 0u, (uint32_t)(y), 0u, 0u, NULL},
#define IVS_EXPECT_MEM(a, v) {DS_MEM, (uint32_t)(a), (uint32_t)(v), 0u, 0u, NULL},
#define IVS_EXPECT_FRAME_DISPLAYED(n, f) {DS_FDISP, (n), (uint32_t)(f), 0u, 0u, NULL},
#define IVS_EXPECT_FRAME_PIXEL(n, x, y, v) {DS_FPIXEL, (n), (x), (y), (v), NULL},
#define IVS_EXPECT_FRAME_HASH(n, h) {DS_FHASH, (n), 0u, 0u, 0u, (h)},
#define IVS_EXPECT_VIDEO_COUNT(n) {DS_VCOUNT, (n), 0u, 0u, 0u, NULL},
static const ds_clause ds_expect_fgbg[] = {
#include "diag_stic_fgbg_pass_expect.inc"
};
static const ds_clause ds_expect_cstack[] = {
#include "diag_stic_cstack_pass_expect.inc"
};
#undef IVS_EXPECT_HALTED
#undef IVS_EXPECT_MEM
#undef IVS_EXPECT_FRAME_DISPLAYED
#undef IVS_EXPECT_FRAME_PIXEL
#undef IVS_EXPECT_FRAME_HASH
#undef IVS_EXPECT_VIDEO_COUNT
#define DS_N_FGBG (sizeof ds_expect_fgbg / sizeof ds_expect_fgbg[0])
#define DS_N_CSTACK (sizeof ds_expect_cstack / sizeof ds_expect_cstack[0])

static const uint32_t ds_domains[DS_DOMAINS] = {IVS_DIGEST_COMPOSITE, IVS_DIGEST_CPU,   IVS_DIGEST_MEM,
                                                IVS_DIGEST_IMAGES,    IVS_DIGEST_INSTANCE, IVS_DIGEST_VIDEO,
                                                IVS_DIGEST_AUDIO,     IVS_DIGEST_INPUT,    IVS_DIGEST_PSGLOG};

/* One run's observations (static: the cases run one after another). */
typedef struct ds_obs {
  uint8_t vid[DS_MAX_FRAMES * DS_SLOT_MAX];
  uint64_t vid_n;
  uint64_t frames;     /* the sum of video_frames_completed */
  uint64_t slot_bytes; /* video_slot_bytes of ivs_info */
  uint16_t ram[DS_RAM_WORDS];
  uint8_t dig[DS_DOMAINS][32];
  uint8_t storage[IVS_MACHINE_BYTES];
  uint64_t now;
  uint32_t stop;
  uint32_t run_state;
  uint32_t stops[8];
  uint32_t n_stops;
  uint64_t calls;
} ds_obs;

typedef struct ds_plan {
  uint32_t keys;   /* controller word 0 from tick 0 (0 none, IVS_CTRL_KEY_1 for Color Stack mode) */
  uint64_t target;
  uint64_t slice;  /* 0: one call to the target */
  uint32_t flags;  /* IVS_RUN_F_* added to IVS_RUN_F_STOP_ON_HALT */
  const uint8_t *image; /* the cartridge container, NULL for the packed diagnostic */
  size_t image_len;
} ds_plan;

static ds_obs ds_o_one;
static ds_obs ds_o_two;
static ds_obs ds_o_ref;

/* ---- helpers ---- */

static uint8_t *ds_copy(const uint8_t *src, size_t n)
{
  uint8_t *b = malloc(n);
  if (b != NULL) {
    memcpy(b, src, n);
  }
  return b;
}

/* A powered instance: stub EXEC, zero GROM, the cartridge image, a power-on reset. 1 on success. */
static int ds_powered(ivs_ms_inst *x, const uint8_t *image, size_t image_len)
{
  ivs_status st;
  if (!ivs_ms_new(x, DS_POISON)) {
    return 0;
  }
  if (!ivs_ms_load(x->m, IVS_KIND_EXEC, ds_copy(ivs_img_boot_stub, IVS_IMG_BOOT_STUB_BYTES), IVS_IMG_BOOT_STUB_BYTES) ||
      !ivs_ms_load(x->m, IVS_KIND_GROM, calloc(DS_ZERO_GROM_BYTES, 1u), DS_ZERO_GROM_BYTES) ||
      !ivs_ms_load(x->m, IVS_KIND_CARTRIDGE, ds_copy(image, image_len), image_len)) {
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

/* Runs the plan on a fresh instance and fills *o. The video buffer of every call is sized from ivs_info by the capacity rule
 * (slot_bytes x (floor(D / F) + 1 + slack)), or exactly one slot for a flagged frame stop. 1 on success. */
static int ds_run(const ds_plan *p, ds_obs *o)
{
  ivs_ms_inst x;
  ivs_info info;
  ivs_cpu_state_report c;
  ivs_input_event ev;
  uint8_t *buf = NULL;
  uint64_t calls = 0u;
  uint32_t consumed = 0u;
  uint32_t i;
  int rc = 0;
  int up = 0;
  memset(o, 0, sizeof *o);
  if (!ds_powered(&x, p->image != NULL ? p->image : ivs_img_diag_stic_1,
                  p->image != NULL ? p->image_len : (size_t)IVS_IMG_DIAG_STIC_1_BYTES)) {
    return 0;
  }
  up = 1;
  memset(&info, 0, sizeof info);
  info.struct_size = (uint32_t)sizeof info;
  DS_CHECK(ivs_machine_info(x.m, &info) == IVS_OK);
  DS_CHECK(info.video_format == IVS_OUTPUT_FORMAT_INDEX8_FRAMES_V1);
  DS_CHECK(info.video_width == DS_W && info.video_height == DS_H);
  DS_CHECK(info.video_header_bytes == IVS_FRAME_HEADER_BYTES);
  DS_CHECK(info.video_slot_bytes == info.video_header_bytes + info.video_width * info.video_height);
  DS_CHECK(info.video_slot_bytes <= DS_SLOT_MAX && info.nominal_frame_ticks == DS_E0);
  o->slot_bytes = info.video_slot_bytes;
  {
    uint64_t max_slots = p->target / info.nominal_frame_ticks + 1u + info.video_slack_frames;
    DS_CHECK(max_slots <= DS_MAX_FRAMES + 8u);
    buf = malloc((size_t)(max_slots * info.video_slot_bytes));
    DS_CHECK(buf != NULL);
  }
  memset(&ev, 0, sizeof ev);
  ev.at_tick = 0u;
  ev.controller[0] = p->keys;
  for (;;) {
    ivs_run_request q;
    ivs_run_report rep;
    ivs_info now_info;
    uint64_t t = p->target;
    uint64_t before;
    uint64_t slots;
    memset(&now_info, 0, sizeof now_info);
    now_info.struct_size = (uint32_t)sizeof now_info;
    DS_CHECK(ivs_machine_info(x.m, &now_info) == IVS_OK);
    before = now_info.time_ticks;
    if (before >= p->target) {
      break;
    }
    calls++;
    DS_CHECK(calls <= DS_MAX_CALLS);
    if (p->slice != 0u && before + p->slice < t) {
      t = before + p->slice;
    }
    slots = (t - before) / info.nominal_frame_ticks + 1u + info.video_slack_frames;
    if ((p->flags & IVS_RUN_F_STOP_ON_FRAME) != 0u) {
      slots = 1u; /* a frame stop needs exactly one slot, whatever the target */
    }
    memset(&q, 0, sizeof q);
    q.struct_size = (uint32_t)sizeof q;
    q.flags = IVS_RUN_F_STOP_ON_HALT | p->flags;
    q.target_ticks = t;
    q.video = buf;
    q.video_capacity = slots * info.video_slot_bytes;
    /* the keys event only when the plan has keys and it is not applied yet */
    q.events = (p->keys != 0u && consumed == 0u) ? &ev : NULL;
    q.event_count = (p->keys != 0u && consumed == 0u) ? 1u : 0u;
    memset(&rep, 0, sizeof rep);
    rep.struct_size = (uint32_t)sizeof rep;
    DS_CHECK(ivs_machine_run(x.m, &q, &rep) == IVS_OK);
    consumed += rep.input_consumed;
    DS_CHECK(o->vid_n + rep.video_bytes_written <= sizeof o->vid);
    DS_CHECK(rep.video_bytes_written == rep.video_frames_completed * info.video_slot_bytes);
    memcpy(o->vid + o->vid_n, buf, (size_t)rep.video_bytes_written);
    o->vid_n += rep.video_bytes_written;
    o->frames += rep.video_frames_completed;
    o->now = rep.now_ticks;
    o->stop = rep.stop_reason;
    if (o->n_stops < 8u && (o->n_stops == 0u || o->stops[o->n_stops - 1u] != rep.stop_reason ||
                            rep.stop_reason == IVS_STOP_FRAME)) {
      o->stops[o->n_stops++] = rep.stop_reason;
    }
    DS_CHECK(rep.now_ticks > before || rep.stop_reason == IVS_STOP_CPU_HALTED);
    if (rep.stop_reason == IVS_STOP_CPU_HALTED) {
      break;
    }
    DS_CHECK(rep.stop_reason == IVS_STOP_TARGET || rep.stop_reason == IVS_STOP_FRAME);
  }
  o->calls = calls;
  DS_CHECK(ivs_machine_peek(x.m, (uint16_t)DS_RAM_BASE, DS_RAM_WORDS, o->ram) == IVS_OK);
  memset(&c, 0, sizeof c);
  c.struct_size = (uint32_t)sizeof c;
  DS_CHECK(ivs_machine_cpu_state(x.m, &c) == IVS_OK);
  o->run_state = c.run_state;
  for (i = 0u; i < DS_DOMAINS; i++) {
    DS_CHECK(ivs_machine_state_digest(x.m, ds_domains[i], o->dig[i]) == IVS_OK);
  }
  memcpy(o->storage, x.storage, IVS_MACHINE_BYTES);
  rc = 1;
done:
  free(buf);
  if (up) {
    ivs_ms_free(&x);
  }
  return rc;
}

static uint64_t ds_be64(const uint8_t *b)
{
  uint64_t v = 0u;
  int i;
  for (i = 0; i < 8; i++) {
    v = (v << 8) | b[i];
  }
  return v;
}

static uint32_t ds_be32(const uint8_t *b)
{
  return ((uint32_t)b[0] << 24) | ((uint32_t)b[1] << 16) | ((uint32_t)b[2] << 8) | b[3];
}

static const uint8_t *ds_slot(const ds_obs *o, uint32_t k)
{
  return o->vid + (size_t)k * (size_t)o->slot_bytes;
}

static uint32_t ds_hexval(char ch)
{
  return ch >= 'A' ? (uint32_t)(ch - 'A') + 10u : (uint32_t)(ch - '0');
}

/* The canonical text form of the plane of slot k: DS_H lines of DS_W upper-case hex digits and an LF, hashed to 64 lower-case hex digits
 * (built here, independent of the core and of the runner). 1 when every position is a color. */
static int ds_text_sha(const uint8_t *slot, char *hex)
{
  static uint8_t text[DS_H * (DS_W + 1u)];
  uint8_t d[32];
  size_t n = 0u;
  uint32_t y;
  uint32_t x;
  for (y = 0u; y < DS_H; y++) {
    for (x = 0u; x < DS_W; x++) {
      uint8_t v = slot[IVS_FRAME_HEADER_BYTES + y * DS_W + x];
      if (v > IVS_PIXEL_COLOR_MAX) {
        return 0;
      }
      text[n++] = (uint8_t)"0123456789ABCDEF"[v];
    }
    text[n++] = (uint8_t)'\n';
  }
  ivs_sha256_digest(text, n, d);
  for (x = 0u; x < 32u; x++) {
    hex[2u * x] = "0123456789abcdef"[d[x] >> 4];
    hex[2u * x + 1u] = "0123456789abcdef"[d[x] & 15u];
  }
  hex[64] = '\0';
  return 1;
}

/* The header and the plane of frame 0 against a literal (index 0 is F-D1, 1 is F-D2): tick E_0, index 0, geometry, displayed flag,
 * reserved word, and every one of the 159 x 96 positions. 1 when equal. */
static int ds_frame_matches(const ds_obs *o, uint32_t lit)
{
  const uint8_t *s = ds_slot(o, 0u);
  uint32_t y;
  uint32_t x;
  if (o->frames != 1u || o->vid_n != o->slot_bytes) {
    ivs_test_diag("%llu frames, %llu bytes delivered; want one frame", (unsigned long long)o->frames,
                  (unsigned long long)o->vid_n);
    return 0;
  }
  if (ds_be64(s) != DS_E0 || ds_be64(s + 8) != 0u || ds_be32(s + 16) != DS_W || ds_be32(s + 20) != DS_H ||
      ds_be32(s + 24) != IVS_FRAME_F_DISPLAYED || ds_be32(s + 28) != 0u) {
    ivs_test_diag("slot header: tick %llu index %llu width %u height %u flags %u reserved %u", (unsigned long long)ds_be64(s),
                  (unsigned long long)ds_be64(s + 8), (unsigned)ds_be32(s + 16), (unsigned)ds_be32(s + 20),
                  (unsigned)ds_be32(s + 24), (unsigned)ds_be32(s + 28));
    return 0;
  }
  for (y = 0u; y < DS_H; y++) {
    for (x = 0u; x < DS_W; x++) {
      uint32_t want = ds_hexval(ds_frames[lit].plane[y][x]);
      uint32_t have = s[IVS_FRAME_HEADER_BYTES + y * DS_W + x];
      if (want != have) {
        ivs_test_diag("%s: pixel (%u,%u) expected %u, delivered %u", ds_frames[lit].name, (unsigned)x, (unsigned)y,
                      (unsigned)want, (unsigned)have);
        return 0;
      }
    }
  }
  return 1;
}

/* Every clause of a predicate against the observations; the frame clauses read slot 0 (and its text hash). 1 when all hold. */
static int ds_clauses_hold(const ds_obs *o, const ds_clause *e, size_t n, uint32_t lit)
{
  char hex[65];
  size_t i;
  uint32_t nframe = 0u;
  for (i = 0u; i < n; i++) {
    const ds_clause *k = &e[i];
    const uint8_t *s = ds_slot(o, 0u);
    switch (k->kind) {
    case DS_HALTED:
      if ((o->run_state == IVS_CPU_STATE_HALTED) != (k->b != 0u)) {
        ivs_test_diag("halted: expected %u, run state %u", (unsigned)k->b, (unsigned)o->run_state);
        return 0;
      }
      break;
    case DS_MEM:
      if (k->a < DS_RAM_BASE || k->a >= DS_RAM_BASE + DS_RAM_WORDS || o->ram[k->a - DS_RAM_BASE] != k->b) {
        ivs_test_diag("mem %04X: expected %04X, actual %04X", (unsigned)k->a, (unsigned)k->b,
                      k->a >= DS_RAM_BASE && k->a < DS_RAM_BASE + DS_RAM_WORDS ? (unsigned)o->ram[k->a - DS_RAM_BASE] : 0u);
        return 0;
      }
      break;
    case DS_FDISP:
      if (k->a != 0u || o->frames < 1u || ((ds_be32(s + 24) & IVS_FRAME_F_DISPLAYED) != 0u) != (k->b != 0u)) {
        ivs_test_diag("frame %u displayed flag differs from the expectation %u", (unsigned)k->a, (unsigned)k->b);
        return 0;
      }
      nframe++;
      break;
    case DS_FPIXEL:
      if (k->a != 0u || o->frames < 1u || k->b >= DS_W || k->c >= DS_H ||
          s[IVS_FRAME_HEADER_BYTES + k->c * DS_W + k->b] != k->d) {
        ivs_test_diag("frame %u pixel (%u,%u): expected %u, delivered %u", (unsigned)k->a, (unsigned)k->b, (unsigned)k->c,
                      (unsigned)k->d,
                      k->b < DS_W && k->c < DS_H ? (unsigned)s[IVS_FRAME_HEADER_BYTES + k->c * DS_W + k->b] : 255u);
        return 0;
      }
      nframe++;
      break;
    case DS_FHASH:
      if (k->a != 0u || o->frames < 1u || !ds_text_sha(s, hex) || strcmp(hex, k->hash) != 0 ||
          strcmp(k->hash, ds_frames[lit].sha) != 0) {
        ivs_test_diag("frame %u hash: expected %s, literal %s", (unsigned)k->a, k->hash, ds_frames[lit].sha);
        return 0;
      }
      nframe++;
      break;
    case DS_VCOUNT:
      if (o->frames != k->a) {
        ivs_test_diag("video count: expected %u, delivered %llu", (unsigned)k->a, (unsigned long long)o->frames);
        return 0;
      }
      break;
    default:
      return 0;
    }
  }
  if (nframe < 4u) {
    ivs_test_diag("the predicate holds only %u frame clauses", (unsigned)nframe);
    return 0;
  }
  return 1;
}

/* 1 when the halt came inside the margins of the cycle budget: after E_0 + 4,000 and before E_1 - 4,000 ticks. */
static int ds_inside_margins(const ds_obs *o)
{
  if (o->stop != IVS_STOP_CPU_HALTED || o->now < DS_E0 + DS_MARGIN || o->now > DS_E1 - DS_MARGIN) {
    ivs_test_diag("stop %u at %llu ticks, outside %u .. %u", (unsigned)o->stop, (unsigned long long)o->now,
                  (unsigned)(DS_E0 + DS_MARGIN), (unsigned)(DS_E1 - DS_MARGIN));
    return 0;
  }
  return 1;
}

/* 1 when two runs agree on the whole instance (bytes and every digest), the delivered frames, the guest RAM and machine time. */
static int ds_same(const ds_obs *a, const ds_obs *b, const char *what)
{
  if (memcmp(a->storage, b->storage, sizeof a->storage) != 0) {
    ivs_test_diag("%s: the instance bytes differ", what);
    return 0;
  }
  if (a->vid_n != b->vid_n || memcmp(a->vid, b->vid, (size_t)a->vid_n) != 0 || a->frames != b->frames) {
    ivs_test_diag("%s: the delivered frames differ (%llu versus %llu bytes)", what, (unsigned long long)a->vid_n,
                  (unsigned long long)b->vid_n);
    return 0;
  }
  if (memcmp(a->ram, b->ram, sizeof a->ram) != 0 || memcmp(a->dig, b->dig, sizeof a->dig) != 0 || a->now != b->now ||
      a->run_state != b->run_state) {
    ivs_test_diag("%s: guest RAM, a digest, the end time or the run state differ", what);
    return 0;
  }
  return 1;
}

static ds_plan ds_plan_of(uint32_t keys, uint64_t slice, uint32_t flags)
{
  ds_plan p;
  memset(&p, 0, sizeof p);
  p.keys = keys;
  p.target = DS_TARGET;
  p.slice = slice;
  p.flags = flags;
  return p;
}

/* ---- cases ---- */

static int ds_fgbg(void)
{
  ds_plan p = ds_plan_of(0u, 0u, 0u);
  if (!ds_run(&p, &ds_o_one)) {
    return IVS_TEST_FAIL;
  }
  if (!ds_inside_margins(&ds_o_one) || !ds_clauses_hold(&ds_o_one, ds_expect_fgbg, DS_N_FGBG, 0u) ||
      !ds_frame_matches(&ds_o_one, 0u)) {
    return IVS_TEST_FAIL;
  }
  return IVS_TEST_PASS;
}

static int ds_cstack(void)
{
  ds_plan p = ds_plan_of(IVS_CTRL_KEY_1, 0u, 0u);
  ds_plan q = ds_plan_of(0u, 0u, 0u);
  if (!ds_run(&p, &ds_o_one) || !ds_run(&q, &ds_o_two)) {
    return IVS_TEST_FAIL;
  }
  if (!ds_inside_margins(&ds_o_one) || !ds_clauses_hold(&ds_o_one, ds_expect_cstack, DS_N_CSTACK, 1u) ||
      !ds_frame_matches(&ds_o_one, 1u)) {
    return IVS_TEST_FAIL;
  }
  /* the same image, the other mode: the two frames differ and the recorded port byte differs (007E versus 00FF) */
  if (memcmp(ds_slot(&ds_o_one, 0u), ds_slot(&ds_o_two, 0u), (size_t)ds_o_one.slot_bytes) == 0 ||
      ds_o_one.ram[0x02F1u - DS_RAM_BASE] == ds_o_two.ram[0x02F1u - DS_RAM_BASE]) {
    ivs_test_diag("the two modes gave the same frame or the same recorded port byte");
    return IVS_TEST_FAIL;
  }
  return IVS_TEST_PASS;
}

static int ds_slice_invariant(void)
{
  static const uint64_t slices[] = {1u, 7u, 468u};
  uint32_t keys[2] = {IVS_CTRL_KEY_1, 0u};
  size_t m;
  size_t i;
  for (m = 0u; m < 2u; m++) {
    ds_plan p = ds_plan_of(keys[m], 0u, 0u);
    if (!ds_run(&p, &ds_o_ref)) {
      return IVS_TEST_FAIL;
    }
    for (i = 0u; i < sizeof slices / sizeof slices[0]; i++) {
      ds_plan s = ds_plan_of(keys[m], slices[i], 0u);
      /* the slice-1 run is made once, for the Color Stack mode (it also covers the tick-0 event) */
      if (slices[i] == 1u && m != 0u) {
        continue;
      }
      if (!ds_run(&s, &ds_o_one)) {
        return IVS_TEST_FAIL;
      }
      if (!ds_same(&ds_o_ref, &ds_o_one, "sliced versus one call") || ds_o_one.calls <= ds_o_ref.calls) {
        return IVS_TEST_FAIL;
      }
    }
  }
  return IVS_TEST_PASS;
}

static int ds_frame_stop(void)
{
  ds_plan p = ds_plan_of(0u, 0u, 0u);
  ds_plan f = ds_plan_of(0u, 0u, IVS_RUN_F_STOP_ON_FRAME);
  if (!ds_run(&p, &ds_o_ref) || !ds_run(&f, &ds_o_one)) {
    return IVS_TEST_FAIL;
  }
  /* the flagged calls stop at the frame end and again at the halt: FRAME then CPU_HALTED */
  if (ds_o_one.n_stops != 2u || ds_o_one.stops[0] != IVS_STOP_FRAME || ds_o_one.stops[1] != IVS_STOP_CPU_HALTED ||
      ds_o_one.calls != 2u) {
    ivs_test_diag("%u stops over %llu calls; want FRAME then CPU_HALTED over two calls", (unsigned)ds_o_one.n_stops,
                  (unsigned long long)ds_o_one.calls);
    return IVS_TEST_FAIL;
  }
  if (!ds_same(&ds_o_ref, &ds_o_one, "frame stop versus one call")) {
    return IVS_TEST_FAIL;
  }
  return IVS_TEST_PASS;
}

static void ds_put16(uint8_t *p, uint32_t v)
{
  p[0] = (uint8_t)((v >> 8) & 0xFFu);
  p[1] = (uint8_t)(v & 0xFFu);
}

static int ds_nodisplay(void)
{
  static uint8_t img[IVS_IMG_DIAG_STIC_1_BYTES];
  ds_plan p = ds_plan_of(0u, 0u, 0u);
  const uint8_t *s;
  uint32_t y;
  uint32_t x;
  /* The container of tools/pack: 32 header bytes, then one big-endian 16-bit word per listing word from the entry 5000, then the
   * SHA-256 of everything before it. The word at 500D (the address of the strobe store) becomes 0022, a reserved register. */
  memcpy(img, ivs_img_diag_stic_1, sizeof img);
  if (sizeof img < 32u + 2u * 0x85u + 32u) {
    return IVS_TEST_FAIL;
  }
  if (img[32u + 2u * (DS_STROBE_ADDR - 0x5000u)] != 0u || img[32u + 2u * (DS_STROBE_ADDR - 0x5000u) + 1u] != 0x20u) {
    ivs_test_diag("the word at %04X is not 0020 in the packed image", (unsigned)DS_STROBE_ADDR);
    return IVS_TEST_FAIL;
  }
  ds_put16(img + 32u + 2u * (DS_STROBE_ADDR - 0x5000u), DS_RESERVED_REG);
  ivs_sha256_digest(img, sizeof img - 32u, img + sizeof img - 32u);
  p.image = img;
  p.image_len = sizeof img;
  if (!ds_run(&p, &ds_o_one)) {
    return IVS_TEST_FAIL;
  }
  /* the guest still halts inside the margins and reaches the same RAM state (the stack registers, the table and the words are
   * written whatever the strobe does); the frame is delivered, not displayed, filled with the border color 0 */
  if (!ds_inside_margins(&ds_o_one) || ds_o_one.frames != 1u || ds_o_one.ram[0x02F0u - DS_RAM_BASE] != 0x03C5u ||
      ds_o_one.ram[0x02A3u - DS_RAM_BASE] != 0x2A01u || ds_o_one.ram[0x02EFu - DS_RAM_BASE] != 0x1C0Fu) {
    ivs_test_diag("nodisplay: stop, frame count or guest state differs from the expectation");
    return IVS_TEST_FAIL;
  }
  s = ds_slot(&ds_o_one, 0u);
  if (ds_be64(s) != DS_E0 || ds_be32(s + 24) != 0u) {
    ivs_test_diag("nodisplay: frame 0 tick %llu flags %u; want %u and 0", (unsigned long long)ds_be64(s),
                  (unsigned)ds_be32(s + 24), (unsigned)DS_E0);
    return IVS_TEST_FAIL;
  }
  for (y = 0u; y < DS_H; y++) {
    for (x = 0u; x < DS_W; x++) {
      if (s[IVS_FRAME_HEADER_BYTES + y * DS_W + x] != 0u) {
        ivs_test_diag("nodisplay: pixel (%u,%u) is %u, not the border color 0", (unsigned)x, (unsigned)y,
                      (unsigned)s[IVS_FRAME_HEADER_BYTES + y * DS_W + x]);
        return IVS_TEST_FAIL;
      }
    }
  }
  /* without the strobe no bus window stalls the CPU: the stop is earlier than in the display-on run of the same image */
  p = ds_plan_of(0u, 0u, 0u);
  if (!ds_run(&p, &ds_o_two) || ds_o_two.frames != 1u || ds_o_one.now >= ds_o_two.now) {
    ivs_test_diag("nodisplay: the stop %llu is not earlier than the display-on stop %llu", (unsigned long long)ds_o_one.now,
                  (unsigned long long)ds_o_two.now);
    return IVS_TEST_FAIL;
  }
  return IVS_TEST_PASS;
}

const ivs_test_case ivs_run_diag_stic_cases[] = {
    {"run.diag.stic.fgbg", ds_fgbg, NULL},
    {"run.diag.stic.cstack", ds_cstack, NULL},
    {"run.diag.stic.slice-invariant", ds_slice_invariant, NULL},
    {"run.diag.stic.frame-stop", ds_frame_stop, NULL},
    {"run.diag.stic.nodisplay", ds_nodisplay, NULL},
};
const size_t ivs_run_diag_stic_count = sizeof ivs_run_diag_stic_cases / sizeof ivs_run_diag_stic_cases[0];

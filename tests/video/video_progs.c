/* SPDX-License-Identifier: MIT */
/* Suite video: the run harness over the internal step seam and the stand-in guests (plan 06-09). Every guest word is derived in the
 * comments of tests/stic/stic_cases.h (MVII, MVO, DIS, B self); the strobe guest is the Task 1 stand-in of the plan: at most 24 words. */
#include <stdlib.h>
#include <string.h>

#include "video_cases.h"

int vp_new(vp_run *r, const uint16_t *w, size_t n, uint64_t cap)
{
  memset(r, 0, sizeof *r);
  if (!sc_guest(&r->x, w, n, 0u)) {
    return 0;
  }
  if (cap != 0u) {
    r->buf = (uint8_t *)malloc((size_t)cap);
    if (r->buf == NULL) {
      ivs_ms_free(&r->x);
      return 0;
    }
    memset(r->buf, (int)VP_POISON, (size_t)cap);
    r->cap = cap;
  }
  ivs_psg_sink_init(&r->sink, NULL, 0u);
  ivs_psg_sink_video(&r->sink, r->buf, r->cap);
  return 1;
}

const struct ivs_machine *vp_im(const vp_run *r)
{
  return ivs_machine_from_handle(r->x.m);
}

int vp_until(vp_run *r, uint64_t ticks)
{
  const struct ivs_machine *im = vp_im(r);
  uint32_t n = 0u;
  while (im->time_ticks < ticks) {
    ivs_cp1610_result res;
    if (n++ >= 400000u || ivs_machine_step_unit_sink_internal(r->x.m, &res, &r->sink) != IVS_OK) {
      return 0;
    }
  }
  return 1;
}

void vp_free(vp_run *r)
{
  free(r->buf);
  r->buf = NULL;
  ivs_ms_free(&r->x);
}

size_t vp_prog_off(uint16_t *w)
{
  return sc_prog_loop(w, 0u);
}

size_t vp_prog_border(uint16_t *w, uint32_t value)
{
  size_t n = sc_w_mvii(w, 0u, 0u, value);
  n = sc_w_mvo(w, n, 0u, 0x002Cu);
  w[n++] = 0x0003u; /* DIS */
  return sc_w_loop(w, n);
}

size_t vp_prog_strobe_once(uint16_t *w)
{
  size_t n = sc_w_mvo(w, 0u, 0u, 0x0020u);
  w[n++] = 0x0003u; /* DIS */
  return sc_w_loop(w, n);
}

/* MVI $0021, R1 is the two decles 0281 0021 (MVI at 0280 + d with d = 1, then the address, tests/stic/stic_cases.h); the read lands inside
 * the register window of frame 0 and selects Color Stack. Then MVO R0, $0020 (0240 0020), DIS (0003) and B self (0220 0001): 8 words. */
size_t vp_prog_cs_strobe_once(uint16_t *w)
{
  size_t n = sc_w_mvi(w, 0u, 0x0021u, 1u);
  n = sc_w_mvo(w, n, 0u, 0x0020u);
  w[n++] = 0x0003u; /* DIS */
  return sc_w_loop(w, n);
}

/* The read once, then MVO R0, $0020 and B back to the MVO (0220 then 2: the branch displacement of sc_w_bback, from the word after the branch
 * decle): the strobe repeats in every frame, so every frame is displayed. */
size_t vp_prog_cs_strobe_loop(uint16_t *w)
{
  size_t n = sc_w_mvi(w, 0u, 0x0021u, 1u);
  size_t start = n;
  n = sc_w_mvo(w, n, 0u, 0x0020u);
  w[n] = 0x0220u;
  w[n + 1u] = (uint16_t)(n + 1u - start);
  return n + 2u;
}

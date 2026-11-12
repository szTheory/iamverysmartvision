/* SPDX-License-Identifier: MIT */
/* Suite input, family single (INP-01, plan 05-05): single controls and the wiki anchors.
 *   input.anchor.wiki-key1   KEY_1 pressed alone on port 0 reads $007E at $01FE: through a guest
 *                            program (stand-in PORTS, which stores each port read into RAM) and through
 *                            ivs_machine_peek; the other port reads $00FF. The value $7E is the wiki
 *                            statement of the literal header (key 1 clears bits 0 and 7), written here as
 *                            a literal, from no table. */
#include <stdint.h>
#include <string.h>

#include "input_cases.h"

/* One powered instance running the PORTS program, one event of word w0 on port 0 and w1 on port 1 at
 * tick 0, run for II_RUN_TICKS. The guest read of port 0 is left in *g0 and of port 1 in *g1, the
 * peeks of $01FE and $01FF in *p0 and *p1. */
static int is_run_pair(uint32_t w0, uint32_t w1, uint32_t *g0, uint32_t *g1, uint32_t *p0, uint32_t *p1)
{
  ivs_ms_inst x;
  ivs_input_event ev;
  ivs_run_report rep;
  int ok = 0;
  if (!ivs_ms_powered_prog(&x, IVS_MS_PROG_PORTS, II_POISON)) {
    return 0;
  }
  ii_event(&ev, 0u, w0, w1);
  II_CHECK(ii_run(x.m, II_RUN_TICKS, &ev, 1u, &rep) == IVS_OK);
  II_CHECK(rep.input_consumed == 1u && rep.stop_reason == IVS_STOP_TARGET);
  *g0 = ii_peek(x.m, 0x0200u);
  *g1 = ii_peek(x.m, 0x0201u);
  *p0 = ii_peek(x.m, 0x01FEu);
  *p1 = ii_peek(x.m, 0x01FFu);
  ok = 1;
done:
  ivs_ms_free(&x);
  return ok;
}

static int is_anchor_wiki_key1(void)
{
  uint32_t g0;
  uint32_t g1;
  uint32_t p0;
  uint32_t p1;
  if (!is_run_pair(IVS_CTRL_KEY_1, 0u, &g0, &g1, &p0, &p1)) {
    return IVS_TEST_FAIL;
  }
  if (g0 != 0x007Eu || p0 != 0x007Eu || g1 != 0x00FFu || p1 != 0x00FFu) {
    ivs_test_diag("KEY_1 on port 0: guest %04X %04X, peek %04X %04X, want 007E 00FF", (unsigned)g0,
                  (unsigned)g1, (unsigned)p0, (unsigned)p1);
    return IVS_TEST_FAIL;
  }
  return IVS_TEST_PASS;
}

const ivs_test_case ivs_input_single_cases[] = {
    {"input.anchor.wiki-key1", is_anchor_wiki_key1, NULL},
};

const size_t ivs_input_single_count = sizeof ivs_input_single_cases / sizeof ivs_input_single_cases[0];

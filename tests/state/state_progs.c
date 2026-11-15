/* SPDX-License-Identifier: MIT */
/* Suite state, hard points (plan 08-14, D-16, D-17): the programs the named cuts of data/expected/state-cuts.ids run, the cuts themselves
 * (read through the generated include state_cuts.inc, so a pinned tick is never retyped, T-08-35) and the end tick of the uninterrupted
 * reference of each cut.
 * The stand-in programs are lists of 10-bit decles loaded at $1000 (docs/state/hard-points.md "The stand-in programs": each decle derived
 * by hand from the printed encodings of data/cp1610/opcode-families.json and checked against the vectors and an independent assembler
 * build before any core code ran). The scenario runs (Lamp Tone idle and with its six events, the second STIC diagnostic with the
 * collision selector) are the builders of state_util.c. Nothing here reads the code under test to learn what it should do.
 *
 * End ticks E (the arithmetic; a frame is 59,736 ticks = 14,934 CPU cycles of 4 ticks, frame k ends at 59,736 (k + 1)):
 *   HP-SDBD       1,000      the program reaches its final B self at cycle 54 (reset entry 5 + MVII 8 + MVO 11 + SDBD 4 + the extended MVII 10
 *                            + MVI 10 + ADDR 6), tick 216, then loops 9 cycles per branch: 1,000 ticks hold about 20 more units
 *   HP-HLT, HP-HALTED 70,000 the halt comes at cycle 103 (tick 412) and the first bus window of frame 0 starts at cycle 3,791 (tick 15,164):
 *                            frame 0 ends at 59,736 and 70,000 is 10,264 idle ticks past it, so a slot is delivered and windows are granted
 *   HP-FAULTED    1,540      the fault boundary (tick 540) plus 1,000
 *   HP-RESETPEND  119,472    two frames of the Lamp Tone after the reset
 *   the lamp cuts, HP-LATCH, HP-MOB 238,944   four frames: the cuts lie in frame 1 or 2, so two or more frames complete after the cut
 *   HP-PSG-MID    238,944    four frames (the cut is at tick 66,336: the first reload of the tone counters is at tick 65,536, see docs/state/hard-points.md)
 *   HP-ENV-MID, HP-LOG 119,472    two frames
 * A cut at or after its end tick is a defect of this table, and the witness ids say so. */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "ivs/ivs.h"
#include "machine/machine.h"
#include "state_cases.h"
#include "state_util.h"

/* P-SDBD (13 words): MVII #$AB, R1; MVO R1, $0100; SDBD; MVII #$1234, R2 (extended); MVI $0100, R3; ADDR R3, R2; B self. */
static const uint16_t sc_p_sdbd[] = {0x02B9u, 0x00ABu, 0x0241u, 0x0100u, 0x0001u, 0x02BAu, 0x0034u,
                                     0x0012u, 0x0283u, 0x0100u, 0x00DAu, 0x0220u, 0x0001u};

/* P-HLT (14 words): MVII #3, R1; loop: MVO R0, $0020; DECR R1; BNZE loop; HLT; BZE skip; MVII #$FF, R2; skip: B self. */
static const uint16_t sc_p_hlt[] = {0x02B9u, 0x0003u, 0x0240u, 0x0020u, 0x0011u, 0x022Cu, 0x0004u,
                                    0x0000u, 0x0204u, 0x0002u, 0x02BAu, 0x00FFu, 0x0220u, 0x0001u};

/* P-FAULT (6 words): MVII #8, R1; loop: DECR R1; BNZE loop; MVO@ R4, R4 (a refused encoding: a sticky guest fault). */
static const uint16_t sc_p_fault[] = {0x02B9u, 0x0008u, 0x0011u, 0x022Cu, 0x0002u, 0x0264u};

/* P-LATCH (35 words): MVI $0021, R1 (the read selects Color Stack); SDBD, MVII #$0801, R2; MVO R2, $02F0; SDBD, MVII #$2000, R1; MVO R1, $0201;
 * SDBD, MVII #$3800, R5; MVII #$FF, R1; MVO@ R1, R5 (GRAM card 0); loop: MVI $02F0, R2; XORI #3, R2; MVO R2, $02F0; MVO R2, $0200 (BACKTAB word 0);
 * MVO R0, $0020 (the display strobe); MVI $0000, R3 (a register read, blocked outside the window); B loop. The loop is 70 cycles. */
static const uint16_t sc_p_latch[] = {0x0281u, 0x0021u, 0x0001u, 0x02BAu, 0x0001u, 0x0008u, 0x0242u, 0x02F0u, 0x0001u, 0x02B9u, 0x0000u, 0x0020u,
                                      0x0241u, 0x0201u, 0x0001u, 0x02BDu, 0x0000u, 0x0038u, 0x02B9u, 0x00FFu, 0x0269u, 0x0282u, 0x02F0u, 0x03FAu,
                                      0x0003u, 0x0242u, 0x02F0u, 0x0242u, 0x0200u, 0x0240u, 0x0020u, 0x0283u, 0x0000u, 0x0220u, 0x000Du};

/* P-PSG (60 words): the mixer, the noise period, the tone A and B fine periods ($40 and $23), CLRR and the coarse periods 0, amplitude A the
 * envelope ($10), amplitude B 10, the envelope period fine $10 and coarse 0, the shape 8 (a repeating decay); a wait of 200 iterations; the tone
 * periods rewritten ($31 and $57); a second wait; B self. */
static const uint16_t sc_p_psg[] = {0x02B8u, 0x0034u, 0x0240u, 0x01F8u, 0x02B8u, 0x0005u, 0x0240u, 0x01F9u, 0x02B8u, 0x0040u, 0x0240u, 0x01F0u,
                                    0x02B8u, 0x0023u, 0x0240u, 0x01F1u, 0x01C0u, 0x0240u, 0x01F4u, 0x0240u, 0x01F5u, 0x02B8u, 0x0010u, 0x0240u,
                                    0x01FBu, 0x02B8u, 0x000Au, 0x0240u, 0x01FCu, 0x02B8u, 0x0010u, 0x0240u, 0x01F3u, 0x01C0u, 0x0240u, 0x01F7u,
                                    0x02B8u, 0x000Au, 0x0240u, 0x01FAu, 0x02B9u, 0x00C8u, 0x0011u, 0x022Cu, 0x0002u, 0x02B8u, 0x0031u, 0x0240u,
                                    0x01F0u, 0x02B8u, 0x0057u, 0x0240u, 0x01F1u, 0x02B9u, 0x00C8u, 0x0011u, 0x022Cu, 0x0002u, 0x0220u, 0x0001u};

#define SC_N(a) (sizeof(a) / sizeof((a)[0]))

static const sc_run sc_runs[] = {
    {"P-SDBD", SC_NO_SCENARIO, sc_p_sdbd, SC_N(sc_p_sdbd)},
    {"P-HLT", SC_NO_SCENARIO, sc_p_hlt, SC_N(sc_p_hlt)},
    {"P-FAULT", SC_NO_SCENARIO, sc_p_fault, SC_N(sc_p_fault)},
    {"P-LATCH", SC_NO_SCENARIO, sc_p_latch, SC_N(sc_p_latch)},
    {"P-PSG", SC_NO_SCENARIO, sc_p_psg, SC_N(sc_p_psg)},
    {"lamp-idle", SU_SC_LAMP_IDLE, NULL, 0u},
    {"lamp-pass", SU_SC_LAMP_PASS, NULL, 0u},
    {"diag-stic-2-collision", SU_SC_STIC2_COLLISION, NULL, 0u},
    {"none", SU_SC_LAMP_IDLE, NULL, 0u}, /* HP-RESETPEND: taken before any unit, continued as the Lamp Tone */
};

const sc_run *sc_run_find(const char *id)
{
  size_t i;
  for (i = 0u; i < SC_N(sc_runs); i++) {
    if (strcmp(sc_runs[i].id, id) == 0) {
      return &sc_runs[i];
    }
  }
  return NULL;
}

int sc_new(ivs_ms_inst *x, const sc_run *r, int power)
{
  if (r->scenario != SC_NO_SCENARIO) {
    return su_scenario_new(x, r->scenario, power);
  }
  return power != 0 && ivs_ms_powered_words(x, r->words, r->nwords, 0x5Au);
}

const ivs_input_event *sc_events(const sc_run *r, uint32_t *count)
{
  if (r->scenario == SC_NO_SCENARIO) {
    *count = 0u;
    return NULL;
  }
  return su_scenario_events(r->scenario, count);
}

/* ---- the cuts ---- */

#define IVSN_SOURCE(p, h)
#define STATE_CUT(c, p, t, f) {c, p, t, f},
static const sc_cut sc_cuts[] = {
#include "state_cuts.inc"
};
#undef STATE_CUT
#undef IVSN_SOURCE

size_t sc_cut_count(void)
{
  return SC_N(sc_cuts);
}

const sc_cut *sc_cut_at(size_t i)
{
  return i < SC_N(sc_cuts) ? &sc_cuts[i] : NULL;
}

const sc_cut *sc_cut_find(const char *id)
{
  size_t i;
  for (i = 0u; i < SC_N(sc_cuts); i++) {
    if (strcmp(sc_cuts[i].id, id) == 0) {
      return &sc_cuts[i];
    }
  }
  return NULL;
}

typedef struct sc_end {
  const char *cut;
  uint64_t end;
} sc_end;

static const sc_end sc_ends[] = {
    {"HP-SDBD", 1000u},
    {"HP-STEAL-A", 4u * SC_FRAME},
    {"HP-STEAL-B", 4u * SC_FRAME},
    {"HP-STEAL-C", 4u * SC_FRAME},
    {"HP-OPEN", 4u * SC_FRAME},
    {"HP-CLOSED", 4u * SC_FRAME},
    {"HP-INT-ENTRY", 4u * SC_FRAME},
    {"HP-HLT", 70000u},
    {"HP-HALTED", 70000u},
    {"HP-FAULTED", 1540u},
    {"HP-RESETPEND", 2u * SC_FRAME},
    {"HP-FRAME-MID", 4u * SC_FRAME},
    {"HP-LATCH", 4u * SC_FRAME},
    {"HP-MOB", 4u * SC_FRAME},
    {"HP-PSG-MID", 4u * SC_FRAME},
    {"HP-ENV-MID", 2u * SC_FRAME},
    {"HP-INPUT", 4u * SC_FRAME},
    {"HP-LOG", 2u * SC_FRAME},
};

uint64_t sc_cut_end(const sc_cut *c)
{
  size_t i;
  for (i = 0u; i < SC_N(sc_ends); i++) {
    if (strcmp(sc_ends[i].cut, c->id) == 0) {
      return sc_ends[i].end;
    }
  }
  return 0u;
}

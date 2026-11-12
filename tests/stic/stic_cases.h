/* SPDX-License-Identifier: MIT */
/* Suite stic (VID-01, plan 06-06): the STIC register file, the access windows, the display-enable handshake and the
 * frame timeline. One executable; each family file defines its case table here declared and stic_main.c
 * concatenates them into one suite. Module-level cases perturb a stack copy of the generated ivs_stic_params; the
 * machine-path cases drive stand-in guests (decles derived in comments from data/cp1610/opcode-families.json).
 * Expected values are relations to the parameters or values from the independent literals, never a policy number
 * (D-02) and never a core table. */
#ifndef IVS_STIC_CASES_H
#define IVS_STIC_CASES_H

#include <stddef.h>
#include <stdint.h>

#include "ivs_test.h"
#include "machine_cases.h"

#define SC_CHECK(cond)                                                                                 \
  do {                                                                                                 \
    if (!(cond)) {                                                                                     \
      ivs_test_diag("check failed at line %d: %s", __LINE__, #cond);                                   \
      return IVS_TEST_FAIL;                                                                            \
    }                                                                                                  \
  } while (0)

/* The registry index of the only profile of the first slice. */
#define SC_PROFILE 0u

extern const ivs_test_case ivs_stic_events_cases[];
extern const size_t ivs_stic_events_count;
extern const ivs_test_case ivs_stic_access_cases[];
extern const size_t ivs_stic_access_count;

/* ---- stand-in guests (stic_progs.c) ----
 * A guest is a list of decles at $1000 (the reset entry of the stand-in EXEC), built by the helpers below; every word
 * is derived from the project's own Phase 2 vector data, as in tests/machine/machine_standin.c:
 *   MVII #imm, Rd   02B8 + d, then imm        (immediate.json, cpu.MVII.r1-load; imm at most 3FF so a 10-bit decle holds it)
 *   MVO  Rs, addr   0240 + s, then addr       (direct.json, MVO R1, $0200 is 0241 0200; addr at most $3FF, a decle)
 *   MVI  addr, Rd   0280 + d, then addr       (direct.json, MVI $0200, R1 is 0281 0200; addr at most $3FF)
 *   an address above $3FF: SDBD, MVII #addr, R3 (lo, hi), then MVO@ Rs, R3 (0258 + s) or MVI@ R3, Rd (0298 + d) with R3
 *                   as a scratch pointer (5 words per access; the register of the access is not R3)
 *   DECR R1         0011                      (register.json, DECR: 0000 010 DDD)
 *   BNZE back       022C, then 2              (branch.json, cpu.BNZE.back-taken: from the word after BNZE d = 2 gives DECR)
 *   B self          0220, then 1              (machine_standin.c: PC+2 - d - 1 is the branch itself)
 * so a delay of N is MVII #N, R1 / DECR R1 / BNZE back (5 words, N iterations). No guest has more than 24 words. */
#define SC_PROG_MAX 24u
size_t sc_w_mvii(uint16_t *w, size_t n, uint32_t r, uint32_t imm);
size_t sc_w_mvo(uint16_t *w, size_t n, uint32_t r, uint32_t addr);
size_t sc_w_mvi(uint16_t *w, size_t n, uint32_t addr, uint32_t r);
size_t sc_w_delay(uint16_t *w, size_t n, uint32_t count);
size_t sc_w_loop(uint16_t *w, size_t n);

/* Delay counts: the access tests only need to land between two window edges, and a test asserts the outcome (a blocked
 * access), never the arithmetic. SC_DELAY_PAST_REG lands after the register window has closed and before the graphics
 * memory window has; SC_DELAY_PAST_GMEM lands after both (and before the frame end). */
#define SC_DELAY_PAST_REG 200u
#define SC_DELAY_PAST_GMEM 300u

/* A POWERED instance running the guest w[0..n) from $1000 with the stand-in GROM and a cartridge of 16 words at
 * cart_addr (0 for the default $5000). 1 on success; the caller frees x with ivs_ms_free. */
int sc_guest(ivs_ms_inst *x, const uint16_t *w, size_t n, uint32_t cart_addr);
/* Steps the instance unit by unit until machine time reaches ticks (at most 40,000 units); 1 on success. */
int sc_run_until(ivs_ms_inst *x, uint64_t ticks);

#endif

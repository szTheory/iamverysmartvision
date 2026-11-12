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
extern const ivs_test_case ivs_stic_reg_cases[];
extern const size_t ivs_stic_reg_count;
extern const ivs_test_case ivs_stic_hook_cases[];
extern const size_t ivs_stic_hook_count;
extern const ivs_test_case ivs_stic_profile_cases[];
extern const size_t ivs_stic_profile_count;
extern const ivs_test_case ivs_stic_judge_cases[];
extern const size_t ivs_stic_judge_count;
extern const ivs_test_case ivs_stic_state_cases[];
extern const size_t ivs_stic_state_count;
extern const ivs_test_case ivs_stic_peek_cases[];
extern const size_t ivs_stic_peek_count;
extern const ivs_test_case ivs_stic_lines_cases[];
extern const size_t ivs_stic_lines_count;
extern const ivs_test_case ivs_stic_irq_cases[];
extern const size_t ivs_stic_irq_count;
extern const ivs_test_case ivs_stic_fetch_cases[];
extern const size_t ivs_stic_fetch_count;
extern const ivs_test_case ivs_stic_busrq_cases[];
extern const size_t ivs_stic_busrq_count;

/* ---- the independent literals (stic_reg.c includes the generated ivs_stic_probe.inc, cmake/IvsSticProbes.cmake) ---- */
typedef struct sl_reg_row {
  uint16_t addr;
  uint16_t cls; /* 0 stored, 1 strobe, 2 collision, 3 reserved (the register literal's class names) */
  uint16_t wmask;
  uint16_t rmask;
} sl_reg_row;
const sl_reg_row *sl_regs(void);                 /* 64 rows by address */
uint32_t sl_hook(int write_view, uint32_t granule); /* expected hook class of a 16-word granule, 0..4095 */
size_t sl_mode_count(void);
uint16_t sl_mode_addr(size_t i);
const ivs_test_source *sl_sources(size_t *n);

/* Checking functions that the judges (stic_judge.c) run on corrupted copies of their oracle input. */
uint32_t sr_table_diff_copy(uint32_t row, uint16_t wmask_delta);   /* stic_reg.c: 0 when a copy of the literal with one write mask flipped still matches */
int sb_late_counter_ok(uint32_t late_after_stream, uint32_t late_after_loop, uint32_t bus_count); /* stic_busrq.c: 1 when the late-grant observations are the expected ones */
int sp_edge_matches(uint64_t expected);                           /* stic_profile.c: 1 when the first blocked register access is stamped at expected */
uint64_t sp_register_edge(void);                                  /* stic_profile.c: the observed first blocked stamp of the register window */

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

/* ---- frame-timeline stand-ins and the unit trace (plan 06-08, stic_progs.c) ----
 * The interrupt programs put their service routine at the address the profile's IAB value points to ($1004), so the first four
 * words are a forward branch over it (the boot stub's trap would halt). Program VBL (at most 24 words, every decle derived from the
 * Phase 2 vectors; cycles per instruction from cp1610_decode.c):
 *   $1000 B $100F              0200, 000D   (branch.json cpu.B.forward-zero: target = addr + 2 + d; d = $100F - $1002)
 *   $1002 two pad words (0000, never executed)
 *   $1004 DIS                  0003         the ISR starts with DIS (C41 = 0: INTFF stays set, so a DIS-first ISR is entered once)
 *   $1005 MVO R0, $0020        0240, 0020   the display-enable strobe, inside the register window
 *   $1007 INCR R2              000A         the interrupt counter (INCR 0000 001 DDD)
 *   $1008 MVII #N, R1          02B9, N      N iterations of DECR R1 (0011) and BNZE back (022C, 0002), about 15 cycles each, so the
 *                                           ISR outlasts the INTRM level (3,791 cycles) before it re-enables
 *   $100D EIS                  0002
 *   $100E PULR R7              02B7         the return (PULR 1010 110 DDD, pops the PC the entry pushed at R6)
 *   $100F MVII #$100, R6       02BE, 0100   the stack at the scratchpad
 *   $1011 EIS                  0002
 *   $1012 the loop body of the chosen shape (instruction length c), see the SC_BODY_* below. */
#define SC_BODY_INCR 0u   /* three INCR R5 (6 cycles each, 000D) and a branch back (9 taken) */
#define SC_BODY_TSTR6 1u  /* three TSTR R6 (7 cycles: 6 plus one for R6, 0B6) and a branch back */
#define SC_BODY_MVII 2u   /* two MVII #0, R5 (8 cycles, 02BD 0000) and a branch back */
#define SC_BODY_BSELF 3u  /* B self (9 cycles taken, 0220 0001) */
#define SC_BODY_MVI 4u    /* MVI $0100, R5 (10 cycles, 0285 0100) and a branch back */
#define SC_BODY_J 5u      /* J self (12 cycles, 0004 0310 0012) */
#define SC_BODIES 6u
#define SC_ISR_DELAY 300u
#define SC_STRADDLE_K 6u  /* stores in front of the swept store */
#define SC_STRADDLE_N0 409u /* the first delay count of the straddle sweep (stic_fetch.c): the store of row 3 lands near its fetch tick */
size_t sc_prog_vbl(uint16_t *w, uint32_t body);
/* A loop that never strobes the display: [EIS or DIS] then B self ($1000, enable 1 for EIS, 0 for DIS; 3 words). */
size_t sc_prog_loop(uint16_t *w, uint32_t enable);
/* MVO R0, $0020 (strobe), HLT, NOP, B self: the CPU halts after the NOP and then runs 1-cycle HALTED units (5 words). */
size_t sc_prog_halt(uint16_t *w);
/* EIS, then count times MVO R0, $0020 (11 cycles, not interruptible), then B self; 1 + 2 count + 2 words, at most 4,096. */
size_t sc_prog_stream(uint16_t *w, size_t count);
/* sc_guest without the 24-word limit (the stream program); 1 on success. */
int sc_guest_n(ivs_ms_inst *x, const uint16_t *w, size_t n);

/* One recorded CPU unit: its start tick (machine time before it), its cycles and kind and the stepper's event bits. */
typedef struct sc_unit {
  uint64_t start;
  uint32_t cycles;
  uint8_t kind;
  uint8_t events;
  uint16_t opcode;
} sc_unit;
/* Steps the instance unit by unit while machine time is below until and records every unit; the count, or (size_t)-1 when a step is
 * refused or more than cap units would be needed. */
size_t sc_trace(ivs_ms_inst *x, uint64_t until, sc_unit *u, size_t cap);

/* ---- the whole-instance oracle of the inspection families (stic_state.c, plan 06-07) ----
 * A snapshot holds a heap copy of the whole instance storage, the nine digests (composite and domains 1..4, 16..19) and
 * machine time; a call that must not touch the machine leaves all three equal. */
#define SS_DIGESTS 9u
typedef struct ss_snap {
  uint8_t *copy;
  uint8_t digest[SS_DIGESTS][32];
  uint64_t time_ticks;
} ss_snap;
int ss_snap_take(ss_snap *s, const ivs_ms_inst *x); /* 1 on success; s->copy is NULL on failure */
int ss_snap_same(const ss_snap *s, const ivs_ms_inst *x); /* 1 when storage, digests and time are all equal */
void ss_snap_free(ss_snap *s);

#endif

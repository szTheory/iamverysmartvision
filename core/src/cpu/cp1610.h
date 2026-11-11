/* SPDX-License-Identifier: MIT */
/* CP-1610 unit stepper: internal API (D-01..D-07). Not installed and not an ABI: it lives in
 * core/src, which only the core and the tests (target ivs_core_internal) see; the public tree
 * core/include/ivs holds the status header it uses. Page numbers are "pdf N" of the GI CP-1600
 * Users Manual (SRC-gi-cp1600-um-1975).
 *
 * One call to ivs_cp1610_step executes exactly one unit: an instruction, an interrupt entry,
 * a reset entry, a bus-grant wait, a halted idle, a fault report or a not-reset report.
 * Time crosses this API only as *_cpu_cycles relative to the start of the unit (D-04): the
 * CPU keeps no tick counter and never sees a tick rate or Hz; the machine layer converts.
 * A step is deterministic given (state, config, lines, bus replies) (D-05). */
#ifndef IVS_CP1610_H
#define IVS_CP1610_H

#include <stddef.h>
#include <stdint.h>

#include "ivs/ivs_status.h"

#define IVS_CP1610_STATE_VERSION 1u
#define IVS_CP1610_MAX_ACCESSES 8u

/* The one narrowing helper (D-07): every 16-bit store goes through it. */
#define IVS_U16(x) ((uint16_t)((x) & 0xFFFFu))

/* Access kinds (D-02, A-04, A-11). DW is the interrupt-entry write of R7, distinct from WRITE.
 * The one wait rule (A-04 as amended by A-11): "BDRDY waits apply only to accesses that begin
 * with a BAR/ADAR address phase, which in the D-02 kind list are FETCH, READ, WRITE and ADAR;
 * INTAK, IAB and DW take no waits". */
#define IVS_ACC_FETCH 1u
#define IVS_ACC_READ 2u
#define IVS_ACC_WRITE 3u
#define IVS_ACC_ADAR 4u
#define IVS_ACC_INTAK 5u
#define IVS_ACC_IAB 6u
#define IVS_ACC_DW 7u
#define IVS_ACC_STROBE_TCI 8u
#define IVS_ACC_STROBE_PCIT 9u

/* Access flags: the reason of an IAB vector read. */
#define IVS_ACC_F_RESET 0x01u
#define IVS_ACC_F_INTERRUPT 0x02u

/* One bus access (12 bytes). offset_cpu_cycles is the BAR (or address-phase) cycle relative
 * to the unit start, after earlier waits; wait_cpu_cycles is the wait the CPU applied. In the
 * log, data is the value transferred (read reply or written value); INTAK and strobe records,
 * which transfer none, carry 0. */
typedef struct ivs_cp1610_access {
  uint32_t offset_cpu_cycles;
  uint16_t addr;
  uint16_t data;
  uint16_t wait_cpu_cycles;
  uint8_t kind;
  uint8_t flags;
} ivs_cp1610_access;

typedef struct ivs_bus_reply {
  uint16_t data;
  uint16_t wait_cpu_cycles;
} ivs_bus_reply;

/* Called once per non-idle access, in order, during the step. Must not re-enter the CPU. */
typedef ivs_bus_reply (*ivs_bus_fn)(void *ctx, const ivs_cp1610_access *a);
/* External branch condition provider for BEXT; returns nonzero for true. */
typedef uint8_t (*ivs_ebci_fn)(void *ctx, uint8_t e);

typedef struct ivs_cp1610_port {
  ivs_bus_fn bus;
  ivs_ebci_fn ebci; /* NULL means false */
  void *ctx;
} ivs_cp1610_port;

/* Profile-backed knobs (plan 02-01 profile fields); see ivs_cp1610_config_default. */
typedef struct ivs_cp1610_config {
  uint8_t interrupt_accept_clears_intff;
  uint8_t release_latency_cpu_cycles;
  uint8_t wait_limit_cpu_cycles;
  uint8_t reserved;
} ivs_cp1610_config;

/* Line levels sampled by the caller for this step (D-05). stpst is a negative-edge event
 * for this step, not a level (A-01). Each field is 0 or 1. */
typedef struct ivs_cp1610_lines {
  uint8_t intr;
  uint8_t intrm;
  uint8_t busrq;
  uint8_t stpst;
} ivs_cp1610_lines;

/* Status bits in the GSWD layout (pdf 45). */
#define IVS_CP1610_C 0x10u
#define IVS_CP1610_OV 0x20u
#define IVS_CP1610_Z 0x40u
#define IVS_CP1610_S 0x80u
#define IVS_CP1610_STATUS_MASK 0xF0u

/* Run states. */
#define IVS_CP1610_RS_UNRESET 0u
#define IVS_CP1610_RS_RESET_PENDING 1u
#define IVS_CP1610_RS_RUNNING 2u
#define IVS_CP1610_RS_HALTED 3u
#define IVS_CP1610_RS_FAULTED 4u

/* Sticky guest fault reasons (D-12). Every decle has a row with a handler, so no reason marks an
 * unimplemented decle; fault_reason above IVS_CP1610_FAULT_LAST fails validation. */
#define IVS_CP1610_FAULT_NONE 0u
#define IVS_CP1610_FAULT_UNSUPPORTED_SDBD_TARGET 1u
#define IVS_CP1610_FAULT_UNSUPPORTED_MVO_SSS_EQ_MMM 2u
#define IVS_CP1610_FAULT_UNSUPPORTED_JUMP_DE 3u
#define IVS_CP1610_FAULT_BUS_WAIT_LIMIT 4u
#define IVS_CP1610_FAULT_LAST IVS_CP1610_FAULT_BUS_WAIT_LIMIT

/* Unit kinds. */
#define IVS_CP1610_UNIT_INSTR 1u
#define IVS_CP1610_UNIT_INT_ENTRY 2u
#define IVS_CP1610_UNIT_RESET_ENTRY 3u
#define IVS_CP1610_UNIT_BUS_GRANT 4u
#define IVS_CP1610_UNIT_HALTED 5u
#define IVS_CP1610_UNIT_FAULT 6u
#define IVS_CP1610_UNIT_NOT_RESET 7u

/* Result events. */
#define IVS_CP1610_EV_TCI 0x01u
#define IVS_CP1610_EV_PCIT 0x02u
#define IVS_CP1610_EV_HALT_LATCHED 0x04u
#define IVS_CP1610_EV_STOPPED 0x08u
#define IVS_CP1610_EV_FAULT 0x10u
#define IVS_CP1610_EV_BUS_RELEASED 0x20u

/* CPU state (D-03): a plain 32-byte struct embedded by value. No pointers, bool, enums or
 * bitfields; every reserved byte is zero. R6 is the stack pointer and R7 the program
 * counter. Boolean fields hold 0 or 1. ivs_cp1610_state_validate also refuses field combinations
 * that no step produces (FAULTED without a reason, a reason outside FAULTED, latches or the grant
 * before the reset entry, DBD outside a closed boundary, the stop latch in HALTED; cp1610.c). */
typedef struct ivs_cp1610 {
  uint16_t r[8];
  uint8_t status;
  uint8_t intff;
  uint8_t dbd;
  uint8_t prev_interruptible;
  uint8_t run_state;
  uint8_t halt_pending;
  uint8_t fault_reason;
  uint8_t busak;
  uint8_t reserved[8];
} ivs_cp1610;

_Static_assert(sizeof(ivs_cp1610) == 32u, "ivs_cp1610 must be 32 bytes (D-03)");
_Static_assert(sizeof(ivs_cp1610_access) == 12u, "ivs_cp1610_access must be 12 bytes (D-02)");

typedef struct ivs_cp1610_result {
  uint32_t elapsed_cpu_cycles;
  uint16_t opcode;
  uint16_t pc_before;
  uint8_t unit_kind;
  uint8_t n_accesses;
  uint8_t events;
  uint8_t reserved;
} ivs_cp1610_result;

/* Decode table row (cp1610_decode.c). A decle matches when (decle & mask) == value and, for a
 * row with eq set, its two register fields are equal (bits 5..3 == bits 2..0: TSTR, CLRR).
 * id is the row id of data/expected/cp1610-rows.ids; handler is the row's IVS_CP1610_OP_* id. */
typedef struct ivs_cp1610_row {
  const char *id;
  uint16_t mask;
  uint16_t value;
  uint8_t eq;
  uint8_t words;
  uint8_t base_cycles;
  uint8_t interruptible;
  uint8_t handler;
} ivs_cp1610_row;

/* Row handler ids (internal). */
#define IVS_CP1610_OP_CLRC 1u
#define IVS_CP1610_OP_SETC 2u
#define IVS_CP1610_OP_EIS 3u
#define IVS_CP1610_OP_DIS 4u
#define IVS_CP1610_OP_HLT 5u
#define IVS_CP1610_OP_SDBD 6u
#define IVS_CP1610_OP_TCI 7u
#define IVS_CP1610_OP_GSWD 8u
#define IVS_CP1610_OP_NOP 9u
#define IVS_CP1610_OP_SIN 10u
#define IVS_CP1610_OP_RSWD 11u
#define IVS_CP1610_OP_ADDR 12u
#define IVS_CP1610_OP_SUBR 13u
#define IVS_CP1610_OP_CMPR 14u
#define IVS_CP1610_OP_ANDR 15u
#define IVS_CP1610_OP_XORR 16u
#define IVS_CP1610_OP_CLRR 17u
#define IVS_CP1610_OP_INCR 18u
#define IVS_CP1610_OP_DECR 19u
#define IVS_CP1610_OP_COMR 20u
#define IVS_CP1610_OP_NEGR 21u
#define IVS_CP1610_OP_ADCR 22u
#define IVS_CP1610_OP_MOVR 23u
#define IVS_CP1610_OP_TSTR 24u
#define IVS_CP1610_OP_JR 25u
#define IVS_CP1610_OP_MVII 26u
#define IVS_CP1610_OP_ADDI 27u
#define IVS_CP1610_OP_SUBI 28u
#define IVS_CP1610_OP_CMPI 29u
#define IVS_CP1610_OP_ANDI 30u
#define IVS_CP1610_OP_XORI 31u
#define IVS_CP1610_OP_MVOI 32u
#define IVS_CP1610_OP_SWAP 33u
#define IVS_CP1610_OP_SLL 34u
#define IVS_CP1610_OP_RLC 35u
#define IVS_CP1610_OP_SLLC 36u
#define IVS_CP1610_OP_SLR 37u
#define IVS_CP1610_OP_SAR 38u
#define IVS_CP1610_OP_RRC 39u
#define IVS_CP1610_OP_SARC 40u
#define IVS_CP1610_OP_PSHR 41u
#define IVS_CP1610_OP_PULR 42u
#define IVS_CP1610_OP_MVO 43u
#define IVS_CP1610_OP_MVI 44u
#define IVS_CP1610_OP_ADD 45u
#define IVS_CP1610_OP_SUB 46u
#define IVS_CP1610_OP_CMP 47u
#define IVS_CP1610_OP_AND 48u
#define IVS_CP1610_OP_XOR 49u
/* The @ rows (indirect through register MMM): MVO@ MVI@ ADD@ SUB@ CMP@ AND@ XOR@. */
#define IVS_CP1610_OP_MVO_AT 50u
#define IVS_CP1610_OP_MVI_AT 51u
#define IVS_CP1610_OP_ADD_AT 52u
#define IVS_CP1610_OP_SUB_AT 53u
#define IVS_CP1610_OP_CMP_AT 54u
#define IVS_CP1610_OP_AND_AT 55u
#define IVS_CP1610_OP_XOR_AT 56u
/* The branches 1000 S? cccc (B, the conditional rows, NOPP and BEXT, pdf 54): one handler; bit 4
 * picks the internal condition cccc or the external condition E, which selects the test. */
#define IVS_CP1610_OP_BRANCH 57u
/* The jump group J, JE, JD, JSR, JSRE, JSRD (first word 0000 000 100, pdf 52, 56). */
#define IVS_CP1610_OP_JUMP 58u

/* Internal decode lookup: the most specific row for decle & 0x3FF. Every one of the 1024
 * decles maps to exactly one row; NULL is never returned for a 10-bit decle. */
const ivs_cp1610_row *ivs_cp1610_decode(uint16_t decle);

size_t ivs_cp1610_state_size(void);
ivs_status ivs_cp1610_state_validate(const ivs_cp1610 *s);
ivs_status ivs_cp1610_reset(ivs_cp1610 *s);
void ivs_cp1610_config_default(ivs_cp1610_config *out);
ivs_status ivs_cp1610_step(ivs_cp1610 *s, const ivs_cp1610_config *cfg, const ivs_cp1610_lines *lines,
                           const ivs_cp1610_port *port, ivs_cp1610_access *log, size_t log_cap,
                           ivs_cp1610_result *out);

#endif

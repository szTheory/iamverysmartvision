/* SPDX-License-Identifier: MIT */
/* CP-1610 unit stepper (D-01..D-07, D-09, D-12). Page numbers are "pdf N" of the GI CP-1600
 * Users Manual (SRC-gi-cp1600-um-1975). No allocation, no host services, integer only. */
#include "common/ivs_prelude.h"

#include "cpu/cp1610.h"

/* Reset entry (pdf 27, initialization sequence): after MSYNC release the CPU idles one
 * cycle, reads the starting program counter in an IAB cycle at offset 1, idles, and puts
 * out its first BAR at offset 5. The unit therefore lasts 5 cycles (POL-RST-01). */
#define IVS_RESET_IAB_OFFSET 1u
#define IVS_RESET_CYCLES 5u

/* Instruction fetch (pdf 28): BAR at offset 0, NACT, DTB at offset 2, NACT: 4 cycles. */
#define IVS_FETCH_CYCLES 4u

/* Data access after the fetch (pdf 29, instruction fetch and data access, indirect register
 * mode; pdf 31, the 9-cycle MVO write): the second BAR, which puts out the effective address,
 * is at offset 4 and its data transfer at offset 6. Immediate mode is that access with the
 * effective address PC+1 (Fig 3.2.5, pdf 49). */
#define IVS_DATA_BAR_OFFSET 4u

/* Stack pull read (inference, C36). The indirect read figure (pdf 29) is 8 cycles with the
 * effective address put out at offset 4; an input through R6 costs 3 more ("Add 3 to number of
 * cycles if MMM=6", pdf 55) and Fig 3.2.4 (pdf 48) orders (MMM) <- (MMM) - 1 before EA <- (MMM),
 * so the BAR is taken to come after those 3 cycles. No figure prints it. */
#define IVS_PULL_BAR_OFFSET 7u

/* Direct mode (instruction fetch and data access, direct addr. mode, pdf 30): BAR at 4 puts out
 * PC+1, where the address word sits; ADAR at 6 ("addressed data to address register", pdf 11)
 * gates that word into the address register, so the operand access has its address phase at 6
 * and its data at 8. The address-word record is a READ at 4 carrying the word; the operand
 * record carries the effective address at 6. */
#define IVS_DIRECT_ADAR_OFFSET 6u

/* Jump group (inference, C36: no figure prints a jump). The 12 printed cycles (pdf 56) are taken
 * as three 4-cycle word transfers like the fetch of pdf 28: the first word at 0, the second at 4
 * (as an immediate operand, pdf 29) and the third at 8. A refused DE = 11 jump stops after the
 * second word, 8 cycles. */
#define IVS_JUMP_W3_BAR_OFFSET 8u
#define IVS_JUMP_DE_FAULT_CYCLES 8u

/* SDBD double byte data (POL-SDBD-01; pdf 47, Fig 3.2.6 and 3.2.7 pdf 49, pdf 57, 58). The
 * instruction after SDBD prints 10 cycles (indirect, pdf 57) or 14 with the 4-cycle SDBD word
 * (immediate, pdf 58), so a double byte operand costs 2 more than the single form's 8 (C34). No
 * figure prints the second read (C36, inference): the low byte read keeps the BAR at offset 4,
 * and the high byte read takes its BAR at 7, in place of the trailing NACT of the 8-cycle read
 * figure (pdf 29), so the unit ends after its data transfer at 9. */
#define IVS_SDBD_HIGH_BAR_OFFSET 7u
#define IVS_SDBD_EXTRA_CYCLES 2u

/* Interrupt entry (interrupt sequence figure, pdf 35; SC.6, pdf 24-25; Fig 2-6, pdf 22): INTAK at
 * cycle 0 puts out R6, NACT, DW at 2 and DWS at 3 write the current PC at R6, NACT, IAB at 5 inputs
 * the service routine address into the PC, three NACT, and the first BAR of the service routine
 * is at 9, so the unit lasts 9 cycles (POL-INT-04, C30). DW is one cycle before DWS (pdf 10), so
 * the write is recorded at DW. */
#define IVS_INT_INTAK_OFFSET 0u
#define IVS_INT_DW_OFFSET 2u
#define IVS_INT_IAB_OFFSET 5u
#define IVS_INT_ENTRY_CYCLES 9u

/* Strobe stamps (C39, D-12). TCI timing (pdf 33): the TCI pulse rises in the cycle after the
 * 4-cycle unit, the NACT that follows the next BAR, so its record sits at unit end plus 1.
 * PCIT timing (pdf 34): the output pulse of the 6-cycle SIN starts in its cycle 4, inside the
 * unit. Waits shift both like any later record. */
#define IVS_TCI_AFTER_END 1u
#define IVS_PCIT_OFFSET 4u

/* Per-unit bookkeeping: the port, the optional log, the running access count, the wait
 * shift applied so far and the wait-limit fault marker. */
typedef struct ivs_unit {
  const ivs_cp1610_port *port;
  ivs_cp1610_access *log;
  uint32_t n;
  uint32_t shift;
  uint32_t wait_limit;
  uint8_t wait_fault;
  uint32_t fault_end;
} ivs_unit;

/* The one wait rule (A-04 as amended by A-11): "BDRDY waits apply only to accesses that
 * begin with a BAR/ADAR address phase, which in the D-02 kind list are FETCH, READ, WRITE
 * and ADAR; INTAK, IAB and DW take no waits". A kind not named here takes no wait. */
static int ivs_kind_takes_wait(uint8_t kind)
{
  return kind == IVS_ACC_FETCH || kind == IVS_ACC_READ || kind == IVS_ACC_WRITE || kind == IVS_ACC_ADAR;
}

/* One bus access at nominal offset `nominal` (no-wait timing). The record carries the offset
 * shifted by earlier waits; the bus port sees it first. A wait taken by this access is
 * inserted as idle cycles before its data transfer (pdf 40, POL-WAIT-01), so it shifts every
 * later access and the unit length. A wait above the configured limit marks a sticky
 * BUS_WAIT_LIMIT fault; the faulting unit ends after that access's address cycle and wait
 * (inference: the page bounds the wait, pdf 12, but defines no behaviour past it). The log
 * receives the same record with the transferred value and the applied wait filled in. */
static uint16_t ivs_access(ivs_unit *u, uint32_t nominal, uint8_t kind, uint8_t flags, uint16_t addr,
                           uint16_t data)
{
  ivs_cp1610_access a;
  ivs_bus_reply reply;
  a.offset_cpu_cycles = nominal + u->shift;
  a.addr = addr;
  a.data = data;
  a.wait_cpu_cycles = 0u;
  a.kind = kind;
  a.flags = flags;
  reply = u->port->bus(u->port->ctx, &a);
  if (kind != IVS_ACC_WRITE && kind != IVS_ACC_DW) {
    a.data = reply.data;
  }
  if (ivs_kind_takes_wait(kind)) {
    a.wait_cpu_cycles = reply.wait_cpu_cycles;
    if ((uint32_t)reply.wait_cpu_cycles > u->wait_limit && !u->wait_fault) {
      u->wait_fault = 1u;
      u->fault_end = a.offset_cpu_cycles + 1u + (uint32_t)reply.wait_cpu_cycles;
    }
    u->shift += (uint32_t)reply.wait_cpu_cycles;
  }
  if (u->n >= IVS_CP1610_MAX_ACCESSES) {
    __builtin_trap();
  }
  if (u->log != NULL) {
    u->log[u->n] = a;
  }
  u->n++;
  return a.data;
}

size_t ivs_cp1610_state_size(void)
{
  return sizeof(ivs_cp1610);
}

static int ivs_is_bit(uint8_t v)
{
  return v == 0u || v == 1u;
}

ivs_status ivs_cp1610_state_validate(const ivs_cp1610 *s)
{
  size_t i;
  if (s == NULL) {
    return IVS_E_BAD_ARG;
  }
  if (((uint32_t)s->status & ~(uint32_t)IVS_CP1610_STATUS_MASK) != 0u) {
    return IVS_E_BAD_STATE;
  }
  if (s->run_state > IVS_CP1610_RS_FAULTED || s->fault_reason > IVS_CP1610_FAULT_LAST) {
    return IVS_E_BAD_STATE;
  }
  if (!ivs_is_bit(s->intff) || !ivs_is_bit(s->dbd) || !ivs_is_bit(s->prev_interruptible) ||
      !ivs_is_bit(s->halt_pending) || !ivs_is_bit(s->busak)) {
    return IVS_E_BAD_STATE;
  }
  for (i = 0u; i < sizeof s->reserved; i++) {
    if (s->reserved[i] != 0u) {
      return IVS_E_BAD_STATE;
    }
  }
  return IVS_OK;
}

void ivs_cp1610_config_default(ivs_cp1610_config *out)
{
  if (out == NULL) {
    return;
  }
  /* Profile intv-2609-ntsc-family-p1 (plan 02-01): cpu.interrupt_accept_clears_intff = 0,
   * bus.release_latency_cpu_cycles = 2, bus.wait_limit_cpu_cycles = 35. */
  out->interrupt_accept_clears_intff = 0u;
  out->release_latency_cpu_cycles = 2u;
  out->wait_limit_cpu_cycles = 35u;
  out->reserved = 0u;
}

/* POL-RST-01: reset models the MSYNC release. Registers and status are zero-filled (profile
 * default), INTFF, DBD, halt and fault are cleared, and the next step is the reset entry. */
ivs_status ivs_cp1610_reset(ivs_cp1610 *s)
{
  if (s == NULL) {
    return IVS_E_BAD_ARG;
  }
  memset(s, 0, sizeof *s);
  s->prev_interruptible = 0u;
  s->run_state = IVS_CP1610_RS_RESET_PENDING;
  return IVS_OK;
}

static void ivs_result_init(ivs_cp1610_result *r, const ivs_cp1610 *s, uint8_t unit_kind)
{
  r->elapsed_cpu_cycles = 0u;
  r->opcode = 0u;
  r->pc_before = s->r[7];
  r->unit_kind = unit_kind;
  r->n_accesses = 0u;
  r->events = 0u;
  r->reserved = 0u;
}

/* IAB access (POL-IAB-01, C32, C42): the bus layer answers the vector, and all 16 bits of the
 * reply are the new PC. The core holds no vector constant for reset or for interrupts. */
static uint16_t ivs_iab(ivs_unit *u, uint32_t nominal, uint8_t flags)
{
  uint16_t v = ivs_access(u, nominal, IVS_ACC_IAB, flags, 0u, 0u);
#ifdef IVS_FAULT_IAB_HARDCODED_1004
  v = 0x1004u;
#endif
  return v;
}

/* Reset entry unit: IAB at offset 1 loads all 16 bits of R7 (pdf 27); other registers are
 * untouched; the first instruction always runs (pdf 22), so prev_interruptible is 0. */
static void ivs_unit_reset_entry(ivs_cp1610 *n, ivs_unit *u, ivs_cp1610_result *r)
{
  n->r[7] = ivs_iab(u, IVS_RESET_IAB_OFFSET, IVS_ACC_F_RESET);
#ifdef IVS_FAULT_RESET_ENTRY
  n->r[7] = (uint16_t)(n->r[7] & 0x00FFu);
#endif
  n->run_state = IVS_CP1610_RS_RUNNING;
  n->prev_interruptible = 0u;
  r->elapsed_cpu_cycles = IVS_RESET_CYCLES;
}

static void ivs_set_fault(ivs_cp1610 *n, ivs_cp1610_result *r, uint8_t reason)
{
  n->run_state = IVS_CP1610_RS_FAULTED;
  n->fault_reason = reason;
  r->unit_kind = IVS_CP1610_UNIT_FAULT;
  r->events = (uint8_t)(r->events | IVS_CP1610_EV_FAULT);
}

/* Status update from a 16-bit result (pdf 45): S is bit 15 of the result, Z is set when the
 * result is all zeros. */
static void ivs_set_sz(ivs_cp1610 *n, uint32_t result)
{
  uint32_t st = (uint32_t)n->status & ~(uint32_t)(IVS_CP1610_S | IVS_CP1610_Z);
  if ((result & 0x8000u) != 0u) {
    st |= IVS_CP1610_S;
  }
  if ((result & 0xFFFFu) == 0u) {
    st |= IVS_CP1610_Z;
  }
  n->status = (uint8_t)st;
}

/* The ALU adder (pdf 45, S3.1.2): result = a + b + cin through a uint32_t temporary. C is the
 * carry out of bit 15; OV is two's complement overflow by the sign rule (a and b have the
 * same sign and the result has the other sign); S and Z come from the result. Subtraction and
 * compare call it with b = ~operand and cin = 1 ("performed by addition of two's complement
 * of the subtrahend operand"). */
static uint16_t ivs_alu_add(ivs_cp1610 *n, uint32_t a, uint32_t b, uint32_t cin)
{
  uint32_t sum = (a & 0xFFFFu) + (b & 0xFFFFu) + (cin & 1u);
  uint32_t st;
  ivs_set_sz(n, sum);
  st = (uint32_t)n->status & ~(uint32_t)(IVS_CP1610_C | IVS_CP1610_OV);
  if ((sum & 0x10000u) != 0u) {
    st |= IVS_CP1610_C;
  }
  if ((~(a ^ b) & (a ^ sum) & 0x8000u) != 0u) {
    st |= IVS_CP1610_OV;
  }
  n->status = (uint8_t)st;
  return IVS_U16(sum);
}

/* ADD family: a + b with no carry in (ADDR here; ADD, ADD@ and ADDI share it later). */
static uint16_t ivs_op_add(ivs_cp1610 *n, uint32_t a, uint32_t b)
{
  uint16_t r = ivs_alu_add(n, a, b, 0u);
#ifdef IVS_FAULT_ADD_OVERFLOW
  n->status = (uint8_t)(((uint32_t)n->status & ~(uint32_t)IVS_CP1610_OV) | (((uint32_t)n->status & IVS_CP1610_C) << 1));
#endif
  return r;
}

/* Register write of the register group. A write to R7 is the new PC: it overrides the PC the
 * fetch already incremented (POL-STK-01). */
static void ivs_wreg(ivs_cp1610 *n, uint32_t reg, uint32_t v)
{
  n->r[reg & 7u] = IVS_U16(v);
#ifdef IVS_FAULT_MOVR_R7_PC
  if ((reg & 7u) == 7u) n->r[7] = IVS_U16((uint32_t)n->r[7] + 1u);
#endif
}

/* SUB family: a - b as a + ~b + 1 (pdf 45); C is the carry out, so C = 1 means no borrow.
 * SUBR and CMPR here; SUB, SUB@, SUBI and the CMP forms share it later (C46 for CMP). */
static uint16_t ivs_op_sub(ivs_cp1610 *n, uint32_t a, uint32_t b)
{
  uint16_t r = ivs_alu_add(n, a, ~b, 1u);
#ifdef IVS_FAULT_SUB_CARRY
  n->status = (uint8_t)((uint32_t)n->status ^ IVS_CP1610_C);
#endif
  return r;
}

/* Immediate operand address (Fig 3.2.5, pdf 49: EA <- PC+1, PC <- PC+2). The fetch already
 * advanced R7 to PC+1, so the operand address is the current R7 and R7 moves past it. */
static uint16_t ivs_imm_addr(ivs_cp1610 *n)
{
  uint16_t ea = n->r[7];
  n->r[7] = IVS_U16((uint32_t)ea + 1u);
#ifdef IVS_FAULT_IMMEDIATE_PC_ADVANCE
  n->r[7] = ea;
#endif
  return ea;
}

static uint16_t ivs_ind_addr(ivs_cp1610 *n, uint32_t mmm, int output);

/* Double byte operand of the instruction after SDBD (POL-SDBD-01). Two READ records: the low
 * byte from the low 8 bits of the first word, the high byte from the low 8 bits of the second
 * (pdf 47; Fig 3.2.7: bits 15..8 "Not Used"). Immediate (MMM = 7, Fig 3.2.7, pdf 49): EA_L = PC+1,
 * EA_H = PC+2, so R7 ends past both words. Indirect (Fig 3.2.6, pdf 49): MMM 4-5 read (MMM), add 1,
 * read (MMM), add 1; MMM 1-3 read (MMM) twice with no change ("the same byte twice", pdf 57; A-03,
 * C37). Both addresses go through the single-word address rules, so those rules hold here too. */
static uint16_t ivs_dbd_read(ivs_cp1610 *n, ivs_unit *u, uint32_t mmm)
{
  uint16_t lo_ea;
  uint16_t hi_ea;
  uint32_t lo;
  uint32_t hi;
  if (mmm == 7u) {
    lo_ea = ivs_imm_addr(n);
    hi_ea = ivs_imm_addr(n);
#ifdef IVS_FAULT_SDBD_IMMEDIATE_PC
    n->r[7] = hi_ea;
#endif
  } else {
    lo_ea = ivs_ind_addr(n, mmm, 0);
    hi_ea = ivs_ind_addr(n, mmm, 0);
#ifdef IVS_FAULT_SDBD_INDIRECT_INCREMENT
    if (mmm >= 4u) n->r[mmm] = hi_ea;
#endif
  }
  lo = (uint32_t)ivs_access(u, IVS_DATA_BAR_OFFSET, IVS_ACC_READ, 0u, lo_ea, 0u) & 0xFFu;
#ifdef IVS_FAULT_SDBD_MODE13_ONE_READ
  if (mmm < 4u) return IVS_U16(lo | (lo << 8));
#endif
  hi = (uint32_t)ivs_access(u, IVS_SDBD_HIGH_BAR_OFFSET, IVS_ACC_READ, 0u, hi_ea, 0u) & 0xFFu;
  return IVS_U16(lo | (hi << 8));
}

/* Immediate operand read: one READ of all 16 bits at the operand address (C51), or the double
 * byte operand when DBD is set (only the six immediate rows reach here with DBD set). */
static uint16_t ivs_imm_read(ivs_cp1610 *n, ivs_unit *u)
{
  if (n->dbd != 0u) {
    return ivs_dbd_read(n, u, 7u);
  }
  return ivs_access(u, IVS_DATA_BAR_OFFSET, IVS_ACC_READ, 0u, ivs_imm_addr(n), 0u);
}

/* Direct effective address (Fig 3.2.3, pdf 48): the word at PC+1 is the address, all 16 bits
 * (C51), and PC moves past it, so it is read like an immediate operand. */
static uint16_t ivs_dir_addr(ivs_cp1610 *n, ivs_unit *u)
{
  return ivs_imm_read(n, u);
}

/* Direct operand read: the address word, then the operand as an ADAR access (pdf 30, pdf 11). */
static uint16_t ivs_dir_read(ivs_cp1610 *n, ivs_unit *u)
{
  uint16_t ea = ivs_dir_addr(n, u);
  uint8_t kind = IVS_ACC_ADAR;
#ifdef IVS_FAULT_DIRECT_ADAR_ORDER
  kind = IVS_ACC_READ;
#endif
  return ivs_access(u, IVS_DIRECT_ADAR_OFFSET, kind, 0u, ea, 0u);
}

/* The operation of a read row of the external reference group (S3.2.1 Fig 3.2.1, pdf 46: bits
 * 8..6 are 010 MVI, 011 ADD, 100 SUB, 101 CMP, 110 AND, 111 XOR; tables pdf 55, 57) applied to
 * register DDD and the operand: MVI loads, ADD adds, SUB subtracts the operand from DDD, CMP
 * forms DDD minus the operand and stores nothing (C46), AND and XOR change S and Z only. */
static void ivs_read_op(ivs_cp1610 *n, uint32_t op, uint32_t ddd, uint32_t v)
{
  uint32_t x;
  switch (op) {
  case 2u:
    ivs_wreg(n, ddd, v);
    break;
  case 3u:
    ivs_wreg(n, ddd, ivs_op_add(n, n->r[ddd], v));
    break;
  case 4u:
    ivs_wreg(n, ddd, ivs_op_sub(n, n->r[ddd], v));
    break;
  case 5u:
    (void)ivs_op_sub(n, n->r[ddd], v);
    break;
  case 6u:
  case 7u:
    x = op == 6u ? ((uint32_t)n->r[ddd] & v) : ((uint32_t)n->r[ddd] ^ v);
    ivs_wreg(n, ddd, x);
    ivs_set_sz(n, x);
    break;
  default:
    __builtin_trap();
  }
}

/* Indirect effective address (Fig 3.2.4, pdf 48; Fig 3.2.1, pdf 46; stack operations, pdf 62;
 * C17 resolved): MMM 1-3 EA = (MMM), no change; MMM 4-5 EA = (MMM), then (MMM) + 1; MMM 6 on an
 * output (MVO@, PSHR) EA = R6, then R6 + 1; MMM 6 on an input R6 - 1 first, then EA = R6. Every
 * pointer update wraps at 16 bits (POL-STK-01). */
static uint16_t ivs_ind_addr(ivs_cp1610 *n, uint32_t mmm, int output)
{
  uint16_t ea;
  if (mmm == 6u && !output) {
    n->r[6] = IVS_U16((uint32_t)n->r[6] + 0xFFFFu);
#ifdef IVS_FAULT_PULL_DIRECTION
    return IVS_U16((uint32_t)n->r[6] + 1u);
#endif
    return n->r[6];
  }
  ea = n->r[mmm];
  if (mmm >= 4u) {
    n->r[mmm] = IVS_U16((uint32_t)ea + 1u);
#ifdef IVS_FAULT_PUSH_DIRECTION
    if (mmm == 6u) { ea = IVS_U16((uint32_t)ea + 0xFFFFu); n->r[6] = ea; }
#endif
#ifdef IVS_FAULT_AUTOINC_R4R5_MISSING
    if (mmm != 6u) n->r[mmm] = ea;
#endif
  }
#ifdef IVS_FAULT_AUTOINC_R1R3
  if (mmm < 4u) n->r[mmm] = IVS_U16((uint32_t)ea + 1u);
#endif
  return ea;
}

/* Indirect operand read: one READ of all 16 bits at the effective address, put out at offset 4
 * (pdf 29), or at the pull offset for MMM = 6; the double byte operand when DBD is set (only the
 * six @ read rows with MMM 1-5 reach here with DBD set). */
static uint16_t ivs_ind_read(ivs_cp1610 *n, ivs_unit *u, uint32_t mmm)
{
  uint32_t off = mmm == 6u ? IVS_PULL_BAR_OFFSET : IVS_DATA_BAR_OFFSET;
  if (n->dbd != 0u) {
    return ivs_dbd_read(n, u, mmm);
  }
  return ivs_access(u, off, IVS_ACC_READ, 0u, ivs_ind_addr(n, mmm, 0), 0u);
}

/* A read row through register MMM (MVI@, ADD@, SUB@, CMP@, AND@, XOR@ and PULR). POL-STK-02
 * (C55, default-chosen): the pointer moves before the operation, so when DDD is the pointer
 * register the loaded or computed value is its final value (MVI@ R4,R4; PULR R6) and ADD@ R4,R4
 * adds the word at the old R4 to R4+1. */
static void ivs_ind_op(ivs_cp1610 *n, ivs_unit *u, uint32_t op, uint32_t mmm, uint32_t ddd)
{
  uint16_t v = ivs_ind_read(n, u, mmm);
  uint16_t p = n->r[mmm];
  ivs_read_op(n, op, ddd, v);
#ifdef IVS_FAULT_STK_RESULT_LOSES
  if (ddd == mmm) n->r[mmm] = p;
#endif
  (void)p;
}

/* Indirect write of register SSS (MVO timing, pdf 31: BAR of the effective address at offset 4,
 * DW at 6, DWS at 7, 9 cycles). The register is read before the pointer moves. */
static void ivs_ind_write(ivs_cp1610 *n, ivs_unit *u, uint32_t mmm, uint32_t sss)
{
  uint16_t v = n->r[sss & 7u];
  (void)ivs_access(u, IVS_DATA_BAR_OFFSET, IVS_ACC_WRITE, 0u, ivs_ind_addr(n, mmm, 1), v);
}

/* POL-BR-01 (C29, default-chosen): the branch table prints 7/9 unlabelled (pdf 54); the row's
 * base is the 7 and a taken branch adds 2 for the 9. NOPP prints a single 7 and is never taken. */
#define IVS_BRANCH_TAKEN_EXTRA 2u

/* Branch condition of the condition field cccc (bits 3..0; branch table, pdf 54). Bits 2..0 pick
 * the test: 000 B (always), 001 BC C = 1, 010 BOV OV = 1, 011 BPL S = 0, 100 BZE Z = 1, 101 BLT
 * S xor OV = 1, 110 BLE Z or (S xor OV) = 1, 111 BUSC C xor S = 1. Each row with bit 3 = 1
 * prints the opposite test of its bit-3 = 0 partner (BNC C = 0, BNOV OV = 0, BMI S = 1, BNZE
 * Z = 0, BGE S xor OV = 0, BGT Z or (S xor OV) = 0, BESC C xor S = 0), so bit 3 negates the test;
 * 1000 NOPP ("NoOPeration, two words") is B negated and never branches. */
static uint32_t ivs_branch_cond(const ivs_cp1610 *n, uint32_t cccc)
{
  uint32_t st = n->status;
  uint32_t s = (st & IVS_CP1610_S) != 0u ? 1u : 0u;
  uint32_t z = (st & IVS_CP1610_Z) != 0u ? 1u : 0u;
  uint32_t ov = (st & IVS_CP1610_OV) != 0u ? 1u : 0u;
  uint32_t c = (st & IVS_CP1610_C) != 0u ? 1u : 0u;
  uint32_t sx = s ^ ov;
  uint32_t neg = (cccc >> 3) & 1u;
  uint32_t t;
#ifdef IVS_FAULT_BRANCH_COND_SIGNED
  sx = s;
#endif
  switch (cccc & 7u) {
  case 0u:
    t = 1u;
    break;
  case 1u:
    t = c;
    break;
  case 2u:
    t = ov;
    break;
  case 3u:
    t = s ^ 1u;
    break;
  case 4u:
    t = z;
    break;
  case 5u:
    t = sx;
    break;
  case 6u:
    t = z | sx;
    break;
  default:
    t = c ^ s;
    break;
  }
#ifdef IVS_FAULT_BRANCH_NEGATE_IGNORED
  neg = 0u;
#endif
#ifdef IVS_FAULT_NOPP_TAKEN
  if ((cccc & 0xFu) == 8u) neg = 0u;
#endif
  return t ^ neg;
}

/* External condition of BEXT 1000 S1 EEEE (branch table, pdf 54: "Field E is externally decoded
 * to select 1 of 16 conditions. Response is tested for true condition."). EBCA0-3 are the 4
 * least significant bits of the instruction register (pdf 11), so E = decle & $F. EBCI is valid
 * throughout the TS1 of the displacement BAR (pdf 32), so the provider is asked after the
 * displacement read. A NULL provider answers false (C57, POL-BR-01); any nonzero answer is true. */
static uint32_t ivs_branch_ext(const ivs_unit *u, uint16_t word)
{
  uint8_t e = (uint8_t)((uint32_t)word & 0xFu);
#ifdef IVS_FAULT_BEXT_E_FIELD_IGNORED
  e = 0u;
#endif
#ifdef IVS_FAULT_BEXT_IGNORES_EBCI
  return 0u;
#endif
  if (u->port->ebci == NULL) {
    return 0u;
  }
  return u->port->ebci(u->port->ctx, e) != 0u ? 1u : 0u;
}

/* Branch 1000 S? cccc plus a displacement word (B, the 14 conditional rows, NOPP and, with bit 4
 * set, BEXT; S3.2.2, pdf 47; branch table, pdf 54). The displacement word at PC+1 is read like
 * an immediate operand: the BAR at offset 4 puts out PC+1 "to fetch displacement" (pdf 32), and
 * R7 is left at PC+2. Bit 4 = 0 tests the internal condition cccc, bit 4 = 1 the external
 * condition E (ivs_branch_ext). Not taken,
 * R7 stays PC+2. Taken, bit 5 (the direction bit, "S is 0 for +, 1 for -") = 0 adds the
 * displacement to PC+2 and 1 adds its one's complement (C50: all 16 bits), wrapping at 16 bits.
 * No status bit changes. Returns the cycles added to the base 7 (POL-BR-01). */
static uint32_t ivs_branch(ivs_cp1610 *n, ivs_unit *u, uint16_t word)
{
  uint32_t disp = ivs_imm_read(n, u);
  uint32_t extra = IVS_BRANCH_TAKEN_EXTRA;
  uint32_t taken;
  if ((((uint32_t)word >> 5) & 1u) != 0u) {
    disp = ~disp & 0xFFFFu;
#ifdef IVS_FAULT_BRANCH_BACKWARD_SIGN
    disp = (disp + 1u) & 0xFFFFu;
#endif
  }
  taken = ((uint32_t)word & 0x10u) != 0u ? ivs_branch_ext(u, word) : ivs_branch_cond(n, (uint32_t)word & 0xFu);
  if (taken == 0u) {
#ifdef IVS_FAULT_BRANCH_NOT_TAKEN_CYCLES
    return IVS_BRANCH_TAKEN_EXTRA;
#endif
    return 0u;
  }
  n->r[7] = IVS_U16((uint32_t)n->r[7] + disp);
#ifdef IVS_FAULT_BRANCH_TAKEN_CYCLES
  extra = 0u;
#endif
  return extra;
}

/* Jump group 0000 000 100, BB AAAAAA DE, AAAAAAAAAA (Fig 3.3.5 and S3.3.5, pdf 52; jump table,
 * pdf 56). The second and third words are read like immediate operands (two READ records), so R7
 * ends at PC+3. The target takes "bits 2-7 of the second word as bits 10-15 of the jump address
 * and ... bits 0-9 of the third word as bits 0-9" (pdf 52); the other bits of both words ("Future
 * Use", "Not Used") take no part. BB = 0..2 saves the return address PC+3 (pdf 56, C58) in
 * register 1BB (R4, R5 or R6) before the PC is loaded; BB = 3 designates R7 itself, the plain
 * jump, so nothing is saved. DE = 01 enables the interrupt system (JE, JSRE), 10 disables it (JD,
 * JSRD) and 00 leaves it (POL-INT-03). No status bit changes. DE = 11 is not listed (pdf 56;
 * POL-ILL-01): the unit stops after the second word with R7 back at the offender and returns 0 so
 * the caller reports the sticky fault UNSUPPORTED_JUMP_DE; else returns 1. */
static int ivs_jump(ivs_cp1610 *n, ivs_unit *u, uint16_t pc)
{
  uint32_t w2 = ivs_imm_read(n, u);
  uint32_t hi = (w2 >> 2) & 0x3Fu;
  uint32_t bb = (w2 >> 8) & 3u;
  uint32_t de = w2 & 3u;
  uint32_t lo;
  uint32_t ret;
#ifdef IVS_FAULT_JUMP_DE11_ALLOWED
  de = de == 3u ? 0u : de;
#endif
  if (de == 3u) {
    n->r[7] = pc;
    return 0;
  }
#ifdef IVS_FAULT_JUMP_TARGET_BITS
  hi = (w2 >> 4) & 0x3Fu;
#endif
#ifdef IVS_FAULT_JE_AFTER_BOUNDARY
  de = de == 1u ? 0u : de;
#endif
  lo = (uint32_t)ivs_access(u, IVS_JUMP_W3_BAR_OFFSET, IVS_ACC_READ, 0u, ivs_imm_addr(n), 0u) & 0x3FFu;
  ret = n->r[7];
#ifdef IVS_FAULT_JSR_RETURN_ADDR
  ret = (ret + 0xFFFFu) & 0xFFFFu;
#endif
  if (bb != 3u) {
    n->r[4u + bb] = IVS_U16(ret);
  }
  n->r[7] = IVS_U16((hi << 10) | lo);
  if (de == 1u) {
    n->intff = 1u;
  } else if (de == 2u) {
    n->intff = 0u;
  }
  return 1;
}

/* Register shifts and SWAP (Fig 3.3.2, pdf 50; shift table, pdf 53): 0001 BLA N RR on R0-R3,
 * N = 0 one bit or one byte, N = 1 two bits or two bytes (the page header adds 2 cycles). S is
 * bit 15 of the result after left shifts and bit 7 after right shifts and SWAP (C47); Z is set
 * when the 16-bit result is all zero. Link bits: N = 0 RLC/SLLC/RRC/SARC use C as bit 16; N = 1
 * RLC/SLLC use C as bit 17 and OV as bit 16, RRC uses C as bit 16 and OV as bit 17 (C48), and
 * SARC takes the RRC layout (inference: the page names no positions for it). Only rows whose
 * status column lists C or OV for that N write them, so OV is kept on every N = 0 form. Returns
 * the extra cycles (2 when N = 1). */
static uint32_t ivs_shift(ivs_cp1610 *n, uint8_t op, uint32_t word)
{
  uint32_t rr = word & 3u;
  uint32_t two = (word >> 2) & 1u;
  uint32_t x = n->r[rr];
  uint32_t c = ((uint32_t)n->status & IVS_CP1610_C) != 0u ? 1u : 0u;
  uint32_t ov = ((uint32_t)n->status & IVS_CP1610_OV) != 0u ? 1u : 0u;
  uint32_t r = x;
  uint32_t s;
  uint32_t st;
  int right = 1;
  switch (op) {
  case IVS_CP1610_OP_SWAP:
    /* N = 1 swaps the bytes and swaps them back to the original form (pdf 53). */
    r = two ? x : ((x << 8) | (x >> 8));
#ifdef IVS_FAULT_SWAP_N1_AS_N0
    r = (x << 8) | (x >> 8);
#endif
    break;
  case IVS_CP1610_OP_SLL:
    r = x << (1u + two);
    right = 0;
    break;
  case IVS_CP1610_OP_RLC:
    r = two ? ((x << 2) | (c << 1) | ov) : ((x << 1) | c);
    if (two) {
      ov = (x >> 14) & 1u;
    }
    c = (x >> 15) & 1u;
    right = 0;
    break;
  case IVS_CP1610_OP_SLLC:
    r = x << (1u + two);
    if (two) {
      ov = (x >> 14) & 1u;
    }
    c = (x >> 15) & 1u;
    right = 0;
    break;
  case IVS_CP1610_OP_SLR:
    r = x >> (1u + two);
    break;
  case IVS_CP1610_OP_SAR:
  case IVS_CP1610_OP_SARC:
    r = two ? ((x >> 2) | ((x & 0x8000u) != 0u ? 0xC000u : 0u)) : ((x >> 1) | (x & 0x8000u));
    if (op == IVS_CP1610_OP_SARC) {
      ov = two ? (x >> 1) & 1u : ov;
      c = x & 1u;
    }
    break;
  case IVS_CP1610_OP_RRC:
    r = two ? ((x >> 2) | (ov << 15) | (c << 14)) : ((x >> 1) | (c << 15));
    ov = two ? (x >> 1) & 1u : ov;
    c = x & 1u;
#ifdef IVS_FAULT_RRC_LINK_SWAP
    if (two) { r = (r & 0x3FFFu) | ((r >> 1) & 0x4000u) | ((r << 1) & 0x8000u); s = c; c = ov; ov = s; }
#endif
    break;
  default:
    __builtin_trap();
  }
  r &= 0xFFFFu;
  n->r[rr] = IVS_U16(r);
  ivs_set_sz(n, r);
  s = right ? (r >> 7) & 1u : (r >> 15) & 1u;
#ifdef IVS_FAULT_RIGHT_SHIFT_S_BIT15
  s = (r >> 15) & 1u;
#endif
  st = (uint32_t)n->status & ~(uint32_t)(IVS_CP1610_S | IVS_CP1610_C | IVS_CP1610_OV);
  st |= (s != 0u ? IVS_CP1610_S : 0u) | (c != 0u ? IVS_CP1610_C : 0u) | (ov != 0u ? IVS_CP1610_OV : 0u);
  n->status = (uint8_t)st;
#ifdef IVS_FAULT_SHIFT_N1_CYCLES
  two = 0u;
#endif
  return two ? 2u : 0u;
}

/* Row semantics. Control group (pdf 56): HLT sets the stop latch evaluated at a later
 * boundary (POL-HLT-01, C40; evaluation is plan 02-15); SDBD sets DBD for the next unit
 * (consumption is plan 02-13); EIS enables and DIS disables the interrupt system (INTFF,
 * POL-INT-03); TCI pulses the TCI pin; CLRC clears C, SETC sets C. Single-register forms
 * (pdf 56, footnote on pdf 51): GSWD writes the status word into R0-R3 with bits 4 and 12 =
 * C, 5 and 13 = OV, 6 and 14 = Z, 7 and 15 = S and bits 0-3, 8-11 = 0; NOP does nothing; SIN
 * pulses PCIT; RSWD loads C, OV, Z, S from bits 4, 5, 6, 7 of register SSS (for SSS = 7 that
 * is the already incremented PC, an inference no vector asserts). Register group (pdf 53):
 * ADDR, SUBR, CMPR (DDD - SSS, not stored; C46), ANDR, XORR, CLRR, INCR, DECR, COMR, NEGR
 * (0 - DDD by two's complement addition; C49), ADCR (DDD + C; C49), MOVR, TSTR and JR. A
 * register read of R7 sees the PC already incremented by the fetch (inference, POL-STK-01).
 * Immediate group (pdf 57): MVII, ADDI, SUBI, CMPI, ANDI and XORI read the word at PC+1 and
 * reuse the register-group flag rules; MVOI writes register SSS there (no flags). Direct and
 * indirect groups (pdf 55): MVO, MVI, ADD, SUB, CMP, AND, XOR through the address word (ADAR,
 * pdf 30), the @ rows, PSHR and PULR through register MMM (C17, POL-STK-01, POL-STK-02).
 * Branches (pdf 54, pdf 47): B, the 14 internal-condition rows, NOPP and BEXT through
 * ivs_branch (C50, C57, POL-BR-01). Jumps (pdf 52, 56): J, JE, JD, JSR, JSRE, JSRD through
 * ivs_jump (C58, POL-INT-03). Only the status change column's flags move. Strobe records carry no address or data. The
 * return value is the cycles added to the row's base count (the MOVR/TSTR R6/R7 bonus). */
static uint32_t ivs_exec(ivs_cp1610 *n, ivs_unit *u, ivs_cp1610_result *r, const ivs_cp1610_row *row,
                         uint16_t word)
{
  uint32_t v;
  uint32_t sss = ((uint32_t)word >> 3) & 7u;
  uint32_t ddd = (uint32_t)word & 7u;
  uint32_t extra = 0u;
  switch (row->handler) {
  case IVS_CP1610_OP_HLT:
    n->halt_pending = 1u;
#ifdef IVS_FAULT_HLT_NO_LATCH
    n->halt_pending = 0u;
#endif
    r->events = (uint8_t)(r->events | IVS_CP1610_EV_HALT_LATCHED);
    break;
  case IVS_CP1610_OP_SDBD:
    n->dbd = 1u;
    break;
  case IVS_CP1610_OP_TCI:
    v = (uint32_t)row->base_cycles + IVS_TCI_AFTER_END;
#ifdef IVS_FAULT_TCI_STROBE_STAMP
    v = (uint32_t)row->base_cycles;
#endif
    (void)ivs_access(u, v, IVS_ACC_STROBE_TCI, 0u, 0u, 0u);
    r->events = (uint8_t)(r->events | IVS_CP1610_EV_TCI);
    break;
  case IVS_CP1610_OP_GSWD:
    v = (uint32_t)n->status | ((uint32_t)n->status << 8);
#ifdef IVS_FAULT_GSWD_LAYOUT
    v = ((uint32_t)n->status >> 4) | ((uint32_t)n->status << 4);
#endif
    n->r[(uint32_t)word & 3u] = IVS_U16(v);
    break;
  case IVS_CP1610_OP_NOP:
    break;
  case IVS_CP1610_OP_SIN:
    (void)ivs_access(u, IVS_PCIT_OFFSET, IVS_ACC_STROBE_PCIT, 0u, 0u, 0u);
    r->events = (uint8_t)(r->events | IVS_CP1610_EV_PCIT);
    break;
  case IVS_CP1610_OP_RSWD:
    n->status = (uint8_t)((uint32_t)n->r[(uint32_t)word & 7u] & IVS_CP1610_STATUS_MASK);
    break;
  case IVS_CP1610_OP_EIS:
    n->intff = 1u;
#ifdef IVS_FAULT_EIS_NO_INTFF
    n->intff = 0u;
#endif
    break;
  case IVS_CP1610_OP_DIS:
    n->intff = 0u;
    break;
  case IVS_CP1610_OP_CLRC:
    n->status = (uint8_t)((uint32_t)n->status & ~(uint32_t)IVS_CP1610_C);
    break;
  case IVS_CP1610_OP_SETC:
    n->status = (uint8_t)((uint32_t)n->status | IVS_CP1610_C);
    break;
  case IVS_CP1610_OP_ADDR:
    ivs_wreg(n, ddd, ivs_op_add(n, n->r[ddd], n->r[sss]));
    break;
  case IVS_CP1610_OP_SUBR:
    ivs_wreg(n, ddd, ivs_op_sub(n, n->r[ddd], n->r[sss]));
    break;
  case IVS_CP1610_OP_CMPR:
    v = ivs_op_sub(n, n->r[ddd], n->r[sss]);
#ifdef IVS_FAULT_CMP_WRITES_DEST
    ivs_wreg(n, ddd, v);
#endif
    break;
  case IVS_CP1610_OP_ANDR:
    v = (uint32_t)n->r[ddd] & (uint32_t)n->r[sss];
    ivs_wreg(n, ddd, v);
    ivs_set_sz(n, v);
    break;
  case IVS_CP1610_OP_XORR:
  case IVS_CP1610_OP_CLRR:
    v = (uint32_t)n->r[ddd] ^ (uint32_t)n->r[sss];
    ivs_wreg(n, ddd, v);
    ivs_set_sz(n, v);
    break;
  case IVS_CP1610_OP_INCR:
    v = (uint32_t)n->r[ddd] + 1u;
    ivs_wreg(n, ddd, v);
    ivs_set_sz(n, v);
#ifdef IVS_FAULT_INCR_TOUCHES_C
    n->status = (uint8_t)(((uint32_t)n->status & ~(uint32_t)IVS_CP1610_C) | ((v >> 12) & IVS_CP1610_C));
#endif
    break;
  case IVS_CP1610_OP_DECR:
    v = ((uint32_t)n->r[ddd] + 0xFFFFu) & 0xFFFFu;
    ivs_wreg(n, ddd, v);
    ivs_set_sz(n, v);
    break;
  case IVS_CP1610_OP_COMR:
    v = ~(uint32_t)n->r[ddd] & 0xFFFFu;
    ivs_wreg(n, ddd, v);
    ivs_set_sz(n, v);
    break;
  case IVS_CP1610_OP_NEGR:
    v = (uint32_t)n->status & IVS_CP1610_C;
    ivs_wreg(n, ddd, ivs_alu_add(n, 0u, ~(uint32_t)n->r[ddd], 1u));
#ifdef IVS_FAULT_NEGR_CARRY
    n->status = (uint8_t)(((uint32_t)n->status & ~(uint32_t)IVS_CP1610_C) | v);
#endif
    break;
  case IVS_CP1610_OP_ADCR:
    v = ((uint32_t)n->status & IVS_CP1610_C) != 0u ? 1u : 0u;
#ifdef IVS_FAULT_ADCR_IGNORES_C
    v = 0u;
#endif
    ivs_wreg(n, ddd, ivs_alu_add(n, n->r[ddd], 0u, v));
    break;
  case IVS_CP1610_OP_MOVR:
  case IVS_CP1610_OP_TSTR:
  case IVS_CP1610_OP_JR:
    /* One move for the three rows (decle $0BF is all three): DDD = SSS, S and Z from the value.
     * Cycles: MOVR +1 when DDD is 6 or 7, TSTR +1 when SSS is 6 or 7, JR a flat 7 (pdf 53). */
    v = n->r[sss];
    ivs_wreg(n, ddd, v);
    ivs_set_sz(n, v);
    if (row->handler == IVS_CP1610_OP_MOVR) {
      extra = ddd >= 6u ? 1u : 0u;
    } else if (row->handler == IVS_CP1610_OP_TSTR) {
      extra = sss >= 6u ? 1u : 0u;
    }
    break;
  case IVS_CP1610_OP_SWAP:
  case IVS_CP1610_OP_SLL:
  case IVS_CP1610_OP_RLC:
  case IVS_CP1610_OP_SLLC:
  case IVS_CP1610_OP_SLR:
  case IVS_CP1610_OP_SAR:
  case IVS_CP1610_OP_RRC:
  case IVS_CP1610_OP_SARC:
    extra = ivs_shift(n, row->handler, word);
    break;
  case IVS_CP1610_OP_MVII:
    /* MVII (pdf 57): DDD = the word at PC+1; no status change. */
    ivs_wreg(n, ddd, ivs_imm_read(n, u));
    break;
  case IVS_CP1610_OP_ADDI:
    v = ivs_imm_read(n, u);
    ivs_wreg(n, ddd, ivs_op_add(n, n->r[ddd], v));
    break;
  case IVS_CP1610_OP_SUBI:
    v = ivs_imm_read(n, u);
    ivs_wreg(n, ddd, ivs_op_sub(n, n->r[ddd], v));
    break;
  case IVS_CP1610_OP_CMPI:
    /* CMPI (pdf 57): 'from contents of register SSS' (bits 2..0), not stored (C46). */
    v = ivs_imm_read(n, u);
    (void)ivs_op_sub(n, n->r[ddd], v);
    break;
  case IVS_CP1610_OP_ANDI:
    v = (uint32_t)ivs_imm_read(n, u) & (uint32_t)n->r[ddd];
    ivs_wreg(n, ddd, v);
    ivs_set_sz(n, v);
    break;
  case IVS_CP1610_OP_XORI:
    v = (uint32_t)ivs_imm_read(n, u) ^ (uint32_t)n->r[ddd];
    ivs_wreg(n, ddd, v);
    ivs_set_sz(n, v);
    break;
  case IVS_CP1610_OP_MVOI:
    /* MVOI (pdf 57): register SSS (bits 2..0) is written to PC+1; no status change. The
     * address is taken (and PC advanced) before the register is read. */
    v = ivs_imm_addr(n);
#ifdef IVS_FAULT_MVOI_ADDRESS
    v = (v + 0xFFFFu) & 0xFFFFu;
#endif
    (void)ivs_access(u, IVS_DATA_BAR_OFFSET, IVS_ACC_WRITE, 0u, IVS_U16(v), n->r[ddd]);
    break;
  case IVS_CP1610_OP_PSHR:
    /* PSHR SSS = MVO@ through R6 (1001 110 SSS, pdf 55, 62): write at R6, then R6 + 1. */
    ivs_ind_write(n, u, 6u, ddd);
    break;
  case IVS_CP1610_OP_MVO_AT:
    /* MVO@ SSS, MMM (1001 MMM SSS, pdf 55, 9 cycles): register SSS (bits 2..0) is written at the
     * address in register MMM (bits 5..3); SSS = MMM in 4..7 never gets here (POL-MVO-01). */
    ivs_ind_write(n, u, sss, ddd);
    break;
  case IVS_CP1610_OP_PULR:
  case IVS_CP1610_OP_MVI_AT:
  case IVS_CP1610_OP_ADD_AT:
  case IVS_CP1610_OP_SUB_AT:
  case IVS_CP1610_OP_CMP_AT:
  case IVS_CP1610_OP_AND_AT:
  case IVS_CP1610_OP_XOR_AT:
    /* PULR DDD = MVI@ through R6 (1010 110 DDD) and the @ read rows ooo MMM DDD (pdf 55, 62),
     * MMM in bits 5..3. '*Add 3 to number of cycles if MMM=6' (pdf 55): the @ rows print 8, so
     * MMM = 6 adds 3; PULR prints 11, which already holds it. */
    ivs_ind_op(n, u, ((uint32_t)word >> 6) & 7u, sss, ddd);
    extra = sss == 6u && row->handler != IVS_CP1610_OP_PULR ? 3u : 0u;
#ifdef IVS_FAULT_MMM6_BONUS_MISSING
    extra = 0u;
#endif
    break;
  case IVS_CP1610_OP_MVO:
    /* MVO SSS, A (1001 000 SSS + address word, pdf 55, 11 cycles): the address word, then a
     * WRITE of register SSS at the effective address in the ADAR address phase (pdf 30, 11);
     * DW and DWS follow at 8 and 9 as in the write figure (pdf 31). Inference (C36): no figure
     * prints the direct write. The register is read after the address word, as for MVOI. */
    v = ivs_dir_addr(n, u);
    (void)ivs_access(u, IVS_DIRECT_ADAR_OFFSET, IVS_ACC_WRITE, 0u, IVS_U16(v), n->r[ddd]);
    break;
  case IVS_CP1610_OP_BRANCH:
    extra = ivs_branch(n, u, word);
    break;
  case IVS_CP1610_OP_JUMP:
    if (!ivs_jump(n, u, r->pc_before)) {
      ivs_set_fault(n, r, IVS_CP1610_FAULT_UNSUPPORTED_JUMP_DE);
    }
    break;
  case IVS_CP1610_OP_MVI:
  case IVS_CP1610_OP_ADD:
  case IVS_CP1610_OP_SUB:
  case IVS_CP1610_OP_CMP:
  case IVS_CP1610_OP_AND:
  case IVS_CP1610_OP_XOR:
    /* Direct read rows ooo 000 DDD + address word (pdf 55, 10 cycles). */
    v = ivs_dir_read(n, u);
    ivs_read_op(n, ((uint32_t)word >> 6) & 7u, ddd, v);
    break;
  default:
    __builtin_trap();
  }
#ifdef IVS_FAULT_R7_CYCLE_BONUS_ALL
  if (row->handler >= IVS_CP1610_OP_ADDR && row->handler <= IVS_CP1610_OP_ADCR && (sss >= 6u || ddd >= 6u)) extra = 1u;
#endif
  return extra;
}

/* POL-MVO-01 (MVO@ note on pdf 55: "SSS = MMM = 4, 5, 6 or 7 not supported"): an output-group
 * decle 1001 MMM SSS with SSS = MMM in 4..7 ($264, $26D, $276 = PSHR R6, $27F = MVOI R7) is
 * refused with a sticky fault before it has any effect. */
static int ivs_mvo_refused(uint16_t word)
{
  uint32_t mmm = ((uint32_t)word >> 3) & 7u;
#ifdef IVS_FAULT_MVO_SSS_EQ_MMM_ALLOWED
  mmm = 0u;
#endif
  return ((uint32_t)word & 0x3C0u) == 0x240u && mmm >= 4u && mmm == ((uint32_t)word & 7u);
}

/* SDBD targets (POL-SDBD-01; pdf 47, 57, 58; Fig 3.2.6, pdf 49): the six immediate rows MVII,
 * ADDI, SUBI, CMPI, ANDI, XORI (mode 7) and the six indirect read rows MVI@, ADD@, SUB@, CMP@,
 * AND@, XOR@ with MMM 1-5 read a double byte operand. Every other decoded target is refused
 * (POL-SDBD-02): modes 0 and 6 ("not supported", pdf 49, 57; MVI@ with MMM = 6 is the PULR row),
 * MVO@, PSHR, MVOI, the direct rows, branches, jumps, register, shift and control rows and SDBD. */
static int ivs_sdbd_target(const ivs_cp1610_row *row, uint16_t word)
{
  uint32_t mmm = ((uint32_t)word >> 3) & 7u;
  switch (row->handler) {
  case IVS_CP1610_OP_MVII:
  case IVS_CP1610_OP_ADDI:
  case IVS_CP1610_OP_SUBI:
  case IVS_CP1610_OP_CMPI:
  case IVS_CP1610_OP_ANDI:
  case IVS_CP1610_OP_XORI:
    return 1;
  case IVS_CP1610_OP_MVI_AT:
  case IVS_CP1610_OP_ADD_AT:
  case IVS_CP1610_OP_SUB_AT:
  case IVS_CP1610_OP_CMP_AT:
  case IVS_CP1610_OP_AND_AT:
  case IVS_CP1610_OP_XOR_AT:
    return mmm >= 1u && mmm <= 5u;
  default:
    return 0;
  }
}

/* Refusal of a fetched decle before it has any effect, or FAULT_NONE. POL-SDBD-02: with DBD set,
 * a target that is not an SDBD target is refused (checked first: the prefix decides how the decle
 * is read). POL-MVO-01: an MVO@ form with SSS = MMM in 4..7. POL-ILL-01: bits 15..10 of the word
 * take no part (the row and every field come from bits 9..0). */
static uint8_t ivs_refusal(ivs_cp1610 *n, const ivs_cp1610_row *row, uint16_t word)
{
  int target = ivs_sdbd_target(row, word);
#ifdef IVS_FAULT_DECODE_HIGH_BITS
  if (word > 0x3FFu) return IVS_CP1610_FAULT_UNSUPPORTED_JUMP_DE;
#endif
#ifdef IVS_FAULT_SDBD_TARGET_ACCEPTED
  if (!target) n->dbd = 0u;
#endif
  if (n->dbd != 0u && !target) {
    return IVS_CP1610_FAULT_UNSUPPORTED_SDBD_TARGET;
  }
  if (ivs_mvo_refused(word)) {
    return IVS_CP1610_FAULT_UNSUPPORTED_MVO_SSS_EQ_MMM;
  }
  return IVS_CP1610_FAULT_NONE;
}

/* Instruction unit: FETCH at offset 0 from R7 (pdf 28), PC advances one per word fetched,
 * decode bits 9..0. The unit lasts the row's base cycles plus the waits taken; the row's
 * interruptible flag becomes prev_interruptible for the next boundary (CPU-02). */
static void ivs_unit_instr(ivs_cp1610 *n, ivs_unit *u, ivs_cp1610_result *r)
{
  uint16_t pc = n->r[7];
  uint16_t word = ivs_access(u, 0u, IVS_ACC_FETCH, 0u, pc, 0u);
  const ivs_cp1610_row *row;
  uint32_t extra;
  uint8_t dbd;
  uint8_t reason;
  r->opcode = word;
  if (u->wait_fault) {
    return;
  }
  row = ivs_cp1610_decode(word);
  if (row == NULL) {
    /* Unreachable: every 10-bit decle has a row (cp1610_decode.c, checked by cpu-sweep). */
    __builtin_trap();
  }
  reason = ivs_refusal(n, row, word);
  if (reason != IVS_CP1610_FAULT_NONE) {
    /* Sticky fault: PC stays at the offending word; the fetch is traced and counted; nothing
     * else changes (no write for a refused MVO; DBD stays set for a refused SDBD target). */
    r->elapsed_cpu_cycles = IVS_FETCH_CYCLES + u->shift;
    ivs_set_fault(n, r, reason);
    return;
  }
  dbd = n->dbd;
  n->r[7] = IVS_U16((uint32_t)pc + 1u);
  extra = ivs_exec(n, u, r, row, word);
  if (dbd != 0u) {
    /* The target consumed DBD (pdf 57: SDBD sets double byte data "for the next instruction"). */
    n->dbd = 0u;
#ifdef IVS_FAULT_SDBD_DBD_NOT_CLEARED
    n->dbd = 1u;
#endif
    extra += IVS_SDBD_EXTRA_CYCLES;
  }
  if (n->run_state == IVS_CP1610_RS_FAULTED) {
    /* A refusal found after the first word (jump DE = 11, POL-ILL-01): R7 is back at the
     * offender and the reads so far are traced and counted. */
    r->elapsed_cpu_cycles = IVS_JUMP_DE_FAULT_CYCLES + u->shift;
    return;
  }
  n->prev_interruptible = row->interruptible;
#ifdef IVS_FAULT_MVO_INTERRUPTIBLE
  if (row->handler == IVS_CP1610_OP_MVO || row->handler == IVS_CP1610_OP_MVO_AT || row->handler == IVS_CP1610_OP_MVOI || row->handler == IVS_CP1610_OP_PSHR) n->prev_interruptible = 1u;
#endif
  r->elapsed_cpu_cycles = (uint32_t)row->base_cycles + extra + u->shift;
}

/* Step-start conventions (plan 02-15; Fig 2-6, pdf 22; pdf 14, 25, 36, 40; C40, C45, C52-C54).
 * No page prints these unit lengths; they are the stepper's conventions, carried by the profile
 * where a number is involved:
 *   - A step that starts in UNRESET or FAULTED takes 0 cycles and makes no access (D-12); the
 *     step that refuses or faults reports a FAULT unit with the cycles spent up to the refusal.
 *   - BUS_GRANT: while BUSAK is asserted and BUSRQ stays high, each step is a 1-cycle hold unit
 *     with no access (the CPU re-samples BUSRQ every TS1, pdf 36). When BUSRQ is low with BUSAK
 *     asserted, the step is the release unit: config release_latency_cpu_cycles cycles (profile
 *     bus.release_latency_cpu_cycles, default 2, C52, inference from pdf 36), BUSAK cleared,
 *     event BUS_RELEASED. A grant never changes prev_interruptible, so the step after the
 *     release runs the interrupt test, not a fetch (Fig 2-6: the bus request loop exits into the
 *     interrupt request test; A-02, POL-BUS-01).
 *   - Boundary test in RUNNING with BUSAK clear: at an interruptible boundary (prev_interruptible
 *     set, no SDBD prefix pending) the priority is BUSRQ before INTR (pdf 25: bus requests are
 *     honoured "before" interrupt requests), then INTR, then INTRM with INTFF (POL-INT-01). A
 *     closed boundary (after a non-interruptible unit, the reset entry or an interrupt entry)
 *     tests nothing and fetches. */
static uint32_t ivs_boundary_open(const ivs_cp1610 *n)
{
  uint32_t open = n->prev_interruptible;
#ifdef IVS_FAULT_PREV_INTERRUPTIBLE_IGNORED
  open = 1u;
#endif
  return open;
}

/* Interrupt acceptance at the start of a step in RUNNING state (CPU-02; Fig 2-6, pdf 22; pdf 12;
 * SC.6, pdf 24). The test runs only at an interruptible boundary: the previous unit was
 * interruptible (prev_interruptible, set from the row after every unit; an entry and the reset
 * entry leave it 0) and no SDBD prefix is pending (POL-SDBD-03). INTR is always honoured, INTRM
 * only when INTFF is set (POL-INT-01). The lines are the levels the caller sampled for this step
 * (D-05; the sampling point is the machine-layer rule POL-LINE-01, C38) and nothing is latched,
 * so a line asserted at a closed boundary is forgotten. */
static int ivs_int_accept(const ivs_cp1610 *n, const ivs_cp1610_lines *l)
{
  uint32_t open = ivs_boundary_open(n);
  uint32_t intr = l->intr;
  uint32_t intff = n->intff;
#ifdef IVS_FAULT_INTFF_IGNORED
  intff = 1u;
#endif
#ifdef IVS_FAULT_LINES_LATCHED
  { static uint32_t seen; seen |= intr; intr = seen; if (open != 0u && n->dbd == 0u) seen = 0u; }
#endif
#ifdef IVS_FAULT_SDBD_INTERRUPTIBLE
  if (n->dbd != 0u) return intr != 0u || (l->intrm != 0u && intff != 0u);
#endif
  if (open == 0u || n->dbd != 0u) {
    return 0;
  }
  return intr != 0u || (l->intrm != 0u && intff != 0u);
}

/* Interrupt entry unit (POL-INT-04; pdf 35, SC.6 pdf 24-25): INTAK puts out R6, DW writes the
 * current PC at R6 (the interrupt-entry write is its own access kind, never WRITE, A-11), R6
 * moves up by one (mod 2^16), and IAB loads all 16 bits of R7 from the bus reply (POL-IAB-01).
 * Flags are not pushed. The first service routine instruction always runs (Fig 2-6, pdf 22: the
 * sequence returns to the fetch), so prev_interruptible is 0. Acceptance leaves INTFF unless the
 * profile says it clears it (POL-INT-03, C41). INTAK, DW and IAB take no waits (A-11). */
static void ivs_unit_int_entry(ivs_cp1610 *n, const ivs_cp1610_config *cfg, ivs_unit *u, ivs_cp1610_result *r)
{
  uint16_t sp = n->r[6];
  uint32_t clears;
  (void)ivs_access(u, IVS_INT_INTAK_OFFSET, IVS_ACC_INTAK, 0u, sp, 0u);
#ifdef IVS_FAULT_INT_ENTRY_ORDER
  n->r[7] = ivs_iab(u, IVS_INT_DW_OFFSET, IVS_ACC_F_INTERRUPT);
#endif
  (void)ivs_access(u, IVS_INT_DW_OFFSET, IVS_ACC_DW, 0u, sp, n->r[7]);
  n->r[6] = IVS_U16((uint32_t)sp + 1u);
  n->r[7] = ivs_iab(u, IVS_INT_IAB_OFFSET, IVS_ACC_F_INTERRUPT);
  clears = cfg->interrupt_accept_clears_intff;
#ifdef IVS_FAULT_ACCEPT_CLEARS_INTFF_IGNORED
  clears = 0u;
#endif
  if (clears != 0u) {
    n->intff = 0u;
  }
  n->prev_interruptible = 0u;
#ifdef IVS_FAULT_FIRST_ISR_INSTR_INTERRUPTIBLE
  n->prev_interruptible = 1u;
#endif
  r->elapsed_cpu_cycles = IVS_INT_ENTRY_CYCLES + u->shift;
}

/* Bus grant unit (POL-BUS-01; pdf 36; Fig 2-6, pdf 22; C52): no access, the bus floats. With
 * BUSAK asserted and BUSRQ low it is the release: release_latency_cpu_cycles cycles, BUSAK
 * cleared, event BUS_RELEASED. Otherwise it grants or holds for 1 cycle with BUSAK asserted.
 * prev_interruptible is left as it was (A-02). */
static void ivs_unit_grant(ivs_cp1610 *n, const ivs_cp1610_config *cfg, const ivs_cp1610_lines *l,
                           ivs_cp1610_result *r)
{
  uint32_t latency = cfg->release_latency_cpu_cycles;
  if (n->busak != 0u && l->busrq == 0u) {
    n->busak = 0u;
    r->elapsed_cpu_cycles = latency;
    r->events = (uint8_t)(r->events | IVS_CP1610_EV_BUS_RELEASED);
  } else {
    n->busak = 1u;
    r->elapsed_cpu_cycles = 1u;
  }
}

/* One step in RUNNING state: a held or released grant first, then the boundary test (see the
 * step-start conventions above), then an instruction. */
static void ivs_step_running(ivs_cp1610 *n, const ivs_cp1610_config *cfg, const ivs_cp1610_lines *l, ivs_unit *u,
                             ivs_cp1610_result *r)
{
  int grant;
  int accept;
  if (n->busak != 0u) {
    ivs_result_init(r, n, IVS_CP1610_UNIT_BUS_GRANT);
    ivs_unit_grant(n, cfg, l, r);
    return;
  }
  grant = ivs_boundary_open(n) != 0u && n->dbd == 0u && l->busrq != 0u;
  accept = ivs_int_accept(n, l);
#ifdef IVS_FAULT_BUSRQ_PRIORITY
  grant = grant && !accept;
#endif
  if (grant) {
    ivs_result_init(r, n, IVS_CP1610_UNIT_BUS_GRANT);
    ivs_unit_grant(n, cfg, l, r);
  } else if (accept) {
    ivs_result_init(r, n, IVS_CP1610_UNIT_INT_ENTRY);
    ivs_unit_int_entry(n, cfg, u, r);
  } else {
    ivs_result_init(r, n, IVS_CP1610_UNIT_INSTR);
    ivs_unit_instr(n, u, r);
  }
}

static int ivs_lines_valid(const ivs_cp1610_lines *l)
{
  return ivs_is_bit(l->intr) && ivs_is_bit(l->intrm) && ivs_is_bit(l->busrq) && ivs_is_bit(l->stpst);
}

static int ivs_config_valid(const ivs_cp1610_config *c)
{
  return ivs_is_bit(c->interrupt_accept_clears_intff) && c->reserved == 0u;
}

ivs_status ivs_cp1610_step(ivs_cp1610 *s, const ivs_cp1610_config *cfg, const ivs_cp1610_lines *lines,
                           const ivs_cp1610_port *port, ivs_cp1610_access *log, size_t log_cap,
                           ivs_cp1610_result *out)
{
  ivs_cp1610 n;
  ivs_unit u;
  ivs_cp1610_result r;
  if (s == NULL || cfg == NULL || lines == NULL || port == NULL || port->bus == NULL || out == NULL) {
    return IVS_E_BAD_ARG;
  }
  if (log != NULL && log_cap < IVS_CP1610_MAX_ACCESSES) {
    return IVS_E_BAD_ARG;
  }
  /* Lines are validated here; INTR and INTRM decide acceptance in RUNNING state (POL-INT-01);
   * BUSRQ and STPST belong to the bus and halt policies (plan 02-15). */
  if (!ivs_lines_valid(lines) || !ivs_config_valid(cfg)) {
    return IVS_E_BAD_ARG;
  }
  if (ivs_cp1610_state_validate(s) != IVS_OK) {
    return IVS_E_BAD_STATE;
  }
  n = *s;
  u.port = port;
  u.log = log;
  u.n = 0u;
  u.shift = 0u;
  u.wait_limit = cfg->wait_limit_cpu_cycles;
  u.wait_fault = 0u;
  u.fault_end = 0u;
  switch (n.run_state) {
  case IVS_CP1610_RS_UNRESET:
    ivs_result_init(&r, &n, IVS_CP1610_UNIT_NOT_RESET);
    break;
  case IVS_CP1610_RS_RESET_PENDING:
    /* The initialization sequence goes from IAB straight to the fetch (Fig 2-6, pdf 22): the
     * lines are not tested here, and the entry leaves prev_interruptible 0, so the first
     * instruction runs before any acceptance (POL-RST-01). */
    ivs_result_init(&r, &n, IVS_CP1610_UNIT_RESET_ENTRY);
    ivs_unit_reset_entry(&n, &u, &r);
    break;
  case IVS_CP1610_RS_RUNNING:
    ivs_step_running(&n, cfg, lines, &u, &r);
    break;
  case IVS_CP1610_RS_HALTED:
    /* Halted idle is defined with the halt policy (plan 02-15). */
    ivs_result_init(&r, &n, IVS_CP1610_UNIT_HALTED);
    break;
  default:
    ivs_result_init(&r, &n, IVS_CP1610_UNIT_FAULT);
    break;
  }
  if (u.wait_fault) {
    /* The unit is abandoned: architectural state stays as before it, the fault is sticky. */
    n = *s;
    ivs_set_fault(&n, &r, IVS_CP1610_FAULT_BUS_WAIT_LIMIT);
    r.elapsed_cpu_cycles = u.fault_end;
  }
  r.n_accesses = (uint8_t)u.n;
  *s = n;
  *out = r;
  return IVS_OK;
}

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
  if (s->run_state > IVS_CP1610_RS_FAULTED || s->fault_reason > IVS_CP1610_FAULT_UNIMPLEMENTED) {
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

/* Reset entry unit: IAB at offset 1 loads all 16 bits of R7 (pdf 27); other registers are
 * untouched; the first instruction always runs (pdf 22), so prev_interruptible is 0. */
static void ivs_unit_reset_entry(ivs_cp1610 *n, ivs_unit *u, ivs_cp1610_result *r)
{
  n->r[7] = ivs_access(u, IVS_RESET_IAB_OFFSET, IVS_ACC_IAB, IVS_ACC_F_RESET, 0u, 0u);
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

/* Row semantics. Control group (pdf 56): HLT sets the stop latch evaluated at a later
 * boundary (POL-HLT-01, C40; evaluation is plan 02-15); SDBD sets DBD for the next unit
 * (consumption is plan 02-13); EIS enables and DIS disables the interrupt system (INTFF,
 * POL-INT-03); TCI pulses the TCI pin; CLRC clears C, SETC sets C. Single-register forms
 * (pdf 56, footnote on pdf 51): GSWD writes the status word into R0-R3 with bits 4 and 12 =
 * C, 5 and 13 = OV, 6 and 14 = Z, 7 and 15 = S and bits 0-3, 8-11 = 0; NOP does nothing; SIN
 * pulses PCIT; RSWD loads C, OV, Z, S from bits 4, 5, 6, 7 of register SSS (for SSS = 7 that
 * is the already incremented PC, an inference no vector asserts). Only the status change
 * column's flags move. Strobe records carry no address or data. */
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
    n->r[ddd] = ivs_op_add(n, n->r[ddd], n->r[sss]);
    break;
  default:
    __builtin_trap();
  }
  return extra;
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
  r->opcode = word;
  if (u->wait_fault) {
    return;
  }
  row = ivs_cp1610_decode(word);
  if (row == NULL || row->handler == IVS_CP1610_OP_NONE) {
    /* Sticky fault: PC stays at the offending word; the fetch is traced and counted. */
    r->elapsed_cpu_cycles = IVS_FETCH_CYCLES + u->shift;
    ivs_set_fault(n, r, IVS_CP1610_FAULT_UNIMPLEMENTED);
    return;
  }
  n->r[7] = IVS_U16((uint32_t)pc + 1u);
  extra = ivs_exec(n, u, r, row, word);
  n->prev_interruptible = row->interruptible;
  r->elapsed_cpu_cycles = (uint32_t)row->base_cycles + extra + u->shift;
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
  /* Lines are validated now and acted on with the interrupt and bus policies (plan 02-14). */
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
    ivs_result_init(&r, &n, IVS_CP1610_UNIT_RESET_ENTRY);
    ivs_unit_reset_entry(&n, &u, &r);
    break;
  case IVS_CP1610_RS_RUNNING:
    ivs_result_init(&r, &n, IVS_CP1610_UNIT_INSTR);
    ivs_unit_instr(&n, &u, &r);
    break;
  case IVS_CP1610_RS_HALTED:
    /* Halted idle is defined with the halt policy (plan 02-14). */
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

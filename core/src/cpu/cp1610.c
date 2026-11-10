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

/* Per-unit bookkeeping: the port, the optional log and the running access count. */
typedef struct ivs_unit {
  const ivs_cp1610_port *port;
  ivs_cp1610_access *log;
  uint32_t n;
} ivs_unit;

/* One bus access: the bus port sees the record first; the log receives the same record with
 * the transferred value filled in (read reply, or the written value). */
static uint16_t ivs_access(ivs_unit *u, uint32_t offset, uint8_t kind, uint8_t flags, uint16_t addr,
                           uint16_t data)
{
  ivs_cp1610_access a;
  ivs_bus_reply reply;
  a.offset_cpu_cycles = offset;
  a.addr = addr;
  a.data = data;
  a.wait_cpu_cycles = 0u;
  a.kind = kind;
  a.flags = flags;
  reply = u->port->bus(u->port->ctx, &a);
  if (kind != IVS_ACC_WRITE && kind != IVS_ACC_DW) {
    a.data = reply.data;
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

/* Instruction unit: FETCH at offset 0 from R7, PC advances, decode bits 9..0. */
static void ivs_unit_instr(ivs_cp1610 *n, ivs_unit *u, ivs_cp1610_result *r)
{
  uint16_t pc = n->r[7];
  uint16_t word = ivs_access(u, 0u, IVS_ACC_FETCH, 0u, pc, 0u);
  const ivs_cp1610_row *row;
  r->opcode = word;
  r->elapsed_cpu_cycles = IVS_FETCH_CYCLES;
  row = ivs_cp1610_decode(word);
  if (row == NULL) {
    /* Sticky fault: PC stays at the offending word; the fetch is traced and counted. */
    ivs_set_fault(n, r, IVS_CP1610_FAULT_UNIMPLEMENTED);
    return;
  }
  n->r[7] = IVS_U16((uint32_t)pc + 1u);
  /* Row handlers arrive with their rows. */
  ivs_set_fault(n, r, IVS_CP1610_FAULT_UNIMPLEMENTED);
  n->r[7] = pc;
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
  if (ivs_cp1610_state_validate(s) != IVS_OK) {
    return IVS_E_BAD_STATE;
  }
  n = *s;
  u.port = port;
  u.log = log;
  u.n = 0u;
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
  r.n_accesses = (uint8_t)u.n;
  *s = n;
  *out = r;
  return IVS_OK;
}

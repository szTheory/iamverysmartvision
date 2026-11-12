/* SPDX-License-Identifier: MIT */
/* Machine-level bus wrapper (plan 05-04, A-01; plan 05-06, A-01, D-16): the CPU port of every unit the machine steps (see
 * machine/machine.h for the context). It sits between the CPU and the memory system so that a
 * device that needs the machine's time (the PSG catch-up of plan 05-06) or its own observation row
 * (the register-write log of plan 05-08) can act on an access BEFORE the memory system applies it,
 * without the memory system or the CPU knowing about either.
 *
 * Plan 05-06: a WRITE or DW whose address decodes to the PSG region (the memory system's own write
 * view, never an address constant) first catches the PSG up to the write's stamp, the unit start plus
 * the access offset in CPU cycles times the ticks per CPU cycle plus psg.write_strobe_ticks, so the
 * chip steps due before the stamp are made with the old register values and the sample at the stamp
 * itself does not see the write (step first, then write, C78); the masked store then happens in
 * ivs_memsys_bus as before. Every other access is forwarded unchanged.
 * Plan 05-08 (D-13, D-18): after the store the write is appended to the write log (the same stamp is the
 * record's tick, the raw 16-bit bus value its value, the low nibble of the bus address its addr and the
 * kind of the access its source); the domain-19 chain is updated whether or not the call has a log buffer.
 * Plan 05-07: after that store a write that landed on the PSG shape register restarts the envelope
 * (ivs_psg_after_write, the same function ivs_psg_write calls, so a stamp-0 vector write and a guest write
 * cannot diverge); the register slot comes from the memory system's own decode, never from an address here.
 * In plan 05-04 it forwarded every access unchanged: the masked store of a write to the PSG region
 * happens inside ivs_memsys_bus (ivs_psg_reg_store), because the memmap probe drives a bare
 * ivs_memsys with no machine and no wrapper and must measure the same behaviour. The wait reply is
 * the memory system's, which is 0 for every kind (a test asserts it; the PSG time chain depends on
 * it, A-03).
 * The file holds no static or global state (D-07). */
#include "common/ivs_prelude.h"

#include "ivs/ivs_machine.h"
#include "machine/machine.h"

ivs_bus_reply ivs_machine_bus(void *ctx, const ivs_cp1610_access *a)
{
  ivs_machine_bus_ctx *c = (ivs_machine_bus_ctx *)ctx;
  uint32_t slot = IVS_MEMSYS_NO_SLOT;
  uint64_t stamp = 0u;
  ivs_bus_reply reply;
  if (a->kind == IVS_ACC_WRITE || a->kind == IVS_ACC_DW) {
    ivs_memsys_desc d;
    if (ivs_memsys_describe(c->m->mem.profile, IVS_MEMSYS_VIEW_W, a->addr, &d) == IVS_OK &&
        d.phys == IVS_MEMSYS_PHYS_PSG && c->sink != NULL) { /* c->psg is set with the sink (a context without a sink is a bare wrapper test) */
      stamp = c->unit_start_tick + (uint64_t)a->offset_cpu_cycles * (uint64_t)c->ticks_per_cpu_cycle +
              (uint64_t)c->psg->write_strobe_ticks;
      ivs_psg_catch_up(c->psg, &c->m->psg, c->m->mem.psg_reg, &c->m->obs.audio, stamp, c->sink);
      slot = ivs_memsys_psg_write_slot_internal(&c->m->mem, a->addr);
    }
  }
  reply = ivs_memsys_bus(&c->m->mem, a);
  if (slot != IVS_MEMSYS_NO_SLOT) {
    uint64_t tick = stamp;
    ivs_psg_after_write(c->psg, &c->m->psg, c->m->mem.psg_reg, slot);
#ifdef IVS_FAULT_PSG_LOG_TICK_UNIT_START
    tick = c->unit_start_tick; /* named fault: the record carries the start of the unit, not the write's cycle */
#endif
#ifdef IVS_FAULT_PSG_LOG_TICK_SLICE_RELATIVE
    /* named fault: the record carries the tick relative to the first write logged by this call */
    if (c->sink->log_records == 0u) {
      c->sink->base_tick = stamp;
    }
    tick = stamp - c->sink->base_tick;
#endif
    ivs_psg_log_append(c->sink, &c->m->obs.psglog, tick, (uint32_t)a->data, a->addr & 0x0Fu,
                       a->kind == IVS_ACC_DW ? IVS_PSGLOG_SRC_CPU_DW : IVS_PSGLOG_SRC_CPU_WRITE);
  }
  return reply;
}

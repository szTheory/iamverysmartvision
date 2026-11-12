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
 * Plan 06-06 (D-04, D-05, D-06, A-07): an access whose hook class (ivs_memsys_hook_class_internal, generated from the
 * profile) is STIC_REGS, GMEM, BACKTAB or STIC_ALIAS_READ is stamped (unit start + offset x ticks per CPU cycle +
 * access_strobe_ticks), the STIC is caught up to the stamp (an event at the stamp is applied first: step first, then
 * act), and then the gate decides: a closed gate means the access is BLOCKED (the memory system's own closed-gate path
 * drops a write and returns the undriven fill for a read; here the saturating blocked counter of the access kind counts
 * it and no effect is applied); an open gate applies the effects of stic.access_effects (the display-enable strobe sets
 * the latch, a mode-select write sets Foreground/Background) before the memory system serves the access. A READ or FETCH
 * whose ADDRESS is a member of stic.mode_select_addrs is a mode-select read, decided on the address before any decode: with
 * the register gate open the mode becomes Color Stack, with it closed the access counts as a blocked read (policy C84, the
 * STIC saw it and could not serve it); an access of class STIC_ALIAS_READ whose address is not a member is class NONE. The
 * wrapper never alters a reply: the CPU receives what the memory system returns (the cartridge word or the undriven
 * fill for the alias addresses). BACKTAB accesses catch up only. A context without params (c->stic NULL) is a bare wrapper.
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
  if (c->stic != NULL) {
    uint32_t cls = ivs_memsys_hook_class_internal(&c->m->mem, a->kind, a->addr);
    uint32_t is_read = (a->kind == IVS_ACC_READ || a->kind == IVS_ACC_FETCH || a->kind == IVS_ACC_ADAR) ? 1u : 0u; /* ADAR: the operand read of a direct-addressed instruction (MVI $0021, R0) */
    uint32_t is_write = (a->kind == IVS_ACC_WRITE || a->kind == IVS_ACC_DW) ? 1u : 0u;
    uint32_t member = is_read != 0u ? ivs_stic_is_mode_select_addr(c->stic, a->addr) : 0u;
#ifdef IVS_FAULT_STIC_ALIAS_NOT_HOOKED
    if (cls == IVS_MEMSYS_HOOK_STIC_ALIAS_READ) {
      cls = IVS_MEMSYS_HOOK_NONE; /* named fault: the alias read class is not hooked, a read of the alias never reaches the STIC */
    }
#endif
#ifdef IVS_FAULT_STIC_BACKTAB_WRITE_NOT_HOOKED
    if (cls == IVS_MEMSYS_HOOK_BACKTAB) {
      cls = IVS_MEMSYS_HOOK_NONE; /* named fault: a BACKTAB access is not caught up first, so a store sees the row latch of the unit end */
    }
#endif
    if (cls == IVS_MEMSYS_HOOK_STIC_ALIAS_READ && member == 0u) {
      cls = IVS_MEMSYS_HOOK_NONE; /* a granule neighbour of a mode-select address: no effect, no count (A-07) */
    }
    if (cls == IVS_MEMSYS_HOOK_STIC_REGS || cls == IVS_MEMSYS_HOOK_GMEM || cls == IVS_MEMSYS_HOOK_BACKTAB ||
        cls == IVS_MEMSYS_HOOK_STIC_ALIAS_READ) {
      uint64_t at = c->unit_start_tick + (uint64_t)a->offset_cpu_cycles * (uint64_t)c->ticks_per_cpu_cycle +
                    (uint64_t)c->stic->access_strobe_ticks;
      ivs_stic_catch_up(c->stic, &c->m->stic, &c->m->mem, at, NULL);
      if (cls != IVS_MEMSYS_HOOK_BACKTAB && (is_read != 0u || is_write != 0u)) {
        uint32_t gate = cls == IVS_MEMSYS_HOOK_GMEM ? IVS_MEMSYS_GATE_GMEM : IVS_MEMSYS_GATE_STIC_REGS;
        uint32_t open = c->m->mem.gate_open[gate];
        if (open == 0u) {
          ivs_stic_count_blocked(&c->m->stic, is_read);
        }
#ifdef IVS_FAULT_STIC_MODE_SWITCH_OUTSIDE_WINDOW
        open = 1u; /* named fault: the effects are applied even when the gate is closed */
#endif
        if (open != 0u && cls != IVS_MEMSYS_HOOK_GMEM) {
          ivs_stic_apply_effect(&c->m->stic,
                                ivs_stic_effect(c->stic, is_read != 0u ? IVS_STIC_ACCESS_READ : IVS_STIC_ACCESS_WRITE, a->addr));
        }
      }
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

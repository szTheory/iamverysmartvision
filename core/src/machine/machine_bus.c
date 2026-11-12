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
 * In plan 05-04 it forwarded every access unchanged: the masked store of a write to the PSG region
 * happens inside ivs_memsys_bus (ivs_psg_reg_store), because the memmap probe drives a bare
 * ivs_memsys with no machine and no wrapper and must measure the same behaviour. The wait reply is
 * the memory system's, which is 0 for every kind (a test asserts it; the PSG time chain depends on
 * it, A-03).
 * The file holds no static or global state (D-07). */
#include "common/ivs_prelude.h"

#include "machine/machine.h"

ivs_bus_reply ivs_machine_bus(void *ctx, const ivs_cp1610_access *a)
{
  ivs_machine_bus_ctx *c = (ivs_machine_bus_ctx *)ctx;
  if (a->kind == IVS_ACC_WRITE || a->kind == IVS_ACC_DW) {
    ivs_memsys_desc d;
    if (ivs_memsys_describe(c->m->mem.profile, IVS_MEMSYS_VIEW_W, a->addr, &d) == IVS_OK &&
        d.phys == IVS_MEMSYS_PHYS_PSG && c->psg != NULL && c->sink != NULL) {
      uint64_t stamp = c->unit_start_tick + (uint64_t)a->offset_cpu_cycles * (uint64_t)c->ticks_per_cpu_cycle +
                       (uint64_t)c->psg->write_strobe_ticks;
      ivs_psg_catch_up(c->psg, &c->m->psg, c->m->mem.psg_reg, &c->m->obs.audio, stamp, c->sink);
    }
  }
  return ivs_memsys_bus(&c->m->mem, a);
}

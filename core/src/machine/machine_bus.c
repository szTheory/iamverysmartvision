/* SPDX-License-Identifier: MIT */
/* Machine-level bus wrapper (plan 05-04, A-01): the CPU port of every unit the machine steps (see
 * machine/machine.h for the context). It sits between the CPU and the memory system so that a
 * device that needs the machine's time (the PSG catch-up of plan 05-06) or its own observation row
 * (the register-write log of plan 05-08) can act on an access BEFORE the memory system applies it,
 * without the memory system or the CPU knowing about either.
 *
 * In plan 05-04 it forwards every access unchanged: the masked store of a write to the PSG region
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
  return ivs_memsys_bus(&c->m->mem, a);
}

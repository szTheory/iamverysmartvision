/* SPDX-License-Identifier: MIT */
/* Suite stic, family hook (plan 06-06; D-04, A-07, T-06-07): the generated hook class tables judged over ALL 65,536 addresses
 * and every access kind against an expectation computed from the independent literals (the memory-map rows of devices psg,
 * stic, grom and gram, the register literal's mode-select row and the geometry rows of the BACKTAB span), never from the
 * generated tables: a read-like kind (READ, FETCH, ADAR, the operand read of a direct-addressed instruction) uses the read
 * view's classes, a write-like kind (WRITE, DW) the write view's, every other kind is NONE. The mode-select addresses of the
 * core equal the literal's (the write-alias bases plus the mode-select register), and each is a read-sensitive address. */
#include <stdint.h>
#include <string.h>

#include "ivs/ivs.h"
#include "machine/machine.h"
#include "stic_cases.h"

static ivs_memsys sh_mem;

static int sh_completeness(void)
{
  static const uint8_t read_kinds[3] = {IVS_ACC_READ, IVS_ACC_FETCH, IVS_ACC_ADAR};
  static const uint8_t write_kinds[2] = {IVS_ACC_WRITE, IVS_ACC_DW};
  static const uint8_t other_kinds[4] = {IVS_ACC_INTAK, IVS_ACC_IAB, IVS_ACC_STROBE_TCI, IVS_ACC_STROBE_PCIT};
  uint32_t a;
  uint32_t k;
  const ivs_stic_params *p = ivs_stic_params_get(SC_PROFILE);
  uint32_t seen5 = 0u;
  SC_CHECK(ivs_memsys_init(&sh_mem, SC_PROFILE) == IVS_OK);
  for (a = 0u; a < 0x10000u; a++) {
    uint32_t rd = sl_hook(0, a >> 4);
    uint32_t wr = sl_hook(1, a >> 4);
    for (k = 0u; k < 3u; k++) {
      if (ivs_memsys_hook_class_internal(&sh_mem, read_kinds[k], a) != rd) {
        ivs_test_diag("address %04X kind %u: class %u, the literals give %u", (unsigned)a, (unsigned)read_kinds[k],
                      (unsigned)ivs_memsys_hook_class_internal(&sh_mem, read_kinds[k], a), (unsigned)rd);
        return IVS_TEST_FAIL;
      }
    }
    for (k = 0u; k < 2u; k++) {
      if (ivs_memsys_hook_class_internal(&sh_mem, write_kinds[k], a) != wr) {
        ivs_test_diag("address %04X kind %u: class %u, the literals give %u", (unsigned)a, (unsigned)write_kinds[k],
                      (unsigned)ivs_memsys_hook_class_internal(&sh_mem, write_kinds[k], a), (unsigned)wr);
        return IVS_TEST_FAIL;
      }
    }
    for (k = 0u; k < 4u; k++) {
      SC_CHECK(ivs_memsys_hook_class_internal(&sh_mem, other_kinds[k], a) == IVS_MEMSYS_HOOK_NONE);
    }
    if (rd == IVS_MEMSYS_HOOK_STIC_ALIAS_READ) {
      seen5++;
      SC_CHECK(wr != IVS_MEMSYS_HOOK_STIC_ALIAS_READ); /* the class exists in the read view only */
    }
  }
  SC_CHECK(seen5 == 3u * 16u); /* three granules, the write aliases' mode-select addresses ($4021, $8021, $C021) */
  /* every mode-select address of the literal is a mode-select address of the core, a read-sensitive one */
  SC_CHECK(p->mode_addr_count == sl_mode_count());
  for (k = 0u; k < sl_mode_count(); k++) {
    uint32_t m = sl_mode_addr(k);
    uint32_t cls = ivs_memsys_hook_class_internal(&sh_mem, IVS_ACC_READ, m);
    SC_CHECK(ivs_stic_is_mode_select_addr(p, m) == 1u);
    SC_CHECK(cls == IVS_MEMSYS_HOOK_STIC_REGS || cls == IVS_MEMSYS_HOOK_STIC_ALIAS_READ);
    SC_CHECK(ivs_stic_effect(p, IVS_STIC_ACCESS_READ, m) == IVS_STIC_EFFECT_MODE_CS);
  }
  return IVS_TEST_PASS;
}

const ivs_test_case ivs_stic_hook_cases[] = {
    {"stic.hook.completeness", sh_completeness, NULL},
};
const size_t ivs_stic_hook_count = sizeof ivs_stic_hook_cases / sizeof ivs_stic_hook_cases[0];

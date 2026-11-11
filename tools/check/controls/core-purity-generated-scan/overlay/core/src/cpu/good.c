/* SPDX-License-Identifier: MIT */
/* Control core-purity-generated-scan: a stand-in core that includes a listed generated table;
 * the generator writes a floating literal into it, which the source-layer scan of the
 * generated text must report (A-06, H2 c-d, plan 03-03). */
#include "common/ivs_prelude.h"
#include "ivs_toy.inc"

int ivs_good_value(void);

int ivs_good_value(void)
{
  int v = (int)ivs_toy_table[0];
#ifdef IVS_FAULT_GOOD_FAULT
  v = 0;
#endif
#ifdef IVS_FAULT_GOOD_FAULT2
  v = 2;
#endif
  return v;
}

/* SPDX-License-Identifier: MIT */
/* Control core-purity-include-outside: a stand-in core that includes a header which exists in
 * the tree only outside core/src and core/include (here under tests/), so it is neither a
 * public nor an internal core header (H1, plan 03-03). */
#include "common/ivs_prelude.h"
#include "other/thing.h"

int ivs_good_value(void);

int ivs_good_value(void)
{
  int v = IVS_OTHER_THING;
#ifdef IVS_FAULT_GOOD_FAULT
  v = 0;
#endif
#ifdef IVS_FAULT_GOOD_FAULT2
  v = 2;
#endif
  return v;
}

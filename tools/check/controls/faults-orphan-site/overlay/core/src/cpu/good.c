/* SPDX-License-Identifier: MIT */
/* Known-good control tree stand-in core source with two named-fault sites. */
#include "common/ivs_prelude.h"

int ivs_good_value(void);

int ivs_good_value(void)
{
  int v = 1;
#ifdef IVS_FAULT_GOOD_FAULT
  v = 0;
#endif
#ifdef IVS_FAULT_GOOD_FAULT2
  v = 2;
#endif
#if defined(IVS_FAULT_ORPHAN)
  v = 2;
#endif
  return v;
}

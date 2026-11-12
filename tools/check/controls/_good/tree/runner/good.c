/* SPDX-License-Identifier: MIT */
/* Known-good control tree stand-in runner source with one named-fault site (D-24, plan 04-03):
 * the faults-verdict site scan covers runner/ as well as core/src. It also includes the one tools/ header the
 * runner-scan include rule allows (plan 05-10, D-21): ivs_refpcm.h. */
#include "ivs_refpcm.h"

int ivs_good_runner_value(void);

int ivs_good_runner_value(void)
{
  int v = 1;
#ifdef IVS_FAULT_GOOD_RUNNER
  v = 0;
#endif
  return v;
}

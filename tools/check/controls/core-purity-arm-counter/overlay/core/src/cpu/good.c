/* SPDX-License-Identifier: MIT */
/* Control core-purity-arm-counter: a stand-in core that reads the AArch64 virtual counter through
 * a compiler intrinsic, an inline instruction that imports no symbol (review WR-04). */
#include "common/ivs_prelude.h"

unsigned long long ivs_good_value(void);

unsigned long long ivs_good_value(void)
{
  return __builtin_arm_rsr64("cntvct_el0");
}

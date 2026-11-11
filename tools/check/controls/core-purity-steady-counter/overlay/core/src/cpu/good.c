/* SPDX-License-Identifier: MIT */
/* Control core-purity-steady-counter: a stand-in core that reads a host counter through the
 * __builtin_readsteadycounter intrinsic, which imports no symbol (review WR-04). */
#include "common/ivs_prelude.h"

unsigned long long ivs_good_value(void);

unsigned long long ivs_good_value(void)
{
  return __builtin_readsteadycounter();
}

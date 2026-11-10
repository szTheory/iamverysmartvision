/* SPDX-License-Identifier: MIT */
/* Control core-purity-prelude: a stand-in core whose first include is not the prelude. */
#include <stdint.h>
#include "common/ivs_prelude.h"

int ivs_good_value(void);

int ivs_good_value(void)
{
  return (int)INT8_C(1);
}

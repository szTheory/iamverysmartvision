/* SPDX-License-Identifier: MIT */
/* Control core-purity-header: a stand-in core that includes a forbidden standard header after
 * the prelude. */
#include "common/ivs_prelude.h"
#include <stdio.h>

int ivs_good_value(void);

int ivs_good_value(void)
{
  return 1;
}

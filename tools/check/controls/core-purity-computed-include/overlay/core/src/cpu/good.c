/* SPDX-License-Identifier: MIT */
/* Control core-purity-computed-include: a stand-in core that names a forbidden standard header
 * through a computed include, an operand that is neither <...> nor "..." (review WR-02). */
#include "common/ivs_prelude.h"
#define IVS_HOST_HEADER <stdio.h>
#include IVS_HOST_HEADER

int ivs_good_value(void);

int ivs_good_value(void)
{
  return (int)sizeof(FILE) + EOF;
}

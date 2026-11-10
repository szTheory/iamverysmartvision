/* SPDX-License-Identifier: MIT */
/* Control core-purity-alloc: a stand-in core that allocates host memory. The prototype is
 * declared locally (no header); the pointer is returned so -O2 cannot drop the call. */
#include "common/ivs_prelude.h"

extern void *malloc(size_t);
void *ivs_good_value(void);

void *ivs_good_value(void)
{
  return malloc(16u);
}

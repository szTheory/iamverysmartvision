/* SPDX-License-Identifier: MIT */
/* Control core-purity-rand: a stand-in core that draws host randomness. The prototype is
 * declared locally (no header), so only the symbol layer can catch it. */
#include "common/ivs_prelude.h"

extern int rand(void);
int ivs_good_value(void);

int ivs_good_value(void)
{
  return rand();
}

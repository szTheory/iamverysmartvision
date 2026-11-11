/* SPDX-License-Identifier: MIT */
/* Control core-purity-clock: a stand-in core that reads a host clock. The prototype is declared
 * locally (no header), so only the symbol layer can catch it. */
#include "common/ivs_prelude.h"

extern int clock_gettime(int, void *);
int ivs_good_value(void);

int ivs_good_value(void)
{
  unsigned char ts[16];
  return clock_gettime(0, ts);
}

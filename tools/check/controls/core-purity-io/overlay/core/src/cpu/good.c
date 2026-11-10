/* SPDX-License-Identifier: MIT */
/* Control core-purity-io: a stand-in core that writes to a host descriptor. The prototype is
 * declared locally (no header), so only the symbol layer can catch it. */
#include "common/ivs_prelude.h"

extern long write(int, const void *, unsigned long);
long ivs_good_value(void);

long ivs_good_value(void)
{
  return write(1, "x", 1u);
}

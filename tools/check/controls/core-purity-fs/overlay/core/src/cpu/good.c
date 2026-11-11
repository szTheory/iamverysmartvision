/* SPDX-License-Identifier: MIT */
/* Control core-purity-fs: a stand-in core that opens a host file. The prototype is declared
 * locally (no header), so only the symbol layer can catch it. */
#include "common/ivs_prelude.h"

extern void *fopen(const char *, const char *);
void *ivs_good_value(void);

void *ivs_good_value(void)
{
  return fopen("state", "r");
}

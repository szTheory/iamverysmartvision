/* SPDX-License-Identifier: MIT */
/* Control core-purity-asm: a stand-in core with inline assembly. */
#include "common/ivs_prelude.h"

int ivs_good_value(void);

int ivs_good_value(void)
{
  __asm__ volatile ("");
  return 1;
}

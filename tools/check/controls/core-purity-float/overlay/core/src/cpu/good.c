/* SPDX-License-Identifier: MIT */
/* Control core-purity-float: a stand-in core with a floating local. The prelude poisons the
 * word, so a compile would fail; the source layer must report it first, before any compile. */
#include "common/ivs_prelude.h"

int ivs_good_value(void);

int ivs_good_value(void)
{
  float f = 3;
  return (int)f;
}

/* SPDX-License-Identifier: MIT */
/* Control core-purity-generated-unlisted: a stand-in core that includes a build-tree generated
 * file whose basename is not listed in data/expected/core-generated.ids (A-06, plan 03-03). */
#include "common/ivs_prelude.h"
#include "ivs_unlisted.inc"

int ivs_good_value(void);

int ivs_good_value(void)
{
  int v = 1;
#ifdef IVS_FAULT_GOOD_FAULT
  v = 0;
#endif
#ifdef IVS_FAULT_GOOD_FAULT2
  v = 2;
#endif
  return v;
}

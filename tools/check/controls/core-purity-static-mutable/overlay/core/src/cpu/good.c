/* SPDX-License-Identifier: MIT */
/* Control core-purity-static-mutable (D-07, plan 03-03): a stand-in core with a file-scope
 * writable static that a function increments, which would be state shared by every machine
 * instance. The writable-data rule of the symbol layer must report it. The fault sites of the
 * good tree are kept. */
#include "common/ivs_prelude.h"

static unsigned ivs_good_count;

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
  ivs_good_count++;
  return v + (int)(ivs_good_count & 0u);
}

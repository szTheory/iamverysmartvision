/* SPDX-License-Identifier: MIT */
/* Known-good control tree stand-in core source with one named-fault site. It also holds a hex
 * constant and a page cite (S3.2.6, pdf 49) that the c-purity float-literal rule must not flag. */
#include "common/ivs_prelude.h"

int ivs_good_value(void);

int ivs_good_value(void)
{
  int v = 1;
#ifdef IVS_FAULT_GOOD_FAULT
  v = 0;
#endif
  return v + (int)(0x3E4u & 0u);
}

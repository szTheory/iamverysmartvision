/* SPDX-License-Identifier: MIT */
/* Control core-purity-hollow: a stand-in core whose every function is static, so the archive
 * defines no external symbol and the check would vacuously pass on it. */
#include "common/ivs_prelude.h"

static int ivs_good_value(void)
{
  return 1;
}

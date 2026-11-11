/* SPDX-License-Identifier: MIT */
/* Control core-purity-include-digraph: a stand-in core that names a forbidden standard header
 * through the `%:` digraph spelling of `#` (C17 6.4.6p3; review WR-04). */
#include "common/ivs_prelude.h"
%:include <stdio.h>

int ivs_good_value(void);

int ivs_good_value(void)
{
  return (int)BUFSIZ;
}

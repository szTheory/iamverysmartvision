/* SPDX-License-Identifier: MIT */
/* Control core-purity-quoted-header: a stand-in core that includes a forbidden standard header
 * through a quoted include, which resolves on the system path (review WR-02). */
#include "common/ivs_prelude.h"
#include "stdio.h"

int ivs_good_value(void);

int ivs_good_value(void)
{
  return (int)sizeof(FILE) + EOF;
}

/* SPDX-License-Identifier: MIT */
/* Control core-purity-float-literal: a stand-in core with a floating literal in an integer
 * expression (no floating type word, so only the literal rule can catch it). */
#include "common/ivs_prelude.h"

int ivs_good_value(void);

int ivs_good_value(void)
{
  return (int)(2.5 * 2);
}

/* SPDX-License-Identifier: MIT */
/* Control core-purity-hw-random: a stand-in core that takes hardware randomness and the TSC through
 * x86 intrinsics (rdrand, rdseed, rdtscp), inline instructions that import no symbol (review WR-04). */
#include "common/ivs_prelude.h"

unsigned int ivs_good_value(void);

unsigned int ivs_good_value(void)
{
  unsigned int v = 0u;
  unsigned int aux = 0u;
  (void)__builtin_ia32_rdrand32_step(&v);
  (void)__builtin_ia32_rdseed32_step(&v);
  return v + (unsigned int)__builtin_ia32_rdtscp(&aux);
}

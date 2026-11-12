/* SPDX-License-Identifier: MIT */
/* Control core-symbols-pcm-defined (plan 05-10, D7 decided as D-21, A-07): a stand-in core that DEFINES a
 * function whose name carries refpcm. No PCM, resampling, filtering or WAV code may live in the core; the
 * defined-symbol rule of the symbol layer must report it. The fault sites of the good tree are kept. */
#include "common/ivs_prelude.h"

int ivs_good_value(void);
int ivs_good_refpcm_convert(void);

int ivs_good_refpcm_convert(void)
{
  return 3;
}

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

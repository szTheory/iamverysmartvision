/* SPDX-License-Identifier: MIT */
/* Control core-symbols-palette-defined (plan 06-09, D-12, D-13): a stand-in core that DEFINES a function whose name carries palette. No
 * palette, RGB, PPM or frame-preview code may live in the core (it produces raw palette indices only; color and presentation belong to the
 * host); the defined-symbol rule of the symbol layer must report it. The fault sites of the good tree are kept. */
#include "common/ivs_prelude.h"

int ivs_good_value(void);
int ivs_good_palette_lookup(void);

int ivs_good_palette_lookup(void)
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

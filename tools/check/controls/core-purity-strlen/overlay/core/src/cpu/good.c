/* SPDX-License-Identifier: MIT */
/* Control core-purity-strlen (RESEARCH Pitfall 4, plan 03-03): a stand-in core with an
 * unbounded scan for a terminating zero over a caller string. No string function is named;
 * at -O2 the compiler recognizes the loop and emits a call to the libc strlen, which the
 * symbol layer must report. The fault sites of the good tree are kept. */
#include "common/ivs_prelude.h"

int ivs_good_value(void);
size_t ivs_good_span(const char *s);

size_t ivs_good_span(const char *s)
{
  size_t n = 0;
  while (s[n] != 0) n++;
  return n;
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

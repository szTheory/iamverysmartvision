/* SPDX-License-Identifier: MIT */
/* runner-scan negative control (plan 04-04): a runner TU includes a core-internal header; the
 * runner may see only the public ivs/<name>.h headers. */
int ivs_good_runner_value(void);
#include "machine/machine.h"

int ivs_good_runner_value(void)
{
  return 1;
}

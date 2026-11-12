/* SPDX-License-Identifier: MIT */
/* runner-scan negative control (plan 04-04): a runner TU other than hostio.c defines a POSIX
 * feature macro; the CMakeLists.txt overlay adds the same name as a compile definition. */
#define _POSIX_C_SOURCE 200809L
int ivs_good_runner_value(void);

int ivs_good_runner_value(void)
{
  return 1;
}

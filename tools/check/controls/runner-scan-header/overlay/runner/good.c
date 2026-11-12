/* SPDX-License-Identifier: MIT */
/* runner-scan negative control (plan 04-04): a runner TU other than hostio.c includes a POSIX
 * header, the class that compiles on Apple clang and fails on hosted gcc/glibc. */
int ivs_good_runner_value(void);
#include <unistd.h>

int ivs_good_runner_value(void)
{
  return 1;
}

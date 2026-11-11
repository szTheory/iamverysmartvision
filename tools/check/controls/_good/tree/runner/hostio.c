/* SPDX-License-Identifier: MIT */
/* Known-good control tree stand-in for the runner's single POSIX file (D-23, plan 04-04):
 * runner-scan exempts runner/hostio.c from the include and feature-macro rules, so this file
 * may define _POSIX_C_SOURCE before its first include and use a POSIX header. */
#define _POSIX_C_SOURCE 200809L
#include <sys/stat.h>

int ivs_good_hostio_is_dir(const struct stat *st);

int ivs_good_hostio_is_dir(const struct stat *st)
{
  return S_ISDIR(st->st_mode) ? 1 : 0;
}

/* SPDX-License-Identifier: MIT */
/* Control c-build-unlisted-bench-source (plan 08-18, D-21): a tracked tools/bench C source that the build's
 * ivs-bench-sources.txt does not list. The c-build unlisted-source guard must report it. */
int ivs_extra_bench_value(void);

int ivs_extra_bench_value(void)
{
  return 4;
}

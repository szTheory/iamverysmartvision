/* SPDX-License-Identifier: MIT */
/* Known-good control tree stand-in runner source without its named-fault site (D-24, plan
 * 04-03): the GOOD_RUNNER id is still listed, so the runner/ site scan must report it. */
int ivs_good_runner_value(void);

int ivs_good_runner_value(void)
{
  return 1;
}

/* SPDX-License-Identifier: MIT */
/* Control c-build-unlisted-asm-source (plan 07-03, D-17): a tracked tools/asm C source that the build's
 * ivs-asm-sources.txt does not list. The c-build unlisted-source guard must report it. */
int ivs_extra_asm_value(void);

int ivs_extra_asm_value(void)
{
  return 3;
}

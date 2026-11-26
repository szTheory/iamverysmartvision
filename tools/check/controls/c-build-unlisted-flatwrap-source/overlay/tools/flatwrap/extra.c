/* SPDX-License-Identifier: MIT */
/* Control c-build-unlisted-flatwrap-source (plan 14-01, D-47): a tracked tools/flatwrap C source that the build's
 * ivs-flatwrap-sources.txt does not list. The c-build unlisted-source guard must report it. */
int ivs_extra_flatwrap_value(void);

int ivs_extra_flatwrap_value(void)
{
  return 4;
}

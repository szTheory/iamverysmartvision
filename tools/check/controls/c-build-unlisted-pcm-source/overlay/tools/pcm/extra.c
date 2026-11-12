/* SPDX-License-Identifier: MIT */
/* Control c-build-unlisted-pcm-source (plan 05-10, D-21, A-07): a tracked tools/pcm C source that the build's
 * ivs-pcm-sources.txt does not list. The c-build unlisted-source guard must report it. */
int ivs_extra_pcm_value(void);

int ivs_extra_pcm_value(void)
{
  return 2;
}

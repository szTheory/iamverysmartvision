/* SPDX-License-Identifier: MIT */
/* Known-good control tree stand-in of the reference PCM converter directory (plan 05-10): listed in the
 * toy build's ivs-pcm-sources.txt, so the c-build unlisted-source guard passes. */
int ivs_good_pcm_value(void);

int ivs_good_pcm_value(void)
{
  return 1;
}

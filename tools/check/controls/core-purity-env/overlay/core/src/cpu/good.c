/* SPDX-License-Identifier: MIT */
/* Control core-purity-env: a stand-in core that reads the host environment. The prototype is
 * declared locally (no header), so only the symbol layer can catch it. */
#include "common/ivs_prelude.h"

extern char *getenv(const char *);
int ivs_good_value(void);

int ivs_good_value(void)
{
  return getenv("IVS_MODE") != NULL;
}

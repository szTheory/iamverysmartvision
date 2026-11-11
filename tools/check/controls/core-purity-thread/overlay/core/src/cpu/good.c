/* SPDX-License-Identifier: MIT */
/* Control core-purity-thread: a stand-in core that starts a host thread. The prototype is
 * declared locally (no header), so only the symbol layer can catch it. */
#include "common/ivs_prelude.h"

extern int pthread_create(void *, const void *, void *(*)(void *), void *);
int ivs_good_value(void);

static void *ivs_good_worker(void *arg)
{
  return arg;
}

int ivs_good_value(void)
{
  unsigned char handle[64];
  return pthread_create(handle, NULL, ivs_good_worker, NULL);
}

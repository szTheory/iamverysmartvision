/* SPDX-License-Identifier: MIT */
/* Control api-call-indented: the toy status header with a second public declaration that is
 * indented (as inside an #ifdef block) and not in the effect table (review IN-06). */
#ifndef IVS_STATUS_H
#define IVS_STATUS_H

#ifndef IVS_API
#define IVS_API
#endif

#define IVS_OK 0
#define IVS_E_BAD_ARG (-1)

IVS_API int ivs_toy(int x);
#ifdef IVS_TOY_EXTRA
  IVS_API int ivs_toy2(void);
#endif

#endif

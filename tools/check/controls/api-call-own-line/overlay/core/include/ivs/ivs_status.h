/* SPDX-License-Identifier: MIT */
/* Control api-call-own-line: the toy status header with a second public declaration whose
 * IVS_API marker stands on its own line, a form the one-line scan cannot name (review IN-06). */
#ifndef IVS_STATUS_H
#define IVS_STATUS_H

#ifndef IVS_API
#define IVS_API
#endif

#define IVS_OK 0
#define IVS_E_BAD_ARG (-1)

IVS_API int ivs_toy(int x);
IVS_API
int ivs_toy2(void);

#endif

/* SPDX-License-Identifier: MIT */
/* Toy public status header of the known-good control tree (plan 03-13): two pinned statuses and
 * one IVS_API declaration, enough for the api-effects step to pass. */
#ifndef IVS_STATUS_H
#define IVS_STATUS_H

#ifndef IVS_API
#define IVS_API
#endif

#define IVS_OK 0
#define IVS_E_BAD_ARG (-1)

IVS_API int ivs_toy(int x);

#endif

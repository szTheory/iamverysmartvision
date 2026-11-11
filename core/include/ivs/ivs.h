/* SPDX-License-Identifier: MIT */
/* Umbrella public header (D-04, D-06, plan 03-09): the one header a host includes. It brings in
 * the status codes and the machine API and defines the API version. ivs_api_version() returns the
 * version the library was built with; a host compares it with IVS_API_VERSION to detect a header
 * and library mismatch. Pre-1.0: no ABI stability promise; struct_size and the append-only
 * status numbers let additions land without a break.
 * Standard headers only. */
#ifndef IVS_H
#define IVS_H

#include <stdint.h>

#include "ivs/ivs_machine.h"
#include "ivs/ivs_status.h"

#ifdef __cplusplus
extern "C" {
#endif

#define IVS_API_VERSION_MAJOR 0u
#define IVS_API_VERSION_MINOR 1u
#define IVS_API_VERSION_PATCH 0u
/* Packed: major << 16 | minor << 8 | patch. */
#define IVS_API_VERSION ((IVS_API_VERSION_MAJOR << 16) | (IVS_API_VERSION_MINOR << 8) | IVS_API_VERSION_PATCH)

/* The packed version of the library (IVS_API_VERSION of the headers it was built with). */
IVS_API uint32_t ivs_api_version(void);

#ifdef __cplusplus
}
#endif

#endif

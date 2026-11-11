/* SPDX-License-Identifier: MIT */
/* Public status header: the first file of the public include tree core/include/ivs (D-06).
 * Status codes of the core API. 0 is success; a negative value is a caller refusal, and a
 * refused call never mutates caller state. Guest faults are not refusals (D-12 of Phase 2).
 * The numbers are frozen and append-only (D-09); the full D-09 code list arrives in plan 03-09.
 * Standard headers only; consumers include it as "ivs/ivs_status.h". */
#ifndef IVS_STATUS_H
#define IVS_STATUS_H

typedef int ivs_status;

#define IVS_OK 0
#define IVS_E_BAD_ARG (-1)
#define IVS_E_BAD_STATE (-2)

#endif

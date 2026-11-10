/* SPDX-License-Identifier: MIT */
/* Status codes of the core API (D-06). 0 is success; a negative value is a caller refusal,
 * and a refused call never mutates caller state. Guest faults are not refusals (D-12). */
#ifndef IVS_STATUS_H
#define IVS_STATUS_H

typedef int ivs_status;

#define IVS_OK 0
#define IVS_E_BAD_ARG (-1)
#define IVS_E_BAD_STATE (-2)

#endif

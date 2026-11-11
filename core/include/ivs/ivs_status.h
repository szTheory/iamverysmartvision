/* SPDX-License-Identifier: MIT */
/* Public status header: the first file of the public include tree core/include/ivs (D-06).
 * Status codes of the core API. 0 is success; a negative value is a caller refusal, and a
 * refused call never mutates caller state. Guest faults are not refusals (D-12 of Phase 2).
 * The numbers are frozen and append-only (D-09): a number is never reused or renumbered.
 * -6..-15 are unassigned; -24..-31 are reserved for later content codes. Detail (reason,
 * segment index, byte offset) travels in an out report struct, never in the status.
 * The numbers are pinned by data/expected/api-statuses.ids (plan 03-09).
 * IVS_API marks every public function (D-06): empty for the Phase 3 static library; a later
 * shared build may define it (visibility or dllexport) before including any ivs/ header.
 * Standard headers only; consumers include it as "ivs/ivs_status.h". */
#ifndef IVS_STATUS_H
#define IVS_STATUS_H

#ifndef IVS_API
#define IVS_API
#endif

#ifdef __cplusplus
extern "C" {
#endif

typedef int ivs_status;

#define IVS_OK 0
#define IVS_E_BAD_ARG (-1)
#define IVS_E_BAD_STATE (-2)
#define IVS_E_BAD_HANDLE (-3)
#define IVS_E_UNSUPPORTED (-4)
#define IVS_E_BUFFER_TOO_SMALL (-5)

/* Content codes (D-09, D-33). */
#define IVS_E_UNKNOWN_PROFILE (-16)
#define IVS_E_MALFORMED (-17)
#define IVS_E_OVERSIZED (-18)
#define IVS_E_OVERLAP (-19)
#define IVS_E_OUT_OF_RANGE (-20)
#define IVS_E_MISSING_SYSTEM_IMAGE (-21)
#define IVS_E_DUPLICATE_LOAD (-22)
#define IVS_E_DIGEST_MISMATCH (-23)

#ifdef __cplusplus
}
#endif

#endif

/* SPDX-License-Identifier: MIT */
/* Control runner-scan-header-tools-other (plan 05-10, D-21, A-07): the include rule allows the quoted operand
 * ivs_refpcm.h and nothing else from tools/. Line 5 is the allowed one (it must not be reported); a
 * neighbour of that name, another tools/ header and the angle form must still fail. */
#include "ivs_refpcm.h"
#include "ivs_refpcm_extra.h"
#include "tools/pack/pack.h"
#include <ivs_refpcm.h>

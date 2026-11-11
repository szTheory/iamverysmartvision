/* SPDX-License-Identifier: MIT */
/* Mandatory first include of every core .c file (D-21, A-09). Only these standard headers
 * are allowed in the core. The poison pragma comes last: placed before <stddef.h> it trips
 * on a standard typedef (RESEARCH Pitfall 1). The core is integer-only. */
#ifndef IVS_PRELUDE_H
#define IVS_PRELUDE_H

#include <limits.h>
#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>
#include <string.h>

#pragma GCC poison float double

#endif

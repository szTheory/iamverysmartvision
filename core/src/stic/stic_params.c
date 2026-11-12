/* SPDX-License-Identifier: MIT */
/* STIC parameter table (plan 06-04, D-03, D-07, D-09, A-11): the rows come from ivs_stic_params.inc, which
 * cmake/IvsSticTable.cmake generates from the profile fields stic.*, video.* and bus.line_sample_lag_cpu_cycles
 * (this file is its only includer). The file holds no static or global state beyond the generated const table
 * (D-07) and no behaviour: plan 06-04 only publishes the data.
 * Named fault: none yet (plan 06-04 task 3 adds STIC_TABLE_SHORT with its test). */
#include "common/ivs_prelude.h"

#include "stic/stic_params.h"

#include "ivs_stic_params.inc"

uint32_t ivs_stic_params_count(void)
{
  return IVS_STIC_PROFILE_COUNT;
}

const ivs_stic_params *ivs_stic_params_get(uint32_t profile)
{
  if (profile >= ivs_stic_params_count()) {
    return NULL;
  }
  return &ivs_stic_params_table[profile];
}

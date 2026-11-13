/* SPDX-License-Identifier: MIT */
/* STIC parameter table (plan 06-04, D-03, D-07, D-09, A-11): the rows come from ivs_stic_params.inc, which
 * cmake/IvsSticTable.cmake generates from the profile fields stic.*, video.* and bus.line_sample_lag_cpu_cycles
 * (this file is its only includer). The file holds no static or global state beyond the generated const table
 * (D-07) and no behaviour: plan 06-04 only publishes the data.
 * Named fault (plan 06-04, suite machine): STIC_TABLE_SHORT. */
#include "common/ivs_prelude.h"

#include "stic/stic_params.h"

#include "ivs_stic_params.inc"

uint32_t ivs_stic_params_count(void)
{
#ifdef IVS_FAULT_STIC_TABLE_SHORT
  return IVS_STIC_PROFILE_COUNT - 1u; /* named fault STIC_TABLE_SHORT: the table covers one profile less than the registry */
#else
  return IVS_STIC_PROFILE_COUNT;
#endif
}

const ivs_stic_params *ivs_stic_params_get(uint32_t profile)
{
  if (profile >= ivs_stic_params_count()) {
    return NULL;
  }
  return &ivs_stic_params_table[profile];
}

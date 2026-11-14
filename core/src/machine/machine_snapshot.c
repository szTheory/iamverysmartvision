/* SPDX-License-Identifier: MIT */
/* The snapshot writer (plan 08-11, D-12, D-13, D-14, docs/formats/ivsn-v1.md): the IVSN v1 container is built here, big-endian, field
 * by field, from the digest field table of machine_digest.c (the single serializer, D-11). This file holds no static or global
 * state and allocates nothing; every length is a 64-bit value on the public side and a 32-bit field in the container.
 * Task 1 of the plan (the tracer) adds the profile parameter digest only: ivs_snapshot_params_digest hashes the four generated
 * table body digests, so a change of any parameter of the profile tables changes the digest and a cite edit changes nothing. */
#include "common/ivs_prelude.h"

#include "common/ivs_sha256.h"
#include "machine/machine.h"

/* The version of the parameter digest preimage (docs/formats/ivsn-v1.md, "Profile identity"). */
#define IVS_PARAMS_DIGEST_VERSION 1u

ivs_status ivs_snapshot_params_digest(uint32_t profile_index, uint8_t out[32])
{
  /* IVSP (4) + be16 version (2) + four 32-byte body digests: the 134-byte preimage is built in one small local. */
  uint8_t pre[4u + 2u + 4u * 32u];
  const uint8_t *d[4];
  uint32_t i;
  if (out == NULL) {
    return IVS_E_BAD_ARG;
  }
  d[0] = ivs_memmap_table_digest(profile_index);
  d[1] = ivs_psg_table_digest(profile_index);
  d[2] = ivs_stic_table_digest(profile_index);
  d[3] = ivs_ctrl_table_digest(profile_index);
  for (i = 0u; i < 4u; i++) {
    if (d[i] == NULL) {
      return IVS_E_BAD_ARG;
    }
  }
  pre[0] = 0x49u; /* I */
  pre[1] = 0x56u; /* V */
  pre[2] = 0x53u; /* S */
  pre[3] = 0x50u; /* P */
  pre[4] = (uint8_t)(IVS_PARAMS_DIGEST_VERSION >> 8);
  pre[5] = (uint8_t)(IVS_PARAMS_DIGEST_VERSION & 0xFFu);
  for (i = 0u; i < 4u; i++) {
    memcpy(pre + 6u + 32u * i, d[i], 32u);
  }
  ivs_sha256_digest(pre, sizeof pre, out);
  return IVS_OK;
}

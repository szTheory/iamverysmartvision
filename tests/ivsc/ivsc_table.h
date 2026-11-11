/* SPDX-License-Identifier: MIT */
/* Row types of the generated IVSC fixture table ivs_cart_table.inc (cmake/IvsCartTable.cmake,
 * plan 03-07), shared by suite ivsc and the cartridge cases of suite machine (plan 03-10). The
 * field order is the order of the generated initializers. Include this header, then the table. */
#ifndef IVS_IVSC_TABLE_H
#define IVS_IVSC_TABLE_H

#include <stddef.h>
#include <stdint.h>

/* Table types of the generated include (field order of its initializers). */
typedef struct ivs_cart_seg {
  uint16_t addr;
  uint32_t len_words;
} ivs_cart_seg;

typedef struct ivs_cart_run {
  uint32_t seg;
  uint16_t word;
  uint32_t count;
} ivs_cart_run;

#define IVS_CART_POSITIVE 0u
#define IVS_CART_BUILD 1u
#define IVS_CART_PATCH 2u

typedef struct ivs_cart_fixture {
  const char *id;
  const char *base; /* patch: the positive record it changes */
  unsigned kind;
  const uint8_t *bytes; /* positive: the container; patch: patched bytes before any reseal */
  size_t n_bytes;
  unsigned reseal;
  const ivs_cart_seg *segs; /* positive: expected segments; build: the recipe's segments */
  size_t n_segs;
  const ivs_cart_run *runs; /* build: word runs in segment order */
  size_t n_runs;
  const uint8_t *trailer;      /* build: the typed 32-byte trailer */
  const uint8_t *whole_sha256; /* positive and build: typed digest of the whole container */
  int status;
  uint32_t reason;
  unsigned has_seg;
  uint32_t seg;
  unsigned has_off;
  uint32_t off;
} ivs_cart_fixture;

#endif

/* SPDX-License-Identifier: MIT */
/* Flat 64K x 16-bit test bus for the CPU suites (D-05, D-14). Every address starts poisoned:
 * a FETCH, READ or ADAR of an address that was neither initialised nor written is counted as
 * an uninit-read and fails the vector. WRITE and DW store. IAB answers the vector's iab value
 * (absent: counted as an unexpected IAB). INTAK and the strobes carry no data. The waits
 * table answers every access at a listed address, of any kind, with its wait; the core alone
 * decides which kinds take it (A-11). Every call is kept in a call log. */
#ifndef IVS_TESTBUS_H
#define IVS_TESTBUS_H

#include <stddef.h>
#include <stdint.h>

#include "cpu/cp1610.h"

#define IVS_TESTBUS_WORDS 65536u
#define IVS_TESTBUS_MAX_WAITS 16u
#define IVS_TESTBUS_CALL_CAP 64u

typedef struct ivs_testbus {
  uint16_t mem[IVS_TESTBUS_WORDS];
  uint8_t valid[IVS_TESTBUS_WORDS]; /* 1: initialised or written */
  uint16_t wait_addr[IVS_TESTBUS_MAX_WAITS];
  uint16_t wait_cycles[IVS_TESTBUS_MAX_WAITS];
  size_t n_waits;
  uint8_t has_iab;
  uint16_t iab;
  uint8_t ebci_true[16]; /* EBCI answer per E code (0..15) */
  uint32_t uninit_reads;
  uint16_t first_uninit_addr;
  uint32_t unexpected_iab;
  ivs_cp1610_access calls[IVS_TESTBUS_CALL_CAP];
  size_t n_calls;   /* calls kept */
  size_t n_dropped; /* calls beyond the cap */
} ivs_testbus;

void ivs_testbus_init(ivs_testbus *b);
void ivs_testbus_poke(ivs_testbus *b, uint16_t addr, uint16_t word);
/* Returns 0, or -1 when the waits table is full. */
int ivs_testbus_add_wait(ivs_testbus *b, uint16_t addr, uint16_t wait_cpu_cycles);
void ivs_testbus_set_iab(ivs_testbus *b, uint16_t value);
void ivs_testbus_set_ebci(ivs_testbus *b, uint8_t e);
ivs_bus_reply ivs_testbus_bus(void *ctx, const ivs_cp1610_access *a);
uint8_t ivs_testbus_ebci(void *ctx, uint8_t e);
/* Port with bus and, when with_ebci is nonzero, the EBCI provider (else NULL = false). */
void ivs_testbus_port(ivs_testbus *b, ivs_cp1610_port *p, int with_ebci);

#endif

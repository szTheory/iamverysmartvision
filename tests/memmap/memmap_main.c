/* SPDX-License-Identifier: MIT */
/* Suite memmap: the profile-driven memory system (D-16..D-20, plan 03-04), driven through its
 * bus function. Expected values come from this file's rule table, keyed on device kind and
 * width, and from addresses written here by hand; never from the generated descriptors.
 * All image words are generated stand-ins built in the test; no rights-bearing bytes. */
#include <stdint.h>
#include <stdio.h>
#include <string.h>

#include "cpu/cp1610.h"
#include "ivs_test.h"
#include "memmap/memsys.h"

/* Fails the case with a diagnostic naming the line and the failed expression. */
#define IVS_CHECK(cond)                                                                                \
  do {                                                                                                 \
    if (!(cond)) {                                                                                     \
      ivs_test_diag("check failed at line %d: %s", __LINE__, #cond);                                   \
      return IVS_TEST_FAIL;                                                                            \
    }                                                                                                  \
  } while (0)

/* Rule table: what a read returns for a stored word, by device kind (D-17, D-B3, D-B9). */
typedef struct mm_rule {
  uint8_t kind;
  uint16_t rmask;
  uint16_t fill;
} mm_rule;

static const mm_rule mm_rules[] = {
    {IVS_MEMSYS_DEVICE_ROM10, 0x03FFu, 0x0000u}, /* 10-bit decle, upper bits driven 0 */
};

static uint16_t mm_expect(uint8_t kind, uint16_t stored)
{
  size_t i;
  for (i = 0; i < sizeof mm_rules / sizeof mm_rules[0]; i++) {
    if (mm_rules[i].kind == kind) {
      return (uint16_t)((stored & mm_rules[i].rmask) | (mm_rules[i].fill & (uint16_t)~mm_rules[i].rmask));
    }
  }
  return 0xFFFFu; /* undriven */
}

#define MM_UNDRIVEN 0xFFFFu

static ivs_memsys mm_m;
static uint8_t mm_exec_img[IVS_MEMSYS_EXEC_BYTES];

static int mm_init(void)
{
  return ivs_memsys_init(&mm_m, 0u) == IVS_OK;
}

static uint16_t mm_bus_read(uint16_t addr)
{
  ivs_cp1610_access a;
  ivs_bus_reply r;
  memset(&a, 0, sizeof a);
  a.kind = IVS_ACC_READ;
  a.addr = addr;
  r = ivs_memsys_bus(&mm_m, &a);
  return r.wait_cpu_cycles == 0u ? r.data : 0x5A5Au;
}

/* A stand-in EXEC image: word i is i ^ 0x0155 (10 bits), word 0 is 0xFFFF (wide). */
static void mm_build_exec(void)
{
  uint32_t i;
  for (i = 0u; i < IVS_MEMSYS_EXEC_WORDS; i++) {
    uint16_t w = (uint16_t)((i ^ 0x0155u) & 0x03FFu);
    if (i == 0u) {
      w = 0xFFFFu;
    }
    mm_exec_img[2u * i] = (uint8_t)(w >> 8);
    mm_exec_img[2u * i + 1u] = (uint8_t)(w & 0xFFu);
  }
}

static int mm_anchor_exec_lo(void)
{
  IVS_CHECK(mm_init());
  mm_build_exec();
  ivs_memsys_install_exec(&mm_m, mm_exec_img);
  IVS_CHECK(mm_bus_read(0x1000u) == mm_expect(IVS_MEMSYS_DEVICE_ROM10, 0xFFFFu));
  IVS_CHECK(mm_bus_read(0x1000u) == 0x03FFu);
  return IVS_TEST_PASS;
}

static int mm_unloaded_exec_reads_ffff(void)
{
  IVS_CHECK(mm_init());
  IVS_CHECK(mm_bus_read(0x1000u) == MM_UNDRIVEN);
  IVS_CHECK(mm_bus_read(0x1FFFu) == MM_UNDRIVEN);
  return IVS_TEST_PASS;
}

static const ivs_test_case mm_cases[] = {
    {"memmap.anchor.exec-lo", mm_anchor_exec_lo, NULL},
    {"memmap.unloaded.exec-reads-ffff", mm_unloaded_exec_reads_ffff, NULL},
};

int main(int argc, char **argv)
{
  ivs_test_suite suite;
  ivs_test_suite_from_cases(&suite, "memmap", mm_cases, sizeof mm_cases / sizeof mm_cases[0]);
  return ivs_test_main(argc, argv, &suite);
}

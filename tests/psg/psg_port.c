/* SPDX-License-Identifier: MIT */
/* Suite psg, family port (AUD-01, INP-01, plan 05-05, D-04, rule PSG-R20): the PSG I/O ports at $01FE
 * and $01FF as a CPU read returns them. Expected values come from the vector file
 * data/psg/vectors/port.txt (ids psg.port.*), turned into a C include by cmake/IvsPsgVectors.cmake
 * (which reads neither the profile nor the core), never from the generated parameter tables or a core
 * output. A vector applies its writes through the memory-system bus (WRITE accesses in the order
 * written: the mixer register $01F8 with its direction bits, the port latches), then drives the pins
 * low as the vector's pin key says (ivs_memsys_set_port_pulldown_internal, the setter the input path
 * uses), then compares every `read:AAAA=HHHH` with a bus READ of that address: input mode reads the idle
 * byte XOR the driven-low mask, output mode the stored latch, the upper byte is 0 (C64). The vectors
 * of kind source cite the 1980 catalog (the pull-up statement, tier 1) and the wiki prose (tier 4); the
 * kind policy rows state the project defaults (POL-PSG-07..10, C65, C73), not hardware evidence.
 *   psg.judge.port-row-changed  the predicate rejects a vector whose expected value was changed (and accepts
 *                               the original) */
#include <stdint.h>
#include <string.h>

#include "cpu/cp1610.h"
#include "ivs/ivs.h"
#include "ivs_test.h"
#include "machine/machine.h"
#include "memmap/memsys.h"
#include "psg_cases.h"

#include "ivs_psg_vectors_port.inc"

static uint32_t pp_profile(void)
{
  uint32_t p;
  for (p = 0u; p < ivs_profile_count(); p++) {
    if (strcmp(ivs_profile_get(p)->id, "intv-2609-ntsc-family-p1") == 0) {
      return p;
    }
  }
  return ivs_profile_count();
}

static uint16_t pp_bus(ivs_memsys *m, uint8_t kind, uint16_t addr, uint16_t data)
{
  ivs_cp1610_access a;
  ivs_bus_reply r;
  memset(&a, 0, sizeof a);
  a.kind = kind;
  a.addr = addr;
  a.data = data;
  r = ivs_memsys_bus(m, &a);
  return r.wait_cpu_cycles == 0u ? r.data : 0x5A5Au;
}

/* 1 when v holds on a fresh memory system; quiet suppresses the diagnostics. */
static int pp_eval(const ivs_pv *v, int quiet)
{
  ivs_memsys m;
  uint32_t k;
  if (ivs_memsys_init(&m, pp_profile()) != IVS_OK) {
    return 0;
  }
  for (k = 0u; k < v->n_writes; k++) {
    if (v->writes[k].step != 0u) {
      return 0;
    }
    (void)pp_bus(&m, IVS_ACC_WRITE, v->writes[k].addr, v->writes[k].value);
  }
  if (v->pin_port >= 0) {
    ivs_memsys_set_port_pulldown_internal(&m, (uint32_t)v->pin_port, v->pin_mask);
  }
  for (k = 0u; k < v->n_reads; k++) {
    uint16_t got = pp_bus(&m, IVS_ACC_READ, v->reads[k].addr, 0u);
    if (got != v->reads[k].value) {
      if (!quiet) {
        ivs_test_diag("%s: bus read of %04X is %04X, vector expects %04X", v->id, (unsigned)v->reads[k].addr,
                      (unsigned)got, (unsigned)v->reads[k].value);
      }
      return 0;
    }
  }
  return 1;
}

static int pp_judge_row_changed(void)
{
  ivs_pv_read altered[4];
  ivs_pv v;
  const ivs_pv *src = &ivs_pv_port[0];
  uint32_t k;
  if (src->n_reads == 0u || src->n_reads > 4u) {
    return IVS_TEST_FAIL;
  }
  if (!pp_eval(src, 0)) {
    ivs_test_diag("the predicate rejects the real vector %s", src->id);
    return IVS_TEST_FAIL;
  }
  v = *src;
  for (k = 0u; k < src->n_reads; k++) {
    altered[k] = src->reads[k];
  }
  altered[0].value = (uint16_t)(altered[0].value ^ 0x0001u);
  v.reads = altered;
  if (pp_eval(&v, 1)) {
    ivs_test_diag("the predicate accepts %s with its first expected value changed", src->id);
    return IVS_TEST_FAIL;
  }
  return IVS_TEST_PASS;
}

const ivs_test_case ivs_psg_port_cases[] = {
    {"psg.judge.port-row-changed", pp_judge_row_changed, NULL},
};
const size_t ivs_psg_port_count = sizeof ivs_psg_port_cases / sizeof ivs_psg_port_cases[0];

size_t ivs_psg_port_vector_count(void)
{
  return (size_t)IVS_PV_port_COUNT;
}

const char *ivs_psg_port_vector_id(size_t i)
{
  return ivs_pv_port[i].id;
}

int ivs_psg_port_vector_run(size_t i)
{
  return pp_eval(&ivs_pv_port[i], 0) ? IVS_TEST_PASS : IVS_TEST_FAIL;
}

const ivs_test_source *ivs_psg_port_sources(size_t *n)
{
  /* The register-address literal is listed by family reg; this family adds its vector file. */
  *n = 1u;
  return &ivs_pv_sources_port[0];
}

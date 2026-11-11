/* SPDX-License-Identifier: MIT */
/* Suite cpu-vectors: runs every JSON vector (D-14) through ivs_cp1610_step on the flat test
 * bus and compares the full state, the full 64K memory image (including which words are
 * initialised), cycles, units, events and the bus sequence. Fields absent from a vector's
 * expect must be unchanged; units and events are asserted only when listed; cycles are not
 * asserted when the generator marked them so. */
#include <stdarg.h>
#include <stdio.h>
#include <string.h>

#include "cpu/cp1610.h"
#include "ivs_test.h"
#include "ivs_testbus.h"
#include "ivs_vec.h"

#include "ivs_vectors.inc"

#define IVS_SEEN_CAP 64u
#define IVS_UNITS_CAP 64u

static ivs_testbus ivs_bus_actual;
static ivs_testbus ivs_bus_expected;

static char ivs_diff[16384];
static size_t ivs_diff_len;

static void ivs_note(const char *fmt, ...)
#if defined(__GNUC__) || defined(__clang__)
    __attribute__((format(printf, 1, 2)))
#endif
    ;

static void ivs_note(const char *fmt, ...)
{
  va_list ap;
  int k;
  if (ivs_diff_len >= sizeof ivs_diff - 4u) {
    return;
  }
  ivs_diff[ivs_diff_len++] = '#';
  ivs_diff[ivs_diff_len++] = ' ';
  va_start(ap, fmt);
  k = vsnprintf(ivs_diff + ivs_diff_len, sizeof ivs_diff - ivs_diff_len - 1u, fmt, ap);
  va_end(ap);
  if (k < 0) {
    return;
  }
  ivs_diff_len += (size_t)k;
  if (ivs_diff_len > sizeof ivs_diff - 2u) {
    ivs_diff_len = sizeof ivs_diff - 2u;
  }
  ivs_diff[ivs_diff_len++] = '\n';
  ivs_diff[ivs_diff_len] = '\0';
}

static const char *ivs_kind_name(uint8_t k)
{
  static const char *const names[] = {"?",     "FETCH", "READ", "WRITE",      "ADAR",
                                      "INTAK", "IAB",   "DW",   "STROBE_TCI", "STROBE_PCIT"};
  return k < sizeof names / sizeof names[0] ? names[k] : "?";
}

static const char *ivs_unit_name(uint8_t k)
{
  static const char *const names[] = {"?",      "INSTR", "INT_ENTRY", "RESET_ENTRY",
                                      "BUS_GRANT", "HALTED", "FAULT", "NOT_RESET"};
  return k < sizeof names / sizeof names[0] ? names[k] : "?";
}

static void ivs_cmp_u(const char *field, unsigned want, unsigned got, int *fail)
{
  if (want != got) {
    ivs_note("field %s: expected 0x%04X, got 0x%04X", field, want, got);
    *fail = 1;
  }
}

static void ivs_cmp_state(const ivs_cp1610 *want, const ivs_cp1610 *got, int *fail)
{
  static const char *const rn[8] = {"r0", "r1", "r2", "r3", "r4", "r5", "r6", "r7"};
  size_t i;
  for (i = 0; i < 8u; i++) {
    ivs_cmp_u(rn[i], want->r[i], got->r[i], fail);
  }
  ivs_cmp_u("status", want->status, got->status, fail);
  ivs_cmp_u("intff", want->intff, got->intff, fail);
  ivs_cmp_u("dbd", want->dbd, got->dbd, fail);
  ivs_cmp_u("prev_interruptible", want->prev_interruptible, got->prev_interruptible, fail);
  ivs_cmp_u("run_state", want->run_state, got->run_state, fail);
  ivs_cmp_u("halt_pending", want->halt_pending, got->halt_pending, fail);
  ivs_cmp_u("fault_reason", want->fault_reason, got->fault_reason, fail);
  ivs_cmp_u("busak", want->busak, got->busak, fail);
  for (i = 0; i < sizeof want->reserved; i++) {
    ivs_cmp_u("reserved", want->reserved[i], got->reserved[i], fail);
  }
}

static void ivs_cmp_memory(const ivs_vec *v, int *fail)
{
  size_t i;
  size_t shown = 0;
  ivs_testbus_init(&ivs_bus_expected);
  for (i = 0; i < v->n_mem; i++) {
    ivs_testbus_poke(&ivs_bus_expected, v->mem[i].addr, v->mem[i].word);
  }
  for (i = 0; i < v->n_expect_mem; i++) {
    ivs_testbus_poke(&ivs_bus_expected, v->expect_mem[i].addr, v->expect_mem[i].word);
  }
  for (i = 0; i < IVS_TESTBUS_WORDS; i++) {
    uint8_t wv = ivs_bus_expected.valid[i];
    uint8_t gv = ivs_bus_actual.valid[i];
    if (wv != gv || (wv && ivs_bus_expected.mem[i] != ivs_bus_actual.mem[i])) {
      *fail = 1;
      if (shown++ < 8u) {
        ivs_note("mem $%04zX: expected %s%04X, got %s%04X", i, wv ? "$" : "poison/", ivs_bus_expected.mem[i],
                 gv ? "$" : "poison/", ivs_bus_actual.mem[i]);
      }
    }
  }
}

static const char *ivs_vec_id(const void *data, size_t i)
{
  (void)data;
  return ivs_vec_table[i].id;
}

static const char *ivs_vec_skip(const void *data, size_t i)
{
  (void)data;
  return ivs_vec_table[i].skip;
}

static int ivs_vec_run(const void *data, size_t i)
{
  const ivs_vec *v = &ivs_vec_table[i];
  ivs_cp1610 s = v->init;
  ivs_cp1610_config cfg;
  ivs_cp1610_port port;
  ivs_cp1610_access log[IVS_CP1610_MAX_ACCESSES];
  ivs_cp1610_access seen[IVS_SEEN_CAP];
  uint32_t seen_cycle[IVS_SEEN_CAP];
  uint8_t units[IVS_UNITS_CAP];
  size_t n_seen = 0;
  size_t k;
  uint32_t total = 0;
  uint8_t events = 0;
  int fail = 0;
  (void)data;

  ivs_diff_len = 0;
  ivs_diff[0] = '\0';
  ivs_cp1610_config_default(&cfg);
  if (v->config_mask & 1u) {
    cfg.interrupt_accept_clears_intff = v->config.interrupt_accept_clears_intff;
  }
  if (v->config_mask & 2u) {
    cfg.release_latency_cpu_cycles = v->config.release_latency_cpu_cycles;
  }
  if (v->config_mask & 4u) {
    cfg.wait_limit_cpu_cycles = v->config.wait_limit_cpu_cycles;
  }
  ivs_testbus_init(&ivs_bus_actual);
  for (k = 0; k < v->n_mem; k++) {
    ivs_testbus_poke(&ivs_bus_actual, v->mem[k].addr, v->mem[k].word);
  }
  for (k = 0; k < v->n_waits; k++) {
    if (ivs_testbus_add_wait(&ivs_bus_actual, v->waits[k].addr, v->waits[k].wait_cpu_cycles) != 0) {
      ivs_note("harness: waits table full");
      fail = 1;
    }
  }
  for (k = 0; k < v->n_ebci; k++) {
    ivs_testbus_set_ebci(&ivs_bus_actual, v->ebci[k]);
  }
  if (v->has_iab) {
    ivs_testbus_set_iab(&ivs_bus_actual, v->iab);
  }
  ivs_testbus_port(&ivs_bus_actual, &port, v->has_ebci);
  if (v->reset && ivs_cp1610_reset(&s) != IVS_OK) {
    ivs_note("reset refused");
    fail = 1;
  }
  if (v->steps > IVS_UNITS_CAP || (v->step_lines != NULL && v->n_step_lines != v->steps)) {
    ivs_note("harness: bad steps or step_lines count");
    return IVS_TEST_FAIL;
  }
  for (k = 0; k < v->steps; k++) {
    ivs_cp1610_result res;
    ivs_cp1610_lines lines = v->step_lines != NULL ? v->step_lines[k] : v->lines;
    size_t j;
    ivs_status rc = ivs_cp1610_step(&s, &cfg, &lines, &port, log, IVS_CP1610_MAX_ACCESSES, &res);
    if (rc != IVS_OK) {
      ivs_note("step %zu refused with status %d", k + 1u, rc);
      fail = 1;
      break;
    }
    for (j = 0; j < res.n_accesses; j++) {
      if (n_seen < IVS_SEEN_CAP) {
        seen[n_seen] = log[j];
        seen_cycle[n_seen] = total + log[j].offset_cpu_cycles;
        n_seen++;
      }
    }
    units[k] = res.unit_kind;
    events = (uint8_t)(events | res.events);
    total += res.elapsed_cpu_cycles;
  }

  ivs_cmp_state(&v->expect, &s, &fail);
  ivs_cmp_memory(v, &fail);
  if (ivs_bus_actual.uninit_reads > 0u) {
    ivs_note("uninit-read: %u read(s) of never-initialised words, first at $%04X",
             (unsigned)ivs_bus_actual.uninit_reads, ivs_bus_actual.first_uninit_addr);
    fail = 1;
  }
  if (ivs_bus_actual.unexpected_iab > 0u) {
    ivs_note("iab-unexpected: the vector supplies no iab value");
    fail = 1;
  }
  if (v->cycles_asserted && total != v->cycles) {
    ivs_note("cycles: expected %u, got %u", (unsigned)v->cycles, (unsigned)total);
    fail = 1;
  }
  if (v->n_units > 0u) {
    if (v->n_units != v->steps) {
      ivs_note("harness: units lists %zu kinds for %u steps", v->n_units, (unsigned)v->steps);
      fail = 1;
    } else {
      for (k = 0; k < v->n_units; k++) {
        if (units[k] != v->units[k]) {
          ivs_note("unit %zu: expected %s, got %s", k + 1u, ivs_unit_name(v->units[k]), ivs_unit_name(units[k]));
          fail = 1;
        }
      }
    }
  }
  if (v->has_events && events != v->events) {
    ivs_note("events: expected 0x%02X, got 0x%02X", (unsigned)v->events, (unsigned)events);
    fail = 1;
  }
  if (v->has_bus) {
    int bus_bad = n_seen != v->n_bus;
    for (k = 0; !bus_bad && k < n_seen; k++) {
      const ivs_vec_bus *b = &v->bus[k];
      if (b->kind != seen[k].kind || b->addr != seen[k].addr || b->data != seen[k].data ||
          (v->bus_cycles_asserted && b->cycle != seen_cycle[k])) {
        bus_bad = 1;
      }
    }
    if (bus_bad) {
      fail = 1;
      ivs_note("bus sequence differs (cycles %s):", v->bus_cycles_asserted ? "asserted" : "not asserted");
      for (k = 0; k < v->n_bus; k++) {
        ivs_note("  expected [%u, %s, $%04X, $%04X]", (unsigned)v->bus[k].cycle, ivs_kind_name(v->bus[k].kind),
                 v->bus[k].addr, v->bus[k].data);
      }
      for (k = 0; k < n_seen; k++) {
        ivs_note("  actual   [%u, %s, $%04X, $%04X]", (unsigned)seen_cycle[k], ivs_kind_name(seen[k].kind),
                 seen[k].addr, seen[k].data);
      }
    }
  }
  if (fail) {
    ivs_test_diag("vector %s (row %s, %s%s%s)", v->id, v->row, v->kind, v->policy ? " " : "",
                  v->policy ? v->policy : "");
    ivs_test_diag("cite: %s", v->cite);
    ivs_test_diag("derivation: %s", v->derivation);
    fputs(ivs_diff, stdout);
    return IVS_TEST_FAIL;
  }
  return IVS_TEST_PASS;
}

int main(int argc, char **argv)
{
  ivs_test_suite suite;
  memset(&suite, 0, sizeof suite);
  suite.name = "cpu-vectors";
  suite.count = ivs_vec_count;
  suite.data = ivs_vec_table;
  suite.id = ivs_vec_id;
  suite.skip = ivs_vec_skip;
  suite.run = ivs_vec_run;
  suite.sources = ivs_vec_sources;
  suite.n_sources = ivs_vec_source_count;
  return ivs_test_main(argc, argv, &suite);
}

/* SPDX-License-Identifier: MIT */
/* Suite psg, family reg (AUD-01, plan 05-04, D-08): the PSG register file at $01F0..$01FF.
 *
 * Expected values come from two sourced text files only, turned into C includes by
 * cmake/IvsPsgVectors.cmake (which reads neither the profile nor the core): the vector files
 * data/psg/vectors/regs.txt (ids psg.reg.*) and reset.txt (ids psg.reset.*), and the register-address
 * literal data/psg/intv-2609-ntsc-family-p1.regmap.literal.txt (16 rows: address, write mask, read
 * mask). A vector applies its writes through the memory-system bus (WRITE accesses, in the order
 * written) and compares each `read:AAAA=HHHH` with the register store of that address, which the
 * internal accessor ivs_memsys_psg_reg_internal returns (the stored word under the read mask, upper
 * byte 0); for $01F0..$01FD the bus read of the same address must give the same value. No assertion
 * here reads $01FE or $01FF through a bus read, peek or the CPU: what a port register reads follows
 * the pins and is plan 05-05; the two latches are read through the internal accessor and only their
 * store behaviour (value & $00FF, any direction) is tested.
 *
 * Fixed cases (hand-written ids):
 *   psg.regmap.matches-literal   the generated ivs_psg_params of the registry profile equal the literal
 *                                (addresses, write masks, read masks); every profile has a row
 *   psg.reset.power-on-zero      a power-on reset of an instance whose storage was poisoned leaves every
 *                                PSG word zero (stores, pull-downs) and reads zero
 *   psg.reset.dirty-file-cleared a power-on reset of an instance with a dirtied register file gives
 *                                storage byte-equal to a fresh instance with the same content
 *   psg.bus.wait-zero            every access kind at every address returns wait 0 (U and g, A-03)
 *   psg.bus.wrapper-forwards     a non-PSG access through the machine bus wrapper equals the same access
 *                                through ivs_memsys_bus: replies and storage
 *   psg.reg.no-mirror            a store changes only its own register word, whose new value is the
 *                                literal's write mask applied; no store elsewhere reaches a register
 *   psg.reg.reads-pure           peeks and bus reads of the registers leave the instance storage unchanged
 *   psg.reg.guest-write-read     a guest MVO of $03FF to $01F4, read back with MVI and stored to $0200,
 *                                leaves $000F there (the tracer of the plan)
 *   psg.judge.mask-row-changed   the predicate that judges a vector rejects a vector whose expected
 *                                value was changed (and accepts the original)
 *   psg.judge.regmap-literal-changed  the predicate that compares the generated masks with the literal
 *                                rejects a literal with one write mask or one read mask changed
 * Test code may use stdio and malloc; the core may not. All image words are generated stand-ins. */
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "cpu/cp1610.h"
#include "ivs/ivs.h"
#include "ivs/ivs_run.h"
#include "ivs_test.h"
#include "machine/machine.h"
#include "machine_cases.h"
#include "memmap/memsys.h"
#include "psg/psg.h"
#include "psg_cases.h"

#include "ivs_psg_vectors_reg.inc"
#include "ivs_psg_vectors_reset.inc"

#define PR_CHECK(cond)                                                                                 \
  do {                                                                                                 \
    if (!(cond)) {                                                                                     \
      ivs_test_diag("check failed at line %d: %s", __LINE__, #cond);                                   \
      return IVS_TEST_FAIL;                                                                            \
    }                                                                                                  \
  } while (0)

#define PR_BASE 0x01F0u
#define PR_REGS 16u
#define PR_POISON 0xA5u

/* ---- helpers ---- */

static uint32_t pr_profile(void)
{
  uint32_t p;
  for (p = 0u; p < ivs_profile_count(); p++) {
    if (strcmp(ivs_profile_get(p)->id, IVS_MS_PROFILE) == 0) {
      return p;
    }
  }
  return ivs_profile_count();
}

static ivs_bus_reply pr_bus(ivs_memsys *m, uint8_t kind, uint8_t flags, uint16_t addr, uint16_t data)
{
  ivs_cp1610_access a;
  memset(&a, 0, sizeof a);
  a.kind = kind;
  a.flags = flags;
  a.addr = addr;
  a.data = data;
  return ivs_memsys_bus(m, &a);
}

static void pr_write(ivs_memsys *m, uint16_t addr, uint16_t value)
{
  (void)pr_bus(m, IVS_ACC_WRITE, 0u, addr, value);
}

/* Bus READ; a reply with a wait gives 0x5A5A, which no vector expects (wait is 0, A-03). */
static uint16_t pr_read(ivs_memsys *m, uint16_t addr)
{
  ivs_bus_reply r = pr_bus(m, IVS_ACC_READ, 0u, addr, 0u);
  return r.wait_cpu_cycles == 0u ? r.data : 0x5A5Au;
}

/* A powered instance of content A in storage poisoned with PR_POISON. 1 on success. */
static int pr_powered(ivs_ms_inst *x)
{
  return ivs_ms_powered(x, IVS_MS_CONTENT_A, PR_POISON);
}

static struct ivs_machine *pr_inst(const ivs_ms_inst *x)
{
  return (struct ivs_machine *)x->m;
}

/* ---- the vector predicate (judged by psg.judge.mask-row-changed) ---- */

/* 1 when every read of v holds on m: the register store of the address (internal accessor) equals the
 * expected value and, below $01FE, so does the bus read; quiet suppresses the diagnostics. */
static int pr_reads_hold(const ivs_pv *v, ivs_memsys *m, int quiet)
{
  uint32_t k;
  for (k = 0u; k < v->n_reads; k++) {
    uint16_t addr = v->reads[k].addr;
    uint16_t want = v->reads[k].value;
    uint16_t store = ivs_memsys_psg_reg_internal(m, (uint32_t)addr - PR_BASE);
    if (store != want) {
      if (!quiet) {
        ivs_test_diag("%s: register store of %04X is %04X, vector expects %04X", v->id, (unsigned)addr,
                      (unsigned)store, (unsigned)want);
      }
      return 0;
    }
    if (addr < 0x01FEu) {
      uint16_t got = pr_read(m, addr);
      if (got != want) {
        if (!quiet) {
          ivs_test_diag("%s: bus read of %04X is %04X, vector expects %04X", v->id, (unsigned)addr,
                        (unsigned)got, (unsigned)want);
        }
        return 0;
      }
    }
  }
  return 1;
}

/* Applies the writes of v in order; 0 when one is stamped after step 0 (no engine in this family). */
static int pr_apply(const ivs_pv *v, ivs_memsys *m)
{
  uint32_t k;
  for (k = 0u; k < v->n_writes; k++) {
    if (v->writes[k].step != 0u) {
      return 0;
    }
    pr_write(m, v->writes[k].addr, v->writes[k].value);
  }
  return 1;
}

/* Runs one vector. via_reset 0: a bare memory system, as the memmap probe uses; 1: an instance
 * whose register words were dirtied and then reset by a power-on reset before the writes. */
static int pr_vector_eval(const ivs_pv *v, int via_reset, int quiet)
{
  ivs_memsys bare;
  ivs_ms_inst x;
  ivs_memsys *m = &bare;
  int ok;
  uint32_t i;
  if (via_reset) {
    struct ivs_machine *im;
    if (!pr_powered(&x)) {
      return 0;
    }
    im = pr_inst(&x);
    for (i = 0u; i < PR_REGS; i++) {
      im->mem.psg_reg[i] = 0xFFFFu;
    }
    if (ivs_machine_reset(x.m, IVS_RESET_POWER_ON) != IVS_OK) {
      ivs_ms_free(&x);
      return 0;
    }
    m = &im->mem;
  } else if (ivs_memsys_init(&bare, pr_profile()) != IVS_OK) {
    return 0;
  }
  ok = pr_apply(v, m) && pr_reads_hold(v, m, quiet);
  if (via_reset) {
    ivs_ms_free(&x);
  }
  return ok;
}

/* ---- vector cases: reg then reset ---- */

size_t ivs_psg_reg_vector_count(void)
{
  return (size_t)IVS_PV_reg_COUNT + (size_t)IVS_PV_reset_COUNT;
}

const char *ivs_psg_reg_vector_id(size_t i)
{
  return i < IVS_PV_reg_COUNT ? ivs_pv_reg[i].id : ivs_pv_reset[i - IVS_PV_reg_COUNT].id;
}

int ivs_psg_reg_vector_run(size_t i)
{
  const ivs_pv *v = i < IVS_PV_reg_COUNT ? &ivs_pv_reg[i] : &ivs_pv_reset[i - IVS_PV_reg_COUNT];
  return pr_vector_eval(v, i >= IVS_PV_reg_COUNT, 0) ? IVS_TEST_PASS : IVS_TEST_FAIL;
}

const ivs_test_source *ivs_psg_reg_sources(size_t *n)
{
  /* The literal is a source of both generated includes; list it once. */
  static ivs_test_source all[3];
  all[0] = ivs_pv_sources_reg[0];
  all[1] = ivs_pv_sources_reg[1];
  all[2] = ivs_pv_sources_reset[0];
  *n = 3u;
  return all;
}

/* ---- psg.regmap.matches-literal and its judge ---- */

/* 1 when the parameters equal the literal rows: address order, write mask and read mask. */
static int pr_regmap_matches(const ivs_psg_params *p, const ivs_pv_lit *lit)
{
  uint32_t i;
  for (i = 0u; i < PR_REGS; i++) {
    if (lit[i].addr != PR_BASE + i || p->reg_wmask[i] != lit[i].wmask || p->reg_rmask[i] != lit[i].rmask) {
      return 0;
    }
  }
  return 1;
}

static int pr_regmap_literal(void)
{
  const ivs_psg_params *p;
  uint32_t q;
  PR_CHECK(pr_profile() < ivs_profile_count());
  p = ivs_psg_params_get(pr_profile());
  PR_CHECK(p != NULL);
  PR_CHECK(pr_regmap_matches(p, ivs_pv_literal));
  for (q = 0u; q < ivs_profile_count(); q++) {
    PR_CHECK(ivs_psg_params_get(q) != NULL);
  }
  PR_CHECK(ivs_psg_params_get(ivs_profile_count()) == NULL);
  return IVS_TEST_PASS;
}

static int pr_judge_regmap(void)
{
  const ivs_psg_params *p = ivs_psg_params_get(pr_profile());
  ivs_pv_lit alt[PR_REGS];
  PR_CHECK(p != NULL);
  memcpy(alt, ivs_pv_literal, sizeof alt);
  PR_CHECK(pr_regmap_matches(p, alt)); /* the positive control */
  alt[10].rmask = 0x000Fu;             /* the one-field reversal of the shape register read mask (C75) */
  PR_CHECK(!pr_regmap_matches(p, alt));
  memcpy(alt, ivs_pv_literal, sizeof alt);
  alt[4].wmask = 0x00FFu; /* the coarse tone period keeping a whole byte */
  PR_CHECK(!pr_regmap_matches(p, alt));
  memcpy(alt, ivs_pv_literal, sizeof alt);
  alt[15].addr = 0x01F0u; /* an address out of order */
  PR_CHECK(!pr_regmap_matches(p, alt));
  return IVS_TEST_PASS;
}

static const ivs_pv *pr_find_reg_vector(const char *id)
{
  size_t i;
  for (i = 0u; i < IVS_PV_reg_COUNT; i++) {
    if (strcmp(ivs_pv_reg[i].id, id) == 0) {
      return &ivs_pv_reg[i];
    }
  }
  return NULL;
}

static int pr_judge_mask_row(void)
{
  const ivs_pv *v = pr_find_reg_vector("psg.reg.tone-high-mask");
  ivs_pv alt;
  ivs_pv_read reads[1];
  PR_CHECK(v != NULL && v->n_reads == 1u);
  PR_CHECK(pr_vector_eval(v, 0, 1)); /* the positive control: the real row passes */
  alt = *v;
  reads[0] = v->reads[0];
  reads[0].value = 0x00FFu; /* the expected value of a coarse period that keeps a whole byte */
  alt.reads = reads;
  PR_CHECK(!pr_vector_eval(&alt, 0, 1));
  reads[0].value = (uint16_t)(v->reads[0].value ^ 0x0001u);
  PR_CHECK(!pr_vector_eval(&alt, 0, 1));
  return IVS_TEST_PASS;
}

/* ---- reset ---- */

static int pr_reset_power_on_zero(void)
{
  ivs_ms_inst x;
  struct ivs_machine *im;
  uint32_t i;
  PR_CHECK(pr_powered(&x));
  im = pr_inst(&x);
  for (i = 0u; i < PR_REGS; i++) {
    if (im->mem.psg_reg[i] != 0u || ivs_memsys_psg_reg_internal(&im->mem, i) != 0u) {
      ivs_test_diag("register %u is not zero after a power-on reset", (unsigned)i);
      ivs_ms_free(&x);
      return IVS_TEST_FAIL;
    }
  }
  for (i = 0u; i < IVS_MEMSYS_PSG_PORTS; i++) {
    if (im->mem.port_pulldown[i] != 0u) {
      ivs_test_diag("port %u pull-down is not zero after a power-on reset", (unsigned)i);
      ivs_ms_free(&x);
      return IVS_TEST_FAIL;
    }
  }
  for (i = 0u; i < 14u; i++) {
    if (pr_read(&im->mem, (uint16_t)(PR_BASE + i)) != 0u) {
      ivs_test_diag("a bus read of %04X is not zero after a power-on reset", (unsigned)(PR_BASE + i));
      ivs_ms_free(&x);
      return IVS_TEST_FAIL;
    }
  }
  ivs_ms_free(&x);
  return IVS_TEST_PASS;
}

static int pr_reset_dirty_cleared(void)
{
  ivs_ms_inst a;
  ivs_ms_inst b;
  struct ivs_machine *im;
  uint32_t i;
  int rc = IVS_TEST_FAIL;
  PR_CHECK(pr_powered(&a));
  if (!pr_powered(&b)) {
    ivs_ms_free(&a);
    return IVS_TEST_FAIL;
  }
  im = pr_inst(&a);
  for (i = 0u; i < PR_REGS; i++) {
    pr_write(&im->mem, (uint16_t)(PR_BASE + i), 0xFFFFu);
  }
  ivs_memsys_set_port_pulldown_internal(&im->mem, 0u, 0xFFu);
  ivs_memsys_set_port_pulldown_internal(&im->mem, 1u, 0x5Au);
  if (memcmp(a.storage, b.storage, IVS_MACHINE_BYTES) == 0) {
    ivs_test_diag("dirtying the register file did not change the storage: the control proves nothing");
    goto done;
  }
  if (ivs_machine_reset(a.m, IVS_RESET_POWER_ON) != IVS_OK) {
    goto done;
  }
  if (memcmp(a.storage, b.storage, IVS_MACHINE_BYTES) != 0) {
    ivs_test_diag("a reset of a dirtied register file differs from a fresh instance");
    goto done;
  }
  rc = IVS_TEST_PASS;
done:
  ivs_ms_free(&a);
  ivs_ms_free(&b);
  return rc;
}

/* ---- bus: wait 0 and the wrapper ---- */

static int pr_bus_wait_zero(void)
{
  static const uint8_t kinds[] = {IVS_ACC_FETCH, IVS_ACC_READ,  IVS_ACC_WRITE,      IVS_ACC_DW,
                                  IVS_ACC_ADAR,  IVS_ACC_IAB,   IVS_ACC_INTAK,      IVS_ACC_STROBE_TCI,
                                  IVS_ACC_STROBE_PCIT};
  static const uint8_t iab_flags[] = {0u, IVS_ACC_F_RESET, IVS_ACC_F_INTERRUPT};
  ivs_memsys m;
  uint32_t a;
  size_t k;
  size_t f;
  PR_CHECK(ivs_memsys_init(&m, pr_profile()) == IVS_OK);
  for (a = 0u; a < 0x10000u; a++) {
    for (k = 0u; k < sizeof kinds / sizeof kinds[0]; k++) {
      size_t nf = kinds[k] == IVS_ACC_IAB ? sizeof iab_flags / sizeof iab_flags[0] : 1u;
      for (f = 0u; f < nf; f++) {
        ivs_bus_reply r = pr_bus(&m, kinds[k], kinds[k] == IVS_ACC_IAB ? iab_flags[f] : 0u, (uint16_t)a,
                                 (uint16_t)(0xA5A5u ^ a));
        if (r.wait_cpu_cycles != 0u) {
          ivs_test_diag("access kind %u at %04X returns wait %u, want 0", (unsigned)kinds[k], (unsigned)a,
                        (unsigned)r.wait_cpu_cycles);
          return IVS_TEST_FAIL;
        }
      }
    }
  }
  return IVS_TEST_PASS;
}

static int pr_same_reply(ivs_bus_reply w, ivs_bus_reply d)
{
  return w.data == d.data && w.wait_cpu_cycles == d.wait_cpu_cycles;
}

static int pr_bus_wrapper_forwards(void)
{
  /* Non-PSG accesses only: later plans add catch-up and logging to the wrapper for PSG writes. */
  static const uint16_t addrs[] = {0x0000u, 0x0080u, 0x0100u, 0x01EFu, 0x0200u, 0x0300u, 0x035Fu,
                                   0x1000u, 0x2000u, 0x3000u, 0x3800u, 0x5000u, 0x7800u, 0xFFFFu};
  static const uint8_t kinds[] = {IVS_ACC_FETCH, IVS_ACC_READ, IVS_ACC_ADAR, IVS_ACC_WRITE, IVS_ACC_DW,
                                  IVS_ACC_IAB,   IVS_ACC_INTAK, IVS_ACC_STROBE_TCI, IVS_ACC_STROBE_PCIT};
  ivs_ms_inst a;
  ivs_ms_inst b;
  ivs_machine_bus_ctx ctx;
  struct ivs_machine *ia;
  struct ivs_machine *ib;
  size_t i;
  size_t k;
  int rc = IVS_TEST_FAIL;
  PR_CHECK(pr_powered(&a));
  if (!pr_powered(&b)) {
    ivs_ms_free(&a);
    return IVS_TEST_FAIL;
  }
  ia = pr_inst(&a);
  ib = pr_inst(&b);
  memset(&ctx, 0, sizeof ctx);
  ctx.m = ia;
  ctx.unit_start_tick = ia->time_ticks;
  ctx.ticks_per_cpu_cycle = ivs_profile_get(ia->profile)->ticks_per_cpu_cycle;
  for (i = 0u; i < sizeof addrs / sizeof addrs[0]; i++) {
    for (k = 0u; k < sizeof kinds / sizeof kinds[0]; k++) {
      ivs_cp1610_access acc;
      ivs_bus_reply rw;
      ivs_bus_reply rd;
      memset(&acc, 0, sizeof acc);
      acc.kind = kinds[k];
      acc.flags = kinds[k] == IVS_ACC_IAB ? IVS_ACC_F_RESET : 0u;
      acc.addr = addrs[i];
      acc.data = (uint16_t)(0x1234u + 7u * (uint32_t)i);
      rw = ivs_machine_bus(&ctx, &acc);
      rd = ivs_memsys_bus(&ib->mem, &acc);
      if (!pr_same_reply(rw, rd)) {
        ivs_test_diag("kind %u at %04X: wrapper replies %04X/%u, memory system %04X/%u", (unsigned)kinds[k],
                      (unsigned)addrs[i], (unsigned)rw.data, (unsigned)rw.wait_cpu_cycles, (unsigned)rd.data,
                      (unsigned)rd.wait_cpu_cycles);
        goto done;
      }
    }
  }
  if (memcmp(a.storage, b.storage, IVS_MACHINE_BYTES) != 0) {
    ivs_test_diag("the wrapper left different storage than the direct memory-system accesses");
    goto done;
  }
  rc = IVS_TEST_PASS;
done:
  ivs_ms_free(&a);
  ivs_ms_free(&b);
  return rc;
}

/* ---- no mirror, purity, the guest tracer ---- */

static int pr_reg_no_mirror(void)
{
  ivs_memsys m;
  ivs_memsys before;
  uint32_t i;
  uint32_t a;
  uint32_t j;
  for (i = 0u; i < PR_REGS; i++) {
    PR_CHECK(ivs_memsys_init(&m, pr_profile()) == IVS_OK);
    before = m;
    pr_write(&m, (uint16_t)(PR_BASE + i), 0xFFFFu);
    /* the only change is the one word, whose value is the literal's write mask (all ones in) */
    PR_CHECK(m.psg_reg[i] == (uint16_t)(ivs_pv_literal[i].wmask & 0xFFFFu));
    before.psg_reg[i] = m.psg_reg[i];
    if (memcmp(&before, &m, sizeof m) != 0) {
      ivs_test_diag("a store to %04X changed more than its own register word", (unsigned)(PR_BASE + i));
      return IVS_TEST_FAIL;
    }
    for (j = 0u; j < PR_REGS; j++) {
      if (j != i) {
        PR_CHECK(m.psg_reg[j] == 0u);
      }
    }
  }
  /* no store outside the block reaches a register word (no alias of the block at any address) */
  PR_CHECK(ivs_memsys_init(&m, pr_profile()) == IVS_OK);
  for (a = 0u; a < 0x10000u; a++) {
    if (a >= PR_BASE && a < PR_BASE + PR_REGS) {
      continue;
    }
    pr_write(&m, (uint16_t)a, 0xFFFFu);
  }
  for (j = 0u; j < PR_REGS; j++) {
    PR_CHECK(m.psg_reg[j] == 0u);
  }
  PR_CHECK(m.port_pulldown[0] == 0u && m.port_pulldown[1] == 0u);
  return IVS_TEST_PASS;
}

static int pr_reg_reads_pure(void)
{
  ivs_ms_inst x;
  struct ivs_machine *im;
  uint8_t *snap;
  uint16_t seen[14];
  uint32_t i;
  uint32_t round;
  int rc = IVS_TEST_FAIL;
  PR_CHECK(pr_powered(&x));
  snap = malloc(IVS_MACHINE_BYTES);
  if (snap == NULL) {
    ivs_ms_free(&x);
    return IVS_TEST_FAIL;
  }
  im = pr_inst(&x);
  for (i = 0u; i < PR_REGS; i++) {
    pr_write(&im->mem, (uint16_t)(PR_BASE + i), (uint16_t)(0x00A0u + 3u * i));
  }
  memcpy(snap, x.storage, IVS_MACHINE_BYTES);
  for (round = 0u; round < 3u; round++) {
    /* $01F0..$01FD only: the port registers are not read here (plan 05-05) */
    if (ivs_machine_peek(x.m, (uint16_t)PR_BASE, 14u, seen) != IVS_OK) {
      goto done;
    }
    for (i = 0u; i < 14u; i++) {
      if (seen[i] != ivs_memsys_psg_reg_internal(&im->mem, i) || pr_read(&im->mem, (uint16_t)(PR_BASE + i)) != seen[i]) {
        ivs_test_diag("a read of %04X is not the register store", (unsigned)(PR_BASE + i));
        goto done;
      }
    }
    if (memcmp(snap, x.storage, IVS_MACHINE_BYTES) != 0) {
      ivs_test_diag("a read of the register file changed the instance storage");
      goto done;
    }
  }
  rc = IVS_TEST_PASS;
done:
  free(snap);
  ivs_ms_free(&x);
  return rc;
}

static int pr_reg_guest_write_read(void)
{
  ivs_ms_inst x;
  uint16_t out[2];
  int rc = IVS_TEST_FAIL;
  PR_CHECK(ivs_ms_new(&x, PR_POISON));
  if (!ivs_ms_load(x.m, IVS_KIND_EXEC, ivs_ms_exec(IVS_MS_PROG_C), IVS_MS_EXEC_BYTES) ||
      !ivs_ms_load(x.m, IVS_KIND_GROM, ivs_ms_grom(0u), IVS_MS_GROM_BYTES) ||
      ivs_machine_reset(x.m, IVS_RESET_POWER_ON) != IVS_OK || !ivs_ms_step(x.m, 8u, NULL)) {
    goto done;
  }
  /* the coarse tone period keeps four bits: $03FF written, $000F read back and stored to $0200 */
  if (ivs_machine_peek(x.m, 0x0200u, 1u, &out[0]) != IVS_OK || ivs_machine_peek(x.m, 0x01F4u, 1u, &out[1]) != IVS_OK) {
    goto done;
  }
  if (out[0] != 0x000Fu || out[1] != 0x000Fu) {
    ivs_test_diag("$0200 reads %04X and $01F4 reads %04X, want 000F both", (unsigned)out[0], (unsigned)out[1]);
    goto done;
  }
  rc = IVS_TEST_PASS;
done:
  ivs_ms_free(&x);
  return rc;
}

const ivs_test_case ivs_psg_reg_cases[] = {
    {"psg.regmap.matches-literal", pr_regmap_literal, NULL},
    {"psg.reset.power-on-zero", pr_reset_power_on_zero, NULL},
    {"psg.reset.dirty-file-cleared", pr_reset_dirty_cleared, NULL},
    {"psg.bus.wait-zero", pr_bus_wait_zero, NULL},
    {"psg.bus.wrapper-forwards", pr_bus_wrapper_forwards, NULL},
    {"psg.reg.no-mirror", pr_reg_no_mirror, NULL},
    {"psg.reg.reads-pure", pr_reg_reads_pure, NULL},
    {"psg.reg.guest-write-read", pr_reg_guest_write_read, NULL},
    {"psg.judge.mask-row-changed", pr_judge_mask_row, NULL},
    {"psg.judge.regmap-literal-changed", pr_judge_regmap, NULL},
};
const size_t ivs_psg_reg_count = sizeof ivs_psg_reg_cases / sizeof ivs_psg_reg_cases[0];

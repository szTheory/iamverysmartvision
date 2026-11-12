/* SPDX-License-Identifier: MIT */
/* Suite machine, shared stand-in content for the families reset, iso and sweep (plan 03-12):
 * instances in exact-size heap storage and generated system images and cartridges (D-24). No
 * rights-bearing byte: the EXEC stand-ins are short project programs written here as literal
 * decles, the rest of each EXEC image and every GROM and cartridge word is a generated pattern.
 *
 * The program decles are hand-assembled from the encodings of the project's own Phase 2 vector
 * data (the JSON files of data/cp1610/vectors, read from the GI CP-1600 Users Manual tables), never from an
 * assembler or another implementation:
 *   MVII #i, Rd   1010 111 ddd, then the immediate word     (immediate.json, cpu.MVII.r1-load)
 *   MVO  Rs, a    1001 000 sss, then the address word       (direct.json, MVO R1, $0200)
 *   INCR Rd       0000 001 ddd                              (register.json, INCR R1)
 *   DECR Rd       0000 010 ddd                              (register.json, DECR R3)
 *   B (back)      1000 10 0000, then the displacement d: the target is PC+2 plus the one's
 *                 complement of d, so PC+2 - d - 1                (branch.json, B $1001)
 * Every word is at most 0x3FF, so the 10-bit EXEC image holds it unmasked. */
#include <stddef.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>

#include "common/ivs_sha256.h"
#include "content/ivsc.h"
#include "ivs/ivs.h"
#include "ivs_test.h"
#include "machine/machine.h"
#include "machine_cases.h"

/* Program A at $1000: R1 = $155, then forever: [$0200] = R1, R1 += 1.
 *   $1000 MVII #$155, R1   $1002 MVO R1, $0200   $1004 INCR R1   $1005 B $1002 (PC+2 $1007, d 4) */
static const uint16_t ms_prog_a[] = {0x02B9u, 0x0155u, 0x0241u, 0x0200u, 0x0009u, 0x0220u, 0x0004u};

/* Program B at $1000: R2 = $02A, then forever: [$0201] = R2, R2 -= 1, R3 += 1.
 *   $1000 MVII #$02A, R2   $1002 MVO R2, $0201   $1004 DECR R2   $1005 INCR R3
 *   $1006 B $1002 (PC+2 $1008, d 5) */
static const uint16_t ms_prog_b[] = {0x02BAu, 0x002Au, 0x0242u, 0x0201u, 0x0012u, 0x000Bu, 0x0220u, 0x0005u};

/* Program C at $1000 (plan 05-04, the PSG register file seen from a guest): R0 = $03FF (the widest
 * immediate a 10-bit EXEC decle holds), [$01F4] = R0 (the channel A coarse tone period, four bits),
 * R1 = [$01F4], [$0200] = R1, then a branch to itself.
 *   $1000 MVII #$3FF, R0   $1002 MVO R0, $01F4   $1004 MVI $01F4, R1   $1006 MVO R1, $0200
 *   $1008 B $1008 (PC+2 $100A, d 1)
 * MVI direct is MVI $0200, R1 = 0281 0200 of direct.json (cpu.MVI.adar-figure), so MVI $01F4, R1 is
 * 0281 01F4; MVO R0, a is 0240 (MVO R1, a is 0241); the branch is as in program A. */
static const uint16_t ms_prog_c[] = {0x02B8u, 0x03FFu, 0x0240u, 0x01F4u, 0x0281u, 0x01F4u,
                                     0x0241u, 0x0200u, 0x0220u, 0x0001u};

/* Program PORTS at $1000 (plan 05-05, the hand controllers seen from a guest): read both PSG I/O
 * ports into RAM once, then a branch to itself.
 *   $1000 MVI $01FE, R1   $1002 MVO R1, $0200   $1004 MVI $01FF, R2   $1006 MVO R2, $0201
 *   $1008 B $1008 (PC+2 $100A, d 1)
 * MVI direct is 0280 + destination (MVI $01F4, R1 is 0281 01F4 above), MVO Rs, a is 0240 + source. */
static const uint16_t ms_prog_ports[] = {0x0281u, 0x01FEu, 0x0241u, 0x0200u, 0x0282u,
                                         0x01FFu, 0x0242u, 0x0201u, 0x0220u, 0x0001u};

/* Program POLL_CONST at $1000 (plan 05-05, API-03 fuzz content B): a poller of CONSTANT cycle count per
 * iteration. Both PSG ports are read and each byte is stored through R4, which post-increments, so RAM
 * from $0300 holds the log: word 2j is the j-th read of $01FE, word 2j + 1 the j-th read of $01FF. No
 * instruction depends on a value read, so the unit boundaries never depend on the input schedule.
 *   $1000 MVII #$0300, R4   $1002 MVI $01FE, R1   $1004 MVO@ R1, R4   $1005 MVI $01FF, R2
 *   $1007 MVO@ R2, R4       $1008 B $1002 (PC+2 $100A, d 7)
 * MVII is 02B8 + destination (02BC for R4); MVO@ Rs, Rm is 1001 mmm sss (MVO@ R1, R2 is 0251 in
 * data/cp1610/vectors/indirect.json), so MVO@ R1, R4 is 0261 and MVO@ R2, R4 is 0262; R4 is an
 * auto-incrementing pointer; MVI direct as in program PORTS; the branch as in program A. */
static const uint16_t ms_prog_poll_const[] = {0x02BCu, 0x0300u, 0x0281u, 0x01FEu, 0x0261u, 0x0282u,
                                              0x01FFu, 0x0262u, 0x0220u, 0x0007u};

/* Program POLL_BRANCH at $1000 (plan 05-05, API-03 fuzz content C): a data-dependent loop. Port 0 is
 * read and XOR-ed with $FF (zero when no control is closed); a nonzero value takes the PRESSED path (two
 * stores of the value to the log at R4, from $0300, then back to the read), zero takes the IDLE path (a
 * counter in R3, then back), so the instruction sequence, the unit lengths and every later boundary
 * depend on what the read returned.
 *   $1000 MVII #$0300, R4   $1002 MVI $01FE, R1   $1004 XORI #$00FF, R1   $1006 BEQ $100C (d 4)
 *   $1008 MVO@ R1, R4       $1009 MVO@ R1, R4     $100A B $1002 (PC+2 $100C, d 9)
 *   $100C INCR R3           $100D B $1002 (PC+2 $100F, d 12)
 * XORI #imm, Rd is 1111 111 ddd (03F9 for R1; ANDI #$8000, R1 is 03B9 in immediate.json, the
 * operation field of XOR is the next one); BEQ is 0204 (BZE in branch.json: 0204, displacement
 * word; forward target PC+2 + d), INCR as in program A, the backward branch as in program A. */
static const uint16_t ms_prog_poll_branch[] = {0x02BCu, 0x0300u, 0x0281u, 0x01FEu, 0x03F9u, 0x00FFu, 0x0204u,
                                               0x0004u, 0x0261u, 0x0261u, 0x0220u, 0x0009u, 0x000Bu, 0x0220u,
                                               0x000Cu};

/* Program TONE at $1000 (plan 05-06, the PSG tone seen from a guest): mixer R7 = $3E (tone A enabled, noise A
 * and channels B and C disabled), amplitude A = $0F (fixed level 15), tone A fine period = 1 and coarse = 0, then
 * a branch to itself. The writes land in the first 300 master ticks, after the first chip step has reloaded the
 * period 0 of the registers as 4096, so the period write takes effect at the next reload: the first edge is at
 * step 4096 and the edges follow every step.
 *   $1000 MVII #$3E, R0   $1002 MVO R0, $01F8   $1004 MVII #$0F, R0   $1006 MVO R0, $01FB
 *   $1008 MVII #$01, R0   $100A MVO R0, $01F0   $100C MVII #$00, R0   $100E MVO R0, $01F4
 *   $1010 B $1010 (PC+2 $1012, d 1)
 * MVII is 02B8 + destination and MVO Rs, a is 0240 + source (program C); the branch is as in program PORTS. */
static const uint16_t ms_prog_tone[] = {0x02B8u, 0x003Eu, 0x0240u, 0x01F8u, 0x02B8u, 0x000Fu, 0x0240u, 0x01FBu,
                                        0x02B8u, 0x0001u, 0x0240u, 0x01F0u, 0x02B8u, 0x0000u, 0x0240u, 0x01F4u,
                                        0x0220u, 0x0001u};

/* Program CONST at $1000 (plan 05-06, the write stamp seen from a guest): mixer R7 = $3F (tone and noise of every
 * channel disabled, so every channel is a constant 1), then amplitude A = $0F, then a branch to itself: channel A
 * is silent until the amplitude write lands and level 15 after it.
 *   $1000 MVII #$3F, R0   $1002 MVO R0, $01F8   $1004 MVII #$0F, R0   $1006 MVO R0, $01FB
 *   $1008 B $1008 (PC+2 $100A, d 1)
 * Encodings as in program TONE. */
static const uint16_t ms_prog_const[] = {0x02B8u, 0x003Fu, 0x0240u, 0x01F8u, 0x02B8u,
                                         0x000Fu, 0x0240u, 0x01FBu, 0x0220u, 0x0001u};

/* Program ENV at $1000 (plan 05-07, the envelope started by a guest write): mixer R7 = $3F (tone and noise of
 * every channel disabled, a constant 1), amplitude A = $30 (select 3: the envelope level as is), envelope period
 * fine = 1 (coarse stays 0), then the shape register = $08 (decay, repeating), then a branch to itself. Until the
 * shape write the generator is held at level 0 (psg.reset), so channel A is silent; the write restarts the
 * generator: the first sample after it is level 15, then the levels fall every second step.
 *   $1000 MVII #$3F, R0   $1002 MVO R0, $01F8   $1004 MVII #$30, R0   $1006 MVO R0, $01FB
 *   $1008 MVII #$01, R0   $100A MVO R0, $01F3   $100C MVII #$08, R0   $100E MVO R0, $01FA
 *   $1010 B $1010 (PC+2 $1012, d 1)
 * Encodings as in program TONE. */
static const uint16_t ms_prog_env[] = {0x02B8u, 0x003Fu, 0x0240u, 0x01F8u, 0x02B8u, 0x0030u, 0x0240u, 0x01FBu,
                                       0x02B8u, 0x0001u, 0x0240u, 0x01F3u, 0x02B8u, 0x0008u, 0x0240u, 0x01FAu,
                                       0x0220u, 0x0001u};

typedef struct ms_prog {
  const uint16_t *words;
  size_t n;
} ms_prog;

static const ms_prog ms_progs[IVS_MS_PROGS] = {
    {ms_prog_a, sizeof ms_prog_a / sizeof ms_prog_a[0]},
    {ms_prog_b, sizeof ms_prog_b / sizeof ms_prog_b[0]},
    {ms_prog_c, sizeof ms_prog_c / sizeof ms_prog_c[0]},
    {ms_prog_ports, sizeof ms_prog_ports / sizeof ms_prog_ports[0]},
    {ms_prog_poll_const, sizeof ms_prog_poll_const / sizeof ms_prog_poll_const[0]},
    {ms_prog_poll_branch, sizeof ms_prog_poll_branch / sizeof ms_prog_poll_branch[0]},
    {ms_prog_tone, sizeof ms_prog_tone / sizeof ms_prog_tone[0]},
    {ms_prog_const, sizeof ms_prog_const / sizeof ms_prog_const[0]},
    {ms_prog_env, sizeof ms_prog_env / sizeof ms_prog_env[0]},
};

uint8_t *ivs_ms_exec(uint32_t program)
{
  const ms_prog *pg = &ms_progs[program < IVS_MS_PROGS ? program : IVS_MS_PROG_A];
  uint8_t *b = malloc(IVS_MS_EXEC_BYTES);
  uint32_t i;
  if (b == NULL) {
    return NULL;
  }
  for (i = 0u; i < IVS_MS_EXEC_BYTES / 2u; i++) {
    uint16_t w = i < pg->n ? pg->words[i] : (uint16_t)((37u * i + 11u + 5u * program) & 0x3FFu);
    b[2u * i] = (uint8_t)(w >> 8);
    b[2u * i + 1u] = (uint8_t)(w & 0xFFu);
  }
  return b;
}

uint8_t *ivs_ms_grom(uint32_t seed)
{
  uint8_t *b = malloc(IVS_MS_GROM_BYTES);
  uint32_t i;
  if (b == NULL) {
    return NULL;
  }
  for (i = 0u; i < IVS_MS_GROM_BYTES; i++) {
    b[i] = (uint8_t)((7u * i + 3u + 29u * seed) & 0xFFu);
  }
  return b;
}

static void ms_put16(uint8_t *p, uint32_t v)
{
  p[0] = (uint8_t)((v >> 8) & 0xFFu);
  p[1] = (uint8_t)(v & 0xFFu);
}

uint8_t *ivs_ms_cart(uint32_t addr, uint32_t len_words, uint32_t seed, size_t *out_len)
{
  static const uint8_t magic[8] = {0x89u, 0x49u, 0x56u, 0x53u, 0x43u, 0x0Du, 0x0Au, 0x1Au};
  size_t total = IVS_IVSC_PREFIX_BYTES + IVS_IVSC_ENTRY_BYTES + 2u * (size_t)len_words + IVS_IVSC_TRAILER_BYTES;
  size_t at = IVS_IVSC_PREFIX_BYTES;
  uint32_t k;
  uint8_t *b = malloc(total);
  if (b == NULL) {
    return NULL;
  }
  memset(b, 0, total);
  memcpy(b, magic, sizeof magic);
  ms_put16(b + 8, 1u);                             /* version */
  ms_put16(b + 16, (uint32_t)(total >> 16));       /* total_bytes */
  ms_put16(b + 18, (uint32_t)(total & 0xFFFFu));
  ms_put16(b + 20, 1u);                            /* segment_count */
  ms_put16(b + at, addr);                          /* entry: addr, len_words, width 10 */
  ms_put16(b + at + 2u, len_words);
  b[at + 4u] = 10u;
  at += IVS_IVSC_ENTRY_BYTES;
  for (k = 0u; k < len_words; k++) {
    ms_put16(b + at, (13u * k + 5u + 41u * seed) & 0x3FFu);
    at += 2u;
  }
  ivs_sha256_digest(b, total - IVS_IVSC_TRAILER_BYTES, b + total - IVS_IVSC_TRAILER_BYTES);
  *out_len = total;
  return b;
}

int ivs_ms_new(ivs_ms_inst *x, uint8_t poison)
{
  ivs_create_options o;
  memset(&o, 0, sizeof o);
  o.struct_size = (uint32_t)sizeof o;
  o.profile_id = IVS_MS_PROFILE;
  x->m = NULL;
  x->storage = aligned_alloc(IVS_MACHINE_ALIGN, IVS_MACHINE_BYTES);
  if (x->storage == NULL) {
    return 0;
  }
  memset(x->storage, poison, IVS_MACHINE_BYTES);
  if (ivs_machine_create(&o, x->storage, IVS_MACHINE_BYTES, &x->m) != IVS_OK) {
    ivs_test_diag("create refused");
    free(x->storage);
    x->storage = NULL;
    return 0;
  }
  return 1;
}

void ivs_ms_free(ivs_ms_inst *x)
{
  free(x->storage);
  x->storage = NULL;
  x->m = NULL;
}

int ivs_ms_load(ivs_machine *m, uint32_t kind, uint8_t *bytes, size_t len)
{
  ivs_status st;
  if (bytes == NULL) {
    return 0;
  }
  st = ivs_machine_load(m, kind, bytes, len, NULL, NULL);
  free(bytes);
  if (st != IVS_OK) {
    ivs_test_diag("load of kind %u refused (%s)", (unsigned)kind, ivs_status_name(st));
    return 0;
  }
  return 1;
}

int ivs_ms_content(ivs_machine *m, uint32_t content)
{
  size_t n = 0u;
  uint32_t seed = content == IVS_MS_CONTENT_B ? 1u : 0u;
  uint32_t prog = content == IVS_MS_CONTENT_B ? IVS_MS_PROG_B : IVS_MS_PROG_A;
  uint8_t *cart;
  if (!ivs_ms_load(m, IVS_KIND_EXEC, ivs_ms_exec(prog), IVS_MS_EXEC_BYTES) ||
      !ivs_ms_load(m, IVS_KIND_GROM, ivs_ms_grom(seed), IVS_MS_GROM_BYTES)) {
    return 0;
  }
  cart = ivs_ms_cart(content == IVS_MS_CONTENT_B ? 0x6000u : 0x5000u, content == IVS_MS_CONTENT_B ? 24u : 16u, seed,
                     &n);
  return ivs_ms_load(m, IVS_KIND_CARTRIDGE, cart, n);
}

int ivs_ms_powered(ivs_ms_inst *x, uint32_t content, uint8_t poison)
{
  ivs_status st;
  if (!ivs_ms_new(x, poison)) {
    return 0;
  }
  if (!ivs_ms_content(x->m, content)) {
    ivs_ms_free(x);
    return 0;
  }
  st = ivs_machine_reset(x->m, IVS_RESET_POWER_ON);
  if (st != IVS_OK) {
    ivs_test_diag("reset refused (%s)", ivs_status_name(st));
    ivs_ms_free(x);
    return 0;
  }
  return 1;
}

int ivs_ms_powered_prog(ivs_ms_inst *x, uint32_t program, uint8_t poison)
{
  size_t n = 0u;
  uint8_t *cart;
  ivs_status st;
  if (!ivs_ms_new(x, poison)) {
    return 0;
  }
  /* the cartridge is built in its own statement: its length n is an out-parameter of ivs_ms_cart, and an
   * argument list that sets n and reads it has no defined order (gcc reads it first) */
  cart = ivs_ms_cart(0x5000u, 16u, 0u, &n);
  if (!ivs_ms_load(x->m, IVS_KIND_EXEC, ivs_ms_exec(program), IVS_MS_EXEC_BYTES) ||
      !ivs_ms_load(x->m, IVS_KIND_GROM, ivs_ms_grom(0u), IVS_MS_GROM_BYTES) ||
      !ivs_ms_load(x->m, IVS_KIND_CARTRIDGE, cart, n)) {
    ivs_ms_free(x);
    return 0;
  }
  st = ivs_machine_reset(x->m, IVS_RESET_POWER_ON);
  if (st != IVS_OK) {
    ivs_test_diag("reset refused (%s)", ivs_status_name(st));
    ivs_ms_free(x);
    return 0;
  }
  return 1;
}

int ivs_ms_step(ivs_machine *m, uint32_t units, uint64_t *cycles)
{
  uint32_t i;
  for (i = 0u; i < units; i++) {
    ivs_cp1610_result r;
    ivs_status st = ivs_machine_step_unit_internal(m, &r);
    if (st != IVS_OK) {
      ivs_test_diag("step %u refused (%s)", (unsigned)i, ivs_status_name(st));
      return 0;
    }
    if (cycles != NULL) {
      *cycles += r.elapsed_cpu_cycles;
    }
  }
  return 1;
}

int ivs_ms_digests(const ivs_machine *m, uint8_t composite[32], uint8_t cpu[32])
{
  return ivs_machine_state_digest(m, IVS_DIGEST_COMPOSITE, composite) == IVS_OK &&
         ivs_machine_state_digest(m, IVS_DIGEST_CPU, cpu) == IVS_OK;
}

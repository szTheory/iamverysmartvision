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

uint8_t *ivs_ms_exec(uint32_t program)
{
  const uint16_t *prog = program == IVS_MS_PROG_C ? ms_prog_c : (program == IVS_MS_PROG_B ? ms_prog_b : ms_prog_a);
  size_t n = program == IVS_MS_PROG_C ? sizeof ms_prog_c / sizeof ms_prog_c[0]
             : (program == IVS_MS_PROG_B ? sizeof ms_prog_b / sizeof ms_prog_b[0] : sizeof ms_prog_a / sizeof ms_prog_a[0]);
  uint8_t *b = malloc(IVS_MS_EXEC_BYTES);
  uint32_t i;
  if (b == NULL) {
    return NULL;
  }
  for (i = 0u; i < IVS_MS_EXEC_BYTES / 2u; i++) {
    uint16_t w = i < n ? prog[i] : (uint16_t)((37u * i + 11u + 5u * program) & 0x3FFu);
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

/* SPDX-License-Identifier: MIT */
/* Suite machine, family digest (D-35, plan 03-09): the canonical state digest.
 * - Literal streams: the INSTANCE and CPU preimages of a fresh instance, and the CPU preimage of a
 *   synthetic CPU state, equal byte sequences written out by hand (big-endian); their SHA-256
 *   values were computed at authoring time with `cmake -E sha256sum` over those bytes, so the
 *   code under test never produced them.
 * - Composite framing: "IVSSTATE" || u16 version || u8 id || digest for domains 1..4, rebuilt here
 *   (version 2 since plan 05-03; the version 1 literals are quoted from
 *   data/expected/digest-v1-literals.ids where they were replaced).
 * - Coverage: every byte of struct ivs_machine is in exactly one field-table row, and every row
 *   names one of the eight domains (1..4 and the observation domains 16..19, plans 04-05, 05-03).
 * - Sensitivity: flipping the first and the last byte of each serialized row changes that row's
 *   domain digest and leaves the other domains' preimages identical; the composite changes for a
 *   row of domains 1..4 and stays the same for a row of 16..19 (D-10, A-03); flipping a declared
 *   padding byte changes nothing.
 * - PSG storage (plan 05-03, D-04, D-16, D-18, A-13): the PSG registers and pull-downs are MEM
 *   rows, the PSG device state is INSTANCE rows (both inside composite version 2), the audio and
 *   write-log rows are observation rows outside it, and no PSG row ever moves the CPU or IMAGES
 *   digest. The INSTANCE stream with the PSG rows and the MEM tail are framed by hand here.
 * - Observation domains (plan 04-05): a fresh and a reset instance agree on VIDEO, AUDIO and
 *   INPUT; a byte of a 16..18 row moves only its own domain; reset returns every observation byte
 *   to its create value, zero except the input chain's h0 (plan 04-11)
 *   (machine.reset.clears-observation lives here with the other observation cases).
 * - Refusals: bad domain and NULL out (BAD_ARG), NULL or destroyed handle (BAD_HANDLE), with no
 *   write to the instance or out.
 * - Stepped literals (plan 04-05): the composite and domain 1..4 digests after content A, reset and
 *   100 units, recorded from the Phase 3 code before the observation rows were added.
 * The fresh composite and stepped literals are labelled repeatability only: they pin the value,
 * not its truth. */
#include <stdint.h>
#include <stdlib.h>
#include <string.h>

#include "common/ivs_sha256.h"
#include "ivs/ivs_machine.h"
#include "ivs_test.h"
#include "machine/machine.h"
#include "machine_cases.h"

/* Fails the case with a diagnostic naming the line and the failed expression. */
#define IVS_CHECK(cond)                                                                                \
  do {                                                                                                 \
    if (!(cond)) {                                                                                     \
      ivs_test_diag("check failed at line %d: %s", __LINE__, #cond);                                   \
      return IVS_TEST_FAIL;                                                                            \
    }                                                                                                  \
  } while (0)

#define MD_PROFILE "intv-2609-ntsc-family-p1"
#define MD_PRE_MAX 120000u /* larger than every domain preimage (MEM is about 113 KB) */

/* The domains with rows (plans 04-05, 05-03): in_composite marks 1..4, the composite's domains;
 * the observation domains 16..19 have their own digests only (D-10, D-18). Arrays below are
 * indexed by the domain id, so MD_DOMAIN_IDS covers 0 (the composite) to 19. */
typedef struct md_domain {
  uint32_t id;
  int in_composite;
} md_domain;

static const md_domain md_domains[] = {
    {IVS_DIGEST_CPU, 1},   {IVS_DIGEST_MEM, 1},   {IVS_DIGEST_IMAGES, 1}, {IVS_DIGEST_INSTANCE, 1},
    {IVS_DIGEST_VIDEO, 0}, {IVS_DIGEST_AUDIO, 0}, {IVS_DIGEST_INPUT, 0}, {IVS_DIGEST_PSGLOG, 0},
};
#define MD_DOMAIN_COUNT (sizeof md_domains / sizeof md_domains[0])
#define MD_DOMAIN_IDS 20u

/* 1 for a listed domain; *in_comp (when not NULL) gets its composite flag. */
static int md_domain_find(uint32_t d, int *in_comp)
{
  size_t i;
  for (i = 0; i < MD_DOMAIN_COUNT; i++) {
    if (md_domains[i].id == d) {
      if (in_comp != NULL) {
        *in_comp = md_domains[i].in_composite;
      }
      return 1;
    }
  }
  return 0;
}

/* A fresh instance in exact-size 16-aligned heap storage (NULL on failure). */
static struct ivs_machine *md_fresh(void)
{
  ivs_create_options o;
  ivs_machine *m = NULL;
  uint8_t *st = aligned_alloc(16u, IVS_MACHINE_BYTES);
  if (st == NULL) {
    return NULL;
  }
  memset(&o, 0, sizeof o);
  o.struct_size = (uint32_t)sizeof o;
  o.profile_id = MD_PROFILE;
  if (ivs_machine_create(&o, st, IVS_MACHINE_BYTES, &m) != IVS_OK) {
    free(st);
    return NULL;
  }
  return m;
}

/* ---- literal streams ---- */

/* INSTANCE preimage of a fresh instance: tag, domain 4, version 1, the profile id (u32 length 24
 * and its ASCII), phase OFF (u32 0), machine time 0 (u64), then (plan 05-03, D-16, table order)
 * the PSG rows: next_sample (u64), tone_counter (u32 count 3 and three u32), tone_out (the same),
 * then the u32 rows noise_counter, noise_lfsr, env_counter, env_level, env_dir, env_held and
 * env_cycles. md_inst_stream is the head up to machine time; the PSG tail follows. */
static const uint8_t md_inst_stream[47] = {
    0x49, 0x56, 0x53, 0x44, 0x04, 0x00, 0x01, 0x00, 0x00, 0x00, 0x18, 'i',  'n',  't',  'v',  '-',
    '2',  '6',  '0',  '9',  '-',  'n',  't',  's',  'c',  '-',  'f',  'a',  'm',  'i',  'l',  'y',
    '-',  'p',  '1',  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
};
/* The PSG tail of a fresh instance (68 bytes) and its SHA-256 over head plus tail (computed at
 * authoring time with shasum over the hand-written bytes; before plan 05-03 the INSTANCE digest of
 * a fresh instance was 203dc162...c269 over the 47-byte head alone). */
static const uint8_t md_inst_psg_fresh[68] = {
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x03, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x03, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00,
};
static const uint8_t md_inst_sha[32] = {
    0xa3, 0xd5, 0xe1, 0x3f, 0x89, 0x91, 0x48, 0xf2, 0x52, 0xaa, 0x58, 0xe2, 0x1b, 0x76, 0xda, 0x2f,
    0xe7, 0x6f, 0xb7, 0x62, 0xa0, 0xc1, 0x34, 0x42, 0x70, 0xd3, 0x5e, 0xd7, 0xa9, 0x90, 0xe2, 0x30,
};

/* The PSG tail of a synthetic state: next_sample 0102030405060708, tone counters 11121314,
 * 21222324, 31323334, tone outputs 41424344, 51525354, 61626364, noise counter 71727374, noise
 * register 00012345, envelope counter 81828384, level 0000000F, direction 91929394, held A1A2A3A4,
 * cycles B1B2B3B4; reserved words EEEEEEEE (not serialized). Every word differs, so a field order
 * or width slip shows. */
static const uint8_t md_inst_psg_synth[68] = {
    0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07, 0x08, 0x00, 0x00, 0x00, 0x03, 0x11, 0x12, 0x13, 0x14,
    0x21, 0x22, 0x23, 0x24, 0x31, 0x32, 0x33, 0x34, 0x00, 0x00, 0x00, 0x03, 0x41, 0x42, 0x43, 0x44,
    0x51, 0x52, 0x53, 0x54, 0x61, 0x62, 0x63, 0x64, 0x71, 0x72, 0x73, 0x74, 0x00, 0x01, 0x23, 0x45,
    0x81, 0x82, 0x83, 0x84, 0x00, 0x00, 0x00, 0x0f, 0x91, 0x92, 0x93, 0x94, 0xa1, 0xa2, 0xa3, 0xa4,
    0xb1, 0xb2, 0xb3, 0xb4,
};
static const uint8_t md_inst_synth_sha[32] = {
    0xda, 0x70, 0x74, 0xfb, 0x10, 0x54, 0xcf, 0x51, 0xe3, 0xaf, 0x81, 0x61, 0xb2, 0x9b, 0x0d, 0x66,
    0x25, 0x89, 0xcb, 0x33, 0x8e, 0x41, 0xb6, 0x05, 0x26, 0xe3, 0x14, 0xaf, 0x81, 0xf8, 0x06, 0x7a,
};

/* PSGLOG preimage of a fresh instance (domain 19, plan 05-03): tag, domain 19, version 1, the
 * chain (u32 count 32 and 32 zero bytes), the record count (u64 0). */
static const uint8_t md_psglog_stream[51] = {
    0x49, 0x56, 0x53, 0x44, 0x13, 0x00, 0x01, 0x00, 0x00, 0x00, 0x20, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00,
};
static const uint8_t md_psglog_sha[32] = {
    0x12, 0xdc, 0xb6, 0x62, 0x08, 0x66, 0x7a, 0x8b, 0x65, 0x9e, 0xfd, 0x3a, 0xc5, 0xd6, 0x98, 0xad,
    0xcc, 0xf0, 0x70, 0x27, 0x67, 0xec, 0x02, 0xad, 0x84, 0x04, 0x85, 0x50, 0x73, 0x4a, 0x44, 0xfa,
};

/* CPU preimage of a fresh instance: tag, domain 1, version 1, u32 count 8 and eight zero
 * registers, then the eight zero status and flag bytes (the reserved bytes are not serialized). */
static const uint8_t md_cpu_stream[35] = {
    0x49, 0x56, 0x53, 0x44, 0x01, 0x00, 0x01, 0x00, 0x00, 0x00, 0x08, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00,
};
static const uint8_t md_cpu_sha[32] = {
    0x22, 0xeb, 0xa1, 0x26, 0x6d, 0xd6, 0x2a, 0x7e, 0x81, 0xcf, 0x15, 0x19, 0x62, 0x05, 0xb3, 0xa1,
    0x99, 0x9d, 0x85, 0x42, 0x2e, 0xb0, 0x5e, 0x69, 0xe5, 0xcf, 0xce, 0xdd, 0x91, 0x6f, 0xdf, 0xcd,
};

/* CPU preimage of a synthetic state: R0..R7 = A000, A111, ..., A777, status C3, then 11, 22, 33,
 * 44, 55, 66, 77 for intff, dbd, prev_interruptible, run_state, halt_pending, fault_reason, busak;
 * reserved bytes EE (not serialized). Every register byte pair differs, so a byte-order slip shows. */
static const uint8_t md_cpus_stream[35] = {
    0x49, 0x56, 0x53, 0x44, 0x01, 0x00, 0x01, 0x00, 0x00, 0x00, 0x08, 0xa0, 0x00, 0xa1, 0x11, 0xa2,
    0x22, 0xa3, 0x33, 0xa4, 0x44, 0xa5, 0x55, 0xa6, 0x66, 0xa7, 0x77, 0xc3, 0x11, 0x22, 0x33, 0x44,
    0x55, 0x66, 0x77,
};
static const uint8_t md_cpus_sha[32] = {
    0x9c, 0x07, 0x7f, 0x62, 0x68, 0x04, 0x96, 0xd2, 0x7c, 0x88, 0x37, 0x9d, 0x9a, 0x9f, 0x5d, 0x89,
    0xec, 0xc3, 0x50, 0x0f, 0xfb, 0x19, 0x47, 0x09, 0xe7, 0xfd, 0x58, 0x3a, 0xff, 0xdd, 0xe4, 0xb1,
};

/* Preimage of domain equals stream, and the public domain digest equals sha. */
static int md_stream_check(const struct ivs_machine *m, uint32_t domain, const uint8_t *stream, size_t n,
                           const uint8_t sha[32])
{
  uint8_t buf[256];
  uint8_t got[32];
  size_t len = ivs_machine_digest_preimage(m, domain, buf, sizeof buf);
  IVS_CHECK(len == n);
  IVS_CHECK(memcmp(buf, stream, n) == 0);
  IVS_CHECK(ivs_machine_state_digest(m, domain, got) == IVS_OK);
  IVS_CHECK(memcmp(got, sha, 32u) == 0);
  return IVS_TEST_PASS;
}

/* head || tail into out (at most 128 bytes); returns the length. */
static size_t md_inst_join(uint8_t out[128], const uint8_t tail[68])
{
  memcpy(out, md_inst_stream, sizeof md_inst_stream);
  memcpy(out + sizeof md_inst_stream, tail, 68u);
  return sizeof md_inst_stream + 68u;
}

static int md_stream_instance(void)
{
  int rc;
  uint8_t stream[128];
  size_t n = md_inst_join(stream, md_inst_psg_fresh);
  struct ivs_machine *m = md_fresh();
  IVS_CHECK(m != NULL);
  rc = md_stream_check(m, IVS_DIGEST_INSTANCE, stream, n, md_inst_sha);
  free(m);
  return rc;
}

/* The same stream with every PSG state member set to a distinct value: the row order, the count
 * prefix of the two three-element rows, the widths and the big-endian bytes of the PSG rows. */
static int md_stream_instance_psg(void)
{
  int rc;
  uint8_t stream[128];
  size_t n = md_inst_join(stream, md_inst_psg_synth);
  struct ivs_machine *m = md_fresh();
  IVS_CHECK(m != NULL);
  m->psg.next_sample = 0x0102030405060708ull;
  m->psg.tone_counter[0] = 0x11121314u;
  m->psg.tone_counter[1] = 0x21222324u;
  m->psg.tone_counter[2] = 0x31323334u;
  m->psg.tone_out[0] = 0x41424344u;
  m->psg.tone_out[1] = 0x51525354u;
  m->psg.tone_out[2] = 0x61626364u;
  m->psg.noise_counter = 0x71727374u;
  m->psg.noise_lfsr = 0x00012345u;
  m->psg.env_counter = 0x81828384u;
  m->psg.env_level = 0x0000000Fu;
  m->psg.env_dir = 0x91929394u;
  m->psg.env_held = 0xA1A2A3A4u;
  m->psg.env_cycles = 0xB1B2B3B4u;
  memset(m->psg.reserved, 0xEE, sizeof m->psg.reserved);
  rc = md_stream_check(m, IVS_DIGEST_INSTANCE, stream, n, md_inst_synth_sha);
  free(m);
  return rc;
}

/* Domain 19 of a fresh instance: the literal stream, its hash, and two fresh instances agree. */
static int md_domain_19_fresh(void)
{
  int rc;
  uint8_t a[32];
  uint8_t b[32];
  struct ivs_machine *m = md_fresh();
  struct ivs_machine *n = md_fresh();
  IVS_CHECK(m != NULL);
  if (n == NULL) {
    free(m);
    return IVS_TEST_FAIL;
  }
  rc = md_stream_check(m, IVS_DIGEST_PSGLOG, md_psglog_stream, sizeof md_psglog_stream, md_psglog_sha);
  if (rc == IVS_TEST_PASS &&
      (ivs_machine_state_digest(m, IVS_DIGEST_PSGLOG, a) != IVS_OK ||
       ivs_machine_state_digest(n, IVS_DIGEST_PSGLOG, b) != IVS_OK || memcmp(a, b, 32u) != 0)) {
    rc = IVS_TEST_FAIL;
  }
  free(m);
  free(n);
  return rc;
}

static int md_stream_cpu(void)
{
  int rc;
  struct ivs_machine *m = md_fresh();
  IVS_CHECK(m != NULL);
  rc = md_stream_check(m, IVS_DIGEST_CPU, md_cpu_stream, sizeof md_cpu_stream, md_cpu_sha);
  free(m);
  return rc;
}

static int md_stream_cpu_synthetic(void)
{
  int rc;
  uint32_t i;
  struct ivs_machine *m = md_fresh();
  IVS_CHECK(m != NULL);
  for (i = 0u; i < 8u; i++) {
    m->cpu.r[i] = (uint16_t)(0xA000u + 0x0111u * i);
  }
  m->cpu.status = 0xC3u;
  m->cpu.intff = 0x11u;
  m->cpu.dbd = 0x22u;
  m->cpu.prev_interruptible = 0x33u;
  m->cpu.run_state = 0x44u;
  m->cpu.halt_pending = 0x55u;
  m->cpu.fault_reason = 0x66u;
  m->cpu.busak = 0x77u;
  memset(m->cpu.reserved, 0xEE, sizeof m->cpu.reserved);
  rc = md_stream_check(m, IVS_DIGEST_CPU, md_cpus_stream, sizeof md_cpus_stream, md_cpus_sha);
  free(m);
  return rc;
}

/* The SHA-256 of each domain preimage (hashed here) equals the public domain digest, so the
 * preimage seam and the hash path serialize the same bytes. */
static int md_preimage_hash(void)
{
  size_t di;
  int ok = 1;
  struct ivs_machine *m = md_fresh();
  uint8_t *buf = malloc(MD_PRE_MAX);
  if (m == NULL || buf == NULL) {
    free(m);
    free(buf);
    return IVS_TEST_FAIL;
  }
  for (di = 0; ok && di < MD_DOMAIN_COUNT; di++) {
    uint32_t d = md_domains[di].id;
    uint8_t a[32];
    uint8_t b[32];
    size_t len = ivs_machine_digest_preimage(m, d, buf, MD_PRE_MAX);
    ok = len > 7u && len <= MD_PRE_MAX && ivs_machine_state_digest(m, d, a) == IVS_OK;
    if (ok) {
      ivs_sha256_digest(buf, len, b);
      ok = memcmp(a, b, 32u) == 0;
    }
    if (!ok) {
      ivs_test_diag("domain %u", (unsigned)d);
    }
  }
  free(buf);
  free(m);
  return ok ? IVS_TEST_PASS : IVS_TEST_FAIL;
}

/* ---- composite ---- */

/* Rebuild the composite from the four domain digests. */
static int md_composite_framing(void)
{
  static const uint8_t tag[8] = {'I', 'V', 'S', 'S', 'T', 'A', 'T', 'E'};
  uint8_t pre[8 + 2 + 4 * 33];
  uint8_t want[32];
  uint8_t got[32];
  uint32_t d;
  int ok = 1;
  struct ivs_machine *m = md_fresh();
  IVS_CHECK(m != NULL);
  memcpy(pre, tag, 8u);
  pre[8] = 0x00;
  pre[9] = 0x02; /* composite version 2 (plan 05-03, D-18) */
  for (d = 1u; ok && d <= 4u; d++) {
    pre[10u + 33u * (d - 1u)] = (uint8_t)d;
    ok = ivs_machine_state_digest(m, d, &pre[11u + 33u * (d - 1u)]) == IVS_OK;
  }
  ok = ok && ivs_machine_state_digest(m, IVS_DIGEST_COMPOSITE, got) == IVS_OK;
  free(m);
  IVS_CHECK(ok);
  ivs_sha256_digest(pre, sizeof pre, want);
  IVS_CHECK(memcmp(got, want, 32u) == 0);
  return IVS_TEST_PASS;
}

/* Composite version 2 (plan 05-03, D-18), rebuilt from independent pieces: the version bytes are
 * the literal 00 02; domains 1 and 4 are hashed here from the literal preimage streams (the CPU
 * head above and the INSTANCE head plus PSG tail), domains 2 and 3 from the preimage seam hashed
 * here with the repo's SHA-256 (the core's digest path is not used for any domain). The framing
 * is IVSSTATE || 00 02 || 01 || d1 || 02 || d2 || 03 || d3 || 04 || d4. */
static int md_composite_v2_framing(void)
{
  static const uint8_t head[10] = {'I', 'V', 'S', 'S', 'T', 'A', 'T', 'E', 0x00, 0x02};
  uint8_t pre[10 + 4 * 33];
  uint8_t stream[128];
  uint8_t want[32];
  uint8_t got[32];
  uint8_t *big = malloc(MD_PRE_MAX);
  size_t n;
  size_t len;
  int ok = 1;
  struct ivs_machine *m = md_fresh();
  IVS_CHECK(m != NULL);
  if (big == NULL) {
    free(m);
    return IVS_TEST_FAIL;
  }
  memcpy(pre, head, sizeof head);
  pre[10] = 1u;
  ivs_sha256_digest(md_cpu_stream, sizeof md_cpu_stream, &pre[11]);
  len = ivs_machine_digest_preimage(m, IVS_DIGEST_MEM, big, MD_PRE_MAX);
  ok = len > 7u && len <= MD_PRE_MAX;
  pre[43] = 2u;
  if (ok) {
    ivs_sha256_digest(big, len, &pre[44]);
    len = ivs_machine_digest_preimage(m, IVS_DIGEST_IMAGES, big, MD_PRE_MAX);
    ok = len > 7u && len <= MD_PRE_MAX;
  }
  pre[76] = 3u;
  if (ok) {
    ivs_sha256_digest(big, len, &pre[77]);
  }
  n = md_inst_join(stream, md_inst_psg_fresh);
  pre[109] = 4u;
  ivs_sha256_digest(stream, n, &pre[110]);
  ok = ok && ivs_machine_state_digest(m, IVS_DIGEST_COMPOSITE, got) == IVS_OK;
  free(big);
  free(m);
  IVS_CHECK(ok);
  ivs_sha256_digest(pre, sizeof pre, want);
  IVS_CHECK(memcmp(got, want, 32u) == 0);
  return IVS_TEST_PASS;
}

/* Repeatability only (not an accuracy claim): the composite of a fresh instance of the profile,
 * composite version 2 (plan 05-03: the PSG rows are in domains 2 and 4 and the framing version is
 * 2). Recorded from the new tree after machine.digest.composite-v2-framing showed the composite
 * equals the framing rebuilt by hand from the literal preimage streams. The Phase 4 value
 * (composite version 1) was ac794821fa80a0230db1d21aaf86a7ecdc695ca5515aa42ceb8dc3e85ea5d137
 * (data/expected/digest-v1-literals.ids, test.fresh-composite). */
static const uint8_t md_fresh_composite[32] = {
    0x10, 0x7b, 0xd1, 0xcb, 0x61, 0xbc, 0x92, 0xcc, 0x3c, 0xec, 0x4e, 0x3d, 0x34, 0x06, 0xeb, 0xe7,
    0xa1, 0xa0, 0x82, 0x9c, 0x5a, 0x3f, 0x4a, 0x89, 0xdf, 0xc8, 0x0d, 0x70, 0x44, 0x66, 0x04, 0x3c,
};

static int md_fresh_composite_case(void)
{
  uint8_t got[32];
  size_t i;
  int ok;
  struct ivs_machine *m = md_fresh();
  IVS_CHECK(m != NULL);
  ok = ivs_machine_state_digest(m, IVS_DIGEST_COMPOSITE, got) == IVS_OK;
  free(m);
  IVS_CHECK(ok);
  if (memcmp(got, md_fresh_composite, 32u) != 0) {
    char hex[65];
    for (i = 0; i < 32u; i++) {
      static const char xd[] = "0123456789abcdef";
      hex[2u * i] = xd[got[i] >> 4];
      hex[2u * i + 1u] = xd[got[i] & 15u];
    }
    hex[64] = '\0';
    ivs_test_diag("fresh composite %s", hex);
    return IVS_TEST_FAIL;
  }
  return IVS_TEST_PASS;
}

/* Repeatability pin; tier deterministic-repeatability; not an independent oracle. Content A
 * loaded, power-on reset, 100 units through the step seam: the composite (index 0) and the
 * domain 1..4 digests. Plan 05-03 re-recorded the composite and domains 2 (MEM, which gained the
 * PSG register and pull-down rows) and 4 (INSTANCE, which gained the PSG state rows) from the
 * new tree (composite version 2); domains 1 (CPU) and 3 (IMAGES) are the Phase 4 values byte for
 * byte, which is the regression proof that the PSG storage did not touch the CPU or the image
 * table. The Phase 4 (composite version 1) values, recorded from the Phase 3 code at commit
 * dae4339 (plan 04-05, A-03) and kept in data/expected/digest-v1-literals.ids (test.stepped.N):
 * the composite 90a23b92...c19e7f, MEM ea7ba62a...801f95 and INSTANCE fca8e62d...eb3ccb (full
 * values in that file). Plan 05-06 re-recorded the composite and INSTANCE again (the PSG now steps:
 * next_sample starts at 1 after a power-on reset and the 100 units advance the tone counters);
 * domains 1 (CPU), 2 (MEM) and 3 (IMAGES) are unchanged byte for byte (program A never touches the PSG).
 * Plan 05-07 re-recorded the composite and INSTANCE a third time (a power-on reset loads the noise seed and holds
 * the envelope; the 100 units shift the noise register); CPU, MEM and IMAGES are again byte for byte unchanged. */
static const uint8_t md_stepped[5][32] = {
    {
        0x61, 0xf8, 0x46, 0xa1, 0x3a, 0xf9, 0x92, 0xa9, 0xb2, 0x8e, 0x01, 0xf3, 0x1d, 0xeb, 0x51, 0x50,
        0x91, 0xe9, 0x2b, 0x3c, 0xef, 0xcb, 0xaf, 0xfd, 0x72, 0xad, 0xaa, 0x74, 0xd4, 0x63, 0x67, 0xa3,
    },
    {
        0x88, 0x10, 0x55, 0xb7, 0x67, 0x26, 0x5a, 0x9e, 0xc9, 0x3b, 0x94, 0x3e, 0xd1, 0x9e, 0x54, 0x7f,
        0x37, 0x0c, 0x0d, 0xef, 0x76, 0x47, 0x12, 0xfa, 0x32, 0xa6, 0xe0, 0x6e, 0x68, 0xce, 0x82, 0xab,
    },
    {
        0xaa, 0xfd, 0x49, 0x69, 0x27, 0x01, 0x79, 0x26, 0x5e, 0xec, 0x31, 0xdc, 0x14, 0xf3, 0x03, 0x95,
        0xaf, 0xc2, 0xe1, 0x66, 0xf2, 0x28, 0x7e, 0x5c, 0x26, 0x41, 0x8a, 0xff, 0x4e, 0xd5, 0x2a, 0x03,
    },
    {
        0x3f, 0x42, 0x08, 0x2e, 0xef, 0x76, 0xbb, 0xfb, 0xea, 0x05, 0xc9, 0x64, 0xfc, 0xf4, 0x7a, 0x2d,
        0xd0, 0x89, 0xd9, 0x05, 0x0e, 0xe8, 0x5c, 0x2f, 0xc3, 0x28, 0xb0, 0xca, 0x83, 0x99, 0x5d, 0xfe,
    },
    {
        0x88, 0x6d, 0xda, 0x2e, 0xcf, 0xdb, 0x7f, 0xe0, 0x95, 0x06, 0xcb, 0x8f, 0x7a, 0xd3, 0x3f, 0xd0,
        0xa0, 0x00, 0xd3, 0xed, 0x41, 0x24, 0xa3, 0x37, 0x66, 0x0d, 0x1e, 0x14, 0xd1, 0x83, 0x18, 0x30,
    },
};

static int md_stepped_literals(void)
{
  static const char xd[] = "0123456789abcdef";
  ivs_ms_inst x;
  uint8_t got[32];
  uint32_t d;
  size_t i;
  int ok = 1;
  IVS_CHECK(ivs_ms_powered(&x, IVS_MS_CONTENT_A, 0x5Au));
  if (!ivs_ms_step(x.m, 100u, NULL)) {
    ivs_ms_free(&x);
    return IVS_TEST_FAIL;
  }
  for (d = 0u; d <= 4u; d++) {
    if (ivs_machine_state_digest(x.m, d, got) != IVS_OK) {
      ivs_test_diag("domain %u refused", (unsigned)d);
      ok = 0;
      continue;
    }
    if (memcmp(got, md_stepped[d], 32u) != 0) {
      char hex[65];
      for (i = 0; i < 32u; i++) {
        hex[2u * i] = xd[got[i] >> 4];
        hex[2u * i + 1u] = xd[got[i] & 15u];
      }
      hex[64] = '\0';
      ivs_test_diag("stepped domain %u %s", (unsigned)d, hex);
      ok = 0;
    }
  }
  ivs_ms_free(&x);
  return ok ? IVS_TEST_PASS : IVS_TEST_FAIL;
}

/* ---- coverage ---- */

static int md_coverage(void)
{
  size_t n = ivs_machine_digest_field_count();
  size_t r;
  size_t i;
  size_t total = 0;
  uint32_t per_domain[MD_DOMAIN_IDS];
  uint8_t *seen = calloc(IVS_MACHINE_BYTES, 1u);
  IVS_CHECK(seen != NULL);
  memset(per_domain, 0, sizeof per_domain);
  for (r = 0; r < n; r++) {
    const ivs_digest_field *f = ivs_machine_digest_field(r);
    size_t len;
    if (f == NULL || f->name == NULL || !md_domain_find(f->domain, NULL) || f->count == 0u ||
        (f->kind != IVS_DIGEST_KIND_UINT && f->kind != IVS_DIGEST_KIND_PROFILE && f->kind != IVS_DIGEST_KIND_MAGIC &&
         f->kind != IVS_DIGEST_KIND_PAD) ||
        (f->kind == IVS_DIGEST_KIND_UINT && f->width != 1u && f->width != 2u && f->width != 4u && f->width != 8u) ||
        (f->kind == IVS_DIGEST_KIND_PROFILE && (f->width != 4u || f->count != 1u))) {
      ivs_test_diag("row %zu malformed", r);
      free(seen);
      return IVS_TEST_FAIL;
    }
    len = (size_t)f->width * f->count;
    if ((size_t)f->offset + len > IVS_MACHINE_BYTES) {
      ivs_test_diag("row %s past the end", f->name);
      free(seen);
      return IVS_TEST_FAIL;
    }
    for (i = 0; i < len; i++) {
      seen[f->offset + i]++;
    }
    total += len;
    if (f->kind == IVS_DIGEST_KIND_UINT || f->kind == IVS_DIGEST_KIND_PROFILE) {
      per_domain[f->domain]++;
    }
  }
  for (i = 0; i < IVS_MACHINE_BYTES; i++) {
    if (seen[i] != 1u) {
      ivs_test_diag("byte %zu covered %u times", i, (unsigned)seen[i]);
      free(seen);
      return IVS_TEST_FAIL;
    }
  }
  free(seen);
  IVS_CHECK(total == sizeof(struct ivs_machine));
  for (i = 0; i < MD_DOMAIN_COUNT; i++) {
    IVS_CHECK(per_domain[md_domains[i].id] > 0u);
  }
  return IVS_TEST_PASS;
}

/* ---- sensitivity ---- */

typedef struct md_base {
  struct ivs_machine *m;         /* the instance under change (a copy of a fresh one) */
  uint8_t *pre[MD_DOMAIN_IDS];   /* baseline preimages of the listed domains, by id */
  size_t len[MD_DOMAIN_IDS];
  uint8_t dig[MD_DOMAIN_IDS][32]; /* baseline digests, [0] the composite */
  uint8_t *scratch;
} md_base;

static void md_base_free(md_base *b)
{
  size_t i;
  for (i = 0; i < MD_DOMAIN_COUNT; i++) {
    free(b->pre[md_domains[i].id]);
  }
  free(b->scratch);
  free(b->m);
}

static int md_base_init(md_base *b)
{
  size_t i;
  memset(b, 0, sizeof *b);
  b->m = md_fresh();
  b->scratch = malloc(MD_PRE_MAX);
  if (b->m == NULL || b->scratch == NULL) {
    return 0;
  }
  if (ivs_machine_state_digest(b->m, IVS_DIGEST_COMPOSITE, b->dig[0]) != IVS_OK) {
    return 0;
  }
  for (i = 0; i < MD_DOMAIN_COUNT; i++) {
    uint32_t d = md_domains[i].id;
    if (ivs_machine_state_digest(b->m, d, b->dig[d]) != IVS_OK) {
      return 0;
    }
    b->len[d] = ivs_machine_digest_preimage(b->m, d, NULL, 0u);
    b->pre[d] = malloc(b->len[d]);
    if (b->pre[d] == NULL || ivs_machine_digest_preimage(b->m, d, b->pre[d], b->len[d]) != b->len[d]) {
      return 0;
    }
  }
  return 1;
}

/* After a change at instance byte pos: own's digest differs, every other listed domain's
 * preimage is identical to the baseline, and when check_comp the composite differs for a domain
 * of the composite and is identical for an observation domain (16..18, Pitfall 2). A changed
 * profile word (seam != 0) damages the handle (review WR-03): the public digest is then
 * BAD_HANDLE, and own's change is judged on its preimage through the seam, which takes no
 * handle. */
static int md_changed_only(md_base *b, uint32_t own, size_t pos, int check_comp, int seam)
{
  uint8_t got[32];
  size_t i;
  int own_in_comp = 0;
  if (!md_domain_find(own, &own_in_comp)) {
    ivs_test_diag("domain %u is not listed", (unsigned)own);
    return 0;
  }
  if (seam) {
    size_t len = ivs_machine_digest_preimage(b->m, own, b->scratch, MD_PRE_MAX);
    if (ivs_machine_state_digest(b->m, own, got) != IVS_E_BAD_HANDLE) {
      ivs_test_diag("byte %zu: a damaged profile word was not BAD_HANDLE", pos);
      return 0;
    }
    if (len == b->len[own] && memcmp(b->scratch, b->pre[own], len) == 0) {
      ivs_test_diag("byte %zu: domain %u preimage unchanged", pos, (unsigned)own);
      return 0;
    }
    check_comp = 0;
  } else if (ivs_machine_state_digest(b->m, own, got) != IVS_OK || memcmp(got, b->dig[own], 32u) == 0) {
    ivs_test_diag("byte %zu: domain %u digest unchanged", pos, (unsigned)own);
    return 0;
  }
  for (i = 0; i < MD_DOMAIN_COUNT; i++) {
    uint32_t d = md_domains[i].id;
    size_t len;
    if (d == own) {
      continue;
    }
    len = ivs_machine_digest_preimage(b->m, d, b->scratch, MD_PRE_MAX);
    if (len != b->len[d] || memcmp(b->scratch, b->pre[d], len) != 0) {
      ivs_test_diag("byte %zu: domain %u changed too", pos, (unsigned)d);
      return 0;
    }
  }
  if (check_comp) {
    int same;
    if (ivs_machine_state_digest(b->m, IVS_DIGEST_COMPOSITE, got) != IVS_OK) {
      ivs_test_diag("byte %zu: composite refused", pos);
      return 0;
    }
    same = memcmp(got, b->dig[0], 32u) == 0;
    if (own_in_comp && same) {
      ivs_test_diag("byte %zu: composite unchanged", pos);
      return 0;
    }
    if (!own_in_comp && !same) {
      ivs_test_diag("byte %zu: composite changed by observation domain %u", pos, (unsigned)own);
      return 0;
    }
  }
  return 1;
}

/* Flip the first and the last byte of every serialized row of domain. */
static int md_sensitivity(uint32_t domain)
{
  md_base b;
  size_t n = ivs_machine_digest_field_count();
  size_t r;
  uint32_t rows = 0u;
  int ok;
  int seam;
  uint8_t *bytes;
  ok = md_base_init(&b);
  bytes = (uint8_t *)b.m;
  for (r = 0; ok && r < n; r++) {
    const ivs_digest_field *f = ivs_machine_digest_field(r);
    size_t first;
    size_t last;
    if (f->domain != domain || (f->kind != IVS_DIGEST_KIND_UINT && f->kind != IVS_DIGEST_KIND_PROFILE)) {
      continue;
    }
    first = f->offset;
    last = (size_t)f->offset + (size_t)f->width * f->count - 1u;
    seam = f->kind == IVS_DIGEST_KIND_PROFILE;
    bytes[first] ^= 0xFFu;
    ok = md_changed_only(&b, domain, first, 1, seam);
    bytes[first] ^= 0xFFu;
    if (ok && last != first) {
      bytes[last] ^= 0xFFu;
      ok = md_changed_only(&b, domain, last, 0, seam);
      bytes[last] ^= 0xFFu;
    }
    if (!ok) {
      ivs_test_diag("row %s", f->name);
    }
    rows++;
  }
  md_base_free(&b);
  IVS_CHECK(ok);
  IVS_CHECK(rows > 0u);
  return IVS_TEST_PASS;
}

static int md_sensitivity_cpu(void)
{
  return md_sensitivity(IVS_DIGEST_CPU);
}

static int md_sensitivity_mem(void)
{
  return md_sensitivity(IVS_DIGEST_MEM);
}

static int md_sensitivity_images(void)
{
  return md_sensitivity(IVS_DIGEST_IMAGES);
}

static int md_sensitivity_instance(void)
{
  return md_sensitivity(IVS_DIGEST_INSTANCE);
}

static int md_sensitivity_video(void)
{
  return md_sensitivity(IVS_DIGEST_VIDEO);
}

static int md_sensitivity_audio(void)
{
  return md_sensitivity(IVS_DIGEST_AUDIO);
}

static int md_sensitivity_input(void)
{
  return md_sensitivity(IVS_DIGEST_INPUT);
}

/* Domain 19 (PSGLOG, plan 05-03): chain and count move only their own domain, never the composite. */
static int md_psglog_rows(void)
{
  return md_sensitivity(IVS_DIGEST_PSGLOG);
}

/* ---- PSG storage rows (plan 05-03, D-04, D-16, D-18, A-13) ---- */

/* 1 when the row has the given name prefix. */
static int md_name_has(const ivs_digest_field *f, const char *prefix)
{
  return strncmp(f->name, prefix, strlen(prefix)) == 0;
}

/* 1 for a serialized row that plan 05-03 added: the PSG registers and pull-downs (MEM), the PSG
 * device state (INSTANCE), the audio rows emitted and prev_levels and the PSGLOG rows. */
static int md_is_new_row(const ivs_digest_field *f)
{
  return f->kind == IVS_DIGEST_KIND_UINT &&
         (md_name_has(f, "mem.psg_reg") || md_name_has(f, "mem.port_pulldown") || md_name_has(f, "psg.") ||
          md_name_has(f, "obs.audio.emitted") || md_name_has(f, "obs.audio.prev_levels") ||
          md_name_has(f, "obs.psglog."));
}

/* Flipping the first and the last byte of every PSG state member changes the INSTANCE digest and
 * the composite and no other domain's preimage (D-16, D-18: guest-visible state is in the
 * composite). Exactly the ten UINT rows of the table must be seen. */
static int md_psg_rows_in_instance(void)
{
  md_base b;
  size_t n = ivs_machine_digest_field_count();
  size_t r;
  uint32_t rows = 0u;
  int ok;
  uint8_t *bytes;
  ok = md_base_init(&b);
  bytes = (uint8_t *)b.m;
  for (r = 0; ok && r < n; r++) {
    const ivs_digest_field *f = ivs_machine_digest_field(r);
    size_t first;
    size_t last;
    if (f->kind != IVS_DIGEST_KIND_UINT || !md_name_has(f, "psg.")) {
      continue;
    }
    first = f->offset;
    last = (size_t)f->offset + (size_t)f->width * f->count - 1u;
    bytes[first] ^= 0xFFu;
    ok = md_changed_only(&b, IVS_DIGEST_INSTANCE, first, 1, 0);
    bytes[first] ^= 0xFFu;
    if (ok && last != first) {
      bytes[last] ^= 0xFFu;
      ok = md_changed_only(&b, IVS_DIGEST_INSTANCE, last, 1, 0);
      bytes[last] ^= 0xFFu;
    }
    if (!ok) {
      ivs_test_diag("row %s", f->name);
    }
    rows++;
  }
  md_base_free(&b);
  IVS_CHECK(ok);
  IVS_CHECK(rows == 10u);
  return IVS_TEST_PASS;
}

/* The tail of the MEM preimage with distinct register words and pull-downs, written out by hand:
 * psg_reg (u32 count 16, sixteen u16: 1000, 1111, 1222 ... 1FFF) then port_pulldown (u32 count 2,
 * A5 5A). */
static const uint8_t md_mem_tail[42] = {
    0x00, 0x00, 0x00, 0x10, 0x10, 0x00, 0x11, 0x11, 0x12, 0x22, 0x13, 0x33, 0x14, 0x44, 0x15, 0x55,
    0x16, 0x66, 0x17, 0x77, 0x18, 0x88, 0x19, 0x99, 0x1A, 0xAA, 0x1B, 0xBB, 0x1C, 0xCC, 0x1D, 0xDD,
    0x1E, 0xEE, 0x1F, 0xFF, 0x00, 0x00, 0x00, 0x02, 0xA5, 0x5A,
};

/* Flipping any byte of psg_reg or port_pulldown changes the MEM digest and the composite and no
 * other domain (A-13: digest.mem moves in every golden), and the MEM preimage ends with the
 * hand-framed rows in table order. */
static int md_mem_rows_psg_regs(void)
{
  md_base b;
  size_t base_reg = offsetof(struct ivs_machine, mem.psg_reg);
  size_t base_pd = offsetof(struct ivs_machine, mem.port_pulldown);
  size_t i;
  size_t len;
  int ok;
  uint8_t *bytes;
  ok = md_base_init(&b);
  bytes = (uint8_t *)b.m;
  for (i = 0u; ok && i < 2u * IVS_MEMSYS_PSG_REGS; i++) {
    bytes[base_reg + i] ^= 0xFFu;
    ok = md_changed_only(&b, IVS_DIGEST_MEM, base_reg + i, 1, 0);
    bytes[base_reg + i] ^= 0xFFu;
  }
  for (i = 0u; ok && i < IVS_MEMSYS_PSG_PORTS; i++) {
    bytes[base_pd + i] ^= 0xFFu;
    ok = md_changed_only(&b, IVS_DIGEST_MEM, base_pd + i, 1, 0);
    bytes[base_pd + i] ^= 0xFFu;
  }
  if (ok) {
    for (i = 0u; i < IVS_MEMSYS_PSG_REGS; i++) {
      b.m->mem.psg_reg[i] = (uint16_t)(0x1000u + 0x0111u * i);
    }
    b.m->mem.port_pulldown[0] = 0xA5u;
    b.m->mem.port_pulldown[1] = 0x5Au;
    len = ivs_machine_digest_preimage(b.m, IVS_DIGEST_MEM, b.scratch, MD_PRE_MAX);
    ok = len == b.len[IVS_DIGEST_MEM] && memcmp(b.scratch + len - sizeof md_mem_tail, md_mem_tail, sizeof md_mem_tail) == 0;
    if (!ok) {
      ivs_test_diag("MEM preimage tail differs from the hand-framed rows");
    }
  }
  md_base_free(&b);
  IVS_CHECK(ok);
  return IVS_TEST_PASS;
}

/* Flipping any byte of the audio rows emitted and prev_levels changes the AUDIO digest and no
 * other domain, the composite included (observation domain, D-18). */
static int md_audio_rows(void)
{
  md_base b;
  size_t base_e = offsetof(struct ivs_machine, obs.audio.emitted);
  size_t base_p = offsetof(struct ivs_machine, obs.audio.prev_levels);
  size_t i;
  int ok;
  uint8_t *bytes;
  ok = md_base_init(&b);
  bytes = (uint8_t *)b.m;
  for (i = 0u; ok && i < sizeof b.m->obs.audio.emitted; i++) {
    bytes[base_e + i] ^= 0xFFu;
    ok = md_changed_only(&b, IVS_DIGEST_AUDIO, base_e + i, 1, 0);
    bytes[base_e + i] ^= 0xFFu;
  }
  for (i = 0u; ok && i < sizeof b.m->obs.audio.prev_levels; i++) {
    bytes[base_p + i] ^= 0xFFu;
    ok = md_changed_only(&b, IVS_DIGEST_AUDIO, base_p + i, 1, 0);
    bytes[base_p + i] ^= 0xFFu;
  }
  md_base_free(&b);
  IVS_CHECK(ok);
  return IVS_TEST_PASS;
}

/* Flipping the first and the last byte of every row plan 05-03 added leaves the CPU and IMAGES
 * digests unchanged (the PSG storage does not touch the CPU state or the image table), and at
 * least the sixteen rows of the table are seen. */
static int md_cpu_images_independent_of_psg(void)
{
  md_base b;
  size_t n = ivs_machine_digest_field_count();
  size_t r;
  uint32_t rows = 0u;
  int ok;
  uint8_t *bytes;
  ok = md_base_init(&b);
  bytes = (uint8_t *)b.m;
  for (r = 0; ok && r < n; r++) {
    const ivs_digest_field *f = ivs_machine_digest_field(r);
    size_t pos[2];
    size_t k;
    if (!md_is_new_row(f)) {
      continue;
    }
    pos[0] = f->offset;
    pos[1] = (size_t)f->offset + (size_t)f->width * f->count - 1u;
    for (k = 0u; ok && k < 2u; k++) {
      uint8_t got[32];
      bytes[pos[k]] ^= 0xFFu;
      ok = ivs_machine_state_digest(b.m, IVS_DIGEST_CPU, got) == IVS_OK && memcmp(got, b.dig[IVS_DIGEST_CPU], 32u) == 0 &&
           ivs_machine_state_digest(b.m, IVS_DIGEST_IMAGES, got) == IVS_OK &&
           memcmp(got, b.dig[IVS_DIGEST_IMAGES], 32u) == 0;
      bytes[pos[k]] ^= 0xFFu;
      if (!ok) {
        ivs_test_diag("row %s moved the CPU or IMAGES digest", f->name);
      }
    }
    rows++;
  }
  md_base_free(&b);
  IVS_CHECK(ok);
  IVS_CHECK(rows == 16u);
  return IVS_TEST_PASS;
}

/* Flipping a declared padding byte leaves its domain's preimage identical; all padding flipped at
 * once leaves every domain digest and the composite unchanged. */
static int md_padding_ignored(void)
{
  md_base b;
  size_t n = ivs_machine_digest_field_count();
  size_t r;
  size_t i;
  uint32_t pads = 0u;
  int ok;
  uint8_t *bytes;
  ok = md_base_init(&b);
  bytes = (uint8_t *)b.m;
  for (r = 0; ok && r < n; r++) {
    const ivs_digest_field *f = ivs_machine_digest_field(r);
    if (f->kind != IVS_DIGEST_KIND_PAD) {
      continue;
    }
    for (i = 0; ok && i < (size_t)f->width * f->count; i++) {
      size_t pos = f->offset + i;
      size_t len;
      bytes[pos] ^= 0xFFu;
      len = ivs_machine_digest_preimage(b.m, f->domain, b.scratch, MD_PRE_MAX);
      ok = len == b.len[f->domain] && memcmp(b.scratch, b.pre[f->domain], len) == 0;
      if (!ok) {
        ivs_test_diag("padding byte %zu (%s) changed domain %u", pos, f->name, (unsigned)f->domain);
      }
      pads++;
    }
  }
  /* Now every padding byte is flipped at once: the composite and every listed domain. */
  for (i = 0; ok && i <= MD_DOMAIN_COUNT; i++) {
    uint8_t got[32];
    uint32_t d = i == 0 ? IVS_DIGEST_COMPOSITE : md_domains[i - 1u].id;
    ok = ivs_machine_state_digest(b.m, d, got) == IVS_OK && memcmp(got, b.dig[d], 32u) == 0;
    if (!ok) {
      ivs_test_diag("domain %u changed with all padding flipped", (unsigned)d);
    }
  }
  md_base_free(&b);
  IVS_CHECK(ok);
  IVS_CHECK(pads > 0u);
  return IVS_TEST_PASS;
}

/* ---- observation domains (plan 04-05, D-10) ---- */

/* The VIDEO, AUDIO, INPUT and PSGLOG digests of m into d[0..3]; 1 when all four are IVS_OK. */
static int md_obs_digests(const ivs_machine *m, uint8_t d[4][32])
{
  return ivs_machine_state_digest(m, IVS_DIGEST_VIDEO, d[0]) == IVS_OK &&
         ivs_machine_state_digest(m, IVS_DIGEST_AUDIO, d[1]) == IVS_OK &&
         ivs_machine_state_digest(m, IVS_DIGEST_INPUT, d[2]) == IVS_OK &&
         ivs_machine_state_digest(m, IVS_DIGEST_PSGLOG, d[3]) == IVS_OK;
}

/* A fresh instance's composite is still md_fresh_composite with the observation rows present,
 * and an instance after content A and a power-on reset has the same VIDEO, AUDIO and INPUT
 * digests as the fresh one. */
static int md_video_fresh(void)
{
  ivs_ms_inst x;
  uint8_t comp[32];
  uint8_t df[4][32];
  uint8_t dr[4][32];
  int ok;
  struct ivs_machine *f = md_fresh();
  IVS_CHECK(f != NULL);
  ok = ivs_machine_state_digest(f, IVS_DIGEST_COMPOSITE, comp) == IVS_OK && memcmp(comp, md_fresh_composite, 32u) == 0 &&
       md_obs_digests(f, df);
  free(f);
  IVS_CHECK(ok);
  IVS_CHECK(ivs_ms_powered(&x, IVS_MS_CONTENT_A, 0x5Au));
  ok = md_obs_digests(x.m, dr) && memcmp(df, dr, sizeof df) == 0;
  ivs_ms_free(&x);
  IVS_CHECK(ok);
  return IVS_TEST_PASS;
}

/* Flipping one byte of a VIDEO, an AUDIO and an INPUT row (one at a time) changes exactly that
 * domain's digest; the composite and every other domain (1..4 and the other two observation
 * domains) keep their baseline digests (D-10, A-03, Pitfall 2). */
static int md_composite_excludes(void)
{
  static const struct {
    size_t off;
    uint32_t own;
  } flips[3] = {
      {offsetof(struct ivs_machine, obs.video.chain), IVS_DIGEST_VIDEO},
      {offsetof(struct ivs_machine, obs.audio.count), IVS_DIGEST_AUDIO},
      {offsetof(struct ivs_machine, obs.input.words), IVS_DIGEST_INPUT},
  };
  md_base b;
  size_t k;
  size_t i;
  int ok;
  uint8_t *bytes;
  ok = md_base_init(&b);
  bytes = (uint8_t *)b.m;
  for (k = 0; ok && k < 3u; k++) {
    bytes[flips[k].off] ^= 0xFFu;
    for (i = 0; ok && i <= MD_DOMAIN_COUNT; i++) {
      uint8_t got[32];
      uint32_t d = i == 0 ? IVS_DIGEST_COMPOSITE : md_domains[i - 1u].id;
      int same;
      ok = ivs_machine_state_digest(b.m, d, got) == IVS_OK;
      same = ok && memcmp(got, b.dig[d], 32u) == 0;
      if (ok && (d == flips[k].own) == same) {
        ivs_test_diag("flip in domain %u: domain %u %s", (unsigned)flips[k].own, (unsigned)d,
                      same ? "unchanged" : "changed");
        ok = 0;
      }
    }
    bytes[flips[k].off] ^= 0xFFu;
  }
  md_base_free(&b);
  IVS_CHECK(ok);
  return IVS_TEST_PASS;
}

/* A power-on reset of a stepped instance whose observation rows are all non-zero leaves the
 * observation rows byte-identical to a fresh instance's: every byte zero except the input log's
 * chain, which holds its start value h0 (plan 04-11, D-15; its literal is pinned in
 * tests/run/run_input.c), and the VIDEO, AUDIO and INPUT digests equal a fresh instance's. */
static int md_reset_clears_obs(void)
{
  ivs_ms_inst x;
  ivs_machine_obs zero;
  ivs_machine_obs h0_only;
  uint8_t df[4][32];
  uint8_t dr[4][32];
  int ok;
  struct ivs_machine *f = md_fresh();
  IVS_CHECK(f != NULL);
  ok = md_obs_digests(f, df);
  memcpy(&zero, &f->obs, sizeof zero);
  free(f);
  IVS_CHECK(ok);
  /* The fresh rows: zero everywhere except a non-zero input chain. */
  memset(&h0_only, 0, sizeof h0_only);
  memcpy(h0_only.input.chain, zero.input.chain, sizeof h0_only.input.chain);
  IVS_CHECK(memcmp(&zero, &h0_only, sizeof zero) == 0);
  memset(&h0_only, 0, sizeof h0_only);
  IVS_CHECK(memcmp(zero.input.chain, h0_only.input.chain, sizeof zero.input.chain) != 0);
  IVS_CHECK(ivs_ms_powered(&x, IVS_MS_CONTENT_A, 0x5Au));
  ok = ivs_ms_step(x.m, 10u, NULL);
  if (ok) {
    memset(&x.m->obs, 0xA5, sizeof x.m->obs);
    ok = ivs_machine_reset(x.m, IVS_RESET_POWER_ON) == IVS_OK;
  }
  if (ok && memcmp(&x.m->obs, &zero, sizeof zero) != 0) {
    ivs_test_diag("observation rows not cleared by reset");
    ok = 0;
  }
  ok = ok && md_obs_digests(x.m, dr) && memcmp(df, dr, sizeof df) == 0;
  ivs_ms_free(&x);
  IVS_CHECK(ok);
  return IVS_TEST_PASS;
}

/* ---- refusals ---- */

/* digest(handle, domain, out) expecting want, with out and the instance untouched. */
static int md_refused(int null_handle, int destroyed, uint32_t domain, int null_out, ivs_status want)
{
  struct ivs_machine *m = md_fresh();
  uint8_t *snap = malloc(IVS_MACHINE_BYTES);
  uint8_t out[32];
  uint8_t poison[32];
  ivs_status got;
  int ok;
  if (m == NULL || snap == NULL) {
    free(m);
    free(snap);
    return IVS_TEST_FAIL;
  }
  if (destroyed && ivs_machine_destroy(m) != IVS_OK) {
    free(m);
    free(snap);
    return IVS_TEST_FAIL;
  }
  memcpy(snap, m, IVS_MACHINE_BYTES);
  memset(out, 0xA5, sizeof out);
  memcpy(poison, out, sizeof out);
  got = ivs_machine_state_digest(null_handle ? NULL : m, domain, null_out ? NULL : out);
  ok = got == want && memcmp(out, poison, sizeof out) == 0 && memcmp(snap, m, IVS_MACHINE_BYTES) == 0;
  if (got != want) {
    ivs_test_diag("status %d, want %d", got, want);
  }
  free(snap);
  free(m);
  return ok ? IVS_TEST_PASS : IVS_TEST_FAIL;
}

static int md_refuse_domain_5(void)
{
  return md_refused(0, 0, 5u, 0, IVS_E_BAD_ARG);
}

/* 20, the first domain after the observation domains 16..19 (plan 04-05 retargeted the Phase 3
 * domain-16 refusal to 19, A-03; plan 05-03 makes 19 a live domain and retargets it to 20). */
static int md_refuse_domain_20(void)
{
  return md_refused(0, 0, IVS_DIGEST_PSGLOG + 1u, 0, IVS_E_BAD_ARG);
}

static int md_refuse_domain_99(void)
{
  return md_refused(0, 0, 99u, 0, IVS_E_BAD_ARG);
}

static int md_refuse_null_out(void)
{
  return md_refused(0, 0, IVS_DIGEST_COMPOSITE, 1, IVS_E_BAD_ARG);
}

static int md_refuse_null_handle(void)
{
  return md_refused(1, 0, IVS_DIGEST_COMPOSITE, 0, IVS_E_BAD_HANDLE);
}

static int md_refuse_destroyed(void)
{
  return md_refused(0, 1, IVS_DIGEST_CPU, 0, IVS_E_BAD_HANDLE);
}

/* A bad domain and a NULL handle: the argument class comes first. */
static int md_refuse_domain_over_handle(void)
{
  return md_refused(1, 0, 99u, 0, IVS_E_BAD_ARG);
}

const ivs_test_case ivs_machine_digest_cases[] = {
    {"machine.digest.audio-rows", md_audio_rows, NULL},
    {"machine.digest.composite-excludes-16-18", md_composite_excludes, NULL},
    {"machine.digest.composite-framing", md_composite_framing, NULL},
    {"machine.digest.composite-v2-framing", md_composite_v2_framing, NULL},
    {"machine.digest.coverage", md_coverage, NULL},
    {"machine.digest.cpu-images-independent-of-psg", md_cpu_images_independent_of_psg, NULL},
    {"machine.digest.domain-19-fresh", md_domain_19_fresh, NULL},
    {"machine.digest.fresh-composite", md_fresh_composite_case, NULL},
    {"machine.digest.mem-rows-psg-regs", md_mem_rows_psg_regs, NULL},
    {"machine.digest.padding-ignored", md_padding_ignored, NULL},
    {"machine.digest.preimage-hash", md_preimage_hash, NULL},
    {"machine.digest.psg-rows-in-instance-domain", md_psg_rows_in_instance, NULL},
    {"machine.digest.psglog-rows", md_psglog_rows, NULL},
    {"machine.digest.sensitivity-audio", md_sensitivity_audio, NULL},
    {"machine.digest.sensitivity-cpu", md_sensitivity_cpu, NULL},
    {"machine.digest.sensitivity-images", md_sensitivity_images, NULL},
    {"machine.digest.sensitivity-input", md_sensitivity_input, NULL},
    {"machine.digest.sensitivity-instance", md_sensitivity_instance, NULL},
    {"machine.digest.sensitivity-mem", md_sensitivity_mem, NULL},
    {"machine.digest.sensitivity-video", md_sensitivity_video, NULL},
    {"machine.digest.stepped-literals", md_stepped_literals, NULL},
    {"machine.digest.stream-cpu", md_stream_cpu, NULL},
    {"machine.digest.stream-cpu-synthetic", md_stream_cpu_synthetic, NULL},
    {"machine.digest.stream-instance", md_stream_instance, NULL},
    {"machine.digest.stream-instance-psg", md_stream_instance_psg, NULL},
    {"machine.digest.video-fresh", md_video_fresh, NULL},
    {"machine.refuse.digest.destroyed", md_refuse_destroyed, NULL},
    {"machine.refuse.digest.domain-20", md_refuse_domain_20, NULL},
    {"machine.refuse.digest.domain-5", md_refuse_domain_5, NULL},
    {"machine.refuse.digest.domain-99", md_refuse_domain_99, NULL},
    {"machine.refuse.digest.domain-over-handle", md_refuse_domain_over_handle, NULL},
    {"machine.refuse.digest.null-handle", md_refuse_null_handle, NULL},
    {"machine.refuse.digest.null-out", md_refuse_null_out, NULL},
    {"machine.reset.clears-observation", md_reset_clears_obs, NULL},
};
const size_t ivs_machine_digest_count = sizeof ivs_machine_digest_cases / sizeof ivs_machine_digest_cases[0];

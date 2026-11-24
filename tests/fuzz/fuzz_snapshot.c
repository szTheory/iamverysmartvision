/* SPDX-License-Identifier: MIT */
/* Optional libFuzzer target for the IVSN v1 snapshot reader and restore (D-20, plan 08-15; threat T-08-36). Never part of the gate, never
 * registered with ctest, never built by a preset, never in CI: the target exists only when the cache option IVS_ENABLE_LIBFUZZER is ON (default OFF)
 * and is excluded from the default build. The gate's evidence is the seeded family state.fuzz.* of suite state (tests/state/state_fuzz.c); this
 * target is a deeper, coverage-guided local run of the same invariants.
 *
 * Recipe (a scratch build directory outside the presets; Apple clang has no libFuzzer runtime, so use a Homebrew LLVM clang,
 * `$(brew --prefix llvm@21)/bin/clang` or llvm@18, or a Linux clang):
 *   cmake -S . -B build/fuzz -G Ninja -DCMAKE_BUILD_TYPE=Debug -DCMAKE_C_COMPILER=<that clang> \
 *     -DCMAKE_C_FLAGS=-fsanitize=fuzzer-no-link -DIVS_ENABLE_LIBFUZZER=ON -DIVS_LIBFUZZER_SANITIZERS=fuzzer,undefined
 *   cmake --build build/fuzz --target ivs_fuzz_snapshot
 *   mkdir -p build/fuzz/corpus && IVS_SNAPSHOT_FUZZ_SEED=build/fuzz/corpus/seed.bin build/fuzz/tests/ivs_fuzz_snapshot -runs=1
 *   build/fuzz/tests/ivs_fuzz_snapshot build/fuzz/corpus -max_total_time=60
 * -fsanitize=fuzzer-no-link in CMAKE_C_FLAGS instruments the core for coverage; the default sanitizer list fuzzer,address,undefined adds
 * AddressSanitizer where its runtime starts (it hung at start-up on a macOS 26 host with Homebrew LLVM 18 and 21, 2026-11-11, hence the
 * fuzzer,undefined above). The corpus and any crash files stay in the build tree (build/ is ignored); a crash input is a snapshot of a machine
 * that ran guest code, so never commit one (.ivsn is binary by name and ignored).
 *
 * Input: byte 0 is a control byte (bit 0: recompute the SHA-256 trailer over the rest) and the bytes after it are the snapshot, copied to a heap
 * buffer of exactly that size, so that a read past the supplied length is an error under a sanitizer. With bit 0 set and at least 32 snapshot bytes
 * the trailer is resealed, so a mutation reaches the validators and the invariants behind the checksum. The seed written by IVS_SNAPSHOT_FUZZ_SEED
 * is the control byte 1 followed by one valid snapshot of the stand-in instance.
 *
 * Per input, in a live instance of the stand-in boot content that is put back from a baseline copy (its STATE and PAD rows poisoned, its images
 * loaded) before every call: the instance-free describe and restore agree on the container (a refusal of steps 2 to 6 has the same reason, a
 * container that describe accepts is refused by restore only with a reason from 72 to 80); every status is the one its reason travels with; a
 * refusal leaves every byte of the instance as it was; an acceptance writes no byte outside the STATE and PAD rows, re-encodes every section
 * (the state-only preimage equals the section of the input) and a full save differs from the input only in the eight informational digest fields and
 * the trailer, and the restored machine runs 6,000 ticks; a second restore into a put back instance gives the identical result. A broken invariant
 * aborts, which libFuzzer reports as a crash with the input saved. */
#include <stddef.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "common/ivs_sha256.h"
#include "ivs/ivs.h"
#include "machine/machine.h"
#include "machine_cases.h"

int LLVMFuzzerInitialize(int *argc, char ***argv);
int LLVMFuzzerTestOneInput(const uint8_t *data, size_t size);

#define FZ_REP 400u
#define FZ_STEPS 3000u

static ivs_ms_inst fz_inst;
static uint8_t *fz_baseline;
static uint8_t fz_valid[IVS_SNAPSHOT_MAX_BYTES];
static size_t fz_valid_n;
static int fz_ready;

/* The status each reason of 58 to 81 travels with (the table of "Refusal reasons" of docs/formats/ivsn-v1.md). */
static const ivs_status fz_status[24] = {
    IVS_E_BAD_ARG,         IVS_E_BAD_ARG,        IVS_E_OVERSIZED,     IVS_E_MALFORMED,     IVS_E_MALFORMED,   IVS_E_UNSUPPORTED,
    IVS_E_UNSUPPORTED,     IVS_E_UNSUPPORTED,    IVS_E_UNSUPPORTED,   IVS_E_MALFORMED,     IVS_E_MALFORMED,   IVS_E_MALFORMED,
    IVS_E_DIGEST_MISMATCH, IVS_E_MALFORMED,      IVS_E_UNKNOWN_PROFILE, IVS_E_STATE_MISMATCH, IVS_E_STATE_MISMATCH, IVS_E_MISSING_SYSTEM_IMAGE,
    IVS_E_STATE_MISMATCH,  IVS_E_MALFORMED,      IVS_E_MALFORMED,     IVS_E_MALFORMED,     IVS_E_MALFORMED,   IVS_E_BAD_ARG};

static void fz_require(int cond)
{
  if (!cond) {
    abort();
  }
}

/* Fills every STATE and PAD row of the instance (the field table says where) with one byte, so that a restore that misses a row leaves it. */
static void fz_poison(uint8_t *storage)
{
  size_t n = ivs_machine_digest_field_count();
  size_t i;
  for (i = 0u; i < n; i++) {
    const ivs_digest_field *f = ivs_machine_digest_field(i);
    if (f != NULL && (f->snap == IVS_SNAP_STATE || f->snap == IVS_SNAP_PAD)) {
      memset(storage + f->offset, 0xA5, (size_t)f->width * f->count);
    }
  }
}

int LLVMFuzzerInitialize(int *argc, char ***argv)
{
  const char *seed = getenv("IVS_SNAPSHOT_FUZZ_SEED");
  uint64_t w = 0u;
  (void)argc;
  (void)argv;
  fz_require(ivs_ms_powered_prog(&fz_inst, IVS_MS_PROG_TONE, 0x5Au));
  fz_require(ivs_ms_step(fz_inst.m, FZ_STEPS, NULL));
  fz_require(ivs_machine_snapshot_save(fz_inst.m, fz_valid, sizeof fz_valid, &w) == IVS_OK && w > 0u && w <= sizeof fz_valid);
  fz_valid_n = (size_t)w;
  if (seed != NULL) {
    FILE *f = fopen(seed, "wb");
    fz_require(f != NULL);
    fputc(1, f);
    fz_require(fwrite(fz_valid, 1u, fz_valid_n, f) == fz_valid_n);
    fclose(f);
  }
  fz_poison(fz_inst.storage);
  fz_baseline = (uint8_t *)malloc(IVS_MACHINE_BYTES);
  fz_require(fz_baseline != NULL);
  memcpy(fz_baseline, fz_inst.storage, IVS_MACHINE_BYTES);
  fz_ready = 1;
  return 0;
}

typedef struct fz_out {
  ivs_status st;
  uint32_t reason;
  uint64_t off;
  uint32_t sec;
} fz_out;

/* One restore into the baseline instance. */
static void fz_restore(const uint8_t *b, size_t n, fz_out *o)
{
  union {
    ivs_snapshot_report r;
    uint8_t b[FZ_REP];
  } rep;
  memset(&rep, 0xA5, sizeof rep);
  rep.r.struct_size = FZ_REP;
  o->st = ivs_machine_snapshot_restore(fz_inst.m, b, n, &rep.r);
  o->reason = rep.r.reason;
  o->off = rep.r.byte_offset;
  o->sec = rep.r.section_index;
}

/* The restored machine runs 6,000 ticks through the public call (a state that restore accepted must not make the core fail). */
static void fz_run(void)
{
  const struct ivs_machine *im = fz_inst.m;
  ivs_run_request q;
  ivs_run_report r;
  memset(&q, 0, sizeof q);
  q.struct_size = (uint32_t)sizeof q;
  q.target_ticks = im->time_ticks + 6000u;
  memset(&r, 0, sizeof r);
  r.struct_size = (uint32_t)sizeof r;
  if (im->time_ticks < ((uint64_t)1u << 62)) {
    fz_require(ivs_machine_run(fz_inst.m, &q, &r) == IVS_OK);
  }
}

static void fz_accepted(const uint8_t *b, size_t n)
{
  static uint8_t pre[4096];
  static uint8_t out[IVS_SNAPSHOT_MAX_BYTES];
  uint8_t *now = fz_inst.storage;
  size_t i;
  uint64_t w = 0u;
  uint32_t k;
  /* nothing outside the STATE and PAD rows was written: wherever the instance differs from the baseline, a STATE or PAD row covers it */
  for (i = 0u; i < IVS_MACHINE_BYTES; i++) {
    if (now[i] != fz_baseline[i]) {
      size_t rows = ivs_machine_digest_field_count();
      size_t r;
      int covered = 0;
      for (r = 0u; r < rows && !covered; r++) {
        const ivs_digest_field *f = ivs_machine_digest_field(r);
        covered = (f->snap == IVS_SNAP_STATE || f->snap == IVS_SNAP_PAD) && i >= f->offset && i < f->offset + (size_t)f->width * f->count;
      }
      fz_require(covered);
    }
  }
  /* every section re-encodes: the state-only preimage of its domain is the bytes of the input's section */
  for (k = 0u; k < IVS_SNAP_SECTIONS; k++) {
    const uint8_t *e = b + IVS_SNAP_OFF_TABLE + k * IVS_SNAP_ENTRY_BYTES;
    uint32_t off = ((uint32_t)e[4] << 24) | ((uint32_t)e[5] << 16) | ((uint32_t)e[6] << 8) | (uint32_t)e[7];
    uint32_t len = ((uint32_t)e[8] << 24) | ((uint32_t)e[9] << 16) | ((uint32_t)e[10] << 8) | (uint32_t)e[11];
    size_t got = ivs_machine_state_preimage(fz_inst.m, e[0], pre, sizeof pre);
    fz_require(got == len && off + len <= n && memcmp(pre, b + off, got) == 0);
  }
  /* a full save equals the input except the eight informational digest fields and the trailer, which a re-save recomputes */
  fz_require(ivs_machine_snapshot_save(fz_inst.m, out, sizeof out, &w) == IVS_OK && w == n);
  memcpy(out + IVS_SNAP_OFF_COMPOSITE, b + IVS_SNAP_OFF_COMPOSITE, 32u);
  for (k = 0u; k < IVS_SNAP_SECTIONS; k++) {
    size_t at = IVS_SNAP_OFF_TABLE + k * IVS_SNAP_ENTRY_BYTES + 12u;
    memcpy(out + at, b + at, 32u);
  }
  ivs_sha256_digest(out, n - IVS_SNAP_TRAILER_BYTES, out + n - IVS_SNAP_TRAILER_BYTES);
  fz_require(memcmp(out, b, n) == 0);
}

int LLVMFuzzerTestOneInput(const uint8_t *data, size_t size)
{
  union {
    ivs_snapshot_report r;
    uint8_t b[FZ_REP];
  } drep;
  fz_out a;
  fz_out c;
  ivs_status dst;
  uint8_t *b;
  uint8_t *orig;
  size_t n;
  if (!fz_ready) {
    fz_require(LLVMFuzzerInitialize(NULL, NULL) == 0);
  }
  if (size == 0u || size - 1u > (size_t)IVS_SNAPSHOT_MAX_BYTES + 1u) {
    return 0; /* the oversize refusal reads no byte; the seeded family covers it */
  }
  n = size - 1u;
  b = (uint8_t *)malloc(n > 0u ? n : 1u);
  fz_require(b != NULL);
  if (n > 0u) {
    memcpy(b, data + 1, n);
  }
  if ((data[0] & 1u) != 0u && n >= IVS_SNAP_TRAILER_BYTES) {
    ivs_sha256_digest(b, n - IVS_SNAP_TRAILER_BYTES, b + n - IVS_SNAP_TRAILER_BYTES);
  }
  orig = (uint8_t *)malloc(n > 0u ? n : 1u);
  fz_require(orig != NULL);
  memcpy(orig, b, n);
  /* the instance-free describe on the same bytes */
  memset(&drep, 0xA5, sizeof drep);
  drep.r.struct_size = FZ_REP;
  dst = ivs_snapshot_describe(b, n, &drep.r);
  /* restore into the baseline instance */
  fz_restore(b, n, &a);
  if (a.st == IVS_OK) {
    fz_require(dst == IVS_OK);
    fz_accepted(b, n);
    fz_run();
  } else {
    fz_require(a.reason >= 58u && a.reason <= 81u && fz_status[a.reason - 58u] == a.st);
    fz_require(memcmp(fz_inst.storage, fz_baseline, IVS_MACHINE_BYTES) == 0);
    if (dst != IVS_OK) {
      fz_require(a.reason == drep.r.reason && a.st == dst && a.off == drep.r.byte_offset && a.sec == drep.r.section_index);
    } else {
      fz_require(a.reason >= 72u && a.reason <= 80u);
    }
  }
  /* a second attempt on the instance put back gives the identical result */
  memcpy(fz_inst.storage, fz_baseline, IVS_MACHINE_BYTES);
  fz_restore(b, n, &c);
  fz_require(c.st == a.st && c.reason == a.reason && c.off == a.off && c.sec == a.sec);
  memcpy(fz_inst.storage, fz_baseline, IVS_MACHINE_BYTES);
  fz_require(memcmp(b, orig, n) == 0); /* no call changed the supplied bytes */
  free(orig);
  free(b);
  return 0;
}

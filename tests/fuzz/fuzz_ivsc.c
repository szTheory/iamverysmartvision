/* SPDX-License-Identifier: MIT */
/* Optional libFuzzer target for the IVSC v1 layer-1 parser (D-34, plan 03-08; threats T-03-25,
 * T-03-26). Never part of the gate, never registered with ctest, never built by a preset: the
 * target exists only when the cache option IVS_ENABLE_LIBFUZZER is ON (default OFF). The gate's
 * fuzz evidence is the seeded family ivsc.fuzz.* of suite ivsc; this target is a deeper,
 * coverage-guided local run.
 *
 * Recipe (a scratch build directory outside the presets; Apple clang has no libFuzzer runtime,
 * so use a Homebrew LLVM clang, `$(brew --prefix llvm)/bin/clang` or a versioned formula such
 * as llvm@21, or a Linux clang):
 *   cmake -S . -B build/fuzz -G Ninja -DCMAKE_BUILD_TYPE=Debug \
 *     -DCMAKE_C_COMPILER=<that clang> -DIVS_ENABLE_LIBFUZZER=ON
 *   cmake --build build/fuzz --target ivs_fuzz_ivsc
 *   mkdir -p build/fuzz/corpus && build/fuzz/tests/ivs_fuzz_ivsc build/fuzz/corpus -max_total_time=60
 * The corpus and any crash files stay in the build tree (build/ is ignored); a crash input is
 * container bytes, so never commit one (.ivsc is binary by name and ignored). Where the LLVM
 * AddressSanitizer runtime hangs at start-up (seen on a macOS 26 host with Homebrew LLVM 18 and
 * 21, 2026-11-11), add -DIVS_LIBFUZZER_SANITIZERS=fuzzer,undefined; the dev preset's seeded
 * family still runs the parser under ASan.
 *
 * Per input: the bytes are copied into a heap buffer of exactly their size and, when at least
 * 32 bytes long, the last 32 are resealed with the core SHA-256 so mutations reach the checks
 * after the digest. The copy is parsed into a poisoned view and report, and the same invariants
 * as the seeded family are checked: a defined status whose reason maps to it, a refusal writes
 * only the report, an acceptance writes only the view and re-encodes canonically to the input,
 * and a second parse gives the identical result. A broken invariant aborts, which libFuzzer
 * reports as a crash with the input saved. */
#include <stddef.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>

#include "common/ivs_sha256.h"
#include "content/ivsc.h"
#include "ivsc_fuzz.h"

int LLVMFuzzerTestOneInput(const uint8_t *data, size_t size);

/* The status each reason maps to (spec, "Refusal reasons"); index 0 (none) is unused. */
static const ivs_status fz_reason_status[IVS_IVSC_REASON_COUNT] = {
    IVS_OK,          IVS_E_MALFORMED, IVS_E_OVERSIZED, IVS_E_MALFORMED,       IVS_E_MALFORMED,
    IVS_E_MALFORMED, IVS_E_MALFORMED, IVS_E_MALFORMED, IVS_E_MALFORMED,       IVS_E_MALFORMED,
    IVS_E_MALFORMED, IVS_E_MALFORMED, IVS_E_MALFORMED, IVS_E_MALFORMED,       IVS_E_OUT_OF_RANGE,
    IVS_E_MALFORMED, IVS_E_OVERLAP,   IVS_E_DIGEST_MISMATCH, IVS_E_MALFORMED};

static void fz_require(int cond)
{
  if (!cond) {
    abort();
  }
}

int LLVMFuzzerTestOneInput(const uint8_t *data, size_t size)
{
  uint8_t *b = NULL;
  ivs_ivsc_view v;
  ivs_ivsc_view v2;
  ivs_ivsc_view poison_v;
  ivs_ivsc_report r;
  ivs_ivsc_report r2;
  ivs_ivsc_report poison_r;
  ivs_status st;
  ivs_status st2;

  if (size > IVS_IVSC_MAX_BYTES) {
    return 0; /* the oversize refusal reads no byte; case ivsc.B08 covers it */
  }
  if (size > 0u) {
    b = malloc(size);
    fz_require(b != NULL);
    memcpy(b, data, size);
    if (size >= IVS_IVSC_TRAILER_BYTES) {
      ivs_sha256_digest(b, size - IVS_IVSC_TRAILER_BYTES, b + size - IVS_IVSC_TRAILER_BYTES);
    }
  }
  memset(&poison_v, 0xA5, sizeof poison_v);
  memset(&poison_r, 0x5A, sizeof poison_r);
  v = poison_v;
  r = poison_r;
  v2 = poison_v;
  r2 = poison_r;
  st = ivs_ivsc_parse(b, size, &v, &r);
  st2 = ivs_ivsc_parse(b, size, &v2, &r2);
  fz_require(st == st2 && memcmp(&v, &v2, sizeof v) == 0 && memcmp(&r, &r2, sizeof r) == 0);
  if (st == IVS_OK) {
    fz_require(memcmp(&r, &poison_r, sizeof r) == 0);
    fz_require(v.total_bytes == size && ivsc_test_reencode_equal(b, size, &v));
  } else {
    fz_require(r.reason > 0u && r.reason < IVS_IVSC_REASON_COUNT && fz_reason_status[r.reason] == st);
    fz_require(memcmp(&v, &poison_v, sizeof v) == 0);
  }
  free(b);
  return 0;
}

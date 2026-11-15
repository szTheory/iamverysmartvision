/* SPDX-License-Identifier: MIT */
/* ivs-bench host services (plan 08-18, D-21): the only file that includes a POSIX header, so the feature macros below precede the first
 * include here and nowhere else (the same rule as runner/hostio.c: strict C17 hides these names on glibc). It exports the monotonic
 * wall clock, the process CPU time and the processor count, and does nothing else: no file, no environment, no signal. The machine
 * core and the runner never see a clock; this tool reads one only after the verification run has matched the committed golden. */
#define _POSIX_C_SOURCE 200809L
#define _DEFAULT_SOURCE 1
#ifdef __APPLE__
/* Apple libc hides _SC_NPROCESSORS_ONLN under a strict _POSIX_C_SOURCE; this file is the one place that may say so. */
#define _DARWIN_C_SOURCE 1
#endif

#include <stdint.h>
#include <time.h>
#include <unistd.h>

#include "bench_host.h"

static int bench_clock(clockid_t id, uint64_t *ns)
{
  struct timespec ts;
  if (clock_gettime(id, &ts) != 0 || ts.tv_sec < 0 || ts.tv_nsec < 0) {
    return -1;
  }
  *ns = (uint64_t)ts.tv_sec * UINT64_C(1000000000) + (uint64_t)ts.tv_nsec;
  return 0;
}

int bench_now_ns(uint64_t *ns) { return bench_clock(CLOCK_MONOTONIC, ns); }

int bench_cpu_ns(uint64_t *ns)
{
#ifdef CLOCK_PROCESS_CPUTIME_ID
  return bench_clock(CLOCK_PROCESS_CPUTIME_ID, ns);
#else
  (void)ns;
  return -1;
#endif
}

uint32_t bench_ncpu(void)
{
  long n = sysconf(_SC_NPROCESSORS_ONLN);
  return n > 0 && n < 1000000L ? (uint32_t)n : 0u;
}

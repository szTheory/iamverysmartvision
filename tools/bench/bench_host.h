/* SPDX-License-Identifier: MIT */
/* ivs-bench host services (plan 08-18, D-21): the three calls of the tool that reach the host. bench_host.c is the only file of the
 * repository that reads the monotonic clock and the process CPU time for a measurement; bench.c is ISO C and calls the two time
 * functions only inside the timed loop, after the digest of the run has been proved equal to the committed golden. Nothing here is
 * linked into the core, the runner or ivs-run. */
#ifndef IVS_BENCH_HOST_H
#define IVS_BENCH_HOST_H

#include <stdint.h>

/* Monotonic wall time in nanoseconds. Returns 0 and writes *ns, or nonzero when the clock is unavailable. */
int bench_now_ns(uint64_t *ns);

/* User plus system CPU time of this process in nanoseconds. Returns 0 and writes *ns, or nonzero when it is unavailable. */
int bench_cpu_ns(uint64_t *ns);

/* The number of processors online, 0 when the host does not say. */
uint32_t bench_ncpu(void);

#endif

/* SPDX-License-Identifier: MIT */
/* Host services of the ivs-run executable (D-17, D-21, D-22, plan 04-19), implemented in
 * runner/hostio.c, the only runner file that uses POSIX (runner-scan). Not part of the runner
 * library: main.c wraps these calls in the ivs_runner_host vtable. */
#ifndef IVS_RUNNER_HOSTIO_H
#define IVS_RUNNER_HOSTIO_H

#include <stddef.h>
#include <stdint.h>

/* Installs, with signal() and BSD semantics (the handler stays installed after a signal, and an
 * interrupted system call restarts; hostio.c requests them on glibc with _DEFAULT_SOURCE), a
 * handler for SIGINT and SIGTERM that only sets the cancel flag, so a second signal is also just
 * a cancel request; and ignores SIGPIPE (a closed stdout is then a checked write error). Returns
 * 0, or -1 when a handler could not be installed. */
int ivs_hostio_install_signals(void);

/* Nonzero once SIGINT or SIGTERM arrived. */
int ivs_hostio_cancelled(void);

/* The host's monotonic time in milliseconds (an arbitrary origin; only differences mean
 * anything); 0 when the host cannot read it, so a broken source never ends a run by itself. */
uint64_t ivs_hostio_now_ms(void);

/* Reads the regular file dir/rel (rel alone when dir is NULL) into buf: opens it without
 * blocking, checks with fstat on the open descriptor that it is a regular file, then reads at
 * most cap + 1 bytes (buf must hold cap + 1). Returns an IVS_RUNNER_READ_* result: OK with *len
 * at most cap, TOO_LARGE with *len = cap + 1, MISSING (no such file or directory),
 * NOT_REGULAR, or IO_ERROR (any other failure, including a joined path over the host limit). */
int ivs_hostio_read(const char *dir, const char *rel, size_t cap, uint8_t *buf, size_t *len);

#endif

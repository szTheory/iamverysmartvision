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

/* An operator output file (plan 05-09, D-20): created exclusively and written only here. The struct is
 * plain data so main.c holds one without including POSIX headers. */
typedef struct ivs_hostio_out {
  int fd;       /* -1 when not open */
  uint64_t dev; /* the identity of the file this run created (st_dev, st_ino), set by the create calls; */
  uint64_t ino; /* it survives the close, so a later removal can tell the file from a replacement */
} ivs_hostio_out;

/* The creation modes (before the umask) of an operator output: the audio outputs keep 0644; the video outputs of plan 06-12
 * are created 0600 (decided deliberately for those two files, review residual IN-05; a frame or a raw stream can be derived
 * from content the operator owns). Octal values as plain numbers so main.c needs no POSIX header. */
#define IVS_HOSTIO_MODE_SHARED 0644u
#define IVS_HOSTIO_MODE_PRIVATE 0600u

/* Creates path for writing with O_CREAT | O_EXCL (the given mode before the umask): an existing path of any
 * kind, a dangling symbolic link included, is refused and never overwritten or truncated. Returns 0
 * (o is open), 1 when the path exists, -1 on any other failure (o is then closed). */
int ivs_hostio_out_create(ivs_hostio_out *o, const char *path, unsigned int mode);

/* The same exclusive create, but opened for reading and writing as well (plan 05-10, the operator WAV: the
 * header is written in front of the PCM after the stream is complete, and the finished file is read back
 * to hash it). Same results as ivs_hostio_out_create. */
int ivs_hostio_out_create_rw(ivs_hostio_out *o, const char *path, unsigned int mode);

/* Writes all len bytes (retrying on EINTR and short writes). Returns 0, or -1 on an error. */
int ivs_hostio_out_write(ivs_hostio_out *o, const void *bytes, size_t len);

/* Writes all len bytes at the absolute offset (the file position is unchanged) and reads up to cap bytes
 * from the absolute offset (*got gets the count, 0 at the end of the file); retry on EINTR. Return 0, or -1
 * on an error or an offset beyond the signed 63-bit range. Only for a file made by ivs_hostio_out_create_rw. */
int ivs_hostio_out_write_at(ivs_hostio_out *o, uint64_t offset, const void *bytes, size_t len);
int ivs_hostio_out_read_at(ivs_hostio_out *o, uint64_t offset, void *buf, size_t cap, size_t *got);

/* Closes o (idempotent on a closed one). Returns 0, or -1 when the close failed. */
int ivs_hostio_out_close(ivs_hostio_out *o);

/* Removes path when it is still the regular file that o created (lstat of the path equals the identity
 * recorded at creation; o may be open or closed), so a file replaced or linked in meanwhile is never
 * removed. Returns 0 (removed), 1 (the path no longer names that file: left alone) or -1 (the identity
 * is unknown or unlink failed). Used for the files a failed run created (review WR-01). */
int ivs_hostio_out_remove_if_same(const ivs_hostio_out *o, const char *path);

/* 1 when path (symbolic links followed, as fopen would) names the file o created, 0 when it does not or
 * does not exist, -1 when o has no recorded identity. Review WR-02: --update-golden must not open an
 * operator output file under another spelling. */
int ivs_hostio_out_is_path(const ivs_hostio_out *o, const char *path);

/* Reads the regular file dir/rel (rel alone when dir is NULL) into buf: opens it without
 * blocking, checks with fstat on the open descriptor that it is a regular file, then reads at
 * most cap + 1 bytes (buf must hold cap + 1). Returns an IVS_RUNNER_READ_* result: OK with *len
 * at most cap, TOO_LARGE with *len = cap + 1, MISSING (no such file or directory),
 * NOT_REGULAR, or IO_ERROR (any other failure, including a joined path over the host limit). */
int ivs_hostio_read(const char *dir, const char *rel, size_t cap, uint8_t *buf, size_t *len);

#endif

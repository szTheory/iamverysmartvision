/* SPDX-License-Identifier: MIT */
/* Host services of ivs-run (D-17, D-21, D-22, D-23, plan 04-19): the only runner file that uses
 * POSIX, so the feature macros below precede the first include here and nowhere else (strict C17
 * hides these names on glibc otherwise; runner-scan refuses the macros in any other runner file).
 * _DEFAULT_SOURCE gives signal() BSD semantics on glibc (review WR-04): without it, under strict
 * -std=c17, glibc maps signal() to the System V form (SA_RESETHAND | SA_NODEFER, no SA_RESTART),
 * so a second SIGINT killed the process and a signal during a blocked stdout write failed it
 * with EINTR. With it the handler stays installed and interrupted calls restart, as on Apple
 * libc and the BSDs; signal() stays (D-23).
 * Holds: the cancel flag (the program's only global), set by a signal() handler that does nothing
 * else; the monotonic timer; bounded reads of regular files; and, since plan 05-09, the operator's
 * raw output files (--audio-raw, --psg-log-out) and the operator WAV (--audio-wav, plan 05-10), created exclusively with O_CREAT | O_EXCL so an
 * existing path is refused and never overwritten. Never reads the environment; the --update-golden
 * target is the one other file the program writes, and main.c does that with ISO stdio. */
#define _POSIX_C_SOURCE 200809L
#define _DEFAULT_SOURCE 1

#include <errno.h>
#include <fcntl.h>
#include <signal.h>
#include <stddef.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <time.h>
#include <unistd.h>

#include "hostio.h"
#include "ivs_runner.h"

/* Longest joined path the runner builds (dir + '/' + rel + NUL). */
#define HOSTIO_PATH_MAX 4096u

static volatile sig_atomic_t hostio_cancel_flag = 0;

static void hostio_on_signal(int sig)
{
  (void)sig;
  hostio_cancel_flag = 1;
}

int ivs_hostio_install_signals(void)
{
  if (signal(SIGINT, hostio_on_signal) == SIG_ERR) {
    return -1;
  }
  if (signal(SIGTERM, hostio_on_signal) == SIG_ERR) {
    return -1;
  }
  if (signal(SIGPIPE, SIG_IGN) == SIG_ERR) {
    return -1;
  }
  return 0;
}

int ivs_hostio_cancelled(void) { return hostio_cancel_flag != 0; }

uint64_t ivs_hostio_now_ms(void)
{
  struct timespec ts;
  if (clock_gettime(CLOCK_MONOTONIC, &ts) != 0 || ts.tv_sec < 0 || ts.tv_nsec < 0) {
    return 0u;
  }
  return (uint64_t)ts.tv_sec * 1000u + (uint64_t)ts.tv_nsec / 1000000u;
}

int ivs_hostio_read(const char *dir, const char *rel, size_t cap, uint8_t *buf, size_t *len)
{
  char path[HOSTIO_PATH_MAX];
  const char *p = rel;
  struct stat sb;
  size_t want;
  size_t got = 0u;
  int fd;
  *len = 0u;
  if (rel == NULL || buf == NULL || cap >= (size_t)-1) {
    return IVS_RUNNER_READ_IO_ERROR;
  }
  if (dir != NULL) {
    size_t dl = strlen(dir);
    size_t rl = strlen(rel);
    if (dl == 0u || dl + 1u + rl + 1u > sizeof path) {
      return IVS_RUNNER_READ_IO_ERROR;
    }
    memcpy(path, dir, dl);
    path[dl] = '/';
    memcpy(path + dl + 1u, rel, rl + 1u);
    p = path;
  }
  /* O_NONBLOCK: opening a FIFO must not wait for a writer; fstat then refuses it. */
  fd = open(p, O_RDONLY | O_NOCTTY | O_NONBLOCK);
  if (fd < 0) {
    return (errno == ENOENT || errno == ENOTDIR) ? IVS_RUNNER_READ_MISSING : IVS_RUNNER_READ_IO_ERROR;
  }
  if (fstat(fd, &sb) != 0) {
    (void)close(fd);
    return IVS_RUNNER_READ_IO_ERROR;
  }
  if (!S_ISREG(sb.st_mode)) {
    (void)close(fd);
    return IVS_RUNNER_READ_NOT_REGULAR;
  }
  want = cap + 1u;
  while (got < want) {
    ssize_t n = read(fd, buf + got, want - got);
    if (n < 0) {
      if (errno == EINTR) {
        continue;
      }
      (void)close(fd);
      return IVS_RUNNER_READ_IO_ERROR;
    }
    if (n == 0) {
      break;
    }
    got += (size_t)n;
  }
  if (close(fd) != 0) {
    return IVS_RUNNER_READ_IO_ERROR;
  }
  *len = got;
  return got > cap ? IVS_RUNNER_READ_TOO_LARGE : IVS_RUNNER_READ_OK;
}

static int hostio_out_create(ivs_hostio_out *o, const char *path, int access, unsigned int mode)
{
  struct stat sb;
  int fd;
  o->fd = -1;
  o->dev = 0u;
  o->ino = 0u;
  if (path == NULL || path[0] == '\0') {
    return -1;
  }
  fd = open(path, access | O_CREAT | O_EXCL | O_NOCTTY, (mode_t)mode);
  if (fd < 0) {
    return errno == EEXIST ? 1 : -1;
  }
  if (fstat(fd, &sb) != 0 || !S_ISREG(sb.st_mode)) {
    /* The file was just created by this call, so it is removed again rather than left unidentified. */
    (void)close(fd);
    (void)unlink(path);
    return -1;
  }
  o->fd = fd;
  o->dev = (uint64_t)sb.st_dev;
  o->ino = (uint64_t)sb.st_ino;
  return 0;
}

int ivs_hostio_out_create(ivs_hostio_out *o, const char *path, unsigned int mode)
{
  return hostio_out_create(o, path, O_WRONLY, mode);
}

int ivs_hostio_out_create_rw(ivs_hostio_out *o, const char *path, unsigned int mode)
{
  return hostio_out_create(o, path, O_RDWR, mode);
}

int ivs_hostio_out_write_at(ivs_hostio_out *o, uint64_t offset, const void *bytes, size_t len)
{
  const uint8_t *p = bytes;
  size_t done = 0u;
  if (o->fd < 0 || offset > (uint64_t)INT64_MAX || (uint64_t)len > (uint64_t)INT64_MAX - offset) {
    return -1;
  }
  while (done < len) {
    ssize_t n = pwrite(o->fd, p + done, len - done, (off_t)(offset + (uint64_t)done));
    if (n < 0) {
      if (errno == EINTR) {
        continue;
      }
      return -1;
    }
    done += (size_t)n;
  }
  return 0;
}

int ivs_hostio_out_read_at(ivs_hostio_out *o, uint64_t offset, void *buf, size_t cap, size_t *got)
{
  *got = 0u;
  if (o->fd < 0 || offset > (uint64_t)INT64_MAX) {
    return -1;
  }
  for (;;) {
    ssize_t n = pread(o->fd, buf, cap, (off_t)offset);
    if (n < 0) {
      if (errno == EINTR) {
        continue;
      }
      return -1;
    }
    *got = (size_t)n;
    return 0;
  }
}

int ivs_hostio_out_write(ivs_hostio_out *o, const void *bytes, size_t len)
{
  const uint8_t *p = bytes;
  size_t done = 0u;
  if (o->fd < 0) {
    return -1;
  }
  while (done < len) {
    ssize_t n = write(o->fd, p + done, len - done);
    if (n < 0) {
      if (errno == EINTR) {
        continue;
      }
      return -1;
    }
    done += (size_t)n;
  }
  return 0;
}

int ivs_hostio_out_close(ivs_hostio_out *o)
{
  int fd = o->fd;
  o->fd = -1;
  if (fd < 0) {
    return 0;
  }
  return close(fd) == 0 ? 0 : -1;
}

int ivs_hostio_out_remove_if_same(const ivs_hostio_out *o, const char *path)
{
  struct stat sb;
  if (o->ino == 0u || path == NULL) {
    return -1;
  }
  if (lstat(path, &sb) != 0) {
    return errno == ENOENT ? 1 : -1;
  }
  if (!S_ISREG(sb.st_mode) || (uint64_t)sb.st_dev != o->dev || (uint64_t)sb.st_ino != o->ino) {
    return 1;
  }
  return unlink(path) == 0 ? 0 : -1;
}

int ivs_hostio_out_is_path(const ivs_hostio_out *o, const char *path)
{
  struct stat sb;
  if (o->ino == 0u || path == NULL) {
    return -1;
  }
  if (stat(path, &sb) != 0) {
    return 0;
  }
  return (uint64_t)sb.st_dev == o->dev && (uint64_t)sb.st_ino == o->ino ? 1 : 0;
}

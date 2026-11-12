#!/bin/sh
# SPDX-License-Identifier: MIT
# Suite headers (D-05, D-06, plan 03-12): every public header core/include/ivs/<h>.h stands
# alone. An optional second argument names another include directory (holding ivs/*.h) to check
# the same way; the gate step pkg-install passes the installed prefix's include/ (D-38, plan
# 04-22). The ctest registration passes only the source root. For each header a one-line TU
# `#include "ivs/<h>.h"` is compiled
#   - as C17: ${CC:-cc} -std=c17 -pedantic -Wall -Wextra -Werror -fsyntax-only -x c
#     (id headers.c17-<stem>), and
#   - as C++17 where a C++ compiler exists: ${CXX:-c++} -std=c++17 -pedantic -Wall -Wextra
#     -Werror -fsyntax-only -x c++ (id headers.cxx-<stem>); without one each C++ case is a
#     visible skip (`ok N <id> # SKIP no C++ compiler`), never silent, and its id is skip-ok in
#     data/expected/tests/headers.ids.
# <stem> is the header's file name without .h, with `_` written as `-`. Output: the harness TAP
# shape (header with IVS_TEST_NONCE or none, plan, one result per case with the compiler output
# as `#` lines after a failure, trailer) on stdout and, when IVS_TEST_OUT_DIR is set, in
# <dir>/headers.txt.
# Usage: sh headers-tap.sh <source root> [<include dir>]   Exit: 0 pass, 1 fail, 77 every case skipped.
# Env: CC (set by ctest from CMAKE_C_COMPILER), CXX, IVS_TEST_NONCE, IVS_TEST_OUT_DIR.
LC_ALL=C; export LC_ALL
[ "$#" -eq 1 ] || [ "$#" -eq 2 ] || { printf 'usage: sh headers-tap.sh <source root> [<include dir>]\n' >&2; exit 1; }
root=$1
inc=${2:-"$root/core/include"}
[ -d "$inc/ivs" ] || { printf 'headers-tap: no %s\n' "$inc/ivs" >&2; exit 1; }
tmp=$(mktemp -d "${TMPDIR:-/tmp}/headers-tap.XXXXXX") || exit 1
trap 'rm -rf "$tmp"' EXIT
trap 'exit 1' HUP INT TERM

suite=headers
nonce=${IVS_TEST_NONCE:-none}
cc=${CC:-cc}
cxx=${CXX:-c++}
have_cxx=0
if command -v "$cxx" > /dev/null 2>&1; then have_cxx=1; fi

: > "$tmp/results"
n=0
executed=0
skipped=0
failed=0
for h in "$inc"/ivs/*.h; do
  [ -f "$h" ] || continue
  name=$(basename "$h")
  stem=$(printf '%s\n' "${name%.h}" | tr '_' '-')
  printf '#include "ivs/%s"\n' "$name" > "$tmp/tu"
  # C17
  n=$((n + 1))
  id="headers.c17-$stem"
  if "$cc" -std=c17 -pedantic -Wall -Wextra -Werror -fsyntax-only -x c -I "$inc" "$tmp/tu" > "$tmp/out" 2>&1; then
    printf 'ok %s %s\n' "$n" "$id" >> "$tmp/results"
    executed=$((executed + 1))
  else
    printf 'not ok %s %s\n' "$n" "$id" >> "$tmp/results"
    sed 's/^/# /' "$tmp/out" >> "$tmp/results"
    executed=$((executed + 1))
    failed=$((failed + 1))
  fi
  # C++17
  n=$((n + 1))
  id="headers.cxx-$stem"
  if [ "$have_cxx" -eq 0 ]; then
    printf 'ok %s %s # SKIP no C++ compiler\n' "$n" "$id" >> "$tmp/results"
    skipped=$((skipped + 1))
  elif "$cxx" -std=c++17 -pedantic -Wall -Wextra -Werror -fsyntax-only -x c++ -I "$inc" "$tmp/tu" > "$tmp/out" 2>&1; then
    printf 'ok %s %s\n' "$n" "$id" >> "$tmp/results"
    executed=$((executed + 1))
  else
    printf 'not ok %s %s\n' "$n" "$id" >> "$tmp/results"
    sed 's/^/# /' "$tmp/out" >> "$tmp/results"
    executed=$((executed + 1))
    failed=$((failed + 1))
  fi
done

{
  printf '# suite=%s nonce=%s\n' "$suite" "$nonce"
  printf '1..%s\n' "$n"
  cat "$tmp/results"
  printf '# suite=%s executed=%s skipped=%s failed=%s\n' "$suite" "$executed" "$skipped" "$failed"
} > "$tmp/tap"
cat "$tmp/tap"
if [ -n "${IVS_TEST_OUT_DIR:-}" ]; then
  cp "$tmp/tap" "$IVS_TEST_OUT_DIR/$suite.txt" || { printf '# cannot write %s\n' "$IVS_TEST_OUT_DIR/$suite.txt" >&2; exit 1; }
fi
[ "$failed" -eq 0 ] || exit 1
[ "$n" -gt 0 ] || exit 1
[ "$executed" -gt 0 ] || exit 77
exit 0

#!/bin/sh
# SPDX-License-Identifier: MIT
# Step integrity: path charset, empty files, CRLF, BOM, ASCII-only records, SPDX lines and
# NUL bytes (CR-01: a NUL byte hides a file from every grep-based scanner, so it is banned in
# every file that is not binary by extension; binary-by-extension files belong to the files step).
# Tokens also: integrity-nul-byte: <file>.
LC_ALL=C; export LC_ALL
: "${ROOT:?ROOT is not set}"
: "${CHECK_DIR:?CHECK_DIR is not set}"
: "${FILELIST:?FILELIST is not set}"
tmp=$(mktemp -d "${TMPDIR:-/tmp}/integrity.XXXXXX") || exit 1
trap 'rm -rf "$tmp"' EXIT
trap 'exit 1' HUP INT TERM
rc=0
cmake -DROOT="$ROOT" -DFILELIST="$FILELIST" -P "$CHECK_DIR/integrity.cmake" > "$tmp/cmake.out" 2>&1 || rc=1
if ! sh "$CHECK_DIR/binscan.sh" nul > "$tmp/nul.out" 2> "$tmp/nul.err"; then
  printf -- '-- FAIL integrity-scan-error: binscan\n'
  cat "$tmp/nul.err"
  rc=1
fi
if [ -s "$tmp/nul.out" ]; then
  rc=1
  # The pass line must not appear next to a failure.
  grep -v -e '^-- integrity pass ' "$tmp/cmake.out"
  while IFS= read -r p; do
    printf -- '-- FAIL integrity-nul-byte: %s\n' "$p"
  done < "$tmp/nul.out"
else
  cat "$tmp/cmake.out"
fi
[ "$rc" -eq 0 ]

#!/bin/sh
# SPDX-License-Identifier: MIT
# Step files: tracked-file rights check (refs/, .env, unlisted binaries, unlisted fixture files).
# Binary is decided by name (tools/check/binary-ext.txt) and by content (binscan.sh binary: NUL
# byte or mostly non-text bytes), so an extensionless or archive-named binary is caught too.
# Tokens also: files-scan-error.
LC_ALL=C; export LC_ALL
: "${ROOT:?ROOT is not set}"
: "${CHECK_DIR:?CHECK_DIR is not set}"
: "${FILELIST:?FILELIST is not set}"
tmp=$(mktemp -d "${TMPDIR:-/tmp}/files.XXXXXX") || exit 1
trap 'rm -rf "$tmp"' EXIT
trap 'exit 1' HUP INT TERM
rc=0
if ! sh "$CHECK_DIR/binscan.sh" binary > "$tmp/contentbin" 2> "$tmp/scan.err"; then
  printf -- '-- FAIL files-scan-error: binscan\n'
  cat "$tmp/scan.err"
  rc=1
fi
cmake -DROOT="$ROOT" -DFILELIST="$FILELIST" -DCONTENT_BINARY="$tmp/contentbin" -P "$CHECK_DIR/files.cmake" > "$tmp/cmake.out" 2>&1 || rc=1
if [ "$rc" -ne 0 ]; then grep -v -e '^-- files pass ' "$tmp/cmake.out"; else cat "$tmp/cmake.out"; fi
[ "$rc" -eq 0 ]

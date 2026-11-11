#!/bin/sh
# SPDX-License-Identifier: MIT
# Step cp1610-vectors: structural check of the CP-1610 vector data, its inventories and its
# expected skips, plus the coverage derived from the opcode table (tools/check/vectors.cmake;
# D-14, D-16, A-05, A-07, A-13). Plan 02-16 adds, on every run, the blinded projection of every
# vector file into this step's temp dir (tools/review/vector-blind.cmake, read from CHECK_DIR so
# a control cannot replace it; nothing is written under build/review) and, when
# data/expected/vector-review.ids lists a stem, the second-read marker record (kind vector-review).
# Tokens: vector-* (listed in vectors.cmake), review-blind-* (vector-blind.cmake),
# vector-review-* (kinds/vector-review.cmake). Stats: vectors=<n> source=<n> policy=<n>
# rows-pending=<n> policies-pending=<n> bus-figure-pages=<n> blind-files=<n>
# second-read=<listed stems>/<vector files>.
LC_ALL=C; export LC_ALL
: "${ROOT:?ROOT is not set}"
: "${CHECK_DIR:?CHECK_DIR is not set}"
: "${FILELIST:?FILELIST is not set}"
tmp=$(mktemp -d "${TMPDIR:-/tmp}/vectors.XXXXXX") || exit 1
trap 'rm -rf "$tmp"' EXIT
trap 'exit 1' HUP INT TERM
rc=0
out=$(cmake -DROOT="$ROOT" -DFILELIST="$FILELIST" -DCHECK_DIR="$CHECK_DIR" -P "$CHECK_DIR/vectors.cmake" 2>&1) || rc=1
printf '%s\n' "$out"
stats=$(printf '%s\n' "$out" | sed -n 's/^-- vectors pass //p')

out=$(cmake -DROOT="$ROOT" -DOUT_DIR="$tmp/blind" -P "$CHECK_DIR/../review/vector-blind.cmake" 2>&1) || rc=1
printf '%s\n' "$out" | grep -v '^blind '
blind=$(printf '%s\n' "$out" | grep -c '^blind ')

listed=0
if [ -f "$ROOT/data/expected/vector-review.ids" ]; then
  listed=$(awk '/^#/ { next } NF { n++ } END { print n + 0 }' "$ROOT/data/expected/vector-review.ids")
fi
files=0
if [ -f "$ROOT/data/expected/cp1610-vector-files.ids" ]; then
  files=$(awk '/^#/ { next } NF { n++ } END { print n + 0 }' "$ROOT/data/expected/cp1610-vector-files.ids")
fi
if [ "$listed" -gt 0 ]; then
  out=$(cmake -DROOT="$ROOT" -DKIND=vector-review -P "$CHECK_DIR/records.cmake" 2>&1) || rc=1
  printf '%s\n' "$out"
fi

[ "$rc" -eq 0 ] || exit 1
[ -n "$stats" ] || exit 1
printf -- '-- cp1610-vectors pass %s blind-files=%s second-read=%s/%s\n' "$stats" "$blind" "$listed" "$files"

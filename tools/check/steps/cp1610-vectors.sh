#!/bin/sh
# SPDX-License-Identifier: MIT
# Step cp1610-vectors: structural check of the CP-1610 vector data, its inventories and its
# expected skips, plus the coverage derived from the opcode table (tools/check/vectors.cmake;
# D-14, D-16, A-05, A-07, A-13). Plan 02-16 adds the review record and the second-read stat.
# Tokens: vector-* (listed in vectors.cmake). Stats: vectors=<n> source=<n> policy=<n>
# rows-pending=<n> policies-pending=<n> bus-figure-pages=<n>.
LC_ALL=C; export LC_ALL
: "${ROOT:?ROOT is not set}"
: "${CHECK_DIR:?CHECK_DIR is not set}"
: "${FILELIST:?FILELIST is not set}"
rc=0
out=$(cmake -DROOT="$ROOT" -DFILELIST="$FILELIST" -DCHECK_DIR="$CHECK_DIR" -P "$CHECK_DIR/vectors.cmake" 2>&1) || rc=1
printf '%s\n' "$out"
[ "$rc" -eq 0 ] || exit 1
stats=$(printf '%s\n' "$out" | sed -n 's/^-- vectors pass //p')
[ -n "$stats" ] || exit 1
printf -- '-- cp1610-vectors pass %s\n' "$stats"

#!/bin/sh
# SPDX-License-Identifier: MIT
# Step cp1610-vectors: structural check of the CP-1610 vector data, its inventories and its
# expected skips (tools/check/vectors.cmake; D-14, A-07). Plan 02-05 adds coverage derivation,
# plan 02-16 the review record and the second-read stat.
# Tokens: vector-* (listed in vectors.cmake). Stats: vectors=<n> source=<n> policy=<n>.
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

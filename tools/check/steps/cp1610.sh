#!/bin/sh
# SPDX-License-Identifier: MIT
# Step cp1610: the CP-1610 family table (record kind cp1610-row), its 1024-pattern decode
# coverage (coverage.cmake) and the notes-to-log C17 status consistency (c17-consistency.cmake).
# All three checks run every time; the step fails if any of them failed.
# Tokens: cp1610-row-*, coverage-*, cp1610-c17-missing, cp1610-c17-inconsistent.
# Stats: cp1610-rows=<n> patterns=<n> unclaimed=<n> ambiguous=<n> coincidences=<n>.
LC_ALL=C; export LC_ALL
: "${ROOT:?ROOT is not set}"
: "${CHECK_DIR:?CHECK_DIR is not set}"
rc=0
out=$(cmake -DROOT="$ROOT" -DKIND=cp1610-row -P "$CHECK_DIR/records.cmake" 2>&1) || rc=1
printf '%s\n' "$out"
rows=$(printf '%s\n' "$out" | sed -n 's/^-- cp1610-row pass records=[0-9][0-9]* //p')
out=$(cmake -DROOT="$ROOT" -P "$CHECK_DIR/coverage.cmake" 2>&1) || rc=1
printf '%s\n' "$out"
cov=$(printf '%s\n' "$out" | sed -n 's/^-- coverage pass //p')
out=$(cmake -DROOT="$ROOT" -P "$CHECK_DIR/c17-consistency.cmake" 2>&1) || rc=1
printf '%s\n' "$out"
[ "$rc" -eq 0 ] || exit 1
[ -n "$rows" ] && [ -n "$cov" ] || exit 1
printf -- '-- cp1610 pass %s %s\n' "$rows" "$cov"

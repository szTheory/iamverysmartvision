#!/bin/sh
# SPDX-License-Identifier: MIT
# Step discrepancies: the discrepancy log (kind discrepancy).
LC_ALL=C; export LC_ALL
: "${ROOT:?ROOT is not set}"
: "${CHECK_DIR:?CHECK_DIR is not set}"
out=$(cmake -DROOT="$ROOT" -DKIND=discrepancy -P "$CHECK_DIR/records.cmake" 2>&1) || rc=1
printf '%s\n' "$out"
[ "${rc:-0}" -eq 0 ] || exit 1
stats=$(printf '%s\n' "$out" | sed -n 's/^-- discrepancy pass records=[0-9][0-9]* //p')
[ -n "$stats" ] || exit 1
printf -- '-- discrepancies pass %s\n' "$stats"

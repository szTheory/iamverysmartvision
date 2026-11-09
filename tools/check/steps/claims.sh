#!/bin/sh
# SPDX-License-Identifier: MIT
# Step claims: the claim ledger (kind claim).
LC_ALL=C; export LC_ALL
: "${ROOT:?ROOT is not set}"
: "${CHECK_DIR:?CHECK_DIR is not set}"
rc=0
out=$(cmake -DROOT="$ROOT" -DKIND=claim -P "$CHECK_DIR/records.cmake" 2>&1) || rc=1
printf '%s\n' "$out"
[ "$rc" -eq 0 ] || exit 1
stats=$(printf '%s\n' "$out" | sed -n 's/^-- claim pass records=[0-9][0-9]* //p')
[ -n "$stats" ] || exit 1
printf -- '-- claims pass %s\n' "$stats"

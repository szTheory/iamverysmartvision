#!/bin/sh
# SPDX-License-Identifier: MIT
# Step cp1610-reader: the blind second-reader verdicts for the CP-1610 family table (kind
# second-reader). Enforces id parity with data/expected/cp1610-rows.ids, the verdict vocabulary,
# a note on every resolved entry and the c17_reading form.
# Tokens: second-reader-*. Stats: agree=<n> resolved=<n>.
LC_ALL=C; export LC_ALL
: "${ROOT:?ROOT is not set}"
: "${CHECK_DIR:?CHECK_DIR is not set}"
rc=0
out=$(cmake -DROOT="$ROOT" -DKIND=second-reader -P "$CHECK_DIR/records.cmake" 2>&1) || rc=1
printf '%s\n' "$out"
[ "$rc" -eq 0 ] || exit 1
stats=$(printf '%s\n' "$out" | sed -n 's/^-- second-reader pass records=[0-9][0-9]* //p')
[ -n "$stats" ] || exit 1
printf -- '-- cp1610-reader pass %s\n' "$stats"

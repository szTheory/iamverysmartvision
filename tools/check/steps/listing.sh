#!/bin/sh
# SPDX-License-Identifier: MIT
# Step listing: the hand-assembled listings of data/expected/listings.ids (D-26) and the
# anti-HLE guards of the project boot stub (D-29); tools/check/listing.cmake, plan 04-15.
# Static: it reads only ROOT and FILELIST and writes nothing.
# Tokens: listing-bits-mismatch, listing-row-mismatch, listing-operand-count, listing-range,
#   listing-header, listing-unlisted, bootstub-size, bootstub-row, bootstub-vector (rules in
#   listing.cmake).
# Stats: listings=<n> listing-words=<n>.
LC_ALL=C; export LC_ALL
: "${ROOT:?ROOT is not set}"
: "${CHECK_DIR:?CHECK_DIR is not set}"
: "${FILELIST:?FILELIST is not set}"
rc=0
out=$(cmake -DROOT="$ROOT" -DFILELIST="$FILELIST" -DCHECK_DIR="$CHECK_DIR" -P "$CHECK_DIR/listing.cmake" 2>&1) || rc=1
# The checker's own pass line is re-printed below, once.
printf '%s\n' "$out" | grep -v '^-- listing pass '
stats=$(printf '%s\n' "$out" | sed -n 's/^-- listing pass //p')
[ "$rc" -eq 0 ] || exit 1
[ -n "$stats" ] || exit 1
printf -- '-- listing pass %s\n' "$stats"

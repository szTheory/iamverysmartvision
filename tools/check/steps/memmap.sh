#!/bin/sh
# SPDX-License-Identifier: MIT
# Step memmap: the sourced literal memory map of every profile (D-21; tools/check/memmap.cmake).
# Plan 03-05 checks the literal files on their own: one per profile, row grammar, exact tiling of
# 0000-FFFF per view, provenance (source id, locator, tag, discrepancy id) and the tier-5 refusal.
# Plan 03-06 adds the comparison with the runtime decode dump. The step is a pure checker: it
# reads only the tree under test (FILELIST paths) and writes nothing outside its temp dir.
# Tokens: memmap-literal-missing, memmap-literal-orphan, memmap-literal-grammar,
#   memmap-literal-hole, memmap-literal-overlap, memmap-literal-no-cite,
#   memmap-literal-unknown-source, memmap-literal-tier5, memmap-literal-unknown-tag,
#   memmap-literal-unknown-conf (rules in memmap.cmake).
# Stats: literal-rows=<rows over all literal files> views=<2 x profiles>.
LC_ALL=C; export LC_ALL
: "${ROOT:?ROOT is not set}"
: "${CHECK_DIR:?CHECK_DIR is not set}"
: "${FILELIST:?FILELIST is not set}"
rc=0
out=$(cmake -DROOT="$ROOT" -DFILELIST="$FILELIST" -DCHECK_DIR="$CHECK_DIR" -P "$CHECK_DIR/memmap.cmake" 2>&1) || rc=1
# The checker's own pass line is re-printed below, once, with the step's name.
printf '%s\n' "$out" | grep -v '^-- memmap pass '
stats=$(printf '%s\n' "$out" | sed -n 's/^-- memmap pass //p')
[ "$rc" -eq 0 ] || exit 1
[ -n "$stats" ] || exit 1
printf -- '-- memmap pass %s\n' "$stats"

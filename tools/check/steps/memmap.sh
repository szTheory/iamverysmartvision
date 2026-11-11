#!/bin/sh
# SPDX-License-Identifier: MIT
# Step memmap: the sourced literal memory map of every profile (D-21; tools/check/memmap.cmake).
# Plan 03-05 checks the literal files on their own: one per profile, row grammar, exact tiling of
# 0000-FFFF per view, provenance (source id, locator, tag, discrepancy id) and the tier-5 refusal.
# Plan 03-06 adds the comparison with the runtime decode dump: suite memmap writes
# <results>/<preset>/memmap-dump.txt during c-tests (dev and release), and this step compares
# each with the literal's structural columns. The step is a pure checker (H6): it reads only the
# tree under test (FILELIST paths) and the results dir, and writes nothing.
# Results dir: IVS_GATE_DIR (set by tools/gate.sh, and by the control runner to its tree's
# build/gate), else $ROOT/build/gate.
# Tokens: memmap-literal-missing, memmap-literal-orphan, memmap-literal-grammar,
#   memmap-literal-hole, memmap-literal-overlap, memmap-literal-no-cite,
#   memmap-literal-unknown-source, memmap-literal-tier5, memmap-literal-unknown-tag,
#   memmap-literal-unknown-conf, memmap-dump-missing, memmap-dump-stale, memmap-dump-diff
#   (rules in memmap.cmake).
# Stats: literal-rows=<rows over all literal files> views=<2 x profiles>
#   dump-lines=<dump lines read over both presets>.
LC_ALL=C; export LC_ALL
: "${ROOT:?ROOT is not set}"
: "${CHECK_DIR:?CHECK_DIR is not set}"
: "${FILELIST:?FILELIST is not set}"
rc=0
gate_dir=${IVS_GATE_DIR:-$ROOT/build/gate}
out=$(cmake -DROOT="$ROOT" -DFILELIST="$FILELIST" -DCHECK_DIR="$CHECK_DIR" -DGATE_DIR="$gate_dir" \
  -P "$CHECK_DIR/memmap.cmake" 2>&1) || rc=1
# The checker's own pass line is re-printed below, once, with the step's name.
printf '%s\n' "$out" | grep -v '^-- memmap pass '
stats=$(printf '%s\n' "$out" | sed -n 's/^-- memmap pass //p')
[ "$rc" -eq 0 ] || exit 1
[ -n "$stats" ] || exit 1
printf -- '-- memmap pass %s\n' "$stats"

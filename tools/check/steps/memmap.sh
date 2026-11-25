#!/bin/sh
# SPDX-License-Identifier: MIT
# Step memmap: the sourced literal memory map of every profile (D-21; tools/check/memmap.cmake).
# Plan 03-05 checks the literal files on their own: one per profile, row grammar, exact tiling of
# 0000-FFFF per view, provenance (source id, locator, tag, discrepancy id) and the tier-5 refusal.
# Plan 03-06 adds the comparison with the runtime decode dump: suite memmap writes
# <results>/<preset>/memmap-dump.txt during c-tests (dev and release), and this step compares
# each with the literal's structural columns. The step is a pure checker (H6): it reads only the
# tree under test (FILELIST paths) and the results dir, and writes only inside its own temporary
# directory.
# Plan 09-07 (EV-03 item 5): the build's own generator cmake/IvsMemmapProbes.cmake runs FIRST
# against the tree under test, writing its .inc into this step's temporary directory. Its path is
# derived from CHECK_DIR, not from ROOT: the generator is a checker-side artifact driven against
# the tree under test, exactly as memmap.cmake is, and a control tree has no cmake/ directory.
# Its refusal does NOT short-circuit the step's own checker. Six existing controls build trees the
# generator refuses (memmap-literal-grammar, memmap-exception-unknown, memmap-literal-hole,
# memmap-literal-overlap, memmap-literal-missing, memmap-literal-orphan), so a step that returned
# at the refusal would print memmap-generator-refused where each of those pins its own
# memmap-literal-* token. The generator's finding is therefore ADDITIVE: it is printed first, then
# memmap.cmake runs and prints its findings, and the step exits non-zero only after both. A
# literal the generator cannot read is reported with the generator's own reason, and nothing that
# used to be reported stops being reported.
# Results dir: IVS_GATE_DIR (set by tools/gate.sh, and by the control runner to its tree's
# build/gate), else $ROOT/build/gate.
# Tokens: memmap-literal-missing, memmap-literal-orphan, memmap-literal-grammar,
#   memmap-literal-hole, memmap-literal-overlap, memmap-literal-no-cite,
#   memmap-literal-unknown-source, memmap-literal-tier5, memmap-literal-unknown-tag,
#   memmap-literal-unknown-conf, memmap-dump-missing, memmap-dump-stale, memmap-dump-diff
#   (rules in memmap.cmake); memmap-generator-missing (the generator script is not where this step
#   expects it, the liveness guard for the invocation) and memmap-generator-refused: <the
#   generator's own first message line> (the generator refused the tree's literals).
# Stats: literal-rows=<rows over all literal files> views=<2 x profiles>
#   dump-lines=<dump lines read over both presets>.
LC_ALL=C; export LC_ALL
: "${ROOT:?ROOT is not set}"
: "${CHECK_DIR:?CHECK_DIR is not set}"
: "${FILELIST:?FILELIST is not set}"
rc=0
tmp=$(mktemp -d "${TMPDIR:-/tmp}/memmap.XXXXXX") || exit 1
trap 'rm -rf "$tmp"' EXIT
trap 'exit 1' HUP INT TERM

# The generator first (see the header). A missing script is a checker-side setup defect with no
# useful checker run behind it, so the step stops there; a refusal is additive and does not.
gen="$CHECK_DIR/../../cmake/IvsMemmapProbes.cmake"
if [ ! -f "$gen" ]; then
  printf -- '-- FAIL memmap-generator-missing: cmake/IvsMemmapProbes.cmake\n'
  exit 1
fi
grc=0
gout=$(cmake -DROOT="$ROOT" -DOUT="$tmp/ivs_memmap_probes.inc" -P "$gen" 2>&1) || grc=1
if [ "$grc" -ne 0 ]; then
  # The generator's own first message, re-joined: cmake wraps a long message at the terminal width
  # and indents the continuation by two spaces, so a single-line sed would report '... : mirror' and
  # drop the reason the token is meant to carry.
  gdetail=$(printf '%s\n' "$gout" | awk '
    $0 ~ /IvsMemmapProbes:/ { inmsg = 1; sub(/^[ \t]+/, ""); d = $0; next }
    inmsg && $0 ~ /^[ \t][ \t]/ { l = $0; sub(/^[ \t]+/, "", l); d = d " " l; next }
    inmsg { exit }
    END { print d }
  ')
  [ -n "$gdetail" ] || gdetail=$(printf '%s\n' "$gout" | sed -n '1p')
  printf -- '-- FAIL memmap-generator-refused: %s\n' "$gdetail"
  rc=1
fi

gate_dir=${IVS_GATE_DIR:-$ROOT/build/gate}
out=$(cmake -DROOT="$ROOT" -DFILELIST="$FILELIST" -DCHECK_DIR="$CHECK_DIR" -DGATE_DIR="$gate_dir" \
  -P "$CHECK_DIR/memmap.cmake" 2>&1) || rc=1
# The checker's own pass line is re-printed below, once, with the step's name.
printf '%s\n' "$out" | grep -v '^-- memmap pass '
stats=$(printf '%s\n' "$out" | sed -n 's/^-- memmap pass //p')
[ "$rc" -eq 0 ] || exit 1
[ -n "$stats" ] || exit 1
printf -- '-- memmap pass %s\n' "$stats"

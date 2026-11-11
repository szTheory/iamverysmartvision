#!/bin/sh
# SPDX-License-Identifier: MIT
# Step c-tests: for the dev and release presets, rebuild first (a stale binary is never
# tested), run ctest with IVS_TEST_OUT_DIR=<results>/<preset> and the gate nonce, and capture
# `ctest -N` as <results>/<preset>/ctest-list.txt. The tests-inventory step judges the TAP
# files; this step fails only when a build or a ctest run fails (D-17, D-22).
# Results dir: IVS_GATE_DIR (set by tools/gate.sh), else $ROOT/build/gate; the nonce is read
# from <results>/NONCE (created with a direct-run value when absent).
# Tokens: c-tests-failed: <preset>. Stats: ctest-runs=<n>.
LC_ALL=C; export LC_ALL
: "${ROOT:?ROOT is not set}"
: "${CHECK_DIR:?CHECK_DIR is not set}"
: "${FILELIST:?FILELIST is not set}"
tmp=$(mktemp -d "${TMPDIR:-/tmp}/c-tests.XXXXXX") || exit 1
trap 'rm -rf "$tmp"' EXIT
trap 'exit 1' HUP INT TERM
rc=0
bad() { printf -- '-- FAIL %s: %s\n' "$1" "$2"; rc=1; }
cd "$ROOT" || exit 1

gate_dir=${IVS_GATE_DIR:-$ROOT/build/gate}
mkdir -p "$gate_dir" || { bad c-tests-failed "results dir"; exit 1; }
if [ ! -f "$gate_dir/NONCE" ]; then
  printf 'direct.%s\n' "$$" > "$gate_dir/NONCE" || { bad c-tests-failed "NONCE"; exit 1; }
fi
nonce=$(cat "$gate_dir/NONCE")

runs=0
for preset in dev release; do
  out="$gate_dir/$preset"
  rm -rf "$out"
  mkdir -p "$out" || { bad c-tests-failed "$preset"; continue; }
  if ! cmake --build --preset "$preset" > "$tmp/$preset.build.log" 2>&1; then
    cat "$tmp/$preset.build.log"
    bad c-tests-failed "$preset"
    continue
  fi
  if IVS_TEST_OUT_DIR="$out" IVS_TEST_NONCE="$nonce" ctest --preset "$preset" > "$tmp/$preset.ctest.log" 2>&1; then
    runs=$((runs + 1))
  else
    cat "$tmp/$preset.ctest.log"
    bad c-tests-failed "$preset"
  fi
  if ! ctest --preset "$preset" -N > "$out/ctest-list.txt" 2> "$tmp/$preset.list.err"; then
    cat "$tmp/$preset.list.err"
    bad c-tests-failed "$preset (ctest -N)"
  fi
done

[ "$rc" -eq 0 ] || exit 1
printf -- '-- c-tests pass ctest-runs=%s\n' "$runs"

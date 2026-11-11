#!/bin/sh
# SPDX-License-Identifier: MIT
# Step c-tests: for the dev and release presets, rebuild first (a stale binary is never
# tested), run ctest with IVS_TEST_OUT_DIR=<results>/<preset> and the gate nonce, and capture
# `ctest -N` as <results>/<preset>/ctest-list.txt. The tests-inventory step judges the TAP
# files; this step fails only when a build or a ctest run fails (D-17, D-22).
# Results dir: IVS_GATE_DIR (set by tools/gate.sh), else $ROOT/build/gate; the nonce is read
# from <results>/NONCE (created with a direct-run value when absent).
# Parallel ctest (D-45 lever, 04-LEVER-02): each preset is built in turn, and its ctest starts
# in the background as soon as its own build ends, with `-j <ncpu>` (nproc, then
# `sysctl -n hw.ncpu`, then `getconf _NPROCESSORS_ONLN`, else 2), so the two presets' suites
# overlap. The suites share no state: each writes only <results>/<preset>/<suite>.txt (memmap
# also memmap-dump.txt there), the shell suites use their own mktemp dirs and the harness
# self-test uses tmpfile(), so test order and overlap cannot change a result file. Findings
# are reported afterwards in preset order (build log, ctest log, `ctest -N` error), as when the
# presets ran one after the other. A signal stops the background ctest runs before exit.
# Tokens: c-tests-failed: <preset>. Stats: ctest-runs=<n>.
LC_ALL=C; export LC_ALL
: "${ROOT:?ROOT is not set}"
: "${CHECK_DIR:?CHECK_DIR is not set}"
: "${FILELIST:?FILELIST is not set}"
tmp=$(mktemp -d "${TMPDIR:-/tmp}/c-tests.XXXXXX") || exit 1
ctest_pids=
trap 'rm -rf "$tmp"' EXIT
trap 'if [ -n "$ctest_pids" ]; then kill $ctest_pids 2> /dev/null; fi; exit 1' HUP INT TERM
rc=0
bad() { printf -- '-- FAIL %s: %s\n' "$1" "$2"; rc=1; }
cd "$ROOT" || exit 1

gate_dir=${IVS_GATE_DIR:-$ROOT/build/gate}
mkdir -p "$gate_dir" || { bad c-tests-failed "results dir"; exit 1; }
if [ ! -f "$gate_dir/NONCE" ]; then
  printf 'direct.%s\n' "$$" > "$gate_dir/NONCE" || { bad c-tests-failed "NONCE"; exit 1; }
fi
nonce=$(cat "$gate_dir/NONCE")

# Online CPUs, the first probe that prints a positive integer (an absent tool prints nothing).
ncpu=
for probe in nproc 'sysctl -n hw.ncpu' 'getconf _NPROCESSORS_ONLN'; do
  # shellcheck disable=SC2086
  ncpu=$($probe 2> /dev/null) || ncpu=
  case $ncpu in '' | *[!0-9]* | 0 | 0*) ncpu= ;; *) break ;; esac
done
[ -n "$ncpu" ] || ncpu=2

# Build each preset, then start its ctest in the background. <preset>.state: build, mkdir or run.
for preset in dev release; do
  out="$gate_dir/$preset"
  rm -rf "$out"
  if ! mkdir -p "$out"; then
    printf 'mkdir\n' > "$tmp/$preset.state"
    continue
  fi
  if ! cmake --build --preset "$preset" > "$tmp/$preset.build.log" 2>&1; then
    printf 'build\n' > "$tmp/$preset.state"
    continue
  fi
  IVS_TEST_OUT_DIR="$out" IVS_TEST_NONCE="$nonce" ctest --preset "$preset" -j "$ncpu" \
    > "$tmp/$preset.ctest.log" 2>&1 < /dev/null &
  ctest_pids="$ctest_pids $!"
  printf 'run %s\n' "$!" > "$tmp/$preset.state"
done

# Report in preset order.
runs=0
for preset in dev release; do
  out="$gate_dir/$preset"
  read -r state pid < "$tmp/$preset.state"
  case $state in
    mkdir) bad c-tests-failed "$preset"; continue ;;
    build) cat "$tmp/$preset.build.log"; bad c-tests-failed "$preset"; continue ;;
  esac
  if wait "$pid"; then
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
ctest_pids=

[ "$rc" -eq 0 ] || exit 1
printf -- '-- c-tests pass ctest-runs=%s\n' "$runs"

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
# Build lock (Phase 16 cost-only lever): tools/gate.sh runs this step overlapped with c-faults,
# which builds the dev preset in the same tree, so every `cmake --build` call here takes the
# mkdir lock defined in the `build-lock` block below; its reason is a measurement, quoted there.
# Tokens: c-tests-failed: <preset>, c-tests-failed: build-lock (<preset>). Stats: ctest-runs=<n>.
LC_ALL=C; export LC_ALL
: "${ROOT:?ROOT is not set}"
: "${CHECK_DIR:?CHECK_DIR is not set}"
: "${FILELIST:?FILELIST is not set}"
tmp=$(mktemp -d "${TMPDIR:-/tmp}/c-tests.XXXXXX") || exit 1
ctest_pids=
# The block below is a byte-identical copy of the one in tools/check/steps/c-faults.sh, and it is a
# copy rather than a shared helper file on purpose: a new tracked file would move `tracked`,
# `files` and `hygiene-files` and destroy the byte-identical summary line the Phase 16 lever
# commit is required to show. The two copies are compared between their own markers.
# build-lock begin
# Serialise this tree's builds: the two steps that tools/gate.sh runs overlapped both build the
# dev preset in one tree, and two concurrent `cmake --build --preset dev` runs were measured to
# fail at link time (`clang: error: no such file or directory: 'core/libivs_core.a'` - one run
# archiving while the other links), so every build takes this mkdir lock. It lives in the per-run
# results directory, which tools/gate.sh recreates (rm -rf, then mkdir -p) for every run, so a
# stale lock cannot outlive a run and no takeover rule is needed. The wait is bounded and polled
# (0.01 s, 60000 polls = about ten minutes); on expiry the caller reports its own token with the
# `build-lock` detail and exits non-zero. build_unlock releases on the success path, the failure
# path and the HUP/INT/TERM and EXIT traps the caller already sets.
build_lock=
lock_held=0
lock_polls=60000
build_unlock() {
  [ "$lock_held" -eq 1 ] || return 0
  rmdir "$build_lock" 2> /dev/null
  lock_held=0
}
lock_acquire() {
  build_lock="$gate_dir/.build-lock"
  _lk=0
  while ! mkdir "$build_lock" 2> /dev/null; do
    if [ "$_lk" -ge "$lock_polls" ]; then
      build_lock=
      return 1
    fi
    _lk=$((_lk + 1))
    sleep 0.01
  done
  lock_held=1
  return 0
}
# build-lock end
trap 'build_unlock; rm -rf "$tmp"' EXIT
trap 'build_unlock; if [ -n "$ctest_pids" ]; then kill $ctest_pids 2> /dev/null; fi; exit 1' HUP INT TERM
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
  if ! lock_acquire; then
    printf 'build-lock\n' > "$tmp/$preset.state"
    continue
  fi
  if ! cmake --build --preset "$preset" > "$tmp/$preset.build.log" 2>&1; then
    build_unlock
    printf 'build\n' > "$tmp/$preset.state"
    continue
  fi
  build_unlock
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
    build-lock) bad c-tests-failed "build-lock ($preset)"; continue ;;
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

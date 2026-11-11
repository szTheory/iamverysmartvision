#!/bin/sh
# SPDX-License-Identifier: MIT
# Step c-faults: build the named-fault executables (target ivs_faults, dev preset) and run each
# fault of every data/expected/faults/*.ids file (sorted; D-37) once, writing its TAP result to
# <results>/faults/<ID>/<suite>.txt with the gate nonce (D-18), where <suite> comes from the
# `# suite=<name>` header of the file that lists the id. A fault run is expected to exit
# non-zero; this step fails only when a binary is absent, no result file is written, a file has
# no suite header or an id is listed twice. The faults-verdict step judges the results.
# Results dir: IVS_GATE_DIR (set by tools/gate.sh), else $ROOT/build/gate; the nonce is read
# from <results>/NONCE (created with a direct-run value when absent).
# Fault executables run in at most 4 lanes (A-07 lever, plan 03-02; the run-controls.sh lane
# pattern): the ids are first listed in file order and sorted id order, lane i runs every 4th
# listed id with its own log, and the findings are printed afterwards in that listed order, so
# the output does not depend on scheduling.
# Watchdog (review IN-04): a fault binary is not a ctest test, so no ctest timeout applies; each
# run gets IVS_FAULT_TIMEOUT seconds (default 120), after which it is sent TERM and reported
# c-faults-run: <ID> (timeout). POSIX sh only: a background sleep in a watchdog subshell, killed
# through its TERM trap when the run ends first. A self-test, run alongside the lanes, proves the
# watchdog stops a 30-second sleep after 1 second and stays silent for a run that ends in time.
# Tokens: c-faults-run: <ID or what>, c-faults-run: no suite in <file>,
#   c-faults-run: duplicate <ID>, c-faults-run: <ID> (timeout),
#   c-faults-run: watchdog self-test (<case>). Stats: fault-runs=<n> (summed over files).
LC_ALL=C; export LC_ALL
: "${ROOT:?ROOT is not set}"
: "${CHECK_DIR:?CHECK_DIR is not set}"
: "${FILELIST:?FILELIST is not set}"
tmp=$(mktemp -d "${TMPDIR:-/tmp}/c-faults.XXXXXX") || exit 1
lane_pids=
trap 'rm -rf "$tmp"' EXIT
trap 'if [ -n "$lane_pids" ]; then kill $lane_pids 2> /dev/null; fi; exit 1' HUP INT TERM
rc=0
bad() { printf -- '-- FAIL %s: %s\n' "$1" "$2"; rc=1; }
cd "$ROOT" || exit 1

# Every faults file and its suite, checked before the build so a bad inventory stops early.
: > "$tmp/files"
for ids in "$ROOT"/data/expected/faults/*.ids; do
  [ -f "$ids" ] || continue
  rel=${ids#"$ROOT"/}
  suite=$(sed -n 's/^#[[:space:]]*suite=\([a-z0-9][a-z0-9-]*\).*$/\1/p' "$ids" | head -n 1)
  if [ -z "$suite" ]; then
    bad c-faults-run "no suite in $rel"
    continue
  fi
  printf '%s %s\n' "$suite" "$rel" >> "$tmp/files"
done
if [ ! -s "$tmp/files" ] && [ "$rc" -eq 0 ]; then
  bad c-faults-run "data/expected/faults/*.ids (none)"
fi
[ "$rc" -eq 0 ] || exit 1

gate_dir=${IVS_GATE_DIR:-$ROOT/build/gate}
mkdir -p "$gate_dir" || { bad c-faults-run "results dir"; exit 1; }
if [ ! -f "$gate_dir/NONCE" ]; then
  printf 'direct.%s\n' "$$" > "$gate_dir/NONCE" || { bad c-faults-run "NONCE"; exit 1; }
fi
nonce=$(cat "$gate_dir/NONCE")

if ! cmake --build --preset dev --target ivs_faults > "$tmp/build.log" 2>&1; then
  cat "$tmp/build.log"
  bad c-faults-run "build ivs_faults"
  exit 1
fi

# run_limited <seconds> <marker> <log> <command...>: runs the command (stdin /dev/null, output to
# <log>); if it is still running after <seconds>, writes <marker> and sends it TERM.
limit=${IVS_FAULT_TIMEOUT:-120}
case $limit in ''|*[!0-9]*|0) bad c-faults-run "IVS_FAULT_TIMEOUT=$limit"; exit 1 ;; esac
run_limited() {
  _lim=$1; _mark=$2; _log=$3; shift 3
  "$@" > "$_log" 2>&1 < /dev/null &
  _pid=$!
  (
    _s=
    trap 'if [ -n "$_s" ]; then kill "$_s" 2> /dev/null; fi; exit 0' TERM
    sleep "$_lim" &
    _s=$!
    wait "$_s"
    : > "$_mark"
    kill -TERM "$_pid" 2> /dev/null
  ) > /dev/null 2>&1 < /dev/null &
  _wd=$!
  wait "$_pid" || :
  kill -TERM "$_wd" 2> /dev/null
  wait "$_wd" 2> /dev/null || :
}

out_root="$gate_dir/faults"
rm -rf "$out_root"
mkdir -p "$out_root" || { bad c-faults-run "faults dir"; exit 1; }
JOBS=4
# Pass 1: list every id in order; an id that cannot run is a finding at its place in the list.
: > "$tmp/seen"
: > "$tmp/plan"
: > "$tmp/runs"
while read -r suite rel; do
  for id in $(awk '/^#/ { next } NF { print $1 }' "$ROOT/$rel" | sort); do
    case $id in
      *[!A-Z0-9_]*) printf 'BAD %s (bad id)\n' "$id" >> "$tmp/plan"; continue ;;
    esac
    if grep -q -x -F -e "$id" "$tmp/seen"; then
      printf 'BAD duplicate %s (%s)\n' "$id" "$rel" >> "$tmp/plan"
      continue
    fi
    printf '%s\n' "$id" >> "$tmp/seen"
    if [ ! -x "$ROOT/build/dev/tests/ivs_fault_$id" ]; then
      printf 'BAD %s (no binary)\n' "$id" >> "$tmp/plan"
      continue
    fi
    if ! mkdir -p "$out_root/$id"; then
      printf 'BAD %s (dir)\n' "$id" >> "$tmp/plan"
      continue
    fi
    printf 'RUN %s %s\n' "$id" "$suite" >> "$tmp/plan"
    printf '%s\n' "$id" >> "$tmp/runs"
  done
done < "$tmp/files"

# Watchdog self-test, alongside the lanes (the `wait` below collects it): a 30-second sleep must
# be stopped after 1 second, and a run that ends at once must leave no marker.
(
  run_limited 1 "$tmp/wd-fire.mark" "$tmp/wd-fire.log" sleep 30
  run_limited 5 "$tmp/wd-quiet.mark" "$tmp/wd-quiet.log" true
) 2> /dev/null &
# Pass 2: run the listed ids in JOBS lanes; a fault run is expected to exit non-zero.
lane=0
while [ "$lane" -lt "$JOBS" ]; do
  (
    k=0
    while IFS= read -r id; do
      if [ $((k % JOBS)) -eq "$lane" ]; then
        IVS_TEST_OUT_DIR="$out_root/$id"
        IVS_TEST_NONCE=$nonce
        export IVS_TEST_OUT_DIR IVS_TEST_NONCE
        run_limited "$limit" "$tmp/$id.timeout" "$tmp/$id.log" "$ROOT/build/dev/tests/ivs_fault_$id"
      fi
      k=$((k + 1))
    done < "$tmp/runs"
  ) &
  lane_pids="$lane_pids $!"
  lane=$((lane + 1))
done
wait
lane_pids=
[ -f "$tmp/wd-fire.mark" ] || bad c-faults-run "watchdog self-test (fire)"
[ ! -f "$tmp/wd-quiet.mark" ] || bad c-faults-run "watchdog self-test (quiet)"

# Pass 3: findings in listed order.
runs=0
while read -r kind id suite; do
  if [ "$kind" = BAD ]; then
    bad c-faults-run "$id${suite:+ $suite}"
    continue
  fi
  if [ -f "$tmp/$id.timeout" ]; then
    cat "$tmp/$id.log"
    bad c-faults-run "$id (timeout after ${limit}s)"
  elif [ -s "$out_root/$id/$suite.txt" ]; then
    runs=$((runs + 1))
  else
    cat "$tmp/$id.log"
    bad c-faults-run "$id (no $suite.txt)"
  fi
done < "$tmp/plan"

[ "$rc" -eq 0 ] || exit 1
printf -- '-- c-faults pass fault-runs=%s\n' "$runs"

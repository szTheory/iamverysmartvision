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
# through its TERM trap when the run ends first. The watchdog signals that it is armed with a file the
# parent polls for within a bound (review WR-03; see run_limited). A self-test, run alongside the lanes,
# proves the watchdog stops a 30-second sleep after 1 second, stays silent for a run that ends in time,
# and that a watchdog which dies or never arms ends the wait as a finding instead of a hang.
# Tokens: c-faults-run: <ID or what>, c-faults-run: no suite in <file>,
#   c-faults-run: duplicate <ID>, c-faults-run: <ID> (timeout), c-faults-run: <ID> (watchdog not armed),
#   c-faults-run: watchdog self-test (<case>: fire, quiet, arm, dead, stall). Stats: fault-runs=<n> (summed over files).
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

limit=${IVS_FAULT_TIMEOUT:-120}
case $limit in ''|*[!0-9]*|0) bad c-faults-run "IVS_FAULT_TIMEOUT=$limit"; exit 1 ;; esac
# wd_body <seconds> <pid> <marker>: the watchdog, run by run_limited in a background subshell. It sets its
# TERM trap first, then starts its sleep, and only then creates <marker>.armed (the arming signal); when
# the sleep ends it writes <marker> and sends <pid> TERM. A function, so the self-test below can
# replace it with a watchdog that never arms.
wd_body() {
  _s=
  trap 'if [ -n "$_s" ]; then kill "$_s" 2> /dev/null; fi; exit 0' TERM
  sleep "$1" &
  _s=$!
  : > "$3.armed"
  wait "$_s"
  : > "$3"
  kill -TERM "$2" 2> /dev/null
}

# run_limited <seconds> <marker> <log> <command...>: runs the command (stdin /dev/null, output to
# <log>); if it is still running after <seconds>, writes <marker> and sends it TERM. It returns only
# once the watchdog is armed, which is a bounded wait (review WR-03): the parent polls for the file
# <marker>.armed that the watchdog creates after its TERM trap exists (a stop that arrives earlier can
# be dropped by dash, and the watchdog would then sleep its whole limit and report a finished run as
# stopped), checking at once and then every 0.01 s, for at most wd_polls polls (500, about 5 s) and
# never past the death of the watchdog. A watchdog that is dead or silent by then is a finding: the
# command is stopped, <marker>.unarmed is written and the caller reports it; there is no blocking read
# left to hang on (a FIFO read held open by the parent itself never saw EOF, and the retry counter
# counted only failed reads).
wd_polls=500
run_limited() {
  _lim=$1; _mark=$2; _log=$3; shift 3
  rm -f "$_mark" "$_mark.armed" "$_mark.unarmed"
  "$@" > "$_log" 2>&1 < /dev/null &
  _pid=$!
  ( wd_body "$_lim" "$_pid" "$_mark" ) > /dev/null 2>&1 < /dev/null &
  _wd=$!
  _n=0
  while [ ! -e "$_mark.armed" ]; do
    if [ "$_n" -ge "$wd_polls" ] || ! kill -0 "$_wd" 2> /dev/null; then
      # A watchdog that armed and ended in the last instant still counts as armed.
      if [ ! -e "$_mark.armed" ]; then
        : > "$_mark.unarmed"
        kill -TERM "$_pid" 2> /dev/null
      fi
      break
    fi
    _n=$((_n + 1))
    sleep 0.01
  done
  wait "$_pid" || :
  kill -TERM "$_wd" 2> /dev/null
  wait "$_wd" 2> /dev/null || :
  rm -f "$_mark.armed"
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
# be stopped after 1 second, and a run that ends at once must leave no marker (60 s
# limit: a scheduling stall on a loaded host must not fire it); a watchdog that dies before it arms
# (dead) and one that never arms (stall, 10 polls) must each end the wait with the .unarmed finding
# and stop their command, never hang the subshell (review WR-03).
(
  run_limited 1 "$tmp/wd-fire.mark" "$tmp/wd-fire.log" sleep 30
  run_limited 60 "$tmp/wd-quiet.mark" "$tmp/wd-quiet.log" true
  wd_body() { exit 0; }
  run_limited 60 "$tmp/wd-dead.mark" "$tmp/wd-dead.log" sleep 30
  wd_body() { exec sleep 3; }
  wd_polls=10
  run_limited 60 "$tmp/wd-stall.mark" "$tmp/wd-stall.log" sleep 30
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
{ [ ! -f "$tmp/wd-fire.mark.unarmed" ] && [ ! -f "$tmp/wd-quiet.mark.unarmed" ]; } || bad c-faults-run "watchdog self-test (arm)"
[ -f "$tmp/wd-dead.mark.unarmed" ] || bad c-faults-run "watchdog self-test (dead)"
[ -f "$tmp/wd-stall.mark.unarmed" ] || bad c-faults-run "watchdog self-test (stall)"

# Pass 3: findings in listed order.
runs=0
while read -r kind id suite; do
  if [ "$kind" = BAD ]; then
    bad c-faults-run "$id${suite:+ $suite}"
    continue
  fi
  if [ -f "$tmp/$id.timeout.unarmed" ]; then
    cat "$tmp/$id.log"
    bad c-faults-run "$id (watchdog not armed)"
  elif [ -f "$tmp/$id.timeout" ]; then
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

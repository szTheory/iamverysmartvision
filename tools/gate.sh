#!/bin/sh
# SPDX-License-Identifier: MIT
# The single local/CI command: sh tools/gate.sh (no arguments). See SKELETON.md section 1.
# Measurement lines (not tokens; the summary line ignores them): `step-time <name> <s>` per step,
# then `gate-env ncpu=<n> cc=<id>-<version>` and `gate-cpu user=<s> sys=<s>` (children CPU from
# POSIX `times`, read in this shell after the controls `wait`), then `gate-time total=<s>`.
# Preflight partition (A-05): the 29 steps whose script references neither $IVS_GATE_DIR nor build/
# are consistency checks that need no built artifact, so they run FIRST and print
# `gate-preflight: PASS steps=<n>/<n>` (or `gate-preflight: FAIL <name>...`) before the 9 producers
# start: a typo in a checker is reported before the build and the suites, not after them. The class
# is DERIVED from each step's source text by the grep below, never a hand-maintained list, and it is
# printed as `gate-partition preflight=<n> producers=<n>: <names...>` before any step runs, so a step
# that starts reading the results directory shows up as a change of class in the log.
# The OUTPUT order is unchanged: each step's whole stdout+stderr is buffered in
# $GATE_TMP/step.out.<name> (one reused file cannot hold 38 bodies) and replayed after both groups in
# steps.have order, so `cat`, keep_fails, the bad lines, the tokens and step-time are all emitted in
# the sorted-name order a reader of an earlier log expects. Nothing is emitted during execution.
# Job summary (D-42): when GITHUB_STEP_SUMMARY is non-empty, the EXIT trap runs
# tools/check/ci-summary.sh on the files kept in $GATE_TMP (summary.line, measure.lines,
# step.times, fail.lines) before the temp dir is removed. It prints nothing and its status is
# ignored, so it cannot change the verdict; early exits are summarized too.
LC_ALL=C; export LC_ALL
gate_t0=$(date +%s)
here=$(cd "$(dirname "$0")" && pwd) || exit 1
ROOT=$(cd "$here/.." && pwd) || exit 1
CHECK_DIR="$ROOT/tools/check"
GATE_TMP=$(mktemp -d "${TMPDIR:-/tmp}/gate.XXXXXX") || exit 1
ctl_pid=
on_exit() {
  if [ -n "$ctl_pid" ]; then kill "$ctl_pid" 2> /dev/null; wait "$ctl_pid" 2> /dev/null; fi
  [ -z "${GITHUB_STEP_SUMMARY:-}" ] || sh "$CHECK_DIR/ci-summary.sh" "$GATE_TMP" > /dev/null 2>&1 || :
  rm -rf "$GATE_TMP"
}
trap on_exit EXIT
trap 'exit 1' HUP INT TERM
FILELIST="$GATE_TMP/filelist"
export ROOT CHECK_DIR FILELIST GATE_TMP
fail=0
tokens=""
bad() {
  printf -- '-- FAIL %s: %s\n' "$1" "$2"
  printf -- '-- FAIL %s: %s\n' "$1" "$2" >> "$GATE_TMP/fail.lines"
  fail=1
}
# Early exit: the summary line on stdout and in $GATE_TMP for the job summary.
early_exit() { printf 'gate: FAIL\n'; printf 'gate: FAIL\n' > "$GATE_TMP/summary.line"; exit 1; }
# FAIL lines of a step's or the controls' output, kept for the job summary.
keep_fails() { grep '^-- FAIL ' "$1" >> "$GATE_TMP/fail.lines"; }
# Wall-clock timing (whole seconds, POSIX date): one `step-time <name> <s>` line per step, so a
# hosted log shows where the time goes. Not tokens: the summary line ignores them.
step_time() {
  printf 'step-time %s %s\n' "$1" $(($2 - $3))
  printf 'step-time %s %s\n' "$1" $(($2 - $3)) >> "$GATE_TMP/step.times"
}

command -v cmake >/dev/null 2>&1 || { bad gate-no-cmake cmake; early_exit; }
command -v ninja >/dev/null 2>&1 || { bad gate-no-ninja ninja; early_exit; }
command -v "${CC:-cc}" >/dev/null 2>&1 || { bad gate-no-cc "${CC:-cc}"; early_exit; }

# FILELIST: tracked plus untracked-not-ignored paths. A failing git must not read as a clean list.
cd "$ROOT" || exit 1
if ! git -c core.quotePath=false ls-files --cached --others --exclude-standard > "$GATE_TMP/files.raw"; then
  bad gate-filelist "git ls-files failed"
  early_exit
fi
: > "$GATE_TMP/files.keep"
while IFS= read -r f; do
  case $f in
    *[!A-Za-z0-9._/-]*) printf '%s\n' "$f" >> "$GATE_TMP/files.keep" ;;
    *) if [ -e "$f" ]; then printf '%s\n' "$f" >> "$GATE_TMP/files.keep"; fi ;;
  esac
done < "$GATE_TMP/files.raw"
sort -u "$GATE_TMP/files.keep" > "$FILELIST"

# Results directory contract: a fresh build/gate/ per run; NONCE is the basename of this run's temp dir.
# The C steps export-read IVS_GATE_DIR and IVS_TEST_NONCE; build/ is ignored, so nothing enters FILELIST.
NONCE=$(basename "$GATE_TMP")
IVS_GATE_DIR="$ROOT/build/gate"
if rm -rf "$IVS_GATE_DIR" && mkdir -p "$IVS_GATE_DIR" && printf '%s\n' "$NONCE" > "$IVS_GATE_DIR/NONCE"; then :; else
  bad gate-results-dir build/gate
  early_exit
fi
IVS_TEST_NONCE=$NONCE
export IVS_GATE_DIR IVS_TEST_NONCE

# Negative controls start now, in the background, and overlap the steps (D-22 next lever).
# The runner reads only tools/check/ and copies of controls/_good/tree into its own temp dir,
# never ROOT's build/ or the steps' outputs, so nothing a step writes can change its result.
# Its output is held in a file and printed after the steps, in the same place as before.
# The runner is the background job itself, so the EXIT trap's TERM reaches it, and it stops
# its lanes and their steps before it exits (review WR-04); it writes its own end time.
ctl_t0=$(date +%s)
CONTROLS_END_FILE="$GATE_TMP/controls.end" sh "$CHECK_DIR/run-controls.sh" > "$GATE_TMP/controls.out" 2>&1 < /dev/null &
ctl_pid=$!
step_time setup "$ctl_t0" "$gate_t0"

# Step inventory: step scripts must equal the union of data/expected/gate-steps/*.ids.
: > "$GATE_TMP/steps.have"
for s in "$CHECK_DIR"/steps/*.sh; do
  [ -f "$s" ] || continue
  basename "$s" .sh >> "$GATE_TMP/steps.have"
done
sort -u "$GATE_TMP/steps.have" -o "$GATE_TMP/steps.have"
: > "$GATE_TMP/steps.want"
for f in "$ROOT"/data/expected/gate-steps/*.ids; do
  [ -f "$f" ] || continue
  awk '/^#/ { next } NF { print $1 }' "$f" >> "$GATE_TMP/steps.want"
done
sort -u "$GATE_TMP/steps.want" -o "$GATE_TMP/steps.want"
if [ ! -s "$GATE_TMP/steps.want" ]; then bad gate-steps-empty "data/expected/gate-steps"; fi
want=$(grep -c . "$GATE_TMP/steps.want")
for name in $(comm -23 "$GATE_TMP/steps.want" "$GATE_TMP/steps.have"); do bad gate-step-missing "$name"; done
for name in $(comm -13 "$GATE_TMP/steps.want" "$GATE_TMP/steps.have"); do bad gate-step-extra "$name"; done

# Preflight partition: a step is a producer iff its script names the results directory or build/.
# Anything else is a pure consistency check and runs in the first group. Both lists are sorted
# because they are derived from steps.have, which is sorted.
: > "$GATE_TMP/preflight"
: > "$GATE_TMP/producers"
for name in $(cat "$GATE_TMP/steps.have"); do
  if grep -q -e 'IVS_GATE_DIR' -e 'build/' "$CHECK_DIR/steps/$name.sh"; then
    printf '%s\n' "$name" >> "$GATE_TMP/producers"
  else
    printf '%s\n' "$name" >> "$GATE_TMP/preflight"
  fi
done
pf_n=$(grep -c . "$GATE_TMP/preflight")
pr_n=$(grep -c . "$GATE_TMP/producers")
printf 'gate-partition preflight=%s producers=%s: %s\n' "$pf_n" "$pr_n" \
  "$(awk '{ printf "%s%s", sep, $0; sep = " " }' "$GATE_TMP/preflight")"

# Run every step (sorted within each group, never stopping at the first failure), preflight first.
# Output goes to a per-step buffer and nothing is printed here: the replay below does all of it.
for group in preflight producers; do
  for name in $(cat "$GATE_TMP/$group"); do
    t0=$(date +%s)
    if sh "$CHECK_DIR/steps/$name.sh" > "$GATE_TMP/step.out.$name" 2>&1; then
      printf '0\n' > "$GATE_TMP/rc.$name"
    else
      printf '1\n' > "$GATE_TMP/rc.$name"
    fi
    printf '%s %s\n' "$(date +%s)" "$t0" > "$GATE_TMP/t.$name"
  done
  # The early verdict: printed as soon as the preflight group has finished, before any producer
  # runs. It never stops the run - the producers and the replay still happen.
  if [ "$group" = preflight ]; then
    pf_failed=
    for name in $(cat "$GATE_TMP/preflight"); do
      [ "$(cat "$GATE_TMP/rc.$name")" -eq 0 ] || pf_failed="$pf_failed $name"
    done
    if [ -z "$pf_failed" ]; then
      printf 'gate-preflight: PASS steps=%s/%s\n' "$pf_n" "$pf_n"
    else
      printf 'gate-preflight: FAIL%s\n' "$pf_failed"
    fi
  fi
done

# Replay in steps.have order, which is sorted name order: the same cat, keep_fails, pass/fail
# branch, token accumulation and step-time line as a single sorted loop emitted before the
# partition existed. keep_fails runs HERE, not during execution, so $GATE_TMP/fail.lines - and so
# the job summary - keeps its order.
passed=0
for name in $(cat "$GATE_TMP/steps.have"); do
  cat "$GATE_TMP/step.out.$name"
  keep_fails "$GATE_TMP/step.out.$name"
  if [ "$(cat "$GATE_TMP/rc.$name")" -eq 0 ]; then
    # The pass line must be the step's LAST non-empty line (A-02/O2, EV-03; review WR-02). This is
    # the only place the rule can cover all 38 steps: run-controls.sh's copy runs once per distinct
    # CHECK, so the five steps no control names would otherwise keep only the weaker
    # 'a pass line exists somewhere'. The rule itself lives in tools/check/pass-line.sh so the
    # runner-selftest st-gate-pass-line case can make it fail for its intended reason. Both shapes
    # report gate-step-no-pass-line, and the detail says which: gate.sh has no ids inventory of its
    # own tokens, so a second token here would be a name nothing pins, and the shape is carried by
    # the detail instead. The token is reused rather than renamed because the invariant is one
    # invariant - a step that exits 0 must END with its verdict - and every earlier log reads this
    # name.
    pl_rc=0
    sh "$CHECK_DIR/pass-line.sh" "$name" "$GATE_TMP/step.out.$name" || pl_rc=$?
    case $pl_rc in
      0)
        passed=$((passed + 1))
        toks=$(sed -n "s/^-- $name pass //p" "$GATE_TMP/step.out.$name" | tr '\n' ' ')
        tokens="$tokens$toks"
        ;;
      2) bad gate-step-no-pass-line "$name (the pass line is not the last non-empty line)" ;;
      *) bad gate-step-no-pass-line "$name" ;;
    esac
  else
    bad gate-step-failed "$name"
  fi
  step_time "$name" $(cat "$GATE_TMP/t.$name")
done

# Negative controls.
ctl_ok=0
ctl_total=0
wait_t0=$(date +%s)
if wait "$ctl_pid"; then ctl_rc=0; else ctl_rc=1; fi
ctl_pid=
wait_t1=$(date +%s)
if [ "$ctl_rc" -ne 0 ]; then bad gate-controls-failed "run-controls"; fi
cat "$GATE_TMP/controls.out"
keep_fails "$GATE_TMP/controls.out"
ctl_t1=$wait_t1
if [ -s "$GATE_TMP/controls.end" ]; then ctl_t1=$(cat "$GATE_TMP/controls.end"); fi
step_time controls "$ctl_t1" "$ctl_t0"
step_time controls-wait "$wait_t1" "$wait_t0"
line=$(sed -n 's/^controls=\([0-9][0-9]*\)\/\([0-9][0-9]*\)$/\1 \2/p' "$GATE_TMP/controls.out" | tail -n 1)
if [ -n "$line" ]; then
  ctl_ok=${line% *}
  ctl_total=${line#* }
else
  bad gate-controls-line "no controls=<ok>/<total> line"
fi
if [ "$ctl_total" -eq 0 ]; then bad gate-controls-empty "no controls ran"; fi

# Measurement (D-44). `times` runs as a plain command in this shell, after every child was
# waited for: inside a pipeline or $( ) it would run in a subshell and report zeros (Pitfall 9).
# `command` keeps a redirection error from exiting the shell (times is a special built-in).
ncpu=$(getconf _NPROCESSORS_ONLN 2> /dev/null) || ncpu=
case $ncpu in '' | *[!0-9]*) ncpu=unknown ;; esac
cc_id=
cc_cfg=
cc_cache="$ROOT/build/dev/CMakeCache.txt"
if [ -f "$cc_cache" ]; then
  cc_ver=$(awk -F= '/^CMAKE_CACHE_MAJOR_VERSION:/ { a = $2 } /^CMAKE_CACHE_MINOR_VERSION:/ { b = $2 }
    /^CMAKE_CACHE_PATCH_VERSION:/ { c = $2 } END { if (a != "" && b != "" && c != "") print a "." b "." c }' "$cc_cache")
  if [ -n "$cc_ver" ] && [ -f "$ROOT/build/dev/CMakeFiles/$cc_ver/CMakeCCompiler.cmake" ]; then
    cc_cfg="$ROOT/build/dev/CMakeFiles/$cc_ver/CMakeCCompiler.cmake"
  fi
fi
if [ -n "$cc_cfg" ]; then
  cc_name=$(sed -n 's/^set(CMAKE_C_COMPILER_ID "\([^"]*\)")$/\1/p' "$cc_cfg" | head -n 1)
  cc_v=$(sed -n 's/^set(CMAKE_C_COMPILER_VERSION "\([^"]*\)")$/\1/p' "$cc_cfg" | head -n 1)
  if [ -n "$cc_name" ] && [ -n "$cc_v" ]; then cc_id="$cc_name-$cc_v"; else cc_id=$cc_name; fi
fi
[ -n "$cc_id" ] || cc_id=unknown
printf 'gate-env ncpu=%s cc=%s\n' "$ncpu" "$cc_id" > "$GATE_TMP/measure.lines"
command times > "$GATE_TMP/times.out"
sh "$CHECK_DIR/ci-summary.sh" --cpu < "$GATE_TMP/times.out" >> "$GATE_TMP/measure.lines"
cat "$GATE_TMP/measure.lines"

status=PASS
[ "$fail" -eq 0 ] || status=FAIL
tokens=${tokens% }
printf 'gate-time total=%s\n' $(($(date +%s) - gate_t0)) | tee -a "$GATE_TMP/measure.lines"
printf 'gate: %s steps=%s/%s controls=%s/%s %s\n' "$status" "$passed" "$want" "$ctl_ok" "$ctl_total" "$tokens" > "$GATE_TMP/summary.line"
cat "$GATE_TMP/summary.line"
[ "$fail" -eq 0 ]

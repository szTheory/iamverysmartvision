#!/bin/sh
# SPDX-License-Identifier: MIT
# Suite runner-process (D-20, D-21, D-23, plan 04-20): the real ivs-run executable, judged at the
# process level by its exit code and report text.
# Scenario cases: per line `<id> <exit> <outcome> <class> <skip> <operator-args>` of
# data/expected/runner-scenarios.ids, run
#   ivs-run --content-dir <content> <operator-args split on ,> <root>/data/scenarios/<id>.ivss
# where <content> is this run's temp copy of the build-tree images dir; id
# runner-process.scenario.<id>. The judge requires the inventory's exit code; a report whose first
# line is `ivsr 1` and whose last is `report.end <n>` (n = the lines before it, LF-terminated);
# exactly one `outcome` and one `exit.code` line equal to the inventory's; and `outcome.detail`
# equal to the scenario's `# expect-detail: <text>` comment line (`-` without one). A scenario
# with a `# expect-stderr: <text>` comment line must instead print no report and exactly the line
# `ivs-run: <text>` on stderr (a parse refusal). A run still alive after IVS_RUNNER_PROCESS_TIMEOUT
# seconds (default 60) is stopped by a watchdog and judged `hung` (no sleeps synchronise a test;
# the watchdog only bounds a broken runner). Class (column 4) is `deterministic` or
# `host-dependent` (wall time, signals; the golden step of plan 04-21 excludes those from the dev
# versus release comparison). Skip (column 5) `skip-ok` marks a scenario that needs operator content
# the gate never has: it is judged like any other, and only a passing judgement is reported as a
# visible `# SKIP`.
# Reports: when IVS_TEST_OUT_DIR is set, every scenario case that printed a report leaves it in
# <dir>/runner/<id>.report (the golden step compares dev with release there).
# Output: the harness TAP shape (header with IVS_TEST_NONCE or none, plan, one result per case
# with the findings as `#` lines after a failure, trailer) on stdout and, when IVS_TEST_OUT_DIR is
# set, in <dir>/runner-process.txt.
# Usage: sh runner-process-tap.sh <source root> <ivs-run> <ivs_pack> <images dir>
# Exit: 0 pass, 1 fail, 77 every case skipped.
# Env: IVS_TEST_NONCE, IVS_TEST_OUT_DIR, IVS_RUNNER_PROCESS_TIMEOUT.
LC_ALL=C; export LC_ALL
usage='usage: sh runner-process-tap.sh <source root> <ivs-run> <ivs_pack> <images dir>'
[ "$#" -eq 4 ] || { printf '%s\n' "$usage" >&2; exit 1; }
# Absolute paths: the cwd case runs from another directory.
abs_dir() { (cd "$1" 2> /dev/null && pwd -P); }
abs_file() { _d=$(abs_dir "$(dirname "$1")") && printf '%s/%s\n' "$_d" "$(basename "$1")"; }
root=$(abs_dir "$1") || { printf 'runner-process-tap: no dir %s\n' "$1" >&2; exit 1; }
ivs_run=$(abs_file "$2") || { printf 'runner-process-tap: no %s\n' "$2" >&2; exit 1; }
pack=$(abs_file "$3") || { printf 'runner-process-tap: no %s\n' "$3" >&2; exit 1; }
images=$(abs_dir "$4") || { printf 'runner-process-tap: no dir %s\n' "$4" >&2; exit 1; }
inv="$root/data/expected/runner-scenarios.ids"
scen="$root/data/scenarios"
[ -f "$inv" ] || { printf 'runner-process-tap: no %s\n' "$inv" >&2; exit 1; }
[ -x "$ivs_run" ] || { printf 'runner-process-tap: %s is not executable\n' "$ivs_run" >&2; exit 1; }
[ -x "$pack" ] || { printf 'runner-process-tap: %s is not executable\n' "$pack" >&2; exit 1; }
limit=${IVS_RUNNER_PROCESS_TIMEOUT:-60}
case $limit in '' | *[!0-9]* | 0) printf 'runner-process-tap: IVS_RUNNER_PROCESS_TIMEOUT=%s\n' "$limit" >&2; exit 1 ;; esac
tmp=$(mktemp -d "${TMPDIR:-/tmp}/runner-process-tap.XXXXXX") || exit 1
trap 'rm -rf "$tmp"' EXIT
trap 'exit 1' HUP INT TERM

ifs0=$IFS
suite=runner-process
nonce=${IVS_TEST_NONCE:-none}
rep_dir=
if [ -n "${IVS_TEST_OUT_DIR:-}" ]; then
  rep_dir="$IVS_TEST_OUT_DIR/runner"
  mkdir -p "$rep_dir" || { printf '# cannot create %s\n' "$rep_dir" >&2; exit 1; }
fi

# The content dir: copies of the build-tree images.
content="$tmp/content"
mkdir "$content" "$tmp/runs" || exit 1
for f in boot-stub.exec diag-cpu-1.ivsc; do
  cp "$images/$f" "$content/$f" || { printf 'runner-process-tap: no image %s/%s\n' "$images" "$f" >&2; exit 1; }
done

# run_one <cwd> <name> <command...>: runs the command in <cwd> with stdin /dev/null, stdout in
# $tmp/runs/<name>.out and stderr in $tmp/runs/<name>.err; sets rc to its exit status and hung to
# 1 when the watchdog stopped it after $limit seconds. A command ending in .sh is run with sh.
run_one() {
  _cwd=$1; _name=$2; shift 2
  case $1 in *.sh) set -- sh "$@" ;; esac
  _mark="$tmp/runs/$_name.hung"
  rm -f "$_mark"
  (cd "$_cwd" && exec "$@") > "$tmp/runs/$_name.out" 2> "$tmp/runs/$_name.err" < /dev/null &
  _pid=$!
  (
    _s=
    trap 'if [ -n "$_s" ]; then kill "$_s" 2> /dev/null; fi; exit 0' TERM
    sleep "$limit" &
    _s=$!
    wait "$_s"
    : > "$_mark"
    kill -TERM "$_pid" 2> /dev/null
  ) > /dev/null 2>&1 < /dev/null &
  _wd=$!
  wait "$_pid"
  rc=$?
  kill -TERM "$_wd" 2> /dev/null
  wait "$_wd" 2> /dev/null
  hung=0
  if [ -f "$_mark" ]; then hung=1; fi
}

# why <token> <text>: records one judge finding in $tmp/why.
why() { printf '%s: %s\n' "$1" "$2" >> "$tmp/why"; }

# judge <name> <exit> <outcome> <detail pattern> <stderr line>: judges run <name> (rc, hung and
# $tmp/runs/<name>.{out,err}) against the expectation; <detail pattern> is a shell pattern for the
# outcome.detail value; a non-empty <stderr line> expects no report and exactly `ivs-run: <line>`
# on stderr. Appends findings to $tmp/why; returns 0 when there are none.
judge() {
  _o="$tmp/runs/$1.out"; _e="$tmp/runs/$1.err"; _x=$2; _oc=$3; _dp=$4; _sl=$5
  : > "$tmp/why"
  if [ "$hung" -eq 1 ]; then why hung "still running after ${limit}s, stopped by the watchdog"; fi
  [ "$rc" = "$_x" ] || why exit-code "exit $rc, expected $_x"
  if [ -n "$_sl" ]; then
    [ ! -s "$_o" ] || why unexpected-report "a report on stdout where the scenario expects none"
    printf 'ivs-run: %s\n' "$_sl" > "$tmp/want.err"
    cmp -s "$_e" "$tmp/want.err" || why stderr "stderr is not exactly: ivs-run: $_sl"
  elif [ ! -s "$_o" ]; then
    why no-report "empty stdout"
  else
    _first=$(sed -n '1p' "$_o")
    [ "$_first" = "ivsr 1" ] || why header "first line '$_first', expected 'ivsr 1'"
    _nr=$(awk 'END { print NR }' "$_o")
    _nl=$(wc -l < "$_o" | tr -d ' ')
    _last=$(sed -n '$p' "$_o")
    if [ "$_nr" != "$_nl" ]; then
      why report-end "the report does not end with LF"
    elif [ "$_last" != "report.end $((_nr - 1))" ]; then
      why report-end "last line '$_last', expected 'report.end $((_nr - 1))'"
    fi
    _n=$(grep -c '^outcome ' "$_o")
    _v=$(sed -n 's/^outcome //p' "$_o")
    if [ "$_n" -ne 1 ]; then
      why outcome "$_n outcome lines"
    elif [ "$_v" != "$_oc" ]; then
      why outcome "outcome '$_v', expected '$_oc'"
    fi
    _n=$(grep -c '^exit\.code ' "$_o")
    _c=$(sed -n 's/^exit\.code //p' "$_o")
    if [ "$_n" -ne 1 ]; then
      why exit-line "$_n exit.code lines"
    elif [ "$_c" != "$_x" ]; then
      why exit-line "exit.code '$_c', expected '$_x'"
    fi
    _n=$(grep -c '^outcome\.detail ' "$_o")
    _d=$(sed -n 's/^outcome\.detail //p' "$_o")
    if [ "$_n" -ne 1 ]; then
      why detail "$_n outcome.detail lines"
    else
      # shellcheck disable=SC2254
      case $_d in
        $_dp) ;;
        *) why detail "outcome.detail '$_d', expected '$_dp'" ;;
      esac
    fi
  fi
  [ ! -s "$tmp/why" ]
}

: > "$tmp/results"
n=0
executed=0
skipped=0
failed=0
# result <id> [<skip reason>]: appends the result of the next case from $ok and $tmp/why; with a
# skip reason a passing case is reported as a visible skip.
result() {
  n=$((n + 1))
  if [ "$ok" -ne 1 ]; then
    printf 'not ok %s %s\n' "$n" "$1" >> "$tmp/results"
    sed 's/^/# /' "$tmp/why" >> "$tmp/results"
    executed=$((executed + 1))
    failed=$((failed + 1))
  elif [ -n "${2:-}" ]; then
    printf 'ok %s %s # SKIP %s\n' "$n" "$1" "$2" >> "$tmp/results"
    skipped=$((skipped + 1))
  else
    printf 'ok %s %s\n' "$n" "$1" >> "$tmp/results"
    executed=$((executed + 1))
  fi
}

# Scenario cases.
while read -r id x oc class skip args rest; do
  case $id in '' | '#'*) continue ;; esac
  ok=1
  : > "$tmp/why"
  f="$scen/$id.ivss"
  if [ -n "$rest" ] || [ -z "$args" ]; then
    ok=0; why inventory "not six columns"
  fi
  case $class in deterministic | host-dependent) ;; *) ok=0; why inventory "class '$class'" ;; esac
  case $skip in run | skip-ok) ;; *) ok=0; why inventory "skip '$skip'" ;; esac
  [ -f "$f" ] || { ok=0; why inventory "no scenario file data/scenarios/$id.ivss"; }
  if [ "$ok" -eq 1 ]; then
    dp=$(sed -n 's/^# expect-detail: //p' "$f")
    sl=$(sed -n 's/^# expect-stderr: //p' "$f")
    [ -n "$dp" ] || dp=-
    if [ "$args" = - ]; then
      run_one "$tmp" "scen-$id" "$ivs_run" --content-dir "$content" "$f"
    else
      # Operator arguments are words without blanks joined by `,`.
      # shellcheck disable=SC2086
      set -f; IFS=,; set -- $args; IFS=$ifs0; set +f
      run_one "$tmp" "scen-$id" "$ivs_run" --content-dir "$content" "$@" "$f"
    fi
    judge "scen-$id" "$x" "$oc" "$dp" "$sl" || ok=0
    if [ -n "$rep_dir" ] && [ -s "$tmp/runs/scen-$id.out" ]; then
      cp "$tmp/runs/scen-$id.out" "$rep_dir/$id.report" || { printf '# cannot write %s\n' "$rep_dir/$id.report" >&2; exit 1; }
    fi
  fi
  if [ "$skip" = skip-ok ]; then
    result "runner-process.scenario.$id" "$id: needs operator content (ended $oc, exit $x, as expected)"
  else
    result "runner-process.scenario.$id"
  fi
done < "$inv"

{
  printf '# suite=%s nonce=%s\n' "$suite" "$nonce"
  printf '1..%s\n' "$n"
  cat "$tmp/results"
  printf '# suite=%s executed=%s skipped=%s failed=%s\n' "$suite" "$executed" "$skipped" "$failed"
} > "$tmp/tap"
cat "$tmp/tap"
if [ -n "${IVS_TEST_OUT_DIR:-}" ]; then
  cp "$tmp/tap" "$IVS_TEST_OUT_DIR/$suite.txt" || { printf '# cannot write %s\n' "$IVS_TEST_OUT_DIR/$suite.txt" >&2; exit 1; }
fi
[ "$failed" -eq 0 ] || exit 1
[ "$n" -gt 0 ] || exit 1
[ "$executed" -gt 0 ] || exit 77
exit 0

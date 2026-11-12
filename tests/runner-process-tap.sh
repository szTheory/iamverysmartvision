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
# Outcome and exit.code must also be a pair of data/expected/runner-outcomes.ids.
# Mutant cases (D-32): per line `<ID> <addr> <word> <clause>` of data/expected/diag-mutations.ids
# the diagnostic is rebuilt here with `ivs_pack --ivsc ... --patch <addr>=<word>` and
# diag-boot-pass run against it must end guest-fail (exit 10) with the detail naming <clause>; id
# runner-process.mutant.<ID>.
# Determinism (D-23, Pitfall 11): runner-process.repeat.<id> (a second run),
# runner-process.slices.<id> (--slice-ticks 1 and 7) for diag-boot-pass and diag-input-pass, and
# runner-process.cwd (diag-boot-pass from another working directory) must equal the scenario case's
# report byte for byte apart from the host.calls line.
# Judge self-controls (runner-process.judge.<slug>): the same checks run against four fake
# runners (shell scripts in the temp dir replaying this run's reports with one defect: exit 0 for
# a guest-fail, no report.end, outcome pass with exit.code 10, a changed digest line); each passes
# only when the check fails for its intended finding.
# runner-process.outcomes-consistent: `ivs-run --help`, the outcome table of docs/runner.md and
# data/expected/runner-outcomes.ids list the same pairs; runner-process.outcomes-covered: every
# pair has a running scenario and every scenario line names a pair.
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
outcomes="$root/data/expected/runner-outcomes.ids"
scen="$root/data/scenarios"
for f in "$inv" "$outcomes"; do
  [ -f "$f" ] || { printf 'runner-process-tap: no %s\n' "$f" >&2; exit 1; }
done
[ -x "$ivs_run" ] || { printf 'runner-process-tap: %s is not executable\n' "$ivs_run" >&2; exit 1; }
[ -x "$pack" ] || { printf 'runner-process-tap: %s is not executable\n' "$pack" >&2; exit 1; }
limit=${IVS_RUNNER_PROCESS_TIMEOUT:-60}
case $limit in '' | *[!0-9]* | 0) printf 'runner-process-tap: IVS_RUNNER_PROCESS_TIMEOUT=%s\n' "$limit" >&2; exit 1 ;; esac
tmp=$(mktemp -d "${TMPDIR:-/tmp}/runner-process-tap.XXXXXX") || exit 1
trap 'rm -rf "$tmp"' EXIT
trap 'exit 1' HUP INT TERM

ifs0=$IFS
# The <outcome> <exit> pairs (D-20), one per line.
awk '!/^#/ && NF { print $1, $2 }' "$outcomes" > "$tmp/pairs" || exit 1
suite=runner-process
nonce=${IVS_TEST_NONCE:-none}
rep_dir=
if [ -n "${IVS_TEST_OUT_DIR:-}" ]; then
  rep_dir="$IVS_TEST_OUT_DIR/runner"
  mkdir -p "$rep_dir" || { printf '# cannot create %s\n' "$rep_dir" >&2; exit 1; }
fi

# The content dir: copies of the build-tree images, plus diag-cpu-1-truncated.ivsc, the diagnostic
# container built here by ivs_pack and cut before its 32-byte trailer (POSIX dd).
listing="$root/fixtures/FX-diag-cpu-1/diag-cpu-1.listing.txt"
content="$tmp/content"
mkdir "$content" "$tmp/runs" "$tmp/build" || exit 1
for f in boot-stub.exec diag-cpu-1.ivsc; do
  cp "$images/$f" "$content/$f" || { printf 'runner-process-tap: no image %s/%s\n' "$images" "$f" >&2; exit 1; }
done
"$pack" --ivsc "$listing" "$tmp/build/full.ivsc" 2> "$tmp/build/pack.err" \
  || { cat "$tmp/build/pack.err" >&2; printf 'runner-process-tap: ivs_pack --ivsc failed\n' >&2; exit 1; }
size=$(wc -c < "$tmp/build/full.ivsc" | tr -d ' ')
[ "$size" -gt 32 ] || { printf 'runner-process-tap: container of %s bytes\n' "$size" >&2; exit 1; }
dd if="$tmp/build/full.ivsc" of="$content/diag-cpu-1-truncated.ivsc" bs=1 count=$((size - 32)) 2> /dev/null || exit 1

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


# why <token> <text>: records one finding in $tmp/why and sets found=1.
found=0
why() { printf '%s: %s\n' "$1" "$2" >> "$tmp/why"; found=1; }

# judge <name> <exit> <outcome> <detail pattern> <stderr line>: judges run <name> (rc, hung and
# $tmp/runs/<name>.{out,err}) against the expectation; <detail pattern> is a shell pattern for the
# outcome.detail value; a non-empty <stderr line> expects no report and exactly `ivs-run: <line>`
# on stderr. The outcome and exit.code lines must also form a pair of
# data/expected/runner-outcomes.ids. Appends findings to $tmp/why; returns 0 when it adds none.
judge() {
  _o="$tmp/runs/$1.out"; _e="$tmp/runs/$1.err"; _x=$2; _oc=$3; _dp=$4; _sl=$5
  found=0
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
    _m=$(grep -c '^exit\.code ' "$_o")
    _c=$(sed -n 's/^exit\.code //p' "$_o")
    if [ "$_m" -ne 1 ]; then
      why exit-line "$_m exit.code lines"
    elif [ "$_c" != "$_x" ]; then
      why exit-line "exit.code '$_c', expected '$_x'"
    fi
    if [ "$_n" -eq 1 ] && [ "$_m" -eq 1 ] && ! grep -q -x -F -e "$_v $_c" "$tmp/pairs"; then
      why outcome-exit-disagree "outcome '$_v' with exit.code '$_c' is no pair of runner-outcomes.ids"
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
  [ "$found" -eq 0 ]
}

# check_scenario <runner> <name> <id> <exit> <outcome> <operator-args>: runs <runner> as run
# <name> on data/scenarios/<id>.ivss with the content dir and the operator arguments (split on
# `,`; `-` for none) and judges it, the expected detail or stderr line taken from the scenario's
# `# expect-detail:` or `# expect-stderr:` comment.
check_scenario() {
  _r=$1; _nm=$2; _f="$scen/$3.ivss"; _ex=$4; _eo=$5; _args=$6
  _edp=$(sed -n 's/^# expect-detail: //p' "$_f")
  _esl=$(sed -n 's/^# expect-stderr: //p' "$_f")
  [ -n "$_edp" ] || _edp=-
  if [ "$_args" = - ]; then
    run_one "$tmp" "$_nm" "$_r" --content-dir "$content" "$_f"
  else
    # Operator arguments are words without blanks joined by `,`.
    # shellcheck disable=SC2086
    set -f; IFS=,; set -- $_args; IFS=$ifs0; set +f
    run_one "$tmp" "$_nm" "$_r" --content-dir "$content" "$@" "$_f"
  fi
  judge "$_nm" "$_ex" "$_eo" "$_edp" "$_esl"
}

# body <report> <out>: the report without its host.calls line (the one host-dependent key).
body() { grep -v '^host\.calls ' "$1" > "$2"; }
# same_body <name> <reference report>: run <name>'s report equals the reference apart from
# host.calls; records a body-differs finding otherwise.
same_body() {
  if [ ! -s "$2" ]; then
    why body-differs "no reference report"
    return 1
  fi
  body "$tmp/runs/$1.out" "$tmp/a.body"
  body "$2" "$tmp/b.body"
  if ! cmp -s "$tmp/a.body" "$tmp/b.body"; then
    why body-differs "run $1 differs from the reference report (< run, > reference):"
    diff "$tmp/a.body" "$tmp/b.body" | sed -n '1,12p' >> "$tmp/why"
    return 1
  fi
  return 0
}

# check_repeat <cwd> <runner> <name> <id> <reference> [<operator arguments>...]: one more run of
# the pass scenario <id> from <cwd>; it must pass the judge and equal <reference> apart from
# host.calls.
check_repeat() {
  _cw=$1; _r=$2; _nm=$3; _id=$4; _ref=$5; shift 5
  run_one "$_cw" "$_nm" "$_r" --content-dir "$content" "$@" "$scen/$_id.ivss"
  judge "$_nm" 0 pass - ""
  _j=$?
  same_body "$_nm" "$_ref" || _j=1
  return "$_j"
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
  if [ -n "$rest" ] || [ -z "$args" ]; then
    ok=0; why inventory "not six columns"
  fi
  case $class in deterministic | host-dependent) ;; *) ok=0; why inventory "class '$class'" ;; esac
  case $skip in run | skip-ok) ;; *) ok=0; why inventory "skip '$skip'" ;; esac
  [ -f "$scen/$id.ivss" ] || { ok=0; why inventory "no scenario file data/scenarios/$id.ivss"; }
  if [ "$ok" -eq 1 ]; then
    check_scenario "$ivs_run" "scen-$id" "$id" "$x" "$oc" "$args" || ok=0
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

# Program mutations (D-32): per line `<ID> <addr> <word> <clause>` of
# data/expected/diag-mutations.ids, the diagnostic patched at <addr> (whose listing row must be
# the <from> of <from>-<to>) must end guest-fail naming <clause>.
muts="$root/data/expected/diag-mutations.ids"
while read -r mid addr word clause; do
  case $mid in '' | '#'*) continue ;; esac
  ok=1
  : > "$tmp/why"
  from=${mid%%-*}
  # The ROW field follows the HEX4 word and the BITS groups (10 binary digits in total).
  row=$(awk -v a="$addr" '!/^#/ && $1 == a { d = 0; i = 3; while (d < 10 && i <= NF) { d += length($i); i++ } print $i; exit }' "$listing")
  md="$tmp/mut-$mid"
  if [ -z "$clause" ]; then
    ok=0; why inventory "not <ID> <addr> <word> <clause>"
  elif [ "$row" != "$from" ]; then
    ok=0; why listing "row at $addr is '$row', expected '$from'"
  elif ! mkdir "$md" || ! cp "$content/boot-stub.exec" "$md/boot-stub.exec"; then
    ok=0; why temp "cannot prepare $md"
  elif ! "$pack" --ivsc "$listing" "$md/diag-cpu-1.ivsc" --patch "$addr=$word" > "$tmp/why.pack" 2>&1; then
    ok=0; why pack "ivs_pack --patch $addr=$word failed: $(cat "$tmp/why.pack")"
  elif cmp -s "$md/diag-cpu-1.ivsc" "$content/diag-cpu-1.ivsc"; then
    ok=0; why pack "the patch $addr=$word leaves the container unchanged"
  else
    run_one "$tmp" "mut-$mid" "$ivs_run" --content-dir "$md" "$scen/diag-boot-pass.ivss"
    judge "mut-$mid" 10 guest-fail "assert=* line=* $clause got *" "" || ok=0
  fi
  result "runner-process.mutant.$mid"
done < "$muts"

# Determinism (D-23, Pitfall 11): a second run, slice sizes 1 and 7, and a run from another
# working directory all equal the scenario case's report apart from host.calls.
for id in diag-boot-pass diag-input-pass; do
  ref="$tmp/runs/scen-$id.out"
  : > "$tmp/why"
  ok=1
  check_repeat "$tmp" "$ivs_run" "rep-$id" "$id" "$ref" || ok=0
  result "runner-process.repeat.$id"
  : > "$tmp/why"
  ok=1
  for s in 1 7; do
    check_repeat "$tmp" "$ivs_run" "sl$s-$id" "$id" "$ref" --slice-ticks "$s" || ok=0
  done
  result "runner-process.slices.$id"
done
mkdir "$tmp/elsewhere" || exit 1
: > "$tmp/why"
ok=1
check_repeat "$tmp/elsewhere" "$ivs_run" cwd diag-boot-pass "$tmp/runs/scen-diag-boot-pass.out" || ok=0
result runner-process.cwd

# Judge self-controls (D-23, T-04-59): the same checks run against wrong runners, shell scripts
# in the temp dir that replay this run's recorded reports with one defect each. A control passes
# only when the check fails with a finding of its intended token.
fakes="$tmp/fakes"
mkdir "$fakes" || exit 1
cp "$tmp/runs/scen-guest-assert-mismatch.out" "$fakes/gam.report" 2> /dev/null || : > "$fakes/gam.report"
cp "$tmp/runs/scen-diag-boot-pass.out" "$fakes/dbp.report" 2> /dev/null || : > "$fakes/dbp.report"
z64=0000000000000000000000000000000000000000000000000000000000000000
{
  printf '#!/bin/sh\n# Fake ivs-run: the guest-fail report, but exit 0.\n'
  printf 'cat "$(dirname "$0")/gam.report"\nexit 0\n'
} > "$fakes/exit-mismatch.sh"
{
  printf '#!/bin/sh\n# Fake ivs-run: the guest-fail report without its report.end line.\n'
  printf 'sed '"'"'$d'"'"' "$(dirname "$0")/gam.report"\nexit 10\n'
} > "$fakes/no-report-end.sh"
{
  printf '#!/bin/sh\n# Fake ivs-run: outcome pass with exit.code 10.\n'
  printf 'sed '"'"'s/^outcome guest-fail$/outcome pass/'"'"' "$(dirname "$0")/gam.report"\nexit 10\n'
} > "$fakes/outcome-exit.sh"
{
  printf '#!/bin/sh\n# Fake ivs-run: the pass report with a different digest.cpu.\n'
  printf 'sed '"'"'s/^digest\\.cpu .*/digest.cpu %s/'"'"' "$(dirname "$0")/dbp.report"\nexit 0\n' "$z64"
} > "$fakes/body.sh"
# self_control <slug> <token> <check...>: runs the check; ok only when it fails with <token>.
self_control() {
  _slug=$1; _tok=$2; shift 2
  : > "$tmp/why"
  if "$@"; then
    ok=0
    : > "$tmp/why"
    why judge-vacuous "the check accepted the fake runner of $_slug"
  elif grep -q -e "^$_tok: " "$tmp/why"; then
    ok=1
  else
    ok=0
    { printf 'judge-wrong-reason: the check failed, but without %s:\n' "$_tok"; cat "$tmp/why"; } > "$tmp/why.2"
    mv "$tmp/why.2" "$tmp/why"
  fi
  result "runner-process.judge.$_slug"
}
self_control exit-mismatch exit-code \
  check_scenario "$fakes/exit-mismatch.sh" fake-exit guest-assert-mismatch 10 guest-fail -
self_control no-report-end report-end \
  check_scenario "$fakes/no-report-end.sh" fake-end guest-assert-mismatch 10 guest-fail -
self_control outcome-exit-disagree outcome-exit-disagree \
  check_scenario "$fakes/outcome-exit.sh" fake-pair guest-assert-mismatch 10 guest-fail -
self_control body-differs body-differs \
  check_repeat "$tmp" "$fakes/body.sh" fake-body diag-boot-pass "$tmp/runs/scen-diag-boot-pass.out"

# Outcome table anti-drift (D-20): `ivs-run --help` (the lines after its `outcomes (` line),
# the outcome table of docs/runner.md and data/expected/runner-outcomes.ids list the same
# `<outcome> <exit>` pairs in the same order.
: > "$tmp/why"
ok=1
run_one "$tmp" help "$ivs_run" --help
[ "$rc" -eq 0 ] || { ok=0; why help "ivs-run --help exit $rc"; }
sed -n '/^outcomes (/,$p' "$tmp/runs/help.out" | sed '1d' > "$tmp/help.pairs"
awk -F '|' '/^## Outcomes and exit codes/ { f = 1; next } /^## / { f = 0 }
  f && /^\| `/ { o = $2; e = $3; gsub(/[ `]/, "", o); gsub(/ /, "", e); print o, e }' \
  "$root/docs/runner.md" > "$tmp/docs.pairs"
[ -s "$tmp/pairs" ] || { ok=0; why outcomes "data/expected/runner-outcomes.ids lists no pair"; }
for src in help docs; do
  if ! cmp -s "$tmp/$src.pairs" "$tmp/pairs"; then
    ok=0
    why outcomes "$src pairs differ from runner-outcomes.ids (< $src, > inventory):"
    diff "$tmp/$src.pairs" "$tmp/pairs" >> "$tmp/why"
  fi
done
result runner-process.outcomes-consistent

# Every outcome has a demonstrating scenario that runs (skip run), and every scenario line names a
# pair of runner-outcomes.ids.
: > "$tmp/why"
ok=1
awk '!/^#/ && NF { print $3, $2, $5 }' "$inv" > "$tmp/scen.pairs"
while read -r o e; do
  grep -q -x -F -e "$o $e run" "$tmp/scen.pairs" || { ok=0; why outcomes "no running scenario for $o $e"; }
done < "$tmp/pairs"
while read -r o e s; do
  grep -q -x -F -e "$o $e" "$tmp/pairs" || { ok=0; why outcomes "scenario pair $o $e is not in runner-outcomes.ids"; }
done < "$tmp/scen.pairs"
[ -s "$tmp/pairs" ] || { ok=0; why outcomes "data/expected/runner-outcomes.ids lists no pair"; }
result runner-process.outcomes-covered

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

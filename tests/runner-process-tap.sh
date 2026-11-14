#!/bin/sh
# SPDX-License-Identifier: MIT
# Suite runner-process (D-20, D-21, D-23, plan 04-20): the real ivs-run executable, judged at the
# process level by its exit code and report text.
# Scenario cases: per line `<id> <exit> <outcome> <class> <skip> <operator-args>` of
# data/expected/runner-scenarios.ids, run
#   ivs-run --content-dir <content> <operator-args split on ,> <root>/data/scenarios/<id>.ivss
# where <content> is this run's temp copy of the build-tree images dir; id
# runner-process.scenario.<id>. The judge requires the inventory's exit code; a report whose first
# line is `ivsr 3` and whose last is `report.end <n>` (n = the lines before it, LF-terminated);
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
# runner-process.mutant.<ID>. Since plan 05-11 (D-24, V-10) a second inventory,
# data/expected/diag-mutations-ctrl-psg.ids, lists `<ID> <addr> <from> <to> <clause> <outcome> <exit>`
# for the second diagnostic (diag-ctrl-psg-pass): the <from> word is explicit and must equal the word
# at <addr> of the unpatched listing (ivs_pack --dump-words) before the patch, and the stated outcome
# (guest-fail 10, timeout 11 or pass 0) is judged; both inventories share one runner (run_mutant).
# Determinism (D-23, Pitfall 11): runner-process.repeat.<id> (a second run),
# runner-process.slices.<id> (--slice-ticks 1 and 7) for diag-boot-pass, diag-input-pass and (plan 05-11,
# cost cap V-16) diag-ctrl-psg-pass, and runner-process.cwd (diag-boot-pass from another working
# directory) and runner-process.cwd.diag-ctrl-psg-pass must equal the scenario case's report byte for
# byte apart from the host.calls line.
# Since plan 06-13 runner-process.repeat.diag-stic-fgbg-pass, runner-process.repeat.diag-stic-cstack-pass (the third diagnostic, one
# more run each) and runner-process.slices.diag-stic-fgbg-pass (--slice-ticks 1 and 7, that scenario only) are judged the same way. Its
# program mutations (data/expected/diag-mutations-stic.ids, ids runner-process.mutant.STIC-*) have a scenario column and comma-joined
# address, from and to lists, judged by the same run_mutant.
# Since plan 07-08 (D-25, D-10, D-13, A-03, A-14, A-17, W4b) the Lamp Tone mutants of data/expected/diag-mutations-scn.ids (ids
# runner-process.mutant.SCN-*) are judged by the same run_mutant, with three additions. Every mutant directory also holds
# boot-stub-2.exec (the Lamp Tone scenarios enter through the boot stub contract 2). A row whose addresses all lie in the stub window
# 1000-101F patches that image instead of the cartridge: it is rebuilt with `ivs_pack --exec <stub-2 listing> <out> --patch ADDR=HEX`
# after the same explicit from check against `ivs_pack --dump-words` of the stub listing, and the cartridge stays the unpatched copy
# (a row with addresses in both places is an inventory finding). An optional last column `fails=<k.j,...>` states the complete set of
# failing asserts: the `step.<k>.assert.<j> fail` lines of the report, in report order, must equal the list exactly (finding fails-set),
# besides the first-clause judgement of outcome.detail. A clause that ends in ` got <value>` pins the value, a clause that starts with
# `stop ` is the detail of a stop mismatch (the halt mutant).
# Since plan 07-07 (D-23, D-06, A-16, W4a) the four scenarios demo-lamp-tone-{pass,idle,shift,swap} are scenario cases like the others, and:
# runner-process.pair.<idA>.<idB>: the metamorphic judge, one case per line `<idA> <idB> differ=<keys> equal=<keys>` of
# data/expected/scenario-pairs.ids (either list may be `-`, keys are report keys such as digest.video). It reads the two reports this
# run just made (no expected value, no golden): every `differ` key has two different values, every `equal` key two equal values, and
# for each scenario of the pair `input.consumed == input.events` when it has input and `output.audio.changes 0` when it has none
# (the idle run is silent). A pair line without both reports, a missing key, a vacuous line (`-` and `-`) or an unknown scenario id
# is a finding. runner-process.repeat.demo-lamp-tone-pass, runner-process.slices.demo-lamp-tone-pass (--slice-ticks 1 and 7) and
# runner-process.cwd.demo-lamp-tone-pass repeat the pass scenario only (cost cap). Three judge self-controls with fake report pairs,
# runner-process.judge.pair-video-equal, runner-process.judge.input-not-consumed and runner-process.judge.idle-audible, pass only when
# the pair judge rejects the fake for its stated token (pair-not-different, input-not-consumed, idle-audible).
# Since plan 07-11 (D-31, T-07-26 to T-07-29): runner-process.refuse-user.audio-raw, .psg-log-out and .audio-wav (each option refused for a
# scenario with a `content ... user` line: exit 14, the one line `ivs-run: <option> refused: content user`, no report, no file) and the nine
# runner-process.owner-run.* cases of the owner-local helper tools/owner-run.sh (refusal, skip and identity cases on a copy of the helper in a
# scratch root outside the work tree, project-made zero-filled files only; see the comment above them), with the judge self-control
# runner-process.judge.owner-run-refusal (a fake helper that exits 0 where it must refuse is rejected).
# Judge self-controls (runner-process.judge.<slug>): the same checks run against four fake
# runners (shell scripts in the temp dir replaying this run's reports with one defect: exit 0 for
# a guest-fail, no report.end, outcome pass with exit.code 10, a changed digest line); each passes
# only when the check fails for its intended finding. runner-process.judge.outcome-uncovered (plan
# 04-24, D-46) runs the outcomes-covered check with the first outcome's scenarios removed and
# passes only when it fails with outcome-uncovered.
# runner-process.watchdog (watchdog self-control): watchdogs stopped at once stay silent, a quick run
# is not hung and a run past a 1-second limit is (see wd_start), and a watchdog that dies before it arms
# or never arms ends the arming wait as a bounded failure, not a hang (review WR-03).
# runner-process.outcomes-consistent: `ivs-run --help`, the outcome table of docs/runner.md and
# data/expected/runner-outcomes.ids list the same pairs; runner-process.outcomes-covered: every
# pair has a running scenario and every scenario line names a pair.
# Reports: when IVS_TEST_OUT_DIR is set, every scenario case that printed a report leaves it in
# <dir>/runner/<id>.report (the golden step compares dev with release there). Since plan 05-11 the case
# runner-process.refpcm.<id> of each line of data/expected/refpcm-scenarios.ids also leaves the report of its
# --audio-wav run in <dir>/runner/<id>.refpcm.report (the golden step compares the .refpcm golden with it).
# Output: the harness TAP shape (header with IVS_TEST_NONCE or none, plan, one result per case
# with the findings as `#` lines after a failure, trailer) on stdout and, when IVS_TEST_OUT_DIR is
# set, in <dir>/runner-process.txt.
# Since plan 08-07 (D-10, D-37, D-40): runner-process.mutant.STIC2-*, one case per row of data/expected/diag-mutations-stic2.ids (six one-word
# mutants of the program ivs-diag-stic-2, the same row grammar as the SCN rows, a `fails=` set on every row), in a sibling loop of the SCN loop.
# The optional fifth argument is the build configuration (`$<CONFIG>` of tests/CMakeLists.txt). With `Release` each row builds the patched
# cartridge, runs its scenario and judges outcome, first clause and fails set like the SCN rows. With any other value (the sanitized Debug
# preset) or none, each row performs its static checks only (the word at every address of the unpatched listing equals its `from`, the three
# lists have one length, the patch changes the image, the scenario file exists, `fails=` is well formed) and is reported ok without running the
# scenario: the ids are the same in both presets, no id is skip-ok and tests-skipped does not move (D-40).
# Usage: sh runner-process-tap.sh <source root> <ivs-run> <ivs_pack> <images dir> [<build configuration>]
# Exit: 0 pass, 1 fail, 77 every case skipped.
# Env: IVS_TEST_NONCE, IVS_TEST_OUT_DIR, IVS_RUNNER_PROCESS_TIMEOUT.
LC_ALL=C; export LC_ALL
usage='usage: sh runner-process-tap.sh <source root> <ivs-run> <ivs_pack> <images dir> [<build configuration>]'
[ "$#" -eq 4 ] || [ "$#" -eq 5 ] || { printf '%s\n' "$usage" >&2; exit 1; }
config=${5:-}
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

# The content dir: copies of the build-tree images (boot stub, since plan 07-05 boot-stub-2, diag-cpu-1, since plan 05-11 diag-ctrl-psg-1, since plan 06-13 diag-stic-1, since plan 07-06 lamp-tone, and since plan 08-07 diag-stic-2; the four demo-lamp-tone scenarios of plan 07-07 use lamp-tone and boot-stub-2, the six diag-stic2 scenarios of plan 08-07 use diag-stic-2 and boot-stub-2),
# plus diag-cpu-1-truncated.ivsc, the diagnostic
# container built here by ivs_pack and cut before its 32-byte trailer (POSIX dd).
listing="$root/fixtures/FX-diag-cpu-1/diag-cpu-1.listing.txt"
content="$tmp/content"
mkdir "$content" "$tmp/runs" "$tmp/build" || exit 1
for f in boot-stub.exec boot-stub-2.exec diag-cpu-1.ivsc diag-ctrl-psg-1.ivsc diag-stic-1.ivsc lamp-tone.ivsc diag-stic-2.ivsc; do
  cp "$images/$f" "$content/$f" || { printf 'runner-process-tap: no image %s/%s\n' "$images" "$f" >&2; exit 1; }
done
"$pack" --ivsc "$listing" "$tmp/build/full.ivsc" 2> "$tmp/build/pack.err" \
  || { cat "$tmp/build/pack.err" >&2; printf 'runner-process-tap: ivs_pack --ivsc failed\n' >&2; exit 1; }
size=$(wc -c < "$tmp/build/full.ivsc" | tr -d ' ')
[ "$size" -gt 32 ] || { printf 'runner-process-tap: container of %s bytes\n' "$size" >&2; exit 1; }
dd if="$tmp/build/full.ivsc" of="$content/diag-cpu-1-truncated.ivsc" bs=1 count=$((size - 32)) 2> /dev/null || exit 1

# wd_child <pid> <mark>: the watchdog, run by wd_start in a background subshell: it sets its TERM trap,
# starts its sleep, and only then creates <mark>.armed (the arming signal); after $limit seconds it writes
# <mark> and sends <pid> TERM. A function, so the self-control below can replace it by a watchdog that
# never arms.
wd_child() {
  _s=
  trap 'if [ -n "$_s" ]; then kill "$_s" 2> /dev/null; fi; exit 0' TERM
  sleep "$limit" &
  _s=$!
  : > "$2.armed"
  wait "$_s"
  : > "$2"
  kill -TERM "$1" 2> /dev/null
}
# wd_start <pid> <mark>: starts the watchdog of process <pid> (a background subshell, its pid in _wd).
# Returns 0 only once the watchdog is armed (its TERM trap set and its sleep started), reported by the
# file <mark>.armed, so wd_stop never signals it earlier. Without that handshake a stop that follows at
# once (a run that ends in milliseconds) can reach the fresh subshell while it still has this script's
# inherited `exit 1` TERM handler: dash (/bin/sh on Linux) records the signal, resets that handler and
# then drops the signal, so the watchdog ran to its limit and judged a finished run `hung` (hosted run
# 43148343848, runner-process.scenario.dead-after-refusal). The wait is a bounded poll (review WR-03): at
# once, then every 0.01 s, for at most wd_polls polls (500, about 5 s) and never past the death of the
# watchdog; then it returns 1 (not armed), which is a finding and never a hang. An earlier form read a
# FIFO that the parent itself held open read-write, so a watchdog that died before it wrote left the read
# blocked forever, and its retry counter counted only failed reads.
# wd_stop: stops the watchdog started last (its sleep too) and waits for it.
wd_polls=500
wd_start() {
  _wp=$1; _wm=$2
  rm -f "$_wm" "$_wm.armed"
  ( wd_child "$_wp" "$_wm" ) > /dev/null 2>&1 < /dev/null &
  _wd=$!
  _wa=1
  _n=0
  while [ ! -e "$_wm.armed" ]; do
    if [ "$_n" -ge "$wd_polls" ] || ! kill -0 "$_wd" 2> /dev/null; then
      # A watchdog that armed and ended in the last instant still counts as armed.
      [ -e "$_wm.armed" ] || _wa=0
      break
    fi
    _n=$((_n + 1))
    sleep 0.01
  done
  rm -f "$_wm.armed"
  [ "$_wa" -eq 1 ]
}
wd_stop() {
  kill -TERM "$_wd" 2> /dev/null
  wait "$_wd" 2> /dev/null
}

# run_one <cwd> <name> <command...>: runs the command in <cwd> with stdin /dev/null, stdout in
# $tmp/runs/<name>.out and stderr in $tmp/runs/<name>.err; sets rc to its exit status and hung to
# 1 when the watchdog stopped it after $limit seconds. A command ending in .sh is run with sh.
run_one() {
  _cwd=$1; _name=$2; shift 2
  case $1 in *.sh) set -- sh "$@" ;; esac
  _mark="$tmp/runs/$_name.hung"
  (cd "$_cwd" && exec "$@") > "$tmp/runs/$_name.out" 2> "$tmp/runs/$_name.err" < /dev/null &
  _pid=$!
  wdbad=0
  if ! wd_start "$_pid" "$_mark"; then
    # No armed watchdog: the run is stopped so nothing can hang, and judge reports the finding.
    wdbad=1
    kill -TERM "$_pid" 2> /dev/null
  fi
  wait "$_pid"
  rc=$?
  wd_stop
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
  if [ "$wdbad" -eq 1 ]; then why watchdog-unarmed "the watchdog did not arm; the run was stopped"; fi
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
    [ "$_first" = "ivsr 3" ] || why header "first line '$_first', expected 'ivsr 3'"
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

# rkey <report> <key>: the value of the first line `<key> <value>` of a report ('' when the key is absent).
rkey() { awk -v k="$2" '$1 == k { print $2; exit }' "$1"; }

# check_pair <idA> <idB> <reportA> <reportB> <differ list> <equal list> (plan 07-07, D-23, A-16): the metamorphic judge of one line of
# data/expected/scenario-pairs.ids on two reports of this run. Every key of the comma-joined differ list has two different values
# (pair-not-different), every key of the equal list two equal values (pair-not-equal), a key absent from a report is pair-missing-key and
# a missing or empty report is pair-no-report; for each scenario of the pair, input.consumed must equal input.events when there is input
# (input-not-consumed) and a scenario with no input must deliver output.audio.changes 0 (idle-audible). Appends findings to $tmp/why;
# returns 0 when it adds none.
check_pair() {
  _pa=$1; _pb=$2; _ra=$3; _rb=$4; _pd=$5; _pe=$6
  found=0
  if [ ! -s "$_ra" ] || [ ! -s "$_rb" ]; then
    why pair-no-report "no report for $_pa or $_pb"
    return 1
  fi
  for _kind in differ equal; do
    if [ "$_kind" = differ ]; then _kl=$_pd; else _kl=$_pe; fi
    [ "$_kl" != - ] || continue
    set -f; IFS=,; set -- $_kl; IFS=$ifs0; set +f
    for _k in "$@"; do
      _va=$(rkey "$_ra" "$_k")
      _vb=$(rkey "$_rb" "$_k")
      if [ -z "$_va" ] || [ -z "$_vb" ]; then
        why pair-missing-key "$_k is absent from the report of $_pa or $_pb"
      elif [ "$_kind" = differ ] && [ "$_va" = "$_vb" ]; then
        why pair-not-different "$_k is equal in $_pa and $_pb ($_va)"
      elif [ "$_kind" = equal ] && [ "$_va" != "$_vb" ]; then
        why pair-not-equal "$_k differs between $_pa and $_pb"
      fi
    done
  done
  for _sp in "$_pa:$_ra" "$_pb:$_rb"; do
    _sid=${_sp%%:*}; _sr=${_sp#*:}
    _ev=$(rkey "$_sr" input.events)
    _co=$(rkey "$_sr" input.consumed)
    _ch=$(rkey "$_sr" output.audio.changes)
    if [ -z "$_ev" ] || [ -z "$_co" ] || [ -z "$_ch" ]; then
      why pair-missing-key "input.events, input.consumed or output.audio.changes is absent from the report of $_sid"
    elif [ "$_ev" != 0 ] && [ "$_co" != "$_ev" ]; then
      why input-not-consumed "$_sid: input.consumed $_co, input.events $_ev"
    elif [ "$_ev" = 0 ] && [ "$_ch" != 0 ]; then
      why idle-audible "$_sid has no input but output.audio.changes is $_ch"
    fi
  done
  [ "$found" -eq 0 ]
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

# patch_args <addr list> <word list>: one `--patch ADDR=HEX` per element of the two comma-joined lists (a single word is a list of
# one; tokens hold only hex digits, so the unquoted use of the output in run_mutant splits it safely).
patch_args() {
  awk -v a="$1" -v w="$2" 'BEGIN { n = split(a, A, ","); split(w, W, ","); for (i = 1; i <= n; i++) printf "--patch %s=%s ", A[i], W[i] }'
}

# run_mutant <id> <listing> <cart file name> <scenario id> <addr> <to> <exit> <outcome> <detail pattern> [stub]:
# rebuilds the cartridge from <listing> in a fresh directory with the words patched, runs the
# scenario against it and judges the stated exit code, outcome and detail. Sets $ok and $ran (1 once
# the scenario was run), adds to $tmp/why. With a tenth argument `stub` <listing> is the listing of the
# boot stub contract 2 and the patched image is boot-stub-2.exec, rebuilt with `ivs_pack --exec` (plan
# 07-08, A-14, A-17), while the cartridge <cart file name> stays the unpatched copy; otherwise the
# cartridge is rebuilt and boot-stub-2.exec is the unpatched copy (the directory always holds both stubs).
run_mutant() {
  _mid=$1; _ml=$2; _mc=$3; _ms=$4; _ma=$5; _mw=$6; _mx=$7; _mo=$8; _mp=$9
  ran=0
  if [ "${10:-cart}" = stub ]; then
    _mimg=boot-stub-2.exec; _mflag=--exec; _mkeep=$_mc
  else
    _mimg=$_mc; _mflag=--ivsc; _mkeep=boot-stub-2.exec
  fi
  md="$tmp/mut-$_mid"
  if ! mkdir "$md" || ! cp "$content/boot-stub.exec" "$md/boot-stub.exec" || ! cp "$content/$_mkeep" "$md/$_mkeep"; then
    ok=0; why temp "cannot prepare $md"
  elif ! "$pack" "$_mflag" "$_ml" "$md/$_mimg" $(patch_args "$_ma" "$_mw") > "$tmp/why.pack" 2>&1; then
    ok=0; why pack "ivs_pack --patch $_ma=$_mw failed: $(cat "$tmp/why.pack")"
  elif cmp -s "$md/$_mimg" "$content/$_mimg"; then
    ok=0; why pack "the patch $_ma=$_mw leaves the image unchanged"
  else
    run_one "$tmp" "mut-$_mid" "$ivs_run" --content-dir "$md" "$scen/$_ms.ivss"
    ran=1
    judge "mut-$_mid" "$_mx" "$_mo" "$_mp" "" || ok=0
  fi
}

# check_fails <name> <list> (plan 07-08, D-25): the asserts that run <name> reported `fail`, as the comma-joined `<step>.<assert>` of
# the lines `step.<k>.assert.<j> fail ...` of its report in report order, equal <list> exactly. Appends a fails-set finding and returns
# 1 otherwise (also for a report that is missing or has no failing assert).
check_fails() {
  _fg=$(awk '$1 ~ /^step[.][0-9]+[.]assert[.][0-9]+$/ && $2 == "fail" { split($1, P, "."); printf "%s%s.%s", (n++ ? "," : ""), P[2], P[4] }' "$tmp/runs/$1.out" 2> /dev/null)
  if [ "$_fg" != "$2" ]; then
    why fails-set "the failing asserts are '$_fg', expected '$2'"
    return 1
  fi
  return 0
}

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
  if [ -z "$clause" ]; then
    ok=0; why inventory "not <ID> <addr> <word> <clause>"
  elif [ "$row" != "$from" ]; then
    ok=0; why listing "row at $addr is '$row', expected '$from'"
  else
    run_mutant "$mid" "$listing" diag-cpu-1.ivsc diag-boot-pass "$addr" "$word" 10 guest-fail "assert=* line=* $clause got *"
  fi
  result "runner-process.mutant.$mid"
done < "$muts"

# Program mutations of the second diagnostic (plan 05-11, D-24, V-10): per line
# `<ID> <addr> <from> <to> <clause> <outcome> <exit>` of data/expected/diag-mutations-ctrl-psg.ids; the
# word at <addr> of the unpatched listing (ivs_pack --dump-words, `ADDR WORD` lines) must be <from>.
listing2="$root/fixtures/FX-diag-ctrl-psg-1/diag-ctrl-psg-1.listing.txt"
muts2="$root/data/expected/diag-mutations-ctrl-psg.ids"
"$pack" --dump-words "$listing2" > "$tmp/words2" 2> /dev/null || : > "$tmp/words2"
while read -r mid addr from to rest; do
  case $mid in '' | '#'*) continue ;; esac
  ok=1
  : > "$tmp/why"
  # The last two fields are the outcome and the exit code; the clause is what lies between.
  mx=${rest##* }
  rest=${rest% *}
  mo=${rest##* }
  clause=${rest% *}
  have=$(awk -v a="$addr" '$1 == a { print $2; exit }' "$tmp/words2")
  if [ -z "$mx" ] || [ -z "$mo" ] || [ -z "$clause" ] || [ "$clause" = "$rest" ]; then
    ok=0; why inventory "not <ID> <addr> <from> <to> <clause> <outcome> <exit>"
  elif [ "$have" != "$from" ]; then
    ok=0; why listing "the word at $addr is '$have', expected from '$from'"
  else
    case $mo in
      guest-fail) mp="assert=* line=* $clause got *" ;;
      timeout) mp="budget step=* line=* expected=halted got=target" ;;
      pass) mp=- ;;
      *) mp=; ok=0; why inventory "outcome '$mo' is not guest-fail, timeout or pass" ;;
    esac
    [ "$ok" -eq 1 ] && run_mutant "$mid" "$listing2" diag-ctrl-psg-1.ivsc diag-ctrl-psg-pass "$addr" "$to" "$mx" "$mo" "$mp"
  fi
  result "runner-process.mutant.$mid"
done < "$muts2"

# Program mutations of the third diagnostic (plan 06-13, D-20, V6-9, V6-12): per line
# `<ID> <scenario> <addr list> <from list> <to list> <clause> <outcome> <exit>` of data/expected/diag-mutations-stic.ids (the lists are
# comma-joined); the word at each <addr> of the unpatched listing (ivs_pack --dump-words) must be its <from>, and the stated outcome
# (guest-fail 10, timeout 11 or pass 0) of the named scenario is judged.
listing3="$root/fixtures/FX-diag-stic-1/diag-stic-1.listing.txt"
muts3="$root/data/expected/diag-mutations-stic.ids"
"$pack" --dump-words "$listing3" > "$tmp/words3" 2> /dev/null || : > "$tmp/words3"
while read -r mid scn addrs froms tos rest; do
  case $mid in '' | '#'*) continue ;; esac
  ok=1
  : > "$tmp/why"
  mx=${rest##* }
  rest=${rest% *}
  mo=${rest##* }
  clause=${rest% *}
  # each address word of the unpatched listing equals its from word, and the three lists have one length
  bad=$(awk -v a="$addrs" -v f="$froms" -v t="$tos" '
    NR == FNR { w[$1] = $2; next }
    END {
      na = split(a, A, ","); nf = split(f, F, ","); nt = split(t, T, ",")
      if (na != nf || na != nt) { print "lists of different lengths"; exit }
      for (i = 1; i <= na; i++) if (w[A[i]] != F[i]) print "the word at " A[i] " is " w[A[i]] ", expected from " F[i]
    }' "$tmp/words3" /dev/null)
  if [ -z "$mx" ] || [ -z "$mo" ] || [ -z "$clause" ] || [ "$clause" = "$rest" ]; then
    ok=0; why inventory "not <ID> <scenario> <addr list> <from list> <to list> <clause> <outcome> <exit>"
  elif [ ! -f "$scen/$scn.ivss" ]; then
    ok=0; why inventory "no scenario file data/scenarios/$scn.ivss"
  elif [ -n "$bad" ]; then
    ok=0; why listing "$bad"
  else
    case $mo in
      guest-fail) mp="assert=* line=* $clause got *" ;;
      timeout) mp="budget step=* line=* expected=halted got=target" ;;
      pass) mp=- ;;
      *) mp=; ok=0; why inventory "outcome '$mo' is not guest-fail, timeout or pass" ;;
    esac
    [ "$ok" -eq 1 ] && run_mutant "$mid" "$listing3" diag-stic-1.ivsc "$scn" "$addrs" "$tos" "$mx" "$mo" "$mp"
  fi
  result "runner-process.mutant.$mid"
done < "$muts3"

# Program mutations of the first original program (plan 07-08, D-25, D-10, D-13, A-14, A-17): per line
# `<ID> <scenario> <addr list> <from list> <to list> <clause> <outcome> <exit> [fails=<k.j,...>]` of data/expected/diag-mutations-scn.ids
# (the lists are comma-joined). The word at each <addr> of the unpatched listing (ivs_pack --dump-words) must be its <from>: of the Lamp Tone
# listing for a program address, of the boot stub contract 2 listing when every address lies in the stub window 1000-101F (a row with
# addresses in both is an inventory finding). The stated outcome (guest-fail 10, timeout 11 or pass 0) of the named scenario is judged
# against outcome.detail; with `fails=` the complete set of failing asserts is judged too (check_fails).
listing4="$root/fixtures/FX-demo-lamp-tone-1/lamp-tone.listing.txt"
listing4s="$root/fixtures/FX-boot-stub-2/boot-stub-2.listing.txt"
muts4="$root/data/expected/diag-mutations-scn.ids"
"$pack" --dump-words "$listing4" > "$tmp/words4" 2> /dev/null || : > "$tmp/words4"
"$pack" --dump-words "$listing4s" > "$tmp/words4s" 2> /dev/null || : > "$tmp/words4s"
while read -r mid scn addrs froms tos rest; do
  case $mid in '' | '#'*) continue ;; esac
  ok=1
  ran=0
  : > "$tmp/why"
  fails=
  case ${rest##* } in
    fails=*) fails=${rest##* }; rest=${rest% *}; fails=${fails#fails=} ;;
  esac
  mx=${rest##* }
  rest=${rest% *}
  mo=${rest##* }
  clause=${rest% *}
  # the image the row patches: stub when every address lies in 1000-101F, cart when none does, mixed otherwise
  win=$(awk -v a="$addrs" 'BEGIN { n = split(a, A, ","); s = 0; c = 0
    for (i = 1; i <= n; i++) { if (A[i] >= "1000" && A[i] <= "101F") s++; else c++ }
    print (s > 0 && c > 0) ? "mixed" : ((s > 0) ? "stub" : "cart") }')
  if [ "$win" = stub ]; then wfile="$tmp/words4s"; else wfile="$tmp/words4"; fi
  # each address word of the unpatched listing equals its from word, and the three lists have one length
  bad=$(awk -v a="$addrs" -v f="$froms" -v t="$tos" '
    NR == FNR { w[$1] = $2; next }
    END {
      na = split(a, A, ","); nf = split(f, F, ","); nt = split(t, T, ",")
      if (na != nf || na != nt) { print "lists of different lengths"; exit }
      for (i = 1; i <= na; i++) if (w[A[i]] != F[i]) print "the word at " A[i] " is " w[A[i]] ", expected from " F[i]
    }' "$wfile" /dev/null)
  fbad=$(printf '%s\n' "$fails" | awk 'NF && $0 !~ /^[0-9]+[.][0-9]+(,[0-9]+[.][0-9]+)*$/ { print "bad" }')
  if [ -z "$mx" ] || [ -z "$mo" ] || [ -z "$clause" ] || [ "$clause" = "$rest" ]; then
    ok=0; why inventory "not <ID> <scenario> <addr list> <from list> <to list> <clause> <outcome> <exit> [fails=<k.j,...>]"
  elif [ ! -f "$scen/$scn.ivss" ]; then
    ok=0; why inventory "no scenario file data/scenarios/$scn.ivss"
  elif [ -n "$fbad" ]; then
    ok=0; why inventory "fails=$fails is not a comma-joined list of <step>.<assert>"
  elif [ -n "$fails" ] && [ "$mo" != guest-fail ]; then
    ok=0; why inventory "fails= needs the outcome guest-fail"
  elif [ "$win" = mixed ]; then
    ok=0; why inventory "addresses $addrs lie in both the boot stub window 1000-101F and the program"
  elif [ -n "$bad" ]; then
    ok=0; why listing "$bad"
  else
    case $mo in
      guest-fail)
        case $clause in
          'stop '*) mp=$clause ;;
          *' got '*) mp="assert=* line=* $clause" ;;
          *) mp="assert=* line=* $clause got *" ;;
        esac ;;
      timeout) mp="budget step=* line=* expected=halted got=target" ;;
      pass) mp=- ;;
      *) mp=; ok=0; why inventory "outcome '$mo' is not guest-fail, timeout or pass" ;;
    esac
    if [ "$ok" -eq 1 ]; then
      if [ "$win" = stub ]; then
        run_mutant "$mid" "$listing4s" lamp-tone.ivsc "$scn" "$addrs" "$tos" "$mx" "$mo" "$mp" stub
      else
        run_mutant "$mid" "$listing4" lamp-tone.ivsc "$scn" "$addrs" "$tos" "$mx" "$mo" "$mp"
      fi
      if [ "$ran" -eq 1 ] && [ -n "$fails" ]; then check_fails "mut-$mid" "$fails" || ok=0; fi
    fi
  fi
  result "runner-process.mutant.$mid"
done < "$muts4"

# Program mutations of the second original program (plan 08-07, D-10, D-37, D-40): per line
# `<ID> <scenario> <addr list> <from list> <to list> <clause> <outcome> <exit> [fails=<k.j,...>]` of data/expected/diag-mutations-stic2.ids, the
# grammar of the scn inventory above (a sibling loop of it: the stic and scn loops are not edited). The word at each <addr> of the unpatched
# listing of ivs-diag-stic-2 (ivs_pack --dump-words) must be its <from>. When the fifth argument of this script, the build configuration, is
# Release the row is run and judged like a scn row (outcome and exit code of the named scenario against outcome.detail, then with `fails=` the
# complete set of failing asserts, check_fails). In any other configuration the row is judged statically (patch_changes_image and the checks
# above) and reported ok without running the scenario; the ids and their number are the same in both presets, and none is skip-ok.
# patch_changes_image <id> <listing> <cart file name> <addr list> <word list>: the static half of run_mutant: builds the patched cartridge in a
# fresh directory and requires it to differ from the unpatched copy in the content dir; nothing is run. Sets $ok, adds to $tmp/why.
patch_changes_image() {
  _pd="$tmp/mutstat-$1"
  if ! mkdir "$_pd"; then
    ok=0; why temp "cannot prepare $_pd"
  elif ! "$pack" --ivsc "$2" "$_pd/$3" $(patch_args "$4" "$5") > "$tmp/why.pack" 2>&1; then
    ok=0; why pack "ivs_pack --patch $4=$5 failed: $(cat "$tmp/why.pack")"
  elif cmp -s "$_pd/$3" "$content/$3"; then
    ok=0; why pack "the patch $4=$5 leaves the image unchanged"
  fi
}
listing5="$root/fixtures/FX-diag-stic-2/diag-stic-2.listing.txt"
muts5="$root/data/expected/diag-mutations-stic2.ids"
"$pack" --dump-words "$listing5" > "$tmp/words5" 2> /dev/null || : > "$tmp/words5"
while read -r mid scn addrs froms tos rest; do
  case $mid in '' | '#'*) continue ;; esac
  ok=1
  ran=0
  : > "$tmp/why"
  fails=
  case ${rest##* } in
    fails=*) fails=${rest##* }; rest=${rest% *}; fails=${fails#fails=} ;;
  esac
  mx=${rest##* }
  rest=${rest% *}
  mo=${rest##* }
  clause=${rest% *}
  # each address word of the unpatched listing equals its from word, and the three lists have one length
  bad=$(awk -v a="$addrs" -v f="$froms" -v t="$tos" '
    NR == FNR { w[$1] = $2; next }
    END {
      na = split(a, A, ","); nf = split(f, F, ","); nt = split(t, T, ",")
      if (na != nf || na != nt) { print "lists of different lengths"; exit }
      for (i = 1; i <= na; i++) if (w[A[i]] != F[i]) print "the word at " A[i] " is " w[A[i]] ", expected from " F[i]
    }' "$tmp/words5" /dev/null)
  fbad=$(printf '%s\n' "$fails" | awk 'NF && $0 !~ /^[0-9]+[.][0-9]+(,[0-9]+[.][0-9]+)*$/ { print "bad" }')
  if [ -z "$mx" ] || [ -z "$mo" ] || [ -z "$clause" ] || [ "$clause" = "$rest" ]; then
    ok=0; why inventory "not <ID> <scenario> <addr list> <from list> <to list> <clause> <outcome> <exit> [fails=<k.j,...>]"
  elif [ ! -f "$scen/$scn.ivss" ]; then
    ok=0; why inventory "no scenario file data/scenarios/$scn.ivss"
  elif [ -n "$fbad" ]; then
    ok=0; why inventory "fails=$fails is not a comma-joined list of <step>.<assert>"
  elif [ -n "$fails" ] && [ "$mo" != guest-fail ]; then
    ok=0; why inventory "fails= needs the outcome guest-fail"
  elif [ -n "$bad" ]; then
    ok=0; why listing "$bad"
  else
    case $mo in
      guest-fail)
        case $clause in
          'stop '*) mp=$clause ;;
          *' got '*) mp="assert=* line=* $clause" ;;
          *) mp="assert=* line=* $clause got *" ;;
        esac ;;
      timeout) mp="budget step=* line=* expected=halted got=target" ;;
      pass) mp=- ;;
      *) mp=; ok=0; why inventory "outcome '$mo' is not guest-fail, timeout or pass" ;;
    esac
    if [ "$ok" -eq 1 ]; then
      if [ "$config" = Release ]; then
        run_mutant "$mid" "$listing5" diag-stic-2.ivsc "$scn" "$addrs" "$tos" "$mx" "$mo" "$mp"
        if [ "$ran" -eq 1 ] && [ -n "$fails" ]; then check_fails "mut-$mid" "$fails" || ok=0; fi
      else
        patch_changes_image "$mid" "$listing5" diag-stic-2.ivsc "$addrs" "$tos"
      fi
    fi
  fi
  result "runner-process.mutant.$mid"
done < "$muts5"

# Determinism (D-23, Pitfall 11): a second run, slice sizes 1 and 7, and a run from another
# working directory all equal the scenario case's report apart from host.calls.
for id in diag-boot-pass diag-input-pass diag-ctrl-psg-pass; do
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
# The third diagnostic (plan 06-13, cost cap V6-12): a second run of each of its two pass scenarios, and the slice replays
# (--slice-ticks 1 and 7) of the Foreground/Background scenario ONLY (about 4,100 slices at 1 tick, at most 150,000 ticks).
for id in diag-stic-fgbg-pass diag-stic-cstack-pass; do
  ref="$tmp/runs/scen-$id.out"
  : > "$tmp/why"
  ok=1
  check_repeat "$tmp" "$ivs_run" "rep-$id" "$id" "$ref" || ok=0
  result "runner-process.repeat.$id"
done
: > "$tmp/why"
ok=1
for s in 1 7; do
  check_repeat "$tmp" "$ivs_run" "sl$s-diag-stic-fgbg-pass" diag-stic-fgbg-pass "$tmp/runs/scen-diag-stic-fgbg-pass.out" --slice-ticks "$s" || ok=0
done
result runner-process.slices.diag-stic-fgbg-pass
mkdir "$tmp/elsewhere" || exit 1
: > "$tmp/why"
ok=1
check_repeat "$tmp/elsewhere" "$ivs_run" cwd diag-boot-pass "$tmp/runs/scen-diag-boot-pass.out" || ok=0
result runner-process.cwd
: > "$tmp/why"
ok=1
check_repeat "$tmp/elsewhere" "$ivs_run" cwd2 diag-ctrl-psg-pass "$tmp/runs/scen-diag-ctrl-psg-pass.out" || ok=0
result runner-process.cwd.diag-ctrl-psg-pass

# The Lamp Tone pass scenario (plan 07-07, D-23, cost cap: that scenario only): a second run, the slice replays (--slice-ticks 1 and 7)
# and a run from another working directory equal the scenario case's report apart from host.calls.
id=demo-lamp-tone-pass
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
: > "$tmp/why"
ok=1
check_repeat "$tmp/elsewhere" "$ivs_run" "cwd3" "$id" "$ref" || ok=0
result "runner-process.cwd.$id"

# The metamorphic judge (plan 07-07, D-23, D-06, A-16): per line `<idA> <idB> differ=<keys> equal=<keys>` of
# data/expected/scenario-pairs.ids, the reports of the two scenario cases above are related to each other (no expected value).
pairs="$root/data/expected/scenario-pairs.ids"
while read -r pa pb pdiff peq prest; do
  case $pa in '' | '#'*) continue ;; esac
  ok=1
  : > "$tmp/why"
  pdl=; pel=
  case $pdiff in differ=*) pdl=${pdiff#differ=} ;; *) ok=0; why inventory "third column is not differ=<keys>" ;; esac
  case $peq in equal=*) pel=${peq#equal=} ;; *) ok=0; why inventory "fourth column is not equal=<keys>" ;; esac
  [ -z "$prest" ] || { ok=0; why inventory "more than four columns"; }
  for _l in "$pdl" "$pel"; do
    case $_l in
      -) ;;
      '' | *[!a-z0-9.,]* | ,* | *, | *,,*) ok=0; why inventory "key list '$_l' is not - or comma-joined report keys" ;;
    esac
  done
  [ "$pdl$pel" != "--" ] || { ok=0; why inventory "differ=- equal=- judges nothing"; }
  for _i in "$pa" "$pb"; do
    awk -v i="$_i" '$1 == i && $4 == "deterministic" { f = 1 } END { exit !f }' "$inv" \
      || { ok=0; why inventory "scenario $_i is not a deterministic line of data/expected/runner-scenarios.ids"; }
  done
  [ "$pa" != "$pb" ] || { ok=0; why inventory "a pair of one scenario"; }
  if [ "$ok" -eq 1 ]; then
    check_pair "$pa" "$pb" "$tmp/runs/scen-$pa.out" "$tmp/runs/scen-$pb.out" "$pdl" "$pel" || ok=0
  fi
  result "runner-process.pair.$pa.$pb"
done < "$pairs"

# Operator-only raw outputs (plan 05-09, D-20, T-05-23): a project-authored EXEC stand-in (nine words
# at $1000: mixer R7 = $3F, then amplitude A = $0F, $00, $07, then a branch to itself; encodings as
# in tests/runner/runner_exec.c) run to tick 3000 under --audio-raw and --psg-log-out into a mktemp
# directory (never the repository). By hand: four PSG writes at ticks 76, 152, 228 and 304, so the
# log is `0 76 8 63 1` and three more lines; a sample at tick 16 k sees the writes stamped below it,
# so sample 10 (byte offset 36, tick 160) is level A = 15: bytes 0f 00 00 00, and sample 1 is silent.
# Each case runs once per preset (no slicing variants); the files are created exclusively.
opc="$tmp/opcontent"
opo="$tmp/opout"
mkdir "$opc" "$opo" || exit 1
{
  printf '\002\270\000\077\002\100\001\370\002\270\000\017\002\100\001\373'
  printf '\002\270\000\000\002\100\001\373\002\270\000\007\002\100\001\373\002\040\000\001'
  dd if=/dev/zero bs=8156 count=1 2> /dev/null
} > "$opc/psg-prog.exec"
printf 'ivss 1\nid operator-psg\nprofile intv-2609-ntsc-family-p1\ncontent exec psg-prog.exec\ncontent grom zero-grom\nrun-to 3000 ticks\n' > "$opc/operator-psg.ivss"
opscn="$opc/operator-psg.ivss"
# op_value <name> <key>: the value of a report line of run <name>.
op_value() { sed -n "s/^$2 //p" "$tmp/runs/$1.out"; }
: > "$tmp/why"
ok=1
run_one "$tmp" op-audio "$ivs_run" --content-dir "$opc" --audio-raw "$opo/audio.raw" "$opscn"
judge op-audio 0 pass - "" || ok=0
sz=$(wc -c < "$opo/audio.raw" 2> /dev/null | tr -d ' ')
[ -n "$sz" ] && [ "$sz" = "$(op_value op-audio output.audio.bytes)" ] && [ "$sz" -gt 40 ] \
  || { ok=0; why operator-audio "file size '$sz', output.audio.bytes '$(op_value op-audio output.audio.bytes)'"; }
[ "$(od -An -tx1 -N4 "$opo/audio.raw" | tr -d ' \n')" = 00000000 ] \
  || { ok=0; why operator-audio "the first sample is not silent"; }
[ "$(od -An -tx1 -j36 -N4 "$opo/audio.raw" | tr -d ' \n')" = 0f000000 ] \
  || { ok=0; why operator-audio "sample 10 (offset 36) is not 0f000000"; }
result runner-process.operator.audio-raw
: > "$tmp/why"
ok=1
run_one "$tmp" op-log "$ivs_run" --content-dir "$opc" --psg-log-out "$opo/log.txt" "$opscn"
judge op-log 0 pass - "" || ok=0
lines=$(wc -l < "$opo/log.txt" 2> /dev/null | tr -d ' ')
[ -n "$lines" ] && [ "$lines" = "$(op_value op-log output.psglog.records)" ] && [ "$lines" = 4 ] \
  || { ok=0; why operator-log "log lines '$lines', output.psglog.records '$(op_value op-log output.psglog.records)'"; }
[ "$(sed -n '1p' "$opo/log.txt")" = "0 76 8 63 1" ] || { ok=0; why operator-log "first line is not '0 76 8 63 1'"; }
[ "$(sed -n '4p' "$opo/log.txt")" = "3 304 11 7 1" ] || { ok=0; why operator-log "fourth line is not '3 304 11 7 1'"; }
result runner-process.operator.psg-log
# A second run with the same path is refused (exit 14, one stderr line, no report) and leaves the
# file byte-identical: no overwrite, no truncation.
: > "$tmp/why"
ok=1
cp "$opo/log.txt" "$opo/log.before" || exit 1
run_one "$tmp" op-excl "$ivs_run" --content-dir "$opc" --psg-log-out "$opo/log.txt" "$opscn"
[ "$rc" -eq 14 ] || { ok=0; why operator-exclusive "exit $rc, expected 14"; }
[ ! -s "$tmp/runs/op-excl.out" ] || { ok=0; why operator-exclusive "a report was printed"; }
printf 'ivs-run: --psg-log-out: output file exists, refusing to overwrite it\n' > "$tmp/want.err"
cmp -s "$tmp/runs/op-excl.err" "$tmp/want.err" || { ok=0; why operator-exclusive "stderr is not the one refusal line"; }
cmp -s "$opo/log.txt" "$opo/log.before" || { ok=0; why operator-exclusive "the existing file changed"; }
result runner-process.operator.exclusive
# The byte bound is a harness failure, not a truncation.
: > "$tmp/why"
ok=1
run_one "$tmp" op-max "$ivs_run" --content-dir "$opc" --audio-raw "$opo/small.raw" --audio-out-max-bytes 100 "$opscn"
judge op-max 14 harness-fail audio-out-max-bytes "" || ok=0
result runner-process.operator.max-bytes
# Every option once, numbers decimal digits only: a usage error is exit 14 with no report.
: > "$tmp/why"
ok=1
run_one "$tmp" op-twice "$ivs_run" --content-dir "$opc" --audio-raw "$opo/t1.raw" --audio-raw "$opo/t2.raw" "$opscn"
[ "$rc" -eq 14 ] && [ ! -s "$tmp/runs/op-twice.out" ] || { ok=0; why operator-options "--audio-raw twice: exit $rc"; }
run_one "$tmp" op-digits "$ivs_run" --content-dir "$opc" --audio-out-max-bytes 12x "$opscn"
[ "$rc" -eq 14 ] && [ ! -s "$tmp/runs/op-digits.out" ] || { ok=0; why operator-options "non-digit bound: exit $rc"; }
run_one "$tmp" op-zero "$ivs_run" --content-dir "$opc" --audio-out-max-bytes 0 "$opscn"
[ "$rc" -eq 14 ] && [ ! -s "$tmp/runs/op-zero.out" ] || { ok=0; why operator-options "zero bound: exit $rc"; }
[ ! -e "$opo/t1.raw" ] || { ok=0; why operator-options "a usage error created a file"; }
result runner-process.operator.options
# A run that ends harness-fail (exit 14) leaves no operator output file behind (review WR-01): a bound hit on
# each of the three files, and all three at once, remove the files this run created, so a retry with the same
# path is not blocked by "output file exists", and the report states no WAV (refpcm.frames and refpcm.sha256
# are `-`). The retry without the bound then succeeds into the same path. Every other outcome keeps its
# files: a budget timeout (exit 11, 252 bytes of audio by hand under --max-ticks 1000) leaves a complete file.
: > "$tmp/why"
ok=1
run_one "$tmp" op-f-raw "$ivs_run" --content-dir "$opc" --audio-raw "$opo/f.raw" --audio-out-max-bytes 100 "$opscn"
judge op-f-raw 14 harness-fail audio-out-max-bytes "" || ok=0
[ ! -e "$opo/f.raw" ] || { ok=0; why operator-failed "--audio-raw: the failed run left $opo/f.raw"; }
run_one "$tmp" op-f-log "$ivs_run" --content-dir "$opc" --psg-log-out "$opo/f.log" --audio-out-max-bytes 10 "$opscn"
judge op-f-log 14 harness-fail psg-log-out-max-bytes "" || ok=0
[ ! -e "$opo/f.log" ] || { ok=0; why operator-failed "--psg-log-out: the failed run left $opo/f.log"; }
run_one "$tmp" op-f-wav "$ivs_run" --content-dir "$opc" --audio-wav "$opo/f.wav" --audio-out-max-bytes 100 "$opscn"
judge op-f-wav 14 harness-fail wav-out-max-bytes "" || ok=0
[ ! -e "$opo/f.wav" ] || { ok=0; why operator-failed "--audio-wav: the failed run left $opo/f.wav"; }
[ "$(op_value op-f-wav refpcm.frames)" = - ] && [ "$(op_value op-f-wav refpcm.sha256)" = - ] \
  || { ok=0; why operator-failed "the report of the failed WAV run states refpcm.frames '$(op_value op-f-wav refpcm.frames)'"; }
run_one "$tmp" op-f-all "$ivs_run" --content-dir "$opc" --audio-raw "$opo/g.raw" --psg-log-out "$opo/g.log" \
  --audio-wav "$opo/g.wav" --audio-out-max-bytes 100 "$opscn"
[ "$rc" -eq 14 ] || { ok=0; why operator-failed "three files: exit $rc, expected 14"; }
for f in g.raw g.log g.wav; do
  [ ! -e "$opo/$f" ] || { ok=0; why operator-failed "three files: the failed run left $opo/$f"; }
done
run_one "$tmp" op-f-retry "$ivs_run" --content-dir "$opc" --audio-raw "$opo/f.raw" "$opscn"
judge op-f-retry 0 pass - "" || ok=0
sz=$(wc -c < "$opo/f.raw" 2> /dev/null | tr -d ' ')
[ -n "$sz" ] && [ "$sz" = "$(op_value op-f-retry output.audio.bytes)" ] \
  || { ok=0; why operator-failed "the retry file size '$sz', output.audio.bytes '$(op_value op-f-retry output.audio.bytes)'"; }
run_one "$tmp" op-f-keep "$ivs_run" --content-dir "$opc" --audio-raw "$opo/k.raw" --max-ticks 1000 "$opscn"
[ "$rc" -eq 11 ] || { ok=0; why operator-failed "budget timeout: exit $rc, expected 11"; }
sz=$(wc -c < "$opo/k.raw" 2> /dev/null | tr -d ' ')
[ "$sz" = 252 ] && [ "$sz" = "$(op_value op-f-keep output.audio.bytes)" ] \
  || { ok=0; why operator-failed "budget timeout: file size '$sz', expected 252 and output.audio.bytes"; }
result runner-process.operator.failed-run
# --update-golden never names an operator output file (review WR-02): the same string, or the same file under
# another spelling (`./g.raw`), is refused (exit 14, one stderr line, no report), and the run's own output files
# are not left behind; a distinct golden path works and leaves the raw file intact.
: > "$tmp/why"
ok=1
mkdir "$tmp/gold" || exit 1
printf 'ivs-run: --update-golden must not name an operator output file\n' > "$tmp/want.err"
for opt in --audio-raw --psg-log-out --audio-wav; do
  run_one "$tmp/gold" op-g-same "$ivs_run" --content-dir "$opc" "$opt" x.out --update-golden x.out "$opscn"
  [ "$rc" -eq 14 ] && [ ! -s "$tmp/runs/op-g-same.out" ] \
    || { ok=0; why operator-golden "$opt with the same string: exit $rc or a report"; }
  cmp -s "$tmp/runs/op-g-same.err" "$tmp/want.err" || { ok=0; why operator-golden "$opt: stderr is not the one refusal line"; }
  [ ! -e "$tmp/gold/x.out" ] || { ok=0; why operator-golden "$opt with the same string: left x.out"; }
  run_one "$tmp/gold" op-g-alias "$ivs_run" --content-dir "$opc" "$opt" x.out --update-golden ./x.out "$opscn"
  [ "$rc" -eq 14 ] && [ ! -s "$tmp/runs/op-g-alias.out" ] \
    || { ok=0; why operator-golden "$opt with ./x.out: exit $rc or a report"; }
  cmp -s "$tmp/runs/op-g-alias.err" "$tmp/want.err" || { ok=0; why operator-golden "$opt: alias stderr is not the one refusal line"; }
  [ ! -e "$tmp/gold/x.out" ] || { ok=0; why operator-golden "$opt with ./x.out: left x.out"; }
done
run_one "$tmp/gold" op-g-ok "$ivs_run" --content-dir "$opc" --audio-raw g.raw --update-golden g.digests "$opscn"
judge op-g-ok 0 pass - "" || ok=0
sz=$(wc -c < "$tmp/gold/g.raw" 2> /dev/null | tr -d ' ')
[ -n "$sz" ] && [ "$sz" = "$(op_value op-g-ok output.audio.bytes)" ] \
  || { ok=0; why operator-golden "a distinct golden path: raw file size '$sz'"; }
[ "$(sed -n '1p' "$tmp/gold/g.digests" 2> /dev/null)" = "# tier: deterministic-repeatability" ] \
  || { ok=0; why operator-golden "a distinct golden path: no golden written"; }
result runner-process.operator.golden-distinct

# Operator-only video outputs (plan 06-12, D-17, T-06-16, T-06-17): the same project-authored looping stand-in, run to tick
# 130000 (two frame ends, at 59736 and 119472), under --video-raw and --frame-ppm into the mktemp directory. By hand: two slots
# of 32 + 159 x 96 = 15296 bytes, so the raw file is 30592 bytes and equals output.video.bytes, the first slot's index field
# (bytes 8 to 15) is 0 and the second slot's (offset 15296 + 8) ends in 01; the PPM starts `P6`, `159 96`, `255` (14 bytes)
# and holds 3 x 15264 pixel bytes (45806 in all); both files are created mode 0600 (ls -l shows -rw-------).
opvid="$opc/operator-vid.ivss"
printf 'ivss 1\nid operator-vid\nprofile intv-2609-ntsc-family-p1\ncontent exec psg-prog.exec\ncontent grom zero-grom\nrun-to 130000 ticks\n' > "$opvid"
mode_of() { ls -l "$1" 2> /dev/null | cut -c1-10; }
: > "$tmp/why"
ok=1
run_one "$tmp" op-v "$ivs_run" --content-dir "$opc" --video-raw "$opo/v.vraw" --frame-ppm "$opo/v.ppm" "$opvid"
judge op-v 0 pass - "" || ok=0
sz=$(wc -c < "$opo/v.vraw" 2> /dev/null | tr -d ' ')
[ "$sz" = 30592 ] && [ "$sz" = "$(op_value op-v output.video.bytes)" ] \
  || { ok=0; why operator-video "raw file size '$sz', output.video.bytes '$(op_value op-v output.video.bytes)'"; }
[ "$(od -An -tx1 -j8 -N8 "$opo/v.vraw" | tr -d ' \n')" = 0000000000000000 ] \
  || { ok=0; why operator-video "the first slot's index field is not 0"; }
[ "$(od -An -tx1 -j15304 -N8 "$opo/v.vraw" | tr -d ' \n')" = 0000000000000001 ] \
  || { ok=0; why operator-video "the second slot's index field is not 1"; }
[ "$(mode_of "$opo/v.vraw")" = "-rw-------" ] || { ok=0; why operator-video "--video-raw mode '$(mode_of "$opo/v.vraw")', expected -rw-------"; }
psz=$(wc -c < "$opo/v.ppm" 2> /dev/null | tr -d ' ')
[ "$psz" = 45806 ] || { ok=0; why operator-video "ppm size '$psz', expected 45806"; }
[ "$(head -c 14 "$opo/v.ppm" | od -An -tx1 | tr -d ' \n')" = 50360a3135392039360a3235350a ] \
  || { ok=0; why operator-video "the ppm header is not P6, 159 96, 255"; }
[ "$(mode_of "$opo/v.ppm")" = "-rw-------" ] || { ok=0; why operator-video "--frame-ppm mode '$(mode_of "$opo/v.ppm")', expected -rw-------"; }
result runner-process.operator.video-raw
# A scenario that delivers no frame (tick 3000) leaves an empty file, still mode 0600, and nothing else is created.
: > "$tmp/why"
ok=1
mkdir "$tmp/vempty" || exit 1
run_one "$tmp" op-v-empty "$ivs_run" --content-dir "$opc" --video-raw "$tmp/vempty/e.vraw" "$opscn"
judge op-v-empty 0 pass - "" || ok=0
[ -f "$tmp/vempty/e.vraw" ] && [ ! -s "$tmp/vempty/e.vraw" ] || { ok=0; why operator-video-empty "the file is missing or not empty"; }
[ "$(mode_of "$tmp/vempty/e.vraw")" = "-rw-------" ] || { ok=0; why operator-video-empty "mode '$(mode_of "$tmp/vempty/e.vraw")'"; }
[ "$(ls "$tmp/vempty" | wc -l | tr -d ' ')" = 1 ] || { ok=0; why operator-video-empty "other files were created"; }
# The empty file is said aloud (Phase 6 review IN-01): one stderr line, the exit code and the report unchanged; the same for --frame-ppm
# (the PPM case of the security residual); a run that delivers frames prints no such line.
printf 'ivs-run: --video-raw: no frame was delivered, the file is empty\n' > "$tmp/want.err"
cmp -s "$tmp/runs/op-v-empty.err" "$tmp/want.err" || { ok=0; why operator-video-empty "--video-raw: stderr is not the one no-frame line"; }
run_one "$tmp" op-v-empty-ppm "$ivs_run" --content-dir "$opc" --frame-ppm "$tmp/vempty/e.ppm" "$opscn"
judge op-v-empty-ppm 0 pass - "" || ok=0
[ -f "$tmp/vempty/e.ppm" ] && [ ! -s "$tmp/vempty/e.ppm" ] || { ok=0; why operator-video-empty "--frame-ppm: the file is missing or not empty"; }
[ "$(mode_of "$tmp/vempty/e.ppm")" = "-rw-------" ] || { ok=0; why operator-video-empty "--frame-ppm: mode '$(mode_of "$tmp/vempty/e.ppm")'"; }
printf 'ivs-run: --frame-ppm: no frame was delivered, the file is empty\n' > "$tmp/want.err"
cmp -s "$tmp/runs/op-v-empty-ppm.err" "$tmp/want.err" || { ok=0; why operator-video-empty "--frame-ppm: stderr is not the one no-frame line"; }
[ ! -s "$tmp/runs/op-v.err" ] || { ok=0; why operator-video-empty "a run that delivered frames wrote to stderr"; }
result runner-process.operator.video-raw-empty-run
# A run that ends harness-fail leaves no video output file (the Phase 5 rule, review WR-01): the byte bound on the raw file and on the
# PPM (a bound below its 45806 bytes), and an existing path is refused without touching it (exclusive create); the retry without
# the bound then succeeds into the same path.
: > "$tmp/why"
ok=1
run_one "$tmp" op-vf-raw "$ivs_run" --content-dir "$opc" --video-raw "$opo/f.vraw" --video-out-max-bytes 100 "$opvid"
judge op-vf-raw 14 harness-fail video-out-max-bytes "" || ok=0
[ ! -e "$opo/f.vraw" ] || { ok=0; why operator-video-failed "--video-raw: the failed run left f.vraw"; }
run_one "$tmp" op-vf-ppm "$ivs_run" --content-dir "$opc" --frame-ppm "$opo/f.ppm" --video-out-max-bytes 1000 "$opvid"
judge op-vf-ppm 14 harness-fail frame-ppm-out-max-bytes "" || ok=0
[ ! -e "$opo/f.ppm" ] || { ok=0; why operator-video-failed "--frame-ppm: the failed run left f.ppm"; }
run_one "$tmp" op-vf-retry "$ivs_run" --content-dir "$opc" --video-raw "$opo/f.vraw" "$opvid"
judge op-vf-retry 0 pass - "" || ok=0
cp "$opo/f.vraw" "$opo/f.before" || exit 1
run_one "$tmp" op-vf-excl "$ivs_run" --content-dir "$opc" --video-raw "$opo/f.vraw" "$opvid"
[ "$rc" -eq 14 ] && [ ! -s "$tmp/runs/op-vf-excl.out" ] || { ok=0; why operator-video-failed "an existing path: exit $rc or a report"; }
printf 'ivs-run: --video-raw: output file exists, refusing to overwrite it\n' > "$tmp/want.err"
cmp -s "$tmp/runs/op-vf-excl.err" "$tmp/want.err" || { ok=0; why operator-video-failed "an existing path: stderr is not the one refusal line"; }
cmp -s "$opo/f.vraw" "$opo/f.before" || { ok=0; why operator-video-failed "the existing file changed"; }
# Two outputs on one path (Phase 6 review IN-01/IN-05): refused before anything is created, with a message that names the clash and not
# an existing file (the path was free).
run_one "$tmp" op-vf-same "$ivs_run" --content-dir "$opc" --video-raw "$opo/same.out" --frame-ppm "$opo/same.out" "$opvid"
[ "$rc" -eq 14 ] && [ ! -s "$tmp/runs/op-vf-same.out" ] || { ok=0; why operator-video-failed "two outputs on one path: exit $rc or a report"; }
printf 'ivs-run: --frame-ppm: names the same path as --video-raw\n' > "$tmp/want.err"
cmp -s "$tmp/runs/op-vf-same.err" "$tmp/want.err" || { ok=0; why operator-video-failed "two outputs on one path: stderr is not the one clash line"; }
[ ! -e "$opo/same.out" ] || { ok=0; why operator-video-failed "two outputs on one path: a file was created"; }
# --update-golden follows the same rule (Phase 6 review WR-02): a run that ends harness-fail writes no golden and leaves an existing
# one byte-identical; no temporary file is left next to it; the retry without the bound writes the golden.
mkdir "$tmp/gfail" || exit 1
printf 'precious\n' > "$tmp/gfail/g.digests"
cp "$tmp/gfail/g.digests" "$tmp/gfail/g.before" || exit 1
run_one "$tmp/gfail" op-vf-gold "$ivs_run" --content-dir "$opc" --video-raw v.raw --video-out-max-bytes 100 --update-golden g.digests "$opvid"
judge op-vf-gold 14 harness-fail video-out-max-bytes "" || ok=0
cmp -s "$tmp/gfail/g.digests" "$tmp/gfail/g.before" || { ok=0; why operator-video-failed "a failed run changed the existing golden"; }
[ ! -e "$tmp/gfail/v.raw" ] || { ok=0; why operator-video-failed "the failed golden run left v.raw"; }
run_one "$tmp/gfail" op-vf-gold2 "$ivs_run" --content-dir "$opc" --video-raw v2.raw --video-out-max-bytes 100 --update-golden n.digests "$opvid"
judge op-vf-gold2 14 harness-fail video-out-max-bytes "" || ok=0
[ ! -e "$tmp/gfail/n.digests" ] || { ok=0; why operator-video-failed "a failed run wrote a new golden"; }
[ "$(ls "$tmp/gfail" | tr '\n' ' ')" = "g.before g.digests " ] || { ok=0; why operator-video-failed "files left next to the golden: $(ls "$tmp/gfail" | tr '\n' ' ')"; }
run_one "$tmp/gfail" op-vf-gold3 "$ivs_run" --content-dir "$opc" --update-golden g.digests "$opvid"
judge op-vf-gold3 0 pass - "" || ok=0
[ "$(sed -n '1p' "$tmp/gfail/g.digests" 2> /dev/null)" = "# tier: deterministic-repeatability" ] \
  || { ok=0; why operator-video-failed "the retry did not replace the golden"; }
[ "$(ls "$tmp/gfail" | tr '\n' ' ')" = "g.before g.digests " ] || { ok=0; why operator-video-failed "files left after the retry: $(ls "$tmp/gfail" | tr '\n' ' ')"; }
result runner-process.operator.video-raw-failed-run
# --update-golden never names a video output (review WR-02 copied): the same string or the same file under another spelling is
# refused (exit 14, one stderr line, no report) and the run's own output file is not left behind.
: > "$tmp/why"
ok=1
printf 'ivs-run: --update-golden must not name an operator output file\n' > "$tmp/want.err"
for opt in --video-raw --frame-ppm; do
  run_one "$tmp/gold" op-vg-same "$ivs_run" --content-dir "$opc" "$opt" v.out --update-golden v.out "$opvid"
  [ "$rc" -eq 14 ] && [ ! -s "$tmp/runs/op-vg-same.out" ] || { ok=0; why operator-video-golden "$opt with the same string: exit $rc or a report"; }
  cmp -s "$tmp/runs/op-vg-same.err" "$tmp/want.err" || { ok=0; why operator-video-golden "$opt: stderr is not the one refusal line"; }
  [ ! -e "$tmp/gold/v.out" ] || { ok=0; why operator-video-golden "$opt with the same string: left v.out"; }
  run_one "$tmp/gold" op-vg-alias "$ivs_run" --content-dir "$opc" "$opt" v.out --update-golden ./v.out "$opvid"
  [ "$rc" -eq 14 ] && [ ! -s "$tmp/runs/op-vg-alias.out" ] || { ok=0; why operator-video-golden "$opt with ./v.out: exit $rc or a report"; }
  cmp -s "$tmp/runs/op-vg-alias.err" "$tmp/want.err" || { ok=0; why operator-video-golden "$opt: alias stderr is not the one refusal line"; }
  [ ! -e "$tmp/gold/v.out" ] || { ok=0; why operator-video-golden "$opt with ./v.out: left v.out"; }
done
result runner-process.operator.video-raw-update-golden-refused
# A frame rendered from an owner image is derived art of rights-bearing content: --frame-ppm AND --video-raw (the same pixels as index
# bytes; Phase 6 review IN-01) are refused for a scenario that names `content exec user` (exit 14, one stderr line, no report, no file),
# before anything is created.
: > "$tmp/why"
ok=1
printf 'ivss 1\nid operator-user\nprofile intv-2609-ntsc-family-p1\ncontent exec user\ncontent grom zero-grom\nrun-to 1000 ticks\n' > "$opc/operator-user.ivss"
printf 'ivs-run: --frame-ppm refused: content user\n' > "$tmp/want.err"
run_one "$tmp" op-vu "$ivs_run" --content-dir "$opc" --frame-ppm "$opo/u.ppm" "$opc/operator-user.ivss"
[ "$rc" -eq 14 ] && [ ! -s "$tmp/runs/op-vu.out" ] || { ok=0; why operator-video-user "exit $rc or a report"; }
cmp -s "$tmp/runs/op-vu.err" "$tmp/want.err" || { ok=0; why operator-video-user "stderr is not the one refusal line"; }
[ ! -e "$opo/u.ppm" ] || { ok=0; why operator-video-user "a file was created"; }
printf 'ivs-run: --video-raw refused: content user\n' > "$tmp/want.err"
run_one "$tmp" op-vu-raw "$ivs_run" --content-dir "$opc" --video-raw "$opo/u.vraw" "$opc/operator-user.ivss"
[ "$rc" -eq 14 ] && [ ! -s "$tmp/runs/op-vu-raw.out" ] || { ok=0; why operator-video-user "--video-raw: exit $rc or a report"; }
cmp -s "$tmp/runs/op-vu-raw.err" "$tmp/want.err" || { ok=0; why operator-video-user "--video-raw: stderr is not the one refusal line"; }
[ ! -e "$opo/u.vraw" ] || { ok=0; why operator-video-user "--video-raw: a file was created"; }
result runner-process.operator.frame-ppm-refused-user-content
# The three derived audio outputs follow the same rule (plan 07-11, D-31, T-07-29): --audio-raw, --psg-log-out and --audio-wav are each
# refused for that scenario with exit 14, exactly one stderr line `ivs-run: <option> refused: content user`, no report and no file,
# before any output is created. refuse_user <slug> <option> <path> is the case runner-process.refuse-user.<slug>; the audio-raw case
# also gives all three options at once and requires the first of the fixed order (--audio-raw, --psg-log-out, --audio-wav) to be named
# and none of the three files to exist.
refuse_user() {
  : > "$tmp/why"
  ok=1
  printf 'ivs-run: %s refused: content user\n' "$2" > "$tmp/want.err"
  run_one "$tmp" "refuse-user-$1" "$ivs_run" --content-dir "$opc" "$2" "$3" "$opc/operator-user.ivss"
  [ "$rc" -eq 14 ] && [ "$hung" -eq 0 ] && [ ! -s "$tmp/runs/refuse-user-$1.out" ] || { ok=0; why refuse-user "$2: exit $rc, hung $hung or a report"; }
  cmp -s "$tmp/runs/refuse-user-$1.err" "$tmp/want.err" || { ok=0; why refuse-user "$2: stderr is not the one refusal line"; }
  [ ! -e "$3" ] || { ok=0; why refuse-user "$2: a file was created"; }
  if [ "$1" = audio-raw ]; then
    run_one "$tmp" refuse-user-all "$ivs_run" --content-dir "$opc" --audio-wav "$opo/ru-all.wav" --psg-log-out "$opo/ru-all.log" \
      --audio-raw "$opo/ru-all.raw" "$opc/operator-user.ivss"
    [ "$rc" -eq 14 ] && [ "$hung" -eq 0 ] && [ ! -s "$tmp/runs/refuse-user-all.out" ] || { ok=0; why refuse-user "all three: exit $rc, hung $hung or a report"; }
    cmp -s "$tmp/runs/refuse-user-all.err" "$tmp/want.err" || { ok=0; why refuse-user "all three: stderr is not the one --audio-raw line"; }
    [ ! -e "$opo/ru-all.wav" ] && [ ! -e "$opo/ru-all.log" ] && [ ! -e "$opo/ru-all.raw" ] || { ok=0; why refuse-user "all three: a file was created"; }
  fi
  result "runner-process.refuse-user.$1"
}
refuse_user audio-raw --audio-raw "$opo/ru.raw"
refuse_user psg-log-out --psg-log-out "$opo/ru.log"
refuse_user audio-wav --audio-wav "$opo/ru.wav"

# Operator WAV (plan 05-10, D-20, D-21, T-05-26): the same stand-in under --audio-wav, into the mktemp directory.
# By hand: the report ends with refpcm.rate (48000), refpcm.frames and refpcm.sha256 right before report.end;
# the file is 44 + 2 x frames bytes (a mono 16-bit RIFF/WAVE file), begins with the four bytes RIFF and the
# header's data size field is 2 x frames; the key refpcm.sha256 is the SHA-256 of the file bytes (cmake -E
# sha256sum); a second run with the same path is refused (exit 14, one stderr line, no report) and leaves the
# file byte-identical; --wav-rate 8000 and a --wav-rate without --audio-wav are usage errors (exit 14, no report,
# no file). One run per preset, no slicing variants.
: > "$tmp/why"
ok=1
run_one "$tmp" op-wav "$ivs_run" --content-dir "$opc" --audio-wav "$opo/a.wav" "$opscn"
judge op-wav 0 pass - "" || ok=0
wframes=$(op_value op-wav refpcm.frames)
wsz=$(wc -c < "$opo/a.wav" 2> /dev/null | tr -d ' ')
wsha=$(cmake -E sha256sum "$opo/a.wav" 2> /dev/null | awk '{ print $1 }')
[ "$(op_value op-wav refpcm.rate)" = 48000 ] || { ok=0; why operator-wav "refpcm.rate '$(op_value op-wav refpcm.rate)'"; }
[ -n "$wframes" ] && [ "$wframes" -gt 0 ] && [ "$wsz" = "$((44 + 2 * wframes))" ] \
  || { ok=0; why operator-wav "file size '$wsz', refpcm.frames '$wframes'"; }
[ "$(od -An -tx1 -N4 "$opo/a.wav" | tr -d ' \n')" = 52494646 ] || { ok=0; why operator-wav "the first four bytes are not RIFF"; }
[ "$wsha" = "$(op_value op-wav refpcm.sha256)" ] && [ "${#wsha}" -eq 64 ] \
  || { ok=0; why operator-wav "sha256 of the file '$wsha', refpcm.sha256 '$(op_value op-wav refpcm.sha256)'"; }
[ "$(tail -n 5 "$tmp/runs/op-wav.out" | sed 's/ .*//' | tr '\n' ' ')" = "host.calls refpcm.rate refpcm.frames refpcm.sha256 report.end " ] \
  || { ok=0; why operator-wav "the refpcm keys are not right before report.end"; }
cp "$opo/a.wav" "$opo/a.before" || exit 1
run_one "$tmp" op-wav2 "$ivs_run" --content-dir "$opc" --audio-wav "$opo/a.wav" "$opscn"
[ "$rc" -eq 14 ] && [ ! -s "$tmp/runs/op-wav2.out" ] || { ok=0; why operator-wav "a second run with the same path: exit $rc"; }
printf 'ivs-run: --audio-wav: output file exists, refusing to overwrite it\n' > "$tmp/want.err"
cmp -s "$tmp/runs/op-wav2.err" "$tmp/want.err" || { ok=0; why operator-wav "stderr is not the one refusal line"; }
cmp -s "$opo/a.wav" "$opo/a.before" || { ok=0; why operator-wav "the existing file changed"; }
run_one "$tmp" op-wrate "$ivs_run" --content-dir "$opc" --audio-wav "$opo/r.wav" --wav-rate 8000 "$opscn"
[ "$rc" -eq 14 ] && [ ! -s "$tmp/runs/op-wrate.out" ] || { ok=0; why operator-wav "--wav-rate 8000: exit $rc"; }
run_one "$tmp" op-wlone "$ivs_run" --content-dir "$opc" --wav-rate 44100 "$opscn"
[ "$rc" -eq 14 ] && [ ! -s "$tmp/runs/op-wlone.out" ] || { ok=0; why operator-wav "--wav-rate without --audio-wav: exit $rc"; }
[ ! -e "$opo/r.wav" ] || { ok=0; why operator-wav "a usage error created a file"; }
result runner-process.refpcm.wav-written
# Reference PCM golden runs (plan 05-11, D-21, D7): per line `<scenario-id> <rate>` of
# data/expected/refpcm-scenarios.ids the scenario runs with `--audio-wav <file> --wav-rate <rate>` into a
# mktemp directory (never the repository); it must end exit 0 as the scenario inventory says, report the
# listed refpcm.rate and leave a WAV of 44 + 2 x refpcm.frames bytes. The report is kept in
# <IVS_TEST_OUT_DIR>/runner/<id>.refpcm.report so the golden step compares the .refpcm golden with it (dev
# and release). One run per preset, no slicing variants.
refpcm_inv="$root/data/expected/refpcm-scenarios.ids"
mkdir "$tmp/refpcm" || exit 1
while read -r rid rrate rrest; do
  case $rid in '' | '#'*) continue ;; esac
  ok=1
  : > "$tmp/why"
  if [ -z "$rrate" ] || [ -n "$rrest" ]; then
    ok=0; why inventory "not <scenario-id> <rate>"
  elif [ ! -f "$scen/$rid.ivss" ]; then
    ok=0; why inventory "no scenario file data/scenarios/$rid.ivss"
  else
    run_one "$tmp" "refpcm-$rid" "$ivs_run" --content-dir "$content" --audio-wav "$tmp/refpcm/$rid.wav" --wav-rate "$rrate" "$scen/$rid.ivss"
    judge "refpcm-$rid" 0 pass - "" || ok=0
    rfr=$(op_value "refpcm-$rid" refpcm.frames)
    rsz=$(wc -c < "$tmp/refpcm/$rid.wav" 2> /dev/null | tr -d ' ')
    [ "$(op_value "refpcm-$rid" refpcm.rate)" = "$rrate" ] || { ok=0; why refpcm "refpcm.rate '$(op_value "refpcm-$rid" refpcm.rate)', expected $rrate"; }
    [ -n "$rfr" ] && [ "$rfr" -gt 0 ] && [ "$rsz" = "$((44 + 2 * rfr))" ] \
      || { ok=0; why refpcm "file size '$rsz', refpcm.frames '$rfr'"; }
    if [ -n "$rep_dir" ] && [ -s "$tmp/runs/refpcm-$rid.out" ]; then
      cp "$tmp/runs/refpcm-$rid.out" "$rep_dir/$rid.refpcm.report" || { printf '# cannot write %s\n' "$rep_dir/$rid.refpcm.report" >&2; exit 1; }
    fi
  fi
  result "runner-process.refpcm.$rid"
done < "$refpcm_inv"
# The listening helper refuses a scenario with a `content ... user` line before it builds or runs anything
# (rights: derived audio of operator content stays out of shared records): exit 2, a message naming the rule.
: > "$tmp/why"
ok=1
run_one "$tmp" listen-user "$root/tools/listen-demo.sh" needs-user-exec
[ "$rc" -eq 2 ] || { ok=0; why listen-demo "exit $rc, expected 2"; }
[ ! -s "$tmp/runs/listen-user.out" ] || { ok=0; why listen-demo "output on stdout from a refusal"; }
grep -q -e 'content .* user' "$tmp/runs/listen-user.err" || { ok=0; why listen-demo "the message does not name the content user rule"; }
result runner-process.listen-demo.refuses-user

# The owner-local helper (plan 07-11, D-31, T-07-26 to T-07-28; docs/owner-images.md): its refusal, skip and identity cases, judged by exit
# code and one line of text, with no owner image anywhere: the only files are project-made zero-filled files of the exact sizes. Hermetic
# by construction: every case runs a COPY of the helper in a fresh scratch root (mktemp -d under this run's temp directory, outside the work
# tree), so the helper's own root, from its $0, is the scratch root and nothing of the repository can be read or written. The scratch root
# holds its own refs/ (locks and files written by the case), a copy of the template scenario and a link build/release/runner/ivs-run to the
# runner under test. The helper's TMPDIR is a directory outside every scratch root: a leftover there is a finding. CI and GITHUB_ACTIONS are
# set empty for every case except the two environment cases (the container and hosted gates set CI).
# By hand, the sizes are EXEC 8192 and GROM 2048 bytes; a lock is `<64 lower-case hex>  exec` and `<64 lower-case hex>  grom`; the nine cases:
# usage (exit 2), ci-env and github-actions-env (exit 2, checked before the absent-images skip), in-tree-path (exit 2: a path whose physical
# directory is inside the root but not under refs/, also by a `..` spelling and for a directory named -x), wrong-size (exit 2: too short, too long, a missing image of
# two, an image in a missing directory), missing-lock (exit 2; then --record writes the lock and a second --record refuses to replace it), bad-lock-line (exit 2, one token per defect), no-images-skip (exit 77 and the SKIPPED line, even
# when the directory of a path does not exist) and identity-mismatch (zero-filled files and a lock of 64 zero digits: the core's
# expect-sha256 refuses at load, exit 14, the report names IVS_E_DIGEST_MISMATCH, nothing was executed and nothing else was written).
or_tmpd="$tmp/or-tmpdir"
mkdir "$or_tmpd" || exit 1
or_ci_msg='owner-run: refusing to run in CI (CI or GITHUB_ACTIONS is set): owner images are never used there'
or_usage_msg='owner-run: usage: owner-run.sh [--preset dev|release] [--record] <exec-image> <grom-image>'
or_lock_msg='owner-run: bad lock line: want 64 lower-case hex digits, two spaces, exec or grom'
or_z=$(printf '%064d' 0)
# or_scratch: a fresh scratch root in $sr.
or_scratch() {
  sr=$(mktemp -d "$tmp/owner-run-root.XXXXXX") || exit 1
  mkdir -p "$sr/tools" "$sr/data/scenarios" "$sr/build/release/runner" "$sr/refs" || exit 1
  cp "$root/tools/owner-run.sh" "$sr/tools/owner-run.sh" || exit 1
  cp "$scen/owner-exec-title.ivss" "$sr/data/scenarios/owner-exec-title.ivss" || exit 1
  ln -s "$ivs_run" "$sr/build/release/runner/ivs-run" || exit 1
}
# or_img <path> <bytes>: a project-made zero-filled file.
or_img() { dd if=/dev/zero of="$1" bs="$2" count=1 2> /dev/null || exit 1; }
# or_run <name> <CI> <GITHUB_ACTIONS> <helper arguments...>: the scratch copy as run or-<name> (its TMPDIR is $or_tmpd unless $or_tmp is set).
or_run() {
  _on=$1; _oc=$2; _og=$3; shift 3
  run_one "$tmp" "or-$_on" env "CI=$_oc" "GITHUB_ACTIONS=$_og" "TMPDIR=${or_tmp:-$or_tmpd}" sh "$sr/tools/owner-run.sh" "$@"
}
# or_same <file> <line>: 0 when the file holds exactly the one LF-terminated line <line>, or nothing when <line> is empty (shell builtins only,
# no process per check: these cases are cost-capped).
or_same() {
  _l1=; _l2=; _r1=0; _r2=0
  { IFS= read -r _l1 || _r1=1; IFS= read -r _l2 || _r2=1; } < "$1"
  if [ -z "$2" ]; then
    [ "$_r1" -eq 1 ] && [ -z "$_l1" ]
  else
    [ "$_r1" -eq 0 ] && [ "$_l1" = "$2" ] && [ "$_r2" -eq 1 ] && [ -z "$_l2" ]
  fi
}
# or_judge <name> <exit> <stdout line> <stderr line>: run or-<name> ended with the exit code, was not hung and wrote exactly that one stdout
# line and that one stderr line ('' for none). Findings owner-run-hung, owner-run-exit, owner-run-stdout, owner-run-stderr; returns 0 when
# it adds none.
or_judge() {
  found=0
  [ "$hung" -eq 0 ] || why owner-run-hung "still running after ${limit}s, stopped by the watchdog"
  [ "$rc" = "$2" ] || why owner-run-exit "exit $rc, expected $2"
  or_same "$tmp/runs/or-$1.out" "$3" || why owner-run-stdout "stdout is not exactly: $3 (got: $(sed -n '1,2p' "$tmp/runs/or-$1.out"))"
  or_same "$tmp/runs/or-$1.err" "$4" || why owner-run-stderr "stderr is not exactly: $4 (got: $(sed -n '1,2p' "$tmp/runs/or-$1.err"))"
  [ "$found" -eq 0 ]
}
# or_end: ends a case: nothing may be left in the helper's TMPDIR (finding owner-run-tmpdir), and the scratch root is removed.
or_end() {
  if [ -n "$(ls -A "$or_tmpd")" ]; then
    ok=0; why owner-run-tmpdir "left in the helper's TMPDIR: $(ls -A "$or_tmpd")"
    rm -rf "$or_tmpd"; mkdir "$or_tmpd"
  fi
  rm -rf "$sr"
}

: > "$tmp/why"
ok=1
or_scratch
or_run usage '' ''
or_judge usage 2 '' "$or_usage_msg" || ok=0
or_run usage '' '' --preset bogus "$sr/refs/a" "$sr/refs/b"
or_judge usage 2 '' "$or_usage_msg" || ok=0
or_end
result runner-process.owner-run.usage

# Both environment variables refuse, and before the absent-images skip (the two paths do not exist, so an unchecked environment would skip).
: > "$tmp/why"
ok=1
or_scratch
or_run ci true '' "$sr/refs/a" "$sr/refs/b"
or_judge ci 2 '' "$or_ci_msg" || ok=0
or_end
result runner-process.owner-run.ci-env
: > "$tmp/why"
ok=1
or_scratch
or_run gha '' true "$sr/refs/a" "$sr/refs/b"
or_judge gha 2 '' "$or_ci_msg" || ok=0
or_end
result runner-process.owner-run.github-actions-env

# An image whose physical directory is inside the root but not under refs/ (also reached through a `..` spelling, and with only one image
# there) is refused.
: > "$tmp/why"
ok=1
or_scratch
mkdir "$sr/data/own"
or_img "$sr/data/own/e.bin" 8192
or_img "$sr/data/own/g.bin" 2048
or_run tree '' '' "$sr/data/own/e.bin" "$sr/data/own/g.bin"
or_judge tree 2 '' "owner-run: an image is inside the work tree outside refs/: $sr/data/own/e.bin" || ok=0
or_img "$sr/refs/e.bin" 8192
or_run tree '' '' "$sr/refs/e.bin" "$sr/refs/../data/own/g.bin"
or_judge tree 2 '' "owner-run: an image is inside the work tree outside refs/: $sr/refs/../data/own/g.bin" || ok=0
# A relative path whose directory is named like an option (-x) is a directory, not an option of cd: it resolves inside the root and is refused
# (an unresolved directory used to pass this check and reach the lock).
mkdir "$sr/-x"
or_img "$sr/-x/e.bin" 8192
or_img "$sr/-x/g.bin" 2048
run_one "$sr" or-tree env "CI=" "GITHUB_ACTIONS=" "TMPDIR=$or_tmpd" sh "$sr/tools/owner-run.sh" -x/e.bin -x/g.bin
or_judge tree 2 '' 'owner-run: an image is inside the work tree outside refs/: -x/e.bin' || ok=0
or_end
result runner-process.owner-run.in-tree-path

# Not a regular file of the exact size: too short, too long, a missing image of two (not the skip).
: > "$tmp/why"
ok=1
or_scratch
or_exec_msg='owner-run: the exec image is not a regular file of exactly 8192 bytes'
or_grom_msg='owner-run: the grom image is not a regular file of exactly 2048 bytes'
or_img "$sr/refs/short.bin" 8191
or_img "$sr/refs/e.bin" 8192
or_img "$sr/refs/long.bin" 2049
or_img "$sr/refs/g.bin" 2048
or_run size '' '' "$sr/refs/short.bin" "$sr/refs/g.bin"
or_judge size 2 '' "$or_exec_msg" || ok=0
or_run size '' '' "$sr/refs/e.bin" "$sr/refs/long.bin"
or_judge size 2 '' "$or_grom_msg" || ok=0
or_run size '' '' "$sr/refs/e.bin" "$sr/refs/none.bin"
or_judge size 2 '' "$or_grom_msg" || ok=0
or_run size '' '' "$sr/refs/e.bin" "$sr/refs/nodir/g.bin"
or_judge size 2 '' "$or_grom_msg" || ok=0
or_end
result runner-process.owner-run.wrong-size

# Right-sized files and no lock.
: > "$tmp/why"
ok=1
or_scratch
or_img "$sr/refs/e.bin" 8192
or_img "$sr/refs/g.bin" 2048
or_run nolock '' '' "$sr/refs/e.bin" "$sr/refs/g.bin"
or_judge nolock 2 '' 'owner-run: no lock refs/owner-images.sha256 (write it with --record)' || ok=0
# --record writes the lock from the two files (the digests of 8192 and of 2048 zero bytes, pinned here from an independent tool) and goes on;
# a TMPDIR that does not exist ends it right after the write, before anything is executed. A second --record refuses to replace the lock.
or_tmp="$tmp/no-such-dir"
or_run record '' '' --record "$sr/refs/e.bin" "$sr/refs/g.bin"
or_tmp=
or_judge record 2 '' 'owner-run: cannot make a temporary directory' || ok=0
printf '%s\n' '9f1dcbc35c350d6027f98be0f5c8b43b42ca52b7604459c0c42be3aa88913d47  exec' 'e5a00aa9991ac8a5ee3109844d84a55583bd20572ad3ffcd42792f3c36b183ad  grom' \
  > "$tmp/or.want"
cmp -s "$sr/refs/owner-images.sha256" "$tmp/or.want" || { ok=0; why owner-run-record "the recorded lock is not the digests of the two zero-filled files"; }
or_run record '' '' --record "$sr/refs/e.bin" "$sr/refs/g.bin"
or_judge record 2 '' 'owner-run: refs/owner-images.sha256 exists: remove it to record again' || ok=0
or_end
result runner-process.owner-run.missing-lock

# A lock line must be exactly 64 lower-case hex digits, two spaces and exec or grom, once each; one defect per run, one token each.
: > "$tmp/why"
ok=1
or_scratch
or_img "$sr/refs/e.bin" 8192
or_img "$sr/refs/g.bin" 2048
or_up=A$(printf '%063d' 0)
or_short=$(printf '%063d' 0)
# or_badlock <expected stderr line> <lock line>...: writes the lock (one argument per line) and runs the helper once.
or_badlock() {
  or_want=$1; shift
  printf '%s\n' "$@" > "$sr/refs/owner-images.sha256"
  or_run lock '' '' "$sr/refs/e.bin" "$sr/refs/g.bin"
  or_judge lock 2 '' "$or_want" || ok=0
}
or_badlock "$or_lock_msg" "$or_up  exec" "$or_z  grom"
or_badlock "$or_lock_msg" "$or_short  exec" "$or_z  grom"
or_badlock "$or_lock_msg" "$or_z exec" "$or_z  grom"
or_badlock 'owner-run: bad lock line: the role is not exec or grom' "$or_z  exec" "$or_z  cart"
or_badlock 'owner-run: bad lock: exec or grom is missing' "$or_z  exec"
or_end
result runner-process.owner-run.bad-lock-line

# Neither image exists: the SKIPPED line and exit 77, checked before any path or file check (a directory that does not exist is no refusal).
: > "$tmp/why"
ok=1
or_scratch
or_run skip '' '' "$sr/refs/none-a" "$sr/refs/none-b"
or_judge skip 77 'SKIPPED owner-run: no operator images (D11)' '' || ok=0
or_run skip '' '' "$sr/no-such-dir/a" "$sr/no-such-dir/b"
or_judge skip 77 'SKIPPED owner-run: no operator images (D11)' '' || ok=0
or_end
result runner-process.owner-run.no-images-skip

# A lock that does not match the loaded bytes: zero-filled project files of the exact sizes and a lock of 64 zero digits. The helper passes
# the lock digests to the core as expect-sha256 lines and the core refuses at load (IVS_E_DIGEST_MISMATCH), so the report says so, nothing was
# executed (cpu.run_state unreset), the banner is the first stdout line, stderr is empty and the scratch root holds no file but its own.
: > "$tmp/why"
ok=1
or_scratch
or_img "$sr/refs/e.bin" 8192
or_img "$sr/refs/g.bin" 2048
printf '%s  exec\n%s  grom\n' "$or_z" "$or_z" > "$sr/refs/owner-images.sha256"
or_run mismatch '' '' "$sr/refs/e.bin" "$sr/refs/g.bin"
[ "$rc" -eq 14 ] && [ "$hung" -eq 0 ] || { ok=0; why owner-run-exit "exit $rc (hung $hung), expected 14"; }
[ ! -s "$tmp/runs/or-mismatch.err" ] || { ok=0; why owner-run-stderr "stderr is not empty"; }
[ "$(sed -n '1p' "$tmp/runs/or-mismatch.out")" = "OWNER-LOCAL RUN (D11): inputs are the owner's, not redistributable, not run in CI, not evidence, no claim" ] \
  || { ok=0; why owner-run-banner "the first stdout line is not the banner"; }
grep -q '^outcome\.detail load-refused role=exec status=IVS_E_DIGEST_MISMATCH' "$tmp/runs/or-mismatch.out" \
  || { ok=0; why owner-run-identity "the report does not name the digest mismatch at load"; }
grep -q -x 'cpu.run_state unreset' "$tmp/runs/or-mismatch.out" || { ok=0; why owner-run-identity "the machine was run"; }
find "$sr" ! -type d | sed "s|^$sr/||" | sort > "$tmp/or.files"
printf '%s\n' build/release/runner/ivs-run data/scenarios/owner-exec-title.ivss refs/e.bin refs/g.bin refs/owner-images.sha256 tools/owner-run.sh \
  | sort > "$tmp/or.want"
cmp -s "$tmp/or.files" "$tmp/or.want" || { ok=0; why owner-run-files "the scratch root holds other files: $(tr '\n' ' ' < "$tmp/or.files")"; }
or_end
result runner-process.owner-run.identity-mismatch

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
# check_covered <pairs> <scenario pairs>: every `<outcome> <exit>` line of <pairs> has a line
# `<outcome> <exit> run` in <scenario pairs> (outcome-uncovered), every scenario line names a pair
# (outcome-unknown) and <pairs> is not empty (outcomes). Appends findings to $tmp/why; returns 0
# when it adds none.
check_covered() {
  found=0
  while read -r o e; do
    grep -q -x -F -e "$o $e run" "$2" || why outcome-uncovered "no running scenario for $o $e"
  done < "$1"
  while read -r o e s; do
    grep -q -x -F -e "$o $e" "$1" || why outcome-unknown "scenario pair $o $e is not in runner-outcomes.ids"
  done < "$2"
  [ -s "$1" ] || why outcomes "data/expected/runner-outcomes.ids lists no pair"
  [ "$found" -eq 0 ]
}
: > "$tmp/why"
ok=1
awk '!/^#/ && NF { print $3, $2, $5 }' "$inv" > "$tmp/scen.pairs"
check_covered "$tmp/pairs" "$tmp/scen.pairs" || ok=0
result runner-process.outcomes-covered

# Judge self-control of outcomes-covered (plan 04-24, D-46): the scenario pairs without every
# scenario of the first outcome (a demonstrating scenario removed) must fail with
# outcome-uncovered.
o1=
read -r o1 e1 < "$tmp/pairs" || :
grep -v -x -F -e "$o1 ${e1:-} run" "$tmp/scen.pairs" > "$tmp/scen.dropped" || :
self_control outcome-uncovered outcome-uncovered check_covered "$tmp/pairs" "$tmp/scen.dropped"

# Judge self-controls of the metamorphic pair judge (plan 07-07, D-23, T-07-17): fake report pairs made from this run's reports of
# demo-lamp-tone-pass and demo-lamp-tone-idle, each with one defect, must be rejected by check_pair for the stated token, using the
# differ and equal lists of the pass and idle line of data/expected/scenario-pairs.ids.
lt_pass="$tmp/runs/scen-demo-lamp-tone-pass.out"
lt_idle="$tmp/runs/scen-demo-lamp-tone-idle.out"
lt_d=$(awk '$1 == "demo-lamp-tone-pass" && $2 == "demo-lamp-tone-idle" { sub(/^differ=/, "", $3); print $3; exit }' "$pairs")
lt_e=$(awk '$1 == "demo-lamp-tone-pass" && $2 == "demo-lamp-tone-idle" { sub(/^equal=/, "", $4); print $4; exit }' "$pairs")
lt_v=$(rkey "$lt_pass" digest.video)
lt_ev=$(rkey "$lt_pass" input.events)
awk -v v="$lt_v" '$1 == "digest.video" { print $1, v; next } { print }' "$lt_idle" > "$fakes/idle-video-equal.report" 2> /dev/null || : > "$fakes/idle-video-equal.report"
awk -v c="$((${lt_ev:-0} - 1))" '$1 == "input.consumed" { print $1, c; next } { print }' "$lt_pass" > "$fakes/pass-not-consumed.report" 2> /dev/null || : > "$fakes/pass-not-consumed.report"
awk '$1 == "output.audio.changes" { print $1, 3; next } { print }' "$lt_idle" > "$fakes/idle-audible.report" 2> /dev/null || : > "$fakes/idle-audible.report"
self_control pair-video-equal pair-not-different \
  check_pair demo-lamp-tone-pass demo-lamp-tone-idle "$lt_pass" "$fakes/idle-video-equal.report" "${lt_d:-}" "${lt_e:-}"
self_control input-not-consumed input-not-consumed \
  check_pair demo-lamp-tone-pass demo-lamp-tone-idle "$fakes/pass-not-consumed.report" "$lt_idle" "${lt_d:-}" "${lt_e:-}"
self_control idle-audible idle-audible \
  check_pair demo-lamp-tone-pass demo-lamp-tone-idle "$lt_pass" "$fakes/idle-audible.report" "${lt_d:-}" "${lt_e:-}"

# Judge self-control of the owner-helper cases (plan 07-11, D-31): a fake helper in its own scratch root that exits 0 where the real one must
# refuse (CI set) is rejected by or_judge for its exit code.
or_fake() {
  or_scratch
  printf '#!/bin/sh\nexit 0\n' > "$sr/tools/owner-run.sh"
  or_run fake true '' "$sr/refs/a" "$sr/refs/b"
  or_judge fake 2 '' "$or_ci_msg"
  _fj=$?
  or_end
  return "$_fj"
}
self_control owner-run-refusal owner-run-exit or_fake

# Watchdog self-control (the hosted false `hung` of run 43148343848, see wd_start): 20 watchdogs
# of a live target, each stopped at once, must leave no marker and the target alive (before the
# arming handshake dash dropped most of these stops: 95 of 100 in a local Ubuntu 24.04 container);
# a run of `true` must end with exit 0 and not hung; `sleep 30` under a 1-second limit must be
# stopped by the watchdog (hung, exit 143). The quiet limit is 10 s and the first marker ends the
# loop, so a regression fails in about 10 seconds.
: > "$tmp/why"
ok=1
limit0=$limit
limit=10
sleep 100 > /dev/null 2>&1 < /dev/null &
tgt=$!
k=0
while [ "$k" -lt 20 ]; do
  wd_start "$tgt" "$tmp/runs/wd-quiet.mark"
  wd_stop
  if [ -f "$tmp/runs/wd-quiet.mark" ]; then
    ok=0
    why watchdog-quiet "a watchdog stopped at once wrote its marker (stop $((k + 1)) of 20)"
    break
  fi
  k=$((k + 1))
done
kill -0 "$tgt" 2> /dev/null || { ok=0; why watchdog-quiet "the target of the stopped watchdogs is gone"; }
# A watchdog that dies before it arms and one that never arms (10 polls) must each end wd_start with a
# failure and no marker, in a subshell so the redefinition of wd_child stays there (review WR-03).
( wd_child() { exit 0; }; wd_start "$tgt" "$tmp/runs/wd-dead.mark"; _r=$?; wd_stop; exit "$_r" ) 2> /dev/null
[ $? -ne 0 ] || { ok=0; why watchdog-unarmed "a watchdog that died before it armed was reported armed"; }
( wd_child() { exec sleep 3; }; wd_polls=10; wd_start "$tgt" "$tmp/runs/wd-stall.mark"; _r=$?; wd_stop; exit "$_r" ) 2> /dev/null
[ $? -ne 0 ] || { ok=0; why watchdog-unarmed "a watchdog that never armed was reported armed"; }
[ ! -f "$tmp/runs/wd-dead.mark" ] && [ ! -f "$tmp/runs/wd-stall.mark" ] \
  || { ok=0; why watchdog-unarmed "an unarmed watchdog wrote its marker"; }
kill "$tgt" 2> /dev/null
wait "$tgt" 2> /dev/null
run_one "$tmp" wd-true true
[ "$hung" -eq 0 ] && [ "$rc" -eq 0 ] || { ok=0; why watchdog-quiet "true: exit $rc, hung $hung"; }
limit=1
run_one "$tmp" wd-fire sleep 30
[ "$hung" -eq 1 ] && [ "$rc" -eq 143 ] || { ok=0; why watchdog-fire "sleep 30 under a 1 s limit: exit $rc, hung $hung"; }
limit=$limit0
result runner-process.watchdog

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

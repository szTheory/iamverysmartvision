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
# Since plan 14-03 (SCN-03, D-47): the cartridge role. runner-process.cart-user.refuse-derived: a scratch scenario naming all three user
# roles (`content exec user`, `content grom user`, `content cart user`) is refused for each of the six derived-output options (exit 14, one
# exact stderr line, no report, no file), the three audio options together name the first of their fixed order, and the snapshot rule is
# proved to be the ABSENCE of a seventh refusal: `--roundtrip-at-ticks` is accepted, writes no file and leaves the report equal to the plain
# run apart from host.calls, with at least one cut. runner-process.judge.flatwrap-roundtrip: a container the project's own wrap tool wrote
# loads as `content cart user` with slot.cart.sha256 equal to the digest the tool printed, and the equality is proved live by flipping one
# byte of the tool's input, which moves both digests. runner-process.judge.owner-run-cart-pass: the helper's `--cart` option runs the pass
# path on project-made files with `exec: -`, and a one-byte EXEC mutation after --record is refused at load (IVS_E_DIGEST_MISMATCH, exit 14),
# so the pass direction is proved able to fail. The optional sixth argument is the build-only wrap tool ivs_flatwrap: without it those three
# cases report `not ok` with that reason, never a skip, so a configuration in which the tool is missing cannot pass.
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
# Since plan 08-lever2 (cost lever 2 of the Phase 8 stop rule, checkpoint A-08): the same rule holds for EVERY mutant inventory of this script
# (diag-mutations.ids, -ctrl-psg.ids, -stic.ids, -scn.ids and -stic2.ids, 35 ids): the one function mutant_case decides between the full run
# (Release: run_mutant, outcome, first clause and fails set judged as before) and the static judgement (any other configuration, the sanitized
# Debug preset, or none). The static judgement is the static half of the full one and every part of it can fail: the checks of the loop of the
# row (inventory form, the word at each address of the unpatched listing equals its from, list lengths, fails= form, outcome vocabulary) and,
# in mutant_case, the scenario file exists, the stated outcome and exit code are a pair of runner-outcomes.ids, the assert that the first
# clause names exists as a line `assert <text>` of the scenario, and the patch builds and changes the image (the boot stub image for a row in
# the stub window). The ids and their number are the same in both presets, no id is skip-ok, `tests-skipped` does not move. What the static
# judgement cannot see (a patched program that no longer fails its scenario) is judged by the Release run of the same gate. A comment line
# `# mutants: configuration=<c> full=<n> static=<m> variants=<v> variant-static=<s>` before the trailer makes the mode of every run visible. A judgement
# that needs the sanitized build to be meaningful would stay a full run on both presets; none of the 35 does (every one is judged by exit code and report).
# Since plan 09-01 (D-09, A-03, A-04): the two demo-lamp-tone variants are column-5 `release-only`, so on the sanitized preset they are judged by the
# derivation equality of docs/video/demo-lamp-tone-1-derivation.md and on Release they run; `variants=` and `variant-static=` count those rows in each
# mode (they sum to the number of such rows), so the counts make the mode of every run visible instead of leaving it to be inferred.
# Since plan 15-01 (TOOL-08, D-48, D-35): the host shim. The seventh argument is the built build-only shared object
# libivs_retro and the eighth is the fake frontend ivs_host_harness that dlopens it; both are `$<TARGET_FILE:...>` paths
# CMake provides, never an environment variable (T-15-01). runner-process.host.digests drives the project's own
# lamp-tone.ivsc with the pass scenario's six events, one call per frame boundary and then exactly 806568, and requires
# the nine `digest.` lines the frontend printed to equal data/golden/demo-lamp-tone-pass.digests byte for byte under
# exactly the two comparisons check_one_golden makes (the sorted line set and the observed domain set), with a positive
# frame count required before anything is compared (a vacuous judge cannot pass). Missing either argument, the case is
# `not ok` with that reason, never a skip.
# Usage: sh runner-process-tap.sh <source root> <ivs-run> <ivs_pack> <images dir> [<build configuration>] [<ivs_flatwrap>] [<libivs_retro>] [<ivs_host_harness>]
# Exit: 0 pass, 1 fail, 77 every case skipped.
# Env: IVS_TEST_NONCE, IVS_TEST_OUT_DIR, IVS_RUNNER_PROCESS_TIMEOUT.
LC_ALL=C; export LC_ALL
usage='usage: sh runner-process-tap.sh <source root> <ivs-run> <ivs_pack> <images dir> [<build configuration>] [<ivs_flatwrap>] [<libivs_retro>] [<ivs_host_harness>]'
[ "$#" -ge 4 ] && [ "$#" -le 8 ] || { printf '%s\n' "$usage" >&2; exit 1; }
config=${5:-}
flatwrap_arg=${6:-}
shim_arg=${7:-}
harness_arg=${8:-}
# Absolute paths: the cwd case runs from another directory.
abs_dir() { (cd "$1" 2> /dev/null && pwd -P); }
abs_file() { _d=$(abs_dir "$(dirname "$1")") && printf '%s/%s\n' "$_d" "$(basename "$1")"; }
root=$(abs_dir "$1") || { printf 'runner-process-tap: no dir %s\n' "$1" >&2; exit 1; }
ivs_run=$(abs_file "$2") || { printf 'runner-process-tap: no %s\n' "$2" >&2; exit 1; }
pack=$(abs_file "$3") || { printf 'runner-process-tap: no %s\n' "$3" >&2; exit 1; }
images=$(abs_dir "$4") || { printf 'runner-process-tap: no dir %s\n' "$4" >&2; exit 1; }
# The wrap tool (plan 14-03): absent, the three cartridge cases report a finding rather than a skip.
flatwrap=
if [ -n "$flatwrap_arg" ]; then
  flatwrap=$(abs_file "$flatwrap_arg") || { printf 'runner-process-tap: no %s\n' "$flatwrap_arg" >&2; exit 1; }
fi
# The build-only shared object and the fake frontend (plan 15-01, TOOL-08): a shared object is a regular file, not an
# executable, so it is required to exist rather than to carry the execute bit. Absent, the host case reports a finding.
shim=
if [ -n "$shim_arg" ]; then
  shim=$(abs_file "$shim_arg") || { printf 'runner-process-tap: no %s\n' "$shim_arg" >&2; exit 1; }
fi
harness=
if [ -n "$harness_arg" ]; then
  harness=$(abs_file "$harness_arg") || { printf 'runner-process-tap: no %s\n' "$harness_arg" >&2; exit 1; }
fi
inv="$root/data/expected/runner-scenarios.ids"
outcomes="$root/data/expected/runner-outcomes.ids"
scen="$root/data/scenarios"
for f in "$inv" "$outcomes"; do
  [ -f "$f" ] || { printf 'runner-process-tap: no %s\n' "$f" >&2; exit 1; }
done
[ -x "$ivs_run" ] || { printf 'runner-process-tap: %s is not executable\n' "$ivs_run" >&2; exit 1; }
[ -x "$pack" ] || { printf 'runner-process-tap: %s is not executable\n' "$pack" >&2; exit 1; }
[ -z "$flatwrap" ] || [ -x "$flatwrap" ] || { printf 'runner-process-tap: %s is not executable\n' "$flatwrap" >&2; exit 1; }
[ -z "$shim" ] || [ -f "$shim" ] || { printf 'runner-process-tap: %s is not a file\n' "$shim" >&2; exit 1; }
[ -z "$harness" ] || [ -x "$harness" ] || { printf 'runner-process-tap: %s is not executable\n' "$harness" >&2; exit 1; }
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
# why <token> <detail>: appends a finding to $whytarget (default $tmp/why). The indirection exists for the
# in-case negative half of the derivation judgement (plan 09-01, A-03), which runs in a subshell with its own
# target so it cannot set the case's own verdict.
why() { printf '%s: %s\n' "$1" "$2" >> "${whytarget:-$tmp/why}"; found=1; }

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

# check_roundtrip <runner> <id> <ticks> <min cuts> (plan 08-16, D-18): one more run of the pass scenario <id> with `--roundtrip-at-ticks <ticks>`:
# it must pass the judge and equal the scenario case's report apart from host.calls (check_repeat, finding body-differs), and the hook must
# have cut: every cut is one more slice and the runner makes one cancel check and one clock check per slice, so host.calls of the hook run
# exceeds the plain run's by at least twice <min cuts> (finding roundtrip-no-cut). Without that second condition a hook that never cut would
# pass the first one. <ticks> is derived by arithmetic from the scenario's run length (its golden's last step.<k>.now_ticks), see the cases below.
check_roundtrip() {
  _rr=$1; _rid=$2; _rtk=$3; _rmc=$4
  _rref="$tmp/runs/scen-$_rid.out"
  check_repeat "$tmp" "$_rr" "rt-$_rid" "$_rid" "$_rref" --roundtrip-at-ticks "$_rtk"
  _rj=$?
  _rhc=$(rkey "$tmp/runs/rt-$_rid.out" host.calls)
  _rpc=$(rkey "$_rref" host.calls)
  case $_rhc$_rpc in
    '' | *[!0-9]*) why roundtrip-no-cut "host.calls is not a number in a report of $_rid"; return 1 ;;
  esac
  if [ -z "$_rhc" ] || [ -z "$_rpc" ] || [ $((_rhc - _rpc)) -lt $((2 * _rmc)) ]; then
    why roundtrip-no-cut "host.calls $_rhc against the plain run's $_rpc: fewer than $_rmc cuts (two host calls each)"
    _rj=1
  fi
  return "$_rj"
}

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

# The frozen derivation of the Lamp Tone program: the oracle of the four demo-lamp-tone scenarios, read and never
# written (plan 09-01, D-09, A-03; GUARDRAILS 2.10, archives and oracles are append-only).
derivation="$root/docs/video/demo-lamp-tone-1-derivation.md"

# derivation_block <id> <out>: the lines strictly between `<!-- asserts-begin <id> -->` and
# `<!-- asserts-end <id> -->` of the derivation, minus its fence lines and minus its LEADING `#` lines (the frame
# literals above the scenario lines; a `#` line further down is a scenario line and is kept).
derivation_block() {
  awk -v id="$1" '/<!-- asserts-begin / { b = ($3 == id); next } /<!-- asserts-end / { if (b) b = 0; next } b' "$derivation" \
    | grep -v '^```' \
    | awk 'BEGIN { s = 1 } s && /^#/ { next } { s = 0; print }' > "$2"
}

# scenario_body <scenario file> <out>: every line after the `ivss 1` line minus the id, profile, content and
# capture lines, which is exactly what a derivation block holds (the input, run-to and assert lines).
scenario_body() {
  awk '/^ivss 1$/ { h = 1; next } h' "$1" | grep -v -E '^(id|profile|content|capture) ' > "$2"
}

# input_ticks <scenario file> <out>: the at_tick value of every input line, one per line.
input_ticks() { sed -n 's/^input at_tick=\([0-9][0-9]*\) .*$/\1/p' "$1" > "$2"; }

# derivation_equal <id> <scenario file> (plan 09-01, D-09, A-03, A-04): the preserving condition of lever (3), the
# fold of demo-lamp-tone-shift and demo-lamp-tone-swap into static repeat variants of demo-lamp-tone-pass. Without
# it the fold would be a coverage deletion. It is file-only and runs on BOTH presets, so a variant that still runs
# is judged by the frozen oracle too. In order: the scenario file exists and the derivation block of <id> is not
# empty (variant-derivation-empty, the liveness guard of the whole judgement); the assert count equals the
# derivation's `Counts:` line at docs/video/demo-lamp-tone-1-derivation.md:316 (variant-assert-count); the `ivss`,
# `profile` and `content` lines equal demo-lamp-tone-pass's and the `id` line is `id <id>` (variant-header-differs;
# amendment A-04: the frozen sentence's "id equal to the pass scenario's" is unsatisfiable, so the variant carries
# its own id); the input lines are the pass inputs transformed as the derivation documents, every tick +2,000 for
# shift (:436) and c0 and c1 exchanged for swap (:514) (variant-input-transform); and the scenario body equals the
# block line for line and in order (variant-derivation-differs). The four judgements run cheapest-and-narrowest
# first and the whole-body equality last on purpose (plan 09-01 deviation, PLAN-LESSONS 20): a token placed behind
# the full equality is masked by it and cannot fire for the reason it claims. Appends findings to $whytarget
# (default $tmp/why) and returns 1 when it adds any.
derivation_equal() {
  _di=$1; _df=$2
  if [ ! -f "$_df" ]; then
    why variant-derivation-differs "$_di: no scenario file $_df"
    return 1
  fi
  derivation_block "$_di" "$tmp/de-$_di.der"
  scenario_body "$_df" "$tmp/de-$_di.scn"
  if [ ! -s "$tmp/de-$_di.der" ]; then
    why variant-derivation-empty "$_di: the derivation block of $_di in docs/video/demo-lamp-tone-1-derivation.md is empty"
    return 1
  fi
  _dan=$(grep -c '^assert ' "$tmp/de-$_di.scn")
  _dk=${_di#demo-lamp-tone-}
  _daw=$(sed -n 's/^Counts: //p' "$derivation" | tr ',' '\n' \
    | awk -v k="$_dk" '{ gsub(/`/, "", $1); if ($1 == k) { v = $2; sub(/[^0-9].*$/, "", v); print v; exit } }')
  if [ -z "$_daw" ]; then
    why variant-assert-count "$_di: the derivation has no Counts: entry for '$_dk'"
    return 1
  fi
  if [ "$_dan" -ne "$_daw" ]; then
    why variant-assert-count "$_di: $_dan assert lines, the derivation counts $_daw"
    return 1
  fi
  _dp="$scen/demo-lamp-tone-pass.ivss"
  for _dl in ivss profile content; do
    grep "^$_dl " "$_dp" > "$tmp/de-$_di.h1"
    grep "^$_dl " "$_df" > "$tmp/de-$_di.h2"
    if [ ! -s "$tmp/de-$_di.h1" ] || ! cmp -s "$tmp/de-$_di.h1" "$tmp/de-$_di.h2"; then
      why variant-header-differs "$_di: the '$_dl' lines differ from demo-lamp-tone-pass (or are absent)"
      return 1
    fi
  done
  printf 'id %s\n' "$_di" > "$tmp/de-$_di.h1"
  grep '^id ' "$_df" > "$tmp/de-$_di.h2"
  if ! cmp -s "$tmp/de-$_di.h1" "$tmp/de-$_di.h2"; then
    why variant-header-differs "$_di: the id line is not 'id $_di'"
    return 1
  fi
  case $_di in
    demo-lamp-tone-shift)
      input_ticks "$_dp" "$tmp/de-$_di.t1"
      input_ticks "$_df" "$tmp/de-$_di.t2"
      if [ ! -s "$tmp/de-$_di.t1" ] || ! awk 'NR == FNR { p[FNR] = $1; n = FNR; next }
        { k = FNR; if (k > n || $1 + 0 != p[k] + 2000) bad = 1 }
        END { if (k != n) bad = 1; exit bad }' "$tmp/de-$_di.t1" "$tmp/de-$_di.t2"; then
        why variant-input-transform "$_di: an input tick is not the pass tick + 2,000 (docs/video/demo-lamp-tone-1-derivation.md:436)"
        return 1
      fi ;;
    demo-lamp-tone-swap)
      awk '$1 == "input" { a = $3; b = $4; sub(/^c0=/, "", a); sub(/^c1=/, "", b); print $1, $2, "c0=" b, "c1=" a }' \
        "$_dp" > "$tmp/de-$_di.t1"
      awk '$1 == "input" { print $1, $2, $3, $4 }' "$_df" > "$tmp/de-$_di.t2"
      if [ ! -s "$tmp/de-$_di.t1" ] || ! cmp -s "$tmp/de-$_di.t1" "$tmp/de-$_di.t2"; then
        why variant-input-transform "$_di: an input line is not the pass line with c0 and c1 exchanged (docs/video/demo-lamp-tone-1-derivation.md:514)"
        return 1
      fi ;;
  esac
  if ! cmp -s "$tmp/de-$_di.der" "$tmp/de-$_di.scn"; then
    why variant-derivation-differs "$_di: the scenario body differs from the derivation block (< derivation, > scenario):"
    diff "$tmp/de-$_di.der" "$tmp/de-$_di.scn" | sed -n '1,12p' >> "${whytarget:-$tmp/why}"
    return 1
  fi
  return 0
}

# variant_negative <id> <scenario file> (plan 09-01, A-03): the in-case negative half of derivation_equal, run on
# every run of the static judgement so the guard is shown able to fail every time rather than once in a scratch
# session. A copy of the scenario with one assert value changed (`assert mem 02F1 == 0003` to `== 0004`, a line
# both folded variants carry) is judged in a subshell with its own why target, so the negative half cannot set this
# case's own verdict, and it must be rejected for variant-derivation-differs. A copy the sed did not change, or one
# the judgement accepts, is judge-vacuous: the stand-in would prove nothing. It adds no test id (A-03, GUARDRAILS
# 2.2) and it writes only inside the suite's own $tmp, never into the work tree.
variant_negative() {
  _ni=$1; _nf=$2
  sed 's/^assert mem 02F1 == 0003$/assert mem 02F1 == 0004/' "$_nf" > "$tmp/neg-$_ni.ivss"
  if cmp -s "$_nf" "$tmp/neg-$_ni.ivss"; then
    why judge-vacuous "$_ni: the one-value stand-in changed nothing (no 'assert mem 02F1 == 0003' line)"
    return 1
  fi
  if ( whytarget="$tmp/neg-$_ni.why"; : > "$whytarget"; derivation_equal "$_ni" "$tmp/neg-$_ni.ivss" ); then
    why judge-vacuous "$_ni: the judgement accepted a variant body with one assert value changed"
    return 1
  fi
  if ! grep -q -e '^variant-derivation-differs: ' "$tmp/neg-$_ni.why"; then
    why judge-wrong-reason "$_ni: the one-value stand-in was rejected, but without variant-derivation-differs:"
    sed 's/^/# /' "$tmp/neg-$_ni.why" >> "${whytarget:-$tmp/why}"
    return 1
  fi
  return 0
}

# static_scenario <id> (plan 09-01, D-09, A-03): the judgement of a column-5 `release-only` scenario on a preset that
# does not run it, the static half of the fold and the same shape as mutant_case's (plan 08-lever2, D-40). It runs
# nothing and judges, in order: the scenario file exists, the derivation equality above (which refuses an empty
# derivation block), and the in-case one-value stand-in that shows the judgement can fail. It never passes on the
# strength of "nothing ran": every branch can fail. The case keeps its ordinary `ok` line, never a `# SKIP` (a skip
# would move tests-skipped 4 to 8).
static_scenario() {
  _sci=$1
  if [ ! -f "$scen/$_sci.ivss" ]; then
    why inventory "no scenario file data/scenarios/$_sci.ivss"
    return 1
  fi
  derivation_equal "$_sci" "$scen/$_sci.ivss" || return 1
  variant_negative "$_sci" "$scen/$_sci.ivss" || return 1
  return 0
}

: > "$tmp/results"
n=0
executed=0
skipped=0
failed=0
# The two counters of the diagnostic line before the trailer (plan 09-01, D-09): the column-5 `release-only`
# scenario cases that ran on this preset and those judged statically. They sum to the number of such rows, so the
# mode of every run is visible in the output.
scen_full=0
scen_static=0
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
  case $skip in run | skip-ok | release-only) ;; *) ok=0; why inventory "skip '$skip'" ;; esac
  [ -f "$scen/$id.ivss" ] || { ok=0; why inventory "no scenario file data/scenarios/$id.ivss"; }
  # The two-mode switch (plan 09-01, D-09, A-03), the shape of mutant_case (plan 08-lever2, D-40): a column-5
  # `release-only` scenario runs on the Release preset and is judged statically on any other, with the same id and
  # an ordinary ok line on both. Every demo-lamp-tone scenario is additionally held to the frozen derivation
  # whether it runs or not, so the oracle is enforced on the Release preset too.
  if [ "$ok" -eq 1 ]; then
    scen_static_now=0
    if [ "$skip" = release-only ]; then
      if [ "$config" = Release ]; then
        scen_full=$((scen_full + 1))
      else
        scen_static=$((scen_static + 1))
        scen_static_now=1
      fi
    fi
    if [ "$scen_static_now" -eq 1 ]; then
      static_scenario "$id" || ok=0
    else
      check_scenario "$ivs_run" "scen-$id" "$id" "$x" "$oc" "$args" || ok=0
      case $id in
        demo-lamp-tone-*) derivation_equal "$id" "$scen/$id.ivss" || ok=0 ;;
      esac
      if [ -n "$rep_dir" ] && [ -s "$tmp/runs/scen-$id.out" ]; then
        cp "$tmp/runs/scen-$id.out" "$rep_dir/$id.report" || { printf '# cannot write %s\n' "$rep_dir/$id.report" >&2; exit 1; }
      fi
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

# patch_changes_image <id> <listing> <image file name> <addr list> <word list> [stub]: the static half of run_mutant (plan 08-07, moved here
# and generalised by plan 08-lever2): builds the patched image in a fresh directory, with `ivs_pack --ivsc` or, with a sixth argument `stub`,
# `ivs_pack --exec`, and requires it to differ from the unpatched copy in the content dir; nothing is run. Sets $ok, adds to $tmp/why.
patch_changes_image() {
  _pd="$tmp/mutstat-$1"
  if [ "${6:-cart}" = stub ]; then _pf=--exec; else _pf=--ivsc; fi
  if ! mkdir "$_pd"; then
    ok=0; why temp "cannot prepare $_pd"
  elif ! "$pack" "$_pf" "$2" "$_pd/$3" $(patch_args "$4" "$5") > "$tmp/why.pack" 2>&1; then
    ok=0; why pack "ivs_pack --patch $4=$5 failed: $(cat "$tmp/why.pack")"
  elif cmp -s "$_pd/$3" "$content/$3"; then
    ok=0; why pack "the patch $4=$5 leaves the image unchanged"
  fi
}

# mutant_case <id> <listing> <cart file name> <scenario id> <addr list> <to list> <exit> <outcome> <detail pattern> [stub] (plan 08-lever2, D-40):
# the arguments of run_mutant, and the one place that decides what a row of any mutant inventory costs. With the build configuration Release
# (the fifth argument of this script) it is run_mutant: the full run and the judgement of outcome, exit code and detail. In any other
# configuration (the sanitized Debug preset) it is the static judgement, which runs nothing: the scenario file exists, the stated outcome
# and exit code are a pair of runner-outcomes.ids, the assert that the first clause names (the detail pattern of a guest-fail row, cut to
# `assert <text>` without its ` got <value>`) is a line of the scenario file, and the patch builds and changes the image. The caller has
# already judged the inventory form, the from words and the list lengths. Sets $ok, $ran (1 only after a run) and the counters of the
# diagnostic line at the end of the output.
mut_full=0
mut_static=0
mutant_case() {
  if [ "$config" = Release ]; then
    mut_full=$((mut_full + 1))
    run_mutant "$@"
    return 0
  fi
  mut_static=$((mut_static + 1))
  ran=0
  _cmi=$1; _cml=$2; _cmc=$3; _cms=$4; _cma=$5; _cmw=$6; _cmx=$7; _cmo=$8; _cmp=$9
  if [ "${10:-cart}" = stub ]; then _cmimg=boot-stub-2.exec; _cmk=stub; else _cmimg=$_cmc; _cmk=cart; fi
  if [ ! -f "$scen/$_cms.ivss" ]; then
    ok=0; why inventory "no scenario file data/scenarios/$_cms.ivss"
  elif ! grep -q -x -F -e "$_cmo $_cmx" "$tmp/pairs"; then
    ok=0; why inventory "outcome '$_cmo' with exit '$_cmx' is no pair of runner-outcomes.ids"
  else
    case $_cmp in
      'assert=* line=* '*)
        _cmt=${_cmp#'assert=* line=* '}
        _cmt=${_cmt% got *}
        if ! grep -q -x -F -e "assert $_cmt" "$scen/$_cms.ivss"; then
          ok=0; why clause "the scenario $_cms has no line 'assert $_cmt'"
        fi ;;
    esac
    if [ "$ok" -eq 1 ]; then patch_changes_image "$_cmi" "$_cml" "$_cmimg" "$_cma" "$_cmw" "$_cmk"; fi
  fi
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
    mutant_case "$mid" "$listing" diag-cpu-1.ivsc diag-boot-pass "$addr" "$word" 10 guest-fail "assert=* line=* $clause got *"
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
    [ "$ok" -eq 1 ] && mutant_case "$mid" "$listing2" diag-ctrl-psg-1.ivsc diag-ctrl-psg-pass "$addr" "$to" "$mx" "$mo" "$mp"
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
    [ "$ok" -eq 1 ] && mutant_case "$mid" "$listing3" diag-stic-1.ivsc "$scn" "$addrs" "$tos" "$mx" "$mo" "$mp"
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
        mutant_case "$mid" "$listing4s" lamp-tone.ivsc "$scn" "$addrs" "$tos" "$mx" "$mo" "$mp" stub
      else
        mutant_case "$mid" "$listing4" lamp-tone.ivsc "$scn" "$addrs" "$tos" "$mx" "$mo" "$mp"
      fi
      if [ "$ran" -eq 1 ] && [ -n "$fails" ]; then check_fails "mut-$mid" "$fails" || ok=0; fi
    fi
  fi
  result "runner-process.mutant.$mid"
done < "$muts4"

# Program mutations of the second original program (plan 08-07, D-10, D-37, D-40): per line
# `<ID> <scenario> <addr list> <from list> <to list> <clause> <outcome> <exit> [fails=<k.j,...>]` of data/expected/diag-mutations-stic2.ids, the
# grammar of the scn inventory above (a sibling loop of it). The word at each <addr> of the unpatched listing of ivs-diag-stic-2
# (ivs_pack --dump-words) must be its <from>. When the fifth argument of this script, the build configuration, is Release the row is run and
# judged like a scn row (outcome and exit code of the named scenario against outcome.detail, then with `fails=` the complete set of failing
# asserts, check_fails). In any other configuration the row is judged statically by mutant_case (the checks above plus patch_changes_image)
# and reported ok without running the scenario; the ids and their number are the same in both presets, and none is skip-ok. Since plan
# 08-lever2 the choice between the two lives in mutant_case, shared with the four loops above.
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
      mutant_case "$mid" "$listing5" diag-stic-2.ivsc "$scn" "$addrs" "$tos" "$mx" "$mo" "$mp"
      if [ "$ran" -eq 1 ] && [ -n "$fails" ]; then check_fails "mut-$mid" "$fails" || ok=0; fi
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

# The round-trip hook (plan 08-16, D-18): a run with --roundtrip-at-ticks saves, resets and restores the machine at every multiple of the value,
# and must still give the scenario's plain report apart from host.calls, with at least the stated number of cuts (check_roundtrip). Each value
# is derived by arithmetic from the run length of the scenario's golden (the last step.<k>.now_ticks, or the run-to bound of a scenario that never
# halts); the frame is 59,736 master ticks and no value is a multiple of it, so no cut sits on a frame boundary by accident. A cut is made at the
# first unit boundary at or after a multiple, never at the end of a run-to, and a multiple that is a step bound or an input tick is avoided.
#   diag-boot-pass       halts at tick 2,084 (data/golden/diag-cpu-1.digests step.1.now_ticks): 600 cuts after 600, 1,200 and 1,800 (three; 2,400 lies
#                        past the halt), the fewest that gives two inside the run. No display.
#   diag-ctrl-psg-pass   halts at tick 16,752 with seven input events at ticks 4,000 to 8,800: 5,000 cuts after 5,000, 10,000 and 15,000 (three), the first
#                        between the second and the third event (two applied, a pull-down in force), the other two after the last event (8,800). No display
#                        (the run ends inside frame 0, which ends at 59,736).
#   diag-stic-fgbg-pass  halts at tick 88,476, the display on from frame 0 (the golden delivers one frame, ending at 59,736): 20,000 cuts after 20,000, 40,000,
#                        60,000 and 80,000 (four): two inside the displayed frame 0 (at 20,000 and 40,000), one 264 ticks into frame 1 (66 CPU cycles, inside
#                        the interrupt service of the frame end, not on the boundary) and one 20,264 ticks into frame 1.
#   demo-lamp-tone-pass  the four run-to bounds 200,000, 440,000, 640,000 and 806,568 (step.4.now_ticks 806,568), six input events at ticks 100,000 to 700,000:
#                        90,000 cuts after 90,000 (frame 1, 30,264 ticks in, the first displayed frame), 180,000 (frame 3, 792 ticks in: inside the routine
#                        of the frame), 270,000, 360,000 (frame 6, 1,584 in), 450,000, 540,000, 630,000 and 720,000 (eight; 810,000 lies past the end). No multiple is a step
#                        bound (the cut at the end of a run-to is never made) or an input tick.
#   diag-stic2-collision-pass  the one run-to bound 640,000 (step.1.now_ticks 640,004), the program never halts, the display on from frame 1: 150,000 cuts after
#                        150,000 (frame 2, which spans 119,472 to 179,207: 30,528 ticks in, so in the frame whose collision registers C0 to C7 are 0002 0001 0008
#                        0004 0000 0000 0100 0000, set at the end of frame 1, and whose scene 8 MOB words the routine of frame 2 wrote), 300,000 (frame 5, 1,320 in: inside
#                        its routine), 450,000 (frame 7, 31,848 in) and 600,000 (frame 10, 2,640 in) (four). The scenario's own asserts judge what the cut in
#                        frame 2 carried: the guest reads C0 to C7 in the routine of frame 3 and stores them at 0300 to 0307, and the asserts mem 0300 == 0002,
#                        0301 == 0001, 0302 == 0008, 0303 == 0004, 0306 == 0100 (and 0304, 0305, 0307 == 0000) hold in the report of the hook run only if the
#                        restore brought the collision bits and the registers back; the frame 5 pixel asserts and video count judge the MOB scenes after the cuts.
: > "$tmp/why"
ok=1
check_roundtrip "$ivs_run" diag-boot-pass 600 2 || ok=0
result runner-process.roundtrip.diag-boot-pass
: > "$tmp/why"
ok=1
check_roundtrip "$ivs_run" diag-ctrl-psg-pass 5000 2 || ok=0
result runner-process.roundtrip.diag-ctrl-psg-pass
: > "$tmp/why"
ok=1
check_roundtrip "$ivs_run" diag-stic-fgbg-pass 20000 2 || ok=0
result runner-process.roundtrip.diag-stic-fgbg-pass
: > "$tmp/why"
ok=1
check_roundtrip "$ivs_run" demo-lamp-tone-pass 90000 2 || ok=0
result runner-process.roundtrip.demo-lamp-tone-pass
: > "$tmp/why"
ok=1
check_roundtrip "$ivs_run" diag-stic2-collision-pass 150000 2 || ok=0
result runner-process.roundtrip.diag-stic2-collision-pass
# The refusals of the option (plan 08-16, D-18; trimmed by plan 09-01, D-09, trim (a)): a zero, one above IVS_RUN_TARGET_MAX (9223372036854775808) as
# decimal digits, a missing value (the option last on the line) and the option twice are each a usage error: exit 14, no report on stdout, and on stderr the
# one message line of every usage error followed by the usage text (the usage block names the option).
# The `zero` launch is the ONE live launch and stays live: it alone proves exit 14, the empty report, the first stderr line and the usage text together, and
# its four judgements are unchanged. `above-max`, `no-value` and `twice` are judged statically, against the parser's own guard sites, which are all in
# runner/main.c and nowhere else (there is no option table in this tree: the parser is a strcmp chain at runner/main.c:588-624, and runner/exec.c holds
# neither the option name nor either refusal message - recorded deviation D-09-02). Each static judgement can fail for the reason it names, and the id keeps
# its single result call, so tests-executed does not move.
: > "$tmp/why"
ok=1
# hex_plus_one <0x...>: the decimal string of that value plus one, computed with decimal-string arithmetic so no shell or awk
# integer width is involved (POSIX awk has no strtonum, and $((0x8000000000000000)) is negative in a signed 64-bit shell).
hex_plus_one() {
  printf '%s\n' "$1" | sed 's/^0[xX]//' | awk '
    {
      h = tolower($0)
      nd = 1; D[1] = 0
      for (i = 1; i <= length(h); i++) {
        v = index("0123456789abcdef", substr(h, i, 1)) - 1
        if (v < 0) { exit 1 }
        c = v
        for (j = 1; j <= nd; j++) { t = D[j] * 16 + c; D[j] = t % 10; c = int(t / 10) }
        while (c > 0) { nd++; D[nd] = c % 10; c = int(c / 10) }
      }
      c = 1
      for (j = 1; j <= nd && c; j++) { t = D[j] + c; D[j] = t % 10; c = int(t / 10) }
      if (c) { nd++; D[nd] = c }
      s = ""
      for (j = nd; j >= 1; j--) s = s D[j]
      print s
    }'
}
# rt_arm <out>: the `--roundtrip-at-ticks` arm of the parser (runner/main.c:589-592), from its strcmp line to the next arm.
rt_arm() {
  awk '/strcmp\(a, "--roundtrip-at-ticks"\) == 0/ { p = 1; print; next } p && /^    } else if/ { exit } p { print }' \
    "$root/runner/main.c" > "$1"
}
# rt_usage <out>: the usage block of the runner (runner/main.c:76 names the option).
rt_usage() { grep -F -e '--roundtrip-at-ticks <n>' "$root/runner/main.c" > "$1"; }
rt_refuse() {
  _rn=$1; _rm=$2; shift 2
  run_one "$tmp" "rt-refuse-$_rn" "$ivs_run" --content-dir "$content" "$@"
  [ "$rc" -eq 14 ] && [ "$hung" -eq 0 ] && [ ! -s "$tmp/runs/rt-refuse-$_rn.out" ] || { ok=0; why roundtrip-refuse "$_rn: exit $rc, hung $hung or a report"; }
  [ "$(sed -n 1p "$tmp/runs/rt-refuse-$_rn.err")" = "ivs-run: $_rm" ] || { ok=0; why roundtrip-refuse "$_rn: the first stderr line is not 'ivs-run: $_rm'"; }
  [ "$(sed -n 2p "$tmp/runs/rt-refuse-$_rn.err")" = 'usage: ivs-run [options] <scenario.ivss>' ] || { ok=0; why roundtrip-refuse "$_rn: the usage text does not follow"; }
  grep -q -F -e '--roundtrip-at-ticks <n>' "$tmp/runs/rt-refuse-$_rn.err" || { ok=0; why roundtrip-refuse "$_rn: the usage text does not name the option"; }
}
rt_refuse zero 'option value out of range' --roundtrip-at-ticks 0 "$scen/diag-boot-pass.ivss"
# above-max, statically: the literal is one above IVS_RUN_TARGET_MAX, the arm binds that macro as its high limit and leaves `slot` NULL, and the range check
# (runner/main.c:623-624) refuses a value above it. A changed macro, a changed literal or an arm that stops binding the bound fails here.
rt_max_dec=9223372036854775809
rt_arm "$tmp/rt-arm"
_rtm=$(sed -n 's/^#define IVS_RUN_TARGET_MAX \(0x[0-9a-fA-F][0-9a-fA-F]*\)u*$/\1/p' "$root/core/include/ivs/ivs_run.h")
_rtd=$(hex_plus_one "$_rtm" 2> /dev/null)
if [ -z "$_rtm" ]; then
  ok=0; why rt-static "above-max: no '#define IVS_RUN_TARGET_MAX 0x...' in core/include/ivs/ivs_run.h"
elif [ -z "$_rtd" ] || [ "$_rtd" != "$rt_max_dec" ]; then
  ok=0; why rt-static "above-max: IVS_RUN_TARGET_MAX $_rtm is not one below the judged literal $rt_max_dec"
elif ! grep -q -F -e 'Largest accepted target: 2^63' "$root/core/include/ivs/ivs_run.h"; then
  ok=0; why rt-static "above-max: the header comment no longer names the 2^63 bound"
elif ! grep -q -F -e 'seen = &have_roundtrip' "$tmp/rt-arm" || ! grep -q -F -e 'hi = IVS_RUN_TARGET_MAX' "$tmp/rt-arm"; then
  ok=0; why rt-static "above-max: the --roundtrip-at-ticks arm (runner/main.c:589-592) does not bind seen and hi = IVS_RUN_TARGET_MAX"
elif grep -q -F -e 'slot =' "$tmp/rt-arm"; then
  ok=0; why rt-static "above-max: the --roundtrip-at-ticks arm binds a slot, so it is no longer the seen arm this judgement reads"
elif ! grep -q -F -e 'if (!main_number(argv[i], lo, hi, &v)) {' "$root/runner/main.c" \
  || ! grep -q -F -e 'return main_usage_error("option value out of range");' "$root/runner/main.c"; then
  ok=0; why rt-static "above-max: the range check at runner/main.c:623-624 is gone"
fi
# no-value, statically: the arm binds `seen` (not `slot`) and consumes no value of its own, so a missing value reaches the refusal at runner/main.c:609-610;
# the usage text names the option, which the live `zero` launch proves is printed. An arm that starts consuming its own value, or that becomes a slot arm, fails.
rt_usage "$tmp/rt-usage"
if ! grep -q -F -e 'seen = &have_roundtrip' "$tmp/rt-arm"; then
  ok=0; why rt-static "no-value: the --roundtrip-at-ticks arm (runner/main.c:589-592) does not bind seen, so a missing value has no refusal"
elif grep -q -F -e 'slot =' "$tmp/rt-arm" || grep -q -F -e 'continue' "$tmp/rt-arm"; then
  ok=0; why rt-static "no-value: the --roundtrip-at-ticks arm consumes its own value or leaves the value tail, so runner/main.c:609-610 is unreachable for it"
elif ! grep -q -F -e "if (i + 1 >= argc || argv[i + 1][0] == '\\0') {" "$root/runner/main.c" \
  || ! grep -q -F -e 'return main_usage_error("option without a value");' "$root/runner/main.c"; then
  ok=0; why rt-static "no-value: the missing-value refusal at runner/main.c:609-610 is gone"
elif [ ! -s "$tmp/rt-usage" ]; then
  ok=0; why rt-static "no-value: the usage block (runner/main.c:76) no longer names --roundtrip-at-ticks <n>"
fi
# twice, statically: the same `seen` binding routes a second occurrence to the `*seen` branch at runner/main.c:620-621 (`slot` is NULL for this option,
# so the slot branch at :613-618 is not the one that fires). An arm that becomes repeatable by design, or that stops binding seen, fails.
if ! grep -q -F -e 'seen = &have_roundtrip' "$tmp/rt-arm"; then
  ok=0; why rt-static "twice: the --roundtrip-at-ticks arm (runner/main.c:589-592) does not bind seen, so a repeat has no refusal"
elif grep -q -F -e 'slot =' "$tmp/rt-arm"; then
  ok=0; why rt-static "twice: the --roundtrip-at-ticks arm is a slot arm, so the slot branch at runner/main.c:613-618 is the one that would fire"
elif ! grep -q -F -e 'if (*seen) {' "$root/runner/main.c" \
  || ! grep -q -F -e 'return main_usage_error("option given twice");' "$root/runner/main.c"; then
  ok=0; why rt-static "twice: the seen branch at runner/main.c:620-621 is gone"
elif [ ! -s "$tmp/rt-usage" ]; then
  ok=0; why rt-static "twice: the usage block (runner/main.c:76) no longer names --roundtrip-at-ticks <n>"
fi
result runner-process.roundtrip.refuse-range

# The input log at the process boundary (plan 11-04, TOOL-01, D-15, A-11-01, A-11-03): the phase's only process case, and the
# only place a produced log is a file at all. It is one id with every assertion below, because they share the same four runs
# and the milestone allows zero new gate steps.
#   Record      `demo-lamp-tone-pass` with `--record-input` into a mktemp directory: a 319-byte log whose four header lines
#               are the scenario's own and whose six `at_tick` lines carry the scenario's own six ticks in order, with the
#               report saying input.events 6 and input.consumed 6. The event list is checked against the scenario, not
#               against itself, so a log that listed its own events would be caught.
#   Exclusive   the same command on the same path: exit 14, one stderr line, no report, and `cmp` equality with a saved copy,
#               so the file was neither overwritten nor truncated (T-11-13).
#   Stripped    `sed '/^input /d'` makes a copy with 0 `input` lines that keeps its `id` and its four `run-to` lines. This is
#               A-11-03: with the scenario's own `input` lines present, a `--replay-input` that parsed the file and ignored it
#               would produce a byte-identical report and every assertion below would still pass.
#   Replay      the stripped copy with `--replay-input`: the recorded report byte for byte, `host.calls` included, all nine
#               `digest.*` lines present, non-`-` and equal, the four live counts nonzero, and `digest.input` equal to the
#               committed golden's value on both runs. That last comparison is the frozen oracle, not the recorded run alone.
#               Nine domains are observable here and only here: the in-process fake host carries two images and produces no
#               video, no audio and no PSG write (A-11-01).
#   No log      the same stripped copy with no `--replay-input`: a different report with input.events 0. Without this the
#               equality above would not be shown to depend on the log at the process boundary.
#   Rights      the real 319-byte file, with the `scenario` and `profile` values masked and their shape pinned, carries zero
#               runs of 32 or more hex digits; the same masked text with the golden's `digest.input` line appended carries at
#               least one, so the check is shown able to fail (A-11-05, lesson 20).
#   Header-only `diag-boot-pass` has no `input` line: it is recordable, its report says input.events 0, and its log is exactly
#               the four header lines, so a run that applied no event is not refused for having nothing to write (O-3).
#   Refusals    a missing, an oversize (4,200 bytes), a malformed log and the two options together: exit 14, no report, and for
#               the mutual exclusion no file created either - the observable form of the ordering that makes "no state change"
#               a property (T-11-15). The usage text names both options.
#   Posture     the two `content ... user` refusal blocks of runner/main.c name exactly the five derived-art options and no log
#               option, so the log's exemption cannot drift by someone adding a line (T-11-17, D15).
mkdir "$tmp/rec" || exit 1
# hex32_runs: the number of runs of 32 or more hex digits in the text on stdin, one run counted once however long it is.
hex32_runs() {
  awk '{
    s = $0
    while (length(s) > 0) {
      if (match(s, /[0-9A-Fa-f]+/)) {
        if (RLENGTH >= 32) { total++ }
        s = substr(s, RSTART + RLENGTH)
      } else { break }
    }
  } END { print total + 0 }'
}
# rec_refuse <name> <expected first stderr line> <args...>: exit 14, no report and exactly that one line on stderr.
rec_refuse() {
  _rn=$1; _rm=$2; shift 2
  run_one "$tmp" "rec-refuse-$_rn" "$ivs_run" --content-dir "$content" "$@"
  [ "$rc" -eq 14 ] && [ "$hung" -eq 0 ] && [ ! -s "$tmp/runs/rec-refuse-$_rn.out" ] || { ok=0; why record-refuse "$_rn: exit $rc, hung $hung or a report"; }
  [ "$(sed -n 1p "$tmp/runs/rec-refuse-$_rn.err")" = "ivs-run: $_rm" ] || { ok=0; why record-refuse "$_rn: the first stderr line is not 'ivs-run: $_rm'"; }
  [ "$(wc -l < "$tmp/runs/rec-refuse-$_rn.err" | tr -d ' ')" -eq 1 ] || { ok=0; why record-refuse "$_rn: stderr is not exactly one line"; }
}
: > "$tmp/why"
ok=1
# Record.
run_one "$tmp" rec-record "$ivs_run" --content-dir "$content" --record-input "$tmp/rec/pass.ivsl" "$scen/demo-lamp-tone-pass.ivss"
judge rec-record 0 pass - "" || ok=0
rec_log="$tmp/rec/pass.ivsl"
[ "$(wc -c < "$rec_log" | tr -d ' ')" -eq 319 ] || { ok=0; why record-log "the log is $(wc -c < "$rec_log" | tr -d ' ') bytes, expected 319"; }
for _h in 'ivsl 1' 'scenario demo-lamp-tone-pass' 'profile intv-2609-ntsc-family-p1' 'content project'; do
  grep -q -x -F -e "$_h" "$rec_log" || { ok=0; why record-log "the header line '$_h' is absent"; }
done
[ "$(sed -n 1p "$rec_log")" = 'ivsl 1' ] || { ok=0; why record-log "the first line is not 'ivsl 1'"; }
[ "$(sed -n 2p "$rec_log")" = 'scenario demo-lamp-tone-pass' ] || { ok=0; why record-log "the second line is not the scenario's"; }
[ "$(sed -n 3p "$rec_log")" = 'profile intv-2609-ntsc-family-p1' ] || { ok=0; why record-log "the third line is not the profile's"; }
[ "$(sed -n 4p "$rec_log")" = 'content project' ] || { ok=0; why record-log "the fourth line is not 'content project'"; }
[ "$(grep -c -E '^at_tick [0-9]+ c0=[0-9A-F]{8} c1=[0-9A-F]{8}$' "$rec_log")" -eq 6 ] || { ok=0; why record-log "the log does not carry six well-formed event lines"; }
_ticks=$(sed -n 's/^at_tick \([0-9][0-9]*\) .*$/\1/p' "$rec_log" | tr '\n' ' ')
[ "$_ticks" = '100000 220000 340000 460000 580000 700000 ' ] || { ok=0; why record-log "the event ticks are '$_ticks', not the scenario's six"; }
[ "$(rkey "$tmp/runs/rec-record.out" input.events)" = 6 ] || { ok=0; why record-log "input.events is '$(rkey "$tmp/runs/rec-record.out" input.events)', expected 6"; }
[ "$(rkey "$tmp/runs/rec-record.out" input.consumed)" = 6 ] || { ok=0; why record-log "input.consumed is '$(rkey "$tmp/runs/rec-record.out" input.consumed)', expected 6"; }
# Exclusive create: the same path again is refused and the file is untouched.
cp "$rec_log" "$tmp/rec/first"
run_one "$tmp" rec-exists "$ivs_run" --content-dir "$content" --record-input "$rec_log" "$scen/demo-lamp-tone-pass.ivss"
[ "$rc" -eq 14 ] && [ "$hung" -eq 0 ] && [ ! -s "$tmp/runs/rec-exists.out" ] || { ok=0; why record-exists "exit $rc, hung $hung or a report"; }
[ "$(sed -n 1p "$tmp/runs/rec-exists.err")" = 'ivs-run: --record-input: output file exists, refusing to overwrite it' ] || { ok=0; why record-exists "the first stderr line is not the exclusive-create refusal"; }
[ "$(wc -l < "$tmp/runs/rec-exists.err" | tr -d ' ')" -eq 1 ] || { ok=0; why record-exists "stderr is not exactly one line"; }
cmp -s "$tmp/rec/first" "$rec_log" || { ok=0; why record-exists "the refused run changed the file"; }
# The stripped copy (A-11-03).
sed '/^input /d' "$scen/demo-lamp-tone-pass.ivss" > "$tmp/rec/pass-noinput.ivss"
[ "$(grep -c '^input ' "$tmp/rec/pass-noinput.ivss")" -eq 0 ] || { ok=0; why record-strip "the copy still carries input lines"; }
grep -q -x -F -e 'id demo-lamp-tone-pass' "$tmp/rec/pass-noinput.ivss" || { ok=0; why record-strip "the copy lost its id line"; }
[ "$(grep -c '^run-to ' "$tmp/rec/pass-noinput.ivss")" -eq 4 ] || { ok=0; why record-strip "the copy does not keep the four run-to lines"; }
# Replay into the stripped copy.
run_one "$tmp" rec-replay "$ivs_run" --content-dir "$content" --replay-input "$rec_log" "$tmp/rec/pass-noinput.ivss"
judge rec-replay 0 pass - "" || ok=0
cmp -s "$tmp/runs/rec-record.out" "$tmp/runs/rec-replay.out" || {
  ok=0; why record-replay "the replayed report differs from the recorded one (< recorded, > replayed):"
  diff "$tmp/runs/rec-record.out" "$tmp/runs/rec-replay.out" | sed -n '1,12p' >> "$tmp/why"
}
[ "$(rkey "$tmp/runs/rec-replay.out" input.events)" = 6 ] || { ok=0; why record-replay "the replay's input.events is not 6"; }
[ "$(rkey "$tmp/runs/rec-replay.out" input.consumed)" = 6 ] || { ok=0; why record-replay "the replay's input.consumed is not 6"; }
for _k in digest.cpu digest.mem digest.images digest.instance digest.composite digest.video digest.audio digest.input digest.psglog; do
  _va=$(rkey "$tmp/runs/rec-record.out" "$_k")
  _vb=$(rkey "$tmp/runs/rec-replay.out" "$_k")
  [ -n "$_va" ] && [ "$_va" != - ] || { ok=0; why record-digest "$_k is absent or - in the recorded report"; }
  [ -n "$_vb" ] && [ "$_vb" != - ] || { ok=0; why record-digest "$_k is absent or - in the replayed report"; }
  [ "$_va" = "$_vb" ] || { ok=0; why record-digest "$_k differs between the recorded and the replayed report"; }
done
for _k in output.video.bytes output.audio.bytes output.psglog.records input.events; do
  for _r in rec-record rec-replay; do
    _v=$(rkey "$tmp/runs/$_r.out" "$_k")
    case $_v in '' | 0 | *[!0-9]*) ok=0; why record-live "$_k is '$_v' in $_r, expected a nonzero count" ;; esac
  done
done
_goldin=$(sed -n 's/^digest\.input //p' "$root/data/golden/demo-lamp-tone-pass.digests")
[ "$_goldin" = '832906ce775d32a98ae607386ce4bbcdd4d08017e3b914b6089e50a5364c7761' ] || { ok=0; why record-golden "the golden's digest.input is '$_goldin'"; }
for _r in rec-record rec-replay; do
  [ "$(rkey "$tmp/runs/$_r.out" digest.input)" = "$_goldin" ] || { ok=0; why record-golden "digest.input of $_r is not the committed golden's value"; }
done
# The no-log witness: the same stripped copy without the option must differ.
run_one "$tmp" rec-nolog "$ivs_run" --content-dir "$content" "$tmp/rec/pass-noinput.ivss"
judge rec-nolog 10 guest-fail 'assert=4 line=27 mem 02F2 == 007E got 00FF' "" || ok=0
cmp -s "$tmp/runs/rec-record.out" "$tmp/runs/rec-nolog.out" && { ok=0; why record-nolog "the run with no --replay-input gave the recorded report: the equality does not depend on the log"; }
[ "$(rkey "$tmp/runs/rec-nolog.out" input.events)" = 0 ] || { ok=0; why record-nolog "the run with no --replay-input says input.events $(rkey "$tmp/runs/rec-nolog.out" input.events)"; }
# The rights check on the file the runner actually wrote (A-11-05): mask the two name fields, pin their shape.
sed -e 's/^scenario .*/scenario x/' -e 's/^profile .*/profile x/' "$rec_log" > "$tmp/rec/masked.ivsl"
[ "$(hex32_runs < "$tmp/rec/masked.ivsl")" -eq 0 ] || { ok=0; why record-rights "the produced log carries $(hex32_runs < "$tmp/rec/masked.ivsl") runs of 32 or more hex digits, expected 0"; }
printf 'digest.input %s\n' "$_goldin" > "$tmp/rec/standin.ivsl"
cat "$tmp/rec/masked.ivsl" "$tmp/rec/standin.ivsl" > "$tmp/rec/withdigest.ivsl"
[ "$(hex32_runs < "$tmp/rec/withdigest.ivsl")" -ge 1 ] || { ok=0; why record-rights "the stand-in carries no 32-hex run: the check cannot fail"; }
_slug=$(sed -n 's/^scenario //p' "$rec_log")
_prof=$(sed -n 's/^profile //p' "$rec_log")
case $_slug in '' | *[!a-z0-9-]*) ok=0; why record-rights "the masked scenario value '$_slug' is outside [a-z0-9-]" ;; esac
[ "${#_slug}" -ge 1 ] && [ "${#_slug}" -le 48 ] || { ok=0; why record-rights "the masked scenario value is ${#_slug} bytes, outside 1..48"; }
case $_prof in '' | *[!a-z0-9-]*) ok=0; why record-rights "the masked profile value '$_prof' is outside [a-z0-9-]" ;; esac
[ "${#_prof}" -ge 1 ] && [ "${#_prof}" -le 64 ] || { ok=0; why record-rights "the masked profile value is ${#_prof} bytes, outside 1..64"; }
# A header-only log is valid (research O-3).
run_one "$tmp" rec-headonly "$ivs_run" --content-dir "$content" --record-input "$tmp/rec/head.ivsl" "$scen/diag-boot-pass.ivss"
judge rec-headonly 0 pass - "" || ok=0
[ "$(rkey "$tmp/runs/rec-headonly.out" input.events)" = 0 ] || { ok=0; why record-headonly "input.events is not 0"; }
[ "$(wc -l < "$tmp/rec/head.ivsl" | tr -d ' ')" -eq 4 ] || { ok=0; why record-headonly "the log is not four lines"; }
[ "$(grep -c '^at_tick ' "$tmp/rec/head.ivsl")" -eq 0 ] || { ok=0; why record-headonly "a log of a scenario with no input carries an event line"; }
_hl=0
while IFS= read -r _ln; do _hl=$((_hl + ${#_ln} + 1)); done < "$tmp/rec/head.ivsl"
[ "$_hl" -eq 80 ] && [ "$(wc -c < "$tmp/rec/head.ivsl" | tr -d ' ')" -eq "$_hl" ] || {
  ok=0; why record-headonly "the header-only log is $(wc -c < "$tmp/rec/head.ivsl" | tr -d ' ') bytes, its four lines give $_hl, expected 80"
}
# A run that never reached the executor names its empty log (review WR-01): `needs-user-exec` is a designed
# missing-fixture outcome (exit 13), so the file created at the exclusive create stays empty and the tool says so on one
# stderr line. The file is left in place, the same choice `--frame-ppm` and `--video-raw` make for a run that delivered
# nothing, rather than removed: an empty .ivsl is describable and a silent removal of a path the operator named would be
# the only such place. The report is unchanged.
run_one "$tmp" rec-mf "$ivs_run" --content-dir "$content" --record-input "$tmp/rec/mf.ivsl" "$scen/needs-user-exec.ivss"
[ "$rc" -eq 13 ] && [ "$hung" -eq 0 ] || { ok=0; why record-empty "exit $rc, hung $hung, expected 13"; }
[ "$(rkey "$tmp/runs/rec-mf.out" outcome)" = 'missing-fixture' ] || { ok=0; why record-empty "the report's outcome is not missing-fixture"; }
[ "$(sed -n 1p "$tmp/runs/rec-mf.err")" = 'ivs-run: --record-input: no event log was written, the file is empty' ] || {
  ok=0; why record-empty "the empty log is not named on stderr: '$(sed -n 1p "$tmp/runs/rec-mf.err")'"
}
[ "$(wc -l < "$tmp/runs/rec-mf.err" | tr -d ' ')" -eq 1 ] || { ok=0; why record-empty "stderr is not exactly one line"; }
[ -f "$tmp/rec/mf.ivsl" ] || { ok=0; why record-empty "the created log file is gone"; }
[ -f "$tmp/rec/mf.ivsl" ] && [ "$(wc -c < "$tmp/rec/mf.ivsl" | tr -d ' ')" -eq 0 ] || { ok=0; why record-empty "the log file is not empty"; }
# The refusals.
rec_refuse missing 'log not readable: missing' --replay-input "$tmp/rec/nope.ivsl" "$scen/demo-lamp-tone-pass.ivss"
head -c 4200 /dev/zero | tr '\0' 'a' > "$tmp/rec/big.ivsl"
rec_refuse oversize 'log not readable: too large' --replay-input "$tmp/rec/big.ivsl" "$scen/demo-lamp-tone-pass.ivss"
printf 'digest.input %s\n' "$_goldin" > "$tmp/rec/bad.ivsl"
rec_refuse malformed '--replay-input: line 1 reason unknown-directive' --replay-input "$tmp/rec/bad.ivsl" "$scen/demo-lamp-tone-pass.ivss"
rec_refuse directory 'log not readable: not a regular file' --replay-input "$tmp/rec" "$scen/demo-lamp-tone-pass.ivss"
run_one "$tmp" rec-refuse-both "$ivs_run" --content-dir "$content" --record-input "$tmp/rec/both.ivsl" --replay-input "$rec_log" "$scen/demo-lamp-tone-pass.ivss"
[ "$rc" -eq 14 ] && [ "$hung" -eq 0 ] && [ ! -s "$tmp/runs/rec-refuse-both.out" ] || { ok=0; why record-refuse "both: exit $rc, hung $hung or a report"; }
[ "$(sed -n 1p "$tmp/runs/rec-refuse-both.err")" = 'ivs-run: --record-input and --replay-input are exclusive' ] || { ok=0; why record-refuse "both: the first stderr line is not the exclusion message"; }
[ "$(sed -n 2p "$tmp/runs/rec-refuse-both.err")" = 'usage: ivs-run [options] <scenario.ivss>' ] || { ok=0; why record-refuse "both: the usage text does not follow"; }
[ ! -e "$tmp/rec/both.ivsl" ] || { ok=0; why record-refuse "both: the refusal created a file"; }
# The usage text names both options, in --help and in a refusal's stderr.
run_one "$tmp" rec-help "$ivs_run" --help
[ "$rc" -eq 0 ] || { ok=0; why record-usage "--help exited $rc"; }
for _o in '--record-input <file>' '--replay-input <file>'; do
  grep -q -F -e "$_o" "$tmp/runs/rec-help.out" || { ok=0; why record-usage "--help does not name '$_o'"; }
  grep -q -F -e "$_o" "$tmp/runs/rec-refuse-both.err" || { ok=0; why record-usage "the refusal's usage text does not name '$_o'"; }
done
# The rights posture is a checked property (T-11-17, D15): the two `content ... user` blocks name exactly the five
# derived-art options and no log option.
rec_block() {
  awk -v start="$2" 'NR < start { next } NR == start { p = 1 } p { print } p && NR > start && /^  }$/ { exit }' \
    "$root/runner/main.c" > "$1"
}
_b1=$(grep -n -F -e 'if (frame_ppm_path != NULL || video_raw_path != NULL) {' "$root/runner/main.c" | sed -n 's/:.*//p')
_b2=$(grep -n -F -e 'if (audio_raw_path != NULL || psg_log_path != NULL || audio_wav_path != NULL) {' "$root/runner/main.c" | sed -n 's/:.*//p')
if [ -z "$_b1" ] || [ -z "$_b2" ]; then
  ok=0; why record-posture "a content-user refusal block of runner/main.c is gone"
else
  rec_block "$tmp/rec/block1" "$_b1"
  rec_block "$tmp/rec/block2" "$_b2"
  _o1=$(grep -o -E -- '--[a-z][a-z-]*' "$tmp/rec/block1" | sort -u | tr '\n' ' ')
  _o2=$(grep -o -E -- '--[a-z][a-z-]*' "$tmp/rec/block2" | sort -u | tr '\n' ' ')
  [ "$_o1" = '--frame-ppm --video-raw ' ] || { ok=0; why record-posture "the frame refusal block names '$_o1', expected '--frame-ppm --video-raw '"; }
  [ "$_o2" = '--audio-raw --audio-wav --psg-log-out ' ] || { ok=0; why record-posture "the audio refusal block names '$_o2', expected '--audio-raw --audio-wav --psg-log-out '"; }
  for _o in --record-input --replay-input; do
    grep -q -F -e "$_o" "$tmp/rec/block1" && { ok=0; why record-posture "the frame refusal block names $_o"; }
    grep -q -F -e "$_o" "$tmp/rec/block2" && { ok=0; why record-posture "the audio refusal block names $_o"; }
  done
fi
result runner-process.record.command-line

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
  # The pair takes the same two-mode switch as its scenarios (plan 09-01, D-09, A-03): a column-5 `release-only`
  # member has no report on the sanitized preset, so there the line is judged by its inventory form alone (the four
  # columns, both ids deterministic lines of $inv, neither key list vacuous, differ=- equal=- refused) and
  # check_pair runs on the Release preset only. data/expected/scenario-pairs.ids keeps its four columns: a fifth
  # column there would break nine pair lines at once.
  pstat=0
  if [ "$config" != Release ]; then
    for _pi in "$pa" "$pb"; do
      awk -v i="$_pi" '$1 == i && $5 == "release-only" { f = 1 } END { exit !f }' "$inv" && pstat=1
    done
  fi
  if [ "$ok" -eq 1 ] && [ "$pstat" -eq 0 ]; then
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

# The cartridge role of the owner-local route (plan 14-03, SCN-03, criterion 4, D-47): a scenario that names all three user roles
# (`content exec user`, `content grom user`, `content cart user`) is refused for every one of the six derived-output options - exit 14,
# exactly one stderr line, no report on stdout and no file at the named path - and the snapshot rule is proved to be the ABSENCE of a
# seventh refusal: `--roundtrip-at-ticks` is accepted for the same scenario, writes nothing, and leaves the report equal to the plain
# run apart from host.calls. The runner's three refusal loops are already role-generic (runner/main.c: one loop over IVS_SCN_ROLES per
# output family), so this case is proof, not implementation. Project-made files only: zero-filled exec/grom images and a container the
# project's own wrap tool writes from a zero-filled flat file, all under this run's mktemp directory; the scenario is a scratch file, so
# no tracked scenario and no skip-ok row moves. The hook must have CUT: a run in which it made no cut would exercise nothing.
: > "$tmp/why"
ok=1
cuc="$tmp/cartuser"
mkdir "$cuc" || exit 1
cucscn="$cuc/cart-user.ivss"
cu_exec="$cuc/exec.bin"
cu_grom="$cuc/grom.bin"
cu_flat="$tmp/cartuser-flat.bin"
cu_cart="$tmp/cartuser-wrapped.ivsc"
dd if=/dev/zero of="$cu_exec" bs=8192 count=1 2> /dev/null || exit 1
dd if=/dev/zero of="$cu_grom" bs=2048 count=1 2> /dev/null || exit 1
dd if=/dev/zero of="$cu_flat" bs=8192 count=1 2> /dev/null || exit 1
printf 'ivss 1\nid cart-user\nprofile intv-2609-ntsc-family-p1\ncontent exec user\ncontent grom user\ncontent cart user\nrun-to 1000 ticks\n' \
  > "$cucscn"
cu_d1=
if [ -z "$flatwrap" ]; then
  ok=0; why cart-user-flatwrap 'the sixth argument (ivs_flatwrap) is missing: the case cannot build the container it loads'
else
  cu_d1=$("$flatwrap" "$cu_flat" 4000 "$cu_cart" 2> "$tmp/cu-wrap.err") \
    || { ok=0; why cart-user-flatwrap "ivs_flatwrap failed: $(sed -n '1p' "$tmp/cu-wrap.err")"; }
  [ "${#cu_d1}" -eq 64 ] || { ok=0; why cart-user-flatwrap "the printed digest is '$cu_d1'"; }
fi
# cu_refuse <slug> <option> <path> <stderr line>: one run per option; the refusal is the exit code, the one line, no report and no file.
cu_refuse() {
  printf 'ivs-run: %s\n' "$4" > "$tmp/cu.want.err"
  run_one "$tmp" "cu-$1" "$ivs_run" --content-dir "$cuc" --exec "$cu_exec" --grom "$cu_grom" --cart "$cu_cart" \
    "$2" "$3" "$cucscn"
  [ "$rc" -eq 14 ] && [ "$hung" -eq 0 ] && [ ! -s "$tmp/runs/cu-$1.out" ] \
    || { ok=0; why cart-user-refused "$2: exit $rc, hung $hung or a report on stdout"; }
  cmp -s "$tmp/runs/cu-$1.err" "$tmp/cu.want.err" \
    || { ok=0; why cart-user-refused "$2: stderr is not the one line 'ivs-run: $4'"; }
  [ ! -e "$3" ] || { ok=0; why cart-user-refused "$2: a file was created at $3"; }
}
cu_refuse frame-ppm --frame-ppm "$tmp/cu.ppm" '--frame-ppm refused: content user'
cu_refuse video-raw --video-raw "$tmp/cu.vraw" '--video-raw refused: content user'
cu_refuse audio-raw --audio-raw "$tmp/cu.raw" '--audio-raw refused: content user'
cu_refuse psg-log-out --psg-log-out "$tmp/cu.log" '--psg-log-out refused: content user'
cu_refuse audio-wav --audio-wav "$tmp/cu.wav" '--audio-wav refused: content user'
cu_refuse update-golden --update-golden "$tmp/cu.digests" 'golden refused: content user'
# The three audio options at once: the first of the fixed order (--audio-raw, --psg-log-out, --audio-wav) is the one named, and none of
# the three files exists afterwards.
run_one "$tmp" cu-all "$ivs_run" --content-dir "$cuc" --exec "$cu_exec" --grom "$cu_grom" --cart "$cu_cart" \
  --audio-wav "$tmp/cu-all.wav" --psg-log-out "$tmp/cu-all.log" --audio-raw "$tmp/cu-all.raw" "$cucscn"
[ "$rc" -eq 14 ] && [ "$hung" -eq 0 ] && [ ! -s "$tmp/runs/cu-all.out" ] \
  || { ok=0; why cart-user-refused "all three audio options: exit $rc, hung $hung or a report on stdout"; }
printf 'ivs-run: --audio-raw refused: content user\n' > "$tmp/cu.want.err"
cmp -s "$tmp/runs/cu-all.err" "$tmp/cu.want.err" \
  || { ok=0; why cart-user-refused 'all three audio options: stderr is not the one --audio-raw line'; }
for f in wav log raw; do
  [ ! -e "$tmp/cu-all.$f" ] || { ok=0; why cart-user-refused "all three audio options: a file was created ($tmp/cu-all.$f)"; }
done
# The snapshot rule (docs/owner-images.md): the save/restore hook fills a caller's buffer and chooses no file, so it is allowed for user
# content - the absence of a seventh refusal. The tick value is derived from the scenario's own `run-to` (half of it, so a 1000-tick run
# is cut once) rather than guessed, and a run that made no cut is a finding, not a pass.
if [ -z "$cu_d1" ]; then
  ok=0; why cart-user-snapshot 'no container was built: the snapshot limb cannot load a cartridge'
else
  run_one "$tmp" cu-plain "$ivs_run" --content-dir "$cuc" --exec "$cu_exec" --grom "$cu_grom" --cart "$cu_cart" "$cucscn"
  judge cu-plain 0 pass - "" || ok=0
  cu_to=$(sed -n 's/^run-to \([0-9][0-9]*\) ticks$/\1/p' "$cucscn")
  case $cu_to in '' | *[!0-9]*) ok=0; why cart-user-snapshot "the scenario's run-to is '$cu_to'"; cu_to=1000 ;; esac
  cu_rt=$((cu_to / 2))
  ls -A "$cuc" > "$tmp/cu.files.before" || exit 1
  run_one "$tmp" cu-rt "$ivs_run" --content-dir "$cuc" --exec "$cu_exec" --grom "$cu_grom" --cart "$cu_cart" \
    --roundtrip-at-ticks "$cu_rt" "$cucscn"
  judge cu-rt 0 pass - "" || ok=0
  same_body cu-rt "$tmp/runs/cu-plain.out" || ok=0
  cu_hc=$(rkey "$tmp/runs/cu-rt.out" host.calls)
  cu_pc=$(rkey "$tmp/runs/cu-plain.out" host.calls)
  case $cu_hc$cu_pc in
    '' | *[!0-9]*) ok=0; why cart-user-snapshot "host.calls is not a number (hook '$cu_hc', plain '$cu_pc')" ;;
    *) [ $((cu_hc - cu_pc)) -ge 2 ] \
         || { ok=0; why cart-user-snapshot "host.calls $cu_hc against the plain run's $cu_pc: the hook made no cut, so the snapshot rule was not exercised"; } ;;
  esac
  ls -A "$cuc" > "$tmp/cu.files.after" || exit 1
  cmp -s "$tmp/cu.files.before" "$tmp/cu.files.after" \
    || { ok=0; why cart-user-snapshot "the hook run changed the scratch tree: $(tr '\n' ' ' < "$tmp/cu.files.after")"; }
fi
result runner-process.cart-user.refuse-derived

# The wrap tool's container round-trips through the runner as `content cart user` (plan 14-03, SCN-03, criterion 3, D-47): the report's
# slot.cart.sha256 equals the digest the tool printed on stdout, slot.cart.length_bytes is the container's own size on disk, and the
# equality is proved live rather than vacuous - flipping ONE byte of the tool's input changes both the printed digest and the report's
# value, and the second half fails if the two digests are equal. The input byte is the first word's low half (00 -> 01, still at most
# $03FF, so the tool's word-range rule stays satisfied). The container, the flat file and both reports live under this run's mktemp
# directory; nothing is written into the tree. The `exec:` label stays `-`, so no line reads as a real-EXEC result.
: > "$tmp/why"
ok=1
if [ -z "$flatwrap" ]; then
  ok=0; why flatwrap-roundtrip 'the sixth argument (ivs_flatwrap) is missing: the round trip cannot be run'
else
  rt_flat="$tmp/rt-flat.bin"
  rt_a="$tmp/rt-a.ivsc"
  rt_b="$tmp/rt-b.ivsc"
  dd if=/dev/zero of="$rt_flat" bs=8192 count=1 2> /dev/null || exit 1
  rt_d1=$("$flatwrap" "$rt_flat" 4000 "$rt_a" 2> "$tmp/rt-wrap.err") \
    || { ok=0; why flatwrap-roundtrip "the first wrap failed: $(sed -n '1p' "$tmp/rt-wrap.err")"; }
  rt_size=$(wc -c < "$rt_a" 2> /dev/null | tr -d ' ')
  run_one "$tmp" rt-a "$ivs_run" --content-dir "$cuc" --exec "$cu_exec" --grom "$cu_grom" --cart "$rt_a" "$cucscn"
  judge rt-a 0 pass - "" || ok=0
  [ "$(op_value rt-a slot.cart.loaded)" = 1 ] \
    || { ok=0; why flatwrap-roundtrip "slot.cart.loaded '$(op_value rt-a slot.cart.loaded)'"; }
  [ "$(op_value rt-a slot.cart.status)" = ok ] \
    || { ok=0; why flatwrap-roundtrip "slot.cart.status '$(op_value rt-a slot.cart.status)'"; }
  [ -n "$rt_size" ] && [ "$(op_value rt-a slot.cart.length_bytes)" = "$rt_size" ] \
    || { ok=0; why flatwrap-roundtrip "slot.cart.length_bytes '$(op_value rt-a slot.cart.length_bytes)', the container is '$rt_size' bytes"; }
  [ -n "$rt_d1" ] && [ "$(op_value rt-a slot.cart.sha256)" = "$rt_d1" ] \
    || { ok=0; why flatwrap-roundtrip "slot.cart.sha256 '$(op_value rt-a slot.cart.sha256)', the tool printed '$rt_d1'"; }
  grep -q -x -F 'exec: -' "$tmp/runs/rt-a.out" || { ok=0; why flatwrap-roundtrip 'the report does not carry the `exec: -` label'; }
  # One byte of the tool's input moves: a judge that accepted the same digest twice would pass the equality above, so it is made to fail here.
  printf '\001' | dd of="$rt_flat" bs=1 seek=1 conv=notrunc 2> /dev/null || exit 1
  rt_d2=$("$flatwrap" "$rt_flat" 4000 "$rt_b" 2> "$tmp/rt-wrap2.err") \
    || { ok=0; why flatwrap-roundtrip "the second wrap failed: $(sed -n '1p' "$tmp/rt-wrap2.err")"; }
  if [ -z "$rt_d1" ] || [ -z "$rt_d2" ] || [ "$rt_d2" = "$rt_d1" ]; then
    ok=0; why flatwrap-roundtrip "the flipped input byte left the printed digest at '$rt_d1' (second '$rt_d2'): the equality is vacuous"
  fi
  run_one "$tmp" rt-b "$ivs_run" --content-dir "$cuc" --exec "$cu_exec" --grom "$cu_grom" --cart "$rt_b" "$cucscn"
  judge rt-b 0 pass - "" || ok=0
  [ -n "$rt_d2" ] && [ "$(op_value rt-b slot.cart.sha256)" = "$rt_d2" ] \
    || { ok=0; why flatwrap-roundtrip "the second run's slot.cart.sha256 '$(op_value rt-b slot.cart.sha256)', the tool printed '$rt_d2'"; }
fi
result runner-process.judge.flatwrap-roundtrip

# The wrap tool's container digested by the shim AND by the runner (plan 17-02, EV-08, criterion 3, W9). The join:
# both processes read the same container the wrap tool wrote, so the harness's `digest.images` for it must equal the
# runner's `digest.images` for it, and the runner's `slot.cart.sha256` must equal the digest the tool printed. The
# IMAGES domain serialises slot.cart.sha256 (core/src/machine/machine_digest.c), which is why no slot accessor is
# added: D-52 forbids one, and no new golden is needed for the join.
#
# Five limbs, each its own finding, and no digest value is typed: every limb is a relation between values observed
# in this case, so a changed container format is not a test edit.
#   (1) the harness accepted container A and its digest.images is compared at a positive frame count;
#   (2) the runner accepted container A, its digest.images equals the harness's, and its slot.cart.sha256 equals
#       the digest the tool printed;
#   (3) after ONE flipped input byte, both hold again for container B;
#   (4) both values MOVE between A and B (join-immobile), so an equality that ignored its input cannot pass;
#   (5) the committed stand-in built below is REFUSED by the same shim_join (join-images).
#
# The two content directories hold a copy of boot-stub-2.exec and the tool's container named lamp-tone.ivsc, because
# the harness reads those two literal names (tests/host/ivs_host_harness.c) and changing them would need new host
# surface. The scenario, the flat input, both content directories and every report live under this run's mktemp
# directory: nothing is written into the work tree, and the second wrap writes a NEW path because
# flatwrap-output-exists refuses an existing one. The `exec:` label names the project boot stub and says in words
# that it is not an EXEC, so no line can read as a real-EXEC result.
#
# One id, not two: plan 15's precedent for the same kind of join minted two extra committed ids (host-vacuous,
# host-digest-diff), and this phase's locked decision is one id at 0.10 cpu-s. The fifth limb reaches the same
# property - a stand-in that ignores its input, committed in the same commit, refused by the same check - at zero
# extra ids and zero extra process launches, and it is re-demonstrated on every gate run rather than once.
: > "$tmp/why"
ok=1
# shim_join <harness-out> <runner-out> <tool-digest> <token>: appends findings to the current why target, each
# prefixed by <token>, and returns non-zero when it appended one. The prefix is `join`, so the three findings are
# join-frames, join-images and join-cart-digest.
shim_join() {
  _sj_h=$1; _sj_r=$2; _sj_d=$3; _sj_t=$4
  _sj_rc=0
  _sj_f=$(sed -n 's/^frames //p' "$_sj_h" | head -n 1)
  case $_sj_f in
    '' | *[!0-9]*) why "$_sj_t-frames" "the harness reported no frame count ('$_sj_f'), so the drive cannot be judged"; _sj_rc=1 ;;
    *) [ "$_sj_f" -gt 0 ] || { why "$_sj_t-frames" "frames $_sj_f"; _sj_rc=1; } ;;
  esac
  _sj_hi=$(sed -n 's/^digest[.]images //p' "$_sj_h" | head -n 1)
  _sj_ri=$(sed -n 's/^digest[.]images //p' "$_sj_r" | head -n 1)
  if [ -z "$_sj_hi" ] || [ -z "$_sj_ri" ] || [ "$_sj_hi" != "$_sj_ri" ]; then
    why "$_sj_t-images" "the harness digested '${_sj_hi:-missing}' and the runner '${_sj_ri:-missing}': the two processes did not digest the same container"
    _sj_rc=1
  fi
  _sj_cs=$(sed -n 's/^slot[.]cart[.]sha256 //p' "$_sj_r" | head -n 1)
  if [ -z "$_sj_d" ] || [ "$_sj_cs" != "$_sj_d" ]; then
    why "$_sj_t-cart-digest" "slot.cart.sha256 '${_sj_cs:-missing}', the tool printed '${_sj_d:-missing}'"
    _sj_rc=1
  fi
  return "$_sj_rc"
}
if [ -z "$flatwrap" ]; then
  ok=0; why flatwrap-shim 'the sixth argument (ivs_flatwrap) is missing: the join cannot be run'
elif [ -z "$shim" ]; then
  ok=0; why flatwrap-shim 'the seventh argument (libivs_retro) is missing: the join cannot be run'
elif [ -z "$harness" ]; then
  ok=0; why flatwrap-shim 'the eighth argument (ivs_host_harness) is missing: the join cannot be run'
else
  fwshim="$tmp/fwshim"
  fwa="$fwshim/a"; fwb="$fwshim/b"; fwf="$fwshim/fakes"
  mkdir -p "$fwa" "$fwb" "$fwf" || exit 1
  fwflat="$tmp/fwshim-flat.bin"
  dd if=/dev/zero of="$fwflat" bs=8192 count=1 2> /dev/null || exit 1
  cp "$images/boot-stub-2.exec" "$fwa/boot-stub-2.exec" || exit 1
  cp "$images/boot-stub-2.exec" "$fwb/boot-stub-2.exec" || exit 1
  fwscn="$fwshim/flatwrap-shim.ivss"
  printf 'ivss 1\nid flatwrap-shim\nprofile intv-2609-ntsc-family-p1\ncontent exec boot-stub-2\ncontent grom zero-grom\ncontent cart lamp-tone.ivsc\nrun-to 1000 ticks\n' \
    > "$fwscn" || exit 1
  # (1) container A: the tool wraps 8192 zero bytes at base 4000, and the shim digests the result.
  fwd1=$("$flatwrap" "$fwflat" 4000 "$fwa/lamp-tone.ivsc" 2> "$tmp/fwshim-wrap1.err") \
    || { ok=0; why flatwrap-shim "the first wrap failed: $(sed -n '1p' "$tmp/fwshim-wrap1.err")"; }
  run_one "$tmp" fwshim-a-harness "$harness" "$shim" "$fwa"
  if [ "$hung" -eq 1 ] || [ "$rc" -ne 0 ]; then
    ok=0; why flatwrap-shim "the harness on container A exited $rc (hung $hung): $(sed -n 's/^fail //p' "$tmp/runs/fwshim-a-harness.out" | head -n 1)"
  fi
  # (2) the runner on the same container, judged as an ordinary pass run.
  run_one "$tmp" fwshim-a-run "$ivs_run" --content-dir "$fwa" "$fwscn"
  judge fwshim-a-run 0 pass - "" || ok=0
  if [ "$ok" -eq 1 ]; then
    shim_join "$tmp/runs/fwshim-a-harness.out" "$tmp/runs/fwshim-a-run.out" "$fwd1" join || ok=0
    grep -q -F -e 'exec: project boot stub contract 2, not an EXEC sha256=' "$tmp/runs/fwshim-a-run.out" \
      || { ok=0; why flatwrap-shim 'the report does not carry the boot-stub label on its exec: line, so a line could read as a real-EXEC result'; }
  fi
  # (3) ONE flipped input byte at offset 1 (the first word's high half, still at most 03FF so the tool's word-range
  # rule stays satisfied), wrapped into a NEW path and run the same way.
  printf '\001' | dd of="$fwflat" bs=1 seek=1 conv=notrunc 2> /dev/null || exit 1
  fwd2=$("$flatwrap" "$fwflat" 4000 "$fwb/lamp-tone.ivsc" 2> "$tmp/fwshim-wrap2.err") \
    || { ok=0; why flatwrap-shim "the second wrap failed: $(sed -n '1p' "$tmp/fwshim-wrap2.err")"; }
  run_one "$tmp" fwshim-b-harness "$harness" "$shim" "$fwb"
  if [ "$hung" -eq 1 ] || [ "$rc" -ne 0 ]; then
    ok=0; why flatwrap-shim "the harness on container B exited $rc (hung $hung): $(sed -n 's/^fail //p' "$tmp/runs/fwshim-b-harness.out" | head -n 1)"
  fi
  run_one "$tmp" fwshim-b-run "$ivs_run" --content-dir "$fwb" "$fwscn"
  judge fwshim-b-run 0 pass - "" || ok=0
  if [ "$ok" -eq 1 ]; then
    shim_join "$tmp/runs/fwshim-b-harness.out" "$tmp/runs/fwshim-b-run.out" "$fwd2" join || ok=0
  fi
  # (4) liveness: both values move with the input byte, so the equalities above are not satisfied by a stand-in that
  # ignores what it is given.
  if [ "$ok" -eq 1 ]; then
    fwi_a=$(sed -n 's/^digest[.]images //p' "$tmp/runs/fwshim-a-harness.out" | head -n 1)
    fwi_b=$(sed -n 's/^digest[.]images //p' "$tmp/runs/fwshim-b-harness.out" | head -n 1)
    if [ -z "$fwd1" ] || [ -z "$fwd2" ] || [ "$fwd1" = "$fwd2" ] \
        || [ -z "$fwi_a" ] || [ "$fwi_a" = "$fwi_b" ]; then
      ok=0; why join-immobile "the flipped input byte left the tool digest at '${fwd1:-missing}'/'${fwd2:-missing}' and digest.images at '${fwi_a:-missing}'/'${fwi_b:-missing}'"
    fi
  fi
  # (5) the committed stand-in: a harness report built here from the FIRST container's lines, judged against the
  # SECOND container's real runner report and the second container's tool digest. It must be refused with
  # join-images; an acceptance records judge-vacuous, because then the comparison proves nothing. The judgement runs
  # in a subshell with its own why target, so the stand-in's expected finding cannot set this case's verdict.
  if [ "$ok" -eq 1 ]; then
    # The guard must be able to fire (lesson 20): `grep ... || exit 1` would abort the whole suite on a report
    # with no matching line, so the judge-vacuous branch below could never run and the case would truncate the
    # TAP stream instead of naming the reason. A `!` test over the same grep makes the branch the only handler.
    if ! grep -E '^(frames|digest[.])' "$tmp/runs/fwshim-a-harness.out" > "$fwf/stand-in.out"; then
      ok=0; why judge-vacuous 'the stand-in was built from no lines of the first harness report'
    fi
  fi
  if [ "$ok" -eq 1 ]; then
    _fw_sj=0
    ( whytarget="$fwf/stand-in.why"; : > "$whytarget"; \
      shim_join "$fwf/stand-in.out" "$tmp/runs/fwshim-b-run.out" "$fwd2" join ) || _fw_sj=$?
    if [ "$_fw_sj" -eq 0 ]; then
      ok=0; why judge-vacuous 'shim_join accepted the stand-in that replays the first container against the second'
    elif ! grep -q -e '^join-images: ' "$fwf/stand-in.why"; then
      ok=0; why judge-wrong-reason "the stand-in was refused, but not with join-images: $(sed -n '1p' "$fwf/stand-in.why")"
    fi
  fi
fi
result runner-process.judge.flatwrap-shim

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
# The usage line carries the cartridge option since plan 14-03 (SCN-03): `--cart <cart-image>` is an option, never a third positional.
or_usage_msg='owner-run: usage: owner-run.sh [--preset dev|release] [--record] [--cart <cart-image>] <exec-image> <grom-image>'
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
# --cart names an argument: the option without a value is the usage refusal, not a positional that shifts the two images.
or_run usage '' '' --cart
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

# The cartridge pass path of the owner-local route (plan 14-03, SCN-03, criterion 4, D-47): `--cart <file>` is an OPTION of the helper, and
# with it the helper inserts one `content cart user` line into its temporary copy, forwards `--cart` to the runner, and the run loads an
# EXEC image and the container. Project-made files only: zero-filled refs/e.bin (8192) and refs/g.bin (2048), and a container the
# project's own wrap tool wrote into refs/ from a zero-filled flat file. The report's slot.cart.sha256 must be the digest the tool printed,
# the `exec:` label stays `-` (nothing reads as a real-EXEC result), and the outcome, exit code and outcome.detail are pinned to what this
# run MEASURED (recorded in the plan's SUMMARY), not to the research's prediction: the inserted `content cart user` line moves the halted
# step's scenario line from 23 to 24. The pass direction is then proved able to fail: after --record, one byte of refs/e.bin is flipped and
# the same run is refused at load (IVS_E_DIGEST_MISMATCH, exit 14), so neither half can pass alone. A container inside the work tree outside
# refs/ is refused exactly as the other two images are, which is why the cartridge joins the helper's location loop.
: > "$tmp/why"
ok=1
or_scratch
or_img "$sr/refs/e.bin" 8192
or_img "$sr/refs/g.bin" 2048
or_img "$sr/refs/flat.bin" 8192
mkdir "$sr/data/own" || exit 1
oc_d1=
if [ -z "$flatwrap" ]; then
  ok=0; why owner-run-cart 'the sixth argument (ivs_flatwrap) is missing: the case cannot build the container it loads'
else
  oc_d1=$("$flatwrap" "$sr/refs/flat.bin" 4000 "$sr/refs/c.ivsc" 2> "$tmp/oc-wrap.err") \
    || { ok=0; why owner-run-cart "ivs_flatwrap failed: $(sed -n '1p' "$tmp/oc-wrap.err")"; }
  [ "${#oc_d1}" -eq 64 ] || { ok=0; why owner-run-cart "the printed digest is '$oc_d1'"; }
  cp "$sr/refs/c.ivsc" "$sr/data/own/c.ivsc" || exit 1
  # A cartridge inside the work tree outside refs/ is refused by the same location rule as the two images.
  or_run carttree '' '' --cart "$sr/data/own/c.ivsc" "$sr/refs/e.bin" "$sr/refs/g.bin"
  or_judge carttree 2 '' "owner-run: an image is inside the work tree outside refs/: $sr/data/own/c.ivsc" || ok=0
  # --record writes the two-role lock from the two images and goes on; with no --cart this run is also the no-cartridge path.
  or_run record '' '' --record "$sr/refs/e.bin" "$sr/refs/g.bin"
  [ "$hung" -eq 0 ] || { ok=0; why owner-run-cart 'the --record run was stopped by the watchdog'; }
  [ ! -s "$tmp/runs/or-record.err" ] || { ok=0; why owner-run-cart "the --record run wrote on stderr: $(sed -n '1p' "$tmp/runs/or-record.err")"; }
  cp "$tmp/runs/or-record.out" "$tmp/oc-nocart.report" || exit 1
  # The lock keeps exactly two roles: no cartridge line was recorded, and the run without --cart names no cartridge role.
  [ "$(grep -c . "$sr/refs/owner-images.sha256")" -eq 2 ] \
    || { ok=0; why owner-run-cart "the lock has $(grep -c . "$sr/refs/owner-images.sha256") lines, expected 2"; }
  or_run cartpass '' '' --cart "$sr/refs/c.ivsc" "$sr/refs/e.bin" "$sr/refs/g.bin"
  [ "$hung" -eq 0 ] || { ok=0; why owner-run-cart 'the cartridge run was stopped by the watchdog'; }
  [ "$rc" -eq 10 ] || { ok=0; why owner-run-cart "exit $rc, expected 10"; }
  [ ! -s "$tmp/runs/or-cartpass.err" ] || { ok=0; why owner-run-cart "stderr is not empty: $(sed -n '1p' "$tmp/runs/or-cartpass.err")"; }
  [ "$(sed -n '1p' "$tmp/runs/or-cartpass.out")" = "OWNER-LOCAL RUN (D11): inputs are the owner's, not redistributable, not run in CI, not evidence, no claim" ] \
    || { ok=0; why owner-run-cart 'the first stdout line is not the banner'; }
  grep -q -x -F 'slot.exec.loaded 1' "$tmp/runs/or-cartpass.out" || { ok=0; why owner-run-cart 'slot.exec.loaded is not 1'; }
  grep -q -x -F 'slot.cart.loaded 1' "$tmp/runs/or-cartpass.out" || { ok=0; why owner-run-cart 'slot.cart.loaded is not 1'; }
  grep -q -x -F "slot.cart.sha256 $oc_d1" "$tmp/runs/or-cartpass.out" \
    || { ok=0; why owner-run-cart "slot.cart.sha256 is not the digest the tool printed ('$oc_d1')"; }
  grep -q -x -F 'exec: -' "$tmp/runs/or-cartpass.out" || { ok=0; why owner-run-cart 'the report does not carry the `exec: -` label'; }
  grep -q -x -F 'outcome guest-fail' "$tmp/runs/or-cartpass.out" || { ok=0; why owner-run-cart 'outcome is not guest-fail'; }
  grep -q -x -F 'exit.code 10' "$tmp/runs/or-cartpass.out" || { ok=0; why owner-run-cart 'exit.code is not 10'; }
  grep -q -x -F 'outcome.detail stop step=1 line=24 expected=target got=halted' "$tmp/runs/or-cartpass.out" \
    || { ok=0; why owner-run-cart "outcome.detail is '$(sed -n 's/^outcome\.detail //p' "$tmp/runs/or-cartpass.out")'"; }
  # The one-byte EXEC mutation, after --record: the core refuses at load, so the pinned pass report is not met.
  printf '\001' | dd of="$sr/refs/e.bin" bs=1 seek=0 conv=notrunc 2> /dev/null || exit 1
  or_run mutate '' '' --cart "$sr/refs/c.ivsc" "$sr/refs/e.bin" "$sr/refs/g.bin"
  [ "$hung" -eq 0 ] || { ok=0; why owner-run-cart 'the mutated EXEC run was stopped by the watchdog'; }
  [ "$rc" -eq 14 ] || { ok=0; why owner-run-cart "the mutated EXEC run: exit $rc, expected 14"; }
  grep -q '^outcome\.detail load-refused role=exec status=IVS_E_DIGEST_MISMATCH' "$tmp/runs/or-mutate.out" \
    || { ok=0; why owner-run-cart 'the mutated EXEC run does not name the digest mismatch at load'; }
  grep -q -x 'cpu.run_state unreset' "$tmp/runs/or-mutate.out" || { ok=0; why owner-run-cart 'the mutated EXEC run executed the machine'; }
  # The scratch root holds no file but the ones this case wrote.
  find "$sr" ! -type d | sed "s|^$sr/||" | sort > "$tmp/oc.files"
  printf '%s\n' build/release/runner/ivs-run data/own/c.ivsc data/scenarios/owner-exec-title.ivss refs/c.ivsc refs/e.bin refs/flat.bin \
    refs/g.bin refs/owner-images.sha256 tools/owner-run.sh | sort > "$tmp/oc.want"
  cmp -s "$tmp/oc.files" "$tmp/oc.want" \
    || { ok=0; why owner-run-cart "the scratch root holds other files: $(tr '\n' ' ' < "$tmp/oc.files")"; }
fi
or_end
result runner-process.judge.owner-run-cart-pass

# ---- the host shim (plan 15-01, TOOL-08, D-48, D-35) ----
# check_digest_lines <golden> <output>: exactly the two comparisons check_one_golden makes, applied to the fake
# frontend's stdout - the sorted `digest.` line set is byte-equal (both non-empty), and the observed domain set the
# golden's `# domains:` line declares equals the non-composite domains the output printed. 0 only when both hold.
check_digest_lines() {
  _cdg=$1; _cdo=$2
  _cdrc=0
  grep '^digest\.' "$_cdg" 2> /dev/null | sort > "$tmp/hd.want"
  grep '^digest\.' "$_cdo" 2> /dev/null | sort > "$tmp/hd.have"
  if [ ! -s "$tmp/hd.want" ] || ! cmp -s "$tmp/hd.want" "$tmp/hd.have"; then
    why digest-diff "golden only: $(comm -23 "$tmp/hd.want" "$tmp/hd.have" | awk '{ print $1 }' | tr '\n' ' ')output only: $(comm -13 "$tmp/hd.want" "$tmp/hd.have" | awk '{ print $1 }' | tr '\n' ' ')"
    _cdrc=1
  fi
  sed -n 's/^# domains: //p' "$_cdg" | tr ' ' '\n' | sed -n 's/=observed$//p' | sort > "$tmp/hd.dom.want"
  sed -n 's/^digest\.\([a-z]*\) .*/\1/p' "$_cdo" | grep -v -x -e composite | sort > "$tmp/hd.dom.have"
  if [ ! -s "$tmp/hd.dom.want" ] || ! cmp -s "$tmp/hd.dom.want" "$tmp/hd.dom.have"; then
    why digest-diff "observed domains: golden $(tr '\n' ' ' < "$tmp/hd.dom.want")output $(tr '\n' ' ' < "$tmp/hd.dom.have")"
    _cdrc=1
  fi
  return "$_cdrc"
}

# check_host_empty <output>: the empty-poll run judged in three limbs, each able to fail on its own (research Open
# Question 5, measured; criterion 3.2). 0 only when all three hold; findings in $tmp/why.
#   (1) at least one `digest.` line DIFFERS from the pass golden's same-named line - the "different set" the
#       criterion names, and the limb a constant judge (one that compared an output with itself) fails;
#   (2) `digest.images` is EQUAL to the pass golden's - the frames are the same, so a shim that stopped rendering
#       fails here while a judge that compared nothing would pass on limb (1) alone;
#   (3) every non-input `digest.` line the IDLE golden carries equals that golden's same-named line, and
#       `digest.input` differs from the pass golden's. The idle golden is read for the set it carries - eight
#       domains today (cpu mem images instance composite video audio psglog); its `# domains:` line reads
#       input=none, so it has no digest.input line - never from a list typed here.
check_host_empty() {
  _eh=$1; _rc=0
  _ehf=$(sed -n 's/^frames //p' "$_eh" | head -n 1)
  case $_ehf in
    '' | *[!0-9]*) why empty-poll-frames "frames '$_ehf' is not a positive integer"; return 1 ;;
  esac
  [ "$_ehf" -gt 0 ] || { why empty-poll-frames "frames $_ehf"; return 1; }
  # (1) at least one line differs from the pass golden's same-named line.
  _ehd=$(awk -v pass="$root/data/golden/demo-lamp-tone-pass.digests" '
    BEGIN { while ((getline l < pass) > 0) { split(l, kv, " "); if (kv[1] ~ /^digest[.]/) g[kv[1]] = kv[2] } close(pass) }
    /^digest[.]/ { if (!($1 in g) || g[$1] != $2) n++ }
    END { print n + 0 }' "$_eh")
  if [ "${_ehd:-0}" -lt 1 ]; then
    why empty-poll-identical 'no digest line differs from the pass golden, so the poll was not dropped'
    _rc=1
  fi
  # (2) digest.images equal to the pass golden's.
  _ehi=$(sed -n 's/^digest[.]images //p' "$_eh" | head -n 1)
  _ehig=$(sed -n 's/^digest[.]images //p' "$root/data/golden/demo-lamp-tone-pass.digests" | head -n 1)
  if [ -z "$_ehi" ] || [ "$_ehi" != "$_ehig" ]; then
    why empty-poll-images "digest.images '${_ehi:-missing}' is not the pass golden's '${_ehig:-missing}': the run stopped rendering, so a difference is not attributable to the input alone"
    _rc=1
  fi
  # (3) the idle golden's non-input lines, and digest.input differing from the pass golden's.
  awk '/^digest[.]/ && $1 != "digest.input" { print $1, $2 }' "$root/data/golden/demo-lamp-tone-idle.digests" \
    > "$tmp/eh.idle.want"
  if [ ! -s "$tmp/eh.idle.want" ]; then
    why empty-poll-idle 'the idle golden carries no non-input digest line, so the limb would compare nothing'
    _rc=1
  else
    while read -r _ehk _ehv; do
      _ehgot=$(sed -n "s/^${_ehk} //p" "$_eh" | head -n 1)
      if [ "$_ehgot" != "$_ehv" ]; then
        why empty-poll-idle "$_ehk '${_ehgot:-missing}' is not the idle golden's '$_ehv'"
        _rc=1
      fi
    done < "$tmp/eh.idle.want"
  fi
  _ehin=$(sed -n 's/^digest[.]input //p' "$_eh" | head -n 1)
  _ehing=$(sed -n 's/^digest[.]input //p' "$root/data/golden/demo-lamp-tone-pass.digests" | head -n 1)
  if [ -z "$_ehin" ] || [ -z "$_ehing" ] || [ "$_ehin" = "$_ehing" ]; then
    why empty-poll-idle "digest.input '${_ehin:-missing}' does not differ from the pass golden's '${_ehing:-missing}': the poll was not dropped"
    _rc=1
  fi
  return "$_rc"
}

# check_host <output>: the drive is not vacuous (a positive frame count is required before anything is compared) and
# the nine digest lines equal the pass golden's, byte for byte.
check_host() {
  _chf=$(sed -n 's/^frames //p' "$1" | head -n 1)
  case $_chf in
    '' | *[!0-9]*) why frames-zero "frames '$_chf' is not a positive integer"; return 1 ;;
  esac
  [ "$_chf" -gt 0 ] || { why frames-zero "frames $_chf"; return 1; }
  check_digest_lines "$root/data/golden/demo-lamp-tone-pass.digests" "$1"
}

: > "$tmp/why"
ok=1
if [ -z "$shim" ]; then
  ok=0; why host-shim 'the seventh argument (libivs_retro) is missing: the host case cannot be run'
elif [ -z "$harness" ]; then
  ok=0; why host-harness 'the eighth argument (ivs_host_harness) is missing: the host case cannot be run'
else
  run_one "$tmp" host-pass "$harness" "$shim" "$content"
  if [ "$hung" -eq 1 ] || [ "$rc" -ne 0 ]; then
    ok=0
    why host-run "exit $rc, hung $hung: $(sed -n 's/^fail //p' "$tmp/runs/host-pass.out" | head -n 1)"
  elif ! check_host "$tmp/runs/host-pass.out"; then
    ok=0
  fi
  # The evidence is inspectable, and deliberately NOT under IVS_TEST_OUT_DIR/runner/: the golden step compares
  # everything in that directory and a frontend's stdout is not a scenario report.
  if [ -n "${IVS_TEST_OUT_DIR:-}" ]; then
    cp "$tmp/runs/host-pass.out" "$IVS_TEST_OUT_DIR/host-pass.out" || exit 1
  fi
fi
result runner-process.host.digests

# ---- the shaped-entry contract (plan 15-02, TOOL-08, D-48): three cases over the SAME pass-mode run ----
# They judge lines of the run host.digests already made and start no second harness process: the cost unit is the
# id, but the wall time is the process, and a second run would double the suite's new time for no new evidence.
pass_out="$tmp/runs/host-pass.out"

# host_field <output> <expected line> <token>: the output carries exactly <expected line>. A missing or different
# line records a finding naming the key and the value the object returned, never a bare `missing`.
host_field() {
  grep -q -x -F -e "$2" "$1" && return 0
  _hf=${2%% *}
  _hv=$(sed -n "s/^${_hf} //p" "$1" | head -n 1)
  why "$3" "$_hf '${_hv:-missing}', expected '$2'"
  return 1
}

# host_scenario_words <scenario>: the six `<at_tick> <c0> <c1>` triples of the scenario's own `input` lines, each
# symbolic name resolved to its bit index through data/expected/input-bits.ids - the pinned table the core's
# IVS_CTRL_* defines are checked against - so the expectation comes from the two committed oracles and is not typed
# here. A name the table does not carry, a word form this cannot read, or a line without at_tick fails the
# derivation (exit 1) instead of printing a guess.
host_scenario_words() {
  awk -v bits="$root/data/expected/input-bits.ids" '
    function word(s,   k, parts, i, v, name) {
      if (s == "0x0") return "00000000"
      if (s ~ /^0[xX]/) { bad = 1; return "?" }
      v = 0
      k = split(s, parts, "+")
      for (i = 1; i <= k; i++) {
        name = "IVS_CTRL_" parts[i]
        if (!(name in bit)) { bad = 1; return "?" }
        v += 2 ^ bit[name]
      }
      return sprintf("%08x", v)
    }
    BEGIN {
      while ((getline l < bits) > 0) {
        if (l ~ /^#/ || l !~ /[^ \t]/) continue
        split(l, b, " ")
        bit[b[1]] = b[2]
      }
      close(bits)
    }
    /^input / {
      tick = ""; c0 = "0x0"; c1 = "0x0"
      for (i = 2; i <= NF; i++) {
        split($i, kv, "=")
        if (kv[1] == "at_tick") tick = kv[2]
        else if (kv[1] == "c0") c0 = kv[2]
        else if (kv[1] == "c1") c1 = kv[2]
      }
      if (tick == "") { bad = 1; next }
      print tick, word(c0), word(c1)
    }
    END { if (bad) exit 1 }
  ' "$1"
}

# runner-process.host.api (plan 15-02, TOOL-08): the shape's version, its refusal of an incomplete host (refused,
# not ignored) and the av_info metadata a frontend places a frame with. Every value is pinned in the harness from
# the profile's own fields - data/profiles/intv-2609-ntsc-family-p1.json: video.card_columns 20,
# video.card_size_px.w 8, video.last_card_column_px 7 (= 159), video.card_rows 12, video.card_size_px.h 8 (= 96),
# scanlines_per_frame 262 x cpu_cycles_per_scanline 57 x ticks_per_cpu_cycle 4 (= 59736), stic.frame_phase_ticks 0
# and crystal_hz.num 3579545 - and sample_rate 48000 is the rate the shim's own converter delivers
# (tools/pcm/ivs_refpcm.c), not the core's native level rate. The harness is not linked against the core, so the
# profile is the shared source both sides read rather than ivs_machine_info.
: > "$tmp/why"
ok=1
if [ ! -s "$pass_out" ]; then
  ok=0; why host-api 'the pass-mode run produced no output'
else
  for _l in 'api.version 1' 'env.denied 1' 'env.accepted 1' \
    'av_info.base_width 159' 'av_info.base_height 96' 'av_info.max_width 159' 'av_info.max_height 96' \
    'av_info.frame_phase_ticks 0' 'av_info.nominal_frame_ticks 59736' 'av_info.fps_num 3579545' \
    'av_info.fps_den 59736' 'av_info.sample_rate 48000'; do
    host_field "$pass_out" "$_l" host-api || ok=0
  done
fi
result runner-process.host.api

# runner-process.host.frames (plan 15-02, TOOL-08): the delivered streams. The frame count is required FIRST, so a
# run that delivered nothing cannot pass on a comparison of nothing; then the index plane (198432 = 13 x 159 x 96
# bytes, the plane the callback receives - deliberately NOT the golden's output.video.bytes 198848, which counts 13
# x 15296 slot bytes with the core's 32-byte header included: comparing the two is the trap, and the 416-byte
# difference is exactly 13 headers), the per-frame geometry and pitch count, the pixel probes, the mono
# signed-16-bit sample width (audio.bytes == 2 x audio.frames) and the poll's six words.
: > "$tmp/why"
ok=1
if [ ! -s "$pass_out" ]; then
  ok=0; why host-frames 'the pass-mode run produced no output'
elif host_field "$pass_out" 'frames 13' host-frames; then
  for _l in 'video.plane.bytes 198432' 'frame.geometry ok 13'; do
    host_field "$pass_out" "$_l" host-frames || ok=0
  done
  # The pixel probe counts DELIVERED PIXELS (plan 15-01), not frames: every delivered pixel must equal exactly one
  # of the 17 pinned-distinct palette triples, so its count equals the plane byte count. Requiring the two to agree
  # is the equality a frame-count form would lose.
  _pp=$(sed -n 's/^pixel[.]probes ok //p' "$pass_out" | head -n 1)
  _pb=$(sed -n 's/^video[.]plane[.]bytes //p' "$pass_out" | head -n 1)
  case ${_pp:-} in
    '' | *[!0-9]*) ok=0; why host-frames "pixel.probes '${_pp:-}' is not a count" ;;
    *) [ "$_pp" -gt 0 ] || { ok=0; why host-frames "pixel.probes $_pp"; }
       [ "$_pp" = "${_pb:-}" ] \
         || { ok=0; why host-frames "pixel.probes $_pp is not video.plane.bytes ${_pb:-}"; } ;;
  esac
  _af=$(sed -n 's/^audio.frames //p' "$pass_out" | head -n 1)
  _ab=$(sed -n 's/^audio.bytes //p' "$pass_out" | head -n 1)
  case ${_af:-} in
    '' | *[!0-9]*) ok=0; why host-frames "audio.frames '${_af:-}' is not a positive integer" ;;
    *) [ "$_af" -gt 0 ] || { ok=0; why host-frames "audio.frames $_af"; }
       [ "${_ab:-}" = "$((_af * 2))" ] \
         || { ok=0; why host-frames "audio.bytes '${_ab:-}' is not 2 x audio.frames $_af"; } ;;
  esac
  if host_scenario_words "$root/data/scenarios/demo-lamp-tone-pass.ivss" > "$tmp/host.words.want"; then
    awk '/^input[.]words / { print $2, $3, $4 }' "$pass_out" > "$tmp/host.words.have"
    cmp -s "$tmp/host.words.want" "$tmp/host.words.have" \
      || { ok=0; why host-frames "input.words differ from the scenario's six events: $(diff "$tmp/host.words.want" "$tmp/host.words.have" | tr '\n' ' ')"; }
  else
    ok=0; why host-frames 'the six scenario words could not be derived from data/scenarios/demo-lamp-tone-pass.ivss and data/expected/input-bits.ids'
  fi
else
  ok=0
fi
result runner-process.host.frames

# runner-process.host.unload (plan 15-02, TOOL-08): a real teardown, proved by the refusal it leaves behind. A single
# `unload.digest.refused 1` line could be printed by a harness that never asked, so the case also requires a digest
# line BEFORE it by line number: the digests were taken while the machine was still loaded, and only then refused.
: > "$tmp/why"
ok=1
if [ ! -s "$pass_out" ]; then
  ok=0; why host-unload 'the pass-mode run produced no output'
else
  grep -q -x -F -e 'unload.digest.refused 1' "$pass_out" \
    || { ok=0; why host-unload "no 'unload.digest.refused 1' line: the teardown answered a digest, or never unloaded"; }
  _dl=$(grep -n -m 1 -e '^digest[.]' "$pass_out" | cut -d: -f1)
  _ul=$(grep -n -m 1 -e '^unload[.]digest[.]refused ' "$pass_out" | cut -d: -f1)
  if [ -z "${_dl:-}" ] || [ -z "${_ul:-}" ]; then
    ok=0; why host-unload 'the output carries no digest line or no unload line, so their order proves nothing'
  elif [ "$_dl" -ge "$_ul" ]; then
    ok=0; why host-unload "the first digest line is line $_dl, not before the unload line $_ul"
  fi
fi
result runner-process.host.unload

# runner-process.host.empty-poll (plan 15-02, TOOL-08, criterion 3.2): the same harness, the same drive shape, no
# input. A judge that compared an output with itself would pass the digests case; this id is the limb that fails it,
# and it fails in three independent ways (check_host_empty). Its stdout is kept outside IVS_TEST_OUT_DIR/runner/ for
# the same reason host-pass.out is: the golden step compares everything in that directory and a frontend's stdout is
# not a scenario report.
: > "$tmp/why"
ok=1
if [ -z "$harness" ]; then
  ok=0; why host-empty-poll 'the eighth argument (ivs_host_harness) is missing: the empty-poll case cannot be run'
elif [ -z "$shim" ]; then
  ok=0; why host-empty-poll 'the seventh argument (libivs_retro) is missing: the empty-poll case cannot be run'
else
  run_one "$tmp" host-empty "$harness" "$shim" "$content" --empty-poll
  if [ "$hung" -eq 1 ] || [ "$rc" -ne 0 ]; then
    ok=0
    why host-empty-run "exit $rc, hung $hung: $(sed -n 's/^fail //p' "$tmp/runs/host-empty.out" | head -n 1)"
  elif ! check_host_empty "$tmp/runs/host-empty.out"; then
    ok=0
  fi
  if [ -n "${IVS_TEST_OUT_DIR:-}" ]; then
    cp "$tmp/runs/host-empty.out" "$IVS_TEST_OUT_DIR/host-empty.out" || exit 1
  fi
fi
result runner-process.host.empty-poll

# runner-process.judge.host-schedule (plan 15-04, TOOL-08, review CR-01): the input schedule is bounded and REFUSES rather
# than truncates. The pass drive's six events cannot reach the bound, so nothing else in the phase could notice a
# regression here: the review measured a probe host returning 5000 events and getting `poll_calls=4096 run=1`, 904
# events silently lost. Two limbs over the same harness, in two processes:
#   full - 5000 events pending in ONE drain. The run must be refused and deliver nothing, AND the poll must have been
#          asked at least once and fewer than 5000 times: the refusal came from the schedule's bound while the host
#          still had events left, never because the host ran out (which would print 5000 and mean nothing).
#   long - 8000 events over two calls, 4000 at a time. Both calls must succeed and all 8000 must be polled, so the
#          bound is on OUTSTANDING events: a schedule that never reclaimed its applied prefix stops polling at 4096
#          here, which is the silent loss the review measured, and one that refuses a merely long run is refused by
#          `runs.ok 2`.
# A refusal asserted only by `runs.ok 0` could pass on a harness that never called run, so the pair is required: the
# same harness, the same images, one count that fits over a lifetime and one that does not fit at once.
: > "$tmp/why"
ok=1
if [ -z "$shim" ] || [ -z "$harness" ]; then
  ok=0
  why host-schedule 'the seventh or eighth argument (libivs_retro, ivs_host_harness) is missing: the schedule case cannot be run'
else
  run_one "$tmp" host-schedule-full "$harness" "$shim" "$content" --schedule-full
  if [ "$hung" -eq 1 ] || [ "$rc" -ne 0 ]; then
    ok=0
    why host-schedule-full "exit $rc, hung $hung: $(sed -n 's/^fail //p' "$tmp/runs/host-schedule-full.out" | head -n 1)"
  else
    _f="$tmp/runs/host-schedule-full.out"
    host_field "$_f" 'schedule.mode full' host-schedule-full || ok=0
    host_field "$_f" 'schedule.runs 1' host-schedule-full || ok=0
    host_field "$_f" 'schedule.runs.ok 0' host-schedule-full || ok=0
    host_field "$_f" 'schedule.frames 0' host-schedule-full || ok=0
    _sp=$(sed -n 's/^schedule[.]polled //p' "$_f" | head -n 1)
    case ${_sp:-} in
      '' | *[!0-9]*) ok=0; why host-schedule-full "schedule.polled '${_sp:-}' is not a count" ;;
      *) [ "$_sp" -gt 0 ] || { ok=0; why host-schedule-full "schedule.polled $_sp: the host was never asked"; }
         [ "$_sp" -lt 5000 ] \
           || { ok=0; why host-schedule-full "schedule.polled $_sp: the host ran out, so the refusal is not the schedule's"; } ;;
    esac
  fi
  run_one "$tmp" host-schedule-long "$harness" "$shim" "$content" --schedule-long
  if [ "$hung" -eq 1 ] || [ "$rc" -ne 0 ]; then
    ok=0
    why host-schedule-long "exit $rc, hung $hung: $(sed -n 's/^fail //p' "$tmp/runs/host-schedule-long.out" | head -n 1)"
  else
    _f="$tmp/runs/host-schedule-long.out"
    host_field "$_f" 'schedule.mode long' host-schedule-long || ok=0
    host_field "$_f" 'schedule.runs 2' host-schedule-long || ok=0
    host_field "$_f" 'schedule.runs.ok 2' host-schedule-long || ok=0
    host_field "$_f" 'schedule.polled 8000' host-schedule-long || ok=0
    host_field "$_f" 'schedule.frames 2' host-schedule-long || ok=0
  fi
fi
result runner-process.judge.host-schedule

# runner-process.judge.host-load-refused (plan 15-04, TOOL-08, review WR-01): a refused load leaves NOTHING committed.
# The review measured the defect with a four-byte cartridge: `load_bad_cart_first=0` and then a digest answered on a
# half-loaded machine (EXEC and GROM stayed installed) and `load_good_retry=0` (the right images were then refused
# IVS_E_DUPLICATE_LOAD until retro_deinit). The harness reports the whole sequence, and the case requires all eight
# observations, so neither half can pass alone: the instance digest is UNMOVED by the refused attempt (nothing was
# committed, and a digest cannot answer a state no load produced), the run is refused (nothing to run), and the SAME
# call with the right images then succeeds and MOVES the digest - a refusal cannot pass as a machine that is simply
# dead. The `unchanged`/`moved` pair is what a bare `load.refused 1` could not establish.
: > "$tmp/why"
ok=1
if [ -z "$shim" ] || [ -z "$harness" ]; then
  ok=0
  why host-load-refused 'the seventh or eighth argument (libivs_retro, ivs_host_harness) is missing: the refused-load case cannot be run'
else
  run_one "$tmp" host-load-refused "$harness" "$shim" "$content" --refused-load
  if [ "$hung" -eq 1 ] || [ "$rc" -ne 0 ]; then
    ok=0
    why host-load-refused "exit $rc, hung $hung: $(sed -n 's/^fail //p' "$tmp/runs/host-load-refused.out" | head -n 1)"
  else
    for _l in 'load.before.digest 1' 'load.refused 1' 'load.refused.digest 1' 'load.refused.unchanged 1' \
      'load.refused.run 0' 'load.retry 1' 'load.retry.digest 1' 'load.retry.moved 1'; do
      host_field "$tmp/runs/host-load-refused.out" "$_l" host-load-refused || ok=0
    done
  fi
fi
result runner-process.judge.host-load-refused

# runner-process.host.exports (plan 15-02, TOOL-08, D-48, D-35): the export surface refused by name, in both
# directions and on both platforms. tools/check/host-symbols.sh compares the object's exported set against
# data/expected/host-symbols.ids and its undefined set against core-symbols-allow.txt plus data/expected/host-imports.ids.
# The three sanitizer prefixes are passed HERE, at the call site, exactly as tests/CMakeLists.txt passes them to
# core-symbols-tap.sh and never baked into the script: they are a property of the target that links ivs_san. The
# second direction is proved LIVE without a second build: a scratch copy of the ids file with one name appended must
# be refused, and refused by that name - on GNU's linker an export-list name nothing defines is accepted silently, so
# this limb is the only detector there. BOTH directions are proved live (review WR-04): a second scratch copy with an
# exported name DELETED must be refused as host-symbols-extra by that name, so an object that grew a symbol the pin
# does not carry is caught rather than silently accepted.
: > "$tmp/why"
ok=1
if [ -z "$shim" ]; then
  ok=0; why host-shim 'the seventh argument (libivs_retro) is missing: the export case cannot be run'
else
  check_sym="$root/tools/check/host-symbols.sh"
  if [ ! -f "$check_sym" ]; then
    ok=0; why host-exports "no $check_sym"
  else
    CHECK_DIR="$root/tools/check" ROOT="$root" sh "$check_sym" --allow-prefix __asan_ --allow-prefix __ubsan_ \
      --allow-prefix __sanitizer_ "$shim" > "$tmp/runs/host-symbols.out" 2>&1
    if ! grep -q -x -F -e '-- host-symbols pass exported=9 imports=0' "$tmp/runs/host-symbols.out"; then
      ok=0
      why host-exports "$(grep -m 1 -e '^-- FAIL ' "$tmp/runs/host-symbols.out")"
    fi
    awk '/^#/ { next } NF { print $1 }' "$root/data/expected/host-symbols.ids" > "$tmp/host-extra.ids"
    printf '%s\n' ivs_not_defined >> "$tmp/host-extra.ids"
    if CHECK_DIR="$root/tools/check" ROOT="$root" sh "$check_sym" --allow-prefix __asan_ --allow-prefix __ubsan_ \
      --allow-prefix __sanitizer_ "$shim" "$tmp/host-extra.ids" > "$tmp/runs/host-symbols-extra.out" 2>&1; then
      ok=0
      why host-exports-live 'the checker accepted an ids list carrying a name the object does not export'
    elif ! grep -q -x -F -e '-- FAIL host-symbols-missing: ivs_not_defined' "$tmp/runs/host-symbols-extra.out"; then
      ok=0
      why host-exports-live "the checker failed, but not by the appended name: $(grep -m 1 -e 'FAIL' "$tmp/runs/host-symbols-extra.out")"
    fi
    # The other direction (review WR-04): an EXPORTED name the list does not carry. The scratch copy drops retro_run,
    # a name the object defines, so the exact-set comparison must refuse it as host-symbols-extra and name it. Without
    # this limb half of the comparison - the half that catches a symbol the pin does not know about - was unproved.
    awk '/^#/ { next } NF { print $1 }' "$root/data/expected/host-symbols.ids" \
      | grep -v -x -F -e retro_run > "$tmp/host-short.ids"
    if [ ! -s "$tmp/host-short.ids" ]; then
      ok=0
      why host-exports-live 'the shortened ids copy is empty, so the extra direction would compare nothing'
    elif CHECK_DIR="$root/tools/check" ROOT="$root" sh "$check_sym" --allow-prefix __asan_ --allow-prefix __ubsan_ \
      --allow-prefix __sanitizer_ "$shim" "$tmp/host-short.ids" > "$tmp/runs/host-symbols-short.out" 2>&1; then
      ok=0
      why host-exports-live 'the checker accepted an ids list missing a name the object exports'
    elif ! grep -q -x -F -e '-- FAIL host-symbols-extra: retro_run' "$tmp/runs/host-symbols-short.out"; then
      ok=0
      why host-exports-live "the checker failed, but not by the dropped name: $(grep -m 1 -e 'FAIL' "$tmp/runs/host-symbols-short.out")"
    fi
  fi
fi
result runner-process.host.exports

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
# self_control_run <slug> <expect> <check...>: runs the check against a stand-in and sets _sc to `ok`, `vacuous`
# or `wrong-reason`. <expect> is the COMPLETE set of finding tokens the stand-in is declared to produce, space
# separated, and the control is healthy only when the check's OBSERVED token set equals it exactly (review WR-02).
# A stand-in differs from a passing output in one limb, so an extra token means it differs in a second place and the
# rejection says nothing about the limb under test; a missing token means the declaration claims a limb the check
# never saw. Requiring the intended token's PRESENCE alone accepted the first case, which is the opposite of the
# property the committed stand-ins claim. Two tokens in a declaration are legitimate when ONE defect is judged by two
# limbs of the same check (an outcome word that is neither the expected outcome nor a pair of runner-outcomes.ids),
# and the declaration is what makes that explicit instead of invisible.
self_control_run() {
  _slug=$1; _exp=$2; shift 2
  : > "$tmp/why"
  if "$@"; then
    _sc=vacuous
    : > "$tmp/why"
    why judge-vacuous "the check accepted the fake runner of $_slug"
    return 0
  fi
  _scwant=$(printf '%s\n' $_exp | sort -u)
  _schave=$(sed -n 's/^\([a-z0-9-]*\):.*/\1/p' "$tmp/why" | sort -u)
  if [ -n "$_schave" ] && [ "$_scwant" = "$_schave" ]; then
    _sc=ok
    return 0
  fi
  _sc=wrong-reason
  {
    printf 'judge-wrong-reason: the check did not fail for exactly the declared set of %s:\n' "$_slug"
    printf 'declared: %s\n' "$(printf '%s ' $_exp)"
    printf 'observed: %s\n' "$(printf '%s ' ${_schave:-none})"
    cat "$tmp/why"
  } > "$tmp/why.2"
  mv "$tmp/why.2" "$tmp/why"
}

# self_control <slug> <expect> <check...>: runs the check; ok only when it fails for exactly the declared set.
self_control() {
  _slug=$1; _exp=$2; shift 2
  self_control_run "$_slug" "$_exp" "$@"
  ok=0
  if [ "$_sc" = ok ]; then ok=1; fi
  result "runner-process.judge.$_slug"
}
self_control exit-mismatch exit-code \
  check_scenario "$fakes/exit-mismatch.sh" fake-exit guest-assert-mismatch 10 guest-fail -
self_control no-report-end report-end \
  check_scenario "$fakes/no-report-end.sh" fake-end guest-assert-mismatch 10 guest-fail -
# One defect, two judged limbs: the outcome word stops being the expected one AND stops being a pair with exit 10.
self_control outcome-exit-disagree 'outcome outcome-exit-disagree' \
  check_scenario "$fakes/outcome-exit.sh" fake-pair guest-assert-mismatch 10 guest-fail -
self_control body-differs body-differs \
  check_repeat "$tmp" "$fakes/body.sh" fake-body diag-boot-pass "$tmp/runs/scen-diag-boot-pass.out"

# The host judge's two COMMITTED stand-ins (plan 15-02, TOOL-08, criterion 3.3): fake frontends written into $fakes
# by this suite and judged by the SAME check_host the real run is judged by. Each differs from a passing output in
# ONE limb, so a rejection for any other token means it differs in two places (self_control reports
# judge-wrong-reason) and a pass means the limb it was written to test is dead (judge-vacuous). No second .so is
# built, no fault site is added and no golden is added (GUARDRAILS 2.1). They are cases, not scratch probes.
cp "$root/data/golden/demo-lamp-tone-pass.digests" "$fakes/host.digests" || exit 1
{
  printf '#!/bin/sh\n'
  printf '# Fake frontend: the pass golden'"'"'s nine digest lines and frames 0 - a stub that rendered nothing.\n'
  printf 'printf "frames 0\\n"\n'
  printf 'grep "^digest[.]" "$(dirname "$0")/host.digests"\n'
} > "$fakes/host-vacuous.sh"
# The digest a poll-dropped host prints IS the empty chain's input digest, so this stand-in takes that value from the
# empty-poll run this suite has just made - never typed - and changes nothing else. With no value to substitute it
# prints the pass golden unchanged, so check_host ACCEPTS it and the case fails as judge-vacuous: the honest report
# of an empty-poll run that produced no input digest, rather than a rejection for a wrong reason.
_empty_input=$(sed -n 's/^digest[.]input //p' "$tmp/runs/host-empty.out" | head -n 1)
{
  printf '#!/bin/sh\n'
  printf '# Fake frontend: the pass golden'"'"'s nine digest lines with the input poll dropped, and frames 13.\n'
  printf 'printf "frames 13\\n"\n'
  if [ -n "$_empty_input" ]; then
    printf 'sed "s|^digest[.]input .*|digest.input %s|" "$(dirname "$0")/host.digests"\n' "$_empty_input"
  else
    printf 'cat "$(dirname "$0")/host.digests"\n'
  fi
} > "$fakes/host-digest-diff.sh"
# host_stand_in <slug>: run the fake frontend, require it to have printed something (a stand-in that produced no
# output would fail for the wrong reason), then judge its stdout with check_host.
host_stand_in() {
  run_one "$tmp" "fake-$1" "$fakes/$1.sh"
  if [ ! -s "$tmp/runs/fake-$1.out" ]; then
    why stand-in-empty "the fake frontend of $1 printed nothing"
    return 1
  fi
  check_host "$tmp/runs/fake-$1.out"
}
self_control host-vacuous frames-zero host_stand_in host-vacuous
self_control host-digest-diff digest-diff host_stand_in host-digest-diff
# The guard ON the guard (plan 15-04, review WR-02): a control is healthy only when the check's OBSERVED token set
# equals the DECLARED one, in both directions. Two limbs, over stand-ins this suite already has:
#   extra   - the pass golden's own lines with `frames 13` are an empty-poll output that dropped nothing, so
#             check_host_empty finds it identical to the pass golden (empty-poll-identical, the declared token) AND
#             still carrying the pass run's input (empty-poll-idle). Two limbs, and only the first is under test.
#   missing - the host-vacuous stand-in is declared to produce frames-zero AND digest-diff, but check_host returns at
#             frames-zero, so the digest limb is never exercised: a declaration that claimed it would report a limb as
#             covered that nothing tested.
# Without this id a self_control that only grepped for the intended token could not tell a one-limb stand-in from a
# two-limb one, which is the defect the review measured. Findings go to their own file because each self_control_run
# starts by clearing $tmp/why.
{ printf 'frames 13\n'; cat "$fakes/host.digests"; } > "$fakes/host-two-limb.out"
: > "$tmp/two.bad"
self_control_run host-two-limb empty-poll-identical check_host_empty "$fakes/host-two-limb.out"
case $_sc in
  ok) printf 'judge-two-limb: self_control accepted a stand-in that differs from a passing output in two limbs\n' >> "$tmp/two.bad" ;;
  wrong-reason) : ;;
  *) printf 'judge-two-limb: self_control refused the two-limb stand-in as %s, not judge-wrong-reason\n' "$_sc" >> "$tmp/two.bad" ;;
esac
self_control_run host-two-limb-missing 'frames-zero digest-diff' host_stand_in host-vacuous
case $_sc in
  ok) printf 'judge-two-limb: self_control accepted a declaration claiming a limb the check never exercised\n' >> "$tmp/two.bad" ;;
  wrong-reason) : ;;
  *) printf 'judge-two-limb: self_control refused the unexercised-limb declaration as %s, not judge-wrong-reason\n' "$_sc" >> "$tmp/two.bad" ;;
esac
: > "$tmp/why"
ok=1
if [ -s "$tmp/two.bad" ]; then ok=0; cat "$tmp/two.bad" > "$tmp/why"; fi
result runner-process.judge.host-two-limb
# Judge self-control of the round-trip comparison (plan 08-16, D-18): check_roundtrip must be shown to fail before it is trusted, twice. A fake runner
# that replays the report of ANOTHER scenario (diag-ctrl-psg-pass) for the hook run must be rejected for body-differs (the comparison with the plain
# report fails on a report that differs); a fake that replays the plain report unchanged, as a hook that never cut would, must be rejected for
# roundtrip-no-cut (equal reports are not enough: host.calls must show the cuts).
cp "$tmp/runs/scen-diag-ctrl-psg-pass.out" "$fakes/cps.report" 2> /dev/null || : > "$fakes/cps.report"
{
  printf '#!/bin/sh\n# Fake ivs-run: the report of another scenario.\n'
  printf 'cat "$(dirname "$0")/cps.report"\nexit 0\n'
} > "$fakes/rt-other.sh"
{
  printf '#!/bin/sh\n# Fake ivs-run: the plain report, as if the hook had never cut.\n'
  printf 'cat "$(dirname "$0")/dbp.report"\nexit 0\n'
} > "$fakes/rt-nocut.sh"
: > "$tmp/rt.bad"
rt_control() {
  _rtok=$1; shift
  : > "$tmp/why"
  if "$@"; then
    printf 'judge-vacuous: the check accepted the fake runner of %s\n' "$_rtok" >> "$tmp/rt.bad"
  elif ! grep -q -e "^$_rtok: " "$tmp/why"; then
    { printf 'judge-wrong-reason: the check failed, but without %s:\n' "$_rtok"; cat "$tmp/why"; } >> "$tmp/rt.bad"
  fi
}
rt_control body-differs check_roundtrip "$fakes/rt-other.sh" diag-boot-pass 600 2
rt_control roundtrip-no-cut check_roundtrip "$fakes/rt-nocut.sh" diag-boot-pass 600 2
cp "$tmp/rt.bad" "$tmp/why"
ok=1
[ ! -s "$tmp/rt.bad" ] || ok=0
result runner-process.judge.roundtrip-differs

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
# One defect, two judged limbs: the helper exits 0 instead of 2 and prints none of the refusal line.
self_control owner-run-refusal 'owner-run-exit owner-run-stderr' or_fake

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
  printf '# mutants: configuration=%s full=%s static=%s variants=%s variant-static=%s\n' \
    "${config:-none}" "$mut_full" "$mut_static" "$scen_full" "$scen_static"
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

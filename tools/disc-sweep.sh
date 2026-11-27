#!/bin/sh
# SPDX-License-Identifier: MIT
# Owner-local display-path sweep probe (plan 19-02, VID-07, criterion 5; docs/owner-images.md). NOT a gate step: the gate never passes it an
# image and CI never runs it. It sweeps the display-enable strobe over a FIXED, SMALL list of offsets against the owner's own EXEC and GROM
# images, one runner run per offset, and writes one line per offset to a file under refs/.
#
# What it is for, and what it is not. Phase 19 separates four rival display-path rules with authored guests inside the stic suite; those
# guests are project-owned content and they are the phase's evidence. This probe is the other route the research kept on purpose: an
# observation on the owner's own images, which the project may never run in CI and may never distribute. Its input is operator content, so it
# can never be a gate case (D11), and it is nobody's exit criterion: no criterion of Phase 19, no id and no verify block reads its output, and
# the phase's criteria 1 to 4 stand whether it runs or not. Its output is private observation - never evidence, never a claim, never a golden,
# never committed.
#
# The offset is the CPU-cycle bound of one run, taken from a fixed list that straddles the end of the first frame: on the NTSC family profile
# the scenario names, one frame is 59,736 master ticks at 4 ticks per CPU cycle, so frame 0 ends at CPU cycle 14,934. A bound below that
# boundary cannot have delivered frame 0 and must record `-` (the frame 0 displayed assert is `not-run`), and a bound at or above it records the
# displayed word the slot header carries. That negative half is what makes the table an observation and not a formality: the same run shape
# gives one value below the boundary and another above it. Nothing about the operator's images is copied into the output: a line holds the
# swept offset, the report's own outcome word and the observed value of the frame 0 displayed assert, and nothing else - no image digest, no
# image size, no image path and no image name, because a digest states identity and not rights and the file is a record of an operator run.
#
# Usage: sh tools/disc-sweep.sh [--preset dev|release] [--max-runs <n>] --out <output> <exec> <grom>
#   (the two positional arguments are the operator's EXEC and GROM files; the placeholders are spelled <exec> and <grom> here because the
#   rights ledger refuses the hyphenated spellings outside the two pages that already carry them, and a new occurrence is not a rename)
#   (default release; build/<preset>/runner/ivs-run must be built; --out is required and must resolve under refs/)
# Env: CI and GITHUB_ACTIONS must be empty or unset. TMPDIR is honoured for the temporary scenario copy (refused inside the work tree).
# Refuses (one stderr line `disc-sweep: <reason>`, exit 2), in this order: a CI environment; bad arguments; an image path whose physical
#   directory is inside the work tree outside refs/; an output path whose physical parent is inside the work tree and not under refs/ (a
#   parent that cannot be resolved is a refusal, never a blind write); a run count above the fixed bound; a missing runner or scenario.
#   The output rule is the flatwrap rule verbatim (tools/flatwrap/flatwrap.c, fw_inside_tree), resolved with a physical cd, so a literal
#   path, a spelling through `..` and a path through a symlink to a directory inside the tree all get the same answer. It comes before every
#   size check and before the images are looked for, so a path defect is reported as a path defect and never as a missing image.
# Skip: when neither image exists it prints `SKIPPED disc-sweep: no operator images (D11)` on stdout and exits 77. That is this helper's own
#   skip, the ctest skip code; the scenario data/scenarios/disc-sweep-probe.ivss declares a different one, `missing-fixture` with exit 13,
#   because the two are different limbs: 77 is a bare invocation with no image, 13 is the suite's case for a scenario whose exec role is the
#   operator's. Both are needed and neither is a failure.
# Exit: 0 the sweep ran and wrote its table; 1 the table could not be written; 2 refusal; 77 no operator images.
# Bounds: the offset list is fixed and small, the run count is refused above a fixed bound rather than silently truncated, every run carries
#   the runner's own wall cap, and the elapsed time is re-checked between runs so the total wall bound is a bound and not a hope. A run that
#   ends inconclusive - a timeout, a fault, a missing frame - is an outcome recorded in the table, never a failure of the probe.
LC_ALL=C; export LC_ALL
unset CDPATH   # a set CDPATH would let cd resolve a relative directory somewhere else and print it
die() { printf 'disc-sweep: %s\n' "$1" >&2; exit 2; }
[ -z "${CI:-}" ] && [ -z "${GITHUB_ACTIONS:-}" ] || die 'refusing to run in CI (CI or GITHUB_ACTIONS is set): owner images are never used there'
usage='usage: disc-sweep.sh [--preset dev|release] [--max-runs <n>] --out <output> <exec> <grom>'
case $0 in */*) here=${0%/*} ;; *) here=. ;; esac
root=$(cd -P -- "$here/.." 2> /dev/null && pwd -P) || die 'cannot find the repository root'
# The fixed, small offset list: the CPU-cycle bound of one run, straddling the 14,934-cycle end of frame 0 on the NTSC family profile.
offsets='14000 14934 15000 16000 18000 20000'
max_bound=16
wall_bound=300
run_wall_ms=20000
preset=release; out=; max_runs=
while [ "$#" -gt 0 ]; do
  case $1 in
    --preset) [ "$#" -ge 2 ] || die "$usage"; preset=$2; shift 2 ;;
    --max-runs) [ "$#" -ge 2 ] || die "$usage"; max_runs=$2; shift 2 ;;
    --out) [ "$#" -ge 2 ] || die "$usage"; out=$2; shift 2 ;;
    *) break ;;
  esac
done
case $preset in dev | release) ;; *) die "$usage" ;; esac
[ "$#" -eq 2 ] || die "$usage"
[ -n "$out" ] || die "$usage"
ex=$1; gr=$2
# The image location rule (tools/owner-run.sh): a path whose physical directory is inside the work tree but not under refs/ is refused. A
# directory that cannot be entered is not a refusal here: no file can be read in it either, so the exact-size check below refuses the image.
for p in "$ex" "$gr"; do
  case $p in */*) d=${p%/*} ;; *) d=. ;; esac
  d=$(cd -P -- "${d:-/}" 2> /dev/null && pwd -P) || continue
  case $d/ in "$root/refs/"*) ;; "$root/"*) die "an image is inside the work tree outside refs/: $p" ;; esac
done
# The output location rule (tools/flatwrap/flatwrap.c fw_inside_tree): the physical parent of the output is compared with the physical
# work-tree root and allowed only under refs/, because a spelling through `..` or through a symlink must get the same answer as the literal.
case $out in */*) od=${out%/*} ;; *) od=. ;; esac
odname=$od
od=$(cd -P -- "${od:-/}" 2> /dev/null && pwd -P) || die "cannot resolve the output directory ${odname:-/}: the output must be under refs/"
case $od/ in
  "$root/refs/"*) ;;
  "$root/"*) die "the output $out is inside the work tree outside refs/" ;;
esac
# The run-count bound, enforced before the first run and before the output is created: a bound that would be exceeded refuses instead of
# truncating the sweep silently.
runs=0
for o in $offsets; do runs=$((runs + 1)); done
if [ -n "$max_runs" ]; then
  case $max_runs in '' | *[!0-9]*) die "$usage" ;; esac
  [ "$max_runs" -le "$max_bound" ] || die "the run count $max_runs is above the bound $max_bound"
  [ "$runs" -le "$max_runs" ] || die "the run count $runs is above the bound $max_runs"
fi
[ "$runs" -le "$max_bound" ] || die "the run count $runs is above the bound $max_bound"
# The helper's own skip, a different code from the scenario's exit 13 (see the header).
if [ ! -e "$ex" ] && [ ! -e "$gr" ]; then
  printf 'SKIPPED disc-sweep: no operator images (D11)\n'
  exit 77
fi
run=$root/build/$preset/runner/ivs-run
scn=$root/data/scenarios/disc-sweep-probe.ivss
[ -x "$run" ] || die "no runner build/$preset/runner/ivs-run (cmake --build --preset $preset)"
[ -f "$scn" ] || die 'no scenario data/scenarios/disc-sweep-probe.ivss'
sized() { [ -f "$2" ] && [ "$(wc -c < "$2" | tr -d ' ')" = "$3" ] || die "the $1 image is not a regular file of exactly $3 bytes"; }
sized exec "$ex" 8192
sized grom "$gr" 2048
tmp=$(mktemp -d "${TMPDIR:-/tmp}/disc-sweep.XXXXXX" 2> /dev/null) || die 'cannot make a temporary directory'
trap 'rm -rf "$tmp"' EXIT
trap 'exit 1' HUP INT TERM
case $(cd -P -- "$tmp" && pwd -P)/ in "$root/"*) die 'TMPDIR is inside the work tree' ;; esac
printf '# disc-sweep: %s runs, run-count bound %s, total wall bound %ss, per-run wall cap %sms\n' \
  "$runs" "$max_bound" "$wall_bound" "$run_wall_ms" > "$out" || { printf 'disc-sweep: cannot write the output %s\n' "$out" >&2; exit 1; }
printf 'offset_cpu_cycles outcome displayed\n' >> "$out" \
  || { printf 'disc-sweep: cannot write the output %s\n' "$out" >&2; exit 1; }
started=$(date +%s)
for off in $offsets; do
  # The wall bound is re-checked between runs: a run that ends late cannot hold the sweep open past it.
  if [ $(( $(date +%s) - started )) -ge "$wall_bound" ]; then
    printf '%s wall-bound -\n' "$off" >> "$out"
    continue
  fi
  # The temporary copy carries this offset as its run-to bound, one in-memory `capture video 0 1` (a frame assert reads a capture, and a
  # capture keeps the slot in memory - it names no output file and no rate), and one frame 0 displayed assert whose `got` value is the
  # observation: an assert line prints `got` whether it passes, fails or never ran. A failing or unrun assert is an outcome of the run, not a
  # failure of the probe.
  awk -v off="$off" '
    /^run-to / { print "capture video 0 1"; printf "run-to %s cpu-cycles stop halted\n", off; next }
    /^assert halted == yes$/ { print; print "assert frame 0 displayed == yes"; next }
    { print }
  ' "$scn" > "$tmp/scn.ivss" || { printf 'disc-sweep: cannot write the temporary scenario copy\n' >&2; exit 1; }
  "$run" --exec "$ex" --grom "$gr" --wall-timeout-ms "$run_wall_ms" "$tmp/scn.ivss" > "$tmp/report" 2> "$tmp/err" || true
  oc=$(sed -n 's/^outcome //p' "$tmp/report" | sed -n '1p')
  dp=$(awk '/ frame 0 displayed / { for (i = 1; i < NF; i++) if ($i == "got") { print $(i + 1); exit } }' "$tmp/report")
  [ -n "$oc" ] || oc=no-report
  [ -n "$dp" ] || dp=-
  printf '%s %s %s\n' "$off" "$oc" "$dp" >> "$out"
done
exit 0

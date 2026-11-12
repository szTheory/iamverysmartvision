#!/bin/sh
# SPDX-License-Identifier: MIT
# CI job-summary helper and `times` parser for tools/gate.sh (D-42, D-44). Not a step: the gate
# calls it, and the step ci-summary self-tests it on synthetic inputs (controls cannot drive it).
# Usage:
#   sh ci-summary.sh --cpu < <output of POSIX times>
#     Reads line 2 of `times` (the children's user and system time) in either observed shape,
#     `0m0.000000s` (dash) or `0m0.002s` (macOS sh), and prints `gate-cpu user=<s.ss> sys=<s.ss>`,
#     or `gate-cpu unknown` when the input does not parse. Exit status 0 always.
#   sh ci-summary.sh <gate-tmp-dir>
#     Writes nothing and exits 0 when GITHUB_STEP_SUMMARY is empty. Otherwise appends markdown to
#     that file, built only from files the gate writes into the directory:
#       summary.line   the final `gate:` line (absent on an interrupted run)
#       measure.lines  the gate-time, gate-env and gate-cpu lines (optional)
#       step.times     `step-time <name> <s>` lines
#       fail.lines     `-- FAIL <token>: <detail>` lines
#     Contents: a headline with the gate status and the tokens of the `gate:` line; a "Skips and
#     missing" table (every token whose key contains skipped, missing, pending or unknown, nonzero
#     values first, one `none` row when there is no such token); the 8 slowest step-time entries;
#     the first 40 FAIL lines in a fenced block with every backtick removed (FAIL details can carry
#     PR-controlled paths) and `(+<n> more)` when capped.
#     Prints nothing on stdout or stderr, emits no workflow annotations, and exits 0 even when the
#     summary file cannot be written, so it can never change the gate's verdict.
LC_ALL=C; export LC_ALL

if [ "${1:-}" = --cpu ]; then
  sed -n 2p | awk '
    function secs(f,   a) {
      if (f !~ /^[0-9]+m[0-9]+([.][0-9]+)?s$/) return -1
      split(f, a, /[ms]/)
      return a[1] * 60 + a[2]
    }
    { u = secs($1); s = secs($2); n = NF }
    END {
      if (NR == 1 && n == 2 && u >= 0 && s >= 0) printf "gate-cpu user=%.2f sys=%.2f\n", u, s
      else print "gate-cpu unknown"
    }'
  exit 0
fi

[ -n "${GITHUB_STEP_SUMMARY:-}" ] || exit 0
exec > /dev/null 2>&1
dir=${1:-}
[ -n "$dir" ] && [ -d "$dir" ] || exit 0
md="$dir/summary.md"

strip() { tr -d '`|\r'; }

{
  status="gate: unknown (no summary line; interrupted run)"
  line=
  if [ -s "$dir/summary.line" ]; then
    line=$(sed -n '$p' "$dir/summary.line" | strip)
    status=$(printf '%s\n' "$line" | awk '{ print $1, $2 }')
  fi
  printf '### %s\n\n' "$status"
  toks=$(printf '%s\n' "$line" | tr ' ' '\n' | sed -n '3,$p' | grep '=' | tr '\n' ' ' | sed 's/ $//')
  meas=
  if [ -s "$dir/measure.lines" ]; then meas=$(strip < "$dir/measure.lines"); fi
  if [ -n "$toks$meas" ]; then
    printf '```\n'
    if [ -n "$toks" ]; then printf '%s\n' "$toks"; fi
    if [ -n "$meas" ]; then printf '%s\n' "$meas"; fi
    printf '```\n\n'
  fi

  printf '#### Skips and missing\n\n| token | value |\n| --- | --- |\n'
  rows=$(printf '%s\n' "$line" | tr ' ' '\n' | awk -F= '
    NF >= 2 && $1 ~ /(skipped|missing|pending|unknown)/ {
      v = substr($0, length($1) + 2)
      print ((v ~ /^0+$/) ? 1 : 0), NR, $1, v
    }' | sort -k1,1n -k2,2n | awk '{ printf "| %s | %s |\n", $3, $4 }')
  if [ -n "$rows" ]; then printf '%s\n' "$rows"; else printf '| none | |\n'; fi
  printf '\n'

  printf '#### Slowest steps\n\n| step | seconds |\n| --- | --- |\n'
  slow=
  if [ -s "$dir/step.times" ]; then
    slow=$(grep '^step-time [^ ][^ ]* [0-9][0-9]*$' "$dir/step.times" | strip | sort -k3,3nr -k2,2 |
      sed -n '1,8p' | awk '{ printf "| %s | %s |\n", $2, $3 }')
  fi
  if [ -n "$slow" ]; then printf '%s\n' "$slow"; else printf '| none | |\n'; fi
  printf '\n'

  nfail=0
  if [ -s "$dir/fail.lines" ]; then nfail=$(grep -c '^-- FAIL ' "$dir/fail.lines"); fi
  printf '#### Failures (%s)\n\n' "$nfail"
  if [ "$nfail" -gt 0 ]; then
    printf '```\n'
    grep '^-- FAIL ' "$dir/fail.lines" | sed -n '1,40p' | tr -d '`\r'
    printf '```\n'
    if [ "$nfail" -gt 40 ]; then printf '(+%s more)\n' $((nfail - 40)); fi
  else
    printf 'none\n'
  fi
  printf '\n'
} > "$md"

cat "$md" >> "$GITHUB_STEP_SUMMARY"
exit 0

#!/bin/sh
# SPDX-License-Identifier: MIT
# Hosted run recorder (PKG-04, D-44). Orchestrator only; never called by the gate, CI or an
# executor (AUTONOMY.md section 4: the GitHub API rate limit is shared by every agent).
#
#   sh tools/ci-record.sh <run-id>
#   sh tools/ci-record.sh --from-files <meta.txt> <log.txt>
#
# Prints one tab-separated row for .planning/ci-runs.tsv on stdout; the orchestrator appends it
# (`>> .planning/ci-runs.tsv`) and the gate step ci-log checks the format. Columns:
#   date run_id sha event conclusion queue_s setup_s build_s test_s gate_s total_s ncpu cpu_s
# date = UTC day of the run's createdAt; queue_s = job startedAt - run createdAt (queue time is
# not measurable inside the job); setup_s = the checkout step's duration; build_s = the gate's
# `step-time c-build`; gate_s = `gate-time total`; test_s = gate_s - build_s; total_s = run
# updatedAt - createdAt; ncpu from `gate-env`; cpu_s = user + sys of `gate-cpu`, rounded.
# A value the inputs lack is -1, so the row stays numeric.
#
# <run-id> mode makes exactly two GitHub CLI calls: the run metadata (JSON reduced by --jq to
# one line) and the run log, filtered to the gate's measurement lines. Both go to files in a
# temporary directory, then the shared parser runs; --from-files runs the same parser offline.
# meta.txt is one space-separated line:
#   <run_id> <createdAt> <job startedAt> <updatedAt> <conclusion> <headSha> <event>
#   <checkout startedAt> <checkout completedAt>
# with ISO-8601 UTC times (YYYY-MM-DDTHH:MM:SS[.fff]Z) and `-` for a missing value. log.txt holds
# gate output lines (`gate-time total=`, `step-time `, `gate-env `, `gate-cpu `), either bare or
# with the `<job> TAB <step> TAB <timestamp> ` prefix of the hosted log.
# Exit: 0 row printed; 1 a GitHub CLI call failed or the inputs give no valid row; 2 usage error
# or the GitHub CLI (gh) is missing. --help and -h print this usage and exit 0.
LC_ALL=C; export LC_ALL

usage() {
  sed -n '3,28p' "$0" | sed 's/^# \{0,1\}//'
}

parse() {
  awk -v meta="$1" '
    function epoch(s,   y, mo, d, h, mi, se, era, yoe, mp, doy, doe) {
      if (s !~ /^[0-9][0-9][0-9][0-9]-[0-9][0-9]-[0-9][0-9]T[0-9][0-9]:[0-9][0-9]:[0-9][0-9](\.[0-9]+)?Z$/) return -1
      y = substr(s, 1, 4) + 0; mo = substr(s, 6, 2) + 0; d = substr(s, 9, 2) + 0
      h = substr(s, 12, 2) + 0; mi = substr(s, 15, 2) + 0; se = substr(s, 18, 2) + 0
      if (y < 1970 || mo < 1 || mo > 12 || d < 1 || d > 31) return -1
      # Days from 1970-01-01 in the proleptic Gregorian calendar (civil-date arithmetic).
      if (mo <= 2) y--
      era = int(y / 400); yoe = y - era * 400
      mp = (mo + 9) % 12
      doy = int((153 * mp + 2) / 5) + d - 1
      doe = yoe * 365 + int(yoe / 4) - int(yoe / 100) + doy
      return (era * 146097 + doe - 719468) * 86400 + h * 3600 + mi * 60 + se
    }
    function dur(a, b) { return (a < 0 || b < 0 || b < a) ? -1 : b - a }
    function num(v) { return v ~ /^[0-9]+$/ ? v + 0 : -1 }
    BEGIN {
      if ((getline line < meta) <= 0) { print "ci-record: empty metadata" > "/dev/stderr"; exit 1 }
      sub(/\r$/, "", line)
      n = split(line, m, " ")
      if (n != 9) { print "ci-record: metadata has " n " fields, want 9" > "/dev/stderr"; exit 1 }
      build = -1; gate = -1; ncpu = -1; cpu = -1
    }
    {
      sub(/\r$/, "")
      sub(/^[^\t]*\t[^\t]*\t/, "")
      sub(/^[0-9][0-9][0-9][0-9]-[0-9][0-9]-[0-9][0-9]T[0-9:.]*Z /, "")
      k = split($0, w, " ")
      if (w[1] == "gate-time" && k == 2 && w[2] ~ /^total=/) { gate = num(substr(w[2], 7)) }
      else if (w[1] == "step-time" && k == 3 && w[2] == "c-build") { build = num(w[3]) }
      else if (w[1] == "gate-env" && w[2] ~ /^ncpu=/) { ncpu = num(substr(w[2], 6)) }
      else if (w[1] == "gate-cpu" && k == 3 && w[2] ~ /^user=[0-9]+(\.[0-9]+)?$/ && w[3] ~ /^sys=[0-9]+(\.[0-9]+)?$/) {
        cpu = int(substr(w[2], 6) + substr(w[3], 5) + 0.5)
      }
      else if (w[1] == "gate-cpu") { cpu = -1 }
    }
    END {
      if (n != 9) exit 1
      created = epoch(m[2]); started = epoch(m[3]); updated = epoch(m[4])
      if (m[1] !~ /^[0-9]+$/ || created < 0 || length(m[6]) != 40 || m[6] !~ /^[0-9a-f]+$/) {
        print "ci-record: metadata lacks a run id, createdAt or 40-digit sha" > "/dev/stderr"; exit 1
      }
      conclusion = m[5] ~ /^[a-z_]+$/ ? m[5] : "-"
      event = m[7] ~ /^[a-z_]+$/ ? m[7] : "-"
      test_s = (gate >= 0 && build >= 0 && gate >= build) ? gate - build : -1
      printf "%s\t%s\t%s\t%s\t%s\t%d\t%d\t%d\t%d\t%d\t%d\t%d\t%d\n", substr(m[2], 1, 10), m[1], m[6], event, conclusion,
        dur(created, started), dur(epoch(m[8]), epoch(m[9])), build, test_s, gate, dur(created, updated), ncpu, cpu
    }
  ' "$2"
}

case ${1-} in
  -h | --help) usage; exit 0 ;;
  '') usage >&2; exit 2 ;;
  --from-files)
    [ "$#" -eq 3 ] && [ -f "$2" ] && [ -f "$3" ] || { usage >&2; exit 2; }
    parse "$2" "$3"
    exit $? ;;
  *[!0-9]*) usage >&2; exit 2 ;;
esac
[ "$#" -eq 1 ] || { usage >&2; exit 2; }
command -v gh > /dev/null 2>&1 || { printf 'ci-record: gh not found\n' >&2; exit 2; }
id=$1
tmp=$(mktemp -d "${TMPDIR:-/tmp}/ci-record.XXXXXX") || exit 1
trap 'rm -rf "$tmp"' EXIT
trap 'exit 1' HUP INT TERM
jq_meta='[.databaseId, .createdAt, .jobs[0].startedAt, .updatedAt, .conclusion, .headSha, .event,
  ((.jobs[0].steps // []) | map(select(.name | test("checkout"; "i"))) | .[0] // {} | .startedAt, .completedAt)]
  | map(if . == null or . == "" then "-" else tostring end) | join(" ")'
gh run view "$id" --json databaseId,createdAt,startedAt,updatedAt,conclusion,headSha,event,jobs \
  --jq "$jq_meta" > "$tmp/meta.txt" || exit 1
gh run view "$id" --log > "$tmp/log.raw" || exit 1
grep -F -e 'gate-time total=' -e 'step-time ' -e 'gate-env ' -e 'gate-cpu ' "$tmp/log.raw" > "$tmp/log.txt"
[ "$?" -le 1 ] || exit 1
parse "$tmp/meta.txt" "$tmp/log.txt"

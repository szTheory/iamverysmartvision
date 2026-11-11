#!/bin/sh
# SPDX-License-Identifier: MIT
# Step ci-log: format check of the hosted run record .planning/ci-runs.tsv (PKG-04, D-44).
# The record is append-only and written by the orchestrator from `sh tools/ci-record.sh <run-id>`
# (one row per hosted run). This step cannot check that every run was recorded (that needs the
# GitHub API); it checks that what is recorded is well formed and comparable.
# Format: tab-separated; line 1 is exactly the 13 column names
#   date run_id sha event conclusion queue_s setup_s build_s test_s gate_s total_s ncpu cpu_s
# then one row per run: date YYYY-MM-DD, run_id digits, sha 40 lower-case hex digits, event and
# conclusion non-empty, the 8 numeric columns non-negative integers or -1 (value not available).
# Tokens: ci-log-header: <detail> (file missing, or line 1 is not exactly the 13 names),
#   ci-log-fields: line <n> (a row without 13 fields, a bad date, run id, sha, string or integer),
#   ci-log-duplicate: line <n> run <id> (a run id already recorded above),
#   ci-log-order: line <n> (a date earlier than the row above).
# Stats: ci-runs=<rows>.
LC_ALL=C; export LC_ALL
: "${ROOT:?ROOT is not set}"
: "${CHECK_DIR:?CHECK_DIR is not set}"
: "${FILELIST:?FILELIST is not set}"
tmp=$(mktemp -d "${TMPDIR:-/tmp}/ci-log.XXXXXX") || exit 1
trap 'rm -rf "$tmp"' EXIT
trap 'exit 1' HUP INT TERM
find="$tmp/findings"; : > "$find"
rec="$ROOT/.planning/ci-runs.tsv"

if [ ! -f "$rec" ]; then
  printf 'ci-log-header %s\n' ".planning/ci-runs.tsv missing" >> "$find"
else
  awk -F '\t' -v find="$find" -v stats="$tmp/rows" '
    function bad(t, d) { printf "%s %s\n", t, d >> find }
    function isint(v) { return v == "-1" || v ~ /^[0-9]+$/ }
    BEGIN {
      want = "date\trun_id\tsha\tevent\tconclusion\tqueue_s\tsetup_s\tbuild_s\ttest_s\tgate_s\ttotal_s\tncpu\tcpu_s"
      rows = 0; prev = ""
    }
    NR == 1 {
      if ($0 != want) bad("ci-log-header", "line 1 is not the 13 column names")
      next
    }
    {
      rows++
      ok = 1
      if (NF != 13) ok = 0
      else {
        if ($1 !~ /^[0-9][0-9][0-9][0-9]-[0-1][0-9]-[0-3][0-9]$/) ok = 0
        if ($2 !~ /^[0-9]+$/) ok = 0
        if (length($3) != 40 || $3 !~ /^[0-9a-f]+$/) ok = 0
        if ($4 !~ /^[a-z_-]+$/ || $5 !~ /^[a-z_-]+$/) ok = 0
        for (i = 6; i <= 13; i++) if (!isint($i)) ok = 0
      }
      if (!ok) { bad("ci-log-fields", "line " NR); next }
      if ($2 in seen) bad("ci-log-duplicate", "line " NR " run " $2)
      seen[$2] = 1
      if (prev != "" && $1 < prev) bad("ci-log-order", "line " NR)
      prev = $1
    }
    END {
      if (NR == 0) bad("ci-log-header", "empty file")
      print rows > stats
    }
  ' "$rec" || printf 'ci-log-header %s\n' "awk error" >> "$find"
fi

if [ -s "$find" ]; then
  sort -u "$find" | while IFS= read -r l; do printf -- '-- FAIL %s: %s\n' "${l%% *}" "${l#* }"; done
  exit 1
fi
printf -- '-- ci-log pass ci-runs=%s\n' "$(cat "$tmp/rows")"

#!/bin/sh
# SPDX-License-Identifier: MIT
# Suite bench (plan 08-18, D-21 to D-26, D-39): the benchmark tool ivs-bench (tools/bench) run in its --verify-only mode, which reads
# no clock, so no timing ever runs in the gate. Every case runs the real tool of this preset against the committed workload table,
# scenarios and goldens (data/bench, data/scenarios, data/golden) and the build-tree images (the content directory), and judges
# its exit code, its stderr and the TSV on its stdout; every scratch input is made in a temporary directory, never in the repository.
# bench.verify.<workload>: `--verify-only --workload <id>` exits 0 with nothing on stderr and prints the format line `bench-format 1`,
# the header and exactly one row of status `verified` for that workload, whose scenario column is the one the table names, whose
# tier column is repeatability-only, whose ci_time column is the pointer text, and whose throughput and latency columns are empty.
# Output: the harness TAP shape (header with IVS_TEST_NONCE or none, plan, one result per case, trailer) on stdout and, when
# IVS_TEST_OUT_DIR is set, in <dir>/bench.txt.
# Usage: sh bench-tap.sh <source root> <ivs-bench> <content dir>   Exit: 0 pass, 1 fail, 77 the tool is absent (every case skipped).
# Env: IVS_TEST_NONCE, IVS_TEST_OUT_DIR.
LC_ALL=C; export LC_ALL
[ "$#" -eq 3 ] || { printf 'usage: sh bench-tap.sh <source root> <ivs-bench> <content dir>\n' >&2; exit 1; }
root=$1
bench=$2
content=$3
if [ ! -x "$bench" ]; then
  printf 'bench-tap: %s is not executable, every case is skipped\n' "$bench" >&2
  exit 77
fi
[ -d "$content" ] || { printf 'bench-tap: no content dir %s\n' "$content" >&2; exit 1; }
[ -f "$root/data/bench/workloads.tsv" ] || { printf 'bench-tap: no %s/data/bench/workloads.tsv\n' "$root" >&2; exit 1; }
tmp=$(mktemp -d "${TMPDIR:-/tmp}/bench-tap.XXXXXX") || exit 1
trap 'rm -rf "$tmp"' EXIT
trap 'exit 1' HUP INT TERM

suite=bench
nonce=${IVS_TEST_NONCE:-none}

: > "$tmp/results"
n=0
executed=0
skipped=0
failed=0
# Appends the result of case $n ($tid) from $ok and $tmp/why.
report() {
  if [ "$ok" -eq 1 ]; then
    printf 'ok %s %s\n' "$n" "$tid" >> "$tmp/results"
  else
    printf 'not ok %s %s\n' "$n" "$tid" >> "$tmp/results"
    sed 's/^/# /' "$tmp/why" >> "$tmp/results"
    failed=$((failed + 1))
  fi
}

# The columns of a row in their three groups (provenance and static, throughput, latency, then the pointer), as the output
# of ivs-bench states them; the test holds its own copy of the list.
cols='workload status scenario tier build_id optimized sanitizers ncpu warmup iterations ticks_per_iter cpu_cycles_per_iter frames_per_iter thr_cycles_per_s_wall thr_cycles_per_s_cpu thr_frames_per_s realtime_x ns_per_guest_cycle lat_first_ms lat_median_ms lat_max_ms ci_time'

# tsv_check <file> <rows> <status> <workload> <scenario>: the format line, the header, <rows> data lines, and for each row the
# provenance rules and, for status verified, filled static columns (8 to 13) and empty throughput and latency columns (14 to 21).
# Prints one line per finding; the caller treats any output as failure.
tsv_check() {
  awk -F '\t' -v cols="$cols" -v want="$2" -v status="$3" -v wl="$4" -v scn="$5" '
    BEGIN { nc = split(cols, c, " "); rows = 0 }
    NR == 1 { if ($0 != "bench-format 1") print "line 1 is not the format line: " $0; next }
    NR == 2 {
      if (NF != nc) { print "the header has " NF " fields, not " nc; next }
      for (i = 1; i <= nc; i++) if ($i != c[i]) print "header column " i " is " $i ", not " c[i]
      next
    }
    {
      rows++
      if (NF != nc) { print "row " rows " has " NF " fields, not " nc; next }
      if ($1 != wl) print "row " rows ": workload is " $1 ", not " wl
      if ($2 != status) print "row " rows ": status is " $2 ", not " status
      if ($3 != scn) print "row " rows ": scenario is " $3 ", not " scn
      if ($4 != "repeatability-only") print "row " rows ": tier is " $4
      if ($5 == "") print "row " rows ": build_id is empty"
      if ($6 != "0" && $6 != "1") print "row " rows ": optimized is " $6
      if ($7 != "on" && $7 != "off") print "row " rows ": sanitizers is " $7
      if ($22 != "see-ci-runs.tsv") print "row " rows ": ci_time is " $22
      if (status == "verified") {
        for (i = 8; i <= 13; i++) if ($i !~ /^[0-9]+$/) print "row " rows ": " c[i] " is not a number: " $i
        for (i = 14; i <= 21; i++) if ($i != "") print "row " rows ": " c[i] " is not empty: " $i
      }
    }
    END { if (NR - 2 != want) print "the output holds " (NR - 2) " rows, not " want }
  ' "$1"
}

# verify_case <workload> <scenario>: bench.verify.<workload>.
verify_case() {
  n=$((n + 1))
  tid="bench.verify.$1"
  executed=$((executed + 1))
  ok=1
  "$bench" --verify-only --root "$root" --content-dir "$content" --workload "$1" > "$tmp/v.out" 2> "$tmp/v.err"
  rc=$?
  if [ "$rc" -ne 0 ]; then
    ok=0
    { printf 'ivs-bench --verify-only exit %s, want 0\n' "$rc"; cat "$tmp/v.err"; } > "$tmp/why"
  elif [ -s "$tmp/v.err" ]; then
    ok=0
    { printf 'stderr is not empty\n'; cat "$tmp/v.err"; } > "$tmp/why"
  else
    tsv_check "$tmp/v.out" 1 verified "$1" "$2" > "$tmp/why"
    if [ -s "$tmp/why" ]; then ok=0; fi
  fi
  report
}

verify_case bench-lamp-idle bench-lamp-idle

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

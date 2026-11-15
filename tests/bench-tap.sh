#!/bin/sh
# SPDX-License-Identifier: MIT
# Suite bench (plan 08-18, D-21 to D-26, D-39): the benchmark tool ivs-bench (tools/bench) run in its --verify-only mode, which reads
# no clock, so no timing ever runs in the gate. Every case runs the real tool of this preset against the committed workload table,
# scenarios and goldens (data/bench, data/scenarios, data/golden) and the build-tree images (the content directory), and judges
# its exit code, its stderr and the TSV on its stdout; every scratch input is made in a temporary directory, never in the repository.
# bench.verify.<workload>: `--verify-only --workload <id>` exits 0 with nothing on stderr and prints the format line `bench-format 1`,
# the header and exactly one row of status `verified` for that workload, whose scenario column is the one the table names, whose
# tier column is repeatability-only, whose ci_time column is the pointer text, and whose throughput and latency columns are empty.
# bench.mutant.refuse-mutated-golden (D-22, D-25): a golden copied to a scratch directory with one hex digit of a digest line changed
# makes the tool exit 15 with the bench-refuse line naming the mutated key and a row of status refused without any measurement, in a
# verification-only run and in a run of one timed iteration (the timed path must stay unreachable). bench.refuse.wrong-ticks: a
# scratch workload table whose ticks differ from the golden's final machine time is refused the same way. bench.refuse.bad-args: an
# unknown option, a missing value, a repeated option, a non-numeric table value, an unknown workload, a missing scenario file, a missing
# golden and a missing --content-dir each end with exit 14, one line on stderr and nothing on stdout. bench.tsv.format: the format
# line, the header, the three contiguous groups, the field count of every row, the tier and ci_time text. bench.judge.refuse-has-no-timing:
# the checker that asserts the absence of any measurement (no_measurement, used by the mutant and wrong-ticks cases) is shown able to
# fail on synthetic rows that carry a value in thr_cycles_per_s_cpu or ns_per_guest_cycle. No case reads a clock: the one run without
# --verify-only is given a mutated golden and is refused after the verification run, before any timing.
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
verify_case bench-lamp-input bench-lamp-input
# The cold workload reuses the existing scenario demo-lamp-tone-pass and its golden: its scenario column names that scenario.
verify_case bench-lamp-cold demo-lamp-tone-pass

# ---- plan 08-18, Task 3: the refusal cases, the format proof and the self-control of the absence check ----

# no_measurement <row file>: the assertion that a refused row holds no measurement. The file holds exactly one row of all 22 fields, and
# every column from ncpu to lat_max_ms (positions 8 to 21: the static numbers, the throughput columns thr_cycles_per_s_wall,
# thr_cycles_per_s_cpu, thr_frames_per_s, realtime_x and ns_per_guest_cycle, and the three latency columns) is empty, so no nanosecond
# figure can appear in a refused row. Prints one line per finding; any output is failure.
no_measurement() {
  awk -F '\t' -v cols="$cols" '
    BEGIN { nc = split(cols, c, " ") }
    NF != nc { print "the row has " NF " fields, not " nc; next }
    { for (i = 8; i <= 21; i++) if ($i != "") print c[i] " holds a value: " $i }
    END { if (NR != 1) print "the file holds " NR " rows, not 1" }
  ' "$1"
}

# golden_ticks <scenario>: the final machine time of the golden of a scenario (its last step.<k>.now_ticks).
golden_ticks() {
  awk '$1 ~ /^step[.][0-9]+[.]now_ticks$/ { t = $2 } END { print t }' "$root/data/golden/$1.digests"
}

# expect_refusal <exit code> <the stderr line> <workload> <scenario>: the files $tmp/r.out and $tmp/r.err are those of a run that must
# have been refused: exit 15, the one bench-refuse line, the format line and header, one row of status refused whose provenance
# columns are filled, and no measurement in it. 0 when so; otherwise 1 with the finding in $tmp/why.
expect_refusal() {
  if [ "$1" -ne 15 ]; then
    { printf 'ivs-bench exit %s, want 15\n' "$1"; cat "$tmp/r.err"; } > "$tmp/why"
    return 1
  fi
  printf '%s\n' "$2" > "$tmp/want.err"
  if ! cmp -s "$tmp/r.err" "$tmp/want.err"; then
    { printf 'stderr is not the one line: %s\n' "$2"; cat "$tmp/r.err"; } > "$tmp/why"
    return 1
  fi
  tsv_check "$tmp/r.out" 1 refused "$3" "$4" > "$tmp/why"
  if [ -s "$tmp/why" ]; then return 1; fi
  sed -n 3p "$tmp/r.out" > "$tmp/r.row"
  no_measurement "$tmp/r.row" > "$tmp/why"
  if [ -s "$tmp/why" ]; then return 1; fi
  return 0
}

# The refusal cases (the mutated golden, the wrong ticks) use the workload bench-lamp-cold, the 13-frame run of the scenario
# demo-lamp-tone-pass: the refusal comes after iteration 0 whatever the workload, so the cheapest one proves the same path (a process
# of the sanitized preset costs about 0.1 cpu-s to start, and a 120-frame run about 0.1 more).
# bench.mutant.refuse-mutated-golden (D-22, D-25): a copy of the golden of the cold workload with one hex digit of its first digest
# line changed, in a scratch directory, is given to the tool through --golden-dir; both the verification-only run and a run of one
# timed iteration (a scratch workload table: the timed path must be unreachable) exit 15, print the bench-refuse line naming the
# workload and the mutated key, and print a refused row without any measurement. The copy is checked to differ from the golden in
# exactly one line, so the mutation cannot be a no-op.
n=$((n + 1))
tid=bench.mutant.refuse-mutated-golden
executed=$((executed + 1))
ok=1
mkdir "$tmp/mg"
awk -v keyfile="$tmp/mut.key" '
  !done && $1 ~ /^digest[.]/ {
    d = substr($2, 1, 1)
    nd = (d == "0") ? "1" : "0"
    print $1 > keyfile
    print $1 " " nd substr($2, 2)
    done = 1
    next
  }
  { print }
' "$root/data/golden/demo-lamp-tone-pass.digests" > "$tmp/mg/demo-lamp-tone-pass.digests"
key=$(cat "$tmp/mut.key" 2> /dev/null)
ticks=$(golden_ticks demo-lamp-tone-pass)
printf 'bench-lamp-cold\tdemo-lamp-tone-pass\t1\t1\t0\t%s\tscratch table of one iteration\n' "$ticks" > "$tmp/one.tsv"
changed=$(diff "$root/data/golden/demo-lamp-tone-pass.digests" "$tmp/mg/demo-lamp-tone-pass.digests" | grep -c '^[<>] ')
if [ -z "$key" ] || [ "$changed" -ne 2 ]; then
  ok=0
  printf 'judge-vacuous: the scratch golden differs from the golden in %s lines (key %s), want one line changed\n' "$((changed / 2))" "$key" > "$tmp/why"
else
  want="bench-refuse bench-lamp-cold digest-mismatch $key"
  "$bench" --verify-only --root "$root" --content-dir "$content" --golden-dir "$tmp/mg" --workload bench-lamp-cold > "$tmp/r.out" 2> "$tmp/r.err"
  rc=$?
  if ! expect_refusal "$rc" "$want" bench-lamp-cold demo-lamp-tone-pass; then
    ok=0
    { printf 'verify-only run: '; cat "$tmp/why"; } > "$tmp/why.2"
    mv "$tmp/why.2" "$tmp/why"
  else
    "$bench" --root "$root" --content-dir "$content" --golden-dir "$tmp/mg" --workloads "$tmp/one.tsv" --workload bench-lamp-cold > "$tmp/r.out" 2> "$tmp/r.err"
    rc=$?
    if ! expect_refusal "$rc" "$want" bench-lamp-cold demo-lamp-tone-pass; then
      ok=0
      { printf 'timed run of one iteration: '; cat "$tmp/why"; } > "$tmp/why.2"
      mv "$tmp/why.2" "$tmp/why"
    elif grep -q "$(printf '\t')measured$(printf '\t')" "$tmp/r.out"; then
      ok=0
      printf 'a measured row was printed for a mutated golden\n' > "$tmp/why"
    fi
  fi
fi
report

# bench.refuse.wrong-ticks (D-22): a scratch workload table whose ticks column is one above the final machine time of the golden of
# the cold workload is refused with the ticks-mismatch line naming both values, exit 15, and a refused row without any measurement.
n=$((n + 1))
tid=bench.refuse.wrong-ticks
executed=$((executed + 1))
ok=1
ticks=$(golden_ticks demo-lamp-tone-pass)
bad=$((ticks + 1))
printf 'bench-lamp-cold\tdemo-lamp-tone-pass\t1\t1\t0\t%s\tscratch table, wrong ticks\n' "$bad" > "$tmp/wt.tsv"
"$bench" --verify-only --root "$root" --content-dir "$content" --workloads "$tmp/wt.tsv" > "$tmp/r.out" 2> "$tmp/r.err"
rc=$?
if ! expect_refusal "$rc" "bench-refuse bench-lamp-cold ticks-mismatch table=$bad run=$ticks" bench-lamp-cold demo-lamp-tone-pass; then
  ok=0
fi
report

# args_refused <label> <argument>...: one tool run that must end with the usage or harness exit 14, one line on stderr that starts with
# `ivs-bench: ` and nothing at all on stdout (no format line, no header, no row). Prints a finding, or nothing.
args_refused() {
  _label=$1
  shift
  "$bench" "$@" > "$tmp/a.out" 2> "$tmp/a.err"
  _rc=$?
  if [ "$_rc" -ne 14 ]; then
    printf '%s: exit %s, want 14\n' "$_label" "$_rc"
  elif [ -s "$tmp/a.out" ]; then
    printf '%s: stdout is not empty\n' "$_label"
  elif [ "$(wc -l < "$tmp/a.err" | tr -d ' ')" -ne 1 ] || ! grep -q '^ivs-bench: ' "$tmp/a.err"; then
    printf '%s: stderr is not exactly one ivs-bench line\n' "$_label"
  fi
}

# bench.refuse.bad-args: an unknown option, an option without a value, an option given twice, a non-numeric value in the table, an
# unknown workload id, a workload whose scenario file is missing, a workload whose golden is missing and a missing --content-dir.
n=$((n + 1))
tid=bench.refuse.bad-args
executed=$((executed + 1))
ok=1
mkdir "$tmp/emptyg"
printf 'x1\tbench-lamp-idle\tthree\t15\t0\t7168352\tnon-numeric warmup\n' > "$tmp/num.tsv"
printf 'x2\tno-such-scenario\t1\t1\t0\t1\tmissing scenario\n' > "$tmp/scn.tsv"
{
  args_refused unknown-option --bogus
  args_refused no-value --root "$root" --content-dir "$content" --workload
  args_refused given-twice --root "$root" --content-dir "$content" --content-dir "$content"
  args_refused non-numeric --root "$root" --content-dir "$content" --workloads "$tmp/num.tsv"
  args_refused unknown-workload --root "$root" --content-dir "$content" --workload no-such-workload
  args_refused missing-scenario --root "$root" --content-dir "$content" --workloads "$tmp/scn.tsv" --workload x2
  args_refused missing-golden --root "$root" --content-dir "$content" --golden-dir "$tmp/emptyg" --workload bench-lamp-idle
  args_refused missing-content-dir --root "$root" --verify-only
} > "$tmp/why"
if [ -s "$tmp/why" ]; then ok=0; fi
report

# bench.tsv.format (D-24): one run of the tool on a scratch two-row table of the cold workload (the cheapest, rows in the order of the
# table): the first line is `bench-format 1` and appears once, the header equals the column list in order, the three groups are
# contiguous (the static group at columns 8 to 13, the throughput columns thr_cycles_per_s_wall to ns_per_guest_cycle at 14 to 18, the
# latency columns lat_first_ms to lat_max_ms at 19 to 21, then ci_time last), every row has the same number of tab-separated fields as
# the header, the tier field is repeatability-only and the ci_time field is the pointer text, and each row is judged by the rules
# of bench.verify.<workload>.
n=$((n + 1))
tid=bench.tsv.format
executed=$((executed + 1))
ok=1
ticks=$(golden_ticks demo-lamp-tone-pass)
{
  printf 'fmt-a\tdemo-lamp-tone-pass\t1\t1\t0\t%s\tscratch row a\n' "$ticks"
  printf 'fmt-b\tdemo-lamp-tone-pass\t2\t3\t0\t%s\tscratch row b\n' "$ticks"
} > "$tmp/fmt.tsv"
"$bench" --verify-only --root "$root" --content-dir "$content" --workloads "$tmp/fmt.tsv" > "$tmp/f.out" 2> "$tmp/f.err"
rc=$?
if [ "$rc" -ne 0 ] || [ -s "$tmp/f.err" ]; then
  ok=0
  { printf 'ivs-bench --verify-only exit %s, want 0 and an empty stderr\n' "$rc"; cat "$tmp/f.err"; } > "$tmp/why"
else
  : > "$tmp/why"
  sed -n 2p "$tmp/f.out" | awk -F '\t' '
    { for (i = 1; i <= NF; i++) pos[$i] = i }
    END {
      if (pos["ncpu"] != 8 || pos["frames_per_iter"] != 13) print "the static group is not at columns 8 to 13"
      if (pos["thr_cycles_per_s_wall"] != 14 || pos["ns_per_guest_cycle"] != 18) print "the throughput group is not at columns 14 to 18"
      if (pos["lat_first_ms"] != 19 || pos["lat_max_ms"] != 21) print "the latency group is not at columns 19 to 21"
      if (pos["ci_time"] != NF || NF != 22) print "ci_time is not the last of 22 columns"
    }' >> "$tmp/why"
  if [ "$(grep -c '^bench-format 1$' "$tmp/f.out")" -ne 1 ]; then
    printf 'the format line does not appear exactly once\n' >> "$tmp/why"
  fi
  if [ "$(wc -l < "$tmp/f.out" | tr -d ' ')" -ne 4 ]; then
    printf 'the output is not 4 lines: the format line, the header and two rows\n' >> "$tmp/why"
  fi
  { sed -n 1,2p "$tmp/f.out"; sed -n 3p "$tmp/f.out"; } > "$tmp/f.row"
  tsv_check "$tmp/f.row" 1 verified fmt-a demo-lamp-tone-pass >> "$tmp/why"
  { sed -n 1,2p "$tmp/f.out"; sed -n 4p "$tmp/f.out"; } > "$tmp/f.row"
  tsv_check "$tmp/f.row" 1 verified fmt-b demo-lamp-tone-pass >> "$tmp/why"
  if [ -s "$tmp/why" ]; then ok=0; fi
fi
report

# synthetic_row <column> <value>: a refused row of 22 fields (the first seven filled, the pointer last) whose column <column> holds
# <value> and every other measurement column is empty.
synthetic_row() {
  awk -v col="$1" -v v="$2" 'BEGIN {
    split("bench-lamp-idle refused bench-lamp-idle repeatability-only build-x 1 off", h, " ")
    for (i = 1; i <= 22; i++) {
      x = (i <= 7) ? h[i] : ""
      if (i == col) x = v
      if (i == 22) x = "see-ci-runs.tsv"
      printf "%s%s", x, (i < 22 ? "\t" : "\n")
    }
  }'
}

# bench.judge.refuse-has-no-timing (D-25): the self-control of no_measurement, the checker that asserts the absence of a measurement
# in a refused row. It is fed synthetic refused rows: one with a value in thr_cycles_per_s_cpu (column 15) and one with a value in
# ns_per_guest_cycle (column 18) must each be rejected, with the finding naming that column and nothing else, and a clean row must be
# accepted. The judge passes only when the checker can fail.
n=$((n + 1))
tid=bench.judge.refuse-has-no-timing
executed=$((executed + 1))
ok=1
: > "$tmp/why"
synthetic_row 15 60366764 > "$tmp/s15.row"
synthetic_row 18 16.60 > "$tmp/s18.row"
synthetic_row 15 '' > "$tmp/clean.row"
no_measurement "$tmp/clean.row" > "$tmp/j.clean"
no_measurement "$tmp/s15.row" > "$tmp/j.s15"
no_measurement "$tmp/s18.row" > "$tmp/j.s18"
if [ -s "$tmp/j.clean" ]; then
  printf 'judge-rejects-clean: the checker rejected a row without any measurement:\n' >> "$tmp/why"
  cat "$tmp/j.clean" >> "$tmp/why"
fi
if [ "$(cat "$tmp/j.s15")" != "thr_cycles_per_s_cpu holds a value: 60366764" ]; then
  printf 'judge-vacuous: the value in thr_cycles_per_s_cpu was not reported alone: %s\n' "$(cat "$tmp/j.s15")" >> "$tmp/why"
fi
if [ "$(cat "$tmp/j.s18")" != "ns_per_guest_cycle holds a value: 16.60" ]; then
  printf 'judge-vacuous: the value in ns_per_guest_cycle was not reported alone: %s\n' "$(cat "$tmp/j.s18")" >> "$tmp/why"
fi
if [ -s "$tmp/why" ]; then ok=0; fi
report

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

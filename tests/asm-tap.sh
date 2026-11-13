#!/bin/sh
# SPDX-License-Identifier: MIT
# Suite asm (D-16 to D-19, plan 07-03): the project mini-assembler ivs_asm (tools/asm) against
# the four hand-assembled listings. This plan starts the third outside check of the encoding table
# (D-18): for each twin of the table at the end of the case list below, the source under
# tests/asm/twins/ is assembled, and the words ivs_pack reads from the generated listing
# (`ivs_pack --dump-words`) must equal, byte for byte, the words ivs_pack reads from the committed
# hand listing. A twin is test data reproducing a hand listing, not a fixture: it has no manifest and
# no image is built from it. The expected words never come from ivs_asm; they come from a listing
# typed by hand from the manual and checked twice before this suite existed. The hand listings are
# untouched (D-19).
# Pairs (plan 07-04 Task 2, the second outside check): tests/asm/pairs.txt, a flat copy of hand-derived
# asm and word pairs of the Phase 2 vectors, is assembled by one `ivs_asm --pairs` process; asm.pairs.<class>
# passes when every pair of that class printed ok and none printed bad. No JSON is read here.
# Ids: asm.twin-<id> per twin (boot-stub 22 words, diag-cpu-1 56, diag-ctrl-psg-1 89, diag-stic-1 133 with its
# data table as DECLE words, which is 300 words in all); asm.repeat: one twin assembled
# twice into two files is byte-identical, the determinism contract of the tool; asm.judge.mutated-twin is the
# self-control of the words comparison: the comparison must fail on a twin with one register field
# changed, and its diff must show exactly that word (judge-vacuous, judge-wrong-reason otherwise);
# asm.judge.mutated-pair does the same for the pairs comparison (one expected word of the corpus changed:
# exactly that pair must print bad and the exit status must be 1).
# An id <suite>.judge.<slug> is also an important-gates mutant (tools/check/steps/important-gates.sh).
# The sources are assembled with the working directory at the source root and relative paths, because
# the listing prints the source path it was given (no absolute path enters any output).
# Output: the harness TAP shape (header with IVS_TEST_NONCE or none, plan, one result per case,
# trailer) on stdout and, when IVS_TEST_OUT_DIR is set, in <dir>/asm.txt.
# Usage: sh asm-tap.sh <source root> <ivs_asm> <ivs_pack>   Exit: 0 pass, 1 fail, 77 every case skipped.
# Env: IVS_TEST_NONCE, IVS_TEST_OUT_DIR.
LC_ALL=C; export LC_ALL
[ "$#" -eq 3 ] || { printf 'usage: sh asm-tap.sh <source root> <ivs_asm> <ivs_pack>\n' >&2; exit 1; }
root=$1
asm=$2
pack=$3
case $asm in /*) ;; *) asm=$PWD/$asm ;; esac
case $pack in /*) ;; *) pack=$PWD/$pack ;; esac
[ -x "$asm" ] || { printf 'asm-tap: %s is not executable\n' "$asm" >&2; exit 1; }
[ -x "$pack" ] || { printf 'asm-tap: %s is not executable\n' "$pack" >&2; exit 1; }
tmp=$(mktemp -d "${TMPDIR:-/tmp}/asm-tap.XXXXXX") || exit 1
trap 'rm -rf "$tmp"' EXIT
trap 'exit 1' HUP INT TERM
cd "$root" || exit 1

suite=asm
nonce=${IVS_TEST_NONCE:-none}

# <id> <twin source> <hand listing>, one per line.
twins='boot-stub tests/asm/twins/boot-stub.asm fixtures/FX-boot-stub/boot-stub.listing.txt
diag-cpu-1 tests/asm/twins/diag-cpu-1.asm fixtures/FX-diag-cpu-1/diag-cpu-1.listing.txt
diag-ctrl-psg-1 tests/asm/twins/diag-ctrl-psg-1.asm fixtures/FX-diag-ctrl-psg-1/diag-ctrl-psg-1.listing.txt
diag-stic-1 tests/asm/twins/diag-stic-1.asm fixtures/FX-diag-stic-1/diag-stic-1.listing.txt'

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
# 0 when the non-empty word dump $1 (the twin) equals the word dump $2 (the hand listing) byte for
# byte; otherwise 1 with the finding in $tmp/why. The judge runs the same function on a mutated twin.
dumps_agree() {
  if [ ! -s "$1" ] || [ ! -s "$2" ]; then
    printf 'empty word dump\n' > "$tmp/why"
    return 1
  fi
  if ! cmp -s "$1" "$2"; then
    { printf 'word dumps differ (< twin, > hand listing)\n'; diff "$1" "$2"; } > "$tmp/why"
    return 1
  fi
  return 0
}
# Assembles source $1 (a path relative to the current directory) to listing $2 and dumps its words
# to $3 with ivs_pack; 0 on success, otherwise 1 with the finding in $tmp/why.
twin_words() {
  if ! "$asm" "$1" "$2" 2> "$tmp/asm.err"; then
    { printf 'ivs_asm failed on %s\n' "$1"; cat "$tmp/asm.err"; } > "$tmp/why"
    return 1
  fi
  if ! "$pack" --dump-words "$2" > "$3" 2> "$tmp/pack.err"; then
    { printf 'ivs_pack failed on the listing of %s\n' "$1"; cat "$tmp/pack.err"; } > "$tmp/why"
    return 1
  fi
  return 0
}

first_twin=
first_id=
printf '%s\n' "$twins" > "$tmp/twins"
while read -r id twin hand; do
  [ -n "$id" ] || continue
  n=$((n + 1))
  tid="asm.twin-$id"
  executed=$((executed + 1))
  ok=1
  if ! twin_words "$twin" "$tmp/$id.listing" "$tmp/$id.twin"; then
    ok=0
  elif ! "$pack" --dump-words "$hand" > "$tmp/$id.hand" 2> "$tmp/pack.err"; then
    ok=0
    { printf 'ivs_pack failed on %s\n' "$hand"; cat "$tmp/pack.err"; } > "$tmp/why"
  elif ! dumps_agree "$tmp/$id.twin" "$tmp/$id.hand"; then
    ok=0
  fi
  report
  if [ -z "$first_twin" ] && [ "$ok" -eq 1 ]; then
    first_twin=$twin
    first_id=$id
  fi
done < "$tmp/twins"

# asm.repeat (determinism, plan 07-03 Task 2): the diag-cpu-1 twin assembled twice, in this preset's
# ivs_asm, into two files that are byte-identical and not empty (a source of 56 words, one label and one
# backward branch); the listing holds no time, host or path byte beyond the relative source path given.
n=$((n + 1))
tid=asm.repeat
executed=$((executed + 1))
ok=1
if ! "$asm" tests/asm/twins/diag-cpu-1.asm "$tmp/rep1.listing" 2> "$tmp/why" \
    || ! "$asm" tests/asm/twins/diag-cpu-1.asm "$tmp/rep2.listing" 2>> "$tmp/why"; then
  ok=0
elif [ ! -s "$tmp/rep1.listing" ] || ! cmp -s "$tmp/rep1.listing" "$tmp/rep2.listing"; then
  ok=0
  { printf 'two assemblies of the same twin differ or are empty:\n'; diff "$tmp/rep1.listing" "$tmp/rep2.listing"; } > "$tmp/why"
fi
report

# asm.judge.mutated-twin: the comparison of asm.twin-boot-stub run on a copy of the boot stub twin
# whose reset code loads R5 instead of R6 (MVII #STACK, R6 is 02BE at 1011, the same line with R5 is
# 02BD). It passes only when the comparison fails and its diff shows that one word and nothing else.
n=$((n + 1))
tid=asm.judge.mutated-twin
executed=$((executed + 1))
ok=1
if [ "$first_id" != boot-stub ]; then
  ok=0
  printf 'asm.twin-boot-stub did not pass, so there is nothing to judge\n' > "$tmp/why"
else
  sed 's/#STACK, R6/#STACK, R5/' "$first_twin" > "$tmp/mut.asm"
  if cmp -s "$first_twin" "$tmp/mut.asm"; then
    ok=0
    printf 'judge-vacuous: the mutation did not change the twin source\n' > "$tmp/why"
  elif ! (cd "$tmp" && twin_words mut.asm mut.listing mut.twin); then
    ok=0
  elif dumps_agree "$tmp/mut.twin" "$tmp/boot-stub.hand"; then
    ok=0
    printf 'judge-vacuous: the comparison accepted the twin with R6 changed to R5\n' > "$tmp/why"
  elif [ "$(grep -c '^[<>] ' "$tmp/why")" -ne 2 ] || ! grep -q -x -F -e '< 1011 02BD' "$tmp/why" \
      || ! grep -q -x -F -e '> 1011 02BE' "$tmp/why"; then
    ok=0
    { printf 'judge-wrong-reason: the comparison failed, but not on 1011 alone:\n'; cat "$tmp/why"; } > "$tmp/why.2"
    mv "$tmp/why.2" "$tmp/why"
  fi
fi
report

# asm.pairs.<class> (plan 07-04 Task 2): one process over the whole corpus. The run is valid when it exits 0 or
# 1, prints exactly one ok or bad line per corpus line and every corpus class is registered below; an invalid
# run fails every class case with the reason. A class passes when all of its corpus lines printed ok.
pairs=tests/asm/pairs.txt
classes='move alu imm shift branch jump stack sdbd control'
grep -v '^#' "$pairs" | awk 'NF' > "$tmp/pairs.lines"
"$asm" --pairs "$pairs" > "$tmp/pairs.out" 2> "$tmp/pairs.err"
pairs_rc=$?
pairs_why=
if [ "$pairs_rc" -gt 1 ]; then
  pairs_why="ivs_asm --pairs exited $pairs_rc"
elif [ "$(wc -l < "$tmp/pairs.out" | tr -d ' ')" != "$(wc -l < "$tmp/pairs.lines" | tr -d ' ')" ]; then
  pairs_why="one result line per corpus line expected: $(wc -l < "$tmp/pairs.out" | tr -d ' ') results for $(wc -l < "$tmp/pairs.lines" | tr -d ' ') pairs"
else
  for c in $(awk '{print $1}' "$tmp/pairs.lines" | sort -u); do
    case " $classes " in *" $c "*) ;; *) pairs_why="corpus class $c is not registered in asm-tap.sh" ;; esac
  done
fi
for c in $classes; do
  n=$((n + 1))
  tid="asm.pairs.$c"
  executed=$((executed + 1))
  ok=1
  want=$(awk -v c="$c" '$1 == c' "$tmp/pairs.lines" | wc -l | tr -d ' ')
  got=$(awk -v c="$c" '$1 == "ok" && $2 == c' "$tmp/pairs.out" | wc -l | tr -d ' ')
  if [ -n "$pairs_why" ]; then
    ok=0
    printf '%s\n' "$pairs_why" > "$tmp/why"
  elif [ "$want" -eq 0 ] || [ "$got" -ne "$want" ]; then
    ok=0
    { printf 'class %s: %s pairs in the corpus, %s ok\n' "$c" "$want" "$got"; awk -v c="$c" '$1 == "bad" && $2 == c' "$tmp/pairs.out"; head -3 "$tmp/pairs.err"; } > "$tmp/why"
  fi
  report
done

# asm.judge.mutated-pair: the first expected word of the first corpus line is changed in a copy; the run must
# exit 1 and print exactly one bad line, for that corpus line, with every other pair still ok.
n=$((n + 1))
tid=asm.judge.mutated-pair
executed=$((executed + 1))
ok=1
first=$(grep -n -v '^#' "$pairs" | awk -F: 'NF > 1 && $2 != "" { print $1; exit }')
awk -v first="$first" 'NR == first { $3 = (substr($3, 1, 1) == "0" ? "1" : "0") substr($3, 2) } { print }' "$pairs" > "$tmp/mut-pairs.txt"
if cmp -s "$pairs" "$tmp/mut-pairs.txt"; then
  ok=0
  printf 'judge-vacuous: the mutation did not change the corpus\n' > "$tmp/why"
else
  (cd "$tmp" && "$asm" --pairs mut-pairs.txt) > "$tmp/mut.out" 2> "$tmp/mut.err"
  mut_rc=$?
  if [ "$mut_rc" -ne 1 ]; then
    ok=0
    printf 'judge-vacuous: the mutated corpus exited %s, not 1\n' "$mut_rc" > "$tmp/why"
  elif [ "$(grep -c '^bad ' "$tmp/mut.out")" -ne 1 ] || ! grep -q "^bad [a-z]* $first " "$tmp/mut.out"; then
    ok=0
    { printf 'judge-wrong-reason: not exactly one bad line for corpus line %s:\n' "$first"; grep -v '^ok ' "$tmp/mut.out"; } > "$tmp/why"
  fi
fi
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

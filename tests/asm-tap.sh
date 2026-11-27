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
# Refusals (plan 07-04 Task 3, T5): data/expected/asm-tokens.ids pins the tokens of ivs_asm. asm.neg.<token> runs
# the one source of that token (tests/asm/neg/<token>.asm; asm-syntax, a CR line end, and asm-limit, a line
# above 255 bytes, are built here at run time because a tracked file may hold no CR) and requires exit 1, no
# stdout, exactly one stderr line `ivs_asm: <token>: ...` and no output file. asm.neg-coverage fails when the
# pinned tokens differ from the tokens tools/asm/asm.c can print or a file under tests/asm/neg names no pinned
# token. asm.var.<slug> are further sources for tokens with several spellings or orders; asm.boundary.branch-03ff
# is the accepted half of the displacement boundary (the refused half, 0400, is asm.neg.asm-branch-range).
# Ids: asm.twin-<id> per twin (boot-stub 22 words, diag-cpu-1 56, diag-ctrl-psg-1 89, diag-stic-1 133 with its
# data table as DECLE words, which is 300 words in all); asm.repeat: one twin assembled
# twice into two files is byte-identical, the determinism contract of the tool; asm.judge.mutated-twin is the
# self-control of the words comparison: the comparison must fail on a twin with one register field
# changed, and its diff must show exactly that word (judge-vacuous, judge-wrong-reason otherwise);
# asm.judge.mutated-pair does the same for the pairs comparison (one expected word of the corpus changed:
# exactly that pair must print bad and the exit status must be 1).
# asm.dis.* and asm.judge.dis-constant-mnemonic (plan 18-03, TOOL-02): the inspection instrument's
# disassembly limb. ivs-inspect, the fourth argument, emits a source from the project's own boot-stub
# listing; ivs_asm reassembles it; and the words ivs_pack --dump-words reads from both listings must be
# byte-identical - the same comparison, dumps_agree, asm.twin-boot-stub makes, because the tool emits a
# source and no listing and the word dump is the observation. Two-word forms, an immediate and a distinct
# mnemonic count are asserted so the round trip cannot be vacuous, an unknown row is refused by name, and
# the constant-mnemonic stand-in shows the comparison failing for the reason the criterion names.
# asm.regen-<listing id> (plan 07-05 Task 1, D-19): per fixtures/*/*.asm in the tree, the source is assembled again and
# the result must equal the committed listing byte for byte, and the words ivs_pack reads from both must agree.
# An id <suite>.judge.<slug> is also an important-gates mutant (tools/check/steps/important-gates.sh).
# The sources are assembled with the working directory at the source root and relative paths, because
# the listing prints the source path it was given (no absolute path enters any output).
# Output: the harness TAP shape (header with IVS_TEST_NONCE or none, plan, one result per case,
# trailer) on stdout and, when IVS_TEST_OUT_DIR is set, in <dir>/asm.txt.
# Usage: sh asm-tap.sh <source root> <ivs_asm> <ivs_pack> [<ivs_inspect>]
#   Exit: 0 pass, 1 fail, 77 every case skipped.
# Env: IVS_TEST_NONCE, IVS_TEST_OUT_DIR.
LC_ALL=C; export LC_ALL
{ [ "$#" -ge 3 ] && [ "$#" -le 4 ]; } \
  || { printf 'usage: sh asm-tap.sh <source root> <ivs_asm> <ivs_pack> [<ivs_inspect>]\n' >&2; exit 1; }
root=$1
asm=$2
pack=$3
# The fourth argument (plan 18-03) is the build-only inspection instrument, used by the disassembly limb's
# cases. A missing one is a finding in each of those cases, never a skip: a skip would move tests-skipped.
inspect=${4:-}
case $asm in /*) ;; *) asm=$PWD/$asm ;; esac
case $pack in /*) ;; *) pack=$PWD/$pack ;; esac
case $inspect in ''|/*) ;; *) inspect=$PWD/$inspect ;; esac
[ -x "$asm" ] || { printf 'asm-tap: %s is not executable\n' "$asm" >&2; exit 1; }
[ -x "$pack" ] || { printf 'asm-tap: %s is not executable\n' "$pack" >&2; exit 1; }
if [ -n "$inspect" ] && [ ! -x "$inspect" ]; then
  printf 'asm-tap: %s is not executable\n' "$inspect" >&2
  exit 1
fi
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

# ---- The disassembly limb (plan 18-03, TOOL-02) ----

# asm.dis.roundtrip-boot-stub: `ivs-inspect dis` emits a source from the project's own boot-stub listing,
# `ivs_asm` reassembles it, and `ivs_pack --dump-words` shows its words are the input listing's. The
# comparison IS the two word dumps, through the same dumps_agree asm.twin-boot-stub uses, and never a
# comparison of listing text: the tool emits a source and no listing, so the word dump is the observation
# the criterion names. Three limbs - an unknown row is refused before any output exists, the round trip
# agrees, and the committed twin's words agree with the emitted source's too, so the three oracles agree.
# Every ivs_asm call here passes a source path relative to the directory the call runs in, because
# as_path_ok (tools/asm/asm.c) refuses a leading `/` and a usage error would then be the verdict instead
# of the round trip.
n=$((n + 1))
tid=asm.dis.roundtrip-boot-stub
executed=$((executed + 1))
ok=1
if [ -z "$inspect" ]; then
  ok=0
  printf 'the fourth argument (ivs_inspect) is missing\n' > "$tmp/why"
else
  rm -f "$tmp/badrow.asm"
  sed '/^1000 /s/^\(1000 [0-9A-F]\{4\} [01 ]*\)[A-Z@][A-Z@]*/\1NOSUCHROW/' \
    fixtures/FX-boot-stub/boot-stub.listing.txt > "$tmp/badrow.listing"
  if cmp -s "$tmp/badrow.listing" fixtures/FX-boot-stub/boot-stub.listing.txt; then
    ok=0
    printf 'judge-vacuous: the unknown-row listing is identical to the real one\n' > "$tmp/why"
  else
    "$inspect" dis "$tmp/badrow.listing" --out "$tmp/badrow.asm" > "$tmp/br.out" 2> "$tmp/br.err"
    br_rc=$?
    if [ "$br_rc" -ne 1 ]; then
      ok=0
      printf 'the unknown row exited %s, expected 1\n' "$br_rc" > "$tmp/why"
    elif [ -s "$tmp/br.out" ]; then
      ok=0
      printf 'the unknown-row refusal wrote on stdout\n' > "$tmp/why"
    elif [ "$(wc -l < "$tmp/br.err" | tr -d ' ')" -ne 1 ] \
        || ! grep -q '^ivs-inspect: inspect-dis-row: ' "$tmp/br.err"; then
      ok=0
      { printf 'expected exactly one stderr line ivs-inspect: inspect-dis-row: ..., got:\n'; cat "$tmp/br.err"; } > "$tmp/why"
    elif ! grep -q 'NOSUCHROW' "$tmp/br.err" || ! grep -q '1000' "$tmp/br.err"; then
      ok=0
      { printf 'the refusal names neither the row nor the address:\n'; cat "$tmp/br.err"; } > "$tmp/why"
    elif [ -e "$tmp/badrow.asm" ]; then
      ok=0
      printf 'the unknown-row refusal left an output file\n' > "$tmp/why"
    fi
  fi
  if [ "$ok" -eq 1 ]; then
    rm -f "$tmp/dis.asm"
    if ! "$inspect" dis fixtures/FX-boot-stub/boot-stub.listing.txt --out "$tmp/dis.asm" 2> "$tmp/why"; then
      ok=0
    elif [ ! -s "$tmp/dis.asm" ]; then
      ok=0
      printf 'the disassembler wrote no source\n' > "$tmp/why"
    elif ! grep -q -x -F -e '# window: 1000-101F' "$tmp/dis.asm"; then
      ok=0
      printf 'the emitted source lacks the header line # window: 1000-101F\n' > "$tmp/why"
    elif ! (cd "$tmp" && twin_words dis.asm dis.listing dis.words); then
      ok=0
    elif ! "$pack" --dump-words fixtures/FX-boot-stub/boot-stub.listing.txt > "$tmp/dis.hand" 2> "$tmp/why"; then
      ok=0
    elif ! dumps_agree "$tmp/dis.words" "$tmp/dis.hand"; then
      ok=0
    elif ! twin_words tests/asm/twins/boot-stub.asm "$tmp/dis.twin.listing" "$tmp/dis.twin"; then
      ok=0
    elif ! dumps_agree "$tmp/dis.twin" "$tmp/dis.words"; then
      ok=0
    fi
  fi
fi
report

# asm.dis.two-word-form: the word dump holds at least two more lines than the emitted source holds
# instruction lines, so a corpus of one-word instructions cannot satisfy the round trip. An instruction
# line is an indented upper-case mnemonic with or without an operand (HLT and DIS are instruction lines)
# and an ORG directive is not one; the emitter writes no labels, so the count is exact on its own output.
# The margin of two means a miscount of one on either side cannot flip the verdict, and the corpus's six
# operand rows make the margin a real observation rather than a coincidence.
n=$((n + 1))
tid=asm.dis.two-word-form
executed=$((executed + 1))
ok=1
if [ -z "$inspect" ]; then
  ok=0
  printf 'the fourth argument (ivs_inspect) is missing\n' > "$tmp/why"
else
  rm -f "$tmp/twf.asm"
  if ! "$inspect" dis fixtures/FX-boot-stub/boot-stub.listing.txt --out "$tmp/twf.asm" 2> "$tmp/why"; then
    ok=0
  elif ! grep -E '^ +[A-Z][A-Z@]*( |$)' "$tmp/twf.asm" | grep -v -E '^ +ORG( |$)' > "$tmp/twf.instr"; then
    ok=0
    printf 'the emitted source holds no instruction line\n' > "$tmp/why"
  elif ! (cd "$tmp" && twin_words twf.asm twf.listing twf.words); then
    ok=0
  else
    twf_words=$(grep -c . "$tmp/twf.words")
    twf_instr=$(grep -c . "$tmp/twf.instr")
    if [ "$twf_instr" -eq 0 ] || [ "$twf_words" -lt $((twf_instr + 2)) ]; then
      ok=0
      printf 'the word dump holds %s words for %s instruction lines: no two-word form\n' \
        "$twf_words" "$twf_instr" > "$tmp/why"
    fi
  fi
fi
report

# asm.dis.immediate: the emitted source contains an immediate form, and the committed twin's source does
# too, so the guard is about the corpus rather than about one emitter's coincidence.
n=$((n + 1))
tid=asm.dis.immediate
executed=$((executed + 1))
ok=1
if [ -z "$inspect" ]; then
  ok=0
  printf 'the fourth argument (ivs_inspect) is missing\n' > "$tmp/why"
else
  rm -f "$tmp/imm.asm"
  if ! "$inspect" dis fixtures/FX-boot-stub/boot-stub.listing.txt --out "$tmp/imm.asm" 2> "$tmp/why"; then
    ok=0
  elif ! grep -q -E 'MVII +#' "$tmp/imm.asm"; then
    ok=0
    printf 'the emitted source holds no immediate form (MVII #...)\n' > "$tmp/why"
  elif ! grep -q -E 'MVII +#' tests/asm/twins/boot-stub.asm; then
    ok=0
    printf 'the committed twin holds no immediate form, so the corpus carries none\n' > "$tmp/why"
  fi
fi
report

# asm.dis.words-nonvacuous: the dump holds more than zero lines, the emitted source holds more than zero
# instruction lines, and the emitted source's distinct mnemonics number at least three. A DECLE-everything
# emitter round-trips perfectly and would pass every other limb, so this is the count that catches it, and
# the constant-mnemonic stand-in below is the same failure by name.
n=$((n + 1))
tid=asm.dis.words-nonvacuous
executed=$((executed + 1))
ok=1
if [ -z "$inspect" ]; then
  ok=0
  printf 'the fourth argument (ivs_inspect) is missing\n' > "$tmp/why"
else
  rm -f "$tmp/nv.asm"
  if ! "$inspect" dis fixtures/FX-boot-stub/boot-stub.listing.txt --out "$tmp/nv.asm" 2> "$tmp/why"; then
    ok=0
  elif ! (cd "$tmp" && twin_words nv.asm nv.listing nv.words); then
    ok=0
  else
    grep -E '^ +[A-Z][A-Z@]*( |$)' "$tmp/nv.asm" | grep -v -E '^ +ORG( |$)' > "$tmp/nv.instr"
    sed -n 's/^ *\([A-Z][A-Z@]*\) .*/\1/p' "$tmp/nv.asm" | grep -v -x -F -e 'ORG' | sort -u > "$tmp/nv.mnem"
    nv_words=$(grep -c . "$tmp/nv.words")
    nv_instr=$(grep -c . "$tmp/nv.instr")
    nv_mnem=$(grep -c . "$tmp/nv.mnem")
    if [ "$nv_words" -eq 0 ] || [ "$nv_instr" -eq 0 ]; then
      ok=0
      printf 'the word dump holds %s lines and the source %s instruction lines\n' \
        "$nv_words" "$nv_instr" > "$tmp/why"
    elif [ "$nv_mnem" -lt 3 ]; then
      ok=0
      printf 'the emitted source uses %s distinct mnemonics, fewer than 3: %s\n' \
        "$nv_mnem" "$(tr '\n' ' ' < "$tmp/nv.mnem")" > "$tmp/why"
    fi
  fi
fi
report

# asm.dis.deterministic: two `dis` runs on the same input into two paths are byte-identical and not empty
# (the asm.repeat shape), so a difference in the round trip above cannot come from run-to-run variation.
# Asserted once, here, and not a second time inside the round-trip id.
n=$((n + 1))
tid=asm.dis.deterministic
executed=$((executed + 1))
ok=1
if [ -z "$inspect" ]; then
  ok=0
  printf 'the fourth argument (ivs_inspect) is missing\n' > "$tmp/why"
else
  rm -f "$tmp/det1.asm" "$tmp/det2.asm"
  if ! "$inspect" dis fixtures/FX-boot-stub/boot-stub.listing.txt --out "$tmp/det1.asm" 2> "$tmp/why" \
      || ! "$inspect" dis fixtures/FX-boot-stub/boot-stub.listing.txt --out "$tmp/det2.asm" 2>> "$tmp/why"; then
    ok=0
  elif [ ! -s "$tmp/det1.asm" ] || ! cmp -s "$tmp/det1.asm" "$tmp/det2.asm"; then
    ok=0
    { printf 'two dis runs of the same listing differ or are empty:\n'; diff "$tmp/det1.asm" "$tmp/det2.asm"; } > "$tmp/why"
  fi
fi
report

# asm.judge.dis-constant-mnemonic: the round trip's own stand-in, in the frozen page's words - an emitter
# that writes DECLE for every word round-trips perfectly and disassembles nothing. Every instruction line
# of the emitted source is replaced by the whole line `DECLE $0000`, the ORG directives left alone, which
# reassembles; the word comparison must then fail with a diff. The replacement is a whole line and not a
# mnemonic swap: keeping the operand text produces `DECLE` with no value (asm-operand-count),
# `DECLE #$0001, R0` (asm-operand-kind) and `DECLE $1010` (asm-decle-range), none of which assembles, and
# an assembler refusal is not the comparison failing - the case reports judge-wrong-reason for that. If the
# comparison accepts the stand-in it reports judge-vacuous. Its declared finding is exactly one: the word
# difference.
n=$((n + 1))
tid=asm.judge.dis-constant-mnemonic
executed=$((executed + 1))
ok=1
if [ -z "$inspect" ]; then
  ok=0
  printf 'the fourth argument (ivs_inspect) is missing\n' > "$tmp/why"
else
  rm -f "$tmp/const.asm" "$tmp/const.mut.asm"
  if ! "$inspect" dis fixtures/FX-boot-stub/boot-stub.listing.txt --out "$tmp/const.asm" 2> "$tmp/why"; then
    ok=0
  elif ! sed -E '/^ +ORG /!s/^ +[A-Z][A-Z@]*( .*)?$/        DECLE $0000/' "$tmp/const.asm" \
      > "$tmp/const.mut.asm"; then
    ok=0
    printf 'the constant-mnemonic stand-in could not be built\n' > "$tmp/why"
  elif cmp -s "$tmp/const.asm" "$tmp/const.mut.asm"; then
    ok=0
    printf 'judge-vacuous: the mutation did not change the emitted source\n' > "$tmp/why"
  elif ! (cd "$tmp" && twin_words const.mut.asm const.listing const.words); then
    # twin_words reports the assembler's refusal in $tmp/why; a refusal is not the comparison failing.
    ok=0
    { printf 'judge-wrong-reason: the constant-mnemonic stand-in does not assemble:\n'; cat "$tmp/why"; } > "$tmp/why.2"
    mv "$tmp/why.2" "$tmp/why"
  elif ! "$pack" --dump-words fixtures/FX-boot-stub/boot-stub.listing.txt > "$tmp/const.hand" 2> "$tmp/why"; then
    ok=0
  elif dumps_agree "$tmp/const.words" "$tmp/const.hand"; then
    ok=0
    printf 'judge-vacuous: the word comparison accepted the constant-mnemonic stand-in\n' > "$tmp/why"
  elif ! grep -q '^word dumps differ' "$tmp/why"; then
    ok=0
    { printf 'judge-wrong-reason: the comparison failed, but not on the words:\n'; cat "$tmp/why"; } > "$tmp/why.2"
    mv "$tmp/why.2" "$tmp/why"
  fi
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

# asm.regen-<listing id> (plan 07-05 Task 1, D-19): one case per fixtures/*/*.asm in the tree. The source is the
# truth and its listing is generated and committed, so the listing must be exactly what the tool writes now: the
# source is assembled with the working directory at the source root and its relative path (the listing prints the
# path it was given), the result must equal the committed listing byte for byte, and the words ivs_pack reads from
# both must agree. The listing id is the `# id:` header line of the source and the committed listing is the path
# data/expected/listings.ids gives for that id, so a source without a registered listing, or a listing nobody
# regenerates, fails here. A source added without its asm.regen-<id> line in data/expected/tests/asm.ids is
# tests-unlisted, and a registered id with no source is tests-missing.
find fixtures -type f -name '*.asm' | sort > "$tmp/regen.srcs"
while read -r src; do
  [ -n "$src" ] || continue
  rid=$(sed -n 's/^# id: //p' "$src" | head -n 1)
  rlst=$(awk -v id="$rid" '!/^#/ && NF >= 3 && $1 == id { print $3; exit }' data/expected/listings.ids)
  n=$((n + 1))
  tid="asm.regen-$rid"
  executed=$((executed + 1))
  ok=1
  rm -f "$tmp/regen.listing" "$tmp/regen.listing.tmp"
  if [ -z "$rid" ]; then
    ok=0
    printf '%s has no # id: header line\n' "$src" > "$tmp/why"
  elif [ -z "$rlst" ] || [ ! -f "$rlst" ]; then
    ok=0
    printf 'data/expected/listings.ids names no committed listing for the id %s of %s\n' "$rid" "$src" > "$tmp/why"
  elif ! "$asm" "$src" "$tmp/regen.listing" 2> "$tmp/asm.err"; then
    ok=0
    { printf 'ivs_asm failed on %s\n' "$src"; cat "$tmp/asm.err"; } > "$tmp/why"
  elif ! cmp -s "$tmp/regen.listing" "$rlst"; then
    ok=0
    { printf 'the listing generated from %s differs from the committed %s (< generated, > committed)\n' "$src" "$rlst"
      diff "$tmp/regen.listing" "$rlst"; } > "$tmp/why"
  elif ! "$pack" --dump-words "$tmp/regen.listing" > "$tmp/regen.words" 2> "$tmp/pack.err" \
      || ! "$pack" --dump-words "$rlst" > "$tmp/regen.words.committed" 2> "$tmp/pack.err"; then
    ok=0
    { printf 'ivs_pack failed on a listing of %s\n' "$src"; cat "$tmp/pack.err"; } > "$tmp/why"
  elif ! dumps_agree "$tmp/regen.words" "$tmp/regen.words.committed"; then
    ok=0
  fi
  report
done < "$tmp/regen.srcs"

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

# ---- Refusals (plan 07-04 Task 3, T5) ----
grep -v '^#' data/expected/asm-tokens.ids | awk 'NF { print $1 }' > "$tmp/tokens"
mkdir -p "$tmp/gen"
neg_head='# id: neg\n# kind: cart\n# window: 5000-5FFF\n        ORG   $5000\n'
{ printf '%b' "$neg_head"; printf '        HLT\r\n'; } > "$tmp/gen/asm-syntax.asm"
{ printf '%b' "$neg_head"; awk 'BEGIN { printf "; "; for (i = 0; i < 300; i++) printf "x"; printf "\n" }'; } > "$tmp/gen/asm-limit.asm"
# 0 when ivs_asm run in directory $2 on the source $3 (a relative path) refuses it with exactly the token $1 on one
# stderr line, exit status 1, nothing on stdout and no output file; otherwise 1 with the finding in $tmp/why.
neg_run() {
  rm -f "$tmp/neg.listing" "$tmp/neg.listing.tmp"
  (cd "$2" && "$asm" "$3" "$tmp/neg.listing") > "$tmp/neg.out" 2> "$tmp/neg.err" < /dev/null
  neg_rc=$?
  if [ "$neg_rc" -ne 1 ]; then
    printf 'expected exit 1 with the token %s, got exit %s\n' "$1" "$neg_rc" > "$tmp/why"
  elif [ -s "$tmp/neg.out" ]; then
    printf 'a refusal wrote on stdout\n' > "$tmp/why"
  elif [ "$(wc -l < "$tmp/neg.err" | tr -d ' ')" -ne 1 ] || ! grep -q "^ivs_asm: $1: " "$tmp/neg.err"; then
    { printf 'expected exactly one stderr line ivs_asm: %s: ..., got:\n' "$1"; cat "$tmp/neg.err"; } > "$tmp/why"
  elif [ -e "$tmp/neg.listing" ] || [ -e "$tmp/neg.listing.tmp" ]; then
    printf 'a refusal left an output file\n' > "$tmp/why"
  else
    return 0
  fi
  return 1
}
while read -r tok; do
  [ -n "$tok" ] || continue
  n=$((n + 1))
  tid="asm.neg.$tok"
  executed=$((executed + 1))
  ok=1
  if [ -f "tests/asm/neg/$tok.asm" ]; then
    neg_run "$tok" "$PWD" "tests/asm/neg/$tok.asm" || ok=0
  elif [ -f "$tmp/gen/$tok.asm" ]; then
    neg_run "$tok" "$tmp/gen" "$tok.asm" || ok=0
  else
    ok=0
    printf 'no case for the pinned token %s\n' "$tok" > "$tmp/why"
  fi
  report
done < "$tmp/tokens"

# asm.neg-coverage: the pinned tokens are the tokens the source can print, once each, and every file of
# tests/asm/neg is the one case of a pinned token (never of a token that is built at run time).
n=$((n + 1))
tid=asm.neg-coverage
executed=$((executed + 1))
ok=1
: > "$tmp/why"
sort "$tmp/tokens" > "$tmp/tokens.sorted"
sort -u "$tmp/tokens" | cmp -s - "$tmp/tokens.sorted" || { ok=0; printf 'a token is pinned twice\n' >> "$tmp/why"; }
grep -v '^asm-[a-z-][a-z-]*$' "$tmp/tokens" | sed 's/^/malformed token: /' >> "$tmp/why"
grep -o '"asm-[a-z-]*"' tools/asm/asm.c | tr -d '"' | sort -u > "$tmp/src.tokens"
if ! cmp -s "$tmp/src.tokens" "$tmp/tokens.sorted"; then
  ok=0
  { printf 'tokens printed by tools/asm/asm.c (<) against pinned tokens (>)\n'; diff "$tmp/src.tokens" "$tmp/tokens.sorted"; } >> "$tmp/why"
fi
for f in tests/asm/neg/*; do
  b=${f##*/}
  t=${b%.asm}
  if [ "$t" = "$b" ] || ! grep -q -x -F -e "$t" "$tmp/tokens"; then
    ok=0
    printf '%s names no pinned token\n' "$f" >> "$tmp/why"
  elif [ -f "$tmp/gen/$b" ]; then
    ok=0
    printf '%s duplicates the case built at run time\n' "$f" >> "$tmp/why"
  fi
done
[ ! -s "$tmp/why" ] || ok=0
report

# asm.var.<slug>: more sources for tokens with several causes or orders, `slug|token|source lines` (the
# header and ORG of the negative sources are put in front; \n separates lines).
variants='spelling-pshr|asm-spelling|        MVO@  R1, R6
spelling-tstr|asm-spelling|        MOVR  R1, R1
spelling-jr|asm-spelling|        MOVR  R1, R7
spelling-clrr|asm-spelling|        XORR  R1, R1
spelling-immediate|asm-spelling|        MVI@  R7, R1
sdbd-at-end|asm-sdbd-target|        SDBD
sdbd-stack-mode|asm-sdbd-target|        SDBD\n        MVI@  R6, R1
sdbd-label|asm-sdbd-target|        SDBD\nlbl:    MVII  #$1234, R0
sdbd-label-alone|asm-sdbd-target|        SDBD\nlbl:\n        MVII  #$1234, R0'
printf '%s\n' "$variants" > "$tmp/variants"
while IFS='|' read -r slug tok body; do
  [ -n "$slug" ] || continue
  n=$((n + 1))
  tid="asm.var.$slug"
  executed=$((executed + 1))
  ok=1
  { printf '%b' "$neg_head"; printf '%b\n' "$body"; } > "$tmp/gen/var-$slug.asm"
  if ! grep -q -x -F -e "$tok" "$tmp/tokens"; then
    ok=0
    printf 'the variant names the unpinned token %s\n' "$tok" > "$tmp/why"
  else
    neg_run "$tok" "$tmp/gen" "var-$slug.asm" || ok=0
  fi
  report
done < "$tmp/variants"

# asm.boundary.branch-03ff: the displacement 03FF is accepted: the branch at 5000 to 5401 is the words 0200 and 03FF.
n=$((n + 1))
tid=asm.boundary.branch-03ff
executed=$((executed + 1))
ok=1
if ! "$asm" tests/asm/pos/branch-03ff.asm "$tmp/pos.listing" 2> "$tmp/why"; then
  ok=0
elif ! grep -q '^5000 0200 ' "$tmp/pos.listing" || ! grep -q '^5001 03FF ' "$tmp/pos.listing"; then
  ok=0
  printf 'the listing of the 03FF boundary source lacks the words 5000 0200 and 5001 03FF\n' > "$tmp/why"
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

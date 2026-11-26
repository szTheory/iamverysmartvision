#!/bin/sh
# SPDX-License-Identifier: MIT
# Suite pack (D-26, plan 04-15): two independent parses of every hand-assembled listing must
# agree word for word. For each line `<id> <kind> <path>` of data/expected/listings.ids the
# output of `ivs_pack --dump-words <path>` (the build-tree packer's decode, tools/pack) is
# compared byte for byte with `cmake -DROOT=<root> -DDUMP=<id> -P tools/check/listing.cmake`
# (the gate checker's own parse). Neither side is compared with the core: the core is not an
# oracle for its own input. Id per listing: pack.words-<id>; a difference or a failing parser
# is `not ok` with the diff or the error as `#` lines.
# For a listing of kind cart a second case pack.trailer-<id> (plan 04-16) builds the IVSC container
# with `ivs_pack --ivsc` and requires its last 32 bytes to equal `cmake -E sha256sum` of every byte
# before them (cut with POSIX dd, printed with od): the trailer is checked by an implementation
# other than the core SHA-256 that ivs_pack seals it with.
# Case pack.judge.patched-word (plan 04-24, D-46) is a self-control of the words comparison:
# it passes only when the comparison fails on a dump with one word patched by ivs_pack --patch.
# A last case pack.c-include.write-error (review IN-08): a failed --c-include write is the
# documented write error line, a nonzero exit and no output file.
# The flatwrap cases (plan 14-01, D-47, TOOL-07) exercise the wrap tool of tools/flatwrap, the second
# writer of the same container. pack.flatwrap.neg.<token> is minted by a loop over the eight tokens pinned in
# data/expected/flat-refusals.ids, so a token added to the pin without a case is a finding rather than a silent
# gap, and each case asserts the refusal and its boundary positive together: exit 1 with exactly one stderr line
# carrying the token, nothing on stdout and no output file, then exit 0 with one 64-hex line and a container of
# the exact expected size (and, where the pair is about a value, the value read back from the container).
# pack.judge.flatwrap-tokens is the coverage judge (the pin against the literals the source can print, and
# against the tokens the cases observed); pack.judge.flatwrap-digest-live is the digest judge (sha256sum, an
# implementation other than the core's, over the whole container and over the trailer, shown live by flipping
# one input byte). pack.flatwrap.word-order covers the tool's one option: lo-first swaps the two bytes of each
# word, hi-first is the default, and the two orders print different digests. pack.flatwrap.usage-word-order
# covers the other side of that parser: an unknown or missing --word-order value is a usage error (exit 2, the
# tool's own usage line, no token on it), not a refusal.
# Output: the harness TAP shape (header with IVS_TEST_NONCE or none, plan, one result per case,
# trailer) on stdout and, when IVS_TEST_OUT_DIR is set, in <dir>/pack.txt.
# Usage: sh pack-tap.sh <source root> <ivs_pack> <ivs_flatwrap>   Exit: 0 pass, 1 fail, 77 every case skipped.
# Env: CMAKE (set by ctest to this build's cmake; default cmake), IVS_TEST_NONCE,
# IVS_TEST_OUT_DIR.
LC_ALL=C; export LC_ALL
[ "$#" -eq 3 ] || { printf 'usage: sh pack-tap.sh <source root> <ivs_pack> <ivs_flatwrap>\n' >&2; exit 1; }
root=$1
pack=$2
flatwrap=$3
cmake=${CMAKE:-cmake}
inv="$root/data/expected/listings.ids"
[ -f "$inv" ] || { printf 'pack-tap: no %s\n' "$inv" >&2; exit 1; }
[ -x "$pack" ] || { printf 'pack-tap: %s is not executable\n' "$pack" >&2; exit 1; }
[ -x "$flatwrap" ] || { printf 'pack-tap: %s is not executable\n' "$flatwrap" >&2; exit 1; }
tokens="$root/data/expected/flat-refusals.ids"
[ -f "$tokens" ] || { printf 'pack-tap: no %s\n' "$tokens" >&2; exit 1; }
tmp=$(mktemp -d "${TMPDIR:-/tmp}/pack-tap.XXXXXX") || exit 1
trap 'rm -rf "$tmp"' EXIT
trap 'exit 1' HUP INT TERM

suite=pack
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
# The comparison of pack.words-<id>: 0 when dump $1 (ivs_pack) is non-empty and equals dump $2
# (listing.cmake) byte for byte; otherwise 1 with the finding in $tmp/why. pack.judge.patched-word
# runs the same function against a dump with one word patched.
dumps_agree() {
  if [ ! -s "$1" ]; then
    printf 'empty word dump\n' > "$tmp/why"
    return 1
  fi
  if ! cmp -s "$1" "$2"; then
    { printf 'word dumps differ (< ivs_pack, > listing.cmake)\n'; diff "$1" "$2"; } > "$tmp/why"
    return 1
  fi
  return 0
}
first_id=
first_path=
while read -r id kind path rest; do
  case $id in '' | '#'*) continue ;; esac
  n=$((n + 1))
  tid="pack.words-$id"
  executed=$((executed + 1))
  ok=1
  if ! "$pack" --dump-words "$root/$path" > "$tmp/pack.out" 2> "$tmp/pack.err"; then
    ok=0
    { printf 'ivs_pack failed\n'; cat "$tmp/pack.err"; } > "$tmp/why"
  elif ! "$cmake" -DROOT="$root" -DDUMP="$id" -P "$root/tools/check/listing.cmake" > "$tmp/cmake.out" 2> "$tmp/cmake.err"; then
    ok=0
    { printf 'listing.cmake failed\n'; cat "$tmp/cmake.out" "$tmp/cmake.err"; } > "$tmp/why"
  elif ! dumps_agree "$tmp/pack.out" "$tmp/cmake.out"; then
    ok=0
  fi
  report
  if [ -z "$first_id" ] && [ "$ok" -eq 1 ]; then
    first_id=$id
    first_path=$path
    cp "$tmp/cmake.out" "$tmp/first.cmake"
  fi
  [ "$kind" = cart ] || continue
  n=$((n + 1))
  tid="pack.trailer-$id"
  executed=$((executed + 1))
  ok=1
  rm -f "$tmp/c.ivsc" "$tmp/body" "$tmp/trail"
  if ! "$pack" --ivsc "$root/$path" "$tmp/c.ivsc" 2> "$tmp/pack.err"; then
    ok=0
    { printf 'ivs_pack --ivsc failed\n'; cat "$tmp/pack.err"; } > "$tmp/why"
  else
    size=$(wc -c < "$tmp/c.ivsc" | tr -d ' ')
    if [ "$size" -le 32 ]; then
      ok=0
      printf 'container of %s bytes has no room for a trailer\n' "$size" > "$tmp/why"
    else
      dd if="$tmp/c.ivsc" of="$tmp/body" bs=1 count=$((size - 32)) 2> /dev/null
      dd if="$tmp/c.ivsc" of="$tmp/trail" bs=1 skip=$((size - 32)) 2> /dev/null
      want=$("$cmake" -E sha256sum "$tmp/body" | cut -d ' ' -f 1)
      got=$(od -An -tx1 -v "$tmp/trail" | tr -d ' \n')
      if [ "${#want}" -ne 64 ] || [ "$want" != "$got" ]; then
        ok=0
        printf 'trailer %s, sha256 of the first %s bytes %s\n' "$got" "$((size - 32))" "$want" > "$tmp/why"
      fi
    fi
  fi
  report
done < "$inv"

# Judge self-control (plan 04-24, D-46): pack.judge.patched-word runs the comparison of
# pack.words-<id> on ivs_pack's dump of the first listing whose case passed, with the first word
# patched to another value (--patch ADDR=HEX). It passes only when the comparison fails and its
# diff shows the patched line and nothing else.
n=$((n + 1))
tid=pack.judge.patched-word
executed=$((executed + 1))
ok=1
addr=
word=
if [ -n "$first_id" ]; then read -r addr word rest < "$tmp/first.cmake" || :; fi
if [ "$word" = 0000 ]; then new=0001; else new=0000; fi
if [ -z "$first_id" ]; then
  ok=0
  printf 'no pack.words case passed, so there is nothing to judge\n' > "$tmp/why"
elif [ -z "$addr" ] || [ -z "$word" ]; then
  ok=0
  printf 'no first word in the dump of %s\n' "$first_id" > "$tmp/why"
elif ! "$pack" --dump-words "$root/$first_path" --patch "$addr=$new" > "$tmp/patched.out" 2> "$tmp/pack.err"; then
  ok=0
  { printf 'ivs_pack --patch %s=%s failed\n' "$addr" "$new"; cat "$tmp/pack.err"; } > "$tmp/why"
elif dumps_agree "$tmp/patched.out" "$tmp/first.cmake"; then
  ok=0
  printf 'judge-vacuous: the comparison accepted the dump of %s with %s patched to %s\n' \
    "$first_id" "$addr" "$new" > "$tmp/why"
elif [ "$(grep -c '^[<>] ' "$tmp/why")" -ne 2 ] || ! grep -q -x -F -e "< $addr $new" "$tmp/why" \
    || ! grep -q -x -F -e "> $addr $word" "$tmp/why"; then
  ok=0
  { printf 'judge-wrong-reason: the comparison failed, but not on %s alone:\n' "$addr"; cat "$tmp/why"; } > "$tmp/why.2"
  mv "$tmp/why.2" "$tmp/why"
fi
report

# pack.c-include.write-error (review IN-08): a write that fails (here a file-size limit of one
# 512-byte block, below the about 1.6 KiB include of a 256-byte input, with SIGXFSZ ignored so the
# write returns an error instead of ending the process; the short stderr file stays below it) is
# the documented `write error` line on stderr, a nonzero exit and no output file left behind.
n=$((n + 1))
tid=pack.c-include.write-error
executed=$((executed + 1))
ok=1
i=0
while [ "$i" -lt 32 ]; do printf 'abcdefgh'; i=$((i + 1)); done > "$tmp/inc.in"
rm -f "$tmp/inc.out"
if (trap '' XFSZ; ulimit -f 1 && exec "$pack" --c-include "$tmp/inc.out" "img=$tmp/inc.in") 2> "$tmp/inc.err"; then
  ok=0
  printf 'ivs_pack --c-include succeeded under a file-size limit of 512 bytes\n' > "$tmp/why"
elif [ "$(cat "$tmp/inc.err")" != "ivs_pack: write error on $tmp/inc.out" ]; then
  ok=0
  { printf 'stderr is not the single write error line:\n'; cat "$tmp/inc.err"; } > "$tmp/why"
elif [ -e "$tmp/inc.out" ]; then
  ok=0
  printf 'the failed output file was left behind\n' > "$tmp/why"
fi
report

# ---- The wrap tool's eight refusals, each beside its boundary positive (plan 14-01, D-47) ----
# The refusal half of a case: exit 1, exactly one stderr line carrying the token $1, nothing on stdout and
# no output file at $2. $3 is `rm` to clear the output path first and require it absent afterwards (the
# ordinary case), or `keep` when the path is the pre-existing file the case is about, whose byte-for-byte
# survival the case asserts itself. The tool's own arguments follow. Appends the token to $tmp/fw.observed,
# which pack.judge.flatwrap-tokens compares with the pin.
fw_refused() {
  f_tok=$1
  f_out=$2
  f_mode=$3
  shift 3
  [ "$f_mode" = keep ] || rm -f "$f_out"
  "$flatwrap" "$@" > "$tmp/fw.out" 2> "$tmp/fw.err"
  f_rc=$?
  if [ "$f_rc" -ne 1 ]; then
    printf 'expected exit 1 with the token %s, got exit %s\n' "$f_tok" "$f_rc" > "$tmp/why"
  elif [ -s "$tmp/fw.out" ]; then
    printf 'a refusal wrote on stdout\n' > "$tmp/why"
  elif [ "$(wc -l < "$tmp/fw.err" | tr -d ' ')" -ne 1 ] || ! grep -q "^ivs_flatwrap: $f_tok: " "$tmp/fw.err"; then
    { printf 'expected exactly one stderr line ivs_flatwrap: %s: ..., got:\n' "$f_tok"; cat "$tmp/fw.err"; } > "$tmp/why"
  elif [ "$f_mode" != keep ] && [ -e "$f_out" ]; then
    printf 'a refusal left an output file\n' > "$tmp/why"
  else
    printf '%s\n' "$f_tok" >> "$tmp/fw.observed"
    return 0
  fi
  return 1
}
# The boundary positive of a case: exit 0, exactly one 64-hex line on stdout and a container of exactly
# $2 bytes at $1. The tool's own arguments follow.
fw_accepted() {
  f_out=$1
  f_want=$2
  shift 2
  rm -f "$f_out"
  if ! "$flatwrap" "$@" > "$tmp/fw.out" 2> "$tmp/fw.err"; then
    { printf 'the boundary positive was refused:\n'; cat "$tmp/fw.err"; } > "$tmp/why"
  elif [ "$(wc -l < "$tmp/fw.out" | tr -d ' ')" -ne 1 ] || ! grep -q -E '^[0-9a-f]{64}$' "$tmp/fw.out"; then
    { printf 'stdout is not exactly one 64-hex line:\n'; cat "$tmp/fw.out"; } > "$tmp/why"
  elif [ "$(wc -c < "$f_out" | tr -d ' ')" != "$f_want" ]; then
    printf 'the container is %s bytes, expected %s\n' "$(wc -c < "$f_out" | tr -d ' ')" "$f_want" > "$tmp/why"
  else
    return 0
  fi
  return 1
}
# The segment entry of a one-segment container, eight hex digits: addr then len_words, big-endian.
fw_entry() { od -An -tx1 -v -j 24 -N 4 "$1" | tr -d ' \n'; }
# The first data word of a container, four hex digits (the data starts at 24 + 8).
fw_word0() { od -An -tx1 -v -j 32 -N 2 "$1" | tr -d ' \n'; }
: > "$tmp/fw.observed"
grep -v '^#' "$tokens" | awk 'NF { print $1 }' > "$tmp/fw.tokens"
dd if=/dev/zero of="$tmp/fl.2" bs=2 count=1 2> /dev/null
while read -r tok; do
  [ -n "$tok" ] || continue
  n=$((n + 1))
  tid="pack.flatwrap.neg.$tok"
  executed=$((executed + 1))
  ok=1
  case $tok in
    flatwrap-empty)
      # A zero-byte input holds no word, and a container segment holds at least one: the core's parser refuses
      # len_words == 0 (IVS_IVSC_R_SEGMENT_LENGTH) and IVS_IVSC_MIN_BYTES is 66, so the 64-byte container the tool
      # used to write from an empty input was a file the project's own loader refuses (review CR-01). The boundary
      # positive is the smallest legal container: one word, 24 + 8 + 2 + 32 = 66 bytes, entry addr 4000 len 0001.
      : > "$tmp/fl.0"
      fw_refused flatwrap-empty "$tmp/fw.out.ivsc" rm "$tmp/fl.0" 4000 "$tmp/fw.out.ivsc" || ok=0
      if [ "$ok" -eq 1 ]; then
        fw_accepted "$tmp/fl.empty.ivsc" 66 "$tmp/fl.2" 4000 "$tmp/fl.empty.ivsc" || ok=0
      fi
      if [ "$ok" -eq 1 ] && [ "$(fw_entry "$tmp/fl.empty.ivsc")" != 40000001 ]; then
        ok=0
        printf 'the entry is %s, expected 40000001 (addr 4000, len_words 0001)\n' \
          "$(fw_entry "$tmp/fl.empty.ivsc")" > "$tmp/why"
      fi
      ;;
    flatwrap-length)
      # 1 byte is not a whole word; 2 bytes is one word, and the container is 24 + 8 + 2 + 32 = 66 bytes.
      dd if=/dev/zero of="$tmp/fl.1" bs=1 count=1 2> /dev/null
      fw_refused flatwrap-length "$tmp/fw.out.ivsc" rm "$tmp/fl.1" 4000 "$tmp/fw.out.ivsc" || ok=0
      if [ "$ok" -eq 1 ]; then
        fw_accepted "$tmp/fl.len.ivsc" 66 "$tmp/fl.2" 4000 "$tmp/fl.len.ivsc" || ok=0
      fi
      ;;
    flatwrap-word-range)
      # 0400 is above the container's word maximum 03FF; 03FF is the boundary that must be accepted, and the
      # word read back from the container is the value the pair is about.
      printf '\004\000' > "$tmp/fl.400"
      printf '\003\377' > "$tmp/fl.3ff"
      fw_refused flatwrap-word-range "$tmp/fw.out.ivsc" rm "$tmp/fl.400" 4000 "$tmp/fw.out.ivsc" || ok=0
      if [ "$ok" -eq 1 ]; then
        fw_accepted "$tmp/fl.3ff.ivsc" 66 "$tmp/fl.3ff" 4000 "$tmp/fl.3ff.ivsc" || ok=0
      fi
      if [ "$ok" -eq 1 ] && [ "$(fw_word0 "$tmp/fl.3ff.ivsc")" != 03ff ]; then
        ok=0
        printf 'the container word is %s, expected 03ff\n' "$(fw_word0 "$tmp/fl.3ff.ivsc")" > "$tmp/why"
      fi
      ;;
    flatwrap-words)
      # This pair is a token pair, not a refuse-to-accept pair, and the arithmetic is why:
      # IVS_IVSC_MAX_SEGMENT_WORDS is 65,535, so a one-segment writer cannot encode 65,536 words at all, and
      # $4000 + 65536 = $14000 crosses $10000 first. So 65,537 words (131,074 bytes) is refused by the count
      # rule and 65,536 words (131,072 bytes) is refused by the crossing rule, which pins the count rule's
      # threshold at exactly 65,536 and proves the writer never emits a wrapped or zero len_words. The real
      # acceptance boundary is flatwrap-cross's pair (49,152 words at 4000).
      dd if=/dev/zero of="$tmp/fl.65537" bs=65537 count=2 2> /dev/null
      dd if=/dev/zero of="$tmp/fl.65536" bs=65536 count=2 2> /dev/null
      fw_refused flatwrap-words "$tmp/fw.out.ivsc" rm "$tmp/fl.65537" 4000 "$tmp/fw.out.ivsc" || ok=0
      if [ "$ok" -eq 1 ]; then
        fw_refused flatwrap-cross "$tmp/fw.out.ivsc" rm "$tmp/fl.65536" 4000 "$tmp/fw.out.ivsc" || ok=0
      fi
      ;;
    flatwrap-base)
      # 3FFF is below the lowest base an accepted image may state; 4000 is that boundary.
      fw_refused flatwrap-base "$tmp/fw.out.ivsc" rm "$tmp/fl.2" 3FFF "$tmp/fw.out.ivsc" || ok=0
      if [ "$ok" -eq 1 ]; then
        fw_accepted "$tmp/fl.base.ivsc" 66 "$tmp/fl.2" 4000 "$tmp/fl.base.ivsc" || ok=0
      fi
      ;;
    flatwrap-cross)
      # $10000 - $4000 = $C000 = 49,152 words is the largest image any accepted base can hold, and its last
      # word sits at $FFFF; one word more crosses $10000. 24 + 8 + 2 x 49,152 + 32 = 98,368 bytes.
      dd if=/dev/zero of="$tmp/fl.49153" bs=49153 count=2 2> /dev/null
      dd if=/dev/zero of="$tmp/fl.49152" bs=49152 count=2 2> /dev/null
      fw_refused flatwrap-cross "$tmp/fw.out.ivsc" rm "$tmp/fl.49153" 4000 "$tmp/fw.out.ivsc" || ok=0
      if [ "$ok" -eq 1 ]; then
        fw_accepted "$tmp/fl.cross.ivsc" 98368 "$tmp/fl.49152" 4000 "$tmp/fl.cross.ivsc" || ok=0
      fi
      if [ "$ok" -eq 1 ] && [ "$(fw_entry "$tmp/fl.cross.ivsc")" != 4000c000 ]; then
        ok=0
        printf 'the entry is %s, expected 4000c000 (addr 4000, len_words C000, ending at FFFF)\n' \
          "$(fw_entry "$tmp/fl.cross.ivsc")" > "$tmp/why"
      fi
      ;;
    flatwrap-output-tree)
      # The rule compares the output's physical parent directory, so the literal spelling, a `..` spelling
      # and a symlink to a directory inside the tree are the same answer; outside the tree is accepted.
      mkdir -p "$root/build" "$tmp/linkdir"
      ln -sfn "$root/build" "$tmp/linkdir/inside"
      rm -f "$root/build/fw-out.ivsc"
      fw_refused flatwrap-output-tree "$root/build/fw-out.ivsc" rm "$tmp/fl.2" 4000 "$root/build/fw-out.ivsc" || ok=0
      if [ "$ok" -eq 1 ]; then
        fw_refused flatwrap-output-tree "$root/tools/../build/fw-out.ivsc" rm "$tmp/fl.2" 4000 \
          "$root/tools/../build/fw-out.ivsc" || ok=0
      fi
      if [ "$ok" -eq 1 ]; then
        fw_refused flatwrap-output-tree "$tmp/linkdir/inside/fw-out.ivsc" rm "$tmp/fl.2" 4000 \
          "$tmp/linkdir/inside/fw-out.ivsc" || ok=0
      fi
      if [ "$ok" -eq 1 ]; then
        fw_accepted "$tmp/fl.tree.ivsc" 66 "$tmp/fl.2" 4000 "$tmp/fl.tree.ivsc" || ok=0
      fi
      ;;
    flatwrap-output-exists)
      # The point of the exclusive create: the refused run must leave the existing file byte-identical, so a
      # writer that truncates first cannot pass on its exit status.
      printf 'keep this file\n' > "$tmp/fl.existing"
      cp "$tmp/fl.existing" "$tmp/fl.existing.before"
      fw_refused flatwrap-output-exists "$tmp/fl.existing" keep "$tmp/fl.2" 4000 "$tmp/fl.existing" || ok=0
      if [ "$ok" -eq 1 ] && ! cmp -s "$tmp/fl.existing" "$tmp/fl.existing.before"; then
        ok=0
        printf 'the refused run changed the existing file\n' > "$tmp/why"
      fi
      if [ "$ok" -eq 1 ]; then
        fw_accepted "$tmp/fl.new.ivsc" 66 "$tmp/fl.2" 4000 "$tmp/fl.new.ivsc" || ok=0
      fi
      ;;
    *)
      ok=0
      printf 'no case for the pinned token %s\n' "$tok" > "$tmp/why"
      ;;
  esac
  report
done < "$tmp/fw.tokens"

# pack.judge.flatwrap-tokens: the pinned tokens are the tokens the source can print, once each and
# well-formed, and every one of them was observed by a case above.
n=$((n + 1))
tid=pack.judge.flatwrap-tokens
executed=$((executed + 1))
ok=1
: > "$tmp/why"
sort "$tmp/fw.tokens" > "$tmp/fw.tokens.sorted"
sort -u "$tmp/fw.tokens" | cmp -s - "$tmp/fw.tokens.sorted" || { ok=0; printf 'a token is pinned twice\n' >> "$tmp/why"; }
grep -v -E '^flatwrap-[a-z][a-z-]*$' "$tmp/fw.tokens" | sed 's/^/malformed token: /' >> "$tmp/why"
grep -o '"flatwrap-[a-z-]*"' "$root/tools/flatwrap/flatwrap.c" | tr -d '"' | sort -u > "$tmp/fw.src"
if ! cmp -s "$tmp/fw.src" "$tmp/fw.tokens.sorted"; then
  ok=0
  { printf 'tokens the source can print (<) against pinned tokens (>)\n'; diff "$tmp/fw.src" "$tmp/fw.tokens.sorted"; } >> "$tmp/why"
fi
sort -u "$tmp/fw.observed" > "$tmp/fw.observed.sorted"
if ! cmp -s "$tmp/fw.observed.sorted" "$tmp/fw.tokens.sorted"; then
  ok=0
  { printf 'tokens a case observed (<) against pinned tokens (>)\n'; diff "$tmp/fw.observed.sorted" "$tmp/fw.tokens.sorted"; } >> "$tmp/why"
fi
[ ! -s "$tmp/why" ] || ok=0
report

# pack.judge.flatwrap-digest-live: the printed digest is sha256sum of the whole container and the trailer is
# sha256sum of every byte before it (an implementation other than the core's), and the equality is shown live:
# the same input wrapped twice gives the same digest, and the same input with one byte flipped (00 00 -> 00 01,
# still at most 03FF) gives a different digest and a different trailer. A judge that accepted the same digest
# twice would be vacuous, so that limb is the liveness guard.
n=$((n + 1))
tid=pack.judge.flatwrap-digest-live
executed=$((executed + 1))
ok=1
dd if=/dev/zero of="$tmp/dg.a" bs=4 count=1 2> /dev/null
printf '\000\000\000\001' > "$tmp/dg.b"
rm -f "$tmp/dg1.ivsc" "$tmp/dg2.ivsc" "$tmp/dg3.ivsc"
dg1=$("$flatwrap" "$tmp/dg.a" 4000 "$tmp/dg1.ivsc" 2> "$tmp/dg.err")
dg1_rc=$?
dg2=$("$flatwrap" "$tmp/dg.a" 4000 "$tmp/dg2.ivsc" 2> "$tmp/dg.err")
dg2_rc=$?
dg3=$("$flatwrap" "$tmp/dg.b" 4000 "$tmp/dg3.ivsc" 2> "$tmp/dg.err")
dg3_rc=$?
if [ "$dg1_rc" -ne 0 ] || [ "$dg2_rc" -ne 0 ] || [ "$dg3_rc" -ne 0 ]; then
  ok=0
  { printf 'a wrap failed (exit %s %s %s):\n' "$dg1_rc" "$dg2_rc" "$dg3_rc"; cat "$tmp/dg.err"; } > "$tmp/why"
elif [ "$dg1" != "$dg2" ]; then
  ok=0
  printf 'the same input wrapped twice gave %s and %s\n' "$dg1" "$dg2" > "$tmp/why"
elif [ "$dg1" != "$("$cmake" -E sha256sum "$tmp/dg1.ivsc" | cut -d ' ' -f 1)" ]; then
  ok=0
  printf 'the printed digest %s is not sha256sum of the whole container\n' "$dg1" > "$tmp/why"
elif [ "$dg1" = "$dg3" ]; then
  ok=0
  printf 'judge-vacuous: flipping one input byte left the digest at %s\n' "$dg1" > "$tmp/why"
else
  for f in dg1 dg3; do
    dg_size=$(wc -c < "$tmp/$f.ivsc" | tr -d ' ')
    dd if="$tmp/$f.ivsc" of="$tmp/$f.body" bs=1 count=$((dg_size - 32)) 2> /dev/null
    dd if="$tmp/$f.ivsc" of="$tmp/$f.trail" bs=1 skip=$((dg_size - 32)) 2> /dev/null
    dg_want=$("$cmake" -E sha256sum "$tmp/$f.body" | cut -d ' ' -f 1)
    dg_got=$(od -An -tx1 -v "$tmp/$f.trail" | tr -d ' \n')
    if [ "$dg_got" != "$dg_want" ]; then
      ok=0
      printf 'the trailer of %s is %s, sha256 of the first %s bytes is %s\n' "$f" "$dg_got" "$((dg_size - 32))" "$dg_want" > "$tmp/why"
      break
    fi
    eval "dg_trail_$f=\$dg_got"
  done
  if [ "$ok" -eq 1 ] && [ "$dg_trail_dg1" = "$dg_trail_dg3" ]; then
    ok=0
    printf 'judge-vacuous: flipping one input byte left the trailer at %s\n' "$dg_trail_dg1" > "$tmp/why"
  fi
fi
report

# pack.flatwrap.word-order: the stated word order decides the pairing and hi-first is the default. One word
# whose two bytes differ (01 02) is wrapped three times: with no flag, with an explicit hi-first and with
# lo-first. The bytes are chosen so that both orders are legal words -- 0102 and its byte-swap 0201 are at or
# below the 03FF word maximum, so neither run is refused by the range rule and the case is about the pairing
# alone. The two container words must be the byte-swap of each other, the two printed digests must differ, the
# header entry must not move with the option, and the no-flag digest must equal the explicit hi-first one (a
# default that had drifted to lo-first would pass a swap-only assertion, so this limb is the default's guard).
n=$((n + 1))
tid=pack.flatwrap.word-order
executed=$((executed + 1))
ok=1
: > "$tmp/why"
printf '\001\002' > "$tmp/wo.flat"
rm -f "$tmp/wo.def.ivsc" "$tmp/wo.hi.ivsc" "$tmp/wo.lo.ivsc"
wo_def=$("$flatwrap" "$tmp/wo.flat" 4000 "$tmp/wo.def.ivsc" 2> "$tmp/wo.err")
wo_def_rc=$?
wo_hi=$("$flatwrap" "$tmp/wo.flat" 4000 "$tmp/wo.hi.ivsc" --word-order hi-first 2> "$tmp/wo.err")
wo_hi_rc=$?
wo_lo=$("$flatwrap" "$tmp/wo.flat" 4000 "$tmp/wo.lo.ivsc" --word-order lo-first 2> "$tmp/wo.err")
wo_lo_rc=$?
if [ "$wo_def_rc" -ne 0 ] || [ "$wo_hi_rc" -ne 0 ] || [ "$wo_lo_rc" -ne 0 ]; then
  ok=0
  { printf 'a wrap failed (exit %s %s %s):\n' "$wo_def_rc" "$wo_hi_rc" "$wo_lo_rc"; cat "$tmp/wo.err"; } > "$tmp/why"
elif [ "$wo_def" != "$wo_hi" ]; then
  ok=0
  printf 'the no-flag digest %s is not the explicit hi-first digest %s, so hi-first is not the default\n' \
    "$wo_def" "$wo_hi" > "$tmp/why"
elif [ "$wo_hi" = "$wo_lo" ]; then
  ok=0
  printf 'judge-vacuous: lo-first printed the hi-first digest %s, so the two orders are one order\n' \
    "$wo_hi" > "$tmp/why"
else
  wo_whi=$(fw_word0 "$tmp/wo.hi.ivsc")
  wo_wlo=$(fw_word0 "$tmp/wo.lo.ivsc")
  wo_swap=$(printf '%s\n' "$wo_whi" | sed 's/\(..\)\(..\)/\2\1/')
  if [ "$wo_whi" != 0102 ]; then
    ok=0
    printf 'the hi-first container word is %s, expected 0102 from the input bytes 01 02\n' "$wo_whi" > "$tmp/why"
  elif [ "$wo_wlo" != "$wo_swap" ]; then
    ok=0
    printf 'the lo-first container word is %s, not the byte-swap %s of the hi-first word %s\n' \
      "$wo_wlo" "$wo_swap" "$wo_whi" > "$tmp/why"
  elif [ "$(fw_entry "$tmp/wo.hi.ivsc")" != "$(fw_entry "$tmp/wo.lo.ivsc")" ]; then
    ok=0
    printf 'the segment entry moved with the word order (%s against %s)\n' \
      "$(fw_entry "$tmp/wo.hi.ivsc")" "$(fw_entry "$tmp/wo.lo.ivsc")" > "$tmp/why"
  fi
fi
report

# pack.flatwrap.usage-word-order: the other side of the same parser. A --word-order value outside the two
# stated words, and a --word-order with no value at all, are file or usage errors, not refusals: exit 2, exactly
# one stderr line that is the tool's own usage line and so carries no token (the typed surface is the eight
# refusal tokens of data/expected/flat-refusals.ids and nothing else), nothing on stdout and no output file.
n=$((n + 1))
tid=pack.flatwrap.usage-word-order
executed=$((executed + 1))
ok=1
: > "$tmp/why"
printf '\001\002' > "$tmp/wo.flat"
wo_use='ivs_flatwrap: usage: ivs_flatwrap <flat> <base> <out> [--word-order hi-first|lo-first]'
rm -f "$tmp/wo.u1.ivsc" "$tmp/wo.u2.ivsc"
"$flatwrap" "$tmp/wo.flat" 4000 "$tmp/wo.u1.ivsc" --word-order lo-hi > "$tmp/wo.out1" 2> "$tmp/wo.err1"
wo_rc1=$?
"$flatwrap" "$tmp/wo.flat" 4000 "$tmp/wo.u2.ivsc" --word-order > "$tmp/wo.out2" 2> "$tmp/wo.err2"
wo_rc2=$?
if [ "$wo_rc1" -ne 2 ] || [ "$wo_rc2" -ne 2 ]; then
  ok=0
  printf 'an unknown value exited %s and a missing value %s, expected 2 and 2\n' "$wo_rc1" "$wo_rc2" > "$tmp/why"
elif [ -s "$tmp/wo.out1" ] || [ -s "$tmp/wo.out2" ]; then
  ok=0
  { printf 'a usage error wrote on stdout:\n'; cat "$tmp/wo.out1" "$tmp/wo.out2"; } > "$tmp/why"
elif [ "$(wc -l < "$tmp/wo.err1" | tr -d ' ')" -ne 1 ] || [ "$(wc -l < "$tmp/wo.err2" | tr -d ' ')" -ne 1 ]; then
  ok=0
  { printf 'a usage error did not print exactly one stderr line:\n'; cat "$tmp/wo.err1" "$tmp/wo.err2"; } > "$tmp/why"
elif ! grep -q -x -F "$wo_use" "$tmp/wo.err1" || ! grep -q -x -F "$wo_use" "$tmp/wo.err2"; then
  ok=0
  { printf 'expected the one usage line %s, got:\n' "$wo_use"; cat "$tmp/wo.err1" "$tmp/wo.err2"; } > "$tmp/why"
elif [ -e "$tmp/wo.u1.ivsc" ] || [ -e "$tmp/wo.u2.ivsc" ]; then
  ok=0
  printf 'a usage error left an output file\n' > "$tmp/why"
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

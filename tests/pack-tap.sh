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
# Output: the harness TAP shape (header with IVS_TEST_NONCE or none, plan, one result per case,
# trailer) on stdout and, when IVS_TEST_OUT_DIR is set, in <dir>/pack.txt.
# Usage: sh pack-tap.sh <source root> <ivs_pack>   Exit: 0 pass, 1 fail, 77 every case skipped.
# Env: CMAKE (set by ctest to this build's cmake; default cmake), IVS_TEST_NONCE,
# IVS_TEST_OUT_DIR.
LC_ALL=C; export LC_ALL
[ "$#" -eq 2 ] || { printf 'usage: sh pack-tap.sh <source root> <ivs_pack>\n' >&2; exit 1; }
root=$1
pack=$2
cmake=${CMAKE:-cmake}
inv="$root/data/expected/listings.ids"
[ -f "$inv" ] || { printf 'pack-tap: no %s\n' "$inv" >&2; exit 1; }
[ -x "$pack" ] || { printf 'pack-tap: %s is not executable\n' "$pack" >&2; exit 1; }
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

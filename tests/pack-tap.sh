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
  elif [ ! -s "$tmp/pack.out" ]; then
    ok=0
    printf 'empty word dump\n' > "$tmp/why"
  elif ! cmp -s "$tmp/pack.out" "$tmp/cmake.out"; then
    ok=0
    { printf 'word dumps differ (< ivs_pack, > listing.cmake)\n'; diff "$tmp/pack.out" "$tmp/cmake.out"; } > "$tmp/why"
  fi
  report
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

#!/bin/sh
# SPDX-License-Identifier: MIT
# Suite stic-table (VID-01, Phase 6 review WR-03): the STIC parameter generator (cmake/IvsSticTable.cmake) is the one owner of
# the geometry the card renderer can draw (20 x 12 cards of 8 x 8 picture elements; the row latch and the working buffers of
# core/src/stic are fixed). Each case runs the generator with `cmake -P` on a scratch copy of the registered profile, never
# on the repository:
#   stic-table.accepts-registered-profile   the unmodified profile generates and writes the table
#   stic-table.refuses-card-columns         video.card_columns 21 is refused (exit non-zero) and names the renderer
#   stic-table.refuses-card-rows            video.card_rows 13 is refused the same way
#   stic-table.refuses-card-size            video.card_size_px {7, 8} is refused the same way
# A refused run must not write the table. Output: the harness TAP shape (header with IVS_TEST_NONCE or none, plan, one result per
# case, `#` lines after a failure, trailer) on stdout and, when IVS_TEST_OUT_DIR is set, in <dir>/stic-table.txt.
# Usage: sh stic-table-tap.sh <source root>   Exit: 0 pass, 1 fail.   Env: CMAKE (set by ctest), IVS_TEST_NONCE, IVS_TEST_OUT_DIR.
LC_ALL=C; export LC_ALL
[ "$#" -eq 1 ] || { printf 'usage: sh stic-table-tap.sh <source root>\n' >&2; exit 1; }
root=$1
cmake=${CMAKE:-cmake}
gen="$root/cmake/IvsSticTable.cmake"
profile="$root/data/profiles/intv-2609-ntsc-family-p1.json"
[ -f "$gen" ] && [ -f "$profile" ] || { printf 'stic-table-tap: missing the generator or the profile under %s\n' "$root" >&2; exit 1; }
tmp=$(mktemp -d "${TMPDIR:-/tmp}/stic-table-tap.XXXXXX") || exit 1
trap 'rm -rf "$tmp"' EXIT
trap 'exit 1' HUP INT TERM

suite=stic-table
nonce=${IVS_TEST_NONCE:-none}
: > "$tmp/results"
n=0
failed=0

# gen_run <name> <sed script or empty>: a scratch ROOT holding the profile with the sed script applied; sets rc and writes the
# generator's text to $tmp/<name>.txt (line breaks folded, so a message the tool wrapped still matches).
gen_run() {
  _d="$tmp/$1"
  mkdir -p "$_d/data/profiles" || exit 1
  if [ -n "$2" ]; then
    sed "$2" "$profile" > "$_d/data/profiles/intv-2609-ntsc-family-p1.json" || exit 1
  else
    cp "$profile" "$_d/data/profiles/intv-2609-ntsc-family-p1.json" || exit 1
  fi
  rc=0
  "$cmake" "-DROOT=$_d" "-DOUT=$_d/out.inc" -P "$gen" > "$tmp/$1.raw" 2>&1 || rc=$?
  tr '\n' ' ' < "$tmp/$1.raw" | tr -s ' ' > "$tmp/$1.txt"
  out_exists=0
  [ ! -e "$_d/out.inc" ] || out_exists=1
}

# record <id> <ok 0|1> <why>
record() {
  n=$((n + 1))
  if [ "$2" -eq 1 ]; then
    printf 'ok %s %s\n' "$n" "$1" >> "$tmp/results"
  else
    printf 'not ok %s %s\n' "$n" "$1" >> "$tmp/results"
    printf '# %s\n' "$3" >> "$tmp/results"
    failed=$((failed + 1))
  fi
}

gen_run base ''
if [ "$rc" -eq 0 ] && [ "$out_exists" -eq 1 ] && grep -q 'ivs_stic_params_table' "$tmp/base/out.inc"; then
  record stic-table.accepts-registered-profile 1 ''
else
  record stic-table.accepts-registered-profile 0 "the registered profile was refused (exit $rc): $(cat "$tmp/base.txt")"
fi

refuse() {
  # refuse <id> <name> <sed script>
  gen_run "$2" "$3"
  if [ "$rc" -ne 0 ] && [ "$out_exists" -eq 0 ] && grep -q 'the card renderer implements only 20 x 12 cards of 8 x 8' "$tmp/$2.txt"; then
    record "$1" 1 ''
  else
    record "$1" 0 "exit $rc, table written $out_exists, text: $(cat "$tmp/$2.txt")"
  fi
}
# Each mutation keeps every other relation of the generator true (width still fits the 167 x 105 grid, backtab words follow the
# geometry), so only the renderer rule can refuse it.
refuse stic-table.refuses-card-columns cols '/"video.card_columns"/,/"value"/ s/"value": 20,/"value": 21,/; s/"words": 240/"words": 252/'
refuse stic-table.refuses-card-rows rows '/"video.card_rows"/,/"value"/ s/"value": 12,/"value": 13,/; s/"words": 240/"words": 260/'
refuse stic-table.refuses-card-size size 's/"value": {"w": 8, "h": 8}/"value": {"w": 7, "h": 8}/'

{
  printf '# suite=%s nonce=%s\n' "$suite" "$nonce"
  printf '1..%s\n' "$n"
  cat "$tmp/results"
  printf '# suite=%s executed=%s skipped=0 failed=%s\n' "$suite" "$n" "$failed"
} > "$tmp/tap"
cat "$tmp/tap"
if [ -n "${IVS_TEST_OUT_DIR:-}" ]; then
  cp "$tmp/tap" "$IVS_TEST_OUT_DIR/$suite.txt" || { printf '# cannot write %s\n' "$IVS_TEST_OUT_DIR/$suite.txt" >&2; exit 1; }
fi
[ "$failed" -eq 0 ]

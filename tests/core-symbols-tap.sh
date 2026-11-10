#!/bin/sh
# SPDX-License-Identifier: MIT
# Suite core-symbols (API-05, D-21): runs tools/check/core-symbols.sh on the shipped ivs_core
# archive and reports one case, core-symbols.ivs-core-archive, in the harness TAP format
# (header with IVS_TEST_NONCE or none, plan, result, trailer) to stdout and, when
# IVS_TEST_OUT_DIR is set, to <dir>/core-symbols.txt. The script output (its pass line or its
# findings) follows the result as `#` lines. The allowlist and deny list come from this source
# tree's tools/check.
# Usage: sh core-symbols-tap.sh <archive> [--allow-prefix P]...   Exit: 0 pass, 1 fail.
# Env: NM and AR (set by ctest from CMAKE_NM and CMAKE_AR), IVS_TEST_NONCE, IVS_TEST_OUT_DIR.
LC_ALL=C; export LC_ALL
[ "$#" -ge 1 ] || { printf 'usage: sh core-symbols-tap.sh <archive> [--allow-prefix P]...\n' >&2; exit 1; }
here=$(cd "$(dirname "$0")" && pwd) || exit 1
CHECK_DIR="$here/../tools/check"
export CHECK_DIR
archive=$1
shift
tmp=$(mktemp -d "${TMPDIR:-/tmp}/core-symbols-tap.XXXXXX") || exit 1
trap 'rm -rf "$tmp"' EXIT
trap 'exit 1' HUP INT TERM

suite=core-symbols
id=core-symbols.ivs-core-archive
nonce=${IVS_TEST_NONCE:-none}
st=0
sh "$CHECK_DIR/core-symbols.sh" "$@" "$archive" > "$tmp/out" 2>&1 || st=$?
{
  printf '# suite=%s nonce=%s\n' "$suite" "$nonce"
  printf '1..1\n'
  if [ "$st" -eq 0 ]; then printf 'ok 1 %s\n' "$id"; else printf 'not ok 1 %s\n' "$id"; fi
  sed 's/^/# /' "$tmp/out"
  if [ "$st" -eq 0 ]; then failed=0; else failed=1; fi
  printf '# suite=%s executed=1 skipped=0 failed=%s\n' "$suite" "$failed"
} > "$tmp/tap"
cat "$tmp/tap"
if [ -n "${IVS_TEST_OUT_DIR:-}" ]; then
  cp "$tmp/tap" "$IVS_TEST_OUT_DIR/$suite.txt" || { printf '# cannot write %s\n' "$IVS_TEST_OUT_DIR/$suite.txt" >&2; exit 1; }
fi
[ "$st" -eq 0 ]

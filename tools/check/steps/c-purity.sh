#!/bin/sh
# SPDX-License-Identifier: MIT
# Step c-purity: the machine core imports no host service (API-05, D-21, ROADMAP SC5).
# Symbol layer (authoritative): every FILELIST path core/src/**/*.c is compiled fresh into a
# temp dir at -O0 and at -O2 with ${CC:-cc} (-std=c17 -fno-lto; a missing compiler fails, never
# skips), each variant is archived with ar, and tools/check/core-symbols.sh checks the archive
# against the reviewed allowlist (lists read from CHECK_DIR, so a control cannot replace them).
# Writes only to its temp dir. Reads ROOT, CHECK_DIR, FILELIST; CC and NM from the environment.
# Tokens: c-purity-no-sources: core/src, c-purity-no-cc: <cc>, c-purity-compile: <file> (<opt>),
#   plus every core-symbols-* token. Stats: core-imports=<imports of the -O2 archive>
#   purity-selftest=<passed>/<cases>.
LC_ALL=C; export LC_ALL
: "${ROOT:?ROOT is not set}"
: "${CHECK_DIR:?CHECK_DIR is not set}"
: "${FILELIST:?FILELIST is not set}"
tmp=$(mktemp -d "${TMPDIR:-/tmp}/c-purity.XXXXXX") || exit 1
trap 'rm -rf "$tmp"' EXIT
trap 'exit 1' HUP INT TERM
rc=0
bad() { printf -- '-- FAIL %s: %s\n' "$1" "$2"; rc=1; }

grep -E '^core/src/.*[.]c$' "$FILELIST" | sort -u > "$tmp/csrc"
if [ ! -s "$tmp/csrc" ]; then
  bad c-purity-no-sources core/src
  exit 1
fi
cc=${CC:-cc}
if ! command -v "$cc" > /dev/null 2>&1; then
  bad c-purity-no-cc "$cc"
  exit 1
fi

# Symbol layer: fresh compile at -O0 and -O2, one archive per variant.
for opt in O0 O2; do
  mkdir -p "$tmp/$opt"
  set --
  while IFS= read -r src; do
    obj="$tmp/$opt/$(printf '%s' "$src" | tr '/' '_').o"
    if "$cc" -std=c17 -fno-lto -Wall -Wextra -Werror=implicit-function-declaration \
      -Werror=date-time "-$opt" -I"$ROOT/core/src" -c "$ROOT/$src" -o "$obj" > "$tmp/cc.log" 2>&1; then
      set -- "$@" "$obj"
    else
      cat "$tmp/cc.log"
      bad c-purity-compile "$src (-$opt)"
    fi
  done < "$tmp/csrc"
  [ "$rc" -eq 0 ] || continue
  if ! ar rcs "$tmp/core-$opt.a" "$@" > "$tmp/ar.log" 2>&1; then
    cat "$tmp/ar.log"
    bad c-purity-compile "archive (-$opt)"
  fi
done
[ "$rc" -eq 0 ] || exit 1

: > "$tmp/sym.out"
for opt in O0 O2; do
  if (cd "$tmp" && sh "$CHECK_DIR/core-symbols.sh" "core-$opt.a") > "$tmp/sym.$opt" 2>&1; then :; else rc=1; fi
  grep -v -e '^-- core-symbols pass ' "$tmp/sym.$opt" >> "$tmp/sym.out"
done
# The same finding at -O0 and -O2 prints once.
sort -u "$tmp/sym.out"
imports=$(sed -n 's/^-- core-symbols pass external=\([0-9][0-9]*\) .*$/\1/p' "$tmp/sym.O2")

selftest_ok=0
selftest_total=0

[ "$rc" -eq 0 ] || exit 1
printf -- '-- c-purity pass core-imports=%s purity-selftest=%s/%s\n' "$imports" "$selftest_ok" "$selftest_total"

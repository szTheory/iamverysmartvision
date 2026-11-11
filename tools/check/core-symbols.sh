#!/bin/sh
# SPDX-License-Identifier: MIT
# Core symbol allowlist check (API-05, D-21). Not a step: called by steps/c-purity.sh on fresh
# -O0/-O2 archives and by the ctest suite core-symbols (tests/core-symbols-tap.sh) on the
# shipped ivs_core archive.
# Usage: sh core-symbols.sh [--allow-prefix P]... FILE...   (no FILE: usage, exit 2)
# Env: NM (default nm), AR (default ar, used only to look inside archives for bitcode members),
#   CHECK_DIR (directory of the lists; default: this script's directory).
# Undefined = `$NM -P -u`, defined = `$NM -P -g --defined-only`, over the union of all FILEs.
# Names are normalized: an `@<version>` suffix is dropped, and on Darwin one leading underscore.
# Forbidden = undefined minus defined minus core-symbols-allow.txt minus every --allow-prefix.
# The allowlist is an allowlist: any other import fails, whether or not it is a known-bad name.
# core-symbols-deny.txt (optional, `<name> <category>` lines) only labels a forbidden name with
# its category (a Darwin `$SUFFIX` variant such as fopen$DARWIN_EXTSN gets the base name's
# label) and must not share a name with the allowlist.
# Fails closed on: a missing FILE, LLVM bitcode (raw or wrapper magic, also as an archive
# member), a GCC LTO object (`.gnu.lto_` section), an nm failure and a FILE with zero external
# definitions (Apple clang emits a local ltmp0 even for an empty object, so count with -g).
# Tokens: core-symbols-forbidden: <name>[ (<category>)], core-symbols-missing: <file>,
#   core-symbols-bitcode: <file>, core-symbols-lto: <file>, core-symbols-nm-failed: <file>,
#   core-symbols-no-definitions: <file>, core-symbols-list-missing: <list>,
#   core-symbols-list-empty: <list>, core-symbols-deny-in-allow: <name>.
# Pass: -- core-symbols pass external=<imports not defined inside the input> defined=<n>.
LC_ALL=C; export LC_ALL
NM=${NM:-nm}
AR=${AR:-ar}
usage() { printf 'usage: sh core-symbols.sh [--allow-prefix P]... FILE...\n' >&2; exit 2; }
prefixes=
while [ "$#" -gt 0 ]; do
  case $1 in
    --allow-prefix)
      # An empty prefix would allow every name, so it is a usage error.
      [ "$#" -ge 2 ] || usage
      case $2 in ''|*[!A-Za-z0-9_.\$]*) usage ;; esac
      prefixes="$prefixes $2"
      shift 2 ;;
    --) shift; break ;;
    -*) usage ;;
    *) break ;;
  esac
done
[ "$#" -ge 1 ] || usage
listdir=${CHECK_DIR:-$(cd "$(dirname "$0")" && pwd)}
tmp=$(mktemp -d "${TMPDIR:-/tmp}/core-symbols.XXXXXX") || exit 1
trap 'rm -rf "$tmp"' EXIT
trap 'exit 1' HUP INT TERM
rc=0
bad() { printf -- '-- FAIL %s: %s\n' "$1" "$2"; rc=1; }

# Lists.
if [ -f "$listdir/core-symbols-allow.txt" ]; then
  awk '/^#/ { next } NF { print $1 }' "$listdir/core-symbols-allow.txt" | sort -u > "$tmp/allow"
  [ -s "$tmp/allow" ] || bad core-symbols-list-empty core-symbols-allow.txt
else
  bad core-symbols-list-missing core-symbols-allow.txt
  : > "$tmp/allow"
fi
: > "$tmp/deny"
if [ -f "$listdir/core-symbols-deny.txt" ]; then
  awk '/^#/ { next } NF >= 2 { print $1, $2 }' "$listdir/core-symbols-deny.txt" | sort -u > "$tmp/deny"
  [ -s "$tmp/deny" ] || bad core-symbols-list-empty core-symbols-deny.txt
  awk '{ print $1 }' "$tmp/deny" | sort -u > "$tmp/deny.names"
  for n in $(comm -12 "$tmp/allow" "$tmp/deny.names"); do bad core-symbols-deny-in-allow "$n"; done
fi

darwin=0
[ "$(uname -s 2> /dev/null)" = Darwin ] && darwin=1
# Symbol lines of `nm -P` output: skip blank lines and archive member headers (`m.o:` or
# `lib.a[m.o]:`), keep the first field, drop `@ver`, strip one Darwin underscore.
norm() {
  awk -v darwin="$darwin" '
    NF < 2 { next }
    $1 ~ /:$/ { next }
    { n = $1; sub(/@.*/, "", n); if (darwin && n ~ /^_/) n = substr(n, 2); if (n != "") print n }'
}
# run_nm <out> <nm args...>: a nonzero nm exit is a failure, except when its only complaint is
# "no symbols" (an object with nothing external), which reads as empty output.
run_nm() {
  _out=$1; shift
  if "$NM" "$@" > "$_out" 2> "$tmp/nm.err"; then return 0; fi
  if [ -s "$tmp/nm.err" ] && ! grep -v -q -e 'no symbols' "$tmp/nm.err"; then return 0; fi
  cat "$tmp/nm.err"
  return 1
}
magic4() { od -An -tx1 -N4 "$1" 2> /dev/null | tr -d ' \n'; }
is_bitcode() {
  case $(magic4 "$1") in 4243c0de|dec0170b) return 0 ;; esac
  return 1
}

: > "$tmp/u.all"
: > "$tmp/d.all"
i=0
for f in "$@"; do
  i=$((i + 1))
  if [ ! -f "$f" ]; then bad core-symbols-missing "$f"; continue; fi
  case $f in /*) abs=$f ;; *) abs=$(pwd)/$f ;; esac
  if is_bitcode "$f"; then bad core-symbols-bitcode "$f"; continue; fi
  if [ "$(od -An -tx1 -N8 "$f" 2> /dev/null | tr -d ' \n')" = 213c617263683e0a ]; then
    # An archive: look at the first bytes of every member.
    mkdir -p "$tmp/ar.$i"
    if ! (cd "$tmp/ar.$i" && "$AR" x "$abs") > "$tmp/ar.err" 2>&1; then
      cat "$tmp/ar.err"
      bad core-symbols-nm-failed "$f (ar x)"
      continue
    fi
    member_bc=0
    for m in "$tmp/ar.$i"/* "$tmp/ar.$i"/.[!.]*; do
      [ -f "$m" ] || continue
      if is_bitcode "$m"; then member_bc=1; fi
    done
    if [ "$member_bc" -eq 1 ]; then bad core-symbols-bitcode "$f"; continue; fi
  fi
  if grep -q -a -F '.gnu.lto_' "$f"; then bad core-symbols-lto "$f"; continue; fi
  if ! run_nm "$tmp/u.raw" -P -u "$f"; then bad core-symbols-nm-failed "$f"; continue; fi
  if ! run_nm "$tmp/d.raw" -P -g --defined-only "$f"; then bad core-symbols-nm-failed "$f"; continue; fi
  if grep -q -e '\.gnu\.lto_' -e '__gnu_lto_' "$tmp/u.raw" "$tmp/d.raw"; then
    bad core-symbols-lto "$f"
    continue
  fi
  norm < "$tmp/u.raw" | sort -u > "$tmp/u.one"
  norm < "$tmp/d.raw" | sort -u > "$tmp/d.one"
  if [ ! -s "$tmp/d.one" ]; then bad core-symbols-no-definitions "$f"; continue; fi
  cat "$tmp/u.one" >> "$tmp/u.all"
  cat "$tmp/d.one" >> "$tmp/d.all"
done

sort -u "$tmp/u.all" -o "$tmp/u.all"
sort -u "$tmp/d.all" -o "$tmp/d.all"
comm -23 "$tmp/u.all" "$tmp/d.all" > "$tmp/imports"
comm -23 "$tmp/imports" "$tmp/allow" \
  | awk -v p="$prefixes" 'BEGIN { n = split(p, a, " ") }
      { for (k = 1; k <= n; k++) if (index($0, a[k]) == 1) next; print }' > "$tmp/forbidden"
awk -v deny="$tmp/deny" '
    BEGIN { while ((getline l < deny) > 0) { split(l, a, " "); cat[a[1]] = a[2] } }
    { k = $1; sub(/[$].*/, "", k); if (k in cat) print $1 " (" cat[k] ")"; else print $1 }' "$tmp/forbidden" \
  | while IFS= read -r n; do printf -- '-- FAIL core-symbols-forbidden: %s\n' "$n"; done
[ -s "$tmp/forbidden" ] && rc=1

[ "$rc" -eq 0 ] || exit 1
printf -- '-- core-symbols pass external=%s defined=%s\n' \
  "$(grep -c . "$tmp/imports")" "$(grep -c . "$tmp/d.all")"

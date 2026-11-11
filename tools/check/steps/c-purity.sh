#!/bin/sh
# SPDX-License-Identifier: MIT
# Step c-purity: the machine core imports no host service (API-05, D-21, ROADMAP SC5).
# Stage 1, source layer (runs before any compile; a finding stops the step here, so a source
# violation is always reported by its own token and never masked by a compile failure):
#   - every FILELIST path core/src/**/*.c, core/src/**/*.h and core/include/**/*.h (the public
#     tree, same rules as core/src; A-06, plan 03-03): every #include or #import line
#     is checked (c-purity-include). An angle include may name only stdint.h, stddef.h,
#     stdbool.h, limits.h or string.h; a quoted include must name a FILELIST header under
#     core/src or core/include, as a path from core/src, from core/include or from the including
#     file's directory, so `#include "stdio.h"` (resolved through the system path) and a header
#     that exists only outside both trees fail; any other form (a computed
#     `#include MACRO`, #include_next, a continuation or a comment before the operand) fails
#     closed (review WR-02). After removing comments and string
#     and character literals, the words asm, __asm, __asm__, __builtin_readcyclecounter,
#     __builtin_ia32_rdtsc, __rdtsc, __rdtscp and syscall fail (c-purity-asm), and so does a
#     floating literal (c-purity-float-literal): a decimal number with `.` or an exponent, or a
#     hex number with `.` or a `p` exponent; integer hex constants such as 0x3E4u and comment
#     cites such as `pdf 49` are not numbers here;
#   - .c files only: the words float and double fail (c-purity-float-type; the prelude header
#     names them in its poison pragma), and the first #include must be "common/ivs_prelude.h"
#     or another quoted path ending ivs_prelude.h (c-purity-prelude).
#   - reviewed generated includes (A-06, H2 c-d, plan 03-03): the inventory
#     data/expected/core-generated.ids (read from ROOT, not CHECK_DIR, so a control can overlay
#     it; an absent file is an empty list) holds lines
#     `<basename> <generator repo path>`. Before the scan, every listed generator is run as
#     `cmake -DROOT=<tree> -DOUT=<temp>/gen/<basename> -P <tree>/<generator>`; a malformed line,
#     a missing generator, a generator failure or a missing output fails
#     (c-purity-generated: <basename> (<reason>)). A quoted include whose operand equals a listed
#     basename resolves; any other name that is not a tracked header stays c-purity-include. Each
#     generated file gets the header-level scan above (include, asm, float literal), reported as
#     <token> gen/<basename>:<line>.
# Stage 2, symbol layer (authoritative): every core .c is compiled fresh into a temp dir at -O0
#   and at -O2 with ${CC:-cc} (-std=c17 -fno-lto -fno-pie -I core/include -I core/src
#   -I <temp>/gen; a missing include directory is ignored by the compiler; a missing compiler
#   fails, never skips), each variant is archived with ar, and tools/check/core-symbols.sh
#   checks each archive against the reviewed allowlist (lists read from CHECK_DIR, so a control
#   cannot replace them) and, with --forbid-writable-data, for writable data symbols
#   (core-symbols-writable; D-07, A-06, H5). -fno-pie keeps an ELF const table with pointers
#   out of .data.rel.ro, so only real writable data fails. The ctest suite core-symbols stays
#   imports-only (its dev archive is sanitized).
# Stage 3, purity self-test: core-symbols.sh must reject an LTO object (lto), a file with the
#   bitcode magic (bitcode-magic), an archive with zero external definitions (no-definitions),
#   a missing file (missing-file) and no input at all (no-input, exit 2). Synthetic inputs live
#   in the temp dir only. The step forces -fno-lto, so an overlay control cannot produce an LTO
#   object; this stage covers that class instead.
# Writes only to its temp dir. Reads ROOT, CHECK_DIR, FILELIST; CC and NM from the environment.
# Tokens: c-purity-no-sources: core/src, c-purity-no-cc: <cc>, c-purity-compile: <file> (<opt>),
#   c-purity-generated: <basename> (<reason>),
#   c-purity-include, c-purity-prelude, c-purity-asm, c-purity-float-type,
#   c-purity-float-literal (each: <file>:<line>), purity-selftest-<case>: <detail>, plus every
#   core-symbols-* token. Stats: core-imports=<imports of the -O2 archive>
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
grep -E '^core/(src|include)/.*[.]h$' "$FILELIST" | sort -u > "$tmp/hsrc"
if [ ! -s "$tmp/csrc" ]; then
  bad c-purity-no-sources core/src
  exit 1
fi
cc=${CC:-cc}
if ! command -v "$cc" > /dev/null 2>&1; then
  bad c-purity-no-cc "$cc"
  exit 1
fi

# Reviewed generated includes: run each listed generator into the temp dir.
mkdir -p "$tmp/gen"
: > "$tmp/genlist"
geninv="$ROOT/data/expected/core-generated.ids"
if [ -f "$geninv" ]; then
  while IFS= read -r line || [ -n "$line" ]; do
    set -f
    # shellcheck disable=SC2086
    set -- $line
    set +f
    [ "$#" -gt 0 ] || continue
    case $1 in '#'*) continue ;; esac
    if [ "$#" -ne 2 ]; then bad c-purity-generated "$1 (bad line)"; continue; fi
    gname=$1; ggen=$2
    case $gname in
      *[!A-Za-z0-9._-]*|.*) bad c-purity-generated "$gname (bad basename)"; continue ;;
    esac
    case $ggen in
      /*|*..*|*[!A-Za-z0-9._/-]*)
        bad c-purity-generated "$gname (bad generator path $ggen)"; continue ;;
    esac
    if grep -q -x -F -e "$gname" "$tmp/genlist"; then
      bad c-purity-generated "$gname (listed twice)"; continue
    fi
    printf '%s\n' "$gname" >> "$tmp/genlist"
    if [ ! -f "$ROOT/$ggen" ]; then
      bad c-purity-generated "$gname (generator $ggen missing)"; continue
    fi
    if ! cmake -DROOT="$ROOT" -DOUT="$tmp/gen/$gname" -P "$ROOT/$ggen" > "$tmp/gen.log" 2>&1; then
      sed 's/^/# /' "$tmp/gen.log"
      bad c-purity-generated "$gname (generator $ggen failed)"; continue
    fi
    [ -f "$tmp/gen/$gname" ] || bad c-purity-generated "$gname (generator $ggen wrote no output)"
  done < "$geninv"
fi

# Stage 1: source layer. One awk pass per file prints "<token> <file>:<line>" findings.
scan() { # $1 reported path, $2 1 for a .c file, 0 for a header, $3 file to read (default ROOT/$1)
  awk -v file="$1" -v isc="$2" -v hlist="$tmp/hsrc" -v glist="$tmp/genlist" '
    function report(t) { print t " " file ":" NR }
    function word(w) {
      if (w in asmw) report("c-purity-asm")
      else if (isc && (w == "float" || w == "double")) report("c-purity-float-type")
    }
    function number(s) {
      if (s ~ /^0[xX]/) { if (s ~ /[.pP]/) report("c-purity-float-literal") }
      else if (s ~ /[.eE]/) report("c-purity-float-literal")
    }
    BEGIN {
      split("stdint.h stddef.h stdbool.h limits.h string.h", a, " ")
      for (k in a) okh[a[k]] = 1
      split("asm __asm __asm__ __builtin_readcyclecounter __builtin_ia32_rdtsc __rdtsc __rdtscp syscall", b, " ")
      for (k in b) asmw[b[k]] = 1
      while ((getline hp < hlist) > 0) corehdr[hp] = 1
      close(hlist)
      while ((getline gp < glist) > 0) genname[gp] = 1
      close(glist)
      dir = file; sub(/\/[^\/]*$/, "", dir)
      incomment = 0; seen_inc = 0
    }
    {
      line = $0
      if (!incomment && line ~ /^[[:space:]]*#[[:space:]]*(include|import)/) {
        if (isc && !seen_inc && line !~ /^[[:space:]]*#[[:space:]]*include[[:space:]]*"([^"]*\/)?ivs_prelude\.h"/)
          report("c-purity-prelude")
        seen_inc = 1
        rest = line
        sub(/^[[:space:]]*#[[:space:]]*(include|import)/, "", rest)
        if (rest ~ /^[A-Za-z0-9_]/) report("c-purity-include")
        else {
          sub(/^[[:space:]]*/, "", rest)
          if (rest ~ /^<[^>]*>/) {
            h = substr(rest, 2); sub(/>.*$/, "", h)
            if (!(h in okh)) report("c-purity-include")
          } else if (rest ~ /^"[^"]*"/) {
            h = substr(rest, 2); sub(/".*$/, "", h)
            if (!(("core/src/" h) in corehdr) && !(("core/include/" h) in corehdr) \
                && !((dir "/" h) in corehdr) && !(h in genname)) report("c-purity-include")
          } else report("c-purity-include")
        }
      }
      n = length(line); i = 1
      while (i <= n) {
        c = substr(line, i, 1)
        if (incomment) {
          if (c == "*" && substr(line, i + 1, 1) == "/") { incomment = 0; i += 2 } else i++
          continue
        }
        if (c == "/" && substr(line, i + 1, 1) == "*") { incomment = 1; i += 2; continue }
        if (c == "/" && substr(line, i + 1, 1) == "/") break
        if (c == "\"" || c == "\047") {
          q = c; i++
          while (i <= n) {
            d = substr(line, i, 1)
            if (d == "\\") { i += 2; continue }
            i++
            if (d == q) break
          }
          continue
        }
        if (c ~ /[A-Za-z_]/) {
          j = i + 1
          while (j <= n && substr(line, j, 1) ~ /[A-Za-z0-9_]/) j++
          word(substr(line, i, j - i)); i = j
          continue
        }
        if (c ~ /[0-9]/ || (c == "." && substr(line, i + 1, 1) ~ /[0-9]/)) {
          # A C preprocessing number: digits, letters, `_`, `.`, and a sign after e/E/p/P.
          j = i + 1
          while (j <= n) {
            d = substr(line, j, 1)
            if ((d == "+" || d == "-") && substr(line, j - 1, 1) ~ /[eEpP]/) { j++; continue }
            if (d ~ /[A-Za-z0-9_.]/) { j++; continue }
            break
          }
          number(substr(line, i, j - i)); i = j
          continue
        }
        i++
      }
    }
    END { if (isc && !seen_inc) print "c-purity-prelude " file ":1" }' "${3:-$ROOT/$1}"
}
: > "$tmp/src.find"
while IFS= read -r p; do scan "$p" 1 >> "$tmp/src.find"; done < "$tmp/csrc"
while IFS= read -r p; do scan "$p" 0 >> "$tmp/src.find"; done < "$tmp/hsrc"
while IFS= read -r g; do
  [ -f "$tmp/gen/$g" ] && scan "gen/$g" 0 "$tmp/gen/$g" >> "$tmp/src.find"
done < "$tmp/genlist"
if [ -s "$tmp/src.find" ]; then
  sort -u "$tmp/src.find" | while IFS= read -r l; do
    printf -- '-- FAIL %s: %s\n' "${l%% *}" "${l#* }"
  done
  exit 1
fi
[ "$rc" -eq 0 ] || exit 1

# Stage 2: symbol layer. Fresh compile at -O0 and -O2, one archive per variant.
for opt in O0 O2; do
  mkdir -p "$tmp/$opt"
  set --
  while IFS= read -r src; do
    obj="$tmp/$opt/$(printf '%s' "$src" | tr '/' '_').o"
    if "$cc" -std=c17 -fno-lto -fno-pie -Wall -Wextra -Werror=implicit-function-declaration \
      -Werror=date-time "-$opt" -I"$ROOT/core/include" -I"$ROOT/core/src" -I"$tmp/gen" \
      -c "$ROOT/$src" -o "$obj" > "$tmp/cc.log" 2>&1; then
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
  if (cd "$tmp" && sh "$CHECK_DIR/core-symbols.sh" --forbid-writable-data "core-$opt.a") \
    > "$tmp/sym.$opt" 2>&1; then :; else rc=1; fi
  grep -v -e '^-- core-symbols pass ' "$tmp/sym.$opt" >> "$tmp/sym.out"
done
# The same finding at -O0 and -O2 prints once.
sort -u "$tmp/sym.out"
imports=$(sed -n 's/^-- core-symbols pass external=\([0-9][0-9]*\) .*$/\1/p' "$tmp/sym.O2")

# Stage 3: purity self-test of core-symbols.sh on synthetic inputs.
st="$tmp/selftest"
mkdir -p "$st"
selftest_ok=0
selftest_total=0
# expect <case> <token> <args...>: core-symbols.sh must exit nonzero and print `FAIL <token>:`
# (several tokens may be given as one `a|b` alternation).
expect() {
  _case=$1; _tok=$2; shift 2
  selftest_total=$((selftest_total + 1))
  _st=0
  (cd "$st" && sh "$CHECK_DIR/core-symbols.sh" "$@") > "$st/out" 2>&1 || _st=$?
  if [ "$_st" -ne 0 ] && grep -q -E -e "^-- FAIL ($_tok): " "$st/out"; then
    selftest_ok=$((selftest_ok + 1))
  else
    bad "purity-selftest-$_case" "exit $_st, wanted $_tok"
    sed 's/^/# /' "$st/out"
  fi
}
printf 'int ivs_st_lto(void);\nint ivs_st_lto(void) { return 1; }\n' > "$st/lto.c"
if "$cc" -std=c17 -flto -c "$st/lto.c" -o "$st/lto.o" > "$st/cc.log" 2>&1 \
  && ar rcs "$st/lto.a" "$st/lto.o" > "$st/ar.log" 2>&1; then
  expect lto 'core-symbols-lto|core-symbols-bitcode' lto.o lto.a
else
  selftest_total=$((selftest_total + 1))
  cat "$st/cc.log" "$st/ar.log" 2> /dev/null
  bad purity-selftest-lto "could not build an LTO object with $cc -flto"
fi
printf 'BC\300\336' > "$st/magic.o"
expect bitcode-magic core-symbols-bitcode magic.o
printf 'static int ivs_st_hidden(void) { return 0; }\n' > "$st/hollow.c"
if "$cc" -std=c17 -fno-lto -w -c "$st/hollow.c" -o "$st/hollow.o" > "$st/cc.log" 2>&1 \
  && ar rcs "$st/hollow.a" "$st/hollow.o" > "$st/ar.log" 2>&1; then
  expect no-definitions core-symbols-no-definitions hollow.a
else
  selftest_total=$((selftest_total + 1))
  cat "$st/cc.log" "$st/ar.log" 2> /dev/null
  bad purity-selftest-no-definitions "could not build the hollow archive"
fi
expect missing-file core-symbols-missing absent.a
selftest_total=$((selftest_total + 1))
_st=0
(cd "$st" && sh "$CHECK_DIR/core-symbols.sh") > "$st/out" 2>&1 || _st=$?
if [ "$_st" -eq 2 ]; then
  selftest_ok=$((selftest_ok + 1))
else
  bad purity-selftest-no-input "exit $_st, wanted 2"
fi

[ "$rc" -eq 0 ] || exit 1
printf -- '-- c-purity pass core-imports=%s purity-selftest=%s/%s\n' "$imports" "$selftest_ok" "$selftest_total"

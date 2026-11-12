#!/bin/sh
# SPDX-License-Identifier: MIT
# Step runner-scan: runner source hygiene (D-23, A-07, plan 04-04). Strict -std=c17 hides POSIX
# names on glibc but not on Apple clang, so a runner file that reaches for POSIX passes locally and
# fails only on a hosted gcc run (Pitfall 4). This step makes that class a local failure: POSIX
# stays confined to the single file runner/hostio.c, and no runner file reaches core internals.
# Scope: every FILELIST path runner/**/*.c and runner/**/*.h that exists, plus the runner CMake
# files (runner/**/CMakeLists.txt, runner/**/*.cmake) for the feature-macro rule only.
# Source lines are read as the compiler does: backslash-newline continuations are joined first
# (a finding names the first physical line), then block and line comments are removed (string and
# character literals are kept and never open a comment). A directive starts with `#`, the digraph
# `%:` or the trigraph `??=`.
#   - include rule (every runner .c/.h except runner/hostio.c): an #include or #import of
#     `<name>` must name an ISO C header of the list below or `ivs/<name>.h`; a quoted include
#     must name a FILELIST header under runner/ (relative to runner/ or to the including file's
#     directory) or `ivs/<name>.h`. Since plan 05-10 (D-21, A-07) exactly one more operand is
#     allowed, the quoted `ivs_refpcm.h` (the reference PCM converter of tools/pcm, found through the
#     include directory of the internal ivs_refpcm target); any other tools/ header, a neighbour of
#     that name included, the angle form `<ivs_refpcm.h>` and a path to it stay
#     runner-scan-header. Anything else, including #include_next, a computed
#     `#include MACRO` and an unterminated operand, is runner-scan-header (fails closed).
#   - internal rule (every runner .c/.h, runner/hostio.c included): an include operand that
#     contains `src/`, `machine/`, `memmap/`, `cpu/`, `common/` or `..` is
#     runner-scan-internal-include (reported instead of runner-scan-header).
#   - feature-macro rule: a #define or #undef of _POSIX_C_SOURCE, _POSIX_SOURCE, _XOPEN_SOURCE
#     (and _XOPEN_SOURCE_EXTENDED), _GNU_SOURCE, _DEFAULT_SOURCE, _BSD_SOURCE or
#     _DARWIN_C_SOURCE in any runner .c/.h other than runner/hostio.c, and any of those names on
#     a non-comment line of a runner CMake file (a compile definition would bypass hostio.c), is
#     runner-scan-posix-define.
# ISO C headers allowed: assert.h errno.h inttypes.h limits.h signal.h stdbool.h stddef.h
#   stdint.h stdio.h stdlib.h string.h.
# Zero runner .c/.h files fail (runner-scan-empty: the directory exists since plan 04-03, so a
# vacuous pass would be a bug). Pure: reads ROOT, CHECK_DIR, FILELIST; writes only its temp dir.
# Tokens: runner-scan-empty: runner, runner-scan-header: <file>:<line> <operand>,
#   runner-scan-internal-include: <file>:<line> <operand>,
#   runner-scan-posix-define: <file>:<line> <name>.
# Stats: runner-files=<n> (scanned runner .c/.h files, runner/hostio.c included).
LC_ALL=C; export LC_ALL
: "${ROOT:?ROOT is not set}"
: "${CHECK_DIR:?CHECK_DIR is not set}"
: "${FILELIST:?FILELIST is not set}"
tmp=$(mktemp -d "${TMPDIR:-/tmp}/runner-scan.XXXXXX") || exit 1
trap 'rm -rf "$tmp"' EXIT
trap 'exit 1' HUP INT TERM
rc=0
bad() { printf -- '-- FAIL %s: %s\n' "$1" "$2"; rc=1; }

: > "$tmp/src"
for p in $(grep -E '^runner/.*[.](c|h)$' "$FILELIST" | sort -u); do
  [ -f "$ROOT/$p" ] && printf '%s\n' "$p" >> "$tmp/src"
done
: > "$tmp/cmk"
for p in $(grep -E '^runner/((.*/)?CMakeLists[.]txt|.*[.]cmake)$' "$FILELIST" | sort -u); do
  [ -f "$ROOT/$p" ] && printf '%s\n' "$p" >> "$tmp/cmk"
done
n=$(grep -c . "$tmp/src")
if [ "$n" -eq 0 ]; then
  bad runner-scan-empty runner
  exit 1
fi
grep -E '^runner/.*[.]h$' "$tmp/src" > "$tmp/hdr" || :

: > "$tmp/find"
while IFS= read -r p; do
  awk -v file="$p" -v hdrs="$tmp/hdr" '
    BEGIN {
      while ((getline h < hdrs) > 0) have[h] = 1
      close(hdrs)
      split("assert.h errno.h inttypes.h limits.h signal.h stdbool.h stddef.h stdint.h stdio.h stdlib.h string.h", iso, " ")
      for (i in iso) ok[iso[i]] = 1
      dir = file
      sub(/\/[^\/]*$/, "", dir)
      exempt = (file == "runner/hostio.c")
      incmt = 0
      acc = ""
    }
    function report(tok, ln, what) { printf "%s: %s:%d %s\n", tok, file, ln, what }
    # Remove comments from one logical line; block comments carry over (state incmt).
    function strip(s,    out, i, c, c2, q) {
      out = ""
      i = 1
      while (i <= length(s)) {
        c = substr(s, i, 1)
        c2 = substr(s, i, 2)
        if (incmt) {
          if (c2 == "*/") { incmt = 0; out = out " "; i += 2 } else i++
          continue
        }
        if (c2 == "/*") { incmt = 1; i += 2; continue }
        if (c2 == "//") break
        if (c == "\"" || c == "\047") {
          q = c
          out = out c
          i++
          while (i <= length(s)) {
            c = substr(s, i, 1)
            out = out c
            if (c == "\\") { out = out substr(s, i + 1, 1); i += 2; continue }
            i++
            if (c == q) break
          }
          continue
        }
        out = out c
        i++
      }
      return out
    }
    function check(s, ln,    op, rest, name, word) {
      if (s !~ /^[ \t]*(#|%:|[?][?]=)/) return
      sub(/^[ \t]*(#|%:|[?][?]=)[ \t]*/, "", s)
      if (s ~ /^(define|undef)[ \t]/) {
        if (exempt) return
        word = s
        sub(/^(define|undef)[ \t]+/, "", word)
        if (match(word, /^[A-Za-z_][A-Za-z0-9_]*/)) {
          name = substr(word, 1, RLENGTH)
          if (name ~ /^(_POSIX_C_SOURCE|_POSIX_SOURCE|_XOPEN_SOURCE|_XOPEN_SOURCE_EXTENDED|_GNU_SOURCE|_DEFAULT_SOURCE|_BSD_SOURCE|_DARWIN_C_SOURCE)$/)
            report("runner-scan-posix-define", ln, name)
        }
        return
      }
      if (s !~ /^(include|import)/) return
      if (s ~ /^(include|import)[A-Za-z0-9_]/) {
        report("runner-scan-header", ln, s)
        return
      }
      sub(/^(include|import)[ \t]*/, "", s)
      if (substr(s, 1, 1) == "<" && match(s, /^<[^>]*>/)) {
        op = substr(s, 2, RLENGTH - 2)
        rest = substr(s, RLENGTH + 1)
      } else if (substr(s, 1, 1) == "\"" && match(s, /^"[^"]*"/)) {
        op = substr(s, 2, RLENGTH - 2)
        rest = substr(s, RLENGTH + 1)
      } else {
        report("runner-scan-header", ln, s)
        return
      }
      if (rest !~ /^[ \t]*$/) {
        report("runner-scan-header", ln, s)
        return
      }
      if (op ~ /(src\/|machine\/|memmap\/|cpu\/|common\/|[.][.])/) {
        report("runner-scan-internal-include", ln, op)
        return
      }
      if (exempt) return
      if (op ~ /^ivs\/[A-Za-z0-9_]+[.]h$/) return
      if (substr(s, 1, 1) == "\"" && op == "ivs_refpcm.h") return
      if (substr(s, 1, 1) == "<") {
        if (op in ok) return
      } else if (("runner/" op) in have || (dir "/" op) in have) {
        return
      }
      report("runner-scan-header", ln, op)
    }
    {
      if (acc == "") start = FNR
      line = $0
      if (line ~ /\\$/) {
        acc = acc substr(line, 1, length(line) - 1)
        next
      }
      acc = acc line
      check(strip(acc), start)
      acc = ""
    }
    END { if (acc != "") check(strip(acc), start) }
  ' "$ROOT/$p" >> "$tmp/find"
done < "$tmp/src"

# Feature macros in runner CMake files (a compile definition would bypass the hostio.c rule).
while IFS= read -r p; do
  awk -v file="$p" '
    /^[ \t]*#/ { next }
    {
      s = $0
      while (match(s, /_(POSIX_C_SOURCE|POSIX_SOURCE|XOPEN_SOURCE_EXTENDED|XOPEN_SOURCE|GNU_SOURCE|DEFAULT_SOURCE|BSD_SOURCE|DARWIN_C_SOURCE)/)) {
        printf "runner-scan-posix-define: %s:%d %s\n", file, FNR, substr(s, RSTART, RLENGTH)
        s = substr(s, RSTART + RLENGTH)
      }
    }
  ' "$ROOT/$p" >> "$tmp/find"
done < "$tmp/cmk"

sort -u "$tmp/find" > "$tmp/sorted"
while IFS= read -r l; do
  bad "${l%%: *}" "${l#*: }"
done < "$tmp/sorted"

[ "$rc" -eq 0 ] || exit 1
printf -- '-- runner-scan pass runner-files=%s\n' "$n"

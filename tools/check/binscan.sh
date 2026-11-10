#!/bin/sh
# SPDX-License-Identifier: MIT
# Content scan over FILELIST (CR-01, WR-02). Not a step; called by steps/integrity.sh and
# steps/files.sh. Prints, one per line, each existing regular file that matches the mode,
# skipping paths whose extension is listed in binary-ext.txt (those are governed by name in
# the files step):
#   nul     content contains a NUL byte (a scanner blind spot in a text file)
#   binary  content is not text: contains a NUL byte, or more than 30 percent of its bytes
#           are outside tab, LF, CR and 0x20-0x7E (catches extensionless and .zip-like binaries).
#           Files with a name in text-ext.txt are not examined here: they are covered by the
#           NUL rule of the integrity step.
# Env: ROOT, CHECK_DIR, FILELIST. Names outside [A-Za-z0-9._/-] are skipped (the integrity
# path-charset rule reports them). Exit status 0 even when files are printed; nonzero on
# misuse. Uses cat, tr, cmp and wc only, so it behaves the same under sh and dash.
# Speed: files are checked in batches (one concatenated stream per batch); only a batch that
# is not provably clean is re-checked file by file, so the common case costs a few processes.
LC_ALL=C; export LC_ALL
: "${ROOT:?ROOT is not set}"
: "${CHECK_DIR:?CHECK_DIR is not set}"
: "${FILELIST:?FILELIST is not set}"
[ "$#" -eq 1 ] || { printf 'usage: sh binscan.sh nul|binary\n' >&2; exit 2; }
mode=$1
case $mode in nul|binary) ;; *) printf 'usage: sh binscan.sh nul|binary\n' >&2; exit 2 ;; esac
exts="$CHECK_DIR/binary-ext.txt"
[ -s "$exts" ] || { printf -- '-- FAIL binscan-ext-list-missing: binary-ext.txt\n'; exit 1; }
extlist=" $(grep -v -e '^#' -e '^[[:space:]]*$' "$exts" | tr '\n' ' ')"
textlist=
if [ "$mode" = binary ]; then
  [ -s "$CHECK_DIR/text-ext.txt" ] || { printf -- '-- FAIL binscan-ext-list-missing: text-ext.txt\n'; exit 1; }
  textlist=" $(grep -v -e '^#' -e '^[[:space:]]*$' "$CHECK_DIR/text-ext.txt" | tr '\n' ' ')"
fi
tmp=$(mktemp -d "${TMPDIR:-/tmp}/binscan.XXXXXX") || exit 1
trap 'rm -rf "$tmp"' EXIT
trap 'exit 1' HUP INT TERM

has_nul() { ! tr -d '\000' < "$1" | cmp -s - "$1"; }
# Per-file verdict (the exact rule), used for batches that are not provably clean.
is_hit() {
  if has_nul "$1"; then return 0; fi
  [ "$mode" = binary ] || return 1
  nontext=$(tr -d '\011\012\015\040-\176' < "$1" | wc -c)
  [ "$nontext" -gt 0 ] || return 1
  size=$(wc -c < "$1")
  [ $((nontext * 100)) -gt $((size * 30)) ]
}
# True if every file of batch list $1 (absolute paths) is provably clean in one pass over the
# concatenation. A cat failure (unreadable file) is "not provably clean", so the per-file check runs.
batch_clean() {
  tr '\n' '\000' < "$1" > "$tmp/abs0"
  xargs -0 cat < "$tmp/abs0" > "$tmp/cat.out" 2> /dev/null || return 1
  if [ "$mode" = nul ]; then
    a=$(tr -d '\000' < "$tmp/cat.out" | wc -c)
    b=$(wc -c < "$tmp/cat.out")
    [ "$a" -eq "$b" ]
  else
    a=$(tr -d '\011\012\015\040-\176' < "$tmp/cat.out" | wc -c)
    [ "$a" -eq 0 ]
  fi
}
flush() { # $1 batch list of repo-relative paths
  [ -s "$1" ] || return 0
  awk -v r="$ROOT" '{ print r "/" $0 }' "$1" > "$tmp/abs"
  if batch_clean "$tmp/abs"; then return 0; fi
  while IFS= read -r q; do
    if is_hit "$ROOT/$q"; then printf '%s\n' "$q"; fi
  done < "$1"
}

: > "$tmp/batch"
count=0
while IFS= read -r p || [ -n "$p" ]; do
  case $p in ''|*[!A-Za-z0-9._/-]*) continue ;; esac
  [ -f "$ROOT/$p" ] || continue
  base=${p##*/}
  case $base in
    *.*)
      ext=${base##*.}
      case $ext in *[A-Z]*) ext=$(printf '%s' "$ext" | tr 'A-Z' 'a-z') ;; esac
      case $extlist in *" $ext "*) continue ;; esac
      case $textlist in *" $ext "*) continue ;; esac
      ;;
  esac
  printf '%s\n' "$p" >> "$tmp/batch"
  count=$((count + 1))
  if [ "$count" -ge 50 ]; then flush "$tmp/batch"; : > "$tmp/batch"; count=0; fi
done < "$FILELIST"
flush "$tmp/batch"

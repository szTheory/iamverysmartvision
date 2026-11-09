#!/bin/sh
# SPDX-License-Identifier: MIT
# Content scan over FILELIST (CR-01, WR-02). Not a step; called by steps/integrity.sh and
# steps/files.sh. Prints, one per line, each existing regular file that matches the mode,
# skipping paths whose extension is listed in binary-ext.txt (those are governed by name in
# the files step):
#   nul     content contains a NUL byte (a scanner blind spot in a text file)
#   binary  content is not text: contains a NUL byte, or more than 30 percent of its bytes
#           are outside tab, LF, CR and 0x20-0x7E (catches extensionless and .zip-like binaries)
# Env: ROOT, CHECK_DIR, FILELIST. Names outside [A-Za-z0-9._/-] are skipped (the integrity
# path-charset rule reports them). Exit status 0 even when files are printed; nonzero on
# misuse. Uses tr, cmp and wc only, so it behaves the same under sh and dash.
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
has_nul() { ! tr -d '\000' < "$1" | cmp -s - "$1"; }
while IFS= read -r p || [ -n "$p" ]; do
  case $p in ''|*[!A-Za-z0-9._/-]*) continue ;; esac
  f="$ROOT/$p"
  [ -f "$f" ] || continue
  base=${p##*/}
  case $base in
    *.*)
      ext=${base##*.}
      case $ext in *[A-Z]*) ext=$(printf '%s' "$ext" | tr 'A-Z' 'a-z') ;; esac
      case $extlist in *" $ext "*) continue ;; esac
      ;;
  esac
  if [ "$mode" = nul ]; then
    if has_nul "$f"; then printf '%s\n' "$p"; fi
    continue
  fi
  nontext=$(tr -d '\011\012\015\040-\176' < "$f" | wc -c)
  [ "$nontext" -gt 0 ] || continue
  if has_nul "$f"; then printf '%s\n' "$p"; continue; fi
  size=$(wc -c < "$f")
  if [ $((nontext * 100)) -gt $((size * 30)) ]; then printf '%s\n' "$p"; fi
done < "$FILELIST"

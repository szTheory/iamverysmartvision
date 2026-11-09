#!/bin/sh
# SPDX-License-Identifier: MIT
# Content scan over FILELIST (CR-01): prints, one per line, each existing regular file whose
# content contains a NUL byte, skipping paths whose extension is listed in binary-ext.txt
# (those are governed by the files step). Not a step; called by steps/integrity.sh.
# Env: ROOT, CHECK_DIR, FILELIST. Names outside [A-Za-z0-9._/-] are skipped (the integrity
# path-charset rule reports them). Exit status 0 even when files are printed; nonzero on
# misuse. Uses tr and cmp only, so it behaves the same under sh and dash.
LC_ALL=C; export LC_ALL
: "${ROOT:?ROOT is not set}"
: "${CHECK_DIR:?CHECK_DIR is not set}"
: "${FILELIST:?FILELIST is not set}"
[ "$#" -eq 1 ] && [ "$1" = nul ] || { printf 'usage: sh binscan.sh nul\n' >&2; exit 2; }
exts="$CHECK_DIR/binary-ext.txt"
[ -s "$exts" ] || { printf -- '-- FAIL binscan-ext-list-missing: binary-ext.txt\n'; exit 1; }
extlist=" $(grep -v -e '^#' -e '^[[:space:]]*$' "$exts" | tr '\n' ' ')"
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
  if ! tr -d '\000' < "$f" | cmp -s - "$f"; then printf '%s\n' "$p"; fi
done < "$FILELIST"

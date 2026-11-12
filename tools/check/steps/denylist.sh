#!/bin/sh
# SPDX-License-Identifier: MIT
# Step denylist: provenance scan (FND-05, D-12.7). Findings name the file, never the matched text.
# Scope: every FILELIST path except .planning/, .claude/, docs/provenance/, tools/check/ and the
# exact paths docs/provenance.md and data/sources.json. Every file is scanned as text (grep -a): the integrity step bans NUL bytes in text files.
# denylist.txt entries (author names, licence-header strings) are never waivable.
# denylist-cite.txt entries (emulator project names) are allowed only on a line that carries [cite-only].
# Lists are read from CHECK_DIR, never from ROOT. '#' lines and blank lines in the lists are ignored.
# Tokens: denylist-hit: <file>, denylist-cite-unwaived: <file>, denylist-no-files, denylist-list-missing: <name>,
#   denylist-list-empty: <name>, denylist-grep-error: <file> (grep exit status 2 or more: never read as clean).
# Stats: scanned=<n> waived=<k> (waived = lines with a cite-list name and the [cite-only] marker).
LC_ALL=C; export LC_ALL
: "${ROOT:?ROOT is not set}"
: "${CHECK_DIR:?CHECK_DIR is not set}"
: "${FILELIST:?FILELIST is not set}"
tmp=$(mktemp -d "${TMPDIR:-/tmp}/denylist.XXXXXX") || exit 1
trap 'rm -rf "$tmp"' EXIT
trap 'exit 1' HUP INT TERM
rc=0
bad() { printf -- '-- FAIL %s: %s\n' "$1" "$2"; rc=1; }

# Clean copies of the lists (drop '#' lines and blank lines; a blank pattern would match every line).
for name in denylist denylist-cite; do
  if [ ! -f "$CHECK_DIR/$name.txt" ]; then
    bad denylist-list-missing "$name.txt"
    : > "$tmp/$name"
    continue
  fi
  grep -v -e '^#' -e '^[[:space:]]*$' "$CHECK_DIR/$name.txt" > "$tmp/$name"
  [ -s "$tmp/$name" ] || bad denylist-list-empty "$name.txt"
done

# In-scope files that exist.
: > "$tmp/scan"
while IFS= read -r f; do
  case $f in
    .planning/*|.claude/*|docs/provenance/*|tools/check/*|docs/provenance.md|data/sources.json) continue ;;
  esac
  [ -f "$ROOT/$f" ] && printf '%s\n' "$f" >> "$tmp/scan"
done < "$FILELIST"
scanned=$(grep -c . "$tmp/scan")
if [ "$scanned" -eq 0 ]; then
  bad denylist-no-files "in-scope files"
  exit 1
fi

# Batch prefilter (D-45 lever, plan 04-LEVER-01): one `grep -l` per list over every in-scope file
# (through xargs, in scan order) names the files that match at all, so only those are read again
# line by line below; a file no list matches adds nothing either way. A grep error (status 2 or
# more) in a batch drops back to the per-file pass over every file, which names the unreadable
# file (denylist-grep-error) as before.
batch() { # $1 list file; prints the matching in-scope paths in scan order; status 1 on a grep error
  (cd "$ROOT" && sed 's|^|./|' "$tmp/scan" | tr '\n' '\000' \
    | LIST=$1 xargs -0 sh -c 'grep -l -a -i -F -f "$LIST" "$@" 2> /dev/null; s=$?; [ "$s" -le 1 ] || exit 255' sh) \
    > "$tmp/batch.out" 2> /dev/null || return 1
  sed 's|^[.]/||' "$tmp/batch.out"
}
mode=batch
: > "$tmp/hit.batch"
: > "$tmp/cite.batch"
if [ -s "$tmp/denylist" ]; then batch "$tmp/denylist" > "$tmp/hit.batch" || mode=each; fi
if [ "$mode" = batch ] && [ -s "$tmp/denylist-cite" ]; then
  batch "$tmp/denylist-cite" > "$tmp/cite.batch" || mode=each
fi

waived=0
: > "$tmp/hit"
: > "$tmp/unw"
if [ "$mode" = batch ]; then
  cp "$tmp/hit.batch" "$tmp/hit"
  each_deny=0
  each="$tmp/cite.batch"
else
  each_deny=1
  each="$tmp/scan"
fi
while IFS= read -r f; do
  reported=0
  if [ "$each_deny" -eq 1 ] && [ -s "$tmp/denylist" ]; then
    gs=0
    grep -q -a -i -F -f "$tmp/denylist" "$ROOT/$f" 2> /dev/null || gs=$?
    if [ "$gs" -eq 0 ]; then
      printf '%s\n' "$f" >> "$tmp/hit"
    elif [ "$gs" -ge 2 ]; then
      bad denylist-grep-error "$f"
      reported=1
    fi
  fi
  if [ -s "$tmp/denylist-cite" ]; then
    gs=0
    grep -a -i -F -f "$tmp/denylist-cite" "$ROOT/$f" > "$tmp/lines" 2> /dev/null || gs=$?
    if [ "$gs" -ge 2 ]; then
      [ "$reported" -eq 1 ] || bad denylist-grep-error "$f"
      : > "$tmp/lines"
    elif [ "$gs" -eq 1 ]; then
      : > "$tmp/lines"
    fi
    if grep -a -q -v -F '[cite-only]' "$tmp/lines"; then
      printf '%s\n' "$f" >> "$tmp/unw"
    fi
    k=$(grep -a -c -F '[cite-only]' "$tmp/lines")
    waived=$((waived + k))
  fi
done < "$each"
while IFS= read -r f; do
  bad denylist-hit "$f"
done < "$tmp/hit"
while IFS= read -r f; do
  bad denylist-cite-unwaived "$f"
done < "$tmp/unw"

[ "$rc" -eq 0 ] || exit 1
printf -- '-- denylist pass scanned=%s waived=%s\n' "$scanned" "$waived"

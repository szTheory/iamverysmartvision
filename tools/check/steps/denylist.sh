#!/bin/sh
# SPDX-License-Identifier: MIT
# Step denylist: provenance scan (FND-05, D-12.7). Findings name the file, never the matched text.
# Scope: every FILELIST path except .planning/, .claude/, docs/provenance/, tools/check/ and the
# exact paths docs/provenance.md and data/sources.json. Every file is scanned as text (grep -a): the integrity step bans NUL bytes in text files.
# denylist.txt entries (author names, licence-header strings) are never waivable.
# denylist-cite.txt entries (emulator project names) are allowed only on a line that carries [cite-only].
# Lists are read from CHECK_DIR, never from ROOT. '#' lines and blank lines in the lists are ignored.
# Tokens: denylist-hit: <file>, denylist-cite-unwaived: <file>, denylist-no-files, denylist-list-missing: <name>,
#   denylist-list-empty: <name>.
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

waived=0
: > "$tmp/hit"
: > "$tmp/unw"
while IFS= read -r f; do
  if [ -s "$tmp/denylist" ] && grep -q -a -i -F -f "$tmp/denylist" "$ROOT/$f"; then
    printf '%s\n' "$f" >> "$tmp/hit"
  fi
  if [ -s "$tmp/denylist-cite" ]; then
    grep -a -i -F -f "$tmp/denylist-cite" "$ROOT/$f" > "$tmp/lines" 2>/dev/null || : > "$tmp/lines"
    if grep -a -q -v -F '[cite-only]' "$tmp/lines"; then
      printf '%s\n' "$f" >> "$tmp/unw"
    fi
    k=$(grep -a -c -F '[cite-only]' "$tmp/lines")
    waived=$((waived + k))
  fi
done < "$tmp/scan"
while IFS= read -r f; do
  bad denylist-hit "$f"
done < "$tmp/hit"
while IFS= read -r f; do
  bad denylist-cite-unwaived "$f"
done < "$tmp/unw"

[ "$rc" -eq 0 ] || exit 1
printf -- '-- denylist pass scanned=%s waived=%s\n' "$scanned" "$waived"

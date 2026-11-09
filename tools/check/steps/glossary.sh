#!/bin/sh
# SPDX-License-Identifier: MIT
# Step glossary: time-base glossary inventory, bare-unit tripwire (R2) and the D2 link.
# Reads ROOT, CHECK_DIR and FILELIST only. Findings are reported in sorted order.
# Tokens: glossary-missing-term: <term>, glossary-doc-missing, glossary-inventory-empty,
#   glossary-bare-clock: <file>:<line>, glossary-d2-unlinked.
# Stats: terms=<n> doc-files=<n>.
LC_ALL=C; export LC_ALL
: "${ROOT:?ROOT is not set}"
: "${CHECK_DIR:?CHECK_DIR is not set}"
: "${FILELIST:?FILELIST is not set}"
tmp=$(mktemp -d "${TMPDIR:-/tmp}/glossary.XXXXXX") || exit 1
trap 'rm -rf "$tmp"' EXIT
trap 'exit 1' HUP INT TERM
rc=0
bad() { printf -- '-- FAIL %s: %s\n' "$1" "$2"; rc=1; }

# (a) every inventory term must appear as **<term>** in docs/time-base.md.
doc="$ROOT/docs/time-base.md"
inv="$ROOT/data/expected/glossary-terms.ids"
: > "$tmp/terms"
if [ -f "$inv" ]; then
  awk '/^#/ { next } { sub(/[ \t]+$/, ""); sub(/^[ \t]+/, "") } NF { print }' "$inv" | sort -u > "$tmp/terms"
fi
terms=$(grep -c . "$tmp/terms")
if [ "$terms" -eq 0 ]; then
  bad glossary-inventory-empty "data/expected/glossary-terms.ids"
fi
if [ ! -f "$doc" ]; then
  bad glossary-doc-missing "docs/time-base.md"
else
  : > "$tmp/missing"
  while IFS= read -r term; do
    grep -q -F -e "**$term**" "$doc" || printf -- '-- FAIL glossary-missing-term: %s\n' "$term" >> "$tmp/missing"
  done < "$tmp/terms"
  cat "$tmp/missing"
  [ -s "$tmp/missing" ] && rc=1
fi

# (b) R2 bare-unit tripwire over repository-root README.md and docs/**/*.md only.
: > "$tmp/scan"
grep -E '^README[.]md$|^docs/.*[.]md$' "$FILELIST" | sort -u > "$tmp/cand"
while IFS= read -r f; do
  [ -f "$ROOT/$f" ] && printf '%s\n' "$f" >> "$tmp/scan"
done < "$tmp/cand"
docfiles=$(grep -c . "$tmp/scan")
: > "$tmp/bare"
while IFS= read -r f; do
  grep -n -i -w -E 'clocks?' "$ROOT/$f" \
    | grep -v -i -E 'input clock|STIC clock|PSG chip clock|\[nc\]' \
    | sed -n 's/^\([0-9][0-9]*\):.*/\1/p' \
    | while IFS= read -r n; do printf '%s:%s\n' "$f" "$n"; done >> "$tmp/bare"
done < "$tmp/scan"
sort -t: -k1,1 -k2,2n "$tmp/bare" | while IFS= read -r loc; do
  printf -- '-- FAIL glossary-bare-clock: %s\n' "$loc"
done
[ -s "$tmp/bare" ] && rc=1

# (c) the D2 row of the PROJECT.md Decision Register links docs/time-base.md.
proj="$ROOT/.planning/PROJECT.md"
if [ ! -f "$proj" ] || ! grep -E '^\| D2 \|' "$proj" | grep -q -F 'docs/time-base.md'; then
  bad glossary-d2-unlinked ".planning/PROJECT.md"
fi

[ "$rc" -eq 0 ] || exit 1
printf -- '-- glossary pass terms=%s doc-files=%s\n' "$terms" "$docfiles"

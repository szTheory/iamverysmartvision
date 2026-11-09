#!/bin/sh
# SPDX-License-Identifier: MIT
# Step sources: the reference-document registry (kind source) and SRC-token resolution (R5).
# Every SRC-<slug> token in data/**/*.json (except data/sources.json), docs/**/*.md and README.md
# must be an id in data/expected/sources.ids. Paths are anchored at the repository root, so control
# overlays and good-tree copies under tools/check/controls/ are never scanned on the real tree.
# Tokens: source-* (see kinds/source.cmake and schema/source.txt), source-unknown-ref: <id>.
# Stats: sources=<n> refs-verified=<k>/<hashed count>.
LC_ALL=C; export LC_ALL
: "${ROOT:?ROOT is not set}"
: "${CHECK_DIR:?CHECK_DIR is not set}"
: "${FILELIST:?FILELIST is not set}"
tmp=$(mktemp -d "${TMPDIR:-/tmp}/sources.XXXXXX") || exit 1
trap 'rm -rf "$tmp"' EXIT
trap 'exit 1' HUP INT TERM
rc=0

out=$(cmake -DROOT="$ROOT" -DKIND=source -P "$CHECK_DIR/records.cmake" 2>&1) || rc=1
printf '%s\n' "$out"

# Registered ids come from the inventory (the record check proves registry and inventory are equal).
: > "$tmp/ids"
if [ -f "$ROOT/data/expected/sources.ids" ]; then
  awk '/^#/ { next } NF { print $1 }' "$ROOT/data/expected/sources.ids" > "$tmp/ids"
fi

# Scanned files: anchored repository-root patterns, existing, sorted.
: > "$tmp/scan"
grep -E -e '^data/.*[.]json$' -e '^docs/.*[.]md$' -e '^README[.]md$' "$FILELIST" | grep -v -x -F 'data/sources.json' | sort -u > "$tmp/cand"
while IFS= read -r f; do
  [ -f "$ROOT/$f" ] && printf '%s\n' "$f" >> "$tmp/scan"
done < "$tmp/cand"

: > "$tmp/refs"
while IFS= read -r f; do
  grep -a -o -E 'SRC-[a-z0-9-]+' "$ROOT/$f" >> "$tmp/refs"
done < "$tmp/scan"
sort -u "$tmp/refs" | while IFS= read -r id; do
  grep -q -x -F -e "$id" "$tmp/ids" || printf -- '-- FAIL source-unknown-ref: %s\n' "$id"
done > "$tmp/unknown"
cat "$tmp/unknown"
[ -s "$tmp/unknown" ] && rc=1

[ "$rc" -eq 0 ] || exit 1
stats=$(printf '%s\n' "$out" | sed -n 's/^-- source pass records=[0-9][0-9]* //p')
[ -n "$stats" ] || exit 1
printf -- '-- sources pass %s\n' "$stats"

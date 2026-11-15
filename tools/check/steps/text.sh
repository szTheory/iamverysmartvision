#!/bin/sh
# SPDX-License-Identifier: MIT
# Step text: claim-language tripwire (R1), claim-reference resolution and tier ids documented.
# Scans README.md, CHANGELOG.md and docs/**/*.md from FILELIST only (CHANGELOG.md since plan 08-01, D-32: the
#   release notes are the most public claim text of the project). Findings are reported in sorted order.
# Claim words (D-27): supports, compatible, passes, accurate, verified, runs, plays, playable. The tier identifier
#   playable-guest-progress is an identifier, not a claim word: that exact string is removed from a line before
#   it is matched (the only exemption); any other use of plays or playable needs a claim tag or the waiver.
# Tokens: text-no-files, text-claim-untagged: <file>:<line>, text-unknown-claim-ref: <id>,
#   text-undocumented-tier: <id>, text-claims-inventory-missing, text-evidence-doc-missing.
# Stats: docs=<n> (scanned files, CHANGELOG.md included) claim-refs=<n>.
LC_ALL=C; export LC_ALL
: "${ROOT:?ROOT is not set}"
: "${CHECK_DIR:?CHECK_DIR is not set}"
: "${FILELIST:?FILELIST is not set}"
tmp=$(mktemp -d "${TMPDIR:-/tmp}/text.XXXXXX") || exit 1
trap 'rm -rf "$tmp"' EXIT
trap 'exit 1' HUP INT TERM
bt='`'
rc=0
bad() { printf -- '-- FAIL %s: %s\n' "$1" "$2"; rc=1; }

# Scanned files: README.md, CHANGELOG.md plus docs/**/*.md, as listed in FILELIST, that exist.
: > "$tmp/scan"
grep -E '^(README\.md|CHANGELOG\.md|docs/.*\.md)$' "$FILELIST" | sort -u > "$tmp/cand"
while IFS= read -r f; do
  [ -f "$ROOT/$f" ] && printf '%s\n' "$f" >> "$tmp/scan"
done < "$tmp/cand"
docs=$(grep -c . "$tmp/scan")
if [ "$docs" -eq 0 ]; then
  bad text-no-files "README.md docs/"
  exit 1
fi

# (a) R1 claim-word tripwire: a claim word needs a claim tag or the [nc] waiver on the same line.
# The tier identifier is removed from a line before the match (sed keeps the line numbers: it never joins or drops a
# line). Only the files that contain the identifier pass through sed: one grep -l over all scanned files finds them,
# so the other files cost no extra process.
nl='
'
set --
while IFS= read -r f; do set -- "$@" "$f"; done < "$tmp/scan"
idfiles=$( (cd "$ROOT" && grep -a -l -F -e playable-guest-progress -- "$@") 2> /dev/null || true )
claim_hits() {
  grep -a -n -i -w -E 'supports|compatible|passes|accurate|verified|runs|plays|playable' \
    | grep -a -v -E '\[claim:CL-[0-9]{4}\]|\[nc\]' \
    | sed -n 's/^\([0-9][0-9]*\):.*/\1/p'
}
: > "$tmp/untagged"
while IFS= read -r f; do
  case "$nl$idfiles$nl" in
    *"$nl$f$nl"*) sed 's/playable-guest-progress//g' < "$ROOT/$f" | claim_hits ;;
    *) claim_hits < "$ROOT/$f" ;;
  esac | while IFS= read -r n; do printf '%s:%s\n' "$f" "$n"; done >> "$tmp/untagged"
done < "$tmp/scan"
sort -t: -k1,1 -k2,2n "$tmp/untagged" | while IFS= read -r loc; do
  printf -- '-- FAIL text-claim-untagged: %s\n' "$loc"
done
[ -s "$tmp/untagged" ] && rc=1

# (b) every [claim:<id>] token must be an id in data/expected/claims.ids.
# The literal placeholder CL-NNNN (the documented citation form) is the only exempt token.
: > "$tmp/refs"
while IFS= read -r f; do
  grep -a -o -E '\[claim:[^]]*\]' "$ROOT/$f" | sed 's/^\[claim://; s/\]$//' >> "$tmp/refs"
done < "$tmp/scan"
refs=$(grep -v -x -F -e CL-NNNN "$tmp/refs" | grep -c .)
: > "$tmp/ids"
if [ -f "$ROOT/data/expected/claims.ids" ]; then
  awk '/^#/ { next } NF { print $1 }' "$ROOT/data/expected/claims.ids" > "$tmp/ids"
else
  bad text-claims-inventory-missing "data/expected/claims.ids"
fi
sort -u "$tmp/refs" | while IFS= read -r id; do
  [ "$id" = "CL-NNNN" ] && continue
  grep -q -x -F -e "$id" "$tmp/ids" || printf -- '-- FAIL text-unknown-claim-ref: %s\n' "$id"
done > "$tmp/unknown"
cat "$tmp/unknown"
[ -s "$tmp/unknown" ] && rc=1

# (c) every source-authority tag id and evidence tier id appears backticked in docs/evidence-tiers.md.
if [ ! -f "$ROOT/docs/evidence-tiers.md" ]; then
  bad text-evidence-doc-missing "docs/evidence-tiers.md"
else
  for inv in source-authority-tags evidence-tiers; do
    [ -f "$ROOT/data/expected/$inv.ids" ] || { bad text-undocumented-tier "data/expected/$inv.ids"; continue; }
    awk '/^#/ { next } NF { print $1 }' "$ROOT/data/expected/$inv.ids" | while IFS= read -r id; do
      grep -q -F -e "$bt$id$bt" "$ROOT/docs/evidence-tiers.md" || printf -- '-- FAIL text-undocumented-tier: %s\n' "$id"
    done
  done > "$tmp/undoc"
  cat "$tmp/undoc"
  [ -s "$tmp/undoc" ] && rc=1
fi

[ "$rc" -eq 0 ] || exit 1
printf -- '-- text pass docs=%s claim-refs=%s\n' "$docs" "$refs"

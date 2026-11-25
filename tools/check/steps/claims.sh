#!/bin/sh
# SPDX-License-Identifier: MIT
# Step claims: the claim ledger (kind claim) and the handoff ledger's grammar (EV-05).
# The handoff ledger data/expected/handoffs.ids holds one row per human-only item, four tokens:
#   <id> <handed-off|not-performed> evidence: none
# A MISSING ledger is an empty set, not a missing inventory (the toy control tree has none), so the
# read is guarded on existence; a ledger that exists with no row is refused, because a ledger that
# records nothing is a different fact from a ledger that is not there. The rows are read directly:
# the single-column inventory reader (tools/check/lib.cmake:75-101) appends whole lines, so it
# cannot parse a four-token file.
# Tokens: claim-handoff-row: line <n> (a row that is not the frozen grammar),
#   claim-handoff-row: no rows (a present ledger with nothing in it).
LC_ALL=C; export LC_ALL
: "${ROOT:?ROOT is not set}"
: "${CHECK_DIR:?CHECK_DIR is not set}"
rc=0
out=$(cmake -DROOT="$ROOT" -DKIND=claim -P "$CHECK_DIR/records.cmake" 2>&1) || rc=1
printf '%s\n' "$out"
[ "$rc" -eq 0 ] || exit 1
stats=$(printf '%s\n' "$out" | sed -n 's/^-- claim pass records=[0-9][0-9]* //p')
[ -n "$stats" ] || exit 1

hb=$(mktemp "${TMPDIR:-/tmp}/claims.XXXXXX") || exit 1
trap 'rm -f "$hb"' EXIT
trap 'exit 1' HUP INT TERM
if [ -f "$ROOT/data/expected/handoffs.ids" ]; then
  awk '
    /^[[:space:]]*#/ { next }
    NF == 0 { next }
    {
      rows++
      if (NF != 4) { print "-- FAIL claim-handoff-row: line " NR; next }
      if ($2 != "handed-off" && $2 != "not-performed") {
        print "-- FAIL claim-handoff-row: line " NR; next
      }
      if ($3 != "evidence:" || $4 != "none") {
        print "-- FAIL claim-handoff-row: line " NR
      }
    }
    END { if (rows == 0) print "-- FAIL claim-handoff-row: no rows" }
  ' "$ROOT/data/expected/handoffs.ids" > "$hb"
  cat "$hb"
  [ -s "$hb" ] && exit 1
fi
printf -- '-- claims pass %s\n' "$stats"

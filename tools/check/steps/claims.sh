#!/bin/sh
# SPDX-License-Identifier: MIT
# Step claims: the claim ledger (kind claim) and the handoff ledger's grammar (EV-05).
# The handoff ledger data/expected/handoffs.ids holds one row per human-only item, four tokens:
#   <id> <handed-off|not-performed> evidence: none
# The ledger is REQUIRED (EV-05): it is this phase's deliverable, so its absence is a refusal, not
# an empty set, and a path that exists without being a readable regular file is refused with its
# own detail. Its row set is pinned too, in both directions: the id set is a property of the
# project, not of the tree, so it lives here rather than in the file's own header, which a deletion
# would take with it. The rows are read directly: the single-column inventory reader
# (tools/check/lib.cmake:75-101) appends whole lines, so it cannot parse a four-token file.
# Tokens: claim-handoff-row with a typed detail, the convention this file already uses for the
# grammar (`line <n>`, `no rows`): `no ledger` (the file is absent), `unreadable` (it exists but is
# not a readable regular file), `unknown id <id>` (a row the project does not have), `missing id
# <id>` (a project id with no row). One token with a detail keeps a control's EXPECT readable while
# each case stays distinguishable.
# The pass line also carries `handoffs=<n>`, the ledger's row count, counted from the file on every
# run so no record has to quote a figure that can go stale (the project's own rule: recompute,
# never quote). It is a STAT for a reader, not a check, and it has no control on purpose: the
# mechanism that keeps the ledger honest is the pair of pinned lists below, where a missing row, an
# extra row, a duplicate and a wrong disposition are each a named finding in both directions. A
# control of the count would prove the count rather than the ledger, and GUARDRAILS 2.4 asks a new
# control only for a new check - this step adds none.
LC_ALL=C; export LC_ALL
: "${ROOT:?ROOT is not set}"
: "${CHECK_DIR:?CHECK_DIR is not set}"
rc=0
out=$(cmake -DROOT="$ROOT" -DKIND=claim -P "$CHECK_DIR/records.cmake" 2>&1) || rc=1
printf '%s\n' "$out"
[ "$rc" -eq 0 ] || exit 1
stats=$(printf '%s\n' "$out" | sed -n 's/^-- claim pass records=[0-9][0-9]* //p')
[ -n "$stats" ] || exit 1

# The eight ids the project's own records already use: an H-id of a phase verification record or a
# milestone close record, or a D-id owner gate. Frozen here so the set cannot be edited away
# together with the ledger. Phase 16 added H16-1, H16-2 and H16-3, the three owner-only questions
# that phase offered as rows.
HANDOFF_IDS='H7-1 H7-2 H8-3 D10 D11 H16-1 H16-2 H16-3'
# The disposition each id carries, pinned here for the same reason the id set is: a row with the
# right id and the wrong disposition reads as a performed handoff that nobody performed, and the
# ledger is the only place a later milestone looks. Edit both lists together or not at all.
HANDOFF_DISP='H7-1=handed-off H7-2=handed-off H8-3=handed-off D10=not-performed D11=not-performed H16-1=handed-off H16-2=handed-off H16-3=handed-off'
hled="$ROOT/data/expected/handoffs.ids"

hb=$(mktemp "${TMPDIR:-/tmp}/claims.XXXXXX") || exit 1
trap 'rm -f "$hb"' EXIT
trap 'exit 1' HUP INT TERM
if [ ! -e "$hled" ]; then
  printf -- '-- FAIL claim-handoff-row: no ledger (data/expected/handoffs.ids)\n'
  exit 1
fi
if [ ! -f "$hled" ] || [ ! -r "$hled" ]; then
  printf -- '-- FAIL claim-handoff-row: unreadable (data/expected/handoffs.ids)\n'
  exit 1
fi
if ! awk '
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
  ' "$hled" > "$hb"; then
  cat "$hb"
  printf -- '-- FAIL claim-handoff-row: unreadable (data/expected/handoffs.ids)\n'
  exit 1
fi
cat "$hb"
[ -s "$hb" ] && exit 1
# The row set, both directions, only once the grammar is clean: a malformed row already fails the
# step, and one finding per ledger is enough to refuse it.
if ! awk -v want="$HANDOFF_IDS" -v disp="$HANDOFF_DISP" '
    BEGIN {
      n = split(want, w, " "); for (i = 1; i <= n; i++) pin[w[i]] = 1
      m = split(disp, d, " "); for (i = 1; i <= m; i++) { split(d[i], kv, "="); want_disp[kv[1]] = kv[2] }
    }
    /^[[:space:]]*#/ { next }
    NF == 0 { next }
    {
      if ($1 in seen) { print "-- FAIL claim-handoff-row: duplicate id " $1; next }
      if (!($1 in seen)) ord[++nr] = $1; seen[$1] = 1
      if (($1 in want_disp) && $2 != want_disp[$1]) {
        print "-- FAIL claim-handoff-row: disposition " $1 " is " $2 ", pinned " want_disp[$1]
      }
    }
    END {
      for (i = 1; i <= nr; i++) if (!(ord[i] in pin)) print "-- FAIL claim-handoff-row: unknown id " ord[i]
      for (i = 1; i <= n; i++) if (!(w[i] in seen)) print "-- FAIL claim-handoff-row: missing id " w[i]
    }
  ' "$hled" > "$hb"; then
  cat "$hb"
  printf -- '-- FAIL claim-handoff-row: unreadable (data/expected/handoffs.ids)\n'
  exit 1
fi
cat "$hb"
[ -s "$hb" ] && exit 1
# The row count is the file's, read with the same predicate the grammar pass uses for a row (a
# non-comment line with fields), and appended after the hook's own stats rather than inserted into
# the captured string, so the hook's token order stays byte-identical.
handoff_rows=$(awk '/^[[:space:]]*#/ { next } NF == 0 { next } { n++ } END { print n + 0 }' "$hled")
printf -- '-- claims pass %s handoffs=%s\n' "$stats" "$handoff_rows"

#!/bin/sh
# SPDX-License-Identifier: MIT
# Step important-gates: the machine-checked form of RUN-04 (D-46, plan 04-24). Pure checker: reads
# only $ROOT/data/expected/, never builds or runs anything, so the control runner can drive it.
# An important gate is one whose silent pass would let a requirement, claim or evidence tier be
# asserted without evidence: it is the sole evidence for a requirement or claim, it can pass
# vacuously (inventory, digest or golden comparison, outcome classifier, refusal/no-state-change
# check, verdict over producer output), or its failure class escaped once. Each such gate has a
# named mechanism that is shown to fail for its intended reason elsewhere in the gate; this step
# checks that the registry names only mechanisms that exist and covers every Phase 4 requirement.
# Registry: data/expected/important-gates.ids, '#' lines and blank lines skipped, then one line
#   <gate> <fault|control|mutant> <id> <REQ>
# per (gate, mechanism, id, requirement). <gate> is a free name of lower-case letters, digits and
# '-'; <REQ> is one of the Phase 4 requirements API-02 API-04 RUN-01 RUN-02 RUN-04 RUN-05 RUN-06
# PKG-01 PKG-03 PKG-04.
# Resolution of <id> by mechanism:
#   fault    first column of any data/expected/faults/*.ids (judged by faults-verdict);
#   control  first column of any data/expected/controls/*.ids (run by tools/check/run-controls.sh);
#   mutant   a case of data/expected/pkg-selftests.ids or data/expected/ci-summary-selftests.ids
#            (in-step selftests), or a test id `<suite>.judge.<slug>` or `<suite>.mutant.<slug>`
#            of any data/expected/tests/*.ids (suite self-controls, judged by tests-inventory).
# Tokens: important-gates-unknown-id: line <n> (not 4 columns, unknown mechanism, bad gate name,
#   or a requirement outside the Phase 4 set) or <mechanism> <id> line <n> (the id is in no
#   inventory of its mechanism); important-gates-uncovered-gate: <REQ> (a Phase 4 requirement
#   with no registry line; also the registry file missing).
# Stats: gates=<distinct gate names> lines=<registry lines>.
LC_ALL=C; export LC_ALL
: "${ROOT:?ROOT is not set}"
: "${CHECK_DIR:?CHECK_DIR is not set}"
: "${FILELIST:?FILELIST is not set}"
tmp=$(mktemp -d "${TMPDIR:-/tmp}/important-gates.XXXXXX") || exit 1
trap 'rm -rf "$tmp"' EXIT
trap 'exit 1' HUP INT TERM
find="$tmp/findings"; : > "$find"
exp="$ROOT/data/expected"
reg="$exp/important-gates.ids"

# The three inventories, one `<mechanism> <id>` line each.
: > "$tmp/known"
for f in "$exp"/faults/*.ids; do
  [ -f "$f" ] || continue
  awk '/^#/ { next } NF { print "fault", $1 }' "$f" >> "$tmp/known"
done
for f in "$exp"/controls/*.ids; do
  [ -f "$f" ] || continue
  awk '/^#/ { next } NF { print "control", $1 }' "$f" >> "$tmp/known"
done
for f in "$exp/pkg-selftests.ids" "$exp/ci-summary-selftests.ids"; do
  [ -f "$f" ] || continue
  awk '/^#/ { next } NF { print "mutant", $1 }' "$f" >> "$tmp/known"
done
for f in "$exp"/tests/*.ids; do
  [ -f "$f" ] || continue
  awk '/^#/ { next } $1 ~ /^[a-z0-9-]+[.](judge|mutant)[.][^ ]+$/ { print "mutant", $1 }' "$f" >> "$tmp/known"
done

if [ ! -f "$reg" ]; then
  printf 'important-gates-uncovered-gate %s\n' "data/expected/important-gates.ids missing" >> "$find"
  : > "$tmp/reg"
else
  cp "$reg" "$tmp/reg"
fi

awk -v knownf="$tmp/known" -v find="$find" -v stats="$tmp/stats" '
  function bad(t, d) { printf "%s %s\n", t, d >> find }
  BEGIN {
    while ((getline l < knownf) > 0) known[l] = 1
    close(knownf)
    nreq = split("API-02 API-04 RUN-01 RUN-02 RUN-04 RUN-05 RUN-06 PKG-01 PKG-03 PKG-04", reqs, " ")
    for (i = 1; i <= nreq; i++) phase4[reqs[i]] = 1
    lines = 0; ngates = 0
  }
  /^#/ { next }
  NF == 0 { next }
  {
    lines++
    if (NF != 4 || $1 !~ /^[a-z0-9][a-z0-9-]*$/ || ($2 != "fault" && $2 != "control" && $2 != "mutant") \
        || !($4 in phase4)) {
      bad("important-gates-unknown-id", "line " FNR)
      next
    }
    if (!(($2 " " $3) in known)) { bad("important-gates-unknown-id", $2 " " $3 " line " FNR); next }
    if (!($1 in gates)) { gates[$1] = 1; ngates++ }
    covered[$4] = 1
  }
  END {
    for (i = 1; i <= nreq; i++) if (!(reqs[i] in covered)) bad("important-gates-uncovered-gate", reqs[i])
    print "gates=" ngates " lines=" lines > stats
  }
' "$tmp/reg" || printf 'important-gates-unknown-id %s\n' "awk error" >> "$find"

if [ -s "$find" ]; then
  sort -u "$find" | while IFS= read -r l; do printf -- '-- FAIL %s: %s\n' "${l%% *}" "${l#* }"; done
  exit 1
fi
printf -- '-- important-gates pass %s\n' "$(cat "$tmp/stats")"

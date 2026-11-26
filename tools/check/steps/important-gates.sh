#!/bin/sh
# SPDX-License-Identifier: MIT
# Step important-gates: the machine-checked form of RUN-04 (D-46, plan 04-24). Pure checker: reads
# only $ROOT/data/expected/ and the requirement records under $ROOT/.planning/, never builds or runs
# anything, so the control runner can drive it. Reading .planning/ is not new for a preflight step:
# ci-log reads $ROOT/.planning/ci-runs.tsv (ci-log.sh:24) and glossary reads
# $ROOT/.planning/PROJECT.md (glossary.sh:64-66), and both stay pure checkers.
# An important gate is one whose silent pass would let a requirement, claim or evidence tier be
# asserted without evidence: it is the sole evidence for a requirement or claim, it can pass
# vacuously (inventory, digest or golden comparison, outcome classifier, refusal/no-state-change
# check, verdict over producer output), or its failure class escaped once. Each such gate has a
# named mechanism that is shown to fail for its intended reason elsewhere in the gate; this step
# checks that the registry names only mechanisms that exist and covers every accepted requirement;
# the phase table below is the whole list, so no sentence here enumerates a phase set that can
# silently omit one.
# Registry: data/expected/important-gates.ids, '#' lines and blank lines skipped, then one line
#   <gate> <fault|control|mutant> <id> <REQ>
# per (gate, mechanism, id, requirement). <gate> is a free name of lower-case letters, digits and
# '-'; <REQ> is one of the accepted requirements in the table below.
# Accepted requirements are DERIVED, never hand-written (plan 09-05, EV-04, amendment A-01): every
# row of the '## Traceability' table ('| <REQ> | Phase <n> | <Status> |', id from field 2, status
# from field 4, 'Complete' as a prefix match) of $ROOT/.planning/REQUIREMENTS.md and of every
# $ROOT/.planning/milestones/*-REQUIREMENTS.md, minus the ids listed in
# data/expected/important-gates-excluded.ids. The '- [x] **REQ-NN**:' checkbox lines are NOT parsed:
# the traceability table is the record the project treats as authoritative. The exclusion set is a
# different file from the registry, so deleting a registry line cannot delete the requirement from
# the must-be-covered set; a missing ledger file is an empty exclusion set, which is safe because
# the twelve requirements it lists then fail important-gates-complete-uncovered.
# The ledger's own row format is validated in the same pass (review WR-01): a row must be exactly
# '<REQ> <reason>', with <REQ> shaped like a requirement id and <reason> at least three
# underscore-joined words; 'none' is refused. A row that fails is reported as
# important-gates-excluded-malformed and does NOT enter the exclusion set, so it can neither
# silence a requirement nor pass as a recorded exclusion.
# The accepted set is also the must-be-covered set, so a plan that accepts a requirement also
# appends one toy line for it to the toy registry, to every control that overlays a whole-file toy
# registry, and one row for it to the two toy requirement records of the good tree.
# The phase table below records which plan added which set. It is history for the reviewer, not the
# definition, so it cannot silently omit a requirement.
#   Phase 1  FND-01 FND-02 FND-03 FND-04 FND-05 FND-06 FND-07                      plan 09-04
#   Phase 2  CPU-01 CPU-02 CPU-03 CPU-04 CPU-05                                    plan 09-04
#   Phase 3  MEM-01 MEM-02 MEM-03 MEM-04 MEM-05                                    plan 09-04
#   Phase 4  API-02 API-04 RUN-01 RUN-02 RUN-04 RUN-05 RUN-06 PKG-01 PKG-03 PKG-04  plan 04-24
#   Phase 5  INP-01 INP-02 INP-03 API-03 AUD-01 AUD-02                             plan 05-12
#   Phase 6  VID-01 VID-02 VID-03 VID-04 VID-06                                    plan 06-14
#   Phase 7  SCN-01 plan 07-02; SCN-02 plan 07-10 (A-13: SCN-02 is the owner-local EXEC/GROM route,
#            and its mechanisms are the two user-content consistency controls of the golden step)
#   Phase 8  VID-05 plan 08-08 (the MOB, collision and delay faults, the four frame judges, the six
#            program mutants of ivs-diag-stic-2 and the four frame-grammar controls); STA-01 STA-02
#            plan 08-17 (the nine snapshot and restore faults, the six judges of the state suite and
#            the seven controls of the state step); PRF-01 plan 08-19 (both test ids of the suite
#            bench: the refusal of a mutated golden and the judge of the absence-of-measurement check)
#   Phase 10 EV-05 plan 10-06 (the three controls of the handoff ledger and of the look/listen prose:
#            the two claims controls and the one text control, the first rows either gate name has had)
#   Phase 11 TOOL-01 plan 11-05 (the runner-record gate: the two input-log faults, the writer's dropped
#            event line and the replay's one-tick shift, and the two judge self-controls of the
#            recorded-and-replayed comparison; the accepted set is derived, so the row's nreq limb is
#            gone and this table and the token prose are the only parts a new requirement retargets)
#   Phase 13 RTS-01 RTS-02 EV-06 EV-07 plan 13-04 (the rights ledger's two limbs under two new gate
#            names, the claim ceiling's two rules, and the ten D-09-06 registrations: five control
#            rows, four mutant rows resolved through the new data/expected/runner-selftest.ids, and
#            one new control directory for important-gates-excluded-stale; zero new faults)
#   Phase 14 TOOL-07 SCN-03 CPU-06 plan 14-04 (three new gate names: flatwrap's one control row
#            (c-build-unlisted-flatwrap-source) and one mutant row (pack.judge.flatwrap-digest-live),
#            cart-role's two mutant rows (runner-process.judge.flatwrap-roundtrip and
#            runner-process.judge.owner-run-cart-pass), and cpu06's one pre-existing fault row
#            (ACCEPT_CLEARS_INTFF_IGNORED); five rows, three gate names, zero new faults. The rows
#            and the three requirement flips land in one commit because this step refuses a row
#            naming a Pending requirement and refuses a Complete requirement with no row, so the
#            two directions move together; the 2.4 debt plan 14-01 recorded closes here)
#   Phase 15 TOOL-08 plan 15-03 (one new gate name `host` and its two `mutant` rows: the digest
#            equality's two must-fail limbs as the committed runner-process judge self-controls
#            runner-process.judge.host-vacuous (refused for frames-zero) and
#            runner-process.judge.host-digest-diff (refused for digest-diff); two rows, one gate
#            name, zero new faults, zero new steps; the rows and the TOOL-08 flip land in one commit
#            for the same two-direction reason as Phase 14, and all ten toy registries gain their
#            toy-phase15 line in that commit; plan 15-04, the code review's CR-01, WR-01 and WR-02, adds
#            three more rows under the same `host` gate name - the bounded input schedule's refusal with
#            its long-run witness pair, the refused load's rollback with its retry, and the judge
#            self-control's two-limb guard - five rows in that gate, one gate name, zero new faults,
#            zero new steps, and no toy line because no requirement is accepted here)
#   Phase 16 PKG-06 plan 16-04 (one new gate name `release-posture` and its three rows: two `control`
#            rows naming the committed files-unlisted-binary-so and files-unlisted-binary-dylib
#            controls, which refuse a shared object by name in the tree, and one `mutant` row naming
#            the committed pkg-install selftest host-so-absent, which refuses the object in a copy of
#            the installed prefix; three rows, one gate name, zero new faults, zero new steps. Every
#            row names a mechanism plan 15-02 already built, so the registration is a pointer; the
#            rows and the PKG-06 flip land in one commit for the same two-direction reason as Phases
#            14 and 15, and all ten toy registries and the three toy requirement records gain their
#            PKG-96 line in that commit. Plan 16-04's own committed-direction probe - a tracked
#            probe.so refused as files-unlisted-binary with probe.txt at the same place not refused -
#            is not a row here: it is a throwaway check on the real tree, not a committed mechanism)
#   Phase 17 EV-08 plan 17-02 (the flatwrap-to-shim join, one row under the existing `cart-role` gate name:
#            a `mutant` row naming the committed runner-process id runner-process.judge.flatwrap-shim, whose
#            five limbs join the harness's digest.images to the runner's and the runner's slot.cart.sha256 to
#            the digest the wrap tool printed, move both values with one flipped input byte, and refuse the
#            committed stand-in that replays the first container against the second. The case is a new id in
#            an existing suite, not a new step: zero new faults, zero new steps, and no new gate name. The row
#            and the EV-08 flip land in one commit for the same two-direction reason as Phases 14 to 16, and
#            all ten toy registries and the three toy requirement records gain their EV-97 line in that commit)
#   Phase 17 EV-08 plan 17-03 (the count-gate contract, PLAN-LESSONS lesson 32, clause 1: one `mutant` row under
#            one new gate name `count-gate`, over the existing `extra-file` case of the `pkg-install` step, whose
#            count comes from the named file data/expected/pkg-install-files.ids and whose committed stand-in the
#            same check_inventory refuses on every gate run. One row, one gate name, zero new cases, zero new
#            test ids, zero new steps and zero new faults: the row points at a mechanism plan 04-22 already
#            built, so no 0.05 selftest price is incurred and the phase's priced line stays 0.11. The gate name
#            is new because the row registers a contract rather than a step, and it is what makes `gates` read
#            120. Lesson 32's clause 3 is the already-registered st-pass-line row, cited not duplicated, and
#            clause 2 has no mechanism and is not registered at all)
# Resolution of <id> by mechanism:
#   fault    first column of any data/expected/faults/*.ids (judged by faults-verdict);
#   control  first column of any data/expected/controls/*.ids (run by tools/check/run-controls.sh);
#   mutant   a case of data/expected/pkg-selftests.ids, data/expected/ci-summary-selftests.ids or
#            data/expected/runner-selftest.ids (in-step selftests), or a test id `<suite>.judge.<slug>`
#            or `<suite>.mutant.<slug>` of any data/expected/tests/*.ids (suite self-controls, judged by
#            tests-inventory).
# Tokens: important-gates-unknown-id: line <n> (not 4 columns, unknown mechanism, bad gate name,
#   or a requirement outside the derived accepted set) or <mechanism> <id> line <n> (the id is in no
#   inventory of its mechanism); important-gates-complete-uncovered: <REQ> (a requirement marked
#   Complete in a record with neither a registry line nor a row in the exclusion ledger);
#   important-gates-excluded-stale: <REQ> (an exclusion ledger row whose requirement is not marked
#   Complete); important-gates-uncovered-gate: <REQ> (an accepted requirement with no registry line;
#   also the registry file missing); important-gates-excluded-malformed: line <n>: <row> (a ledger
#   row that is not exactly '<REQ> <reason>' with <REQ> shaped like a requirement id and <reason> at
#   least three underscore-joined words, or that gives 'none' as the reason).
# A Complete requirement that is neither registered nor excluded emits BOTH
# important-gates-complete-uncovered and important-gates-uncovered-gate, because the derived accepted
# set is also the must-be-covered set. important-gates-complete-uncovered is authoritative in that
# pair: it states WHY (marked Complete, with neither a registry line nor a recorded exclusion), while
# important-gates-uncovered-gate keeps its generic meaning (it also fires when the registry file is
# missing) and is what the earlier req-uncovered controls pin. Neither is renamed or dropped, and a
# control that forbids the second while pinning the first would be unsatisfiable.
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

# The derived must-be-covered set: the Complete ids of the requirement records, minus the ledger.
: > "$tmp/complete"
for f in "$ROOT/.planning/REQUIREMENTS.md" "$ROOT"/.planning/milestones/*-REQUIREMENTS.md; do
  [ -f "$f" ] || continue
  awk -F'|' '/^\| *[A-Z]+-[0-9]+ *\|/ {
      id = $2; st = $4
      gsub(/^ +| +$/, "", id); gsub(/^ +| +$/, "", st)
      if (st ~ /^Complete/) print id
    }' "$f" >> "$tmp/complete"
done
sort -u "$tmp/complete" -o "$tmp/complete"
: > "$tmp/excluded"
: > "$tmp/excl.bad"
if [ -f "$exp/important-gates-excluded.ids" ]; then
  # The ledger is the one file that can remove a requirement from the must-be-covered set, so its
  # declared row format is enforced here and a malformed row contributes nothing (review WR-01).
  # The reason is words joined by underscores, at least three of them: two or more underscores
  # with no empty word, so a leading, trailing or doubled underscore is malformed too.
  awk '
    /^#/ { next }
    NF == 0 { next }
    {
      words = gsub(/_/, "_", $2) + 1
      if (NF != 2 || $1 !~ /^[A-Z][A-Z0-9]*-[0-9][0-9]*$/ || $2 == "none" \
          || words < 3 || $2 ~ /__/ || $2 ~ /^_/ || $2 ~ /_$/) {
        print "MALFORMED line " FNR ": " $0
        next
      }
      print $1
    }
  ' "$exp/important-gates-excluded.ids" > "$tmp/excl.raw"
  grep -v -e '^MALFORMED ' "$tmp/excl.raw" > "$tmp/excluded" || :
  grep -e '^MALFORMED ' "$tmp/excl.raw" >> "$tmp/excl.bad" || :
  while IFS= read -r l; do
    printf 'important-gates-excluded-malformed %s\n' "${l#MALFORMED }" >> "$find"
  done < "$tmp/excl.bad"
fi

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
for f in "$exp/pkg-selftests.ids" "$exp/ci-summary-selftests.ids" "$exp/runner-selftest.ids"; do
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

awk -v knownf="$tmp/known" -v find="$find" -v stats="$tmp/stats" -v reqf="$tmp/complete" \
    -v excf="$tmp/excluded" '
  function bad(t, d) { printf "%s %s\n", t, d >> find }
  BEGIN {
    while ((getline l < knownf) > 0) known[l] = 1
    close(knownf)
    while ((getline l < reqf) > 0) if (l != "") { complete[l] = 1; reqs[++nreq] = l }
    close(reqf)
    while ((getline l < excf) > 0) if (l != "") excluded[l] = 1
    close(excf)
    for (i = 1; i <= nreq; i++) if (!(reqs[i] in excluded)) accepted[reqs[i]] = 1
    lines = 0; ngates = 0
  }
  /^#/ { next }
  NF == 0 { next }
  {
    lines++
    if (NF != 4 || $1 !~ /^[a-z0-9][a-z0-9-]*$/ || ($2 != "fault" && $2 != "control" && $2 != "mutant") \
        || !($4 in accepted)) {
      bad("important-gates-unknown-id", "line " FNR)
      next
    }
    if (!(($2 " " $3) in known)) { bad("important-gates-unknown-id", $2 " " $3 " line " FNR); next }
    if (!($1 in gates)) { gates[$1] = 1; ngates++ }
    covered[$4] = 1
  }
  END {
    for (i = 1; i <= nreq; i++) if (!(reqs[i] in covered) && !(reqs[i] in excluded)) \
      bad("important-gates-complete-uncovered", reqs[i])
    for (x in excluded) if (!(x in complete)) bad("important-gates-excluded-stale", x)
    for (i = 1; i <= nreq; i++) if (!(reqs[i] in covered) && !(reqs[i] in excluded)) \
      bad("important-gates-uncovered-gate", reqs[i])
    print "gates=" ngates " lines=" lines > stats
  }
' "$tmp/reg" || printf 'important-gates-unknown-id %s\n' "awk error" >> "$find"

if [ -s "$find" ]; then
  sort -u "$find" | while IFS= read -r l; do printf -- '-- FAIL %s: %s\n' "${l%% *}" "${l#* }"; done
  exit 1
fi
printf -- '-- important-gates pass %s\n' "$(cat "$tmp/stats")"

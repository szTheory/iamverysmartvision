---
phase: 09-registered-evidence-and-gate-cost
plan: 05
subsystem: gate-evidence
tags: [A2, EV-04, A-01, A-08, derived-accepted-set, exclusion-ledger, toy-records, control, D-09-05, D-09-06, T-09-09, T-09-10]
requirements-completed: [EV-04]
dependency-graph:
  requires: [09-04]
  provides: [derived-must-be-covered-set, important-gates-excluded.ids, important-gates-complete-uncovered, toy-requirement-records]
  affects: [09-06, 09-07, 09-09]
tech-stack:
  added: []
  patterns: [derived-not-hand-written-accepted-set, two-directional-exclusion-ledger, minimal-toy-records]
key-files:
  created:
    - data/expected/important-gates-excluded.ids
    - tools/check/controls/_good/tree/.planning/REQUIREMENTS.md
    - tools/check/controls/_good/tree/.planning/milestones/v0.1.0-REQUIREMENTS.md
    - tools/check/controls/_good/tree/data/expected/important-gates-excluded.ids
    - tools/check/controls/important-gates-complete-uncovered/CHECK
    - tools/check/controls/important-gates-complete-uncovered/EXPECT
    - tools/check/controls/important-gates-complete-uncovered/FORBID
    - tools/check/controls/important-gates-complete-uncovered/overlay/.planning/REQUIREMENTS.md
    - tools/check/controls/important-gates-complete-uncovered/overlay/data/expected/important-gates-excluded.ids
  modified:
    - tools/check/steps/important-gates.sh
    - data/expected/controls/important-gates.ids
decisions:
  - "The must-be-covered set is derived from the requirement records, never hand-written; the exclusion ledger is a different file from the registry, so deleting a registry line re-opens a refusal instead of shrinking coverage silently"
  - "important-gates-complete-uncovered is authoritative over the co-firing important-gates-uncovered-gate; both fire by design and neither is renamed"
  - "A seventh exclusion row (EV-02) was required by fact, not by the plan: 09-04 marked EV-02 Complete and the derivation reads the live record, so a six-row ledger fires on the pristine tree"
metrics:
  duration: 47m
  completed: 2026-11-25
  tasks: 2
  files: 11
  commits: 2
  plan_head_before: b27d592129f69465b4bfe0272aba8938883d1584
  plan_head_after: 5f9d25bf44b6136fdefd2cd1205883458cc1046a
status: complete
actuals:
  tokens: 5560
  tasks: 2
  commits: 2
---

# Phase 9 Plan 05: The derived must-be-covered set and the exclusion ledger Summary

The important-gates step stops carrying a hand-written accepted set and derives the must-be-covered
set from the `## Traceability` tables of `.planning/REQUIREMENTS.md` and
`.planning/milestones/v0.1.0-REQUIREMENTS.md`, minus a seven-row exclusion ledger, so a deleted
registry line is now a refusal (`important-gates-complete-uncovered`) instead of a silence, and both
directions of the ledger's contract are shown able to fail by one committed control.

## What landed

| Artifact | Change |
|---|---|
| `tools/check/steps/important-gates.sh` | `nreq = split(...)` and the hand-written `accepted[]` build deleted; `accepted[]` built in the same awk program from `$tmp/complete` minus `$tmp/excluded`; two refusals added; purity note amended |
| `data/expected/important-gates-excluded.ids` | the exclusion ledger: docstring (owning plan, grammar, two categories, two-directional rule, no `#allow-empty`), one comment per group, seven `<REQ> <reason>` rows |
| `tools/check/controls/_good/tree/.planning/REQUIREMENTS.md` and `.../milestones/v0.1.0-REQUIREMENTS.md` | minimal toy records: a `## Traceability` table listing exactly the 44 ids the toy registry covers, every one `Complete` |
| `tools/check/controls/_good/tree/data/expected/important-gates-excluded.ids` | toy ledger, empty, with `#allow-empty: the toy registry covers every requirement its records mark Complete` |
| `tools/check/controls/important-gates-complete-uncovered/` | CHECK / EXPECT / FORBID plus two whole-file overlays |
| `data/expected/controls/important-gates.ids` | one inventory line plus its plan/decision comment |

## The derivation, measured

Parse rule exactly as A-01 states: `^\| *[A-Z]+-[0-9]+ *\|`, id from field 2, status from field 4,
`^Complete` as a prefix match, over `$ROOT/.planning/REQUIREMENTS.md` and every
`$ROOT/.planning/milestones/*-REQUIREMENTS.md`. The `- [x] **REQ-NN**:` checkbox lines are not
parsed. A missing ledger file is an empty exclusion set.

```
complete=51  excluded=7  registered=44      51 - 7 = 44 = registered
```

The 44 derived ids are exactly the registry's distinct `<REQ>` values:
`API-02 API-03 API-04 AUD-01 AUD-02 CPU-01..05 FND-01..07 INP-01..03 MEM-01..05 PKG-01 PKG-03 PKG-04
PRF-01 RUN-01 RUN-02 RUN-04 RUN-05 RUN-06 SCN-01 SCN-02 STA-01 STA-02 VID-01..06`. The real tree
passes with `-- important-gates pass gates=107 lines=330`; `lines` and `gates` did not move.

### The exclusion ledger (seven rows)

| REQ | Reason | Category | Evidence the requirement actually has |
|---|---|---|---|
| `REL-01` | `close_record_and_release_action_only` | close_record_only | the milestone close record and the stage-2 release action, recorded in the close record |
| `API-01` | `instance_isolation_proved_by_suite_test_ids` | gate_machinery_self_evidence | `run.input.isolation` and `input.iso.interleaved-instances`, suite test ids, not fault/control/mutant rows |
| `API-05` | `core_purity_controls_registered_under_INP-03` | gate_machinery_self_evidence | the `c-purity` controls (`core-purity-io/fs/clock`), already registered under INP-03 |
| `API-06` | `api_effects_controls_are_unregistered_here` | gate_machinery_self_evidence | the `api-effects` controls exist and run; they carry no registry row (see D-09-06) |
| `RUN-03` | `tests_inventory_step_is_the_inventory_gate_itself` | gate_machinery_self_evidence | the `tests-inventory` gate step and its controls; the step is the mechanism |
| `PKG-02` | `tools_gate_sh_is_the_documented_local_command` | gate_machinery_self_evidence | `tools/gate.sh` itself; the `ci-parity` controls are registered under PKG-03 |
| `EV-02` | `the_registration_commit_is_its_own_evidence` | gate_machinery_self_evidence | the 17 registrations landed by 09-04; a row naming EV-02 would be the registry citing itself |

## The stand-in, and its output

`tools/check/controls/important-gates-complete-uncovered` overlays the toy
`.planning/REQUIREMENTS.md` with `| FND-99 | Phase 1 | Complete |` (no registry line) and the toy
ledger with `EV-99 not_a_gate_mechanism_at_all` (whose requirement is not Complete). Observed:

```
-- FAIL important-gates-complete-uncovered: FND-99
-- FAIL important-gates-excluded-stale: EV-99
-- FAIL important-gates-uncovered-gate: FND-99
control ok   important-gates-complete-uncovered
```

The third line is expected and is **not** forbidden: because the derived accepted set *is* the
must-be-covered set, the Complete-and-unregistered row trips the generic coverage refusal as well, so
`FORBID` names **one** token, `important-gates-unknown-id` — a `FORBID` on
`important-gates-uncovered-gate` would make the control unsatisfiable against its own output. That
line is also the evidence that `important-gates-complete-uncovered` is the authoritative finding for
this case: it states *why*, while the generic token also fires when the registry file is missing.

Not decoration: the same `CHECK` on the pristine toy tree passes (`-- important-gates pass gates=43
lines=44`), so the `EXPECT` lines only exist because of the overlay.

## Toy records, consistency by construction

Both toy records carry the same 44 ids and the same `Complete` status; `diff` of the sorted id list
against the toy registry's distinct `<REQ>` set is empty, and `diff` between the two records is empty.
The toy registry is 44 lines / 43 gate names, so `_good/important-gates` stays green.

## Gate evidence

```
sh   tools/gate.sh -> gate: PASS steps=38/38 controls=380/380 ... gates=107 lines=330  (Task 1)
sh   tools/gate.sh -> gate: PASS steps=38/38 controls=381/381 ... gates=107 lines=330  (Task 2)
dash tools/gate.sh -> gate: PASS steps=38/38 controls=381/381 ... gates=107 lines=330  (Task 2)
```

`faults=235/235`, `tests-expected=4958`, `tests-executed=4954`, `tests-skipped=4`,
`tests-missing=0`, `claims=28 provisional=27`, `steps=38/38` all unchanged. Recorded observation
(never a predicate): `files=tracked=hygiene-files=2520 -> 2524 -> 2529`, i.e. exactly +4 then +5
against the task-start baselines `2520` and `2524`.

Control pricing, load average 13.14 15.09 14.81, three runs at 381 controls: cpu-s (user+sys)
65.13 / 73.37 / 75.97, median 73.37 -> 0.193 cpu-s per control against the recorded 0.185 unit, i.e.
1.04x, inside the phase allowance. One measurement point rather than the before/after alternation the
plan asked for; the control was already committed in the same task, so the comparison is against the
recorded unit (budget deviation, recorded below).

## Deviations from Plan

**1. [Rule 1 - Bug] The ledger needs seven rows, not six: 09-04 marked `EV-02` Complete**

- **Found during:** Task 1, first run of the new derivation on the pristine tree.
- **Issue:** the plan's arithmetic counts the 50 `Complete` rows of
  `.planning/milestones/v0.1.0-REQUIREMENTS.md` only, but the derivation it mandates also reads
  `.planning/REQUIREMENTS.md`, where `| EV-02 | Phase 9 | Complete |` was written by the commit that
  followed 09-04 (`b27d592`). `EV-02` has no registry line (it appears in the registry only in two
  `#` comments), so a six-row ledger produced
  `-- FAIL important-gates-complete-uncovered: EV-02` on the pristine tree — exactly pitfall 5, the
  failure this plan exists to avoid.
- **Fix:** `EV-02` is the seventh ledger row, reason `the_registration_commit_is_its_own_evidence`
  (the `gate_machinery_self_evidence` category 09-09 uses). The identity is restored exactly:
  `51 - 7 = 44 = registered`. The two alternatives were rejected: registering `EV-02` breaks the
  frozen `lines=330` (A-08, ROADMAP criterion 3), and reverting `EV-02` to `Pending` falsifies a
  requirement 09-04 legitimately delivered.
- **Files modified:** `data/expected/important-gates-excluded.ids`.
- **Commit:** `f7708f8`.
- **Effect on D-09-05:** `09-09` now adds **three** exclusion rows (`EV-01`, `EV-03`, `EV-04`) beside
  the status flip, not four; the ledger ends at ten rows.

**2. [Budget] The plan's verify arithmetic was restated to the measured facts**

- The `<verify>` clause `[ "$c" = 50 ] && [ "$x" = 6 ]` is stale for the reason above; it was run as
  `c = 51`, `x = 7`, `r = 44` with the identity `c - x = r` intact. The identity, not the literals,
  is what the clause exists to assert.

**3. [Budget] One gate run per task plus one `dash` run at Task 2 only**

- Approved by the dispatch: `sh tools/gate.sh` at the end of each task, `dash tools/gate.sh` once at
  the end of Task 2. The container gate at the wave boundary is out of scope here.

**4. [Budget] Control pricing is one measurement, not an alternated before/after pair**

- The control was created and committed inside Task 2, so there is no post-plan "before" tree to
  alternate against; the median is compared with the recorded 0.185 unit instead.

## Recorded deviation D-09-05 (forward reference)

The four requirements this phase delivers (`EV-01` to `EV-04`) are marked `Complete` at the phase
close, and the derivation then demands a registry line or an exclusion for each. `EV-02` already has
its row (deviation 1 above). Adding registry rows for the rest would break the frozen `lines=330`, so
`09-09` adds the three remaining exclusion rows (`EV-01`, `EV-03`, `EV-04`) in the same commit as the
status flip, under the `gate_machinery_self_evidence` category the ledger docstring names. The
`important-gates-excluded-stale` refusal is what proves the ordering is not arbitrary: a row that
lands before its requirement is `Complete` fails.

## `requirements mark-complete EV-04` was run and then reverted, on purpose

The standard state update flips `EV-04` to `Complete` in `.planning/REQUIREMENTS.md`. Run here, it
made the derivation fire on the pristine tree in the next step run, measured:

```
-- FAIL important-gates-complete-uncovered: EV-04
-- FAIL important-gates-uncovered-gate: EV-04
```

That is D-09-05's ordering, not a defect: the status flip and the exclusion row must land in the
same commit, which is `09-09`'s. The flip was therefore reverted and `EV-04` stays `Pending` until
then, so the tree is green at the end of this plan. The `requirements-completed: [EV-04]` line in the
frontmatter records what this plan delivered, not what the traceability table says yet.

## Recorded deviation D-09-06 (GUARDRAILS 2.3 registration limb, deferred)

This is the first plan in the phase that adds a must-fail mechanism with **no** registry line.
GUARDRAILS 2.3 requires every new check to be "registered in `data/expected/important-gates.ids`
under its requirement". These are not registered, because the registry's row count is frozen at
`lines=330` by ROADMAP Phase 9 criterion 3 and by amendment A-08 (`gates=90 to 107`,
`lines=306 to 330`, stated as the EV-02 commit's exact delta): any row added after 09-04 makes the
phase end at a count its own criterion forbids.

The deferral covers **nine mechanisms and no fewer**, and this is the canonical enumeration that
`09-06`, `09-07` and `09-09` cite by name instead of re-listing a subset. **Appended note (review fix
WR-01, commit `0d84484`):** the list is now **ten** — `important-gates-excluded-malformed`, which
validates the exclusion ledger's own declared row format, was added by the code review's fix pass.
The enumeration below is left as written (this file is not rewritten); the tenth is named here and
in `09-REVIEW.md`'s `## Fixes applied`.

1. `important-gates-complete-uncovered` (this plan)
2. `important-gates-excluded-stale` (this plan)
3. `control-toy-drift` (09-06)
4. `control-deviation-unused` (09-06)
5. `control-pass-line-not-last` (09-06)
6. `sources-src-token-whole` (09-07)
7. `manifests-output-anchor` (09-07)
8. `memmap-generator-refused` (09-07)
9. `memmap-generator-missing` (09-07, the liveness guard for 8)

A plan that names fewer under-reports the standing gap, and `09-09`'s durable declined-item row must
name all nine because that row is what a later milestone reads.

**Reversal condition:** a later milestone may add these rows once `lines=330` is re-derived as a new
baseline; the rows then belong under `EV-03`/`EV-04` with the same non-vacuity sentence every other
row carries. The other two limbs of GUARDRAILS 2.3 *are* met here — a liveness guard and a committed,
re-runnable stand-in showing the guard fail — so what is deferred is registration only, and it is
deferred visibly rather than silently.

## Threat surface

`T-09-09` and `T-09-10` (both `mitigate` in the plan's register) are addressed by the derivation
reading records the registry cannot edit, the exclusion set being a different file, the
`important-gates-excluded-stale` refusal, the 44-id toy records that make a vacuous derivation fail
the positive control, and the `51 - 7 = 44` arithmetic check. No new trust boundary, endpoint, file
access pattern outside `.planning/` reads, or schema change: no Threat Flags. No package manager was
invoked.

## Known Stubs

None. Every file added here is data, a control fixture, or a step mechanism that runs in every gate.

## Self-Check: PASSED

- `data/expected/important-gates-excluded.ids` — FOUND (7 rows, `awk '!/^#/ && NF'` = 7)
- `tools/check/steps/important-gates.sh` — FOUND (`important-gates-complete-uncovered`,
  `important-gates-excluded-stale` present; `nreq = split` absent)
- `tools/check/controls/_good/tree/.planning/REQUIREMENTS.md` — FOUND (44 rows)
- `tools/check/controls/_good/tree/.planning/milestones/v0.1.0-REQUIREMENTS.md` — FOUND (44 rows)
- `tools/check/controls/important-gates-complete-uncovered/EXPECT` — FOUND (both tokens pinned)
- commit `f7708f8` (Task 1) and `5f9d25b` (Task 2) — both ancestors of `HEAD`

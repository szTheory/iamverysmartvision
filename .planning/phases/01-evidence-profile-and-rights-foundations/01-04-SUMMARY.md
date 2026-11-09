---
phase: 01-evidence-profile-and-rights-foundations
plan: 04
subsystem: evidence-records
tags: [discrepancy-log, records-dsl, gate-step, negative-controls, profile-conflicts, fnd-03]
requires:
  - phase: 01-evidence-profile-and-rights-foundations
    provides: "records.cmake generic checker, profile record and controls runner (plan 01-01); reference notes with the C17 status line and discrepancy candidates (plan 01-02)"
provides:
  - "data/discrepancies.json: 35 entries, C1 to C35, each with a default policy and a resolver"
  - "discrepancy record kind (schema, hook, inventory) and the discrepancies gate step with stats discrepancies=, pending-resolvers=, open="
  - "Profile schema line enumerating conflict ids from the log; profile reset fields reference C32"
  - "Seven negative controls (six discrepancy controls plus profile-conflict-unknown-id)"
affects: [01-07 sources and denylist steps, 01-09 notes gate checks, Phase 2 CPU vectors citing C-ids, later profile revisions]
actuals:
  tokens: 13100
  tasks: 2
  commits: 2
plan_head_before: 8c878ac21ab21f8524b349511cee3bf0a0cfd016
plan_head_after: 35f22e1e0fd3c07ff1695eb1358eb5f6f3c53553
tech-stack:
  added: []
  patterns:
    - "Single-file record kind with a hook for ordering, resolved-needs-a-page and policy-prefix rules"
    - "Step name differs from record kind: the step re-labels the checker's success line so the gate collects its stats"
key-files:
  created:
    - data/discrepancies.json
    - data/expected/discrepancies.ids
    - data/expected/gate-steps/discrepancies.ids
    - data/expected/controls/discrepancies.ids
    - tools/check/schema/discrepancy.txt
    - tools/check/kinds/discrepancy.cmake
    - tools/check/steps/discrepancies.sh
    - tools/check/controls/_good/tree/data/discrepancies.json
    - tools/check/controls/_good/tree/data/expected/discrepancies.ids
    - tools/check/controls/discrepancies-missing-c17/
    - tools/check/controls/discrepancies-missing-key/
    - tools/check/controls/discrepancies-duplicate-id/
    - tools/check/controls/discrepancies-bad-resolver/
    - tools/check/controls/discrepancies-open-without-ukdc/
    - tools/check/controls/discrepancies-resolved-without-page/
    - tools/check/controls/profile-conflict-unknown-id/
  modified:
    - data/profiles/intv-2609-ntsc-family-p1.json
    - tools/check/schema/profile.txt
key-decisions:
  - "C17 is resolved-primary, copied from the notes' C17 status line (plan 01-02 read pdf 24, 46, 48, 62); C19 is partial"
  - "The seven notes candidates became C29 to C35 in the order the notes list them; the device-supplied reset PC is C32"
  - "The profile fields reset.r7, reset.registers_other and reset.flags all reference C32"
  - "Branch policy for C29: unknown-default-chosen reading of 7 as not taken and 9 as taken, stated as a named policy the manual does not state"
requirements-completed: [FND-03]
duration: about 15min
completed: 2026-11-09
status: complete
---

# Phase 1 Plan 04: Discrepancy log Summary

**`data/discrepancies.json` holds 35 checked entries (C1 to C35) with default policies and resolvers, wired into the gate as the `discrepancies` step, and every profile conflict id is now machine-checked against it.**

## Performance

- **Duration:** about 15 min
- **Tasks:** 2 of 2 (tracer, seeding and reconciliation)
- **Files:** 16 created, 2 modified (counting control directories as one each)

## Accomplishments

- Discrepancy inventory: the C1 to C28 seed from research plus C29 onward appended from the notes per D-21, final count 35 (C1 to C35), equal to `data/expected/discrepancies.ids`.
- C29 to C35, from the notes' Discrepancy candidates in listed order: branch `7/9` (the manual prints `7/9` for the unconditional branch, the condition rows and BEXT, and NOPP prints `7`; pdf 32 does not resolve it, so open), interrupt-entry cycles not tabulated, CP-1600 versus CP-1610, device-supplied reset PC via IAB, shift mode-bit naming, SDBD indirect cycle ambiguity (pdf 57), stop wording.
- Statuses: C17 `resolved-primary` with `pdf 62; pdf 46; pdf 48; pdf 24` evidence, equal to the notes' `C17 status:` line; C19 `partial`; 33 entries open; 18 resolvers start `pending:` (counted, not failed).
- Gate summary now carries `discrepancies=35 pending-resolvers=18 open=33`.
- Profile: `reset.r7`, `reset.registers_other` and `reset.flags` reference C32 and cite `SC.1, pdf 21` (and pdf 27 for r7); `enumfrom conflict data/discrepancies.json discrepancies id` in the profile schema makes an unknown conflict id fail `profile-unknown-ref`.
- Seven new controls, all `ok`: `discrepancies-missing-c17`, `-missing-key`, `-duplicate-id`, `-bad-resolver`, `-open-without-ukdc`, `-resolved-without-page`, `profile-conflict-unknown-id`.

## Task Commits

1. Task 1 (tracer): `9901e94` feat(01-04): discrepancy log tracer with C17, schema, hook, step and two controls
2. Task 2: `35f22e1` feat(01-04): seed C1-C35 discrepancy log, reconcile profile conflicts, add five controls

## Verification (executed)

- Tracer gate re-run before expansion: `-- discrepancies pass discrepancies=1 pending-resolvers=0 open=0`, both tracer controls ok; then expanded.
- `sh tools/gate.sh` and `dash tools/gate.sh`: `gate: PASS steps=4/4 controls=27/27 discrepancies=35 pending-resolvers=18 open=33 files=164 records=23 fields=23 unknown-fields=12 tags=10 tiers=8`.
- `grep -c '"id": "C'` = 35 = `grep -c '^C'` of the inventory; the C19 entry carries `"partial"`.
- `LC_ALL=C grep -n '[^[:print:][:space:]]' data/discrepancies.json` prints nothing; the person-name, emulator-name (outside `SRC-` ids) and upper-case-underscore constant greps all print nothing; no `[cite-only]` line was needed.
- `tools/check/denylist.txt` does not exist yet (plan 01-07), so the denylist step check was not applicable; the equivalent greps above were run by hand.

## Deviations from Plan

### Auto-fixed Issues

**1. [Rule 3 - Blocking] Open and partial default-policy prefix checked in the hook, not by a schema regex**
- **Found during:** Task 1 (first control run reported `control-good-failed: discrepancies`)
- **Issue:** The plan's `when status=open match default_policy ^unknown-default-chosen: .+` cannot work: the records DSL splits each schema line on whitespace, so a regex holding a space is truncated, and the `[ ]` workaround breaks because the bracket expression is split too (CMake then failed to compile `^unknown-default-chosen:[`).
- **Fix:** Removed the two `when ... match default_policy` schema lines (a comment records why) and made `tools/check/kinds/discrepancy.cmake` test `^unknown-default-chosen: .+` for open and partial entries, emitting the same token the plan specified, `discrepancy-format: <id>/default_policy`. The check is not weakened; control `discrepancies-open-without-ukdc` proves it fails.
- **Files modified:** `tools/check/schema/discrepancy.txt`, `tools/check/kinds/discrepancy.cmake`
- **Commit:** `9901e94`

**2. [Rule 2 - Consistency] Profile reset registers and flags also reference C32**
- **Found during:** Task 2
- **Issue:** The plan only names `reset.r7`, but `reset.registers_other` and `reset.flags` sit under the same logged conflict (the C32 note lists all three as non-manual defaults), so leaving `conflict: null` would contradict the log.
- **Fix:** Set `conflict` to C32 on both and cited `SC.1, pdf 21`. Other profile fields unchanged. The prior `reset.r7` reference was C27 (mini-EXEC contract), replaced by C32 as the plan directs; C27 stays in the log.
- **Commit:** `35f22e1`

### Notes (not deviations)

- The step is named `discrepancies` while the record kind is `discrepancy`, so the step re-labels the checker's `-- discrepancy pass records=<n> ...` line as `-- discrepancies pass <stats>` (without `records=`) so the gate collects the stats once.
- The tracer's C17 entry text was kept verbatim when Task 2 regenerated the file.
- Branch cycles are described as printed: `7/9` for the unconditional branch, the condition rows and BEXT, and a single `7` for NOPP (pdf 54). The `7 not taken, 9 taken` default in C29 is labelled as a chosen policy, not a manual statement.
- The profile-conflict control and the existing `profile-bad-conflict-id` control both still pass; the latter's format failure now also triggers an unknown-ref failure.
- Duplicate JSON member keys are not detectable by the records checker (known limit, SKELETON section 8).
- FND-03 is marked complete: this plan delivers the log, its checks and the controls named in the success criteria.
- Emulator projects and people are not named in the record; the permissive driver and the GPL fork appear only as `SRC-mame-intv` and `SRC-community-emu-docs`. Where the research cited a measurement by a named person or site, the entry says "a community measurement" or "an archived community page".

## Known Stubs

None. Pending resolvers (18) are deliberate and counted by the gate (`pending-resolvers=`), not stubs.

## Threat Flags

None. T-01-11: entries are paraphrased, cite by SRC id, and carry no person names or emulator constants. T-01-12: exact inventory, ordering and duplicate-id checks, required-key controls and the unknown-conflict-id control are in place. No absolute home or scratch path appears in any tracked file written here.

## Issues Encountered

A gate run exceeded the tool timeout while another build was using the machine; it completed normally when re-run (a full gate takes roughly 40 to 120 seconds).

## Next Phase Readiness

Plan 01-07 can add the denylist and sources steps over `data/discrepancies.json` (every SRC token used is already in `data/sources.json`). Phase 2 vectors can cite C17, C19, C29 to C35 by id. Plan 01-09 can check the `C17 status:` line against the C17 entry's status.

## Self-Check: PASSED

- All created files exist; commits `9901e94` and `35f22e1` are ancestors of HEAD (verified below before commit).

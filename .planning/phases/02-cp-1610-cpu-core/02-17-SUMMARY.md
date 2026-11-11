---
phase: 02-cp-1610-cpu-core
plan: 17
subsystem: testing
tags: [cp1610, second-reader, adjudication, vector-review, D-15, A-12]

requires:
  - phase: 02-cp-1610-cpu-core
    provides: "blind tool, diff tool, vector-review record kind, handoff-record check, reader protocol and policy document (plan 02-16); 412 committed vectors in 12 files (plans 02-02..02-15)"
provides:
  - "data/expected/vector-review-handoff.ids: committed verbatim copy of the orchestrator HANDOFF.txt (12 stems, derived sha256, reviewer, run id)"
  - "data/cp1610/vector-review.json: 12 marker rows (412 vectors, 382 agree, 30 disagreements, 30 page-based resolutions)"
  - "data/expected/vector-review.ids complete (12 stems); gate second-read=12/12"
  - "reader documents tightened where they caused disagreements (bus kind meanings and per-unit order; p07 increment order)"
affects: [02-18, verify-work, second-reader, vector-review]

actuals:
  tokens: 5967
  tasks: 3
  commits: 2
plan_head_before: 5b6993342b0d4131f631ac14940b770dff81d310
plan_head_after: d4a1f51a408225221131557af568f0d317f8a999

tech-stack:
  added: []
  patterns:
    - "Adjudication rows generated from the diff tool's own DIFF lines, so every disagreement string is the tool's `<id> <field>` and the counts come from its diff-summary"

key-files:
  created:
    - data/cp1610/vector-review.json
  modified:
    - data/expected/vector-review-handoff.ids
    - data/expected/vector-review.ids
    - data/discrepancies.json
    - docs/cp1610/second-reader-protocol.md
    - docs/cp1610/second-reader-policies.md

key-decisions:
  - "All 30 disagreements resolved for the committed value from the page: direct-mode READ/ADAR labels (pdf 23, pdf 30), TCI strobe list order (pdf 33), ADD@ R4,R4 increment order (pdf 48), SDBD immediate pair total (pdf 58, kept under open C34); no vector, init or core change"
  - "No new discrepancy id: every disagreement touches an already recorded convention or open item (stepper conventions, C34, C39, C55); C34 and C55 notes record the readers' alternative readings and both stay open"
  - "Reader documents completed where their wording caused disagreements, so a future re-read (protocol step 7) does not repeat them; the edit happened after the readings and changes no digest the gate checks"
  - "requirements-completed stays empty: CPU-01..CPU-05 are still declared by plan 02-18"

patterns-established:
  - "A disagreement caused by a reader-document gap is resolved from the page and the document is completed in the same commit, recorded as a deviation"

requirements-completed: []
requirements-advanced: [CPU-01, CPU-02, CPU-03, CPU-04, CPU-05]

coverage:
  - id: D1
    description: "Committed handoff record equal to the orchestrator HANDOFF.txt, checked against every marker row by the gate"
    requirement: CPU-01
    verification:
      - kind: other
        ref: "test \"$(grep -v '^#' data/expected/vector-review-handoff.ids)\" = \"$(cat build/review/derived/HANDOFF.txt)\" (verbatim-ok)"
        status: pass
      - kind: integration
        ref: "sh tools/gate.sh (gate: PASS ... second-read=12/12)"
        status: pass
    human_judgment: false
  - id: D2
    description: "Every vector file diffed against its isolated reading; every disagreement resolved from a page image and recorded"
    requirement: CPU-01
    verification:
      - kind: other
        ref: "for each stem: cmake -DROOT=$PWD -DFILE=<stem> -P tools/review/vector-diff.cmake (review-all-ok)"
        status: pass
      - kind: other
        ref: "test reviewed set = cp1610-vector-files.ids (reviewed-set-ok); 12 reviewer matches"
        status: pass
    human_judgment: true
    rationale: "Each resolution is a reading of a page image by the adjudicator; the gate checks digests, counts and that every disagreement has a resolution, not the reading itself"

duration: 7min
completed: 2026-11-11
status: complete
---

# Phase 2 Plan 17: Adjudication of the isolated blind second reading Summary

**All 12 vector files carry a marker for their isolated reading, matching the committed handoff record by digest: 412 vectors, 382 agree. All 30 disagreements went to the committed value from the page (direct-mode READ/ADAR labels per pdf 23/30, TCI list order per pdf 33, the ADD@ R4,R4 increment order per pdf 48, and the SDBD immediate pair total kept under open C34). No vector or core change was needed; the gate reports second-read=12/12.**

## Performance

- **Duration:** about 7 min of execution, timed from the recorded start (reading the plan and context before it is not counted)
- **Started:** 2026-11-11T01:10:03Z
- **Completed:** 2026-11-11T01:17:00Z
- **Tasks:** 3 of 3 (Task 1 performed by the orchestrator, gate verified here)
- **Files modified:** 6 (2 task commits)

## Accomplishments

- Task 1 gate verified before any work. The verify command printed `handoff-complete`. HANDOFF.txt has 12 well-formed lines, and each digest equals `cmake -E sha256sum build/review/derived/<stem>.json`. Each derived file's `blind_sha256` equals the current blinded copy digest (12 `blind-ok`). The blinded copies contain no `POL-` string. The manual's sha256 is `ecdabe5c...dc2b72`.
- `data/expected/vector-review-handoff.ids` now holds the 12 HANDOFF.txt lines byte for byte below its header (`verbatim-ok`).
- All 12 files were diffed mechanically. Each of the 30 `DIFF` lines was settled from a page image and recorded in `data/cp1610/vector-review.json`, with 12 rows whose disagreement and resolution lists have equal length.
- The C34 and C55 notes now record the alternative readings the isolated readers chose. Both items stay open.

## Task Commits

1. **Task 1: Checkpoint, orchestrator fan-out.** Performed by the orchestrator under --auto (AUTONOMY section 3). There is no executor commit; the gate was verified as above.
2. **Task 2: Tracer: handoff record and the control marker row.** `bb4b6ba` (feat). Tracer verified end to end: the diff printed a `diff-summary` line and the gate passed with `second-read=1/12`.
3. **Task 3: Adjudicate the other eleven vector files.** `d4a1f51` (feat).

**Plan metadata:** recorded in the docs(02-17) completion commit.

## Per-file results

`diff-summary` lines, exactly as the tool printed them:

```
diff-summary file=control vectors=27 agree=27 disagree=0 derived_sha256=69ee7d91578c66f5ee0cae5532e91c8a45ce0d75976981f534c39bf7b25a2f08 run_id=run-control-a1
diff-summary file=register vectors=62 agree=62 disagree=0 derived_sha256=ba23474b31509e3565c0e05a7b2326dadc606089bfc06eb562f3cd47b78b4fe3 run_id=run-register-a1
diff-summary file=immediate vectors=24 agree=24 disagree=0 derived_sha256=9edd73d4325791e0a9beaf63fede849f79ac98fc050a5f651daa8a21a61c382c run_id=run-immediate-a2
diff-summary file=shift vectors=43 agree=43 disagree=0 derived_sha256=6c6e4c755f6907f24ccc5f476035e7d72c71d8ba246165cbf458da57863b0bce run_id=run-shift-a1
diff-summary file=direct vectors=19 agree=0 disagree=19 derived_sha256=93fc19699babb62735e86a9fd196fb5fc346c6bb4fb1efa1061984ceae08fb90 run_id=run-direct-a2
diff-summary file=indirect vectors=35 agree=34 disagree=1 derived_sha256=072a331451bc3ca2d497610f6322bf56d21265f9cff12d0d924766f8957f09c4 run_id=run-indirect-a1
diff-summary file=branch vectors=98 agree=98 disagree=0 derived_sha256=c30f3942e07855a3c0111edf2079c14c310597195cb724de03a488893365a30b run_id=run-branch-a1
diff-summary file=jump vectors=11 agree=11 disagree=0 derived_sha256=e7d74fc66a4046635db8be702a3f9408654fdfff3caf9c3f4f03624ea366c0fc run_id=run-jump-a1
diff-summary file=sdbd vectors=27 agree=20 disagree=7 derived_sha256=e6f1656cba922edbb78d4be03ce4211e59efc7391dd7cdc871eddf5edf21ceb3 run_id=run-sdbd-a1
diff-summary file=encoding vectors=5 agree=5 disagree=0 derived_sha256=015861b8ad523c67deb2049f6d52d90352eb7ca4032299758db9394ea3573311 run_id=run-encoding-a1
diff-summary file=interrupt vectors=39 agree=36 disagree=3 derived_sha256=507266a4cd3959d356c3bdc9951f104312ed820441cb56748e8c3100ad53175a run_id=run-interrupt-a2
diff-summary file=busrq-halt vectors=22 agree=22 disagree=0 derived_sha256=9c7ac3d1d33be56ca5dc8c442e7b24d7ac017a02d0257b0394e7b3c387d04117 run_id=run-busrq-halt-a2
```

| File | Vectors | Agree | Disagree | Field | Winner | Page |
|------|---------|-------|----------|-------|--------|------|
| control | 27 | 27 | 0 | - | - | - |
| register | 62 | 62 | 0 | - | - | - |
| immediate | 24 | 24 | 0 | - | - | - |
| shift | 43 | 43 | 0 | - | - | - |
| direct | 19 | 0 | 19 | bus (kind labels) | committed | pdf 23, pdf 30 |
| indirect | 35 | 34 | 1 | r4 | committed | pdf 48 |
| branch | 98 | 98 | 0 | - | - | - |
| jump | 11 | 11 | 0 | - | - | - |
| sdbd | 27 | 20 | 7 | cycles | committed (C34 open) | pdf 58, pdf 57 |
| encoding | 5 | 5 | 0 | - | - | - |
| interrupt | 39 | 36 | 3 | bus (2), cycles (1) | committed (C34 open for cycles) | pdf 23, pdf 33, pdf 58 |
| busrq-halt | 22 | 22 | 0 | - | - | - |
| **total** | **412** | **382** | **30** | | | |

No code change came from any resolution: `core/src/cpu/cp1610.c` and `core/src/cpu/cp1610_decode.c` are unchanged, and no vector's `init`, `program` or other blinded input changed, so every reading stays valid for the tree.

## Resolutions

The resolution strings are committed in `data/cp1610/vector-review.json`. They fall into five groups.

**1. Direct-mode access labels: 16 direct vectors with figure cycles, plus 3 direct MVO vectors.** The reader labelled the access at the address word `ADAR` and the operand access `READ`. Committed has `READ` then `ADAR`. Cycles agree (4 and 6), as do addresses and data.

- The pdf 23 note to C.3 says "The CPU will issue BAR, sample BDRDY, then issue ADAR to readdress memory, sample BDRDY again, and finally issue DTB".
- The pdf 30 figure prints the bus control sequence BAR, NACT, DTB, NACT, BAR, NACT, ADAR, NACT, DTB, which puts ADAR at cycle 6.
- So the access at the address word begins with BAR and is a READ. The operand access is the one with the ADAR address phase. **Committed wins.** Resolution form: `cpu.MVI.adar-figure: bus committed (pdf 23; C.3 note: BAR puts out the address of the address word, then ADAR re-addresses memory before the operand DTB, ...)`.
- The three direct MVO vectors (`cpu.MVO.write-sequence`, `cpu.MVO.overwrites-word`, `cpu.MVO.address-ffff`) follow the same reading. The write that follows the ADAR re-address is `WRITE` on both sides.

**2. `cpu.MVO.defers-intr` (interrupt), bus.** Same label difference and same resolution: committed wins (pdf 23).

**3. `cpu.TCI.defers-intr` (interrupt), bus.**

- Both lists hold the same six entries with the same cycles: the next FETCH at 4 and STROBE_TCI at 5, exactly as pdf 33 prints them (the next BAR at cycle 4, the TCI pulse rising in cycle 5).
- They differ only in order. The project groups records by unit and keeps the strobe with the TCI unit, one cycle past its end (C39, stepper conventions). The protocol's "in order of occurrence" led the reader to sort by cycle.
- No page value differs. **Committed kept.**

**4. `cpu.ADD@.r4-r4-sees-increment` (indirect), r4.** Committed has 0311; the reader has 0310.

- pdf 48, Fig 3.2.4, lists `(MMM) <- (MMM) + 1` as part of forming the address for modes 100 and 101. The paragraph above it says the R7 increment of address formation happens "before the Operation Code is performed".
- So the operation sees R4 + 1 = 0301, and 0010 + 0301 = 0311 (POL-STK-02, C55). **Committed wins.**
- The reader's policy statement p07 lacked the increment-order clause of POL-STK-02. The reader wrote 0311 down as its alternative.

**5. SDBD immediate pair totals.** Six sdbd vectors (XORI, MVII low-high, ANDI, ADDI, CMPI, SUBI) have committed 14 against the reader's 18. `cpu.MVII.sdbd-dbd-cleared` has 22 against 26, and `cpu.MVII.sdbd-then-intr-deferred` in interrupt has 23 against 27.

- pdf 58 prints one 14 against a four-word row that begins with the SDBD word. pdf 57 splits the indirect forms as SDBD 4 + target 10 = 14.
- The committed reading takes the 14 as the pair total. The reader's reading takes it as the target alone, which gives 18.
- The page is genuinely ambiguous, and C34 already records this ambiguity as open. The committed value is kept, nothing is settled, and the C34 note now records the reader's alternative.

## Isolation attestation

The gate does not attest reader isolation. The orchestrator's transcript of Task 1 attests it (A-12). The run ids are the ones in the committed handoff record `data/expected/vector-review-handoff.ids`:

`run-branch-a1`, `run-busrq-halt-a2`, `run-control-a1`, `run-direct-a2`, `run-encoding-a1`, `run-immediate-a2`, `run-indirect-a1`, `run-interrupt-a2`, `run-jump-a1`, `run-register-a1`, `run-sdbd-a1`, `run-shift-a1`.

Facts the orchestrator reported:

- On 2026-11-10 it regenerated the 12 blinded copies (`cmake -DROOT=$PWD -P tools/review/vector-blind.cmake`): 12 `blind <stem>` lines, exit 0, 412 vectors.
- It spawned one fresh general-purpose subagent per stem (model opus), in parallel batches of at most 4. Each prompt held only:
  - a role statement and the stem
  - the blinded copy path, the protocol path and the policy-document path
  - the forbidden-inputs reminder and the output path

  No vector content, plan, SUMMARY, CONTEXT or other reader output was passed.
- Four first attempts (interrupt, direct, busrq-halt, immediate) failed with an API 429 rate-limit error before writing any output. The orchestrator checked that no derived file existed for them, then replaced each with a new fresh subagent (`-a2`), never resuming the failed one.
- The busrq-halt reader (a2) reported a protocol slip. It ran `ls build/review/derived/` while creating its output directory; the command failed and displayed nothing, so it saw no other reader's output.
- The readers ran as generic subagents of this project, so the project CLAUDE.md research appendix was in their context. That appendix is general project text with no vector expectations.
- The orchestrator checked each derived file's `blind_sha256` against the current blinded copy digest, and the vector counts per stem.

What the gate does verify (A-12):

- each marker row's `derived_sha256` and reviewer against the committed handoff record
- each `file_sha256` against the current vector file
- the counts
- that disagreements and resolutions have equal length

The diff tool also checked HANDOFF.txt digests and the blind digest before every comparison, and none of its input checks failed. This executor never created, edited or deleted anything under `build/review/derived/`: after both commits, each derived file's sha256 still equals its HANDOFF.txt line.

## Files Created/Modified

- `data/expected/vector-review-handoff.ids`: the 12 HANDOFF.txt lines below the header.
- `data/cp1610/vector-review.json`: the `method` text and 12 rows.
- `data/expected/vector-review.ids`: 12 stems.
- `data/discrepancies.json`: plan 02-17 notes appended to C34 and C55. Status and default are unchanged.
- `docs/cp1610/second-reader-protocol.md`: bus entry order is per unit with strobes kept in their unit, and each bus kind's meaning is stated (READ is a BAR-addressed data read, ADAR is the ADAR-addressed operand read, pdf 23).
- `docs/cp1610/second-reader-policies.md`: p07 gains the increment-order clause of POL-STK-02.

## Decisions Made

- Every disagreement was decided from a page crop or full page image, never from code output and never by majority:
  - `refs/pages/gi-023.png`, `gi-030.png`, `gi-031.png`, `gi-033.png`, `gi-048.png`, `gi-057.png`, `gi-058.png`
  - all already rendered and gitignored; no new render was needed
- No new C-id. The ADAR labelling and per-unit record order are recorded conventions (stepper conventions, C39), and the SDBD total and the pointer increment are the open items C34 and C55.
- Bus `cycle` values: the readers wrote integers (direct, interrupt and others) or quoted strings (control, indirect). The diff tool reads both through `string(JSON GET)`, which returns the same text, so no normalisation change was needed. Figure-basis agreements in both encodings show it, for example control 27/27 with strings and the interrupt figure vectors with integers.

## Deviations from Plan

### Auto-fixed Issues

**1. [Rule 2 - Missing critical] The reader documents were incomplete where three of the disagreement groups arose**
- **Found during:** Task 3, adjudication of direct, interrupt and indirect.
- **Issue:** the protocol named the bus kinds without saying what each one means, and said "in order of occurrence" without the per-unit grouping. Separately, the p07 statement omitted the increment-order clause of POL-STK-02. Readers following them literally produced the ADAR, TCI-order and ADD@ disagreements, and a future re-read under protocol step 7 would repeat them.
- **Fix:** two sentences in the protocol (bus kind meanings, per-unit order) and one in p07. Neither document carries a vector id, a `POL-` string or a cycle count.
- **Files modified:** `docs/cp1610/second-reader-protocol.md`, `docs/cp1610/second-reader-policies.md` (not in the plan's file list).
- **Verification:** gate PASS. The readings stay valid because the documents are not part of any digest the diff tool or the gate checks.
- **Committed in:** `d4a1f51`

**2. [Minor] Status check pattern matched the document names before the commit**
- The acceptance check `grep -c -E 'build/review|reader-|blind-'` over `git status` printed 2 before the Task 3 commit, because the edited `second-reader-*.md` files match `reader-`. After the commit it printed 0. No reader output or blinded copy was ever in the tree.

---

**Total deviations:** 1 auto-fixed (missing critical), 1 minor.
**Impact on plan:** none on scope. No vector, core or tool change.

## Issues Encountered

None.

## Gate

- `sh tools/gate.sh` (unpiped run, output captured): exit 0, 44 s wall. Last line: `gate: PASS steps=24/24 controls=190/190 ... vectors=412 source=291 policy=121 ... blind-files=12 second-read=12/12 ... discrepancies=58 ... tests-expected=1040 tests-executed=1040 tests-skipped=0 tests-missing=0 ...`.
- `dash tools/gate.sh` (output captured): exit 0, 45 s wall, `gate: PASS steps=24/24 controls=190/190 ... second-read=12/12 ...`.
- Task 2 gate: `gate: PASS ... second-read=1/12`, 47.5 s wall.

## User Setup Required

None. No external service configuration required.

## Next Phase Readiness

- Plan 02-18 can rely on a complete second-read marker for all 12 files. A later edit to any vector file makes its row stale (`vector-review-stale`), which would need a new isolated reading under protocol step 7.
- Open items C34 and C55 still await a worked example or hardware evidence, as before.

## Self-Check: PASSED

- Files: `data/cp1610/vector-review.json`, `data/expected/vector-review-handoff.ids` and `data/expected/vector-review.ids` found.
- Commits: `bb4b6ba` and `d4a1f51` are ancestors of HEAD.
- Task 3 verify commands: `review-all-ok`, gate PASS with `second-read=12/12` under sh and dash, and `reviewed-set-ok`. The reviewer count is 12, the status check prints 0, and every derived file's sha256 still equals its HANDOFF.txt line.

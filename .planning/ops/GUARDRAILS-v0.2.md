# Guardrails for milestone v0.2.0

**Superseded 2026-11-26 by `.planning/ops/GUARDRAILS-v0.3.md`** (milestone v0.3.0). This file stays as recorded and is append-only; its R-rows still bind wherever `GUARDRAILS-v0.3` cites them (V5, V7).

**Status:** frozen upstream artifact, 2026-11-24, written before any v0.2.0
phase plan exists. Every phase plan, executor and verifier of this milestone
obeys it by path and row number. It is the answer to "every structural phase
rediscovers the same breakages" (LESSONS-m1 item 1) and to "a milestone that
re-verifies at the end" (lesson 25).

Cite rows as `GUARDRAILS R<n>` in a plan's `files_modified` notes.

## 1. The retarget table

A row means: **if you cause this change, these files move in the same commit.**
A count edited in a later "re-bless" commit is a defect.

| # | Causing change | Must move in the same commit |
|---|---|---|
| R1 | a public status, refusal reason, stop reason or `IVS_API` call | `data/api/api-effects.json`; `core/include/ivs/ivs_status.h`; `data/expected/api-{statuses,reasons,stop-reasons}.ids`; `data/expected/pkg-consumer-calls.ids`; `tools/check/core-symbols-allow.txt`; the suite that exercises it. Never a number in the reserved band −25..−31 |
| R2 | accepting a requirement into the important-gates registry | `tools/check/steps/important-gates.sh` (the `nreq` split string **and** its header comment **and** the token prose); `data/expected/important-gates.ids`; **all ten** whole-file toy registries (`tools/check/controls/_good/tree/...` plus the nine `important-gates-*/overlay/...`), one toy line per new REQ in each; retarget any `important-gates-bad-req-*` control whose never-valid id became valid |
| R3 | a STIC rule, frame literal or video family | `data/stic/rules.json`; `tools/check/schema/stic-rule.txt`; `tools/check/kinds/stic-rule.cmake`; `tools/check/video.cmake`; `data/expected/{stic-rules,video-frames,video-frame-pairs}.ids`; `data/expected/tests/video.ids`; `tools/check/controls/video.ids` |
| R4 | any `.github/workflows/*` change | `tools/check/steps/ci-parity.sh` (the per-file exemption branch, the `run:` allowlist, the timeout, the tag glob); `tools/check/controls/ci*.ids`; and if the job id `gate` or a matrix changes, the ruleset's required context in the same PR |
| R5 | public claim text | every claim word on the line carries `[claim:CL-NNNN]` or `[nc]` (scanned in `README.md`, `CHANGELOG.md`, `docs/**/*.md`); `data/claims.json`; `data/expected/claims.ids`; the generated `CHANGELOG.md` claim table (drift-checked) |
| R6 | a new tool directory `tools/<name>/*.c` | the `c-build` step's module list; the CMakeLists writing `build/dev/ivs-<name>-sources.txt`; the source lists in `tools/check/controls/_good/tree`; a `c-build-unlisted-<name>-source` control |
| R7 | a new public header or installed file | `data/expected/pkg-install-files.ids`; the `core/CMakeLists.txt` install rule; the `docs/packaging.md` table; the `pkg-install` step's prose and independence header list; `.planning/ops/RELEASE-OPS.md`; `data/expected/pkg-selftests.ids` if a case is added |
| R8 | a new gate step (**forbidden this milestone**) | a step script with its `-- <name> pass <stats>` line; `data/expected/gate-steps/<name>.ids`; `data/expected/controls/<name>.ids` |
| R9 | a new test id | `data/expected/tests/<suite>.ids`; **delete its pending line in the same commit** (`data/state/tests-pending.ids`, `data/psg/vectors-pending.ids`, `data/expected/cp1610-coverage-pending.ids`); the `# source` hash lines the inventory re-hashes |
| R10 | a new fault site | `data/expected/faults/<suite>.ids` plus the site, same commit (an orphan site fails) |
| R11 | a new golden | `data/golden/<name>.{expect,digests}` with exactly one `# tier:` / `# oracle:` / `# scenario:` header; `data/expected/runner-scenarios.ids`; `data/expected/scenario-pairs.ids`; the writing command added to `docs/runner.md` |

### Note on R2, appended by plan 10-06 (2026-11-25)

The R2 row above names an `nreq` split string that no longer exists. Measured on
this tree with `grep -n nreq tools/check/steps/important-gates.sh`: four lines
(156, 160, 177, 180), and all four are the same awk counter over the derived
accepted set (`reqs[++nreq]`, then `i <= nreq`), never a hand-written list of
requirement ids. Plan 09-05 deleted the hand-written list when it made the
accepted set derived (EV-04, amendment A-01), so the only parts of the row that
can still need a retarget are the header comment and the token prose. The row is
left exactly as recorded; this note sits under it rather than replacing it
(GUARDRAILS 2.10, lesson 21).

The same row's toy-registry limb was measured and does not apply to the Phase 10
acceptance: `grep -rn EV-05 tools/check/controls/_good/tree/` returns nothing, so
the toy requirement record carries no EV-05 and no toy registry line is due. The
toy `control-toy-drift` check stays green.

## 2. Milestone-level rules

1. **No new gate step.** `ls tools/check/steps/*.sh | wc -l` stays 38 and
   `data/expected/gate-steps/` is unchanged. Every new capability lands as ids
   inside a suite that already exists.
2. **Phase 9 wave A1 is cost-only.** No test id, control, fault or registry line
   may be committed before the hosted row recorded after lever (3) and trims
   (a)–(c). Prove it with
   `git merge-base --is-ancestor <row-commit> <first-id-adding-commit>`.
3. **Every new check ships three things in one commit:** a must-fail mechanism
   registered in `data/expected/important-gates.ids` under its requirement, a
   liveness guard, and a committed scratch stand-in showing the *guard* fail.
4. **No case may assert only `exit 0` / `outcome pass`.** It asserts an observed
   domain, a witness pair or a count.
5. **Price before you land.** A plan that adds ids states a measured local
   cpu-s cost (three alternated `/usr/bin/time -p` runs, both presets, load
   average recorded) against the unit-cost table, and each phase boundary reads
   the last-five median `r` and records `no-lever` / `lever-required`.
6. **Nothing is dropped to save time:** not a control, fault, judge, oracle,
   golden or pinned-default test. Caching, `continue-on-error`, a matrix or a
   path filter that skips the gate are all forbidden.
7. **The core gains no colour, palette, scaling, aspect, file I/O or content
   parsing.** Operator files live in `runner/hostio.c` only.
8. **A claim never rises above `provisional`** while D10 and D11 are
   unperformed, and no claim may cite a handoff artifact, a `build/` path, a
   `.wav`, a `.ppm` or a bench `.tsv`.

   *Read as a gap in this milestone (audit G-09) and **scoped precisely in
   v0.3.0 housekeeping, 2026-11-26**: the ceiling is a **behavioural-claim**
   ceiling. D10 and D11 gate claims about what the machine does, so every
   `original-diagnostic`, `deterministic-repeatability`,
   `installed-host-behavior`, `playable-guest-progress`, `public-vector` and
   `independent-implementation-comparison` claim stays `provisional`. A
   `source-expectation` claim may stand `established` when its fields carry
   `tier1-primary-doc` tags and it rests on no run, no measurement and no owner
   image — `CL-0002` is the only such claim. This clause is not rewritten, per
   the append-only rule; `PROJECT.md` D-32 carries the decision and the
   reversal evidence, and the mechanical check that enforces it is the first
   row of the next gate-hardening pass.*
9. **Oracle first, provably.** The format or policy document, the expected
   values and the field table are committed *before* the code, and the ancestry
   is proved with `git merge-base --is-ancestor`. When plan prose and the frozen
   document differ, the document wins and the plan sentence becomes a recorded
   deviation.
10. **Archives are append-only.** Dispositions go in a live record, never into a
    rewrite of an archived `VALIDATION.md`, phase record or oracle.

## 3. Definition of done (copy into every plan's verify block)

```
sh  tools/gate.sh >/tmp/g.out 2>&1 ; dash tools/gate.sh >/tmp/d.out 2>&1
tail -1 /tmp/g.out ; tail -1 /tmp/d.out    # both equal the plan's predicted token string, byte for byte
sh  tools/gate-linux.sh >/dev/null 2>&1 ; grep -q '^gate: PASS' build/gate-linux.log
git status --porcelain                      # empty, except orchestrator state the executor must not stage
git log --oneline -1                        # the counts moved in THIS commit
```

- One container gate at a time; copy `build/gate-linux.log` before anything else
  runs (lesson 18). If docker is unavailable, write a visible `not run` line —
  never a pass (lesson 24).
- The summary-line delta is stated in the plan *before* execution. An unexpected
  token is a stop, not a re-bless.

## 4. Discuss-phase boilerplate for this milestone

Every discuss answers, in writing, before planning:

1. Which `R` rows does this phase touch, by number and path?
2. What is the exact count delta (`steps`, `controls`, `faults`,
   `tests-expected`, `gates`, `lines`, `goldens`, `claims`, install list) and
   its measured unit cost?
3. Does it add a test id, control, fault or step — and if yes, which hosted row
   permits it?
4. What is the oracle, at which commit is it frozen, and what is its liveness
   guard? What scratch stand-in proves the guard can fail?
5. What is the rights posture, in one line, and which file records it?
6. What is deferred, and where is the deferral recorded?
7. Which evidence tier can this reach, and which can it not? Write the ceiling
   before the work, not after.

## 5. Verify-work boilerplate for this milestone

1. Re-run the gate on the final tree. Never read an older SUMMARY.
2. Compare the summary line token by token against the plan's prediction.
3. Prove each new mechanism can fail on a scratch stand-in, and that its
   liveness guard (not the comparison) does the work.
4. Prove the frozen oracle is unchanged by comparing the frozen sections only.
5. Run the hygiene, `binary-ext` and `git ls-files` checks.
6. State in the VERIFICATION record: what was automated, what was handed off and
   why, and what could not be judged by a tool.
7. Every `H<n>-<k>` or `D<n>` id mentioned has a row in
   `data/expected/handoffs.ids`, and no claim cites it.

## 6. Close-state reconciliation (run in this order)

1. Push run on the merge commit recorded as an `event=push` row; recompute the
   last-five median `r` **by arithmetic from `.planning/ci-runs.tsv`**; write
   exactly one `checkpoint:` line.
2. `git status --porcelain` empty on `main`; one worktree; no stashes; no
   unmerged phase branches.
3. One read: open PRs = 0, open issues = 0, both rulesets = id or "not applied".
4. Records reconciled (see `.planning/ops/MILESTONE-RITUAL.md` step 7).
5. Stale-text sweep over every recorded fact the milestone changed.
6. Release decision as one line with the checks recomputed.

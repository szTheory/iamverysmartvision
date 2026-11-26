# Guardrails for milestone v0.4.0

**Status:** current, frozen 2026-11-26 by the milestone ritual
(`.planning/ops/MILESTONE-RITUAL.md` step 4). Supersedes
`.planning/ops/GUARDRAILS-v0.3.md`, which stays byte-identical except for one
appended supersession line; its rows **V1 to V10 carry forward and bind where
cited by number**. Its V11 is spent (the ten D-09-06 mechanisms closed at the
v0.3.0 close) and is not a new obligation.

Cite rows as `GUARDRAILS-v0.4 W<n>` (retarget) and `GUARDRAILS-v0.4 §<n>`
(rules). The reasoning behind every row is in
`.planning/research/v0.4.0-scoping-5-guardrails-autonomy.md`.

## 1. The retarget table

What must move **in the same commit** as what. A commit that changes the left
column and not the right one is a stop, not a re-bless.

| # | Causing change | Must move with it |
|---|---|---|
| **W1** | a display-path rule (VID-07) | the profile field with its tag and cite; `data/expected/profile-fields.ids`; the profile parameter digest in `data/expected/ivsn-literals.ids` and the id that holds it; **every** snapshot literal in that file; a memmap literal if a map row moves; the moved ids in the `stic` test and fault inventories; the C-row `evidence` line in `data/discrepancies.json` and `data/expected/discrepancies.ids`; the new pin-both-settings ids; a `PROJECT.md` decision row; an appended note in `docs/stic/reference-notes.md`. The field stays `unknown-default-chosen`; **zero claims move** |
| **W2** | a test id, control or registry row | the pending line deleted; the ten toy registries extended; the orphan site failing. Zero new faults, zero new goldens |
| **W3** | a trace or disassembly artifact (TOOL-09) | `data/rights/<slug>.json`; `data/expected/rights.ids`; `docs/RIGHTS.md`; `tools/check/binary-ext.txt` with token **and** scope, never a global grep; `.gitignore`; the `files` id that refuses it; one `files-unlisted-*` control; `docs/inspection.md`. Never tracked, never a golden, fixture, CI or claim artifact |
| **W4** | `tools/inspect/*` | the `c-build` module line; the `ivs-inspect-sources.txt` writer; the toy source lists and the unlisted-source control; the `docs/packaging.md` "not installed" line naming the tool; `pkg-install-files.ids` **unchanged at 11** plus one `pkg-install` selftest that the binary is absent; one `release-assets` case |
| **W5** | `docs/inspection.md` | frozen before any code; ancestry via `git merge-base --is-ancestor`; prose that differs becomes a recorded deviation |
| **W6** | a handoff row (D10-VID-08) | `data/expected/handoffs.ids`; a `docs/handoffs.md` section; the `docs/stic/d10-procedure.md` anchor; never an exit criterion; no claim cites it |
| **W7** | a requirement | `.planning/REQUIREMENTS.md`; the `PROJECT.md` current milestone; `STATE.md`; the `ROADMAP.md` near horizon; archived at the close |
| **W8** | a count gate | the plan names **the file it counts and the plan's own start commit sha in one sentence**; the mechanism is a `mutant` row over an existing selftest plus `PLAN-LESSONS` lesson 32 |
| **W9** | a refusal token or a format sentence (EV-08) | `data/expected/flat-refusals.ids` stays append-only; a `docs/formats/ivsc-v1.md` section; the one new id guarding the flatwrap-to-shim join; a changed token re-opens every case citing it |
| **W10** | version, changelog or release text | `pkg-install-files.ids`; `docs/packaging.md`; `.planning/ops/RELEASE-OPS.md`; `release-assets` |

## 2. Milestone-level rules

1. **No new gate step, fault or golden.** `ls tools/check/steps/*.sh | wc -l`
   stays 38; `faults` stays 238; `goldens` stays 29. Every capability lands as an
   id inside a suite that already exists, a control, an in-step selftest or a
   registry row. `claims=29` and `provisional=28` are invariants too: **this
   milestone mints no claim at all.**
2. **Cost is `cpu_s`, priced locally and monitored hosted** (D-55). The hosted
   last-five window spans 221 cpu-s, 60 percent of its own median, so a phase's 1
   to 3 cpu-s addition is at most 1.4 percent of the span: the hosted median
   cannot price a phase. Price from an alternated local measurement times 1.675
   against the recomputed allowance, and record the hosted row to monitor.
   `r` stays retired.
3. **The allowance is 21 hosted cpu-s; this milestone prices 4.72** (17 → 0.10,
   18 → 2.96, 19 → 1.64, 20 → 0.02), hard ceiling 5.6.
4. **The checkpoint rule.** At every phase boundary and at the close, recompute
   the last-five medians and the token line **by arithmetic from
   `.planning/ci-runs.tsv`**, charge the phase's priced addition against 21, and
   record exactly one of `discharged-within-allowance`, `reprice` (the median
   moved more than the priced addition with an unchanged token line: a runner
   event, no lever invented) or `lever-required`. A phase overrunning its line by
   more than 25 percent is repriced before its next plan.
5. **The instrument wraps seams; the core gains no sink** (D-52). No new core
   function, no new sink, no file I/O in the core, no new installed file. The
   tool is build-only: never installed, outside the eleven-path prefix, its own
   target under the existing presets.
6. **A default is decided by discrimination, never by fit** (D-54). It may change
   only when an authored guest's observation moves under that field and under no
   other, at the default of the remaining three. A flip that merely makes a frame
   appear is a probe result. If nothing discriminates, the exit is "default
   unchanged, observation appended, rivals named", and that is a success.
7. **Every new check ships three things in one commit:** a must-fail mechanism
   registered in `data/expected/important-gates.ids` under its requirement, a
   liveness guard, and a committed scratch stand-in showing the **guard** fail.
8. **No case may assert only `exit 0` / `outcome pass`.** It asserts an observed
   domain, a witness pair, a count, a reconciliation with an existing report line,
   or a byte equality.
9. **Nothing is dropped to save time:** not a control, fault, judge, oracle,
   golden or pinned-default test. Caching, `continue-on-error`, a matrix or a path
   filter that skips the gate are all forbidden.
10. **No claim above `provisional`**, and the ceiling is a behavioural-claim
    ceiling (`PROJECT.md` D-32). No claim may cite a handoff artifact, a `build/`
    path, a `.wav`, a `.ppm`, a `.ivsn`, an `.ivst` or a bench `.tsv`.
11. **Oracle first, provably.** `docs/inspection.md` is committed before any code,
    and the ancestry is proved with `git merge-base --is-ancestor`. When plan
    prose and the frozen document differ, the document wins and the plan sentence
    becomes a recorded deviation.
12. **Archives are append-only.** Dispositions go in a live record, never into a
    rewrite of an archived record, phase record or oracle. A live document a
    person reads gets an inline pointer at the wrong sentence as well as an
    appended amendment.
13. **Rights before bytes** (D-53). No third-party file is read, and no code or
    operator page names a refused item, until its `data/rights/` row exists with a
    disposition. A `cleared` row cites a `SRC-` id or it is not cleared. **No
    trace or disassembly is ever committed, whatever its content.**
14. **The count-gate contract** (D-56). Every count gate names the file it counts;
    every baseline names the plan's own start commit sha and is measured there; no
    verify block decides an outcome by a pipeline tail.

## 3. Tier ceilings (written before the work, not after)

| Phase | Ceiling |
|---|---|
| 17 | **No claim.** Records and one guard id only |
| 18 | **No claim.** The tool is build-only and its output is repeatability information |
| 19 | **No claim.** The observation stays `unknown-default-chosen` even when a default is decided |
| 20 | **No claim.** The packet is a handoff row |

Nothing reaches `measured-hardware`. `CL-0002` stays the only `established` claim
and D-50's check is what permits it.

## 4. Definition of done (copy into every plan's verify block)

```
sh  tools/gate.sh >/tmp/g.out 2>&1 ; dash tools/gate.sh >/tmp/d.out 2>&1
tail -1 /tmp/g.out ; tail -1 /tmp/d.out    # both equal the plan's predicted token string, byte for byte
sh  tools/gate-linux.sh >/dev/null 2>&1 ; grep -q '^gate: PASS' build/gate-linux.log
sh  tools/hygiene.sh --tree                 # silent
git ls-files | grep -E '\.(ivsc|bin|rom|so|dylib|cfg|ivsn|ivst)$'   # empty
git status --porcelain                      # empty, except orchestrator state the executor must not stage
git log --oneline -1                        # the counts moved in THIS commit
```

- The predicted `tests-expected` and `controls` delta is stated in the plan
  **before** execution. An unexpected token is a stop, not a re-bless.
- One container gate at a time; copy `build/gate-linux.log` before anything else
  runs (lesson 18). If docker is unavailable, write a visible `not run` line —
  never a pass (lesson 24).

## 5. Discuss-phase boilerplate

Every discuss answers, in writing, before planning:

1. Which **W** rows and which `data/rights/` rows does this phase touch, by
   number and path?
2. Which rival rule **pair** does this phase separate, and which trace line
   differs?
3. The exact count delta (`steps`, `controls`, `faults`, `tests-expected`,
   `tests-skipped`, `gates`, `lines`, `goldens`, `claims`, install list) and its
   priced hosted cpu-s against the allowance **recomputed from
   `.planning/ci-runs.tsv`**.
4. The oracle, its freeze commit, its liveness guard, and the scratch stand-in
   that proves the guard can fail.
5. The rights posture in one line, which row records it, and what may never be
   named.
6. What is deferred, and where the deferral is recorded with its reversal
   condition.
7. Which evidence tier this can reach, and which it cannot. The answer for every
   phase of this milestone is: **none** — it mints no claim.

## 6. Verify-work boilerplate

1. Re-run the gate on the final tree. Never read an older SUMMARY.
2. Compare the summary line token by token against the plan's prediction.
3. Prove each new mechanism can fail on a scratch stand-in, and that its liveness
   guard (not the comparison) does the work.
4. Prove the frozen oracle is unchanged by comparing the frozen sections only.
5. Prove the rival traces are pairwise distinct; prove the disassembly round trip
   is non-vacuous; prove the tool is absent from the install prefix; prove no
   trace artifact is tracked.
6. Run the hygiene, `binary-ext`, `git ls-files` and rights-ledger checks.
7. State in the VERIFICATION record: what was automated, what was thrown away,
   what was handed off and why, and what could not be judged by a tool.
8. Every `H<n>-<k>` or `D<n>` id mentioned has a row in
   `data/expected/handoffs.ids`, and no claim cites it.

## 7. Close-state reconciliation (run in this order)

1. Push run on the merge commit recorded as an `event=push` row; recompute the
   last-five `cpu_s`, `build_s` and `gate_s` medians **by arithmetic from
   `.planning/ci-runs.tsv`**; write exactly one checkpoint line in the §2
   vocabulary.
2. `git status --porcelain` empty on `main`; one worktree; no stashes; no
   unmerged phase branches.
3. One read: open PRs = 0, open issues = 0, both rulesets = id or "not applied"
   with a reason.
4. Records reconciled (`.planning/ops/MILESTONE-RITUAL.md` step 7), with the three
   named dispositions this milestone owes: the disassembly-limb drop **if it
   happened**, Phase 19's exit **which of its two honest exits it took**, and the
   release-decision line recomputed, which reads *the release ships the static
   package and the source archive, which carries the source of the build-only
   tools and of the build-only host shim; no shared object, no built host shim, no
   built inspection tool, no trace or disassembly output, no content*.
5. Stale-text sweep over every recorded fact the milestone changed.
6. Release decision as one line with the checks recomputed.
7. The close rows go in **one** records PR; its merge push row is **v0.5.0's
   opening row, deferred by exactly one commit and named**.

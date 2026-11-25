# Guardrails for milestone v0.3.0

**Status:** frozen upstream artifact, 2026-11-26, written before any v0.3.0
phase plan exists. Every phase plan, executor and verifier of this milestone
obeys it by path and row number. It supersedes `GUARDRAILS-v0.2.md`, which stays
untouched except for one supersession line, and it is the answer to "every
structural phase rediscovers the same breakages".

Cite rows as `GUARDRAILS-v0.3 V<n>` in a plan's `files_modified` notes. Rows
carried unchanged from v0.2 are cited as `GUARDRAILS-v0.2 R<n>` and are not
restated here; this file adds only what is new.

## 1. The retarget table

A row means: **if you cause this change, these files move in the same commit.**
A count edited in a later "re-bless" commit is a defect.

| # | Causing change | Must move in the same commit |
|---|---|---|
| V1 | a rights-ledger row | `data/rights/<slug>.json`; `data/expected/rights.ids`; the `docs/RIGHTS.md` row; the `files` or `manifests` id that reads it |
| V2 | a refused item's spelling | that row's `tokens`; `data/expected/rights-pinned.ids` for a pre-existing hit; its control overlay. **Token + scope, never a global grep**: `cfg` and `ivsc` already sit in `binary-ext.txt` and `.gitignore` as *refusal surfaces*, and removing them would delete a refusal |
| V3 | any claim | `data/claims.json`; `data/expected/claims.ids`; a `[claim:CL-NNNN]` or `[nc]` tag on the line; the generated `CHANGELOG.md` table; `tools/check/kinds/claim.cmake` |
| V4 | an `established` standing | D-50's mechanism in `claim.cmake`; two must-fail controls plus one legal positive; `PROJECT.md` D-32 |
| V5 | `tools/flatwrap/*.c` | `GUARDRAILS-v0.2` R6 in full (the `c-build` module list, the `ivs-flatwrap-sources.txt` writer, the toy source lists, an unlisted-source control); the `docs/packaging.md` "not installed" line |
| V6 | a `host/` shared target | a build-only target outside the install rule; `tools/check/binary-ext.txt` gains `so dylib dll bundle`; `files-unlisted-binary-so` and `-dylib` controls; one `pkg-install` selftest that the shared object is **absent** (19 → 20); `docs/packaging.md`; `RELEASE-OPS.md`; one `release-assets` case |
| V7 | a test id, control, registry row or fault | `GUARDRAILS-v0.2` R9, R2 and R10 verbatim: the pending line deleted, all ten toy registries extended, an orphan site fails |
| V8 | a handoff row | `data/expected/handoffs.ids`; the `docs/handoffs.md` packet; never a phase exit criterion; no claim cites it |
| V9 | a version, changelog or release text | `data/expected/pkg-install-files.ids`; `docs/packaging.md`; `RELEASE-OPS.md`; `release-assets` |
| V10 | an operator page describing owner content | `docs/owner-images.md` and `docs/RIGHTS.md` in the behaviour's commit, claiming nothing the gate cannot exercise |
| V12 | a profile field that changes a CPU, memory or STIC behaviour | that field's tag and cite; `data/memmap/<profile>.literal.txt` if the field is a map row; the profile parameter digest and **every** snapshot literal in `data/expected/ivsn-literals.ids`; every test or golden whose expectation the behaviour moves (measured for `cpu.interrupt_accept_clears_intff`: `state.read.describe-ok`, `state.literal.params-digest` and `run.stub2.jd-as-j`); the discrepancy note that records the observation. An `unknown-default-chosen` field may be resolved by an owner-local observation (D-51), and it **stays** `unknown-default-chosen` — a resolved default is an observation, never a claim |
| V11 | one of the ten D-09-06 must-fail mechanisms | an `important-gates.ids` **`mutant` row over an existing selftest**, never a new fault. Ten faults would cost about 4.4 hosted cpu-s, 82 % of this milestone's allowance, on the step already 29 % of the gate |

`steps=38/38` is an invariant, not a target: any commit whose summary line reads
anything else stops the milestone.

## 2. Milestone-level rules

1. **No new gate step, fault or golden.** `ls tools/check/steps/*.sh | wc -l`
   stays 38; `faults` stays 238; `goldens` stays 29. Every capability lands as an
   id inside a suite that already exists, a control directory, an in-step
   selftest or a registry row.
2. **Cost is `cpu_s`, not `r`.** `r` is retired as a decision input (v0.2.0 close,
   audit G-04) until `build_s` is stable. The figures are the last-five `cpu_s`
   median (361), the last-five `gate_s` median (118) and the measured per-step
   profile. The allowance is **21 hosted cpu-s**; this milestone prices 5.2.
3. **The checkpoint rule.** At every phase boundary and at the close, recompute
   the last-five medians and the token line by arithmetic from
   `.planning/ci-runs.tsv`, charge the phase's priced addition, and record exactly
   one of `discharged-within-allowance`, `reprice` (the median moved more than the
   priced addition with an unchanged token line: a runner event, no lever
   invented) or `lever-required`.
4. **Every new check ships three things in one commit:** a must-fail mechanism
   registered in `data/expected/important-gates.ids` under its requirement, a
   liveness guard, and a committed scratch stand-in showing the **guard** fail.
5. **No case may assert only `exit 0` / `outcome pass`.** It asserts an observed
   domain, a witness pair or a count.
6. **The core gains no colour, palette, scaling, aspect, file I/O or content
   parsing.** The host shim lives outside `core/` and outside `core/src`; the one
   palette is `runner/frame_export.c`, **linked, never copied**.
7. **Nothing is dropped to save time:** not a control, fault, judge, oracle,
   golden or pinned-default test. Caching, `continue-on-error`, a matrix or a path
   filter that skips the gate are all forbidden.
8. **No claim above `provisional`**, and the ceiling is a behavioural-claim
   ceiling (`PROJECT.md` D-32): every tier that rests on a run stays
   `provisional`, and D-50's check is what enforces it. No claim may cite a
   handoff artifact, a `build/` path, a `.wav`, a `.ppm`, a `.ivsn` or a bench
   `.tsv`.
9. **Oracle first, provably.** The policy, format or expected-value document is
   committed *before* the code, and the ancestry is proved with
   `git merge-base --is-ancestor`. When plan prose and the frozen document
   differ, the document wins and the plan sentence becomes a recorded deviation.
10. **Archives are append-only.** Dispositions go in a live record, never into a
    rewrite of an archived `VALIDATION.md`, phase record or oracle. A live
    document a person reads gets an inline pointer at the wrong sentence as well
    as an appended amendment (v0.2.0 audit, G-07).
11. **Rights before bytes.** No third-party file is read, and no code or operator
    page names a refused item, until its `data/rights/` row exists with a
    disposition. A `cleared` row cites a `SRC-` id or it is not cleared.

## 3. Tier ceilings (written before the work, not after)

| Phase | Ceiling |
|---|---|
| 13 | **No claim.** Mechanisms only; nothing new may stand `established` |
| 14 | **No claim.** The wrap and the cartridge round trip are repeatability `[nc]` |
| 15 | One `deterministic-repeatability` claim, `provisional`, resting on the nine-digest golden. **Not** `installed-host-behavior`: the shim is build-only and is never installed, so the tier's own definition (the installed package consumed by a separate host) does not apply; the scoping pass disagreed on this row and the stricter reading wins |
| 16 | No new claim. The owner route is a handoff row, never a claim |

Nothing reaches `measured-hardware`. `CL-0002` stays the only `established` claim
and D-50's check is what permits it.

## 4. Definition of done (copy into every plan's verify block)

```
sh  tools/gate.sh >/tmp/g.out 2>&1 ; dash tools/gate.sh >/tmp/d.out 2>&1
tail -1 /tmp/g.out ; tail -1 /tmp/d.out    # both equal the plan's predicted token string, byte for byte
sh  tools/gate-linux.sh >/dev/null 2>&1 ; grep -q '^gate: PASS' build/gate-linux.log
sh  tools/hygiene.sh --tree                 # silent
git ls-files | grep -E '\.(ivsc|bin|rom|so|dylib|cfg|ivsn)$'   # empty
git status --porcelain                      # empty, except orchestrator state the executor must not stage
git log --oneline -1                        # the counts moved in THIS commit
```

- One container gate at a time; copy `build/gate-linux.log` before anything else
  runs (lesson 18). If docker is unavailable, write a visible `not run` line —
  never a pass (lesson 24).
- The summary-line delta is stated in the plan *before* execution. An unexpected
  token is a stop, not a re-bless.

## 5. Discuss-phase boilerplate

Every discuss answers, in writing, before planning:

1. Which `V` rows and which `data/rights/` rows does this phase touch, by number
   and path?
2. What is the exact count delta (`steps`, `controls`, `faults`,
   `tests-expected`, `tests-skipped`, `gates`, `lines`, `goldens`, `claims`,
   install list) and its priced hosted cpu-s against the 21 allowance?
3. Does it add a test id, control, fault, golden or step — and if yes, which
   hosted row and which allowance line permits it?
4. What is the oracle, at which commit is it frozen, what is its liveness guard,
   and what scratch stand-in proves the guard can fail?
5. What is the rights posture in one line, which row records it, and what may
   never be named?
6. What is deferred, and where is the deferral recorded with its reversal
   condition?
7. Which evidence tier can this reach, and which can it not? The ceiling is in
   section 3; a plan may not raise it.

## 6. Verify-work boilerplate

1. Re-run the gate on the final tree. Never read an older SUMMARY.
2. Compare the summary line token by token against the plan's prediction.
3. Prove each new mechanism can fail on a scratch stand-in, and that its liveness
   guard (not the comparison) does the work.
4. Prove the frozen oracle is unchanged by comparing the frozen sections only.
5. Run the hygiene, `binary-ext`, `git ls-files` and rights-ledger checks.
6. State in the VERIFICATION record: what was automated, what was handed off and
   why, and what could not be judged by a tool.
7. Every `H<n>-<k>` or `D<n>` id mentioned has a row in
   `data/expected/handoffs.ids`, and no claim cites it.

## 7. Close-state reconciliation (run in this order)

1. Push run on the merge commit recorded as an `event=push` row; recompute the
   last-five `cpu_s`, `build_s` and `gate_s` medians **by arithmetic from
   `.planning/ci-runs.tsv`**; write exactly one checkpoint line in the section 2.3
   vocabulary.
2. `git status --porcelain` empty on `main`; one worktree; no stashes; no
   unmerged phase branches.
3. One read: open PRs = 0, open issues = 0, both rulesets = id or "not applied"
   with a reason.
4. Records reconciled (`.planning/ops/MILESTONE-RITUAL.md` step 7), with three
   named dispositions this milestone owes: **G-09 closes by D-50's check**, the
   ten D-09-06 mechanisms close by V11, and the release-decision line reads
   *the release ships the static package and the source archive; no shared
   object, no host shim, no content*.
5. Stale-text sweep over every recorded fact the milestone changed.
6. Release decision as one line with the checks recomputed.

---
phase: 20-route-witness-and-release-posture
verified: 2026-11-27T07:49:00Z
status: passed
score: 9/9 must-haves verified
covered_files: [".planning/ops/RELEASE-OPS.md", ".planning/phases/20-route-witness-and-release-posture/20-01-PLAN.md", ".planning/phases/20-route-witness-and-release-posture/20-01-SUMMARY.md", ".planning/phases/20-route-witness-and-release-posture/20-02-PLAN.md", ".planning/phases/20-route-witness-and-release-posture/20-02-SUMMARY.md", "data/expected/handoffs.ids", "docs/handoffs.md", "docs/inspection.md", "docs/owner-images.md", "docs/packaging.md", "docs/stic/d10-procedure.md", "tools/check/controls/_good/tree/data/expected/handoffs.ids", "tools/check/controls/claims-handoff-disposition/overlay/data/expected/handoffs.ids", "tools/check/controls/claims-handoff-id-duplicate/overlay/data/expected/handoffs.ids", "tools/check/controls/claims-handoff-id-extra/overlay/data/expected/handoffs.ids", "tools/check/controls/claims-handoff-id-missing/overlay/data/expected/handoffs.ids", "tools/check/steps/claims.sh"]
covered_digest: "v4:sha256:0f9658bfd79dc679ed16b9503abd3c10dd1b048e6f030e4003c0ac876269d328"
behavior_unverified: 0
overrides_applied: 0
---

# Phase 20: Route, Witness and Release Posture Verification Report

**Phase Goal:** An operator can reach the instrument, the D10-VID-08 hardware question
is a named packet of the exact trace lines a probe must contradict, and the release
decision is recomputed rather than quoted.
**Verified:** 2026-11-27T07:49:00Z
**Status:** passed
**Re-verification:** No — initial verification (no prior `20-VERIFICATION.md` existed; `ls` of the phase
directory held the two PLANs, the two SUMMARYs and `20-CONTEXT.md` only)

## How this was verified

`GUARDRAILS-v0.4` §6 applied literally: the gate was **re-run on the final tree**, never read from a SUMMARY;
the two summary lines were `cmp`'d; the operator section's thirteen command lines were **extracted and executed
verbatim** in a state with **no `refs/` directory**; the packet was proved by running the committed stand-in in
both directions; and each release clause was **recomputed** from archives a fresh `tools/release.sh --no-gate`
wrote on this tree. Nothing below is quoted from `20-01-SUMMARY.md` or `20-02-SUMMARY.md`.

## Goal Achievement

### Observable Truths

| # | Truth | Status | Evidence |
|---|-------|--------|----------|
| 1 | `docs/inspection.md` carries an operator section whose command lines run as written against a project-authored scenario (build, `trace`, `break`, `dis`, read the output under `refs/`), and `docs/owner-images.md` names the instrument in the owner-local route; the naming limb widens by one page and zero rights tokens, so `rights-pinned` stays 4/4 | ✓ VERIFIED | `awk`-extracted 13 command lines from the one ` ```sh ` fence tag and ran `sh -e` on them **with `refs/` moved away** (`mv refs /tmp/…`; `[ -e refs ]` → `no`): **exit 0**, all thirteen lines, shapes matching the page. `mkdir -p refs` proved load-bearing: with `refs/` absent the `trace` line alone exits **2** and prints `ivs-inspect: cannot resolve the output directory refs`, exactly the page's stated consequence. `docs/owner-images.md` `## The inspection instrument` names target `ivs_inspect`, path `build/release/tools/inspect/ivs-inspect`, the frozen surface and the `refs/` containment. `grep -v '^#' data/expected/rights-pinned.ids \| awk 'NF' \| wc -l` = **4**; `data/rights/` and `data/expected/rights.ids` unchanged in the phase (`git diff --name-only` → 0); gate prints `rights-pinned=4/4` |
| 2 | `data/expected/handoffs.ids` gains exactly one row `D10-VID-08 not-performed evidence: none`, so the claims step prints `handoffs=9`; `docs/handoffs.md` carries the packet's section and `docs/stic/d10-procedure.md` carries the anchor that section cites | ✓ VERIFIED | Ledger holds **9** data rows, the last the new one; the diff adds 5 lines to that file (one row plus four header/comment lines) and removes none. `ROOT=$PWD CHECK_DIR=$PWD/tools/check sh tools/check/steps/claims.sh` → `-- claims pass claims=29 provisional=28 handoffs=9`, exit 0. Both pinned sets in `claims.sh` gained the id and its `not-performed` disposition, and all five control-tree ledgers gained the row (each `grep -c D10-VID-08` = 1); the three controls that exit before the id-set pass are untouched (`git diff --numstat` → 0 lines). Packet section at `docs/handoffs.md:78`; anchor sentence present in `### stic-strobe` with the fixed bare literal |
| 3 | The packet names the exact trace lines a hardware probe must contradict — the record fields and, per guest, the observable line and its two candidate values under Phase 19's rival rules — as a comparison list; no claim cites the row and the claim-artifact predicate's refusal is shown able to fail on a committed stand-in | ✓ VERIFIED | Packet carries the **19** frozen record field names (access 8, step 4, end 8, with `tick` shared), each present as a `field=` token in `docs/inspection.md` `## The trace file`; four comparison rows, each with its field, its trace lines, its registered observable with **both** values, its witness and its mask (`0xAAAA`/`0xCCCC`/`0xF0F0`/`0xFF00`, Phase 19's four masks). `data/claims.json` holds **0** occurrences of `handoffs.ids` and **0** of `D10-VID-08`, and still 29 `CL-` entries. Committed stand-in run: `ROOT=/tmp/v-ctl/tree CHECK_DIR=$PWD/tools/check sh tools/check/steps/claims.sh` printed `-- FAIL claim-handoff-artifact: CL-0005` and **no** `claim-dangling-path` for that id; after repointing `CL-0005` to `data/expected/profile-fields.ids` the run printed **0** `CL-0005` findings while `CL-0001/0002/0006/0007` stayed refused by `claim-handoff-artifact` and `CL-0003/0004/0008` by `claim-dangling-path` — the refusal is neither absent nor blanket. Control registered at `data/expected/important-gates.ids:468` (`claims control claims-handoff-artifact EV-05`) |
| 4 | The packet's status line reads the four rival candidates from Phase 19's guests, naming the trace lines as `docs/inspection.md` specifies, and no criterion anywhere in the milestone requires the packet's answer or the D11 sweep to be performed — the packet is a row, not a gate | ✓ VERIFIED | Status line reads `` `G1` `0xAAAA`, `G2` `0xCCCC`, `G3` `0xF0F0` and `G4` `0xFF00` are the four rival candidates … none of them is an answer `` and the row stays `not-performed`. Bands match Phase 19's measured stamps (`19-01-SUMMARY.md`: G1 at tick **9004**, inside `8000 < tick < 11600`; G2 at tick **12064**, inside `11600 < tick < 15120`), and **G2's row carries its own band, not G1's** (`11600 < tick < 15120` on the `G2` line). `d10-items=47` unmoved; no plan's verify block, no ROADMAP criterion and no `REQUIREMENTS.md` line conditions a pass on the probe's answer or on D11; `D11` stays `not-performed` in the ledger |
| 5 | The release-decision line is recomputed per `.planning/ops/RELEASE-OPS.md` section (i) and reads the exact sentence, with `pkg-install-files.ids` still 11 and `release-assets` unchanged | ✓ VERIFIED | `### (i.2)` carries the required sentence **byte-exactly** (`grep -qF`, exactly 1 occurrence, and that occurrence is inside lines 177–219). Re-ran `sh tools/release.sh --no-gate --out /tmp/v-rel` (exit 0) and recomputed **every clause from the archives it wrote**: source `members=2859` = `git ls-files \| grep -c .`; install `members=11`; install member list == `pkg-install-files.ids` after prefix strip, `cmp` identical in both directions; build-only prefixes in the source archive `tools/inspect/` 7, `tools/pack/` 3, `tools/asm/` 9, `tools/flatwrap/` 5, `host/` 6; `so\|dylib\|dll\|bundle` members **0 and 0**; `libivs_retro`/`ivs-inspect` **0**; `refs/` or `.ivst` **0**; content extensions **0 and 0**; `bin/` in the install archive **0**; `git ls-files` content-extension paths **0**. `data/expected/pkg-install-files.ids` = **11**; `tools/check/steps/release-assets.sh` not in the phase diff; gate prints `release-selftest=13/13`, `pkg-selftest=21/21`, `prefix-files=11`, `binaries-listed=0` |
| 6 | No frozen sentence of `docs/inspection.md` moved: the diff from the freeze commit `0627be92` adds lines and removes none, and the appended section is the page's last heading | ✓ VERIFIED | `git merge-base --is-ancestor 0627be92 HEAD` → ok; `git diff --numstat 0627be92..HEAD -- docs/inspection.md` → `188  0`. `grep -n '^## '` ends at `332:## The operator route`, after `## Amendments` and all six notes. `docs/owner-images.md` from the phase start: `11  0` |
| 7 | This phase adds no id to the gate: only `handoffs` moves (8 → 9) | ✓ VERIFIED | Re-run gate summary line carries `steps=38/38 controls=405/405 faults=238/238 goldens=29 claims=29 provisional=28 prefix-files=11 rights-pinned=4/4 gates=123 lines=378 tests-expected=5114 tests-executed=5108 tests-skipped=6 d10-items=47 handoffs=9` — every token at the phase's predicted value, `handoffs` the only one that moved. `tools/check/steps/release-assets.sh` and `data/expected/important-gates.ids` are absent from the phase diff |
| 8 | No claim word and no refused rights spelling appears on any added line of either page, no `.ivst` or `refs/` path is tracked, and `refs/` holds nothing after the run | ✓ VERIFIED | `git diff 57f9b31..HEAD` added lines on the two pages: claim words (`supports\|compatible\|passes\|accurate\|verified\|runs\|plays\|playable\|looks\|sounds`, word-bounded, case-insensitive) → **0**; `exec-image`/`grom-image` → **0**. `git ls-files \| grep -cE '\.ivst$\|^refs/'` → **0**. After the run `refs/` held only the four `op-*` files the page itself says to remove; they were removed, the pre-existing owner directory was restored intact (13 entries) and `ls refs \| grep -c '^op-'` → **0** |
| 9 | `docs/packaging.md`'s `## Release content (D-33)` carries the one clause it was missing — no trace or disassembly output in a release — with its mechanism, so the release text and the recomputed line agree clause for clause (W10) | ✓ VERIFIED | The list carries `- A release carries no trace or disassembly output, and no path of one: … the `ivst` extension is binary by name in `tools/check/binary-ext.txt` …`; `ivst` is present in `tools/check/binary-ext.txt:49` with its `files-unlisted-binary-ivst` control; the page diff is `1  0` |

**Score:** 9/9 truths verified (0 present, behavior-unverified)

The truths above are documentation, inventory and recomputation truths, and each was decided by **execution**,
not by presence: the operator section was run, the claims step was run, the stand-in was run in both directions,
and the release line was rebuilt from fresh archives. No truth here rests on a symbol merely existing.

### Required Artifacts

| Artifact | Expected | Status | Details |
|----------|----------|--------|---------|
| `docs/inspection.md` | appended `## The operator route`, 7 ` ```sh ` blocks, 13 lines, stated shapes, containment, command-surface agreement | ✓ VERIFIED | Exists, 441 lines, `188 0` from the freeze commit, last `## ` heading, all 13 lines execute |
| `docs/owner-images.md` | `## The inspection instrument` between `## The sweep probe` and `## The template scenario and the gate` | ✓ VERIFIED | Present at the stated position, `11 0`, names target, release-preset path, frozen surface, `refs/` containment, never-a-result |
| `data/expected/handoffs.ids` | exactly one new row, `D10-VID-08 not-performed evidence: none` | ✓ VERIFIED | 9 data rows; one appended header paragraph names the row, its procedure, its packet and why no tool here can perform it |
| `tools/check/steps/claims.sh` | id set and disposition set both gaining the id | ✓ VERIFIED | `HANDOFF_IDS` and `HANDOFF_DISP` both carry it; the 3 deleted lines are the two extended pin lines and one rewritten comment line, no assertion lost |
| five control-tree ledgers | each gaining the same row | ✓ VERIFIED | `_good/tree` + the four overlays, each `grep -c D10-VID-08` = 1 |
| `docs/handoffs.md` | the packet section | ✓ VERIFIED | `## D10-VID-08: …` at line 78: 19 record fields, 4 comparison rows, the frame-word paragraph, the status line, the no-leak and never-a-claim paragraphs |
| `docs/stic/d10-procedure.md` | one appended sentence in `### stic-strobe` | ✓ VERIFIED | Bare literal phrase present, `2  0`, bounds are `## Frame edge and timing` |
| `.planning/ops/RELEASE-OPS.md` | `### (i.2)` between `### (i.1)` and `## (j)` | ✓ VERIFIED | Line 177, `43 0`; (i) and (i.1) lost no line |
| `docs/packaging.md` | one added bullet in `## Release content (D-33)` | ✓ VERIFIED | `1  0`, the trace-and-disassembly clause with its `ivst` mechanism |
| `data/expected/rights-pinned.ids` | unchanged, four pairs | ✓ VERIFIED | Four pairs, `rights-pinned=4/4` |

### Key Link Verification

| From | To | Via | Status | Details |
|------|----|-----|--------|---------|
| `docs/inspection.md` | `build/dev/tools/inspect/ivs-inspect` | the appended section's command lines, executed as written | ✓ WIRED | Four `ivs-inspect` invocations all exit 0 from a clean `refs/` state |
| `docs/owner-images.md` | `docs/inspection.md` | the `## The operator route` section it points a person at | ✓ WIRED | Cited by name, not restated |
| `docs/inspection.md` | `refs/` | every `--out` in the section, confined by `inspect-output-tree` | ✓ WIRED | All four `--out` paths under `refs/`; the tree-refusal branch confirmed by the missing-directory run (exit 2, `cannot resolve the output directory refs`) |
| `docs/inspection.md` | `data/scenarios/diag-boot-pass.ivss` | the project-authored scenario every invocation names | ✓ WIRED | Ran against it; `end tick=2084 records=112 units=70 steps=1` |
| `data/expected/handoffs.ids` | `docs/handoffs.md` | the row's id in the packet's heading | ✓ WIRED | Section heading and first line both name `D10-VID-08` |
| `docs/handoffs.md` | `docs/stic/d10-procedure.md` | the `### stic-strobe` anchor the packet cites | ✓ WIRED | Packet cites it; the section carries the reciprocal sentence |
| `docs/handoffs.md` | `19-03-SUMMARY.md` / `19-01-SUMMARY.md` | the four masks and the two candidate values per guest | ✓ WIRED | Masks `0xAAAA/0xCCCC/0xF0F0/0xFF00` and stamps 9004 / 12064 match Phase 19's record |
| `data/claims.json` | `data/expected/handoffs.ids` | the claim-artifact predicate, proved by the committed control | ✓ WIRED | Control registered; refusal reproduced in both directions |
| `.planning/ops/RELEASE-OPS.md` | `tools/check/steps/release-assets.sh` | each clause read from the archives `tools/release.sh` writes | ✓ WIRED | Every clause value re-derived from a fresh archive pair |

### Data-Flow Trace (Level 4)

The phase renders no dynamic UI; the analogous question is whether each documented value flows from a real
source rather than a literal. All three chains were traced to a live source.

| Artifact | Value | Source | Produces Real Data | Status |
|----------|-------|--------|--------------------|--------|
| `docs/inspection.md` operator section | `report.end 78`, `end tick=2084`, `records=112`, 13 step lines | a live `ivs-inspect trace`/`step` run on `data/scenarios/diag-boot-pass.ivss` | Yes — `2084` = the trace `end` record = the step record's `end=` = `report.end`-independent `step.1.now_ticks`; `78` = the report's own line count (79 printed − 1) | ✓ FLOWING |
| `docs/handoffs.md` comparison list | four masks, four bands, eight candidate values | Phase 19's committed `stic.disc.sweep-diagonal` measurement | Yes — stamps 9004 / 12064 sit inside the stated bands; masks match `19-03-SUMMARY.md` | ✓ FLOWING |
| `.planning/ops/RELEASE-OPS.md` `(i.2)` | 11 install paths, 2859 source members, five prefix counts, five zero-counts | a fresh `tools/release.sh --no-gate` archive pair on this tree | Yes — every figure re-derived from the tarballs, none copied from (i) or (i.1) | ✓ FLOWING |

### Behavioral Spot-Checks

| Behavior | Command | Result | Status |
|----------|---------|--------|--------|
| The operator section runs as written on a tree with no `refs/` | `mv refs /tmp/…; sh -e <13 extracted lines>` | exit 0; `ivsr 3`, `report.end 78`, `end tick=2084 records=112 … break_hits=1 other_hits=0`, `112`, `1`, `0`, `# id: boot-stub`, `J $5000`, 13 step lines ending `step.mem.0200 7FFF` | ✓ PASS |
| `mkdir -p refs` is load-bearing, not decoration | `ivs-inspect trace … --out refs/op-trace.ivst` with `refs/` absent | exit **2**, `ivs-inspect: cannot resolve the output directory refs` — exactly the page's stated consequence | ✓ PASS |
| Report/trace/break/dis shapes the page states | `wc -l`, `sed -n '1p;$p'`, `grep -c '^break\.'`, `head`/`tail` on the four outputs | 79 report lines; 7 break keys in pinned order with `records 112`, `end_tick 2084`, `steps 1`; 8 dis header keys, `ORG $1000`, ends `J $5000`; trace's first record `kind=iab` | ✓ PASS |
| The claims step prints the new count | `ROOT=$PWD CHECK_DIR=$PWD/tools/check sh tools/check/steps/claims.sh` | `-- claims pass claims=29 provisional=28 handoffs=9`, exit 0 | ✓ PASS |
| The claim-artifact refusal can fail, and only for its own reason | the committed stand-in, then a repointed copy | `claim-handoff-artifact: CL-0005` with no `claim-dangling-path`; after the repoint 0 `CL-0005` findings with the other seven unchanged | ✓ PASS |
| The release line's clauses are recomputed | `sh tools/release.sh --no-gate --out /tmp/v-rel` then `tar -tzf` per clause | exit 0; every clause value as tabulated in truth 5 | ✓ PASS |
| The gate passes in both shells, sequentially, byte-identically | `sh tools/gate.sh >/tmp/g.out` then `dash tools/gate.sh >/tmp/d.out`; `cmp` of the summary lines | both exit 0, both `gate: PASS …`; summary lines **IDENTICAL** (`cmp` on the extracted last lines); the full logs differ only in `step-time` lines | ✓ PASS |
| Hygiene | `sh tools/hygiene.sh --tree` | silent, then `-- hygiene pass mode=tree hygiene-files=2859` | ✓ PASS |

**Note on `-- FAIL` occurrences.** `grep -c -- '-- FAIL'` returns **2** on each gate log, and both are the
*observed* text of a designed negative control, not gate failures:
`-- runner-selftest proof control-pass-line-not-last: profile (observed: -- FAIL control-pass-line-not-last: profile)`
and `-- runner-selftest proof runner-selftest-missing: st-no-case (observed: -- FAIL runner-selftest-missing: …)`.
The gate exits 0 and prints `gate: PASS`, so the §6 requirement is met in substance; the two SUMMARYs' phrase
"zero `-- FAIL` lines" is imprecise as literal grep arithmetic. Recorded as ℹ️ Info below, not as a gap.

### Probe Execution

No probe is declared by either plan and no `scripts/*/tests/probe-*.sh` exists in this repository, so Step 7c is
not applicable to this phase. The equivalent proof for a documentation phase — executing the page's own commands
— was run and is recorded under Behavioral Spot-Checks.

### Requirements Coverage

| Requirement | Source Plan | Description | Status | Evidence |
|-------------|-------------|-------------|--------|----------|
| VID-08 | `20-01-PLAN.md`, `20-02-PLAN.md` | The D10-VID-08 hardware question is a named packet rather than a wish: the exact trace lines a hardware probe must contradict are written down, the row exists in `data/expected/handoffs.ids` with `evidence: none`, and no claim cites it and no phase exit criterion names it | ✓ SATISFIED | Truths 2, 3 and 4: the row exists and prints `handoffs=9`; the packet carries 19 record fields and four per-guest comparison rows with both candidate values; `data/claims.json` names neither the ledger nor the id and the refusal was demonstrated in both directions. The record's own checkbox in `REQUIREMENTS.md` still reads `Pending` — flipping it is the verify-work workflow's act, not this record's |

No orphaned requirement: `REQUIREMENTS.md` maps **VID-08** and only VID-08 to Phase 20, and both plans declare it.

### Anti-Patterns Found

| File | Line | Pattern | Severity | Impact |
|------|------|---------|----------|--------|
| — | — | `TBD` / `FIXME` / `XXX` in any file the phase modified | — | **None.** The one grep hit is `mktemp "${TMPDIR:-/tmp}/claims.XXXXXX"` in an **unmodified** line of `claims.sh` — a shell template, not a debt marker. The debt-marker gate is clear |
| — | — | `TODO` / `HACK` / `PLACEHOLDER` in modified files | — | **None.** The `placeholder` hits are pre-existing `RELEASE-OPS.md` prose about the stage-2 record (lines 21, 56, 60, 70 — outside the `(i.2)` block) and the SUMMARY's own sentence |
| — | — | stub returns, hardcoded-empty props, ignored fetch results | — | **None.** No source file was modified by this phase: the diff touches two pages, two ledgers' worth of markdown/ids, one check step's two pin lines, one plan record and one packaging page |
| `20-01-SUMMARY.md`, `20-02-SUMMARY.md` | "zero `-- FAIL` lines" | ℹ️ Info | Evidence-accuracy wording: the gate log carries two `-- FAIL` strings inside `proof` observations. The gate passes and the controls are designed to fail; the phrase should read "no gate failure" | Not a gap: no truth depends on it and the gate is green |

### Recorded Deviations — judged

Each deviation the two SUMMARYs record, and whether it leaves a criterion unmet.

| # | Deviation | Judgement |
|---|-----------|-----------|
| 20-01 D1 | Task 1's constructed-negative example held four command lines where the predicate asserted five; one `dis` line was added to the example | **No criterion affected.** The change is inside a verify block, not in the page; the example is strictly stronger. A recorded, owned reading |
| 20-01 D2, 20-02 D2/D4 | `wc -l` pads on BSD, so `test "$(… \| wc -l)" = 4` failed on a correct file; `tr -d ' '` was added | **No criterion affected.** The subject (four pairs, nine rows) is unchanged and re-measured here independently: 4 and 9. Recorded, owned |
| 20-01 D3, 20-02 D3 | The pre-commit protected-branch limb is satisfied by project policy (`branching_strategy: none`, `use_worktrees: false`, sequential executor on `main`) rather than by `git.allow_default_branch_commits` | **No criterion affected.** A process limb, not a phase truth; the reason is written down and the alternative would have dirtied the tree and taken a different limb red. Recorded, owned |
| 20-02 D1 | The `### stic-strobe` anchor's fixed literal had to be written **bare** (no backticks around `docs/handoffs.md`) because the predicate greps the literal | **No criterion affected.** The plan's stated literal is unchanged and the page now carries it; verified here with `grep -F` on the exact phrase, and the negative limb (a section naming `D10-VID-08` without the sentence) still returns 0. Recorded, owned |
| 20-02 D5 | A constructed negative used a capitalised `No trace …` against a lowercase `grep -F` literal | **No criterion affected.** Example-only fix; the page's real bullet carries both the clause and its mechanism, verified here |

**One reading that is not a deviation but deserves the explicit judgement the task asks for: the packet's dual
observable for `G1`/`G2`.** Phase 19 registered `blocked_writes` as the observable for both guests, and
`blocked_writes` is machine state (`ivs_stic_report.blocked_writes`, read through `ivs_machine_inspect`) with **no
runner-report and no `ivs-inspect` report line** — confirmed by grep over `core/`, `host/` and `runner/`. A
capture recorded in `## Record format` therefore cannot carry it. The packet handles this honestly rather than
papering over it: each row gives the **registered** observable with both values *and* a **witness** (`frame 0
displayed`, which a capture does record), and the "Why the frame value alone does not separate the four"
paragraph states the consequence plainly — `G1` and `G2` carry the same witness pair, and what separates them is
the **stamp band** (`8000 < tick < 11600` vs `11600 < tick < 15120`) together with the display latch's state, so
"a capture that records no stamp cannot separate them at all". That paragraph is exactly what criterion 3 asks
for ("a list a probe's capture is compared against") and criterion 4's "naming the trace lines". **Judged: a
recorded, owned reading that leaves criteria 3 and 4 met — not a gap.** The one thing it cannot do is make the
frame word separate the rules, and it says so.

**One textual tension, judged explicitly.** `REQUIREMENTS.md` VID-08 says "no phase exit criterion names it",
while ROADMAP success criterion 2 names the row as the artifact to book. The two are consistent under W6's actual
prohibition: what is forbidden is the row becoming a *pass condition* or a *verdict*, and the ledger's own
grammar refuses the verdict vocabulary while `evidence: none` is pinned in `HANDOFF_DISP`. No criterion in the
milestone requires the packet's answer or the D11 sweep. **Judged: criterion 4 met.**

## Evidence limitations

- **`tools/gate-linux.sh`: NOT RUN.** The container daemon is unresponsive on this host — `docker info` did not
  return within 8 s (exit 124) and `build/gate-linux.log` is a 0-byte file dated 2026-11-26. Per `GUARDRAILS-v0.4`
  §4 this is recorded as a visible **not run**, never a pass. No Phase 20 truth depends on the Ubuntu gate: every
  artifact here is markdown, an id file or a check step's two pin lines, and the macOS gate ran the same 38 steps
  in both shells.
- **Cost checkpoint.** Recomputed by arithmetic from the last five rows of `.planning/ci-runs.tsv` rather than
  quoted: `build_s` {51, 51, 49, 49, 35} → median **49**; `gate_s` {171, 165, 164, 154, 115} → **164**; `cpu_s`
  {584, 580, 572, 530, 368} → **572**. Phase 20 is priced 0.02 against the 21 hosted cpu-s allowance and adds no
  in-process id, process-launch id or registry row, so the verdict is **`discharged-within-allowance`**. The
  SUMMARYs' arithmetic agrees with the file.
- **What was automated:** the gate in both shells, the thirteen operator commands extracted and executed, the
  claims step, the claim-refusal stand-in in both directions, the whole release dry run with every clause
  re-derived from its archives, the hygiene pass, `git ls-files` and the rights-pin count. **What was thrown
  away:** the four `refs/op-*` files, the scratch control tree and the release archive directory — all removed;
  the pre-existing owner `refs/` directory was moved aside for the no-`refs/` run and restored intact (13
  entries), and `git status --porcelain` is empty. **What was handed off:** the D10-VID-08 hardware probe and the
  D11 owner-local sweep, both as `not-performed` ledger rows with `evidence: none` — recorded, never performed
  and never a criterion. **What no tool can judge:** whether a real hardware capture laid against the packet's
  comparison list actually settles the strobe rule; that is the probe itself, deferred by design.
- **Tier ceiling: none**, as written before the work. `claims=29` and `provisional=28` are unmoved, no claim was
  minted, and no claim cites the row. The packet is a row, not a gate.

### Human Verification Required

**None.** No truth in this phase needs a person: the operator page was executed rather than proofread, the
packet's values trace to Phase 19's committed measurement, the refusal was shown failing, and the release line
was rebuilt from fresh archives. The two items a person does owe this project — the D10-VID-08 probe and the D11
sweep — are `not-performed` ledger rows with `evidence: none`, deliberately outside this phase's criteria, and
are not verification items for it.

### Gaps Summary

**No gaps.** All five ROADMAP success criteria and the four plan-level truths that add detail to them are
VERIFIED against the codebase and against commands re-run on the final tree. The phase added no capability, no
gate id, no claim and no rights token: the only count that moved is `handoffs` 8 → 9. The two SUMMARYs' claims
were checked rather than trusted, and the three places where they would have been easiest to over-trust — the
"as written" execution, the "recomputed" release line and the "shown able to fail" refusal — each reproduced
independently. The single evidence limitation is the container gate, which could not run on this host and is
recorded as `not run`, never as a pass.

---

_Verified: 2026-11-27T07:49:00Z_
_Verifier: Claude (gsd-verifier)_

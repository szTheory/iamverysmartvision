---
phase: 20-route-witness-and-release-posture
plan: 02
subsystem: witness-and-release-posture
tags: [handoff-row, d10-vid-08, comparison-list, stamp-band, claim-artifact-refusal, release-decision-line, recomputed-not-quoted]

requires:
  - phase: 19-discriminating-the-display-path
    plan: 01
    provides: "the four authored guests, their registered observables (`blocked_writes` for G1 and G2, the frame words for G3 and G4), their measured stamps (G1 at tick 9004, G2 at tick 12064) and the four profile fields the four rival rules belong to"
  - phase: 19-discriminating-the-display-path
    plan: 03
    provides: "`stic.disc.sweep-diagonal` and its four alternate-value masks `0xAAAA`, `0xCCCC`, `0xF0F0`, `0xFF00`, and Exit B: the candidates exist and no default was decided"
  - phase: 20-route-witness-and-release-posture
    plan: 01
    provides: "`docs/inspection.md` `## The operator route`, whose trace-record shapes the packet quotes, and the phase's start tree"

provides:
  - "`data/expected/handoffs.ids`'s ninth row `D10-VID-08 not-performed evidence: none`, with both pinned sets in `tools/check/steps/claims.sh` and all five control-tree ledgers moving in the same commit, so the step prints `handoffs=9`"
  - "`docs/handoffs.md` `## D10-VID-08: the display strobe and the latch, at swept offsets`: the frozen record fields, the four comparison rows with per guest its registered observable, its witness, its own stamp band and both candidate values, the four masks, the status line, the no-leak paragraph and the never-a-claim paragraph"
  - "`docs/stic/d10-procedure.md`'s `### stic-strobe` anchor naming the packet and the four candidate pairs"
  - "the claim-artifact refusal demonstrated in both directions on the committed stand-in `tools/check/controls/claims-handoff-artifact`, with no new id, registry row or control minted for it"
  - "`.planning/ops/RELEASE-OPS.md` `### (i.2)`: the close line complete in all seven clauses, the commit it was recomputed at, and a per-clause table whose every value was read from the archives the dry run wrote"
  - "`docs/packaging.md` `## Release content (D-33)`: the one clause the list was missing - a release carries no trace or disassembly output - with its `ivst` mechanism named inside the same region"

affects:
  - "`data/claims.json`: unmoved; no claim names the ledger or the row id, and `claims=29`/`provisional=28` hold"
  - "the gate line: the only count that moves is `handoffs` 8 to 9, plus `tracked`/`hygiene-files` by this SUMMARY"
  - "`.planning/ops/RELEASE-OPS.md` and `docs/packaging.md`: the release-decision line recomputed and the one release-content clause the page was missing"

tech-stack:
  added: []
  patterns:
    - "a handoff row is one line in an existing ledger whose id set and disposition set are pinned in the step that judges it, so the row, both pinned lists and every tree the step judges move in one commit or the control run goes red for the wrong reason"
    - "a packet is a comparison list a capture is laid against, not prose: per guest the field, the trace line that carries it, the registered observable with its two values, and the guest's own stamp band"

key-files:
  created:
    - .planning/phases/20-route-witness-and-release-posture/20-02-SUMMARY.md
  modified:
    - data/expected/handoffs.ids
    - tools/check/steps/claims.sh
    - tools/check/controls/_good/tree/data/expected/handoffs.ids
    - tools/check/controls/claims-handoff-id-extra/overlay/data/expected/handoffs.ids
    - tools/check/controls/claims-handoff-id-missing/overlay/data/expected/handoffs.ids
    - tools/check/controls/claims-handoff-id-duplicate/overlay/data/expected/handoffs.ids
    - tools/check/controls/claims-handoff-disposition/overlay/data/expected/handoffs.ids
    - docs/handoffs.md
    - docs/stic/d10-procedure.md
    - .planning/ops/RELEASE-OPS.md
    - docs/packaging.md

key-decisions:
  - "`G1` and `G2` carry the same witness pair, because one guest writes `$0020` once and the frame word reads the same under both rules; what separates them is the stamp band - `8000 < tick < 11600` for `G1` and `11600 < tick < 15120` for `G2` - together with the display latch's state, so the packet says plainly that a capture recording no stamp cannot separate the two."
  - "`G2`'s band is its own: it is separated by a write made after the rival window has already closed (2900 to 3780 CPU cycles, four ticks per cycle), and the packet's own row is asserted not to inherit `G1`'s band."
  - "No new mechanism was minted for the claim-refusal limb: the artifact predicate is already registered as `claims control claims-handoff-artifact EV-05` and its control already cites the ledger path, so the criterion is satisfied by running the committed stand-in - a second registration for one proof is the thing plan 13-04 named and refused, and the gate delta is zero."
  - "The `### stic-strobe` anchor carries the plan's fixed literal phrase written bare, with no markup around `docs/handoffs.md`, because the frozen predicate greps that literal exactly; the section's existing paragraph already names `D10-VID-08`, so a check on that token alone would pass whether or not the sentence landed."

patterns-established:
  - "A pinned id set is edited in both directions at once: `HANDOFF_IDS` and `HANDOFF_DISP` together, and every control tree the same step judges, or a control forbidding `missing id` fires for a reason that is not its own."

requirements-completed: [VID-08]

metrics:
  duration: "~1h40m"
  completed: 2026-11-27
  tasks: 2
  files: 10

status: complete

actuals:
  tokens: 11800
  tasks: 2
  commits: 2
---

# Phase 20 Plan 02: Route, Witness and Release Posture Summary

The D10-VID-08 hardware question becomes a named packet of the exact trace lines a probe must contradict, the ledger records it as one `not-performed` row that no claim can cite, and the release-decision line is recomputed clause by clause from the archives and the checks on this tree rather than quoted from an earlier record.

## Baselines

start-commit: 3cedf54280d4f9d1b8998bbc25b24bc916bd97fb

Measured at `start-commit` before the first edit of this plan, each count read from the file that produces it:

| Count | Value | Read from |
|---|---|---|
| handoff ledger data rows | 8 | `grep -v '^#' data/expected/handoffs.ids \| awk 'NF' \| wc -l` |
| installed paths | 11 | `grep -v '^#' data/expected/pkg-install-files.ids \| awk 'NF' \| wc -l` |
| rights pins | 4 | `grep -v '^#' data/expected/rights-pinned.ids \| awk 'NF' \| wc -l` |
| control directories (excluding `_good`) | 405 | `ls -d tools/check/controls/*/ \| grep -v _good \| wc -l` |
| claim entries | 29 | `grep -c '"id": "CL-' data/claims.json` |
| tracked files | 2858 | `git ls-files \| grep -c .` |

Gate summary line measured at `start-commit`, one `sh tools/gate.sh` run, verbatim:

```
gate: PASS steps=38/38 controls=405/405 api-calls=19 api-statuses=15 api-rows=61 pinned-defines=120 cc=AppleClang c-presets=2 fault-runs=238 core-imports=7 purity-selftest=7/7 ctest-runs=2 ci-runs=118 ci-uses=2 ci-workflows=2 ci-summary-selftest=11/11 claims=29 provisional=28 handoffs=8 cp1610-rows=74 patterns=1024 unclaimed=0 ambiguous=0 coincidences=1 agree=71 resolved=3 vectors=412 source=291 policy=121 rows-pending=0 policies-pending=0 bus-figure-pages=35 blind-files=12 second-read=12/12 scanned=705 waived=0 discrepancies=110 pending-resolvers=20 open=101 d10-items=47 faults=238/238 tracked=2858 binaries-listed=0 rights-pinned=4/4 terms=9 doc-files=64 profile-keys=155 goldens=29 reports=31 host-dependent=3 release-only=2 mode=tree hygiene-files=2858 history-commits=1 gates=123 lines=378 input-rows=31 input-policies=11 input-profile-fields=31 files=2858 listings=7 listing-words=1386 manifests=11 effective-yes=8 effective-no=3 rights=11 rights-cleared=1 literal-rows=80 views=2 dump-lines=160 prefix-files=11 archive=members public-symbols=19 defined=141 hdr-alone=10/10 consumer-lines=164 cxx-link=ok pkg-selftest=21/21 records=155 fields=155 unknown-fields=48 psg-rules=21 psg-vectors=120 psg-policies=12 release-selftest=13/13 runner-files=18 cases=14 sources=13 refs-verified=3/4 state-rows=88 state-cuts=18 tests-expected=5114 tests-executed=5108 tests-skipped=6 tests-missing=0 docs=65 claim-refs=27 stic-rules=67 stic-rows=64 video-frames=26 video-hash-ties=18 video-frame-pairs=9 tags=10 tiers=8
```

## Task 1: The ninth handoff row, its whole chain, the packet and the refusal

### The row and its chain

`data/expected/handoffs.ids` gains exactly one data row, `D10-VID-08 not-performed evidence: none`, after `H16-3`, and one appended header sentence naming it, its procedure section, its packet and the reason no tool on this host can perform it. The file now holds **nine** data rows.

`tools/check/steps/claims.sh` gains `D10-VID-08` in `HANDOFF_IDS` and `D10-VID-08=not-performed` in `HANDOFF_DISP`, in the same commit, with one appended sentence to the comment above them naming the row and the phase that added it.

The five control-tree ledgers the same step judges each gain the same row:

- `tools/check/controls/_good/tree/data/expected/handoffs.ids`
- `tools/check/controls/claims-handoff-id-extra/overlay/data/expected/handoffs.ids`
- `tools/check/controls/claims-handoff-id-missing/overlay/data/expected/handoffs.ids`
- `tools/check/controls/claims-handoff-id-duplicate/overlay/data/expected/handoffs.ids`
- `tools/check/controls/claims-handoff-disposition/overlay/data/expected/handoffs.ids`

The reason is mechanical: the step judges every one of those trees against the same pinned set, so after the pin each would report `missing id D10-VID-08`, and three of the four controls forbid exactly that token, which would take `controls=405/405` red. `claims-handoff-row`, `claims-handoff-missing` and `claims-handoff-unreadable` are deliberately **not** touched - the first two hold a grammar-broken or absent ledger and the step exits before the id-set pass, and the third has no ledger at all. `git diff --numstat` over those three directories is empty.

The step on the real tree, run directly:

```
-- claim pass records=29 claims=29 provisional=28
-- claims pass claims=29 provisional=28 handoffs=9
```

Exit 0, zero `-- FAIL` lines.

### The packet

`## D10-VID-08: the display strobe and the latch, at swept offsets` sits in `docs/handoffs.md` immediately after `## D10: the physical measurements` and before `## D11: the owner-local system image`. It carries, in order: the row and its procedure anchors (`### stic-strobe`, and `### stic-window-probes` for the gating candidate's window edges); the nineteen frozen record field names, quoted from `docs/inspection.md` `## The trace file` and from nowhere else; the comparison list; the paragraph on why the frame value alone does not separate the four; the status line; the no-leak paragraph; and the never-a-claim paragraph.

The comparison list, one row per guest:

| guest | field | band | registered observable | witness | mask |
|---|---|---|---|---|---|
| `G1` | `stic.reg_window_cpu_cycles` `2000` then `2900` | `8000 < tick < 11600` | `blocked_writes` `1` then `0` | `frame 0 displayed` `no` then `yes` | `0xAAAA` |
| `G2` | `stic.access_gating_when_off` `apply` then open-while-off | `11600 < tick < 15120` | `blocked_writes` `1` then `0` | `frame 0 displayed` `no` then `yes` | `0xCCCC` |
| `G3` | the `$0020` READ row of `stic.access_effects`, `none` then `arm` | (the same frame as the read) | `frame 0 displayed` `no` then `yes` | - (the frame word is the observable) | `0xF0F0` |
| `G4` | `stic.display_latch_cleared_each_frame` `1` then `0` | (exactly one write in the whole invocation) | `frame 0 displayed` `yes` then `yes`, `frame 1 displayed` `no` then `yes`, `frame 2 displayed` `no` then `yes` | - (the frame words are the observable) | `0xFF00` |

Every one of the nineteen frozen field names is present in the packet and present as a `field=` token in the frozen `## The trace file` block: `tick`, `kind`, `addr`, `value`, `wait`, `cycle`, `unit_start`, `unit_end`, `step`, `target`, `end`, `stop`, `records`, `units`, `steps`, `break_pc`, `break_hits`, `other_pc`, `other_hits`.

The status line reads the four candidates and nothing that could be mistaken for an answer:

```
Status: `G1` `0xAAAA`, `G2` `0xCCCC`, `G3` `0xF0F0` and `G4` `0xFF00` are the four rival candidates; each guest's two values are measured at the default of the other three fields, so every row is a comparison a capture is laid against and none of them is an answer. The row stays `not-performed` because no hardware probe has produced an answer.
```

The packet names no `refs/` path, no 64-hex digest, no content extension and no pinned rights spelling.

### The anchor

`docs/stic/d10-procedure.md`'s `### stic-strobe` gains one sentence after its existing paragraph and before `## Frame edge and timing`:

```
The four candidate pairs a capture is compared against, with the instrument's own lines for each, are the comparison list of the packet in docs/handoffs.md (`## D10-VID-08`); the capture is recorded in the `## Record format` shape unchanged, and nothing above this sentence changes.
```

The section's bounds are the next `##` heading, `## Frame edge and timing`, not the `### stic-frame-edge` subsection under it. The page lost no line: `git diff --numstat` shows 2 additions and 0 deletions.

### The claim-refusal demonstration, in both directions

The scratch copy is the good tree plus the control's `overlay/`, and **without** the control's `MAKE` transforms, so three ids are refused for their own reasons there and only `CL-0005`'s token and its absence of `claim-dangling-path` are judged. The exact commands:

```sh
rm -rf /tmp/p20ctl && mkdir -p /tmp/p20ctl
cp -R tools/check/controls/_good/tree /tmp/p20ctl/tree
cp -R tools/check/controls/claims-handoff-artifact/overlay/. /tmp/p20ctl/tree/
ROOT=/tmp/p20ctl/tree CHECK_DIR=$PWD/tools/check sh tools/check/steps/claims.sh
```

Its findings, in full:

```
-- FAIL claim-handoff-artifact: CL-0001
-- FAIL claim-handoff-artifact: CL-0002
-- FAIL claim-dangling-path: CL-0003
-- FAIL claim-dangling-path: CL-0004
-- FAIL claim-handoff-artifact: CL-0005
-- FAIL claim-handoff-artifact: CL-0006
-- FAIL claim-handoff-artifact: CL-0007
-- FAIL claim-dangling-path: CL-0008
```

`CL-0005` is refused by `claim-handoff-artifact` and by **no** `claim-dangling-path`: the ledger exists, so the refusal is the handoff predicate and not the missing-file token.

Then `CL-0005`'s `artifact` was repointed in the scratch copy from `data/expected/handoffs.ids` to `data/expected/profile-fields.ids` - a path the toy tree holds and which is in none of the five refused classes:

```sh
python3 - <<'PY'
import json
p='/tmp/p20ctl/tree/data/claims.json'
d=json.load(open(p))
for c in d['claims']:
    if c['id']=='CL-0005': c['artifact']='data/expected/profile-fields.ids'
json.dump(d,open(p,'w'),indent=2)
PY
ROOT=/tmp/p20ctl/tree CHECK_DIR=$PWD/tools/check sh tools/check/steps/claims.sh
```

The re-run prints **no** `CL-0005` finding, and still prints four `claim-handoff-artifact` findings (`CL-0001`, `CL-0002`, `CL-0006`, `CL-0007`) and the same three `claim-dangling-path` findings - so the refusal is neither absent nor blanket.

The two real-tree facts: `data/claims.json` contains **no** occurrence of `handoffs.ids` and **none** of `D10-VID-08`, and it still holds 29 `CL-` entries with `provisional=28`.

### Why no new mechanism was minted

The artifact predicate is already registered as `claims control claims-handoff-artifact EV-05` in `data/expected/important-gates.ids`, and its control already cites the ledger path. The criterion's "shown able to fail on a committed stand-in" is therefore satisfied by **running the committed stand-in**, not by minting a second one; a second registration for one proof is the thing plan 13-04 named and refused. The gate delta of that decision is zero: `gates=123`, `lines=378`, `controls=405/405`, `tests-expected=5114`.

## Task 2: The release-decision line recomputed clause by clause

### The recomputation

One fresh asset directory, never `build/release-assets`, so a stale asset could not be read as this invocation's:

```sh
rm -rf /tmp/p20-rel && mkdir -p /tmp/p20-rel
sh tools/release.sh --no-gate --out /tmp/p20-rel
```

Exit 0, zero `release: FAIL` lines. Its own lines, in the order it prints them:

```
release: notes: CHANGELOG.md section [0.3.0] present
release: tag v0.3.0, publish=0, release-checks=0, gate=skipped
release-assets source ok members=2859
release-assets install ok members=11
release: consumer against ivs-v0.3.0-install-darwin-arm64.tar.gz: output equals data/expected/pkg-consumer.golden (164 lines)
```

The tag is the header macros' version and not a milestone label, and the note says so.

Then, from the archives that invocation wrote and from nothing else:

| reading | command | value |
|---|---|---|
| source archive member count | `tar -tzf <src> \| grep -vc '/$'` | 2859, equal to `git ls-files \| grep -c .` = 2859 |
| install archive member count | `tar -tzf <inst> \| grep -vc '/$'` | 11 |
| install archive vs the pinned inventory | strip the archive prefix, `sort`, `cmp` | **identical**, both directions, 11 paths each |
| `tools/inspect/` in the source archive | `grep -c -F -e '/tools/inspect/'` | 7 members |
| `tools/pack/` | same | 3 members |
| `tools/asm/` | same | 9 members |
| `tools/flatwrap/` | same | 5 members |
| `host/` | same | 6 members |
| shared objects in either archive | `grep -E '\.(so\|dylib\|dll\|bundle)$'` | 0 and 0 |
| `libivs_retro*` or `ivs-inspect` members | `grep -E -e 'libivs_retro' -e 'ivs-inspect'` | 0 |
| `refs/` or `.ivst` members | `grep -E -e '(^\|/)refs/' -e '\.ivst$'` | 0 |
| content-extension members | `grep -E '\.(ivsc\|bin\|grom\|img\|rom\|exec)$'` | 0 |
| `bin/` in the install archive | `grep -c -E '(^\|/)bin/'` | 0 |
| tracked paths with a content extension | `git ls-files \| grep -E` | none |

The four figures the note cites, read from the gate line measured at `start-commit`: `prefix-files=11`,
`binaries-listed=0`, `release-selftest=13/13`, `pkg-selftest=21/21`. The temporary asset directory was removed
once the readings were recorded: the phase leaves no archives behind.

### The note

`### (i.2) The line recomputed at plan 20-02 (VID-08, GUARDRAILS-v0.4 section 7.4)` is appended to
`.planning/ops/RELEASE-OPS.md` after `### (i.1)` and before `## (j)` (line 177, between 114 and 214), as an
appended note: neither (i) nor (i.1) lost a line. It carries the close line as one block quote with all seven
clauses; the statement that this is the guardrails' close line complete for the first time, because (i) named the
three missing clauses and this note is their recomputation; the commit it was recomputed at; the per-clause table;
and one sentence on what did not move.

The note also states precisely where its values came from, because the archives were rebuilt on the tree that
carries Task 1's commit `e10c45f` and not on the start commit itself. The only difference between the two trees is
this plan's own record files, so the single value that moves is the source archive's member count - a count of
tracked files, which includes this plan's SUMMARY. Every other clause's value is identical at both commits, and
the note says so rather than implying the archives were built at the start commit.

### The release-content clause `docs/packaging.md` was missing

`## Release content (D-33)` stated six of the seven clauses and was missing exactly one: nothing there said a
release carries no trace or disassembly output. One bullet is added immediately after the "No shared object is in
a release" bullet, in the page's own register, resting on mechanisms that exist rather than on intent: the
instrument is build-only and never installed, its output is refused inside the work tree except under the ignored
`refs/`, `refs/` is not a member of either archive, the `ivst` extension is binary by name in
`tools/check/binary-ext.txt` so a tracked trace is refused in the tree, in an archive and in a copy of the
installed prefix, and nothing the instrument writes is ever committed. No claim tag is added, because this is a
statement of what ships and not a claim. The page lost no line: 1 addition, 0 deletions.

## Token line, before and after

| token | before | after | why |
|---|---|---|---|
| `handoffs` | 8 | **9** | the one row, counted from `data/expected/handoffs.ids` |
| `controls` | 405/405 | 405/405 | five control-tree ledgers gain a row; no control directory added or removed |
| `claims` / `provisional` | 29 / 28 | 29 / 28 | no claim minted or edited |
| `gates` / `lines` | 123 / 378 | 123 / 378 | no registry row |
| `steps` / `faults` / `goldens` | 38/38 / 238/238 / 29 | same | nothing added to the gate |
| `prefix-files` / `rights-pinned` | 11 / 4/4 | same | no installed path, no rights token |
| `tests-expected` / `tests-executed` | 5114 / 5108 | same | no test id |
| `d10-items` | 47 | 47 | no D10 inventory row |
| `tracked` / `hygiene-files` | 2858 | 2859 | `20-02-SUMMARY.md`; every other file is a modification |
| `binaries-listed` / `release-selftest` / `pkg-selftest` | 0 / 13/13 / 21/21 | same | the `release-assets` step and its case list are unchanged |

## Cost arithmetic

Recomputed by arithmetic from the last five rows of `.planning/ci-runs.tsv`, never quoted:

| column | the five values | median |
|---|---|---|
| `build_s` | 51, 51, 49, 49, 35 | **49** |
| `gate_s` | 171, 165, 164, 154, 115 | **164** |
| `cpu_s` | 584, 580, 572, 530, 368 | **572** |

This plan adds zero in-process ids (0.065), zero process-launch ids (0.10) and zero registry rows (0.005), so the frozen unit prices give **0.00**; its share of the phase's 0.02 is **0.01**, charged against the 21 hosted cpu-s allowance. The release dry run is a local build-and-package step and adds nothing to the hosted window. The checkpoint verdict is **`discharged-within-allowance`**.

## Deviations from Plan

### 1. Rule 1 - Plan defect: the `### stic-strobe` anchor predicate could not be satisfied by the plan's own constructed positive

**Found during:** Task 1, before the anchor sentence was written.

**Issue:** the plan fixes the anchor's literal phrase as `the comparison list of the packet in docs/handoffs.md` and the verify greps that exact string with `grep -F`. The plan's own constructed **positive** example for that predicate writes the path inside backticks - `the comparison list of the packet in `docs/handoffs.md`` - so the literal the grep looks for does not occur in it and the limb returns 0 on the example it is meant to accept. This is lesson 34's class again: the predicate's positive limb cannot pass.

**Fix:** the page's sentence carries the phrase **bare**, exactly as the plan's fixed literal reads, with the backticked `## D10-VID-08` section after it. The constructed positive in the verify block was rewritten to carry the same bare phrase, so the predicate and its positive example now agree. The plan's stated literal is unchanged, no limb was dropped, and the negative limb (a section that names `D10-VID-08` without the sentence) still returns 0 and is still asserted.

**Files modified:** `docs/stic/d10-procedure.md` (the sentence), and the constructed positive inside the verify block only.

### 2. Rule 1 - Plan defect: `wc -l` pads on BSD in two baseline limbs

**Found during:** Task 1, first verify run.

**Issue:** the limbs that read the ledger's row count and the rights-pin count through `... | wc -l` compare the result with `9` and `4`. On this host's `wc`, `wc -l` pads to a fixed width, so the substitution is `"       9"` and the comparison fails on a correct file. This is the same defect class plan 20-01 hit, and the fix is the same.

**Fix:** the two substitutions strip the padding with `tr -d ' '` before comparing. The assertions' subjects - nine data rows, four pairs - are unchanged.

**Files modified:** none (the predicates live only inside the verify block).

### 3. The pre-commit protected-branch limb, as recorded in plan 20-01

`gsd_run query git.base-branch --is-protected main` reads `true`, `git.allow_default_branch_commits` is not set, and `.planning/config.json` must never be staged by an executor. The project configures `branching_strategy: none` with `use_worktrees: false`, the orchestrator dispatched a **SEQUENTIAL** executor onto the main working tree, and every prior phase of this repository committed on `main`. Recorded rather than worked around silently.

### 4. Rule 1 - Plan defect: `wc -l` padding, as in deviation 2

The `(a)` limb that counts the ledger's data rows and the `(j)` limbs that count `CL-` entries were unaffected
because they use `grep -c` and `awk`, but the Task 1 limb counting the ledger rows through `wc -l` needed the same
`tr -d ' '` fix recorded above. No further limb needed adjusting: every other count in this plan's two verify
blocks is a `grep -c` or an `awk` count, which does not pad.

## Open items

- `tools/gate-linux.sh` was **not run**: the container daemon is unresponsive on this host, which the orchestrator's rules for this phase record. A visible `not run` line is carried here rather than a pass.

### 5. Rule 1 - Plan defect: the packaging constructed negative used a capitalised clause

The `(g)` limb builds a constructed bullet that states the clause and drops the `ivst` mechanism, and asserts
that the clause literal is present and the mechanism absent. It wrote the bullet as `- No trace or disassembly
output is in a release.`, with a capital `N`, while the predicate greps the lowercase literal
`no trace or disassembly output` with `grep -F`. The negative therefore reported "clause absent" and failed on
the example it was meant to accept as the clause-bearing case. The fix is in the constructed example, not in the
page or the predicate: the bullet now reads `- A release carries no trace or disassembly output.`, which is
also the page's own register. The limb still proves what it was written to prove - the clause present, the
mechanism absent, so a bullet without its mechanism is not accepted - and the page's real bullet carries both.

## Final gate

Both shells, sequentially and never concurrently (lesson 33), each summary line captured and the two compared
with `cmp`.

`sh tools/gate.sh` - exit 0, zero `-- FAIL` lines:

```
gate: PASS steps=38/38 controls=405/405 api-calls=19 api-statuses=15 api-rows=61 pinned-defines=120 cc=AppleClang c-presets=2 fault-runs=238 core-imports=7 purity-selftest=7/7 ctest-runs=2 ci-runs=118 ci-uses=2 ci-workflows=2 ci-summary-selftest=11/11 claims=29 provisional=28 handoffs=9 cp1610-rows=74 patterns=1024 unclaimed=0 ambiguous=0 coincidences=1 agree=71 resolved=3 vectors=412 source=291 policy=121 rows-pending=0 policies-pending=0 bus-figure-pages=35 blind-files=12 second-read=12/12 scanned=705 waived=0 discrepancies=110 pending-resolvers=20 open=101 d10-items=47 faults=238/238 tracked=2859 binaries-listed=0 rights-pinned=4/4 terms=9 doc-files=64 profile-keys=155 goldens=29 reports=31 host-dependent=3 release-only=2 mode=tree hygiene-files=2859 history-commits=1 gates=123 lines=378 input-rows=31 input-policies=11 input-profile-fields=31 files=2859 listings=7 listing-words=1386 manifests=11 effective-yes=8 effective-no=3 rights=11 rights-cleared=1 literal-rows=80 views=2 dump-lines=160 prefix-files=11 archive=members public-symbols=19 defined=141 hdr-alone=10/10 consumer-lines=164 cxx-link=ok pkg-selftest=21/21 records=155 fields=155 unknown-fields=48 psg-rules=21 psg-vectors=120 psg-policies=12 release-selftest=13/13 runner-files=18 cases=14 sources=13 refs-verified=3/4 state-rows=88 state-cuts=18 tests-expected=5114 tests-executed=5108 tests-skipped=6 tests-missing=0 docs=65 claim-refs=27 stic-rules=67 stic-rows=64 video-frames=26 video-hash-ties=18 video-frame-pairs=9 tags=10 tiers=8
```

`dash tools/gate.sh` - exit 0, zero `-- FAIL` lines, and `cmp` of the two summary lines: **identical**.

`sh tools/hygiene.sh --tree` - silent, then `-- hygiene pass mode=tree hygiene-files=2859`.

The only token that moved across the whole phase is `handoffs` 8 to 9 (and `tracked`/`hygiene-files` by the two
SUMMARYs): `steps=38/38`, `controls=405/405`, `faults=238/238`, `goldens=29`, `claims=29 provisional=28`,
`prefix-files=11`, `rights-pinned=4/4`, `gates=123 lines=378`, `d10-items=47`,
`tests-expected=5114 tests-executed=5108` are all unmoved.

Container gate: **not run** - `tools/gate-linux.sh` needs a responsive container daemon, which this host does not
have; recorded as a visible `not run` line rather than a pass.

## Self-Check: PASSED

- `data/expected/handoffs.ids` exists and holds nine data rows, the last being `D10-VID-08 not-performed evidence: none`.
- `docs/handoffs.md` exists and carries `## D10-VID-08: the display strobe and the latch, at swept offsets`.
- `docs/stic/d10-procedure.md` exists and carries the anchor sentence inside `### stic-strobe`.
- `tools/check/steps/claims.sh` exists and names both pinned entries for the row.
- `.planning/ops/RELEASE-OPS.md` exists and carries `### (i.2)` between `### (i.1)` and `## (j)`.
- `docs/packaging.md` exists and carries the trace-and-disassembly clause inside `## Release content (D-33)`.

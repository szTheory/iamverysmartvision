---
phase: 18-the-inspection-surface
verified: 2026-11-27T03:14:54Z
status: passed
score: 26/26 must-haves verified
covered_files: [
  ".agents/skills/ops-hygiene/SKILL.md", ".gitignore", ".planning/phases/18-the-inspection-surface/18-01-PLAN.md",
  ".planning/phases/18-the-inspection-surface/18-01-SUMMARY.md", ".planning/phases/18-the-inspection-surface/18-02-PLAN.md", ".planning/phases/18-the-inspection-surface/18-02-SUMMARY.md",
  ".planning/phases/18-the-inspection-surface/18-03-PLAN.md", ".planning/phases/18-the-inspection-surface/18-03-SUMMARY.md", "CMakeLists.txt",
  "README.md", "data/expected/controls/rights.ids", "data/expected/important-gates.ids",
  "data/expected/pkg-selftests.ids", "data/expected/release-assets-selftests.ids", "data/expected/rights.ids",
  "data/expected/tests/asm.ids", "data/expected/tests/runner-process.ids", "data/rights/inspection-output.json",
  "docs/RIGHTS.md", "docs/inspection.md", "docs/packaging.md",
  "tests/CMakeLists.txt", "tests/asm-tap.sh", "tests/runner-process-tap.sh",
  "tools/check/binary-ext.txt", "tools/check/controls/_good/tree/.planning/REQUIREMENTS.md", "tools/check/controls/_good/tree/.planning/milestones/v0.1.0-REQUIREMENTS.md",
  "tools/check/controls/_good/tree/CMakeLists.txt", "tools/check/controls/_good/tree/data/expected/important-gates.ids", "tools/check/controls/_good/tree/tools/inspect/good.c",
  "tools/check/controls/files-unlisted-binary-ivst/CHECK", "tools/check/controls/files-unlisted-binary-ivst/EXPECT", "tools/check/controls/files-unlisted-binary-ivst/MAKE",
  "tools/check/controls/important-gates-bad-req-phase5/overlay/data/expected/important-gates.ids", "tools/check/controls/important-gates-bad-req-phase6/overlay/data/expected/important-gates.ids", "tools/check/controls/important-gates-complete-uncovered/overlay/.planning/REQUIREMENTS.md",
  "tools/check/controls/important-gates-req-uncovered-phase5/overlay/data/expected/important-gates.ids", "tools/check/controls/important-gates-req-uncovered-phase6/overlay/data/expected/important-gates.ids", "tools/check/controls/important-gates-req-uncovered-phase7/overlay/data/expected/important-gates.ids",
  "tools/check/controls/important-gates-uncovered-gate/overlay/data/expected/important-gates.ids", "tools/check/controls/important-gates-unknown-id-phase5/overlay/data/expected/important-gates.ids", "tools/check/controls/important-gates-unknown-id-phase6/overlay/data/expected/important-gates.ids",
  "tools/check/controls/important-gates-unknown-id/overlay/data/expected/important-gates.ids", "tools/check/integrity.cmake", "tools/check/steps/c-build.sh",
  "tools/check/steps/important-gates.sh", "tools/check/steps/pkg-install.sh", "tools/check/steps/release-assets.sh",
  "tools/inspect/CMakeLists.txt", "tools/inspect/inspect.c", "tools/inspect/inspect_dis.c",
  "tools/inspect/inspect_trace.c"
]
covered_digest: "v4:sha256:27e1dc0bb0476f4058a270b59942db5a6aef2e823f7295394ce0dc26f9c1847d"
behavior_unverified: 0
overrides_applied: 0
re_verification: null
---

# Phase 18: The Inspection Surface Verification Report

**Phase Goal:** A build-only, scripted, headless inspection surface exists over the seams the core already
exposes — disassembly of the project's own listings, breakpoints with an asserted hit and non-hit count, and a
cycle-stamped access trace that reconciles with the runner's own report — and its output is an operator-local,
rights-bearing artifact.
**Verified:** 2026-11-27T03:14:54Z
**Status:** passed
**Re-verification:** No — initial verification
**Tier ceiling:** **none** (`GUARDRAILS-v0.4` §3). This phase mints no claim, cites none, and reaches no
evidence tier. `claims=29` and `provisional=28` are unmoved and were re-read from the gate's own line.

## Verification posture

Every claim below is a command run on the final tree at `9a51d68`, not a quotation from a SUMMARY. Where a
SUMMARY's figure disagrees with the tree, the tree's figure is recorded and the difference is named (one such
case: `18-02-SUMMARY.md` deviation 13 predicted `tests-expected=5070` and called the dispatch's `5082` wrong;
`18-03`'s six ids then moved `5070 → 5082`, so the dispatch's figure is the one the tree reads and no
re-bless was involved).

---

## Gate on the final tree (`GUARDRAILS-v0.4` §6.1, §6.2)

```
sh   tools/gate.sh   -> rc 0, `grep -c '^-- FAIL '` = 0
dash tools/gate.sh   -> rc 0, `grep -c '^-- FAIL '` = 0
cmp -s <last line sh> <last line dash>  -> IDENTICAL
```

Both shells end in the same line, byte for byte:

```
gate: PASS steps=38/38 controls=405/405 api-calls=19 api-statuses=15 api-rows=61 pinned-defines=120 cc=AppleClang c-presets=2 fault-runs=238 core-imports=7 purity-selftest=7/7 ctest-runs=2 ci-runs=118 ci-uses=2 ci-workflows=2 ci-summary-selftest=11/11 claims=29 provisional=28 handoffs=8 cp1610-rows=74 patterns=1024 unclaimed=0 ambiguous=0 coincidences=1 agree=71 resolved=3 vectors=412 source=291 policy=121 rows-pending=0 policies-pending=0 bus-figure-pages=35 blind-files=12 second-read=12/12 scanned=702 waived=0 discrepancies=110 pending-resolvers=20 open=101 d10-items=47 faults=238/238 tracked=2834 binaries-listed=0 rights-pinned=4/4 terms=9 doc-files=64 profile-keys=154 goldens=29 reports=30 host-dependent=3 release-only=2 mode=tree hygiene-files=2834 history-commits=1 gates=122 lines=372 input-rows=31 input-policies=11 input-profile-fields=31 files=2834 listings=7 listing-words=1386 manifests=11 effective-yes=8 effective-no=3 rights=11 rights-cleared=1 literal-rows=80 views=2 dump-lines=160 prefix-files=11 archive=members public-symbols=19 defined=141 hdr-alone=10/10 consumer-lines=164 cxx-link=ok pkg-selftest=21/21 records=154 fields=154 unknown-fields=47 psg-rules=21 psg-vectors=120 psg-policies=12 release-selftest=13/13 runner-files=18 cases=14 sources=13 refs-verified=3/4 state-rows=88 state-cuts=18 tests-expected=5082 tests-executed=5078 tests-skipped=4 tests-missing=0 docs=65 claim-refs=27 stic-rules=67 stic-rows=64 video-frames=26 video-hash-ties=18 video-frame-pairs=9 tags=10 tiers=8
```

Token-by-token against the phase's final state — all fifteen the brief names read exactly:

| token | brief | gate | |
|---|---|---|---|
| `steps` | 38/38 | `steps=38/38` | ✓ |
| `faults` | 238/238 | `faults=238/238` | ✓ |
| `goldens` | 29 | `goldens=29` | ✓ |
| `claims` | 29 | `claims=29` | ✓ |
| `provisional` | 28 | `provisional=28` | ✓ |
| `prefix-files` | 11 | `prefix-files=11` | ✓ |
| `controls` | 405/405 | `controls=405/405` | ✓ |
| `rights` | 11 | `rights=11` (`rights-cleared=1`) | ✓ |
| `pkg-selftest` | 21/21 | `pkg-selftest=21/21` | ✓ |
| `release-selftest` | 13/13 | `release-selftest=13/13` | ✓ |
| `tests-expected` | 5082 | `tests-expected=5082` | ✓ |
| `tests-executed` | 5078 | `tests-executed=5078` (`tests-skipped=4`, `tests-missing=0`) | ✓ |
| `gates` | 122 | `gates=122` | ✓ |
| `lines` | 372 | `lines=372` | ✓ |
| `docs` | 65 | `docs=65` (`doc-files=64`) | ✓ |

Also green: `sh tools/hygiene.sh --tree` → `-- hygiene pass mode=tree hygiene-files=2834`, silent of findings;
`git ls-files | grep -E '\.(ivsc|bin|rom|so|dylib|cfg|ivsn|ivst)$'` → empty; `git status --porcelain` → empty.

`tracked`/`files`/`hygiene-files` read **2834**, not the `2832` the `18-03` record measured at `b639319`. The
two extra files are `18-03-SUMMARY.md` and `18-REVIEW.md`, both added after that measurement; the arithmetic
recomputes, so this is not a drift.

### Container gate — `not run`, and never claimed as a pass (`GUARDRAILS-v0.4` §4, lesson 24)

Measured twice on this host, not assumed:

```
docker info            -> no answer, rc 124 at a 15 s bound
sh tools/gate-linux.sh -> rc 124 at a 90 s bound, no output at all, build/gate-linux.log 0 bytes
docker ps -a           -> no answer; no container, image or volume created
```

The visible `not run` line lives in the phase records, not in the log (the script truncates the log through
its `docker run` redirect, and docker never starts):

```
gate-linux: not run — docker daemon unresponsive on this host (retried at 2026-11-27T01:57:58Z, `docker info` timed out after 20s with no answer)
```
(`18-03-SUMMARY.md` deviation 6; `18-02-SUMMARY.md` deviation 14 records the same for its own commit.)

**The container check is outstanding evidence, not a pass.** It is a local pre-push helper and not a gate step
(`tools/gate-linux.sh:3`), so its absence does not weaken any id this phase added — every id below ran under
the local gate on this host, and `ctest --preset dev -R 'runner-process|asm'` was re-run here: `asm` 7.39 s
passed, `runner-process` 28.93 s passed, 2/2.

---

## Goal Achievement

### Observable Truths

Nine from `18-01` (the frozen oracle), ten from `18-02` (the instrument), seven from `18-03` (the disassembly
limb). Merged, deduplicated, and **added to** by the five ROADMAP success criteria — never reduced by them.

| # | Truth | Status | Evidence |
|---|---|---|---|
| 1 | `docs/inspection.md` exists and is committed in a commit of its own, before any path under `tools/inspect/` exists anywhere in the branch history | ✓ VERIFIED | `git log --diff-filter=A -- docs/inspection.md` → one commit, `0627be9`; `git show --stat 0627be9` → `docs/inspection.md \| 253 +++`, 1 file; `git ls-tree -r --name-only 0627be9 \| grep -c 'tools/inspect/'` → 0 |
| 2 | The freeze is not rewritable: the page at its freeze commit is a byte prefix of the page now | ✓ VERIFIED | `git show 0627be9:docs/inspection.md` 15,699 B vs `docs/inspection.md` 21,194 B; `now.startswith(frozen)` → `PREFIX: True` |
| 3 | The page pins one binary, four subcommands (`trace`, `break`, `dis`, `step`), each grammar, and the three exit statuses | ✓ VERIFIED | page `## Command surface`; `ivs-inspect --help` prints all four grammars and `exit: 0 …, 1 …, 2 …`, rc 0, nothing on stderr; `inspect.surface` asserts `--help` names each, an unknown subcommand exits 2 with a usage line and no token, `break` without `--pc` exits 2 |
| 4 | The page pins the trace's three record kinds, every field name, `tick=` first, and resolves O1 (the `end` record's tick is the last step's end) | ✓ VERIFIED | page `## The trace file`; the live trace: `tick=4 kind=fetch …` first field `tick=`, `step=1 target=16000 end=2084 stop=halted`, `end tick=2084 records=112 units=70 steps=1 …` |
| 5 | The page pins the break report's seven keys and the step report's keys by name | ✓ VERIFIED | live `break` prints `break.pc/hits/other.pc/other.hits/records/end_tick/steps` in that order; live `step` prints `step.units/ticks/halted/pc/reg.R0…R7/mem.<AAAA>` in that order |
| 6 | The page pins the five refusal tokens each with a trigger and an accepted boundary, and the liveness guards with their stand-ins | ✓ VERIFIED | page block; the tool's token set equals the page's block in both directions: `sed -n '/^## Refusal tokens/,/^## Liveness guards/p' docs/inspection.md \| grep -o -E 'inspect-[a-z-]+' \| sort -u` = `grep -h -o -E '"inspect-[a-z-]+"' tools/inspect/*.c` = the same five names; `inspect.surface` asserts `-eq 5` on both sides and both `comm` directions empty; guards 1–5 each have a committed stand-in (§ Stand-ins below). Guard 2's wording is noted at O1 |
| 7 | The page states the disassembly limb's source, its re-assembler oracle, and the drop's reversal condition | ✓ VERIFIED | page `## Disassembly`; the limb was **built**, not dropped (`18-03-SUMMARY.md`, key-decisions), so the drop note was correctly not appended |
| 8 | The page is written from seams that exist today and says the report is the library's and the trace the tool's own loop | ✓ VERIFIED | `tools/inspect/inspect.c:678,748` (`ivs_scenario_parse`, `ivs_runner_execute`), `inspect_trace.c:590,593,698,783,791,793,809,816` (`ivs_input_apply_due`, `ivs_machine_step_unit_trace_internal`, `ivs_machine_state_digest`, `ivs_machine_cpu_state`, `ivs_machine_peek`); `ivs_runner_report_write` appears in **no** tool source |
| 9 | The page passes the gate's prose rules with no waiver invented and no claim minted | ✓ VERIFIED | gate `claims=29 provisional=28 claim-refs=27` unmoved; `text` and `glossary` steps green inside the gate; the two `[nc]` waivers are recorded as `18-01` deviation 2 |
| 10 | One build-only target `ivs_inspect` → `ivs-inspect` under both presets, no install rule, linking `ivs_runner_lib ivs_core ivs_core_internal ivs_warnings ivs_san`; in the SPDX scan and the `c-build` list; the toy tree writes the eighth list | ✓ VERIFIED | `tools/inspect/CMakeLists.txt:22-32` (source list, `add_executable`, `target_link_libraries`, **no `install(`**); `tools/check/integrity.cmake:82` `tools/(pack\|cart\|pcm\|asm\|bench\|flatwrap\|inspect)/`; `tools/check/steps/c-build.sh:51` `'^tools/inspect/.*[.]c$ ivs-inspect-sources.txt'`; `tools/check/controls/_good/tree/CMakeLists.txt:17` writes `ivs-inspect-sources.txt` and `tools/inspect/good.c` exists |
| 11 | The report is the library's bytes through `write_report`; the trace is the tool's own per-unit loop; the tool never calls `ivs_runner_report_write` | ✓ VERIFIED | § truth 8's grep (zero hits for the writer); the tool's nine `digest.*` lines are byte-identical to `ivs-run`'s for the same scenario: `cmp` on the two 9-line extractions → identical (whole-report diff is `output.audio.*` only, i.e. the host sink, not the machine) |
| 12 | The two passes are provably the same run: nine digests equal, each step `end` = that step's `now_ticks`, end `tick` = last step `end`, ticks non-decreasing **and not all equal**; failure is `inspect-reconcile`, exit 1, no output file | ✓ VERIFIED | `inspect_trace.c:698,707` (instance digests vs the captured text), `:647` (`now_ticks`), `:663-668` (fifth equality); live run: `end tick=2084` = `step.1.now_ticks 2084`, 112 access records, 112 distinct ticks; the poked-report run → `ivs-inspect: inspect-reconcile: … digest.cpu …`, rc 1, 0 B stdout, no output file |
| 13 | The trace ids live in the existing `tests/runner-process-tap.sh` as its ninth argument and assert a domain, never a bare exit status | ✓ VERIFIED | `tests/runner-process-tap.sh:121-159` (ninth argument; a missing one is `not ok` with a reason, never a skip — `:3128-3129`); the ids assert `records > 0`, a tick domain, a witness pair and byte equalities (`ix_records`, `ix_tick_domain`, `ix_digests_equal`, `ix_break_pair`, `ix_end_tick`) |
| 14 | Breakpoints are a pair in one id (hit 1 at `1000`, 0 at `1002`) with the in-run and file-derived counts reconciling, and non-interference is a second id | ✓ VERIFIED | live: `break_hits=1 other_hits=0`; `break` report `break.hits 1` / `break.other.hits 0`; `inspect.break-noninterference` asserts the nine digests and the access records byte-identical armed vs unarmed, and the two traces differ on exactly the two `end` lines |
| 15 | The `step` subcommand reads only public side-effect-free seams and its id is a count plus a witness pair | ✓ VERIFIED | `inspect_trace.c:809,816` only (`ivs_machine_cpu_state`, `ivs_machine_peek`) for observation; live `step --units 70 --peek 0200 3` → `step.units 70` (the trace's own `units`), `step.ticks 2084` (the trace's end tick), `step.halted yes`, `step.reg.R0 03C5 … R6 0340`, `step.mem.0200 7FFF` — each equal to the scenario's own `assert reg`/`assert mem` lines |
| 16 | Four committed stand-ins each fail for its own stated reason, judged by the same check the real case uses | ✓ VERIFIED | five stand-ins, all reproduced by hand below; each declared token set is exactly one, and each case reports `judge-vacuous` when its check accepts the stand-in and `judge-wrong-reason` otherwise |
| 17 | The output policy is decided before anything is created: three in-tree spellings refused with `inspect-output-tree` while `refs/` is accepted; an existing path refused with `inspect-output-exists` byte-identical; a new path created exclusively | ✓ VERIFIED | literal, `..`, and symlink spellings each rc 1 with the token and **no file left**; `refs/inspect-probe.ivst` rc 0, 9,669 B, then removed; existing file rc 1, sha256 unchanged; a dangling symlink rc 1 with the token |
| 18 | The rights row lands in one commit with its `rights.ids` line, its `docs/RIGHTS.md` row, `ivst` in both name lists, and the `files-unlisted-binary-ivst` control | ✓ VERIFIED | `git show --stat 5ed5610` carries all of `data/rights/inspection-output.json`, `data/expected/rights.ids`, `docs/RIGHTS.md`, `tools/check/binary-ext.txt`, `.gitignore`, the three control files and `data/expected/controls/rights.ids`; `git ls-files \| grep -E '\.ivst$'` → empty |
| 19 | The tool is build-only and nothing it touches moves | ✓ VERIFIED | `data/expected/pkg-install-files.ids` 11 data lines; `cmake --install build/release --prefix <tmp>` → **11 files, zero named `*inspect*`**; `ls tools/check/steps/*.sh \| wc -l` → 38; `git diff --name-only 48470fa..HEAD` touches no profile field, parameter digest, snapshot literal, golden, claim or handoff |
| 20 | `dis` round-trips: the emitted source reassembles with `ivs_asm` and the `ivs_pack --dump-words` dumps are byte-identical, the reading named rather than implicit | ✓ VERIFIED | live: 22 words from the input, 22 from the reassembled source, `cmp` clean; and the committed twin's 22 words are byte-identical to both |
| 21 | The round trip is non-vacuous by construction: more words than instruction lines, an immediate present, more than one distinct mnemonic | ✓ VERIFIED | 22 words vs 16 instruction lines (`^ +[A-Z]` minus `ORG`) = margin 6 against a required 2; `MVII #$0001, R0` and `MVII #$0340, R6` present; 7 distinct mnemonics `B DIS HLT J MVII MVO TSTR` against a required 3 |
| 22 | The mnemonic is the listing's `ROW` column and the operand its `ASM` column by the three fixed cuts; the eight-key header rule; an unknown row refused with `inspect-dis-row` before any output | ✓ VERIFIED | `tools/inspect/inspect_dis.c` (466 lines) implements the pinned list; live unknown row → `ivs-inspect: inspect-dis-row: the row NOSUCHROW at 1000 is not one of the mnemonics ivs_asm accepts`, exactly one stderr line, no output file; the emitted header is the eight accepted keys and the listing's bare `#`/prose lines are dropped |
| 23 | The three oracles agree: the input listing's words, the committed twin's words, and the disassembler's output's words | ✓ VERIFIED | `cmp` on all three pairwise → identical (22/22/22) |
| 24 | A constant-mnemonic stand-in fails the byte equality with a diff | ✓ VERIFIED | reproduced by hand: the mutation changes the source, `ivs_asm` accepts it (rc 0) and produces 16 words whose dump differs from the input's 22 — so the **word comparison**, not a presence assertion, is the clause it defeats (matching Amendment 3) |
| 25 | Two `dis` runs are byte-identical, asserted once and not a second time inside the round trip | ✓ VERIFIED | `asm.dis.deterministic` is the only such case; two live runs `cmp` clean |
| 26 | The limb stays inside the priced line and adds no step, fault, golden, claim or installed file; six `asm` ids; one registry row | ✓ VERIFIED | `steps=38/38`, `faults=238/238`, `goldens=29`, `claims=29`, `prefix-files=11` unmoved; `data/expected/tests/asm.ids:33-38` six ids; `asm mutant asm.judge.dis-constant-mnemonic TOOL-02` at `data/expected/important-gates.ids:682`; `gates` 121 → 122, `lines` 371 → 372 |

**Score: 26/26 truths verified** (0 present-but-behaviour-unverified, 0 overrides, 0 failed).
No truth was accepted on symbol presence alone: every behavioural truth above was exercised by a committed id
that this verification re-ran, and five of the six stand-ins were additionally reproduced by hand.

### Deferred Items

None. Nothing this phase failed to deliver is scheduled in a later phase; the two observations below are
records questions, not work items.

### Required Artifacts

| Artifact | Expected | Status | Details |
|---|---|---|---|
| `docs/inspection.md` | the frozen oracle, append-only | ✓ VERIFIED | 253 lines at `0627be9`, 21,194 B now, freeze is a byte prefix; six dated amendments, no frozen line rewritten |
| `tools/inspect/inspect.c` | command grammar, output policy, `break`, `dis` dispatch, the host | ✓ VERIFIED | 1,293 lines, `SPDX-License-Identifier: MIT` on line 1; header names all four grammars (IN-02) |
| `tools/inspect/inspect_trace.c` | the two passes and the reconciliation | ✓ VERIFIED | 854 lines; equalities 2, 3, 5 computed, `inspect-reconcile` on failure |
| `tools/inspect/inspect_dis.c` | the disassembler | ✓ VERIFIED | 466 lines; no decoder table — the mnemonic is the listing's leaf row id |
| `tools/inspect/CMakeLists.txt` | the target, the source list, no install rule | ✓ VERIFIED | 40 lines; `IVS_INSPECT_SOURCES` → `ivs-inspect-sources.txt`; zero `install(` |
| `data/rights/inspection-output.json` | `owner-local-only`, `source: null`, nothing taken, three tokens | ✓ VERIFIED | all five cells read exactly; `taken: "nothing"`, `tokens: ["trace-file","disassembly-dump","inspection-dump"]` |
| `data/expected/rights.ids`, `docs/RIGHTS.md` | the row's line and its operator-page row | ✓ VERIFIED | `rights.ids:12` `inspection-output`; `docs/RIGHTS.md:46` the item with its disposition |
| `tools/check/binary-ext.txt`, `.gitignore` | `ivst` refused by name in both | ✓ VERIFIED | `binary-ext.txt:49` `ivst` (body, token **and** scope: the four readers and the control are named at `:6-17`); `.gitignore:25` `*.ivst` |
| `tools/check/controls/files-unlisted-binary-ivst/` | the phase's one control | ✓ VERIFIED | `CHECK files`, `EXPECT FAIL files-unlisted-binary: docs/run.ivst`, `MAKE write docs/run.ivst x`; `controls=405/405` |
| `data/expected/pkg-selftests.ids` | one `inspect-absent` case | ✓ VERIFIED | `:38`; the case plants `bin/ivs-inspect` in a copy of the prefix and requires `-- FAIL pkg-inventory-extra: bin/ivs-inspect` from the same `check_inventory` (`pkg-install.sh:683-692`) |
| `data/expected/release-assets-selftests.ids` | one `inspect-unlisted-member` case | ✓ VERIFIED | `:27`; the case requires `release-assets-extra-member: bin/ivs-inspect` and a negative limb requiring neither `release-assets-binary-member` nor `-missing-member` to fire (`release-assets.sh:202-216`) |
| `data/expected/tests/runner-process.ids` | thirteen inspection ids | ✓ VERIFIED | `:280-292` — nine `inspect.*` plus four `judge.*`, each with a case in the suite |
| `data/expected/tests/asm.ids` | six `asm.dis.*` / stand-in ids | ✓ VERIFIED | `:33-38` |
| `data/expected/important-gates.ids` | six rows under `TOOL-02`/`TOOL-09` | ✓ VERIFIED | `:666-669` four `runner-process mutant`, `:673` `files control files-unlisted-binary-ivst TOOL-09`, `:682` `asm mutant … TOOL-02` |
| `docs/packaging.md` | the "not installed" line naming the tool | ✓ VERIFIED | `:38` and `:122` name `ivs-inspect` in the build-only enumeration |

### Key Link Verification

| From | To | Via | Status |
|---|---|---|---|
| `docs/inspection.md` | `core/src/machine/machine.h` | names `ivs_machine_step_unit_trace_internal` and `IVS_CP1610_MAX_ACCESSES` as the internal seam | ✓ WIRED — page `## What it is built on`; the tool calls it at `inspect_trace.c:593,793` with that bound |
| `docs/inspection.md` | `core/include/ivs/ivs_machine.h` | the nine-domain public digest | ✓ WIRED — `inspect_trace.c:698` over the nine `ixt_domain` names |
| `docs/inspection.md` | `data/expected/listings.ids` | the corpus is the registered listings | ✓ WIRED — seven registered; `dis` decodes six and refuses `ivs-diag-stic-2` fail-closed (Amendment 4, reproduced) |
| `docs/inspection.md` | `runner/ivs_runner.h` | `ivs_scenario_parse` / `ivs_runner_execute` / `write_report`; `ivs_runner_report_write` never the tool's call | ✓ WIRED — two calls present, zero hits for the writer |
| `tools/inspect/inspect_trace.c` | `core/src/machine/machine_bound.c` | the run ladder: fault break, halt break, `ivs_input_apply_due` before each unit, a refused step ending the loop | ✓ WIRED — `inspect_trace.c:577-600`; the end-tick stand-in proves a loop that stepped to the target would be refused |
| `tools/inspect/inspect_trace.c` | `core/include/ivs/ivs_machine.h` | the reconciliation is a byte equality, not a proxy | ✓ WIRED — `:698-710`; a proxy would make the poked-report stand-in unfalsifiable, and it is falsifiable (reproduced) |
| `tests/runner-process-tap.sh` | `tools/inspect/inspect.c` | the ninth argument, a missing one `not ok` never a skip | ✓ WIRED — `:137,156-159,3128-3129` |
| `data/expected/important-gates.ids` | `.planning/REQUIREMENTS.md` | the derived accepted set: a `Pending` requirement refused, a `Complete` one with no row refused | ✓ WIRED — `important-gates.sh:157-164` derives the set from the two requirement records; the both-directions rule is what forced the §2.7 split recorded below |
| `tools/check/binary-ext.txt` | `.gitignore` | one extension list feeds three refusal surfaces; `.gitignore` is the second net | ✓ WIRED — both carry `ivst` in the same commit `5ed5610` |
| `tools/inspect/CMakeLists.txt` | `tools/check/steps/c-build.sh` | `IVS_INSPECT_SOURCES` → `ivs-inspect-sources.txt`, so an uncompiled tracked source is a finding | ✓ WIRED — writer at `CMakeLists.txt:35-40`, consumer at `c-build.sh:51` |
| `tests/asm-tap.sh` | `tools/asm/asm.c` | the round trip reuses `twin_words`/`dumps_agree`, the same comparator `asm.twin-boot-stub` uses | ✓ WIRED — no second comparator in the file |
| `data/expected/tests/asm.ids` | `data/expected/important-gates.ids` | the `<suite>.(judge\|mutant).<slug>` shape so the row resolves | ✓ WIRED — `asm.judge.dis-constant-mnemonic` present in both |

### Data-Flow Trace (Level 4)

| Artifact | Data Variable | Source | Produces Real Data | Status |
|---|---|---|---|---|
| `trace` output | access records | `ivs_machine_step_unit_trace_internal` filling a caller array, one record per access | yes — 112 records, 112 distinct ticks, `tick = unit_start + cycle × ticks_per_cpu_cycle` from `ivs_machine_info` (no literal) | ✓ FLOWING |
| `trace` output | the report it reconciles against | the bytes the library handed `write_report` | yes — nine digests byte-identical to `ivs-run`'s | ✓ FLOWING |
| `break` report | `break.hits` / `break.other.hits` | the trace file's own `tick=` records, `kind ∈ {fetch, iab}` | yes — `(1, 0)`, and `break.records` 112 = `grep -c '^tick='` 112 (two independent computations) | ✓ FLOWING |
| `step` report | `step.units` / `step.ticks` / `step.reg.*` / `step.mem.*` | the traced instance through `ivs_machine_cpu_state` and `ivs_machine_peek` | yes — 70/2084/`R0 03C5`/`0200 7FFF`, each equal to the scenario's own asserts | ✓ FLOWING |
| `dis` output | instruction lines | the listing's `ROW` and `ASM` columns | yes — 16 instruction lines, 22 words after reassembly, identical to the input's dump | ✓ FLOWING |

No value ends in a static return, a hardcoded literal or a mock. The only typed literals are **pinned
expectations** (`(1, 0)`, `16000`), and each is the scenario's or the listing's own number, not a substitute
for a computation.

### Stand-ins — each proved to fail, and proved to fail for its own reason (`GUARDRAILS-v0.4` §6.3)

All six are **committed cases**, re-run by the two suites in this verification (`asm` and `runner-process`
both passed). Five were additionally reproduced by hand in `/tmp`; that is stated per row.

| Stand-in | Guard it must defeat | Reproduced | What actually fails |
|---|---|---|---|
| `judge.trace-empty` | `records > 0` (`ix_records`) | by hand | a zero-record trace reads `records=0` → `trace-records`. **Committed case re-run.** |
| `judge.trace-repeat-tick` | the fifth equality's *not all equal* clause (`ix_tick_domain`) | by hand | every tick set to the first value (`tick=4`): the monotonicity clause **passes** (a constant column is non-decreasing) and only the distinct-value clause refuses it, `distinct=1` → `trace-ticks`. So the guard, not the comparison, does the work. **Committed case re-run.** |
| `judge.trace-pokes-register` | the nine-digest equality (`ix_digests_equal`) then the tool's own `--report` reconciliation | by hand | `digest.cpu` replaced by `digest.mem`'s value → the digest equality refuses it (`digest-differs`) and the tool refuses it: `ivs-inspect: inspect-reconcile: the stored report's digest.cpu is not the one this run printed …`, rc 1, 0 B stdout, no output file. **Committed case re-run.** |
| `judge.break-always-hits` | the pair's second number (`ix_break_pair`) | by hand | `break.other.hits 1` → the pair reads `1 1` and the pinned `1 0` clause refuses it (`break-zero`). **Committed case re-run.** |
| `trace-target-instead-of-stop` (second limb **inside** `inspect.trace-state`, no new id) | the end-tick equality (`ix_end_tick`) | by hand | both the step record's `end` and the end record's `tick` set to `16000`, so clause 1 (`stepend == endrec`) **passes** and only clause 2 — the comparison with the report's own `step.1.now_ticks 2084` — refuses it. Internally consistent and still refused: exactly what makes it a liveness guard rather than a self-check. **Committed case re-run.** |
| `asm.judge.dis-constant-mnemonic` | the word comparison (`dumps_agree`) | by hand | every instruction line replaced by `DECLE $0000`: assembles (rc 0) to 16 words, and the dump **differs** from the input's 22 — so it fails the *word comparison*, not the presence assertions (Amendment 3's correction, verified). The case reports `judge-vacuous` if the comparison accepts it and `judge-wrong-reason` unless the reason begins `word dumps differ`. **Committed case re-run.** |

No demonstration is a one-off throwaway: every one of the six is a committed case in a suite the gate runs.

### Behavioural Spot-Checks

| Behaviour | Command | Result | Status |
|---|---|---|---|
| The suites pass on the final tree | `ctest --preset dev -R 'runner-process\|asm' --output-on-failure` | 2/2, `asm` 7.39 s, `runner-process` 28.93 s | ✓ PASS |
| Trace end-to-end | `ivs-inspect trace data/scenarios/diag-boot-pass.ivss --content-dir <tmp> --out <tmp>/t.ivst` | rc 0, 112 access records, `end tick=2084 records=112 units=70 steps=1` | ✓ PASS |
| Digests equal `ivs-run`'s | `ivs-run … \| sed -n 's/^digest\.//p'` vs the tool's | 9 vs 9 lines, `cmp` clean | ✓ PASS |
| Break pair | `ivs-inspect break --pc 1000 --other 1002 --trace … --out …` | `break.hits 1`, `break.other.hits 0`, `break.records 112`, `break.end_tick 2084` | ✓ PASS |
| `break` both directions of `inspect-break-empty` | zero records declared `records=5`; one record declared `records=0` | rc 1 with the token, no file / rc 0 with the pair `1 0` and `break.records 0` | ✓ PASS |
| `step` reconciles | `ivs-inspect step … --units 70 --peek 0200 3` | `step.units 70`, `step.ticks 2084`, `step.halted yes`, registers and `0200` equal to the scenario's asserts | ✓ PASS |
| `dis` round trip | `dis` → `ivs_asm` → `ivs_pack --dump-words` on both | 22 == 22, `cmp` clean; twin 22 also equal | ✓ PASS |
| `dis` refusal on a registered listing | `dis fixtures/FX-diag-stic-2/diag-stic-2.listing.txt` | rc 1, `inspect-dis-row: the row DATA at 512E …`, no file (six of seven registered decode) | ✓ PASS |
| `--help` | `ivs-inspect --help` | four grammars, three exit statuses, rc 0, stderr empty | ✓ PASS |
| Install prefix | `cmake --install build/release --prefix <tmp>` | 11 files, none named `*inspect*` | ✓ PASS |
| `--pc == --other` (Amendment 6's factual claim) | `trace … --break 1000 --other 1000` | rc 0, pair `1 1` — so the pair is an operator precondition, not an enforced one, exactly as Amendment 6 states | ✓ PASS (the amendment is accurate) |

### Probe Execution

Not applicable: this phase declares no `scripts/*/tests/probe-*.sh`, and neither the plans nor the summaries
name one. The probes are the committed suite ids above, which the gate re-runs.

### Requirements Coverage

| Requirement | Source Plan | Description | Status | Evidence |
|---|---|---|---|---|
| **TOOL-02** | `18-01`, `18-02`, `18-03` | A build-only, scripted, headless inspection surface over the seams the core already exposes: disassembly of the project's own listings, breakpoints with an asserted hit and non-hit count, and a cycle-stamped access trace whose records are non-empty, cycle-monotonic, end at the report's own step tick and leave every digest byte-identical to the same run untraced; never installed, never in the eleven-path prefix, no gate step added | ✓ SATISFIED | truths 10–26: the target with no install rule; `prefix-files=11` with the installed prefix copied and inspected; `steps=38/38`; the trace's 112 records, 112 distinct ticks, `end tick 2084` = `step.1.now_ticks 2084`, nine digests identical to `ivs-run`; the `(1, 0)` break pair; the `dis` round trip |
| **TOOL-09** | `18-01`, `18-02` | Trace and disassembly output over operator content is an operator-local, rights-bearing artifact: one question-shaped `data/rights/` row with a disposition and a "what was taken" cell, an in-tree output path refused by name except under the ignored `refs/`, `ivst` refused in the binary-extension list and `.gitignore`, and no trace or disassembly ever committed as a golden, fixture, CI artifact or claim artifact | ✓ SATISFIED | truths 17–19: the row in three places in one commit; the three in-tree spellings refused and `refs/` accepted; `inspect-output-exists` byte-identical; `ivst` in both name lists with the `files-unlisted-binary-ivst` control; `git ls-files \| grep -E '\.ivst$'` empty; `rights=11`, `rights-cleared=1` |

**Orphan check.** `grep -E '\| Phase 18 \|' .planning/REQUIREMENTS.md` yields exactly `TOOL-02` and `TOOL-09`,
both `Complete`, and the plans declare exactly those two (`18-01`: both; `18-02`: both; `18-03`: `TOOL-02`).
**No orphaned requirement.** No requirement mapped to this phase is unclaimed by a plan.

### Anti-Patterns Found

| File | Line | Pattern | Severity | Impact |
|---|---|---|---|---|
| — | — | `TBD`/`FIXME`/`XXX` in any phase-modified file | none | The only `XXX` matches are `mktemp -d …XXXXXX` templates (`tests/asm-tap.sh:64`, `tests/runner-process-tap.sh:178,2097`, `c-build.sh:23`, `important-gates.sh:150`, `pkg-install.sh:92`, `release-assets.sh:58`) — the template idiom, not a debt marker |
| `tools/inspect/*.c` | — | `TODO`/`HACK`/`PLACEHOLDER`/"not yet implemented" | none | zero hits |
| `tools/inspect/*.c` | — | empty returns (`return null`, `return {}`, `=> {}`) | none | zero hits; every path either writes a report or refuses by name |
| — | — | hardcoded empty data flowing to output | none | the tool renders no UI; the only typed literals are the pinned expectations named under Level 4 |
| `docs/inspection.md` | `## Liveness guards`, guard 2 | a page sentence whose *mechanism* wording is not literally what shipped | ℹ️ Info (O1) | the guard, its committed stand-in and its refusal are all present and demonstrated; only the construction wording differs, and it is disclosed in `18-02-PLAN.md` and in the case's comment, but not in a dated amendment nor in a SUMMARY deviation row |
| `tools/check/integrity.cmake` | 82 | scope widened into an existing scan | ℹ️ Info | the widening is the trapdoor the plan intended (`tools/inspect/` would otherwise be silently outside the SPDX/ASCII scan); proved by the eight existing controls, not by a new one |
| `tools/inspect/CMakeLists.txt` | — | a new build target | ℹ️ Info | build-only, no install rule, absent from a copied prefix by measurement; `steps` unmoved at 38 |

### Review findings — each disposition against the tree (`18-REVIEW.md`, 1 critical / 6 warning / 5 info)

| Finding | Disposition claimed | Does it match the tree? |
|---|---|---|
| **CR-01** `inspect-break-empty` fired on the declared `records=` field | fixed `5f68b30` | **YES.** `inspect.c:892,966,995` count `records_seen` in the parse loop and refuse on it; `end_records` stays the copied number. Reproduced both directions: zero records declared 5 → rc 1 + token + no file; one record declared 0 → rc 0 with `break.records 0` and the pair `1 0`. The suite's third limb asserts both. |
| **WR-01** the page stated two operand derivations | fixed by amendment `e936f8d` | **YES.** `## Amendments` Amendment 1 names the `ASM`-column three cuts as authoritative, quotes the superseded sentence, and no frozen line moved (prefix check True). `inspect_dis.c` implements the pinned list. |
| **WR-02** the page named a stand-in never committed and mis-described another | fixed `8b65f88`, `e936f8d` | **YES.** The end-tick stand-in is a `self_control_run` limb **inside** `inspect.trace-state` (`runner-process-tap.sh:3275-3295`), judged by `ix_end_tick` — the same function the real case uses — and it is internally consistent so only the report comparison refuses it (reproduced). Amendment 3 corrects the `DECLE` clause (verified: the stand-in defeats the word comparison) and adds the repeat-tick guard to a list that now reads five. No new id, no count moved. |
| **WR-03** `inspect.surface` excluded `inspect-dis-row` on a closed rationale | fixed `2e07362` | **YES.** No `grep -v` exclusion remains; both sides assert `-eq 5` and both `comm` directions are empty. I re-ran the two extractions: both yield the same five tokens. |
| **WR-04** the `refs/` positive limb relied on an explicit `rm -f` | fixed `f6bb8e7` | **YES.** `runner-process-tap.sh:184` — `trap 'rm -rf "$tmp"; rm -f "$root/build/inspect-probe.ivst" "$root/refs/inspect-probe.ivst"' EXIT` — covers both probe paths; `HUP INT TERM` also `exit 1` through it. |
| **WR-05** `break.records`/`end_tick`/`steps` were compared to their own source | fixed `cba2ff6` | **YES.** The case now compares `break.records` to `grep -c '^tick='`, `break.end_tick` to the last step record's `end`, and `break.steps` to `grep -c '^step='` — three independent computations. Measured: 112/112, 2084/2084, 1/1. |
| **WR-06** the limb refuses one of the seven registered listings | fixed by amendment `e936f8d` | **YES as recorded** — Amendment 4 states which listings decode and why the `DATA` row is refused by name and address; I reproduced the exact line on `ivs-diag-stic-2`. **The review's Fix text also asked for an `asm.dis` case on that registered listing, and that half was not taken** — see O2. The recorded disposition (amendment only) is accurate about what shipped. |
| **IN-01** `--pc` may equal `--other` | fixed by amendment `e936f8d` | **YES.** Amendment 6 states the precondition and that the surface does not enforce it. Verified factually: `--break 1000 --other 1000` → rc 0, pair `1 1`. |
| **IN-02** the tool's header omitted `dis` | fixed `7662a15` | **YES.** `inspect.c:4-5` reads "the `trace`, `break`, `dis` and `step` grammars". |
| **IN-03** the page said "five equalities" where two hold by construction | fixed by amendment `e936f8d` | **YES.** Amendment 5 names equalities 2/3/5 as the tool's refusals and 1/4 as asserted on the artifacts by `inspect.trace-digests` and the end-tick comparison; the code matches (`inspect_trace.c` computes 2, 3, 5; equality 4 is one variable written twice). |
| **IN-04** `integrity.cmake`'s inserted lines broke the block's shape | fixed `7662a15` | **YES.** `:76-77` are two-space indented and the `tools/inspect/` sentence follows the complete `host/` sentence. |
| **IN-05** the page named the artifact two commits before the rights row | recorded, no code change | **YES.** The disposition row records the ordering and the reasoning; §2.13's letter is met at the commit that first writes bytes (`5ed5610`, where the row and the only writer landed together). No byte could have preceded the row. |

**11 of 12 dispositions match the tree exactly; the twelfth (WR-06) matches as recorded but the review's own
Fix text asked for one thing more than the disposition took (O2). No finding marked fixed lacks a demonstrable
mechanism.**

### Observations (non-blocking, recorded so nothing is silently absorbed)

**O1 — `docs/inspection.md` `## Liveness guards`, guard 2: mechanism wording not amended.**
The frozen sentence reads "The stand-in is a tracing pass that pokes one register; it diverges on exactly one
domain, and the refusal names that domain." The committed stand-in is a **report** with `digest.cpu` replaced
by `digest.mem`'s value. The second half of the sentence is exactly true (one domain, named in the refusal);
the first half describes a build that was never committed. `18-02-PLAN.md` states the application openly
("applied to the tool's own reconciliation so the mechanism under test is the one the page names") and the
case's comment repeats it, but no dated amendment and no SUMMARY deviation row records it — which is the
treatment Amendment 3 gave guard 1 for the same class of mismatch. **Recommendation:** an appended
Amendment 7 naming the section and the superseded clause. Not blocking: the guard, its committed stand-in and
its refusal all exist and were demonstrated.

**O2 — WR-06's second half was not taken.** The review's Fix text asked for "one `asm.dis` case that asserts
the fail-closed refusal on the registered listing — exit 1, exactly one stderr line …, no output file". The
disposition took the amendment only, so no committed id covers `ivs-diag-stic-2`; the committed id's unknown-row
limb (`asm.dis.roundtrip-boot-stub`) uses a **synthetic** `NOSUCHROW` listing built from the boot stub. The
refusal itself works on the registered listing (reproduced: `inspect-dis-row: the row DATA at 512E …`, rc 1,
no file) and the amendment records it, so this is a missing regression net rather than a missing behaviour.
**Recommendation:** one `asm.dis` id on the registered listing, and a registry row if it becomes a mutant.

**O3 — `GUARDRAILS-v0.4` §2.4's phase-boundary checkpoint line is not recorded for this phase.** §2.4 requires
exactly one of `discharged-within-allowance` / `reprice` / `lever-required` at every phase boundary. No such
word appears in `18-0*-SUMMARY.md` or in `.planning/STATE.md` for phase 18 (phases 13 and 17 recorded theirs
there); `18-02-SUMMARY.md` says only "carried to the milestone close as a repricing candidate". The arithmetic
the phase does carry is checkable: priced 2.96, recomputed 3.59, +21 % — inside §2.4's 25 % repricing
threshold, so no reprice is required before phase 19's plan. **This is a records obligation for the close, not
an implementation gap**, and it is named here so the close does not inherit a silent omission.

**O4 — §2.7's one-commit rule was satisfied across two commits, mechanically forced.** The four stand-ins and
their guards landed in `e876c93`; their four `important-gates` mutant rows landed in `d8eb593` with both
requirement flips. The reason is real and I verified it: `important-gates.sh:157-164` **derives** its accepted
set from the two requirement records and refuses a row naming a `Pending` requirement *and* a `Complete`
requirement with no row, so the rows and the flips cannot move separately. The split is recorded in both
commits and in `18-02-SUMMARY.md` (items 6 and 12), so it is a recorded deviation, not a silent one.

**O5 — the tool's report differs from `ivs-run`'s on `output.audio.*` only.** `output.audio.bytes 520/130` for
`ivs-run` vs `0/0` for the tool: the build-only instrument supplies no audio sink, so the host's output capture
differs while the nine `digest.*` lines (which include the AUDIO domain) are byte-identical. That is exactly the
separation the page asserts for equality 1, which is scoped to "the nine `digest.*` lines" and not to the whole
report. Recorded so a later reader does not read the difference as a reconciliation weakness.

### Frozen page — the criterion-3 reading (faithful, and a narrowing that is recorded)

Criterion 3 says "disassemble, re-assemble with `ivs_asm`, **bytes equal to the input**". The phase discharges
this as the **`ivs_pack --dump-words` word dump** of both listings, never a comparison of listing text.

**This is a narrowing, and it is recorded in two places**: `docs/inspection.md` `## Disassembly` — frozen at
`0627be9`, before any tool code — pins it ("the comparison is the `ivs_pack --dump-words` word dump of both
listings, never a comparison of listing text"), and `18-03-PLAN.md`'s `must_haves` truth 1 names the reading in
as many words ("the criterion's 'bytes equal to the input' is discharged as **the `ivs_pack --dump-words` word
dump** … so a later reader does not read the criterion as a listing-byte equality that this limb never
asserts").

**Is the narrowing faithful?** Yes, and it is the strongest equality available: the tool emits a *source* and
never a listing, and `ivs_asm`'s listing output prints the source path it was given, so a literal listing-byte
equality against the input listing is not reachable by any implementation. The word dump is the observation
`asm.twin-boot-stub` already takes, and the committed listings' own bytes are pinned separately by the
`asm.regen-*` ids, which require the committed listing to equal what `ivs_asm` writes for the committed twin
byte for byte. So: word equality here, listing-byte equality there, no gap between them.

---

## What was automated, what was thrown away, what was handed off, what a tool could not judge
(`GUARDRAILS-v0.4` §6.7)

**Automated, and re-run by this verification (nothing below is a quotation):**
`sh tools/gate.sh` and `dash tools/gate.sh` on the final tree, byte-identical last lines, zero `-- FAIL`; the
fifteen-token comparison; `sh tools/hygiene.sh --tree`; the `git ls-files` binary-extension check; `ctest
--preset dev -R 'runner-process|asm'` (2/2); eleven behavioural spot-checks driving the built binary; six
stand-in demonstrations; the ancestry proof; the byte-prefix freeze proof; the five-token both-directions
comparison; the install-prefix copy (11 files, no `*inspect*`); `git show --stat` on the rights-row commit.

**Thrown away:** nothing of this phase's evidence. The only discarded artifact is this verification's own
`/tmp` working tree (`mktemp -d`), removed; the two probe paths under `build/` and `refs/` were removed after
each use, and `refs/` was left as it was found (it is pre-existing, gitignored, and holds rights-bearing owner
material — it was **not** created here).

**Handed off:** one item — the container gate (`sh tools/gate-linux.sh`), which this host cannot run. It is
outstanding evidence, not a pass, with the visible `not run` line and the measured reason above. Nothing else
was handed off, and no `data/expected/handoffs.ids` row was added: this project forbids a handoff row for
something a tool can check, and every other check here ran.

**What no tool could judge:** whether the frozen page states the *right* contract. That judgement was made in
`18-DISCUSS`/`18-RESEARCH` and is not re-asked here (§6.5's "not re-asked" rule). Within that, three
records-level questions are surfaced rather than decided by a tool: O1 (should guard 2's wording be amended?),
O2 (should the registered-listing refusal get its own id?), O3 (the §2.4 boundary line is owed to the close).
None of them is a behavioural unknown.

---

## Gaps Summary

**No gaps.** Every one of the five ROADMAP success criteria is observably true in the codebase, with the
command and its output recorded above; every one of the 26 must-have truths is verified; both requirement IDs
are satisfied and no requirement mapped to this phase is orphaned; eleven of twelve review dispositions match
the tree exactly and the twelfth matches as recorded; and no truth was accepted on symbol presence alone.

The five observations are records/precision items, not goal failures, and each is stated with the exact quote,
the exact file and line, and a recommendation. The single outstanding item is the container gate, which this
host cannot run — it is recorded as outstanding evidence with a measured reason and is **not** claimed as a
pass.

---

_Verified: 2026-11-27T03:14:54Z_
_Verifier: Claude (gsd-verifier)_

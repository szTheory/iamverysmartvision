---
phase: 10-reachable-runs-and-presentation-policy
verified: 2026-11-25T10:11:16Z
verifier: Claude (gsd-verifier)
status: passed
score: 5/5
re_verification: false
phase_base: d3952ea
verified_tree: 5731f4a
tier_ceiling:
  documented_route: installed-host-behavior
  handoff: none
covered_files:
  - .planning/REQUIREMENTS.md
  - .planning/phases/10-reachable-runs-and-presentation-policy/10-01-PLAN.md
  - .planning/phases/10-reachable-runs-and-presentation-policy/10-01-SUMMARY.md
  - .planning/phases/10-reachable-runs-and-presentation-policy/10-02-PLAN.md
  - .planning/phases/10-reachable-runs-and-presentation-policy/10-02-SUMMARY.md
  - .planning/phases/10-reachable-runs-and-presentation-policy/10-03-PLAN.md
  - .planning/phases/10-reachable-runs-and-presentation-policy/10-03-SUMMARY.md
  - .planning/phases/10-reachable-runs-and-presentation-policy/10-04-PLAN.md
  - .planning/phases/10-reachable-runs-and-presentation-policy/10-04-SUMMARY.md
  - .planning/phases/10-reachable-runs-and-presentation-policy/10-05-PLAN.md
  - .planning/phases/10-reachable-runs-and-presentation-policy/10-05-SUMMARY.md
  - .planning/phases/10-reachable-runs-and-presentation-policy/10-06-PLAN.md
  - .planning/phases/10-reachable-runs-and-presentation-policy/10-06-SUMMARY.md
  - README.md
  - data/expected/controls/claims.ids
  - data/expected/handoffs.ids
  - data/expected/important-gates-excluded.ids
  - data/expected/important-gates.ids
  - docs/handoffs.md
  - docs/packaging.md
  - docs/presentation.md
  - docs/running.md
  - tools/check/kinds/claim.cmake
  - tools/check/steps/claims.sh
  - tools/check/steps/text.sh
  - tools/release-rehearse.sh
covered_digest: "v4:sha256:6e79161ddc6e4c30db7c16ec2afba1396236b28ecdd3f9f07ff5f56a128d39ee"
---

# Phase 10 Verification: Reachable Runs and Presentation Policy

**Goal (ROADMAP):** A person obtains and runs the emulator from the released source archive using documented commands, the presentation and palette policy is frozen and enforced, and the look/listen handoff is recorded rather than assumed.

**Verified tree:** `5731f4a` (phase branch), phase base `d3952ea` (the research commit; amendment A-04's baseline).
**Method:** every criterion re-measured on this tree in this session. No SUMMARY value was accepted as evidence; each row below names the command and quotes its output.

## The gate, re-run three ways on the final tree

```
sh    tools/gate.sh   -> exit 0, 41 step-time lines, 0 lines beginning '-- FAIL'
dash  tools/gate.sh   -> exit 0, 0 lines beginning '-- FAIL'
sh    tools/gate-linux.sh -> gate-linux: exit 0 (x86_64, gcc 13.3.0, cmake 3.28.3), log copied to /tmp before anything else ran
```

The `sh` and `dash` summary lines are **byte-identical** (1462 characters each; `[ "$s" = "$d" ]` true):

```
gate: PASS steps=38/38 controls=389/389 api-calls=19 ... claims=28 provisional=27 ... faults=235/235 ...
cases=13 goldens=29 doc-files=62 ... prefix-files=11 ... files=2601 tracked=2601 hygiene-files=2601 ...
gates=109 lines=333 ... tests-expected=4958 tests-executed=4954 tests-skipped=4 tests-missing=0
docs=63 claim-refs=26 d10-items=47
```

The container tail carries the same frozen tokens (`steps=38/38 controls=389/389 claims=28 provisional=27 claim-refs=26 gates=109 lines=333 prefix-files=11 docs=63 doc-files=62 goldens=29 faults=235/235 cases=13 tests-* 4958/4954/4/0 d10-items=47`) and 0 `-- FAIL` lines in 527 log lines. `core-imports` is 7 on Darwin and 5 on Linux — a pre-existing platform difference, not one of the frozen tokens.

## Success Criteria

| # | Criterion (frozen, as amended) | Verdict | Evidence (command and output) |
|---|---|---|---|
| 1 | `docs/running.md` names the three commands and the exact expected stdout tail, and `tools/release-rehearse.sh` builds from the **source archive** in a clean directory and runs `demo-lamp-tone-pass` to its golden — recorded at release time, not as a gate step | **met** | (a) The three commands and the nine-line tail are in `docs/running.md`. (b) I ran the route from the archive: `sh tools/release.sh --no-gate --out $w/a` → `ivs-v0.1.0-source.tar.gz`, 5804486 bytes, 4263 members, prefix read from the first member = `ivs-v0.1.0`; extracted into `$w/clean` with **no `.git`**; `cmake --preset release` + `cmake --build --preset release` ok; 7 files in `build/release/images`; `ivs-run --content-dir build/release/images data/scenarios/demo-lamp-tone-pass.ivss` exit 0, report **131 lines**, golden body **124 lines**, **124 of 124** golden lines found exactly once, `diff` of the document's nine-line block against the measured tail **empty** (byte-identical). (c) The rehearsal itself: `sh tools/release-rehearse.sh --no-gate --source-route` → exit 0, `rehearse: source-route: ivs-v0.1.0-source.tar.gz, golden match (report 131 lines, 124 of 124 compared), prefix ivs-v0.1.0`, last line `rehearse: ok`. (d) Not a gate step: `grep -n 'rehearse\|release-rehearse' tools/gate.sh` → nothing; `grep -rn 'release-rehearse' tools/check/` → nothing; no `step-time` line names it; the transcript's own header says it is "**not a gate step**, **not a gate result**, **not evidence for any claim**" |
| 2 | `docs/presentation.md` holds the six-clause policy, and one deny-list check refuses a colour symbol defined under `core/src`, shown able to fail by a committed scratch stand-in; the check adds no gate step | **met on recorded terms (A-02)** — the mechanism pre-dates the policy, so the oracle order is inverted and the inversion is on record | (a) `grep -c -E '^[1-6]\. ' docs/presentation.md` → **6**, at lines 12, 18, 31, 38, 43, 50, covering indices / one preview palette "not STIC colour data" / no colour claim / host chooses palette-scaling-aspect / index frame and the picture-and-sound words / a real colour table as a profile field with a source tier. (b) The refusal is the **pre-existing** one: `tools/check/core-symbols.sh:171` carries `if (tolower(n) ~ /(palette|rgb|ppm|framepreview)/)`; its stand-in `tools/check/controls/core-symbols-palette-defined/` holds 3 files and `EXPECT` names `FAIL core-symbols-palette-defined:`; run in isolation → `control ok   core-symbols-palette-defined`, `controls=4/4`; in the gate → `control ok   core-symbols-palette-defined` (line 220 of the `sh` log). (c) Ancestry: `git log --diff-filter=A -1 -- tools/check/controls/core-symbols-palette-defined/EXPECT` → `51515f1` (Phase 6, 2026-11-13); `-- docs/presentation.md` → `b5ad01d`. `git merge-base --is-ancestor 51515f1 b5ad01d` exits **0**; `--is-ancestor b5ad01d 51515f1` **fails**. The mechanism is an ancestor of the policy — the document does **not** precede the citation, and A-02 records exactly that inversion, in the document ("it does not create the check … amendment A-02") and in `10-02-SUMMARY.md`. (d) No new gate step: 38 step scripts, `git diff --stat d3952ea..HEAD -- data/expected/gate-steps/` empty |
| 3 | `data/expected/handoffs.ids` holds H7-1, H7-2, H8-3, D10, D11, every row `evidence: none` and each dispositioned `handed-off`/`not-performed`; a claim citing a handoff artifact, a `build/` path, a `.wav`, a `.ppm` or a bench `.tsv` is refused, and the `text` step's claim-word list contains the look/listen words — both shown able to fail | **met** | (a) `awk '!/^#/ && NF {print NF" | "$0}' data/expected/handoffs.ids` → five rows, **4 tokens each**: `H7-1 handed-off evidence: none`, `H7-2 handed-off evidence: none`, `H8-3 handed-off evidence: none`, `D10 not-performed evidence: none`, `D11 not-performed evidence: none`; dispositions `3 handed-off / 2 not-performed`; no tier token in any row. (b) The claims guard, probed on a scratch copy of the good toy tree with five claims citing `build/x.report`, `out/x.wav`, `out/x.ppm`, `data/bench/workloads.tsv` and `data/expected/handoffs.ids` (every file present, so `claim-dangling-path` cannot fire): output `-- FAIL claim-handoff-artifact: CL-0001` … `CL-0005`, exit 1 — all five classes refused, none by the dangling-path token. The same step on the **unmodified** good tree passes (`-- claims pass claims=1 provisional=0`, exit 0). Handoff-row grammar probed separately: a `done` row and a 3-token row → `-- FAIL claim-handoff-row: line 2`, `: line 3`, exit 1; a comments-only ledger → `-- FAIL claim-handoff-row: no rows`; a well-formed ledger → exit 0. (c) The text guard, probed on a scratch `docs/probe.md`: `The picture looks like a console.` / `The tone sounds right.` → `-- FAIL text-claim-untagged: docs/probe.md:1`, `:2`, exit 1; the same two lines with `[nc]` → `-- text pass docs=1 claim-refs=0`, exit 0. The list is one list: `tools/check/steps/text.sh:47` matches `supports|compatible|passes|accurate|verified|runs|plays|playable|looks|sounds`, and the header comment at `:6-9` names the two new words. (d) The three committed controls run green in the gate and in isolation: `control ok   claims-handoff-artifact`, `claims-handoff-row`, `text-untagged-look-listen`; `FORBID` on the first holds `claim-dangling-path`, which the probe confirmed does not fire |
| 4 | `README.md` and `docs/packaging.md` carry one identical "nothing downloadable is runnable" sentence (a grep proves a single matching line in each) and one paragraph pointing at `docs/running.md` | **met, as amended by A-01** (the phrase is *made* identical, not kept) | `grep -c -F 'nothing downloadable is runnable' README.md docs/packaging.md` → `README.md:1`, `docs/packaging.md:1`. The checked object is the substring, as A-01 requires. Base measurement: `git show d3952ea:README.md | grep -c -F …` → **0**, so `README.md:17` gained the phrase (minimally — the reason clause "because neither the runner nor `ivs_pack` nor `ivs_asm` is installed" is kept); `docs/packaging.md:121` is **byte-identical to the phase base**: `diff <(git show d3952ea:docs/packaging.md \| sed -n 121p) <(sed -n 121p docs/packaging.md)` empty. One pointer paragraph in each: `README.md:18` and `docs/packaging.md:125`, both naming `docs/running.md`, neither duplicating a command |
| 5 | The gate summary keeps `steps=38/38` and `claims=` at its Phase 9 value, and no claim cites a handoff row: the documented route reaches at most `installed-host-behavior` and the handoff carries no tier | **met** (`gates`/`lines` moved as A-04 permits, with A-04's `lines` figure corrected by measurement — see Deviations) | (a) At the phase base: 38 step scripts (`git ls-tree -r --name-only d3952ea tools/check/steps/ \| grep -c '\.sh$'`), `data/claims.json` 28 claims of which 27 provisional, and 26 claim references (`git grep -h -o -E '\[claim:[^]]*\]' d3952ea -- README.md CHANGELOG.md docs/ \| grep -v -x -F '[claim:CL-NNNN]' \| grep -c .`) → **28 / 27 / 26**. On the final tree the gate reads `steps=38/38 claims=28 provisional=27 claim-refs=26` and the same grep over `HEAD` reads 26. (b) `data/claims.json` and `data/expected/claims.ids` are **byte-identical** to the base (`git diff --stat d3952ea..HEAD` empty), so no claim was minted, raised or added; `grep -n 'handoffs' data/claims.json` → none; no claim cites `build/`, a `.wav`, `.ppm` or `.tsv`; no claim mentions the route or the rehearsal. (c) The ceiling is stated as a ceiling only, in `10-01-SUMMARY.md` ("stated here as a ceiling; nothing was written into `data/claims.json`"); the handoff carries no tier (criterion 3's rows). (d) `gates` 107 → **109** and `lines` 330 → **333** — both are *not* frozen by A-04, and both were predicted before the commit (`10-06-SUMMARY.md` predicted 109/333, A-04 predicted 109/335; the measurement wins and the mechanism is recorded) |

**Score: 5/5 criteria met** — criterion 2 on the recorded terms of amendment A-02, criterion 4 on the recorded terms of amendment A-01. No criterion required human judgement.

## Requirements

| Requirement | Source plan | Description (REQUIREMENTS.md) | Status | Evidence |
|---|---|---|---|---|
| PKG-05 | 10-01, 10-04, 10-06 | A person can build and run the emulator from the released source archive using documented commands only, and the release rehearsal proves that route end to end against the demo cartridge golden from a moved prefix | ✓ SATISFIED (closed on an **exclusion** row) | Criterion 1's measurements; `.planning/REQUIREMENTS.md:102` reads `Complete`; `data/expected/important-gates-excluded.ids:27` `PKG-05 close_record_only_release_rehearsal_transcript_and_moved_prefix_run` — the evidence is a release-time rehearsal, which no gate step can register |
| TOOL-04 *(narrowed)* | 10-02, 10-06 | A frozen presentation and palette policy keeps colour, scaling and aspect out of the core and out of every claim, and a check refuses a colour symbol in the core | ✓ SATISFIED (closed on an **exclusion** row, per A-02) | Criterion 2's measurements; `.planning/REQUIREMENTS.md:103` `Complete`; `important-gates-excluded.ids:38` `TOOL-04 gate_machinery_self_evidence_enforcement_predates_the_requirement_core_symbols_palette_defined`; the frontend remainder stays in "Out of Scope" (`REQUIREMENTS.md:49`) |
| EV-05 | 10-03, 10-05, 10-06 | A handoff ledger records every human-only item with `evidence: none`, a claim may not cite a handoff artifact, and a later milestone cites the row instead of re-asking | ✓ SATISFIED (three **registry** rows) | Criterion 3's measurements; `.planning/REQUIREMENTS.md:101` `Complete`; `data/expected/important-gates.ids:469-471` `claims control claims-handoff-artifact EV-05`, `claims control claims-handoff-row EV-05`, `text control text-untagged-look-listen EV-05` |

The derived must-be-covered identity closes in both directions, measured with the step's own awk: **Complete 57 − Excluded 12 = Accepted 45 = Registered 45**, `comm -23` and `comm -13` both empty. No orphaned requirement: no id in `REQUIREMENTS.md` maps to Phase 10 that no plan claims.

## What was automated / handed off / could not be judged by a tool

(GUARDRAILS §5.6.)

**Automated, and re-run by me in this session.** The three gate shapes (sh, dash, container) on the final tree; the source-archive route (archive → clean directory with no `.git` → three documented commands → 131-line report → 124/124 golden, with the document's nine-line tail diffed against the measured tail); the rehearsal's own `--source-route` mode; both guards made to fail on scratch copies and both shown green on the good tree; the three new controls plus the colour control run in isolation; the phrase counts and the base-body comparison for criterion 4; the registry identity; the base-versus-final token comparison; the rights and hygiene sweep.

**Handed off, and why.** Five rows in `data/expected/handoffs.ids`, every one `evidence: none`, none citable by a claim:

| Row | Disposition | What the owner owes |
|---|---|---|
| `H7-1` | handed-off | listen to `build/listen/demo-lamp-tone-pass.wav` (21674 bytes) and say whether the tone is the one the program intends |
| `H7-2` | handed-off | look at a `--frame-ppm` export (45806 bytes, `P6` / `159 96` / `255`) and say whether it is the intended picture |
| `H8-3` | handed-off | the same two acts plus a look at a `diag-stic2-` picture; one sitting discharges H8-3 and both Phase 7 rows |
| `D10` | not-performed | physical-hardware measurement (47 `D10-*` items in `data/expected/d10-measurements.ids`) |
| `D11` | not-performed | owner-local real EXEC/GROM images (`sh tools/owner-run.sh` exits 77 without them) |

These are the phase's **deliverable**, not its open criteria: the frozen goal is "the look/listen handoff is recorded rather than assumed", and it is recorded. Nothing here raises the phase to `human_needed`, because no criterion was left to a human — but nothing here is evidence either, and a person's answer will be recorded as `evidence: none` on the same row, never as a claim.

**Could not be judged by a tool.** Only the content of the five handoff rows above (whether the tone is right, whether the picture is intended, what real hardware does, what an owner-local ROM does) — deliberately, structurally, and by prior decision. Everything else in this phase's criteria was judged by a command.

## Deviations accepted

1. **A-01 — the phrase is made identical, not kept identical.** `d3952ea:README.md` contains the phrase **0** times; the frozen sentence "kept identical" was unsatisfiable. Accepted because the amendment is appended, measured and specific: the checked object is the 34-byte substring, the proof is `grep -c -F` = 1 in each file, `grep -x` is refused as unsatisfiable on a `- ` bullet, and `docs/packaging.md:121` is left byte-identical.
2. **A-02 — the oracle order is inverted.** The colour mechanism (`51515f1`, Phase 6) is an ancestor of the policy (`b5ad01d`), so the policy does not create the check. Accepted because it is recorded in the document itself and in `10-02-SUMMARY.md`, proved by `git merge-base --is-ancestor`, and closed on an exclusion row rather than a registry row that would mis-attribute a Phase 6 mechanism to a Phase 10 requirement.
3. **A-04's `lines` figure is wrong and was corrected by measurement.** A-04 predicted `gates=109, lines=335`; the measured values are **109** and **333**. The mechanism is recorded: `lines` counts registry rows only, and the two new exclusion rows live in a different file. `gates`/`lines` were never frozen for this phase, the prediction was made before the commit, and a plan that carried 335 forward would have halted on a green tree.
4. **`controls` moved 386 → 389, one more than plan 10-05's predicted two.** The third control (`claims-handoff-row`) proves the ledger's own grammar; without it both guards would test artifacts and neither the shape of a row. Recorded in `10-05-SUMMARY.md` with its reason. `controls` is not frozen by criterion 5.
5. **Two pre-existing documents gained `[nc]` waivers** (`docs/provenance.md`, `docs/video/demo-lamp-tone-1-derivation.md`) where the newly-added words `looks`/`sounds` appear in descriptive prose. Accepted: `[nc]` is the documented waiver (R5), the words there describe a channel or a scope rather than assert hardware behaviour, and no claim tag was silently removed.
6. **The rehearsal transcript records commit `0cce18b`, not the final tree** (archive 5786235 bytes there, 5804486 on the tree I measured). Accepted: a transcript is a point-in-time record; I re-ran the mode on the final tree and it reports the same golden match. The transcript is explicitly not gate evidence.
7. **`10-06-SUMMARY.md` recorded that its post-SUMMARY gate re-run was not taken** and stated the prediction instead. Closed here: the `sh`, `dash` and container gates were all re-run on the final tree (which carries that file) and all three PASS with every frozen token as predicted, `files=tracked=hygiene-files=2601`.

## Open items

- **None blocking.** No gap, no stub, no orphaned artifact, no broken key link, no debt marker added by this phase (`TBD|FIXME|XXX` in the files this phase touched: none outside archived records that reference formal ids).
- ℹ️ **Info — a cosmetic doc/implementation mismatch.** `data/expected/handoffs.ids`'s header says the reader "must guard on existence with `if(EXISTS ...)`" (CMake idiom); the actual reader is `tools/check/steps/claims.sh:26`, a shell `[ -f "$ROOT/data/expected/handoffs.ids" ]`. The semantics required (absent = empty set) is met and exercised: the good toy tree carries no such file and the step passes. No change needed; recorded so the next reader does not hunt for a CMake guard that is not there.
- ℹ️ **Info — `H7-3` is mentioned in this phase's records** (in the appended note correcting a stale hardware-item count) and is **not** a row in `handoffs.ids`. That is correct: criterion 3 freezes the ledger at exactly H7-1, H7-2, H8-3, D10 and D11, and H7-3 is the Phase 7 record's summary of the D10 items, which `D10` already carries. A later milestone must cite `D10`, not book a sixth row.
- ℹ️ **Info — `core-imports` reads 7 on Darwin and 5 on Linux.** Pre-existing, not a frozen token, unchanged by this phase.
- ℹ️ **Info — the container run uses an unnamed container and the `ubuntu:24.04` image** (`docker run --rm`), which is `tools/gate-linux.sh`'s pre-existing behaviour; the phase did not change that script. It is removed at exit by `--rm`.
- **Deferred, correctly:** installing `bin/ivs-run` or any image (D14, structurally blocked), a windowed frontend, a palette table, a second cartridge, and any new gate step — all excluded by the phase's own boundary and confirmed absent by measurement (`git diff --stat d3952ea..HEAD -- data/expected/pkg-install-files.ids` empty; `prefix-files=11`; `ls bin` → no such directory; 38 step scripts; `gate-steps/` unchanged; no `find_package`/`FetchContent` line added to any `CMakeLists.txt`).

## Rights, hygiene and integrity sweep

| Check | Command | Result |
|---|---|---|
| no tracked binaries | `git ls-files \| grep -E '\.(bin\|rom\|ivsc\|wav\|ppm)$'` | none |
| `binary-ext.txt`, `.gitignore` unchanged | `git diff --stat d3952ea..HEAD -- tools/check/binary-ext.txt .gitignore` | empty |
| single author | `git log --format='%an <%ae>' d3952ea..HEAD \| sort -u` | `88 szTheory <szTheory@users.noreply.github.com>` |
| no tilde-home literal in the phase's files | grep over every file in `git diff --name-only d3952ea..HEAD` | no hits; the only repo-wide hits are `tools/hygiene.sh`'s own patterns and the rule text in `.planning/AUTONOMY.md` |
| no secrets | `git grep -i -E 'BEGIN (RSA\|OPENSSH\|PGP) PRIVATE KEY\|ghp_[A-Za-z0-9]{20,}\|sk-[A-Za-z0-9]{20,}\|AKIA[0-9A-Z]{16}'` | none |
| nothing weakened to go faster | `git diff --stat d3952ea..HEAD -- tools/gate.sh tools/gate-linux.sh .github/ CMakeLists.txt CMakePresets.json` | empty; no `continue-on-error`, no path filter, no dropped control/fault/test (`controls` 386→389, `faults` 235/235, `goldens` 29, `tests-*` 4958/4954/4/0 all unmoved) |
| no archive rewritten | `git show --numstat 4ff8a7c -- <the four corrected files>` | `12/0`, `14/0`, `17/0`, `21/0` — **zero deleted lines** in all four |
| the guards were strengthened, not relaxed | `git diff d3952ea..HEAD -- tools/check/steps/text.sh tools/check/steps/claims.sh tools/check/kinds/claim.cmake` | the only removed lines are two header comments and the claim-word regex line, replaced by the same regex plus `looks\|sounds`; nothing else removed |
| frozen criteria untouched | `git diff d3952ea..HEAD -- .planning/ROADMAP.md` | only the `**Plans**` checklist was filled; the five criteria, the tier ceiling and the not-in-this-phase line are byte-identical |

## VERIFICATION: passed

---
phase: 16-owner-route-claim-and-release-posture
verified: 2026-11-26T12:46:34Z
status: passed
score: 20/20 must-haves verified
covered_files:
  - ".planning/CI-EFFICIENCY.md"
  - ".planning/REQUIREMENTS.md"
  - ".planning/ROADMAP.md"
  - ".planning/STATE.md"
  - ".planning/WINDOWS.md"
  - ".planning/ci-runs.tsv"
  - ".planning/ops/RELEASE-OPS.md"
  - ".planning/phases/16-owner-route-claim-and-release-posture/16-01-PLAN.md"
  - ".planning/phases/16-owner-route-claim-and-release-posture/16-01-SUMMARY.md"
  - ".planning/phases/16-owner-route-claim-and-release-posture/16-02-PLAN.md"
  - ".planning/phases/16-owner-route-claim-and-release-posture/16-02-SUMMARY.md"
  - ".planning/phases/16-owner-route-claim-and-release-posture/16-03-PLAN.md"
  - ".planning/phases/16-owner-route-claim-and-release-posture/16-03-SUMMARY.md"
  - ".planning/phases/16-owner-route-claim-and-release-posture/16-04-PLAN.md"
  - ".planning/phases/16-owner-route-claim-and-release-posture/16-04-SUMMARY.md"
  - ".planning/phases/16-owner-route-claim-and-release-posture/16-05-PLAN.md"
  - ".planning/phases/16-owner-route-claim-and-release-posture/16-05-SUMMARY.md"
  - ".planning/phases/16-owner-route-claim-and-release-posture/16-VALIDATION.md"
  - "data/expected/handoffs.ids"
  - "data/expected/important-gates.ids"
  - "docs/handoffs.md"
  - "docs/owner-images.md"
  - "tools/check/controls/_good/tree/.planning/REQUIREMENTS.md"
  - "tools/check/controls/_good/tree/.planning/milestones/v0.1.0-REQUIREMENTS.md"
  - "tools/check/controls/_good/tree/data/expected/handoffs.ids"
  - "tools/check/controls/_good/tree/data/expected/important-gates.ids"
  - "tools/check/controls/claims-handoff-disposition/overlay/data/expected/handoffs.ids"
  - "tools/check/controls/claims-handoff-id-duplicate/overlay/data/expected/handoffs.ids"
  - "tools/check/controls/claims-handoff-id-extra/overlay/data/expected/handoffs.ids"
  - "tools/check/controls/claims-handoff-id-missing/overlay/data/expected/handoffs.ids"
  - "tools/check/controls/important-gates-bad-req-phase5/overlay/data/expected/important-gates.ids"
  - "tools/check/controls/important-gates-bad-req-phase6/overlay/data/expected/important-gates.ids"
  - "tools/check/controls/important-gates-complete-uncovered/overlay/.planning/REQUIREMENTS.md"
  - "tools/check/controls/important-gates-req-uncovered-phase5/overlay/data/expected/important-gates.ids"
  - "tools/check/controls/important-gates-req-uncovered-phase6/overlay/data/expected/important-gates.ids"
  - "tools/check/controls/important-gates-req-uncovered-phase7/overlay/data/expected/important-gates.ids"
  - "tools/check/controls/important-gates-uncovered-gate/overlay/data/expected/important-gates.ids"
  - "tools/check/controls/important-gates-unknown-id-phase5/overlay/data/expected/important-gates.ids"
  - "tools/check/controls/important-gates-unknown-id-phase6/overlay/data/expected/important-gates.ids"
  - "tools/check/controls/important-gates-unknown-id/overlay/data/expected/important-gates.ids"
  - "tools/check/steps/c-faults.sh"
  - "tools/check/steps/c-tests.sh"
  - "tools/check/steps/claims.sh"
  - "tools/check/steps/important-gates.sh"
  - "tools/gate.sh"
  - "tools/release-rehearse.sh"
covered_digest: "v4:sha256:3ba82cc9aea90fa465d9ac470603eb6657d4140f3e2befaadc9afc8c6d31967b"
behavior_unverified: 0
overrides_applied: 0
re_verification:
  previous_status: gaps_found
  previous_score: 19/20
  gaps_closed:
    - "16-VALIDATION.md's Validation Sign-Off no longer ticks the superseded `Feedback latency < 120s (measured ~72 s full gate)`; line 93 now reads `Feedback latency within the 180 s budget (measured 66–147 s on a loaded host; the lever's own effect was 79 → 72 s on a quiet tree)`, which agrees with the file's own `Measured runtime` row, and the Manual-Only closing sentence now says `All five` over the five-row table. Every `**Satisfied by**` is lowercase, so plan 16-05 Task 2's case-sensitive proxy now reads 10 against the required ≥8."
    - "16-05-SUMMARY.md now names all eight handoff ids (`D10`, `D11`, `H7-1`, `H7-2`, `H8-3`, `H16-1`, `H16-2`, `H16-3` — each `grep -c` ≥ 1), states `CL-0002` as the only `established` claim, states that no claim reaches `measured-hardware`, carries the growth-gate statement and the four tokens the phase owns, keeps the no-shared-object sentence, and carries an explicit `A handoff row is not evidence` sentence."
  gaps_remaining: []
  regressions: []
advisory:
  - finding: "16-05-SUMMARY.md's token reconciliation is prose ('the four entitled growth gates ... nothing else moved', with the moved tokens named) rather than the one-row-per-token table plan 16-05 Task 1's action describes."
    category: other
    reason: "The substance is present and was independently re-derived in this verification (every token of the closing line checked against the five plans' predictions), so this is a form deviation, not a missing reconciliation. The plan's binding acceptance criteria and its verify block ask for the growth-gate statement, which is present. A table would make the next reader's check mechanical."
    evidence_status: "verified by re-derivation; form differs from the plan's action text"
  - finding: "16-05 Task 2's action text also asks the SUMMARY to enumerate the milestone's five additions (rights ledger with an enforced claim ceiling, flat-image wrap with typed refusals, cartridge role on the owner route, build-only host shim with one `provisional` `deterministic-repeatability` claim, documented route rehearsed from the source archive) and to say which handoff a reader would open first; the SUMMARY states the claim posture and every id but not that enumeration or that ordering hint."
    category: other
    reason: "Neither the plan's own `<verify><automated>` block nor its `<acceptance_criteria>` requires them — both require the growth-gate statement, the plainly worded can/cannot-claim statement and all eight ids, which are present. The phase truth these serve ('states plainly what the milestone can and cannot now claim and names the handoffs that remain') is met. Recorded so the next plan-writing pass tightens the action text to its own contract, or adds the sentence."
    evidence_status: "checked mechanically against both lists; not required by either"
---

# Phase 16: Owner Route, Claim and Release Posture Verification Report

**Phase Goal:** The owner route from the source archive is documented and rehearsed, the one question only
the owner can answer is a handoff row, and the release posture states plainly that no host artifact and no
content ships.
**Verified:** 2026-11-26T12:46:34Z at `9dda518`
**Status:** passed
**Re-verification:** Yes — after `1600d96` and `9dda518` closed the two record gaps the previous report left

**What was re-derived this session, and what was carried forward deliberately.** This round re-ran the full
gate **once** under `sh` (exit 0, `gate: PASS`, 106 tokens, 1,520-byte final line, zero `^-- FAIL` lines,
**188 s** wall on a loaded host), re-read every invariant token on that line, re-derived the handoff ledger,
the D10 inventory and the package inventory from their files, re-read the claim ledger's posture from
`data/claims.json`, re-medianed the last five `.planning/ci-runs.tsv` rows by arithmetic, re-read the
roadmap's progress table and the Phase 16 section, re-read the retained `build/gate-linux.log`, and ran plan
16-05 Task 2's assertions one by one against both `16-VALIDATION.md` and `16-05-SUMMARY.md`.
`sh tools/hygiene.sh --tree` exited 0 with its pass line. Not re-run, by instruction and because nothing they
measure has changed: the container leg (read from the retained `build/gate-linux.log`, written 06:11 local,
before these commits — a second container run would overwrite the only copy, lesson 18), the `dash` shell
leg, the overlap on/off pair, the release-time rehearsal, and the committed-direction probe.
`git diff --name-status 9703594..HEAD` is exactly three planning files (`16-VALIDATION.md`,
`16-05-SUMMARY.md`, and this report's previous revision); no source file moved. Every phase source file's
last touch is unchanged (`tools/gate.sh`/`c-tests.sh`/`c-faults.sh` `f9fe6a9`, `tools/release-rehearse.sh`
`14f81ed`, `claims.sh`/`handoffs.ids`/`docs/handoffs.md` `43b4394`, `important-gates.ids`/`important-gates.sh`
`298be25`, `docs/owner-images.md` `6b57148`, `tools/check/steps/files.sh` `c44aa48`), so the runs that
produced the carried-forward results were made on byte-identical files. No SUMMARY figure was accepted as
evidence.

## Goal Achievement

### Observable Truths

| # | Truth | Status | Evidence |
|---|-------|--------|----------|
| 1 | **Criterion 1.** One documented owner route (EXEC + GROM + a wrapped cartridge, three commands) is rehearsed from the **source archive** to a project-cartridge golden, with the transcript recorded; a release-time rehearsal, not a gate step. | ✓ VERIFIED | Judgment carried forward, re-anchored on unchanged files. `tools/release-rehearse.sh` is byte-identical to the version whose run printed `rehearse: source-archive: equal (6793979 bytes)`, `rehearse: source-route: ivs-v0.2.0-source.tar.gz, golden match (report 131 lines, 124 of 124 compared), prefix ivs-v0.2.0`, `rehearse: ok`, exit 0; the limb builds and runs inside the extracted prefix (`:164-215`), so the route is the archive's. `docs/owner-images.md:55-92` documents the three commands and cites `.planning/ops/RELEASE-OPS.md` section (j), whose transcript names its commit (`14f81ed`). No gate step runs it (`grep -rn release-rehearse tools/check/steps/` = 0). |
| 2 | **Criterion 2.** The route's pass path is exercised with project-made files, and a one-byte EXEC mutation proves the id can fail; the `exec:` label stays `-` so no run reads as a real-EXEC result. | ✓ VERIFIED | Carried forward on unchanged script and records: `rehearse: owner-route: wrap base=4000 word-order=hi-first digest=8d73b98c58bb75a6515c6a791ebd2925265d05874e324c31a5836e61055b74f5`, `run exit=10 outcome=guest-fail exec-label=- cart-digest-matched=yes`, `mutation exit=14 status=IVS_E_DIGEST_MISMATCH run-state=unreset`. The pass path cannot satisfy the second limb (the mutation must be refused), and `D11` is `not-performed` in the ledger read this session. |
| 3 | **Criterion 3.** The owner-only questions are `handoffs.ids` rows, `evidence: none`, dispositioned, cited by no claim and by no phase exit criterion. | ✓ VERIFIED | Ledger re-read this session: eight rows, `H7-1/H7-2/H8-3/H16-1/H16-2/H16-3 handed-off evidence: none`, `D10/D11 not-performed evidence: none`. `data/claims.json` holds **zero** `H16-`, `H7-1`, `H7-2`, `H8-3` and `handoffs` occurrences; `D10`/`D11` appear only in `note`-field strings of exclusion prose (17 and 9 hits, every one in `claims[i].note`, none in `artifact` or any other field), which is the opposite of citing them as evidence. `tools/check/kinds/claim.cmake:145` refuses the ledger path as a claim artifact. The five ROADMAP success criteria cite no id; `docs/handoffs.md` carries one section per row (eight) plus the closing 'These rows are never a result' paragraph. |
| 4 | **Criterion 4a.** A tracked shared object is refused by name — including the **committed** direction Phase 15 could not reach from a control. | ✓ VERIFIED | Judgment and probe carried forward on unchanged `tools/check/steps/files.sh` (last touched `c44aa48`) / `files.cmake` / `binary-ext.txt`: the probe staged `probe.so` and `probe.txt`, produced exactly `-- FAIL files-unlisted-binary: probe.so` with no `probe.txt` occurrence and no pass line, `binscan.sh binary` stayed silent, and both files were removed with `git status --porcelain` empty. The gate this session is green with `binaries-listed=0` and `rights-pinned=4/4`; `release-selftest=12/12` and `pkg-selftest=20/20` keep `so-member` and `host-so-absent` live, and `important-gates.ids:608,611,620` register the three `release-posture` rows. |
| 5 | **Criterion 4b.** The installed package stays eleven files, and the release-decision line reads: the release ships the static package and the source archive; no shared object, no host shim, no content. | ✓ VERIFIED | Recomputed this session: `awk '!/^#/ && NF' data/expected/pkg-install-files.ids \| wc -l` = **11**, and the gate line carries `prefix-files=11` and `binaries-listed=0`. The line appears verbatim in `.planning/ops/RELEASE-OPS.md` (i) at `:78` and (i.1) at `:108`, `.planning/PROJECT.md:297`, `.planning/ROADMAP.md:284`, and in substance at `docs/packaging.md:123`. Section (i.1) remains the 63-added/0-deleted append whose five values are all on this session's closing line. |
| 6 | **Criterion 5a.** The concurrency lever is taken in **one cost-only commit** whose summary token line is byte-identical and whose `step.out.<name>` buffers are compared. | ✓ VERIFIED | Judgment carried forward; the lever's files are unchanged since `f9fe6a9` (`git diff 9703594..HEAD` touches no source). `overlap_bg=c-faults`/`overlap_fg=c-tests` are declared once (`tools/gate.sh:44-45`) and validated against the producer list before `gate-overlap pair=c-faults+c-tests` prints before any step this session; the partition is `preflight=29 producers=9`, and both step pass lines are present in the log body exactly once (`-- c-tests pass ctest-runs=2`, `-- c-faults pass fault-runs=238`). **Caveat kept honestly:** each of the two steps emits exactly one line, so for these two steps the per-buffer comparison *is* the pass-line comparison. It is not vacuous (a changed count, an extra diagnostic or a missing pass line changes it, and a failing step breaks the liveness guard), but it is not stronger than the token-line comparison for these two steps — no gate weakening, and no additional coverage claimed. |
| 7 | The lever adds no tracked file, and the inline build lock is byte-identical between its own markers. | ✓ VERIFIED | Carried forward on unchanged `c-tests.sh`/`c-faults.sh`: the `# build-lock begin`/`end` regions are `cmp`-equal with no normalisation (re-checked this session: both blocks present, equal, 1,246 bytes), and each script carries the sentence naming the other copy and why it is a copy. |
| 8 | The pair's two builds are serialised by a bounded lock whose necessity is a measurement, not an assumption. | ✓ VERIFIED | Carried forward: one polled `mkdir` lock in the per-run results directory, released on the success path, the failure path and the caller's `EXIT`/`HUP`/`INT`/`TERM` traps, with a `build-lock` detail on expiry; the recorded reason (two unlocked concurrent builds fail at link with a half-written `core/libivs_core.a`) is in `.planning/CI-EFFICIENCY.md` and the scripts' comments. |
| 9 | The overlap costs wall time only: `gate-time total` falls and `gate-cpu` does not rise beyond noise; no allowance credit and no checkpoint word is claimed for it. | ✓ VERIFIED | Carried forward. The phase's pre/post figures are not reproducible from the committed tree (the two logs were kept in `/tmp`), and the host was not idle across the sessions that measured it — this session's single `sh` run (188 s wall) is one more sample of that variance, which is why the validation record's runtime row quotes a range with a disclaimer. `.planning/CI-EFFICIENCY.md` claims **no allowance credit and no checkpoint word** for the lever and states the hosted saving as a prediction (~144 s). |
| 10 | Nothing else moved: `steps=38/38`, `controls=404/404`, `faults=238/238`, `goldens=29`, the partition stays `preflight=29 producers=9`, and both steps keep their own pass lines. | ✓ VERIFIED | Every token re-read from this session's closing line: `steps=38/38 controls=404/404 faults=238/238 goldens=29`, `gate-partition preflight=29 producers=9`, `ls tools/check/steps/*.sh \| wc -l` = 38, and each step pass line appears exactly once in the log body. |
| 11 | The route stays release-time: no gate step runs it, CI never runs it, no test id/control/fault/golden is added, and every token is in place. | ✓ VERIFIED | `grep -rn release-rehearse tools/check/steps/` is empty; the mode refuses a CI environment with exit 2 rather than skipping; `steps=38/38`, `controls=404/404`, `faults=238/238`, `goldens=29`, `gates=119`, `lines=364`, `tests-expected=5042` — the only phase movements are the four entitled growth gates. |
| 12 | The ledger's row count is a stat counted from the file on every run, not a quoted figure. | ✓ VERIFIED | `tools/check/steps/claims.sh:108` prints `handoffs=<n>` from the ledger with the grammar pass's own row predicate; this session's line carries `handoffs=8` and the file's non-comment, non-empty row count is 8. |
| 13 | The id set and every disposition are pinned in both directions, so a missing, extra, duplicated or re-dispositioned row is a named finding. | ✓ VERIFIED | `HANDOFF_IDS` and `HANDOFF_DISP` name each id and disposition; the four `claims-handoff-*` controls plus `-row`/`-missing`/`-unreadable` are committed and all 404 controls pass this session. |
| 14 | Every ledger copy moved in the same commit, so each control still prints only its own intended finding. | ✓ VERIFIED | The good tree's ledger and the four overlays each carry the three rows; `run-controls.sh` printed `controls=404/404` inside this session's gate with no toy-drift, good-failed, token-missing, exit-zero, inventory or orphan finding. |
| 15 | Nothing about the rows is a result: an answer is `evidence: none` either way, no row is a pass, no claim may cite one, and `D10`/`D11` stay `not-performed`. | ✓ VERIFIED | Ledger read this session: every row's last two tokens are literally `evidence: none`, `D10`/`D11` are `not-performed`, and no claim cites any of them. The packet repeats the rule in 'The one rule to carry away' and 'These rows are never a result'. |
| 16 | PKG-06 is `Complete` in the same commit as the three registry rows and the thirteen toy lines, so neither `important-gates-unknown-id` nor `-complete-uncovered` can fire. | ✓ VERIFIED | `.planning/REQUIREMENTS.md:46` `- [x] **PKG-06**`, `:112` `\| PKG-06 \| Phase 16 \| Complete \|`; the three `release-posture` rows resolve in their inventories (`important-gates.ids:608,611,620`); this session's line carries `gates=119 lines=364` with `rows-pending=0`, and the gate is green so neither finding fired. |
| 17 | The three rows are pointers to mechanisms that already exist: `steps` and `faults` do not move. | ✓ VERIFIED | `steps=38/38`, `faults=238/238`, `fault-runs=238` this session; the Phase 16 block in `tools/check/steps/important-gates.sh:81-91` still records 'zero new faults, zero new steps' and why the two omissions are deliberate. |
| 18 | The closing gate line is measured under `sh` and `dash` and is byte-identical, and the container leg ran once with exactly the four documented host tokens differing. | ✓ VERIFIED | Re-ran the `sh` leg once this session: exit 0, `gate: PASS`, final line **1,520 bytes** / 106 tokens, zero `^-- FAIL` lines, every predicted token present exactly once. The `dash` leg and the container leg are carried forward from the previous verification (same scripts, same records; `build/gate-linux.log` is unchanged — mtime 06:11, before these commits — and still shows `gate: PASS`, a 1,512-byte line and exactly `cc`, `core-imports`, `archive`, `refs-verified` differing from the *recorded* line; the byte arithmetic closes at 1,519 − 7 − 1 = 1,511 characters, i.e. 1,520 − 8 = 1,512 bytes with the newline). **Measured difference from the recorded line:** three `git ls-files`-derived count tokens read **2789** where the record reads 2787 — `tracked`, `hygiene-files` **and `files`**. All three are `git ls-files \| wc -l`-shaped, and the delta is exactly the two planning records added after the measurement (`16-05-SUMMARY.md` in `438bffd`, `16-VERIFICATION.md` in `9703594`). Every evidence token is unchanged and the line is still 1,520 bytes, so the claim about the gate's content holds; the quoted line is reproducible at its measurement commit, not at `HEAD`. Recorded as ℹ️ Info, not a gap — a `tracked` count necessarily counts the records that quote it. |
| 19 | The phase boundary records **exactly one** checkpoint verdict in the section 2.3 vocabulary, with the medians recomputed by arithmetic from `.planning/ci-runs.tsv`. | ✓ VERIFIED | Recomputed this session from the last five rows (`45723269901`, `45747709457`, `45751979661`, `45778926664`, `45779277467`): `cpu_s` 368, 366, 359, 441, 352 → **366**; `build_s` 35, 34, 34, 38, 33 → **34**; `gate_s` 122, 115, 120, 145, 115 → **120**, identical to the record's 366 / 34 / 120 against the frozen 361 / 34 / 118. `discharged-within-allowance` is unreachable (366 > 361.4 by 4.6); `lever-required` is unreachable (cumulative priced total 2.0 + 1.3 + 1.5 + 0.4 = **5.2 of 21**, the phase's actual addition 0.015 against the 0.4 it priced); `reprice` is therefore forced and is the single word in `.planning/CI-EFFICIENCY.md:2123`, `.planning/STATE.md:55,64,791`, with `steps=38/38` recorded as the invariant. The previous report's rule-wording note (§2.3's 'with an unchanged token line' versus this phase's four entitled growth gates) stands unchanged as an observation for the next milestone, not a defect in this record. |
| 20 | **16-05 Task 2.** `16-VALIDATION.md` is signed off in the required shape, and the phase SUMMARY states plainly what the milestone can and cannot now claim and names the handoffs that remain. | ✓ VERIFIED | **Both former gaps are closed, checked mechanically against Task 2's own assertion list.** *Record half:* `status: validated`, `nyquist_compliant: true`, `wave_0_complete: true`, no placeholder brace, no `TBD`, no `<stage2>`, **10** task rows (`^\| 16-0[1-5]-T[12] \|`), `PKG-06` ×15 (≥10), Wave 0 resolved **10 of 10** with lowercase `**satisfied by**` (proxy reads **10** against the required ≥8), five Manual-Only data rows naming `H16-1`, `H16-2`, `H16-3`, `D10`, `D11` with the closing sentence now reading `All five`, line 93 now `Feedback latency within the 180 s budget (measured 66–147 s on a loaded host; the lever's own effect was 79 → 72 s on a quiet tree)` — agreeing with the file's own measured row and no longer the superseded `< 120s / ~72 s` estimate — and an Approval line quoting the measured 1,520-byte closing line verbatim in a fenced block (`cmp`-identical to its single occurrence in `STATE.md` apart from the three count tokens below). *SUMMARY half:* all eight ids present (`D10` ×3, `D11` ×2, `H7-1`/`H7-2`/`H8-3`/`H16-1`/`H16-2`/`H16-3` ×1 each), `CL-0002` stated as the only `established` claim, `measured-hardware` stated as unreachable, the growth-gate statement (×3), `gates=119`, `lines=364`, `handoffs=8`, `provisional`, the four owned tokens and their meaning, and the explicit sentence `A handoff row is not evidence`. Two literal greps still read 0 on form only — `no shared object` is line-wrapped (`no shared`/`object, no host shim, no content byte.`) and is present when whitespace is collapsed; `sh tools/hygiene.sh --tree` prints its pass line rather than being silent. The remaining four failing assertions are plan-proxy defects, not record defects: `T-16-(0[1-9]\|1[0-9]\|SC)` reads 9 because it cannot match `T-16-20`; `grep -E '✅ green' \| wc -l` reads 11 because it counts the `*Status: …*` legend line; `no shared object` as above; and the verbatim-line grep reads 0 at `HEAD` only through the 2787→2789 count drift. All are recorded below as ℹ️ Info and in `advisory`; the plan's binding `<acceptance_criteria>` are met. |

**Score:** 20/20 truths verified (0 present, behavior-unverified; 0 overrides).

### Deferred Items

None. Every must-have of this phase is either met or was met in the phase itself; nothing here is addressed by
a later milestone phase.

### Advisory (New Scope, Unevidenced)

| # | Finding | Category | Why Advisory |
|---|---------|----------|--------------|
| 1 | `16-05-SUMMARY.md`'s token reconciliation is prose, not the one-row-per-token table plan 16-05 Task 1's action describes | other | Form only; the substance was re-derived in this verification against the gate's own line, and the plan's binding acceptance criteria ask for the growth-gate statement, which is present. |
| 2 | Task 2's action text asks the SUMMARY to enumerate the milestone's five additions and to say which handoff a reader would open first; neither the `<verify>` block nor the `<acceptance_criteria>` requires them, and neither is present | other | The phase truth these would serve is met by the can/cannot-claim statement and the eight named ids. Recorded so the next plan-writing pass either tightens the action text to its own contract or adds the sentence. |

### Required Artifacts

| Artifact | Expected | Status | Details |
|----------|----------|--------|---------|
| `tools/gate.sh` | the declared overlap pair, the `gate-overlap` measurement line, join on both paths, trap kill, safety net | ✓ VERIFIED | `overlap_bg`/`overlap_fg` declared once (`:44-45`) and checked against the producer list (`:154-155`); `gate-overlap pair=c-faults+c-tests` printed before any step this session; `record_rc`/`t.<name>` written exactly as the sequential loop did; replay untouched. |
| `tools/check/steps/c-tests.sh`, `c-faults.sh` | the byte-identical inline build lock around every `cmake --build` | ✓ VERIFIED | `cmp`-equal between `# build-lock begin`/`end` (re-checked: 1,246 bytes, identical); bounded polled `mkdir` in the per-run results directory; released on both paths and the caller's traps. |
| `tools/release-rehearse.sh` | `--source-route` performs the three owner-route commands against the archive | ✓ VERIFIED | Byte-unchanged since the run that printed `rehearse: ok`; the route limb extracts, configures, builds, wraps and runs inside `$work/route/$prefix` (`:164-215`). |
| `docs/owner-images.md` | "The route from the source archive, in three commands", citing section (j) | ✓ VERIFIED | `:55-92`; three commands, the base/word-order contract, `refs/`, the container as derived material, the pointer to section (j), the one-byte refusal. |
| `.planning/ops/RELEASE-OPS.md` | section (j) transcript; section (i.1) recomputation | ✓ VERIFIED | (j) at `:163` with the four printed lines verbatim and the three "must not infer" sentences; (i) at `:78` and (i.1) at `:108` as a 63-added/0-deleted append. |
| `data/expected/handoffs.ids` | eight rows, three new, all `evidence: none` | ✓ VERIFIED | Read this session: `H7-1/H7-2/H8-3/H16-1..H16-3 handed-off`, `D10/D11 not-performed`, every row ending `evidence: none`. |
| `docs/handoffs.md` | packet sections in the page's shape | ✓ VERIFIED | Sections at `:15, :38, :58, :71, :78, :92, :122, :134` — one per row plus the rule and closing paragraphs; `H16-1` names the shim target and the nine `digest.*` lines, `H16-2` the route + section (j), `H16-3` the two wraps at `hi-first`/`lo-first`. |
| `data/expected/important-gates.ids` | three `release-posture` rows with per-surface comments | ✓ VERIFIED | `:598-620`; each row resolves; the two deliberate omissions are recorded with their reasons; `gates=119 lines=364 rows-pending=0`. |
| `.planning/REQUIREMENTS.md` | PKG-06 `Complete` (checkbox, traceability row, `Last updated`, coverage) | ✓ VERIFIED | `:46`, `:112`, `:118`, `:128`, `:136`; no `Pending` PKG-06 remains; PKG-06 is Phase 16's only requirement, so nothing is orphaned. |
| `tools/check/steps/important-gates.sh` | the Phase 16 block | ✓ VERIFIED | `:81-91`, naming PKG-06, the one new gate name, the three rows, "zero new faults, zero new steps" and both omissions. |
| `.planning/phases/16-…/16-VALIDATION.md` | the signed record plan 16-05 Task 2 specifies | ✓ VERIFIED | Signed and frontmatter-complete, ten task rows with the right threat refs, Wave 0 resolved 10/10 lowercase, five Manual-Only rows with `All five`, the measured line quoted verbatim and `cmp`-identical to `STATE.md`, the recomputed Sampling Rate and the corrected latency bullet. Both former defects closed. |
| `.planning/phases/16-…/16-05-SUMMARY.md` | what the milestone can and cannot claim, naming every remaining handoff | ✓ VERIFIED | Names all eight ids with their dispositions, states `CL-0002` as the sole `established` claim, no claim reaches `measured-hardware`, the growth gates and the four owned tokens, the no-shared-object sentence, and "A handoff row is not evidence". Both former defects closed. |
| `.planning/STATE.md`, `.planning/ROADMAP.md`, `.planning/CI-EFFICIENCY.md` | the closing line once, the phase tokens, one verdict, the plan list | ✓ VERIFIED | STATE carries the full line once (1,519 bytes without the newline, 106 tokens), the tokens, the `reprice` bullet with its arithmetic and the `not-recorded` hosted row with its reason; CI-EFFICIENCY carries `phase16: reprice (…)` at `:2123`; ROADMAP's v0.3.0 table reads `16. Owner Route, Claim and Release Posture \| 5/5 \| Complete \| 2026-11-26` at `:519` and the Phase 16 section reads `**Plans**: 5/5 plans executed` at `:385`. |

### Key Link Verification

| From | To | Via | Status | Details |
|------|----|-----|--------|---------|
| `tools/gate.sh` | `tools/check/steps/c-tests.sh` / `c-faults.sh` | background partner started with the same redirection, foreground member, join writing `rc.`/`t.` | ✓ WIRED | Both step pass lines appear in this session's log body, the partition and pair lines are printed, and the gate is green with the same tokens as the serial run measured two rounds ago. |
| `docs/owner-images.md` | `.planning/ops/RELEASE-OPS.md` section (j) | the route's rehearsal note cites the section by path and letter | ✓ WIRED | The pointer resolves (`## (j)` exists exactly once, at `:163`) and every figure it points at reproduced in the previous session's run on unchanged files. |
| `tools/release-rehearse.sh` | `data/golden/demo-lamp-tone-pass.digests` | the filtered-report comparison, unchanged, still last in the limb | ✓ WIRED | `124 of 124 compared`, report 131 lines, `rehearse: ok`. |
| `tools/release-rehearse.sh` | the archive's `tools/owner-run.sh` and `ivs_flatwrap` | paths relative to `$work/route/$prefix` | ✓ WIRED | `[ -x "$wrap" ]` and `[ -f "$run" ]` guards; both are the extracted archive's, never the repository's. |
| `data/expected/handoffs.ids` | `tools/check/steps/claims.sh` | grammar pass, then the id set and each disposition against the two pinned lists; `handoffs=` from the same read | ✓ WIRED | `handoffs=8` equals the file's row count; a missing/extra/duplicate/re-dispositioned row is a named finding proved by the four controls. |
| `data/expected/handoffs.ids` | `docs/handoffs.md` | the ledger is the inventory, the packet the instruction sheet | ✓ WIRED | One section per row (eight sections read this session); the page states no count. |
| `.planning/REQUIREMENTS.md` | `data/expected/important-gates.ids` | the traceability table is the derived covered set | ✓ WIRED | `gates=119 lines=364`, `rows-pending=0`; both refusal directions are exercised by committed controls. |
| `.planning/ci-runs.tsv` | the checkpoint bullet | last five rows, each window sorted, middle value printed | ✓ WIRED | Recomputed this session: 366 / 34 / 120, identical to the record. |
| `.planning/ROADMAP.md` v0.3.0 table | the same file's Phase 15 and Phase 16 sections | the table's `Plans Complete`/`Status` columns against each section's `**Plans**` line and ticked entries | ✓ WIRED | `15 … 3/3 Complete`, `16 … 5/5 Complete`, and the Phase 16 section reads `**Plans**: 5/5 plans executed` with five ticked entries; unchanged this round. |

### Data-Flow Trace (Level 4)

| Artifact | Data Variable | Source | Produces Real Data | Status |
|----------|---------------|--------|--------------------|--------|
| `tools/check/steps/claims.sh` | `handoffs` | `awk` over `data/expected/handoffs.ids` on every run | yes — 8 rows, equals the file | ✓ FLOWING |
| `tools/check/steps/important-gates.sh` | `gates`/`lines` | the registry file parsed on every run | yes — 119/364, re-derived | ✓ FLOWING |
| `.planning/STATE.md` checkpoint bullet | `cpu_s`/`build_s`/`gate_s` medians | the last five `.planning/ci-runs.tsv` rows | yes — 366/34/120, recomputed this session | ✓ FLOWING |
| `.planning/ops/RELEASE-OPS.md` §(j) | archive name, prefix, golden count, wrap digest, outcomes | the rehearsal's own printed lines | yes — reproduced by the previous session's re-run, digest included; script unchanged since | ✓ FLOWING |
| `docs/handoffs.md` H16-1 | object digest and the nine `digest.*` lines | the release preset's build and `tests/host/ivs_host_harness.c` | yes — the command the packet prints is the one the plan ran | ✓ FLOWING |

### Behavioral Spot-Checks

| Behavior | Command | Result | Status |
|---------|---------|--------|--------|
| The full gate is green on this tree | `sh tools/gate.sh` (one run this session) | exit 0, `gate: PASS steps=38/38 … gates=119 lines=364 handoffs=8`, 106 tokens, 1,520-byte final line, 0 `^-- FAIL`, **188 s** wall on a loaded host | ✓ PASS |
| Every invariant token is where the plans predicted | token grep over the closing line | all 18 checked tokens present exactly once (`steps controls faults goldens gates lines handoffs claims provisional prefix-files binaries-listed rows-pending rights-pinned release-selftest pkg-selftest d10-items tests-expected tests-missing`) | ✓ PASS |
| The overlap pair and partition are live | `grep` over the gate log body | `gate-overlap pair=c-faults+c-tests` ×1, `gate-partition preflight=29 producers=9`, `-- c-tests pass ctest-runs=2` ×1, `-- c-faults pass fault-runs=238` ×1 | ✓ PASS |
| The recorded medians reproduce by arithmetic | Python `statistics.median` over the last five `ci-runs.tsv` rows | 366 / 34 / 120, identical to the record | ✓ PASS |
| The handoff ledger, D10 inventory and package inventory are what the records say | `awk` row counts over the three `.ids` files | `handoffs=8`, `d10-items=47`, `pkg-install-files=11` | ✓ PASS |
| The claim ledger's posture is what the SUMMARY says | Python read of `data/claims.json` | 29 claims, 28 `provisional` / 1 `established` (`CL-0002`, `source-expectation`), no `measured-hardware`, no handoff artifact, zero `H16-`/`H7-1`/`H7-2`/`H8-3`/`handoffs`; `D10`/`D11` only inside `note` prose | ✓ PASS |
| The hygiene step is clean | `sh tools/hygiene.sh --tree` | exit 0, `-- hygiene pass mode=tree hygiene-files=2789` | ✓ PASS |
| The roadmap's status surface agrees with its phase sections | read of the v0.3.0 table + Phase 16 section | `16 … 5/5 \| Complete \| 2026-11-26` at `:519`; section says `5/5 plans executed` at `:385` | ✓ PASS |
| Plan 16-05 Task 2's assertions against `16-VALIDATION.md` | each assertion run separately | 12 pass (frontmatter ×3, no placeholders/`TBD`/`<stage2>`, 10 task rows, `PKG-06` ×15, `satisfied by` = **10**, five handoff ids, Approval) and 2 fail on plan-proxy defects only: `T-16-…` regex = 9 (cannot match `T-16-20`) and `✅ green` = 11 (counts the legend line) | ✓ PASS (record); ℹ️ plan proxies defective |
| Plan 16-05 Task 2's assertions against `16-05-SUMMARY.md` | each assertion run separately | 9 pass (`growth gate`, `gates=119`, `lines=364`, `handoffs=8`, `provisional`, `CL-0002`, `measured-hardware`, `not evidence`, and the eight-id loop — all eight now ≥1); 1 fails on form only: `no shared object` is line-wrapped and reads 0 raw, present when whitespace is collapsed | ✓ PASS (record); ℹ️ one literal grep |
| The container leg | read of the retained `build/gate-linux.log` (not re-run) | `gate: PASS`, 1,512-byte line, exactly four documented token differences against the recorded line; unchanged file (mtime 06:11, pre-commit) | ✓ PASS (carried forward from the retained log) |

### Probe Execution

| Probe | Command | Result | Status |
|-------|---------|--------|--------|
| committed-direction shared-object probe | `git add -f probe.so probe.txt` → `files.sh` over `git ls-files` | Not re-run this session by instruction: `tools/check/steps/files.sh` (last touched `c44aa48`), `files.cmake` and `binary-ext.txt` are byte-unchanged since the run that produced `-- FAIL files-unlisted-binary: probe.so` with `probe.txt` silent and the tree clean. This session's gate keeps `binaries-listed=0` and `rights-pinned=4/4` green and the three `release-posture` rows registered. | ✓ PASS (carried forward, files unchanged) |
| release-time source-archive rehearsal | `sh tools/release-rehearse.sh --no-gate --release --source-route` | Not re-run this session by instruction; `tools/release-rehearse.sh` (`14f81ed`) and every record it writes are unchanged since the run that printed `rehearse: ok` with `124 of 124` compared and the mutation refused at exit 14. | ✓ PASS (carried forward, files unchanged) |

### Requirements Coverage

| Requirement | Source Plan | Description | Status | Evidence |
|-------------|-------------|-------------|--------|----------|
| PKG-06 | 16-01 … 16-05 (all five declare it) | A tracked shared object is refused by name, the installed package stays eleven files, and the release-decision line states that no shared object, host shim or content is shipped. | ✓ SATISFIED | Committed-direction probe reproduced (truth 4); `prefix-files=11` and the eleven-path inventory (truth 5); the line verbatim in `RELEASE-OPS.md` (i)/(i.1), `PROJECT.md:297`, `ROADMAP.md:284` and in substance at `docs/packaging.md:123`; `binaries-listed=0`; `so-member` and `host-so-absent` live. `.planning/REQUIREMENTS.md:112` reads `Complete`; no PKG-06 remains `Pending`; Phase 16 carries no other requirement, so none is orphaned. |

### Anti-Patterns Found

| File | Line | Pattern | Severity | Impact |
|------|------|---------|----------|--------|
| `.planning/phases/16-…/16-VALIDATION.md` | 93 | `Feedback latency within the 180 s budget (measured 66–147 s on a loaded host …)` — this session's single run took **188 s**, above both the quoted range and the 180 s bound | ℹ️ Info | Not a defect in the record: the same file's `Measured runtime` row states the bound is exceeded on a loaded host and quotes a range, not a promise, and the phase's exit criteria carry no latency requirement. The five-run range is a historical measurement; this session's 188 s is one more loaded-host sample. Worth widening the range or wording the tick at the milestone audit so the sign-off does not read as a guarantee. |
| `.planning/ROADMAP.md` | 414 | `**Plans**: TBD` | ℹ️ Info | Belongs to the **dropped** Phase 12 inside the archived v0.2.0 `<details>` block (`12 … 0/0 \| Dropped`). Confirmed pre-existing: the string occurs twice at `857f45d^` and once now; a dropped phase has no plans. The file was not modified since the previous verification (`9703594`), so under the re-verification evidence gate this is unevidenced new scope, reported not blocking. Worth a sweep at the milestone close. |
| `.planning/STATE.md`, `16-05-SUMMARY.md`, `16-VALIDATION.md` | closing line | `tracked=2787`, `hygiene-files=2787`, `files=2787` quoted; **2789** measured for all three at `HEAD` | ℹ️ Info | Three count tokens, all `git ls-files \| wc -l`-shaped; the delta is exactly the two planning records added after the measurement (`16-05-SUMMARY.md` in `438bffd`, `16-VERIFICATION.md` in `9703594`). No evidence token moved and the line is still 1,520 bytes; the quoted line is reproducible at its measurement commit, not at `HEAD`. Inherent to quoting a count that counts the records that quote it. |
| `16-05-PLAN.md` | 347-385 (verify block) | Five proxy assertions that fail on a correct record: `T-16-(0[1-9]\|1[0-9]\|SC)` cannot match `T-16-20`; `grep -E '✅ green' \| wc -l` counts the `*Status: …*` legend line; `grep -c 'satisfied by'` was case-sensitive against `**Satisfied by**` (the record now writes it lowercase, so this one passes); `grep -q 'no shared object'` cannot match a line-wrapped sentence; `sh tools/hygiene.sh --tree` prints its pass line rather than being silent | ℹ️ Info | Four of the five still fail against a record that is correct on substance; the fifth was cleared by the record rather than the plan. Record, not chase, at the next plan-writing pass: a proxy that a correct artifact cannot satisfy is a defect in the proxy. |
| `tools/check/steps/*.sh`, `tools/gate.sh`, `tools/release-rehearse.sh` | — | `XXXXXX` in `mktemp` templates | ℹ️ Info | False positive of the debt-marker scan; not a marker. |
| `16-05-SUMMARY.md` | 100-106 | reconciliation is prose, not the one-row-per-token table | ⚠️ Warning | Form only; the substance was re-derived in this verification. See `advisory`. |
| `16-05-SUMMARY.md` | — | the milestone's five additions are not enumerated and no handoff is named as the one to open first | ⚠️ Warning | Asked by Task 2's action text, not by its verify block or acceptance criteria; the phase truth is met without them. See `advisory`. |

### Human Verification Required

None. Every phase behavior has an automated check and each was exercised in this session or the previous one on
byte-unchanged files; the owner-only items (`H16-1`, `H16-2`, `H16-3`, `D10`, `D11`, and the earlier `H7-1`,
`H7-2`, `H8-3`) are ledger rows with procedures, are not evidence and are not criteria, so they are not
verification items. No record defect remains.

### Gaps Summary

**No gaps.** The phase's goal is achieved and unchanged across all three verification rounds: all five ROADMAP
success criteria hold, and PKG-06 is `Complete` on evidence re-derived rather than read — the owner route
genuinely runs from a freshly built source archive with the transcript's own container digest, the one-byte
mutation is refused at load while `exec:` stays `-`, the handoff rows are `evidence: none` and cited by nothing,
a committed shared object is refused by name with a witness pair, the package is eleven files, the
release-decision line reads as required, and the concurrency lever is a one-commit, cost-only change whose token
line was proved byte-identical on one tree by running the gate both ways. This round's single gate run
reproduced every evidence token, both step pass lines, the partition, the medians (366 / 34 / 120) and the single
`reprice` verdict.

**Both record gaps the previous report left are closed, and closed by fixing the records rather than the
assertions.** `16-VALIDATION.md`'s Sign-Off no longer ticks a `< 120s` threshold that its own measured row
exceeded; the bullet now names the 180 s budget and the measured 66–147 s range, and the Manual-Only sentence
reads `All five` over five rows. The Wave 0 clause is lowercase, so the plan's own case-sensitive proxy reads
10 of 10. `16-05-SUMMARY.md` now names all eight handoff ids with their dispositions, states `CL-0002` as the
only `established` claim, states that no claim reaches `measured-hardware`, carries the growth-gate statement
and the four tokens the phase owns, and says in as many words that a handoff row is not evidence. Nothing about
the phase's work changed: `git diff --name-status 9703594..HEAD` is three planning files, and every source file
the phase touched is byte-unchanged.

**What remains is form, and it belongs to the plan rather than the record.** Four of plan 16-05 Task 2's
assertions still fail against an artifact that is correct on substance: a threat-id regex that cannot match
`T-16-20`, a green-row count that includes the status legend, a literal `no shared object` that the SUMMARY's
line wrap defeats, and a `hygiene.sh --tree` that prints a pass line instead of staying silent. The SUMMARY's
reconciliation is prose where Task 1 asked for a table, and Task 2's action text asks for two sentences its own
acceptance criteria do not require. All are recorded as ℹ️ Info / `advisory` and none changes a token: this
session's closing line reads `steps=38/38`, `controls=404/404`, `faults=238/238`, `goldens=29`, `gates=119`,
`lines=364`, `handoffs=8`, `prefix-files=11`, `claims=29 provisional=28` — exactly where the five plans
predicted. The only movement since the close is the three `git ls-files` count tokens 2787 → 2789, from the
planning records themselves, with the line still 1,520 bytes. This phase is ready to proceed.

### Note for the milestone audit

**Is the phase SUMMARY's "what the milestone can and cannot claim" statement accurate against the tree?**
Yes, with one imprecision carried forward. Checked item by item this session: `data/claims.json` holds 29 claims
with 28 `provisional` and one `established` (`CL-0002`, `source-expectation`), no claim at `measured-hardware`,
and no claim whose artifact is the handoff ledger or a `D10`/`D11`/`H16-` id (`D10`/`D11` occur only inside
`note` prose); nothing is installed beyond the eleven paths and no shared object is in the release; `D11` is
`not-performed` and the route's `exec:` label is `-`; the eight remaining questions are the ledger's rows with
procedures and no evidence, all eight now named in the SUMMARY. The imprecision: the SUMMARY says "D10, 47 items
handed off" while `D10`'s ledger row is `not-performed` — the 47 items are inventoried in
`data/expected/d10-measurements.ids` (`d10-items=47` on the gate line) and offered, but the row's own
disposition is "needs hardware no tool on this host can supply", not `handed-off`. Worth a word at the close; it
does not change what the milestone can claim. The SUMMARY's nine-deviation count also matches
`.planning/WINDOWS.md` (rows 5-13, all waived).

**Two carry-forwards for the next milestone's records:** (1) the plan-proxy defects above — a verify block whose
assertions a correct artifact cannot satisfy teaches the next reader to discount the block; (2) the quoted
closing line's three `git ls-files` count tokens, which drift by construction whenever a planning record is
added. Neither is a defect in this phase's work.

---

_Verified: 2026-11-26T12:46:34Z_
_Verifier: Claude (gsd-verifier)_

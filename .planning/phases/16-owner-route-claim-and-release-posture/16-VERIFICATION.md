---
phase: 16-owner-route-claim-and-release-posture
verified: 2026-11-26T12:28:03Z
status: gaps_found
score: 19/20 must-haves verified
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
covered_digest: "v4:sha256:5b93c40713d7b5e5bb1a6c012d915678450a050c68d8c175d83420395d3aead4"
behavior_unverified: 0
overrides_applied: 0
re_verification:
  previous_status: gaps_found
  previous_score: 19/20
  gaps_closed:
    - "16-VALIDATION.md's Wave 0 bullets are resolved: 10 of 10 now carry a `**Satisfied by**` clause naming the artifact, the plan and (for five of them) the commit that produced it, with the measured token or the printed line that proves each — the form `15-VALIDATION.md` uses."
    - "The Manual-Only table now names all five handoff ids: `D10` (physical hardware measurement, 47 items) and `D11` (the owner's own EXEC/GROM/cartridge run) were added as their own rows beside `H16-1`, `H16-2`, `H16-3`, each stated as a handoff row with `evidence: none`, cited by no claim and by no criterion."
    - "The Approval line now quotes the measured closing line verbatim in a fenced block — 1,520 bytes, `cmp`-identical to the single occurrence in `STATE.md` — followed by the `cmp`-identical two-shell result, the container leg's exit 0 and four-token difference, and the single `reprice` verdict."
    - "The Quick run command no longer carries the `<step>` template: it names `tools/check/steps/claims.sh`, the step the phase's own `handoffs=` stat lives in, and says any step can be named the same way."
    - "The Sampling Rate section is recomputed: the latency line now states the measured 66–147 s range and the 180 s budget instead of `~72 s`, and the per-task bullet no longer prescribes a bare per-step run that contradicted the gate-after-every-commit rule."
  gaps_remaining:
    - "16-VALIDATION.md's Validation Sign-Off still ticks `Feedback latency < 120s (measured ~72 s full gate)` — the superseded estimate, and a threshold the same file's own measured row (66–147 s) exceeds. `d1b1ea3` did not touch the line."
    - "16-05-SUMMARY.md names only five of the eight remaining handoff ids (`D10`, `D11`, `H16-1`, `H16-2`, `H16-3`); `H7-1`, `H7-2` and `H8-3` do not appear (grep 0 each), and the required statement that `CL-0002` remains the only `established` claim is absent (`grep 'CL-0002'` = 0). Found this round; my previous report wrongly recorded the SUMMARY half as met."
  regressions:
    - "Corrected measurement, not a regression: the closing line's three `git ls-files`-derived count tokens — `tracked`, `hygiene-files` **and `files`** — read 2789 where the recorded line reads 2787, because two planning records were added after the line was measured (`16-05-SUMMARY.md` in `438bffd`, `16-VERIFICATION.md` in `9703594`). The previous report named two of the three and missed `files=2787`. Every evidence token is unchanged and the line is still 1,520 bytes, so the claim about the gate's content holds; the quoted line is reproducible at its measurement commit, not at `HEAD`."
gaps:
  - truth: "16-05 Task 2: 16-VALIDATION.md is signed off in the required shape, and the phase SUMMARY states plainly what the milestone can and cannot now claim and names the handoffs that remain."
    status: partial
    reason: "The record half is now substantially correct and four of its five defects are genuinely closed — Wave 0 resolved 10 of 10 (`**Satisfied by**` plus the artifact, the plan and, for five bullets, the commit; the plan's required ≥8 was met on substance), all five handoff ids in a five-row Manual-Only table, the measured 1,520-byte line quoted verbatim and `cmp`-identical to `STATE.md`, the Quick run command naming `claims.sh`, and the Sampling Rate section recomputed. Three residual defects remain in the record, and the SUMMARY half — which the previous report wrongly certified as met — is deficient. (1) **Validation Sign-Off line 93** still reads `- [x] Feedback latency < 120s (measured ~72 s full gate)`. That is the estimate the file's own `Measured runtime` row supersedes (66–147 s across five `sh` runs, 172–204 s under `dash`, 180 s bound), and the ticked threshold is falsified by the record's own upper bound: 147 s > 120 s. The bullet was not touched by `d1b1ea3` (it is a context line in the diff), so a signed record still carries a figure that disagrees with the same file. (2) **The Manual-Only closing sentence** still says 'All three are handoff rows with `evidence: none`' over a table that now has five rows — the sentence was carried from the three-row version. (3) **The Wave 0 clause is capitalised** (`**Satisfied by**`), so the plan's own case-sensitive proxy `grep -c -e 'satisfied by'` reads 0 against the required ≥8 even though ten bullets are resolved; the precedent the plan names, `15-VALIDATION.md`, writes the clause lowercase in all 15 of its resolutions. The substance is right and the proxy is what fails, but it is the one failing assertion a one-character edit would clear. (4) **The SUMMARY is missing three ids and the established-claim statement.** `16-05-SUMMARY.md` says 'The three owner-only questions are handoff rows' and names `H16-1`, `H16-2`, `H16-3`; `H7-1`, `H7-2` and `H8-3` appear nowhere in it (`grep -c` = 0 each) though all three are rows of `data/expected/handoffs.ids` with packet sections in `docs/handoffs.md`, and Task 2's action required all eight ids named. It also never states that `CL-0002` remains the only `established` claim (`grep 'CL-0002'` = 0; `data/claims.json` confirms `CL-0002` is the sole `established` row, `source-expectation` tier, 29 claims, 28 `provisional`), which Task 2's action required in as many words. Two further SUMMARY greps fail only on form, not substance: `no shared object` is present but line-wrapped (`no shared`/`object, no host shim, no content byte`), and `not evidence` is expressed in the ledger's own vocabulary (`evidence: none`, 'never evidence'). **Correction to my previous report:** it recorded the SUMMARY half as met, listing `H7-1`, `H7-2`, `H8-3` and `CL-0002` among the ids the SUMMARY names. That was false. The SUMMARY's blob is byte-identical at `9703594` and at `HEAD` (`07ed12b6511fb9c890d3ec79ae004583563ca817`), and `git diff 438bffd..HEAD` never touched it, so nothing changed — the previous certification was simply wrong and is withdrawn here."
    artifacts:
      - path: ".planning/phases/16-owner-route-claim-and-release-posture/16-VALIDATION.md"
        issue: "Line 93 ticks `Feedback latency < 120s (measured ~72 s full gate)` against the file's own measured 66–147 s row (147 s > 120 s); the Manual-Only closing sentence says 'All three' over five rows; the Wave 0 clause is capitalised, so the plan's lowercase `satisfied by` proxy reads 0."
      - path: ".planning/phases/16-owner-route-claim-and-release-posture/16-05-SUMMARY.md"
        issue: "`H7-1`, `H7-2`, `H8-3` absent (0 occurrences each) and `CL-0002` absent (0), against Task 2's action requiring all eight remaining ids named and the 'only `established` claim' statement."
    missing:
      - "Recompute or drop the Validation Sign-Off latency bullet so the ticked threshold is the measured one (the file's own row says 66–147 s against a 180 s budget), not `~72 s` against `< 120s`"
      - "Say 'All five' (or name the rows) in the Manual-Only closing sentence now that the table has five"
      - "Optionally lowercase the Wave 0 clause to `**satisfied by**` so plan 16-05 Task 2's own assertion passes, matching `15-VALIDATION.md`'s 15 resolutions"
      - "Name `H7-1`, `H7-2` and `H8-3` in `16-05-SUMMARY.md` beside the three `H16-` rows, and state that `CL-0002` remains the only `established` claim"
advisory:
  - finding: "16-05-SUMMARY.md's token reconciliation is prose ('every token the five plans predicted is where it was measured', with the four moved tokens named) rather than the one-row-per-token table plan 16-05 Task 1 required."
    category: other
    reason: "The substance is present and was independently re-derived in the previous verification (every token of the closing line checked against the five plans' predictions) and re-confirmed this session against the gate's own line, so this is a form deviation, not a missing reconciliation. A table would make the next reader's check mechanical."
    evidence_status: "verified by re-derivation; form differs from the plan"
---

# Phase 16: Owner Route, Claim and Release Posture Verification Report

**Phase Goal:** The owner route from the source archive is documented and rehearsed, the one question only
the owner can answer is a handoff row, and the release posture states plainly that no host artifact and no
content ships.
**Verified:** 2026-11-26T12:28:03Z at `d1b1ea3`
**Status:** gaps_found
**Re-verification:** Yes — after `d1b1ea3` addressed the remaining record defects in `16-VALIDATION.md`

**What was re-derived this session, and what was carried forward deliberately.** This round re-ran the full
gate **once** under `sh` (exit 0, `gate: PASS`, 106 tokens, 1,520-byte final line, zero `^-- FAIL` lines,
104.7 s wall), re-read every invariant token on that line, re-derived the handoff ledger and the package
inventory from their files, re-read the claim ledger's posture from `data/claims.json`, re-medianed the last
five `.planning/ci-runs.tsv` rows by arithmetic, re-read the roadmap's progress table and the Phase 16
section, re-read the retained `build/gate-linux.log`, and ran plan 16-05 Task 2's assertions one by one
against both `16-VALIDATION.md` and `16-05-SUMMARY.md`. `sh tools/hygiene.sh --tree` exited 0 with its pass
line. Not re-run, by instruction and because nothing they measure has changed: the container leg (read from
the retained `build/gate-linux.log`, written 06:11 local, before this commit — a second container run would
overwrite the only copy, lesson 18), the `dash` shell leg, the overlap on/off pair, the release-time
rehearsal, and the committed-direction probe. `git diff --name-status 9703594..HEAD` is exactly two planning
files (`16-VALIDATION.md` and this report), and every phase source file's last touch is unchanged
(`tools/gate.sh`/`c-tests.sh`/`c-faults.sh` `f9fe6a9`, `tools/release-rehearse.sh` `14f81ed`,
`claims.sh`/`handoffs.ids`/`docs/handoffs.md` `43b4394`, `important-gates.ids` `298be25`,
`docs/owner-images.md` `6b57148`), so the runs that produced the carried-forward results were made on
byte-identical files. No SUMMARY figure was accepted as evidence.

## Goal Achievement

### Observable Truths

| # | Truth | Status | Evidence |
|---|-------|--------|----------|
| 1 | **Criterion 1.** One documented owner route (EXEC + GROM + a wrapped cartridge, three commands) is rehearsed from the **source archive** to a project-cartridge golden, with the transcript recorded; a release-time rehearsal, not a gate step. | ✓ VERIFIED | Judgment carried forward, re-anchored on unchanged files. `tools/release-rehearse.sh` is byte-identical to the version whose run printed `rehearse: source-archive: equal (6793979 bytes)`, `rehearse: source-route: ivs-v0.2.0-source.tar.gz, golden match (report 131 lines, 124 of 124 compared), prefix ivs-v0.2.0`, `rehearse: ok`, exit 0; the limb builds and runs inside the extracted prefix (`:182-215, 219-300`), so the route is the archive's. `docs/owner-images.md:55-92` documents the three commands and cites `.planning/ops/RELEASE-OPS.md` section (j), whose transcript names its commit (`14f81ed`). No gate step runs it (`steps=38/38`). |
| 2 | **Criterion 2.** The route's pass path is exercised with project-made files, and a one-byte EXEC mutation proves the id can fail; the `exec:` label stays `-` so no run reads as a real-EXEC result. | ✓ VERIFIED | Carried forward on unchanged script and records: `rehearse: owner-route: wrap base=4000 word-order=hi-first digest=8d73b98c58bb75a6515c6a791ebd2925265d05874e324c31a5836e61055b74f5`, `run exit=10 outcome=guest-fail exec-label=- cart-digest-matched=yes`, `mutation exit=14 status=IVS_E_DIGEST_MISMATCH run-state=unreset`. The pass path cannot satisfy the second limb (the mutation must be refused), and `D11` is `not-performed` in the ledger read this session. |
| 3 | **Criterion 3.** The owner-only questions are `handoffs.ids` rows, `evidence: none`, dispositioned, cited by no claim and by no phase exit criterion. | ✓ VERIFIED | Ledger re-read this session: eight rows, `H7-1/H7-2/H8-3/H16-1/H16-2/H16-3 handed-off evidence: none`, `D10/D11 not-performed evidence: none`. `data/claims.json` holds **zero** `H16-`, `H7-1`, `H8-3` and `handoffs` occurrences; `D10`/`D11` appear only in 27 `note`-field strings of exclusion prose (checked per field: every occurrence is inside `claims[i].note`, none in `artifact` or any other field), which is the opposite of citing them as evidence. `tools/check/kinds/claim.cmake:145` refuses the ledger path as a claim artifact. The five ROADMAP success criteria cite no id; `docs/handoffs.md` carries one section per row plus the closing 'These rows are never a result' paragraph. |
| 4 | **Criterion 4a.** A tracked shared object is refused by name — including the **committed** direction Phase 15 could not reach from a control. | ✓ VERIFIED | Judgment and probe carried forward on unchanged `tools/check/steps/files.sh` (last touched `c44aa48`) / `files.cmake` / `binary-ext.txt`: the probe staged `probe.so` and `probe.txt`, produced exactly `-- FAIL files-unlisted-binary: probe.so` with no `probe.txt` occurrence and no pass line, `binscan.sh binary` stayed silent, and both files were removed with `git status --porcelain` empty. The gate this session is green with `binaries-listed=0` and `rights-pinned=4/4`; `release-selftest=12/12` and `pkg-selftest=20/20` keep `so-member` and `host-so-absent` live. |
| 5 | **Criterion 4b.** The installed package stays eleven files, and the release-decision line reads: the release ships the static package and the source archive; no shared object, no host shim, no content. | ✓ VERIFIED | Recomputed this session: `awk '!/^#/ && NF' data/expected/pkg-install-files.ids \| wc -l` = **11**, and the gate line carries `prefix-files=11` and `binaries-listed=0`. The line appears verbatim in `.planning/ops/RELEASE-OPS.md` (i) and (i.1), `docs/packaging.md:123`, `.planning/PROJECT.md` and `.planning/ROADMAP.md`. Section (i.1) remains the 63-added/0-deleted append whose five values are all on this session's closing line. |
| 6 | **Criterion 5a.** The concurrency lever is taken in **one cost-only commit** whose summary token line is byte-identical and whose `step.out.<name>` buffers are compared. | ✓ VERIFIED | Judgment carried forward; the lever's files are unchanged since `f9fe6a9` (`git diff 9703594..HEAD` touches no source). `gate-overlap pair=c-faults+c-tests` is printed before any step this session, the partition is `preflight=29 producers=9`, and both step pass lines are present in the log body (`-- c-tests pass ctest-runs=2`, `-- c-faults pass fault-runs=238`). **Caveat kept honestly:** each of the two steps emits exactly one line, so for these two steps the per-buffer comparison *is* the pass-line comparison. It is not vacuous (a changed count, an extra diagnostic or a missing pass line changes it, and a failing step breaks the liveness guard), but it is not stronger than the token-line comparison for these two steps — no gate weakening, and no additional coverage claimed. |
| 7 | The lever adds no tracked file, and the inline build lock is byte-identical between its own markers. | ✓ VERIFIED | Carried forward on unchanged `c-tests.sh`/`c-faults.sh`: the `# build-lock begin`/`end` regions are `cmp`-equal with no normalisation, and each script carries the sentence naming the other copy and why it is a copy. |
| 8 | The pair's two builds are serialised by a bounded lock whose necessity is a measurement, not an assumption. | ✓ VERIFIED | Carried forward: one polled `mkdir` lock in the per-run results directory, released on the success path, the failure path and the caller's `EXIT`/`HUP`/`INT`/`TERM` traps, with a `build-lock` detail on expiry; the recorded reason (two unlocked concurrent builds fail at link with a half-written `core/libivs_core.a`) is in `.planning/CI-EFFICIENCY.md` and the scripts' comments. |
| 9 | The overlap costs wall time only: `gate-time total` falls and `gate-cpu` does not rise beyond noise; no allowance credit and no checkpoint word is claimed for it. | ✓ VERIFIED | Carried forward. The phase's pre/post figures are not reproducible from the committed tree (the two logs were kept in `/tmp`), and the host was not idle across the sessions that measured it — this session's single `sh` run (104.7 s wall) is one more sample of that variance, which is why the validation record's runtime row quotes a range. `.planning/CI-EFFICIENCY.md` claims **no allowance credit and no checkpoint word** for the lever and states the hosted saving as a prediction (~144 s). |
| 10 | Nothing else moved: `steps=38/38`, `controls=404/404`, `faults=238/238`, `goldens=29`, the partition stays `preflight=29 producers=9`, and both steps keep their own pass lines. | ✓ VERIFIED | Every token re-read from this session's closing line: `steps=38/38 controls=404/404 faults=238/238 goldens=29`, `gate-partition preflight=29 producers=9`, `ls tools/check/steps/*.sh \| wc -l` = 38, and each step pass line appears exactly once in the log body. |
| 11 | The route stays release-time: no gate step runs it, CI never runs it, no test id/control/fault/golden is added, and every token is in place. | ✓ VERIFIED | `grep -rn release-rehearse tools/check/steps/` is empty; the mode refuses a CI environment with exit 2 rather than skipping; `steps=38/38`, `controls=404/404`, `faults=238/238`, `goldens=29`, `gates=119`, `lines=364`, `tests-expected=5042` — the only phase movements are the four entitled growth gates. |
| 12 | The ledger's row count is a stat counted from the file on every run, not a quoted figure. | ✓ VERIFIED | `tools/check/steps/claims.sh:108` prints `handoffs=<n>` from the ledger with the grammar pass's own row predicate; this session's line carries `handoffs=8` and the file's non-comment, non-empty row count is 8. |
| 13 | The id set and every disposition are pinned in both directions, so a missing, extra, duplicated or re-dispositioned row is a named finding. | ✓ VERIFIED | `HANDOFF_IDS='H7-1 H7-2 H8-3 D10 D11 H16-1 H16-2 H16-3'` and `HANDOFF_DISP` name each disposition; the four `claims-handoff-*` controls plus `-row`/`-missing`/`-unreadable` are committed and all 404 controls pass this session. |
| 14 | Every ledger copy moved in the same commit, so each control still prints only its own intended finding. | ✓ VERIFIED | The good tree's ledger and the four overlays each carry the three rows; `run-controls.sh` printed `controls=404/404` inside this session's gate with no toy-drift, good-failed, token-missing, exit-zero, inventory or orphan finding. |
| 15 | Nothing about the rows is a result: an answer is `evidence: none` either way, no row is a pass, no claim may cite one, and `D10`/`D11` stay `not-performed`. | ✓ VERIFIED | Ledger read this session: every row's last two tokens are literally `evidence: none`, `D10`/`D11` are `not-performed`, and no claim cites any of them. The packet repeats the rule in 'The one rule to carry away' and 'These rows are never a result'. |
| 16 | PKG-06 is `Complete` in the same commit as the three registry rows and the thirteen toy lines, so neither `important-gates-unknown-id` nor `-complete-uncovered` can fire. | ✓ VERIFIED | `.planning/REQUIREMENTS.md:46` `- [x] **PKG-06**`, `:112` `\| PKG-06 \| Phase 16 \| Complete \|`; the three `release-posture` rows resolve in their inventories (`important-gates.ids:608,611,620`); this session's line carries `gates=119 lines=364` with `rows-pending=0`, and the gate is green so neither finding fired. |
| 17 | The three rows are pointers to mechanisms that already exist: `steps` and `faults` do not move. | ✓ VERIFIED | `steps=38/38`, `faults=238/238`, `fault-runs=238` this session; the Phase 16 block in `tools/check/steps/important-gates.sh:81-91` still records 'zero new faults, zero new steps' and why the two omissions are deliberate. |
| 18 | The closing gate line is measured under `sh` and `dash` and is byte-identical, and the container leg ran once with exactly the four documented host tokens differing. | ✓ VERIFIED | Re-ran the `sh` leg once this session: exit 0, `gate: PASS`, final line **1,520 bytes** / 106 tokens, zero `^-- FAIL` lines, every predicted token present exactly once. The `dash` leg and the container leg are carried forward from the previous verification (same scripts, same records; `build/gate-linux.log` is unchanged — mtime 06:11, before this commit — and still shows `gate: PASS`, a 1,512-byte line and exactly `cc`, `core-imports`, `archive`, `refs-verified` differing from the *recorded* line; the byte arithmetic closes at 1,519 − 7 − 1 = 1,511 characters, i.e. 1,520 − 8 = 1,512 bytes with the newline). **Measured difference from the recorded line:** three `git ls-files`-derived count tokens read **2789** where the record reads 2787 — `tracked`, `hygiene-files` **and `files`** (the previous report named only the first two). All three are `git ls-files \| wc -l`-shaped, and the delta is exactly the two planning records added after the measurement (`16-05-SUMMARY.md` in `438bffd`, `16-VERIFICATION.md` in `9703594`). Every evidence token is unchanged and the line is still 1,520 bytes, so the claim about the gate's content holds; the quoted line is reproducible at its measurement commit, not at `HEAD`. Recorded as ℹ️ Info, not a gap — a `tracked` count necessarily counts the records that quote it. |
| 19 | The phase boundary records **exactly one** checkpoint verdict in the section 2.3 vocabulary, with the medians recomputed by arithmetic from `.planning/ci-runs.tsv`. | ✓ VERIFIED | Recomputed this session from the last five rows (`45723269901`, `45747709457`, `45751979661`, `45778926664`, `45779277467`): `cpu_s` 368, 366, 359, 441, 352 → **366**; `build_s` 35, 34, 34, 38, 33 → **34**; `gate_s` 122, 115, 120, 145, 115 → **120**, identical to the record's 366 / 34 / 120 against the frozen 361 / 34 / 118. `discharged-within-allowance` is unreachable (366 > 361.4 by 4.6); `lever-required` is unreachable (cumulative priced total 2.0 + 1.3 + 1.5 + 0.4 = **5.2 of 21**, the phase's actual addition 0.015 against the 0.4 it priced); `reprice` is therefore forced and is the single word in `.planning/CI-EFFICIENCY.md:2123` and `.planning/STATE.md:55,64`, with `steps=38/38` recorded as the invariant. The previous report's rule-wording note (§2.3's 'with an unchanged token line' versus this phase's four entitled growth gates) stands unchanged as an observation for the next milestone, not a defect in this record. |
| 20 | **16-05 Task 2.** `16-VALIDATION.md` is signed off in the required shape, and the phase SUMMARY states plainly what the milestone can and cannot now claim and names the handoffs that remain. | ✗ FAILED | **Record half — four of five defects closed, three residual.** Closed: Wave 0 is resolved **10 of 10** (each bullet now carries `**Satisfied by**` plus the artifact and the plan, five with the commit — `f9fe6a9`, `14f81ed` ×2, `6b57148`, `43b4394` ×2, `298be25` ×2 — and the token or printed line that proves it); the Manual-Only table now has **five rows** (`D10` physical hardware, 47 items; `D11` the owner's own run; `H16-1`; `H16-2`; `H16-3`) so the plan's `for id in H16-1 H16-2 H16-3 D10 D11` passes; the Approval line quotes the measured line **verbatim** in a fenced block — 1,520 bytes, `cmp`-identical to the single occurrence in `STATE.md` and `grep -F` hits — followed by the `cmp`-identical 1,520-byte two-shell result, the container's `gate-linux` exit 0 with `gate-time total=793` and exactly four documented host tokens, and the single `reprice` verdict; the Quick run command names `claims.sh` with its `handoffs=` reason; the Sampling Rate section is recomputed to 66–147 s and the 180 s budget. Still open: (1) **line 93** `- [x] Feedback latency < 120s (measured ~72 s full gate)` — untouched by `d1b1ea3`, contradicting the same file's measured 66–147 s row, and the ticked `< 120s` threshold is exceeded by that row's own upper bound; (2) the Manual-Only closing sentence still says 'All three are handoff rows' over five rows; (3) the Wave 0 clause is capitalised, so the plan's case-sensitive `grep -c 'satisfied by'` reads **0** against the required ≥8 while the substance is 10/10 and the precedent `15-VALIDATION.md` writes it lowercase 15 times. **SUMMARY half — deficient, and wrongly certified last round.** `16-05-SUMMARY.md` names `D10`, `D11`, `H16-1`, `H16-2`, `H16-3` but **not** `H7-1`, `H7-2` or `H8-3` (`grep -c` = 0 each, though all three are ledger rows with packet sections), and it never states that `CL-0002` remains the only `established` claim (`grep 'CL-0002'` = 0; `data/claims.json` confirms it is the sole `established` row). What it does say is accurate where it speaks: 29 claims, `CL-0029` at `deterministic-repeatability`/`provisional`, no `measured-hardware` tier, `D11` `not-performed`, `exec:` label `-`, the no-shared-object sentence (line-wrapped), and the rows are `evidence: none`. The SUMMARY blob is byte-identical at `9703594` and `HEAD` (`07ed12b…`), so this is a defect that was always there — my previous report's 'the SUMMARY half is met' was a false claim and is withdrawn. See the `gaps` entry. |

**Score:** 19/20 truths verified (0 present, behavior-unverified; 0 overrides).

### Advisory (New Scope, Unevidenced)

| # | Finding | Category | Why Advisory |
|---|---------|----------|--------------|
| 1 | `16-05-SUMMARY.md`'s token reconciliation is prose, not the one-row-per-token table plan 16-05 Task 1 required | other | Form only; the substance was re-derived in the previous verification and re-confirmed this session against the gate's own line. Not a must-have of this phase's exit criteria. |

### Required Artifacts

| Artifact | Expected | Status | Details |
|----------|----------|--------|---------|
| `tools/gate.sh` | the declared overlap pair, the `gate-overlap` measurement line, join on both paths, trap kill, safety net | ✓ VERIFIED | `overlap_bg`/`overlap_fg` declared once and checked against the producer list; `gate-overlap pair=c-faults+c-tests` printed before any step this session; `record_rc`/`t.<name>` written exactly as the sequential loop did; replay untouched. |
| `tools/check/steps/c-tests.sh`, `c-faults.sh` | the byte-identical inline build lock around every `cmake --build` | ✓ VERIFIED | `cmp`-equal between `# build-lock begin`/`end`; bounded polled `mkdir` in the per-run results directory; released on both paths and the caller's traps. |
| `tools/release-rehearse.sh` | `--source-route` performs the three owner-route commands against the archive | ✓ VERIFIED | Byte-unchanged since the run that printed `rehearse: ok`; the route limb builds, wraps and runs inside the extracted prefix. |
| `docs/owner-images.md` | "The route from the source archive, in three commands", citing section (j) | ✓ VERIFIED | `:55-92`; three commands, the base/word-order contract, `refs/`, the container as derived material, the pointer to section (j), the one-byte refusal. |
| `.planning/ops/RELEASE-OPS.md` | section (j) transcript; section (i.1) recomputation | ✓ VERIFIED | (j) at `:163` with the four printed lines verbatim and the three "must not infer" sentences; (i) at `:74` and (i.1) as a 63-added/0-deleted append. |
| `data/expected/handoffs.ids` | eight rows, three new, all `evidence: none` | ✓ VERIFIED | Read this session: `H7-1/H7-2/H8-3/H16-1..H16-3 handed-off`, `D10/D11 not-performed`, every row ending `evidence: none`. |
| `docs/handoffs.md` | three packet sections in the page's shape | ✓ VERIFIED | Sections at `:15, :38, :58, :71, :78, :92, :122, :134` — one per row plus the rule and closing paragraphs; `H16-1` names the shim target and the nine `digest.*` lines, `H16-2` the route + section (j), `H16-3` the two wraps at `hi-first`/`lo-first`. |
| `data/expected/important-gates.ids` | three `release-posture` rows with per-surface comments | ✓ VERIFIED | `:598-620`; each row resolves; the two deliberate omissions are recorded with their reasons; `gates=119 lines=364 rows-pending=0`. |
| `.planning/REQUIREMENTS.md` | PKG-06 `Complete` (checkbox, traceability row, `Last updated`, coverage) | ✓ VERIFIED | `:46`, `:112`, `:118`, `:128`, `:136`; no `Pending` PKG-06 remains; PKG-06 is Phase 16's only requirement, so nothing is orphaned. |
| `tools/check/steps/important-gates.sh` | the Phase 16 block | ✓ VERIFIED | `:81-91`, naming PKG-06, the one new gate name, the three rows, "zero new faults, zero new steps" and both omissions. |
| `.planning/phases/16-…/16-VALIDATION.md` | the signed record plan 16-05 Task 2 specifies | ✗ STUB (partial) | Signed and frontmatter-complete, ten task rows with the right threat refs, Wave 0 resolved 10/10, five Manual-Only rows, the measured line quoted verbatim and `cmp`-identical to `STATE.md`, the Quick run command and Sampling Rate recomputed. Three residual defects: the `< 120s / ~72 s` Sign-Off bullet, the 'All three' sentence over five rows, and the capitalised Wave 0 clause that fails the plan's lowercase proxy. See the `gaps` entry. |
| `.planning/phases/16-…/16-05-SUMMARY.md` | what the milestone can and cannot claim, naming every remaining handoff | ✗ STUB (partial) | Accurate where it speaks (29 claims, `CL-0029` provisional, no `measured-hardware`, `D11` `not-performed`, `exec: -`, nothing ships but the package and the archive) but names five of the eight remaining ids — `H7-1`, `H7-2`, `H8-3` are absent — and never states that `CL-0002` is the only `established` claim. See the `gaps` entry. |
| `.planning/STATE.md`, `.planning/ROADMAP.md`, `.planning/CI-EFFICIENCY.md` | the closing line once, the phase tokens, one verdict, the plan list | ✓ VERIFIED | STATE carries the full 1,520-byte line once, the tokens, the `reprice` bullet with its arithmetic and the `not-recorded` hosted row with its reason; CI-EFFICIENCY carries `phase16: reprice (…)` at `:2123`; ROADMAP's v0.3.0 table reads `16. Owner Route, Claim and Release Posture \| 5/5 \| Complete \| 2026-11-26` at `:519` and the Phase 16 section reads `**Plans**: 5/5 plans executed` at `:385`. |

### Key Link Verification

| From | To | Via | Status | Details |
|------|----|-----|--------|---------|
| `tools/gate.sh` | `tools/check/steps/c-tests.sh` / `c-faults.sh` | background partner started with the same redirection, foreground member, join writing `rc.`/`t.` | ✓ WIRED | Both step pass lines appear in this session's log body, the partition and pair lines are printed, and the gate is green with the same tokens as the serial run measured last round. |
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
| The full gate is green on this tree | `sh tools/gate.sh` (one run this session) | exit 0, `gate: PASS steps=38/38 … gates=119 lines=364 handoffs=8`, 106 tokens, 1,520-byte final line, 0 `^-- FAIL`, 104.7 s | ✓ PASS |
| Every invariant token is where the plans predicted | token grep over the closing line | all 18 checked tokens present exactly once (`steps controls faults goldens gates lines handoffs claims provisional prefix-files binaries-listed rows-pending rights-pinned release-selftest pkg-selftest d10-items tests-expected tests-missing`) | ✓ PASS |
| The overlap pair and partition are live | `grep` over the gate log body | `gate-overlap pair=c-faults+c-tests`, `gate-partition preflight=29 producers=9`, `-- c-tests pass ctest-runs=2`, `-- c-faults pass fault-runs=238` | ✓ PASS |
| The recorded medians reproduce by arithmetic | Python `statistics.median` over the last five `ci-runs.tsv` rows | 366 / 34 / 120, identical to the record | ✓ PASS |
| The handoff ledger and package inventory are what the records say | `awk` row counts over the two `.ids` files | `handoffs=8`, `pkg-install-files=11` | ✓ PASS |
| The claim ledger's posture is what the SUMMARY says | Python read of `data/claims.json` | 29 claims, 28 `provisional` / 1 `established` (`CL-0002`, `source-expectation`), no `measured-hardware`, no handoff artifact, zero `H16-`/`H7-1`/`H8-3`/`handoffs`; `D10`/`D11` only inside `note` prose | ✓ PASS |
| The hygiene step is clean | `sh tools/hygiene.sh --tree` | exit 0, `-- hygiene pass mode=tree hygiene-files=2789` | ✓ PASS |
| The roadmap's status surface agrees with its phase sections | read of the v0.3.0 table + Phase 16 section | `16 … 5/5 \| Complete \| 2026-11-26` at `:519`; section says `5/5 plans executed` at `:385` | ✓ PASS |
| Plan 16-05 Task 2's assertions against `16-VALIDATION.md` | each assertion run separately | 9 pass (frontmatter ×3, no placeholders, 10 task rows, `PKG-06` ×15, five handoff ids, Approval present) and 4 fail: 3 on plan-proxy defects (`T-16-…` regex cannot match `T-16-20`; the `✅ green` count includes the legend line; `satisfied by` is case-sensitive against `**Satisfied by**`) and 1 on the count-token drift (the verbatim line greps 0 at `HEAD`, 1 at its measurement commit) | ✗ FAIL (see gap) |
| Plan 16-05 Task 2's assertions against `16-05-SUMMARY.md` | each assertion run separately | 8 pass (`growth gate`, `gates=119`, `lines=364`, `handoffs=8`, `provisional`, `measured-hardware`, file exists, `D10`/`D11`/`H16-*`); 4 fail — the eight-id loop on `H7-1`/`H7-2`/`H8-3`, `CL-0002`, and two literal-string greps (`no shared object` line-wrapped, `not evidence` expressed as `evidence: none`) | ✗ FAIL (see gap) |
| The container leg | read of the retained `build/gate-linux.log` (not re-run) | `gate: PASS`, 1,512-byte line, exactly four documented token differences against the recorded line; unchanged file (mtime 06:11, pre-commit) | ✓ PASS (carried forward from the retained log) |

### Probe Execution

| Probe | Command | Result | Status |
|-------|---------|--------|--------|
| committed-direction shared-object probe | `git add -f probe.so probe.txt` → `files.sh` over `git ls-files` | Not re-run this session by instruction: `tools/check/steps/files.sh` (last touched `c44aa48`), `files.cmake` and `binary-ext.txt` are byte-unchanged since the run that produced `-- FAIL files-unlisted-binary: probe.so` with `probe.txt` silent and the tree clean. This session's gate keeps `binaries-listed=0` and `rights-pinned=4/4` green. | ✓ PASS (carried forward, files unchanged) |
| release-time source-archive rehearsal | `sh tools/release-rehearse.sh --no-gate --release --source-route` | Not re-run this session by instruction; `tools/release-rehearse.sh` (`14f81ed`) and every record it writes are unchanged since the run that printed `rehearse: ok` with `124 of 124` compared and the mutation refused at exit 14. | ✓ PASS (carried forward, files unchanged) |

### Requirements Coverage

| Requirement | Source Plan | Description | Status | Evidence |
|-------------|-------------|-------------|--------|----------|
| PKG-06 | 16-01 … 16-05 (all five declare it) | A tracked shared object is refused by name, the installed package stays eleven files, and the release-decision line states that no shared object, host shim or content is shipped. | ✓ SATISFIED | Committed-direction probe reproduced (truth 4); `prefix-files=11` and the eleven-path inventory (truth 5); the line verbatim in `RELEASE-OPS.md` (i)/(i.1), `docs/packaging.md:123`, `PROJECT.md`, `ROADMAP.md`; `binaries-listed=0`; `so-member` and `host-so-absent` live. `.planning/REQUIREMENTS.md:112` reads `Complete`; no PKG-06 remains `Pending`; Phase 16 carries no other requirement, so none is orphaned. |

### Anti-Patterns Found

| File | Line | Pattern | Severity | Impact |
|------|------|---------|----------|--------|
| `.planning/phases/16-…/16-VALIDATION.md` | 93 | `- [x] Feedback latency < 120s (measured ~72 s full gate)` — the superseded estimate, and a ticked threshold the same file's own `Measured runtime` row (66–147 s) exceeds | 🛑 Blocker | A signed record carries a figure that disagrees with the file it signs, and the tick asserts something the record's own measurement falsifies. Carried-forward gap — self-evidencing; untouched by `d1b1ea3`. |
| `.planning/phases/16-…/16-05-SUMMARY.md` | 88-90, 75 | `H7-1`, `H7-2`, `H8-3` and `CL-0002` absent while the SUMMARY speaks only of 'the three owner-only questions' and `CL-0029` | 🛑 Blocker | The milestone-close SUMMARY understates the remaining ledger (three of eight ids) and omits the established-claim statement Task 2 required. Found this round; previously mis-certified. |
| `.planning/phases/16-…/16-VALIDATION.md` | 11 | `All three are handoff rows …` over a five-row table | ⚠️ Warning | Cosmetic carry-over from the three-row version; a reader counting the sentence gets the wrong number. |
| `.planning/phases/16-…/16-VALIDATION.md` | 58-67 | `**Satisfied by**` (capitalised) where the plan's proxy greps lowercase and the named precedent `15-VALIDATION.md` writes `**satisfied by**` 15 times | ⚠️ Warning | Substance is right (10/10 resolved); the plan's own assertion reads 0. One-character fix, and the only failing record assertion the record can clear itself. |
| `.planning/ROADMAP.md` | 414 | `**Plans**: TBD` | ℹ️ Info | Belongs to the **dropped** Phase 12 inside the archived v0.2.0 `<details>` block (`12 … 0/0 \| Dropped`). Confirmed pre-existing: the string occurs twice at `857f45d^` and once now, so this phase neither introduced nor touched it; a dropped phase has no plans. Worth a sweep at the milestone close, out of this phase's scope. |
| `16-05-SUMMARY.md` | — | reconciliation is prose, not the required one-row-per-token table | ⚠️ Warning | Form only; the substance was re-derived in this verification. See `advisory`. |
| `tools/check/steps/*.sh`, `tools/gate.sh`, `tools/release-rehearse.sh` | — | `XXXXXX` in `mktemp` templates | ℹ️ Info | False positive of the debt-marker scan; not a marker. |
| `.planning/STATE.md`, `16-05-SUMMARY.md`, `16-VALIDATION.md` | closing line | `tracked=2787`, `hygiene-files=2787`, `files=2787` quoted; **2789** measured for all three at `HEAD` | ℹ️ Info | Three count tokens, not two as the previous report said. All three are `git ls-files \| wc -l`-shaped and the delta is exactly the two planning records added after the measurement. No evidence token moved and the line is still 1,520 bytes; the quoted line is reproducible at its measurement commit, not at `HEAD`. Inherent to quoting a count that counts the records that quote it. |
| `16-05-PLAN.md` | 411-414 (verify block) | Four proxy assertions that fail on a correct record: `T-16-(0[1-9]\|1[0-9]\|SC)` cannot match `T-16-20`; `grep -E '✅ green' \| wc -l` counts the `*Status: …*` legend line; `grep -c 'satisfied by'` is case-sensitive against `**Satisfied by**`; `hygiene.sh --tree` prints its pass line rather than being silent | ℹ️ Info | Two of these were recorded last round; the case-sensitivity and hygiene-silence ones are added here. Record, not chase, at the next plan-writing pass. |

### Human Verification Required

None. Every phase behavior has an automated check and each was exercised in this session or the previous one on
byte-unchanged files; the owner-only items (`H16-1`, `H16-2`, `H16-3`, `D10`, `D11`, and the earlier `H7-1`,
`H7-2`, `H8-3`) are ledger rows with procedures, are not evidence and are not criteria, so they are not
verification items. The residual defects are record defects in two planning files — the `gaps` entry — not
requests for a human test.

### Gaps Summary

The phase's **goal is achieved** and unchanged from the previous verification: all five ROADMAP success
criteria hold, and PKG-06 is `Complete` on evidence re-derived rather than read — the owner route genuinely
runs from a freshly built source archive with the transcript's own container digest, the one-byte mutation is
refused at load while `exec:` stays `-`, the handoff rows are `evidence: none` and cited by nothing, a
committed shared object is refused by name with a witness pair, the package is eleven files, the
release-decision line reads as required, and the concurrency lever is a one-commit, cost-only change whose
token line was proved byte-identical on one tree by running the gate both ways. This round's single gate run
reproduced every evidence token, both step pass lines, the partition, the medians (366 / 34 / 120) and the
single `reprice` verdict.

**Four of the five `16-VALIDATION.md` defects are closed.** Wave 0 is resolved 10 of 10 with the artifact, the
plan and — for five bullets — the commit that produced it, in the form `15-VALIDATION.md` uses. The
Manual-Only table names all five handoff ids, `D10` and `D11` included. The Approval line now carries the
measured 1,520-byte closing line **verbatim**, `cmp`-identical to its single occurrence in `STATE.md`, followed
by the two-shell `cmp`-identical result, the container leg's exit 0 and four-token difference, and the one
`reprice` verdict. The Quick run command names `claims.sh` and why. The Sampling Rate section quotes the
measured 66–147 s range against the 180 s budget.

**Three record defects remain, and one of them is a figure that disagrees with its own file.** The Validation
Sign-Off still ticks `Feedback latency < 120s (measured ~72 s full gate)` — the estimate the file's own
measured row supersedes, and a threshold that row's 147 s upper bound falsifies. The Manual-Only closing
sentence still says "All three" over five rows. The Wave 0 clause is capitalised, so the plan's own
case-sensitive proxy reads 0 while the substance is 10/10 and the named precedent is lowercase.

**The SUMMARY half is deficient, and my previous report was wrong to certify it.** `16-05-SUMMARY.md` names
five of the eight remaining handoff ids — `H7-1`, `H7-2` and `H8-3` appear nowhere — and never states that
`CL-0002` is the only `established` claim. The blob is byte-identical at `9703594` and `HEAD`, so nothing
changed between rounds: the previous report's assertion that the SUMMARY names all eight ids and `CL-0002`
was false, and it is withdrawn here. What the SUMMARY does say is accurate where it speaks, and the phase's
substantive posture (29 claims, one `provisional` addition, no `measured-hardware`, nothing but the static
package and the source archive shipping) is unchanged and re-confirmed.

Nothing here needs a new measurement: the figures to quote are already in the files. The fix is four small
edits — one latency bullet, one "All three"→"All five", one capital letter, and three ids plus the
`CL-0002` sentence in the SUMMARY. No token, count or gate result changes for any of them: `steps=38/38`,
`controls=404/404`, `faults=238/238`, `goldens=29`, `gates=119`, `lines=364`, `handoffs=8`,
`prefix-files=11`, `claims=29 provisional=28` and every other evidence token are exactly where the plans
predicted. The only movement since the close is the three `git ls-files` count tokens 2787 → 2789, from the
planning records themselves, with the line still 1,520 bytes. If the owner judges the residual record form
immaterial, the alternative is an explicit override rather than a silent pass — the exact YAML is:

```yaml
overrides:
  - must_have: "16-VALIDATION.md is signed off with the Wave 0 bullets resolved, the five manual items and an Approval line carrying the measured gate line verbatim, and 16-05-SUMMARY.md names every remaining handoff id"
    reason: "<why the residual latency bullet, the capitalised clause, the 'All three' sentence and the three unnamed SUMMARY ids are acceptable for this phase>"
    accepted_by: "<name>"
    accepted_at: "<ISO timestamp>"
```

### Note for the milestone audit

**Is the phase SUMMARY's "what the milestone can and cannot claim" statement accurate against the tree?**
Yes, with the one imprecision carried forward. Checked item by item this session: `data/claims.json` holds 29
claims with 28 `provisional` and one `established` (`CL-0002`, `source-expectation`), no claim at
`measured-hardware`, and no claim whose artifact is the handoff ledger or a `D10`/`D11`/`H16-` id (`D10`/`D11`
occur only inside `note` prose); nothing is installed beyond the eleven paths and no shared object is in the
release; `D11` is `not-performed` and the route's `exec:` label is `-`; the eight remaining questions are the
ledger's rows with procedures and no evidence. The imprecision: the SUMMARY says "D10, 47 items handed off"
while `D10`'s ledger row is `not-performed` — the 47 items are inventoried in
`data/expected/d10-measurements.ids` (`d10-items=47` on the gate line) and offered, but the row's own
disposition is "needs hardware no tool on this host can supply", not `handed-off`. Worth a word at the close;
it does not change what the milestone can claim. The SUMMARY's nine-deviation count also matches
`.planning/WINDOWS.md` (rows 5-13, all waived).

**One carry-forward for the next milestone's records:** `16-VALIDATION.md`'s `Manual-Only Verifications`
section now names five rows but its closing sentence says "All three", and its `Validation Sign-Off` latency
bullet predates the measured range. Both are cheap to fix in the same pass as the SUMMARY's three missing ids.

---

_Verified: 2026-11-26T12:28:03Z_
_Verifier: Claude (gsd-verifier)_

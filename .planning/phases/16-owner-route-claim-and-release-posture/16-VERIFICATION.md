---
phase: 16-owner-route-claim-and-release-posture
verified: 2026-11-26T12:09:18Z
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
covered_digest: "v4:sha256:e87271d73e9e9cba7a1e45711d2b5d74b57f594ab63cea04ef5a1a2b7bb153fb"
behavior_unverified: 0
overrides_applied: 0
re_verification:
  previous_status: gaps_found
  previous_score: 19/20
  gaps_closed:
    - "The roadmap's own status surface now agrees with its phase sections: the v0.3.0 progress table row reads `16. Owner Route, Claim and Release Posture | 5/5 | Complete | 2026-11-26`, and the narrative below it says all four phases are executed and verified with Phase 15 delivered inside its priced line rather than 2 of 3 plans executed and droppable."
  gaps_remaining:
    - "16-VALIDATION.md is still not the record plan 16-05 Task 2 specifies: the ten Threat Ref cells and the measured runtime row are now correct, but the Wave 0 bullets are unresolved (0 of 10 in the required form against a required 8), `D10` is absent from the Manual-Only table (3 rows, not the required 4), and the Approval line carries a 14-token reordered subset of the closing line rather than the line verbatim (full-line grep: 0 hits). The Sampling Rate section and the Quick run command are also still the seeded template."
  regressions:
    - "The closing gate line's `tracked` and `hygiene-files` tokens have moved 2787 → 2789 because two planning records were added after the line was measured (`16-05-SUMMARY.md` in `438bffd`, `16-VERIFICATION.md` in `9703594`). Every evidence token is unchanged and the line is still 1,520 bytes, so the substance of the recorded line holds; the drift is structural (a `tracked` count necessarily counts the records that quote it), but it means a fresh gate at `HEAD` is no longer byte-identical to the line quoted in `STATE.md` and `16-05-SUMMARY.md`."
gaps:
  - truth: "16-VALIDATION.md is signed off as plan 16-05 Task 2 requires — ten task rows carrying the threat ids each task's own plan assigns, the Wave 0 bullets resolved to their artifacts, the genuinely manual items in the Manual-Only table, the measured runtime, and an Approval line carrying the measured closing gate line verbatim."
    status: failed
    reason: "Two of the five defects this report carried are genuinely closed. (b) Threat Ref: the ten cells now name T-16-01, T-16-02, T-16-05, T-16-06, T-16-10, T-16-11, T-16-14, T-16-15, T-16-19, T-16-20, and each one is the register id of the plan that owns the task (`16-01-PLAN.md:432-435`, `16-02:409-413`, `16-03:365-368`, `16-04:426-430`, `16-05:418-422`) — 10 of 10 correct, previously 8 of 10 wrong. (e) Runtime: the row is now a measured range (`66–147 s` across five `sh` runs, `172–204 s` under `dash`, the lever's own `79 → 72` on a quiet tree, bound 180 s) instead of the seeded `~72 s / 66 s` estimate. Three defects remain, and two of them are the exact items this report listed under `missing` last time. (c) Wave 0: all ten bullets are still the seeded one-line form (`- [x] \\`tools/gate.sh\\` — the lever's overlap and the inline build lock (plan 16-01)`); `grep -c 'satisfied by'` is **0** against the plan's required ≥8, and Phase 15's `15-VALIDATION.md` — the precedent the plan names — carries **15** such resolutions each with a measured token. No Wave 0 bullet names the figure that proves it. (a) Manual-Only: three rows (H16-1, H16-2, H16-3), not the four the plan requires (`H16-1`, `H16-2`, `H16-3` **and the D10/D11 pair**); `D10` appears **nowhere** in the file (`grep -c 'D10'` = 0) and `D11` appears only as a parenthetical inside the H16-2 row, so the plan's own assertion `for id in H16-1 H16-2 H16-3 D10 D11; do grep -q -e \"$id\" \"$f\"; done` fails on `D10`. The sentence that these are ledger rows with `evidence: none`, cited by no claim and no criterion, is present and correct — that half of (a) is met. (d) Approval: the line is present but is a 14-token, reordered, three-line-wrapped subset (`steps controls faults goldens gates lines handoffs claims provisional pkg-selftest release-selftest tests-expected tests-executed tests-skipped tests-missing prefix-files`); the real line orders `handoffs` after `provisional` and `goldens/gates/lines` after `ci-summary-selftest`, and carries ~110 tokens including `api-calls`, `pinned-defines`, `cc`, `fault-runs`, `core-imports`, `tracked` and every other stat. `grep -F` for the measured 1,520-byte line in the file returns **0**, exactly as last time; the text instead says the full line is 'quoted in full in `STATE.md` and in `16-05-SUMMARY.md`', which is a pointer, not the verbatim quote the plan's `<automated>` block greps for. Additionally, two sections this report folded into 'seeded template' were not touched and still contradict the plan's action: the **Quick run command** row is still the per-step `<step>` incantation the plan explicitly told the author not to invent ('say that the quick run is `sh tools/gate.sh`'), and the **Sampling Rate** section still says 'run the single affected step directly' / '`sh tools/check/run-controls.sh`' / 'Max feedback latency: ~72 s', where the plan asked for the gate after every task commit, the gate's final line against the plan's prediction after every wave, gate + hygiene + `git ls-files` before verify-work, and the measured wall figure — the same `~72 s` estimate the file's own runtime row now supersedes. The SUMMARY half of the truth is still met (see truth 20's note). Phase 15's `15-VALIDATION.md` remains the in-repo precedent for the required shape."
    artifacts:
      - path: ".planning/phases/16-owner-route-claim-and-release-posture/16-VALIDATION.md"
        issue: "Wave 0 bullets unresolved (0 'satisfied by' vs required ≥8); D10 absent and D11 only parenthetical in a 3-row Manual-Only table (required 4 rows incl. the D10/D11 pair); Approval line is a reordered 14-token subset, not the measured line (full-line grep 0); Quick run command and Sampling Rate still the seeded template with the ~72 s estimate."
    missing:
      - "Each of the ten Wave 0 bullets resolved as `— satisfied by <path>` plus the measured token that proves it (the form `15-VALIDATION.md` uses 15 times)"
      - "A fourth Manual-Only row for the D10/D11 pair with its reason (physical hardware) and its procedure's location, so `D10` is named in the file"
      - "The measured 1,520-byte closing line quoted verbatim on one line in the Approval line (`grep -F` must hit)"
      - "The Quick run command row saying plainly that this phase's quick run is `sh tools/gate.sh`, and the Sampling Rate bullets and max-feedback-latency figure recomputed to the measured wall time"
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
**Verified:** 2026-11-26T12:09:18Z at `9703594`
**Status:** gaps_found
**Re-verification:** Yes — after the two record gaps this report raised were addressed in `9703594`

**What was re-derived this session, and what was carried forward deliberately.** This round measured: the full
gate once under `sh` (exit 0, `gate: PASS`, 1,520-byte final line, zero `^-- FAIL` lines), every invariant
token on that line, the handoff ledger and the package inventory read from their files, the claim ledger's
posture read from `data/claims.json`, the last five `.planning/ci-runs.tsv` rows re-medianed by arithmetic,
the roadmap's progress table and closing narrative, and plan 16-05 Task 2's own record assertions run one by
one against `16-VALIDATION.md`. Not re-run, by instruction and because nothing they measure has changed:
the container leg (read from the retained `build/gate-linux.log`; a second container run would overwrite the
only copy — lesson 18), the `dash` shell leg, the overlap on/off pair, the release-time rehearsal, and the
committed-direction probe. `git diff --name-status 438bffd..HEAD` is exactly three planning files
(`ROADMAP.md`, `16-VALIDATION.md` modified, `16-VERIFICATION.md` added), so `tools/gate.sh`,
`tools/release-rehearse.sh`, `tools/check/steps/*` and every registry are byte-unchanged since the runs that
produced those results. No SUMMARY figure was accepted as evidence.

## Goal Achievement

### Observable Truths

| # | Truth | Status | Evidence |
|---|-------|--------|----------|
| 1 | **Criterion 1.** One documented owner route (EXEC + GROM + a wrapped cartridge, three commands) is rehearsed from the **source archive** to a project-cartridge golden, with the transcript recorded; a release-time rehearsal, not a gate step. | ✓ VERIFIED | Judgment carried forward, re-anchored on unchanged files. `tools/release-rehearse.sh` is byte-identical to the version whose run printed `rehearse: source-archive: equal (6793979 bytes)`, `rehearse: source-route: ivs-v0.2.0-source.tar.gz, golden match (report 131 lines, 124 of 124 compared), prefix ivs-v0.2.0`, `rehearse: ok`, exit 0; the limb builds and runs inside the extracted prefix (`:182-215, 219-300`), so the route is the archive's. `docs/owner-images.md:55-92` documents the three commands and cites `.planning/ops/RELEASE-OPS.md` section (j), whose transcript names its commit (`14f81ed`). No gate step runs it (`steps=38/38`). |
| 2 | **Criterion 2.** The route's pass path is exercised with project-made files, and a one-byte EXEC mutation proves the id can fail; the `exec:` label stays `-` so no run reads as a real-EXEC result. | ✓ VERIFIED | Carried forward on unchanged script and records: `rehearse: owner-route: wrap base=4000 word-order=hi-first digest=8d73b98c58bb75a6515c6a791ebd2925265d05874e324c31a5836e61055b74f5`, `run exit=10 outcome=guest-fail exec-label=- cart-digest-matched=yes`, `mutation exit=14 status=IVS_E_DIGEST_MISMATCH run-state=unreset`. The pass path cannot satisfy the second limb (the mutation must be refused), and D11 stays `not-performed` in the ledger read this session. |
| 3 | **Criterion 3.** The owner-only questions are `handoffs.ids` rows, `evidence: none`, dispositioned, cited by no claim and by no phase exit criterion. | ✓ VERIFIED | Ledger read this session: eight rows, `H7-1/H7-2/H8-3/H16-1/H16-2/H16-3 handed-off evidence: none`, `D10/D11 not-performed evidence: none`. `data/claims.json` holds **zero** `H16-` and **zero** `handoffs` occurrences, and no claim's `artifact` field names the ledger or any `D10`/`D11`/`H16-` id (checked over all 29 claims); `D10`/`D11` appear only in exclusion prose ('D10 and D11 are unperformed, so nothing here is measured on hardware'), which is the opposite of citing them as evidence. `tools/check/kinds/claim.cmake:145` refuses the ledger path as a claim artifact. The five ROADMAP success criteria cite no id; `docs/handoffs.md` carries one section per row plus the closing 'These rows are never a result' paragraph. |
| 4 | **Criterion 4a.** A tracked shared object is refused by name — including the **committed** direction Phase 15 could not reach from a control. | ✓ VERIFIED | Judgment and probe carried forward on unchanged `tools/check/steps/files.sh` / `files.cmake` / `binary-ext.txt`: the probe staged `probe.so` and `probe.txt`, produced exactly `-- FAIL files-unlisted-binary: probe.so` with no `probe.txt` occurrence and no pass line, `binscan.sh binary` stayed silent, and both files were removed with `git status --porcelain` empty. The gate this session is green with `binaries-listed=0` and `rights-pinned=4/4`; `release-selftest=12/12` and `pkg-selftest=20/20` keep `so-member` and `host-so-absent` live. |
| 5 | **Criterion 4b.** The installed package stays eleven files, and the release-decision line reads: the release ships the static package and the source archive; no shared object, no host shim, no content. | ✓ VERIFIED | Recomputed this session: `awk '!/^#/ && NF' data/expected/pkg-install-files.ids \| wc -l` = **11**, and the gate line carries `prefix-files=11` and `binaries-listed=0`. The line appears verbatim in `.planning/ops/RELEASE-OPS.md` (i) and (i.1), `docs/packaging.md:123`, `.planning/PROJECT.md` and `.planning/ROADMAP.md`. Section (i.1) remains the 63-added/0-deleted append whose five values are all on this session's closing line. |
| 6 | **Criterion 5a.** The concurrency lever is taken in **one cost-only commit** whose summary token line is byte-identical and whose `step.out.<name>` buffers are compared. | ✓ VERIFIED | Judgment carried forward; the lever's files are unchanged since `f9fe6a9` (`git diff 438bffd..HEAD` touches no source). `gate-overlap pair=c-faults+c-tests` is printed before any step this session, the partition is `preflight=29 producers=9`, and both step pass lines are present in the log body (`-- c-tests pass ctest-runs=2`, `-- c-faults pass fault-runs=238`). **Caveat kept honestly:** each of the two steps emits exactly one line, so for these two steps the per-buffer comparison *is* the pass-line comparison. It is not vacuous (a changed count, an extra diagnostic or a missing pass line changes it, and a failing step breaks the liveness guard), but it is not stronger than the token-line comparison for these two steps — no gate weakening, and no additional coverage claimed. |
| 7 | The lever adds no tracked file, and the inline build lock is byte-identical between its own markers. | ✓ VERIFIED | Carried forward on unchanged `c-tests.sh`/`c-faults.sh`: the `# build-lock begin`/`end` regions are `cmp`-equal with no normalisation, and each script carries the sentence naming the other copy and why it is a copy. |
| 8 | The pair's two builds are serialised by a bounded lock whose necessity is a measurement, not an assumption. | ✓ VERIFIED | Carried forward: one polled `mkdir` lock in the per-run results directory, released on the success path, the failure path and the caller's `EXIT`/`HUP`/`INT`/`TERM` traps, with a `build-lock` detail on expiry; the recorded reason (two unlocked concurrent builds fail at link with a half-written `core/libivs_core.a`) is in `.planning/CI-EFFICIENCY.md` and the scripts' comments. |
| 9 | The overlap costs wall time only: `gate-time total` falls and `gate-cpu` does not rise beyond noise; no allowance credit and no checkpoint word is claimed for it. | ✓ VERIFIED | Carried forward. The phase's pre/post figures are not reproducible from the committed tree (the two logs were kept in `/tmp`), and the host was not idle across the sessions that measured it — this session's single `sh` run is one more sample of that variance, which is why the validation record's runtime row now quotes a range. `.planning/CI-EFFICIENCY.md` claims **no allowance credit and no checkpoint word** for the lever and states the hosted saving as a prediction (~144 s). |
| 10 | Nothing else moved: `steps=38/38`, `controls=404/404`, `faults=238/238`, `goldens=29`, the partition stays `preflight=29 producers=9`, and both steps keep their own pass lines. | ✓ VERIFIED | Every token re-read from this session's closing line: `steps=38/38 controls=404/404 faults=238/238 goldens=29`, `gate-partition preflight=29 producers=9`, `ls tools/check/steps/*.sh \| wc -l` = 38, and each step pass line appears exactly once in the log body. |
| 11 | The route stays release-time: no gate step runs it, CI never runs it, no test id/control/fault/golden is added, and every token is in place. | ✓ VERIFIED | `grep -rn release-rehearse tools/check/steps/` is empty; the mode refuses a CI environment with exit 2 rather than skipping; `steps=38/38`, `controls=404/404`, `faults=238/238`, `goldens=29`, `gates=119`, `lines=364`, `tests-expected=5042` — the only phase movements are the four entitled growth gates. |
| 12 | The ledger's row count is a stat counted from the file on every run, not a quoted figure. | ✓ VERIFIED | `tools/check/steps/claims.sh:108` prints `handoffs=<n>` from the ledger with the grammar pass's own row predicate; this session's line carries `handoffs=8` and the file's non-comment, non-empty row count is 8. |
| 13 | The id set and every disposition are pinned in both directions, so a missing, extra, duplicated or re-dispositioned row is a named finding. | ✓ VERIFIED | `HANDOFF_IDS='H7-1 H7-2 H8-3 D10 D11 H16-1 H16-2 H16-3'` and `HANDOFF_DISP` name each disposition; the four `claims-handoff-*` controls plus `-row`/`-missing`/`-unreadable` are committed and all 404 controls pass this session. |
| 14 | Every ledger copy moved in the same commit, so each control still prints only its own intended finding. | ✓ VERIFIED | The good tree's ledger and the four overlays each carry the three rows; `run-controls.sh` printed `controls=404/404` inside this session's gate with no toy-drift, good-failed, token-missing, exit-zero, inventory or orphan finding. |
| 15 | Nothing about the rows is a result: an answer is `evidence: none` either way, no row is a pass, no claim may cite one, and `D10`/`D11` stay `not-performed`. | ✓ VERIFIED | Ledger read this session: every row's last two tokens are literally `evidence: none`, `D10`/`D11` are `not-performed`, and no claim cites any of them. The packet repeats the rule in 'The one rule to carry away' and 'These rows are never a result'. |
| 16 | PKG-06 is `Complete` in the same commit as the three registry rows and the thirteen toy lines, so neither `important-gates-unknown-id` nor `-complete-uncovered` can fire. | ✓ VERIFIED | `.planning/REQUIREMENTS.md:46` `- [x] **PKG-06**`, `:112` `\| PKG-06 \| Phase 16 \| Complete \|`; the three `release-posture` rows resolve in their inventories; this session's line carries `gates=119 lines=364` with `rows-pending=0`, and the gate is green so neither finding fired. |
| 17 | The three rows are pointers to mechanisms that already exist: `steps` and `faults` do not move. | ✓ VERIFIED | `steps=38/38`, `faults=238/238`, `fault-runs=238` this session; the Phase 16 block in `tools/check/steps/important-gates.sh:81-91` still records 'zero new faults, zero new steps' and why the two omissions are deliberate. |
| 18 | The closing gate line is measured under `sh` and `dash` and is byte-identical, and the container leg ran once with exactly the four documented host tokens differing. | ✓ VERIFIED | Re-ran the `sh` leg once this session: exit 0, `gate: PASS`, final line **1,520 bytes**, zero `^-- FAIL` lines, every predicted token present. The `dash` leg and the container leg are carried forward from the previous verification (same scripts, same records; `build/gate-linux.log` still shows `gate: PASS`, a 1,512-byte line and exactly `cc`, `core-imports`, `archive`, `refs-verified` differing, and the byte arithmetic 1,520 − 7 − 2 + 1 = 1,512 still closes). **One measured difference from last round:** the `tracked`/`hygiene-files` tokens read **2789** where the recorded line reads 2787, because two planning records were added after the measurement (`16-05-SUMMARY.md` in `438bffd`, `16-VERIFICATION.md` in `9703594`). Every evidence token is unchanged and the line is still 1,520 bytes, so the claim about the gate's content holds; the quoted line is reproducible at its measurement commit, not at `HEAD`. Recorded as ℹ️ Info, not a gap — a `tracked` count necessarily counts the records that quote it. |
| 19 | The phase boundary records **exactly one** checkpoint verdict in the section 2.3 vocabulary, with the medians recomputed by arithmetic from `.planning/ci-runs.tsv`. | ✓ VERIFIED | Recomputed this session from the last five rows (`45723269901`, `45747709457`, `45751979661`, `45778926664`, `45779277467`): `cpu_s` 368, 366, 359, 441, 352 → **366**; `build_s` 35, 34, 34, 38, 33 → **34**; `gate_s` 122, 115, 120, 145, 115 → **120**, identical to the record's 366 / 34 / 120 against the frozen 361 / 34 / 118. `discharged-within-allowance` is unreachable (366 > 361.4 by 4.6); `lever-required` is unreachable (cumulative priced total 2.0 + 1.3 + 1.5 + 0.4 = **5.2 of 21**, the phase's actual addition 0.015 against the 0.4 it priced); `reprice` is therefore forced and is the single word in `.planning/CI-EFFICIENCY.md:2123` and `.planning/STATE.md:791`, with `steps=38/38` recorded as the invariant. The previous report's rule-wording note (§2.3's 'with an unchanged token line' versus this phase's four entitled growth gates) stands unchanged as an observation for the next milestone, not a defect in this record. |
| 20 | **16-05 Task 2.** `16-VALIDATION.md` is signed off in the required shape, and the phase SUMMARY states plainly what the milestone can and cannot now claim and names the handoffs that remain. | ✗ FAILED | The SUMMARY half is met and unchanged: `16-05-SUMMARY.md` names `D10`, `D11`, `H7-1`, `H7-2`, `H8-3`, `H16-1`, `H16-2`, `H16-3`, says plainly that no host artifact, shared object, content or runner installer ships, that nothing reaches `measured-hardware`, that `CL-0002` is the only `established` claim and that the rows are not evidence — re-checked this session against `data/claims.json` (29 claims, standing 28 `provisional` / 1 `established`, no `measured-hardware` tier, no handoff artifact). The record half is **partly** met. Closed since last round: the ten Threat Ref cells now name the right plan's register ids (T-16-01, -02, -05, -06, -10, -11, -14, -15, -19, -20 — 10 of 10), and the runtime row is a measured range rather than the seeded estimate. Still open: the Wave 0 bullets are unresolved (`grep -c 'satisfied by'` = **0** against the required ≥8; `15-VALIDATION.md` carries 15), `D10` is absent from the file and the Manual-Only table has 3 rows where the plan requires 4 (the D10/D11 pair), the Approval line is a reordered 14-token subset rather than the measured line (`grep -F` for the 1,520-byte line: **0** hits), and the Quick run command plus the Sampling Rate section are still the seeded template with the `~72 s` estimate. Three of plan 16-05 Task 2's own record assertions fail on the current file. See the `gaps` entry. |

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
| `.planning/ops/RELEASE-OPS.md` | section (j) transcript; section (i.1) recomputation | ✓ VERIFIED | (j) at `:163` with the four printed lines verbatim and the three "must not infer" sentences; (i.1) at `:101` as a 63-added/0-deleted append. |
| `data/expected/handoffs.ids` | eight rows, three new, all `evidence: none` | ✓ VERIFIED | Read this session: `H16-1..H16-3 handed-off`, `D10/D11 not-performed`, every row ending `evidence: none`. |
| `docs/handoffs.md` | three packet sections in the page's shape | ✓ VERIFIED | `H16-1` (shim target, object digest/size, the nine `digest.*` lines), `H16-2` (the route + section (j)), `H16-3` (two wraps at `hi-first`/`lo-first`), plus the closing 'never a result' paragraph. |
| `data/expected/important-gates.ids` | three `release-posture` rows with per-surface comments | ✓ VERIFIED | `:598-620`; each row resolves; the two deliberate omissions are recorded with their reasons; `gates=119 lines=364 rows-pending=0`. |
| `.planning/REQUIREMENTS.md` | PKG-06 `Complete` (checkbox, traceability row, `Last updated`, coverage) | ✓ VERIFIED | All four moved in `298be25`; no `Pending` PKG-06 remains; PKG-06 is Phase 16's only requirement, so nothing is orphaned. |
| `tools/check/steps/important-gates.sh` | the Phase 16 block | ✓ VERIFIED | `:81-91`, naming PKG-06, the one new gate name, the three rows, "zero new faults, zero new steps" and both omissions. |
| `.planning/phases/16-…/16-VALIDATION.md` | the signed record plan 16-05 Task 2 specifies | ✗ STUB (partial) | Signed and frontmatter-complete, ten task rows, correct Threat Ref cells, measured runtime. But Wave 0 bullets unresolved (0 `satisfied by`), `D10` absent from a 3-row Manual-Only table, Approval line a 14-token subset rather than the measured line, and the Quick run command + Sampling Rate sections still the seeded template. See the `gaps` entry. |
| `.planning/STATE.md`, `.planning/ROADMAP.md`, `.planning/CI-EFFICIENCY.md` | the closing line once, the phase tokens, one verdict, the plan list | ✓ VERIFIED | STATE carries the full 1,520-byte line once, the tokens, the `reprice` bullet with its arithmetic and the `not-recorded` hosted row with its reason; CI-EFFICIENCY carries `phase16: reprice (…)`; **ROADMAP is now consistent** — the v0.3.0 table reads `16 … 5/5 \| Complete \| 2026-11-26` and the narrative below says all four phases are executed and verified. |

### Key Link Verification

| From | To | Via | Status | Details |
|------|----|-----|--------|---------|
| `tools/gate.sh` | `tools/check/steps/c-tests.sh` / `c-faults.sh` | background partner started with the same redirection, foreground member, join writing `rc.`/`t.` | ✓ WIRED | Both step pass lines appear in this session's log body, the partition and pair lines are printed, and the gate is green with the same tokens as the serial run measured last round. |
| `docs/owner-images.md` | `.planning/ops/RELEASE-OPS.md` section (j) | the route's rehearsal note cites the section by path and letter | ✓ WIRED | The pointer resolves (`## (j)` exists exactly once) and every figure it points at reproduced in the previous session's run on unchanged files. |
| `tools/release-rehearse.sh` | `data/golden/demo-lamp-tone-pass.digests` | the filtered-report comparison, unchanged, still last in the limb | ✓ WIRED | `124 of 124 compared`, report 131 lines, `rehearse: ok`. |
| `tools/release-rehearse.sh` | the archive's `tools/owner-run.sh` and `ivs_flatwrap` | paths relative to `$work/route/$prefix` | ✓ WIRED | `[ -x "$wrap" ]` and `[ -f "$run" ]` guards; both are the extracted archive's, never the repository's. |
| `data/expected/handoffs.ids` | `tools/check/steps/claims.sh` | grammar pass, then the id set and each disposition against the two pinned lists; `handoffs=` from the same read | ✓ WIRED | `handoffs=8` equals the file's row count; a missing/extra/duplicate/re-dispositioned row is a named finding proved by the four controls. |
| `data/expected/handoffs.ids` | `docs/handoffs.md` | the ledger is the inventory, the packet the instruction sheet | ✓ WIRED | One section per row; the page states no count. |
| `.planning/REQUIREMENTS.md` | `data/expected/important-gates.ids` | the traceability table is the derived covered set | ✓ WIRED | `gates=119 lines=364`, `rows-pending=0`; both refusal directions are exercised by committed controls. |
| `.planning/ci-runs.tsv` | the checkpoint bullet | last five rows, each window sorted, middle value printed | ✓ WIRED | Recomputed this session: 366 / 34 / 120, identical to the record. |
| `.planning/ROADMAP.md` v0.3.0 table | the same file's Phase 15 and Phase 16 sections | the table's `Plans Complete`/`Status` columns against each section's `**Plans**` line and ticked entries | ✓ WIRED | **Closed this round:** `15 … 3/3 Complete`, `16 … 5/5 Complete`, and the Phase 16 section reads `**Plans**: 5/5 plans executed` with five ticked entries; the narrative no longer calls Phase 15 half-executed. |

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
|----------|---------|--------|--------|
| The full gate is green on this tree | `sh tools/gate.sh` (one run this session) | exit 0, `gate: PASS steps=38/38 … gates=119 lines=364 handoffs=8`, 1,520-byte final line, 0 `^-- FAIL` | ✓ PASS |
| Every invariant token is where the plans predicted | token grep over the closing line | all 16 checked tokens present exactly once (`steps controls faults goldens gates lines handoffs claims provisional prefix-files tests-expected tests-executed tests-skipped tests-missing binaries-listed rows-pending`) | ✓ PASS |
| The overlap pair and partition are live | `grep` over the gate log body | `gate-overlap pair=c-faults+c-tests`, `gate-partition preflight=29 producers=9`, `-- c-tests pass ctest-runs=2`, `-- c-faults pass fault-runs=238` | ✓ PASS |
| The recorded medians reproduce by arithmetic | Python median over the last five `ci-runs.tsv` rows | 366 / 34 / 120, identical to the record | ✓ PASS |
| The handoff ledger and package inventory are what the records say | `awk` row counts over the two `.ids` files | `handoffs=8`, `pkg-install-files=11` | ✓ PASS |
| The claim ledger's posture is what the SUMMARY says | Python read of `data/claims.json` | 29 claims, 28 `provisional` / 1 `established`, no `measured-hardware`, no handoff artifact, zero `H16-`/`handoffs` | ✓ PASS |
| The roadmap's status surface agrees with its phase sections | read of the v0.3.0 table + narrative + Phase 16 section | `16 … 5/5 \| Complete \| 2026-11-26`; narrative says all four phases executed and verified; section says `5/5 plans executed` | ✓ PASS (gap 2 closed) |
| Plan 16-05 Task 2's record assertions | each assertion run separately against `16-VALIDATION.md` | 11 pass; 3 fail on record defects (`satisfied by` 0, `D10` missing, verbatim line absent); 2 fail on plan-verify defects (threat-ref regex cannot match `T-16-20`, green-row count counts the legend line) | ✗ FAIL (see gap) |
| The container leg | read of the retained `build/gate-linux.log` (not re-run) | `gate: PASS`, 1,512-byte line, exactly four documented token differences | ✓ PASS (carried forward from the retained log) |

### Probe Execution

| Probe | Command | Result | Status |
|-------|---------|--------|--------|
| committed-direction shared-object probe | `git add -f probe.so probe.txt` → `files.sh` over `git ls-files` | Not re-run this session by instruction: `tools/check/steps/files.sh`, `files.cmake` and `binary-ext.txt` are byte-unchanged since the run that produced `-- FAIL files-unlisted-binary: probe.so` with `probe.txt` silent and the tree clean. This session's gate keeps `binaries-listed=0` and `rights-pinned=4/4` green. | ✓ PASS (carried forward, files unchanged) |
| release-time source-archive rehearsal | `sh tools/release-rehearse.sh --no-gate --release --source-route` | Not re-run this session by instruction; `tools/release-rehearse.sh` and every record it writes are unchanged since the run that printed `rehearse: ok` with `124 of 124` compared and the mutation refused at exit 14. | ✓ PASS (carried forward, files unchanged) |

### Requirements Coverage

| Requirement | Source Plan | Description | Status | Evidence |
|-------------|-------------|-------------|--------|----------|
| PKG-06 | 16-01 … 16-05 (all five declare it) | A tracked shared object is refused by name, the installed package stays eleven files, and the release-decision line states that no shared object, host shim or content is shipped. | ✓ SATISFIED | Committed-direction probe reproduced (truth 4); `prefix-files=11` and the eleven-path inventory (truth 5); the line verbatim in `RELEASE-OPS.md` (i)/(i.1), `docs/packaging.md:123`, `PROJECT.md`, `ROADMAP.md`; `binaries-listed=0`; `so-member` and `host-so-absent` live. `.planning/REQUIREMENTS.md:112` reads `Complete`; no PKG-06 remains `Pending`; Phase 16 carries no other requirement, so none is orphaned. |

### Anti-Patterns Found

| File | Line | Pattern | Severity | Impact |
|------|------|---------|----------|--------|
| `.planning/phases/16-…/16-VALIDATION.md` | 74-88, 30-38, 96-101 | seeded-template remnants: unresolved Wave 0 bullets (0 `satisfied by`), Manual-Only table without `D10`, Approval line a reordered 14-token subset, `~72 s` latency estimate, `<step>` quick-run template | 🛑 Blocker | The validation contract still does not describe the phase's Wave 0 evidence, its fourth manual item or its measured line. Carried-forward gap — self-evidencing. |
| `.planning/ROADMAP.md` | 414 | `**Plans**: TBD` | ℹ️ Info | Belongs to the **dropped** Phase 12 inside the archived v0.2.0 `<details>` block (`12 … 0/0 \| Dropped`). Confirmed pre-existing: the string occurs twice at `857f45d^` and once now, so this phase neither introduced nor touched it; a dropped phase has no plans. Worth a sweep at the milestone close, out of this phase's scope. |
| `16-05-SUMMARY.md` | — | reconciliation is prose, not the required one-row-per-token table | ⚠️ Warning | Form only; the substance was re-derived in this verification. See `advisory`. |
| `tools/check/steps/*.sh`, `tools/gate.sh`, `tools/release-rehearse.sh` | — | `XXXXXX` in `mktemp` templates | ℹ️ Info | False positive of the debt-marker scan; not a marker. |
| `.planning/STATE.md`, `16-05-SUMMARY.md`, `16-VALIDATION.md` | closing line | `tracked=2787` quoted, `2789` measured at `HEAD` | ℹ️ Info | Two planning records were added after the line was measured. No evidence token moved and the line is still 1,520 bytes; the quoted line is reproducible at its measurement commit, not at `HEAD`. Inherent to quoting a `tracked` count from a commit that adds records. |
| `16-05-PLAN.md` | 411-414 (verify block) | `T-16-(0[1-9]\|1[0-9]\|SC)` cannot match `T-16-20`; `grep -E '✅ green' \| wc -l` counts the `*Status: …*` legend line | ℹ️ Info | Two assertions in the plan's own `<automated>` block are self-defeating: they fail on a correct record (9 vs 10 threat lines, 11 vs 10 green rows). Record, not chase, at the next plan-writing pass. |

### Human Verification Required

None. Every phase behavior has an automated check and each was exercised in this session or the previous one on
byte-unchanged files; the owner-only items (`H16-1`, `H16-2`, `H16-3`, `D10`, `D11`) are ledger rows with
procedures, are not evidence and are not criteria, so they are not verification items. (Their incomplete
presence in `16-VALIDATION.md`'s Manual-Only table is a record defect — the `gaps` entry — not a request for a
human test.)

### Gaps Summary

The phase's **goal is achieved** and unchanged from the previous verification: all five ROADMAP success
criteria hold, and PKG-06 is `Complete` on evidence re-derived rather than read — the owner route genuinely
runs from a freshly built source archive with the transcript's own container digest, the one-byte mutation is
refused at load while `exec:` stays `-`, the three handoff rows are `evidence: none` and cited by nothing, a
committed shared object is refused by name with a witness pair, the package is eleven files, the
release-decision line reads as required, and the concurrency lever is a one-commit, cost-only change whose
token line was proved byte-identical on one tree by running the gate both ways.

**Gap 2 of the previous report is closed.** `.planning/ROADMAP.md`'s v0.3.0 progress table now reads
`16. Owner Route, Claim and Release Posture | 5/5 | Complete | 2026-11-26`, and the paragraph below it says
all four phases are executed and verified with Phase 15 delivered inside its priced line. That surface now
agrees with the Phase 15 and Phase 16 sections in the same file, and no `TBD`/`Not started` cell remains for
this phase.

**Gap 1 is not closed.** Two of its five defects are fixed — the ten Threat Ref cells now name each task's own
plan register (10 of 10, previously 8 wrong), and the runtime row is a measured range rather than the seeded
estimate. Three remain, and two of them are the items this report listed under `missing` last time:

1. **Wave 0 is unresolved.** All ten bullets are still the seeded one-line form. `grep -c 'satisfied by'` is
   **0** against the plan's required ≥8, and Phase 15's `15-VALIDATION.md` — the precedent the plan names —
   resolves 15 bullets with the path and the measured token that proves each.
2. **`D10` is absent.** The Manual-Only table has three rows (`H16-1`, `H16-2`, `H16-3`) where the plan
   requires four including the `D10`/`D11` pair; `D11` survives only as a parenthetical inside the H16-2 row.
3. **The Approval line is not the measured line.** It is a 14-token, reordered, three-line-wrapped subset, and
   `grep -F` for the 1,520-byte line returns **0** — exactly the defect last report described. The text now
   points at `STATE.md` and `16-05-SUMMARY.md` for the full line, which is a pointer, not the verbatim quote
   the plan's `<automated>` block greps for.

Plus two sections this report had folded into "seeded template" that are still untouched and still contradict
the plan's action: the **Quick run command** row is the per-step `<step>` incantation the plan explicitly told
the author not to invent, and the **Sampling Rate** section still prescribes per-step runs and a `~72 s`
latency — the same estimate the file's own new runtime row supersedes.

Three of plan 16-05 Task 2's own record assertions fail on the current file, so the gap is mechanical, not a
matter of taste. It is one file, and the fix needs no new measurement: the figures to quote (1,520-byte line,
the ten Wave 0 artifacts with their tokens, the `D10`/`D11` hardware row, the measured runtime range) are all
already in `STATE.md`, `16-05-SUMMARY.md` and the plans. If the owner judges the residual form immaterial, the
alternative is an explicit override rather than a silent pass — the exact YAML is:

```yaml
overrides:
  - must_have: "16-VALIDATION.md is signed off with the Wave 0 bullets resolved, the four manual items and an Approval line carrying the measured gate line verbatim"
    reason: "<why the seeded Wave 0, Sampling Rate and Approval form is acceptable for this phase>"
    accepted_by: "<name>"
    accepted_at: "<ISO timestamp>"
```

No token, count or gate result needs to change for the fix: `steps=38/38`, `controls=404/404`,
`faults=238/238`, `goldens=29`, `gates=119`, `lines=364`, `handoffs=8`, `prefix-files=11`,
`claims=29 provisional=28` and every other evidence token are exactly where the plans predicted. The only
movement since the close is `tracked`/`hygiene-files` 2787 → 2789, from the two planning records themselves,
with the line still 1,520 bytes.

### Note for the milestone audit

**Is the phase SUMMARY's "what the milestone can and cannot claim" statement accurate against the tree?**
Yes, with the one imprecision carried forward. Checked item by item this session: `data/claims.json` holds 29
claims with 28 `provisional` and one `established` (`CL-0002`), no claim at `measured-hardware`, and no claim
whose artifact is the handoff ledger or a `D10`/`D11`/`H16-` id; nothing is installed beyond the eleven paths
and no shared object is in the release; `D11` is `not-performed` and the route's `exec:` label is `-`; the
eight remaining questions are the ledger's rows with procedures and no evidence. The imprecision: the SUMMARY
says "D10, 47 items handed off" while `D10`'s ledger row is `not-performed` — the 47 items are inventoried in
`data/expected/d10-measurements.ids` (`d10-items=47` on the gate line) and offered, but the row's own
disposition is "needs hardware no tool on this host can supply", not `handed-off`. Worth a word at the close;
it does not change what the milestone can claim. The SUMMARY's nine-deviation count also matches
`.planning/WINDOWS.md` (rows 5-13, all waived).

---

_Verified: 2026-11-26T12:09:18Z_
_Verifier: Claude (gsd-verifier)_

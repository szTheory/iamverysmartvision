---
phase: 16-owner-route-claim-and-release-posture
verified: 2026-11-26T11:51:12Z
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
covered_digest: "v4:sha256:dbb6b97f33890e1787dead7b31de3a47987212935d54000a0e438c7fe31b5ce2"
behavior_unverified: 0
overrides_applied: 0
gaps:
  - truth: "16-VALIDATION.md is signed off as plan 16-05 Task 2 requires — ten task rows carrying the threat ids each task's own plan assigns, the Wave 0 bullets resolved to their artifacts, the five genuinely manual items in the Manual-Only table, and an Approval line carrying the measured closing gate line verbatim."
    status: failed
    reason: "The record is signed (status: validated, nyquist_compliant: true, wave_0_complete: true, ten task rows, Approval line present) but four of its sections are still the seeded template. (a) The Manual-Only table reads 'none' — H16-2, D10 and D11 appear nowhere in the file, and the phase's own exit is defined by three handoff rows plus two not-performed owner gates. (b) The per-task Threat Ref column carries T-16-01..T-16-05, which are plans 16-01/16-02's register ids; 16-03's tasks were assigned T-16-10..T-16-13, 16-04's T-16-14..T-16-18 and 16-05's T-16-19..T-16-23, so 8 of the 10 rows point at another plan's threat. (c) No Wave 0 bullet is resolved with the 'satisfied by <path>' + measured-figure form the plan asked for (grep count 0 against a required 8). (d) The Approval line carries an abbreviated token subset ('gate: PASS steps=38/38 controls=404/404 faults=238/238 goldens=29 gates=119 lines=364 handoffs=8 ...'), not the measured closing line; grepping the file for the full 1,520-byte line returns 0. (e) 'Estimated runtime ~72 s ... 66 s at the final tree' is still an estimate against the close's measured 104 s (sh) / 204 s (dash) / 793 s (container), which the plan's own action forbids. The SUMMARY half of the truth is met (see truth 20's note): 16-05-SUMMARY.md states what the milestone can and cannot claim and names all eight remaining handoff ids. Phase 15's 15-VALIDATION.md is the in-repo precedent for the required shape and was genuinely rewritten; this one was not."
    artifacts:
      - path: ".planning/phases/16-owner-route-claim-and-release-posture/16-VALIDATION.md"
        issue: "Manual-Only table says 'none' (five items missing); 8 of 10 Threat Ref cells name the wrong plan's register ids; 0 Wave 0 bullets resolved in the required form; Approval line is an abbreviated token list rather than the measured line; runtime row is still the seeded estimate."
    missing:
      - "Manual-Only rows for H16-1, H16-2, H16-3 and the D10/D11 pair, each with its reason and the location of its procedure, plus the sentence that they are ledger rows and not evidence"
      - "Threat Ref cells corrected to each task's own plan register (16-03: T-16-10..T-16-13; 16-04: T-16-14..T-16-18; 16-05: T-16-19..T-16-23)"
      - "Wave 0 bullets resolved as '— satisfied by <path>' with the measured token that proves each"
      - "The measured closing gate line quoted verbatim in the Approval line"
      - "The measured runtimes (104 s sh, 204 s dash, 793 s container) in place of the '~72 s / 66 s' estimate"
  - truth: "The phase close leaves the roadmap's own status surface consistent: the v0.3.0 progress table and its closing narrative agree with the phase sections that record 5/5 plans executed and 3/3 for Phase 15."
    status: failed
    reason: "`.planning/ROADMAP.md` was modified by this phase's close (commit 438bffd) and its v0.3.0 progress table still reads `| 16. Owner Route, Claim and Release Posture | 0/TBD | Not started | — |` while the same file's Phase 16 section reads `**Plans**: 5/5 plans executed` with five ticked plan entries. The narrative immediately below the table is stale the same way: it says 'Phase 15 is planned in three waves and 2 of 3 plans are executed ... Phase 15 is droppable' against the table's own `15. ... | 3/3 | Complete | 2026-11-26`. A `TBD` cell on a phase-modified file, contradicting the file's own content, is exactly the 'quoted figure that disagrees with the file' defect AGENTS.md says to correct, and it is the surface a reader lands on first. The fix is one row and one paragraph; the phase's substantive records are correct."
    artifacts:
      - path: ".planning/ROADMAP.md"
        issue: "v0.3.0 progress table row for Phase 16 still '0/TBD | Not started'; trailing v0.3.0 narrative still describes Phase 15 as 2 of 3 executed and droppable."
    missing:
      - "The Phase 16 progress row recomputed to `5/5 | Complete | 2026-11-26`"
      - "The v0.3.0 closing narrative recomputed (Phase 15 3/3 complete and not dropped; Phase 16 5/5 executed)"
advisory:
  - finding: "16-05-SUMMARY.md's token reconciliation is prose ('every token the five plans predicted is where it was measured', with the four moved tokens named) rather than the one-row-per-token table plan 16-05 Task 1 required."
    category: other
    reason: "The substance is present and was independently re-derived in this verification (every token of the closing line was checked against the five plans' predictions), so this is a form deviation, not a missing reconciliation. A table would make the next reader's check mechanical."
    evidence_status: "verified by re-derivation; form differs from the plan"
---

# Phase 16: Owner Route, Claim and Release Posture Verification Report

**Phase Goal:** The owner route from the source archive is documented and rehearsed, the one question only
the owner can answer is a handoff row, and the release posture states plainly that no host artifact and no
content ships.
**Verified:** 2026-11-26T11:51:12Z at `438bffd`
**Status:** gaps_found
**Re-verification:** No — initial verification

**What was re-derived rather than read.** Every figure below was produced in this session on this tree: the
full gate under `sh` (three times) and under `dash` (once), the same gate with the phase's overlap **disabled**
in a clean worktree at `HEAD` and again in the main tree, a byte-for-byte comparison of the serial and
overlapped logs, the release-time rehearsal re-run end to end from a freshly built source archive
(`sh tools/release-rehearse.sh --no-gate --release --source-route`), the committed-direction probe re-run on
the real tree, the handoff ledger, the claim ledger and the important-gates registries read from the files,
the cost checkpoint recomputed from `.planning/ci-runs.tsv`, and the container leg read from the retained
`build/gate-linux.log` rather than re-run (a second container run would overwrite the only copy — lesson 18).
No SUMMARY figure was accepted as evidence.

## Goal Achievement

### Observable Truths

| # | Truth | Status | Evidence |
|---|-------|--------|----------|
| 1 | **Criterion 1.** One documented owner route (EXEC + GROM + a wrapped cartridge, three commands) is rehearsed from the **source archive** to a project-cartridge golden, with the transcript recorded; a release-time rehearsal, not a gate step. | ✓ VERIFIED | My run: `sh tools/release-rehearse.sh --no-gate --release --source-route` → `rehearse: source-archive: equal (6793979 bytes)`, `rehearse: source-route: ivs-v0.2.0-source.tar.gz, golden match (report 131 lines, 124 of 124 compared), prefix ivs-v0.2.0`, `rehearse: ok`, exit 0. The limb extracts `$work/a/$src_a` into `$work/route`, runs `cmake --preset release` + `cmake --build --preset release` **inside the extracted prefix**, and uses that prefix's own `build/release/tools/flatwrap/ivs_flatwrap` and `tools/owner-run.sh` (`tools/release-rehearse.sh:182-215, 219-300`), so the route is the archive's, not the working tree's. `docs/owner-images.md:55-92` documents exactly the three commands and cites `.planning/ops/RELEASE-OPS.md` section (j); section (j) is the transcript and names the commit it ran on (`14f81ed`, an ancestor of `HEAD`). The only figure that differs from the transcript is the archive's byte size (6,759,881 recorded vs 6,793,979 measured), which is a function of the commit — the record says so, and every other quoted figure reproduced exactly, including the container digest. No gate step runs it (`steps=38/38`). |
| 2 | **Criterion 2.** The route's pass path is exercised with project-made files, and a one-byte EXEC mutation proves the id can fail; the `exec:` label stays `-` so no run reads as a real-EXEC result. | ✓ VERIFIED | The rehearsal makes an 8,192-byte EXEC, a 2,048-byte GROM and an 8,192-byte flat image with `dd` from the zero device inside the extraction's `refs/` (`:230-241`), then asserts eight report lines and then flips one byte at offset 0 and re-runs the `--cart`-only form. My run printed `rehearse: owner-route: wrap base=4000 word-order=hi-first digest=8d73b98c58bb75a6515c6a791ebd2925265d05874e324c31a5836e61055b74f5`, `run exit=10 outcome=guest-fail exec-label=- cart-digest-matched=yes`, `mutation exit=14 status=IVS_E_DIGEST_MISMATCH run-state=unreset` — identical to the transcript, digest included. The asserts behind those lines are `grep -x -F 'exec: -'`, `outcome guest-fail`, `exit.code 10`, `slot.cart.sha256 <digest>`, `outcome.detail load-refused role=exec status=IVS_E_DIGEST_MISMATCH`, `cpu.run_state unreset`; a pass path that could not fail would leave the second limb red, so neither half passes alone. D11 stays `not-performed` and no claim cites section (j). |
| 3 | **Criterion 3.** The owner-only questions are `handoffs.ids` rows, `evidence: none`, dispositioned, cited by no claim and by no phase exit criterion. | ✓ VERIFIED | `data/expected/handoffs.ids` holds eight rows; `H16-1`, `H16-2`, `H16-3` are `handed-off evidence: none`, `D10`/`D11` are `not-performed evidence: none`. `data/claims.json` contains **zero** occurrences of `handoffs` and no `H16-`/`D10`/`D11` id; `tools/check/kinds/claim.cmake:145` refuses `data/expected/handoffs.ids` as a claim artifact. `grep -rn 'H16-'` over `.planning/REQUIREMENTS.md`, `data/claims.json`, `CHANGELOG.md` is empty, and the only ROADMAP mention is the plan-inventory line `:296` that names the ids as rows *to add* (recorded as `.planning/WINDOWS.md` row 10 — a citation of work, not of a result, and not a success criterion). The five ROADMAP success criteria cite no id. `docs/handoffs.md` carries one section per row with a command, an artefact, the deterministic identities and what a person judges, plus the closing "These rows are never a result" paragraph. |
| 4 | **Criterion 4a.** A tracked shared object is refused by name — including the **committed** direction Phase 15 could not reach from a control. | ✓ VERIFIED | Re-ran the probe myself on the real tree: `printf 'probe\n' > probe.so; printf 'probe\n' > probe.txt; git add -f …; git ls-files > fl; … sh tools/check/steps/files.sh` → exactly `-- FAIL files-unlisted-binary: probe.so` (plus the CMake failure trailer), **no** `probe.txt` occurrence, **no** pass line, exit 1; `binscan.sh binary` over the same list printed nothing (it skips a name-listed extension, so the refusal is the name, not a byte scan). Both files unstaged and removed; `git ls-files` holds no `.so/.dylib/.dll/.bundle` path and `git status --porcelain` is empty. The other two surfaces are live in the gate: `release-assets` case `so-member` (`release-selftest=12/12`) and `pkg-install` selftest `host-so-absent` (`pkg-selftest=20/20`). |
| 5 | **Criterion 4b.** The installed package stays eleven files, and the release-decision line reads: the release ships the static package and the source archive; no shared object, no host shim, no content. | ✓ VERIFIED | `awk '!/^#/ && NF' data/expected/pkg-install-files.ids \| wc -l` = **11** (five headers, four CMake package files, `lib/libivs_core.a`, `share/doc/ivs/LICENSE`) and the gate prints `prefix-files=11`, `binaries-listed=0`. The line appears verbatim in `.planning/ops/RELEASE-OPS.md` twice (section (i)'s quote block and section (i.1)'s unchanged re-quote), in `docs/packaging.md:123`, `.planning/PROJECT.md` and `.planning/ROADMAP.md`. Section (i.1) is an append: commit `9c621a8` is 63 added / **0 deleted** lines, and its five values (`binaries-listed=0`, `prefix-files=11`, `pkg-selftest=20/20`, `release-selftest=12/12`, `controls=404/404`) are all on the closing gate line. |
| 6 | **Criterion 5a.** The concurrency lever is taken in **one cost-only commit** whose summary token line is byte-identical and whose `step.out.<name>` buffers are compared. | ✓ VERIFIED | The lever is one commit (`f9fe6a9`: `tools/gate.sh` 58+/2−, `c-tests.sh` 50+/3−, `c-faults.sh` 49+/3−, **no file created**, so `tracked`/`files`/`hygiene-files` could not move). `tools/gate.sh` declares `overlap_bg=c-faults` / `overlap_fg=c-tests`, backgrounds the earlier member, runs the later one in the foreground, joins the partner on the normal **and** failing paths, joins again after the producer loop as a safety net, kills a live partner from `on_exit`, and refuses a pair member that is not a producer (`gate-overlap-pair`). Independent proof, not the SUMMARY's: I ran the gate with the overlap **disabled** and then **enabled** in the same tree, back to back, and got `gate: PASS` both times with the final lines **byte-identical** (`cmp` clean, 1,520 bytes each), the whole logs identical with only the measurement lines (`step-time`, `gate-time`, `gate-cpu`, `gate-env`, `gate-overlap`) filtered out (**0 diff lines** over 498 lines), and both step blocks equal: `-- c-tests pass ctest-runs=2`, `-- c-faults pass fault-runs=238`. A third confirmation in a clean worktree at `HEAD` differed only in `refs-verified` (`3/4` vs `0/4`), which is the ignored `refs/` directory absent from a fresh checkout, not the lever. The byte-count arithmetic also closes: 1,509 bytes at the phase baseline + the 11-byte ` handoffs=8` token = the 1,520 bytes measured here. **Caveat recorded honestly:** each of the two steps emits exactly one line, so for these two steps the per-buffer comparison *is* the pass-line comparison. It is not vacuous (a changed count, an extra diagnostic or a missing pass line changes it, and a failing step breaks the liveness guard), but it is not stronger than the token-line comparison for these two steps — no gate weakening, and no additional coverage claimed. |
| 7 | The lever adds no tracked file, and the inline build lock is byte-identical between its own markers. | ✓ VERIFIED | `sed -n '/^# build-lock begin$/,/^# build-lock end$/p'` on both step scripts, `cmp`-equal with no normalisation; each script carries the sentence above the marker that names the other copy and says why it is a copy (a new tracked file would move the token line). |
| 8 | The pair's two builds are serialised by a bounded lock whose necessity is a measurement, not an assumption. | ✓ VERIFIED | Both scripts wrap every `cmake --build` call in one polled `mkdir` lock at `<results>/.build-lock` (0.01 s poll, ~60,000 polls), release it on the success path, the failure path and the caller's `EXIT`/`HUP`/`INT`/`TERM` traps, and on expiry report their own existing token with a `build-lock` detail. The lock lives in the per-run results directory that `gate.sh` recreates, so no stale-lock rule is needed. The recorded reason (two unlocked concurrent builds of one tree fail at link with a half-written `core/libivs_core.a`) is in `.planning/CI-EFFICIENCY.md` and in the scripts' comments. |
| 9 | The overlap costs wall time only: `gate-time total` falls and `gate-cpu` does not rise beyond noise; no allowance credit and no checkpoint word is claimed for it. | ✓ VERIFIED | Same tree, same host, back to back: serial `gate-time total=147`, `gate-cpu user=186.40 sys=225.12` (sum 411.52) → overlapped `gate-time total=82`, `gate-cpu user=123.15 sys=109.77` (sum 232.92). Wall fell 65 s and cpu **fell** in this sample, so no cost regression; the pair's own `step-time` went 14+33 (serial) to a joined 22. The phase's recorded pre/post figures (79 → 72 s; cpu 228.23 → 260.77, delta 32.54 inside a 45.6 band) are not reproducible from the committed tree because the two logs were kept in `/tmp` and not committed, and my single pair measures a different host state (three `sh` runs this session read 90, 147 and 82 s, and `dash` read 172 s — the host was not idle). `.planning/CI-EFFICIENCY.md` claims **no allowance credit and no checkpoint word** for the lever and records the container leg as not run in that entry; the hosted saving is stated as a prediction (~144 s) for the first hosted row to settle, not as a result. |
| 10 | Nothing else moved: `steps=38/38`, `controls=404/404`, `faults=238/238`, `goldens=29`, the partition stays `preflight=29 producers=9`, and both steps keep their own pass lines. | ✓ VERIFIED | Every one of those tokens is on my closing line, `gate-overlap pair=c-faults+c-tests` and `gate-partition preflight=29 producers=9` are both printed, `ls tools/check/steps/*.sh \| wc -l` = 38, and `-- c-tests pass ctest-runs=2` / `-- c-faults pass fault-runs=238` each appear exactly once. |
| 11 | The route stays release-time: no gate step runs it, CI never runs it, no test id/control/fault/golden is added, and every token is in place. | ✓ VERIFIED | No gate step invokes `release-rehearse.sh` (`grep -rn release-rehearse tools/check/steps/` is empty), and the mode refuses a CI environment with exit 2 rather than skipping (`:219-221`). `steps=38/38`, `controls=404/404`, `faults=238/238`, `goldens=29`, `gates=119`, `lines=364`, `tests-expected=5042` — the only phase movements are the four entitled growth gates. |
| 12 | The ledger's row count is a stat counted from the file on every run, not a quoted figure. | ✓ VERIFIED | `tools/check/steps/claims.sh:108` prints `handoffs=<n>` from `$hled` with the grammar pass's own row predicate; the gate line carries `handoffs=8` and `awk '!/^#/ && NF' data/expected/handoffs.ids \| wc -l` is 8. The header states why it is a stat and not a check, and why it adds no control. |
| 13 | The id set and every disposition are pinned in both directions, so a missing, extra, duplicated or re-dispositioned row is a named finding. | ✓ VERIFIED | `HANDOFF_IDS='H7-1 H7-2 H8-3 D10 D11 H16-1 H16-2 H16-3'` and `HANDOFF_DISP` names each disposition; the four `claims-handoff-*` controls plus `-row`/`-missing`/`-unreadable` are committed and all 404 controls pass. |
| 14 | Every ledger copy moved in the same commit, so each control still prints only its own intended finding. | ✓ VERIFIED | The good tree's ledger and the four overlays (`id-missing`, `id-duplicate`, `id-extra`, `disposition`) each carry the three rows; `claims-handoff-row` is untouched; `run-controls.sh` prints `controls=404/404` with no toy-drift, good-failed, token-missing, exit-zero, inventory or orphan finding in my gate run. |
| 15 | Nothing about the rows is a result: an answer is `evidence: none` either way, no row is a pass, no claim may cite one, and `D10`/`D11` stay `not-performed`. | ✓ VERIFIED | The ledger's header states the rule and the packet repeats it in "The one rule to carry away" and "These rows are never a result"; every row's last two tokens are literally `evidence: none` (an awk check for any other shape returns 0 rows); `D10`/`D11` are `not-performed`; `data/claims.json` cites none of them. |
| 16 | PKG-06 is `Complete` in the same commit as the three registry rows and the thirteen toy lines, so neither `important-gates-unknown-id` nor `-complete-uncovered` can fire. | ✓ VERIFIED | `.planning/REQUIREMENTS.md:46` `- [x] **PKG-06**`, `:112` `\| PKG-06 \| Phase 16 \| Complete \|`, and the coverage paragraph. `release-posture control files-unlisted-binary-so PKG-06`, `… -dylib PKG-06`, `release-posture mutant host-so-absent PKG-06` are in `data/expected/important-gates.ids`, each resolving in `data/expected/controls/rights.ids` / `data/expected/pkg-selftests.ids`; `gates=119 lines=364` with `rows-pending=0`. Exactly **10** toy registries carry `toy-phase16 fault GOOD_FAULT PKG-96` and exactly **3** toy requirement records carry `\| PKG-96 \| Phase 16 \| Complete \|` (13 one-line additions, 0 deletions on those paths in `298be25`). |
| 17 | The three rows are pointers to mechanisms that already exist: `steps` and `faults` do not move. | ✓ VERIFIED | `steps=38/38`, `faults=238/238`, `fault-runs=238`; the Phase 16 block in `tools/check/steps/important-gates.sh:81-91` says "zero new faults, zero new steps" and records why the `release-assets` `so-member` case and the throwaway probe are deliberately not rows. |
| 18 | The closing gate line is measured under `sh` and `dash` and is byte-identical, and the container leg ran once with exactly the four documented host tokens differing. | ✓ VERIFIED | My `sh` and `dash` runs both exit 0 with `cmp`-clean final lines at **1,520 bytes**, zero `^-- FAIL` lines. The container leg I verified from the retained `build/gate-linux.log` (one `gate: PASS`, final line 1,512 bytes) rather than re-running it: a second container run would overwrite the only copy (lesson 18), and the log's own tree state (`tracked=2787`, `gates=119`, `handoffs=8`) is the phase's close. Its four differences are exactly `cc=GNU`, `core-imports=5`, `archive=sha256`, `refs-verified=0/4`; the byte arithmetic closes (1,520 − 7 for `AppleClang`→`GNU` − 2 for `7`→`5` + 1 for `members`→`sha256` = 1,512). No `ivs-` container, image or volume survives (`docker ps -a --filter name=ivs-` empty, `docker images` has no `ivs-` repository), `hygiene --tree` exits 0, and `git ls-files` holds no refused binary extension. |
| 19 | The phase boundary records **exactly one** checkpoint verdict in the section 2.3 vocabulary, with the medians recomputed by arithmetic from `.planning/ci-runs.tsv`. | ✓ VERIFIED | Recomputed here from the last five rows (`45723269901`, `45747709457`, `45751979661`, `45778926664`, `45779277467`): `cpu_s` 352, 359, 366, 368, 441 → **366**; `build_s` 33, 34, 34, 35, 38 → **34**; `gate_s` 115, 115, 120, 122, 145 → **120**, against the frozen 361 / 34 / 118. `discharged-within-allowance` is unreachable (366 > 361 + 0.4 = 361.4, by 4.6); `lever-required` is unreachable (the cumulative priced total is 2.0 + 1.3 + 1.5 + 0.4 = **5.2 of 21**, and the phase's actual addition is 0.015 against the 0.4 it priced — 0.385 under, so no line overran); `reprice` is therefore forced and is the single word in `.planning/CI-EFFICIENCY.md:2123` and `.planning/STATE.md:791`, with `steps=38/38` recorded as the invariant. **Note on wording:** §2.3 describes `reprice` as "the median moved more than the priced addition **with an unchanged token line**"; this phase's token line did move, by the four growth gates it was entitled to and priced for. The substance the rule names — the median moved for a runner reason while the gate content is exactly what the plans predicted — holds, and the other two words are arithmetically out of reach, so the verdict is correct; a stricter reading of the clause would leave no word available at all, which is a rule-wording observation for the next milestone, not a defect in this record. |
| 20 | **16-05 Task 2.** `16-VALIDATION.md` is signed off in the required shape, and the phase SUMMARY states plainly what the milestone can and cannot now claim and names the handoffs that remain. | ✗ FAILED | The SUMMARY half is met: `16-05-SUMMARY.md` names `D10`, `D11`, `H7-1`, `H7-2`, `H8-3`, `H16-1`, `H16-2`, `H16-3`, says plainly that no host artifact, shared object, content or runner installer ships, that nothing reaches `measured-hardware`, that `CL-0002` is the only `established` claim and that the rows are not evidence — all of which I checked against `data/claims.json` (29 claims, 28 provisional, one `source-expectation` `established`; `CL-0029` `deterministic-repeatability`/`provisional`) and the ledger. The record half is **not** met: see the `gaps` entry — the Manual-Only table says "none" (H16-2, D10, D11 absent), 8 of 10 Threat Ref cells name another plan's register ids, 0 Wave 0 bullets are resolved in the required form, the Approval line carries an abbreviated token list rather than the measured line (full-line grep: 0 hits), and the runtime row is still the seeded `~72 s / 66 s` estimate against the measured 104/204/793 s. |

**Score:** 19/20 truths verified (0 present, behavior-unverified; 0 overrides).

### Required Artifacts

| Artifact | Expected | Status | Details |
|----------|----------|--------|---------|
| `tools/gate.sh` | the declared overlap pair, the `gate-overlap` measurement line, join on both paths, trap kill, safety net | ✓ VERIFIED | `overlap_bg`/`overlap_fg` declared once, checked against the producer list, `gate-overlap pair=c-faults+c-tests` printed before any step, `record_rc`/`t.<name>` written exactly as the sequential loop did, replay untouched. |
| `tools/check/steps/c-tests.sh`, `c-faults.sh` | the byte-identical inline build lock around every `cmake --build` | ✓ VERIFIED | `cmp`-equal between `# build-lock begin`/`end`, bounded polled `mkdir` in the per-run results directory, released on both paths and the caller's traps. |
| `tools/release-rehearse.sh` | `--source-route` performs the three owner-route commands against the archive, and `usage()`'s line range re-derived | ✓ VERIFIED | `sed -n '3,49p'` matches the 47-line header; `--help` prints the whole contract (`Local rehearsal of the release job …` through the `Exit:` line) and no function body; the route limb is real (see truth 1). |
| `docs/owner-images.md` | "The route from the source archive, in three commands", citing section (j) | ✓ VERIFIED | `:55-92`; three commands as commands, the base/word-order contract, `refs/`, the container as derived material, the frontend's ownership of picture and sound, the pointer to section (j), and the one-byte refusal. No untagged claim word (the gate's docs step is green). |
| `.planning/ops/RELEASE-OPS.md` | section (j) transcript; section (i.1) recomputation | ✓ VERIFIED | (j) at `:163` with the four printed lines verbatim, the archive and prefix, the three "must not infer" sentences and the `not run` rule; (i.1) at `:101` as a 63-added/0-deleted append. |
| `data/expected/handoffs.ids` | eight rows, three new, all `evidence: none` | ✓ VERIFIED | `H16-1..H16-3 handed-off`, `D10/D11 not-performed`, header pointer sentence added rather than rewriting the five-id sentence. |
| `docs/handoffs.md` | three packet sections in the page's shape | ✓ VERIFIED | `H16-1` (shim target `ivs_retro_shim`, object digest/size, the harness's nine `digest.*` lines), `H16-2` (the route + section (j), the report as identity, no derived file), `H16-3` (two wraps at `hi-first`/`lo-first`, base required, nothing auto-detected), plus the closing "never a result" paragraph. |
| `data/expected/important-gates.ids` | three `release-posture` rows with per-surface comments | ✓ VERIFIED | `:598-620`; each row resolves in its inventory; the two deliberate omissions are recorded with their reasons. |
| `.planning/REQUIREMENTS.md` | PKG-06 `Complete` (checkbox, traceability row, `Last updated`, coverage) | ✓ VERIFIED | All four moved in `298be25`; no `Pending` PKG-06 remains; PKG-06 is Phase 16's only requirement, so nothing is orphaned. |
| `tools/check/steps/important-gates.sh` | the Phase 16 block | ✓ VERIFIED | `:81-91`, naming PKG-06, the one new gate name, the three rows, "zero new faults, zero new steps" and both omissions. |
| `.planning/phases/16-…/16-VALIDATION.md` | the signed record plan 16-05 Task 2 specifies | ✗ STUB | Signed and frontmatter-complete, but four sections are the seeded template: Manual-Only "none", wrong Threat Ref cells (8/10), 0 resolved Wave 0 bullets, abbreviated Approval line, estimated runtime. See the `gaps` entry. |
| `.planning/STATE.md`, `.planning/ROADMAP.md`, `.planning/CI-EFFICIENCY.md` | the closing line once, the phase tokens, one verdict, the plan list | ✓ VERIFIED (ROADMAP row stale) | STATE carries the 1,520-byte line once, the tokens, the `reprice` bullet with its arithmetic and the `not-recorded` hosted row with its reason; CI-EFFICIENCY carries `phase16: reprice (…)`; ROADMAP's Phase 16 section reads `5/5 plans executed` with five entries — but its v0.3.0 progress table row is stale (gap 2). |

### Key Link Verification

| From | To | Via | Status | Details |
|------|----|-----|--------|---------|
| `tools/gate.sh` | `tools/check/steps/c-tests.sh` / `c-faults.sh` | background partner started with the same redirection, foreground member, join writing `rc.`/`t.` | ✓ WIRED | Proved live: with the overlap disabled and enabled on one tree, the two runs' final lines are byte-identical and the filtered logs differ by 0 lines. |
| `docs/owner-images.md` | `.planning/ops/RELEASE-OPS.md` section (j) | the route's rehearsal note cites the section by path and letter | ✓ WIRED | The pointer resolves (`## (j)` exists exactly once) and every figure it points at reproduced in my run. |
| `tools/release-rehearse.sh` | `data/golden/demo-lamp-tone-pass.digests` | the filtered-report comparison, unchanged, still last in the limb | ✓ WIRED | `124 of 124 compared`, report 131 lines, `rehearse: ok`. |
| `tools/release-rehearse.sh` | the archive's `tools/owner-run.sh` and `ivs_flatwrap` | paths relative to `$work/route/$prefix` | ✓ WIRED | `[ -x "$wrap" ]` and `[ -f "$run" ]` guards; both are the extracted archive's, never the repository's. |
| `data/expected/handoffs.ids` | `tools/check/steps/claims.sh` | grammar pass, then the id set and each disposition against the two pinned lists; `handoffs=` from the same read | ✓ WIRED | `handoffs=8` equals the file's row count; a missing/extra/duplicate/re-dispositioned row is a named finding proved by the four controls. |
| `data/expected/handoffs.ids` | `docs/handoffs.md` | the ledger is the inventory, the packet the instruction sheet | ✓ WIRED | One section per row; the page states no count. |
| `.planning/REQUIREMENTS.md` | `data/expected/important-gates.ids` | the traceability table is the derived covered set | ✓ WIRED | `gates=119 lines=364`, `rows-pending=0`; both refusal directions are exercised by committed controls. |
| `.planning/ci-runs.tsv` | the checkpoint bullet | last five rows, each window sorted, middle value printed | ✓ WIRED | Recomputed here: 366 / 34 / 120, identical to the record. |

### Data-Flow Trace (Level 4)

| Artifact | Data Variable | Source | Produces Real Data | Status |
|----------|---------------|--------|--------------------|--------|
| `tools/check/steps/claims.sh` | `handoffs` | `awk` over `data/expected/handoffs.ids` on every run | yes — 8 rows, equals the file | ✓ FLOWING |
| `tools/check/steps/important-gates.sh` | `gates`/`lines` | the registry file parsed on every run | yes — 119/364, re-derived | ✓ FLOWING |
| `.planning/STATE.md` checkpoint bullet | `cpu_s`/`build_s`/`gate_s` medians | the last five `.planning/ci-runs.tsv` rows | yes — 366/34/120, recomputed | ✓ FLOWING |
| `.planning/ops/RELEASE-OPS.md` §(j) | archive name, prefix, golden count, wrap digest, outcomes | the rehearsal's own printed lines | yes — reproduced by re-running the mode, digest included | ✓ FLOWING |
| `docs/handoffs.md` H16-1 | object digest and the nine `digest.*` lines | the release preset's build and `tests/host/ivs_host_harness.c` | yes — the command the packet prints is the one the plan ran | ✓ FLOWING |

### Behavioral Spot-Checks

| Behavior | Command | Result | Status |
|----------|---------|--------|--------|
| The full gate is green on this tree | `sh tools/gate.sh` | exit 0, `gate: PASS steps=38/38 … gates=119 lines=364 handoffs=8`, 1,520-byte final line | ✓ PASS |
| The same line under a second shell | `dash tools/gate.sh` | exit 0, `cmp`-clean against the `sh` tail, 1,520 bytes | ✓ PASS |
| The lever changes no evidence | gate with `overlap_bg`/`overlap_fg` emptied, then restored, same tree back to back | final lines byte-identical; filtered whole logs 0 diff lines; both step blocks equal | ✓ PASS |
| The route runs from the source archive | `sh tools/release-rehearse.sh --no-gate --release --source-route` | `rehearse: ok`, golden 124/124, the three `owner-route:` lines, mutation refused exit 14 | ✓ PASS |
| A committed shared object is refused by name | probe `probe.so`/`probe.txt` staged, `files.sh` over `git ls-files` | one finding for `probe.so`, none for `probe.txt`, no pass line, tree clean after | ✓ PASS |
| The claim ledger's posture | Python read of `data/claims.json` | 29 claims, 28 provisional, one `established` (`CL-0002`), none `measured-hardware`, `CL-0029` `deterministic-repeatability`/`provisional` | ✓ PASS |
| The container leg | read of the retained `build/gate-linux.log` (not re-run) | `gate: PASS`, 1,512-byte line, exactly four documented token differences | ✓ PASS (from the retained log) |
| Controls | `sh tools/check/run-controls.sh` (inside the gate) | `controls=404/404`, no toy-drift or inventory finding | ✓ PASS |

### Probe Execution

| Probe | Command | Result | Status |
|-------|---------|--------|--------|
| committed-direction shared-object probe | `git add -f probe.so probe.txt` → `files.sh` over `git ls-files` | `-- FAIL files-unlisted-binary: probe.so`, `probe.txt` absent, no pass line, `binscan.sh` silent, both files removed, `git status` empty | ✓ PASS |

### Requirements Coverage

| Requirement | Source Plan | Description | Status | Evidence |
|-------------|-------------|-------------|--------|----------|
| PKG-06 | 16-01 … 16-05 (all five declare it) | A tracked shared object is refused by name, the installed package stays eleven files, and the release-decision line states that no shared object, host shim or content is shipped. | ✓ SATISFIED | Committed-direction probe reproduced (truth 4); `prefix-files=11` and the eleven-path inventory (truth 5); the line verbatim in `RELEASE-OPS.md` (i)/(i.1), `docs/packaging.md:123`, `PROJECT.md`, `ROADMAP.md`; `binaries-listed=0`; `so-member` and `host-so-absent` live. `.planning/REQUIREMENTS.md:112` reads `Complete`; no PKG-06 remains `Pending`; Phase 16 carries no other requirement, so none is orphaned. |

### Anti-Patterns Found

| File | Line | Pattern | Severity | Impact |
|------|------|---------|----------|--------|
| `.planning/ROADMAP.md` | 519 | `TBD` in a phase-modified file, contradicting the same file's Phase 16 section (`5/5 plans executed`) | 🛑 Blocker | The status surface a reader lands on says the phase is "Not started"; a `0/TBD` cell with no follow-up reference is the self-evidencing debt-marker case. Gap 2. |
| `.planning/phases/16-…/16-VALIDATION.md` | 79-88, 24, 66 | seeded-template remnants (Manual-Only "none", placeholder Threat Refs, estimated runtime, abbreviated Approval) | 🛑 Blocker | The validation contract does not describe this phase's manual items or its measured line. Gap 1. |
| `.planning/ROADMAP.md` | 414 | `**Plans**: TBD` | ℹ️ Info | Belongs to the dropped Phase 12 inside the archived v0.2.0 `<details>` block; pre-dates this phase and is not a Phase 16 claim. Worth recomputing at the milestone close, out of this phase's scope. |
| `16-05-SUMMARY.md` | — | reconciliation is prose, not the required one-row-per-token table | ⚠️ Warning | Form only; the substance was re-derived in this verification. See `advisory`. |
| `tools/check/steps/*.sh`, `tools/gate.sh`, `tools/release-rehearse.sh` | — | `XXXXXX` in `mktemp` templates | ℹ️ Info | False positive of the debt-marker scan; not a marker. |

### Human Verification Required

None. Every phase behavior has an automated check and each was exercised in this session; the owner-only items
(`H16-1`, `H16-2`, `H16-3`, `D10`, `D11`) are ledger rows with procedures, are not evidence and are not
criteria, so they are not verification items. (Their absence from `16-VALIDATION.md`'s Manual-Only table is
gap 1, a record defect — not a request for a human test.)

### Gaps Summary

The phase's **goal is achieved**: all five ROADMAP success criteria hold, and PKG-06 is `Complete` on
evidence I re-derived rather than read — the owner route genuinely runs from a freshly built source archive
with the transcript's own container digest, the one-byte mutation is refused at load while `exec:` stays `-`,
the three handoff rows are `evidence: none` and cited by nothing, a committed shared object is refused by
name with a witness pair, the package is eleven files, the release-decision line reads as required, and the
concurrency lever is a one-commit, cost-only change whose token line I proved byte-identical on one tree by
running the gate both ways.

Two **record** gaps block `passed`, and neither is a re-do of any work:

1. `16-VALIDATION.md` was signed but not written — four sections are still the seeded template, including the
   Manual-Only table (which says "none" for a phase whose exit is defined by three handoff rows) and the
   Approval line (an abbreviated token list rather than the measured 1,520-byte line). Eight of its ten
   Threat Ref cells name another plan's threat ids, and its runtime row is the seeded estimate against the
   close's measured 104/204/793 s. Phase 15's `15-VALIDATION.md` is the in-repo precedent for the shape.
2. `.planning/ROADMAP.md`'s v0.3.0 progress table still says `16 … 0/TBD | Not started` — and its closing
   narrative still calls Phase 15 "2 of 3 plans executed … droppable" — while the same file records `5/5`
   and `3/3 Complete`. That is the "quoted figure that disagrees with the file" defect AGENTS.md names, on
   the first surface a reader opens.

Both are one-file record fixes; a `--gaps` plan for this phase should touch nothing else. If the owner judges
the validation record's omissions immaterial, the alternative is an explicit override entry rather than a
silent pass — the exact YAML is:

```yaml
overrides:
  - must_have: "16-VALIDATION.md is signed off with ten task rows, the Wave 0 bullets resolved, the five manual items and an Approval line carrying the measured gate line"
    reason: "<why the seeded sections are acceptable for this phase>"
    accepted_by: "<name>"
    accepted_at: "<ISO timestamp>"
```

No token, count or gate result needs to change for either fix: `steps=38/38`, `controls=404/404`,
`faults=238/238`, `goldens=29`, `gates=119`, `lines=364`, `handoffs=8`, `prefix-files=11`,
`claims=29 provisional=28` and `tracked` are all where the plans predicted, and the only byte-count movement
is `16-VALIDATION.md`'s own text.

### Note for the milestone audit

**Is the phase SUMMARY's "what the milestone can and cannot claim" statement accurate against the tree?**
Yes, with one imprecision. Checked item by item: `CL-0029` is `deterministic-repeatability`/`provisional` and
its artifact is `tests/host/ivs_host_harness.c` (not a handoff, a `build/` path, a `.wav`, a `.ppm`, an
`.ivsn` or a bench `.tsv`); `CL-0002` is the only `established` claim (29 claims, 28 provisional); no claim is
at `measured-hardware`; nothing is installed beyond the eleven paths and no shared object is in the release;
`D11` is `not-performed` and the route's `exec:` label is `-`; the eight remaining questions are the ledger's
rows with procedures and no evidence. The imprecision: the SUMMARY says "D10, 47 items handed off" while
`D10`'s ledger row is `not-performed` — the 47 items are inventoried in `data/expected/d10-measurements.ids`
(`d10-items=47` on the gate line) and offered, but the row's own disposition is "needs hardware no tool on
this host can supply", not `handed-off`. Worth a word at the close; it does not change what the milestone can
claim. The SUMMARY's nine-deviation count also matches `.planning/WINDOWS.md` (rows 5-13, all waived).

---

_Verified: 2026-11-26T11:51:12Z_
_Verifier: Claude (gsd-verifier)_

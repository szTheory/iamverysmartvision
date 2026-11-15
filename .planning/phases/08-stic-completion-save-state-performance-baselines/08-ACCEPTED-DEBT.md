# Phase 8: accepted tech debt and deferrals

**Written:** 2026-11-15 by plan 08-20 (D-34). **Purpose:** the milestone audit reads this file as the list of things that are known, not fixed on purpose, and why. It changes no requirement mark and no claim: every claim stays provisional, D10 (physical measurement) and D11 (owner-local real EXEC and GROM runs) are open owner hard stops and were not performed, and nothing here is evidence for anything.

Labels: [OBS] read or run in this plan, [INF] inference, [REC] recommendation. Paths are repository-relative.

## 1. The six draft validation files (Phases 2 to 7)

Each file is a per-phase validation contract seeded by plan-phase: a test infrastructure table, a sampling rate, a per-task verification map, a Wave 0 list, a manual-only list and a sign-off block. [OBS] Each is still `status: draft` with `wave_0_complete: false` and `Approval: pending`; `nyquist_compliant: true` is set in all six. [OBS] `.planning/phases/01-evidence-profile-and-rights-foundations/01-VALIDATION.md` is `validated` and is not one of the six.

The reason a re-run of validation adds no evidence now is the same for all six [INF]: every per-task check named in the map ran in its own plan's verify step, its result is recorded in that plan's SUMMARY and in the phase VERIFICATION record, and `sh tools/gate.sh` re-runs the whole inventory (every test id, named fault and negative control) under sh, dash and the Ubuntu container at every plan end. A validation pass would restate those results in a second format.

| File | Status | What the draft was meant to hold | Why a re-run adds nothing now | Where the evidence is |
|------|--------|----------------------------------|-------------------------------|-----------------------|
| `.planning/phases/02-cp-1610-cpu-core/02-VALIDATION.md` | draft | the CPU phase map from the opcode inventory, vectors and named-fault controls to checks, and the Wave 0 test infrastructure | the Wave 0 items (presets, harness, fault registry) exist and run in every gate; the opcode inventory is an exact-count gate step | `.planning/phases/02-cp-1610-cpu-core/02-VERIFICATION.md` and the gate |
| `.planning/phases/03-memory-map-content-loading-and-refusal/03-VALIDATION.md` | draft | the map for the memory map, content loading and the refusal matrix | the refusal matrix and the decode-table dump are covered by test ids and gate steps | `.planning/phases/03-memory-map-content-loading-and-refusal/03-VERIFICATION.md` and the gate |
| `.planning/phases/04-headless-runner-and-installed-consumer/04-VALIDATION.md` | draft | the map for the runner, the digests, the relocated installed consumer and the CI mirror of the local command | the `runner-process`, `golden` and `pkg-install` steps run every item of the map at every gate; the consumer is replayed against the moved prefix | `.planning/phases/04-headless-runner-and-installed-consumer/04-VERIFICATION.md` and the gate |
| `.planning/phases/05-hand-controllers-and-psg-sound/05-VALIDATION.md` | draft | the map for controllers and PSG sound, with its requirement-to-check table | the controller and PSG suites, the second replay of the consumer and the write-log goldens run in every gate | `.planning/phases/05-hand-controllers-and-psg-sound/05-VERIFICATION.md` and the gate |
| `.planning/phases/06-stic-video-first-pass/06-VALIDATION.md` | draft | the map for the STIC first pass (windows, timing, raw frame, cards, inspection), with its requirement-to-check table | the frame oracles, the `video` step and the third replay of the consumer run in every gate | `.planning/phases/06-stic-video-first-pass/06-VERIFICATION.md` and the gate |
| `.planning/phases/07-first-visible-original-scenario/07-VALIDATION.md` | draft | the map for the first visible original scenario (the checks V1 to V20) | the V-rows are tabulated with ids and landing commits in the verification record and every id runs in the gate | `.planning/phases/07-first-visible-original-scenario/07-VERIFICATION.md` (table 1b) and the gate |

[OBS] `.planning/phases/08-stic-completion-save-state-performance-baselines/08-VALIDATION.md` is Phase 8's own file and is also `draft`; it is outside the six of D-34 and is left to the close record of plan 08-21.

## 2. The carried-forward items of the STATE Deferred Items table

The table is in `.planning/STATE.md`; plan 08-21 refreshes it with the close record. The four rows are accepted as carried, with the status the table gives them [OBS]:

| Item (by name) | Status | Why accepted |
|----------------|--------|--------------|
| Gate hardening IN-01 to IN-05 of Phase 1 (whole `SRC-` token match, fail glossary and sources on zero documents, require each step's pass line, anchor manifest outputs to their fixture directory, runner-selftest coverage of the remaining tokens, the denylist grep status read as no hit, the `--tree` grep-error control) | open, low severity (`.planning/phases/01-evidence-profile-and-rights-foundations/01-REVIEW.md`, `01-SECURITY.md` residual risks) | [INF] each is a hardening of a check that already fails for its intended reasons in its controls |
| Gate speed: run negative controls in parallel if the hosted gate step passes about 30 s | watching | the hosted gate is measured every run (`.planning/ci-runs.tsv`, `.planning/CI-EFFICIENCY.md`); the lever order is recorded there |
| Gate hardening: `memmap-gen-*` controls of `cmake/IvsMemmapTable.cmake` | open | needs a dedicated toy memmap profile; [INF] the generator output is checked by the decode-table dump against the sourced literal map |
| Evidence: C11 (no AY-3-8914 data sheet found), CP-1600 against CP-1610 inference (C31), same-model second reader | open, carried by the discrepancy log | resolvers are a primary data sheet or a D10 capture; every claim that depends on them is provisional |

## 3. Open review items of Phases 4 to 7, by name

The review records are `.planning/phases/04-headless-runner-and-installed-consumer/04-REVIEW.md`, `.planning/phases/05-hand-controllers-and-psg-sound/05-REVIEW.md`, `.planning/phases/06-stic-video-first-pass/06-REVIEW.md` and `.planning/phases/07-first-visible-original-scenario/07-REVIEW.md` (fixed). The items below are the ones their dispositions list as deferred or carried [OBS]; none changes behaviour on the one registered profile.

| Phase | Item | Status |
|-------|------|--------|
| 4 | IN-04 clock failure at the wall start | deferred: a broken monotonic clock is not seen on supported hosts |
| 4 | IN-06 a POSIX-identifier denylist and a `C_EXTENSIONS` flag for runner-scan | deferred: the container gate (`tools/gate-linux.sh`) catches the class before a push |
| 4 | IN-09 `ARCH_INDEPENDENT` | deferred: D-33's explicit choice; revisit when a 32-bit consumer is in scope |
| 4 | IN-10 golden `.expect` and dev-versus-release coverage | deferred: `runner-process` covers both today |
| 4 | IN-11 impossible-state guards | deferred: documented belts, an owner scope decision |
| 5 | IN-01 port direction bits read through the R7 read mask | deferred: both masks are the same on the one profile |
| 5 | IN-03 `IVS_RUN_REQUEST_V1_SIZE` is 64 on every ABI | deferred: a pinned ABI statement; revisit if a 32-bit host is scoped |
| 5 | IN-05 operator outputs created with mode 0644 | deferred: a policy choice for the owner |
| 5 | IN-06 two unreachable defensive branches (`period == 0`) | deferred: cosmetic |
| 5 | residuals of `05-SECURITY.md`: operator-path trust (symbolic links followed, `--update-golden` truncates by design), watchdog orphan after a kill of the harness, no named control for the measured-tier claim rule, the event-array time-of-check gap, the `refs-verified=3/4` cache verifier, two guarded test skips | carried, non-blocking |
| 6 | IN-02 profile values hard-coded in the core, IN-03 runner slot, PPM and chain literals | deferred: one profile exists |
| 6 | residuals of `06-SECURITY.md`: operator-path trust carried from Phases 4 and 5, an empty `--frame-ppm` file when no frame was delivered, audio and write-log outputs at mode 0644 | carried, non-blocking |
| 7 | the owner-run helper does not detect a symbolic link inside `refs/` that points elsewhere in the work tree, the SKIPPED line does not carry the words "D11, no claim", an erratum note on the gate-window starts of the derivation section (d) | optional, listed in `07-VERIFICATION.md` section 4 |

## 4. The three-source check

[OBS] The check is a throwaway script kept in the untracked build tree (never committed). For every requirement that `.planning/REQUIREMENTS.md` marks Complete for Phases 1 to 7 it lists the SUMMARY files of that phase whose frontmatter `requirements-completed` value (the text inside the brackets, never the trailing comment) names it and whether the phase VERIFICATION record names it.

- **Requirements checked:** 45 (the 50 of v1 minus VID-05, STA-01, STA-02, PRF-01 and REL-01, which are Phase 8).
- **After the backfill of this plan:** every one of the 45 is listed by at least one SUMMARY of its phase and named by its phase VERIFICATION record; **findings: 0**.
- **Before the backfill** (the same check over the SUMMARY files as committed at the plan head): 11 requirements had no SUMMARY, namely API-03, AUD-01, AUD-02, INP-01, INP-02 and INP-03 of Phase 5 and VID-01, VID-02, VID-03, VID-04 and VID-06 of Phase 6. The 39 backfilled lines closed that gap for the milestone audit; no requirement is partial by omission.
- **Reading rule for the backfilled lines [INF]:** the value is the plan's own `requirements` field verbatim, so a plan that served a requirement without closing it lists it (for example the Phase 7 plans 07-01 to 07-06 list SCN-01, which plan 07-12 closed). The closure evidence is the phase VERIFICATION record, as each inserted line says; a `requirements-completed` line is an index for the audit, not a claim. No requirement mark moved.

## 5. Found by the sweep, not fixed

- `core/src/machine/machine_digest.c` (an internal comment near the top) still says the composite is version 2 since plan 05-03; it is version 3 since plan 06-05 (`IVS_DIGEST_COMPOSITE_VERSION` in `core/src/machine/machine.h` says so). The plan edits comment text in the public header and the call record only and forbids a change to any source file, so the internal comment is left. [REC] correct it in the next commit that touches the file. Comments in `tests/machine/machine_digest.c` that say "composite version 2" describe the historical pins of Phase 5 and are true as written.

## 6. Deferred out of Phase 8

| Item | Reason | Recorded in |
|------|--------|-------------|
| The snapshot file options (`--snapshot-out`, `--snapshot-in`) and any runner file persistence | D-18 made them an optional last plan and the budget went to the corruption family, the hook and the benchmarks; no consumer needs a file; the core does no file IO; the policy that would govern them is decided (runner-only, explicit path, refused for `content user`, `.ivsn` rights-bearing by name) | `08-CONTEXT.md` D-18 and D-19, `08-17-SUMMARY.md` |
| 192-line video publication | would add about two plans and re-bless every frame digest; no evidence reverses D8, and the reversal is one profile field | `08-CONTEXT.md` deferred section, `.planning/PROJECT.md` D8 row |
| libFuzzer runs in CI | the target `tests/fuzz/fuzz_snapshot.c` exists behind the existing option, outside the gate and every inventory; a CI job needs a failure class that no gate step catches (AGENTS.md), and Apple clang lacks the runtime; TOOL-05 | `08-CONTEXT.md` D-20, `08-15-SUMMARY.md` |
| SHA-256 frame-hash optimization | about 43 percent of a verified benchmark run is SHA-256 over every frame, measured once on a loaded host [INF]; the digest check is the evidence design and no consumer states a frame-rate need | `08-CONTEXT.md` D-25, `docs/bench.md`, `.planning/PROJECT.md` row OQ11 |
| D10 physical measurement | an owner hard stop that needs hardware; 47 items in `data/expected/d10-measurements.ids` are handed off and no claim uses the measured tier | `.planning/PROJECT.md` D10 row |
| D11 owner-local real EXEC and GROM runs | an owner hard stop that needs the owner's files; not performed, no digest recorded | `.planning/PROJECT.md` D11 row, `docs/owner-images.md` |
| A history rewrite | an owner-only hard stop; six old planning commits trip `hygiene --history` on a path shape, a rewrite would not purge pull-request refs; the disposition is acceptance without rewrite, pinned by commit, with a release check that fails on any new finding | `08-CONTEXT.md` D-33, `08-02-SUMMARY.md` |
| Name clearance | an owner item; the working name is not trademark-cleared and every release text says so | `README.md`, `CHANGELOG.md`, `.planning/PROJECT.md` |
| The applied-event count of an instance has no public getter | an API addition (a reasons or struct-size change); a host keeps the count its run calls reported next to the snapshot | `08-17-SUMMARY.md` open item 1 |
| Two pinned acceptances of the state wave: a STIC register word with a bit outside its write mask (an INV-7 candidate) and machine time not tied to a unit boundary | each would change the core and the invariant function; both are named in the claim note of CL-0028 | `08-17-SUMMARY.md` open item 3 |
| A non-default witness for `stic.late_grants`, and the exemption of `obs.input.words` while the next event lies inside the window | needs a designed program with a non-interruptible stretch of at least 1,026 cycles | `08-17-SUMMARY.md` open item 4 |
| The golden-writing commands of the two benchmark goldens are not listed in `docs/runner.md` | the list there was already incomplete; the commands are in `08-18-SUMMARY.md` | `08-19-SUMMARY.md` open item 3 |
| The benchmark baseline is informational | `data/bench/baseline-darwin-arm64.tsv` was taken on a host that was not idle and says so on its idle line; an idle-host record replaces it by a reviewed change; the timed path has run on macOS arm64 only | `08-19-SUMMARY.md` open items 1 and 2 |

## 7. What this file does not do

It does not mark or unmark any requirement, does not add or change any claim, does not change any gate count, and does not record any physical or owner-local result. A requirement stays complete only on the terms its close-out note in `.planning/REQUIREMENTS.md` states.

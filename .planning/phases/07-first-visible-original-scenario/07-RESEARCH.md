# Phase 7: First Visible Original Scenario - Research

**Researched:** 2026-11-13
**Domain:** project-internal (C17 tool, sh/CMake gate steps, runner grammar, CP-1610 listings); no external package.
**Confidence:** HIGH for repo facts; MEDIUM for timing numbers (from the discuss notes, not re-measured).
**Labels:** [OBS] read or computed in the repo this session (path/line where a discrete value matters); [OBS-note] measurement carried from a `07-DISCUSS-*.md` note, not re-run; [INF] inference; [REC] recommendation; [UNK] open. No package is installed, so no registry claim exists.

<user_constraints>
## User Constraints (from CONTEXT.md)

Size deviation: the 35 KB cap forbids re-copying 25 KB of decisions. Authoritative text: `.planning/phases/07-first-visible-original-scenario/07-CONTEXT.md` (D-01..D-36, R-1..R-5); the planner must read it. Digest, no alternatives researched:

D-01..D-08 program "Lamp Tone" (`FX-demo-lamp-tone-1`, scenarios `demo-lamp-tone-{pass,idle,shift,swap}`), raw active-low lamps, PSG per port, RAM `$02F0..$02F5` predicate, never halts, six events, `run-to 806568`, GRAM first, polling fallback. D-09..D-15 `FX-boot-stub-2` / `boot-stub-2` token, `$5000`/`$5004`, stub `JD $5004`, tick-driven ISR, strobe first, re-arm at or after frame cycle 3,777, RAM layout. D-16..D-19 `ivs_asm` (one C17 file, hard stop 1,200 lines, never installed), row token `DATA`, `.asm` + listing committed, suite `asm`. D-20..D-27 IVSS/`ivsr 3` frozen, `video-frame-pair-region`, constant-level audio oracle, oracle first + blind read + judge, shape lint, mutants + `fails=`, fourth consumer replay, CL-0023/CL-0024. D-28..D-33 manifest `recipe`, recipe-source hashing, `names` rule, `important-gates` accepts SCN-01/02, `docs/owner-images.md`, `redistributable: no` manifests, `owner-exec-title` `skip-ok` (`tests-skipped` 2 -> 4), SKIPPED lines, `tools/owner-run.sh`, runner refuses audio/log outputs for user content, D11 not performed, source-only release. D-34..D-36 CI trigger check first, waves W0..W7 (serial, <= 5 tasks / 8 faults per plan), hard stops (no D10/D11 run, no third-party bytes).

### Claude's Discretion (verbatim)
- Exact lamp layout, card colours and row placement within the 20x12 matrix, provided the expected region is a declared rectangle.
- `EQU` and `DECLE` spelling in `ivs_asm`, whether mutants name addresses by label, how the gate ties the stub's interrupt-entry target to the program address (S-2 plus S-3 open item), exact test counts within the cost budget.
- The consumer replay's id prefix.

### Deferred Ideas (OUT OF SCOPE, verbatim)
- MOB-based sprites and collision in the scenario: Phase 8 (VID-05).
- A reference host frontend that displays the program: v2 (TOOL-04).
- Promoting the four hand-assembled legacy listings to `.asm` sources: only when a listing next needs an edit.
- A reader cartridge for the D10-VID items (Phase 6 hand-off): not required by SCN-01/02; revisit with D10 evidence collection.
- Known EXEC/GROM digests in the repo: not recorded (rights and identity are the owner's); revisit only with documented permission.
- Installing the runner or `ivs_pack` so a release is runnable: a release-packaging decision, not Phase 7.
</user_constraints>

<phase_requirements>
## Phase Requirements

| ID | Description | Research Support |
|----|-------------|------------------|
| SCN-01 | Original rights-clear program runs in CI through the installed consumer, reaches a guest-state progress predicate, shows input changing guest state, video and audio. | F1-F22, defects X1-X10, file inventory, Validation V1-V14, waves W0-W5. |
| SCN-02 | Owner-local EXEC/GROM route documented with identity checks, labelled non-redistributable, kept outside CI (gated on D11). | F13-F15, V15-V19, wave W6; D11 not performed. |
</phase_requirements>

## Summary

Phase 7 touches nearly every gate surface once; each touch has a Phase 4-6 precedent. No CONTEXT decision is contradicted at the mechanism level, but the notes carry ten spec defects (below) to settle before any listing exists. Most load-bearing: the proposed per-label `# label:` listing comment collides with the closed header key set (`listing-header`); D-04's counter address and offset disagree with D-06's 806,568 bound; a `.expect` cannot precede its scenario and program in a green history (golden step); and the `boot-stub-2` token is a runner source change, not "no runner code".

Cost: last-five hosted `r` = 7.68, 9.02, 7.53, 8.02, 8.17, median **8.02** [OBS computed from `.planning/ci-runs.tsv`], so the W0 lever is due. Headroom to the 8.3 trigger is about 11.5 hosted cpu-s (about 16 after the lever) against my summed estimate of 10-19 for the phase: tight; keep the lever order and checkpoints.

**Primary recommendation:** W0 lever first; W1 commits only derivation, frame literals and pure checker tokens (no `data/golden` file); settle X1-X6 before writing the `.asm`; add every check inside existing steps/suites (no new step, job or dependency).

## Architectural Responsibility Map

| Capability | Primary tier | Secondary | Rationale |
|---|---|---|---|
| Program and stub-2 sources | `fixtures/` | `tools/asm` | project-authored text; images only in the build tree |
| `.asm` to listing | `tools/asm` (`ivs_asm`) | listing step | build-only; committed listing is the checked artifact |
| Listing to image | `tools/pack` (unchanged) | CMake `ivs_images` | `--exec/--ivsc` exist |
| Token `boot-stub-2` | runner (`scenario.c`, `exec.c`, `report.c`) | `docs/runner.md` | the symbolic name carries the "not an EXEC" label |
| Oracle literals | `data/video`, docs derivation | `video`/`golden` steps | hand-derived, pure text checks |
| Behaviour proof | `runner-process` (real `ivs-run`) | installed consumer | runner is the record; consumer is the independent replay |
| Owner route | docs, template, helper | runner refusals, `manifests` | machine-readable `no` |

## Standard Stack

No library is added. C17 (`CMAKE_C_EXTENSIONS OFF`, `ivs_warnings`, `ivs_san` in dev) [OBS CMakeLists.txt:25]; CMake floor 3.25 (local 4.4.3, hosted 3.28.3); Ninja 1.13.2; POSIX sh/awk (gates run under `sh` and `dash`); `ivs_test_harness` and shell TAP. **Package Legitimacy Audit:** none, nothing is installed; `libasm` (optional T8) is absent here and stays non-gating.

## Verified repository facts

| # | Fact [OBS] | Source |
|---|---|---|
| F1 | Phase 6 final: `steps=36/36 controls=339/339 faults=221/221 ... tests-expected=4422 tests-executed=4420 tests-skipped=2`. Now: 344 control dirs (345 incl. `_good`), 222 fault ids, 2,216 test ids (x2 presets = 4,432). Re-run the gate for the plan-start baseline. | `06-14-SUMMARY.md`; counts of `data/expected` |
| F2 | Next free: **CL-0023** (`claims.ids` tail `CL-0022`, 22 claims); discrepancy **C106** (last `C105`); d10 items 42. Tiers `playable-guest-progress`, `installed-host-behavior` already exist in `data/tiers.json` and `evidence-tiers.ids`. | data files |
| F3 | `runner-scenarios.ids`: six columns `<id> <exit> <outcome> <class> <skip> <operator-args>`; 22 rows = 22 scenario files; only `needs-user-exec 13 missing-fixture deterministic skip-ok -` is `skip-ok` (line 19). | `golden.sh:89` (`NF != 6`) |
| F4 | `listings.ids` `<id> <kind> <path>` (kinds `boot-stub`, `cart`; ids `boot-stub`, `diag-cpu-1`, `ivs-diag-ctrl-psg-1`, `ivs-diag-stic-1`; header `# id:` must equal it); suite `pack` makes `pack.words-<id>` per line and `pack.trailer-<id>` for carts only; `fixtures.ids` is an exact set of 6 manifest ids. | `pack.ids`, `pack-tap.sh` |
| F5 | Header keys are closed: `set(HEADER_KEYS id kind contract profile window label entry stack)`; a repeated key is `listing-header`. Stub guards: window `1000-101F`, <= 16 non-zero words, rows only from `bootstub-rows.ids` (HLT DIS EIS MVII MVO TSTR B J); `cp1610-rows.ids` has `J`, no `JD`, so a `JD` word is row `J`. | `listing.cmake:77`, `bootstub-rows.ids` |
| F6 | Content kinds `NONE 0, PATH 1, USER 2, BOOT_STUB 3, ZERO_GROM 4` (`ivs_runner.h:123-127`); `boot-stub` parsed at `scenario.c:344`, read via `IVS_RUNNER_BOOT_STUB_FILE "boot-stub.exec"` (`ivs_runner.h:361`, `exec.c:205-207`); only that kind prints `exec:` (`report.c:312`: `project boot stub, not an EXEC sha256=<hex>`); the path form prints `exec: -`. Fuzz invariant `c->kind <= IVS_SCN_CONTENT_ZERO_GROM` (`tests/runner/runner_fuzz.c`) fails for a kind above 4. | runner sources |
| F7 | `runner/main.c` has 0 `IVS_FAULT` sites (`exec.c` 12, `report.c` 3, `scenario.c` 2). User-content refusals today: frame/ppm/video-raw (`main.c:697-707`) and golden (`:710-712`); `--audio-raw`, `--psg-log-out`, `--audio-wav` are not refused. | grep, `main.c` |
| F8 | Images: `tools/pack/CMakeLists.txt` builds `boot-stub.exec` + three `.ivsc`; `tests/CMakeLists.txt:151` feeds `ivs_pack --c-include`; `tests/runner-process-tap.sh:102` copies a fixed 4-name list; `pkg-install.sh` checks 4 files and runs the consumer with 4 args (`tests/installed-consumer/main.c:997`, `if (argc != 5)`). | files |
| F9 | Mutant inventories: `<ID> <scenario> <addr list> <from list> <to list> <clause> <outcome> <exit>` (stic form); `run_mutant` copies **only** `boot-stub.exec` into the mutant dir (`tap.sh:359`) and judges the first failing clause; `ivs_pack --patch` works for `--exec` too (usage, `pack.c:17-23`). | `tap.sh`, `pack.c` |
| F10 | `golden` step: a golden needs its scenario file, a `runner-scenarios.ids` row and the dev report (else `golden-mismatch`); `.expect` asserts == scenario asserts in order (`golden-expect-drift`); a scenario with `content <role> user` may have no golden; dev == release reports apart from `host.calls`. The tier is in the golden header, not the scenario. | `golden.sh` |
| F11 | `tests/expect-inc.cmake` accepts only `halted`, `reg`, single `mem`, `psglog count`/record, `frame displayed|pixel|hash`, `video count`, all `==`; anything else (e.g. `audio at`, `!=`, ranges, `digest`) stops the build; no step boundaries. | its header |
| F12 | `important-gates.sh` hard-codes the accepted requirements (`nreq = split("API-02 ... VID-06", ...)`, line 71, plus header text); SCN-01/02 are rejected until extended. A `mutant` id resolves to `pkg-selftests.ids`, `ci-summary-selftests.ids` or a `<suite>.judge|mutant.<slug>` test id. | `important-gates.sh` |
| F13 | `manifests.cmake` shape-checks `recipe` (lines 210-216) but hashes only `outputs` (285-330); `files.cmake` has no names rule; `manifest-lib.cmake:54-58` already lists recipe sources as accounted files. | files |
| F14 | `tests-inventory.sh` prints only `tests-skipped=<k>`; a skip is legal only with `skip-ok`; the other `skip-ok` ids executed here (skipped=2 is `needs-user-exec` x2). | script, `tests/*.ids` |
| F15 | Text tripwires scan `README.md` and `docs/**/*.md` only: `supports|compatible|passes|accurate|verified|runs` need `[claim:CL-NNNN]` or `[nc]` (`text.sh:34`); bare `clock(s)` fails unless `input clock`, `STIC clock`, `PSG chip clock` or `[nc]` (`glossary.sh:53`); `plays|playable` not yet listed; `.planning/` not scanned. | scripts |
| F16 | `integrity.cmake`: ASCII-only, no CR under `data fixtures tools tests cmake runner core .github`; SPDX required for `tools/(pack|cart|pcm)/*.c|h` (line 74), not for `tools/asm` unless the regex is extended. | `integrity.cmake:58-76` |
| F17 | `c-build.sh:43` checks tracked sources against four generated lists; the toy tree `controls/_good/tree` mirrors them (control `c-build-unlisted-pcm-source`); a `tools/asm` list needs a fifth entry and a toy counterpart. | `c-build.sh` |
| F18 | Controls: `tools/check/controls/<name>/{CHECK,EXPECT,overlay|MAKE}` on the toy tree, registered in `data/expected/controls/<group>.ids` (`rights.ids`: `manifests-*`, `files-*`; `runner-hygiene.ids`: `c-build-*`). Pure control about 0.07 hosted cpu-s, symbol-layer control 2.2 [OBS CI-EFFICIENCY]. | files |
| F19 | Controller bytes: KEY_1 `7E`, KEY_9 `DB`, DISC_E `FD`, SIDE_TOP `5F`, SIDE_LOWER_LEFT `9F` (lines 50, 58, 66, 78, 79); KEY_1+KEY_9 = `7E & DB` = `5A`, as D-02. | `data/input/*.literal.txt` |
| F20 | `13 x 59,736 + 30,000 = 806,568`; events at 100,000 + 120,000 k sit 40,264..42,904 ticks into their frames, i.e. 19,472 down to 16,832 before the frame end: D-06's 16,800 minimum holds (event 6 tightest). | computed |
| F21 | Caps: 64 asserts, 64 steps, 64 input lines, 4,096 capture entries per target, 16 video frames; `audio at <tick>` must be a multiple of the sample period. | `docs/runner.md` |
| F22 | Gate has 36 steps; `ci-parity` bans cache, upload, secrets, matrix, other runner/timeout; consumer 1,074 lines, golden 88 lines, 15 pkg selftests; `.gitignore` covers `/refs/`, `*.bin`, `*.ivsc`, `*.wav`, `*.ppm`, `*.vraw`, `/build/` but not `*.exec`, `*.grom`, `*.img`; tools here: cmake 4.4.3, ninja 1.13.2, Apple clang 21, dash, shellcheck, docker client 29.5.2 (daemon [UNK]). | steps, `.gitignore`, commands run |

## Spec defects found (settle before writing the listing)

| # | Defect | Resolution [REC] |
|---|---|---|
| X1 | S-3 proposes `# label: loop 5026` per label in the generated listing; `label` is a header key (F5), so each is a repeated key (`listing-header`; `listing.cmake:363-367`). | Use a non-header key, e.g. `# sym: loop 5026`. |
| X2 | D-04 calls `$02F0` both the marker `$03C5` and a counter `floor(T/59,736)+1`; S-1 puts the counter at `$02F1`. `806,568 / 59,736 = 13.50`, so `000D` equals `floor(T/59,736)`: first ISR in frame 1, frame 0 not displayed. The `+1` form belongs to the frame-0 band (init `EIS` under about 1,900 cycles). | W1 fixes one band: frame-1 band, marker `$02F0`, counter `$02F1`, counter `floor(T/59,736)`, `frame 0 displayed == no`; state the rule, not only constants. |
| X3 | D-11 describes the pad-in-ISR form (`EIS; PULR R7`), D-13 defaults to the main-loop re-arm (`DECR R6; J wait`); margins and mutants differ (pad band 245+, about 200 cycles; delay band N 200..800). | W1 derives both margins from `EIS start >= S_k + 3,777`, takes the wider, rewrites M-pad for that form (delay 170 / 850). |
| X4 | S-1 spike counts (25 PSG records, 7 init, 47 / 14 mutant records) predate D-22's constant-level gate (+about 16 words, +1 branch, +2 records per change). | Re-derive the whole PSG sequence by hand; spike numbers are leads only. |
| X5 | One assert per PSG record plus mem steps, frames and `audio at` is about 76, over the 64 cap (S-4 soft limit 56). | Assert `psglog count`, the init tail and the first record of each change; push per-record detail into one scenario; count asserts in W1. |
| X6 | S-4's CL-0023 draft says "boot stub (contract 1)"; D-09 uses contract 2. | Claim names `boot-stub-2`, contract 2, `$5000/$5004`. |
| X7 | S-4 commits `.expect` in W1, but the golden step needs scenario, inventory row and dev report (F10), which need the program. | W1: derivation (assert block as fenced text), frame literals, pair-region token, shape-lint tokens on the toy tree. `.expect` enters `data/golden/` with the registered scenario at the tracer; the W1 commit id in the SUMMARY evidences the order (06-13 precedent). |
| X8 | `expect-inc.cmake` cannot ingest `audio at`, `!=`, ranges or step boundaries (F11), yet S-4 proposes about 6 in-process `run.scn.*` ids from the `.expect`. | Do not feed this `.expect` to `expect-inc.cmake`; in-process tests are `run.stub2.*` only. |
| X9 | D-16 adds a `DATA` row token but D-01 has no data table, so no generated listing here contains `DATA`. | `ivs_asm` still supports `DECLE` (stic twin); the `listing.cmake` `DATA` rule + 2-3 controls is optional, a cost-trim candidate. |
| X10 | "At most 3 new controls" (D-25) covers S-4 only; phase-wide the plan implies about 11-14 pure controls. | Sum honestly at checkpoints (about 1 hosted cpu-s); the 25-per-plan limit is not near. |

## Architecture Patterns

```
fixtures/FX-*/*.asm --ivs_asm--> *.listing.txt --listing step + pack.words/trailer--> checked
listing --ivs_pack--> build/<preset>/images/{boot-stub-2.exec, lamp-tone.ivsc} (never committed)
scenarios + images --ivs-run (dev, release)--> build/gate/<preset>/runner/<id>.report
   --golden step--> .expect == scenario asserts; .digests == dev report; dev == release
   pass/idle reports --judge--> digests differ, input.consumed == input.events
docs derivation + data/video/frames --video step--> popcount, hash tie, pair-region
images --installed consumer (STOP_ON_FRAME frame loop)--> scn.* == consumer golden; scn.digest.* == runner .digests
```
Patterns: tracer first, gate-green per task (PLAN-LESSONS 1, 3, 4); 0 faults planned; copy-paste for replay 4 and mutant inventories; the generated listing is the checked artifact. Anti-patterns: plain `J` at the stub vector, `HLT` idle, strobe after cycle 2,000, GRAM write after the first 3,780 cycles of frame 0, BACKTAB work after the row-0 latch (about 3,848) [OBS-note S-1/S-2]; counter-only progress assert; emulator output as an expected value.

## File inventory (C create / M modify)

| Paths | C/M | Role | Wave |
|---|---|---|---|
| `tools/check/vectors.cmake`, `tools/review/vector-blind.cmake` | M | batch the per-vector `string(JSON GET)` re-parse (about 40 guarding controls) | W0 |
| `.planning/CI-EFFICIENCY.md` | M | trigger entry, checkpoints, `phase7-close` | W0, W4, W5, W7 |
| `docs/video/demo-lamp-tone-1-derivation.md`; `data/video/frames/F-S*.frame` (idle + 7E, 5A, FD-only, 5F+FD, 5A+9F); `data/expected/video-frames.ids` | C/M | oracle: spec, per-event RAM/BACKTAB/PSG, assert block, rights checklist, blind-read record (mind F15 words) | W1 |
| `tools/check/video.cmake`, control `video-frame-pair-region-drift`, `controls/video.ids` | M/C | `# pair:` header token | W1 |
| `tools/check/steps/golden.sh`, controls `golden-shape-*`, `controls/golden.ids`, toy-tree files | M/C | shape lint (`.expect` tier `playable-guest-progress`: input line, mem/reg after first input, frame and psglog assert, last input before last `run-to`) | W1 toy / W4 real |
| `tools/asm/{asm.c,CMakeLists.txt}`, root `CMakeLists.txt`, `c-build.sh`, `integrity.cmake`, control `c-build-unlisted-asm-source` + toy tree, `controls/runner-hygiene.ids` | C/M | `ivs_asm`, registration like `tools/pcm`, fifth source list | W2 |
| `tests/asm-tap.sh`, `tests/asm/{pairs.txt,twins/*.asm,neg/*}`, `data/expected/{asm-tokens.ids,tests/asm.ids}`, `tests/CMakeLists.txt` | C/M | suite `asm` (T1 pairs, T2 four twins, T3 regen, T5 about 22 negatives, T6 judge) | W2 |
| `docs/diagnostics.md`, `docs/packaging.md`; `listing.cmake` `DATA` rule + controls (optional, X9) | M | grammar, "never installed" | W2 |
| `fixtures/FX-boot-stub-2/{boot-stub-2.asm,*.listing.txt,manifest.json}`; `listings.ids`, `fixtures.ids` | C/M | stub contract 2 (22 words, 10 non-zero, `JD` at `1004`) | W3 |
| `runner/{ivs_runner.h,scenario.c,exec.c,report.c}`, `docs/runner.md`, `tests/runner/{runner_parse,runner_exec,runner_fuzz}.c`, `data/expected/tests/runner.ids` | M | token `boot-stub-2` (kind 5, file `boot-stub-2.exec`, label names contract 2, fuzz bound) | W3 |
| `tools/pack/CMakeLists.txt`, `tests/CMakeLists.txt` (`--c-include`, source lists), `tests/runner-process-tap.sh`, `tests/run/run_stub*.c`, `data/expected/tests/run.ids` | M/C | build/distribute images; `run.stub2.path`, `run.stub2.handoff`, control `STUB2_JD_AS_J` | W3 |
| `fixtures/FX-demo-lamp-tone-1/{lamp-tone.asm,*.listing.txt,manifest.json}` | C | program, listing, first non-null recipe | W3 |
| `tools/check/manifests.cmake`, `files.cmake`, controls `manifests-recipe-source-digest`, `files-trademark-name`, `controls/rights.ids` | M/C | recipe-source hashing; `names` rule | W3 |
| `data/scenarios/demo-lamp-tone-{pass,idle,shift,swap}.ivss`, `data/golden/demo-lamp-tone-{pass,idle}.expect`, `*.digests`, `runner-scenarios.ids`, `tests/runner-process.ids` | C/M | scenarios, goldens (new files only), inventories | W4 |
| `data/expected/{diag-mutations-scn.ids,scenario-pairs.ids}`, `runner-process-tap.sh` | C/M | mutants, `fails=` column, stub-2 copy, `--exec` patch variant, judge | W4 |
| `tests/installed-consumer/main.c`, `pkg-consumer.golden`, `pkg-selftests.ids`, `pkg-install.sh` | M | replay 4 (`scn.`), 9 domains, selftests 15 -> 17, args 5 -> 7, golden additions only | W5 |
| `docs/owner-images.md`, `docs/{stic,input}/d10-procedure.md` (`#owner-exec`), `fixtures/FX-owner-{exec,grom}/manifest.json`, `fixtures.ids`, `data/scenarios/owner-exec-title.ivss`, inventories | C/M | SCN-02 route and labels | W6 |
| `tools/owner-run.sh` + tests, `runner/main.c`, `tests-inventory.sh` (+ self-test), consistency rule (`runner-scan.sh` or suite) + 2 controls | C/M | helper, refusals, SKIPPED output | W6 |
| `data/claims.json`, `claims.ids`, `text.sh` + control, `README.md`, `docs/evidence-tiers.md`, `important-gates.sh` + `.ids` + controls, `.planning/{REQUIREMENTS,ROADMAP,STATE,PROJECT}.md`, `07-VERIFICATION.md`, SUMMARY files | M/C | close-out | W7 |

## Don't Hand-Roll

Use `ivs_pack` (no second packer); `listing.cmake` row and `-> ADDR` recomputation (independent of the assembler); address-keyed `--patch` mutants with explicit `from` words (no mutation generator); hand `.frame` literals with popcount/hash/pair checks (no renderer, no new assert subject); `cmake -E sha256sum` lock plus core `expect-sha256` for owner files (no digest DB, no CLI flag).

## Common Pitfalls

1. Header-key collision (X1): verify with the listing step on the tracer listing.
2. Hosted-only failures (PLAN-LESSONS 11): run `sh`, `dash` and the container gate before push.
3. Doc tripwires (F15): derivation/owner pages must avoid `runs`, `passes`, `verified`, `supports`, `accurate`, `compatible`, bare `clock`, later `plays|playable`; use `[nc]` or other words.
4. Consumer/golden skew (lesson 2): replay digests and observed-domain set equal the runner `.digests` at every plan end; consumer golden additions-only.
5. Mutant dir lacks stub-2 (F9); stub mutants need the `--exec --patch` path.
6. Events after the last bound are silently ignored: judge `input.consumed == input.events`; lint last input tick < last `run-to`.
7. Idle must be silent and static, else pass/idle is vacuous; assert idle literals, not a timeout.
8. No blank position in asserted frames (`hash-needs-colors-only`); spike values are not oracle (X2-X4).
9. ASCII/CR/SPDX (F16) apply to `.asm`, twins, pairs; hygiene rejects home/scratch paths and `@` before a dotted word, also under `.planning/`.
10. Helper scenario injection: validate each lock line as exactly 64 lower-case hex before inserting it.

## Open questions and risks for planning

1. X2/X3 band and re-arm form: W1 decides with the arithmetic in the derivation.
2. **Stub-to-program ISR tie** (discretion): (a) header key `isr` in both listings plus a `listing.cmake` cross-check (needs `HEADER_KEYS` change); (b) `ISR EQU $5004` repeated in both `.asm` plus the behavioural tie. [REC] (b) plus a mandatory mutant (stub target moved off the program's ISR must fail `frame k displayed == yes`).
3. **Spelling** [REC]: `NAME EQU expr`, `DECLE v1, v2`, `$` hex, no local labels, explicit `SDBD`.
4. **Mutant addressing** [REC]: absolute `<addr> <from> <to>` with the `from` check, `# sym:` lines for addresses; `label+N` only after two label-driven re-derivations.
5. **Audio windows**: a constant-level `audio at` needs a tick multiple of `audio.sample_ticks` and the regmap rule (`docs/psg`); if no window can be made, D-22's fallback (`assert audio changes`) is a grammar change needing an explicit re-bless classification.
6. **`owner-exec-title`** [UNK]: whether a scenario with only exec+grom user roles parses (gate ends `missing-fixture` before any run); prove parse in W6; pick `run-to <n> frames`, no asserts.
7. **SKIPPED line vs `ci-summary`**: `keep_fails` greps `^-- FAIL ` only (`gate.sh`), so a `-- SKIPPED` line is safe there; run the ci-summary selftests after the edit.
8. Shift probe sweep (+-1,000..10,000 ticks) is a throwaway; CI keeps +2,000. Optional discrepancy C106 for the manual's branch-range text vs the decle formula (no hardware claim). Six frame literals of 96 rows: derive as deltas of the idle frame.

## Environment Availability

cmake 4.4.3, ninja, Apple clang 21, dash, shellcheck present; docker client present, daemon [UNK] (fallback: visible "not run" line in the SUMMARY); libasm absent (optional T8, skip visibly); `gh`/hosted CI are orchestrator-only (rows via `tools/ci-record.sh`).

## Validation Architecture

### Test Framework
| Property | Value |
|---|---|
| Framework | gate steps (sh/cmake) + ctest suites in TAP shape; real `ivs-run` in `runner-process` |
| Config | `CMakePresets.json` (dev ASan+UBSan, release), `data/expected/tests/*.ids` |
| Quick run [INF, not executed] | `ctest --preset dev -R '^(asm\|pack\|runner\|runner-process)$' --output-on-failure` after `cmake --build --preset dev` |
| Full | `sh tools/gate.sh` and `dash tools/gate.sh`; container: `sh tools/gate-linux.sh >/dev/null 2>&1 && grep -q '^gate: PASS' build/gate-linux.log` |
| Targets | `steps=36/36`; `tests-skipped` 2 -> 4 after W6 |

### Requirement and decision map
| ID | Behaviour | Automated check | Fails when | Hand-off |
|---|---|---|---|---|
| V1 SCN-01, D-28 | manifest, recipe, digests, licence, `redistributable: yes` | `manifests` (+recipe hashing, control `manifests-recipe-source-digest`), `files`, `listing`, `pack.words/trailer` | listing, `.asm` or `pack.c` edited without refresh; row/branch/word mismatch | rights attestation stays author-attested |
| V2 D-16..19 | `ivs_asm` correct | suite `asm`: pairs, 4 twins (300 legacy words), `asm.regen-<id>`, negatives pinned by `asm-tokens.ids`, judge `asm.judge.mutated-twin` | wrong opcode/field/displacement; an escape found only by a run | none |
| V3 D-09/10 | stub hand-off | listing guards; `run.stub2.path`, `run.stub2.handoff`; control `STUB2_JD_AS_J`; scenario `frame k displayed == yes` | `JD` -> `J` (178-181 entries/frame, display never on; the counter alone still passes) | none |
| V4 D-04/05/14 | guest-state predicate | multi-step `assert mem` at >= 3 `run-to` steps, `halted == no`, cumulative counters, `.expect` | wrong store address, counter stuck, input not decoded | none |
| V5 D-12/13 | sustained display, re-arm | `frame k displayed == yes` per frame, counter `floor(T/59,736)`; mutants delay short/long, late strobe, missing `EIS`, `HLT` idle; negative control must pass | 2 entries/frame, blocked strobe | none |
| V6 D-21/23 | video differs exactly in region | two literals (hash + pixels) in `.expect`; `video-frame-popcount`, `video-frame-hash-expect-drift`, new `video-frame-pair-region`; consumer `scn.diff inside=<k> outside=0` | extra differing pixel; hash drift; video-only mutants (LAMP-NOCOMR, SEL-BIT, STROBE-ADDR, GRAM-LATE) | viewing frames is not evidence (H7-2) |
| V7 D-22 | audio changes by derivation | `psglog` addr/value/source asserts, `audio at` silent and constant-level windows, idle `output.audio.changes == 0` | wrong amplitude/period; sound ungated (judged under `idle`) | listening (H7-1) |
| V8 D-23 | idle vs pass | judge over reports via `scenario-pairs.ids`: differ `digest.video/audio/psglog/input/mem`; `input.consumed == input.events`; 3 `runner-process.judge.<slug>` self-controls | input ignored; events after last bound | none |
| V9 D-24 | not "title screen or hash alone" | shape lint in `golden` + 2 controls | no input line, no mem/reg after first input, no frame/psglog assert | none |
| V10 D-25 | each channel's asserts can fail | `diag-mutations-scn.ids` mutants, `fails=` set on isolation mutants, `runner-process.mutant.<ID>`; one wrong clause and one wrong `from` made to fail once | non-distinct mutant | none |
| V11 D-26, PKG-01 | installed replay | `pkg-install`: moved-prefix build, `scn.*` == `pkg-consumer.golden`, `scn.digest.*` and 9 domains == runner golden; selftests `scn-domain-missing`, `scn-digest-drift` | digest or domain drift | none |
| V12 | repeatability | `repeat`, `slices` (1 and 7), `cwd` for `demo-lamp-tone-pass`; dev == release | host-dependent byte | none |
| V13 D-29 | SCN ids accepted, names | `important-gates` (+lines), `files-trademark-name` | "Intellivision" in a fixture/scenario/target name | none |
| V14 D-27 | claim wording | `claims` (artifact exists, weakest-field => provisional), `text` with `plays|playable` + control | untagged "playable" | none |
| V15 SCN-02, D-30 | owner route labelled | `manifests` (two `system-image`, `redistributable: no`), `files` (tracked image), doc anchors | owner manifest says `yes`; image tracked | owner places images in `refs/` |
| V16 D-31 | visible, counted SKIP | `owner-exec-title` `skip-ok` in both inventories, exit 13, detail `missing role=exec user`; `-- SKIPPED ...` line + `tests-skipped=4`; step stdout self-test | scenario file removed (`tests-missing`), skip without `skip-ok` | none |
| V17 D-31 | user-content consistency | rule both directions + 2 controls | owner-content scenario as ordinary test | none |
| V18 D-31 | helper refusals | `runner-process.owner-run.*`: `CI`/`GITHUB_ACTIONS` set, in-tree path outside `refs/`, wrong size, missing lock, absent images (exit 77, `SKIPPED owner-run:`), bad lock line, judge self-control | exit 0 where it must refuse | owner runs it (H7-4) |
| V19 D-31 | derived outputs refused | one process test per option (`--audio-raw`, `--psg-log-out`, `--audio-wav`): exit 14, one stderr line, no file | file created | none |
| V20 D-34 | cost | last-5 `r` from `ci-runs.tsv`, checkpoints in `CI-EFFICIENCY.md` | median `r` > 8.3 => lever plan, no more cases | orchestrator records rows |

**Wave 0 gaps (absent today):** `asm` suite files and ids; `run.stub2.*`; runner token tests; `diag-mutations-scn.ids`, `scenario-pairs.ids`; consumer replay lines; helper tests. The frameworks exist, nothing to install.
**Sampling:** per task its verify + `sh tools/gate.sh`; per plan adding C/sh/cmake also `dash` and the container gate.
**Handed off to the owner (never evidence or claim):** H7-1 listen to the WAV from `tools/listen-demo.sh`; H7-2 view `--frame-ppm` exports; H7-3 D10 items unchanged (check `d10-measurements.ids`); H7-4 the D11 EXEC/GROM run (not performed); H7-5 publication (delegated flow).

## Wave order, sizing and cost budget

| Wave | Plans (<= 5 tasks each) | Depends on | Notes |
|---|---|---|---|
| W0 | 1: recompute `r` (8.02 now => lever), lever, CI entry | none | no new tests before it lands |
| W1 | 1: derivation, frame literals, pair-region token + control, shape-lint tokens (toy), blind-read record | W0 | no C, no `data/golden` file (X7); settles X2-X5 |
| W2 | 2: (a) tracer: skeleton, registration, c-build control, suite `asm` for the stub and `diag-cpu-1` twins + judge; (b) rest of the table, SDBD, DECLE, pairs, twins, negatives, docs | W0 | oracle never depends on the assembler |
| W3 | 2: (a) stub-2: runner token + tests, image plumbing, `run.stub2.*`, control, manifest; (b) program `.asm`, listing, manifest + recipe-hash check + control, `names` rule, tracer scenario vs literals | W1, W2 | (a) before (b); 0 faults; `.expect` + scenario registered in one commit |
| W4 | 2: (a) scenarios, `.expect`, `.digests`, real shape lint, judge + 3 self-controls, repeat/slices/cwd; (b) mutants, `fails=`, `--exec` patch variant; **checkpoint C-B** | W3 | heaviest; keep <= 19 process cases per preset for S-4's slice |
| W5 | 1: replay 4, golden additions, `pkg-install` args and selftests, `docs/packaging.md`; checkpoint C-C | W4 | args 5 -> 7 |
| W6 | 1-2: docs, manifests, template, consistency rule; helper, refusals, SKIPPED output | independent, serial | shares `runner-scenarios.ids`/`runner-process.ids` with W4: never concurrent |
| W7 | 1: claims, tripwire word, `important-gates` + SCN lines, README/PROJECT/ROADMAP tidy, D4/D11 notes, `07-VERIFICATION.md`, `phase7-close`, final gate x3 | all | mark SCN-01/02 only when evidence closes them |

Limits: <= 5 tasks, <= 8 faults (0 planned), <= 25 controls per plan (phase-wide about 11-14, X10). Task verifies hold at their own commit; a tracer may be red only with its named steps and no push between.

**Budget [INF, hosted cpu-s]:** headroom (8.3 - 8.02) x 41 s build = about 11.5; lever about -5 => about 16. Estimates: S-4 6-12, `asm` 1.5-3, S-5 plus stub-2 in-process 2-4, controls about 1: total 10-19, upper end over headroom. Lever order: (1) W0 batch re-parse; (2) mutants on release only; (3) drop `swap` or fold `shift` into repeat variants; (4) owner-decision levers (`-O0` fault objects, per-TU object, two-job split) out of scope. Not levers: dropping a mutant class or the judge, weakening an oracle, caching. Machine time is not a driver (20 M ticks: 0.11 s release [OBS-note]; this scenario is 0.81 M ticks).

## Security Domain

`security_enforcement` on, ASVS level 1 [OBS `.planning/config.json`].

| ASVS | Applies | Control |
|---|---|---|
| V5 input validation | yes | `ivs_asm` total on hostile input (limits, ASCII/LF, pinned tokens, no `getenv`/time, writes only its output); scenario parser change under existing parse tests/fuzz; helper checks 64-hex lock lines |
| V12 files | yes | helper refuses `CI`/`GITHUB_ACTIONS`, in-tree paths outside `refs/`, wrong sizes; `mktemp` scenario outside the tree; runner refuses derived outputs for user content |
| V14 config | yes | no secrets, no workflow change |
| V6, V2-V4 | no | `cmake -E sha256sum` and in-tree SHA-256 only; no accounts |

Threats: owner image committed or shipped (`/refs/` ignored, `files` fails a tracked image, `redistributable: no` labels, unchanged release allowlist); derived audio/log/frames of an owner image (runner refusals); injection via lock file or ids (strict hex/slug validation, no `eval`); stale recipe hides a changed tool or listing (recipe-source hashing, `asm.regen-<id>`); third-party glyph/tune/EXEC dependency (zero GROM and boot stub in CI, rights checklist, `names` rule, denylist step).

## Assumptions Log

| # | Claim | Risk if wrong |
|---|---|---|
| A1 | All timing numbers (strobe cycle 29-35, `EIS >= 3,777`, window 2,000, bands, pad 245) come from the notes; no probe was re-run. | wrong band/pad; W1 re-derives from profile fields, W3 tracer re-measures |
| A4 | An exec+grom-only scenario parses and loads. | add a cart role or adjust the template |
| A5 | Hosted rows may be appended before W0; recompute `r` then. | lever skipped or misapplied |
| A6 | `JD` passes the listing step as row `J` (S-2 ran it in a scratch tree: `listings=5`); here only the row files were read. | add `JD` to the row lists deliberately |

## Sources

Repository files read this session (the `07-*` notes, `AGENTS.md`, `.planning/ops/*`, `.planning/{REQUIREMENTS,ROADMAP,STATE,CI-EFFICIENCY}.md`, `ci-runs.tsv`, the gate steps and cmake checkers named in F1-F22, `tools/pack`, `tests/`, `runner/`, fixtures, `data/`, `docs/runner.md`). No external source was consulted.

## Metadata

Research date 2026-11-13; valid until a commit touches the cited gate steps or `ci-runs.tsv` (re-check F1, F2 and the `r` median at plan start).

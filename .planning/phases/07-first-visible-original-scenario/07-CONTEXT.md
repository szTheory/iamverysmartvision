# Phase 7: First Visible Original Scenario - Context

**Gathered:** 2026-11-13
**Status:** Ready for planning
**Mode:** `--auto` with the fan-out protocol (`.planning/AUTONOMY.md` section 2). Five parallel research agents, one per decision point: the scenario program (`07-DISCUSS-scenario.md`, S1-*), the boot and runtime architecture (`07-DISCUSS-runtime.md`, S2-*), the D4 assembler re-evaluation (`07-DISCUSS-assembler.md`, S3-*), verification and evidence (`07-DISCUSS-verify.md`, S4-*), rights, manifest and SCN-02 (`07-DISCUSS-rights.md`, S5-*). The synthesis below adopts one recommendation per point and resolves the conflicts between the notes (section "Conflict resolutions"). The notes carry the evidence; this file carries the decisions. Labels in the notes: [OBS] read or measured, [INF], [REC], [UNK], [BG].

<domain>
## Phase Boundary

An original, project-authored program ("Lamp Tone") runs from reset through the public API, the headless runner and an independently built installed consumer. A scripted controller input sequence changes guest state (RAM markers), raw video (the index framebuffer in the expected region) and raw audio (PSG write log and samples). The program, its recipe, digests and licence are in the fixture manifest as `redistributable: yes`. The display is sustained from a project interrupt service routine. SCN-02 documents the owner-local route for user-supplied EXEC/GROM images with identity checks, labelled non-redistributable and kept out of CI, with a visible SKIPPED line the inventory gate counts.

In scope: SCN-01, SCN-02, D4 re-evaluation (outcome: a project mini-assembler), the stub contract 2 image, claims for the new tier, the checker gaps listed below.

Not in scope: MOBs, collision, delay registers, border extension (Phase 8, VID-05); a reference host frontend (v2, TOOL-04); any real-hardware claim (D10); any real-EXEC result (D11); third-party homebrew (Geese, IntyBASIC output, 4-Tris: none considered, so no D9 trigger); PAL; ECS.
</domain>

<decisions>
## Implementation Decisions

### Scenario program (S-1, open question 9)

- **D-01:** The program is "Lamp Tone", a two-port lamp board with one PSG voice per port. Fixture `FX-demo-lamp-tone-1`, program id `ivs-demo-lamp-tone-1`, scenarios `demo-lamp-tone-{pass,idle,shift,swap}`. Functional, brand-free names, no on-screen text, no GROM, EXEC, glyph or tune data, no data table, no MOBs. The slug is final unless the name rule (D-26) rejects it. — **Reversibility:** costly — names land in manifest ids, scenario ids, goldens and inventories.
- **D-02:** Each frame the program renders the raw active-low byte of each controller port as 8 lamps (lit means bit cleared, no decode). `c0` is the `$01FE` port and `c1` the `$01FF` port, never "player N" (C66). Expected pictures derive from the controller literal alone (KEY_1 reads 7E, KEY_1+KEY_9 reads 5A, DISC_E FD, SIDE_TOP 5F), not from the program.
- **D-03:** On a port change the program retunes that port's PSG voice (pitch equals the byte) and sets the amplitude to 15 when any control is closed, 0 when idle. Idle is silent. The PSG is programmed once at init and written only on change; mixer bits 6/7 (I/O direction) stay clear.
- **D-04:** Progress predicate (guest state, not a title screen or hash): RAM `$02F0..$02F5` holds an init marker `$03C5`, a per-frame counter, and per port the last byte and a change counter. After the timeline: frame counter `000D`, change counters `0006` and `0003`, last bytes `5A` and `9F`. The values are re-derived by hand in the oracle plan before the listing; the spike values are leads, not oracles. `$02F0` counter equals floor(T/59,736)+1 (S-2). The counter alone is a vacuous predicate (S-2 trap), so every pass assertion also asserts `frame k displayed == yes` and a video or PSG consequence.
- **D-05:** The program never halts (a halt blanks the display, S-1/S-2). The scenario ends at a `run-to` bound and asserts `halted == no` plus the progress asserts at that tick.
- **D-06:** Timeline: six input events 120,000 ticks apart, each at least 16,800 ticks from any frame end (S-4 requires at least 10,000), `run-to 806568`, `capture video 0 13`. The `shift` scenario moves every event +2,000 ticks and must give byte-identical frames. All events lie before the last run bound (events after it are silently ignored today, S-4 finding).
- **D-07:** The graphics-RAM write, if any, must come first in init (a GRAM write after the first 3,780 cycles of frame 0 is dropped silently and blanks the picture while the state asserts still pass, S-1). Lamp glyphs are drawn from project-authored pattern words or plain card colours; no third-party glyph set.
- **D-08:** Fallback, only if the interrupt design fails in planning: a polling main loop (114 words, stub contract 1 unchanged) renders the same lamps and PSG sequence and loses the counter and the interrupt-path evidence.

### Boot and runtime architecture (S-2)

- **D-09:** Boot-stub contract 2 as a second image: new fixture `FX-boot-stub-2`, listing id `boot-stub-2`, runner content token `boot-stub-2`. Contract 1 is unchanged, so no existing diagnostic, golden, mutant or `.digests` file moves (zero re-bless). Convention: cartridge reset entry `$5000`, interrupt entry `$5004`. The stub stays a stub (at most 16 non-zero words at `$1000-$101F`); the interrupt service routine lives in the program's cartridge segment (S-5). — **Reversibility:** costly — the token, the fixture id and the `$5000/$5004` convention are referenced by later fixtures. Reversal evidence: if two stubs prove a burden, replace contract 1 in place (measured: six `.digests` goldens plus the consumer golden move, all classified in the plan that moves them).
- **D-10:** The stub's interrupt entry at `$1004` is a single `JD $5004`. A plain `J` is a measured trap: with C41=0 the second ISR instruction is interruptible, the vector re-enters 178-181 times per frame, R6 climbs to `$06C2` and the strobe lands outside the window, yet the frame counter still reads correctly. A mutant covers `JD` to `J`.
- **D-11:** Tick-driven execution: the ISR does the strobe, counter, input poll, PSG and screen writes, then `EIS` and `PULR R7`; main is `EIS; B self`. Never idle with `HLT` (HALTED ignores INTRM). No register save is needed.
- **D-12:** Display strobe `$0020` is the first ISR action (measured at cycle 29-35, window closes at 2,000). Visual work ends before frame cycle about 3,700 (BACKTAB row 0 latches at 3,848). Startup: the init `EIS` timing selects a band (up to about 1,900 shows frame 0; 2,000-3,776 gives a blocked strobe and frame 0 not displayed; from 3,777 the first ISR is in frame 1); the oracle plan fixes which band the scenario relies on.
- **D-13:** Re-arm rule (measured): INTRM is a level held 3,791 cycles, so `EIS` must start at or after frame cycle 3,777 (at least 3,776 cycles from the start of the entry unit); pad 244 re-enters, pad 245 does not. Two forms were measured: padding inside the ISR (about 200 cycles of margin, plan for about 3,950-4,050) and a main-loop re-arm `DECR R6; J wait` (works for N 200..800, a wider band). The oracle plan picks the form with the wider measured margin, defaulting to the main-loop form, and a "pad too short" mutant plus a pad 245 to 260 negative control guard it.
- **D-14:** Input is polled once per frame in the ISR. An input event in frame j shows in frame j+1 (probe: ticks 130,000 and 250,000 changed and restored an 8x8 region). No assertion on the frame of an input or the next one (S-4 settling margin).
- **D-15:** RAM layout: BACKTAB `$0200-$02EF`, variables `$02F0-$02FF`, stack from `$0340` (depth 1). A display-on frame has 14,934 cycles with 13 bus-window stalls of about 1,446, leaving about 13,488. No core change is needed (measured on an unmodified scratch release).

### D4 assembler policy (S-3)

- **D-16:** The D4 trigger fires on three independent counts: a fourth project-authored program, labels (interrupt entry, loop heads, table-address immediates), and a 10-branch program (the 06-13 rule of more than 8 branches fires by its letter; the 132-136 word size does not). Decision: build `ivs_asm`, a project mini-assembler that generates the existing committed listing grammar plus one new row token `DATA`. `ivs_pack` is unchanged and the generated listing stays committed, so the existing `listing` and `pack.words` checks still cover it. — **Reversibility:** reversible — the listings remain valid hand-checkable artifacts. Reversal evidence (S-3): two encoding escapes found only by a run, the tool passing 1,200 lines, or any call for macros: stop and hand-assemble.
- **D-17:** `ivs_asm` is one C17 file `tools/asm/asm.c` (estimate 700 to 900 lines, hard stop 1,200), registered like `tools/pcm`, never installed or shipped, no dependency. The encoding table is hand-written from `docs/cp1610/reference-notes.md` and the cited manual pages, with no GPL input. SDBD stays an explicit prefix (no auto-SDBD, no size fixpoint). An immediate, direct address or branch displacement above `03FF` is a named error; a spelling that lands on a different leaf row (`MOVR R1,R1` for `TSTR R1`) is an error. Branch displacement is forward `target - (pc + 2)` and backward `(pc + 1) - target`, the formula the checker already uses.
- **D-18:** Three outside checks police the table: the listing checker's decle-to-row check against `data/cp1610/opcode-families.json`; hand-derived blind-second-read asm/word pairs copied once from the Phase 2 vectors (S-3 counted 317); byte-for-byte reproduction of the 300 words in the four existing hand-assembled listings through `.asm` twins kept as test data. The corpus is a flat copy (no third JSON parse).
- **D-19:** Each assembled fixture commits the `.asm` (source of truth) and the generated listing, both in the manifest `outputs` with the `recipe` slot filled; a suite test regenerates each listing and requires byte equality. The boot stub contract 1 and the three diagnostics stay hand-assembled and untouched (twins prove the assembler; a twin is promoted only when its legacy listing next needs an edit). A new `asm` test suite runs under the existing `c-tests`; no new step, job or dependency (estimate 1.5 to 3 cpu-s). The D4 Decision Register row is updated with the S-3 wording at the close-out.

### Verification and evidence (S-4)

- **D-20:** IVSS v1 / `ivsr 3` is frozen: no new assert subject, no report bump, no golden re-bless from grammar changes. The timed input script, multi-step mem/reg progress asserts, frame hash and pixel probes, `frame displayed`, psglog and `audio at` already cover the criteria.
- **D-21:** "Framebuffer differs in the expected region" is proved without runner code: the run asserts two hand-derived frame literals (hashes plus pixel probes), and a new `video-frame-pair-region` token in the gate checks that the two literals differ exactly in the declared rectangle. The installed consumer does its own slot diff.
- **D-22:** Non-circular audio oracle: idle is silent; the first audible state is a constant level (PSG R15/R16 give level equal to volume) so samples are derivable without cycle accounting. The psglog is asserted by address, value and source, never tick. Tone pitch is asserted through the write log, not through sample phase. The program therefore includes a constant-level gate on one channel (S-1 estimate about 16 words) in addition to the per-port tone writes. Fallback only if no constant-level window can be made: an `assert audio changes` subject (a grammar change, so it needs a plan-level decision and a re-bless classification).
- **D-23:** Oracle-first: the derivation, the literals, the `.expect` and the claim wording are committed before the program; a blind re-derivation by a fresh agent is recorded. Scenarios: `pass`, `idle` (asserts the idle literals, not the nokey timeout), `shift`, `swap` (events on swapped ports). A metamorphic judge compares the pass and idle reports (video, audio, psglog and input digests differ; `input.consumed == input.events`).
- **D-24:** A scenario-shape lint in the `golden` step, for scenarios with tier `playable-guest-progress`, requires an input line, a mem/reg assert after the first input, a frame assert and a psglog assert, with two controls. It is the mechanical form of "not a title screen or hash alone".
- **D-25:** Mutants: six program mutants plus one negative control, one per failure class (input decode, progress write, video write, sound value, sound gate judged under `idle`, interrupt re-arm), extended with the S-2 runtime mutants (stub `JD` to `J`, pad too short, late strobe, missing `EIS`, `HLT` idle, wrong port, wrong PSG register) where they are distinct classes; at most 3 new controls and 0 new faults in the cost model. A new optional `fails=` column judges the full set of failing asserts, not only the first. Seven of eleven S-1 mutants are visible to a single channel only, so every pass scenario asserts all three channels (guest state, video, PSG).
- **D-26:** Installed consumer: a fourth hand-written replay (`scn.`-style, copy-paste, no refactor), host-style frame loop with `STOP_ON_FRAME`, its own region diff, decoding of audio and write-log bytes; the first replay observing all nine domains. Compared with the runner golden.
- **D-27:** Claims: provisional claims CL-0023 (tier `playable-guest-progress`) and CL-0024 (`installed-host-behavior`); the planner re-checks the next free ids (last in `data/claims.json` is CL-0022). The only allowed wording is "playable guest progress, for this original program only". `plays|playable` joins the text tripwire.

### Rights, manifest and SCN-02 (S-5)

- **D-28:** One manifest `FX-demo-lamp-tone-1`, kind `original-source`, MIT with the same `LICENSE` evidence as the existing fixtures, `redistributable: yes`, `allowed_use` ci/repo/release/local-test, `rights_review: author-attested`. It is the first fixture with a non-null `recipe` (`ivs_asm`, then `ivs_pack --ivsc`). The project licence is already MIT (D9, owner instruction), so it is not an owner gate. A third-party-content attestation covers glyphs, tunes, EXEC calls and GROM reads.
- **D-29:** Checker gaps closed: `tools/check/manifests.cmake` hashes `recipe.sources` and `tool_sha256` (about 20 lines, control `manifests-recipe-source-digest`, tool sources listed); a `names` rule in the `files` step makes the "Intellivision in a name" hard stop machine-checked; the `important-gates` requirement set is extended to accept SCN-01 and SCN-02.
- **D-30:** SCN-02 owner-local route: one page `docs/owner-images.md` with three identity layers (core shape check, an owner-written digest lock in gitignored `refs/`, the core's `expect-sha256` at load). No known EXEC/GROM digests are recorded in the repo. Two uncommitted `kind: system-image` manifests (`FX-owner-exec`, `FX-owner-grom`, `redistributable: no`) put the non-redistributable label where the gate reads it. Route is EXEC+GROM only, no cartridge: the program's `$5000`-is-code layout is not an EXEC cartridge header and is not claimed to run under a real EXEC.
- **D-31:** A tracked template scenario `owner-exec-title.ivss` (`content exec user` + `content grom user`, no assert stating a result) registered as a second `skip-ok` scenario (exit 13, `missing-fixture`), taking `tests-skipped` from 2 to 4; `tests-inventory` prints `-- SKIPPED <preset>/<suite>/<id>: <reason>`. A consistency rule: every scenario with a `content <role> user` line is `skip-ok` and the reverse. A small `tools/owner-run.sh` (modelled on `listen-demo.sh`) refuses when `CI` is set, an in-tree path outside `refs/`, a wrong size or a missing lock; exit 77 with a SKIPPED line when no images exist. The runner also refuses `--audio-raw`, `--psg-log-out` and `--audio-wav` for user content (frame, video and golden outputs are already refused), about 3 lines plus a test.
- **D-32:** D11 stays open and not performed: SCN-02 is complete on documented-route terms (Phase 5 and 6 close-out precedent), with no claim-ledger entry and no tier. Docs under `docs/` are scanned for supports, compatible, passes, accurate, verified, runs: use `[nc]` or a claim tag.
- **D-33:** Releases ship the program only as tracked source in the tarball; neither the runner nor `ivs_pack` is installed, so nothing is downloadable and runnable yet, and the release notes say so. The stale README ("no machine code, no software release") and the stale "licence is an owner gate" lines in PROJECT.md and REL-01 are fixed at the Phase 7 transition.

### Budget and sequencing

- **D-34:** The CI trigger check opens the phase: the last-five hosted `r = cpu_s/build_s` median is about 8.0 against a 7.8 budget and an 8.3 trigger. If it is at or above 8.0, run the first cheap lever (batch the cp1610-vectors re-parse) before new tests. Headroom is about 11 hosted cpu-s (about 16 after the lever); S-4's additions estimate 6 to 12 cpu-s (no new job, about 15 to 19 runner-process cases per preset); a cost checkpoint follows the scenario wave. Machine time is not a cost driver (a 20 M-tick run takes 0.11 s release, 0.34 s dev).
- **D-35 (plan shape, verify in planning):** W0 trigger and lever; W1 oracle (no C: derivation, frame literals, `.expect`, audio oracle, claim allocation, blind second read); W2 `ivs_asm` tracer, twins and tests; W3 `FX-boot-stub-2`, program `.asm`, manifest, recipe checker, tracer; W4 scenarios, goldens, shape lint, mutants, judge, cost checkpoint; W5 installed-consumer replay; W6 SCN-02 (docs, template scenario, owner-run helper, runner refusal, skip rows, inventory output); W7 close-out (claims, README and PROJECT tidy, D4 register, VERIFICATION with automated / handed-off / caveat tables). Serial executors, at most 5 tasks and 8 faults per plan, `sh tools/gate-linux.sh` on every plan that adds C, shell or cmake.
- **D-36 (hard stops):** D10 and D11 are not performed and evidence needing them is recorded as missing. No third-party bytes, glyphs, tunes or datasheet text enter the repo. Hand-off to the owner is only: listening to the exported WAV, viewing the exported frames (neither is evidence), the D10 items (unchanged), the D11 owner-local EXEC/GROM run.

### Conflict resolutions between the research notes

- **R-1 (audio exactness):** S-1 said tone phase is not derivable without cycle accounting; S-4 requires a non-circular oracle. Resolved by D-22: derive audio only from constant-level states and the write log, never from tone phase.
- **R-2 (halt marker):** S-4 preferred a halting end marker; S-1 and S-2 showed a halt blanks the display. Resolved by D-05: never halt, assert `halted == no` and the progress values at a `run-to` bound.
- **R-3 (re-arm form):** S-2's in-ISR pad versus S-1's main-loop re-arm. Resolved by D-13: oracle plan picks the wider measured margin, default main-loop.
- **R-4 (scenario ids):** S-4 abbreviated `scn-*`; S-1/S-5 use `demo-lamp-tone-*`. Resolved by D-01.
- **R-5 (assembler timing):** S-3's own adversarial pass calls the assembler break-even on Phase 7 alone. Adopted anyway because the trigger fires on three counts and Phase 8 needs about five single-feature fixtures.

### Claude's Discretion

- Exact lamp layout, card colours and row placement within the 20x12 matrix, provided the expected region is a declared rectangle.
- `EQU` and `DECLE` spelling in `ivs_asm`, whether mutants name addresses by label, how the gate ties the stub's interrupt-entry target to the program address (S-2 plus S-3 open item), exact test counts within the cost budget.
- The consumer replay's id prefix.

</decisions>

<canonical_refs>
## Canonical References

**Downstream agents MUST read these before planning or implementing.**

### Policy and decisions
- `AGENTS.md`, `.planning/AUTONOMY.md` (sections 2-6), `.planning/PROJECT.md` (Decision Register D4, D9, D10, D11), `.planning/REQUIREMENTS.md` (SCN-01, SCN-02), `.planning/ROADMAP.md` (Phase 7, 8)
- `.planning/ops/EXEC-RULES.md`, `.planning/ops/PLAN-LESSONS.md`, `.planning/ops/verify-push.sh` — give to every planner, checker and executor
- `.planning/CI-EFFICIENCY.md`, `.planning/ci-runs.tsv` — budgets, the `r` metric, the trigger

### Phase 7 research notes (evidence behind the decisions)
- `.planning/phases/07-first-visible-original-scenario/07-DISCUSS-scenario.md` (S1-1..S1-11, program, mutant table, per-frame lamp table, PSG sequence)
- `.planning/phases/07-first-visible-original-scenario/07-DISCUSS-runtime.md` (S2-1..S2-12, measured ISR/strobe/re-arm bands, RAM layout, mutant list)
- `.planning/phases/07-first-visible-original-scenario/07-DISCUSS-assembler.md` (S3-1..S3-11, proposed D4 register wording, migration, tests)
- `.planning/phases/07-first-visible-original-scenario/07-DISCUSS-verify.md` (S4-1..S4-9, scenario set, judge, lint, consumer replay, cost model, lever order)
- `.planning/phases/07-first-visible-original-scenario/07-DISCUSS-rights.md` (S5-1..S5-11, manifest shape, SCN-02 route, draft D11 wording, release table)

### Prior phase artifacts
- `.planning/phases/06-stic-video-first-pass/06-CONTEXT.md`, `06-13-SUMMARY.md`, `06-14-SUMMARY.md` — Phase 7 hand-offs (data directive, ISR, D4, reader cartridge), the timing model D-05..D-09
- `.planning/phases/05-hand-controllers-and-psg-sound/05-CONTEXT.md` — controller electrical model, PSG, audio domain
- `.planning/phases/06-stic-video-first-pass/06-DISCUSS-verify.md` — expected depth of the verification note

### Repo documents
- `docs/fixture-manifest.md`, `docs/diagnostics.md`, `docs/runner.md`, `docs/RIGHTS.md`, `docs/evidence-tiers.md`, `docs/packaging.md`, `docs/provenance.md`, `docs/time-base.md`, `docs/cp1610/reference-notes.md`, `docs/input/`, `docs/psg/`, `docs/stic/`, `docs/video/`
- `data/cp1610/opcode-families.json`, `data/tiers.json`, `data/claims.json`, `data/scenarios/*.ivss`, `data/golden/`, `data/expected/`

### Fixtures and tools
- `fixtures/FX-boot-stub/`, `fixtures/FX-diag-cpu-1/`, `fixtures/FX-diag-ctrl-psg-1/`, `fixtures/FX-diag-stic-1/` (manifests and listings)
- `tools/check/manifests.cmake`, `tools/check/steps/important-gates.sh`, `tools/gate.sh`, `tools/gate-linux.sh`, `tools/pack/pack.c`, `tools/listen-demo.sh`, `tools/ci-record.sh`
</canonical_refs>

<code_context>
## Existing Code Insights

### Reusable Assets
- `tools/pack/pack.c` (`ivs_pack --ivsc`/`--patch`): turns a listing into a cartridge image; unchanged.
- Listing checker (`listing` and `pack.words` steps) and `data/cp1610/opcode-families.json`: the independent police of generated listings.
- Existing scenario grammar (timed input script, mem/reg, frame hash and pixel probes, `frame displayed`, psglog, `audio at`), `ivsr 3` reports and goldens, the three hand-written installed-consumer replays (copy-paste pattern).
- `tools/pcm` registration pattern for a build-only tool; `tools/listen-demo.sh` as the model for the owner helper.
- `needs-user-exec.ivss` and the user-content refusal paths in the runner.

### Established Patterns
- Oracle-first plans; hand-derived literals committed before code; blind second reads by isolated readers; fault sites travel with their registration; goldens re-blessed with classification in the plan that moves them.
- Executors run `gate-linux` natively; hosted CI is the x86 check; the orchestrator records hosted runs with `sh tools/ci-record.sh`, agents never call `gh`.
- Commit trailer rewrite to Sonnet 5.5 before push; sanitize scratch paths in reports.

### Integration Points
- Stub contract 2 and the `boot-stub-2` runner token (runner content tokens, listing registry).
- Manifest checker (recipe source hashing), `files` step (names rule), `important-gates` (SCN set), `golden` step (shape lint, `video-frame-pair-region`), `tests-inventory` (SKIPPED output), claims and tiers data.
- `ci-parity` and the gate wiring: no new job; the `asm` suite joins `c-tests`.
</code_context>

<specifics>
## Specific Ideas

- The spike in `07-DISCUSS-scenario.md` agreed with an independent model built from the controller literal alone (13 frames, 25 PSG records, all lamp patterns; the no-input run is silent with 7 PSG records; shifting every event +2,000 ticks gives byte-identical frames). It is a lead for the oracle plan, not evidence.
- The owner may listen to the exported WAV and look at exported frames: operator-side exports, never evidence.
</specifics>

<deferred>
## Deferred Ideas

- MOB-based sprites and collision in the scenario: Phase 8 (VID-05).
- A reference host frontend that displays the program: v2 (TOOL-04).
- Promoting the four hand-assembled legacy listings to `.asm` sources: only when a listing next needs an edit.
- A reader cartridge for the D10-VID items (Phase 6 hand-off): not required by SCN-01/02; revisit with D10 evidence collection.
- Known EXEC/GROM digests in the repo: not recorded (rights and identity are the owner's); revisit only with documented permission.
- Installing the runner or `ivs_pack` so a release is runnable: a release-packaging decision, not Phase 7.

</deferred>

---

*Phase: 07-first-visible-original-scenario*
*Context gathered: 2026-11-13*

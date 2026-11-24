# Phase 5 discuss: verification and evidence strategy (zero human UAT)

Status: recommendation (fan-out protocol, `.planning/AUTONOMY.md` section 2). Labels: [OBS] read or measured in this session from the repo, [INF] inference, [REC] recommendation, [UNK] unknown, [BG] background knowledge not re-fetched this session.
Inputs read [OBS]: AGENTS.md, AUTONOMY.md (all sections), PROJECT.md Decision Register, REQUIREMENTS.md (INP/AUD/API-03), ROADMAP.md Phase 5 and Phase 6/7 dependencies, CI-EFFICIENCY.md (D-45 budget and the Phase 4 close-out), `docs/diagnostics.md`, `docs/runner.md`, `docs/evidence-tiers.md`, `docs/time-base.md`, `docs/cp1610/second-reader-protocol.md`, `fixtures/FX-diag-cpu-1/`, `data/scenarios/`, `data/golden/`, `data/expected/` (inventories, faults, controls, important-gates), `data/discrepancies.json` (C10-C16, C20, C22, C23, C63-C68), `data/claims.json`, the profile, `core/include/ivs/ivs_run.h`, `tools/gate.sh`, `tools/gate-linux.sh`, `tools/check/` (steps, controls, `listing.cmake`), Phase 4 CONTEXT, `04-DISCUSS-ctrl.md`, `04-DISCUSS-diag.md`, `04-VERIFICATION.md`, `04-VALIDATION.md`, `04-25-SUMMARY.md`, `.planning/ci-runs.tsv`.
No emulator source was opened and no network fetch was made. No file other than this one was written. No git or gh call was made. Other decision points (PSG architecture, run-API growth, D7) are being researched in parallel; section 10 lists what this note assumes from them.

## 0. Recommendation on one screen

1. **Oracle-first, in a separate plan before any C.** Plan 05-01 writes only data and documents: sourced literal tables (controller bytes, PSG rule vectors, register-address map), hand-derived `.expect` predicates, the new checkers with their controls, and the blind second-read requests. Implementation plans consume those files; they never author their own expectations (V-01, V-02).
2. **Separate what the sources can prove from what is a project policy.** Controller: two wiki anchors plus prose-structure properties (tier-4) are the only independent oracles; the 62-row byte table is tier-4 LOW and stays `provisional` until D10. PSG: rules are derived from the AY-3-8910 pages of the 1980 GI catalog (tier-1 if the pages carry the equations; plan 05-01 must read them), and the AY-3-8914 register-address map is a separate tier-4 table (V-03, V-04).
3. **Slicing invariance and log-replay equivalence are the strongest non-circular tests** because they do not depend on any table. Make them the primary proof for API-03 and AUD-02 (V-07, V-08).
4. **One new guest fixture, `ivs-diag-ctrl-psg-1`** (about 90 to 110 hand-assembled words, one IVSC cartridge, no SDBD, at most 4 branches), two new scenarios (`diag-ctrl-psg-pass`, `diag-ctrl-psg-nokey`), four program mutants, replayed by the installed consumer. The full 62-control matrix is proven in-process through the public API with two read paths (guest read and `ivs_machine_peek`), not through 62 scenario spawns (V-10 to V-13).
5. **Two new suites, `input` and `psg`**, each with its own fault file. New suites keep fault reruns small and keep the collateral-prefix lists of existing faults untouched (V-15).
6. **Cost cap: at most 26 new named faults, about 22 new controls, about 380 new test ids, two new pure gate steps.** Projected hosted effect +15 to +28 cpu-s and +4 to +8 s wall. The hosted wall budget (median 75 s) is safe; the **cpu_s trigger (median of last 5 above 200; last 5 today 175) is the binding constraint**, so the plan pre-commits to a checkpoint and a lever order (V-16, V-17).
7. **Missing evidence is recorded, not hidden.** Claims stay `provisional`; every D10/D11 item is a discrepancy with a `measurement:` resolver and is listed in a new exact inventory that the existing `discrepancies` step cross-checks; the VERIFICATION record has three tables: automated, handed off, accepted caveats (V-20 to V-23).
8. **Ten plans in eight waves, not twenty-five** (V-27).

## 1. Facts that shape the strategy [OBS]

- Phase 5 success criteria (ROADMAP): SC1 each of 12 keys, 16 discs, side-action inputs, each player, individually, with a tier tag per value; SC2 combinations, opposite directions, ambiguous combinations under the selected model, no host mapping in core; SC3 input script gives the same guest-visible results and digests in one call or many slices; SC4 each PSG register/tone/noise/envelope rule has a passing vector, and behaviours without a primary source are tier-tagged profile data; SC5 cycle-stamped register-write log and integer chip-rate samples, PCM only where D7 puts it and outside the raw-audio evidence path.
- Today's inventory: gate `steps=33 controls=277 faults=149`, `tests-expected=2816`, 26 important gates (102 registry lines), 4 goldens, 15 runner scenarios. Median hosted `gate_s` 59 s, median `cpu_s` 164 (last 5: 59 s and 175). Budget since 04-25: median `gate_s` 75 s and median `cpu_s` 200; revisit trigger: last-5 `gate_s` median above 90 s, any run above 120 s, or last-5 `cpu_s` median above 200 (CI-EFFICIENCY.md). [INF] The headroom in cpu-s (about 25 over the last-5 median) is smaller than in wall seconds (about 30).
- Cost shape on native Linux from the Phase 4 profiles: a control about 0.04 cpu-s, a fault about 0.15 to 0.25 cpu-s (one small TU rebuild plus the suite run plus about 17 execs in the verdict), a test id about 0.003 cpu-s average (dominated by a few heavy ASan sweeps), a cold two-preset build 11 to 17 s wall on 4 cpus. `cp1610-vectors` costs about 5 cpu-s because it re-parses about 400 JSON vectors with `string(JSON)`.
- `ivs_machine_peek` is side-effect-free and reads "what a CPU read at that address returns now"; the IVSS `assert mem` clause uses it. Caps per scenario: 64 steps, 64 asserts, 64 input lines. The report key table is closed and fixed order; goldens are filtered by a key list.
- A halted CPU stops a run before any unit begins, so input events are not applied after a halt (the stop test comes first, `ivs_run.h`). A guest that must observe input has to be running.
- `tests/run/run_input.c` already runs a guest that loops reading `$01FE`/`$01FF` and asserts it is blind to input (`run.input.guest-invisible`); its comment says Phase 5 replaces it with a positive read test.
- The PSG region `$01F0-$01FF` is `impl=stub` in the sourced literal memory map; profile fields `ticks_per_psg_clock` (2, unknown-default-chosen), `power_on_fill.psg` (zero), `psg.part` (tier4, C11), memmap row tag tier4. The composite digest is v1 over domains 1 to 4; domains 16 to 18 (video, audio, input) are reported individually. CL-0006 binds the `impl`/width labels of the literal map, so flipping the PSG rows from stub to real changes a claimed artifact.
- `docs/time-base.md`: "PSG step" = 16 PSG clocks (INF), so 32 master ticks at ratio 2; both flagged "to be checked in Phase 5".
- Listing checker `tools/check/listing.cmake` checks bits=hex, row label, operand counts, range, headers; it does **not** check that a branch displacement reaches the printed target, and it does not model SDBD. `refs/` holds only the CP-1600 manual locally.
- Phase 4 summaries defer to Phase 5: fault `INPUT_EXTRA_CONTROLLER_UNCHECKED` (one comparison, no dedicated fault today), the positive PSG-port read test, C14 and C63-C67 resolvers, v1-size constants for any appended request/report field (A-02), and an SDBD operand rule only if a diagnostic needs SDBD.
- Past hosted-only failures: gcc argument evaluation order in a test helper, `_GLOBAL_OFFSET_TABLE_` under PIE, GNU `nm` output shapes, glibc signal semantics, a TERM race in a shell TAP test. `tools/gate-linux.sh` (about 3 minutes under emulation) is the container proxy.
- D4 trigger for a project assembler: more than 3 diagnostics, labels/macros, or a second author. Current diagnostics: `diag-cpu-1` (boot stub is not counted).

## 2. Oracle taxonomy: what is independent, what is circular [REC]

The same author writes the listing, the expected values and the core, and the project's second reader is the same model family. Therefore "independent" below means independent of the emulator's execution and of the implementation tables, not independent of the author (`docs/diagnostics.md` states this for Phase 4).

| Oracle kind | Independent of the core? | Used for | Tier it can support |
|---|---|---|---|
| Wiki literals: idle low byte `$FF`, key 1 `$7E`, keys 1+9 `$5A` (oldid 14808) | yes | INP-01/02 anchors | `source-expectation` (tier-4 source, LOW) |
| BSR code widths: disc 5-bit, action 3-bit, keypad 8-bit; wiki decodability statements (side+disc unique, keypad+side/disc and side+side not) | yes, as properties of the 31-row table, not values | INP-01/02 structure checks (enumerated over all pairs in C) | `source-expectation` |
| Pin-table transcription (31 controls, 2 pins each, apparent duplicate) read blind by a second reader | partly (second read of one tier-4 LOW page) | INP-01 byte literal | `source-expectation, second-read` |
| AY-3-8910 datasheet equations (tone `f = fclk / (16 x TP)`, noise and envelope analogues, 16 shapes, mixer, 4-bit amplitude) from the 1980 GI catalog pages, hand arithmetic | yes if the pages carry them [UNK until 05-01 reads them; BG: the 8910 sheet prints the tone/noise/envelope frequency formulas] | AUD-01 rule vectors | `source-expectation`, and only for rate and ordering, not edge phase |
| Closed-form rate bounds (transitions in K PSG clocks lie in {floor, ceil}(K / (8 x TP))) | yes, phase-convention free | AUD-01/02 tone, noise, envelope rate | `source-expectation` |
| Datasheet-independent algebraic properties: LFSR maximal period 2^17-1 for the chosen taps, DAC table monotonic, envelope shapes end in their documented hold state | yes (self-contained property, not hardware) | catches a wrong tap or a swapped shape without a golden | none by itself (guards, not evidence) |
| Slicing invariance (single call vs many slices, seeded schedules and writes) | yes (no table involved) | API-03, AUD-02, input log | `deterministic-repeatability` |
| Register-write log replay equivalence: replaying the logged writes into a fresh PSG gives the same sample digest | yes (second path through the device) | AUD-02 completeness (a state change that bypasses the log fails) | `deterministic-repeatability` |
| Log versus CPU-side bus trace: every logged write equals a cycle-stamped write access of the stepper to `$01F0-$01FF` | yes (two observers of one program) | AUD-02 | `original-diagnostic` when the program is the project listing |
| Hand-derived write sequence of the project listing (address, value, order), tick only as a window from the printed cycle table | yes | AUD-02, SC5 | `original-diagnostic` |
| Two read paths agree: guest `MVI $01FE` result equals `ivs_machine_peek($01FE)` equals the literal | yes (two code paths) | INP-01 | consistency, not accuracy |
| `cmake -E sha256sum` over a framed byte string | yes (not the core's SHA) | input log chain, log chain framing | `deterministic-repeatability` (framing only, labelled) |
| Self-recorded `.digests` goldens | **no** (repeat only) | regression pins | `deterministic-repeatability` |

**Circular, to avoid (anti-patterns):**
1. Computing the expected byte with the core's byte function, the profile mask table, or a helper that includes `core/src` headers. The expected table is a separate literal file.
2. Generating the `.expect` assert lines from a run (Phase 4 rule: derivation committed before the program first ran).
3. Treating agreement with MAME or jzIntv register tables, LFSR taps, DAC tables or envelope shapes as evidence. Tier-5 hints may appear only as `unknown-default-chosen` profile values (AGENTS.md; sources.json authority). Nothing is copied.
4. A second full PSG model in cmake or shell (about 150 lines duplicating chip semantics): rejected for the same reason Phase 4 rejected a second CPU interpreter for one program (shared misreading, high cost). Use closed-form bounds instead.
5. Exact sample sequences as accuracy evidence. They are `deterministic-repeatability` only, because the datasheet gives no edge-phase convention.
6. Asserting the upper byte of a port read or the readback of a PSG register as fact: both are policy (C64, new C71). The guest fixture masks the low byte (`ANDI #$00FF`).
7. Using the same plan to write the literal and the implementation in one commit (the memmap precedent: CL-0006 compares an implementation to a literal edited separately).

## 3. Decisions

### A. Evidence architecture

**V-01 [REC] Oracle-first plan.** Plan 05-01 contains no C. It produces: `docs/psg/reference-notes.md` (page-cited rules, a table of every rule with source tier), `docs/input/controller-notes.md`, `data/input/intv-2609-ntsc-family-p1.literal.txt` (one line per control: name, pins, line mask, expected active-low byte, tag, `src=SRC-id@locator`, `conf=C16`; grammar copied from the memmap literal), `data/psg/vectors/*.txt` (line-based, not JSON), `data/expected/psg-rules.ids`, the PSG register-address literal (16 CPU addresses to register roles, tag tier4, C11/C69), the hand-derived `data/golden/diag-ctrl-psg-1.expect` with every derivation step shown, discrepancies C69 onward, and the second-read requests. Implementation plans cite these files by path and hash-free (same repo). Process control, not gate-enforced: each plan SUMMARY names the oracle commit that precedes the implementation commit; the verifier checks `git log` order (Phase 4 did the same).

**V-02 [REC] Second-read protocol, copy-pasted.** Copy `docs/cp1610/second-reader-protocol.md` to `docs/input/second-reader-protocol.md` and `docs/psg/second-reader-protocol.md` with the allowed-inputs list changed (page images or the fetched page text, the protocol, no `data/`, `core/`, `.planning/`, other emulators). One blind reader per file: the controller literal (1 reader, from the archived pin page and the wiki revision), the PSG vector files (1 reader per file, from the datasheet pages). Records are `source-expectation, second-read`, never `independent-implementation-comparison` (C43 wording). A disagreement is resolved from the source, written in the literal file's note, and counted. Cost: no gate cost beyond a mechanical diff step that already exists for CP-1610; for the new files the diff is a one-time orchestrator task, committed as a `*-handoff.ids` like `vector-review-handoff.ids`. Rights: source pages are read into the gitignored cache only; only derived tables and short paraphrases are committed.

**V-03 [REC] PSG evidence has two layers, tested separately.**
- Layer F (function): rules at the level of the AY-3-8910 register numbers (R0..R15 roles: tone periods, noise period, mixer, amplitudes, envelope period and shape, I/O ports). Vectors are expressed in register numbers and PSG clocks; sources: the 1980 catalog 8910 section (printed 5-23) read directly in plan 05-01. Claims say "8910-derived rule, part identity C11".
- Layer M (map): the CPU address to register-role map for `$01F0-$01FF` (tier4, unknown-default-chosen, new C69; the AY-3-8914 may order registers differently from the 8910, [BG], unverified). Tested by a literal map file compared with the core's decode, exactly the memmap pattern (V-05).
This keeps a wrong address map from invalidating the rule vectors and vice versa, and it lets a later 8914 datasheet or D10 probe correct only layer M.

**V-04 [REC] Unknown behaviours are profile data and proven to be data.** Period 0, envelope step count and shapes under the 2-bit select (C12), noise width and taps, DAC levels, register readback, reset state, port direction semantics: each is a profile field with a tag, never a literal in `core/src/psg`. The proof is behavioural, not a grep: for each such field a test builds an instance with the field perturbed (internal profile-descriptor seam, as the memmap tests do) and requires the predicted output change (`psg.profile.<field>-changes-output`, 8 ids). A fault that hard-codes the value (for example `PSG_NOISE_TAP_HARDCODED`) must fail exactly that test. Property guards for the defaults: LFSR period is maximal for the chosen taps (a wrong tap fails without a golden), DAC table monotonic with level 0 the minimum, every envelope shape reaches its documented hold level.

**V-05 [REC] Literal map flips are separate and reviewed.** `$01F0-$01FF` moves from `impl=stub` to `impl=real` in `data/memmap/*.literal.txt` in plan 05-01 (data only, tag tier4, cite unchanged); the `memmap` suite and CL-0006 are refreshed in the plan that implements the device, with the literal edit already committed. Width and upper-byte policy go in as labelled defaults (C60, C64).

### B. Success-criterion proof plan

| SC | Automated proof | Oracles (section 2) | Tier recorded |
|---|---|---|---|
| SC1 INP-01 | Literal file (62 expected rows via 31 controls x 2 ports) generated into a probe table; suite `input` checks, for every control and both ports: derived byte (from `ivs_machine_peek` and from a guest `MVI` read, both) equals the literal; idle rows; anchors; structure properties; tier tag per row checked by the new `input` gate step; one subset (8 controls, both ports) through the real guest fixture in `diag-ctrl-psg-pass` | wiki anchors, BSR/wiki structure, pin-table second read, two read paths | `source-expectation` (rows tier4 or unknown-default-chosen); guest subset `original-diagnostic` |
| SC2 INP-02 | Named combination rows (about 24: wiki 1+9, key+key, disc+side, disc+disc adjacent, disc+disc opposite, side+side, three sides, key+disc, key+side, all closed, port independence) plus exhaustive pairwise loops over 31 controls per port (930 pairs in 2 ids): union/commutative/idempotent/monotone-clear, and the three wiki decodability claims as properties. Rows with no source are `policy` rows tagged `unknown-default-chosen`, listed as policy, not evidence. A static check that `core/` has no host mapping symbols is already `core-symbols` plus the profile field names | wiki anchor, wiki decodability, algebraic properties | `source-expectation` for anchor/decodability; policy rows carry no tier claim |
| SC3 API-03 | (a) seeded schedule fuzz with a **reading guest** (so differences show in CPU/MEM digests, not only the input chain), five slicing patterns plus slice 1 and slices that end inside an overshoot window, 8 seeds; (b) unit-start boundary test by single-step: event at the start tick of the unit that reads sees the new byte, event at start+1 sees the old one and the next unit sees the new one, with the starts observed (report `now_ticks` before each single-step), not predicted from the cycle table; (c) real fixture replayed by the runner at default slice, 1 and 7: identical reports except `host.calls` (existing `runner-process` variants); (d) flip `run.input.guest-invisible` to `run.input.guest-visible` with a reviewed golden diff (V-19); (e) two interleaved instances with different schedules equal their sequential runs | slicing invariance, unit-start contract (C67) | `deterministic-repeatability`; C67 stays a contract, not a hardware claim |
| SC4 AUD-01 | `data/psg/vectors/*.txt` (about 120 vectors, one rule each, page cite and tier per line) run by suite `psg` at register-number level; rule inventory `psg-rules.ids` (every rule has at least one vector or a profile-parameter test); 16 envelope shapes each one vector; profile perturbation tests (V-04); `psg-vectors` gate step checks grammar, tiering, cites and orphan/missing rules | datasheet equations, closed-form rate bounds, property guards | `source-expectation`; behaviour phase and unknown parameters `unknown-default-chosen` |
| SC5 AUD-02 | (a) log completeness by replay equivalence; (b) log versus CPU-side access trace; (c) effect boundary: samples before a logged write tick use the old register value and the first sample at or after it the new value; (d) sample count equals floor((now - start) / step) at every slice boundary and slice 1; (e) hand-derived (address, value, order) list for the project listing asserted through `assert psg-log`; (f) a worst-case writer (tight `MVO` loop) never exceeds the log capacity formula; (g) raw samples are integer; no float in `core/` (`c-purity` already fails `float`/`double`); (h) any reference PCM conversion lives outside `core/` (D7) and in no golden or claim | replay, bus-trace cross-check, hand-derived list | `deterministic-repeatability`; (e) `original-diagnostic` |

### C. The guest fixture and scenarios

**V-06 [REC] Fixture `ivs-diag-ctrl-psg-1`** (kind `cart`, `fixtures/FX-diag-ctrl-psg-1/diag-ctrl-psg-1.listing.txt` plus `manifest.json`, redistributable yes, MIT, author-attested, one IVSC v1 segment at `5000`, entered through boot-stub contract 1, no SDBD, no interrupt, no STIC or GROM access).

Sections (one RAM word per clause so a failure names its section):
1. **PSG programming**: write channel A tone period (low then high), mixer (tone A on, noise off, ports as inputs), channel A amplitude, then channel B tone and amplitude, noise period and the envelope registers once each (about 10 `MVO` writes, straight-line, `MOVR`/`MVII` setup; every address below `0400`, so each operand is one decle). Expected log: the exact (address, value) list in listing order.
2. **Poll table**: a loop that reads `$01FE`, masks with `ANDI #$00FF`, and on a value different from the previous stores it at `[0210+k]`; the same for `$01FF` into `[0220+k]`; stops when 8 changes have been stored. The scripted schedule (events spaced at least 600 ticks, far above one loop of about 30 CPU cycles) produces: idle `FF`, key 1 `7E` (wiki anchor), keys 1+9 `5A` (wiki anchor), a disc position, a side code, disc+side, a keypad+side ambiguous row, and one on port 1. Anchors are independent literals; the other rows are the tier-4 literal file (labelled in the `.expect` header per assert).
3. **Delay and end**: a counted delay loop (about 3,000 CPU cycles so the PSG produces enough samples for the rate-bound check), then `[020F] = marker` last, then `HLT` and three `TSTR R0` followers (Phase 4 rule: a bare `HLT` needs a follower).
The program is at most 110 words and at most 20,000 ticks from reset to halt, so slice-1 replays stay near 0.2 s per preset under ASan (V-26).

**V-07 [REC] Timing is not asserted from the cycle table in the guest predicate.** `diag-cpu-1.expect` excluded machine time for the same reason (reset-entry 5 cycles is an inference, branch counts are policy C29). The unit-start boundary is proven in-process with observed starts (SC3 b). The `.expect` holds guest state (RAM words, registers, halted) and the PSG write list; `output.audio.bytes` is printed and goldened, not predicted.

**V-08 [REC] Scenarios (4 new files, 2 listed fixtures changes).**

| Scenario | Outcome | Purpose |
|---|---|---|
| `diag-ctrl-psg-pass` | pass, exit 0 | the fixture with an 8-event schedule on both ports; `.expect` (tier `original-diagnostic`, oracle `independent-derivation`) plus `.digests` (repeatability: cpu, mem, images, instance, audio, input; video none) |
| `diag-ctrl-psg-nokey` | timeout (budget), exit 11 | same cartridge, no input lines: the guest keeps polling and the run ends by the machine-time budget with the guest still in the poll loop. Contrast with the pass scenario is the proof that controller input, not time, changes guest state (SCN-01 groundwork) |
| `diag-ctrl-psg-order` | pass, exit 0 | events for port 1 listed before port 0 at interleaved ticks, same table (checks the per-port index never means player or side) |
| `psg-log-assert-mismatch` | guest-fail, exit 10 | a wrong `assert psg-log` value must be reported as a guest failure with the assert named (the runner's new assert subject cannot pass vacuously) |
The existing `diag-input-pass` stays (its cpu/mem/images digest lines must be byte-identical; see V-19). The installed consumer replays `diag-ctrl-psg-pass` in addition (V-14).

**V-09 [REC] Runner grammar growth, minimal.** Two new assert forms: `assert psg-log count ==|!= <n>` and `assert psg-log <k> == <AAAA> <HHHH>` (tick printed in the report, not asserted). Optional third, recommended: a masked memory assert `assert mem <AAAA> & <MASK> ==|!= <HHHH>`, so a scenario can state the low byte as tier-4 fact without asserting the upper byte (C64). If the mask form is deferred, the `.expect` asserts the full 16-bit value under the profile upper-byte default and tags each line `# policy: C64`. Parser tests, a parser fuzz extension and 2 faults (`RUNNER_PSGLOG_ASSERT_IGNORED`, `RUNNER_MASK_IGNORED`). New report keys go before `report.end` and the golden key list is extended once, with a reviewed diff of every golden.

**V-10 [REC] Program mutants, not faults, for the fixture.** Four to five new lines in `data/expected/diag-mutations.ids` via `ivs_pack --patch`: `ANDI` to `ORI` on the mask, poll branch sense inverted, `$01FE` read changed to `$01FF`, tone-high write value changed (must change the log assert), delay count changed (must not change any predicate: a negative mutant proving the predicate is timing-independent). Each must end guest-fail naming its clause (the last must pass). Cost: one `ivs_pack` run and one `ivs-run` per mutant, no fault build.

**V-11 [REC] Add a branch-target rule to the listing checker.** The largest hand-assembly risk in a 100-word listing with loops is a displacement that reaches the wrong word and still runs. Extend `tools/check/listing.cmake` (about 25 lines): a branch or jump line must carry `-> <ADDR>` in its ASM text and the checker recomputes the target from the printed displacement word (rules of the CP-1610 manual pages already cited in `docs/diagnostics.md`), token `listing-branch-target`, with 2 controls. The retroactive check of `diag-cpu-1` (one loop) is free. This keeps D4 (hand listings) valid through Phase 6.

**V-12 [REC] D4 stays; set the assembler trigger now.** After this phase diagnostics are two (`diag-cpu-1`, `diag-ctrl-psg-1`); Phase 6 adds a third; Phase 7's scenario would be the fourth and fires the trigger. Record in the context: build the project mini-assembler as the first plan of Phase 7 (the 400 vector `asm`/`program` pairs plus the three listings are its acceptance corpus), not earlier.

**V-13 [REC] Splitting rejected.** Two cartridges (`ctrl` and `psg`) would reach the D4 trigger a phase sooner and double the scenario and mutant overhead for one extra localisation benefit that per-section RAM words already give.

**V-14 [REC] Installed consumer replays `diag-ctrl-psg-pass`** (`tests/installed-consumer`, plus `pkg-consumer.golden` and `pkg-consumer-calls.ids`). It must supply input events, an audio buffer and the log buffer through the public header only, so it proves the appended v1 request/report fields and their sizes from the installed package, which the in-tree suites cannot (A-02 growth is exactly what an installed header can get wrong). It prints the audio and input digests and the log count and the gate compares them to the runner golden, as for `diag-input-pass`. Cost under 1 s. Tier `installed-host-behavior`.

### D. Inventory, faults, controls, cost

**V-15 [REC] Two new suites, separate executables and fault files.**
- `input` (`data/expected/tests/input.ids`, `data/expected/faults/input.ids`): about 130 ids.
- `psg` (`tests/psg.ids`, `faults/psg.ids`): about 170 ids.
Reason: a named fault runs only its own suite, and `faults-verdict` demands that every failing vector start with an allowed prefix; adding 300 ids to `run` or `machine` would force collateral-prefix edits on about 45 existing faults. Each new suite stays under 0.3 s release and 1.5 s dev (ASan) in plain runs; the plan SUMMARY records the measured plain-run time per preset (Phase 4 habit). Split `core/src/psg/` into small TUs (`psg_regs.c`, `psg_tone.c`, `psg_noise.c`, `psg_env.c`, `psg_mix.c`, `psg_log.c`) so a fault recompiles one small file.

**Test inventory and cost (hosted native Linux, gcc, cpu-s; wall is about 0.7 x cpu-s / 4 cores plus critical-path effects):**

| Area | New ids | Naming (exact ids in `data/expected/tests/`) | Fault/control/judge cost | Added cpu-s | Notes |
|---|---|---|---|---|---|
| INP-01 singles | 64 | `input.single.p<0\|1>.<control>` 62, `input.idle.p<0\|1>` 2 | judge self-control `input.judge.table-row-changed` (a deliberately altered literal row must be detected, zero fault builds) | 0.3 | one process, table driven; two read paths |
| INP-01/02 anchors and structure | 9 | `input.anchor.*` 3, `input.struct.*` 6 | covered by the faults below | 0.1 | exhaustive loops over 31 controls |
| INP-02 combinations | 28 | `input.combo.<row>` about 24, pairwise properties 4 (930 pairs inside) | faults per V-17 | 0.2 | policy rows flagged |
| Port semantics | 10 | `input.port.*` (direction, upper byte, unplugged, drive readback; all policy) | faults per V-17 | 0.1 | C63-C65 |
| API-03 guest-visible | 18 | `input.slice.*`, `input.boundary.*`, `input.fuzz.seed<n>` 8, `input.iso.*`, `input.reset.*` | existing input faults now also reach the guest | 0.8 | reading guest, 5 patterns |
| AUD-01 rules | 110 | `psg.reg.*` 28, `psg.tone.*` 16, `psg.noise.*` 12, `psg.env.*` 22, `psg.mix.*` 10, `psg.amp.*` 10, `psg.port.*` 10 | faults per V-17 | 0.8 | vectors from text file |
| Profile perturbation | 8 | `psg.profile.<field>` | faults per V-17 | 0.2 | V-04 |
| AUD-02 samples and log | 52 | `psg.sample.*` 10, `psg.log.*` 16, `psg.slice.*` 8, `psg.fuzz.seed<n>` 8, `psg.buffers.*` 6, `psg.reset.*` 4 | faults per V-17 | 1.2 | worst-case writer, replay |
| Run-level (diag, API, digests) | 40 | `run.diag.ctrl-psg.*` 10, `run.input.guest-visible` and related 6, `run.struct.*` 4, `run.buffers.*` real audio 6, `machine.digest.*` composite v2 and psg domain 8, `headers.*` 6 | faults per V-17 | 0.8 | the flipped Phase 4 case is a rename, not an addition |
| Runner and process | 26 | `runner.parse.*` 12, `runner-process.*` 14 | faults per V-17 | 1.6 | about 40 spawns (V-26) |
| **Subtotal tests** | **about 365** | `tests-expected` about 3180 | | **about 6** | |

| Other additions | Count | Added cpu-s | Added wall s |
|---|---|---|---|
| Named faults (V-17, table below) | 26 | 4.5-7 | 1-2 |
| Controls (checker mutants) | about 22 | 1-3 | 0.3-0.8 |
| New pure steps `input`, `psg-vectors` (text formats, no `string(JSON)`) | 2 | 0.8-1.5 | 0.4-0.8 |
| New and larger C (two presets, cold) | about 1,800 lines core, 4,500 test lines | 3-6 | 2-4 |
| Runner-process spawns, 5 mutants (V-10), consumer replay | about 40 spawns | 1.5-2.5 | 0.5-1 |
| **Total projection** | | **+17 to +28 cpu-s** (central 21) | **+4 to +8 s** |

[INF] Against last-5 medians (175 cpu-s, 59 s) this gives 192-203 cpu-s and 63-67 s. Wall budget (75 s) holds with margin; cpu_s sits at the revisit trigger (200). Hence V-16.

**V-16 [REC] Cost caps, checkpoints and the pre-committed lever order.**
- Caps (stop and re-plan if exceeded): 26 new faults, 25 new controls, 2 new steps, no new workflow job, no new cache, no matrix.
- Prefer, when equal in value: an in-suite judge self-control (`input.judge.*`, `psg.judge.*`, counted by `important-gates.ids` as a mutant like `runner-process.judge.*`) over a fault; a control over a fault for checker steps; one process per matrix over one process per row (the 62-control matrix is one in-process loop, not 62 scenarios); runner-process variants (slice 1, slice 7, other cwd, repeat) only for the two e2e scenarios, a single run per preset for the rest.
- Text line formats for new vectors and literals (cmake `file(STRINGS)` and `string(REGEX)`), not JSON, because the Phase 4 profile shows `string(JSON)` re-parsing is the largest pure-step cost.
- Checkpoints, by the orchestrator with `tools/ci-record.sh`, in addition to the D-45 rule: after plan 05-03 (first PSG suite and its faults: measure real per-fault cost), after 05-07, and phase end. Projection after each: last-5 median cpu_s plus the remaining planned adds.
- Lever order if a projection or a last-5 median reaches 190 cpu-s: (1) batch the `cp1610-vectors` JSON re-parse (about 5 cpu-s, no evidence change); (2) fault reruns limited to the suites a fault names (verdict keeps its must-pass vector); (3) background `c-build` only if `gate-cpu` shows idle cores (`cpu_s / (ncpu x gate_s)` was about 0.7). Not levers: dropping a control, merging checks, weakening an oracle, a two-job split (redefines PKG-02).
- One conditional lever plan (05-L1) is reserved in the outline; it is created only when the rule fires.

**V-17 [REC] Named faults (26), each with an owner TU, must-fail id and allowed reason** (suite in parentheses; ids are final names for planning).

| Fault | Suite / TU | Must fail | Reason it proves |
|---|---|---|---|
| INPUT_EXTRA_CONTROLLER_UNCHECKED (deferred from Phase 4) | run / `machine_input.c` | `run.input.refuse.extra-controller-word` | refusal of words at or above `input.controllers` |
| INPUT_BYTE_ACTIVE_HIGH | input / `input_port.c` | `input.single.*`, `input.idle.*` | polarity |
| INPUT_PORT_SWAPPED | input | `input.single.p1.*` | index is the PSG port, not a player/side |
| INPUT_COMBO_LAST_WINS | input | `input.combo.*`, `input.anchor.wiki-1-9` | union of closed switches |
| INPUT_UPPER_BYTE_LEAK | input | `input.port.upper-byte` | upper byte is the profile policy |
| INPUT_BYTE_CACHED_PER_CALL | input | `input.slice.guest-visible`, `input.fuzz.*` | byte read at access time, not at call start |
| PSG_TONE_PERIOD_OFF_BY_ONE | psg / `psg_tone.c` | `psg.tone.rate-*` | rate bound |
| PSG_TONE_HIGH_NIBBLE_UNMASKED | psg / `psg_regs.c` | `psg.reg.tone-high-mask` | 12-bit period |
| PSG_MIXER_SENSE_INVERTED | psg / `psg_mix.c` | `psg.mix.*` | disable-bit polarity |
| PSG_NOISE_WRONG_TAP | psg / `psg_noise.c` | `psg.noise.lfsr-maximal-period` | LFSR property guard (no golden) |
| PSG_NOISE_RATE_USES_TONE_DIVISOR | psg | `psg.noise.rate-*` | noise divisor |
| PSG_NOISE_TAP_HARDCODED | psg | `psg.profile.noise-taps` | profile data not constant (V-04) |
| PSG_ENV_SHAPE_ATTACK_DECAY_SWAPPED | psg / `psg_env.c` | `psg.env.shape-*` | shape table |
| PSG_ENV_PERIOD_TIMES_16 | psg | `psg.env.rate-*` | envelope rate |
| PSG_AMP_ENV_BIT_IGNORED | psg / `psg_mix.c` | `psg.amp.env-mode` | amplitude register bit 4 |
| PSG_DAC_TABLE_HARDCODED | psg | `psg.profile.dac-levels` | profile data |
| PSG_REG_WRITE_NOT_LOGGED | psg / `psg_log.c` | `psg.log.replay-equals-run`, `psg.log.vs-cpu-trace` | log completeness |
| PSG_LOG_TICK_UNIT_START | psg | `psg.log.effect-boundary`, `psg.log.window` | access-time stamp, not unit start |
| PSG_LOG_TICK_SLICE_RELATIVE | psg | `psg.slice.log-*` | absolute ticks |
| PSG_SAMPLE_PHASE_RESET_PER_CALL | psg / `psg_sample.c` | `psg.slice.samples-*`, `psg.sample.count-*` | sample phase is instance state |
| PSG_WRITE_EFFECT_LATE_ONE_STEP | psg | `psg.log.effect-boundary` | write takes effect at its logged tick |
| PSG_STATIC_CARRY | psg | `psg.iso.*` | multi-instance isolation |
| PSG_RESET_KEEPS_STATE | psg | `psg.reset.*` | reset clears the PSG and the log |
| PSG_OUTBUF_PARTIAL_WRITE | psg | `psg.buffers.too-small-*` | capacity checked before any byte (extends API-04 to the real audio layout) |
| RUNNER_PSGLOG_ASSERT_IGNORED | runner | `runner.exec.psglog-assert` | the new assert subject cannot pass vacuously |
| RUNNER_MASK_IGNORED | runner | `runner.exec.mask-assert` | masked assert |

Existing Phase 4 faults `INPUT_APPLY_AT_RUN_START`, `RUN_EVENT_AT_TARGET_ONLY`, `INPUT_NO_FLOOR_CHECK`, `INPUT_LOG_SLICE_RELATIVE_TICK`, `INPUT_STATIC_CARRY` keep their must-fail ids; the plan only adds the guest-visible ids to their allowed prefixes if they fail there too (check `fault-collateral`).

**V-18 [REC] Controls and exact inventories.** New or changed inventories in `data/expected/`: `tests/input.ids`, `tests/psg.ids` (and `run.ids`, `runner.ids`, `runner-process.ids` additions), `faults/input.ids`, `faults/psg.ids` (plus lines in `run.ids`, `runner.ids`), `gate-steps/input.ids`, `gate-steps/psg-vectors.ids`, `psg-rules.ids`, `listings.ids` (+1), `fixtures.ids` (+`FX-diag-ctrl-psg-1`), `runner-scenarios.ids` (+4), `diag-mutations.ids` (+5), `profile-fields.ids` (new `input.*` and `psg.*` fields, each with tag), `claims.ids`, `discrepancies.ids`, `important-gates.ids` (about 30 lines; each new fault/judge/control mapped to INP-01, INP-02, API-03, AUD-01 or AUD-02), `d10-measurements.ids` (V-21), `pkg-consumer.golden`, `api-reasons.ids` (new appended reasons), `controls/*.ids` (about 22). Controls (EXPECT tokens the new steps print): `input` step: missing control row, duplicate control, tag missing, unknown source cite, bad hex byte, 31-control tiling gap; `psg-vectors` step: unknown rule, rule without vector, vector without cite or tier, unknown-default parameter without a profile tag, orphan vector id, bad grammar, unlisted file; `listing-branch-target` (2); `discrepancies` D10 inventory (2). Existing `golden`, `listing`, `important-gates`, `tests-inventory` controls already cover the new scenarios, listings and ids generically.

### E. Evidence, claims and discrepancies

**V-19 [REC] Phase 4 goldens change in a classified way.** PSG state enters the composite (hidden-state rule: it changes future guest-visible behaviour), so `IVS_DIGEST_COMPOSITE_VERSION` goes to 2. Order: (1) pin the current composite literal of an idle instance and of `diag-cpu-1` in a test before the bump (Phase 4 did this); (2) bump and re-record. The reviewed diff of `data/golden/*.digests` and `pkg-consumer.golden` must show **only** these changes: `digest.composite`, `digest.instance`, `digest.audio`/`output.audio.bytes` (now observed) and new report keys. `digest.cpu`, `digest.mem`, `digest.images` and every `step.*`, `slot.*`, `cpu.*` line of `diag-cpu-1` and `diag-input-pass` must be byte-identical, because those guests never touch the PSG: this is the regression proof that adding the PSG did not perturb the CPU. Record the classification in the plan SUMMARY and, where cheap, as a `golden-allowed-changes` comment in the golden header; the changed digest lines are the only ones a reviewer should accept. `run.input.guest-invisible` is renamed `run.input.guest-visible` and its three digest equalities become inequalities on the guest-visible words, with the input chain still independent.

**V-20 [REC] Claims (new ledger entries, ascending ids from CL-0009; `provisional` by the weakest-field rule).**

| Id | Statement scope | Tier | Artifact | Depends on |
|---|---|---|---|---|
| CL-0009 | Controller byte for each of the 31 controls on both ports equals the project literal derived from the cited tier-4 sources; second read recorded | `source-expectation` | `data/input/intv-2609-ntsc-family-p1.literal.txt` | `input.*` fields (tier4 / unknown-default-chosen) |
| CL-0010 | Combination results: wiki anchor, wiki decodability properties, union rule; policy rows excluded and named | `source-expectation` | the literal file | same |
| CL-0011 | The same input schedule gives identical guest-visible state and digests whatever the slicing (contract C67: effect at the first unit boundary at or after the tick) | `deterministic-repeatability` | `docs/runner.md` or `docs/input/controller-notes.md` | `input.*` |
| CL-0012 | PSG rule vectors derived from the cited AY-3-8910 pages hold for the register-number-level model; address map and unknown parameters excluded | `source-expectation` | `data/psg/vectors/` | `psg.part` (C11, tier4), `ticks_per_psg_clock` |
| CL-0013 | A run emits a register-write log and integer chip-rate samples that are slicing-invariant, replay-complete and consistent with the CPU-side write trace | `deterministic-repeatability` | `docs/psg/reference-notes.md` | `psg.*` |
| CL-0014 | `diag-ctrl-psg-1` reaches the hand-derived guest state and write list from a power-on reset through the boot stub | `original-diagnostic` | the listing | `reset.r7`, `cartridge.slots`, `input.*`, `psg.*` |
| CL-0015 | The installed package, consumed by the separate consumer, reproduces the runner's input, audio and log results | `installed-host-behavior` | `tests/installed-consumer/main.c` | none beyond CL-0014's |
Claim wording must avoid the tripwire words (accurate, validated, hardware, and so on) or carry `[claim:CL-NNNN]` on the line.

**V-21 [REC] Discrepancies and the D10/D11 inventory.**
- Update: C11 (decision recorded: 8910-derived function rules, 8914-class label, still open), C12 (per-parameter status: rules taken from the datasheet where read, the rest listed and profile-tagged), C14 and C15 (D6 stands), C16 (second read recorded; still tier4 LOW; resolver D10), C23 (D7 outcome), C63-C67 (each gets its Phase 5 default and the test id that exercises it).
- New, numbered from C69 (each with a default policy and a resolver): C69 CPU address to PSG register-role map and whether the 8914 reorders the 8910 registers; C70 PSG clock ratio and the chip-rate sample period (ratio 2 is an emulator hint; step 16 clocks is INF); C71 CPU read of a PSG register (readback value, upper byte); C72 port direction bits and the value an output-driven port reads back (links C65); C73 PSG power-on and reset register state beyond the zero default; C74 sample alignment after a register write (effect tick versus sample boundary; a project contract, not a hardware claim, like C67).
- **`data/expected/d10-measurements.ids`** (new, exact): one line per owner-measurement item, `<id> <discrepancy> <what the owner records> <procedure doc anchor>`, about 14 lines (controller bytes for all 31 controls both ports and which connector is which; idle upper byte; unplugged port; output-driven readback; adjacent and opposite disc closures; PSG tone and noise capture; envelope shapes; DAC levels; period 0; register readback; power-on state; PSG clock ratio; address map probe). The existing `discrepancies` step gains one check: every discrepancy whose resolver starts `measurement:` appears in the inventory, and every inventory line names an existing open discrepancy (2 controls). Cost under 0.2 s. This makes "missing evidence" a gate-visible list rather than prose.
- D11 items (real EXEC reading the controllers; commercial game input and audio) are listed in the same file with the tag `D11`; there is no Phase 5 scenario for them (SCN-02 is Phase 7), so no skip-ok scenario is added.

**V-22 [REC] D10 reader diagnostic is a Phase 7 deliverable, the procedure is a Phase 5 document.** A reader cartridge that shows the 31 x 2 byte values to a human needs the STIC and a font (Phase 6/7); without video the owner cannot read RAM. Phase 5 writes `docs/input/d10-procedure.md` (what to press, what to record, the record format that a later ingest step will validate) and lists the diagnostic under Phase 7 in the roadmap notes. [UNK] whether the owner has an Intellicart or equivalent to run an unsigned cartridge on real hardware; that is an owner question and does not block Phase 5.

**V-23 [REC] How the VERIFICATION record separates automated from handed-off.** Same frontmatter as Phase 4 (`status: passed`, `score: N/N`, no human gate for the phase goal) plus three tables.
1. **Automated** (one row per SC and per requirement): the command or test ids (`sh tools/gate.sh` final line tokens, `ctest -R '^(input|psg)$'`, `build/release/runner/ivs-run ... diag-ctrl-psg-pass.ivss`), the oracle kind from section 2, the tier recorded, and the verifier's own recomputation (the verifier re-derives the `.expect` words and a sample-count formula in a scratch script, as the Phase 4 verifier did).
2. **Handed off** (`owner-handoff`): item, why a tool cannot judge it (subjective, physical, owner-held), the discrepancy id, the exact owner action (at most 5 minutes each where possible), how the result is ingested, and the claim that stays provisional until then. Rows: subjective sound quality of any host PCM (D7, not a Phase 5 artifact unless a reference PCM helper ships); real controller feel and ergonomics (not core); the D10 list of `d10-measurements.ids` summarised by count; D11 real-EXEC controller and audio behaviour; a tier-1 AY-3-8914 datasheet if the owner holds one (scan stays in the local cache).
3. **Accepted caveats**: the shared-misreading statement, tier-4 LOW pin table, C67 granularity, sample-exact goldens are repeatability only, `BUFFER_TOO_SMALL` now reachable through the public API for audio (closes the Phase 4 warning 3; say so), plus any budget notes.
`human_verification_required`: none for the goal; the handoff table is non-blocking missing evidence, counted (`handoff-items=<n>` from the inventory) so it cannot silently shrink.

### F. Risks

**V-24 [REC] Scope creep guards (Phase 6 and 7).**
- No STIC, no interrupts from devices, no BUSRQ stalls, no frames-as-events: the PSG advances by catch-up to CPU access times only. `IVS_RUN_F_RESERVED_FRAME` stays reserved.
- No host audio, no WAV writer, no resampler, no `--dump-audio` runner option (the rate check is in-process, V-03). D7: any reference PCM is a function outside `core/` (or not built at all in Phase 5), appears in no golden or claim, and the raw-audio digest never includes it (SC5 g).
- No SDBD diagnostic, no debugger/trace feature, no save-state work (Phase 8). Do not add ECS ports or words 2 and 3 of the input event.
- Phase 6 will change timing (bus steals, interrupts, a stub contract 2). Guest predicates must therefore be state-only (V-07); every timing-bearing golden is `.digests` (repeatability) with a documented re-bless, and the fixture must not depend on stub internals beyond contract 1.
- Whoever changes `ivs_run.h` appends only, with new v1-size constants and `run.struct.*` tests (A-02).

**V-25 [REC] Determinism hazards, each with a named check.**
| Hazard | Check |
|---|---|
| 16-bit counters and LFSR under `int` promotion (gcc and clang agree on the standard, but `-Wconversion` is only a warning class) | internal state in `uint32_t` with explicit masks after every operation; dev preset runs UBSan; release and dev reports compared byte-equal by the existing `golden` step |
| Struct padding in log records or sample frames hashed by memory | digest framing writes fields explicitly (`be64`, `be32`), never hashes a struct; `psg.log.framing` test with a hand-framed expected byte string and `cmake -E sha256sum` |
| Sample format endianness and signedness | integer samples defined as little-endian `uint16` or `int16` written byte by byte; `psg.sample.bytes` fixes the byte order literally |
| Argument evaluation order in new test helpers and table loops (hosted-only failure before) | no call with two side-effecting arguments; `rd(next(), next())` pattern banned by review; `gate-linux.sh` run |
| Uninitialised log/sample bytes beyond the written count | `psg.buffers.*` fills buffers with a sentinel and checks that the tail is untouched; `OUTBUF_PARTIAL_WRITE` fault |
| Instance size or layout drift between presets | the existing "instance size changed" static assert and whole-storage `memcmp` helpers (`bf_refused`) extend to the PSG rows |
| New public symbol names rejected by GNU `nm` allowlist | add to `core-symbols-allow.txt` in the same plan; the step runs under `gate-linux.sh` |
| Float creeping in for DAC or envelope | `c-purity` bans `float`/`double` and float literals in `.c`; the DAC is an integer table in the profile |
| Slice-1 replays blow the wall budget | scenario and unit caps in V-06; no `sleep`, no wall-clock in any new test (the Phase 4 TAP TERM race was a timing-sensitive shell test) |
| Hosted-only classes again | run `sh tools/gate-linux.sh` before pushing plans 05-02, 05-05, 05-07, 05-08, 05-09, 05-10; shell and cmake new code dash-safe and cmake 3.25-safe (no `pushd`, no `[[`, no `string(JSON)` new use) |
| Two read paths drift (guest read versus peek) | `input.single.*` asserts both; peek must stay side-effect-free for the PSG ports (`run.observe.*` extended with the PSG region) |
| Log capacity underestimated | capacity formula derived from the shortest possible write instruction; `psg.buffers.worst-case-writer` and a refusal test at exact capacity minus one |

**V-26 [REC] Process-spawn budget.** New runner-process spawns about 40: 4 scenarios x 2 presets (8), 2 e2e scenarios x 3 variants x 2 presets (12), 5 mutants (5), judge self-controls reuse existing fakes (0), consumer (1). The matrix is not per-row. A scenario never exceeds 20,000 ticks; the audio buffer is allocated once by the runner for the slice, not per tick. No ctest test is added per scenario: they join the existing `runner-process` TAP script, whose id list grows by the exact new lines.

### G. Plan and wave outline

**V-27 [REC] Ten plans, eight waves (+ one conditional lever).** Phase 4's 25 plans came from one cross-cutting pin or inventory plan per concern (04-05, 04-08, 04-13, 04-15, 04-21, 04-24, 04-25), two lever plans and per-feature fault plans. Here each plan creates its own inventory lines, faults and controls, records land in the plan that makes the artifact, and cross-cutting registry work is one closing plan. Cap per plan: at most 5 tasks, 8 faults, 25 controls; a plan over the cap splits.

| Plan | Wave | Content | New faults / controls (about) | Gate before push |
|---|---|---|---|---|
| 05-01 Sources and oracles first | 1 | docs and data only: AY-3-8910 page reading, controller literal, vector files (text), rule inventory, address-map literal, `.expect` derivation, C69-C74 and updates, steps `input` and `psg-vectors` with controls, listing branch-target rule, `d10-measurements.ids` and its discrepancies check, blind second-read requests and handoff | 0 / 17 | sh, dash |
| 05-02 API/ABI and profile growth | 2 | appended request/report fields with v1-size pins, `ivs_info` audio layout, new reasons, profile `input.*` and `psg.*` fields with tags, composite literal pin then v2 bump, api-effects rows, headers suite, core stubs for the PSG device with idle behaviour | 0 / 2 | sh, dash, linux |
| 05-03 PSG register file | 3 | address map (layer M) against the literal, writes and reads, reset, mixer register and port direction bits, digest row, memmap literal flip, suite `psg` created, first measurement checkpoint | 3 / 0 | sh, dash |
| 05-04 Input byte and port reads | 4 | byte function from the literal-derived profile table, ports at `$01FE/$01FF` through the PSG, peek and guest read paths, suite `input` created, flip `run.input.guest-invisible`, `INPUT_EXTRA_CONTROLLER_UNCHECKED` | 6 / 0 | sh, dash |
| 05-05 Tone, mixer, amplitude, sampling | 4 (parallel with 05-04 only if file-disjoint; default sequential) | tone, mixer, amplitude, chip-rate sample emission, audio buffer and digest, real BUFFER_TOO_SMALL for audio, rate-bound vectors, slicing | 6 / 0 | sh, dash, linux |
| 05-06 Noise and envelope | 5 | noise LFSR, envelope shapes and rates, profile perturbation tests, property guards | 5 / 0 | sh, dash |
| 05-07 Write log | 6 | log buffer, replay equivalence, bus-trace cross-check, effect boundary, worst-case writer, second measurement checkpoint | 4 / 0 | sh, dash, linux |
| 05-08 Runner and scenarios | 7 | `assert psg-log`, masked assert, report keys, `psg-log-assert-mismatch`, parser fuzz extension, goldens key list, classified golden diff (V-19) | 2 / 0 | sh, dash, linux |
| 05-09 Fixture and end to end | 7 | `ivs-diag-ctrl-psg-1` listing and manifest, scenarios, mutants, installed consumer replay, `pkg-consumer.golden`, CL-0014 and CL-0015 | 0 / 0 | sh, dash, linux |
| 05-10 Close-out | 8 | claims CL-0009 to CL-0013, discrepancy status updates, important-gates registry lines, record reconciliation, D-45 evaluation and CI-EFFICIENCY entry, VERIFICATION evidence table with the handed-off list | 0 / 3 | sh, dash, linux |
| 05-L1 (conditional) | on trigger | lever from V-16 | | |

Waves are sequential by default (shared files: `ivs_run.h`, profile JSON, digest code, `important-gates.ids`). 05-04 and 05-05 can run in parallel worktrees only after 05-03 if the planner confirms their file sets are disjoint; the saved time is small against the merge risk, so the default stays sequential.

## 4. Options considered and rejected

| Option | Why not |
|---|---|
| Golden digest of the whole 62-control matrix as the proof | repeatability only; says nothing about correctness; one reviewed re-bless would hide a wrong table |
| 62 runner scenarios or 62 asserts through the process boundary | 124 spawns for no extra evidence over one in-process loop that uses the same public API as the consumer; the e2e subset covers the boundary |
| A cmake PSG reference model producing expected samples | shared misreading, large, and edge phase is not in the source; closed-form bounds give the independent part |
| A `--dump-audio` runner option plus an external transition counter | new file-output surface in an untrusted-scenario tool (D-22) for a check that runs in-process with the public buffer |
| One fixture per requirement (ctrl and psg split) | trips D4 trigger a phase early, doubles overhead (V-13) |
| Putting new tests into suites `run` and `machine` | forces collateral-prefix edits on about 45 existing faults and slows every fault rerun (V-15) |
| Electrical mode switch or raw-byte input for the controller to "test electrical combinations" | D6 forbids a second input path; ambiguity is a table property tested once |
| Hosting INP-01 in `.json` vectors | cost: `string(JSON)` re-parse; line-based text is cheaper and matches the memmap literal precedent |
| Add a macOS CI leg for the new float-free integer code | no Apple-only escape has occurred; D12 stands |
| A human listening check as a gate | subjective sound is handed off, not automated; the raw-audio proof does not need ears |
| An assembler now (as1600, libasm) | D4; the added risk in Phase 5 is branch targets, solved by V-11 at 25 lines |

## 5. Adversarial pass against this recommendation

1. **"The 62-row literal is the profile table copied twice."** True for the byte values; the independent parts are the anchors, the structural properties from the prose sources, the two read paths, and the second read of the pin page. The record says `source-expectation, second-read`, tier-4 LOW, `provisional`, and D10 is the only upgrade. The recommendation never claims the table is right, only that the core reproduces the documented table and that two ways of reading it agree.
2. **"If plan 05-01 finds that the 1980 catalog 8910 pages do not print the equations, SC4 loses its tier-1 anchor."** Then rule vectors become `unknown-default-chosen` profile defaults with only the structural and rate properties (closed-form from the project's own stated model) as checks, the claim CL-0012 says so, and C12 stays open; Phase 5 still passes its SC4 wording (documented, tested, tier-tagged profile data). Plan 05-01 must record which case holds before 05-03 starts; this is the first unknown below.
3. **"26 faults and 365 ids will tip cpu_s over 200 and trigger a revisit."** Possible; the trigger is a revisit, not a failure, and V-16 names the levers and the checkpoints. The estimate range is wide because the PSG TU cost per fault is unmeasured; 05-03 measures it before 05-04..07 spend the budget. If the measurement is above 0.4 cpu-s per fault, drop the six lowest-value faults (the noise-rate and envelope-period duplicates are the first to go) in favour of judge self-controls.
4. **"The replay-equivalence test is circular: log and state come from one write handler."** It is a second path through the device, and its failure mode (a state change that bypasses the log) is exactly the bug class that makes raw logs lie. It is `deterministic-repeatability`, labelled so. The bus-trace cross-check adds a different observer (the stepper) for the writes themselves.
5. **"Skipping the guest timing assertion weakens API-03."** The guest predicate is state-only on purpose; the timing contract is tested with observed unit starts and slicing equality, which is what SC3 asks. Hardware granularity (C67) is a labelled contract with a profile-flag reversal path.
6. **"Branch-target rule is gate scope creep."** It is 25 lines and one token for the single most likely hand-assembly error in the phase's largest listing; without it the only catch is that the program happens to misbehave.
7. **"Text vector files invent a format."** It copies the listing and memmap-literal line grammars (comment lines, header keys, one record per line), so the checker is a copy of an existing one; the alternative costs about 2 cpu-s per gate.
8. **"The audio check cannot detect a wrong sample period."** Correct for the datasheet-level claim; the period (C70, ratio and 16-clock step) is a profile default whose value is tested for consistency (sample count formula) and recorded as unknown; D10 tone capture resolves it.
9. **"Eight waves is still long."** The waves follow real dependencies (API growth, register file, then rules that need it); parallelism buys little because shared files collide. The saving versus Phase 4 comes from fewer plans, not parallel plans.
10. **Reversal triggers for this strategy:** a tier-1 AY-3-8914 datasheet or schematic (re-baseline layer M and several C12 items to tier 1); the owner supplies D10 measurements (re-tier INP-01 and AUD-01 rows to `measured-hardware`); cpu_s trigger fires (V-16 levers); Phase 6 timing changes force a re-bless of timing-bearing goldens (documented, expected).

## 6. Unknowns [UNK]

1. **Do the AY-3-8910 pages of the 1980 GI catalog (printed 5-23) print the tone, noise and envelope equations, the 16 shape diagrams and the amplitude behaviour?** The repo records only that the catalog lists the 8910 as the 8900 system's sound generator; `refs/` has no copy. Plan 05-01 reads it (gitignored cache) and records the result before anything else. Biggest swing factor for SC4's tier.
2. **How faithfully can the controller pin table be second-read?** One archived page, LOW confidence, an apparent duplicate pin (C16). A blind reader may disagree on rows; each disagreement stays a tier-4 LOW row with both readings noted. Hardware measurement (D10) is the only real fix.
3. **Real per-fault and per-suite hosted cost of the PSG suites.** Everything in the cost table is projected from Phase 4 profiles; the first checkpoint (after 05-03) replaces estimates with measurements, and last-5 cpu_s sits at 175 against a 200 trigger.
4. Whether the AY-3-8914 register order differs from the 8910 (C69), whether CPU reads of PSG registers return values (C71), the PSG clock ratio and sample step (C70), and the upper byte of a port read (C64): all policy defaults; tests must keep them behind profile fields.
5. Whether the owner can run an unsigned cartridge on hardware (D10 route). Not blocking.

## 7. What this note assumes from the other decision points

- **D7 / PSG architecture**: raw output is integer chip-rate samples into the caller-owned audio buffer with a nonzero `audio_bytes_per_frame` in `ivs_info`; PCM conversion is outside `core/` and outside the evidence path. The audio digest must be a **streaming** hash over sample bytes (or records of fixed machine-time size), never a chain over per-call records (the Phase 4 video/audio chain text `index || len || record` would make the digest depend on slicing; flag to the API/PSG agents).
- **Run API**: a register-write log reaches the host through a caller-owned buffer (or a tagged stream) with a capacity formula that the shortest write instruction bounds; appended fields carry v1 sizes; the log record has an explicit byte framing; the log has its own digest (domain 19 or folded into 17, either works for V-25).
- **Profile**: `input.port_map`, `input.line_model`, `input.idle_byte`, upper-byte and unplugged policies, 31 control line masks, `psg.*` parameter fields exist with tags; a test seam can build an instance from a perturbed descriptor.
- **Input**: the byte is a pure function of the switch words in force and the port drive, evaluated at access time inside the unit; `peek` calls the same function without side effects.

# Phase 1: Evidence, Profile and Rights Foundations - Context

**Gathered:** 2026-11-09
**Status:** Ready for planning
**Mode:** advisor fan-out with autonomous selection. The owner delegated decision gates on 2026-11-09 (`.planning/AUTONOMY.md`). Six parallel research agents covered: first profile, time base, record formats and checks, rights and provenance, CP-1610 notes, and language/build. Each synthesized recommendation was auto-selected. Decisions marked "Delegated decision" must also be entered in the `PROJECT.md` Decision Register.

<domain>
## Phase Boundary

No machine code is written in this phase. It delivers the project's evidence foundation as tracked docs and data:

- the first-profile record (FND-01)
- the evidence-tier vocabulary and claim ledger (FND-02)
- a discrepancy log seeded with C1–C28 (FND-03)
- the fixture-manifest schema (FND-04)
- the no-copy provenance rule and review checklist (FND-05)
- page-cited CP-1610 reference notes read from the GI manual page images (FND-06)
- the time-base glossary with D2 decided (FND-07)

Alongside these, the phase adds:

- one zero-dependency local gate command that machine-checks all of the above, with negative controls
- the delegated gate decisions (D1, D3, D9) recorded
- the repository published publicly once a hygiene check passes

Not in scope:

- CPU, STIC, PSG or loader code (Phase 2+)
- the CMake build and test skeleton for the core (Phase 2)
- fixture bytes (Phase 4)
- PAL or variant profiles (v2)
</domain>

<decisions>
## Implementation Decisions

### Gate decisions (delegated; record in PROJECT.md Decision Register)

- **D-01:** (D1: language and build) C17 + CMake (floor 3.25, Presets v6) + Ninja + CTest, about 75% confidence. Rust stays the named alternative.
  - Flip conditions: recurring UB found by UBSan or fuzzing despite the integer rules, Rust sanitizers reaching stable, or a Rust-first consumer.
  - Standard settings: `C_STANDARD 17`, `C_EXTENSIONS OFF`, hidden visibility, a hand-written public header.
  - Warnings: schwaystation's list with `-Werror` in the gate, plus `-ffp-contract=off`. The core uses no floats.
  - Tests: an in-repo harness of about 150 lines (no Unity) and an exact `tests/expected-tests.txt` inventory. Register tests via `include()`, per gabbaboy D-037.
  - Phase 1 only records this decision. Phase 2 builds it.
  - **Reversibility:** costly. The whole core, build and installed-consumer packaging depend on it.
- **D-02:** (D3: first profile) a family-level profile `intv-2609-ntsc-family-p1` that models documented function. It does not pin a physical PCB revision. Per-field defaults are in D-10.
  - Board-pinned profiles come later as new profile ids, for example `…-p2-owner-unit` once D10 measurement exists.
  - Aliases (Tandyvision One, GTE MC100, Sears) are recorded as `alias-candidate, unverified`. None is claimed.
- **D-03:** (D2: time base) the canonical machine-time unit is the **master tick**, defined as one period of the profile's crystal.
  - Type: `uint64_t`, absolute since reset, origin 0.
  - The CPU cycle is a profile ratio, `ticks_per_cpu_cycle` (NTSC 4).
  - Contract: `run(target_tick)` returns `now_tick`, `overshoot_ticks` and a stop reason. A target above 2^63 is refused with an explicit stop reason.
  - The tick is a unit, not a stepping granularity. The CPU still advances in whole cycles under Option B catch-up scheduling, with no lockstep.
  - **Reversibility:** one-way once Phase 2 ships an API. The unit appears in the ABI, traces, save states and goldens, so changing it would break all of them.
- **D-04:** (D9: licence, visibility, name)
  - Licence: **MIT**, copyright line `Copyright (c) 2026 iamverysmartvision contributors`. This matches the sibling convention (gabbaboy). Each new source file carries one `SPDX-License-Identifier: MIT` line; no REUSE tooling.
  - Visibility: the repo becomes **public on GitHub via `gh`** (`szTheory/iamverysmartvision`) at the end of this phase, only after the hygiene check (`.planning/AUTONOMY.md` §6) passes.
  - Name: keep `iamverysmartvision`. Never use "Intellivision" in a package or product name. The README carries a non-affiliation and trademark notice.
  - **Reversibility:** one-way. Publication cannot be withdrawn from forks or caches.
- **D-05:** D5 (system images), D6 (controller representation), D7, D8 and D12 are **not decided here**. They are decided by the same fan-out protocol when their phase needs them. D10 and D11 stay hard stops; their evidence is marked missing.

### Record formats, layout and checks

- **D-06:** Format structured records are **JSON**. Prose lives in Markdown, and the checker never parses Markdown tables.
  - Why JSON: D1 = CMake means `cmake -P` with `string(JSON)` and `file(SHA256)` gives a zero-new-dependency, cross-platform checker on the toolchain the project needs anyway. It is sibling-proven (gabbaboy `VerifyFixtureManifest.cmake`), handles nested manifest fields naturally, and avoids awk dialect drift.
  - JSON has no comments, so records carry a `note` string field where needed.
  - Rejected alternatives: deb822 `.rec` + awk (dialect risk, second checker language); TOML/YAML (parser dependency); Markdown tables as source of truth (brittle).
- **D-07:** Layout. These are project artifacts; `.planning/` remains GSD process state only.
  - `docs/`
    - `evidence-tiers.md`
    - `provenance.md` (no-copy rule plus review checklist)
    - `time-base.md`
    - `cp1610/reference-notes.md`
    - `provenance/CONSULTATIONS.md`
    - `RIGHTS.md`
  - `data/`
    - `profiles/intv-2609-ntsc-family-p1.json`
    - `tiers.json` (both vocabularies, see D-09)
    - `claims.json`
    - `discrepancies.json`
    - `sources.json` (reference-document registry)
    - `cp1610/opcode-families.json`
  - `data/expected/*.ids`: exact inventories (discrepancy ids C1–C28, profile field paths, claim ids, control names).
  - `fixtures/`: per-fixture `manifest.json` beside the bytes. It is empty in Phase 1 apart from the schema.
  - `tools/gate.sh`: the single local command.
  - `tools/check/*.cmake`: generic checkers. Cap them small and prefer one generic record checker to many bespoke ones (lesson GB-CI-004: gabbaboy's gate grew to about 4,000 lines). Each `-P` script sets an explicit `cmake_policy` baseline.
  - `tools/check/controls/<name>/`: seeded bad trees plus an `EXPECT` failure-token file.
  - `refs/`: gitignored local cache for third-party PDFs. Tracking anything under it fails the gate.
- **D-08:** IDs
  - `C<n>` discrepancies: never reused; retired entries get `status: withdrawn` and are never deleted.
  - `CL-NNNN` claims: monotonic.
  - Profile slug, with dotted field paths (`stic.part`).
  - `FX-<slug>` fixtures.
  - `SRC-<slug>` sources.
  - Docs cite claims as `[claim:CL-0007]`.
  - The ID set must equal `data/expected/*.ids`, so removing an entry needs a visible inventory edit.
- **D-09:** Two distinct vocabularies. Never conflate them.
  - (a) **Source-authority tags** for profile fields and facts: `tier1-primary-doc`, `tier2-first-party-adjacent`, `tier3-service-hw`, `tier4-community`, `tier5-emulator-hint` (never evidence on its own), `inference`, `measured` (reserved for D10), `owner-decision` / `delegated-decision`, `unknown-default-chosen`.
  - (b) **Evidence tiers** for claims, the eight from REQUIREMENTS: source expectation, public vector, original diagnostic, independent-implementation comparison, measured hardware, deterministic repeatability, playable guest progress, installed-host behavior.
  - A claim lists the profile fields it depends on and cannot claim a higher standing than its weakest field. A value backed only by tier 5 must be `unknown-default-chosen`.
- **D-10:** Profile record one object per field, with keys `value`, `tag`, `cite`, `conflict` (C-id or null), `resolving_evidence`, `revisit_trigger` and `decision`. Blank values fail. Defaults:
  - `model`: "Mattel Intellivision Master Component, 2609 family" (tier2).
  - `board_revision`: `unspecified-2609-family` (unknown-default-chosen).
  - `region_video`: NTSC (tier1, GI 1980 STIC section).
  - `stic.part`: AY-3-8900-1 (tier1). STIC1A is explicitly not modelled.
  - `psg.part`: "AY-3-8914-class, Intellivision register map at $01F0–$01FF" (tier4). The primary datasheet is `none-found`, C11 stays open, and AUD-01 must tag each rule separately.
  - `crystal_hz`: rational `{num: 3579545, den: 1}` (tier1-nominal, GI 1980). 315/88 MHz is recorded as a note tagged `inference`. The core never reads Hz; only host adapters use it.
  - `ticks_per_cpu_cycle` = 4 (inference).
  - `ticks_per_stic_clock` = 1 (tier1).
  - `ticks_per_psg_clock` = 2 (tier5-emulator-hint, so unknown-default-chosen).
  - `scanlines_per_frame` = 262 and `cpu_cycles_per_scanline` = 57 (tier4, C7). A nominal frame is 14,934 CPU cycles = 59,736 ticks. A frame is a timeline event and must never be derived by modulo.
  - `reset.r7` = $1000 (tier4). The GI manual says the start PC is device-supplied over the bus via IAB (pdf 21), so whether the EXEC or the board supplies it is `unknown-default-chosen` (Phase 2 / D5). Other registers and flags at reset are undefined per the manual, with zero chosen (`unknown-default-chosen`).
  - `power_on_fill` = `zero` for RAM, GRAM, STIC and PSG (unknown-default-chosen). The schema allows `zero | pattern(v) | seeded(algo,seed)`. A non-default fill is recorded in every run, report and snapshot. A fill-sensitivity matrix is noted for later phases.
  - `cartridge`: one slot, ROM-only 10-bit decles, no bank switching, cartridge RAM, ECS or IntelliVoice. Address windows come from the per-content descriptor, not the profile (unknown-default-chosen).
  - `system_images`: a required EXEC/GROM slot whose digest is bound at run time; a different digest means a different profile identity. Policy is pending D5 (unknown-default-chosen).
- **D-11:** Time-base glossary terms (`docs/time-base.md`):
  - Crystal (master oscillator).
  - Master tick.
  - CPU cycle: equals the GI "micro cycle" and the unit of every manual cycle count and of 14,934 per frame.
  - CPU input clock: two-phase; one CPU cycle is two input-clock periods (manual). NTSC is crystal/2, about 1.79 MHz [INF]. Glossary only, never a code unit.
  - Time slot TS1–TS4: four per CPU cycle.
  - STIC clock.
  - PSG chip clock.
  - PSG step: 16 PSG chip clocks [INF]. Its master-tick equivalent is derived from `ticks_per_psg_clock` (2 × 16 = 32 ticks under the current default), consistent with the tier-4 tone formula F = crystal/(32·P). To be checked in Phase 5. (Corrected at plan-check iteration 3: the original draft said 16 ticks, which was unit-inconsistent.)
  - Frame (event).
  - The bare word "clock" is banned. Profile keys carry unit suffixes (`_ticks`, `_cpu_cycles`) and are converted once at load. Core internals, traces and goldens use ticks. A CPU-cycle convenience helper lives in the runner, not the core API. Wall-time and PCM conversion happen only in host adapters, with exact rational accumulators.
  - A named-fault control for unit swap and a "NTSC frame = 59,736 ticks = 14,934 cycles" check are queued for Phase 2.
- **D-12:** Gate and automated checks. Run with `sh tools/gate.sh`. Phase 2 and later phases append build and CTest steps to the same script, and CI runs exactly this script. It prints a visible summary, for example `claims=n pending-resolvers=m unknown-fields=k controls=n/n`. Checks:
  1. Every expected profile field exists with a known tag, non-empty value and cite; `unknown-default-chosen` requires `resolving_evidence`; D3 is cited.
  2. Discrepancies are set-equal to C1–C28, and each has a `default_policy` and a `resolver` (`test:`, `measurement:` or `pending:`). Pending resolvers are counted, not failed, in Phase 1.
  3. Claims resolve: the profile exists, the tier is known, and the artifact path exists. Every `[claim:…]` in `README.md` and `docs/` resolves.
  4. Claim-language tripwire: lines in `README.md` and `docs/` matching `supports|compatible|passes|accurate|verified|runs` need a `[claim:…]` tag or a `[nc]` waiver.
  5. Fixture manifests are fail-closed (D-13).
  6. `git ls-files` versus manifests: ROM-like extensions (`.int .bin .rom .itv .cfg .cc3`), PDFs or anything under `refs/` that is unlisted fails.
  7. Denylist grep for emulator authors, names and licence headers (`Zbiciak`, `jzintv`, `Nathan Woods`, `Palazzolo`, `license:BSD-3-Clause`), excluding `docs/provenance/` and `.planning/`.
  8. Every CP-1610 table row carries a page-cite token (`pdf <n>`); the sources registry entries are complete.
  9. A 1024-pattern coverage check: every 10-bit decle pattern maps to exactly one opcode family or to an explicit `unspecified-by-manual` class. Run it in `cmake -P` if it finishes in under about 2 s, otherwise move it to a Phase 2 C test.
  10. Hygiene check for publication: author emails are noreply only, no secret patterns, no absolute home paths in tracked files.
  11. CRLF, non-ASCII keys, empty files and empty inventories fail (no vacuous pass).

  **Negative controls:** each runs the real checker on a seeded bad tree and requires a nonzero exit plus the expected token, so a control cannot pass for the wrong reason. Cases:
  - missing C17; duplicate id
  - blank profile value; unknown tag
  - dangling claim artifact; untagged claim word; missing `[claim:CL-9999]`
  - manifest `redistributable: unknown` with `committed: true`; `yes` without licence evidence; misspelled key
  - unlisted file under `fixtures/`; a file under `refs/` tracked
  - denylisted string; CRLF; empty record file
  - one positive good tree

  The control list is itself an exact inventory.
- **D-13:** Fixture-manifest schema (`schema_version: 1`):
  - `id`
  - `kind`: `original-source | original-built | third-party-test | system-image | third-party-doc | derived-output`
  - `committed`: bool
  - `path`
  - `local_path`
  - `source`: `{origin, pin, accessed, holder}`
  - `recipe`: `{tool, tool_version, tool_sha256, tool_licence, command, sources[{path, sha256}]}`
  - `inputs[]`
  - `outputs[{path, sha256, size_bytes}]`
  - `licence`: SPDX expression, `LicenseRef-*` or `NOASSERTION`
  - `licence_evidence`: `{url, text_sha256, accessed}`
  - `copyright[]`
  - `notices[{path, sha256}]`
  - `redistributable`: exactly `yes | no | unknown`
  - `allowed_use`: subset of `local-test | ci | repo | release`
  - `rights_review`: `{status: owner-confirmed | author-attested | unreviewed, by, date}`
  - `derived_from[]`: the least permissive input wins

  **Effective redistributable** is yes only when `redistributable` is `yes`, `licence_evidence` is present and `rights_review.status` is not `unreviewed`. Anything else, including a missing or misspelled field, is no.

  **Rules and controls:**
  - `committed: true` and `allowed_use` containing `repo` or `release` require effective yes.
  - Success criterion 3 is shown by two sample entries: a `redistributable: unknown, committed: false, allowed_use: [local-test]` entry that passes as non-redistributable, and a negative control of the same entry with `committed: true` that fails.

### Provenance and rights rules

- **D-14:** No-copy rule wording. This goes into `docs/provenance.md` and the review checklist.
  - Behavior, constants and tables come from cited tier 1–3 sources. Tier 4–5 sources are leads only. A tier-4/5-only value is `[UNK]` and enters code only as an explicit provisional item with an open evidence task.
  - Nothing from jzIntv, MAME or any emulator is copied, ported or paraphrased, whether by a human or an AI: no code, comments, identifiers, table layouts or file structure.
  - **GPL emulator source (jzIntv, FujiNet fork) is never opened, and no copy is kept in the workspace.** GPL documentation may be read as a tier-4 lead.
  - MAME (BSD-3) source may be read only for discrepancy hints. Each reading is logged in `docs/provenance/CONSULTATIONS.md`, and the resulting value is re-derived from a tier 1–3 source.
  - A fact shared by an emulator and a doc is cited to the non-emulator source and derived where possible (for example 14,934 = 262 × 57).
  - The process is called "documented independent derivation", not "clean-room".
  - Review checklist: a `Source:` cite on every constant or table; consultations logged; no emulator identifiers; every binary listed in a manifest; any flip of `redistributable` to yes needs explicit review; nothing under `refs/`; spot-check one or two constants per PR against the cited page.
  - The checks catch accidents, not authorship (renaming evades the denylist). This limit must be stated honestly in the doc.
- **D-15:** Reference-document cache. `refs/<SRC-id>.<ext>` is gitignored and excluded from CI caches and artifacts. Tracked `data/sources.json` records `id`, title, URL (plus Wayback if available), accessed date, sha256, size, `rights: unknown`, authority tier and local path. Quote only short phrases; paraphrase and cite. Hashes of single system images are allowed as facts; bulk ROM-hash databases from other projects are not.

### CP-1610 reference notes

- **D-16:** Source. The GI CP-1600 Microprocessor Users Manual, S16DOC-CP1600-04, May 1975 (bitsavers):
  - sha256 `ecdabe5c52b5e4429a40d1779d82315ab74bae9db00df774e605f8a882dc2b72`, 9,917,417 bytes, 116 pages.
  - It is a scanned image PDF whose OCR layer is damaged in tables (`DOD`, `lllO`).
  - A local copy (not in the repo) is at `<local-scratch>`. Executors should copy it to `refs/SRC-gi-cp1600-um-1975.pdf`, or re-download it with a browser UA and verify the sha256.
- **D-17:** Evidence rule. Every encoding, cycle count and status-bit claim is read from the **page image** (`pdftoppm`). The OCR text is used only to find pages and for prose. A claim is `[OBS]` only if the image was read. The notes state that this is the CP-1600 manual applied to the CP-1610 [INF], and that electrical specifications are "Tentative".
- **D-18:** Citation form. "§3.4.6, pdf 62". The scan has no printed page numbers. The sha256 is pinned once in the sources registry, and the notes say this once rather than inventing printed page numbers.
- **D-19:** Format and scope.
  - Markdown prose in `docs/cp1610/reference-notes.md`.
  - A hand-authored family-row table `data/cp1610/opcode-families.json` of about 90 rows: mnemonic, field pattern and mask/match, mode rules, base cycles plus modifiers, status bits, interruptible flag with cite or `manual-silent`, and a cite per row.
  - Content to cover:
    - SDBD rules: modes 4, 5 and 7 supported; modes 1–3 read one word for both bytes; modes 0 and 6 unsupported by this CPU version.
    - MVO@ with SSS = MMM ∈ {4..7} is unsupported.
    - GSWD R4/R5 is a NOP; R6/R7 is a NOP plus a PCIT pulse.
    - Reset/MSYNC/IAB sequence (pdf 12, 21, 27).
    - Interrupt sequence (pdf 25, 35).
    - Bus requests are honored before interrupts (pdf 25).
  - Page map: summary tables at pdf 53–58; addressing and stack at 46–49; formats at 50–52; state sequences at 21–25; timing diagrams at 26–37; interrupts at 75; start/stop at 74.
- **D-20:** Independence rule. The family table is a coverage **inventory only**. Phase 2 test vectors are hand-derived per opcode from the cited manual page and carry their own cite; they are never generated from the table. Phase 1 ends with a second-reader pass that diffs the table against the page images, done by a separate agent. Named-fault controls such as wrong stack direction are queued for Phase 2.
- **D-21:** Conflict status to record.
  - **C17: confirmed.** The stack grows upward, R6 post-increments on push and pre-decrements on pull (pdf 46, 62, 24). Recorded as `resolved-primary`.
  - **C19: partial.** The manual marks all shifts and EIS, DIS, TCI, CLRC and SETC non-interruptible (pdf 53, 56). It is silent on HLT, SDBD and MVO, so the wiki list is not corroborated; those stay open.
  - **New discrepancies to append (C29+):**
    - Branch cycles are printed "7/9" with no taken/not-taken label (pdf 54); resolve from the pdf 32 timing figure.
    - The interrupt-entry cycle count is not tabulated; derive it from the pdf 35 diagram.
    - The manual covers the CP-1600, not the CP-1610.
    - The reset PC is device-supplied via IAB rather than a fixed vector.
  - Appending these extends the inventory; C1–C28 stay as seeded.

### Publication and CI in this phase

- **D-22:** Publication. This is the last step of the phase and must pass the gate.
  - Scrub absolute home paths (`~`) from tracked files: `.claude/CLAUDE.md` and `.planning/research/*.md` currently contain them. Use relative sibling names (for example `../gabbaboy`).
  - The history contains about 10 such occurrences and has never been published. The executor may rewrite unpublished history with built-in `git filter-branch`, adding no dependency, or accept the history. **Rewriting is preferred**, because the owner's rule is no PII in the public repo.
  - Then verify noreply author emails and no secrets, add `LICENSE` (MIT), add a `README.md` with a non-affiliation notice and evidence-bound wording, run `gh repo create szTheory/iamverysmartvision --public --source . --push`, and set `main` as the default branch.
- **D-23:** CI. One GitHub Actions job on `ubuntu-latest` runs `sh tools/gate.sh`. It catches a distinct recurring failure class (evidence and rights drift) in about a second. Use `concurrency` with cancel-in-progress and no matrix. Create `.planning/CI-EFFICIENCY.md` on the first run (AUTONOMY §4). macOS is the local dev machine; a macOS CI leg is added only when the C build arrives.
- **D-24:** Release. No software release this phase: there is no artifact yet, and GSD milestones are separate from versions. The first release candidate follows the first buildable artifact (Phase 2+), per AUTONOMY §5.

### Claude's Discretion

- Exact JSON key spelling within the schemas above, how checker scripts are split up (keep them small and generic), and the wording of the docs.
- Whether the 1024-pattern coverage check lives in `cmake -P` or is deferred to a Phase 2 C test (see the speed bound in D-12).
- Whether to rewrite pre-publication history (preferred) or accept it (D-22).
</decisions>

<canonical_refs>
## Canonical References

**Downstream agents MUST read these before planning or implementing.**

### Project policy
- `AGENTS.md`: owner durable policy, including evidence labelling, rights and the autonomy pointer.
- `.planning/AUTONOMY.md`: delegation, hard stops, fan-out discuss, shift-left verification, the CI efficiency review, release steps, and the hygiene check (§6).
- `.planning/PROJECT.md`: Core Value, constraints and Decision Register. Record D-01..D-04 there as delegated decisions.
- `.planning/REQUIREMENTS.md`: FND-01..FND-07, the evidence-tier list (line ~20), Out of Scope and Decision Gates D1–D13.
- `.planning/ROADMAP.md` §"Phase 1": goal, research flags and five success criteria.

### Research inputs (dated; recommendations, not decisions)
- `.planning/research/SUMMARY.md` §"Source Conflicts": C1–C28, the seed for `data/discrepancies.json`.
- `.planning/research/STACK.md` Part 1 (language), Part 2 (assembler licence analysis), Part 3 (authority tiers, source-verification table, claim cross-check).
- `.planning/research/ARCHITECTURE.md` (Option B scheduler, time, API) and `.planning/research/PITFALLS.md` (PIT-1..5, 15, 16, 18, 19).
- `.planning/bootstrap/EMULATOR-DNA.md`: the bootstrap source list. It has known defects: the owner-manual link returns 404, and the iFixit link is an HTML viewer.

### Primary sources (do not commit; register in `data/sources.json`)
- GI CP-1600 Microprocessor Users Manual, May 1975: https://www.bitsavers.org/components/gi/CP1600/CP-1600_Microprocessor_Users_Manual_May75.pdf (sha256 in D-16; page map in D-19).
- GI 1980 Microelectronics Data Catalog: https://www.bitsavers.org/components/gi/_dataBooks/1980_GI_Microelectronics_Data_Catalog.pdf. The 8900 system is at 5-9/5-10 and the AY-3-8910/8912 at 7-88; the text contains no "8914".
- GI 1982 Microelectronics Data Catalog: https://www.bitsavers.org/components/gi/_dataBooks/1982_GI_Microelectronics_Data_Catalog.pdf
- Blue Sky Rangers 2609 overview (tier 2): https://history.blueskyrangers.com/hardware/2609.html

### Sibling method precedents (read-only; method only, no gates or licences transferred)
- `../gabbaboy/cmake/VerifyFixtureManifest.cmake`, `VerifyEvidenceLedger.cmake`, `ExpectedTests.cmake`, `CheckCoreNoHostClock.cmake`, `fixtures/*/manifest.json`, `THIRD_PARTY_NOTICES.md`, `.planning/context/LESSONS.md` (GB-CI-004, D-037).
- `../nesturbator/PROVENANCE.md`, `ASSET_POLICY.md`, `scripts/hygiene.sh`.
- `../schwaystation/CMakeLists.txt` (warning list), `.planning/preparation/ENGINEERING.md`.
- `../libfuturo/include/libfuturo/libfuturo.h` (`lf_time_unit`: add a distinct Intellivision master-tick unit later; do not reuse the NES one).
</canonical_refs>

<code_context>
## Existing Code Insights

### Reusable Assets
- None in this repository yet. It contains planning docs only.
- Same-owner sibling patterns to adapt, not copy blindly:
  - gabbaboy's CMake checker idioms (`string(JSON)`, failure tokens, empty-inventory guards, relocated installed consumer)
  - nesturbator's hygiene script and asset policy

### Established Patterns
- Exact checked-in inventories, vacuous-pass guards, failure tokens and negative controls (gabbaboy).
- "Facts may enter, another emulator's expression may not" (nesturbator PROVENANCE).
- Siblings are public, MIT-licensed, C17 + CMake 3.25.

### Integration Points
- `tools/gate.sh` is the seam that Phase 2 extends with the CMake configure, build and CTest steps; CI calls exactly this script.
- `data/profiles/*.json` becomes the single source for a generated C header or constants in Phase 2. The generation is one-way and checked for freshness. The core never parses JSON and never touches the filesystem.
- `data/cp1610/opcode-families.json` is the Phase 2 coverage inventory, never the vector oracle.
</code_context>

<specifics>
## Specific Ideas

- Owner taste: "another copy-paste is better than another dependency". Phase 1 adds no new tools beyond CMake, POSIX sh, git and poppler (poppler is a local dev aid for reading page images, not part of the gate).
- Aim for zero human UAT. Every success criterion except "is the content true" is machine-checked. The honest limits that stay review-only are:
  - whether tier labels are assigned honestly
  - whether page cites match the images (mitigated by the second-reader agent pass)
  - whether the no-copy rule was actually followed
  - tripwire paraphrases, such as "works with", that the claim-language check does not catch
</specifics>

<deferred>
## Deferred Ideas

- A fill-sensitivity matrix (running fixtures under non-zero power-on fills): Phase 4+, once fixtures exist.
- Board-pinned owner-unit profile `…-p2-owner-unit` with measured fields: needs D10 (hard stop).
- STIC1A, PAL (AY-3-8900 at 4.000 MHz), Tandyvision/GTE/Sears aliases: v2, after dump comparison.
- MSan, valgrind and a scheduled fuzz job: Linux-only optional jobs, added only when a failure class justifies them (D12, Phase 4).
- LibFuturo Intellivision master-tick time unit: D13, not before a real consumer.
- A generated C header from the profile JSON: Phase 2.
</deferred>

---

*Phase: 01-evidence-profile-and-rights-foundations*
*Context gathered: 2026-11-09*

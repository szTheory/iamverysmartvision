# iamverysmartvision

## What This Is

iamverysmartvision (owner-selected working name; not trademark-cleared) is a new
emulator for the Mattel Intellivision family. It aims to be accurate,
reproducible and useful: a single canonical, deterministic, headless machine
core whose CPU/device behavior, controller input, guest progress, raw video and
raw audio can each be tested independently, and which well-designed hosts can
integrate. It is for the owner and for future host/frontend integrators; support
claims are always tied to an exactly named hardware/content profile and the
evidence that backs it.

## Core Value

A deterministic, observable machine core whose every compatibility claim is
bound to an exactly named profile and independent evidence — never an
unqualified "it runs Intellivision games" label.

## Current Milestone: v0.3.0 Owner-Supplied Content and a Driveable Host

**Goal:** A person can load their own EXEC, GROM and a flat cartridge image into
this core and drive it in real time through a build-only libretro-shaped host,
while the project ships, describes and commits none of that content — and every
capability is proved headlessly with original fixtures, with the one question
only the owner can answer recorded as a handoff row.

**Target features:**
- A question-shaped rights ledger and a mechanical claim ceiling, plus the ten
  must-fail mechanisms that have a guard but no registry row (`RTS-01`, `RTS-02`,
  `EV-06`, `EV-07`).
- A build-only tool that wraps a flat image at an address the operator states
  into the project's cartridge container, with typed refusals each beside its
  boundary positive, and a cartridge role in the owner-local route (`TOOL-07`,
  `SCN-03`).
- A build-only host surface outside the core and outside the installed package,
  proved by a fake frontend that reproduces the runner's nine digest lines and
  guarded three ways (`TOOL-08`).
- One documented owner route from the source archive, the handoff rows, and a
  release posture that states no host artifact and no content ships (`PKG-06`).

**Scope record:** `.planning/MILESTONE-CONTEXT.md` (frozen upstream artifact);
the five scoping outputs are `.planning/research/v0.3.0-scoping-*.md`.
**Guardrails:** `.planning/ops/GUARDRAILS-v0.3.md`.
**Milestone label v0.3.0; software version 0.3.0 if released** (a minor bump under
D-30: a new tool, a new build target and a new operator route; no API break).
**Droppable:** Phase 15, pre-authorized, degrading to a content-only v0.3.0.

## Requirements

### Validated

Each capability below is complete on documented terms (requirement text met by
gate evidence, recorded in the phase VERIFICATION record and the REQUIREMENTS
close-out notes), its claims are provisional (one claim, CL-0002, is
established) and nothing was measured on hardware: D10 and D11 are not
performed. REL-01 completes at stage 2 (the records pull request after the merge,
Phase 8 D-28), which is recorded: the push run on the artifact commit, the
GitHub reads and the release decision. The Active list below is kept as written
at kickoff.

- Evidence, profile and rights foundations (FND-01..07, Phase 1): complete on
  documented terms, claims provisional, nothing measured.
- CP-1610 CPU (CPU-01..05, Phase 2): complete on documented terms, claims
  provisional, nothing measured.
- Memory map, content loading and refusal (MEM-01..05, Phase 3): complete on
  documented terms, claims provisional, nothing measured.
- Core contract and host independence (API-01..06, Phases 2 to 5): complete on
  documented terms, claims provisional, nothing measured.
- Headless runner and verification (RUN-01..06, Phases 2 and 4): complete on
  documented terms, claims provisional, nothing measured.
- Packaging, local gate and CI (PKG-01..04, Phases 2 and 4): complete on
  documented terms, claims provisional, nothing measured.
- Hand controllers (INP-01..03, Phase 5): complete on documented terms, claims
  provisional, nothing measured.
- PSG sound (AUD-01, AUD-02, Phase 5): complete on documented terms, claims
  provisional, nothing measured.
- STIC video (VID-01..06, Phases 6 and 8): complete on documented terms, claims
  provisional, nothing measured; the unresolved STIC behaviours stay on the
  `unknown-default-chosen` list of the discrepancy log.
- Visible original scenario (SCN-01, SCN-02, Phase 7): complete on documented
  terms, claims provisional, nothing measured; SCN-02 is the documented
  owner-local route only, with no real EXEC or GROM run (D11 not performed).
- Snapshots, restore and performance baselines (STA-01, STA-02, PRF-01,
  Phase 8): complete on documented terms, claims provisional, nothing
  measured; the committed benchmark baseline is informational because it was
  taken on a host that was not idle.
- Milestone close and release (REL-01, Phase 8): complete on documented terms.
  Main green at the artifact commit, 0 open pull requests and 0 open issues, no
  ruleset applied, and `v0.1.0` published as a pre-release of that exact commit
  after checks P1 to P8 held. Claims provisional, nothing measured.

**v0.2.0 (shipped 2026-11-25, release `v0.2.0` on the artifact commit `8f017e7`).**
Eight requirements Complete on documented terms and two Deferred, recorded in
`.planning/milestones/v0.2.0-REQUIREMENTS.md`; 20 plans over Phases 9 to 11;
Phase 12 dropped at a written entry gate.

- Registered evidence and gate cost (`EV-01`..`EV-04`, Phase 9): complete. Lever
  (3) and trims (a), (c), (d) landed cost-only, a hosted row followed them, and
  the registry grew to 337 rows naming 110 gates without a new fault or test id.
- Handoff ledger (`EV-05`, Phase 10): complete. Five rows, every one
  `evidence: none`, and a claim may not cite one.
- Reachable runs and presentation policy (`PKG-05`, `TOOL-04` narrowed, Phase
  10): complete. `docs/running.md` is measured, the release rehearsal takes the
  source archive to 124 of 124 golden lines, and `docs/presentation.md` freezes
  six clauses.
- Input record and replay (`TOOL-01`, Phase 11): complete. IVSL v1, fifteen new
  ids, two named faults, and the vacuity closed three ways.
- Bank modes and `.bin`/`.cfg` import (`FMT-01`, `FMT-04`, Phase 12): **Deferred**
  at the entry gate — the only known `.cfg` description is a GPL implementation
  this project's research rules exclude. v0.3.0 narrows the question with
  `TOOL-07` rather than answering it.
- Every claim stays `provisional` except `CL-0002` (source-expectation, tier 1
  primary doc); D10 and D11 remain unperformed.

### Active

All Active items are hypotheses until delivered and verified. Detailed,
ID-tagged requirements live in `.planning/REQUIREMENTS.md`; the v0.1.0 and
v0.2.0 sets are archived under `.planning/milestones/`.

- [ ] A rights ledger records one row per third-party-content question the
      milestone touches, with a disposition, a resolvable source when cleared,
      and a naming rule that a refused spelling cannot escape (`RTS-01`,
      `RTS-02`).
- [ ] The claims step refuses an `established` claim that is not a
      `source-expectation` claim resting on a `tier1-primary-doc` dependency
      (`EV-06`, D-32, D-50).
- [ ] The ten must-fail mechanisms that have a guard and a re-runnable stand-in
      but no registry row are registered as `mutant` rows, with zero new faults
      (`EV-07`).
- [ ] A person can wrap a flat cartridge image of their own, at an address they
      state, into the project's cartridge container with one build-only command,
      and every malformed input is refused by a typed token beside a positive
      case (`TOOL-07`).
- [ ] The owner-local route accepts a cartridge image, with every derived-output
      refusal extended and the pass path exercised on project-made files
      (`SCN-03`).
- [ ] The core can be driven in real time by a host that loads it as a shared
      object outside the installed package, proved by an automated frontend that
      reproduces the runner's nine digest lines (`TOOL-08`).
- [ ] A tracked shared object is refused by name, the installed package stays
      eleven files, and the release-decision line states that no host artifact
      and no content ships (`PKG-06`).

### Out of Scope (for now — not permanent ceilings)

- Redistributing EXEC, GROM, MTE-201 or any commercial ROM/manual/overlay —
  rights-bearing; no documented permission.
- Copying code from MAME, jzIntv or other emulators without deliberate license
  review — jzIntv is GPL-2.0-or-later; MAME licensing is per-file and must be
  checked; their behavior is also not an independent oracle.
- Promising IntelliVoice, ECS, keyboard component, Intellivision II or clone
  (e.g. Sears Super Video Arcade) support in the first slice — existence in
  another emulator does not imply required support; each is a later roadmap
  decision.
- Physical controller discovery, remapping, accessibility, windowing and audio
  device output inside the core — host-layer responsibilities. **v0.3.0 note:** a
  *build-only* host surface outside `core/` and outside the installed package is
  in scope (`TOOL-08`, D-48); a shipped or installed host is not (D-49), and a
  window is never the evidence.
- Vendoring the libretro header, or claiming libretro compatibility (D-48). The
  shim states only the subset it implements and is *libretro-shaped*.
- Parsing or describing the `.cfg` container, or implementing bank switching
  (D-16, D-47). A flat image at an operator-stated address is accepted; `.cfg`
  is refused by name.
- Committing, golden-ing or CI-creating a wrapped image or any third-party
  content byte (D-47). The wrap output is derived material.
- Unqualified compatibility labels or percentage claims — contradicts Core
  Value.
- Trademark clearance, physical hardware testing, owner-local EXEC/GROM runs
  and third-party content without explicit owner action — these are owner
  gates. The licence (MIT), repository visibility and release publication are
  delegated decisions (D9, `.planning/AUTONOMY.md`), no longer owner gates.

## Context

- **Project state at kickoff (verified 2026-11-09):** directory contained only
  `AGENTS.md`, `.planning/config.json` and `.planning/bootstrap/`
  (START-HERE, PROJECT-SEED, EMULATOR-DNA). No code, no git repository (git
  initialized at kickoff), no ROMs, system images, saves or binaries. GSD
  installed: `@opengsd/gsd-core` 1.20.1 (identity verified).
- **Bootstrap research is dated input** (EMULATOR-DNA.md, 2026-11-09). Its
  Intellivision source table (MAME driver, 2609 service manual scan, Blue Sky
  Rangers 2609 overview, GI 1977 data catalog, owner-manual scan,
  intellivision.us hand-controller notes, jzIntv/FujiNet fork) has mixed
  authority. Primary manuals and component data sheets must be checked before
  freezing timing/register requirements. Claims from another emulator do not
  establish Intellivision behavior.
- **Sibling emulator projects and LibFuturo** supply method-level lessons
  (exact test inventories, named-fault controls, fixture provenance, evidence
  tiers, installed-consumer proof, CI critical-path measurement). Their
  hardware assumptions, APIs, performance budgets, gates and licenses do not
  transfer.
- **LibFuturo** is a *candidate* integration only. Its experimental C API
  (instance lifecycle, single content span + config id, 32-bit logical button
  mask sampled at advance start, bounded execution, caller-owned A/V buffers,
  profile-defined persistence) has no Intellivision profile. Fit for keypad +
  disc combinations, content + system images, variable output and
  memory-backed saves is unconfirmed.
- **Bounded search found no permissively redistributable Intellivision
  conformance ROM suite** (evidence limit, not proof of absence). MTE-201 may
  be used only to discover diagnostic categories.
- **Evidence tiers are distinct:** source expectation, public vector, original
  diagnostic, independent implementation comparison, measured hardware,
  deterministic repeatability, playable guest progress, installed-host
  behavior. One tier does not imply the next; repeated hashes prove
  repeatability, not accuracy; shared-core agreement is not an independent
  oracle.

## Constraints

- **Determinism / host independence:** machine behavior must not depend on
  host clocks, filesystems, physical controller discovery, windows, A/V
  presentation or persistence policy — owner North Star.
- **Rights:** binaries, diagnostic ROMs, EXEC/GROM images, manuals and overlays
  are rights-bearing; nothing committed or redistributed without documented
  permission. A hash establishes identity, not rights — AGENTS.md.
- **Dependencies:** small dependency surface; add a dependency only for a
  concrete capability whose maintenance cost is justified — AGENTS.md.
- **Performance:** high runtime performance is a goal, but only after
  representative workloads and correctness baselines exist — North Star.
- **CI:** one local command aligned with the fast PR gate; a CI check is added
  only for a distinct recurring failure class at useful cost; measure
  queue/setup/build/test before changing caches/concurrency/matrix — AGENTS.md.
- **Milestones vs versions:** GSD milestones and software versions are separate.
  Release only the exact tested artifact when rights, package readiness,
  qualification and authority permit — AGENTS.md.
- **Autonomy boundary:** routine reversible work proceeds autonomously; stop for
  material owner choices, unresolved rights, external publication, physical
  qualification or another explicit safety boundary — AGENTS.md.

## Decision Register

Kept deliberately separate so that a recommendation is never mistaken for an
owner decision.

### Owner decisions (accepted)

| Decision | Source |
|----------|--------|
| Domain is the Mattel Intellivision family | START-HERE / PROJECT-SEED |
| Working name `iamverysmartvision` (not trademark clearance) | START-HERE |
| Goal is high-quality, accurate, useful emulation; first delivery is sequencing, not a ceiling | AGENTS.md |
| Routine reversible GSD work runs autonomously (config `mode: yolo`, `auto_advance: true`) | AGENTS.md / config |
| Bootstrap stops at project initialization — no phase planning or implementation | START-HERE / kickoff prompt |
| Third-party ROMs/manuals/overlays treated as rights-bearing | AGENTS.md |
| Release, license, visibility, third-party content, external qualification and hardware testing are explicit owner gates | PROJECT-SEED (superseded in part by the next row: the licence is MIT, visibility is public and release publication follows the delegated flow, D9; third-party content, external qualification, physical measurement and owner-local EXEC runs remain owner gates) |
| Autonomous operation: decision gates are delegated to researched recommendations (recorded below as "Delegated decision"); public GitHub repo via `gh`, frequent releases, green `main` and PR auto-triage are authorized. Third-party content, PII/credentials, physical measurement, owner-local EXEC runs, destructive history changes and the trademark name stay hard stops | Owner, 2026-11-09 — `.planning/AUTONOMY.md` |
| Discuss = per-decision subagent fan-out (multi-lens, adversarial, primary sources) → one coherent auto-selected recommendation; "another copy-paste is better than another dependency" | Owner, 2026-11-09 — `.planning/AUTONOMY.md` §2 |
| Verification shifts left toward zero human UAT; CI only for checks with recurring value; review CI efficiency after every run | Owner, 2026-11-09 — `.planning/AUTONOMY.md` §3–4 |

### Delegated decisions (owner authorization 2026-11-09, `.planning/AUTONOMY.md`)

These come from a researched fan-out recommendation that was auto-adopted. Each is reversible through the same protocol, and the reversal evidence is listed. Details are in `.planning/phases/01-evidence-profile-and-rights-foundations/01-CONTEXT.md`.

| Gate | Decision | Would reverse it |
|------|----------|------------------|
| D1 | C17 + CMake 3.25+/Presets/Ninja/CTest, in-repo test harness, no runtime deps (~75%) | Recurring UB despite integer rules; stable Rust sanitizers; Rust-first consumer |
| D2 | Canonical machine time = master tick (one crystal period), `uint64_t` from reset; CPU cycle = profile ratio (NTSC 4); core never reads Hz. Details: `docs/time-base.md`. | Evidence of a device clock not an integer divisor of the crystal |
| D3 | First profile `intv-2609-ntsc-family-p1`: family-level 2609 NTSC, AY-3-8900-1 STIC, AY-3-8914-class PSG (tier 4), zero power-on fill, single-slot ROM-only cartridge, per-field tags | Owner unit measurement (D10) → new board-pinned profile id |
| D9 | MIT licence; public GitHub repo `szTheory/iamverysmartvision` after hygiene check; keep working name; never "Intellivision" in product/package name; README non-affiliation notice | Owner instruction |

| D5 | System images are user-supplied only; project boot stub through the EXEC slot; no built-in/HLE image (Phase 3 D-24) | Permission to redistribute an image, or owner wants a labelled HLE |
| D-cart | Core cartridge format is the project container IVSC v1 (`docs/formats/ivsc-v1.md`); `.rom`/`.bin+.cfg` importers live outside core and later (Phase 3 D-29) | Owner wants third-party cartridges in this milestone, or a permissive full `.rom` spec |

| D4 | Fixture assembler policy: project mini-assembler `ivs_asm` (C, `tools/asm/asm.c`, never installed or shipped, no dependency) generates the committed listing from a project-syntax `.asm` source (the source of truth, committed with the generated listing, both named in the fixture manifest with the recipe); the listing stays checked by the `listing` step and the pack suite (`ivs_pack` unchanged); the four legacy listings stay hand-assembled and are reproduced byte for byte by `.asm` twins kept as test data; as1600 rejected (GPL), libasm an optional non-gating cross-check (Phase 4 D-25, re-evaluated in Phase 7 D-16, D-19; Delegated decision, owner authorization 2026-11-09) | Reversal evidence: two encoding escapes found only by a run, the tool passing 1,200 lines of source, a call for macros, or a program so small that the tool costs more than it saves; then stop and hand-assemble (the generated listings remain valid, hand-checkable artifacts) |
| D6 | Controller core input is logical switch state per PSG port; the electrical byte is derived in-core from profile data (Phase 4 D-12) | Evidence the output is not a function of switch state plus port drive, or a need for raw port injection |
| D7 | Reference PCM is outside the core and outside every installed artifact: a pure integer converter in `tools/pcm/` is linked only by the runner's operator-side `--audio-wav` and its own tests; raw audio is per-channel level samples every 16 master ticks plus a cycle-stamped write log (Phase 5 D-15, D-21) | An adapter that needs bit-identical PCM across hosts (promote to an installed `ivs::refpcm` target, still outside the core) |
| D8 | Framebuffer geometry: the raw framebuffer of `intv-2609-ntsc-family-p1` is 159 x 96 display rows (19 x 8 + `video.last_card_column_px` = 7 columns by 12 x 8 rows; the tier 1 count from the GI 1980 and 1982 catalog pages, 159 of a 167 x 105 grid), one byte per position holding the 4-bit STIC color number 0..15 or `0xFF` for a position not displayed; a slot of a 32-byte header plus the plane (15,296 bytes) per frame, completed at the frame event (`E_k`), delivered into a caller-owned buffer (NULL plus 0 means not requested) with the run flag `IVS_RUN_F_STOP_ON_FRAME` and a per-frame digest chain (domain 16); palette, scaling, aspect and border painting stay outside the core (Phase 6 D-12..D-17; Delegated decision, owner authorization 2026-11-09, decided 2026-11-12) | One-field reversal: `video.last_card_column_px` = 8 gives 160 columns, then re-bless the goldens as one classified diff; or a new format id if pixels outside the window are wanted (hardware or primary-page evidence, D10-VID-01, C10 stays `partial`, C101, C105) |
| D12 | CI shape: one ubuntu-24.04 job, no matrix, no cache, no artifacts, no macOS leg, job summary from the gate, run record file (Phase 4 D-41..D-45) | An Apple-only failure reaching main, a shared library, or a median hosted gate above budget after levers |
| D10 | Physical hardware measurement: OPEN, a hard stop and not performed; the 47 items of `data/expected/d10-measurements.ids` (42 before the five collision, priority, delay, extension and half-height items of Phase 8) are handed off and no claim uses the `measured-hardware` tier | Owner-approved measurement evidence (D10), ingested per `docs/input/d10-procedure.md` and `docs/stic/d10-procedure.md` |
| D11 | Owner-local run of a real EXEC and GROM image through the runner (the SCN-02 route, `docs/owner-images.md`): OPEN, a hard stop and not performed; SCN-02 is complete on documented-route terms only (the route page, a counted skipped template scenario, the owner-run helper with tested refusals, the refusal of derived audio and log output for user content, manifests labelled not redistributable); no EXEC or GROM digest is recorded, and no result, claim or tier exists | The owner runs the helper with their own images (never committed, never in CI) and records the outcome as D11 items; documented redistribution permission would be needed before any image or digest enters the repository |
| OQ8 | Snapshot persistence policy (open question 8; Phase 8 D-19, plan 08-17; Delegated decision, owner authorization 2026-11-09): the core returns bytes and nothing else, so it writes no file and chooses no name, slot, location, compression, encryption or autosave, and no machine behaviour depends on a file; where a snapshot goes is a host's policy, with no default save location; the runner has no snapshot file option in this phase (`--snapshot-out` and `--snapshot-in` are deferred and, when added, are runner-only, take an explicit path and are refused for a `content user` scenario); the container IVSN v1 is exact-build and not archival (one pair of format version and schema version per build, any other pair refused as unsupported, no migration before 1.0); `.ivsn` is rights-bearing by name | Reversal evidence: a consumer that needs archival snapshots (a migration story and a versioning policy), a default location, or battery-backed cartridge RAM; reversible by adding sections and raising the section count and the schema version together |
| OQ11 | Performance targets (open question 11; Phase 8 PRF-01; Delegated decision, owner authorization 2026-11-09): PRF-01 sets none. The benchmark tool asserts a workload's digest before it reports any figure, a figure is repeatability-only information about one build on one host and never a claim, no check compares a figure with a threshold, and Phase 8 optimized nothing; the committed baseline record is informational | Reversal evidence: a consumer states a frame-rate need (then a target, a workload that represents it and a baseline taken on an idle host) |
| REL-01 / D-29 | First release: a draft pre-release, published by the orchestrator only after read-only checks. The tag-triggered release workflow creates `v0.1.0` with `--draft --prerelease --verify-tag --latest=false`, and the orchestrator publishes the draft only after the checks P1 to P8 (green `main` at the artifact commit, hygiene, rights, version agreement, notes from the claim ledger, the history disposition, the release machinery, a read of the draft with `gh release view --json`); the tag goes on the Phase 8 merge commit (the artifact commit) and never on a records commit; a failing check is recorded as `ready, not published: blocked by P<n>`; deleting a published release is a hard stop (`.planning/AUTONOMY.md` section 1), and the draft is the cheap reversible step before it, an amendment of the draft-first detail of the section 5 design, which is untouched (Phase 8 D-29; plans 08-01, 08-02 and 08-21; Delegated decision, owner authorization 2026-11-09). **Status, 2026-11-24:** at the Phase 8 close the release had not started (the owner paused the flow, relayed by the orchestrator); the owner then resumed it, the orchestrator drove the close-out to the end of the milestone, and `v0.1.0` was published as a pre-release on the artifact commit `3fe5373` after every check P1 to P8 held. This row records the decision and is not rewritten by that outcome | Reversal evidence: the draft step finds no defect in two releases and costs more than it saves (drop the draft flag, one line of `tools/release.sh`); or a published release is ever wrong and the owner chooses a delete-and-recreate policy |
| REL-01 / D-30 | Three separate version numbers, never conflated: the GSD milestone label (planning only, never a git tag), the software version 0.1.0 (the three `IVS_API_VERSION_*` macros of `core/include/ivs/ivs.h`, equal to the CMake `ivs_VERSION` and to the installed package version) and the format versions (IVSC v1, IVSN v1 with schema version 1, the video slot header, the state digest composite version 3). The tag is strictly `vMAJOR.MINOR.PATCH` and must equal the macros: the release script and the version policy check refuse a skew, and the release trigger is the glob `v[0-9]*.[0-9]*.[0-9]*`. For 0.x a patch means no API or format change and a minor means anything else, consistent with `SameMinorVersion`; the first release is 0.1.0 because nothing was ever tagged, and the snapshot calls are additive inside 0.1 (Phase 8 D-30, D-32; plans 08-01 and 08-02; Delegated decision, owner authorization 2026-11-09) | Reversal evidence: a consumer needs a pre-release suffix or build metadata in the version, a breaking format change before 1.0 that the minor rule handles badly, or a tag is made in another form; then the policy and the check change in the same commit |
| REL-01 / D-31, D-42 | The milestone tool must not create a tag. `.planning/config.json` carries `git.create_tag: true`, which would make `gsd-complete-milestone` create, and offer to push, a milestone-style tag such as `v1.0`: that is not a 0.x software version and would match a wide release trigger. The release trigger is the strict glob and the release checks refuse a tag that is not the header version; changing that one key to false is an orchestrator-only change in a separate commit with no other file in it, and executors never stage `.planning/config.json` or `.planning/state.json` (Phase 8 D-31, D-42; `.planning/ops/RELEASE-OPS.md` item a; Delegated decision, owner authorization 2026-11-09). The key reads `true` in the working-tree copy at the Phase 8 close, so the change is still to be made. **Correction, plan 09-09 (2026-11-25), measured:** the key now reads `"create_tag": false` at `.planning/config.json:15`, set by `e0c88b4` (the records commit that closed stage 2 of REL-01, 2026-11-24) before the milestone tool was used, so the D-31 item was a **prose** fix only; the historical sentence above is kept as written (lesson 21) and the reversal evidence is the key's current value | Reversal evidence: the milestone tool gains a no-tag flag or a configurable tag name that cannot match the release glob, or the owner wants a milestone tag; then the key can stay as it is |
| REL-01 / D-33 | History disposition: six old planning commits on public `main` trip `sh tools/hygiene.sh --history` on the shape of a scratch-directory path (five commits by the token `hygiene-history-home-path` and one commit message by `hygiene-message-home-path`; the matched text carries no user, host or session content). They are **accepted without a rewrite and not cleared**: pinned by (token, short sha) pair in `data/expected/hygiene-history-pinned.ids`, a finding that is not on the list fails the release check, the baseline file `tools/check/hygiene-baseline.txt` is not moved, no rule is widened, and the readiness record prints each finding with its disposition and never says that the history check passed (Phase 8 D-33; plan 08-02 and the full scan of plan 08-21, six findings and none new; Delegated decision, owner authorization 2026-11-09). **The owner may veto this disposition and choose a rewrite**, which is an owner-only hard stop (`.planning/AUTONOMY.md` section 1) and would not purge the pull request refs that GitHub keeps | Reversal evidence: an owner veto; a finding whose matched text carries identifying content (a user name, a host, a session id); or a request to purge the objects (a GitHub Support action) |
| D14 | The runner is **not** installed in 0.2.x; the person route is the documented build from the source archive (`.planning/MILESTONE-CONTEXT.md` §3; Delegated decision, owner authorization 2026-11-09, decided 2026-11-24). Installing it is structurally blocked, not merely expensive: an installed `ivs-run` has nothing to run, because every cartridge is a build product of `ivs_pack`/`ivs_asm` and both are never shipped, and any release member with an extension in `tools/check/binary-ext.txt` is refused by `release-assets` whatever the list says | Reversal evidence: a named consumer who wants a downloadable binary **and** an images story that survives the release-asset check — concretely a runner-synthesised symbolic content name (like `zero-grom`) for the boot stub plus a project-authored cartridge the runner can make; that is the correct trigger to reopen, and it is not this milestone's |
| D15 | The replay log is an **input artifact**: ASCII, input events and ticks only, no field able to hold a content digest or path, a `content project\|user` line, not in `binary-ext.txt`, not gitignored (`.planning/MILESTONE-CONTEXT.md` §3; Delegated decision, decided 2026-11-24). Under D11 it is sufficient: an input log holds nothing the images produced, so it is not derived material, and a replay of an owner-image run still needs the images locally and produces no result, claim or tier in the repository | Reversal evidence: a consumer that needs content identity inside the format (then reuse the existing `expect-sha256` directive, not a new field), or a defect found only at the process boundary (then promote two or three in-process cases to the command line) |
| D16 | The `.bin`/`.cfg` importer lives **outside the core**; its refusals are closed append-only tool tokens pinned in `data/expected/cfg-refusals.ids`, its output is operator-side and refused inside the work tree, and it is rights-gated **per mode** (`.planning/MILESTONE-CONTEXT.md` §3; Delegated decision, decided 2026-11-24). No new `IVS_E_*` reason, no core change, no `api-effects` retarget; a mode the review did not clear appears as `cfg-bank-mode-unmapped`, not as a default | Reversal evidence: a consumer that needs import inside the installed core (then D-cart is revisited first), or a primary permissively described source that makes the description provably clean (then the importer can be un-gated per mode) |
| EV-scope | The milestone adds **zero new gate steps**: `ls tools/check/steps/*.sh \| wc -l` stays 38 and every new capability lands as ids inside an existing suite (`.planning/MILESTONE-CONTEXT.md` §4; Delegated decision, decided 2026-11-24). The stop rule is a policy stop, and a step is the expensive unit while an id is the cheap one | Reversal evidence: new ids exceed about 15 percent of an existing suite, a suite's measured per-id cost rises above the Phase 9 unit-cost table, or a defect is found only at the seam between two capabilities' cases; then a step is justified — after a hosted row, never before |
| D-46 | **The rights ledger is a second, question-shaped ledger** (v0.3.0, Delegated decision 2026-11-26): `data/rights/<slug>.json`, one file per item, `disposition` in {`cleared`, `refused`, `owner-local-only`}, a `SRC-` id or null, a "what was taken" cell and the spellings a refused item must never appear as. Enforced as ids inside `files`/`manifests`; zero new steps. The naming limb is token+scope with a `(token, path)` pinned baseline, never a global grep, because `cfg` and `ivsc` already sit in `binary-ext.txt` and `.gitignore` as refusal surfaces and removing them would delete a refusal | Reversal evidence: a question whose answer is a byte set becomes a fixture manifest instead |
| D-47 | **The flat wrap states bytes, never a format** (v0.3.0, Delegated decision 2026-11-26): `tools/flatwrap <flat> <base> <out> --word-order hi-first\|lo-first` takes a byte length, a word order and an address the operator states; nothing is auto-detected and nothing is described from another implementation. `.cfg` is refused by name. The output is derived material of rights-bearing content: never committed (`*.ivsc` is already ignored and refused), never a golden, fixture, CI-created or claim artifact, and no digest of it but on stdout | Reversal evidence: an owner shows a flat image neither word order expresses |
| D-48 | **The host is build-only with a project-authored ABI subset** (v0.3.0, Delegated decision 2026-11-26): no libretro header is vendored — the shim needs about six `retro_*` entry points, so `host/` states only the subset it implements and says *libretro-shaped*, never compatible. If an exact header is ever required, its licence text is read first and recorded as `licence_evidence {url, text_sha256, accessed}`, and the manifest's effective-`yes` rule governs it | Reversal evidence: a real frontend loads the shim, or an exact header is needed |
| D-49 | **The package stays static and eleven files** (v0.3.0, Delegated decision 2026-11-26): the build-only shared target is the only place `IVS_API` is defined; installed headers keep it empty; `binary-ext.txt` gains `so`, `dylib`, `dll`, `bundle` so a tracked shared library is refused by name; `pkg-install-files.ids` stays 11 with a selftest that the shared object is absent. The release-decision line reads: the release ships the static package and the source archive; no shared object, no host shim, no content | Reversal evidence: a decision to ship a shared core, which moves `pkg-install-files.ids` and `docs/packaging.md` in the same commit |
| D-50 | **The claim ceiling is enforced mechanically** (v0.3.0, Delegated decision 2026-11-26): an `established` claim must be `source-expectation` resting on a `tier1-primary-doc` dependency; no other tier may stand `established`. `CL-0002` is the only claim this permits, and it is checked rather than assumed. This is the mechanical half of D-32 and it closes audit gap G-09 | Reversal evidence: D10 or D11 performed, after which behavioural claims may legitimately rise above `provisional` |
| D-32 | **The provisional ceiling is a behavioural-claim ceiling, and it is stated as one** (v0.3.0 housekeeping 2026-11-26, closing audit gap G-09's ambiguity; Delegated decision, owner authorization 2026-11-09). GUARDRAILS-v0.2 §2.8 said "a claim never rises above `provisional` while D10 and D11 are unperformed", and `tools/check/kinds/claim.cmake` does not enforce it, so every phase quoted `claims=28 provisional=27` as if it discharged the rule. The rule was written one class too wide: D10 (hardware measurement) and D11 (owner-local EXEC/GROM runs) gate claims about **what the machine does**. `CL-0002` is a `source-expectation` claim that a printed figure names the STIC part and crystal, its three fields carry `tier1-primary-doc` tags, and no run, hardware measurement or owner image is involved — so its `established` standing is correct and is **not** an exemption from the ceiling. The ceiling applies unchanged to every `original-diagnostic`, `deterministic-repeatability`, `installed-host-behavior`, `playable-guest-progress`, `public-vector` and `independent-implementation-comparison` claim, all of which rest on runs. The mechanical check that makes this distinction enforceable — an `established` claim must be `source-expectation` resting on a `tier1-primary-doc` dependency, and no other tier may stand `established` — is the first row of the next gate-hardening pass, priced as gate content, not smuggled in as a housekeeping edit | Reversal evidence: an owner-approved hardware measurement (D10) or a performed D11 run, after which behavioural claims may legitimately rise above `provisional`; or a `source-expectation` claim shown to rest on a page the project never read |

D4, D6 and D12 were decided by the fan-out protocol on 2026-11-11; D7 on 2026-11-12 (Phase 5), D8 on 2026-11-12 (Phase 6) (Delegated decision, owner authorization 2026-11-09).

Phase 5 notes on the register above (plan 05-12, 2026-11-12; Delegated decision, owner authorization 2026-11-09; nothing here is a new owner decision and each row is reversible by the evidence in its last column):

- **D3 (note):** the PSG part stays `AY-3-8914-class` at tier 4 (profile field `psg.part`). The PSG function rules (tone, mixer, noise, envelope, amplitude) are read from the AY-3-8910 pages of the 1980 and 1982 catalogs and tagged `inference` (C11, C12, C69), so every claim that depends on them (CL-0012, CL-0013) stays `provisional`. No tier 1 page for the 8914 was found; a primary data sheet of the console part or a D10 capture is the resolver.
- **D4 (note):** D4 stands after the second original diagnostic (`ivs-diag-ctrl-psg-1`, 89 hand-assembled words, derivation committed before the listing). The project mini-assembler trigger is a third diagnostic or the need for labels and macros, expected in Phase 7; the two listings so far needed neither.
- **D6 (note):** the controller words are guest-visible from Phase 5: the in-core byte function reads the generated control table (31 rows on two ports, tier 4, byte table per Phase 5 D-02, `data/input/intv-2609-ntsc-family-p1.literal.txt`) and the electrical combinations follow the ground-union rule (inference). The reversal evidence is unchanged: output that is not a function of switch state plus port drive, or a need for raw port injection. The wiring is unmeasured (C16, C63 to C66, D10).
- **D7 (confirmed, not changed):** the row above reads as decided and matches Phase 5 D-21: reference PCM outside the core and every installed artifact, raw audio as per-channel level samples every 16 master ticks plus a cycle-stamped write log, reversal trigger an adapter that needs bit-identical PCM across hosts (promote to an installed `ivs::refpcm` target, still outside the core). The Phase 5 listening handoff (D-22) was not performable and is recorded as a deferred item: no Phase 5 artifact has a listenable tone.

Phase 6 notes on the register above (plan 06-14, 2026-11-12; Delegated decision, owner authorization 2026-11-09; nothing here is a new owner decision and each row is reversible by the evidence in its last column):

- **D8 (decided and corrected in place):** the D8 row above is the decision of Phase 6 (D-12..D-17): the published raw framebuffer is 159 x 96 positions, not a 160-column plane, because the only tier 1 count for the `-1` part is the 159 x 96 of the GI catalog pages, while a 160-column nominal grid appears in the Blue Sky Rangers page and a 160th column in community material about the STIC1A. The choice rests on one profile field, so the reversal is cheap and is stated in the last column; discrepancy C10 stays `partial`, never `resolved-primary`, with the owner capture D10-VID-01 as its resolver. Which seven pixels of the last card column are displayed (C101), the border and window origin (C105) and the content of a display-off frame (C99) are labelled defaults. Claims CL-0018 and CL-0019 stay `provisional`.
- **D3 (note):** the STIC is pinned as the AY-3-8900-1 NTSC part (profile field `stic.part`, tier 1, CL-0002). The geometry counts, the 14-bit bus width, the 12 rows and the 240 BACKTAB words are tier 1 (GI 1980 and 1982 catalog pages); the register map, the strobe and alias behaviour, the access windows, the interrupt width, the bus-window table and the card word layout are tier 4 community material, inference or labelled defaults, so register-level behaviour is tier 4 and every claim that depends on it (CL-0016 to CL-0022) is `provisional`. The PAL AY-3-8900 part and the STIC1A variant are out of scope. A tier 1 to 3 register description or a D10 capture is the resolver.
- **D4 (note):** D4 stands after the third original diagnostic (`ivs-diag-stic-1`, 133 hand-assembled words: 101 code and 32 pattern data, five listed branches, a listing of 220 lines; the derivation was committed before the listing). The crossing rule of plan 06-13 was evaluated and not crossed (more than 170 words or more than 8 branches: no; an error found only by a run: none; a second STIC cartridge: no; a second author: no), and a count of three is not above three. Effort data: zero errors caught by the listing checker, two slips caught by hand before it, zero escapes found by a run. Caveats recorded with it: the checker decodes a data table as program (a data directive is the one thing this diagnostic asks of an assembler), the boot stub has no interrupt routine so exactly one picture exists (a project interrupt routine is a Phase 7 need), the derivation's cycle budget ran 31 cycles above the observed run (an inference gap), and the `BITS` column came from a scratch formatter so the `listing-bits-mismatch` check was not exercised by it. The project mini-assembler decision is expected to be re-evaluated at the start of Phase 7, when a fourth diagnostic, labels or macros, or the interrupt routine make the trigger fire.

Phase 7 notes on the register above (plan 07-12, 2026-11-13; Delegated decision, owner authorization 2026-11-09; nothing here is a new owner decision and each row is reversible by the evidence in its last column):

- **D4 (re-evaluated, decided: build `ivs_asm`):** the D4 trigger fired on three independent counts at the Phase 7 start (a fourth project-authored program, labels for the interrupt entry and the loop heads, and a program of about ten branches), so the project mini-assembler of the D4 reversal clause was built (Phase 7 D-16, D-17, D-19; plans 07-03 and 07-04). The D4 row above carries the wording. Effort data: 1,137 lines of C (hard stop 1,200); an encoding table written from `docs/cp1610/reference-notes.md` and the cited manual pages with no outside code; checked by the listing step against `data/cp1610/opcode-families.json`, a corpus of 304 hand-derived pairs, byte-for-byte twins of the four legacy listings and the pinned refusals; the Lamp Tone source (110 statements, 198 words, 17 labels, 15 relative branches, 3 absolute jump targets) assembled at the first attempt with zero encoding errors caught, zero escapes found by a run and zero hand slips, so the reversal evidence (two escapes found only by a run) is absent. `tools/asm/asm.c` and `tools/pack/pack.c` are pinned by the recipe digests of the manifests.
- **D5 (extended):** the project boot stub gained a second contract (`boot-stub-2`: cartridge reset entry `$5000`, cartridge interrupt entry `$5004`, a `JD` at the interrupt vector); contract 1 is unchanged, so no earlier diagnostic, golden or mutant moved. It is still a stub, not an EXEC.
- **SCN-01 and the first playable-tier claim:** the original program Lamp Tone (`ivs-demo-lamp-tone-1`, fixture `FX-demo-lamp-tone-1`, MIT, `redistributable: yes`, tracked source only) is recorded by CL-0023 (tier `playable-guest-progress`) and CL-0024 (tier `installed-host-behavior`), both provisional by the weakest-field rule; the only allowed wording is "playable guest progress, for this original program only". No person has looked at the picture or listened to the sound, no timing is claimed, no commercial content or real EXEC or GROM is involved, and nothing was measured (D10).
- **SCN-02 and D11:** SCN-02 is complete on documented-route terms (Phase 5 and 6 close-out precedent); D11 stays open and not performed, so there is no claim-ledger entry and no tier for any real-EXEC result, and no known EXEC or GROM digest is recorded in the repository.
- **D9 (applied to stale text):** the licence is MIT and release publication is a delegated flow; the lines that still called the licence an owner gate (the Out of Scope list, the owner-gate row above, REL-01) were corrected at the Phase 7 transition. A release ships the program only as tracked source; neither the runner nor `ivs_pack` nor `ivs_asm` is installed, so nothing downloadable is runnable yet (`docs/packaging.md`, "Release content").

D5 and the cartridge format were decided by the fan-out protocol on 2026-11-11 (Delegated decision, owner authorization 2026-11-09). D6, D7, D8 and D12 were not decided in Phase 1; each was decided by the fan-out protocol in the phase that needed it (rows above). D10 and D11 remain hard stops with evidence marked missing (D-05) and were not performed.

### Recommendations (not owner decisions)

| Recommendation | Rationale | Status |
|----------------|-----------|--------|
| Early end-to-end slice: one explicitly named Master Component profile + original diagnostic fixture + independent headless consumer | Exercises core, runner, fixture provenance and consumer boundary together | ✓ Phase 4: Slice 0, an original diagnostic through the runner and the relocated installed consumer |
| 2609-family Master Component as the candidate first profile | Original NTSC console; best-documented; MAME/jzIntv/manuals all reference it | ✓ Adopted as D3, `intv-2609-ntsc-family-p1` (delegated decision); a board revision stays unmeasured (D10) |
| Keep one playable/homebrew scenario visible early | Prevent research-only drift | ✓ Phase 7: the original program Lamp Tone (tracked source, CL-0023), no third-party title |
| Original source-built diagnostics over third-party test ROMs | No cleared conformance suite found | ✓ Applied: project-authored diagnostics and the Lamp Tone program, no third-party test ROM |

### GSD-generated decisions (reversible defaults)

| Decision | Rationale | Outcome |
|----------|-----------|---------|
| Keep owner-provided `.planning/config.json` unchanged rather than re-asking auto-mode config questions | Owner pre-set the established emulator-project defaults | ✓ Applied at init |
| Project structure mode: vertical MVP slices | GSD `--auto` default; matches seed's "verified vertical slices" | ✓ Applied at init |
| Do not set `workflow._auto_chain_active` at init | Kickoff explicitly stops before discuss/plan-phase | ✓ Applied at init |

### Open questions (unresolved — preserve, do not silently decide)

1. Exact first console/variant profile; region and TV timing (NTSC/PAL).
2. CPU, STIC, PSG and memory/timing architecture; required raster and audio
   precision.
3. How EXEC/GROM system images are supplied, if at all (user-supplied path,
   original replacement, or none), and their identity verification.
4. Cartridge formats and unusual memory maps to support first.
5. Controller electrical representation vs logical host mapping; overlays as
   metadata vs feature.
6. Whether IntelliVoice, ECS, keyboard component, Intellivision II and clones
   are first-release, later-roadmap or out of scope.
7. Implementation language and build system; public API/ABI; error, ownership
   and versioning policy.
8. Debugger/headless runner shape; save-state and battery-backed persistence.
   (Persistence policy decided in Phase 8: Decision Register row OQ8.)
9. Initial original/cleared fixture and independent oracle; first playable or
   homebrew scenario.
10. First consumer and supported runtime/platforms; whether/when to pursue a
    LibFuturo adapter.
11. Dependency policy details, new-code license, repository visibility, release
    artifacts, performance targets. (Licence MIT and visibility public: D9;
    release content: `docs/packaging.md`; performance targets: PRF-01 sets none,
    Decision Register row OQ11.)

## Evolution

This document evolves at phase transitions and milestone boundaries.

**After each phase transition** (via `/gsd-transition`):
1. Requirements invalidated? → Move to Out of Scope with reason
2. Requirements validated? → Move to Validated with phase reference
3. New requirements emerged? → Add to Active
4. Decisions to log? → Add to the Decision Register (correct category)
5. "What This Is" still accurate? → Update if drifted

**After each milestone** (via `/gsd-complete-milestone`):
1. Full review of all sections
2. Core Value check — still the right priority?
3. Audit Out of Scope — reasons still valid?
4. Update Context with current state
5. Main green, open PRs triaged, rolling roadmap and lessons updated, release
   of exact tested artifact targeted when rights/readiness/authority permit

---
*Last updated: 2026-11-24 at the v0.2.0 milestone opening, after the milestone ritual, the corrected cost reading and the D14 to D16 scope decisions*

---
phase: 01-evidence-profile-and-rights-foundations
verified: 2026-11-10T00:25:00Z
status: passed
score: 5/5 must-haves verified
covered_files: [".planning/phases/01-evidence-profile-and-rights-foundations/01-01-PLAN.md", ".planning/phases/01-evidence-profile-and-rights-foundations/01-01-SUMMARY.md", ".planning/phases/01-evidence-profile-and-rights-foundations/01-02-PLAN.md", ".planning/phases/01-evidence-profile-and-rights-foundations/01-02-SUMMARY.md", ".planning/phases/01-evidence-profile-and-rights-foundations/01-03-PLAN.md", ".planning/phases/01-evidence-profile-and-rights-foundations/01-03-SUMMARY.md", ".planning/phases/01-evidence-profile-and-rights-foundations/01-04-PLAN.md", ".planning/phases/01-evidence-profile-and-rights-foundations/01-04-SUMMARY.md", ".planning/phases/01-evidence-profile-and-rights-foundations/01-05-PLAN.md", ".planning/phases/01-evidence-profile-and-rights-foundations/01-05-SUMMARY.md", ".planning/phases/01-evidence-profile-and-rights-foundations/01-06-PLAN.md", ".planning/phases/01-evidence-profile-and-rights-foundations/01-06-SUMMARY.md", ".planning/phases/01-evidence-profile-and-rights-foundations/01-07-PLAN.md", ".planning/phases/01-evidence-profile-and-rights-foundations/01-07-SUMMARY.md", ".planning/phases/01-evidence-profile-and-rights-foundations/01-08-PLAN.md", ".planning/phases/01-evidence-profile-and-rights-foundations/01-08-SUMMARY.md", ".planning/phases/01-evidence-profile-and-rights-foundations/01-09-PLAN.md", ".planning/phases/01-evidence-profile-and-rights-foundations/01-09-SUMMARY.md", ".planning/phases/01-evidence-profile-and-rights-foundations/01-10-PLAN.md", ".planning/phases/01-evidence-profile-and-rights-foundations/01-10-SUMMARY.md", ".planning/phases/01-evidence-profile-and-rights-foundations/01-11-PLAN.md", ".planning/phases/01-evidence-profile-and-rights-foundations/01-11-SUMMARY.md", ".planning/phases/01-evidence-profile-and-rights-foundations/01-12-PLAN.md", ".planning/phases/01-evidence-profile-and-rights-foundations/01-12-SUMMARY.md", ".planning/phases/01-evidence-profile-and-rights-foundations/01-13-PLAN.md", ".planning/phases/01-evidence-profile-and-rights-foundations/01-13-SUMMARY.md", ".planning/phases/01-evidence-profile-and-rights-foundations/01-14-PLAN.md", ".planning/phases/01-evidence-profile-and-rights-foundations/01-14-SUMMARY.md", "README.md", "data/claims.json", "data/discrepancies.json", "data/profiles/intv-2609-ntsc-family-p1.json", "docs/cp1610/reference-notes.md", "docs/evidence-tiers.md", "docs/fixture-manifest.md", "docs/provenance.md", "docs/time-base.md", "fixtures/FX-sample-rights-unknown/manifest.json", "tools/gate.sh", "tools/hygiene.sh"]
covered_digest: "v4:sha256:58ee61e51a87304fed52ce13be2da3782082f399095ed42892f3dc37d850c8e7"
behavior_unverified: 0
overrides_applied: 0
coincidental_reliance_items: []
flagged_prohibitions:
  - statement: "MUST NOT copy, port or paraphrase any emulator's code, comments, identifiers, table layouts or file structure; MUST NOT open GPL emulator source for implementation"
    verification: judgment
    verdict: "non-authoritative LLM-judge: no violation found (denylist scan green, consultation log lists two disclosed research-era readings, table values re-checked against the manual page images)"
    flag: "unverified-prohibition - human review recommended (docs/provenance.md states authorship cannot be proven by any scan)"
  - statement: "MUST NOT take an encoding, cycle count or status bit from the OCR text layer or from any emulator source; only the page image counts as [OBS]"
    verification: judgment
    verdict: "non-authoritative LLM-judge: no violation found (three cited pages re-read from the PDF and matched; decode coverage and the second-reader pass are same-model-family evidence)"
    flag: "unverified-prohibition - human review recommended (only a sample of pages was independently re-read)"
human_verification: []
---

# Phase 1: Evidence, Profile and Rights Foundations - Verification Report

**Phase Goal:** Before any machine code exists, every later claim has a defined place to cite a named profile, an evidence tier, a provenance statement and a rights status. The CP-1610 oracle also exists as project-owned, page-cited notes.
**Verified:** 2026-11-10
**Status:** passed (complete with 2 flagged judgment-tier prohibitions, recorded non-authoritatively below; no tooling-resolvable gap and no human UAT item)
**Re-verification:** No - initial verification

## Method and what was independently run

SUMMARY.md claims were not used as evidence. This run executed, from the repository root:

| Check | Result |
|-------|--------|
| `sh tools/gate.sh` (twice, plus once for exit status) | `gate: PASS steps=17/17 controls=112/112`, exit 0, 35 discrepancies, 74 CP-1610 rows, 1024 patterns, `unclaimed=0 ambiguous=0 coincidences=1`, `agree=71 resolved=3`, `claims=5 provisional=4`, `manifests=1 effective-yes=0 effective-no=1` |
| `sh tools/hygiene.sh --tree` | `hygiene pass mode=tree hygiene-files=532` |
| `sh tools/hygiene.sh --history` | `hygiene pass mode=history commits=74` |
| `gh repo view` | `visibility: PUBLIC`, default branch `main` |
| `git ls-remote origin refs/heads/main` | `c1a8d07fac0b7b3be3dc5e659b48acbb3a70a01b` (local `main` is one docs-only commit ahead, unpushed) |
| `gh run list` | NOT RE-VERIFIED: the shared API quota was exhausted (HTTP 403). `.planning/CI-EFFICIENCY.md` logs a `success` run for commit `c1a8d07`, which equals the remote `main` head. Treated as reported, not independently confirmed. |
| Mutation check on a fresh clone (no `refs/` cache) | Flipping the sample manifest to `committed: true`, appending an unqualified support sentence to `README.md`, and deleting discrepancy C20 each turned the gate to `gate: FAIL steps=16/17`. The unmodified clone passes with `refs-verified=0/4`, so the gate does not depend on the local PDF cache. |
| Page-image spot checks against the locally cached GI manual (SRC-gi-cp1600-um-1975) | pdf 62 (stack operations): matches the notes (post-increment on output, pre-decrement on input, PSHR/PULR forms). pdf 54 (branches): matches (`7/9` printed unlabelled on B and condition rows, NOPP `7`). pdf 21 (initialization sequence): matches (MSYNC*, IAB supplies the starting PC, registers and status undefined). |
| Independent re-read of the research-era tier-1 cite | Downloaded the 1980 catalog (sha256 equals the registry value) and read pdf 191 (printed 5-10): "AY-3-8900-1 for NTSC", "3.579545MHz clock for AY-3-8900-1", 159 x 96 visible positions. This confirms the STIC part, NTSC region and crystal figure the profile tags `tier1-primary-doc`, which the profile otherwise marks "research-era reading, not re-read in phase 1". |

## Goal Achievement

### Observable Truths (ROADMAP success criteria)

| # | Truth | Status | Evidence |
|---|-------|--------|----------|
| 1 | First-profile record names model, revision, region/video timing, STIC part, PSG part, reset/power-on policy and cartridge arrangement; every field tagged; unknowns say `unknown-default-chosen`; cites D3 | VERIFIED | `data/profiles/intv-2609-ntsc-family-p1.json` has 23 fields: `model`, `board_revision`, `aliases`, `region_video`, `cpu.part`, `stic.part`, `psg.part`, `crystal_hz`, `ticks_per_*`, `scanlines_per_frame`, `cpu_cycles_per_scanline`, `reset.r7`/`reset.registers_other`/`reset.flags`, `power_on_fill.{ram,gram,stic,psg}`, `cartridge.slots`/`cartridge.address_windows`, `system_images.exec_grom`. Each has a `tag`, a `cite`, and (when `unknown-default-chosen`, 12 of 23) `resolving_evidence` and `revisit_trigger`. `selected_by` names D3, and the D3 row exists at `.planning/PROJECT.md` Decision Register line 165. Gate step `profile` plus 12 `profile-*` negative controls (missing field, blank value, no D3, tier-5 tag, unknown tag, unknown-default-chosen without evidence) all pass. The revision is honestly `unspecified-2609-family` (a family-level profile, not a pinned board); that is a recorded unknown, not a blank. |
| 2 | Tier vocabulary, claim ledger, discrepancy log with an entry for each of C1-C28 (each with default policy and resolver); no support claim in docs without profile + tier + artifact | VERIFIED | `docs/evidence-tiers.md` and `data/tiers.json` define two disjoint vocabularies (10 source-authority tags, 8 evidence tiers); the `vocab` step enforces disjointness. `data/discrepancies.json` holds C1-C35 (35 ids, set-equal to `data/expected/discrepancies.ids`); every entry has `default_policy` and `resolver`, and 33 are `open`, one `resolved-primary` (C17) and one `partial` (C19). `data/claims.json` has 5 claims, each with profile, tier, existing artifact, `depends_on` and standing; the weakest-field rule keeps 4 provisional, and CL-0002 (STIC AY-3-8900-1, 3.579545 MHz) is the only established claim, which I confirmed against catalog pdf 191. `README.md` support-style sentences carry `[claim:CL-0002]`/`[claim:CL-0003]`; my own grep for support/compatible/works/accurate across `README.md` and `docs/` found no untagged assertion (remaining hits are rule text, "not supported" manual statements, or `[nc]` waivers). Mutation check: an appended unqualified support sentence fails the gate; deleting C20 fails the gate. |
| 3 | Fixture-manifest schema + no-copy provenance rule in the review checklist; sample `redistributable: unknown` entry shown to be non-redistributable | VERIFIED | `docs/fixture-manifest.md` documents schema version 1 (source, recipe tool/version/hash, inputs/outputs with digests, licence, licence evidence, notices, `allowed_use`, `redistributable`, rights review) and the computed "effective redistributable" rule (fail closed). `fixtures/FX-sample-rights-unknown/manifest.json` has `redistributable: "unknown"`, `committed: false`, `allowed_use: ["local-test"]`; the gate reports `effective-yes=0 effective-no=1`. Control `manifests-unknown-committed` and my own flip to `committed: true` both fail the gate. The no-copy rule and a 7-item review checklist are in `docs/provenance.md` and `.github/pull_request_template.md`. `git ls-files refs` is empty and `.gitignore` excludes `/refs/`, PDFs and images; the `files` step refuses tracked binaries outside an effective-yes manifest. |
| 4 | CP-1610 reference notes: opcode encodings, cycle counts, reset/interrupt sequence, each page-cited to the directly read GI manual; C17 state stated | VERIFIED | `docs/cp1610/reference-notes.md` (296 lines) cites `pdf <n>` / `§section, pdf <n>` with [OBS]/[INF] labels on every fact. `data/cp1610/opcode-families.json` has 74 rows tiling all 1024 decle patterns (`unclaimed=0 ambiguous=0`, one declared coincidence); `data/cp1610/second-reader.json` has 74 verdicts (71 agree, 3 resolved page-internal differences, no row changed). Reset (`§C.1, pdf 21`, `pdf 27`) and interrupt (`§C.6, pdf 24-25`, `pdf 35`) sequences are present; unlabelled `7/9` branch counts, SDBD indirect counts and the interrupt-entry count are recorded as open discrepancies (C29, C30, C34) rather than invented. The notes state `C17 status: resolved-primary` (stack grows upward: push writes then increments R6, pull decrements then reads, pdf 24, 46, 48, 62); the `cp1610` step cross-checks that line against the discrepancy log, and the pdf 62 page image agrees. Three page cites were independently re-read (pdf 21, 54, 62) and matched. |
| 5 | Time-base glossary separating CPU cycle, input clock, STIC/PSG clocks and crystal constant; D2 recorded as an architecture decision | VERIFIED | `docs/time-base.md` defines Crystal, Master tick, CPU cycle, CPU input clock, Time slot, STIC clock, PSG chip clock, PSG step and Frame (9 terms, equal to `data/expected/glossary-terms.ids`), plus the D2 decision with rationale, reversibility and reversal trigger. `.planning/PROJECT.md` line 164 records D2 (master tick, `uint64_t`, CPU cycle = profile ratio 4). The `glossary` step bans the bare unit word in `README.md` and `docs/` and checks profile key naming; a control (`vocab-conflated-id` family and `glossary-*`) covers it. The CPU-cycle versus input-clock confusion is cross-referenced to C8. |

**Score:** 5/5 truths verified (0 present, behavior-unverified; 0 coincidental-reliance).

### Behavior-dependent truths

The gate's checks are the behaviors here, and each has been exercised by its own negative controls (112/112 pass under `sh tools/gate.sh`) and by three independent mutations run in a clean clone. No truth rests on symbol presence alone, so none is marked PRESENT_BEHAVIOR_UNVERIFIED.

### Required Artifacts

| Artifact | Expected | Status | Details |
|----------|----------|--------|---------|
| `data/profiles/intv-2609-ntsc-family-p1.json` | Tagged first profile | VERIFIED | 23 fields, all tagged and cited |
| `data/discrepancies.json` | C1-C28 plus later entries | VERIFIED | 35 entries, all with policy and resolver |
| `data/claims.json`, `data/tiers.json`, `docs/evidence-tiers.md` | Ledger and vocabularies | VERIFIED | 5 claims, 10 tags, 8 tiers |
| `docs/fixture-manifest.md`, `fixtures/FX-sample-rights-unknown/manifest.json` | Schema and sample | VERIFIED | Fail-closed, effective-no |
| `docs/provenance.md`, `docs/provenance/CONSULTATIONS.md`, `docs/RIGHTS.md` | No-copy rule, log, rights policy | VERIFIED | Rule, checklist, two logged consultations |
| `docs/cp1610/reference-notes.md`, `data/cp1610/opcode-families.json`, `data/cp1610/second-reader.json` | Page-cited oracle notes | VERIFIED | 74 rows, 1024-pattern coverage |
| `data/sources.json` | Reference registry | VERIFIED | 9 sources, digests, `rights: unknown`; local PDF digest matches |
| `docs/time-base.md` | Glossary and D2 | VERIFIED | 9 terms |
| `tools/gate.sh`, `tools/hygiene.sh`, `tools/check/**`, `.github/workflows/gate.yml` | One local command mirrored by CI | VERIFIED | 17 steps, 112 controls, one workflow, one `uses:` pinned |

### Key Link Verification

| From | To | Via | Status |
|------|----|-----|--------|
| Profile `selected_by` | `.planning/PROJECT.md` D3 row | `profile` step requires D3 (`profile-no-d3` control) | WIRED |
| Profile `conflict` ids | `data/discrepancies.json` | `profile-conflict-unknown-id`, `profile-bad-conflict-id` controls | WIRED |
| Claims `artifact` / `depends_on` | Files and profile fields | `claims` step; weakest-field rule | WIRED |
| `[claim:CL-NNNN]` in `README.md`/`docs/` | `data/expected/claims.ids` | `text` step (`text-missing-claim-ref`, `text-untagged-claim-word`) | WIRED |
| `SRC-<slug>` tokens | `data/sources.json` | `sources` step (`sources-unknown-ref`) | WIRED |
| Notes `C17 status:` line | Discrepancy C17 status | `cp1610` step (`c17-consistency`) | WIRED |
| Manifests | `files` step binary rule | effective-yes computed rule | WIRED |
| CI workflow | `sh tools/gate.sh` | `ci-parity` step | WIRED |

### Data-Flow Trace (Level 4)

Not applicable: no code renders dynamic data. The data-flow analogue (a value tagged tier 1 actually traces to a source page) was sampled for `stic.part`, `region_video` and `crystal_hz` (catalog pdf 191) and for the stack and reset facts (manual pdf 21, 62); all matched.

### Behavioral Spot-Checks

| Behavior | Command | Result | Status |
|----------|---------|--------|--------|
| Gate passes on the working tree | `sh tools/gate.sh` | `gate: PASS steps=17/17 controls=112/112` | PASS |
| Sample unknown manifest cannot be committed | clone, set `committed: true`, run gate | FAIL | PASS |
| Unqualified support sentence is caught | clone, append sentence to `README.md`, run gate | FAIL | PASS |
| Discrepancy inventory is exact | clone, drop C20, run gate | FAIL | PASS |
| Clean clone passes without the PDF cache | run gate in unmodified clone | PASS, `refs-verified=0/4` | PASS |

### Probe Execution

SKIPPED: no `probe-*.sh` is declared by any PLAN, and no conventional `scripts/*/tests/probe-*.sh` exists. The gate itself is the executable proof.

### Requirements Coverage

Every requirement ID in the plans' frontmatter is accounted for in `.planning/REQUIREMENTS.md`, and every ID mapped to Phase 1 there is claimed by at least one plan (no orphans).

| Requirement | Source plans | Description | Status | Evidence |
|-------------|--------------|-------------|--------|----------|
| FND-01 | 01-01, 01-08, 01-13 | First hardware profile as tagged data record (D3) | SATISFIED | Truth 1 |
| FND-02 | 01-01, 01-05, 01-11, 01-13 | Tier vocabulary and claim ledger; claims cite profile, tier, artifact | SATISFIED | Truth 2 |
| FND-03 | 01-04 | Discrepancy log seeded with C1-C28, default policy, resolver | SATISFIED | Truth 2 (C1-C35) |
| FND-04 | 01-06 | Fixture manifest with `redistributable` (unknown = no) | SATISFIED | Truth 3 |
| FND-05 | 01-06, 01-07, 01-11, 01-12 | Provenance rule documented and checked in review | SATISFIED (with flagged prohibitions, below) | Truth 3 |
| FND-06 | 01-02, 01-03, 01-07, 01-09, 01-10, 01-14 | CP-1610 notes from the directly read manual, page-cited | SATISFIED | Truth 4 |
| FND-07 | 01-08 | Time-base vocabulary and D2 | SATISFIED for documents and profile keys; the code-level check is explicitly queued for Phase 2 (`docs/time-base.md`, "Queued for Phase 2"), consistent with the requirement being about vocabulary used in "code and docs" where no code exists yet | Truth 5 |

Decision coverage (`check.decision-coverage-verify`): 24 of 24 trackable CONTEXT.md decisions honored.

### Anti-Patterns Found

| File | Line | Pattern | Severity | Impact |
|------|------|---------|----------|--------|
| (phase files) | - | TBD/FIXME/XXX debt markers | None found by the gate's own scans or by hygiene | - |
| `.planning/phases/01-evidence-profile-and-rights-foundations/01-REVIEW.md` | IN-01..IN-05 | Open Info items: SRC- token truncation, zero-file scans pass, manifest output paths not anchored to own directory, gate counts a step on exit 0 without its pass line, thin control-runner self-test | Info | Not blockers; hardening of the checker. IN-03 only matters once an effective-yes committed manifest exists (none yet). |
| `.planning/phases/01-evidence-profile-and-rights-foundations/01-VALIDATION.md` | frontmatter | `status: draft`, `nyquist_compliant: false` | Info | Stale planning metadata; validate-phase was not run. The gate it describes is green. |

### Flagged prohibitions (ADR-550 D4, autonomous mode)

Both are judgment-tier, so no scan can prove them (`docs/provenance.md` "Honest limits" says so itself). The verdicts below are the verifier's NON-AUTHORITATIVE LLM-judge readings and are not a pass certificate.

1. No copying or paraphrasing of another emulator's code, comments, identifiers, table layouts or file structure, and no GPL emulator source opened for implementation. Reading: no violation found. The denylist and hygiene scans are green; `docs/provenance/CONSULTATIONS.md` discloses the two research-era readings (licence headers only, and crystal-divisor definition lines) and says values were re-derived from tier 1 sources; the profile tags the one value backed only by an emulator (`ticks_per_psg_clock`) as `unknown-default-chosen` with a Phase 5 resolver. Flag: unverified-prohibition, human review recommended.
2. No encoding, cycle count or status bit taken from the OCR text layer or an emulator. Reading: no violation found in the three pages re-read here (pdf 21, 54, 62) plus the catalog page; the table's own coverage and second-reader checks are same-model-family evidence. Flag: unverified-prohibition, human review recommended.

The test-tier prohibitions in the plans (no tracked rights-bearing bytes, no absolute or scratch paths or secrets in tracked files, no unqualified claims in `README.md` or `docs/`, no tracked `refs/`) all have wired enforcement in the gate and hygiene tools and were exercised (gate, `hygiene --tree`, `hygiene --history`, mutation check). These are not flagged.

Per the project owner's autonomy policy (`.planning/AUTONOMY.md`: zero human UAT; reserve humans for judgment tooling cannot make), these flags are recorded for the owner's optional review at milestone close and do not hold the phase. They are not entered as `human_verification` items.

### Human Verification Required

None. Nothing in the five success criteria needs subjective judgment, physical evidence or a decision tooling cannot make. D10 (hardware measurement) and D11 (owner EXEC/GROM) are future gates, correctly recorded as pending resolvers, not Phase 1 obligations.

## Residual risks and honest limits (stated, not blockers)

- The manual is the CP-1600 users manual; its application to the CP-1610 is an inference, logged as C31 and carried into every CP-1610 claim (CL-0001, 0004, 0005 stay provisional).
- The second reader of the opcode table is the same model family as the first. A shared misreading is possible; decode coverage proves encoding tiling only, and cycle counts, status bits and interruptibility rest on two same-family readings. This phase re-read three pages independently and found no disagreement, which is a sample, not a proof.
- Several profile fields tagged `tier1-primary-doc` (`cpu.part`, `ticks_per_stic_clock`, plus the ones I re-read) carry "research-era reading" notes. The STIC part, region and crystal cite were re-verified in this run; `cpu.part` (catalog printed 5-3) and the STIC-clock note were not re-read.
- The `psg.part` field is `tier4-community` (C11): no primary datasheet for the AY-3-8914 was found, and the roadmap research flag to search for one ended in an honest unknown rather than a source.
- The claim-language tripwire is a word list; a paraphrase such as "works with" escapes it (`docs/evidence-tiers.md`). Duplicate JSON keys are undetectable.
- Tool-level residual risks from `01-REVIEW-FIX.md`: `grep -a` is not POSIX; the byte-class heuristic for binaries is a 30 percent threshold; a NUL-free binary with a text extension is not detected; the CI parity check is line-based, not a YAML parser.
- Only 1 of the 4 registry references with a local cache path is present in this working copy (`refs-verified=1/4`), and a clean clone verifies 0/4 by design (rights policy: references stay local). Reproducing the page cites requires re-downloading from the registry URLs.
- CI success for `c1a8d07` is reported by `.planning/CI-EFFICIENCY.md` but was not independently confirmed here because the GitHub API quota was exhausted.

## Gaps Summary

No gaps. Process notes, none affecting the verdict:

- `.planning/ROADMAP.md` marks Phase 1 `**Mode:** mvp`, but its goal is not in "As a ..., I want ..., so that ..." user-story form (`user-story.validate` returns `valid: false`). The verifier guard for MVP mode would normally refuse. The caller supplied the five success criteria explicitly and this phase is a walking skeleton of tooling and documents, not a user flow, so verification proceeded against the five criteria. If MVP-mode formality matters later, either run `/gsd-mvp-phase 1` to restate the goal or drop the `Mode` line.
- The working tree has an uncommitted `.planning/config.json` change (`_auto_chain_active` set to `true`) and an untracked `.planning/phases/01-evidence-profile-and-rights-foundations/01-SECURITY.md` that this verification did not create or touch. The local `main` is one docs-only commit ahead of `origin/main`.

---

_Verified: 2026-11-10_
_Verifier: Claude (gsd-verifier)_

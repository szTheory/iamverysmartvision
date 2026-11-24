# Phase 1: Evidence, Profile and Rights Foundations - Discussion Log

> **Audit trail only.** Do not use as input to planning, research, or execution agents.
> Decisions are captured in CONTEXT.md — this log preserves the alternatives considered.

**Date:** 2026-11-09
**Phase:** 01-evidence-profile-and-rights-foundations
**Mode:** advisor fan-out, autonomous selection. The owner gave mid-session instructions to fan out per decision and auto-follow the synthesized recommendations, and delegated the gates to autonomous mode (`.planning/AUTONOMY.md`). No AskUserQuestion prompts were issued.
**Areas discussed:** first profile (D3), time base (D2), record formats and checks, rights and provenance (FND-04/05, D9), CP-1610 reference notes (FND-06), language/build (D1)

---

## First profile (D3)

| Option | Description | Selected |
|--------|-------------|----------|
| Family-level 2609 NTSC | Model documented function; abstract revision; per-field tags | ✓ |
| Pin owner's physical board | Measured chip markings and values; needs D10 hardware | |

**Choice:** [auto] Family-level `intv-2609-ntsc-family-p1`.
**Notes:**
- No primary AY-3-8914 datasheet was found; the GI 1980 catalog text contains no "8914".
- STIC1A is not modelled.
- Default power-on fill is zero. Seeded-random was rejected because it freezes a PRNG.

## Time base (D2)

| Option | Description | Selected |
|--------|-------------|----------|
| CPU cycle as unit | Matches manual numbers; retrofit needed for non-multiple clocks | |
| Master tick (crystal period), u64 | One integer unit; ratios are profile fields | ✓ |
| LCM fine tick | Invents unsupported precision | |
| Attotime / float scalar | Rounding policy, determinism risk | |

**Choice:** [auto] Master tick.

**Reconciliation:** the time-base agent proposed a 315/88 MHz default for the crystal; the profile agent proposed the tier-1 nominal 3,579,545 Hz. The profile stores a rational with 3,579,545/1 as the default, tagged tier1-nominal, and records 315/88 as an inference note. The core never reads Hz.

## Record formats and checks

| Option | Description | Selected |
|--------|-------------|----------|
| deb822 `.rec` + sh/awk | Zero dependencies, D1-neutral; awk dialect risk | |
| JSON + `cmake -P` | Sibling-proven, nested fields, one toolchain | ✓ (after D1) |
| TOML / YAML / Markdown tables | Parser dependency or brittle | |

**Choice:** [auto] JSON + `cmake -P`. The records agent preferred `.rec` only while D1 was open. Once D1 resolved to CMake, its own fallback (option B) became the better fit.

## Rights and provenance

| Option | Description | Selected |
|--------|-------------|----------|
| Own JSON manifest, SPDX/SLSA-shaped fields | Matches FND-04, no dependencies | ✓ |
| REUSE / SPDX documents as primary | Lacks recipe, `redistributable` and "unknown"; extra tool | |
| No-copy: rule + checklist + default-deny hygiene checks | Cheap, catches accidents | ✓ |
| Similarity / licence scanners | Heavy dependencies, needs a contaminating corpus | |

**Choice:** [auto] The first and third options.
**D9 override:** the rights agent's interim recommendation (private repo, no licence) was superseded by the owner's delegation of public publication. MIT was chosen to match the sibling repos.

## CP-1610 reference notes

| Option | Description | Selected |
|--------|-------------|----------|
| Prose + tables only | No coverage check | |
| Prose + cited family-row JSON + 1024-pattern coverage, used as inventory only | Independence rules locked | ✓ |
| Generated table feeding emulator and vectors | Shared misreading | |

**Choice:** [auto] The second option.
**Findings:**
- The PDF is a scan with a damaged OCR layer.
- C17 is confirmed (stack grows upward).
- C19 is only partially resolved.
- Branch cycles "7/9" are unlabeled.

## Language / build (D1)

| Option | Description | Selected |
|--------|-------------|----------|
| C17 + CMake | Sibling fit, stable sanitizers, C ABI native | ✓ (~75%) |
| Rust + cargo-c | No UB; nightly sanitizers, second toolchain | |

**Choice:** [auto] C17 + CMake.

## Claude's Discretion

- JSON key spelling.
- How the checker is split.
- Where the coverage check lives.
- Whether to rewrite pre-publication history.

## Deferred Ideas

- Fill-sensitivity matrix.
- Owner-unit profile (D10).
- STIC1A, PAL and aliases.
- MSan, valgrind and fuzz scheduling.
- LibFuturo tick unit.
- Generated C header (Phase 2).

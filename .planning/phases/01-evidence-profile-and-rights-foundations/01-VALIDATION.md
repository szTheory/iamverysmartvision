---
phase: "1"
slug: "evidence-profile-and-rights-foundations"
# status lifecycle: draft (seeded by plan-phase) → validated (set by validate-phase §6)
status: draft
nyquist_compliant: false
wave_0_complete: false
created: "2026-11-09"
---

# Phase 1 — Validation Strategy

> Per-phase validation contract for feedback sampling during execution.
> Source: `01-RESEARCH.md` § Validation Architecture.

---

## Test Infrastructure

| Property | Value |
|----------|-------|
| **Framework** | None installed. A POSIX `sh` orchestrator runs `cmake -P` checkers and overlay-based negative controls |
| **Config file** | None. Inventories live in `data/expected/*.ids` plus the fragment directories `data/expected/controls/` and `data/expected/gate-steps/` (one fragment per owning plan; see SKELETON.md) |
| **Quick run command** | `sh tools/gate.sh` |
| **Full suite command** | `sh tools/gate.sh && dash tools/gate.sh` (publication adds `sh tools/hygiene.sh --history`) |
| **Estimated runtime** | ~3 seconds |

---

## Sampling Rate

- **After every task commit:** run `sh tools/gate.sh`.
- **After every plan wave:** run `sh tools/gate.sh && dash tools/gate.sh`.
- **Before `/gsd-verify-work`:** the gate must be green under both shells. Also required: `hygiene.sh --history` green, the gate green on a clean clone, and the first CI run green.
- **Max feedback latency:** 10 seconds. Plan 01-11 re-measures with all controls; if over budget it falls back to parallel or batched control runs. If the cp1610 step alone exceeds 2 s, the 1024-pattern decode moves to a Phase 2 C test (D-12).

---

## Per-Task Verification Map

There are 14 plans in 5 waves (01-01 to 01-14). Each task's `<automated>` verify plus `<fails_when>` is in its PLAN.md; all 46 commands pass the failing-direction probe. The table maps requirements to the gate steps that verify them.

| Req / SC | Behavior | Test Type | Automated Command | File Exists | Status |
|----------|----------|-----------|-------------------|-------------|--------|
| FND-01 / SC1 | Every expected profile field is present, with a known tag, a non-blank value and a cite. `unknown-default-chosen` carries `resolving_evidence`. D3 is cited | check + controls | `sh tools/gate.sh` (records:profile) | ❌ W0 | ⬜ pending |
| FND-02 / SC2 | Two distinct vocabularies. Claims resolve. Every `[claim:]` in README/docs resolves. Claim-language tripwire | check + controls | `sh tools/gate.sh` (claims, text) | ❌ W0 | ⬜ pending |
| FND-03 / SC2 | Discrepancy IDs are set-equal to the inventory (C1–C28 plus C29+). Each has `default_policy` and a resolver | check + controls | `sh tools/gate.sh` (discrepancies, ids) | ❌ W0 | ⬜ pending |
| FND-04 / SC3 | Manifest is fail-closed. The sample `unknown` entry is treated as non-redistributable. A flipped copy fails | check + controls | `sh tools/gate.sh` (manifests) | ❌ W0 | ⬜ pending |
| FND-05 / SC3 | The no-copy rule and checklist exist. Denylist runs on code paths. `refs/` is untracked. Unlisted binaries fail | check + controls | `sh tools/gate.sh` (text, files) | ❌ W0 | ⬜ pending |
| FND-06 / SC4 | Row cites, sources registry, 1024-pattern coverage, second-reader ID parity, C17 `resolved-primary` | check + controls | `sh tools/gate.sh` (cp1610) | ❌ W0 | ⬜ pending |
| FND-07 / SC5 | Glossary term inventory. Unit-suffixed profile keys. Bare-"clock" tripwire. D2 is linked | check + inventory | `sh tools/gate.sh` (glossary) | ❌ W0 | ⬜ pending |
| Gate integrity | CRLF, non-ASCII, empty file and empty inventory each fail | controls | `sh tools/gate.sh` (controls) | ❌ W0 | ⬜ pending |
| Hygiene | No home paths, non-noreply emails, secrets or unlisted binaries (tree and history) | check | `sh tools/hygiene.sh --tree` / `--history` | ❌ W0 | ⬜ pending |
| CI parity | The workflow runs exactly `sh tools/gate.sh` | check | `sh tools/gate.sh` (ci-parity) | ❌ W0 | ⬜ pending |

*Status: ⬜ pending · ✅ green · ❌ red · ⚠️ flaky*

---

## Wave 0 Requirements

- [ ] `tools/gate.sh`, `tools/hygiene.sh`, `tools/check/*.cmake`, `tools/check/run-controls.sh`
- [ ] `tools/check/controls/_good/` plus about 72 negative-control directories, with exact fragment inventories under `data/expected/controls/`
- [ ] `data/expected/*.ids` inventories and `data/expected/gate-steps/` fragments
- [ ] `.gitignore`, `LICENSE`, `README.md`, `.github/workflows/gate.yml`
- [ ] No framework install is needed

---

## Manual-Only Verifications

Each of these is mitigated as far as tooling allows, and the residual limit is stated honestly in `docs/provenance.md`.

| Behavior | Requirement | Why Manual | Mitigation |
|----------|-------------|------------|------------|
| Tier labels are assigned honestly | FND-01/02 | Truth of a label is not machine-decidable | Vocabulary and presence checks. A claim cannot exceed its weakest field |
| Page cites match the page images | FND-06 | Requires reading the scan | A blind second-reader agent pass (D-20). The coverage check catches encoding mistakes |
| The no-copy rule was actually followed | FND-05 | Authorship is not detectable | Denylist plus consultation log plus a review checklist |
| Paraphrased claim language ("works with") | FND-02 | Gap in the tripwire's word list | `[nc]` convention. Docs keep one sentence per line |

None of these needs the owner. They are handled by agent review and recorded as residual risk.

---

## Validation Sign-Off

- [ ] All tasks have `<automated>` verify or Wave 0 dependencies
- [ ] Sampling continuity: no 3 consecutive tasks without automated verify
- [ ] Wave 0 covers all MISSING references
- [ ] No watch-mode flags
- [ ] Feedback latency < 10s
- [ ] `nyquist_compliant: true` set in frontmatter

**Approval:** pending

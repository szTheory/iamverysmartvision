## What this changes

Describe the change in a few sentences.

## Provenance review checklist

Rule: `docs/provenance.md`.

- [ ] Every constant or table carries a `Source:` cite (registry id and page).
- [ ] Every consultation of an emulator is logged in `docs/provenance/CONSULTATIONS.md`.
- [ ] No emulator identifier appears in code, data or docs.
- [ ] Every binary is listed in a fixture manifest.
- [ ] Any flip of `redistributable` to yes has had an explicit review.
- [ ] Nothing under `refs/` is tracked.
- [ ] One or two constants were spot-checked against the cited page.

## Other checks

- [ ] Claims cite `[claim:CL-NNNN]`.
- [ ] No absolute local paths appear in tracked files.
- [ ] `sh tools/gate.sh` ends with `gate: PASS`.

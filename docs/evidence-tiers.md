# Evidence tiers and the claim ledger

This document defines the vocabulary the project uses to say how well a statement is backed.
It is the reference for `data/tiers.json` and `data/claims.json`.
The text step of `sh tools/gate.sh` checks that every id in `data/tiers.json` appears here.

## Two vocabularies

The project keeps two separate vocabularies, and an id never appears in both.
Source-authority tags describe how well a single fact or profile field is sourced.
Evidence tiers describe how a claim is backed.
A profile field carries exactly one source-authority tag.
A claim names exactly one evidence tier.
An id that appears in both lists fails the `vocab` step, and a claim that names a source-authority tag as its tier fails the claims step.

## Source-authority tags

Each tag below is an id in `data/tiers.json` under `source_authority_tags`.

- `tier1-primary-doc` is a manufacturer primary document, such as a data sheet, a data catalog or a users manual.
- `tier2-first-party-adjacent` is first-party-adjacent material, such as an edited reproduction of an in-house technical document.
- `tier3-service-hw` is service or hardware documentation, such as a service manual or a schematic.
- `tier4-community` is community documentation or a community measurement, and is a lead that needs a cross-check.
- `tier5-emulator-hint` is a hint taken from another emulator, and it is never evidence on its own.
- `inference` is a value the project derived from cited sources, with the derivation stated.
- `measured` is reserved for owner-approved physical measurement (D10), and no field carries it yet.
- `owner-decision` is a choice the owner made and recorded in the Decision Register.
- `delegated-decision` is a choice the project made under the owner's delegation and recorded in the Decision Register.
- `unknown-default-chosen` means the truth is unknown, a default was chosen, and the resolving evidence and the revisit trigger are recorded.

A profile field may not carry `tier5-emulator-hint` as its tag.
A value backed only by another emulator is recorded as `unknown-default-chosen`.

## Evidence tiers

Each tier below is an id in `data/tiers.json` under `evidence_tiers`.
A claim names exactly one of them.

- `source-expectation` means the expected value is taken from a cited source document.
- `public-vector` means a public test vector with documented provenance and rights backs the claim.
- `original-diagnostic` means a project-authored diagnostic program with a recorded recipe backs the claim.
- `independent-implementation-comparison` means an independently written implementation agrees, and agreement with a shared core does not count.
- `measured-hardware` means an owner-approved physical measurement of real hardware backs the claim (D10), and no claim uses it yet.
- `deterministic-repeatability` means repeated executions give identical output, which establishes repeatability and not accuracy.
- `playable-guest-progress` means a guest program makes observable progress on the named profile.
- `installed-host-behavior` means the installed package, consumed by a separate host program, behaves as stated.

## Claim ledger

The ledger is `data/claims.json`, with its exact id set in `data/expected/claims.ids`.
Each claim has these fields.

- `id` is `CL-NNNN`, with four digits.
- `statement` is the claim in one sentence.
- `profile` is the id of an existing profile in `data/profiles/`.
- `tier` is one evidence tier id.
- `artifact` is a repository-relative path of a file that exists and backs the claim.
- `depends_on` is the list of profile field paths the claim rests on, and it may be empty.
- `standing` is `established` or `provisional`.
- `note` records the caveats.

Claim ids are strictly ascending in file order, are never reused and are never renumbered.
Removing a claim is a visible edit of `data/expected/claims.ids`.
A claim without a profile, a tier and an artifact is not a claim.

## Weakest-field rule

A claim is only as strong as the weakest profile field it depends on.
If any field in `depends_on` is tagged `tier4-community`, `tier5-emulator-hint`, `inference` or `unknown-default-chosen`, the claim must be `provisional`.
Otherwise the claims step fails with `claim-overstated`.
The tier `measured-hardware` fails with `claim-measured-without-d10` until the owner decision D10 exists.

## Citing claims in prose

A statement in `README.md` or in a document under `docs/` that uses a claim word cites a claim on the same line, in the form `[claim:CL-NNNN]` with a real id.
The claim words are those matched by the tripwire in the text step.
A line that must use a claim word as rule text, such as a line that explains the tripwire, carries the waiver `[nc]` instead.
Every cited id must exist in `data/expected/claims.ids`, or the text step fails with `text-unknown-claim-ref`.
A line using a claim word without either form fails with `text-claim-untagged`.

## Honest limits

Whether a source-authority tag is true is checked by review and not by the gate.
The tripwire matches a short word list, so a paraphrase such as "works with" escapes it and is a review-only gap.
The word list is the only mechanical definition of a claim word, and it is not a proof that no overclaim exists.
Repeated identical hashes show repeatability and not accuracy.
Agreement between two parts of this project is not independent evidence.

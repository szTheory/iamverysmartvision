# Provenance

This page states what may enter this repository and how that is reviewed.
The process is called documented independent derivation.
It is not called clean-room, because the project cannot prove that no contributor has ever seen another emulator.
The rule below is a documented practice that a reviewer can check against cited sources.

## Facts may enter

What the hardware does is a fact, and a fact may be recorded in this project's own words.
Behaviour, constants and tables come from cited tier 1 to 3 sources, as defined in `docs/evidence-tiers.md`.
Every constant or table names its source by registry id (`SRC-<slug>`) and by manual page (`§<section>, pdf <n>`).
Tier 4 and tier 5 sources are leads only, and they never settle a value.
A value that only a tier 4 or tier 5 source offers is [UNK].
It enters code only as an explicit provisional item with an open evidence task.

## Another emulator's expression may not

Nothing from jzIntv, MAME or any other emulator is copied, ported or paraphrased.
This holds for a person and for an AI assistant alike.
The excluded material is code, comments, identifiers, table layouts and file structure.
A fact that both an emulator and a document state is cited to the document and derived where possible.
For example, 14,934 CPU cycles per frame is derived as 262 x 57 from the cited line and cycle counts.

## Source tiers

Tier 1 is a manufacturer primary document.
Tier 2 is first-party-adjacent material.
Tier 3 is service or hardware documentation.
Tier 4 is community documentation.
Tier 5 is another emulator, and it is a hint at most.
A tier 5 reading may point at a discrepancy to investigate.
The resolution comes from a tier 1 to 3 source, from a test, or from an owner-approved measurement.

## Copyleft emulator source

The source code of a copyleft (GPL) emulator is never opened for implementation, and no copy of it is kept in the workspace.
The documentation of such a project may be read as a tier 4 lead.
Before this rule was recorded, the research phase made one limited reading, and it is disclosed here.
On 2026-11-09, the licence notice, the assembler README and two example source files of the GPL emulator fork were read.
That reading only determined licences for the fixture-tooling decision.
No code, table or identifier was taken, and no copy was kept.
The reading is logged in `docs/provenance/CONSULTATIONS.md`.

## Permissive emulator source

The source of a permissively licensed emulator may be read only for discrepancy hints.
Each reading is logged in `docs/provenance/CONSULTATIONS.md` with its date, its purpose and the reader.
The value that results is re-derived from a tier 1 to 3 source, and the log says which one.
A value that cannot be re-derived stays provisional.

## Name of the process

The process is documented independent derivation.
Its parts are a cited source for every constant, a log of every consultation, a checklist at review, and a denylist scan in the gate.
It is not clean-room, which would need separated teams and a record that nobody crossed the line.

## AI assistance

An assistant works from the cited documents, the files of this repository and test results.
It is not asked to reproduce another emulator.
A constant, table or name that the cited sources do not explain is suspect.
Such an item is re-derived from a cited source, or it is removed, before merge.
Commits made with an assistant say so in a co-author trailer.

## Review checklist

The same list is in `.github/pull_request_template.md`.

- Every constant or table carries a `Source:` cite.
- Every consultation of an emulator is logged in `docs/provenance/CONSULTATIONS.md`.
- No emulator identifier appears in code, data or docs.
- Every binary is listed in a fixture manifest.
- Any flip of `redistributable` to yes has had an explicit review.
- Nothing under `refs/` is tracked.
- One or two constants per pull request are spot-checked against the cited page.

## How this is checked

The `denylist` step of `sh tools/gate.sh` scans every tracked text file.
Its scope is everything except `.planning/`, `.claude/`, `docs/provenance.md`, `docs/provenance/`, `data/sources.json` and `tools/check/`.
Those paths are excluded because they must name the things the scan looks for: the rule itself, the consultation log, the registry URLs, the lists and the control inputs.
`.claude/` is excluded because `.claude/CLAUDE.md` is a generated mirror of research text under `.planning/`, which is already excluded.
Author names and licence-header strings in `tools/check/denylist.txt` fail the scan wherever they appear in scope, and they cannot be waived.
Emulator project names in `tools/check/denylist-cite.txt` fail the scan unless the same line carries the marker `[cite-only]`.
Use that marker only where a name must stay, for example in a quoted title of a cited document.
Otherwise cite the source by its `SRC-<slug>` id.
The findings name the file and never print the matched text.
The `files` and `manifests` steps keep rights-bearing binaries out of the tree, as described in `docs/RIGHTS.md` and `docs/fixture-manifest.md`.
The `sources` step checks `data/sources.json` and requires every `SRC-<slug>` token in `data/`, `docs/` and `README.md` to resolve to a registry entry.

## Honest limits

These checks catch accidents, not authorship.
A renamed identifier or a reworded comment evades the denylist.
Duplicate JSON keys cannot be detected, because the parser keeps the last one.
Whether a tag such as tier 1 is honest is a review question that no scan answers.
Whether a page cite matches the cited page is also a review question.
The blind second reader, who checks a cite without seeing the claim first, mitigates the page-cite risk.
Whether the no-copy rule was followed is confirmed only by review against the consultation log and the checklist above.
This is recorded as a residual risk.

## What this file does not do

This file is not legal advice.
It does not grant any right in a third-party document, and a hash in the registry identifies a copy without granting rights.
It keeps this project's own work clean under its own licence, as set out in `docs/RIGHTS.md`.

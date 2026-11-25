# Rights policy

This repository is licensed under the MIT licence, and its GitHub repository is public.
Everything tracked here is therefore published and redistributed.
This page says what may never be tracked, what may, and how the gate catches an accident.
A hash proves identity, not rights.

## Never committed

- EXEC and GROM images, in whole or in part.
- Game ROMs and cartridge images of any kind.
- Diagnostic ROMs without documented permission to redistribute them.
- Manuals, datasheets and their scans.
- Rendered page images of a scan, which are derivative of a rights-bearing document.
- Controller overlays and other scanned artwork.
- Bulk hash databases taken from other projects.
- Credentials, tokens, `.env` files and absolute private paths.

## Allowed

- Material authored for this project, under the project licence.
- A single identity hash of a system image, recorded as a fact.
- Short quotations from a cited source.
  Otherwise paraphrase and cite the source.

## Questions this milestone touches

Each row below is one question about third-party content, recorded as a ledger row in `data/rights/`.
A row states a disposition of `cleared`, `refused` or `owner-local-only`, what was taken, and the spellings the item must never be named as.
The `manifests` step refuses a row that has lost its disposition or its "what was taken" cell, a `cleared` row whose source id does not resolve in `data/sources.json`, a row the inventory in `data/expected/rights.ids` does not list, and a row this table does not name.
It also refuses, by name, any tracked file under `data/rights/` whose name is not a row slug: the file selection and the row grammar are read in both directions, so a misnamed row cannot hide its refused spellings by falling outside the selection.
A disposition is a decision about naming and shipping, not a finding about behaviour: a `cleared` row says a thing may be named, never that anything about it is true.

| Item | Disposition | What that means |
|------|-------------|-----------------|
| the frontend ABI subset | `cleared` | may be named; nothing is vendored, and the project writes its own entry points |
| a flat cartridge image | `owner-local-only` | may be read only from the owner's own files and never committed |
| the configuration container | `refused` | may not be named as a shipped artifact; the project does not parse or describe it |
| a bank-switched cartridge mode | `refused` | may not be named as a shipped capability; no such mode is implemented or described |
| the executive image | `owner-local-only` | may be read only from the owner's own files and never committed |
| the graphics image | `owner-local-only` | may be read only from the owner's own files and never committed |
| a commercial cartridge | `refused` | may not be committed, copied into a fixture, digested or named as a shipped artifact |
| a rendered page image | `refused` | may not be committed or named; a render of a rights-bearing page is derivative of it |
| a saved machine state | `owner-local-only` | may be read only from the owner's own files and never committed as a golden, a fixture or a claim artifact |
| the core information strings | `owner-local-only` | may be read only from the owner's own build output and never committed |

## The refs/ cache

Reference documents, page renders and downloads live only in the local `refs/` directory.
Files there are named `refs/SRC-<slug>.<ext>`, where `<slug>` stands for the source id.
The directory is gitignored, is never cached or uploaded by CI, and is never tracked.
The registry in `data/sources.json` records each source with `rights: unknown`, a digest and a local path, so a developer can re-fetch and confirm identity.

## Fixtures

Every fixture has a manifest that states its source, licence evidence, rights review and allowed use.
The schema and the effective-redistributable rule are in `docs/fixture-manifest.md`.
Bytes are committed only for a manifest whose effective value is `yes`.

## Names

The word "Intellivision" is never part of a package, product or repository name.
It appears only to say which machine the emulator targets.
The non-affiliation notice lives in the README.

## How this is checked

- The `manifests` step reads every `fixtures/<id>/manifest.json` and fails closed on a missing or misspelled key, an unproven `yes`, committed bytes without effective `yes`, or a digest that does not match.
- The `files` step fails on any tracked path under `refs/`, any tracked `.env` file, any tracked file that is binary by name (`tools/check/binary-ext.txt`) or by content (a NUL byte, or mostly non-text bytes) and is not a committed output of an effective `yes` manifest, and any unlisted file inside a fixture directory.
- The same step refuses a ledger row's refused spellings inside four scopes and nowhere else: `core/src/`, `runner/`, `tools/` (the control overlays under `tools/check/controls/` are exempt, or the tree would refuse its own test data) and six pinned operator pages, which are `README.md`, `docs/running.md`, `docs/owner-images.md`, `docs/packaging.md`, `docs/diagnostics.md` and this page.
  The list of pages is written in the check, not a directory glob, so naming a further page is a deliberate diff.
- A spelling that was already there when the rule landed is suppressed only by a `(token, path)` pair in `data/expected/rights-pinned.ids`, written as two whitespace-separated fields: a tab or a run of spaces is a separator, in both directions.
  The list is read in both directions: an occurrence with no pair is refused, and a pair that matches nothing is refused too, so the list cannot rot into a wildcard and deleting a line is a behaviour change rather than a tidy-up.
  The four pairs recorded today are the operator's own argument placeholders in `tools/owner-run.sh` and `docs/owner-images.md`: an operator is told to supply their own images under exactly those placeholders, so renaming them would break the instruction without refusing anything.
- Each failure class has a negative control that must fail for its intended reason.
- The checks compare recorded identities and file names.
  They cannot decide whether the author of a licence statement had the authority to give it, and a binary that is deliberately made to look like text (no NUL byte, mostly printable bytes, or a text extension from `tools/check/text-ext.txt` and no NUL byte) is not detected.
  A committed output of an effective `yes` manifest that has no extension in the binary list still fails the NUL rule of the `integrity` step, so give such outputs a binary extension.

## If something gets in

- Remove it going forward with a normal commit and record what happened.
- Escalate to the owner, because a rights decision is not the agent's to make.
- Never force-push `main` or rewrite published history; that is an irreversible action reserved to the owner under the autonomy policy.

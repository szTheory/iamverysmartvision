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
- Each failure class has a negative control that must fail for its intended reason.
- The checks compare recorded identities and file names.
  They cannot decide whether the author of a licence statement had the authority to give it, and a binary that is deliberately made to look like text (no NUL byte, mostly printable bytes, or a text extension from `tools/check/text-ext.txt` and no NUL byte) is not detected.
  A committed output of an effective `yes` manifest that has no extension in the binary list still fails the NUL rule of the `integrity` step, so give such outputs a binary extension.

## If something gets in

- Remove it going forward with a normal commit and record what happened.
- Escalate to the owner, because a rights decision is not the agent's to make.
- Never force-push `main` or rewrite published history; that is an irreversible action reserved to the owner under the autonomy policy.

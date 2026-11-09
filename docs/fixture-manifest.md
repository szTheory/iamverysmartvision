# Fixture manifest

Every fixture, reference input or derived output has one record at `fixtures/<id>/manifest.json`.
The record states where the bytes came from, who may redistribute them and what the project may do with them.
A hash proves identity, not rights.
The `manifests` gate step reads these records and fails closed: anything missing, misspelled or unproven is treated as not redistributable.

## Schema version 1

All keys below are required at the top level, and no other key is accepted.
A key that has no value is written as `null` (where the type says so), an empty array or an empty string, never omitted.
The file is ASCII JSON with LF line endings.

| Key | Type | Meaning |
|---|---|---|
| `schema_version` | number | Always `1`. |
| `id` | string | `FX-<slug>`, equal to the directory name `fixtures/<id>/`. |
| `kind` | string | One of `original-source`, `original-built`, `third-party-test`, `system-image`, `third-party-doc`, `derived-output`. |
| `committed` | boolean | `true` only when the bytes are tracked in this repository. |
| `path` | string or null | In-repo path of the primary output when `committed` is `true` (it must be listed in `outputs`), otherwise `null`. |
| `local_path` | string or null | Gitignored local path under `refs/` where a developer may keep the bytes, or `null`. |
| `source` | object | `origin` (string), `pin`, `accessed`, `holder` (string or null; `accessed` is `YYYY-MM-DD`). |
| `recipe` | object or null | How the bytes are built: `tool`, `tool_version`, `tool_sha256`, `tool_licence`, `command`, `sources[{path, sha256}]`. `null` for material that is not built. |
| `inputs` | array of string | Free-text references to inputs (paths or ids); may be empty. |
| `outputs` | array of object | `{path, sha256, size_bytes}` for every file the entry accounts for; empty when no bytes are tracked. |
| `licence` | string | An SPDX expression, a `LicenseRef-*` identifier, or `NOASSERTION`. The checker validates the character set only. |
| `licence_evidence` | object or null | `{url, text_sha256, accessed}`: where the licence text was read and its digest. `null` when no evidence exists. |
| `copyright` | array of string | Copyright lines as found at the source; may be empty. |
| `notices` | array of object | `{path, sha256}` for every notice file that must travel with the bytes. |
| `redistributable` | string | Exactly `yes`, `no` or `unknown`. |
| `allowed_use` | array of string | A subset of `local-test`, `ci`, `repo`, `release`. |
| `rights_review` | object | `{status, by, date}` where `status` is `owner-confirmed`, `author-attested` or `unreviewed`. |
| `derived_from` | array of string | Ids of the manifests this entry was derived from. |
| `note` | string | Free text for reviewers. |

Inside `source`, `recipe`, `licence_evidence`, `rights_review`, `outputs` and `notices`, every listed key is required as well and no other key is accepted.
Digests are 64 lowercase hexadecimal characters.
Paths are repository-relative, use only letters, digits, `.`, `_`, `-` and `/`, never start with `/` and never contain a `..` segment.
The set of ids found on disk must equal `data/expected/fixtures.ids`.

## Effective redistributable

The recorded `redistributable` value is a claim.
The effective value is computed, and only the effective value counts.
It is `yes` only when all of these hold.

1. `redistributable` is `yes`.
2. `licence_evidence` is present, with a non-blank `url` and a `text_sha256` that is a 64-character digest.
3. `rights_review.status` is `owner-confirmed` or `author-attested`, not `unreviewed`.
4. Every id in `derived_from` names a manifest that is itself effective `yes`.

Anything else is `no`: a value of `no` or `unknown`, a missing or misspelled field, an unknown `derived_from` id, or a cycle among `derived_from` entries.
The least permissive input wins, so a derived output can never be more redistributable than the material it came from.
A record that says `yes` but fails the computed rule is reported as `manifest-yes-without-evidence`.

## Rules

- `committed: true`, or an `allowed_use` containing `repo` or `release`, requires effective `yes` (`manifest-committed-not-redistributable`).
- A committed manifest must list at least one output, and every listed output must exist with the recorded `size_bytes` and `sha256` (`manifest-missing-output`, `manifest-digest-mismatch`).
- Outputs of an uncommitted manifest must not be tracked (`manifest-uncommitted-output-tracked`).
- Flipping `redistributable` to `yes` needs licence evidence, a rights review that is not `unreviewed`, and an explicit reviewer.
- The `files` gate step additionally refuses any tracked file under `refs/`, any tracked `.env` file, any tracked ROM, PDF or image-like file that is not a committed output of an effective `yes` manifest, and any file inside a fixture directory that its manifest does not list.

## Sample entry

`fixtures/FX-sample-rights-unknown/manifest.json` is an illustrative entry with `redistributable: unknown`, `committed: false` and `allowed_use: ["local-test"]`.
No bytes exist for it.
The gate accepts it and counts it as effective `no`.
The negative control `manifests-unknown-committed` flips the same entry to `committed: true` and must fail with `manifest-committed-not-redistributable`.

## What the checker cannot decide

- A matching digest shows that the bytes are the bytes the manifest describes.
  It does not show that the person who wrote `licence_evidence` had the authority to grant it.
- The `licence` string is not checked against the SPDX list.
- Duplicate JSON keys inside one record are not detectable with the JSON reader the checker uses.
- `rights_review` records who reviewed and when; the checker cannot judge whether the review was sound.

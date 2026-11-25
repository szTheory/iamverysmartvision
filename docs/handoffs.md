# Handoff packet

This page is the packet for the human-only items of `data/expected/handoffs.ids`: for every row of that ledger it gives the exact command, the artefact path and the deterministic identities (byte size, file mode, the one digest line) that a person would compare, so the owner can perform the item without rebuilding the procedure from a phase record.
It is not a check, not a gate step, not evidence and not citable by a claim: no gate step opens this page, no test reads it, and no `data/claims.json` record points at it.
The ledger is the inventory; this page is only the instruction sheet for it.

## The one rule to carry away

An answer of yes or no is a handoff **result**.
It is recorded in the ledger as `evidence: none` either way, it never becomes a claim and it never carries a tier.
No row records an act that a human has performed: `handed-off` records that the packet below exists and the item is offered, and it is not a result; `not-performed` records that the item needs hardware or owner-local images that no tool on this host can supply.
A row stays `not-performed` unless a human reports back, and even then its last two tokens stay the literal words `evidence: none`.

## H7-1: the exported audio of the pass scenario

From the repository root:

```
sh tools/listen-demo.sh demo-lamp-tone-pass
```

The helper builds the release preset, renders the scenario through the runner's `--audio-wav` option and prints the path, the size and the digest line below.
It refuses, before building anything, a scenario whose text carries a `content ... user` line, because derived audio of rights-bearing content stays out of shared records.

- Artefact: `build/listen/demo-lamp-tone-pass.wav`, **21674 bytes**, mode `0644`.
- The directory `build/` is ignored: the file is never committed, and `wav` is a binary extension that the files step refuses, so a `.wav` cannot enter the tree by accident.
- Compare this line, printed by the helper and recorded in `build/listen/demo-lamp-tone-pass.report`:

```
refpcm.sha256 e3b6638887947d64f7fa7d2be5687aff0eca85981b0ea79cfcfd3c74d057f1ad
```

That digest is the deterministic identity of the file: the SHA-256 of the WAV as it lies on disk, produced by the reference converter (label `reference conversion v1, converter repeatability only`), and the same command on any host prints the same line.
The converter is an aliasing reference, not chip truth and not a high-fidelity renderer.
What a person judges is whether the tone is the one the program intends — a judgement of what they hear, not a measurement, and not a statement about a real console.

## H7-2: the exported frame of the pass scenario

```
build/release/runner/ivs-run --content-dir build/release/images --frame-ppm build/h7-2-pass.ppm data/scenarios/demo-lamp-tone-pass.ivss
```

The last argument is the scenario; the `--frame-ppm` path is the operator's own choice and must not exist yet (the file is created exclusively and an existing path is refused, exit 14, with no report).
`build/release/images` is what `cmake --preset release` and `cmake --build --preset release --target ivs_images` produce; `sh tools/listen-demo.sh demo-lamp-tone-pass` builds both targets first if they are missing.

- Expected: exit 0, a report whose last line is `report.end 130` for this scenario, and a file of **45806 bytes** with mode `0600` whose header is `P6`, then `159 96`, then `255` — a binary PPM of the last delivered frame, 159 by 96 pixels.
- Compare this line, recorded in the committed golden `data/golden/demo-lamp-tone-pass.expect`:

```
assert frame 12 hash == a7b9889b136cf47b2586def7f992c7caaf19c451e043a9a676b2f838bd91c2d7
```

`--frame-ppm` adds no report key of its own, so that golden line is the only deterministic identity of the picture and it is what ties the exported file to the frame the project already pins.
The preview palette is arbitrary: sixteen project-chosen colours plus one for a blank position, **operator preview palette, not STIC colour data**.
What a person judges is whether the picture is the one the program intends; the colours in the file are not STIC colour data, and a judgement about them is not a statement about the chip.

## H8-3: the Phase 8 close record's id for the same two acts

H8-3 is not a third act.
The Phase 8 close record (`.planning/milestones/v0.1.0-phases/08-stic-completion-save-state-performance-baselines/08-MILESTONE-CLOSE.md`, section K) uses it for the same two acts as H7-1 and H7-2 plus a look at the STIC pictures of a `diag-stic2-` scenario, so one sitting discharges H8-3 and both Phase 7 rows; a later milestone must not book a third act on the strength of the row count.
The picture limb, for a person who wants those extra frames:

```
build/release/runner/ivs-run --content-dir build/release/images --frame-ppm build/h8-3-stic.ppm data/scenarios/diag-stic2-mob-pass.ivss
```

- Expected: exit 0, `report.end 81`, a file of **45806 bytes** with mode `0600` and the same `P6` / `159 96` / `255` header.
- No golden pins this picture, so the size, the mode and the header are its whole deterministic identity; the pixels are what a person judges.

## D10: the physical measurements

The `D10-*` items are held in `data/expected/d10-measurements.ids`, one line per item, `<id> | <discrepancy id> | <what the owner records> | <anchor>`; this page cites that file by path and never restates how many rows it holds, because the row count is the inventory's business and a quoted count is a figure that goes stale.
The procedure is `docs/input/d10-procedure.md`, and `docs/stic/d10-procedure.md` for the `D10-VID-*` items and for `D11-EXEC-03`; the anchor in the fourth column names the section of those documents that the item belongs to.
The third column says what the owner records, and a recorded measurement is a fact about one console under the conditions the procedure asks for.
These items need physical hardware, so no tool on this host can perform them and the ledger row is `not-performed`.

## D11: the owner-local system image

The procedure is `docs/owner-images.md` and the helper is `tools/owner-run.sh`.
The images live in the ignored `refs/` directory (`refs/FX-owner-exec.bin` and `refs/FX-owner-grom.bin`); they are never committed, cached, uploaded or loaded by CI, and a tracked path under `refs/` fails the files step.
On a host with no operator image the helper prints exactly

```
SKIPPED owner-run: no operator images (D11)
```

and exits `77`.
That is the state of this host, and it is why the ledger row is `not-performed` rather than a gap in the inventory: the item is named, the procedure exists, and no image exists here to read.
The owner-local invocation, once the images and the digest lock are in place, is `sh tools/owner-run.sh refs/FX-owner-exec.bin refs/FX-owner-grom.bin`.

## When a person does report back

The row's disposition changes and the note names what was judged, the row still carries `evidence: none`, and a claim that wants to rest on the result must be raised by a separate decision — never by editing this packet and never by editing a row's last two tokens.
A reported result is a fact about one person's judgement on one day; the project records it as such and does not promote it to a tier.

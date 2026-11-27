# Handoff packet

This page is the packet for the human-only items of `data/expected/handoffs.ids`: for every row of that ledger it gives the exact command, the artefact path and the deterministic identities (byte size, file mode, the one digest line) that a person would compare, so the owner can perform the item without rebuilding the procedure from a phase record.
It is not a check, not a gate step, not evidence and not citable by a claim: no gate step opens this page, no test reads it, and no `data/claims.json` record points at it.
The ledger is the inventory; this page is only the instruction sheet for it.
Phase 16 added three rows to that inventory — `H16-1`, `H16-2` and `H16-3` — and their sections below are this page's newest ones; the page states no count of rows anywhere, because the ledger is the inventory and the claims step prints that count as `handoffs=<n>` from the file itself.

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

## D10-VID-08: the display strobe and the latch, at swept offsets

This section is the packet for the ledger row `D10-VID-08 not-performed evidence: none`.
Its procedure is `docs/stic/d10-procedure.md` `### stic-strobe`, with `### stic-window-probes` beside it for the gating candidate's window edges, and the row it belongs to in `data/expected/d10-measurements.ids` carries the anchor `#stic-strobe`.
It is a row and not a gate: no criterion of any phase requires its answer, no claim cites it, and an answer recorded here would still carry `evidence: none`.

### The record fields a capture is compared field for field

The names below are the frozen page's own, quoted from `docs/inspection.md` `## The trace file` and from nowhere else, so a capture recorded in the `## Record format` shape of the procedure can be laid beside them field for field.

- An access record, one per bus access: `tick`, `kind`, `addr`, `value`, `wait`, `cycle`, `unit_start`, `unit_end`.
- A step record, one per run-to: `step`, `target`, `end`, `stop`.
- The end record, last in the file: `tick`, `records`, `units`, `steps`, `break_pc`, `break_hits`, `other_pc`, `other_hits`.

### The comparison list

One row per guest. The third column is the trace line that separates the two candidate values, the fourth is the observable line with its shipped value and its rival value, and the fifth is the guest's alternate-value mask as `stic.disc.sweep-diagonal` measured it.

| guest | the field it separates | the trace lines that carry it | the observable line, shipped then rival | the mask |
|---|---|---|---|---|
| `G1` | `stic.reg_window_cpu_cycles` `2000` then `2900` CPU cycles | one `write` record at `addr=0020` whose `tick=` lies strictly inside `8000 < tick < 11600` - 2000 and 2900 CPU cycles at four ticks per cycle - with no earlier strobe in the invocation, plus the `end` record's `tick=`, `records=` and `units=` | registered observable `blocked_writes` `1` then `0` (machine state, with no report line at all, which the packet says plainly); witness `frame 0 displayed` `no` then `yes` | `0xAAAA` |
| `G2` | `stic.access_gating_when_off` `apply` then the windows open while the display is off | one `write` record at `addr=0020` whose `tick=` lies strictly inside `11600 < tick < 15120` - 2900 and 3780 CPU cycles at four ticks per cycle - made with the display latch clear across the frame boundary, plus the `end` record's `tick=`, `records=` and `units=` | registered observable `blocked_writes` `1` then `0`; witness `frame 0 displayed` `no` then `yes` | `0xCCCC` |
| `G3` | the `$0020` READ row of `stic.access_effects`, `none` then `arm` | one `read` record at `addr=0020` and no `write` at that address in the same frame | registered observable `frame 0 displayed` `no` then `yes` | `0xF0F0` |
| `G4` | `stic.display_latch_cleared_each_frame` `1` then `0` | exactly one `write` record at `addr=0020` in the whole invocation | registered observable `frame 0 displayed` `yes` then `yes`, `frame 1 displayed` `no` then `yes`, `frame 2 displayed` `no` then `yes` | `0xFF00` |

### Why the frame value alone does not separate the four

Phase 19 measured that the single-strobe reading of the frame-0 word moves under the gating field as well; its committed stand-in `stic.judge.disc-diagonal-two-fields` rejects that reading at cell `0x2`, bits `0x2`, observed `1`.
A capture must therefore record the access's stamp and the display latch's state beside the frame value: the separation is the pair (stamp, registered observable), never the frame value alone.
The consequence is the first thing a reader will notice, so it is said here: `G1` and `G2` carry the same witness pair, for the reason that one guest writes `$0020` once and the frame word reads the same under both rules, and what separates those two rules is the **stamp band** - `8000 < tick < 11600` for `G1` and `11600 < tick < 15120` for `G2` - together with the display latch's state, so a capture that records no stamp cannot separate them at all.
`G3`'s and `G4`'s registered observables are the frame words themselves, which is why those two rows need no separate witness.
The masks above were measured by `stic.disc.sweep-diagonal` over its sixteen stack-copied cells, so the table is traceable to a committed measurement rather than asserted.

### Status

Status: `G1` `0xAAAA`, `G2` `0xCCCC`, `G3` `0xF0F0` and `G4` `0xFF00` are the four rival candidates; each guest's two values are measured at the default of the other three fields, so every row is a comparison a capture is laid against and none of them is an answer. The row stays `not-performed` because no hardware probe has produced an answer.

### What this packet does not carry

It names trace lines and profile keys only. It carries no digest, no size, no path and no name of any file: a hash establishes identity and not rights, and this row is not about anyone's file.

### No claim cites this row

A claim may not cite this row or the ledger that holds it, and the mechanism is the claim-artifact predicate's refusal of `data/expected/handoffs.ids`, which is demonstrated rather than promised: the committed stand-in `tools/check/controls/claims-handoff-artifact` is run in plan 20-02 and prints `-- FAIL claim-handoff-artifact: CL-0005` and no `claim-dangling-path` for that id, and a copy whose `CL-0005` is repointed to another existing path stops being refused while the other rows are still refused.

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

## H16-1: a real frontend loading the build-only shared object

The object is build-only: it has no install rule, it is never installed and never shipped, and it is not one of the installed package's paths (`docs/packaging.md`).
From the repository root, with the release preset:

```
cmake --preset release
cmake --build --preset release --target ivs_retro_shim
```

The target is `ivs_retro_shim` and its output name is `ivs_retro`, so the artefact is the shared object that target writes in the preset's own build tree: `build/release/host/libivs_retro.dylib` on this host, `build/release/host/libivs_retro.so` on Linux.

- Compare before and after the frontend is pointed at it, because a rebuild is a different object and its identity says so:

```
cmake -E sha256sum build/release/host/libivs_retro.dylib
ls -l build/release/host/libivs_retro.dylib
```

- The deterministic identity is that digest and that byte size, and nothing about the object is a statement about any frontend.
- The project's own reproducible statement about the object is the nine digest lines its own fake frontend prints. That frontend is `tests/host/ivs_host_harness.c`, whose usage line is the command, and the release preset's object, content directory and no mode flag are all it needs:

```
build/release/tests/ivs_host_harness build/release/host/libivs_retro.dylib build/release/images
```

Those nine lines are what a person with a real frontend can hold their frontend's own answer against; they are the project's answer through the object's entries, not a statement about anyone else's frontend.
What a person judges is whether their frontend accepts the object and presents something from it.
The project states nothing about any frontend: it has no frontend of its own, it offers no compatibility toward any other project's frontend ABI, and a frontend that refuses the object is a fact about that frontend.

## H16-2: an owner image's picture and its sound

The procedure is the three-command route of `docs/owner-images.md` ("The route from the source archive, in three commands"), and its rehearsal transcript is `.planning/ops/RELEASE-OPS.md` section (j), cited by path and section letter rather than restated here.
The route loads the owner's EXEC image, GROM image and wrapped cartridge container through `tools/owner-run.sh`; the images stay in the ignored `refs/` directory and are never committed.

- The deterministic identity of such a run is that run's own report: its loader lines and its outcome, read on the owner's machine.
- It is not any picture or sample file: for a scenario with a `content <role> user` line the runner refuses the frame, video, golden, audio and psg-log output options, so the route writes no file of what the images caused, and no derived file of owner content may be written into the repository.
- The picture and the sound can only be judged through a frontend that presents them, which is `H16-1`'s act: the object plus the owner's images in a frontend the owner chooses.

What a person judges is whether the picture is the one their program intends and whether the sound is the one it intends.
The project states nothing about either: it names no digest of any owner image, it says no real EXEC, GROM or commercial cartridge is known to behave in any particular way on this core, and a judgement of a picture or a sound is a judgement about one person's observation on one day.

## H16-3: the word order of the owner's flat image

The procedure is the wrap command of that same route, run twice into two containers, once at its default `hi-first` and once with `lo-first`, followed by loading each container and comparing the loader lines and the outcome.
The base address is a required input that the owner states as four hex digits without a prefix, `$4000` to `$FFFF`, and nothing is auto-detected: the tool takes the base and the word order from the command line and reads no other format.

```
build/release/tools/flatwrap/ivs_flatwrap refs/FX-owner-flat.bin 4000 refs/FX-owner-cart.ivsc
build/release/tools/flatwrap/ivs_flatwrap refs/FX-owner-flat.bin 4000 refs/FX-owner-cart-lo.ivsc --word-order lo-first
sh tools/owner-run.sh --cart refs/FX-owner-cart.ivsc refs/FX-owner-exec.bin refs/FX-owner-grom.bin
sh tools/owner-run.sh --cart refs/FX-owner-cart-lo.ivsc refs/FX-owner-exec.bin refs/FX-owner-grom.bin
```

Each wrap prints the digest of the container it wrote, and each run publishes the container's own digest as `slot.cart.sha256`, so the container a run loaded is the container the tool wrote.
What a person judges is which of the two orders makes their image mean what they expect — a judgement of their own dump, not a measurement.
The project makes no statement about any real dump's byte layout: it does not know whether a given flat file matches this container without conversion, and this page does not say that it does.

## These rows are never a result

A row is never a pass, never evidence and never a claim's support: an answer of yes or no is a handoff result, recorded as `evidence: none` either way, and these three change no exit criterion of any phase.
They are offered, not performed, and a later milestone opens this packet and cites the row instead of asking the owner the same question again.

## When a person does report back

The row's disposition changes and the note names what was judged, the row still carries `evidence: none`, and a claim that wants to rest on the result must be raised by a separate decision — never by editing this packet and never by editing a row's last two tokens.
A reported result is a fact about one person's judgement on one day; the project records it as such and does not promote it to a tier.

# IVSN v1 snapshot container

This note specifies IVSN v1, the container in which the machine core writes and reads one snapshot of a machine instance (STA-01, STA-02).
It is the project's own format, written for this project; it is not a copy or a subset of any other save-state or image format.
The file extension is `.ivsn` by convention only: the core never sees a file name, only bytes and a length, and it opens no file.
No snapshot is committed to the repository (see "Rights").

This document is the oracle of plan 08-09: it is written before any serializer exists, and the code of plans 08-10 to 08-13 is held to the numbers below.
Every number was derived on paper from the text of the digest field table (`core/src/machine/machine_digest.c`) and the struct headers, then checked by a scratch recount over `data/state/schema.literal.txt`.
A label `[derived]` marks such a number, `[planned]` a decision that later code implements, and `[candidate]` a rule that is not yet proved on reachable states.
Pins of these numbers live in `data/expected/ivsn-literals.ids`; they are repeatability pins of text written by hand, never evidence that a snapshot restores a machine correctly. [nc]

## Conventions

Every multi-byte field is big-endian: the most significant byte comes first.
This is the one wire byte order of the project, shared with the cartridge container, the system-image container and the state digest serialization (D-12).
The bytes are assembled field by field, so the container does not depend on the byte order of the host, on the padding of a struct or on the layout of the instance.
Offsets and sizes are in bytes unless a name says otherwise; every offset is counted from the first byte of the container.
A row is a row of the digest field table; an element is one value of a row (a row with a count above 1 is an array of elements).

A snapshot is exact-build state, not an archival format (D-13): it is meant to be restored into an instance of the same build and the same profile, and it makes no statement about any other build.
The project is before 1.0; there is no migration code and no promise that a later build reads an earlier snapshot.
The core is policy-free: bytes in, bytes out; it chooses no name, path, slot, location, compression, encryption or autosave (D-19).

## Classes

Every row of the table has exactly one of four classes (D-11).
The class is the whole rule for what a snapshot does with the row; no row is without a class, and an array row has one class for all its elements.

<!-- classes-begin -->
- **STATE**: a member of the instance that some later step can read, or that changes something the instance delivers (a stream, a report, a digest). A STATE row is written into the section of its domain and is restored byte for byte.
- **IMAGE_BEARING**: a member that holds the contents of a loaded system or cartridge image. It is read-only after the load, it is never written into a snapshot (rights) and a restore never writes it.
- **BOUND**: a member that records the identity of the instance (handle validity, the registry index of the profile) or of what has been loaded into it (the structure of the loaded images and the three slot records). It is never written into a snapshot, a restore checks it against the live target and never writes it.
- **PAD**: a reserved word that carries no meaning. It is never written into a snapshot and a restore leaves zero in every one.
<!-- classes-end -->

The class of each of the 88 rows is the second column of `data/expected/state-checklist.ids` and of `data/state/schema.literal.txt`; the counts are STATE 51, IMAGE_BEARING 3, BOUND 28 and PAD 6 [derived].
The handle magic (`magic`) is BOUND and not PAD: a restore zeroes the PAD rows and must never touch the magic of the handle, which is what makes the handle valid.
The reasoning per row (why a row can hide, its witness, why a zero target would not show it) is in `docs/state/hidden-state-checklist.md`.

## Layout

| Offset | Size | Field | Rule |
|---|---|---|---|
| 0 | 8 | magic | exactly `89 49 56 53 4E 0D 0A 1A`; the high-bit first byte, the CR LF and the SUB catch 7-bit and text-mode transfers |
| 8 | 2 | format version | 1 |
| 10 | 2 | flags | 0; any bit set is refused |
| 12 | 4 | total_bytes | the size of the whole container, trailer included; it must equal the length given and the size this build derives (4,299 for the first profile) |
| 16 | 4 | schema version | 1 (`IVS_SNAPSHOT_SCHEMA_VERSION`, bumped on a deliberate change of the state rows) |
| 20 | 4 | section count | 7 |
| 24 | 32 | schema digest | SHA-256 of the exact bytes of `data/state/schema.literal.txt` (see "Schema digest") |
| 56 | 64 | profile id | the profile id string, NUL padded; at most 63 bytes before the first NUL; every byte after the first NUL is zero |
| 120 | 32 | profile parameter digest | see "Profile identity" |
| 152 | 3 x 40 | image slot records | slots in the order exec, grom, cartridge; one record: u32 loaded (0 or 1), 32 bytes SHA-256 of the container bytes supplied to the load, u32 length_bytes (diagnostic only) |
| 272 | 32 | composite digest | the machine-level composite digest at save time; informational (see "Digests") |
| 304 | 7 x 44 | section table | one entry per section, ascending by domain: u8 domain, 3 bytes zero, u32 offset, u32 length, 32 bytes full-domain digest (informational) |
| 612 | 3,655 | sections | the seven sections in table order, adjacent, no gap, no overlap |
| 4,267 | 32 | trailer | SHA-256 of every byte before it |

The header is 612 bytes [derived]: 24 bytes of fixed fields, 32 + 64 + 32 for the digests and the profile, 120 for the three slot records, 32 for the composite digest and 308 for the seven section entries (24 + 32 + 64 + 32 + 120 + 32 + 308 = 612).
The seven sections are the domains 1 (CPU), 2 (MEM), 4 (INSTANCE), 16 (VIDEO), 17 (AUDIO), 18 (INPUT) and 19 (PSGLOG).
Domain 3 (IMAGES) holds no STATE row, so it has no section: the whole domain is identity, carried by the three slot records.
Note that the three image arrays sit in domain 2 of the table (MEM) but are class IMAGE_BEARING, so the filter is the class and never the domain.

## Sections

A section is the preimage of its domain with only the STATE rows: the four bytes `49 56 53 44` (the ASCII text IVSD), the domain as one byte, the domain version as a big-endian u16 (1 for every domain), then the STATE rows of the domain in table order [planned].
A row with a count above 1 is preceded by its count as a u32; the count must equal the count of the table row, because a snapshot never changes the shape of a row.
An element is 1, 2, 4 or 8 bytes wide as the table says and is written most significant byte first.
No PAD, BOUND or IMAGE_BEARING row is written, so a section holds no pad bytes at all and no byte of an image.
Because the snapshot payload bytes are the digest row encoding, one serializer gives both the digests and the snapshot (D-11).

## Size derivation

The 7-byte frame of every section is the text IVSD (4 bytes), the domain (1) and the domain version (2).
An array row costs 4 bytes of count plus count times width; a scalar row costs its width.

| Domain | Section | Rows (bytes) | Frame | Length |
|---|---|---|---|---|
| 1 | CPU | cpu.r 4 + 8 x 2 = 20; cpu.status, intff, dbd, prev_interruptible, run_state, halt_pending, fault_reason, busak 8 x 1 = 8 | 7 | 35 |
| 2 | MEM | gate_open 4 + 4 = 8; sysram 4 + 512 x 2 = 1,028; scratchpad 4 + 256 x 2 = 516; gram 1,028; psg_reg 4 + 16 x 2 = 36; port_pulldown 4 + 2 = 6; stic_reg 4 + 64 x 2 = 132 (sum 2,754) | 7 | 2,761 |
| 4 | INSTANCE | phase 4; time_ticks 8; the 10 PSG rows 8 + 16 + 16 + 7 x 4 = 68; the 11 STIC rows 8 + 9 x 4 + (4 + 240 x 2) = 44 + 484 = 528 (sum 608) | 7 | 615 |
| 16 | VIDEO | obs.video.chain 4 + 32 = 36; obs.video.count 8 | 7 | 51 |
| 17 | AUDIO | obs.audio.chain 36; count 8; emitted 8; prev_levels 4 | 7 | 63 |
| 18 | INPUT | obs.input.words 4 + 4 x 4 = 20; count 8; floor_tick 8; chain 36 | 7 | 79 |
| 19 | PSGLOG | obs.psglog.chain 36; count 8 | 7 | 51 |

The sections sum to 3,655 bytes (35 + 2,761 + 615 + 51 + 63 + 79 + 51) [derived].
The total is the header 612, the sections 3,655 and the trailer 32: **4,299 bytes** [derived].
These figures equal the research estimate of the phase (08-RESEARCH 2.1); no difference was found.
The absolute offsets are 612 (CPU), 647 (MEM), 3,408 (INSTANCE), 4,023 (VIDEO), 4,074 (AUDIO), 4,137 (INPUT) and 4,216 (PSGLOG), the trailer starts at 4,267 and the container ends at 4,299.
The size is a constant of the profile and the build; the call that reports it exists so that a host never hard-codes it.
`sysram` serializes its full 512-word backing although the extent is 352 words, and `scratchpad` its 256-word backing for a 240-word extent: the unreachable tails are part of the row, and the whole row is swept as one (see the checklist).

## Digests

The 32-byte digest of a section table entry is the machine-level digest of the whole domain, the value that `ivs_machine_state_digest` gives for that domain at save time; for the MEM domain it covers the image rows as well, so it is an identity of what was loaded (a hash establishes identity, not rights).
The composite digest at offset 272 is the machine-level composite at save time.
These eight digest fields (the composite and the seven domain digests) are informational (D-41): restore does not authenticate them, because it checks the structure, the trailer, the identity and the content instead.
A test compares the digests of the restored instance with the saved ones and fails on a difference.
The consequence is stated once, here: the canonical re-encode rule of the corruption family (a snapshot that is accepted re-encodes to exactly its own bytes) has these eight digest fields as its only exception, because a re-save recomputes them from the restored instance.
The trailer is a plain SHA-256 over every byte before it. It is integrity, not authenticity: it detects accidental damage, and a hostile snapshot with a recomputed trailer is refused by the validators, never by the checksum.

## Schema digest

The schema digest is the SHA-256 of the exact bytes of `data/state/schema.literal.txt` [planned].
That text is the canonical descriptor of the 88 rows: the first line `IVSSCHEMA 1`, then one line per row in table order, `<name> <width> <count> <domain> <kind> <class>`, with decimal numbers, the kind as UINT, PROFILE, MAGIC or PAD and the class as STATE, IMAGE_BEARING, BOUND or PAD.
Offsets are never part of it, because they depend on the host layout.
The writer produces the same text from the table descriptors; a state field added without a deliberate bump of the schema version changes the digest, and the pinned literal in `data/expected/ivsn-literals.ids` then breaks.
The gate step `state` recomputes the digest of the literal file and compares it with the pin, and compares the literal with the table text row by row.

## Profile identity

A snapshot names its profile twice: by the id string, and by a parameter digest, because the profile file has changed behaviour within one id in earlier phases and the id alone does not bind the parameters.
The parameter digest is the SHA-256 over the four bytes `IVSP` (the ASCII text), a big-endian u16 version 1 and four 32-byte body digests in this order: the memory map, the PSG, the STIC and the controller tables [planned].
Each body digest is the SHA-256 of the text that the generator of that table writes for the profile, never of comments, cites, tags or other provenance text, so an edit of a cite changes no digest; plan 08-11 makes each generator emit its constant.
Restore refuses a snapshot whose id is not registered, whose id differs from the instance's id, or whose parameter digest differs from the build's.

## Check order

Restore checks in this order; the first failing class wins, nothing is read beyond the length given, and nothing in the instance changes before the last check has passed [planned].

1. Arguments: a NULL buffer with a length above 0 (58), a report that overlaps the buffer (59), a buffer or report that overlaps the instance (81); then the handle (BAD_HANDLE).
2. Length: above the bound `IVS_SNAPSHOT_MAX_BYTES` (OVERSIZED, 60) before any byte is read; below the smallest possible container (61).
3. Header structure: magic (62), format version (63), flags (64), schema version (65), schema digest (66), total_bytes against the length and the derived size (67), section count (68).
4. Section table: known domains only, strictly ascending, offsets and lengths inside the payload, adjacent, summing to the payload (69).
5. Trailer: the SHA-256 over all earlier bytes (70). It sits after the structure and before anything that interprets content; version and schema come first because a later version may change the trailer.
6. Profile: the id form (71), the id registered (72), the id equal to the instance's (73), the parameter digest equal to the build's (74).
7. Images: for each slot the loaded flag and the SHA-256 equal the live slot (75 for a slot the snapshot has loaded and the target has not, 76 for a difference), then the live IMAGES domain digest equals the recorded structure.
8. Sections: every section length equals its derived length (80), every row count equals the table's (77), every decoded value meets its range and validator (78), the derived invariants hold (79).
9. Commit: the non-image, non-slot region is zeroed, then every decoded value is stored with copies that cannot fail; the magic, the profile index, the slots and the image arrays are untouched, and the phase becomes POWERED.

A refused restore leaves the whole instance byte-identical and writes only the report.

## Refusal reasons

The reasons are appended to the `IVS_REASON_*` list after 57; the numbers are frozen and append-only once pinned in `data/expected/api-reasons.ids`.
Each reason travels with one status. Plan 08-12 adds 58 to 71, plan 08-13 adds 72 to 81; the status `IVS_E_STATE_MISMATCH` (-24) is added by plan 08-11.

| Number | Reason | Status | Meaning |
|---|---|---|---|
| 58 | SNAP_BUF_NULL | BAD_ARG | a NULL buffer with a length above 0 |
| 59 | SNAP_REPORT_OVERLAP | BAD_ARG | the report overlaps the buffer |
| 60 | SNAP_OVERSIZED | OVERSIZED | the length is above the bound; refused before any byte is read |
| 61 | SNAP_TRUNCATED | MALFORMED | the length is below the smallest container, or total_bytes is above the length |
| 62 | SNAP_MAGIC | MALFORMED | the magic differs; the byte offset of the first difference is reported |
| 63 | SNAP_FORMAT_VERSION | UNSUPPORTED | a format version other than 1 |
| 64 | SNAP_FLAGS | UNSUPPORTED | a flag bit is set |
| 65 | SNAP_SCHEMA_VERSION | UNSUPPORTED | a schema version other than the build's |
| 66 | SNAP_SCHEMA_DIGEST | UNSUPPORTED | the schema digest differs from the build's |
| 67 | SNAP_TOTAL_LENGTH | MALFORMED | total_bytes differs from the length or from the derived size |
| 68 | SNAP_SECTION_COUNT | MALFORMED | a section count other than 7 |
| 69 | SNAP_SECTION_TABLE | MALFORMED | an unknown, duplicate or out-of-order domain, or an offset or length that does not fit |
| 70 | SNAP_TRAILER | DIGEST_MISMATCH | the trailer differs from the SHA-256 of the earlier bytes |
| 71 | SNAP_PROFILE_ID_FORM | MALFORMED | the profile id has no NUL, a byte after the NUL, or a byte that is not printable ASCII |
| 72 | SNAP_UNKNOWN_PROFILE | UNKNOWN_PROFILE | the id is not registered in this build |
| 73 | SNAP_PROFILE_MISMATCH | STATE_MISMATCH | the id is registered and differs from the instance's |
| 74 | SNAP_PROFILE_PARAMS | STATE_MISMATCH | the parameter digest differs from the build's |
| 75 | SNAP_SLOT_MISSING | MISSING_SYSTEM_IMAGE | the snapshot has a slot loaded that the target has not |
| 76 | SNAP_SLOT_MISMATCH | STATE_MISMATCH | a slot is loaded on both sides with another identity, or loaded only in the target |
| 77 | SNAP_ROW_COUNT | MALFORMED | a row count differs from the table's |
| 78 | SNAP_FIELD_RANGE | MALFORMED | a decoded value fails its range or its validator |
| 79 | SNAP_INVARIANT | MALFORMED | a derived invariant does not hold |
| 80 | SNAP_SECTION_LENGTH | MALFORMED | a section length differs from its derived length |
| 81 | SNAP_ARG_OVERLAP_INSTANCE | BAD_ARG | the buffer or the report overlaps the instance |

With one registered profile, a second registered id cannot be built through the public API; plan 08-13 tests the id comparison (73) as a pure seam function with a synthetic descriptor and says so.

## Derived invariants

A restore accepts a snapshot only if the decoded values satisfy rules that every reachable state satisfies. Each rule is `[candidate]` until plan 08-13 proves it on the states of the hard-point programs and the diagnostic executions; a wrong rule would refuse real snapshots.

1. INV-1: the CPU members pass the CPU state validator, the STIC members pass the STIC state validator and `event_index` is below the event count of the profile.
2. INV-2: the next STIC event tick (the frame phase plus `frame_index` times the frame length plus the tick of event `event_index`) is above `time_ticks`, and the event before it is at or below it; the gate flags equal the replay of the event table up to `event_index` (index 0 always open, index 3 always closed).
3. INV-3: `psg.next_sample` equals `time_ticks / sample_period_ticks + 1`; `obs.audio.emitted` equals `psg.next_sample - 1`; the tone outputs, `env_dir` and `env_held` are 0 or 1; `env_level` and `env_cycles` are at most 15; the bytes of `obs.audio.prev_levels` are at most 15; the noise register fits its width; every stored PSG register equals itself under its write mask.
4. INV-4: a controller word at an index at or above the profile's controller count is 0 with bit 31 clear; `mem.port_pulldown` equals the value derived from the words; `obs.input.floor_tick` is at most `time_ticks`.
5. INV-5: `phase` is POWERED and `time_ticks` is at most 2^63.
6. INV-6: `obs.video.count` equals `stic.frame_index` (frames chained equal frames completed on the machine path); `obs.psglog.count` and `obs.input.count` have no derivable relation and are free.

Counters, chains, RAM words and the latched card rows are free values: for them the trailer is the only check.

## Rights

A snapshot never carries an image byte: the three image arrays are class IMAGE_BEARING, and only the SHA-256 identities and the loaded flags of the slots are written.
A hash establishes identity, not rights.
A snapshot of a run with owner-supplied images still holds guest RAM and graphics RAM that those images caused to be written; it is derived rights-bearing material, like a raw video or audio dump.
It is never committed, `.ivsn` is added to the binary extension list and to `.gitignore` by a later plan of this phase (D-19), and the core writes no file.
No fixture of the repository is a snapshot.

## Evolution

The format version changes only for a change of the container layout; the schema version and the schema digest change for a change of the state rows.
A build restores exactly one pair of format version and schema version; any other pair is refused UNSUPPORTED with the version in the report, never loaded on a best effort.
The instance-free describe call reads a header up to the version fields, so a host can say that a snapshot was made by another version.
There is no migration code before 1.0.
Sections can be added later (for example a battery-backed cartridge RAM section) by raising the section count and the schema version together.

## Worked example

The first 24 bytes of every snapshot of the first profile, written by hand from the table above:

```
89 49 56 53 4E 0D 0A 1A   magic
00 01                     format version 1
00 00                     flags 0
00 00 10 CB               total_bytes 4,299
00 00 00 01               schema version 1
00 00 00 07               section count 7
```

The pinned literal is `header-prefix-hex` in `data/expected/ivsn-literals.ids`.
The CPU section entry of the section table (at offset 304) reads domain `01`, three zero bytes, offset `00 00 02 64` (612), length `00 00 00 23` (35) and the 32-byte digest of the domain.
The section itself starts at byte 612 with `49 56 53 44 01 00 01` (the text IVSD, domain 1, domain version 1) and continues with the count `00 00 00 08` of `cpu.r`, the eight register words, then the eight one-byte rows.

## Fuzzing

Plan 08-15 adds a seeded family of corruption cases to the gate and a libFuzzer target outside it.
The cases are exhaustive over the 4,299 bytes: every truncation length, every single-byte flip (refused by the trailer or by its header field), every single-byte flip with the trailer recomputed (reaching the validators), every single-bit flip of the header, length and section table lies, version and flag values, the forms of the profile id, flipped parameter and slot identities, and each derived invariant violated one at a time.
For every case the status is one of the documented set and matches the reason, a refusal leaves the whole instance byte-identical, an acceptance re-encodes canonically (with the eight digest fields of "Digests" as the only exception) and a second attempt gives the same result.

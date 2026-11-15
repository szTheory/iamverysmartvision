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

## As built (plan 08-11)

This section is appended after the sections above, which are the oracle of plan 08-09 and are not edited: it records what the writer of plan 08-11 produced when it was held to them.
The writer is `core/src/machine/machine_snapshot.c`; the public header is `core/include/ivs/ivs_state.h` with `ivs_machine_snapshot_size` and `ivs_machine_snapshot_save`; the suite is `state` (`tests/state/`, ids in `data/expected/tests/state.ids`).
Describe and restore are not part of this plan: the reasons 58 to 81 and the check order above are still [planned].

**Figures against the oracle.** The code was written after the derivation and compared with it; no figure had to be changed.

| Domain | Section | Oracle length | Writer length | Oracle offset | Writer offset |
|---|---|---|---|---|---|
| 1 | CPU | 35 | 35 | 612 | 612 |
| 2 | MEM | 2,761 | 2,761 | 647 | 647 |
| 4 | INSTANCE | 615 | 615 | 3,408 | 3,408 |
| 16 | VIDEO | 51 | 51 | 4,023 | 4,023 |
| 17 | AUDIO | 63 | 63 | 4,074 | 4,074 |
| 18 | INPUT | 79 | 79 | 4,137 | 4,137 |
| 19 | PSGLOG | 51 | 51 | 4,216 | 4,216 |

The header is 612 bytes, the sections sum to 3,655, the trailer starts at 4,267 and the container is 4,299 bytes, for a reset instance, for an instance stepped 100, 3,333 and 9,999 units, for the second stand-in content, for an instance without a cartridge and after a run of several frames with a video buffer.
The schema digest that the writer computes from the live descriptors of the table is `6cd4932f5ebf0b0fc2377555ca079d0313bb27a4df1a4fda9e15f37c6eb1d193`, the SHA-256 of `data/state/schema.literal.txt`: the hand derivation and the code agree on all 88 descriptor lines.
The test `state.literal.schema-digest` compares the lines first and names the first differing line, because a disagreement is a finding about the derivation or the code and never a reason to change the literal.

**Profile parameter digest.** Each of the four generators (`cmake/IvsMemmapTable.cmake`, `IvsPsgTable.cmake`, `IvsSticTable.cmake`, `IvsCtrlTable.cmake`) emits, after every byte it wrote before, one 32-byte constant per profile: the SHA-256 of the table text it wrote for that profile with the block comments removed, so no cite, tag or provenance string of the profile file is in it.
`ivs_snapshot_params_digest` hashes the four ASCII bytes `IVSP`, the big-endian u16 1 and the four constants in the order memory map, PSG, STIC, controller (134 bytes).
Its value for the registered profile is `89ad65073a3e357acbdd687f6a29016457864a1666b953e9327b7559dc0c0522`, pinned as `params-digest-sha256` in `data/expected/ivsn-literals.ids` as a repeatability pin of the current profile files, never as evidence that the tables are right.
The value was first computed by a scratch script outside the tree from the generated includes, and the test also rebuilds the preimage from the specification alone.
An edit of one cite string of the profile file changed none of the four constants, an edit of one table value changed exactly the constant of its table.

**The report struct.** `ivs_snapshot_report` v1 is 360 bytes (`IVS_SNAPSHOT_REPORT_V1_SIZE`): struct_size at 0, reason 4, format_version 8, schema_version 12, total_bytes 16, byte_offset 24, section_index 32, kind 36, profile_id 40 (64 bytes), profile_params_sha256 104, composite_sha256 136, three image reports at 168.
The size and save calls do not fill it; it is pinned now so that the layout exists before describe and restore use it.

**The two calls.** The refusal order is the one above for the writer half: argument pointers (BAD_ARG), handle (BAD_HANDLE), buffer or output overlapping the instance (BAD_ARG), OFF (BAD_STATE), capacity (BUFFER_TOO_SMALL, which writes the exact size to `out_written` and never a byte of the buffer).
Two cases are refused BAD_ARG in the first and third classes that the plan text did not list: an `out_written` that lies inside the `cap` bytes of the buffer (a successful call would overwrite the container with the length), and a capacity that reaches past the top of the address space (such a span names no real object, as for the buffers of the run call).
Everything that can refuse is decided before the first byte of the buffer is written, so a refusal never changes the buffer.

**The status band.** The section "Refusal reasons" above says that the status `IVS_E_STATE_MISMATCH` (-24) is added by plan 08-11. The writer has no refusal that needs it: the first call that can return it is restore, so it is added by plan 08-13 together with the narrowing of the reserved band, and the status count stays 14 here.

**The stepped snapshot literal.** `stepped-snapshot-sha256` in `data/expected/ivsn-literals.ids` is the SHA-256 of the whole snapshot of a stand-in boot program stepped 5,000 units after the power-on reset (the PSG writes of the stand-in tone program, then the RAM loop of the first stand-in program, both from decles already derived in `tests/machine/machine_standin.c`).
The state that the 5,000 units reach, asserted by the test, is: 43,338 CPU cycles and 173,352 master ticks (two frames completed, a cut in the third), the PSG registers and system RAM written, the audio chain moved and four write-log records.
The value was produced by the writer, so it is a repeatability pin and nothing more; because the same value is compared in the dev build (sanitized, `-O1`) and in the release build (`-O2`), the snapshot bytes are identical across the two builds.

**The named faults.**

| Fault | Site | What the test that catches it shows |
|---|---|---|
| SNAPSHOT_LITTLE_ENDIAN | `machine_snapshot.c`, the integer writer of the header | `state.literal.header-structure` reads a header that is not the 24 bytes derived on paper, while every length stays right |
| SNAPSHOT_PAD_SERIALIZED | `machine_digest.c`, the state-only walk | `state.size.derived-and-constant` finds sections longer than the literals by their reserved words (CPU 43 against 35, MEM 2,763 against 2,761, INSTANCE 635 against 615, AUDIO 67 against 63) |
| SNAPSHOT_IMAGE_BYTES_EMBEDDED | `machine_digest.c`, the state-only filter | `state.save.no-image-bytes` finds 16-byte windows of the image arrays in the snapshot (the MEM section grows from 2,761 to 113,365 bytes) and state bytes that depend on the images |

`state.save.no-image-bytes` also holds that each of the three slot SHA-256 values occurs exactly once in the snapshot, at its slot record, and that two instances that differ only in their image arrays give snapshots that differ only in the slot records, the composite and section digests and the trailer.
`state.judge.header-literal-corrupt` applies the comparison of the structure test to damaged copies (one byte of each field, the length, the trailer) and requires each to be reported by the comparison that names the field.

## As built (plan 08-12)

This section is appended after the sections above, which are the oracle of plan 08-09 and are not edited: it records what the reader of plan 08-12 does when it is held to them.
The reader is `core/src/machine/machine_snapshot_read.c`; its public face is `ivs_snapshot_describe` in `core/include/ivs/ivs_state.h`; the reasons 58 to 71 are defined there and pinned in `data/expected/api-reasons.ids`; the cases are the family `state.read.` and the self-control `state.judge.matrix-expectation-corrupt` (`tests/state/state_read.c`, ids in `data/expected/tests/state.ids`).
Restore is not part of this plan: the reasons 72 to 81 and steps 6 to 9 of the check order above are still [planned].
The layout constants (the header offsets, the section count, the header and trailer sizes, the domain table) moved from `machine_snapshot.c` into `machine.h` without changing a byte, so the writer and the reader share one set of numbers.

**The check order as produced.** The reader implements steps 2 to 5 of the check order above and the form of the profile id of step 6, in the order of the specification; the first failing class wins.

| Step | Check | Reason, status |
|---|---|---|
| args | the report struct (NULL, struct_size below 360 or above 4096) | BAD_ARG, nothing written |
| args | a NULL buffer with a length above 0; then the report overlapping the buffer | 58, 59 BAD_ARG |
| 2 | a length above 16,384, before any byte is read | 60 OVERSIZED |
| 2 | a length below 644 | 61 MALFORMED |
| 3 | the magic (eight bytes), format version, flags, schema version, schema digest | 62 MALFORMED; 63, 64, 65, 66 UNSUPPORTED |
| 3 | the total the bytes state above the length; below it or different from the size this build derives | 61; 67 MALFORMED |
| 3 | the section count | 68 MALFORMED |
| 4 | the section table, entry by entry | 69 MALFORMED |
| 5 | the SHA-256 trailer | 70 DIGEST_MISMATCH |
| 6 (form) | the form of the profile id | 71 MALFORMED |

**What is reported with a refusal.** The byte offset is the first differing byte for the magic, the supplied length for a truncation (also when the total is above it), the bound 16,384 for OVERSIZED, 0 for the two argument reasons, the offset of the failing field for the header reasons (8, 10, 16, 24, 12, 20), the start of the trailer for 70, and the first offending byte of the profile id for 71 (the last byte of the field, offset 119, when the id has no NUL).
For a section table defect the section index is the first entry, in order, whose rule fails (the domain, the three zero bytes, the offset against the end of the entry before, the length against the end of the payload) and the byte offset is the offset of that field (the entry start, the first nonzero zero byte, +4 for the offset, +8 for the length); when every entry holds and the last section does not end at the trailer start the entry is the last one and the field its length.
Every other reason reports section index 0xFFFFFFFF.
A refusal writes only reason, byte offset, section index and the header facts the supplied length let it read (format version from 12 bytes, total from 16, schema version from 24), read from the bytes as they stand whatever the defect; it keeps struct_size and every other member of the caller's report.
A success writes the 360 bytes of the v1 report and keeps the tail of a larger caller struct.

**Where the code and the plan text differ, and which is right.** The specification is the contract; where plan 08-12 reads differently the code follows the specification.
1. The form of the profile id (71) is judged after the trailer (step 6 follows step 5), so a bad id with a stale trailer is reported as 70. The prose of the plan listed the form before the trailer; the specification and the numbering of the reasons (70 before 71) put it after.
2. The reader does not compare the length of each section with the length derived from the table of the build. Step 4 of the specification covers known domains, order, offsets and lengths that fit, adjacency and the sum; the comparison with the derived lengths is step 8 of restore, reason 80. A length that is one more or less than derived breaks adjacency or the end of the last section and is reason 69 in every row of the matrix; a pair of changes that chain and keep the sum is accepted by describe (two rows) and is left to restore, so that reason 80 stays reachable. The total is compared with the size the build derives from the descriptors of the table (`ivs_snapshot_derived_total`), with no instance.
3. A length above the bound counts as the bound for the overlap rule of the report (the overlap is judged over the first 16,384 bytes at most), so that an absurd length is OVERSIZED and not an overlap.
4. An empty profile id and an unregistered id of good form are accepted by describe (the specification lists only three defects for 71 and only restore consults the registry).
5. The three image slot records of a success carry struct_size of an image report, the kind (slot index plus one), loaded, length and SHA-256 as the snapshot states them, and 0 in every other member; describe does not judge them (restore does).

**The matrix.** Rows per reason (a row is one call): 58: 7, 59: 5, 60: 11, 61: 36, 62: 13, 63: 7, 64: 18, 65: 12, 66: 35, 67: 14, 68: 12, 69: 92, 70: 95, 71: 24, that is 381 refusal rows, and 25 rows that are accepted (the snapshot as written, informational fields changed with a valid trailer, an empty, unregistered or 63 byte id, chained section lengths, and two argument rows that place the report beside the buffer), 406 rows in all.
Every row is run twice, into a report poisoned with 0xA5 and with 0x5A, and the whole report is compared with the one the contract allows; the sweep `state.read.describe-heap-exact` describes every length from 0 to the total plus one (4,301 lengths) from a heap buffer of exactly that length, and the self-control corrupts one expectation of each family in turn and requires the runner to report that row.

**The named faults.**

| Fault | Site | What the test that catches it shows |
|---|---|---|
| SNAPSHOT_TRAILER_UNCHECKED | `machine_snapshot_read.c`, the trailer comparison | `state.read.describe-refuse-trailer` finds a damaged trailer and damaged bytes that only the trailer covers accepted instead of reason 70 |
| SNAPSHOT_LENGTH_TRUSTED | `machine_snapshot_read.c`, the refusal of a total above the supplied length | `state.read.describe-refuse-length` finds a truncated snapshot reported with another reason (the buffers of a fault build carry a zeroed tail, so the reader reads defined bytes and fails by its status) |

## As built (plan 08-13)

This section is appended after the sections above, which are the oracle of plan 08-09 and are not edited: it records what restore of plan 08-13 does when it is held to them, and what the programs decided about the derived invariants.
Restore is `core/src/machine/machine_snapshot_restore.c`; its public face is `ivs_machine_snapshot_restore` in `core/include/ivs/ivs_state.h`; the status `IVS_E_STATE_MISMATCH` (-24) and the reasons 72 to 81 are defined there and in `ivs_status.h` and pinned in `data/expected/api-statuses.ids` and `data/expected/api-reasons.ids` (the reserved band is -25 to -31 from this plan); the cases are the family `state.restore.` and the self-control `state.judge.refusal-expectation-corrupt` (`tests/state/state_restore.c`, `tests/state/state_invariants.c`, `tests/state/state_util.c`, ids in `data/expected/tests/state.ids`).
Steps 6 to 9 of the check order above are now implemented; STA-01 and STA-02 stay open until the hard-point suite of plan 08-14 exists.

**The check order as produced.** The reader of plan 08-12 is called unchanged, so steps 2 to 6 (the form of the id) are exactly the table of "As built (plan 08-12)"; restore adds the arguments before it and the layers after it.

| Step | Check | Reason, status |
|---|---|---|
| 1 | the report struct (not NULL with struct_size below 360 or above 4,096) | BAD_ARG, nothing written |
| 1 | a NULL buffer with a length above 0; then the report overlapping the first min(len, 16,384) bytes of the buffer | 58, 59 BAD_ARG |
| 1 | a NULL or invalid handle (the report gets the reason IVS_REASON_HANDLE) | BAD_HANDLE |
| 1 | the first min(len, 16,384) bytes of the buffer, or the report, overlapping the storage of the valid instance (the report is not written when it is the one that overlaps) | 81 BAD_ARG |
| 2 to 6 | the container, as describe judges it | 60 to 71 |
| 6 | the profile id registered; the id equal to the instance's; the parameter digest equal to this build's | 72 UNKNOWN_PROFILE; 73, 74 STATE_MISMATCH |
| 7 | per slot in the order EXEC, GROM, cartridge: a loaded flag above 1, or an empty record that holds a length or a digest; a slot loaded in the snapshot and not in the instance; a slot loaded only in the instance, or loaded on both sides with another SHA-256 or length | 78 MALFORMED; 75 MISSING_SYSTEM_IMAGE; 76 STATE_MISMATCH |
| 8 | every section length equal to the derived length | 80 |
| 8 | every array row announces the count of the table | 77 |
| 8 | the frame of each section (IVSD, the domain, the domain version 1); the CPU, PSG and STIC states through their validators and the event cursor; the phase POWERED; machine time at most 2^63 | 78 |
| 8 | the derived invariants INV-2, INV-3, INV-4 and INV-6 | 79 |
| 9 | the commit: every STATE row decoded into the instance, every PAD row zeroed, no BOUND or IMAGE row written, the phase becomes the snapshot's POWERED | none: it cannot refuse |

Every refusal of steps 1 to 8 leaves the whole instance byte-identical and writes only the report: the sections are decoded into a probe of a few hundred bytes (`ivs_snap_probe`: the CPU, PSG and STIC structs, the input row and the few scalars the invariants read) and judged there, and the commit has no refusal path.
A refusal writes reason, byte offset, section index and the header facts the length let the reader see, as describe does, and the slot kind for a refusal about a slot; a success writes the full v1 report exactly as describe does.
The byte offset is the field of the slot record (75, 76, and 78 about a record), the id field (72, 73), the parameter digest field (74), the length field of the section table entry (80), the count prefix of the array row (77), the byte of the frame (78 about a frame), and for 78 and 79 about a struct or scalar the offset in the container of its first element (the section index says which section): the CPU, the phase, machine time, the PSG state and the STIC state for 78, and the STIC state (INV-2), the PSG state (INV-3), the input row (INV-4) and the video count (INV-6) for 79.

**Where the code and the plan text differ, and which is right.** The specification is the contract; where plan 08-13 reads differently the code follows the specification and the difference is recorded here.
1. INV-1 (the three validators and the event cursor) and INV-5 (the phase and the range of machine time) are judged as range rules, reason 78, as the plan text lists them with the validators; INV-2, INV-3, INV-4 and INV-6 are the derived rules of reason 79. The one function `ivs_snapshot_invariants` evaluates all six, in the order 1, 2, 3, 4, 5, 6, and judges a live instance with the same code.
2. The check of step 7 against the recorded IMAGES domain digest is not made: a snapshot has no IMAGES section (the three slot records are its whole image structure), so there is nothing recorded to compare with the live structure; the slot identities are what is compared.
3. The record of a slot must be a record a writer makes: a loaded flag above 1, or an empty record that holds a length or a digest, is reason 78 at the slot (the instance zeroes a slot when it is empty). It is judged slot by slot together with the comparison of that slot, so the first failing slot in the order EXEC, GROM, cartridge decides.
4. The frame of each section (IVSD, the domain of the table entry, the domain version 1) is checked: the reader does not look at it and a snapshot with another frame would not re-encode canonically. A wrong frame byte is reason 78 at that byte.
5. Step 8 is ordered as the specification orders it (the lengths 80 first, then the counts 77, then the ranges 78, then the invariants 79); the prose of the plan names 77 before 80.
6. The comparison of the event cursor with the event count of the profile (INV-1) is the same bound as the validator's own limit for the registered profile (16 events, the table maximum), so the rows that exercise one exercise the other.

**The derived invariants, decided by the programs.** `state.restore.invariants-hold-reachable` builds the live probe of a running instance and judges it with the function restore uses, at every unit boundary of 23 programs: the Lamp Tone with no input (4 frames, 11,104 boundaries) and with the six events of its scenario (14 frames, 42,229), the boot stub contract 2 alone (7,469), the first STIC diagnostic without and with its selector (41,729 and 41,722), the second STIC diagnostic with no selector and with each of the five selector words (11,086, 11,089, 11,071, 11,080, 11,065 and 11,100), the nine stand-in programs of the machine suite and the boot program (20,001 each), a program that halts (301) and one that ends in a guest fault (20): 411,075 boundaries in all, reached through the public run call one unit at a time with the events resubmitted from the applied-event count (the programs with input) or through the internal step seam.
No invariant is violated at any of them. The states the programs reach give each invariant something to say: the register gate closed at 362,123 boundaries and the graphics gate at 319,106, a held key with a pull-down at 129,365, a halted CPU at 75,510 and a faulted one at 1, the audio level 15, 14 frames completed, and the reset state (machine time 0, the cursor at the very start) at exactly 23 boundaries, one per program.

| Rule | Status | The state that decided it |
|---|---|---|
| INV-1 | kept | the validators and the event cursor hold at every boundary, including the halted and the faulted CPU |
| INV-2 | corrected | at the reset state (machine time 0, frame 0, event 0) the first event, at tick 0, has not been applied: the timeline is caught up at the end of every unit and never at the reset, so "the next event is above machine time" fails there. The rule keeps that clause for every state but that one (23 boundaries, one per program); the previous-event clause and the gate flags hold at the reset state as written |
| INV-3 | kept, strengthened | the sample index, the emitted count and the registers' masks hold as written; the previous levels hold the form of the specification ("the bytes are at most 15") at every boundary, and the rule kept is the stricter one derived from the step code (three four-bit levels in the low three bytes and a fourth byte of 0, the sample being A, B, C, 0): no reachable state forced the change, a state with a fourth byte of 1 is one no step makes |
| INV-4 | kept | the words, the pull-downs derived from them and the input floor hold at every boundary of the Lamp Tone with its six events and of the diagnostics with a selector |
| INV-5 | kept | a snapshot is of a POWERED machine and machine time stays below 2^63 |
| INV-6 | kept | the candidate `obs.video.count == stic.frame_index` holds at every boundary of every program (frames reach 14): it is decided true and is kept |

The probes that show each rule can refuse (a change of one rule in the working tree, each caught by the ids named for it) are in the summary of the plan.
The correction of INV-2 and the strengthening of INV-3 are recorded here and in the code (`ivs_snap_inv2`, `ivs_snap_inv3`), as the plan requires, and the oracle sections of this document above are not edited.

**The digest fields (D-41).** Restore does not authenticate the composite digest or the seven section digests of the header, and does not recompute them: `state.restore.digest-fields-informational` accepts a snapshot whose eight digest fields were flipped and whose trailer was recomputed, and afterwards the target's real digests are the saved instance's real digests, not the header's.
A snapshot of the restored target then differs from the accepted bytes in exactly the eight fields and the trailer, which is the one exception of the canonical re-encode rule named in "Digests" above.
The trailer is integrity, not authenticity: a hostile snapshot with a recomputed trailer is refused by the validators and the invariants (the rows of `state.restore.refuse-content-validators` and `state.restore.refuse-content-invariants` are all resealed).

**The profile id seam.** With one registered profile a second registered id cannot be built through the public API, so reason 73 (the id is registered and differs from the instance's) cannot be produced by an API row.
The comparison is the pure function `ivs_snapshot_profile_id_matches(view, descriptor)`, and `state.restore.profile-id-seam` is a seam test, not an API test: it gives the function a synthetic descriptor (equal, different in the first or the last byte, a prefix, longer, empty, 63 bytes, another case).
The API rows reach reason 72 (an unregistered id, with the forms of a prefix, a longer id and another case) and 74 (a flipped parameter digest); `state.restore.reasons-covered` counts reason 73 through the seam rows and says so.

**The matrices.** The refusal rows are written from this specification and the header of `ivs_state.h`, and the position of every member they change is found from the digest field table read as data, never from the restore code: 17 rows of the container structure through restore (one per reason 60 to 71 and the order of the classes), 18 of the profile, 37 of the slots (each of three slots with the flag, the empty record, the missing image and the flipped digest and length, and the order of slots and classes), 81 of the content ranges, counts and lengths (the CPU validator rule by rule, the PSG bounds at their first refused value, the STIC members, the scalars, the section frames, the array counts of every section and the chained section lengths) and 27 of the invariants, 180 rows against four targets (powered, without a cartridge, OFF without EXEC, OFF without GROM), plus 20 argument rows written as code.
Every row is run twice into a report poisoned 0xA5 and 0x5A and the whole 400-byte caller struct is compared with the report the contract allows; the whole target instance must be byte-identical afterwards, and `state.restore.atomic-every-refusal` repeats the whole set against poisoned non-trivial targets with the nine digests.
`state.restore.poisoned-target-equals-source` restores 13 cuts of the boot program, the Lamp Tone and two diagnostics into targets whose STATE and PAD rows hold a poison byte (two different bytes in turn) and requires the whole instance, the pad bytes and the nine digests to equal the source.

**The named faults.**

| Fault | Site | What the test that catches it shows |
|---|---|---|
| RESTORE_PROFILE_UNCHECKED | `machine_snapshot_restore.c`, the identity checks of step 6 | `state.restore.refuse-identity-profile` finds an unregistered id and a flipped parameter digest accepted |
| RESTORE_IMAGE_DIGEST_UNCHECKED | `machine_snapshot_restore.c`, the SHA-256 and length comparison of a slot loaded on both sides | `state.restore.refuse-identity-slots` finds a snapshot taken with other images accepted |
| RESTORE_MUTATES_BEFORE_VALIDATE | `machine_snapshot_restore.c`, the CPU rows committed before the STIC validators | `state.restore.atomic-every-refusal` finds a target changed by a refusal that only a STIC validator makes |

## Fuzzing (as built, plan 08-15)

This section is appended below the sections above, which are the oracle of plan 08-09 and are not edited: it records what the corruption family of plan 08-15 does with the snapshot and with the check order, the invariants it holds on every case, the places where it accepts a changed byte and why, and the libFuzzer target outside the gate.
The family is `tests/state/state_fuzz.c` (ids `state.fuzz.` and `state.judge.fuzz-invariant-detects`, listed in `data/expected/tests/state.ids`); the target is `tests/fuzz/fuzz_snapshot.c`.

**The input.** One valid snapshot of 4,299 bytes: a Lamp Tone instance with the first of its input events applied, stopped at the first unit boundary at or after machine time 156,400 (frame 2, in the middle of the frame), so that the counters, the RAM, the latched card rows, the pull-downs and the observation rows are not default.
It is restored into a Lamp Tone instance of the same three images whose STATE and PAD rows hold a poison byte; the instance is put back from a baseline copy after every accepted case.
The generator of the random cases is an in-repo xorshift with a fixed seed; nothing reads a host source of randomness or time.

**The invariants of every case.** The status is one of the documented set and is the one its reason travels with; the outcome is one the case's expectation allows; a refusal leaves every byte of the 114,416-byte instance equal to the baseline copy and writes only the report (a 400-byte caller struct poisoned 0xA5 or 0x5A alternately: every byte the contract does not name keeps its poison, and the header facts are those of the bytes the length let the reader see); an acceptance writes no byte outside the STATE and PAD rows of the digest field table, zeroes every PAD row, and leaves the state-only preimage of each section that differs from the valid snapshot's equal to the input's section.
On a bounded subset of the accepted cases (the first 16 of an id and every 97th) all seven sections are compared, the report equals the report `ivs_snapshot_describe` writes for the same bytes, a full save of the restored instance equals the input bytes except the eight informational digest fields and the trailer (the exception of "Digests": the check copies those ranges from the input and recomputes the trailer before it compares), and the restored machine executes 6,000 ticks through the public run call.
A case may run twice, once with each poison, on a target put back from the baseline: the status, the reason, the offset, the section, the slot kind and the instance after an acceptance are identical.
The expectation of a flipped byte is a position classifier written from the Layout table and the Check order, never from the reader or from restore.

| Class of the byte | Trailer left as it was | Trailer recomputed |
|---|---|---|
| magic, format version, flags, schema version, schema digest, section count | 62, 63, 64, 65, 66, 68 at the field | the same |
| total_bytes | 61 when the flipped total is above the length (offset the length), else 67 (offset 12) | the same |
| section table entry: domain, reserved bytes, offset, length | 69 | 69 |
| profile id, parameter digest | 70 | 71 at the first offending byte, or 72 for a well formed id that is not registered; 74 |
| slot record: loaded flag, SHA-256, length | 70 | 78 at the flag or the length (kind of the slot), 76 at the SHA-256 or the length |
| composite digest, the seven section digests | 70 | accepted (informational) |
| frame of a section, count prefix of an array row | 70 | 78 at the byte, 77 at the prefix |
| value of a row | 70 | accepted, 78 or 79 (a free row: always accepted) |
| trailer | 70 | the flip is repaired: the valid snapshot, accepted |

**What the ids hold.** The counts are those of the normal build; a fault run takes every 16th point of each sweep.

| Id | Cases | Outcome |
|---|---|---|
| `state.fuzz.truncations` | 4,299 (every length from 0 to the size minus one) | all refused, 61 at the supplied length |
| `state.fuzz.byte-flips` | 8,598 (every offset, XOR 0xFF and XOR 0x01) | all refused, 61 to 70 by the table above |
| `state.fuzz.byte-flips-resealed` | 3,190 (XOR 0xFF at every offset of the header, the frames, the counts, the constrained rows and the trailer and at the first 8, the last 8 and every 4th element of the long free rows, XOR 0x01 at every offset but the free rows) | 1,258 refused, 1,932 accepted |
| `state.fuzz.header-bit-flips` | 3,968 (all eight bits of every header byte that is not a digest byte, one rotating bit of each of the 416 digest bytes; each with and without the reseal) | 3,712 refused, 256 accepted |
| `state.fuzz.length-and-table-lies` | 126 (the totals, the supplied length, the versions, each flag bit, the schema version, the section counts, the magic, the section table, the forms of the profile id, the parameter digest and the three slot records) | all refused, with the reason and the offset the specification gives |
| `state.fuzz.semantic-lies` | 28 (each derived invariant and device validator violated alone, with a valid trailer) | all refused, 78 or 79 at the first element of the struct the rule concerns |
| `state.fuzz.saturated-counters` | 2 (every 32-bit counter at 0xFFFFFFFF and every free 64-bit count at 2^64 - 1; machine time at its largest with the frame index, the video count, the PSG sample index and the emitted count moved with it) | both accepted; the second machine reaches 2^63 |
| `state.fuzz.random-resealed` | 1,200 (one to eight bytes set to random values, every second case with the trailer recomputed and 15 of 16 of its bytes in the payload) | 777 refused, 423 accepted; the expectation is a model of the container layer written from the Layout table |
| `state.fuzz.describe-heap` | 4,484 (the families above through the instance-free describe on a heap buffer of exactly the input size) | status, reason, offset and section equal the model's, the report is written as the contract says |
| `state.fuzz.different-handle` | 10 (a NULL, destroyed and damaged handle, three buffers and one report that overlap the instance, the arguments before the handle) | BAD_HANDLE with the reason 29 and no byte of the buffer read, BAD_ARG 58 and 81, nothing written to an overlapped report |
| `state.fuzz.second-attempt-identical` | 623 cases, each twice | identical |
| `state.judge.fuzz-invariant-detects` | the checker against a wrong expectation, an acceptance and a refusal the wrong way round, a wrong offset, a classifier row of each of the 19 classes with the wrong reason, and a byte of the target, of the report or of the re-encoded section changed behind its back | each is reported, and the untampered control of each is not |

The second value (XOR 0x01) is not taken for the bytes of a free row, and the interior of a long free row (the system and scratchpad RAM words, the graphics RAM words, the latched card rows) is taken at every 4th element, with the first 8 and the last 8: XOR 0xFF flips every bit of a byte, the valid snapshot's own round trip shows each bit restored as it was, and a free row is one array decoded by one loop, so what the omitted flips would show is the boundary elements, which are taken.
A byte of a 32-byte digest is compared or ignored as a whole, so the bit sweep takes one bit of each digest byte (rotating with the offset) and the byte sweeps take both values of every one.
Both choices are made for the cost of the gate (plan 08-15, the cost stop rule of the phase), not because the checks differ.

**Accepted by design.** A flip that ends in an acceptance is one of: a free row, which the specification names (counters, chains, RAM words and the latched card rows; here `cpu.r`, `mem.sysram`, `mem.scratchpad`, `mem.gram`, `stic.row_latch`, the four STIC counters, the four chains and the three free counts) and which every flip changed without a refusal; an informational digest (D-41); the repaired trailer; or a value that lands on another value of the row's range that no rule connects to the rest of the state (a Boolean member 0 for 1, a PSG counter or level inside its bounds, the color stack pointer inside 0 to 3, an audio level at most 15, an input floor at most machine time, a PSG register bit inside its write mask, a CPU run state HALTED for RUNNING).
No such value is a validator gap that this plan could close from a proof about reachable states.

**The known acceptances.** Two behaviors are pinned and not closed.
The STIC register words (`mem.stic_reg`, 64 words) are stored masked by the write path of the memory system, so a word with a bit outside the write mask of its register is a state no step makes; no validator or invariant refuses it and all 256 flips of the sweep are accepted (the test pins this, so a later validator is a conscious change).
Machine time is not tied to a unit boundary: a snapshot whose time differs by one tick from the saved one is accepted when the sample index, the frame cursor and the input floor still agree with it.
Closing either needs a new invariant proved on the reachable states of the programs of plan 08-13 and a change to the core; both are open items, with a restored machine of each accepted case executing without a sanitizer report (the bounded subset).

**The libFuzzer target.** `tests/fuzz/fuzz_snapshot.c` builds only when the cache option `IVS_ENABLE_LIBFUZZER` is ON (it joins the block of `ivs_fuzz_ivsc` in `tests/CMakeLists.txt`, `EXCLUDE_FROM_ALL`), is registered with no ctest, gate step or CI job, and needs a clang with the libFuzzer runtime (a Homebrew LLVM or a Linux clang; Apple clang has none).
The input is a control byte (bit 0: recompute the trailer) and the snapshot bytes, copied to a heap buffer of exactly their size; the target restores into a live instance of the stand-in content that is put back from a baseline copy, holds the invariants above and aborts on a violation, and the header of the file gives the recipe, the corpus seed (`IVS_SNAPSHOT_FUZZ_SEED`) and the sanitizer choice.
A crash input is the snapshot of a machine that executed guest code, so it is never committed.

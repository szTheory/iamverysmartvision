# IVSC v1 cartridge container

This note specifies IVSC v1, the cartridge container that the machine core reads (MEM-05).
It is the project's own format, written for this project (D-29); it is not a copy or a subset of any other cartridge or image format.
Importers that turn other formats into IVSC live outside the core, arrive in a later phase, and each needs its own rights review before it is written.
The file extension is `.ivsc`; the core never sees a file name, only bytes and a length.
No IVSC container is committed to the repository: the fixtures are text (see "Fixtures").

The parser is `core/src/content/ivsc.c` (layer 1, profile-independent).
Layer 2, the load into a machine instance, checks the profile's cartridge span and features (plan 03-11).
A label [INF] marks a design reason; every other line is a rule of the format.

## Conventions

Every multi-byte field is big-endian: the most significant byte comes first.
This is the one wire byte order of the project, shared with the system-image container and the state digest serialization (D-26, D-30, D-35).
A word is a 16-bit field whose value is at most `03FF` (a 10-bit decle); a larger value is refused, never masked.
Offsets and sizes are in bytes unless a name says words.
All offsets are counted from the first byte of the container.

## Layout

| Offset | Size | Field | Rule |
|---|---|---|---|
| 0 | 8 | magic | exactly `89 49 56 53 43 0D 0A 1A` |
| 8 | 2 | version | 1 |
| 10 | 2 | flags | 0 |
| 12 | 4 | required_features | bit mask of required cartridge features; v1 defines no bit |
| 16 | 4 | total_bytes | the size of the whole container, trailer included |
| 20 | 2 | segment_count | 1 to 16 |
| 22 | 2 | reserved | 0 |
| 24 | 8 per segment | segment table | one entry per segment, in table order |
| 24 + 8n | 2 per word | data | the words of every segment, segment after segment in table order, no gaps, no padding |
| total_bytes - 32 | 32 | trailer | SHA-256 (FIPS 180-4) of every byte before the trailer |

The fixed prefix is the first 24 bytes, magic included; there is no padding anywhere (A-03).

A segment entry is 8 bytes:

| Entry offset | Size | Field | Rule |
|---|---|---|---|
| 0 | 2 | addr | first word address of the segment |
| 2 | 2 | len_words | 1 to 65,535 |
| 4 | 1 | width | 10 (bits per word) |
| 5 | 1 | attr | 0 (read-only ROM) |
| 6 | 2 | reserved | 0 |

The container stores no data offsets: the words of segment i start at `24 + 8n + 2 * (len_words of segments 0 .. i-1)`, where n is segment_count.
So no offset can be forged to point outside the data or into another segment [INF].
The magic starts with a byte above 7F and holds a CR LF pair and a SUB byte, so a 7-bit transfer or a text-mode line-ending conversion changes it and the container is refused [INF].

## Limits

- 1 to 16 segments; 1 to 65,535 words per segment.
- Segments are listed with ascending start addresses and must not overlap; adjacent segments (a start equal to the previous segment's end) are allowed.
- There is no alignment rule: a segment may start at any address (A-01).
- `addr + len_words` must not exceed 10000 (hex): a segment may end at the last address FFFF but never wraps around.
- The smallest container is 66 bytes: prefix 24, one entry 8, one word 2, trailer 32.
- The largest container is 131,256 bytes: prefix 24, sixteen entries 128, 65,536 words 131,072, trailer 32.
- A caller length above 131,256 is refused before any byte of the container is read.
- Every value not defined here is refused: another version, any flags bit, a nonzero reserved field, another width or attr. There are no optional or ignorable sections and no metadata (title, year, notes).

## Check order

The parser checks in this order, and the first failing check decides the refusal (D-33):

1. Length: above 131,256 (too-large), then below 66 (truncated). Nothing is read before this check.
2. Magic: the first byte that differs from the magic is reported.
3. Version.
4. Prefix: flags, reserved, then total_bytes against the limit (too-large), against the length (truncated when above it, trailing when below it), then segment_count (0 or above 16).
5. Table: first a size check that every entry plus one word per entry plus the trailer fits in total_bytes (size-mismatch); then each entry in table order: len_words 0, width, attr, reserved, then the address-space end (addr-wrap); then total_bytes against the size the table implies (size-mismatch).
6. Order and overlap, pair by pair in table order: a start below the previous start (order), then a start below the previous segment's end (overlap).
7. Digest: the SHA-256 of every byte before the trailer against the trailer, compared over all 32 bytes.
8. Word range: every word of every segment in table order; the first word above `03FF` is reported.

The digest check comes before the word range, so a damaged data bit is reported as a digest mismatch; the checks before the digest look only at the prefix and the table [INF].
required_features is read but not judged by the parser; it is a profile question (see "Required features").

## Refusal reasons

A refusal returns a status and fills a report: reason, segment index (the entry concerned, or FFFFFFFF when none), byte offset, and the required_features value (0 when the length check refused).
The reason numbers are stable; a new reason gets the next number.

| No. | Reason | Status | Byte offset | Segment |
|---|---|---|---|---|
| 1 | truncated | MALFORMED | the length | none |
| 2 | too-large | OVERSIZED | 131,256 for a caller length; 16 for total_bytes | none |
| 3 | magic | MALFORMED | the first differing magic byte | none |
| 4 | version | MALFORMED | 8 | none |
| 5 | flags | MALFORMED | 10 | none |
| 6 | reserved | MALFORMED | 22 | none |
| 7 | trailing | MALFORMED | total_bytes (the first byte after the container) | none |
| 8 | segment-count | MALFORMED | 20 | none |
| 9 | size-mismatch | MALFORMED | 16 | none |
| 10 | segment-length | MALFORMED | the entry's len_words field | the entry |
| 11 | segment-width | MALFORMED | the entry's width field | the entry |
| 12 | segment-attr | MALFORMED | the entry's attr field | the entry |
| 13 | segment-reserved | MALFORMED | the entry's reserved field | the entry |
| 14 | addr-wrap | OUT_OF_RANGE | the entry's addr field | the entry |
| 15 | order | MALFORMED | the later entry's addr field | the later entry |
| 16 | overlap | OVERLAP | the later entry's addr field | the later entry |
| 17 | digest | DIGEST_MISMATCH | total_bytes - 32 (the trailer) | none |
| 18 | word-range | MALFORMED | the word | its segment |

Reason 0 means none.
The statuses are the content codes of `core/include/ivs/ivs_status.h` (D-09): MALFORMED -17, OVERSIZED -18, OVERLAP -19, OUT_OF_RANGE -20, DIGEST_MISMATCH -23.
A NULL view or report, or NULL bytes with a length above 0, is BAD_ARG and nothing is written; this check comes before the length check.
A refusal writes only the report; an acceptance writes only the view, which holds the prefix values and, per segment, addr, len_words and the computed data offset.
The parser keeps no pointer into the container and allocates nothing.

## Digests

The trailer is the SHA-256 of bytes 0 to total_bytes - 33.
The loader reports a second value, the SHA-256 of the whole supplied container, trailer included, which equals `sha256sum` of an `.ivsc` file (A-04, D-25).
The two values differ; the loader computes both in one pass.
The trailer protects the container against damage; it does not establish rights or provenance (a hash establishes identity, not rights).

## Required features

required_features is a 32-bit mask; v1 defines no bit, so v1 containers made by this project carry 0.
The parser copies the value into the view unchanged.
The load (layer 2, D-32) refuses any bit that the profile does not list in `cartridge.features_supported` with UNSUPPORTED and reports the missing bits; the D3 profile lists none.

## Evolution

The format changes only by a new version number.
A v1 parser refuses every other version, so a reader never guesses at fields it does not know.
A later version may add sections, each counted and refused when unknown.

## Worked example

The smallest container, fixture `ivsc.P01-min`: one segment of one word `03FF` at address 5000, 66 bytes.

```
offset  bytes
0       89 49 56 53 43 0D 0A 1A      magic
8       00 01                        version 1
10      00 00                        flags
12      00 00 00 00                  required_features
16      00 00 00 42                  total_bytes 66
20      00 01                        segment_count 1
22      00 00                        reserved
24      50 00 00 01 0A 00 00 00      entry: addr 5000, len_words 1, width 10, attr 0, reserved 0
32      03 FF                        data: one word
34      C1 19 92 EF 49 47 43 4D A0 1A EC 35 A6 43 97 06
50      51 50 01 1F 25 D7 76 F8 9F CE E5 18 9A 49 90 EF   trailer
```

The trailer is the SHA-256 of the 34 bytes at offsets 0 to 33, and the SHA-256 of all 66 bytes is `5d774e95472f05ec629b2673481d0e05cd8a8e7f25db7cda6d94807424957dfd`; both values were computed with `cmake -E sha256sum` on a scratch file and are typed into the fixture, so the core's SHA-256 meets an independent value.

## Fixtures

The parse-level vectors are text: `data/cart/ivsc-parse.json`, listed in `data/expected/cart-fixture-files.ids` and recorded with their digests and sizes in the fixture manifest `fixtures/FX-ivsc-vectors/manifest.json`.
Positive records give the whole container as hex with an authoring-time trailer and whole-file digest; long containers are described by repeated words and assembled by the test; negative records patch a positive base in one stated place.
`cmake/IvsCartTable.cmake` turns them into a table in the build tree, and suite `ivsc` checks each against the parser in the dev and release presets.
Any edit of a fixture file reruns `sh tools/cart/manifest-outputs.sh` and refreshes the manifest in the same commit.

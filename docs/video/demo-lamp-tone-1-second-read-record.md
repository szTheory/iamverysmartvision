state: closed
oracle-commit: 4e0febef675c2e81fe38d6c0caf353625218b1eb
corrections: 0

# Lamp Tone blind second-read record

Evidence kind: `source-expectation, second-read`. This is not an `independent-implementation-comparison`: the reader is the same model family as the author of the derivation, so a shared misreading stays possible (C43). Claims that rest on this record stay provisional.

Date: 2026-11-13. Request: `docs/video/demo-lamp-tone-1-second-read-request.md`. Reader: one fresh isolated agent, no access granted to anything but the inputs of the request.

## What was read

The reader received a scratch directory outside the repository that held exactly five files: the specification text between the markers `<!-- blind-spec-begin -->` and `<!-- blind-spec-end -->` (made with the range `sed` of the request), the controller literal, the PSG register-address literal, `docs/runner.md` and `docs/cp1610/reference-notes.md`. The reader used no script, no emulator and no file of the repository. It derived the five items of the request from the specification alone.

## Result against sections (c) to (f)

| Item | Compared with | Result |
|------|---------------|--------|
| 1. Lamp patterns of row 4 and row 7, frame slots 0 to 12 | section (c) per-frame table | 13 of 13 slots agree (slot 0 not displayed; event 1 first seen by frame 2; event 6 first seen by frame 12) |
| 2. RAM `$02F0` to `$02F7` at the four stops, PSG records so far | section (c) RAM table | 4 of 4 stops agree, all eight words each; records 9, 17, 23, 30 agree |
| 3. PSG write log in order | section (d) list | 30 of 30 records agree (index, address, value, frame) |
| 4. Channel levels at the eight stamps | section (d) windows | 6 of 6 asserted ticks agree (100,000 and 690,000 silent; 140,000, 160,000, 740,000, 760,000 at `A=0 B=0 C=15`); the reader marked 200,000 and 500,000 `not derivable`, which agrees with section (d), where a tone-enabled channel with a non-zero amplitude is never asserted |
| 5. Differing rectangle and count of the seven pairs | section (e) pair table and `data/expected/video-frame-pairs.ids` | 7 of 7 counts agree (128, 256, 128, 64, 192, 384, 320); every rectangle of the reader lies inside the declared rectangle of the inventory (the reader gave the tightest box, the inventory a declared container, which the request allows) |

No disagreement was found. No value in the derivation, in the six frame literals F-S1 to F-S6 or in the pair inventory changes, so `corrections: 0`.

## The two items given a second look

- A lit lamp is a cleared bit: the specification, the controller literal and the rule that an idle port reads `FF` agree; no contradiction.
- The lamp builder (SLLC sets the carry from bit 15, ADCR adds the carry): the blinded inputs do not state either rule (the reference notes list ADCR only as an opcode row and do not restate the shift rules), so the reader could neither confirm nor contradict the reading. This item is therefore not closed by this record. It is carried to plan 07-06, which measures the listing against the core and the instruction tests, and to the listing review, and it is recorded in section (k) of the derivation if it forces a change.

## Reader remarks (no change forced)

1. The reader read "RAM after the last event read before each stop" as the state at the stop tick; section (c) says the same.
2. The reader computed the end of frame 13 as 835,304; the product 14 x 59,736 is 836,304, as section (c) states. Both lie beyond the last stop, so the set of delivered frames (0 to 12) is the same.
3. The reader assumed the fine-period value is the byte with a zero upper byte (`00XX`), which is what the derivation lists.
4. Frame 0 is delivered and not displayed, so no pattern exists for it; the derivation says so.

## Baseline

This commit adds the record with zero corrections, so the oracle baseline is the oracle commit named in line 2: sections (a) to (j) and the literals F-S1 to F-S6 stay as committed there, and change only by an appended section (k) or by bare claim tags.

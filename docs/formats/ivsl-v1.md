# IVSL v1 input log

This note specifies IVSL v1, the input log that the runner writes for one run and reads back to
replay that run (TOOL-01).
It is the project's own format, written for this project (D-15); it is not a copy or a subset of
any other log, macro or trace format.
The file is a text file of input events and their ticks.
It is not a recording of anything the machine produced: it holds no video, no audio, no register
state, no memory image, and no digest of any of those.
Under D-11 that is what keeps it out of derived material: an input log holds nothing the images
produced.
The file extension is `.ivsl`; the core never sees the file, only the events the runner derives
from it, and no IVSL file is committed to the repository (see "Rights").
The writer and the parser are `runner/ivsl.c`; this document is the rule that translation unit is
read against.
A label [INF] marks a design reason; every other line is a rule of the format.

## Conventions

Every line ends with a single LF byte (`0x0A`). A file whose last byte is not an LF is refused.
Every byte is printable ASCII: `0x20` to `0x7E`, plus the LF line separator. No byte above
`0x7E`, no NUL, no CR, no horizontal tab and no byte-order mark.
One directive per line. No line carries trailing whitespace and no line carries a comment.
Hexadecimal fields are exactly eight upper-case hex digits, zero-padded. A lower-case hex digit
is refused rather than folded.
Ticks are decimal digits only: no sign, no `+`, no `0x`, and no leading zero beyond the single
digit `0`.
A tick is a master tick, the same unit the scenario's `at_tick=` and the run request's
`target_ticks` use (the time base is owned by `docs/time-base.md`).
Directives are separated by exactly one space; there is no other whitespace in the file.

## Grammar

The file has exactly five directives and no others.

| Directive | Shape | Meaning |
|---|---|---|
| `ivsl <version>` | `ivsl 1` | the format and its version; must be the first line of the file |
| `scenario <id>` | `scenario <slug>` | the id of the scenario the run was given |
| `profile <id>` | `profile <slug>` | the id of the profile the run was given |
| `content project\|user` | `content project` or `content user` | the content class of the run's material |
| `at_tick <n> c0=<HEX> c1=<HEX>` | `at_tick <decimal> c0=<HEX8> c1=<HEX8>` | one input event at one master tick |

The four header directives appear exactly once each, in the order above, at the start of the
file: `ivsl <version>`, then `scenario <id>`, then `profile <id>`, then `content project|user`.

Zero or more event lines follow the header, in nondecreasing tick order:

```
at_tick <n> c0=<HEX> c1=<HEX>
```

`<n>` is a master tick, `c0` is the control word of PSG port 0 (`$01FE`) and `c1` of PSG port 1
(`$01FF`).
A word is the set of switches that are closed, in the bit layout pinned by
`data/expected/input-bits.ids`: keypad bits 0..11, disc positions bits 12..27 as sixteen 22.5
degree steps clockwise from north, side codes bits 28..30, and bit 31 reserved.
Several bits may be set at once.
Two events may share a tick; the later line is the state in force after the earlier one, which is
the core's own application rule for equal ticks.
A log with a header and no event line is well formed: it records a run that supplied no input.

## Fields

The grammar has seven fields and no others. This table is the whole field set.

| **Field** | Width | Charset | Rule |
|---|---|---|---|
| version | 1 byte | `1` | the only version this document specifies |
| scenario | 1 to 48 bytes | `[a-z0-9-]` | the recorded scenario id; informational, see below |
| profile | 1 to 64 bytes | `[a-z0-9-]` | the recorded profile id; informational, see below |
| content | 7 bytes | `project` or `user` | the content class of the run's material |
| at_tick | 1 to 19 digits | `[0-9]` | nondecreasing across events; at most 4611686018427387903 |
| word c0 | 8 bytes | `[0-9A-F]` | the `c0=` field: control word of PSG port 0; bit 31 zero |
| word c1 | 8 bytes | `[0-9A-F]` | the `c1=` field: control word of PSG port 1; bit 31 zero |

There is no field able to hold a content digest, an image name, a length, a frame byte, an audio
byte or a path, and the grammar cannot express one. Field by field: `version` is a single decimal
digit; `scenario` and `profile` are slugs over `[a-z0-9-]`, so no `/`, `.`, `:`, `=` or `_` is
expressible and the only separator is a hyphen; `content` is one of two fixed words; `at_tick` is
a decimal of at most 19 digits; `c0` and `c1` are eight upper-case hex digits each. A name is at
most 48 or 64 characters and a tick at most 19 digits, so no 32-character hex sequence and no
path is expressible anywhere in the file (D-15).

The absence is a property of the grammar, not a promise about the writer, and this document does
not enforce it. It is enforced in two other places: the writer in `runner/ivsl.c` emits only the
seven fields above, and the check `runner.record.no-content-digest` scans a produced log for a
32-character hex sequence and for an all-digit slug.

The `scenario` and `profile` values are **informational**. V1 does not compare either value with
the scenario being replayed, and a log whose recorded id differs from the scenario's id is not
refused. The recorded disposition of research open questions O-1 and O-2 is in "Identity".

## Refusals

A log is refused with exactly one token from this table, and the token names the line it judged.
A refusal produces no report, leaves the caller's output untouched, and is decided before any
machine instance or output file exists: the log is read and judged after the scenario is parsed
and before the machine is created, so "no state change" is a property of the order rather than an
assertion.

| Token | Trigger | Line named |
|---|---|---|
| `unknown-directive` | the first token of a line is not one of `ivsl`, `scenario`, `profile`, `content`, `at_tick` | that line |
| `version` | the first line is `ivsl` and its version token is not exactly `1` | line 1 |
| `arity` | a line-shape fault: the wrong number of tokens on a line; a header directive missing, duplicated or out of order; a name outside `[a-z0-9-]` or longer than 48 (`scenario`) or 64 (`profile`); a `content` value that is neither `project` nor `user`; an `at_tick` with a sign, a `0x` prefix or a leading zero | that line |
| `hex` | a `c0=` or `c1=` value that is not exactly eight upper-case hex digits | that line |
| `trailing` | a file-shape fault: a byte after the last complete line; a line not ending in LF; a CR, a NUL or a byte outside printable ASCII | the last line before the offending byte |
| `tick-order` | an `at_tick` below the `at_tick` of the previous event line | that event line |
| `tick-bound` | an `at_tick` above 4611686018427387903 (2^62 - 1) | that event line |
| `event-cap` | a 65th event line | that event line |
| `controller-bit` | bit 31 set in `c0` or `c1` | that event line |

`arity` and `trailing` each carry an enumerated set of triggers and neither means "something was
wrong somewhere". `arity` covers the line's shape and `trailing` covers the file's shape; every
other token covers one value on one line. A token that meant "the end of the file, or maybe the
middle of a line" could not be asserted as a distinct finding, which is why the table has nine
rows rather than seven (A-11-02).

## Limits

- **64 events.** The event cap is `IVS_SCN_MAX_INPUTS`, the same constant the scenario grammar
  enforces on its own `input` lines. The recorder can never produce more events than the scenario
  supplied, so the log's cap is the scenario's cap and not a new number.
- **4096 bytes.** The largest well-formed log is 3,482 bytes, and 4096 is that figure plus 614
  bytes of slack. A file above 4096 bytes is refused `trailing` before its directives are judged.
  The exact counts are 3482 and 614, and both are derived below.
- No separate line cap is needed. The field widths bound one event line to 52 bytes and one
  header line to 73 bytes, so a longer line is already refused `arity` or `trailing`.

The two byte figures are derived, not chosen.

The largest header is 154 bytes:

```
  7  ivsl 1            (6 + LF)
 58  scenario <48>     (9 + 48 + LF)
 73  profile <64>      (8 + 64 + LF)
 16  content project   (15 + LF)
154
```

The longest event line is 52 bytes:

```
  8  at_tick
 19  the largest tick, 4611686018427387903
  1  space
  3  c0=
  8  eight hex digits
  1  space
  3  c1=
  8  eight hex digits
  1  LF
 52
```

Sixty-four of those lines is 3,328 bytes, so:

```
 154  largest header
3328  64 event lines at 52 bytes each
3482  largest log
 614  unused bytes under the 4096 cap
```

A realistic log is much smaller. The log for `demo-lamp-tone-pass` is 319 bytes: a header of 85
bytes and six event lines of 39 bytes each (234). Its header is `7 + 29 + 33 + 16`, where the
`scenario` line is 29 bytes (19-character id) and the `profile` line is 33 bytes (24-character
id, `intv-2609-ntsc-family-p1`).

## Profile scope

[INF] The core's applied-event chain hashes four controller words per event:
`h0 = SHA-256("IVS-INPUT-LOG-V1")` and
`h(n+1) = SHA-256(h(n) || be64(at_tick) || be32(controller[0..3]))`.
An `ivsl` v1 event line carries two of those four words.
The other two are provably zero on `intv-2609-ntsc-family-p1`, whose
`fields.input.controllers.value` is 2, and the core refuses a nonzero word at or above that index
(`UNSUPPORTED`, `INPUT_EXTRA_CONTROLLER`). So v1 reproduces the chain exactly on this profile.

This is a property of the format and not a note about one implementation: `ivsl` v1 is lossless
for a profile with two connected controllers, and a profile with three or four connected
controllers needs `ivsl` v2 with `c2=` and `c3=`. A reader must not treat v1 as lossless for every
profile, and a later profile change that adds a controller is a version bump rather than a
silent loss.

## Rights

The log is an **input artifact** (D-15). It is ASCII, it holds input events and ticks and nothing
else, it has no content-digest field, and it carries a `content project|user` line instead.
`ivsl` is not in `tools/check/binary-ext.txt` and not in `.gitignore`: the file is text, and an
operator may keep one without it being silently dropped by a checkout.
A produced log is operator output and is never a tracked artifact. The runner writes it to a path
the operator names, and no `.ivsl` file is committed to the repository.

The check `runner.record.no-content-digest` scans a produced log for a sequence of 32 or more hex
digits and for a slug that is all digits. What it can prove: a well-formed v1 log carries no such
sequence, because the only hex fields are eight digits each and the only decimal is a tick of at
most 19 digits. What it cannot prove by itself: that the scan would notice one, since no
well-formed log can carry it. The check is therefore shown able to fail by a committed stand-in
that does carry such a sequence (A-11-05), and the all-digit-slug branch closes the hole a
hex-only scan would leave.

## Identity

Identity binding is not in the format. An operator who wants a log bound to a content image puts
an `expect-sha256` line in their own untracked scenario, where the scenario grammar and the run
already judge it. A digest field here would make the log derived material under D-11, which is
the one outcome the format exists to avoid (D-15).

A mismatch refusal on `scenario` or `profile` is **deferred**, not forgotten (research open
questions O-1 and O-2). The reason: it is an operator-error guard and not a property of the
format; the process case's stripped copy keeps its `id` line, so no case needs it; and adding it
later is a reader-side change that needs no version bump, because no field changes. Until then
both values are informational, as the field table says.

## Appended amendment A-11-V1 (Phase 11 verification, 2026-11-25): one sentence above is too strong

The paragraph on this page that says *"no 32-character hex sequence ... is expressible anywhere in
the file"* is **false as written, and it is left as written** because this document is a frozen
oracle (GUARDRAILS §2.10, lesson 21). The correction is this note.

**Measured:** `scenario 832906ce775d32a98ae607386ce4bbcd` is a legal value — `scenario` is a slug
over `[a-z0-9-]`, and a slug of 32 lower-case hex characters satisfies it. The runner records it,
and a produced log then contains exactly one run of 32 or more hex digits.

**The rights property still holds, and this is why the sentence's conclusion survives its own
premise failing.** The value in that field is the **operator's own scenario id**, written by the
runner from the file the operator named. It is never anything the images produced: no digest, no
frame byte, no audio byte, no image name, no length, no path. D-15's requirement is that the log
hold nothing derived from content, and it does not.

**What `runner.record.no-content-digest` therefore checks, stated accurately:** zero sequences of 32
or more hex digits **outside the two slug fields**, which it masks by rule, plus the stand-in that
carries such a sequence and must be caught. The check's meaning is unchanged; only the sentence that
described it was overstated.

**No format change, no code change, no version bump follows.** The grammar is unchanged, the
version stays 1, and no reader or writer moves. A future revision of this document should replace
the overstated sentence with this one, and may then delete this note.

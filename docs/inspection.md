# The inspection surface

`ivs-inspect` is the project's build-only inspection instrument: one binary, four subcommands, no window,
never installed, driven by a script only. It is built over seams the core already exposes and it adds none:
no new core function, no new sink, no file access inside the core, no installed file. Its output is an
operator-local, rights-bearing artifact, and this page refuses it inside the work tree. The page mints no
claim and cites none: every sentence below is a condition the implementation can be held to, and a later
plan whose prose disagrees with one of them is the recorded deviation rather than the page.

This page is the oracle of plan 18-01. It is committed before any path under `tools/inspect/` exists, and
the ordering is the whole of its evidentiary value: the command surface, the record fields, the report keys,
the refusal tokens, the liveness guards and the disassembly corpus are frozen here first, so a later plan
implements them rather than inventing them while implementing.

## What it is built on

Five seams, each named with the file that declares it.

- `ivs_machine_peek` and `ivs_machine_cpu_state` (`core/include/ivs/ivs_run.h`): public and side-effect-free.
  The `step` subcommand observes guest state through these two and nothing else, so reading a word advances
  no time and changes no digest.
- `ivs_machine_state_digest` (`core/include/ivs/ivs_machine.h`): public. It computes one SHA-256 per domain,
  and the nine domains the report prints are CPU, MEM, IMAGES, INSTANCE, COMPOSITE, VIDEO, AUDIO, INPUT and
  PSGLOG. The equality in `## The reconciliation` is byte equality against the report's own numbers, not a
  proxy for them.
- `ivs_machine_step_unit_trace_internal` (`core/src/machine/machine.h`): internal, declared under
  `core/src` and nowhere under `core/include`, which is the one reason the tool links `ivs_core_internal`.
  It steps one unit and fills a caller array of at most `IVS_CP1610_MAX_ACCESSES` accesses; the `psg`,
  `stic` and `machine` suites already drive it through that same header. The machine state, the log and the
  samples do not depend on the array it fills.
- `ivs_input_apply_due` (`core/src/machine/machine.h`): internal, and it is the core's own input floor. The
  trace loop calls it before each unit rather than re-deriving a "next event at or before this unit" rule of
  its own.
- the runner library: `ivs_scenario_parse` and `ivs_runner_execute` (`runner/ivs_runner.h`) read the
  scenario and produce the report. The report's bytes arrive through the host's `write_report` callback,
  which is the tool's only view of the report and therefore its oracle. `ivs_runner_report_write` is the
  library's own writer behind that seam and is never called by the tool, because it takes the record
  `ivs_runner_execute` does not return.

The report pass and the trace pass are forced, and this is the sentence that says why. The report comes
from the library,
because a second report writer would be a second truth. The trace comes from the tool's own per-unit loop,
because the executor steps through the public run call, which carries no access log, and its state struct
is static inside `runner/exec.c`, so its live instance is not reachable from a second `main`. The two
therefore cannot come from one call, and the reconciliation below is what makes them provably the same run.

## Command surface

One binary, `ivs-inspect`, with four subcommands and no fifth name. A fifth name is a usage error, never a
quiet alias. Each subcommand takes its arguments only as written here.

```
ivs-inspect trace <scenario> --content-dir <dir> --out <file.ivst> [--report <file>] [--break <HHHH> --other <HHHH>]
ivs-inspect break --pc <HHHH> --other <HHHH> --trace <file.ivst> --out <file>
ivs-inspect dis <listing> --out <file.asm>
ivs-inspect step <scenario> --content-dir <dir> --units <n> [--peek <HHHH> <count>]
```

Three exit statuses, and only three.

- 0 when the work is done: the output file exists and the report is written.
- 1 for a named refusal. Exactly one stderr line, prefixed `ivs-inspect: ` and carrying the token; nothing
  on stdout; and no output file left behind, whether the refusal came before the create or after it.
- 2 for a usage or file error, which carries no token: an unknown subcommand, a missing or repeated
  argument, an unreadable scenario or listing, an argument that is not four hex digits where four are
  required.

`--report <file>` makes `trace` reconcile against a stored report instead of the one this run prints; the
stored report and the run's own captured text must agree, so a stored report that disagrees is refused.

`--help` is part of the surface and is pinned here by name. It prints the four subcommand names and their
grammars on stdout, prints no refusal token, writes no file and exits 0.

The `step` report is one `key value` line each, in this order: `step.units`, `step.ticks`, `step.halted`
(`yes` or `no`), `step.pc`, then `step.reg.R0` through `step.reg.R7`, then one `step.mem.<AAAA>` line per
peeked word in ascending address order. Addresses and values are four upper-case hex digits; counts are
decimal.

## The trace file

The trace is ASCII text with LF line endings and one record per line, in a file whose extension is `ivst`.
Fields are decimal except `addr` and `value`, which are four upper-case hex digits. There are exactly three
record kinds.

An access record, one per bus access, with `tick=` first and then the remaining seven fields in this order:

```
tick=<n> kind=<kind> addr=<HHHH> value=<HHHH> wait=<n> cycle=<n> unit_start=<n> unit_end=<n>
```

`kind` is one of the nine `IVS_ACC_*` names of `core/src/cpu/cp1610.h`, lower-cased, with the two strobes
hyphenated: `fetch`, `read`, `write`, `adar`, `intak`, `iab`, `dw`, `strobe-tci`, `strobe-pcit`. `tick` is
`unit_start + cycle * ticks_per_cpu_cycle`, with the multiplier read from the profile's time base and never
written as a literal. `value` is the value transferred, and is `0000` for the kinds that transfer none.

A step record, one per run-to, with `k` counting from 1:

```
step=<k> target=<n> end=<n> stop=<target|halted|faulted>
```

An end record, last in the file, and the only record that is not an observation:

```
end tick=<n> records=<n> units=<n> steps=<k> break_pc=<HHHH|none> break_hits=<n> other_pc=<HHHH|none> other_hits=<n>
```

`records` counts access records only, so it is a real number to assert even though `end` is itself a line.
`units` counts machine units stepped, `steps` counts step records, and the two breakpoint fields carry
`none` when the corresponding option was absent.

The one genuine ambiguity in the frozen criterion is resolved here, once. The criterion's "the last
record's tick" is the **`end` record's** tick. That tick equals the last step record's `end`, which equals
the report's own `step.<k>.now_ticks` for that step. An access record keeps its own cycle stamp, so the
trace is cycle-stamped and the criterion's equality is literally true rather than approximately true. The
alternative reading - make every record's `tick` the unit's end - was refused because it loses the
per-access cycle stamp that the requirement names, and a trace whose every record carries one unit stamp
asserts nothing about the cycle an access began in.

## The reconciliation

The tool requires five equalities before it reports success, and it prints its trace only after all five
hold.

1. The nine `digest.*` lines of its report are byte-identical to `ivs-run`'s for the same scenario and the
   same options, because both come from the same library over the same bytes.
2. The nine digests of the traced instance, computed with `ivs_machine_state_digest`, equal the nine the
   report printed. This is what makes the two sweeps provably the same run rather than two near-identical
   ones.
3. Each step record's `end` equals that step's `step.<k>.now_ticks` in the report.
4. The end record's `tick` equals the last step record's `end`.
5. Access ticks are non-decreasing **and not all equal**: at least two distinct tick values must appear.
   Monotonicity alone is satisfied by a trace that repeats one tick and therefore asserts nothing, so the
   committed stand-in that repeats one tick is judged by this fifth equality.

A failure of any of the five is the refusal `inspect-reconcile`, with exit 1 and no output file: never a
warning, and never a printed trace. A trace that disagrees with the report is worse than no trace, because
it would be read as an observation of the run it disagrees with.

## Breakpoints

A hit is an access at the named address whose `kind` is `fetch` or `iab`. Both must count: the reset entry
is an `IAB` carrying the reset flag, not a `fetch`, so a rule that names only `fetch` reads the entry as
never executed. The matched access is reported once per access; a second pass over the same address inside
one unit counts again.

One report prints the hit count at `--pc` and the hit count at `--other`, so a matcher that always hits
cannot satisfy both. Arming a breakpoint changes no access record and no digest: that non-interference is a
separate equality from the trace's own and is asserted separately, because a later implementation that
re-stepped the machine under a breakpoint would break it silently.

The break report is one `key value` line each, in this order: `break.pc`, `break.hits`, `break.other.pc`,
`break.other.hits`, `break.records`, `break.end_tick`, `break.steps`. Addresses are four upper-case hex
digits and counts are decimal. The last three are copied from the trace's own `end` record, so the two
documents can be compared rather than trusted.

## Disassembly

The corpus is the project's own listings, registered in `data/expected/listings.ids`; there is no other
corpus and no second decoder table. The mnemonic is the `ROW` column of a word line, and the operands are
the following `-` rows plus the field groups of the `BITS` column, so the table the disassembler reads is
the one the listings and the assembler already pin. A row the tool does not know is the refusal
`inspect-dis-row`, which is fail-closed and is falsifiable the moment the corpus grows.

The oracle is the re-assembler: the emitted source is assembled by `ivs_asm` and the comparison is the
`ivs_pack --dump-words` word dump of both listings, never a comparison of listing text. The round trip is
`words > 0` and byte-identical words, with at least one two-word form and at least one immediate present in
the emitted source, so a `DECLE`-everything emitter - which round-trips perfectly and disassembles nothing -
cannot satisfy the case.

The derivation is pinned here, because a cut the implementer has to guess is a cut that silently emits a
source assembling to different words.

- The emitted source's header is the listing's leading run of `# key: value` lines whose key is one of the
  eight `ivs_asm` accepts - `id`, `kind`, `contract`, `profile`, `window`, `label`, `entry`, `stack` -
  copied verbatim. Every other `#` line is dropped, because the assembler's header block ends at the first
  line that is not one of those eight: a bare `#` or a prose line inside the block is refused as
  `asm-header`, not read as a comment.
- The operand text is the listing's own `ASM` column with the trailing note cut at the first run of two or
  more spaces, and the branch or jump target annotation cut at ` -> `. The leading token is dropped only
  when it repeats the row field; where it does not, the `ASM` column on that line is a note and the
  mnemonic stands alone.
- An `ORG $HHHH` directive is emitted before the first instruction and wherever the address is not the
  previous instruction's address plus that instruction's word count, so a gap in the listing is preserved
  rather than silently packed.

The limb may be dropped, and only on this condition: it is dropped if it cannot round-trip a corpus that
contains a two-word form and an immediate inside the priced line. When that happens this section is amended
by an appended note reading that the limb is not built and that a reader uses the committed listings named
in `data/expected/listings.ids`. Dropping the limb for schedule reasons alone is not authorised by this
page, and a drop recorded without the appended note is a plan deviation, not an amendment.

## Refusal tokens

Five tokens, each as the first text on its own line, with the condition that raises it and the boundary
that must still be accepted. A later id reads this block between its two headings and compares it with the
tokens the tool's source can print, so each token appears exactly once here and this block holds nothing
else of that shape.

`inspect-output-tree`
An output whose physical parent lies inside the work tree and not under `refs/` is refused, whatever the
spelling of the path - literal, with `..`, or through a symlink. An output outside the tree, and any path
under `refs/`, is accepted.

`inspect-output-exists`
A path that exists is refused, a dangling symlink included, and the refused invocation leaves the existing
path byte-identical. A path that does not exist is created exclusively.

`inspect-reconcile`
One of the five equalities of `## The reconciliation` fails. An invocation that satisfies all five writes
its trace and exits 0.

`inspect-break-empty`
`break` is given a trace with zero access records. A trace with one access record yields a count of 1 or 0
and is not refused.

`inspect-dis-row`
A listing row the disassembler does not know is refused. Every row the boot-stub corpus uses is decoded.

## Liveness guards

Four guards, each with the committed stand-in that shows the guard can fail.

- The end-tick equality against the report's own `step.<k>.now_ticks`. The stand-in is a trace that steps to
  the scenario's target instead of to the stop: it prints 16000 for a run that ended at 2084, and the
  equality fails by name.
- The nine-digest equality of the traced instance. The stand-in is a tracing pass that pokes one register;
  it diverges on exactly one domain, and the refusal names that domain.
- The breakpoint pair's zero case. The stand-in is a matcher that always hits; it reports 1 where the answer
  is 0, and the pair fails on its second number.
- The round trip's `words > 0` with at least one two-word form and one immediate. The stand-in is an emitter
  that writes `DECLE` for every word; it round-trips perfectly and disassembles nothing, so it fails the two
  presence assertions rather than the word comparison.

## Rights

The output is covered by one `data/rights/` row with disposition `owner-local-only`, source `null` and
nothing taken, together with its line in `data/expected/rights.ids` and its row in `docs/RIGHTS.md`. The
extension `ivst` is refused by name in `tools/check/binary-ext.txt` and in `.gitignore`, so a trace cannot
be committed by accident in either mechanism. An output inside the work tree is refused except under the
ignored `refs/`.

Nothing the tool writes is ever committed - not a golden, not a fixture, not a CI artifact, not a claim
artifact - whatever its content, and no claim may cite one.

## What this instrument is not

It is not a frontend and it opens no window. It adds no core function, no sink, no file access in the core
and no installed file. It holds no second decoder table. It mints no claim: what it produces is
repeatability information for an operator on one build, and it is never evidence about any guest program.

Its consumer is Phase 19, which uses it to decide a display-path default by discrimination: an observation
that moves under one profile field and under no other, at the default of the remaining fields.

## Amendments

Appended, never edited in place: 18-01's freeze check is a byte-prefix comparison, so no sentence above may
move. Each note names the section it corrects, quotes the sentence it supersedes where there is one, and states
what shipped.

### Amendment 1 (plan 18-03, 2026-11-27): the operand derivation

The second sentence of `## Disassembly` reads "the operands are the following `-` rows plus the field groups of
the `BITS` column". That sentence is superseded by the pinned derivation list in the same section, which is the
rule `tools/inspect/inspect_dis.c` implements: the operand text is the listing's own `ASM` column cut by the
three pinned rules, in the pinned order. The `BITS` groups are consumed only to locate the `ROW` field, and a
`-` row only to count an instruction's words; neither contributes an operand. The pinned list is authoritative
because it is pinned for the reason it states - a cut the implementer has to guess is a cut that silently emits
a source assembling to different words.

### Amendment 2 (fix pass, 2026-11-27): `inspect-break-empty` is decided on the records present

`## Refusal tokens` pins the trigger as "`break` is given a trace with zero access records" and the boundary as
"A trace with one access record yields a count of 1 or 0 and is not refused". `break` now decides on the access
records it actually parses, never on the `end` record's declared `records=` field: a trace that declares
`records=5` and holds no access record is refused, and a trace that holds one access record and declares
`records=0` is accepted with its own count. The declared field stays the number `break.records` copies, so the
report's copy and the file's record count are two independent computations of one number rather than one
number checked against itself.

### Amendment 3 (fix pass, 2026-11-27): `## Liveness guards`

The list above opens "Four guards", and its first entry named a stand-in that did not exist when this page was
frozen: "a trace that steps to the scenario's target instead of to the stop: it prints 16000 for a run that
ended at 2084". That stand-in now exists, as a second limb of `runner-process.inspect.trace-state`: the real
trace with the step record's `end` and the end record's `tick` both set to 16000, judged by the same end-tick
equality the real case uses, so the stand-in is internally consistent and only its disagreement with the
report's own `step.1.now_ticks` refuses it. Two further corrections to the same list:

- The `DECLE`-everything entry says the stand-in "fails the two presence assertions rather than the word
  comparison". The committed `asm.judge.dis-constant-mnemonic` fails the **word comparison**, and its case
  enforces that: it reports `judge-vacuous` when the word dumps agree on the stand-in and `judge-wrong-reason`
  unless the reason begins `word dumps differ`. The presence assertions are still asserted on the emitted
  source, but they are not the clause the stand-in defeats.
- The list is five, not four. The repeat-tick stand-in of `## The reconciliation`'s fifth equality,
  `runner-process.judge.trace-repeat-tick`, is a guard with a committed stand-in and belongs in the list beside
  the other four.

### Amendment 4 (plan 18-03, fix pass 2026-11-27): the registered listing that is refused

`## Disassembly` opens "The corpus is the project's own listings, registered in `data/expected/listings.ids`",
which reads as a promise that the whole corpus decodes. Six of the seven do. `ivs-diag-stic-2` is refused,
fail-closed, with exactly

```
ivs-inspect: inspect-dis-row: the row DATA at 512E is not one of the mnemonics ivs_asm accepts
```

because its `DATA` row is not one of the mnemonics `ivs_asm` accepts. That is the operative boundary: the
`## Refusal tokens` sentence "Every row the boot-stub corpus uses is decoded" is narrower than the corpus
sentence above it, and it is the one that holds. A data row is refused by name and address rather than emitted
as a guess, which is the fail-closed behaviour the same section requires.

### Amendment 5 (fix pass, 2026-11-27): `## The reconciliation`, equalities 1 and 4

The section says the tool "requires five equalities". Equalities 2, 3 and 5 are the tool's own refusals.
Equalities 1 and 4 hold by construction and are asserted on the artifacts rather than computed by the tool:
equality 1 is structural, because the report the trace is reconciled against is the library's own bytes, and it
is asserted by `runner-process.inspect.trace-digests`; equality 4 is one number written into two records of the
trace, and it is asserted by the end-tick comparison in `runner-process.inspect.trace-records`. Neither is a
weaker claim for being asserted outside the tool: both are byte comparisons on the two documents, not proxies.

### Amendment 6 (fix pass, 2026-11-27): the breakpoint pair's precondition

`## Breakpoints` says "One report prints the hit count at `--pc` and the hit count at `--other`, so a matcher
that always hits cannot satisfy both". That sentence holds only when the two addresses differ. The surface
accepts `--pc` equal to `--other` - neither `trace` nor `break` refuses it, and the exit-status 2 list above
does not name such a refusal - and under two equal addresses a matcher that always hits satisfies both numbers.
The pair is therefore an operator precondition of this page, not a condition the tool enforces: a
discrimination invocation names two different addresses, as every committed case does.

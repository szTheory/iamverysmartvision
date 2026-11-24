pending: blind read not yet run

# Hidden-state checklist blind second-read request

State: this request is written in plan 08-09, the oracle plan of the snapshot work, which precedes every implementation plan of it. The blinded copy is built and the reader started by the orchestrator after that plan, and no reader output is committed in plan 08-09. Plan 08-10 (the walker refactor) starts only when the record named below exists and its first line reads `state: closed`; it applies every `correction:` line of the record to the oracle data (the checklist, the row descriptors and the pinned digest) in its own first commit.

## What is to be blind-read (D-05)

One job, one fresh isolated reader, on the question that the hidden-state checklist answers: which members does the machine instance have, and what is each one for a snapshot?
The reader receives the struct headers and the class definitions and nothing else (see the inputs below). It never sees the digest field table, the checklist, the schema literal or any plan file, so its member list is independent of the table that the checklist was derived from.
The reader produces one file with one line for every scalar and every array member that is reachable from `struct ivs_machine` (an array counts once, as one member; a member of a nested struct is named by its path from the instance, for example `cpu.r`, `mem.sysram`, `obs.input.words`, `slot[0].sha256`), in this form:

`<member path> | <class> | <situation>`

- class is one of the four classes of the definitions: STATE, IMAGE_BEARING, BOUND or PAD.
- for a member the reader calls STATE, the situation is one concrete case in which the member is not at its power-on value and a later step or a delivered output depends on it; for the other classes the situation is the word none.
- a member that the reader cannot classify from the headers and the definitions is listed with the class `unclear` and the reason, never guessed.

## Inputs of the reader (exhaustive)

1. The five blinded header copies and the class definitions, made by the orchestrator with these commands and placed under `build/review/blind/` (gitignored). The filter removes every line that mentions a digest, a snapshot, serialization or a checklist, so a comment that names the table or a class of a member cannot reach the reader; a line deleted inside a block comment only shortens the comment.

```
mkdir -p build/review/blind
F='/[Dd]igest/d;/[Ss]napshot/d;/[Ss]erializ/d;/checklist/d;/hidden state/d'
sed -n '/^\/\* One image slot (kind fixed at create/,/^_Static_assert(_Alignof(struct ivs_machine)/p' core/src/machine/machine.h | sed "$F" > build/review/blind/machine-structs.h
for h in cpu/cp1610.h psg/psg.h stic/stic.h memmap/memsys.h; do sed "$F" core/src/$h > build/review/blind/$(basename $h); done
sed -n '/^<!-- classes-begin -->$/,/^<!-- classes-end -->$/p' docs/formats/ivsn-v1.md > build/review/blind/classes.md
```

   The first range keeps the structs of `machine.h` (the image slot, the observation rows, the instance struct and the layout assertions) and leaves out the block that describes the digest. The reader opens these six files, `build/review/blind/machine-structs.h`, `cp1610.h`, `psg.h`, `stic.h`, `memsys.h` and `classes.md`, and nothing else.
2. This request, for the output form.

Nothing else may be opened, searched or recalled. Forbidden: the digest field table `core/src/machine/machine_digest.c`, the checklist `data/expected/state-checklist.ids`, the schema literal `data/state/schema.literal.txt`, everything else in `core/src/machine/` other than the first file above, every other file of `core/`, `runner/`, `tests/`, `tools/` and `data/`, `docs/state/` (the checklist narrative and the hard points), the rest of `docs/formats/ivsn-v1.md`, `.planning/`, the git history of any path, the output of any other reader, any emulator, assembler or disassembler, its source or its output, and web searches. The general reading rules are those of `docs/stic/second-reader-protocol.md` (an isolated reader; the record says `source-expectation, second-read`); the inputs of this job are the ones listed here.

## Comparison and record

The orchestrator compares the reader's member list with the 88 rows of the checklist, and the classes with the checklist's, and writes the record `docs/state/checklist-second-read-record.md`.
- A member of the reader's list that no row of the checklist covers is a member the digest table misses, because the checklist has one line for every table row: that is a finding about the core, reported and decided by the orchestrator before the record is closed. A row that no reader member covers is listed (a pad or a slot row that the reader folded into a neighbour is expected and is not a correction).
- A class that differs from the checklist is resolved from the class definitions and the header text alone; the resolution is either a `correction:` line (the checklist was wrong) or a note (the reader was wrong).
- The slot rows of the checklist (`slot.exec.kind` and the like) are the members `slot[0]` to `slot[2]` of the reader's list; the names are matched by position and field.

The record says `source-expectation, second-read` and never `independent-implementation-comparison`, because the same model family reads the same headers and a shared misreading stays possible.
The head of the record is fixed here and tested by command by plan 08-10 before its first edit:
- line 1 is exactly `state: closed` (or `state: open` while any disagreement is unresolved);
- line 2 is `oracle-commit: <40 lower-case hex digits>`, the oracle commit named in the plan 08-09 SUMMARY (the last commit of that plan);
- line 3 is `corrections: <n>`;
- then exactly n lines `correction: <row> <field> <old> -> <new>`, one per change that the reading forces in the checklist (field class, witness, test or sweep) or a line `correction: <member> row-missing -> <what the table needs>` for a member the table misses;
- then the prose (date, reader label, what was read, how many members agree, each disagreement with its resolution).

A correction to the checklist, to the descriptor text of the rows (its class words) or to the pinned schema digest of `data/expected/ivsn-literals.ids` is committed in the same commit that adds the record or in the first commit of plan 08-10, and nowhere else. That commit is the frozen oracle baseline for every later plan: from then on the Classes section of `docs/formats/ivsn-v1.md`, the checklist and the descriptor text change only by an appended note. If the record is `state: open`, plan 08-10 halts.

## Sources and rights

The reader uses no source other than the inputs above; the headers are project-authored. Nothing in this request or in the record reproduces text of a datasheet, a manual or a catalog.

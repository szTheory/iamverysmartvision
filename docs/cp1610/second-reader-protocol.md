# CP-1610 vector second-reader protocol

This protocol is for an isolated reader that re-derives the expectations of one CP-1610 vector file, and for the orchestrator that starts those readers.
It holds no vector id and no expected value.
Page cites are "pdf N" of SRC-gi-cp1600-um-1975, the General Instrument CP-1600 Microprocessor Users Manual of May 1975.

## Purpose

An isolated reader re-derives the expectations of one vector file from the page images, without seeing the committed expectations, ids or citations.
Plan 02-17 diffs the reader's result against the committed file mechanically and resolves every disagreement from the page (D-15).
The record of this reading says `source-expectation, second-read` and never `independent-implementation-comparison`, because the same model family reads the same pages, so a shared misreading stays possible (C43).

## Allowed inputs

This list is exhaustive (A-12).
1. The reader's own blinded copy `build/review/blind/<stem>.json`.
2. The page images `refs/pages/*.png` and the manual itself, `refs/SRC-gi-cp1600-um-1975.pdf`; a page or crop that is missing may be rendered with `pdftoppm -r 110` or `pdftoppm -r 200` into `refs/pages/`.
3. This protocol.
4. The policy-statement document `docs/cp1610/second-reader-policies.md`.

Nothing else may be opened, searched or recalled for a value.

## Forbidden inputs

- Everything under `.planning/`, in particular `.planning/phases/02-cp-1610-cpu-core/02-CONTEXT.md` and every PLAN and SUMMARY file, because they state access offsets, cycle counts, encodings and values.
- `docs/cp1610/reference-notes.md`, for the same reason.
- `docs/cp1610/stepper-conventions.md`, for the same reason.
- Everything under `data/`: the vector files, the expected lists, the discrepancy log, the profiles, the family table and the second-reader records.
- Everything under `core/`, `tests/`, `cmake/` and `tools/`.
- Anything under `build/` except the reader's own blinded copy and its own output path.
- The output of any other reader.
- The git history of any path: log, show, diff and blame.
- Any emulator, assembler or disassembler, its source or its output, including other repositories on the machine.
- Web searches for expected values.

## Output

The reader writes exactly one file, `build/review/derived/<stem>.json`, once, at the end of its work.
Its shape is:

```json
{
  "schema_version": 1,
  "file": "<stem>",
  "blind_sha256": "<sha256 of the blinded copy that was read>",
  "reviewer": "isolated-subagent-blind-v1",
  "vectors": [
    {
      "index": "<index of the blinded copy>",
      "expect": { "...": "delta over init, schema below" },
      "bus": [["<cycle>", "<kind>", "<addr>", "<data>"]],
      "derivation": "<page and arithmetic paraphrase>"
    }
  ]
}
```

- `blind_sha256` is the digest printed by `cmake -E sha256sum` for the blinded copy that was read.
- `vectors` holds one entry per `index` of the blinded copy, and no other entry.
- `expect` is a delta over `init`: it lists every field whose value after the last step differs from `init`, and may list unchanged fields.
- `bus` is given when the blinded vector has `has_bus` true, and is otherwise omitted.
- `derivation` paraphrases the page and shows the arithmetic; it quotes at most short labels of the page.

The `expect` schema, in full:
- `r`: an object whose keys are `"0"` to `"7"` and whose values are four-digit hex words; R6 is the stack pointer and R7 the program counter.
- `status`: an object with keys `S`, `Z`, `OV` and `C`, each 0 or 1.
- `intff`, `dbd`, `prev_interruptible`, `halt_pending`, `busak`: each 0 or 1.
- `run_state`: one of `UNRESET`, `RESET_PENDING`, `RUNNING`, `HALTED`, `FAULTED`.
- `fault_reason`: one of `NONE`, `UNSUPPORTED_SDBD_TARGET`, `UNSUPPORTED_MVO_SSS_EQ_MMM`, `UNSUPPORTED_JUMP_DE`, `BUS_WAIT_LIMIT`.
- `mem`: an array of `["<addr>", "<word>"]` pairs of four-digit hex words, one per changed memory word.
- `cycles`: the total number of CPU cycles over all steps of the vector.
- `units`: one unit kind per step, in order, each one of `INSTR`, `INT_ENTRY`, `RESET_ENTRY`, `BUS_GRANT`, `HALTED`, `FAULT`, `NOT_RESET`.
- `events`: the events that occurred over all steps, each one of `TCI`, `PCIT`, `HALT_LATCHED`, `STOPPED`, `FAULT`, `BUS_RELEASED`.

Event meanings: `TCI` and `PCIT` mark the strobe of that name, `HALT_LATCHED` marks HLT setting the stop latch, `STOPPED` marks a step that entered the halted state, `FAULT` marks a step that set a sticky guest fault, and `BUS_RELEASED` marks a step that released a granted bus.

Each `bus` entry is `[cycle, kind, addr, data]` in order of occurrence, where `cycle` is the address-phase cycle counted from the start of the vector (across all steps), `kind` is one of `FETCH`, `READ`, `WRITE`, `ADAR`, `INTAK`, `IAB`, `DW`, `STROBE_TCI`, `STROBE_PCIT`, and `addr` and `data` are four-digit hex words.
A bus entry is recorded for every access that is not an idle cycle; idle cycles are the gaps between entries.
An `IAB` entry carries address `0000` and the answered vector as data, and a strobe entry carries address and data `0000`.

Test-environment conventions (no values):
- The test bus is a flat 64K by 16-bit memory that returns the stored word for every read.
- The program words are stored from `program_at` (default: `init` R7) upward, then the `init.mem` pairs are stored.
- Memory outside the program and `init.mem` is poison: a vector never reads it, so a derivation that reads it has gone wrong.
- `init` is the full starting state; a missing register is `0000`, a missing flag is 0, `prev_interruptible` defaults to 1, `run_state` to `RUNNING`, `fault_reason` to `NONE`, and every other missing field to 0.
- `init.lines` gives the levels of `intr`, `intrm`, `busrq` and the `stpst` edge for every step (each 0 or 1, default 0).
- `init.stimulus` holds the run inputs:
  - `steps` is the number of units stepped, one call each (default 1).
  - `step_lines` is an array of lines objects, one per step, used instead of `init.lines`.
  - `config` overrides fields of the default configuration for the whole vector: `interrupt_accept_clears_intff`, `release_latency_cpu_cycles`, `wait_limit_cpu_cycles`.
  - `ebci` lists the external branch condition codes that the provider answers true; without the key the provider is absent.
  - `waits` lists `["<addr>", n]` pairs: the test bus answers every access at that address, of any kind, with a wait of n cycles, and the CPU decides which access kinds take it.
  - `iab` is the word the test bus answers to an IAB access; without it no IAB access may occur.
  - `reset` set to true calls the reset function before the first step.
  - `program_at` is the address of the first program word.
- A vector that carries `policy` names the token of the policy its expectation depends on.

The reader writes nothing else, does not run the diff tool, and reports only that it finished and how many entries it wrote.

## Derivation rules

- Values come only from the page images: encodings from the format figures, results by hand arithmetic, cycle counts from the summary tables, flag meaning from pdf 45, and access offsets from the timing figures of pdf 27-37 where a figure prints them.
- Everything not listed in `expect` is asserted unchanged, so every change must be listed.
- The rules of the policy document hold for every vector wherever they apply; a vector carrying a `policy` token applies the rule under the heading of that token in `docs/cp1610/second-reader-policies.md` exactly.
- Every vector gives `cycles`; the diff ignores cycles where the committed vector asserts none.
- An ambiguous page is written down in `derivation` with the reading chosen and the alternative, never decided silently.

## Orchestrator procedure

1. Plans 02-01 to 02-16 are complete, the gate is green and the working tree is clean.
2. Run `rm -rf build/review && cmake -DROOT="$PWD" -P tools/review/vector-blind.cmake` and confirm 12 `blind <stem>` lines and exit 0.
3. For each stem, spawn one fresh subagent with no prior conversation whose prompt contains only the blinded copy path, the protocol path, the policy document path and the instruction to follow the protocol exactly and write `build/review/derived/<stem>.json`; run them in parallel batches of at most 4; pass no vector content, plan text, SUMMARY text, CONTEXT text or other readers' outputs.
4. After each batch, check that each output exists and parses as JSON; replace a failed or incomplete reader with a new fresh subagent, never by resuming the failed one.
5. Write `build/review/derived/HANDOFF.txt` with one line `<stem> <sha256> isolated-subagent-blind-v1 <run-id>` per stem, using `cmake -E sha256sum` and the run id the Agent tool returned for that subagent.
6. Resume plan 02-17, and never edit a derived file after the handoff.
7. If plan 02-17 later stops with `review-blind-mismatch` for a file, repeat steps 2-5 for that stem only, appending a new HANDOFF line that replaces the old one; for one stem, step 2 is `rm -f build/review/blind/<stem>.json build/review/derived/<stem>.json && cmake -DROOT="$PWD" -DFILE=<stem> -P tools/review/vector-blind.cmake` with one `blind <stem>` line, so the other stems' outputs and HANDOFF.txt are kept.

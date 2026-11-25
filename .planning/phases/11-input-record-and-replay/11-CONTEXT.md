# Phase 11: Input Record and Replay - Context

**Gathered:** 2026-11-25
**Status:** Ready for planning

<domain>
## Phase Boundary

A run is recorded as a **rights-clean, input-only log**, and replaying that log
reproduces the recorded run's **digest chain** and **applied-event chain** — with
the vacuity guards that v0.1.0's round-trip hook lacked.

**Not in this phase:** any content digest, image name, length, frame byte, audio
byte or path in the log format (D15 makes the log an *input* artifact); a new gate
step (38 stays 38); a new claim above `provisional`; a content-identity field
(reuse `expect-sha256` in the operator's own untracked scenario if identity
binding is wanted); file-option persistence beyond the two new runner options.

**Tier ceiling:** `deterministic-repeatability` — the same core, so never an
independent comparison.

</domain>

<decisions>
## Implementation Decisions

### The format comes first, and it is provably input-only

- **`docs/formats/ivsl-v1.md` is committed before any code**, and the ancestry is
  proved with `git merge-base --is-ancestor`. Its grammar has **version, scenario
  id, profile id, a `content project|user` line, and `at_tick <n> c0=<HEX>
  c1=<HEX>` records** — and no field able to hold a content digest, an image name,
  a length, a frame byte, an audio byte or a path. An unknown directive is refused
  with a typed token. — **Reversibility:** costly — the format is a published
  contract; a later change means a version bump and a reader that accepts both.
- **Identity binding is not in the format.** When an operator wants it, they put
  `expect-sha256` in their own untracked scenario. A digest field in the log would
  make the log derived material under D11 and would be the one thing the format
  exists to avoid.

### The two options, operator side

- **`--record-input` (exclusive create) and `--replay-input`** live in
  `runner/hostio.c`'s territory: operator files never touch the core. Exclusive
  create means an existing path is refused, not overwritten.
- **The log is text, is not in `tools/check/binary-ext.txt`, and is not
  gitignored.** A produced log is operator output, never a tracked artifact.

### The ids live in existing suites — zero new gate steps

- `runner.record.replay-equals-straight` and `runner.record.format-roundtrip` in
  process; the refusal family (unknown directive, non-monotonic tick, tick beyond
  bound, event cap, undefined controller bit, wrong version, trailing garbage)
  each with a typed token and no state change; and **exactly one** process case
  for the command line (argument parsing, `--content-dir`, exit codes).
- **Two named faults**, `IVS_FAULT_RECORD_DROPS_LAST_EVENT` and
  `IVS_FAULT_REPLAY_APPLIES_LATE` (one tick), on digest domain 18, registered
  under TOOL-01 in the same commit as the ids that detect them.

### The liveness guards are the point

- The equality case asserts **at least one event**, `input.consumed ==
  input.events`, an `at_tick` list equal to the recorded one, and **all nine
  domains observed**. Without those, a hook that does nothing matches every
  report — the exact 08-16 failure class.
- **Two judge self-controls**, each shown able to fail: `replay-differs` (one
  digest nibble changed → rejected) and `replay-inert` (a fake pair with zero
  applied events → rejected **by the guard**, not by the equality).

### Rights

- `runner.record.no-content-digest` greps a produced log for a run of 32 or more
  hex digits and requires **zero hits**. The log holds nothing the images
  produced, so under D11 it is not derived material.

### Decided autonomously
- [Format] Q: "One directive per event or one line per event?" → Selected: "One `at_tick <n> c0=<HEX> c1=<HEX>` line per event, with the header carrying version/scenario/profile/content." — a text file, reversible before the format is published.
- [Options] Q: "Where do the two options live?" → Selected: "`runner/` only; the core gains no file I/O." — a runner change, reversible.
- [Refusals] Q: "One token for all malformed logs or one per class?" → Selected: "One per class, seven of them, each pinned in an ids file." — ids files, reversible.
- [Guards] Q: "Assert the chains, or assert the chains plus a liveness guard?" → Selected: "Both, with the guard doing the work and two self-controls proving it." — test ids, reversible.

### Claude's Discretion
The plan chooses the exact token spellings, whether the process case is one id or
two, and where the `at_tick` bound comes from (a measured log-size probe sizes the
event cap).

</decisions>

<code_context>
## Existing Code Insights

### Reusable Assets
- `runner/hostio.c` already owns operator files; the two new options join it.
- `runner/exec.c` already has the round-trip hook (plan 08-16) with
  `IVS_RUNNER_OPT_ROUNDTRIP` — the seam to copy for a replay hook.
- `tests/runner/runner_exec.c` already carries the round-trip cases and the
  `runner.exec.roundtrip-cut-cap` harness failure; the record cases join it.
- `data/expected/faults/runner.ids` and `data/expected/tests/runner.ids` are where
  the two faults and the new ids are registered.

### Established Patterns
- Oracle-first with a proved ancestry (GUARDRAILS §2.9).
- Every equivalence check needs a guard that the compared thing happened, plus a
  stand-in proving the guard fails (lesson 20 — the 08-16 lesson, restated here).
- A new fault site travels with its registration (lesson 4).

### Integration Points
- `ivs_runner_options` gains two fields; `runner/main.c`'s option parser gains two
  arms at its existing `strcmp` chain.
- The digest chain is the report's nine digest lines; the applied-event chain is
  the input events the executor actually consumed.

</code_context>

<specifics>
## Specific Ideas

Frozen upstream scope: `.planning/MILESTONE-CONTEXT.md` §2 Phase 11, six numbered
deliverables, and decision **D15** (§3) — "the replay log is an input artifact:
ASCII, input events and ticks only, no content-digest field, a `content
project|user` line, not in `binary-ext.txt`, not gitignored."

The sentence that shapes the whole phase: *"A vacuous replay oracle (the 08-16
failure class)"* is the risk, and the counter is the guards, not the equality.

</specifics>

<deferred>
## Deferred Ideas

- **A content-identity field in the log** — D15; use `expect-sha256` instead.
- **Promoting the process cases from in-process to real-executable** — the D15
  reversal trigger is a defect found only at the process boundary.
- **Snapshot file options** — deferred from Phase 8, still deferred.
- **A new gate step** — forbidden this milestone.

</deferred>

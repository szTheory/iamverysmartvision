---
phase: 02-cp-1610-cpu-core
plan: 16
subsystem: testing
tags: [cp1610, second-reader, blinding, cmake-script, gate, controls, D-15, A-12]

requires:
  - phase: 02-cp-1610-cpu-core
    provides: "412 committed vectors in 12 files (plans 02-02..02-15), vectors.cmake schema, cp1610-vectors gate step, record-kind DSL (records.cmake), control runner"
provides:
  - "tools/review/vector-blind.cmake: leak-checked blinded copies with opaque indices v001.. and opaque policy tokens p01..p19 (ivs_blind_project, ivs_blind_policy_tokens, POLICY_MAP)"
  - "tools/review/vector-diff.cmake: input checks, index-to-id and token-to-policy mapping, merged-state DIFF lines and diff-summary"
  - "record kind vector-review (schema + hook) with the committed handoff record check; inventories data/expected/vector-review.ids and vector-review-handoff.ids"
  - "cp1610-vectors step runs the blind projection every gate run: stats blind-files=12 second-read=0/12"
  - "8 controls vectors-review-{stale,reviewer,unresolved,format,count,handoff-mismatch}, vectors-blind-{unknown-key,leak}; controls=190/190"
  - "docs/cp1610/second-reader-protocol.md (reader inputs, output shape, expect schema, orchestrator procedure) and docs/cp1610/second-reader-policies.md (19 rules under p01..p19)"
affects: [02-17, 02-18, second-reader, vector-review]

actuals:
  tokens: 19497
  tasks: 3
  commits: 3
plan_head_before: a20dd1f4797cf4f5ea8795cb94e424222fc32595
plan_head_after: 09a26914717670147793b8e9e391bcb29f15abe2

tech-stack:
  added: []
  patterns:
    - "Blinded projection with a canonical one-line JSON writer (sorted members) so the blind digest does not depend on the CMake version's pretty-printer"
    - "Scan-before-write: a projected text that fails the leak scan is never written, and an older copy is removed"
    - "Bare stdout lines from cmake -P through one `cmake -E echo` of a newline-joined list"

key-files:
  created:
    - tools/review/vector-blind.cmake
    - tools/review/vector-diff.cmake
    - tools/check/schema/vector-review.txt
    - tools/check/kinds/vector-review.cmake
    - data/expected/vector-review.ids
    - data/expected/vector-review-handoff.ids
    - data/expected/controls/vectors-review.ids
    - docs/cp1610/second-reader-protocol.md
    - docs/cp1610/second-reader-policies.md
    - tools/check/controls/_good/tree/data/cp1610/vector-review.json
    - tools/check/controls/_good/tree/data/expected/vector-review.ids
    - tools/check/controls/_good/tree/data/expected/vector-review-handoff.ids
  modified:
    - tools/check/steps/cp1610-vectors.sh
    - tools/check/controls/sources-no-files/MAKE
    - tools/check/controls/vectors-file-missing/EXPECT

key-decisions:
  - "Opaque policy tokens p01..p19 in ascending sha256 order of the policy ids (stricter than A-12's neutral policy id); the token map is printed only with -DPOLICY_MAP=1 and recorded below, never given to a reader"
  - "The blind leak scan searches each dropped key as a quoted string anywhere in the text (key or value), a superset of the key-only rule, plus every committed id and cite string and the literal POL-"
  - "A later HANDOFF line for a stem replaces an earlier one, in both the diff tool and the vector-review hook (the last line for the stem is authoritative)"
  - "Extra fail-closed token review-blind-input (missing or unparseable vector file or policy list, program word without hex4, policy without token), asserted by the existing vectors-file-missing control"
  - "requirements-completed stays empty: CPU-01 and RUN-03 are still declared by later plans of this phase (requirements.ready-ids reports both blocked)"

patterns-established:
  - "Review tooling lives under tools/review/ and is read from CHECK_DIR/../review by the gate step, so a control tree cannot replace it"
  - "Reader outputs and blinded copies exist only under the gitignored build/review/ or the scratchpad; the gate projects into the step's own temp dir"

requirements-completed: []

coverage:
  - id: D1
    description: "Blinded projection with opaque indices and policy tokens, fail-closed unknown-key and leak checks, run on every gate pass"
    requirement: CPU-01
    verification:
      - kind: integration
        ref: "sh tools/gate.sh (gate: PASS ... blind-files=12 second-read=0/12)"
        status: pass
      - kind: integration
        ref: "sh tools/check/run-controls.sh (control ok vectors-blind-unknown-key, vectors-blind-leak; controls=190/190)"
        status: pass
      - kind: other
        ref: "real-tree grep for dropped keys (0 files) and POL- (0) over a scratch projection; POL-TOY-01 probe -> FAIL review-blind-leak"
        status: pass
    human_judgment: false
  - id: D2
    description: "Record kind vector-review with the committed handoff-record check, each token shown by a control"
    requirement: RUN-03
    verification:
      - kind: integration
        ref: "sh tools/check/run-controls.sh (6 vectors-review-* controls ok)"
        status: pass
    human_judgment: false
  - id: D3
    description: "Diff tool mapping indices back to ids with six input-check tokens"
    requirement: CPU-01
    verification:
      - kind: other
        ref: "scratch toy probe: diff-summary agree=3, DIFF, review-derived-incomplete, review-handoff-mismatch, review-blind-mismatch, review-derived-missing (exit codes 0/0/1/1/1/1)"
        status: pass
    human_judgment: false
  - id: D4
    description: "Reader protocol and policy-statement document give an isolated reader enough to derive expectations without forbidden inputs"
    requirement: CPU-01
    verification:
      - kind: other
        ref: "Task 3 grep acceptance checks (6 sections, 19 tokens equal to the blind tool's, no vector ids, no POL-, no unparam'd cycle counts)"
        status: pass
    human_judgment: true
    rationale: "Whether the documents are sufficient for an isolated reader is only shown by the real fan-out at the plan 02-17 checkpoint"

duration: 15min
completed: 2026-11-10
status: complete
---

# Phase 2 Plan 16: Blind second-reader tooling and protocol Summary

**Leak-checked blinded vector copies (opaque v-indices and p-tokens) projected on every gate run, a diff tool that maps reader outputs back to ids and refuses missing, unlisted, stale or incomplete inputs, the vector-review marker kind checked against a committed handoff record, and the reader protocol plus 19 policy rules in words; no expectation was derived.**

## Performance

- **Duration:** 15 min (wall clock from the recorded start; reading time before it not counted)
- **Started:** 2026-11-10T09:50:13Z
- **Completed:** 2026-11-10T10:05:05Z
- **Tasks:** 3 of 3
- **Files modified:** 39 (3 commits)

## Accomplishments

- `tools/review/vector-blind.cmake` writes one blinded copy per stem with only `index`, `policy` (opaque token, policy vectors only), `has_bus`, hex-only `program` and `init` plus `init.stimulus`; it fails `review-blind-unknown-key` on a vector key outside the 02-02 schema and `review-blind-leak` on any dropped key, committed id, committed cite or `POL-` string in the projected text, and writes nothing for a failing file.
- The `cp1610-vectors` gate step projects all 12 files into its temp dir on every run (0.8 s) and reports `blind-files=12 second-read=0/12`; when `data/expected/vector-review.ids` lists a stem it also runs the `vector-review` record kind.
- Record kind `vector-review` checks the current file digest, digest and handoff-line shapes, the isolated reviewer label, counts, unresolved disagreements and the committed handoff record; eight new controls prove the six marker and two blind tokens (controls=190/190).
- `tools/review/vector-diff.cmake` maps reader indices and policy tokens back through the same projection, compares merged states field by field, and prints `DIFF` lines and a `diff-summary` with the run id.
- `docs/cp1610/second-reader-protocol.md` and `docs/cp1610/second-reader-policies.md` give the isolated reader its exhaustive inputs, output schema and every policy rule in words, keyed by opaque token, with parameters only on `[param]` lines.

## Task Commits

1. **Task 1: Tracer - blinded projection with leak scan and the vector-review kind, live in cp1610-vectors, eight controls** - `5498b6c` (feat). Tracer verified end to end: gate PASS, controls=190/190.
2. **Task 2: Diff tool with index-to-id mapping and input checks, proven end to end on toy data** - `59a8176` (feat)
3. **Task 3: Reader protocol and the policy-statement document** - `09a2691` (docs)

**Plan metadata:** recorded in the docs(02-16) completion commit.

## Files Created/Modified

- `tools/review/vector-blind.cmake` - blinded-copy writer (`ivs_blind_project`, `ivs_blind_policy_tokens`, `POLICY_MAP`), canonical JSON writer, leak scan.
- `tools/review/vector-diff.cmake` - input checks, index and token mapping, merged-state comparison, `DIFF`/`MISSING`/`diff-summary` output.
- `tools/check/schema/vector-review.txt`, `tools/check/kinds/vector-review.cmake` - marker record kind and hook.
- `tools/check/steps/cp1610-vectors.sh` - runs the blind projection and the marker kind; new stats.
- `data/expected/vector-review.ids`, `data/expected/vector-review-handoff.ids` - empty inventories with `#allow-empty:` reasons (plan 02-17 fills them).
- `data/expected/controls/vectors-review.ids` and 8 control dirs under `tools/check/controls/`.
- `tools/check/controls/_good/tree/data/cp1610/vector-review.json`, `.../expected/vector-review.ids`, `.../expected/vector-review-handoff.ids` - good-tree marker row `toy` and its handoff line `run-good-0`.
- `tools/check/controls/sources-no-files/MAKE`, `tools/check/controls/vectors-file-missing/EXPECT` - adjusted (see deviations).
- `docs/cp1610/second-reader-protocol.md`, `docs/cp1610/second-reader-policies.md` - reader documents.

## Task 1 proof (real tree, scratch OUT_DIR, deleted afterwards)

```
blind branch vectors=98
blind busrq-halt vectors=22
blind control vectors=27
blind direct vectors=19
blind encoding vectors=5
blind immediate vectors=24
blind indirect vectors=35
blind interrupt vectors=39
blind jump vectors=11
blind register vectors=62
blind sdbd vectors=27
blind shift vectors=43
```

- Forbidden-key grep `grep -l -E '"(id|row|kind|tier|cite|sources|asm|derivation|expect|bus|bus_basis|covers)"' <scratch>/*.json | grep -c .`: `0`.
- `POL-` grep summed over the 12 files: `0`.
- Probe on a scratch good tree whose alpha `init` gained `"x": "POL-TOY-01"`: `-- FAIL review-blind-leak: toy POL- string` (and no file written).
- `grep -c -F 'POL-' tools/review/vector-blind.cmake`: 3. `test ! -e build/review`: exit 0.
- `out=$(sh tools/check/run-controls.sh) && printf '%s\n' "$out" | grep -c -E '^control ok   vectors-(review|blind)-'`: 8; last line `controls=190/190`.

## Task 2 proof (toy fixture in the scratchpad only, deleted afterwards)

- Passing run: `diff-summary file=toy vectors=3 agree=3 disagree=0 derived_sha256=5dc252b18150d6ab672251f82eb70984b97f90b51a9b54294c9a310e2cbf6929 run_id=toy-probe-run` (exit 0).
- One derived `expect` changed: `DIFF cpu.ZERO.beta cycles committed=4 reader=5` then `diff-summary ... agree=2 disagree=1 ...` (exit 0).
- One index deleted: `MISSING v003 cpu.ZERO.alpha` and `-- FAIL review-derived-incomplete: toy 1 index(es) without a derived entry` (exit 1).
- HANDOFF.txt changed: `-- FAIL review-handoff-mismatch: toy sha256 of the derived file differs from HANDOFF.txt` (exit 1).
- Toy init edited: `-- FAIL review-blind-mismatch: toy blind_sha256 is not the projection of the current committed file` (exit 1).
- Derived file deleted: `-- FAIL review-derived-missing: toy toy.json absent` (exit 1).
- Extra probes: `-- FAIL review-derived-format: toy top key note`, `-- FAIL review-derived-extra: toy v009`, and with beta turned into a policy vector `DIFF cpu.ZERO.beta cycles committed=4 reader=6 policy=POL-TOY-01`, `DIFF cpu.ZERO.gamma mem:2000 committed=unset reader=1234`, `DIFF cpu.ZERO.alpha C committed=1 reader=0`.
- `test ! -e build/review` exit 0, and `git status --porcelain --untracked-files=all | grep -c 'build/review'`: 0.

## Task 3 checks

- Section counts: 6 protocol sections, 19 `## pNN` sections; `policy-tokens-ok` (headings equal the blind tool's tokens).
- No vector ids (count 0), no `POL-` (0), no `@<digit>` (0), no unparam'd cycle count (0) in the policy document; `HANDOFF.txt` appears in the protocol.
- `02-CONTEXT.md`, `reference-notes.md` and `stepper-conventions.md` occur 3 times, all in `## Forbidden inputs` (checked by reading).

Policy-token table (for the doc author and plan 02-17 only; never given to a reader):

| Token | Policy | Token | Policy |
|-------|--------|-------|--------|
| p01 | POL-MVO-01 | p11 | POL-IAB-01 |
| p02 | POL-SDBD-02 | p12 | POL-INT-03 |
| p03 | POL-HLT-02 | p13 | POL-INT-02 |
| p04 | POL-ILL-01 | p14 | POL-LINE-01 |
| p05 | POL-BUS-01 | p15 | POL-RST-01 |
| p06 | POL-WAIT-01 | p16 | POL-INT-04 |
| p07 | POL-STK-02 | p17 | POL-INT-01 |
| p08 | POL-STK-01 | p18 | POL-SDBD-01 |
| p09 | POL-HLT-01 | p19 | POL-BR-01 |
| p10 | POL-SDBD-03 | | |

## Orchestrator checkpoint (plan 02-17 Task 1)

1. Plans 02-01 to 02-16 are complete, the gate is green and the working tree is clean.
2. Run `rm -rf build/review && cmake -DROOT="$PWD" -P tools/review/vector-blind.cmake` and confirm 12 `blind <stem>` lines and exit 0.
3. For each stem, spawn one fresh subagent with no prior conversation whose prompt contains only the blinded copy path, the protocol path, the policy document path and the instruction to follow the protocol exactly and write `build/review/derived/<stem>.json`; run them in parallel batches of at most 4; pass no vector content, plan text, SUMMARY text, CONTEXT text or other readers' outputs.
4. After each batch, check that each output exists and parses as JSON; replace a failed or incomplete reader with a new fresh subagent, never by resuming the failed one.
5. Write `build/review/derived/HANDOFF.txt` with one line `<stem> <sha256> isolated-subagent-blind-v1 <run-id>` per stem, using `cmake -E sha256sum` and the run id the Agent tool returned for that subagent.
6. Resume plan 02-17, and never edit a derived file after the handoff.
7. If plan 02-17 later stops with `review-blind-mismatch` for a file, repeat steps 2-5 for that stem only, appending a new HANDOFF line that replaces the old one; for one stem, step 2 is `rm -f build/review/blind/<stem>.json build/review/derived/<stem>.json && cmake -DROOT="$PWD" -DFILE=<stem> -P tools/review/vector-blind.cmake` with one `blind <stem>` line, so the other stems' outputs and HANDOFF.txt are kept.

Isolation of the readers is attested only by the orchestrator transcript of that checkpoint; plan 02-17's SUMMARY records it (A-12).

## Uncontrolled-token exceptions

- `review-derived-missing`: orchestrator-output token; exercised by the toy probe of Task 2 and by the real fan-out in plan 02-17, not by a gate control (the gate never reads `build/review/derived/`).
- `review-derived-format`: orchestrator-output token; exercised by the real fan-out in plan 02-17 (reader outputs exist only there).
- `review-blind-mismatch`: orchestrator-output token; exercised by the toy probe of Task 2 and by the real fan-out in plan 02-17.
- `review-derived-incomplete`: orchestrator-output token; exercised by the toy probe of Task 2 and by the real fan-out in plan 02-17.
- `review-derived-extra`: orchestrator-output token; exercised by the real fan-out in plan 02-17.
- `review-handoff-mismatch`: orchestrator-output token; exercised by the toy probe of Task 2 and by the real fan-out in plan 02-17 (HANDOFF.txt is written only by the orchestrator).

## Decisions Made

- Opaque policy tokens (`p01`..`p19`, sha256 order) instead of a neutral policy id, as the plan required; the table above is the only place the mapping is written.
- The init copy goes through a canonical writer (members sorted, `, ` and `: ` separators, `true`/`false`) rather than CMake's own JSON pretty-printer, so `blind_sha256` is stable across CMake versions; the content is the committed init unchanged.
- A later HANDOFF line for a stem replaces an earlier one in both the diff tool and the hook, matching step 7 of the orchestrator procedure.

## Deviations from Plan

### Auto-fixed Issues

**1. [Rule 3 - Blocking] `sources-no-files` control broken by the new good-tree marker file**
- **Found during:** Task 1 (first full gate run)
- **Issue:** the control deletes every `data/**/*.json` of the good tree to provoke `sources-no-files`; the new `data/cp1610/vector-review.json` kept one JSON file, so the token was missing (controls=189/190).
- **Fix:** added `delete data/cp1610/vector-review.json` to its MAKE.
- **Files modified:** `tools/check/controls/sources-no-files/MAKE`
- **Verification:** controls=190/190.
- **Committed in:** `5498b6c`

**2. [Rule 2 - Missing critical] Fail-closed input token for the blind tool, asserted by an existing control**
- **Found during:** Task 1
- **Issue:** the plan names only `review-blind-unknown-key` and `review-blind-leak`; a missing or unparseable vector file, a program word without hex4 or a policy without a token needed a failure that never writes a copy.
- **Fix:** token `review-blind-input`; the existing `vectors-file-missing` control (inventory lists a `ghost` stem) now also expects `FAIL review-blind-input:`, so the token is controlled.
- **Files modified:** `tools/review/vector-blind.cmake`, `tools/check/controls/vectors-file-missing/EXPECT`
- **Committed in:** `5498b6c`

**3. [Rule 1 - Bug] Orchestrator step 7 would have deleted every reader output**
- **Found during:** Task 3
- **Issue:** step 7 says to repeat steps 2-5 for one stem, but step 2 begins with `rm -rf build/review`, which deletes the other stems' outputs and HANDOFF.txt.
- **Fix:** step 7 now names the single-stem form of step 2 (`rm -f` of that stem's two files, then the blind tool with `-DFILE=<stem>`); the SUMMARY section above copies the corrected text verbatim.
- **Files modified:** `docs/cp1610/second-reader-protocol.md`
- **Committed in:** `09a2691`

**4. [Minor] Blind digest computed from the projected text**
- **Found during:** Task 2
- **Issue:** the plan says the diff tool writes the projection to a temp dir; `string(SHA256)` of the projected text equals `file(SHA256)` of the file the blind tool writes (`file(WRITE)` writes the bytes verbatim, LF), so no temp file is needed.
- **Fix:** none needed beyond the design; verified by the toy pass/mismatch probes.
- **Committed in:** `59a8176`

**5. [Minor] Hook checks a little more than listed**
- `vector-review-format` also requires `file` to be `data/cp1610/vectors/<id>.json` and whole-number counts; a missing vector file reports `vector-review-stale`.
- The protocol's single-line note that the forbidden documents state values is split into one bullet per document so each name is on its own line (acceptance criterion counts lines).

---

**Total deviations:** 3 auto-fixed (1 blocking, 1 missing critical, 1 bug) plus 2 minor refinements.
**Impact on plan:** none on scope; all shapes fixed in the plan's `<context>` are unchanged.

## Issues Encountered

- BSD `sed` has no `0,/re/` address; one Task 2 probe was re-run with a Python edit so the init change actually happened (the corrected run is the one quoted).

## Gate

- `sh tools/gate.sh`: 36.8 s wall, `gate: PASS steps=24/24 controls=190/190 ... vectors=412 source=291 policy=121 rows-pending=0 policies-pending=0 bus-figure-pages=35 blind-files=12 second-read=0/12 ... doc-files=11 ... docs=11 ...`.
- `dash tools/gate.sh`: 37.0 s wall, `gate: PASS steps=24/24 controls=190/190 ...`.
- `build/review` absent at plan end; no file under `build/review/derived/` was ever written in the repository.

## User Setup Required

None - no external service configuration required.

## Next Phase Readiness

- Plan 02-17 opens with the orchestrator checkpoint above; the blind tool, the protocol and the policy document are its only reader-facing inputs.
- Plan 02-17 fills `data/cp1610/vector-review.json`, `data/expected/vector-review.ids` and `data/expected/vector-review-handoff.ids`; the gate then reports `second-read=<n>/12` and checks every row against the handoff record.
- No blocker.

## Self-Check: PASSED

- Files: all 12 created key files found on disk.
- Commits: `5498b6c`, `59a8176`, `09a2691` are ancestors of HEAD.
- Acceptance criteria of all three tasks re-run and passing; gate PASS under sh and dash.

---
*Phase: 02-cp-1610-cpu-core*
*Completed: 2026-11-10*

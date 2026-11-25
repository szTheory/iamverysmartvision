---
phase: 11-input-record-and-replay
status: passed
score: 5/5 success criteria met (1 on recorded terms, accepted as an override)
verified: 2026-11-25T16:44:09Z
verifier: gsd-verifier (Claude)
overrides_applied: 1
overrides:
  - must_have: "its grammar has no field able to hold a content digest, image name, length, frame byte, audio byte or path"
    reason: "No field is designated for content and the writer emits only the seven table fields, but the two slug fields (scenario 1..48, profile 1..64 over [a-z0-9-]) can literally spell a 32-hex run, which the oracle's sentence 'no 32-character hex sequence ... is expressible anywhere in the file' denies. Measured: a scenario whose id is a 32-hex string records a log carrying one 32-hex run. Accepted because a slug holds the operator's own scenario id (already present in their untracked scenario file), never a value the images produced, so D15's 'the log holds nothing the images produced' still holds; the rights check masks the two fields by rule and pins their shape (A-11-05, runner.record.no-content-digest assertion 4). The recommended remedy landed: A-11-V1 is appended to docs/formats/ivsl-v1.md and the frozen sentence is left as written."
    accepted_by: "gsd-verifier, under the owner's autonomous-operation delegation (.planning/AUTONOMY.md, 2026-11-09)"
    accepted_at: "2026-11-25T15:52:00Z"
behavior_unverified: 0
re_verification:
  previous_status: passed
  previous_score: "5/5"
  trigger: "11-REVIEW.md (1 blocker, 4 warnings, 4 infos) after this record was first written; five commits then changed covered source"
  gaps_closed:
    - "BL-01: the applied-prefix recorder had no liveness — no id exercised a run that applies only part of its schedule"
  gaps_remaining: []
  regressions: []
covered_files: [".planning/phases/11-input-record-and-replay/11-01-PLAN.md", ".planning/phases/11-input-record-and-replay/11-01-SUMMARY.md", ".planning/phases/11-input-record-and-replay/11-02-PLAN.md", ".planning/phases/11-input-record-and-replay/11-02-SUMMARY.md", ".planning/phases/11-input-record-and-replay/11-03-PLAN.md", ".planning/phases/11-input-record-and-replay/11-03-SUMMARY.md", ".planning/phases/11-input-record-and-replay/11-04-PLAN.md", ".planning/phases/11-input-record-and-replay/11-04-SUMMARY.md", ".planning/phases/11-input-record-and-replay/11-05-PLAN.md", ".planning/phases/11-input-record-and-replay/11-05-SUMMARY.md", "data/expected/faults/runner.ids", "data/expected/important-gates.ids", "data/expected/tests/runner-process.ids", "data/expected/tests/runner.ids", "docs/formats/ivsl-v1.md", "docs/runner.md", "runner/CMakeLists.txt", "runner/exec.c", "runner/ivs_runner.h", "runner/ivsl.c", "runner/ivsl.h", "runner/main.c", "runner/report.c", "tests/runner-process-tap.sh", "tests/runner/runner_exec.c", "tools/check/controls/_good/tree/.planning/REQUIREMENTS.md", "tools/check/controls/_good/tree/.planning/milestones/v0.1.0-REQUIREMENTS.md", "tools/check/controls/_good/tree/data/expected/important-gates.ids", "tools/check/controls/important-gates-bad-req-phase5/overlay/data/expected/important-gates.ids", "tools/check/controls/important-gates-bad-req-phase6/overlay/data/expected/important-gates.ids", "tools/check/controls/important-gates-complete-uncovered/overlay/.planning/REQUIREMENTS.md", "tools/check/controls/important-gates-req-uncovered-phase5/overlay/data/expected/important-gates.ids", "tools/check/controls/important-gates-req-uncovered-phase6/overlay/data/expected/important-gates.ids", "tools/check/controls/important-gates-req-uncovered-phase7/overlay/data/expected/important-gates.ids", "tools/check/controls/important-gates-uncovered-gate/overlay/data/expected/important-gates.ids", "tools/check/controls/important-gates-unknown-id-phase5/overlay/data/expected/important-gates.ids", "tools/check/controls/important-gates-unknown-id-phase6/overlay/data/expected/important-gates.ids", "tools/check/controls/important-gates-unknown-id/overlay/data/expected/important-gates.ids", "tools/check/steps/important-gates.sh"]
covered_digest: "v4:sha256:e7553a24f0c173f9145c0b866f301311318febbe0319a80b381bc4feb6704b87"
---

# Phase 11: Input Record and Replay Verification Report

**Phase Goal:** A run is recorded as a rights-clean, input-only log, and a replay of
that log reproduces the recorded run's digest chain and applied-event chain — with
the vacuity guards that v0.1.0's round-trip hook lacked.
**Verified:** 2026-11-25T16:44:09Z, on branch `gsd/phase-11-input-record-and-replay`
**Status:** passed
**Tier ceiling:** `deterministic-repeatability` — the replay runs the same core, so it
is never an independent comparison. Nothing in this report raises that ceiling.
**Re-verification:** Yes — re-run on the final tree after the code-review fixes. The
figures in the sections up to "Re-verification after the review fixes" are the ones
this verifier measured **before** those fixes; that section carries the final tree's
gate tails, the two-directional blocker probe and every number that moved. Read the
two together; where they differ, the re-verification section wins.

Every figure below was measured by this verifier on this tree. No SUMMARY claim was
taken as evidence; each is cited only where it agreed with a measurement, and the two
places where the record and the tree differ are in "Deviations accepted".

## Gate, re-run on the final tree (before the review fixes)

Three shells, all on this tree after the probe/restore cycle (`git status --porcelain`
= ` M .planning/state.json` only, the orchestrator's file; `git diff` empty):

```
sh    tools/gate.sh  -> exit 0, gate-time total=76, 0 '-- FAIL' lines
      gate: PASS steps=38/38 controls=395/395 faults=237/237 fault-runs=237 gates=110
      lines=337 tests-expected=4988 tests-executed=4984 tests-skipped=4 tests-missing=0
      runner-files=18 docs=64 doc-files=63 cases=13 goldens=29 claims=28 provisional=27
      api-calls=19 public-symbols=19 pinned-defines=120 files=2647 tracked=2647
      hygiene-files=2647 mode=tree
dash  tools/gate.sh  -> exit 0, 0 '-- FAIL' lines, summary line byte-identical to sh's
linux sh tools/gate-linux.sh -> exit 0, gate-time total=346, 0 '-- FAIL' lines
      gate: PASS ... (Ubuntu 24.04, gcc 13.3.0, cmake 3.28.3; host-dependent tokens as designed)
```

The orchestrator's measured token string is reproduced token for token. **These are
the pre-fix tails; the final tree's three tails are in the re-verification section
(`faults=238/238`, `tests-expected=4990`, `tests-executed=4986`).**

## Success Criteria

| # | Criterion (ROADMAP, frozen) | Verdict | Evidence |
|---|---|---|---|
| 1 | `docs/formats/ivsl-v1.md` committed before any code, ancestry proved; grammar has no field able to hold a content digest, image name, length, frame byte, audio byte or path; an unknown directive is refused with a typed token | **met on recorded terms** (override, see Open item O-11-V1) | Ordering: `git log --diff-filter=A -1 --format=%H -- docs/formats/ivsl-v1.md` = `dcfcd65b6861…` (08:53:42); `… -- runner/ivsl.c` = `50148e7166d8…` (09:16:24); `git merge-base --is-ancestor dcfcd65 50148e7` → exit 0, different commits. Fields: the table has exactly seven (version, scenario, profile, content, at_tick, c0, c1); the parser enforces them — `ivs_ivsl_slug` accepts only `[a-z0-9-]` (so `/`, `.`, `:`, `=` and `_` are inexpressible), `ivs_ivsl_dec` 1..19 digits with no sign/`0x`/leading zero, `ivs_ivsl_hex8` exactly eight upper-case hex digits; `ivs_ivsl_format` refuses any other name/content and writes only those seven fields. Typed token: `runner.record.refuse.unknown-directive` = `ok 205`, three trigger rows, each requiring `ivs_ivsl_reason_name(err.reason)` == the document's spelling `unknown-directive` (`IVS_IVSL_R_UNKNOWN_DIRECTIVE`). **Measured limit:** a `scenario` id that is a 32-hex string is a legal slug and the runner records it verbatim — `scenario 832906ce775d32a98ae607386ce4bbcd` in a produced log, 1 run of 32+ hex digits; see Open item O-11-V1, whose remedy has since landed as A-11-V1 |
| 2 | `runner.record.replay-equals-straight` and `runner.record.format-roundtrip` pass in process: a recorded log replays to the straight run's nine digest lines and applied-event chain, and write, parse, write again is byte-identical | **met** | `./build/dev/tests/ivs_tests_runner`: `ok 202 runner.record.replay-equals-straight`, `ok 203 runner.record.format-roundtrip`, `# suite=runner executed=261 skipped=0 failed=0`. Round trip = five assertions (byte-identical second write with equal length; format/parse fixpoint over events, ticks, both words, names, content class; byte figures computed from the strings; `IVS_IVSL_MAX_EVENTS == 64u == IVS_SCN_MAX_INPUTS`; a refusal leaves the caller's buffer untouched). **Non-vacuity, probed:** the replay path was made to parse the log and ignore it (`runner/exec.c`: the `log_in` branch never taken) → `not ok 202 … # the recorded and the replayed reports: record-no-events`. So the case fails if the log is ignored, which is A-11-03's requirement. Both ids were re-run green on the final tree (`ok 202`, `ok 203`, `executed=262`) |
| 3 | The liveness guards do the work: the equality case asserts ≥1 event, `input.consumed == input.events`, an `at_tick` list equal to the recorded one and all nine domains observed, while `replay-differs` and `replay-inert` are rejected **by the guard**, each shown able to fail | **met** | Guards are `rx_record_compare` (`tests/runner/runner_exec.c:3088`), in a fixed order: `record-no-events` (log empty or either report's `input.events` 0), `record-count-mismatch` (≠ the log's count), `record-not-consumed` (`input.consumed` ≠ its own `input.events`), then `record-differs` (whole-report `strcmp`, `host.calls` included — the log is a caller buffer, never a host call). `at_tick` list asserted at lines 3184-3196: `100`/`KEY_1`/`0` and `200`/`0`/`DISC_E`, count 2. "All nine domains observed" is in the **process** case (A-11-01) — `tests/runner-process-tap.sh:1254-1266` requires all nine `digest.*` lines present, non-`-` and equal, plus `output.video.bytes`, `output.audio.bytes`, `output.psglog.records`, `input.events` nonzero on both runs; the in-process case asserts the nine lines only, and says so in its comment (the fake host observes 6 of 9). **Probes on a scratch copy (all restored, `git diff` empty afterwards):** P1 equality disabled → `not ok 214 runner.judge.record-replay-differs` `# the one-nibble pair gave '(accepted)', want record-differs`, `ok 215` inert; P2 no-events clause disabled → `not ok 215 runner.judge.record-replay-inert` `# the identical zero-event pair gave '(accepted)', want record-no-events`, `ok 214` differs, and `not ok 202` with `record-count-mismatch`; P4 `input.consumed` printed as `consumed+1` → `not ok 202 … record-not-consumed`. Each mechanism is live and each is separable from the other. **Re-run on the final tree:** the equality probe and the producer-side `consumed+1` probe both still fire, and a fifth mechanism — the recorder's applied-prefix read — now has its own id and fault (see the re-verification section) |
| 4 | Every malformed log in the refusal family is refused with a typed token and no state change, and the two named faults fail exactly their ids | **met** | Nine ids, all `ok`: `runner.record.refuse.{unknown-directive,version,arity,hex,trailing,tick-order,tick-bound,event-cap,controller-bit}` (`ok 205`–`ok 213` on the final tree). Each row is judged five ways by `rx_refuse`: parse refused; `err.reason` is the row's code; `ivs_ivsl_reason_name` renders the document's token (document against code, not code against a copy of itself); the 1-based line; and the `0xA5`-poisoned `ivs_ivsl_log` the caller passed is byte-identical afterwards (first differing byte printed on failure) — that is the "no state change" assertion, plus the process case's "a refusal creates no file". Four acceptance rows bound the four classes that have a legal neighbour: equal ticks accepted (2 events), `IVS_SCN_TIME_MAX-1` accepted / `IVS_SCN_TIME_MAX` and `+1` refused, 64 events accepted / 65 refused, bit 30 accepted / bit 31 refused (`rx_accept` at 3721, 3753, 3786, 3828 on the final tree; 3637, 3669, 3702, 3744 before the review fix shifted them). `arity` now carries ten rows, three of them the documented non-decimal triggers (`+100`, `0x10`, `0100`, WR-03). **The named faults (three on the final tree, not two):** `RECORD_DROPS_LAST_EVENT` → `not ok 202`, `not ok 203`, `not ok 204` (all three named in its allowed column); `REPLAY_APPLIES_LATE` → `not ok 202` only; `RECORD_WHOLE_SCHEDULE` → `not ok 204` only, its must-fail and only allowed prefix. Each matches its registry row (`data/expected/faults/runner.ids`), and no site is outside `IVS_RUNNER_SOURCES` (`build/dev/ivs-runner-sources.txt:1,3` lists `runner/exec.c` and `runner/ivsl.c`) |
| 5 | `runner.record.no-content-digest` finds zero 32-hex runs in a produced log; the log is text, absent from `binary-ext.txt` and not gitignored; the gate ends `steps=38/38` with the predicted count delta and the tier ceiling `deterministic-repeatability`, provisional | **met** | `ok 214 runner.record.no-content-digest`, diagnostic `# no-content-digest: the produced log 152 bytes, 0 masked 32-hex runs; the stand-in 1; the 32-digit slug 1 unmasked and 0 masked`. **Independently reproduced on a real file** the runner wrote: `ivs-run --content-dir … --record-input pass.ivsl demo-lamp-tone-pass.ivss` → exit 0, log **319 bytes**, `grep -oE '[0-9A-Fa-f]{32,}' pass.ivsl \| wc -l` = **0**; the same file with the golden's `digest.input` line appended = **1** (A-11-05: without the stand-in the check cannot fail; the stand-in is a string literal in committed source, never a tracked `.ivsl`). Text/hygiene: `grep -n ivsl tools/check/binary-ext.txt` and `grep -n ivsl .gitignore` both empty; `git ls-files \| grep -Ei '\.(ivsl\|bin\|rom\|ivsc)$'` empty; `find . -name '*.ivsl'` empty after the gate, the suites and the process case. `steps=38/38` at the phase base (`git ls-tree c98db20 tools/check/steps/ \| wc -l` = 38; `git diff c98db20..HEAD -- data/expected/gate-steps/` empty) and at HEAD (gate above; unchanged again since the verification commit). Predicted delta measured exactly: `faults`/`fault-runs` 235→237, `gates` 109→110, `lines` 333→337, `steps` unchanged, `claims=28 provisional=27` unchanged. The review fixes then moved exactly three tokens again, as predicted: `faults`/`fault-runs` 237→238, `tests-expected` 4988→4990, `tests-executed` 4984→4986. TOOL-01's close: see the Requirements table |

## Requirements

| Requirement | Source plan | Description | Status | Evidence |
|---|---|---|---|---|
| TOOL-01 | 11-01…11-05 | A run can be recorded as an input-only log and replayed at machine-time boundaries, and the replay reproduces the recorded run's digest chain and applied-event chain, guarded against a vacuous match | **Complete, and the flip is one commit** | `git show --stat 16afb31` = **16 files**, carrying the checkbox flip (`- [ ] **TOOL-01**` → `- [x]`), the Traceability row (`\| TOOL-01 \| Phase 11 \| Pending \|` → `Complete`), four `runner-record` registry rows, the R2 header retarget, ten toy registry lines (`grep -rl '^toy-tool01 fault GOOD_FAULT TOOL-01$' tools/check/controls/ \| wc -l` = 10) and three toy requirement rows (= 3). Identity recomputed with the gate's own awk: `complete=58 excluded=12 accepted=46 registered=46`, `58 - 12 = 46`, `diff accepted registered` **empty**, `TOOL-01` in accepted (1) and in the exclusion ledger (0), `-- important-gates pass gates=110 lines=337` |

No orphaned requirement: `grep -E "Phase 11" .planning/REQUIREMENTS.md` names only TOOL-01, and every plan's `requirements` field names TOOL-01.

## What was automated / handed off / could not be judged by a tool

GUARDRAILS §5.6.

**Automated (this verifier ran it, on this tree).** The three gate shapes (`sh`, `dash`,
`gate-linux` in an `ivs-` container, all `gate: PASS`, 0 `-- FAIL`); the in-process
suite (261 ids) and the process suite (133 ids, 2 skipped by design); the two fault
executables and the exact set of ids each one fails; four scratch probes that each
disabled one mechanism (equality, no-events guard, `consumed == events`, the log
substitution) and each produced the finding it names, with the tree restored and
`git diff` empty afterwards; a real record/replay from the command line; an
independent 32-hex scan on the produced file; the ancestry proof; the requirement
identity arithmetic; the hygiene, `binary-ext`, `.gitignore` and `git ls-files`
checks; the registry/allowlist diffs (`api-effects.json`, `core/`, `data/golden/`,
`claims.json`, `pkg-install-files.ids`, `data/expected/gate-steps/` all unchanged).

**Handed off: nothing.** No item was deferred to a human or to a later run. The
container gate, which plan 11-05 recorded as a visible `not run`, was run here by the
orchestrator's boundary and passed (`exit 0`).

**Could not be judged by a tool: nothing in the criteria.** The two judgement calls
this report makes are stated rather than hidden: whether the drop fault's failure of
`format-roundtrip` is honest (yes — see "Deviations accepted") and whether the two slug
fields defeat criterion 1's field-absence clause (they narrow it — Open item O-11-V1).
Both are decided on measurement, not on taste.

## Deviations accepted

1. **`IVS_FAULT_RECORD_DROPS_LAST_EVENT` also fails `runner.record.format-roundtrip`,
   and the registry says so.** Judged honest and correctly recorded. The fault sits in
   `ivs_ivsl_format` itself, so a formatted-then-reformatted log drops a second time and
   format is not a fixpoint under it; no edit to the round-trip id can change that
   without making the id accept a zero-event log. The allowed column therefore names
   `runner.record.replay-equals-straight|runner.record.format-roundtrip` — two **exact**
   ids, never a `runner.record.` prefix, so nothing is hidden from the id that must
   catch the fault. Measured: 2 not-ok ids, both named. `11-05-SUMMARY.md` records the
   two routes it considered and why the second was taken; the registry comment above
   the row states it in `faults-verdict`'s own terms. The round-trip id remains green
   in every normal build and remains able to fail, so nothing was weakened. **Amended
   by the review fix:** the column now names a third exact id,
   `runner.record.partial-run`, for the same code property (a one-event log loses its
   only line); measured 3 not-ok ids, all three named, none unnamed.
2. **The no-log witness of the stripped copy ends `guest-fail 10`, not `pass`.**
   Recorded as plan 11-04's deviation 1 and judged correct: the stripped
   `demo-lamp-tone-pass` supplies no key, so the lamp-tone program's own assert fails
   (`assert=4 line=27 mem 02F2 == 007E got 00FF`). The witness exists to show the report
   *differs*, and it does; the vacuity argument is unaffected.
3. **`arity` and `trailing` are classes, not single triggers, and the family has nine
   tokens, not seven (A-11-02).** Judged correct: both tokens enumerate their members
   in the document and in the cases (`arity` 10 line shapes after WR-03, `trailing` 3
   file shapes), and neither is a bucket — no value another token owns is fed to them.
   The count of seven was an estimate in the frozen prose; the property the criterion is
   for (a closed family, each with a distinct typed token) is what was measured.
4. **Criterion 1's field-absence clause, on the recorded terms stated in the override
   above.** The clause is met for `image name` and `path` (inexpressible: the slug
   charset has no `/`, `.`, `:`), and for every content-bearing field in the sense that
   no field is designated for content and the writer emits only the seven table fields.
   It is **not** met literally for `content digest` (a 32-hex run fits the slug fields —
   measured) and cannot be met literally for `length`/`frame byte`/`audio byte` in any
   numeric format (a 19-digit tick and an 8-hex-digit word can spell any such number).
   The rights property the clause serves still holds: the runner writes the operator's
   own scenario id, never a value the images produced, so the log is not derived
   material under D-11.

## Open items

- **O-11-V1 — CLOSED by the review round.** `docs/formats/ivsl-v1.md` line 82-83
  states: *"A name is at most 48 or 64 characters and a tick at most 19 digits, so no
  32-character hex sequence and no path is expressible anywhere in the file (D-15)."*
  Measured false for the two name fields. The recommended remedy — an **appended**
  amendment, never a rewrite (GUARDRAILS 2.10) — is now in the document as **A-11-V1**
  (`docs/formats/ivsl-v1.md:218`), with the measured 32-hex slug, the reason the rights
  conclusion survives its premise failing, the accurate statement of what
  `runner.record.no-content-digest` checks (zero sequences outside the two masked slug
  fields, plus the stand-in), and "no format change, no code change, no version bump".
  `673d54e` reworded one word of it ("runs" → "sequences") because the text step's claim
  word made the gate red at `HEAD`; no meaning changed, nothing waived. The override in
  this record's frontmatter is unchanged and now rests on the amendment as well as on
  the measurement.
- **O-11-V2 (new, from the review, still open — not a criterion).** `ivs_ivsl_format`
  accepts events its own parser refuses: an `at_tick` at or above `IVS_IVSL_TICK_MAX`,
  a 20-digit tick, or a controller word with bit 31 set. Measured by the review (P5) and
  unchanged by the fixes; not reachable from the shipped CLI (`runner/scenario.c` bounds
  both), so it is a library-API contract defect in a header that is now public through
  `runner/ivs_runner.h`. Criterion 2 is unaffected: the round trip is asserted on a log
  the runner produced, which is already inside the parser's domain.
- **O-11-V3 (new, from the review, still open — documentation only).** The format
  document's Limits sentence attributes an oversize file to `trailing` while the reader
  refuses it as `too large` before the parser runs (IN-01); and an empty file is refused
  `arity` at line 0, a trigger the document's `arity` row does not enumerate (IN-02).
  Both are document-versus-code prose gaps; the refusals themselves are typed and
  tested, so criterion 4 is met. The sanctioned shape for the correction is an appended
  amendment (A-11-V1/A-11-V2 precedent).
- **O-11-V4 (new, from the review, still open — published line).** Under replay, a run
  that never reached `ex_run_steps_body` reports the scenario's `input.events` rather
  than the log's (IN-03). Measured again on the final tree: a `--record-input` run of
  `demo-lamp-tone-pass` with `--content-dir /nonexistent` ends `missing-fixture`, exit
  13, `input.events 6`, `input.consumed 0`. `docs/runner.md` states the rule with no
  exception for a run that never started. A `missing-fixture` report is not evidence and
  is not golden-compared, so nothing in the criteria moves.
- **O-11-V5 (new, from the review, still open — documented decision with no liveness).**
  A log whose recorded `scenario`/`profile` differ from the scenario being replayed is
  deliberately **not** refused, and `content` is never compared (IN-04). No id pins that
  acceptance, so a future reader that started refusing a mismatch would pass every id.
  Two lines of test would turn the documented decision into a pinned one.
- **Carried from the phase records, not re-verified as work:** the five deferred items
  in `.planning/STATE.md` (the `scenario`/`profile` mismatch refusals O-1/O-2, a
  content-identity field under D-15, promoting the process case to a real-executable
  gate case, the snapshot file options, and a new gate step for record/replay
  evidence). None is a criterion, none is due in Phase 11, and the frozen reversal
  trigger for the third is "a defect found only at the process boundary".

## Re-verification after the review fixes

**When:** 2026-11-25T16:44:09Z, on `gsd/phase-11-input-record-and-replay` at `26376c3`.
`git status --porcelain` = ` M .planning/state.json` only (the orchestrator's file);
`git diff` empty for every source file named below; no `ivs-` container left
(`docker ps -a | grep -c '^ivs-'` = 0).

**Why:** `11-REVIEW.md` found **1 blocker, 4 warnings, 4 infos** after this record was
first written. Five commits then changed covered source:

| Commit | Finding | Change |
|---|---|---|
| `718ecf2` | **BL-01** | new id `runner.record.partial-run` + new fault `IVS_FAULT_RECORD_WHOLE_SCHEDULE` (`n = x->sched_n`), registered as `RECORD_WHOLE_SCHEDULE`; `RECORD_DROPS_LAST_EVENT`'s allowed column names the new id |
| `7f58494` | WR-01 | one stderr line naming an empty log on a designed non-pass outcome, asserted in `runner-process.record.command-line` |
| `1746151` | WR-03 | three `arity` rows (`+100`, `0x10`, `0100`), comment seven → ten |
| `fb323f6` | WR-04 | appended amendment `A-11-V2` for the `trailing` line prose; the case's comment corrected |
| `673d54e` | pre-existing red gate | `HEAD` was already red (`-- FAIL text-claim-untagged: docs/formats/ivsl-v1.md:234`); one word reworded, no waiver |

### Gate, re-run on the final tree — three shells, one at a time, 0 `-- FAIL` lines

```
sh    tools/gate.sh         -> exit 0, gate-time total=75, 0 '-- FAIL' lines
dash  tools/gate.sh         -> exit 0, gate-time total=81, 0 '-- FAIL' lines
      summary line byte-identical to sh's (`diff` of the two `gate: PASS` lines: IDENTICAL)
linux sh tools/gate-linux.sh -> exit 0, gate-time total=287, 0 '-- FAIL' lines
      Ubuntu 24.04, gcc 13.3.0, cmake 3.28.3

gate: PASS steps=38/38 controls=395/395 faults=238/238 fault-runs=238 gates=110
      lines=337 tests-expected=4990 tests-executed=4986 tests-skipped=4 tests-missing=0
      claims=28 provisional=27
```

Every token the task predicted is reproduced token for token, and the four tokens that
were supposed to stay frozen did (`steps=38/38`, `gates=110`, `lines=337`,
`runner-files=18`). The container's log is `build/gate-linux.log`; the previous run's
log was copied aside before the run, as the phase records require.

**One token moved that the prediction did not name, and it is not drift:** `files` and
`tracked` went 2647 → 2649. The two added paths are this phase's own records —
`11-VERIFICATION.md` (committed in `643cfec`, after the pre-fix gate run that produced
2647) and `11-REVIEW.md` (committed in `26376c3`). No source, data, document or control
file was added: `git diff --name-status 643cfec..HEAD` lists one addition (the review)
and seven modifications, nothing else.

### BL-01, probed in both directions on the final tree

The fix's claim is that `runner.record.partial-run` **fails** when the recorder writes
the whole schedule and **passes** when it writes the applied prefix. Both directions
were reproduced by hand, not read from the fault machinery:

**Direction 1 — whole-schedule recorder.** `runner/exec.c:1001` changed from
`uint32_t n = x->rec->input_consumed;` to `uint32_t n = x->sched_n;` (the review's own
P1 mutation, applied directly, not through the `#ifdef`), rebuilt with `ninja -C
build/dev ivs_tests_runner`, suite run:

```
not ok 204 runner.record.partial-run
# the partial log took 153 bytes, want 116: the header and the one applied event line
...
# suite=runner executed=262 skipped=0 failed=1
```

`grep -c '^not ok '` = **1** — exactly the id the fix added, and no other. The failing
assertion is the byte count (116 = the 80-byte header plus one 36-byte event line,
against 153 = the header plus two), which is the strongest of the id's three checks: it
fails before the parse and before the `n_events` comparison.

**The process suite is green under the same mutation** (`ctest -R '^runner-process$'` →
`1/1 Test #22: runner-process ... Passed 18.70 sec`, `100% tests passed out of 1`).
That is the honest reading of the fix: the in-process id is the **sole** detector of
this property, and the review's optional second half — a process-case assertion with
`--max-ticks` — was not taken. It is enough, because the fault registry pins that id
(`RECORD_WHOLE_SCHEDULE runner.record.partial-run runner.record.partial-run
runner.parse.diag-boot-pass`) and the gate fails if the fault ever stops failing it, but
the limitation is recorded here rather than implied away.

**Direction 2 — restored applied-prefix recorder.** `runner/exec.c` restored from a
copy, `git diff --quiet runner/exec.c` empty, rebuilt:

```
ok 202 runner.record.replay-equals-straight
ok 203 runner.record.format-roundtrip
ok 204 runner.record.partial-run
# suite=runner executed=262 skipped=0 failed=0
```

`grep -c '^not ok '` = **0**. The same id is the one that flips, in both directions, on
one line of source: this is a guard, not a comment.

**And the property holds in the shipped binary, measured end to end.** A run that stops
early (the review's P8, re-run on the final tree):

```
ivs-run --content-dir build/dev/images --max-ticks 300000 --record-input /tmp/part.ivsl \
        data/scenarios/demo-lamp-tone-pass.ivss
  rc=11  outcome timeout  outcome.detail budget step=2 line=37 max-ticks=300000
  input.events 6   input.consumed 2
  the log: 163 bytes, exactly 2 `at_tick` lines —
    100000 c0=00000001 c1=00000000
    220000 c0=00000101 c1=00000000
```

Six declared events, two applied, two recorded. Replaying that log with the same budget
(`--max-ticks 300000 --replay-input`) gives a report **byte-identical apart from
`host.calls` except one line**: `input.events` is 6 in the recorded report and 2 in the
replay, which is the documented rule (the report's `input.events` is the log's count
under replay). All nine digest lines and `input.consumed` are identical. The applied-event
chain is reproduced, which is criterion 2's property, on the case BL-01 was about.

### The fix weakened nothing — checked in the four ways that could hide a loss

1. **`RECORD_DROPS_LAST_EVENT` still fails its must-fail id, and its wider allowed
   column hides nothing.** The gate's own result file
   (`build/gate/faults/RECORD_DROPS_LAST_EVENT/runner.txt`) lists exactly
   `not ok 202`, `not ok 203`, `not ok 204` — the must-fail id and the two allowed
   ids, no fourth. `tools/check/steps/faults-verdict.sh` judges `mf in okv` →
   `fault-not-detected` **separately** from the prefix match, and any not-ok id that
   starts with no allowed prefix → `fault-wrong-reason`; an empty field in the column →
   `fault-unknown-vector`. So a collateral failure cannot ride in on the column, and the
   must-fail cannot be satisfied by it. The third id is honest by construction: the
   partial log holds one event and the drop fault removes one event line, so the parse
   asserts 1 and reads 0 — the same property of `ivs_ivsl_format` that already put
   `format-roundtrip` in the column.
2. **The new fault fails exactly one id.**
   `build/gate/faults/RECORD_WHOLE_SCHEDULE/runner.txt` lists `not ok 204` only;
   `REPLAY_APPLIES_LATE` lists `not ok 202` only. `faults=238/238` in all three gate
   runs means every fault's result file was judged and every one was clean.
3. **The ids that must stay green did.** `ok 202 runner.record.replay-equals-straight`,
   `ok 203 runner.record.format-roundtrip`, `ok 204 runner.record.partial-run`, the nine
   refusals `ok 205`–`ok 213`, `ok 214 runner.record.no-content-digest` with its
   stand-in still counted (`# no-content-digest: the produced log 152 bytes, 0 masked
   32-hex runs; the stand-in 1; the 32-digit slug 1 unmasked and 0 masked`), and the two
   judge ids `ok 215`, `ok 216`. Suite total: `executed=262 skipped=0 failed=0`; process
   suite `1/1 ... Passed 16.37 sec`.
4. **The diff is additive or corrective, never subtractive.** `git diff
   643cfec..HEAD -- runner tests data docs`: 153 insertions, 11 deletions, and every
   deletion is a replaced line — the registry row that gained a column entry, the A-11-V1
   sentence that was reworded, the `arity` comment's count, the `trailing` comment that
   claimed the two readings coincide, and the line of `exec.c` the fault block was
   inserted above. No assertion, no id, no row and no trigger was removed.

### The five frozen criteria, re-checked on the final tree

| # | Verdict on the final tree | What changed since the pre-fix report |
|---|---|---|
| 1 | **met on recorded terms** (override unchanged) | Ancestry unchanged and re-proved: `dcfcd65b6861…` 08:53:42 is an ancestor of `50148e7166d8…` 09:16:24. The document gained two **appended** amendments (A-11-V1, A-11-V2) and lost nothing: `git diff 643cfec..HEAD -- docs/formats/ivsl-v1.md` is additions plus one reworded sentence. The measured limit stands (`the 32-digit slug 1 unmasked and 0 masked`), and it is now documented where it is measured |
| 2 | **met** | `ok 202`, `ok 203` re-run green on the final tree; the round trip's five assertions are untouched (`git diff` of that case: comment only). New: the applied-prefix property is reproduced on a real partial run (above) |
| 3 | **met, and stronger** | The four guards are unchanged in order (`record-no-events`, `record-count-mismatch`, `record-not-consumed`, `record-differs`) and re-probed live on the final tree — see the probe table. A **fifth** mechanism, the recorder's `input_consumed` read, now has an id and a fault; it was the one the four original probes could not reach |
| 4 | **met, with three named faults instead of two** | Nine refusals `ok 205`–`ok 213`; `arity` grew from seven rows to ten, the three added ones being the documented non-decimal triggers. All three faults fail exactly their registered ids (above) |
| 5 | **met** | `steps=38/38` in all three runs; `data/expected/gate-steps/` unchanged since the phase base and `tools/check/steps/` unchanged since the verification commit; no `.ivsl` tracked (`git ls-files`), none named in `tools/check/binary-ext.txt` or `.gitignore`, none on disk after the gates and both suites; the predicted delta measured exactly (`faults` 237→238, `tests-expected` 4988→4990, `tests-executed` 4984→4986), and `gates=110 lines=337 steps=38/38 runner-files=18` unchanged; tier ceiling `deterministic-repeatability`, provisional |

### Probes re-run on the final tree

Every probe was applied to the working file, rebuilt with `ninja -C build/dev
ivs_tests_runner`, run, and reverted from a copy; afterwards `git diff --quiet
runner/exec.c runner/report.c tests/runner/runner_exec.c` is empty, the suite is
`executed=262 skipped=0 failed=0` and the process suite passes.

| # | Probe | Result |
|---|---|---|
| R1 | `runner/exec.c:1001`: `n = x->rec->input_consumed` → `n = x->sched_n` (whole-schedule recorder) | `not ok 204 runner.record.partial-run`, `# the partial log took 153 bytes, want 116`, `executed=262 skipped=0 failed=1`, exactly one not-ok id; process suite still `Passed` (18.70 s) — **BL-01 closed** |
| R2 | same file restored | `ok 202`, `ok 203`, `ok 204`, `executed=262 skipped=0 failed=0` |
| R3 | `runner/report.c:346`: `r->input_consumed` → `r->input_consumed + 1u` (a producer-side wrong value) | `not ok 202 … # the recorded and the replayed reports: record-not-consumed` — the guard is live; also `not ok 167`, `not ok 204` (`# report: input.consumed is '2', want '1'`), `not ok 215` (now `record-not-consumed`, the guard order doing its work), `failed=4` |
| R4 | `rx_record_compare`: `ca != ea \|\| cb != eb` → `0` (the guard clause disabled, producer correct) | `executed=262 skipped=0 failed=0` — **recorded as a negative control on the probe method, not on the guard**: a guard is only observable through a wrong value, which R3 supplies. Disabling the clause alone cannot fail, and that is why R3, not R4, is the liveness evidence |
| R5 | `rx_record_compare`: `if (strcmp(a, b) != 0)` → `if (0)` (equality disabled) | `not ok 215 runner.judge.record-replay-differs`, `# the one-nibble pair gave '(accepted)', want record-differs`, `failed=1` — the differs guard is live on the final tree |
| R6 | `--content-dir /nonexistent --record-input /tmp/mf.ivsl data/scenarios/demo-lamp-tone-pass.ivss` | `rc=13`, `outcome missing-fixture`, exactly one stderr line `ivs-run: --record-input: no event log was written, the file is empty`, and the created file present and **0 bytes** — WR-01 as fixed, and the assertion in `runner-process.record.command-line` (`tests/runner-process-tap.sh:1308`) matches this line byte for byte |
| R7 | `--max-ticks 300000 --record-input` then `--max-ticks 300000 --replay-input` of the same log | recorded: `rc=11 timeout`, `input.events 6`, `input.consumed 2`, log 163 bytes with exactly the two applied `at_tick` lines; replay: `input.consumed 2`, all nine digest lines equal, the only differing report line `input.events` (6 → 2, the documented replay rule) |
| R8 | counts and registries: `awk` over `data/expected/faults/*.ids` (238), `data/expected/tests/runner.ids` (262 = the suite's `executed`), `runner-process.ids` (135 = 133 + 2 skipped), `gate-steps` (38), `git ls-tree c98db20 tools/check/steps` (38) | all agree with the gate's own tokens; the new fault's site is `runner/exec.c:1002`, inside `IVS_RUNNER_SOURCES` |

### Revised verdict

**passed, 5/5.** The blocker is closed by a guard that flips on one line of source in
both directions, and the three tokens it moved are exactly the three predicted, in all
three shells. The four warnings and four infos the review raised do not touch a frozen
criterion: one is fixed (WR-01), two are documentation corrections now appended as
amendments (WR-03's rows are asserted, WR-04's A-11-V2), and the rest are recorded as
**O-11-V2 … O-11-V5** with their measurements, so they are neither lost nor mistaken for
criteria. No human verification item remains, and `behavior_unverified` stays 0: every
truth this report asserts has a probe, a fault or a suite run behind it on the final
tree.

---

_Verified: 2026-11-25T16:44:09Z (re-verified on the final tree after `11-REVIEW.md`)_

## VERIFICATION: passed

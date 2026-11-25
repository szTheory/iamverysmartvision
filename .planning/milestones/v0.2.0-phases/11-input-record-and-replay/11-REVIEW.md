---
phase: 11-input-record-and-replay
status: findings
blockers: 1
warnings: 4
infos: 4
reviewed: 2026-11-25
depth: deep
files_reviewed: 29
files_reviewed_list:
  - runner/ivsl.c
  - runner/ivsl.h
  - runner/exec.c
  - runner/report.c
  - runner/ivs_runner.h
  - runner/main.c
  - runner/CMakeLists.txt
  - tests/runner/runner_exec.c
  - tests/runner-process-tap.sh
  - data/expected/faults/runner.ids
  - data/expected/tests/runner.ids
  - data/expected/tests/runner-process.ids
  - data/expected/important-gates.ids
  - tools/check/steps/important-gates.sh
  - tools/check/controls/_good/tree/.planning/REQUIREMENTS.md
  - tools/check/controls/_good/tree/.planning/milestones/v0.1.0-REQUIREMENTS.md
  - tools/check/controls/_good/tree/data/expected/important-gates.ids
  - tools/check/controls/important-gates-bad-req-phase5/overlay/data/expected/important-gates.ids
  - tools/check/controls/important-gates-bad-req-phase6/overlay/data/expected/important-gates.ids
  - tools/check/controls/important-gates-complete-uncovered/overlay/.planning/REQUIREMENTS.md
  - tools/check/controls/important-gates-req-uncovered-phase5/overlay/data/expected/important-gates.ids
  - tools/check/controls/important-gates-req-uncovered-phase6/overlay/data/expected/important-gates.ids
  - tools/check/controls/important-gates-req-uncovered-phase7/overlay/data/expected/important-gates.ids
  - tools/check/controls/important-gates-uncovered-gate/overlay/data/expected/important-gates.ids
  - tools/check/controls/important-gates-unknown-id-phase5/overlay/data/expected/important-gates.ids
  - tools/check/controls/important-gates-unknown-id-phase6/overlay/data/expected/important-gates.ids
  - tools/check/controls/important-gates-unknown-id/overlay/data/expected/important-gates.ids
  - docs/formats/ivsl-v1.md
  - docs/runner.md
---

# Phase 11: Code Review Report

**Reviewed:** 2026-11-25
**Depth:** deep
**Files Reviewed:** 29 (7 source, 2 suites, 6 registry/expected, 11 control trees, 2 documents)
**Status:** issues_found

## Summary

The format library, the replay substitution, the operator layer and the registry are, taken one
file at a time, in good shape. The parser is disciplined: every read of an operator-supplied byte
is bounded, no `atoi`/`strtoul`/`sscanf` is used, the decimal reader cannot overflow (`19 * '9'` =
9.99e18 < `UINT64_MAX`), the hex reader stops on the NUL because NUL is not a hex digit, `*out` is
written only on success, and the "no state change" ordering claim for `--replay-input` is a
property of `runner/main.c` rather than an assertion — I confirmed a malformed log refuses before
any output file exists. The exclusive create is `O_CREAT|O_EXCL` with a dev/ino identity check on
discard, so a symlink, a directory and a check-then-create race are all refused rather than
followed. Both new faults fail exactly their registered ids, and the drop fault's extra
`format-roundtrip` failure is a property of the code (`ivs_ivsl_format` is not a fixpoint under
it), not a widened allow-set: `faults-verdict` enforces the must-fail id separately from the
allowed prefixes.

One finding is a BLOCKER, and it is the phase's own risk. `runner.record.replay-equals-straight`
and the process case both exercise runs that consume their **entire** schedule, so the recorder's
central claim — "the events the run applied", not "the events the scenario declared" — has no
liveness. Changing `ex_write_log`'s `n = x->rec->input_consumed` to `n = x->sched_n` leaves the
in-process suite at 261/261 ok **and** the process suite green. That is the fifth mutation the
verification's four probes did not cover, and it is exactly the one that matters: a log of a
timed-out or guest-failed run would then record events the run never applied, and a replay of that
log would not be the recorded run.

The four warnings are smaller and all measured: a 0-byte unparseable log left behind by a
`missing-fixture` run; a writer that accepts events its own parser refuses; three documented
`arity` triggers with no assertion; and a one-line disagreement between the format document's
`trailing` row and the code that the test comment claims does not exist.

## Findings

### BL-01: the applied-prefix recorder has no liveness — the whole gate stays green if it writes the whole schedule

**File:** `runner/exec.c:1001` (the recorder), `tests/runner/runner_exec.c:3113-3240` (the
equality case), `tests/runner-process-tap.sh:1217` (the process case)

**What.** `ex_write_log` takes its event count from the run's applied prefix:

```c
uint32_t n = x->rec->input_consumed;   /* exec.c:1001 */
```

and both `docs/runner.md` (`--record-input`: "one `at_tick <n> c0=<hex> c1=<hex>` line per input
event the run applied") and the comment at `runner/exec.c:993-995` ("the events the run consumed",
"the recorder writes what the run applied rather than what the scenario said") state that as the
format's contract. **No id exercises a run that applies only part of its schedule.** The
in-process case runs the diagnostic, which consumes both of its two events; the process case runs
`demo-lamp-tone-pass`, which consumes all six. So `input.consumed == input.events` is `2 == 2` and
`6 == 6` in every id, and the recorder's `input_consumed` read is never distinguished from
`sched_n`.

**Measured (probe P1).** Replacing `n = x->rec->input_consumed;` with `n = x->sched_n;` and
rebuilding:

```
./tests/ivs_tests_runner   -> # suite=runner executed=261 skipped=0 failed=0
ctest -R '^runner-process$' -> 100% tests passed out of 1   (18.19 s)
```

No id fails. `runner.record.format-roundtrip` does not help — it compares a log to a log, and a
whole-schedule log is a perfectly well-formed log.

**Why it is a BLOCKER.** The phase's requirement is that "a replay of that log reproduces the
recorded run's digest chain and applied-event chain". For a run that stops early — a budget
timeout (`--max-ticks`), a guest-fail before the last `input` line, a cancelled run — a
whole-schedule log records events the run never applied, and a replay of it applies them, so the
replayed chain is *not* the recorded one. The current code is correct; the gate cannot tell.
This is precisely "the gate can pass while evidence is missing", and it is the mutation the four
verification probes (equality disabled, no-events clause disabled, `consumed+1`, `log_in` branch
untaken) each missed because all four act on the *comparison* rather than on the *recorder*.

**Fix.** One assertion, using fixtures that already exist, in the equality case: a fourth run `A'`
of the diagnostic text with `opts.max_ticks` set between its two input ticks, then

```c
/* A': a run that applies only its first event records only that event. */
if (!rx_is(a2.out, "input.events", "2") || !rx_is(a2.out, "input.consumed", "1")) { ... }
/* the log must carry exactly one at_tick line, equal to the first, and nothing else */
```

and the same in the process case with `--max-ticks 300000` on `demo-lamp-tone-pass` (measured
today: the log then carries exactly the two applied lines, `at_tick 100000 …` and
`at_tick 220000 …`, while the report says `input.events 6`, `input.consumed 2`). To match the
phase's own convention, add a third named fault in `runner/exec.c` —
`IVS_FAULT_RECORD_WHOLE_SCHEDULE`, `n = x->sched_n` — with its `runner.record.…` must-fail row and
its `runner-record` gate row; that makes the assertion a fault's target instead of a bare
assertion. The assertion alone closes the hole; the fault makes it a permanent one.

---

### WR-01: `--record-input` leaves a 0-byte, unparseable log on a `missing-fixture` run, and says nothing

**File:** `runner/main.c:1018-1027` (the write/close block) and `runner/main.c:1048-1056` (the
discard block)

**What.** The log is written only when the run produced bytes:

```c
if (code != MAIN_EXIT_HARNESS && res.log_out_bytes != 0u &&
    ivs_hostio_out_write(&mh.ivsl_out, opts.log_out, (size_t)res.log_out_bytes) != 0) { … }
```

and it is discarded only when the outcome is `harness-fail`:

```c
if (code == MAIN_EXIT_HARNESS) { … main_discard(&mh.ivsl_out, log_out_path, "--record-input"); }
```

A run that never reaches `ex_run_steps` — `missing-fixture`, exit 13, which is **not**
`MAIN_EXIT_HARNESS` — therefore sets neither: `res.log_out_bytes` is 0 and `code` is 13, so the
file created at `runner/main.c:931` is left behind empty, with no stderr line.

**Measured (probe P3).**

```
ivs-run --content-dir /nonexistent --record-input mf.ivsl demo-lamp-tone-pass.ivss
rc=13   outcome missing-fixture   outcome.detail missing role=exec path=boot-stub-2.exec
-rw-r--r--  1 <user> wheel 0 … mf.ivsl          <-- left behind, 0 bytes
stderr: (empty)
```

**Why it matters.** `missing-fixture` is a *designed* outcome in this project — the
`needs-user-exec` and `owner-exec-title` scenarios exist to produce it and the gate asserts it — so
this is a routine path, not a corner. The operator is left with a file that is not a log (and that
a second `--record-input` to the same path then refuses with "output file exists"), and the tool
prints nothing about it. Every other output has an explicit story for the empty case:
`--video-raw`/`--frame-ppm` print `no frame was delivered, the file is empty`
(`runner/main.c:1031-1039`). The log has neither a note nor a removal.

**Fix.** Mirror the frame-ppm precedent, but remove rather than keep, because an empty `.ivsl` is
not a valid log (see IN-02) while an empty PPM/raw stream is at least a prefix:

```c
if (log_out_path != NULL && code != MAIN_EXIT_HARNESS && res.log_out_bytes == 0u) {
  fprintf(stderr, "ivs-run: --record-input: the run applied no event and wrote no log\n");
  main_discard(&mh.ivsl_out, log_out_path, "--record-input");
}
```

Place it after the close block and before the `code == MAIN_EXIT_HARNESS` discard, and add one
assertion to `runner-process.record.command-line` (a `--record-input` run of
`owner-exec-title`/`needs-user-exec` must end exit 13 with no file left).

---

### WR-02: `ivs_ivsl_format` accepts events its own parser refuses, so `parse(format(l))` can fail

**File:** `runner/ivsl.c:131-190` (the writer) vs `runner/ivsl.c:285-330` (the parser's bounds)

**What.** `ivs_ivsl_format` refuses a log only for a NULL argument, a bad name slug, a bad content
class, more than `IVS_IVSL_MAX_EVENTS` events, or a `need` above `cap`/`IVS_IVSL_MAX_BYTES`
(`runner/ivsl.c:167`). It does **not** refuse an `at_tick` at or above `IVS_IVSL_TICK_MAX`
(`0x4000000000000000`), an `at_tick` of 20 digits, or a controller word with bit 31 set — all three
of which `ivs_ivsl_parse` refuses.

**Measured (probe P5, a scratch program linked against `build/dev/runner/libivs_runner_lib.a`).**

```
well formed                          format=0 bytes=80  parse=0 reason=ok
at_tick == IVS_IVSL_TICK_MAX         format=0 bytes=96  parse=1 reason=tick-bound line=5
at_tick == IVS_IVSL_TICK_MAX-1       format=0 bytes=96  parse=0 reason=ok
at_tick == 2^63                      format=0 bytes=96  parse=1 reason=tick-bound line=5
at_tick == UINT64_MAX                format=0 bytes=97  parse=1 reason=arity line=5
c0 bit 31 set                        format=0 bytes=80  parse=1 reason=controller-bit line=5
c1 bit 31 set                        format=0 bytes=80  parse=1 reason=controller-bit line=5
```

**Why it matters.** `runner/ivsl.h` documents the writer's contract as "writes log as IVSL v1
bytes", and `runner.record.format-roundtrip` asserts "parse and format are fixpoints of each
other" — but only on a log the runner produced, i.e. one already inside the parser's domain. The
asymmetry means the writer's accepted domain is strictly wider than the format's, so
`ivs_ivsl_format` can hand back bytes that `--replay-input` would refuse. It is not reachable from
the shipped CLI today: `runner/scenario.c:281-282` refuses a reserved bit and
`runner/scenario.c:917` bounds an `input at_tick` at `IVS_SCN_TIME_MAX`, which is
`IVS_IVSL_TICK_MAX`, and the replay path only ever formats a parsed log. So this is a library-API
contract defect, not a live failure — but `runner/ivsl.h` is now a public header (via
`runner/ivs_runner.h`), and the next caller of `ivs_ivsl_format` has no reason to know.

**Fix.** Make the writer refuse exactly what the parser refuses, in the same pre-write position
(`runner/ivsl.c:160`, beside the name/content checks), so nothing is written:

```c
for (uint32_t i = 0u; i < log->n_events; i++) {
  if (log->events[i].at_tick >= IVS_IVSL_TICK_MAX ||
      (log->events[i].controller[0] & IVS_CTRL_RESERVED_MASK) != 0u ||
      (log->events[i].controller[1] & IVS_CTRL_RESERVED_MASK) != 0u) {
    return 1;
  }
}
```

(and note it in `ivs_ivsl_format`'s doc comment at `runner/ivsl.h:74-78`, which currently
enumerates the refusal set and would otherwise be wrong). Then extend
`runner.record.format-roundtrip`'s assertion 4 with the three refused values, so the symmetry is
pinned rather than promised.

---

### WR-03: three documented `arity` triggers have no assertion, and the case's comment claims the enumeration is complete

**File:** `tests/runner/runner_exec.c:3485-3509` (the comment and `rows[]`)

**What.** `docs/formats/ivsl-v1.md:105` enumerates the `arity` triggers, and one of them is "an
`at_tick` with a sign, a `0x` prefix or a leading zero". The case's own header comment says "Seven
rows, and they are the enumerated members of the class rather than a place to put everything".
None of the seven rows feeds a sign, a `0x` prefix or a leading zero. The code handles all three
correctly — measured:

```
at_tick +100   -> rc=14 ivs-run: --replay-input: line 5 reason arity
at_tick 0x10   -> rc=14 ivs-run: --replay-input: line 5 reason arity
at_tick 0100   -> rc=14 ivs-run: --replay-input: line 5 reason arity
at_tick 100    -> rc=0
at_tick 0      -> rc=0
```

**Why it matters.** A-11-02's whole argument is that `arity` is a *closed, enumerated* class, and
criterion 4 asserts "every malformed log in the refusal family is refused with a typed token".
Today a change to `ivs_ivsl_dec` that folded `+100`, `0x10` or `0100` into an accepted tick would
pass all 261 runner ids and all 135 process ids — three documented refusals with no liveness. The
comment asserting completeness makes the gap harder to see, not easier.

**Fix.** Three rows, appended to `rows[]` at `tests/runner/runner_exec.c:3501`, all `on_head = 1`
and `line = 5u`:

```c
{"arity: at_tick with a plus sign", "at_tick +100 c0=00000000 c1=00000000\n", 1, 5u},
{"arity: at_tick with a 0x prefix", "at_tick 0x10 c0=00000000 c1=00000000\n", 1, 5u},
{"arity: at_tick with a leading zero", "at_tick 0100 c0=00000000 c1=00000000\n", 1, 5u},
```

and correct the comment's "Seven rows" to the new count.

---

### WR-04: `trailing`'s documented line attribution disagrees with the code by one line, and the test comment asserts they agree

**File:** `docs/formats/ivsl-v1.md:108` (the row) vs `runner/ivsl.c:363-372` and
`runner/ivsl.c:380-390` (the pre-scan and the missing-LF branch) vs
`tests/runner/runner_exec.c:3564-3576` (the case and its comment)

**What.** The document says the `trailing` token names "the last line before the offending byte".
The code names the line the byte sits on. For the first trigger in the document's own list — "a
byte after the last complete line" — those differ by exactly one: the last complete line is line
`N` and the code reports `N+1`. The same holds for a CR inside a line (the last line *before* it is
`N-1`, the code reports `N`).

**Measured (probe P6).** A 4-line header followed by a bare `x`, and a CR inside line 5:

```
trailing: a byte after the last complete line -> line 5   (the last complete line is 4)
CR inside line 5                             -> line 5   (the last complete line before it is 4)
```

and the test asserts `5u` for both, with a comment claiming "the document's 'last line before the
offending byte' and 'the line the byte sits on' agree here because the offending byte is inside
line 5". They do not agree; the comment is a rationalisation of the code's reading.

**Why it matters.** The document is the phase's frozen oracle and criterion 4 is judged as "the
document against the code". `err.line` is user-visible (`ivs-run: --replay-input: line N reason
trailing`), and it is the one field of a refusal whose meaning the document pins in words rather
than by enumeration, so a reader implementing from the document gets a different number from the
shipped parser. Nothing is functionally wrong — no consumer branches on `err.line` — which is why
this is a warning and not a blocker.

**Fix.** The document is frozen (GUARDRAILS §2.10, lesson 21) and A-11-V1 already shows the
sanctioned shape for a correction, so append an amendment rather than editing the row:

> **A-11-V2:** the `trailing` row's "Line named" column reads "the last line before the offending
> byte". The parser names **the line the offending byte sits on** (`runner/ivsl.c:363-372`), which
> for the row's first trigger is the line the byte *begins*, one past the last complete line.
> Measured: a 4-line header followed by a bare `x` reports line 5, not 4. No code change; the
> nine tokens and their reasons are unaffected.

and fix the test comment at `tests/runner/runner_exec.c:3564-3567` to say what the code does
rather than that the two readings coincide.

---

### IN-01: the Limits section says an oversize file is refused `trailing`; it is refused by the reader as `too large`

**File:** `docs/formats/ivsl-v1.md:126` vs `runner/ivsl.c:352-354` and `runner/hostio.c:105-131`

The document says "A file above 4096 bytes is refused `trailing` before its directives are
judged". Measured: a 4097-byte file is refused at the read layer, before the parser is called —

```
4096 bytes: rc=14 ivs-run: --replay-input: line 1 reason trailing   (the parser judged it)
4097 bytes: rc=14 ivs-run: log not readable: too large              (the reader refused it)
```

and `ivs_ivsl_parse` returns `-1` (not a `trailing` refusal, and without touching `err`) for
`len > IVS_IVSL_MAX_BYTES`. The `--replay-input` usage text and `docs/runner.md` both say
"oversize (above 4096 bytes)", which matches the code; only the format document's sentence does
not. **Fix:** an appended amendment (same shape as A-11-V1) stating that the size cap belongs to
the caller and is reported as a read refusal, with `trailing` reserved for the file's shape inside
the cap — or, if the document is meant literally, map `len > MAX` to
`IVS_IVSL_R_TRAILING`/`err.line = 1` in `ivs_ivsl_parse` and let `runner/main.c` stop pre-empting
it. The first is cheaper and keeps the better token.

### IN-02: an empty log is refused `arity` at line 0, a trigger the format document does not enumerate

**File:** `runner/ivsl.c:373-378`

`len == 0` sets `IVS_IVSL_R_ARITY` and `err.line = 0`. `runner/ivsl.h:79-80` documents line 0 as
"when the file has no line to name", so the header is consistent, but the format document's
`arity` row enumerates "a header directive missing" and not "an empty file", and criterion 4 is
judged against the document. Measured — and this is exactly what WR-01's leftover artifact
produces:

```
ivs-run --replay-input mf.ivsl …  -> rc=14  ivs-run: --replay-input: line 0 reason arity
```

**Fix:** name the empty file in the document's `arity` row (an appended amendment, or in the next
revision), or give it its own reason if a tenth token is ever wanted. No code change needed.

### IN-03: under replay, a run that never reached `ex_run_steps_body` reports the scenario's `input.events`, not the log's

**File:** `runner/exec.c:1508` (the initializer) vs `runner/exec.c:1069` (the replay assignment)

`rec.input_events_effective = scn->n_inputs` is the initial value and is replaced inside
`ex_run_steps_body`, which a `missing-fixture` run never reaches. Measured, replaying a 2-event log
against `demo-lamp-tone-pass` with no content dir:

```
outcome missing-fixture
input.events 6        <-- the scenario's count; the log holds 2
input.consumed 0
```

`docs/runner.md` says "The report's `input.events` is the log's event count", with no exception for
a run that never started. Low impact — a missing-fixture report is not evidence and is not
golden-compared — but it is a published line that disagrees with its own documentation. **Fix:**
set the field where the option is bound, in `ivs_runner_execute` beside `x.log_in`
(`runner/exec.c:1511-1513`):

```c
if (x.log_in != NULL) {
  rec.input_events_effective = x.log_in->n_events > IVS_SCN_MAX_INPUTS ? IVS_SCN_MAX_INPUTS : x.log_in->n_events;
}
```

### IN-04: the log's identity fields are "informational" with no liveness

**File:** `docs/formats/ivsl-v1.md` ("Identity"), `runner/ivsl.c:243-263`

The document states, deliberately and with the research open questions O-1/O-2 recorded, that a log
whose recorded `scenario`/`profile` differ from the scenario being replayed "is not refused", and
that `content` is never compared. No id asserts that acceptance: every replay in the two suites
uses a log whose header matches the scenario it is replayed against. A future reader that started
refusing a mismatch — the exact thing the document defers — would pass all 396 ids. **Fix:** one
row in `runner.record.refuse.*`'s neighbour set, or a line in
`runner.record.replay-equals-straight`: rewrite the log's `scenario` line to `scenario x` and
assert the replay still ends pass with a byte-identical report. Two lines of test, and it turns a
documented decision into a pinned one.

## What was checked and found sound

- **The parser's attack surface.** Every length is bounded before use: `ll >= sizeof buf` before
  the `memcpy` into `buf[128]`; `ivs_ivsl_hex8` stops on the NUL because NUL is not a hex digit, so
  `c0=` with an empty value cannot read past the buffer; `ivs_ivsl_slug` and `ivs_ivsl_dec` both
  `strlen` first and bound the length; `memcpy` into `scenario[49]`/`profile[65]` is preceded by
  the 48/64-byte slug check. No `atoi`, `strtoul`, `sscanf`, `eval`, `system`, `memcpy` from an
  unbounded count, and no allocation at all in the TU.
- **Decimal overflow.** `ivs_ivsl_dec` accumulates at most 19 digits, so its maximum is
  9,999,999,999,999,999,999 < `UINT64_MAX` (18,446,744,073,709,551,615). A 20-digit field is
  refused as a shape fault before any arithmetic, so no wrap and no negative tick is reachable. The
  bound is then `tick >= IVS_IVSL_TICK_MAX`, and `IVS_IVSL_TICK_MAX == IVS_SCN_TIME_MAX == 2^62 <
  IVS_RUN_TARGET_MAX == 2^63`, so no legal log can produce the replay's `input-out-of-range`
  harness failure.
- **"Every refusal leaves the output untouched."** `*out = st.log` is the only write to `out` and
  it is the last statement before `return 0`; `st` is a stack local. Nothing in `ivs_ivsl_line`
  touches `out`. The nine ids each poison the struct with `0xA5` and compare it byte for byte, and
  `runner.record.format-roundtrip`'s assertion 5 repeats it for the writer. Verified by reading
  every `return 1` in `ivs_ivsl_parse` (there are six) and every `return 0` in `ivs_ivsl_line`
  (there are twelve).
- **Exclusive create and the operator file.** `hostio_out_create` is `O_WRONLY|O_CREAT|O_EXCL|
  O_NOCTTY`, so there is no check-then-create window, a symlink at the target is `EEXIST` rather
  than followed, and a directory is `EEXIST`. Measured: a symlink at the `--record-input` path is
  refused and its target is not created; `--record-input` and `--replay-input` naming one path is a
  usage error that creates no file; `--update-golden` naming the log path is refused and creates no
  file; the log is created 0644 as documented. The discard path is
  `ivs_hostio_out_remove_if_same`, which compares the dev/ino recorded at creation, so a file
  replaced between the create and the discard is not removed. `git status --porcelain` is
  `M .planning/state.json` after the full process suite and `find . -name '*.ivsl'` is empty, so no
  test leaves an artifact in the tree (the process case uses a `mktemp` directory throughout).
- **The ordering claim for `--replay-input`.** The read and parse sit at `runner/main.c:800-840`,
  after the scenario parse and after every `content … user` refusal, and before the first
  `main_open_out` at `runner/main.c:873`. Measured with an output option in play: a malformed log
  plus `--audio-raw` gives exit 14, one stderr line, a 0-byte report and **no** `--audio-raw` file.
- **The two faults.** Both sites are inside `IVS_RUNNER_SOURCES` (`runner/ivsl.c:145-152` and
  `runner/exec.c:1051-1058`), both are `#ifdef`-guarded so a normal build cannot compile them, and
  the measured failing sets match the registry rows exactly:
  `RECORD_DROPS_LAST_EVENT` → `not ok 202 runner.record.replay-equals-straight` + `not ok 203
  runner.record.format-roundtrip`; `REPLAY_APPLIES_LATE` → `not ok 202` only. The drop fault's
  extra `format-roundtrip` failure is a property of the code (a drop inside `ivs_ivsl_format` is
  applied again to the log it just wrote, so format is not a fixpoint under it) and the allow-set
  does not hide the must-fail: `tools/check/steps/faults-verdict.sh` tests `mf in okv` separately
  from the `|`-split prefix list, so the fault would still be caught if it failed only the round
  trip.
- **Portability.** No non-ASCII byte in any new or changed source file (`grep -c '[^ -~\t]'` is 0
  for `runner/ivsl.c`, `runner/ivsl.h`, `runner/exec.c`, `runner/main.c`, `runner/report.c`,
  `runner/ivs_runner.h`, `tests/runner/runner_exec.c`, `tests/runner-process-tap.sh`). No locale
  function, no `isspace`/`tolower`, no argument with a side effect in a multi-argument call, so
  argument-evaluation order is irrelevant. `runner/ivsl.c` uses only `<stddef.h>`, `<stdint.h>`,
  `<string.h>` and `ivs/ivs.h` — no feature-test macro, no POSIX call, no `snprintf` (the writer
  builds bytes itself), so it compiles as C17 anywhere. The POSIX calls that exist
  (`O_NOCTTY`/`O_NONBLOCK`/`fstat`/`mkfifo` in the harness) are in `runner/hostio.c` and the test,
  not in the format library.
- **The counts and the registry.** Recomputed on this tree: `faults` = 237 non-comment lines
  across `data/expected/faults/*.ids` ✓; `tests-expected` = 2494 × 2 presets = 4988 ✓; `runner.ids`
  = 261 non-comment lines = the suite's `executed=261` ✓; `runner-process.ids` = 135 = 133 executed
  + 2 skipped by design ✓; `-- important-gates pass gates=110 lines=337` ✓. TOOL-01's close is one
  coherent commit (`16afb31`, 16 files: the checkbox and Traceability flip, the four
  `runner-record` rows, the R2 header retarget, ten toy registry lines and three toy requirement
  rows), `grep -rl '^toy-tool01 fault GOOD_FAULT TOOL-01$'` = 10, and TOOL-01 has no
  exclusion-ledger row.
- **The documents' published commands.** Every option and every message `docs/runner.md` and the
  `--help` text publish was run and matched: `--record-input: output file exists, refusing to
  overwrite it`, `log not readable: missing` / `too large` / `not a regular file` (a FIFO and a
  directory both give the last), `--replay-input: line 1 reason unknown-directive`, `--record-input
  and --replay-input are exclusive` (with the usage text), `--update-golden must not name an
  operator output file`, the 319-byte log with the four header lines and the six `at_tick` lines,
  and the header-only 4-line log for a scenario with no `input`.

## Probes run

Every probe was applied to a scratch copy of the file, rebuilt with `ninja` in `build/dev`, run,
and reverted; `git status --porcelain` is `M .planning/state.json` afterwards and
`./tests/ivs_tests_runner` is back to `executed=261 skipped=0 failed=0`.

| # | Probe | Result |
|---|---|---|
| P1 | `runner/exec.c:1001`: `n = x->rec->input_consumed` → `n = x->sched_n` (the recorder writes the whole schedule, not the applied prefix) | **Nothing fails.** in-process `executed=261 skipped=0 failed=0`; `ctest -R '^runner-process$'` `100% tests passed out of 1` (18.19 s). This is BL-01 |
| P2 | Baseline | `executed=261 skipped=0 failed=0`; `runner-process` passed 18.26 s |
| P3 | `--content-dir /nonexistent --record-input mf.ivsl` (missing-fixture) | exit 13, `outcome missing-fixture`, **0-byte `mf.ivsl` left behind**, empty stderr (WR-01) |
| P4 | `--replay-input` on that 0-byte file | exit 14, `ivs-run: --replay-input: line 0 reason arity` (IN-02) |
| P5 | scratch program over `ivs_ivsl_format` → `ivs_ivsl_parse` on 7 inputs | `format=0` for `at_tick` = 2^62, 2^63, `UINT64_MAX` and for bit 31 in either word; the parser refuses all four (`tick-bound`, `tick-bound`, `arity`, `controller-bit`) (WR-02) |
| P6 | `--replay-input` with a bare `x` after a 4-line header, and with a CR inside line 5 | both report `line 5`; the last complete line is 4 (WR-04) |
| P7 | `at_tick +100` / `0x10` / `0100` / `100` / `0` | `arity` × 3 at line 5, then accepted, accepted — the code is right, the ids are missing (WR-03) |
| P8 | `--max-ticks 300000 --record-input` on `demo-lamp-tone-pass` | exit 11, `input.events 6`, `input.consumed 2`, and the log carries **exactly the two applied** `at_tick` lines — the current code is correct and this is the case BL-01's fix should assert |
| P9 | refusal matrix: 4096 / 4097 bytes, missing, directory, FIFO, both options, `--update-golden` = log path, symlink at the log path | `trailing` / `too large` / `missing` / `not a regular file` ×2 / usage + no file / refused + no file / `output file exists` + target not created (IN-01 and "sound") |
| P10 | malformed log + `--audio-raw` | exit 14, 0-byte report, no `--audio-raw` file — the ordering claim holds |
| P11 | `--replay-input part.ivsl` (2 events) with `--content-dir /nonexistent` | `input.events 6` (the scenario's count) while the log holds 2 (IN-03) |
| P12 | counts: `awk` over `data/expected/faults/*.ids` and `data/expected/tests/*.ids`; `important-gates.sh`; `runner.ids` vs `executed`; `16afb31 --stat` | 237 / 2494×2 / `gates=110 lines=337` / 261=261 / 135 / 16 files, 10 toy rows — all agree |
| P13 | fault result files in `build/gate/faults/` | `RECORD_DROPS_LAST_EVENT`: `not ok 202`, `not ok 203`; `REPLAY_APPLIES_LATE`: `not ok 202` — matches both registry rows |
| P14 | `find . -name '*.ivsl'`, `git status --porcelain` after the full process suite | none; only `M .planning/state.json` |

---

_Reviewed: 2026-11-25_
_Reviewer: Claude (gsd-code-reviewer)_
_Depth: deep_

## REVIEW COMPLETE

**1 BLOCKER, 4 WARNINGS, 4 INFO** (9 findings).

## Fixes applied

- **BL-01** `718ecf2`. New id `runner.record.partial-run`: the diagnostic with a second input past the tick it halts on, so the run ends pass, reports `input.events 2` / `input.consumed 1`, and its log is the header plus one `at_tick 100` KEY_1 line that parses back to one event. New fault `IVS_FAULT_RECORD_WHOLE_SCHEDULE` (`n = x->sched_n`, `runner/exec.c`), registered as `RECORD_WHOLE_SCHEDULE` with that id as its must-fail and its only allowed prefix; `RECORD_DROPS_LAST_EVENT` also catches the new id (a one-event log loses its only line) and names it. Proven both ways: applied-prefix recorder `ok 204 runner.record.partial-run`, `executed=262 failed=0`; whole-schedule fault `not ok 204 runner.record.partial-run`, `executed=262 failed=1` - exactly one id.
- **WR-01** `7f58494`. `--frame-ppm` precedent followed: keep the file and name it, do not remove it - `ivs-run: --record-input: no event log was written, the file is empty`, one stderr line, exit 13 and report unchanged. Chosen because a silent removal of a path the operator named would be the only such place, and the empty file replays as a typed `line 0 reason arity`. Asserted in `runner-process.record.command-line` (`needs-user-exec`, 13, one line, file present and empty).
- **WR-03** `1746151`. Three `arity` rows (`+100`, `0x10`, `0100`, line 5) on the existing id; comment seven -> ten.
- **WR-04** `fb323f6`. `A-11-V2` appended (frozen row left as written) and the `trailing` case's comment corrected to the parser's reading; measured `line 5` for both triggers.
- **Pre-existing red gate** `673d54e`. `HEAD` was already red before any fix: `-- FAIL text-claim-untagged: docs/formats/ivsl-v1.md:234`, the word "runs" in the note `643cfec` added. Reworded to "sequences"; no waiver, no meaning change.
- **Deltas, predicted = measured:** `faults`/`fault-runs` 237 -> 238, `tests-expected` 4988 -> 4990, `tests-executed` 4984 -> 4986; `steps=38/38`, `gates=110`, `lines=337`, `runner-files=18` unchanged. Tails: `sh` `gate: PASS steps=38/38 controls=395/395 faults=238/238 fault-runs=238 gates=110 lines=337 runner-files=18 tests-expected=4990 tests-executed=4986 tests-skipped=4 tests-missing=0 claims=28 provisional=27`; `dash` identical.
- **Not attempted:** WR-02 (writer wider than parser - a library-API change plus a new refusal set) and IN-01..IN-04.

## FIX COMPLETE

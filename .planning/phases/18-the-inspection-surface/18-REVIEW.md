---
phase: 18-the-inspection-surface
reviewed: 2026-11-27T03:10:00Z
depth: standard
fix_pass: 2026-11-27T05:20:00Z
status: findings
critical: 1
warning: 6
info: 5
scope_base: 48470fa
scope_head: 4d907dc
files_reviewed: 45
files_reviewed_list:
  - .gitignore
  - .planning/REQUIREMENTS.md
  - CMakeLists.txt
  - README.md
  - data/expected/controls/rights.ids
  - data/expected/important-gates.ids
  - data/expected/pkg-selftests.ids
  - data/expected/release-assets-selftests.ids
  - data/expected/rights.ids
  - data/expected/tests/asm.ids
  - data/expected/tests/runner-process.ids
  - data/rights/inspection-output.json
  - docs/RIGHTS.md
  - docs/inspection.md
  - docs/packaging.md
  - tests/CMakeLists.txt
  - tests/asm-tap.sh
  - tests/runner-process-tap.sh
  - tools/check/binary-ext.txt
  - tools/check/controls/_good/tree/.planning/REQUIREMENTS.md
  - tools/check/controls/_good/tree/.planning/milestones/v0.1.0-REQUIREMENTS.md
  - tools/check/controls/_good/tree/CMakeLists.txt
  - tools/check/controls/_good/tree/data/expected/important-gates.ids
  - tools/check/controls/_good/tree/tools/inspect/good.c
  - tools/check/controls/files-unlisted-binary-ivst/CHECK
  - tools/check/controls/files-unlisted-binary-ivst/EXPECT
  - tools/check/controls/files-unlisted-binary-ivst/MAKE
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
  - tools/check/integrity.cmake
  - tools/check/steps/c-build.sh
  - tools/check/steps/important-gates.sh
  - tools/check/steps/pkg-install.sh
  - tools/check/steps/release-assets.sh
  - tools/inspect/CMakeLists.txt
  - tools/inspect/inspect.c
  - tools/inspect/inspect_dis.c
  - tools/inspect/inspect_trace.c
findings:
  critical: 1
  warning: 6
  info: 5
  total: 12
  necessity: 0
status: findings
---

# Phase 18: Code Review Report

**Reviewed:** 2026-11-27T03:10:00Z
**Depth:** standard
**Files Reviewed:** 45 of the 49 paths in `48470fa..4d907dc`
**Status:** findings

Three paths were read as required reading rather than reviewed as code (`18-02-SUMMARY.md`,
`18-03-SUMMARY.md`), and two were out of the stated scope (`.agents/skills/ops-hygiene/SKILL.md`,
`.planning/ops/INDEX.md`). No structural (fallow) pre-pass was supplied for this phase, so this report has no
structural section and every finding below is narrative.

## Summary

The instrument itself is sound and I did not take the records' word for it. I ran the built
`build/dev/tools/inspect/ivs-inspect` against the tree and against traces and reports I built in `/tmp`:

- `dis` on the boot-stub listing emits exactly the derivation `18-03-SUMMARY.md` claims (`ORG $1000`, then
  `B $1010`, eight `HLT`, `MVII #$0001, R0`, `MVO R0, $035F`, `TSTR R0`, `DIS`, `MVII #$0340, R6`, `J $5000`)
  — 16 instruction lines, 7 distinct mnemonics, the immediate present.
- The three output-tree spellings (literal, through `..`, through a symlink) are each refused with
  `inspect-output-tree`, nothing on stdout, and no file left; the exists refusal leaves the existing file
  byte-identical; a poked `--report` is refused with `inspect-reconcile`, nothing on stdout, no output file.
- `trace` on `diag-boot-pass` gives 112 access records, `end tick=2084 records=112 units=70 steps=1`, and
  `step --units 70` reconciles to `step.units 70`, `step.ticks 2084`, `step.halted yes`.
- I extracted the suite's four judge helpers verbatim and re-ran each against the stand-in the suite builds.
  All four demonstrations are real, not claimed: `ix_records` refuses `trace-empty` for exactly
  `trace-records`; `ix_tick_domain` refuses `trace-repeat-tick` for exactly `trace-ticks` **while a
  monotonicity-only check accepts the same file** (so the distinct-value clause is load-bearing);
  `ix_break_pair` passes on the real `(1, 0)` pair and refuses `break-always-hits` for exactly `break-zero`;
  `ix_digests_equal` passes report-against-report and refuses the poked report for exactly `digest-differs`.
  `sh tests/asm-tap.sh` on the current tree is `executed=56 skipped=0 failed=0`.
- The counts recompute from the files: `pkg-selftests.ids` 21 data lines, `release-assets-selftests.ids` 13,
  `tests/asm.ids` 56, `tests/runner-process.ids` 163, `important-gates.ids` 372 lines over 122 gate names,
  `rights.ids` 11. All thirteen toy registry/requirement files are pure last-line appends, and the two
  pinned line numbers (`bad-req-phase5` line 19, `bad-req-phase6` line 24) are untouched. The naming limb is
  clean: zero occurrences of `trace-file`, `disassembly-dump` or `inspection-dump` in `core/src`, `runner`,
  `tools`, `host` or the seven operator pages. Nothing is installed (`prefix-files` stays 11, no `install(`
  in `tools/inspect/CMakeLists.txt`), and no step, fault, golden or claim moved.

The defects are of two kinds. One is behavioural: `inspect-break-empty` is raised from the end record's
*declared* `records=` field rather than from the access records actually present, which falsifies the frozen
page's trigger **and** its accepted boundary in both directions — I reproduced both. The rest are the frozen
page and the phase's own evidence: `docs/inspection.md` still states two different operand derivations, names
a liveness stand-in that was never committed, and describes the `DECLE` stand-in as failing a clause it does
not fail; and one both-directions token comparison still excludes the disassembly token on a wave-boundary
rationale that `18-03` has now closed.

## Adjudication of `18-03-SUMMARY.md` deviation 3

**The page is internally inconsistent; the built limb satisfies the criterion as the page's authoritative
list states it; the correction belongs in the page, as an appended amendment.**

`docs/inspection.md:159-163` says "the operands are the following `-` rows plus the field groups of the
`BITS` column". `docs/inspection.md:179-185` — introduced two paragraphs later as "the derivation is pinned
here, because a cut the implementer has to guess is a cut that silently emits a source assembling to
different words" — says the operand text is the listing's own `ASM` column cut by three rules. These are two
different implementations, and the page's own justification for the second is what makes it authoritative: a
summary sentence cannot be the pinned rule, because the page pins the rule *precisely because* the summary
form is the one an implementer would get wrong.

`tools/inspect/inspect_dis.c` implements the pinned list, and that is the right call: the emitted source is
byte-for-byte the three-cut derivation, and it round-trips (verified). It does consume `BITS` groups and `-`
rows, but only to locate the `ROW` field and to count an instruction's words — never to derive an operand.
So the built limb is correct and the *opening paragraph* is the wrong sentence.

Where the correction goes: the page, not the record. `18-03-SUMMARY.md` deviation 3 is an accurate
observation, but a SUMMARY is a phase record; the next reader of this limb — Phase 19's consumer, or anyone
extending `dis` — reads the page. `GUARDRAILS-v0.4` §2.11 and §2.12 and `PLAN-LESSONS` lesson 21 make an
oracle correction an **appended** amendment and never a rewrite, and 18-01's record makes the freeze check a
byte-prefix comparison. §2.12 also asks for an inline pointer at the wrong sentence; here the byte-prefix
freeze forbids one, so the appended note must itself name the section and quote the sentence it supersedes.
See WR-01 for the concrete text and WR-02/WR-06 for two further amendments the same note should carry.

## Critical Issues

### CR-01: `inspect-break-empty` fires on the declared `records=` field, falsifying the frozen trigger and its boundary in both directions

**File:** `tools/inspect/inspect.c:987-993` (with `tests/runner-process-tap.sh:3288-3299`)

**Issue:** The comment two lines above the code restates the frozen page verbatim — "A trace with one access
record yields 1 or 0 and is not refused" — and the code below it does the opposite. `break` never counts the
`tick=` records it walks; it refuses on `end_records == 0u`, where `end_records` is the `records=` field
parsed out of the trace's own `end` record (`inspect.c:974-976`). `docs/inspection.md:213-215` pins the
trigger as "`break` is given a trace with zero access records" and the boundary as "A trace with one access
record yields a count of 1 or 0 and is not refused". I reproduced both directions against the built binary:

- one access record, `end … records=0` → `exit 1`, `ivs-inspect: inspect-break-empty: …` — the page says this
  invocation must **not** be refused;
- zero access records, `end … records=5` → `exit 0`, printing `break.records 5` — the page says this
  invocation **must** be refused.

Neither direction is recorded as a deviation anywhere in the three SUMMARYs, so the implementation silently
departs from the oracle on a named refusal token — exactly the class `GUARDRAILS-v0.4` §2.11 exists to stop.
The phase's own case does not catch it because it builds the empty trace by hand *with* `records=0`, which
happens to satisfy the declared-field rule; the case therefore exercises the wrong condition and would stay
green against either implementation.

**Fix:** count the records the loop actually parses and use that count for the refusal, keeping
`break.records` copied from the end record as the page requires:

```c
uint64_t records_seen = 0u;
...
    if (n > 5u && memcmp(line, "tick=", 5u) == 0) {
      ...
      records_seen++;
    }
...
  /* `inspect-break-empty`: the trace holds no access record to count a breakpoint against. */
  if (records_seen == 0u) {
    (void)snprintf(detail, sizeof detail, "the trace %s holds no access record to count against", trace_path);
    rc = ix_refuse(ix_tok_break_empty, detail);
    goto done;
  }
```

Then extend the third limb of `runner-process.inspect.break-pair` with both directions the page names: a
one-access-record trace whose end record says `records=0` must exit 0 with a `0`/`1` count, and a zero-record
trace whose end record says `records=5` must exit 1 with the token. Add `records_seen` as a second independent
number beside `break.records` so WR-05's fix has a place to live.

## Warnings

### WR-01: the frozen page still states two operand derivations, and the correction was recorded only in a SUMMARY

**File:** `docs/inspection.md:159-163` versus `docs/inspection.md:179-185`

**Issue:** The opening paragraph of `## Disassembly` names the `-` rows plus the `BITS` groups as the operand
source; the pinned derivation list two paragraphs later names the `ASM` column cut by three rules. Both cannot
be the rule. The implementation follows the pinned list (correctly — see the adjudication above), and the
deviation is recorded only in `18-03-SUMMARY.md` deviation 3. A later reader of the oracle gets the wrong
sentence, and the phase record that corrects it is not the artifact anyone consults for this limb.

**Fix:** append, after `## What this instrument is not` and with its own commit, an amendment that does not
edit a frozen line:

```markdown
### Amendment 1 (plan 18-03): the operand derivation

(The heading is a top-level `## ` section in `docs/inspection.md`; it is written `### ` here only so this
report's own section list stays unambiguous.)

The second sentence of `## Disassembly` reads "the operands are the following `-` rows plus the field groups
of the `BITS` column". That sentence is superseded by the pinned derivation list of the same section, which is
the rule `tools/inspect/inspect_dis.c` implements: the operand text is the listing's own `ASM` column cut by
the three pinned rules, in the pinned order. The `BITS` groups are consumed only to locate the `ROW` field,
and a `-` row is consumed only to count an instruction's words; neither contributes an operand. The pinned
list is authoritative because it is pinned for the reason it states - a cut the implementer has to guess is a
cut that silently emits a source assembling to different words.
```

Do not edit lines 159-163: 18-01's record makes the freeze check a byte-prefix comparison.

### WR-02: the page's `## Liveness guards` names a stand-in that was never committed, and mis-describes another

**File:** `docs/inspection.md:220-233`

**Issue:** Three separate mismatches between the guard list and what shipped.

1. Guard 1 (`:224-226`) names "a trace that steps to the scenario's target instead of to the stop: it prints
   16000 for a run that ended at 2084". No such case exists: `16000` occurs nowhere in
   `tests/runner-process-tap.sh` or `data/expected/tests/runner-process.ids`, and the five committed
   stand-ins are `trace-empty`, `trace-repeat-tick`, `trace-pokes-register`, `break-always-hits` and
   `dis-constant-mnemonic`. The end-tick limb of `runner-process.inspect.trace-records`
   (`tests/runner-process-tap.sh:3131-3141`) therefore has no stand-in, so `GUARDRAILS-v0.4` §2.7's third
   requirement — "a committed scratch stand-in showing the guard fail" — is unmet for that limb.
2. Guard 4 (`:231-233`) says the `DECLE`-everything stand-in "fails the two presence assertions rather than
   the word comparison". The shipped `asm.judge.dis-constant-mnemonic` must fail the **word comparison**, and
   the case enforces that: it reports `judge-vacuous` if `dumps_agree` accepts the stand-in and
   `judge-wrong-reason` unless `$tmp/why` begins `word dumps differ`. The page describes the wrong clause.
3. The repeat-tick stand-in, which is the guard for reconciliation equality 5 and is named as such at
   `docs/inspection.md:133-134`, is absent from a list that opens "Four guards".

**Fix:** commit the guard-1 stand-in as a `runner-process` case judged by the same comparison the real case
uses, and register it beside the other four:

```sh
# runner-process.judge.trace-target-instead-of-stop: the end-tick equality's committed stand-in.
sed 's/^\(step=[0-9]* target=[0-9]* end=\)[0-9]*\( .*\)$/\116000\2/' "$tmp/ix/trace.ivst" \
  > "$fakes/trace-target.ivst"
sed -n 's/^step=[0-9][0-9]* target=[0-9][0-9]* end=\([0-9][0-9]*\) .*/\1/p' "$fakes/trace-target.ivst" \
  > "$fakes/trace-target.stepend"
sed -n 's/^end tick=\([0-9][0-9]*\) .*/\1/p' "$tmp/ix/trace.ivst" > "$fakes/trace-target.reportend"
self_control trace-target-instead-of-stop trace-end-tick cmp -s \
  "$fakes/trace-target.stepend" "$fakes/trace-target.reportend"
```

(then hoist the `cmp -s` pair into an `ix_end_tick` helper so the real case and the stand-in are judged by one
function, as the other four are). Add its `runner-process mutant … TOOL-02` row, and append an amendment
correcting guard 4's clause and adding the repeat-tick guard to the list.

### WR-03: `inspect.surface` still excludes `inspect-dis-row` from both directions, on a rationale 18-03 has closed

**File:** `tests/runner-process-tap.sh:3385-3433` (the exclusion at `:3422-3426`)

**Issue:** The case's comment says `inspect-dis-row` "belongs to the `dis` surface, which plan 18-03 lands in
the next wave, so it is excluded from BOTH sides". `18-03` landed in this phase, and the two sets are now
identical: `sed -n '/^## Refusal tokens/,/^## Liveness guards/p' docs/inspection.md | grep -o -E
'inspect-[a-z-]+' | sort -u` and `grep -h -o -E '"inspect-[a-z-]+"' tools/inspect/*.c | tr -d '"' | sort -u`
both yield the same five tokens. So the exclusion is no longer a wave-boundary courtesy; it is dead leniency,
and the one token it removes is the disassembly refusal's — the phase's substance. A rename or a removal of
`inspect-dis-row` on either side now goes unnoticed by the only id that compares the two sets, and the case
asserts `-eq 4` on both sides, so nothing else catches it either.

**Fix:** delete the two `grep -v -x -F -e 'inspect-dis-row'` lines and change both assertions to `-eq 5`,
keeping both `comm` directions. Verified safe on the current tree: the sets are byte-identical, so the id
passes with the exclusion removed.

### WR-04: `inspect.neg.inspect-output-tree` writes inside the work tree and relies on an explicit `rm -f`, not the suite's trap

**File:** `tests/runner-process-tap.sh:3174`, `:3190-3196`

**Issue:** This is the only place in either suite that creates a file inside the work tree.
`mkdir -p "$root/build" "$root/refs"` creates two directories in the source tree, and the `refs/` positive
limb really writes `$root/refs/inspect-probe.ivst` and then removes it with two explicit `rm -f` calls. The
suite's EXIT trap (`:179`) removes only `$tmp`, so a failure between `:3192` and `:3196` — or a `SIGKILL`, or
the `HUP INT TERM` trap that only `exit 1`s — leaves a rights-bearing `.ivst` inside the work tree. It is
gitignored, so neither `git ls-files` nor `hygiene --tree` can see it, which is the worst combination: the
artifact the phase's own rights row says is never committed, invisible to both mechanisms that would refuse
it. The nearest precedent (`tests/pack-tap.sh:328-330`) only attempts writes that must be **refused**, so it
never creates anything.

**Fix:** cover the probe paths with the suite's own trap, and keep the `mkdir` because `refs/` is legitimately
absent on a clean checkout:

```sh
trap 'rm -rf "$tmp"; rm -f "$root/refs/inspect-probe.ivst" "$root/build/inspect-probe.ivst"' EXIT
```

### WR-05: `break.records`/`break.end_tick`/`break.steps` are copied from the end record and the case compares the copy to its own source

**File:** `tests/runner-process-tap.sh:3279-3286` with `tools/inspect/inspect.c:1009-1018`

**Issue:** The three keys are `fprintf`ed straight from `end_records`, `end_tick` and `end_steps`, which were
parsed out of the trace's `end` record. The case then extracts the same field from the trace and `cmp`s it
against the report's copy. The number is computed once and copied, so the comparison can only fail if the copy
itself is broken — it is not the "two independent computations of the same number" the phase claims for
`break.hits`, and it is the weakest form of the pattern the brief names ("a reconciliation that reads the same
document twice"). `break` never counts the access records it walks, so no independent number exists to compare
against.

**Fix:** keep the copy (the page mandates it: "The last three are copied from the trace's own `end` record")
and add the independent limb in the case:

```sh
  grep -c '^tick=' "$tmp/ix/brk.ivst" > "$tmp/ix.brk.counted"
  cmp -s "$tmp/ix.brk.counted" "$tmp/ix.brk.brkrecords" \
    || { ok=0; why break-pair "break.records '$(cat "$tmp/ix.brk.brkrecords")' is not the $(cat "$tmp/ix.brk.counted") access records in the trace"; }
```

With CR-01's fix this becomes the same number reached two ways, which is what the pair claim needs.

### WR-06: the disassembly limb refuses one of the seven registered listings, and nothing asserts or records it

**File:** `tools/inspect/inspect_dis.c:20-26` and `:59-66`, with `docs/inspection.md:159-160`

**Issue:** The row vocabulary is opcodes only, so a `DATA` row is refused fail-closed — a decision 18-03
deviation 4 records. The consequence is not recorded and not asserted: I ran `dis` over every listing in
`data/expected/listings.ids` and six decode while `ivs-diag-stic-2` exits 1 with

```
ivs-inspect: inspect-dis-row: the row DATA at 512E is not one of the mnemonics ivs_asm accepts
```

The page's `## Disassembly` opens "The corpus is the project's own listings, registered in
`data/expected/listings.ids`", which a reader takes to mean the corpus is decodable; the operative boundary
sentence (`## Refusal tokens`, "Every row the boot-stub corpus uses is decoded") is narrower, so the page
contradicts itself again, and no `asm.dis.*` id covers the refusal on a *registered* listing. An operator
meets this as a surprise and a reader meets it as a bug.

**Fix:** add one `asm.dis` case that asserts the fail-closed refusal on the registered listing — exit 1,
exactly one stderr line beginning `ivs-inspect: inspect-dis-row: `, naming the row and the address, no output
file — and carry it in WR-01's appended amendment, which should say which of the registered listings decode
and that a data row is refused by name rather than emitted as a guess.

## Info

### IN-01: `--pc` may equal `--other`, under which the page's "a matcher that always hits cannot satisfy both" is false

**File:** `tools/inspect/inspect.c:908-922` (argument parsing), `docs/inspection.md:147-148`

**Issue:** Neither `trace` nor `break` refuses two equal addresses. Verified: `trace … --break 1000 --other
1000` exits 0 and writes `break_pc=1000 break_hits=1 other_pc=1000 other_hits=1`, a pair an always-hitting
matcher satisfies. The phase's case pins 1000/1002, so its assertion is real; the surface simply does not
enforce the precondition the page's sentence rests on.

**Fix:** refuse `--pc` equal to `--other` as a usage error (exit 2, no token) and record the new usage rule in
WR-01's appended amendment; or state the precondition in the page instead.

### IN-02: `inspect.c`'s header omits `dis` from the grammars it implements

**File:** `tools/inspect/inspect.c:4-5`

**Issue:** The header says the file implements "`## Command surface` (the `trace`, `break` and `step` grammars,
`--help`, the three exit statuses)". `ix_cmd_dis` lives in this file (`:1046`) since 18-03, and `dis` is a
fourth grammar the same `main` dispatches.

**Fix:** read "the `trace`, `break`, `dis` and `step` grammars".

### IN-03: the page says the tool "requires five equalities"; two of the five hold by construction and are never checked by the tool

**File:** `docs/inspection.md:122-146`, with `tools/inspect/inspect_trace.c:646-651,687`

**Issue:** Equality 1 (the report against `ivs-run`) is structural — the tool's report *is* the library's
bytes — and equality 4 (the end tick against the last step record's `end`) is one variable written twice:
`last_step_end` feeds both the `step=` record and the `end` record. Only equalities 2, 3 and 5 are computed
checks inside the tool. The evidence for 1 and 4 exists, but it is asserted on the two documents by
`inspect.trace-digests` and by the `cmp` in `inspect.trace-records`, not by the tool.

**Fix:** fold into WR-01's appended amendment — say that 1 and 4 hold by construction and are asserted on the
artifacts by the ids, while 2, 3 and 5 are the tool's own refusals.

### IN-04: `tools/check/integrity.cmake`'s inserted comment lines break the block's shape

**File:** `tools/check/integrity.cmake:76-77`

**Issue:** Both new lines start at column 0 inside a two-space-indented comment block, and they are inserted
between the first sentence of the `host/` paragraph and its continuation, so the block now reads as three
fragments and the `host/` sentence has lost its subject line's company.

**Fix:** indent both lines by two spaces and move the `tools/inspect/` sentence to sit after the complete
`host/` sentence, so each paragraph stays contiguous.

### IN-05: the frozen page names the artifact and its disposition two commits before the governing rights row exists

**File:** `docs/inspection.md:235-241` versus `data/rights/inspection-output.json`

**Issue:** `git log --diff-filter=A` puts the page at `0627be9` (18-01) and the row at `5ed5610` (18-02 T1).
`GUARDRAILS-v0.4` §2.13 reads "no code or operator page names a refused item, until its `data/rights/` row
exists with a disposition". In substance nothing is exposed: no byte could precede the row, because the only
writer landed in the same commit as the row, and the page names the posture rather than a refused spelling —
the naming limb is clean (zero hits for all three tokens across the four prefixes and seven pages). The
ordering is still the letter of §2.13 unmet, and it is not noted anywhere.

**Fix:** none required in code. Record the ordering in the phase record (or in the close's stale-text sweep)
so a later reader does not read it as a §2.13 breach, and keep 18-01's requirement that the page state the
rights posture — it is the page's job to freeze the posture and 18-02's to land the row.

---

_Reviewed: 2026-11-27T03:10:00Z_
_Reviewer: Claude (gsd-code-reviewer)_
_Depth: standard_

---

## Disposition (fix pass, 2026-11-27)

Every Critical and Warning is closed, and four of the five Info findings. One Info (`IN-05`) is a
records-ordering note closed by this disposition rather than by code. Each row names the commit
that closed it; a row with no commit is a recorded decision, not an omission.

| Finding | Disposition | Where |
|---|---|---|
| **CR-01** `inspect-break-empty` fired on the declared `records=` field | **fixed** — the boundary is now decided on the access records actually present, both directions | `5f68b30` |
| **WR-01** the page stated two operand derivations | **fixed by amendment** — the page gains a dated `## Amendments` section naming the `ASM`-column three cuts as authoritative; no frozen sentence rewritten (§2.12) | `e936f8d` |
| **WR-02** the page named a stand-in that was never committed, and mis-described another | **fixed** — the end-tick guard's stand-in is now an extra `self_control` limb **inside the existing `inspect.trace-state` id** (no new id, no count moved), and the page's wording is corrected by the same amendment | `8b65f88`, `e936f8d` |
| **WR-03** `inspect.surface` still excluded `inspect-dis-row` on a closed rationale | **fixed** — the exclusion is gone from both sides; the token is compared in both directions | `2e07362` |
| **WR-04** the `refs/` positive limb relied on an explicit `rm -f` | **fixed** — the probe is held under the suite's `EXIT` trap, so an interrupted run cannot leave a rights-bearing `.ivst` in the tree | `f6bb8e7` |
| **WR-05** `break.records`/`end_tick`/`steps` were compared to their own source | **fixed** — `break` counts the records it parses, and the case reconciles the number reached two ways | `cba2ff6` |
| **WR-06** the disassembly limb refuses one of the seven registered listings | **fixed by amendment** — the amendment says which registered listings decode and why the `DATA`-bearing one is refused by design | `e936f8d` |
| **IN-01** `--pc` may equal `--other` | **fixed by amendment** — the precondition is stated in the page | `e936f8d` |
| **IN-02** the tool's header omitted `dis` | **fixed** | `7662a15` |
| **IN-03** the page said "five equalities" where two hold by construction | **fixed by amendment** — the amendment says 1 and 4 hold by construction and are asserted on the report, not re-checked by the tool | `e936f8d` |
| **IN-04** `integrity.cmake`'s inserted comment lines broke the block's shape | **fixed** — the block's shape is restored | `7662a15` |
| **IN-05** the page named the artifact two commits before the rights row existed | **recorded, no code change** — the ordering is the page's job (freeze the posture) and 18-02's to land the row (§2.13's letter met at the commit that first writes bytes); named here and carried into the phase record so a later reader does not read it as a breach | this row |

**Gate after the fix pass** — `sh tools/gate.sh` and `dash tools/gate.sh`, byte-identical summary lines,
zero `-- FAIL ` lines:

```
gate: PASS steps=38/38 controls=405/405 faults=238/238 goldens=29 gates=122 lines=372
tests-expected=5082 tests-executed=5078 tests-skipped=4 tests-missing=0 prefix-files=11
claims=29 provisional=28 rights=11 rights-cleared=1 pkg-selftest=21/21 release-selftest=13/13
```

No test-id count moved during the fix pass. `tools/gate-linux.sh` was **not run**: the docker daemon on
this host is unresponsive (`docker info` times out); the phase records that as a visible `not run` line
and never a pass (lesson 24).

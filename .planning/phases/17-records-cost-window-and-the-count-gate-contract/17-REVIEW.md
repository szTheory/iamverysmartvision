---
phase: 17-records-cost-window-and-the-count-gate-contract
reviewed: 2026-11-26T21:40:00Z
depth: standard
scope_base: 21d023c
scope_head: a275852
files_reviewed: 25
files_reviewed_list:
  - tests/runner-process-tap.sh
  - tools/check/steps/important-gates.sh
  - data/expected/important-gates.ids
  - data/expected/tests/runner-process.ids
  - docs/formats/ivsc-v1.md
  - docs/packaging.md
  - .planning/ops/RELEASE-OPS.md
  - .planning/ops/PLAN-LESSONS.md
  - .planning/ops/INDEX.md
  - .planning/REQUIREMENTS.md
  - .planning/STATE.md
  - .planning/ROADMAP.md
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
findings:
  critical: 1
  warning: 3
  info: 3
  total: 7
status: findings
fix_pass:
  at: 2026-11-26
  fixed: 5
  left_open: 2
  commits:
    - aee972a
    - 01baed9
    - b5473f6
    - 3c0daea
    - 7e1c1a2
---

# Phase 17: Code Review Report

**Reviewed:** 2026-11-26T21:40:00Z
**Depth:** standard
**Files Reviewed:** 25
**Status:** findings

## Summary

The phase's executable change is one new case, `runner-process.judge.flatwrap-shim`, and it survives
adversarial reading: five limbs, no typed digest value, a genuine liveness guard (one flipped input byte must
move both digest limbs) and a must-fail stand-in the same `shim_join` refuses with exactly `join-images`. I
re-ran `shim_join` against synthetic reports in a scratch directory to confirm the stand-in limb refuses for
the claimed reason and that its findings land in the subshell's own `why` target rather than the case's — both
hold. The block is `sh -n` and `dash -n` clean, uses no bashism, no process substitution, no GNU-only `grep -o`
or `sed -i`, and writes nothing outside the suite's `mktemp` directory. The registry rows resolve through the
kinds they claim (`runner-process.judge.flatwrap-shim` as a `<suite>.judge.<slug>` id, `extra-file` through
`data/expected/pkg-selftests.ids`), the count-gate exemplar's mechanism is committed code
(`pkg-install.sh:101`, `:241`, `:666-668`), and every invariant the brief names recomputes exactly:
`steps=38`, `faults=238`, `goldens=29`, `claims=29`, `provisional=28`, `prefix-files=11`,
`tests-expected=5044` (2522 ids x 2 presets), `tests-executed=5040`, `gates=120`, `lines=366`. All thirteen
control files are pure last-line appends and both pinned line numbers (`bad-req-phase5` line 19,
`bad-req-phase6` line 24) are byte-identical to the base. `### (i.1)` of `RELEASE-OPS.md` is `cmp`-identical
to its pre-phase state, and the canonical source-archive sentence is byte-identical in all three places it
appears.

The defects are all in the prose that records the change. The most serious is that section (i) of
`RELEASE-OPS.md` now says the line it just installed *was superseded* — the record reads as if the dead
v0.3.0 wording were current. Two lesser record defects follow from the same amendment, and `STATE.md`'s
Session Continuity block contradicts its own frontmatter.

No rights-bearing artifact, binary or new claim was committed (`data/rights/`, `data/claims.json`,
`data/golden/` and `data/expected/flat-refusals.ids` are untouched by the diff), and no unqualified
compatibility claim was minted.

## Disposition after the fix pass (2026-11-26)

Every Critical and Warning finding is fixed, and IN-01 with them. IN-02 and IN-03 are left open with the
reasons recorded beneath each finding; both are prose-only and neither moves a count, case, id, gate or
invariant. The gate is green under `sh`, under `dash` and in the Ubuntu container on the fixed tree.

| finding | disposition | commit |
|---|---|---|
| CR-01 | fixed | `aee972a` |
| WR-01 | fixed | `b5473f6` |
| WR-02 | fixed | `01baed9` |
| WR-03 | fixed | `3c0daea` |
| IN-01 | fixed | `7e1c1a2` |
| IN-02 | left open | — |
| IN-03 | left open | — |

Gate summary lines on the fixed tree (`tracked`/`files`/`hygiene-files` read 2810 rather than 2809 because
this report is still untracked):

- `sh tools/gate.sh` → `gate: PASS steps=38/38 controls=404/404 … faults=238/238 … goldens=29 … gates=120
  lines=366 … prefix-files=11 … tests-expected=5044 tests-executed=5040 tests-skipped=4 tests-missing=0 …`
- `dash tools/gate.sh` → the same line, byte for byte
- `sh tools/gate-linux.sh` → `gate-linux: exit 0`, the same tokens with the known container platform tokens
  (`cc=GNU`, `core-imports=5`, `archive=sha256`, `refs-verified=0/4`), `gate-time total=249`

The three runs above are on the final tree, after the disposition commit; the container log is
`build/gate-linux.log` and the earlier one was copied aside first (lesson 18).

Every invariant the brief names still recomputes: `steps=38`, `faults=238`, `goldens=29`, `claims=29`,
`provisional=28`, `prefix-files=11`, `tests-expected=5044` (2522 ids x 2 presets), `tests-executed=5040`,
`gates=120`, `lines=366`.

## Critical Issues

### CR-01: `RELEASE-OPS.md` section (i) says the line it installs was superseded, and mis-states what changed

**File:** `.planning/ops/RELEASE-OPS.md:80-82`
**Disposition:** **fixed** (`aee972a`). The pointer now reads *The line above **replaces** the v0.3.0 wording,
which was superseded on 2026-11-26 by plan 17-02 (EV-08)*, states the two changes that actually happened (the
added relative clause; `no host shim` → `no built host shim`) in place of the false *which is why it names the
source archive*, and the quoted v0.3.0 sentence is named as superseded and dated at the quote. The heading now
cites `GUARDRAILS-v0.4` section 7.4 for the v0.4.0 line and keeps `GUARDRAILS-v0.3` V6, which still binds where
cited. Guardrails section 2.12's shape is met: inline pointer at the wrong sentence plus the quoted, dated,
named-as-superseded sentence; nothing archived was rewritten.
**Issue:** The quoted canonical line at `:78` is the v0.4.0 release-decision line, and the paragraph
immediately beneath it reads:

> The line above was superseded on 2026-11-26 by plan 17-02 (EV-08), which is why it names the source archive
> rather than only the static package. The v0.3.0 wording is quoted here instead of being deleted, because the
> record of what was said then is append-only:

"The line above" is the replacement, not the superseded text — so the record states that the line the
milestone close must carry is dead, and that the v0.3.0 sentence quoted at `:84` is the live one. A reader
following `GUARDRAILS-v0.4` §7.4 (which this section implements) reaches the opposite conclusion from the one
the phase intended. The clause that follows is also false: the superseded v0.3.0 line already named the source
archive (`the release ships the static package and the source archive; no shared object, no host shim, no
content`), so "which is why it names the source archive rather than only the static package" describes a change
that did not happen. The actual change is the added relative clause (*which carries the source of the build-only
tools and of the build-only host shim*) and `no host shim` becoming `no built host shim`.

Consequence for the phase's own criterion 4: the superseded sentence is quoted beneath the corrected line, but
it is **not dated** — the date `2026-11-26` attaches to the wrong quote — and the corrected line carries no
in-place pointer saying it replaced anything. The two live documents therefore do agree sentence for sentence
on the line itself (verified byte-identical, three occurrences), but the record that explains the correction
asserts the reverse of what happened.

Secondary, same section: the heading at `:74` was re-versioned to `for v0.4.0` while its parenthetical still
cites `GUARDRAILS-v0.3 V6 and section 7.4`. The v0.4.0 line's source is `GUARDRAILS-v0.4` §7.4, which states
the same sentence plus the three deferred clauses.

**Fix:**
```markdown
## (i) The release-decision line for v0.4.0 (plan 15-02; recomputed by plan 17-02 against `GUARDRAILS-v0.4` section 7.4)

The milestone close carries exactly one release-decision line, in this wording:

> the release ships the static package and the source archive, which carries the source of the build-only tools and of the build-only host shim; no shared object, no built host shim

The line above replaces the v0.3.0 wording, superseded on 2026-11-26 by plan 17-02 (EV-08): it adds the
relative clause saying the source archive carries the source of the build-only tools and of the build-only host
shim, and it says `no built host shim` rather than `no host shim`. The v0.3.0 wording, superseded 2026-11-26,
is quoted here instead of being deleted, because the record of what was said then is append-only:

> the release ships the static package and the source archive; no shared object, no host shim, no content
```

## Warnings

### WR-01: `docs/packaging.md` now contradicts itself about whether the build-only tools are "shipped"

**File:** `docs/packaging.md:38` (and `:122`)
**Disposition:** **fixed** (`b5473f6`). Both surviving sentences are qualified the same way the corrected host-shim
sentence is: `never installed and never shipped as a built artifact`, and the source tarball is named as a
release member (`which the release does carry`). The canonical source-archive sentence is untouched and still
byte-identical in all three places.
**Issue:** Line 38 still reads that `ivs_pack`, `ivs_asm` and `ivs_flatwrap` "are never installed or shipped",
and line 122 still reads that none of them "is installed or shipped ... a reader builds the runner and the image
tools from the source tarball". The sentence the phase added immediately below line 38 (line 39) and below line
122 (line 123) now asserts that "the release ships the static package and the source archive, **which carries
the source of the build-only tools**". So the same paragraph says the tools are never shipped and that the
release ships their source, and the same bullet list says they are not shipped and that the source tarball a
reader builds them from is a release member (`:119`). The phase corrected exactly this imprecision for the host
shim (`never installed and never shipped` → `no built host shim`) and left the identical imprecision in place
for the tools one line above it, where the new clause now makes it a visible contradiction rather than a loose
phrase.

**Fix:** qualify the two surviving sentences the same way the corrected one is qualified.
```markdown
:38 ... are build-only: they have no install rule, are never installed and never shipped as a built artifact,
    and are not part of the core.
:122 - Neither the runner (`ivs-run`), nor `ivs_pack`, nor `ivs_asm`, nor `ivs_flatwrap` is installed or shipped
    as a built artifact, so nothing downloadable is runnable yet: a reader builds the runner and the image tools
    from the source tarball, which the release does carry.
```

### WR-02: the deferral sentence names one omitted clause while the new line omits three

**File:** `.planning/ops/RELEASE-OPS.md:86-89`
**Disposition:** **fixed** (`01baed9`). The deferral sentence now names all three omitted clauses — `no built
inspection tool` (Phase 18 builds the tool), `no trace or disassembly output` and `no content` — and says
Phase 20 recomputes the full section 7.4 line, with the content claim stated in `docs/packaging.md`'s
release-content list until then.
**Issue:** `GUARDRAILS-v0.4` §7.4 fixes the close line as *the release ships the static package and the source
archive, which carries the source of the build-only tools and of the build-only host shim; no shared object, no
built host shim, **no built inspection tool, no trace or disassembly output, no content***. The line the phase
installed stops at `no built host shim`, and the only explanation offered is "The inspection-limb clause of the
guardrails' close line is not written here". Three clauses are in fact absent, and one of them — `no content` —
was present in the v0.3.0 line that is quoted two lines above, so the record shows a rights-relevant assertion
disappearing from the release-decision line with no explanation. `docs/packaging.md:124` still carries the
content claim, so the two live documents no longer cover the same ground.

**Fix:** name every deferred clause in the one sentence that already explains the deferral.
```markdown
Three clauses of the guardrails' close line are not written here — `no built inspection tool` (the tool does not
exist until Phase 18), `no trace or disassembly output` and `no content` — and Phase 20 recomputes the full
section 7.4 line with all of them. The content claim is stated in `docs/packaging.md`'s release-content list
until then.
```

### WR-03: `STATE.md`'s Session Continuity block contradicts its own frontmatter

**File:** `.planning/STATE.md:741` (against `:9` and `:740`)
**Disposition:** **fixed** (`3c0daea`). The block now reads `Last session: 2026-11-26T21:01:59.000Z` / `Stopped
at: Completed 17-03-PLAN.md — lesson 32, the count-gate row, gates 120 / lines 366`, matching the frontmatter's
`stopped_at` and `last_updated`. The stray `Resume file: None` after the pre-existing Phase 13 note is outside
this block and was left untouched.
**Issue:** Frontmatter line 9 reads `stopped_at: Completed 17-03-PLAN.md — lesson 32 ...`, and the body's
`## Session Continuity` block still reads `Last session: 2026-11-26T20:45:02.896Z` / `Stopped at: Completed
17-02-PLAN.md`, while `last_updated` is `2026-11-26T21:01:59.000Z`. `Resume file: None`, so a session resuming
from this record is told the phase stopped one plan earlier than it did, and is pointed at 17-02's position
while `current_plan: 3` and `completed_plans: 3` say otherwise. This is the one-hop record the next run reads
before asking anything, which is the case `AGENTS.md` says must not happen. (The block looks
harness-generated — the plan executors are forbidden to stage orchestrator state — but the contradiction is
committed either way.)

**Fix:** regenerate the block from the frontmatter when the plan-metadata commit runs.
```yaml
## Session Continuity

Last session: 2026-11-26T21:01:59.000Z
Stopped at: Completed 17-03-PLAN.md — lesson 32, the count-gate row, gates 120 / lines 366
Resume file: None
```

## Info

### IN-01: the `judge-vacuous` guard on an empty stand-in file cannot fire, and a judgement path aborts the suite

**File:** `tests/runner-process-tap.sh:1964-1967`
**Disposition:** **fixed** (`7e1c1a2`). The limb is now `if ! grep -E '^(frames|digest[.])' … > stand-in.out;
then ok=0; why judge-vacuous …` — the guard is the only handler, so it is reachable and a judgement no longer
truncates the TAP stream. Shown firing one-off in a scratch directory against a report with no matching line:
before, exit 1 with no `why` line (guard never ran); after, `ok=0` and `judge-vacuous: the stand-in was built
from no lines of the first harness report`; against a full report the guard stays silent and the stand-in holds
its two lines. No case, id or fault added.
**Issue:** `grep -E '^(frames|digest[.])' ... > "$fwf/stand-in.out" || exit 1` aborts the whole suite when
`grep` matches nothing, so the following `if [ ! -s "$fwf/stand-in.out" ]` branch (finding `judge-vacuous`, "the
stand-in was built from no lines of the first harness report") is unreachable: `grep` either wrote at least one
line and returned 0, or returned 1 and the shell has already exited. A check that cannot fire is the defect
class lesson 20 names. The `exit 1` also turns a judgement into a truncated TAP stream with no trailer, which
`tests-inventory` reports as `tests-truncated` without naming the case.

**Fix:**
```sh
    if ! grep -E '^(frames|digest[.])' "$tmp/runs/fwshim-a-harness.out" > "$fwf/stand-in.out"; then
      ok=0; why judge-vacuous 'the stand-in was built from no lines of the first harness report'
    fi
```

### IN-02: the clause-1 exemplar's own count token is counted from the observed listing, not from the named file

**File:** `.planning/ops/PLAN-LESSONS.md:103` and `data/expected/important-gates.ids:632`
**Disposition:** **left open.** The correction is prose in a live document (`PLAN-LESSONS.md`) plus the registry
comment, and under `GUARDRAILS-v0.4` section 2.12 a live document gets an inline pointer *and* an appended
amendment — more than the one-line, zero-risk change this pass is scoped to. It would also make
`17-03-SUMMARY.md`'s quoted clause-to-mechanism map and its `count-gate` comment description stale, which is a
records edit this fixer may not make to a completed plan record. No count, case, row, gate or invariant moves
either way, so the correction belongs to Phase 20's stale-text sweep, where the count-gate contract and its
exemplar are recomputed. Recorded here rather than silently dropped.
**Issue:** Lesson 32's clause-1 bullet and the registry comment both say "a count taken from anywhere else is a
finding rather than a number", citing `pkg-install`'s `check_inventory`. The step's count token is
`prefix-files=` (`pkg-install.sh:864`), and its value is `$files`, assigned at `pkg-install.sh:242` as
`grep -c . "$T/inv.have"` — the count of the **observed** installed listing, not of the named file
`data/expected/pkg-install-files.ids` (`$inv` → `$T/inv.want`). The named file pins the count only by
reconciliation (`comm -23` / `comm -13`, `:113-114`), which is a weaker shape than clause 1 as written. The
project's direct-count exemplar is `ci-log`, whose `ci-runs=<rows>` is counted inside the `awk` that reads
`.planning/ci-runs.tsv` (`tools/check/steps/ci-log.sh:26`, `:59`, `:68`) — the file `17-CONTEXT.md` itself
calls "the count gate that already names the file it counts". A reader applying clause 1 literally will find
the cited mechanism does not instantiate it.

**Fix:** add one sentence to the clause-1 bullet (and the same to the registry comment), e.g.
"`prefix-files=` is counted from the observed listing and pinned to the named file by `comm`; the direct-count
shape clause 1 describes is `ci-log`'s `ci-runs=<rows>`, read from the named record inside the `awk` that
validates it." No row, case or count changes.

### IN-03: EV-08 read Complete for one commit before its count-gate clause had an artifact

**File:** `.planning/REQUIREMENTS.md:21` and `:88` (flipped in `be69a5e`, the last commit of plan 17-02)
**Disposition:** **left open, deliberately.** The reviewer's own fix (record the ordering in
`17-02-SUMMARY.md`) would rewrite a completed plan record, which `GUARDRAILS-v0.4` section 2.12 forbids: a
disposition goes in a live record, never into a rewrite of an archived record or phase record. The ordering is
recorded here instead — EV-08's flip landed in `be69a5e`, lesson 32 and its `count-gate` row landed one commit
later in `bfa0815` — and both plan records already declare `requirements-completed: [EV-08]`, so the phase's
final state is correct. No code change is available that would close a one-commit traceability gap
retroactively.
**Issue:** EV-08's text requires "the count-gate contract ... recorded as a numbered lesson **with a
mechanism**". The flip landed in `be69a5e`; lesson 32 and its `count-gate` row landed one commit later, in
`bfa0815`. For that one commit the requirement read Complete while one of its five stated conditions had no
artifact, and `important-gates` was green only because EV-08 was already covered by the `cart-role` row. Both
plan records declare `requirements-completed: [EV-08]`, so the phase's final state is correct; this is a
one-commit traceability gap for anyone bisecting the requirement.

**Fix:** no code change — record the ordering in `17-02-SUMMARY.md` (the flip is carried in 17-02 by the frozen
plan, and the lesson lands in 17-03), or move the flip to `bfa0815` if a future re-run of the plan is taken.

---

## Verified clean (evidence, so a later reader need not re-derive it)

- **Non-vacuity of the new case.** Re-ran `shim_join` in a scratch directory against synthetic harness/runner
  reports: the stand-in built from container A's lines judged against container B's report returns 1 with
  exactly one finding, `join-images: harness '<A>' runner '<B>'`, and the parent `$tmp/why` is untouched (the
  subshell's `whytarget` is a plain assignment, so it persists for `shim_join` — no bashism, works under
  `dash`). `[ "$_fw_sj" -eq 0 ]` → `judge-vacuous` and the `grep -q -e '^join-images: '` fallback to
  `judge-wrong-reason` both close the loop.
- **Liveness.** `join-immobile` is not redundant with limbs (2) and (3): if the flip were inert the two
  equality limbs would still pass, and only `fwi_a != fwi_b` catches it. The guard is on the harness side, the
  side being joined.
- **Portability.** `sh -n` and `dash -n` clean; no arrays, no `$( )` process substitution, no `local`, no
  `grep -o`/`sed -i`; `case $_sj_f in '' | *[!0-9]*)` and `printf '\001'` are POSIX. Every path used is under
  `$tmp` (`fwshim`, `fwshim-flat.bin`, `fwshim-wrap{1,2}.err`, `runs/*`, `fwshim/fakes/*`); nothing is written
  into the work tree.
- **The flipped byte stays legal.** 8192 zero bytes at base `4000` is 4096 words (`4000 + 1000 = 5000`), and
  flipping the byte at offset 1 makes the first word `0x0100`, still `<= 03FF`, so `flatwrap-word-range` and
  `flatwrap-cross` both stay satisfied and the wrap succeeds.
- **The `exec:` deviation is sound.** `content exec boot-stub-2` cannot print `exec: -` (`runner/report.c`
  derives the label from the symbolic name), and the committed assertion
  `exec: project boot stub contract 2, not an EXEC sha256=` is strictly stronger for the stated property. It is
  recorded as deviation 1 in `17-02-SUMMARY.md` rather than silently substituted.
- **The join is real.** `slot.cartridge.sha256` is an `IVS_DIGEST_IMAGES` row
  (`core/src/machine/machine_digest.c:54-55`), so `digest.images` genuinely serialises the value the case
  compares, and the harness reads the tool's container through the shim (`tests/host/ivs_host_harness.c:430-435`).
- **The documented token contract is exact.** The eight line-initial tokens of the new
  `docs/formats/ivsc-v1.md` section `diff` silent against `data/expected/flat-refusals.ids` in **both** set and
  order; each trigger and boundary it states is backed by a committed `tests/pack-tap.sh` case
  (`:250-302` for empty/length/words/word-range, `:303-329` for base/cross), and the "eighteen numbered refusal
  reasons" it distinguishes itself from is exactly 18 (`docs/formats/ivsc-v1.md:79-110`). The `flatwrap-words`
  row's counter-intuitive claim (a 65,536-word input is refused by `flatwrap-cross`, above that by
  `flatwrap-words`) matches both the tool's check order (`tools/flatwrap/flatwrap.c:323-338`) and the pack case.
- **The registry.** `gates=120` (119 distinct gate names at `21d023c` + `count-gate`), `lines=366` (364 + 2),
  `cart-role` pre-existing with two rows. Both new rows resolve: `runner-process.judge.flatwrap-shim` is in
  `data/expected/tests/runner-process.ids` and matches the `<suite>.judge.<slug>` shape; `extra-file` is line 18
  of `data/expected/pkg-selftests.ids`. `count-gate` is not a step name, which is the intent (it registers a
  contract, as `d09-06-registration` does) and the only reason `gates` moved.
- **The controls.** All 10 toy registries and all 3 toy requirement records carry `EV-97` as their last line and
  no others were missed; the two `EXPECT` pins (`important-gates-unknown-id: line 19`, `: line 24`) still land
  on the same rows as at `21d023c`; `_good` has no `EXPECT`, so no stats line needed re-pinning;
  `important-gates-complete-uncovered`'s two expected findings are unaffected because EV-97 is both Complete and
  covered.
- **Lesson 32's shape.** Exactly one line begins `32. `; the three clause lines are indented list items, so
  `^N. ` parsing still yields 32 as the highest lesson (the duplicated `25.` at lines 38 and 42 is pre-existing,
  present at `21d023c`, and not introduced here); lessons 20 and 31 exist and say what lesson 32 attributes to
  them; `tools/check/pass-line.sh` returns 2 for a verdict followed by a line and 1 for no verdict, exactly as
  clause 3 states; `d09-06-registration mutant st-pass-line EV-07` is registered and cited, not duplicated.
  `INDEX.md`'s range now reads `1–32` and no `1–25` survives.
- **Invariants.** `faults=238` (`cat data/expected/faults/*.ids | grep -v '^#' | awk NF | wc -l`), `goldens=29`
  (`ls data/golden | wc -l`), `claims=29` / `provisional=28` (`data/claims.json`), `steps=38`
  (`ls tools/check/steps/*.sh`), `prefix-files=11`, `tests-expected=5044` (2521 ids at `21d023c` → 2522 now, x 2
  presets), `tests-executed=5040`, `gates=120`, `lines=366`. `data/expected/flat-refusals.ids`,
  `data/claims.json`, `data/golden/`, `data/rights/` and `pkg-install-files.ids` are untouched by the diff.
- **Records that agree.** `### (i.1)` of `RELEASE-OPS.md` is `cmp`-identical to `21d023c`; the canonical
  source-archive sentence is byte-identical across `docs/packaging.md:39`, `:123` and `RELEASE-OPS.md:78`; each
  plan's recorded `start-commit` sha exists and is an ancestor of `HEAD`; every commit's file set matches what
  its record claims. No binary, no rights-bearing byte and no new claim entered the diff.

---

_Reviewed: 2026-11-26T21:40:00Z_
_Reviewer: Claude (gsd-code-reviewer)_
_Depth: standard_

_Fix pass: 2026-11-26 — 5 fixed, 2 left open with reasons (IN-02, IN-03); gate green under `sh`, `dash` and the
Ubuntu container; every named invariant recomputes._
_Fixer: Claude (gsd-code-fixer)_

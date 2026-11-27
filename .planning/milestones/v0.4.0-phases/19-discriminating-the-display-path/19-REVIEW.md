---
phase: 19-discriminating-the-display-path
reviewed: 2026-11-27T06:01:16Z
fix_pass: 2026-11-27T07:20:00Z
depth: standard
files_reviewed: 28
files_reviewed_list:
  - core/src/stic/stic_regs.c
  - core/src/stic/stic_events.c
  - core/src/stic/stic_params.h
  - core/src/stic/stic.h
  - core/src/machine/machine_bus.c
  - core/src/machine/machine.h
  - core/src/machine/machine_snapshot.c
  - cmake/IvsSticTable.cmake
  - tests/machine/machine_stic_params.c
  - tests/stic/stic_disc.c
  - tests/stic/stic_cases.h
  - tests/stic/stic_main.c
  - tests/CMakeLists.txt
  - tools/disc-sweep.sh
  - data/scenarios/disc-sweep-probe.ivss
  - tests/runner-process-tap.sh
  - data/profiles/intv-2609-ntsc-family-p1.json
  - data/expected/profile-fields.ids
  - data/expected/ivsn-literals.ids
  - data/expected/important-gates.ids
  - data/expected/tests/stic.ids
  - data/expected/tests/runner-process.ids
  - data/expected/runner-scenarios.ids
  - data/expected/faults/stic.ids
  - data/discrepancies.json
  - docs/stic/reference-notes.md
  - docs/owner-images.md
  - .planning/phases/19-discriminating-the-display-path/19-DECISION.md
findings:
  critical: 0
  warning: 3
  info: 7
  total: 10
  necessity: 0
status: issues_found
---

# Phase 19: Code Review Report

**Reviewed:** 2026-11-27T06:01:16Z
**Depth:** standard
**Files Reviewed:** 28
**Status:** issues_found

## Summary

The core change is correct and the phase's central claims survive independent re-measurement. I re-ran the evidence rather
than reading it:

- `sh tools/gate.sh` → rc 0, zero `-- FAIL` lines, and the token line carries every invariant the phase is bound to:
  `steps=38/38 controls=405/405 claims=29 provisional=28 discrepancies=110 faults=238/238 goldens=29 gates=123 lines=378
  prefix-files=11 records=155 fields=155 unknown-fields=48 tests-expected=5114 tests-executed=5108 tests-skipped=6
  tests-missing=0 stic-rules=67`.
- `ctest --preset dev -R '^stic$'` → `executed=111 skipped=0 failed=0`, and the binary's own stdout reproduces the matrix
  in `19-DECISION.md` exactly: masks `0xAAAA`, `0xCCCC`, `0xF0F0`, `0xFF00` and
  `the diagonal rejected the entangled guest at cell 2 (bits 0x2), observed 1`.
- `ctest --preset dev -R '^runner-process$'` → green, with `ok 128 …disc-sweep.ci-refusal`, `ok 129 …output-tree`,
  `ok 130 …bound` and `ok 36 …disc-sweep-probe # SKIP … (ended missing-fixture, exit 13, as expected)`.

**Default reproduction (item 1).** Each of the three branches short-circuits on its parameter at the shipped value and is
byte-identical to the code it replaced: `machine_bus.c:90` requires `access_gating_when_off == 0` before touching `open`;
`stic_events.c:59` guards the latch clear; `stic_regs.c:45-57` reduces to the old masked match for every write and to the
old early return for every read once `exact` is consulted. The `$4021`/`$8021`/`$C021` aliases still route through
`ivs_stic_is_mode_select_addr` (`stic_regs.c:47`), which is the only reason `stic.alias.read-4021-switches-mode` stays
green. I also verified *why* the four snapshot literals moved and the six size/schema/header literals did not: the params
digest is written into the snapshot trailer (`machine_snapshot.c:346`), so a STIC-table-only digest change moves
`stepped-snapshot-sha256` and the three `cut-snapshot-*` values while leaving `header-bytes`, `trailer-bytes`,
`sections-total-bytes`, `total-bytes`, `schema-digest-sha256` and `header-prefix-hex` untouched. No state byte moved.

**Stack copies, not global mutation (item 2).** `sd_cell` (`stic_disc.c:1078`) does `*out = *p` from the `const` generated
row on every call, writes only through `out`, and returns 0 when a field is not reachable; `sd_observe*` builds a fresh
machine per reading (`sd_blank`). There is no path by which cell *k* can see cell *k-1*, so the matrix is order-independent
by construction, not by luck. `c.stic` is `const ivs_stic_params *` (`machine.h:240`), so no const-cast exists to abuse.
**Minor caveat:** `sd_measure` composes the stamps from the shipped `p->ticks_per_cpu_cycle`/`p->access_strobe_ticks`
(`stic_disc.c:179`) while `sd_observe*` subtracts `q->access_strobe_ticks` and stamps with `q->ticks_per_cpu_cycle`; the two
agree today only because no cell perturbs those fields. See WR-02.

**The diagonal can fail (item 3).** The six `stic-disc mutant` rows are one per committed stand-in, `stic.judge.disc-
diagonal-two-fields` included, and each guest's sweep entry uses the observable its own case asserts (`sd_guests`,
`stic_disc.c:1141`): G1/G2 `blocked_writes`, G3 frame-0 `windows_active`, G4 the three-element vector. G1's `displayed`
reading is committed as the *entangled control* in `stic.judge.disc-diagonal-two-fields`, not as a registration — as the
phase records.

**`W1`'s chain (item 4).** Commit `6cffdb4` really is the causing commit and carries the whole chain in one place: the
`stic.display_latch_cleared_each_frame` row with tag `unknown-default-chosen`, its `none-found:` cite, conflict `C82`,
resolving evidence, revisit trigger and `decision: null`; `data/expected/profile-fields.ids`; `data/expected/ivsn-literals.ids`
with **all five** digest-carrying values retargeted; C82's `evidence` sentence; the three branches. `data/expected/
discrepancies.ids` is untouched (`discrepancies=110`).

**The exit record was not improvised (item 5).** The Exit B enumeration extracted from `19-DECISION.md` is byte-identical
between `2118d71`'s blob and `HEAD` (`cmp` clean, 9 bullets); `git diff 2118d71..HEAD -- 19-DECISION.md` removes exactly one
line, the `--- a/…` header, and adds exactly one section heading (`## Measured matrix`). `9b8c720`'s file list is those 9
paths, equal in both directions. The exit stated (`## Measured matrix` → "Exit B taken") is the enumeration honoured.

**The probe cannot leave `refs/` and cannot gate (item 6).** `tools/disc-sweep.sh` parses under `sh -n` and `dash -n`, is
not a gate step (`steps=38`), is `skip-ok` in both inventories, and its three refusal cases judge a copy in a scratch root.
The deletion of the output path is checked before the first run and no file is left behind on any refusal.

**Nothing moved that must not (item 7) and the grammars are clean (item 8).** Verified by the gate run above; both result
files use `ok <n> <id>` with no dash.

**Adjudication of deviation 2 (the unsatisfiable verify predicate).** The plan's limb
`Observation, \d{4}-\d{2}-\d{2} \(Phase 19 sweep[^.]*\.` cannot satisfy its companion limb "the matched text contains
`stic.disc.sweep-diagonal`": `[^.]*` cannot cross a dot, and the id's first segment is `stic.`, so the match necessarily
ends at `(Phase 19 sweep, stic.` for *any* wording that keeps both the `(Phase 19 sweep` literal and the id. It is not a
record defect, it is a predicate defect, and the executor was right to run the equivalent form rather than reword four
discrepancy notes to satisfy a scanner.

**The records do satisfy the criterion's intent.** Measured directly from `data/discrepancies.json`: each of C1, C6, C82 and
C94 carries exactly one new `Observation, 2026-11-27 (Phase 19 sweep, stic.disc.sweep-diagonal)` sentence, the `Observation, `
count grew by exactly one in each (C82 reads 2 because its pre-existing 2026-11-26 sentence is still there), all four keep
`status: open`, their `unknown-default-chosen:` policy prefix and a valid resolver. The intent — a *new* dated observation
naming the sweep, distinct from the pre-existing one — is met exactly.

**But the check should be corrected, not worked around.** The workaround is right for this frozen run; the durable defect is
in the plan text. The predicate should be two independent greps (a dated shape that did not exist at the start commit, and
the id inside the note), which is what the equivalent form already does — so the fix is to write the two-limb form into
`PLAN-LESSONS.md` as a fourth shape of the lesson-32 class ("a predicate whose two limbs cannot both be satisfied"), so the
next planner writes it correctly rather than recording the same deviation again.

One recorded process mismatch is not a code defect and is not numbered below: `6cffdb4` adds one file
(`data/expected/faults/stic.ids`) that is not in plan 19-01's `files_modified`, forced by `faults-verdict`'s
allowed-prefix column. It is disclosed in `19-01-SUMMARY.md` under Rule 3 and adds no fault site (`faults=238/238`).

No Critical finding. The three Warnings are a stale public-header contract, a fivefold copy of the bus-replay loop whose
two hardcoded copies would silently mis-stamp a future guest, and a probe option whose every accepted value is inert.

## Warnings

### WR-01: The public declaration comment for `ivs_stic_effect` states a guarantee the code no longer provides

**File:** `core/src/stic/stic.h:251-253`
**Issue:** The comment still says *"A read has an effect only when its address is in stic.mode_select_addrs (decided on
the address before decode, A-07)."* That was true of the unconditional early return removed by this phase. It is now false as
a description of the mechanism: `ivs_stic_effect` matches a READ by **exact address** for every address that is not a
mode-select member (`stic_regs.c:45-57`), so the `$0020` READ row is reachable and a caller that sets its effect to anything
but `none` gets an effect. It is observationally true only because the shipped profile's only non-mode-select READ row
carries `none`. `stic_regs.c`'s own file header was updated accurately; the declaration a consumer reads was not. This is the
comment that would let a future implementer "fix" the exact-address limb back out of existence, and it is the only place in
the tree that still claims the old mechanism.
**Fix:**
```c
/* Access effects (stic_regs.c, plan 06-06; plan 19-01 for the READ match; pure functions of the parameters, no address
 * constant). access is IVS_STIC_ACCESS_READ (a READ or FETCH) or IVS_STIC_ACCESS_WRITE (a WRITE or DW); the result is an
 * IVS_STIC_EFFECT_*. A WRITE or DW matches a row by register index (address & 63, the write aliases included). A READ whose
 * address is a member of stic.mode_select_addrs keeps the register-index match (so $4021 still selects Color Stack mode,
 * A-07); a READ of any other address matches stic.access_effects by EXACT address, so the $0020 READ row is reachable and a
 * read of $0040 still has no effect (C84). */
```

### WR-02: The bus-replay loop is copied five times, and two copies hardcode the access kind and address

**File:** `tests/stic/stic_disc.c:223` (and `:269`, `:482`, `:525`, `:950`)
**Issue:** The same 12-line body — build an `ivs_machine_bus_ctx`, derive `unit_start_tick = h->stamp[i] -
q->access_strobe_ticks`, set `c.stic = q`, replay, then catch up — appears in `sd_observe_x`, `sd_observe_hits`,
`sd_words_at`, `sd_equiv_dead` and `sd_g4_vector`. The stamp composition is the one thing the family's whole claim rests on
("the access is replayed at the stamp the guest itself measured"), and it now exists in five independent copies. Two of them
(`sd_words_at`, `sd_equiv_dead`, `sd_g4_vector`) hardcode `a.kind = IVS_ACC_WRITE` and `a.addr = 0x0020u` instead of taking
them from the hits. Today every caller that reaches them is used only with G1's `$0020` write, so nothing is wrong; the day
a guest's observable is read at another address or of the other kind, those three will replay the wrong access and the cell
will report a plausible number rather than a break. This is the divergence risk `PLAN-LESSONS` lesson 13 names, applied to
the stamp rather than to a timing assert.
**Fix:** one helper, and the five call sites keep their own catch-up policy:
```c
/* Replays the measured hits through the bus wrapper with the row q. `filter` is SD_READS, SD_WRITES or SD_ANY
 * (SD_ANY takes each hit's own kind). The stamp composition lives here and nowhere else. */
static void sd_replay(struct ivs_machine *im, const ivs_stic_params *q, const sd_hits *h, uint32_t filter, uint16_t addr)
{
  size_t i;
  for (i = 0u; i < h->n; i++) {
    ivs_machine_bus_ctx c;
    ivs_cp1610_access a;
    uint32_t w = (uint32_t)h->write[i];
    if (filter != SD_ANY && w != filter) {
      continue;
    }
    memset(&c, 0, sizeof c);
    c.m = im;
    c.unit_start_tick = h->stamp[i] - q->access_strobe_ticks; /* the access is offset 0 of its unit */
    c.ticks_per_cpu_cycle = q->ticks_per_cpu_cycle;
    c.psg = ivs_psg_params_get(im->profile);
    c.stic = q;
    memset(&a, 0, sizeof a);
    a.kind = w != 0u ? IVS_ACC_WRITE : IVS_ACC_READ;
    a.addr = addr;
    (void)ivs_machine_bus(&c, &a);
  }
}
```
`sd_observe_x` then becomes `sd_replay(im, q, h, is_write, addr)` and `sd_observe_hits` becomes
`sd_replay(im, q, h, SD_ANY, addr)`; `sd_words_at`/`sd_equiv_dead`/`sd_g4_vector` pass `SD_WRITES, 0x0020u`. The two
functions collapse into one.

### WR-03: `--max-runs` cannot change the sweep; every accepted value runs the same six offsets

**File:** `tools/disc-sweep.sh:86-91`
**Issue:** With the fixed six-entry offset list, the accepted range of `--max-runs` is exactly 6..16, and every value in it
runs all six offsets and writes the same table. Below 6 it refuses ("the run count 6 is above the bound N"); above 16 it
refuses ("above the bound 16"). So the option's only observable effects are two refusals, and its name — and `usage: …
[--max-runs <n>]` — reads as "run at most N". The only exercised limb is the refusal
(`runner-process.disc-sweep.bound` passes `--max-runs 100000`), so nothing in the suite would notice. A reader of the
header will infer a cap that does not exist, and the next person to add an offset list will assume the option already
truncates.
**Fix:** honour it as a cap and record the truncation in the table's own header, so the table states how much of the list it
covered:
```sh
if [ -n "$max_runs" ]; then
  case $max_runs in '' | *[!0-9]*) die "$usage" ;; esac
  [ "$max_runs" -ge 1 ] || die "$usage"
  [ "$max_runs" -le "$max_bound" ] || die "the run count $max_runs is above the bound $max_bound"
fi
bound=${max_runs:-$max_bound}
# ... then walk the fixed list and stop once $bound runs have been made, printing "$bound" in the `# disc-sweep:` header line
```
If the intended semantic really is a caller-declared ceiling rather than a cap, rename it (`--allow-runs`) and say so in the
header, and add one `runner-process.disc-sweep.bound` limb for an accepted in-range value so the middle of the range is
exercised at all.

## Info

### IN-01: Two file headers describe the changed events without the parameter that now decides them

**File:** `core/src/stic/stic_events.c:15-16` (and `core/src/machine/machine_bus.c:20-31`)
**Issue:** The events header still reads *"FRAME_START clears display_latch, windows_active and grants_in_frame and opens the
register gate and the graphics-memory gate"* with no mention of `display_latch_cleared_each_frame`, and the bus header's
description of the gate decision has no mention of `access_gating_when_off`. Both changes carry an accurate inline comment at
the branch, and `stic_regs.c`'s header was amended rather than left alone — so the tree is now inconsistent about the same
convention. `stic_params.h:95-103` is the only place a reader learns the parameters exist.
**Fix:** append one `Plan 19-01 (D-54, C82/C94): …` paragraph to each file header, in the same shape `stic_regs.c` uses, and
leave the 06-06 narrative intact (append-only, per `GUARDRAILS-v0.4` §2.12).

### IN-02: `sd_hits.cycle[]` is written and never read

**File:** `tests/stic/stic_disc.c:138`
**Issue:** `SD_HIT_MAX` cycles are recorded at `:181` (`h->cycle[h->n] = …`) and no reader exists anywhere in the file. It
is a dead field carrying a division per recorded hit.
**Fix:** delete the field and the assignment, or use it — the natural use is the `SD_G1_BROKEN` diagnostic, which currently
reports a tick where a CPU cycle would be the readable number.

### IN-03: The `else()` branch of the new gating select is unreachable (dead code)

**File:** `cmake/IvsSticTable.cmake:378-383`
**Issue:** `st_policy(gating_off stic.access_gating_when_off "apply" …)` is called 5 lines above and its body is
`if(NOT "${${var}}" STREQUAL "${want}") st_fail(...) endif()` with `st_fail` = `message(FATAL_ERROR)`
(`cmake/IvsSticTable.cmake:76-78`, `:194-201`). Configure therefore cannot reach `if(gating_off STREQUAL "apply")` with any
other value, so the `else() st_fail("no other access-gating policy is implemented")` can never run and duplicates a
diagnostic `st_policy` already produced with a better message. It is dead code, not a necessity finding: the branch does not
run at all.
**Fix:** drop the `else()` and keep `set(gating_apply 1)` inside the `if`, or keep it and say in the comment that it is the
guard for the day `st_policy` stops being fatal — in which case the second half of the risk (a stale `gating_apply` leaking
across `foreach(st_profiles)` iterations because `set()` is directory-scoped) also needs an `unset(gating_apply)` at the top
of the loop.

### IN-04: The second run-count check is always true

**File:** `tools/disc-sweep.sh:91`
**Issue:** `[ "$runs" -le "$max_bound" ] || die "the run count $runs is above the bound $max_bound"` runs after the
`--max-runs` block. `runs` is the length of a fixed six-entry literal and `max_bound` is 16, so the test is a constant
comparison. It cannot fail without editing both literals in the same file, and if that ever happened line 88/89 would already
have fired for a supplied `--max-runs`.
**Fix:** remove it, or fold the bound into one assertion on the list length next to the definition of `offsets`
(`[ "$(printf '%s\n' $offsets | wc -l)" -le "$max_bound" ]`), so the invariant is stated once.

### IN-05: Exit A's trigger is interpreted only in the post-measurement section

**File:** `.planning/phases/19-discriminating-the-display-path/19-DECISION.md:241-245`
**Issue:** The record's opening promises that nothing which decides the outcome is written after the measurement, and the
enumeration half of that promise holds (verified byte-identical). The *rule* half does not: the sentence that turns the
plan's "Exit A when exactly one field discriminates" into "exactly one field yields evidence that prefers one of its
settings" appears only inside the appended `### Why the measurement is Exit B` section, i.e. after the sweep ran; a reader of
`2118d71`'s blob cannot predict which exit the measurement will produce. This is **not** a fit: `GUARDRAILS-v0.4` §2.6 and
D-54 already define a diagonal with no preference as "nothing discriminates → default unchanged", and under either reading
the outcome is B. But the record's own standard ("a reader can check it rather than infer it") is met only after the fact.
**Fix:** the record is append-only, so add the reading to `.planning/ops/PLAN-LESSONS.md` (the plan-satisfiability class,
beside lesson 32) — *a phase-exit trigger must be interpreted in the plan or the pre-measurement record, never only in the
appended result section* — and, in the next plan that carries an exit choice, state the reading in the frozen block.

### IN-06: `PROJECT.md`'s status line still says "after Phase 18" while its body now describes Phase 19

**File:** `.planning/PROJECT.md:459`
**Issue:** The single status line was extended in place with a Phase 19 paragraph but its prefix still reads
`*Last updated: 2026-11-27 after Phase 18 — The Inspection Surface.` A reader scanning the last line for the project's
state gets the wrong phase number; `STATE.md` and `ROADMAP.md` both say 19.
**Fix:** change the prefix to `after Phase 19 — Discriminating the Display Path` and leave the appended paragraphs as they
are (append-only).

### IN-07: `19-VALIDATION.md` still describes an unstarted phase

**File:** `.planning/phases/19-discriminating-the-display-path/19-VALIDATION.md:4` (pre-existing; last touched at the
scope base `57fe055`, not by this phase's commits)
**Issue:** For a phase whose three plans are complete and whose requirement is `Complete`, the validation contract still
reads `status: draft`, `nyquist_compliant: false`, `wave_0_complete: false`, every task in the Per-Task Verification Map
`⬜ pending` with `File Exists: ❌ W0`, and every Wave 0 box unchecked. A `/gsd:verify-work` or `/gsd:audit-milestone` pass
reads this file as its contract and will find nothing to sample.
**Fix:** set `status: complete`, `nyquist_compliant: true`, `wave_0_complete: true`, mark the eight task rows and the four
Wave 0 boxes, and record `gate-linux: not run` with the docker-daemon reason as a visible not-run line rather than leaving
the sign-off block empty.

---

_Reviewed: 2026-11-27T06:01:16Z_
_Reviewer: Claude (gsd-code-reviewer)_
_Depth: standard_

---

## Disposition (fix pass, 2026-11-27)

The three Warnings and two of the seven Info findings are closed by code or record changes, IN-04 is closed as a side
effect of WR-03's rewrite, and the four remaining Info findings are left open with the reasons below. Each closed row
names the commit; a row with no commit is a recorded decision, not an omission. The dispatch's scope was the three
warnings, `IN-06`, `IN-07` and the new `PLAN-LESSONS` entry, with `IN-01`…`IN-05` to stay open where cosmetic.

| Finding | Disposition | Where |
|---|---|---|
| **WR-01** the declaration comment stated the removed READ rule | **fixed** — the comment now states the two-limb READ match (mode-select keeps the register-index match, every other READ matches by exact address), so the one comment a consumer reads no longer describes the old mechanism | `48c7ba4` |
| **WR-02** the bus-replay loop was copied five times | **fixed** — `sd_replay_one` is the only place the stamp is composed and the only place the access kind is decided (from the hit, never a literal); `sd_replay` is the kind filter; `sd_observe_hits` collapses into `sd_observe_x` with `SD_ANY`; `sd_g4_vector` keeps its ascending pass and calls the helper. No behaviour change, measured | `812b838` |
| **WR-03** `--max-runs` was inert above the refusal | **fixed** — the option is a cap on the walk, the table's header states how much of the list it covered, and `runner-process.disc-sweep.bound` asserts an accepted in-range value (two rows, the first two offsets, a header naming the cap). No id added | `f24ad2c` |
| **IN-01** two file headers omit the parameter that now decides the event | **open, out of scope** — both branches carry an accurate inline comment, `stic_params.h` is where a reader learns the parameters exist, and the remedy is an append-only paragraph per header (§2.12) that belongs with the next commit touching those files | — |
| **IN-02** `sd_hits.cycle[]` is written and never read | **open, out of scope** — deliberately left alone by WR-02, whose contract was "no behaviour change"; the natural fix changes a diagnostic's text (`SD_G1_BROKEN` reports a tick where a CPU cycle reads better), so it wants its own commit and its own before/after evidence | — |
| **IN-03** the gating select's `else()` is unreachable | **open, out of scope** — the branch cannot run, so it can neither pass nor fail anything. Its second half (a stale `gating_apply` leaking across `foreach(st_profiles)` iterations) wants an `unset()` plus a configure-time negative control, which is a build-time policy change and not a review tidy-up | — |
| **IN-04** the second run-count check was always true | **fixed as a side effect of WR-03** — the constant comparison is gone; the list-length invariant is asserted once, next to the list definition | `f24ad2c` |
| **IN-05** Exit A's trigger is interpreted only after the measurement | **open, append-only** — the record cannot be rewritten (§2.12), and the finding's own remedy is a `PLAN-LESSONS` entry. This pass added lesson 34 (the predicate a correct record cannot satisfy); IN-05's lesson — *a phase-exit trigger must be interpreted in the plan or the pre-measurement record* — is owed to the next plan that carries an exit choice | — |
| **IN-06** `PROJECT.md`'s status line still said "after Phase 18" | **fixed** — the prefix names Phase 19 and states the outcome in one sentence (Exit B, the diagonal matrix, the field unchanged); the appended paragraphs are untouched | `12a9cdd` |
| **IN-07** `19-VALIDATION.md` described an unstarted phase | **fixed** — `status: complete`, `nyquist_compliant: true`, `wave_0_complete: true`, the seven shipped tasks green against commands run in this pass (V1–V5), the six Wave 0 boxes checked, and the container gate as a visible `not run` line | `fb94092` |

**The `PLAN-LESSONS` entry the review asked for** (its adjudication of deviation 2): lesson 34, the fourth shape of the
lesson-32 class — *a verify predicate that a correct record cannot satisfy* — with the two-limb rule that the predicate
must be shown to match a **constructed** correct example and to fail a constructed wrong one before it is committed,
and the corollary that an "equivalent form" run by hand is a workaround, not a check.

### Verification after the fix pass

Run in the **main checkout** (`workflow.use_worktrees` is `false` in `.planning/config.json`, the documented opt-out),
so every figure is reproducible from the committed tree.

`stic` suite (`IVS_TEST_OUT_DIR=<tmp> ctest --preset dev -R '^stic$'`):

```
# suite=stic executed=111 skipped=0 failed=0
```

111 `ok` lines, 0 `not ok`, and the result file's id set is byte-identical to `data/expected/tests/stic.ids`. The
matrix masks are unchanged: `G1 field 0 mask 0xAAAA`, `G2 field 1 mask 0xCCCC`, `G3 field 2 mask 0xF0F0`,
`G4 field 3 mask 0xFF00`, and `the diagonal rejected the entangled guest at cell 2 (bits 0x2), observed 1`.

`runner-process` suite: `# suite=runner-process executed=164 skipped=3 failed=0`, 167 ids identical to
`data/expected/tests/runner-process.ids`, with `ok 128`–`ok 130` for the three `disc-sweep` cases and the probe's
`# SKIP` line at `ok 36`.

**Gate after the fix pass** — `sh tools/gate.sh` (rc 0, 69 s) then `dash tools/gate.sh` (rc 0, 67 s), sequentially and
never concurrently (lesson 33), zero `-- FAIL` lines under each, the two summary lines compared whole with `cmp`
(**identical**):

```
gate: PASS steps=38/38 controls=405/405 claims=29 provisional=28 discrepancies=110 faults=238/238
goldens=29 gates=123 lines=378 prefix-files=11 records=155 fields=155 unknown-fields=48
tests-expected=5114 tests-executed=5108 tests-skipped=6 tests-missing=0 stic-rules=67
```

Every invariant the phase is bound to held: no id added or removed (`tests-expected=5114`, `tests-executed=5108`,
`tests-skipped=6`), no step, control, fault, golden, claim, installed file, profile value or rule statement moved. The
gate printed exactly two skip lines for the probe, one per preset:

```
-- SKIPPED dev/runner-process/runner-process.scenario.disc-sweep-probe: disc-sweep-probe: needs operator content (ended missing-fixture, exit 13, as expected)
-- SKIPPED release/runner-process/runner-process.scenario.disc-sweep-probe: disc-sweep-probe: needs operator content (ended missing-fixture, exit 13, as expected)
```

The gate's file counts read `tracked=2852` / `files=2852` / `hygiene-files=2852` at the moment it ran: 2,850 tracked
paths plus this untracked review and its fix report under judgement. Committing both leaves `tracked` at 2,852, which
is the only token this pass moves and is a record count, not evidence.

**`gate-linux: not run`** — the docker daemon is unresponsive on this host (measured in Phase 18: `timeout 20 docker
info` → rc 124, twice), so the container gate is recorded as not run and never as a pass (lesson 24). No `ivs-*`
image, container or volume was created.

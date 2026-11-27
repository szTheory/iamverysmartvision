# Phase 19 Plan 01: Discriminating the Display Path Summary

Authored guests, parameter-driven branches and the display-latch profile row that make the
four rival display-path rules separable at the machine-state seam.

## Baselines

start-commit: 57fe0550248d0e814b1b5903b9ada185ae9bb83a

Measured at `start-commit` before the first edit of this plan:

| Count | Value | Read from |
|---|---|---|
| `stic` inventory data lines | 99 | `grep -v '^#' data/expected/tests/stic.ids \| awk 'NF' \| wc -l` |
| `profile-fields.ids` data lines | 154 | `grep -v '^#' data/expected/profile-fields.ids \| awk 'NF' \| wc -l` |
| `important-gates.ids` data lines | 372 | `grep -v '^#' data/expected/important-gates.ids \| awk 'NF' \| wc -l` |
| `important-gates.ids` distinct first columns | 122 | `grep -v '^#' data/expected/important-gates.ids \| awk 'NF{print $1}' \| sort -u \| wc -l` |
| `ivsn-literals.ids` data lines | 26 | `grep -v '^#' data/expected/ivsn-literals.ids \| awk 'NF' \| wc -l` |
| control directories (excluding `_good`) | 405 | `ls -d tools/check/controls/*/ \| grep -v _good \| wc -l` |
| gate steps | 38 | `ls tools/check/steps/*.sh \| wc -l` |
| profile `fields` members | 154 | `data/profiles/intv-2609-ntsc-family-p1.json` |
| rows tagged `unknown-default-chosen` | 47 | same file, `tag == "unknown-default-chosen"` |

Gate summary line measured at `start-commit` (sh only, before the first edit):

```
gate: PASS steps=38/38 controls=405/405 ... profile-keys=154 goldens=29 ... gates=122 lines=372 ... records=154 fields=154 unknown-fields=47 ... tests-expected=5082 tests-executed=5078 tests-skipped=4 tests-missing=0 ... stic-rules=67 stic-rows=64 ...
```

`stic` suite measured at `start-commit`, before the first edit (`IVS_TEST_OUT_DIR=<tmp>
ctest --preset dev -R '^stic$'`): `# suite=stic executed=99 skipped=0 failed=0`, 99 `ok` lines,
`1..99`. This is the "before" half of the byte-identity evidence for the three parameter
branches: the same suite, same 99 ids, same results after Task 2.

## Deviations

### Rule 3 - Blocking issue: G1's gating-coupling limb lands with the member, in Task 2's commit

The plan's Task 1 step (4) asks `stic.disc.g1-window` for a coupling limb "under a stack copy
with the gating member at 0", while Task 1 step (8) and Task 1's own verify require that no
parameter digest and no snapshot literal move in Task 1.

Those two are mutually unsatisfiable as written: the parameter digest is the SHA-256 of the
*generated row text* (`cmake/IvsSticTable.cmake` `st_body_digest(row_digest "${row_text}")`), so
a member the generator emits changes the digest and therefore all five digest-carrying literals
of `data/expected/ivsn-literals.ids`. Adding the member in Task 1 would move the digest in Task 1
and again in Task 2 — two retargets, with a red `state` suite between them, which is exactly what
lesson 1 forbids and what the plan's own Task 2 preamble forbids ("Splitting this into two commits
would move the digest twice and leave a red gate between them").

Resolution: the `access_gating_when_off` member, its generator emission, its range check, its
branch in `machine_bus.c` **and** `stic.disc.g1-window`'s gating-coupling limb all land in Task 2's
commit, which is the commit that already pays the single retarget. Task 1's case still records the
coupling it can measure on the tree as it stands — the criterion guest's displayed word takes both
values under the **window** field alone (limbs 1 and 2), which is the same statement the plan wants
("the displayed word is not a diagonal witness") and is what makes the blocked-write counter the
registered witness. Task 2 extends the same case with the two gating limbs. `tests/stic/stic_disc.c`
is in this plan's `files_modified`, so Task 2 touching it is inside the plan's enumeration.

### The §2.7 registry split (derived-accepted-set)

§2.7 requires every new check to ship a must-fail mechanism registered in
`data/expected/important-gates.ids` under its requirement, in the same commit as the check. That is
impossible in this plan: `tools/check/steps/important-gates.sh` **derives** its accepted set from
the requirement records' `Complete` ids and refuses a row naming a `Pending` requirement, and
VID-07 stays `Pending` until plan 19-03's second task flips it. The derived-accepted-set rule
therefore forces the split: all ten ids this plan registers (`stic.disc.g1-window`,
`stic.judge.disc-g1-dead`, `stic.disc.displayed-equivalence`, `stic.judge.disc-equivalence-dead`
in Task 1; `stic.disc.g2-gating`, `stic.disc.g3-read`, `stic.disc.g4-latch`,
`stic.judge.disc-g2-dead`, `stic.judge.disc-g3-dead`, `stic.judge.disc-g4-dead` in Task 3) land
without a registry row here, and their rows land in plan 19-03's Task 2 commit with the VID-07
flip. Phase 18's observation O4 is the precedent. Each id ships its liveness guard and its
committed stand-in in the same commit as the id, so only the registry row is deferred.

## Task 1: G1, its measured stamp, its guard and the displayed-word equivalence

`tests/stic/stic_disc.c` (new, in `IVS_STIC_TEST_SOURCES`, so the same list feeds `ivs_fault_suite(stic ...)`
and the module adds no fault site), `ivs_stic_disc_cases[]` in `stic_cases.h`, the concatenation in
`stic_main.c`, four inventory lines.

**Measured, not derived.** `sd_measure` steps the guest unit by unit through
`ivs_machine_step_unit_trace_internal` and composes each stamp exactly as `machine_bus.c` does
(`unit_start + offset_cpu_cycles x ticks_per_cpu_cycle + access_strobe_ticks`). The gap write of
`sd_prog_g1_criterion` (`MVII #0, R0` / `sc_w_delay(149)` / `MVO R0, $0020` / `B self`) lands at CPU cycle
2251, tick 9004 - asserted strictly inside `(p->reg_window_ticks, 2900 x p->ticks_per_cpu_cycle)` =
`(8000, 11600)`, computed from the profile's own parameters and never written as a tick literal.
`SD_G1_DELAY = 149` is the search start, not the expected value.

**G1's four limbs, all relations.** Criterion guest (one strobe at the gap): `displayed` 0 at the shipped
window, 1 at the 2900-cycle stack copy. Witness guest (the same delay and the same gap write with one
strobe in front of it): `blocked_writes` 1 at the shipped window, 0 at the copy. Plus the coupling limbs
added with Task 2's member (see the deviation): under `access_gating_when_off = 0` the criterion guest
displays frame 0 and the witness's counter stays 1.

**Stand-ins, proven able to fail.** Throwaway experiments, reverted before the commit:
- `sd_row_wide` reduced to a no-op copy (alternate == shipped) -> `stic.disc.g1-window` red with
  `limbs 0x0A` (the two wide-window limbs), so the guard's second value is load-bearing.
- `sd_prog_g1_dead` replaced by the real witness in the judge's slot -> `judge-vacuous`, red. Restored ->
  green.
- the coupling limb's `access_gating_when_off = 0` set back to 1 -> red with
  "the criterion guest's displayed word is 0, not 1 (blocked 1)". Restored -> green.

**Equivalence (research O2), pinned not chosen.** `stic.disc.displayed-equivalence` asserts
`windows_active` at the graphics-memory window close equals "the display latch is non-zero" **at that same
instant** - a tick equality and a value equality - in both polarities (both 0 on the shipped row, both 1 on
the alternate). `sd_equiv_dead` feeds the same guard `windows_active` read at the close and the latch read
after the next frame start's clear: on the shipped row both values are 0, so a value-only guard would
accept it, and `stic.judge.disc-equivalence-dead` requires the guard to reject it for the instant
(`judge-vacuous` / `judge-wrong-reason` otherwise).

**Before/after suite evidence.** Before the plan: `# suite=stic executed=99 skipped=0 failed=0` (99 `ok`
lines). After Task 1: `# suite=stic executed=103 skipped=0 failed=0`. `stic` inventory data lines 99 -> 103.

## Task 2: the three branches, the latch field and the whole W1 chain in one commit

### The members and the three branches

| member | profile row | value 1 (today's bytes) | value 0 (the alternate) | branch |
|---|---|---|---|---|
| `access_gating_when_off` | `stic.access_gating_when_off` (`apply`) | the windows apply whatever the display latch is | the windows are open while the display is off | `core/src/machine/machine_bus.c`, the gate decision |
| `display_latch_cleared_each_frame` | `stic.display_latch_cleared_each_frame` (new, value 1) | the latch is cleared at every frame start | the latch persists across frame starts | `core/src/stic/stic_events.c`, the frame-start event |
| - (no member) | the `$0020` **READ** row of `stic.access_effects` (effect `none`) | a read has no effect | the row's effect code, set on a stack copy | `core/src/stic/stic_regs.c`, the effect lookup |

The gating member is emitted from the `st_policy(gating_off ...)` result (a `set(gating_apply 1)` branch
that the fail-closed `st_policy` already makes unreachable for any other value), and the latch member from
the new row through `st_num(latch_clear ... 1)`; both are range-checked (`p-><member> <= 1u`) in the
existing loop of `tests/machine/machine_stic_params.c`, beside the `alias_read_sensitive` precedent.

`ivs_stic_effect`'s READ match is now two-limb and the old unconditional early return is gone: a READ whose
address is a member of `stic.mode_select_addrs` keeps the **register-index** match (so `$0021`, `$4021`,
`$8021` and `$C021` still select Color Stack mode and `stic.alias.read-4021-switches-mode` stays green),
and every other READ matches a row by **exact address** (so the `$0020` READ row is reachable and `$0040`
stays inert - a masked match would give `$0040` register index 0 and arm the display, which C84 forbids).
Writes keep the masked match.

### The retarget arithmetic (recipe proven by reproducing the old value first)

| step | value |
|---|---|
| preimage | ASCII `IVSP` + big-endian u16 `1` + the four body digests (memmap, PSG, STIC, controller) |
| preimage length | 134 bytes |
| control: start-commit profile + start-commit generator | `efba85009a57e91ad21060742dadc34949b976829525bb5e18d6b0897af1425c` - **reproduces the pinned value exactly** |
| body digests at start | memmap `c6e522ee...2413ed`, PSG `f19494a3...b2f88d`, STIC `ad2585ee...6e3a58bf`, controller `9b00c1c4...bd7848` |
| body digests now | memmap and PSG and controller **unchanged**, STIC `8493669c...0eb19c` (only the STIC table body moved) |
| `params-digest-sha256` | `ff4eb16ace37a4a7dd875a94daa5945f0ea65dcf4da657ede9356b412fe87d30` |

The four snapshot literals are the `state` suite's own diagnostics, copied in place in the same edit:
`stepped-snapshot-sha256` `6e08bf00...5515c3`, `cut-snapshot-sha256.hp-frame-mid` `78f960a3...083c62`,
`cut-snapshot-sha256.hp-latch` `5ad5ab30...5f711`, `cut-snapshot-sha256.hp-mob` `1cb793fa...163d3`. The six
size literals (`header-bytes`, `trailer-bytes`, `sections-total-bytes`, `total-bytes`,
`schema-digest-sha256`, `header-prefix-hex`) did **not** move: no STATE row was added and no section was
resized. `state.literal.params-digest` is green, and it also rebuilds the preimage from the specification,
so the retarget is held by two readers.

### The four profile counts

| count | before (`start-commit`) | after | read from |
|---|---|---|---|
| `fields` | 154 | 155 | `data/profiles/intv-2609-ntsc-family-p1.json`, `len(fields)` |
| `profile-fields.ids` data lines (= `profile-keys`) | 154 | 155 | `grep -v '^#' data/expected/profile-fields.ids \| awk 'NF' \| wc -l` |
| `unknown-fields` | 47 | 48 | the same profile file, rows tagged `unknown-default-chosen` |
| `records` | 154 | 155 | the profile record count the gate prints |

### The C82 evidence sentence, and `discrepancies.ids` non-movement

Appended to C82's `evidence` (one string, `note`/`status`/`default_policy`/`resolver`/`id` untouched):

> Plan 19-01: the sustain rule is now a profile field, stic.display_latch_cleared_each_frame = 1 (cleared at
> every frame start; the alternate 0 is the persist rule), emitted by cmake/IvsSticTable.cmake into
> ivs_stic_params.display_latch_cleared_each_frame and read at the frame-start event of
> core/src/stic/stic_events.c; the $0020 READ row of stic.access_effects is now reachable through the
> exact-address READ match in ivs_stic_effect (core/src/stic/stic_regs.c), which keeps the register-index
> match for stic.mode_select_addrs so $4021 still selects Color Stack mode (C84). Exercised by
> stic.disc.g1-window, stic.disc.g3-read, stic.disc.g4-latch, stic.disc.displayed-equivalence and their
> judges stic.judge.disc-g1-dead, stic.judge.disc-g3-dead, stic.judge.disc-g4-dead and
> stic.judge.disc-equivalence-dead. No new C id is minted, so data/expected/discrepancies.ids does not move.

No new C id is minted, so `data/expected/discrepancies.ids` is unchanged and `discrepancies=110` is the W1
limb's evidence: its non-movement is what proves no rival row was added or dropped.

### What this commit pays, and what plan 19-03 pays

This commit pays W1's **field-addition** half: the field row with its tag, cite, conflict, resolving
evidence, revisit trigger and null decision; the inventory line; the parameter digest and all five
digest-carrying snapshot literals; C82's `evidence`; the appended `docs/stic/reference-notes.md` note; and the
`PROJECT.md` decision row (D-57). A **decided value** would pay W1's value-decision half in plan 19-03's
commit, and only if it decides one. Criterion 3 is satisfied per causing change; the two commits together
are the chain, not one of them alone.

### Predicted token delta, and the measured line

Predicted for this plan, relative to the line measured at `start-commit`: `tests-expected`/`tests-executed`
+20 (ten ids across two presets), `profile-keys`/`fields`/`records` +1 each, `unknown-fields` +1, and every
other token 0. Measured after Task 2 (4 of the 10 ids landed so far):

```
steps=38/38 controls=405/405 claims=29 provisional=28 discrepancies=110 faults=238/238 profile-keys=155
goldens=29 gates=122 lines=372 prefix-files=11 records=155 fields=155 unknown-fields=48
tests-expected=5090 tests-executed=5086 stic-rules=67 tracked=2845
```

`sh` and `dash` ended in byte-identical summary lines (`cmp` clean, 0 `-- FAIL` lines under each).

### Two collateral records this commit had to extend

- `data/expected/faults/stic.ids`: `STIC_WINDOW_OFF_BY_ONE` and `STIC_MODE_SWITCH_OUTSIDE_WINDOW` gained
  `stic.disc.|stic.judge.disc-` in their allowed-prefix column. Both are genuine collateral: the first makes
  an event at exactly the close tick not apply, so the equivalence case's "read both words at the close
  tick" polarity cannot see `windows_active`; the second applies an effect the closed gate should have
  refused, so G1's blocked write latches. `faults=238/238` is unchanged - this adds no fault site, it
  classifies three existing ones' collateral.
- `tests/stic/stic_disc.c`: one em dash replaced with `-` (the `integrity-non-ascii` step).

### Byte-identity evidence for the three branches

The whole `stic` suite (103 ids) and the whole `state`, `video`, `machine` and `runner-process` suites pass
unchanged after the three branches landed at their default, and the gate's `goldens=29` did not move: the
value 1 of each member reproduces the previous bytes. The five ids the plan names for C84 and C94 were
checked individually from the suite's own result file
(`stic.alias.read-4021-switches-mode`, `stic.alias.read-4021-returns-read-view-value`, `stic.alias.read-0040`,
`stic.enable.seq.strobe-outside-window-dropped`, `stic.window.applies-when-display-off`), all `ok`.

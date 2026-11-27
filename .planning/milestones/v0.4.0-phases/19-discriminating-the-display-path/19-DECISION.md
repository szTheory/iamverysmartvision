# Phase 19 Decision Record: the sixteen-cell sweep and the exit it lands

Plan 19-03, VID-07 criterion 3. This record was written and committed **before** the sweep was ever run.
Both exits' file enumerations are below, measured at this plan's start commit
`15db4e6a888548e452ce2a3a076550aa9314223c`, and the measured matrix and the exit statement are
**appended** after the run as sections below them. No line above an appended section is edited by the
measurement: an enumeration written after the result is known cannot be told from a fit, which is the
whole reason the order is fixed here.

The record has four parts and no fifth: the four fields and their two settings, the sixteen cells with
each guest's predicted observable, Exit B's enumeration, and Exit A's.

## The four fields and their two settings

One line each: the profile row, the two settings, and what the engine does differently. "Shipped" is
the value in `data/profiles/intv-2609-ntsc-family-p1.json` at the start commit; "alternate" is what a
cell's stack copy holds, and it is never a profile value - the generator refuses anything but the
shipped one, so an alternate exists only as a stack copy of the generated row inside a test.

| bit | field | shipped | alternate | what the engine does differently |
|---|---|---|---|---|
| 0 | `stic.reg_window_cpu_cycles` (value `2000`, tag `tier4-community`, conflict C1) | the register window closes at 2000 CPU cycles (the REG_CLOSE event tick, 8000 master ticks at 4 ticks per CPU cycle) | 2900 CPU cycles, the figure its own cite names as the alternate | the REG_CLOSE event tick and the two recorded forms of the same number (`reg_window_cycles`, `reg_window_ticks`) move together, so an access stamped in the 2000-to-2900 gap is blocked under the shipped setting and served under the alternate |
| 1 | `stic.access_gating_when_off` (value `"apply"`, tag `unknown-default-chosen`, conflict C94) | the access windows apply whatever the display latch is (member `access_gating_when_off` = 1) | **a policy that opens the windows while the display is off** (member = 0) | the gate decision in `core/src/machine/machine_bus.c` stops consulting the window schedule for an access whose stamp is outside it while the display latch is clear: an access that was blocked is served, and a `$0020` READ row that had no effect can arm |
| 2 | the `$0020` **READ** row of `stic.access_effects` (effect `none`, tag `tier4-community`, conflict C82) | a read of `$0020` has no effect (rule STIC-R15) | the row's effect code set to `arm` | `ivs_stic_effect` returns the row's effect instead of "no effect", so a read of `$0020` inside the window strobes the display exactly as a write does |
| 3 | `stic.display_latch_cleared_each_frame` (value `1`, tag `unknown-default-chosen`, conflict C82) | the display latch is cleared at every frame start, so one strobe displays one frame (rule STIC-R13's sustain note) | the latch persists across frame starts until something else clears it | the FRAME_START event in `core/src/stic/stic_events.c` no longer clears the latch, so one write to `$0020` keeps the display on for every later frame |

**The four rival rows this sweep speaks to, named rather than left to be re-derived:** C1 carries the
window length, C94 carries the gating rule, C82 carries two of the four rivals (the `$0020` read effect
and the per-frame re-strobe) and C6 carries the display-enable strobe whose note already states the
sustain rule, so the set is C1, C6, C82 and C94 and C82 alone appears twice. C83 is the window
*mechanism* row rather than a rival rule, and C85 (power-on) and C91 (interrupt line) are neither. Both
exits append one dated observation to each of the four.

The gating field's alternate is stated in the generator's own words on purpose. `cmake/IvsSticTable.cmake`
validates the string through `st_policy` and refuses any value but `apply`, and the policy the refusal
message and the profile's `note` name is "the windows are open while the display is off". The cell
therefore models **that** policy as the member at 0, not a second policy string and not a second profile
file. Deciding that field the other way in Exit A would be the only case that also needs the generator
to accept a second policy string (`cmake/IvsSticTable.cmake` is in Exit A's enumeration for that case
only).

## The sixteen cells, predicted

A cell is a four-bit number: bit 0 `W` the register-window length, bit 1 `G` the gating rule, bit 2 `R`
the `$0020` READ row's effect, bit 3 `L` the latch sustain rule. A set bit means "the alternate setting".
Each cell is a stack copy of the generated row with exactly those members changed - never an edit of the
profile index and never a global, because the machine rebuilds its bus context from the profile index on
every unit and a global edit would leave every later cell comparing against its own predecessor.

The prediction is derived from plan 19-01's observable table (its "The observable table" section), not
from a run of the sweep. `G4`'s observable is a three-element vector, written here as the displayed word
of frames 0, 1 and 2.

| cell | W | G | R | L | G1 `blocked_writes` | G2 `blocked_writes` | G3 frame-0 `displayed` | G4 frames 0/1/2 |
|---|---|---|---|---|---|---|---|---|
| 0 | 0 | 0 | 0 | 0 | 1 | 1 | 0 | 1/0/0 |
| 1 | 1 | 0 | 0 | 0 | 0 | 1 | 0 | 1/0/0 |
| 2 | 0 | 1 | 0 | 0 | 1 | 0 | 0 | 1/0/0 |
| 3 | 1 | 1 | 0 | 0 | 0 | 0 | 0 | 1/0/0 |
| 4 | 0 | 0 | 1 | 0 | 1 | 1 | 1 | 1/0/0 |
| 5 | 1 | 0 | 1 | 0 | 0 | 1 | 1 | 1/0/0 |
| 6 | 0 | 1 | 1 | 0 | 1 | 0 | 1 | 1/0/0 |
| 7 | 1 | 1 | 1 | 0 | 0 | 0 | 1 | 1/0/0 |
| 8 | 0 | 0 | 0 | 1 | 1 | 1 | 0 | 1/1/1 |
| 9 | 1 | 0 | 0 | 1 | 0 | 1 | 0 | 1/1/1 |
| 10 | 0 | 1 | 0 | 1 | 1 | 0 | 0 | 1/1/1 |
| 11 | 1 | 1 | 0 | 1 | 0 | 0 | 0 | 1/1/1 |
| 12 | 0 | 0 | 1 | 1 | 1 | 1 | 1 | 1/1/1 |
| 13 | 1 | 0 | 1 | 1 | 0 | 1 | 1 | 1/1/1 |
| 14 | 0 | 1 | 1 | 1 | 1 | 0 | 1 | 1/1/1 |
| 15 | 1 | 1 | 1 | 1 | 0 | 0 | 1 | 1/1/1 |

Read down each guest's column: it takes its alternate value exactly in the cells whose own bit is set
(0xAAAA, 0xCCCC, 0xF0F0 and 0xFF00 respectively). That is the diagonal, and it is why the sweep is one
comparison rather than sixteen expectations.

**The off-diagonal entry the phase already knows about.** G1's *registered* witness is the blocked
counter, and the reason is a measured coupling rather than a preference: the criterion's own
single-strobe guest reads its observation as the frame-0 displayed word, and that word also moves when
the **gating** field flips (cells 2, 6, 10 and 14, where `W` is clear and `G` is set, give `displayed` 1
- the alternate value for a field that is not the guest's own). Plan 19-01 measured it and moved G1's
registered observable to the witness's counter with the display already on, because with the latch
already set the gating alternate's condition ("the latch is clear") is false and the window schedule
alone decides the write. The predicted table above therefore carries the registered observable, and the
entangled reading is committed as the stand-in `stic.judge.disc-diagonal-two-fields` precisely so that
the diagonal check is shown able to reject it. A prediction that turns out wrong is recorded as a
deviation against the measured matrix below; a prediction that is right is the measurement's own
control.

## Exit B enumeration

The exit that reads "default unchanged, observation appended, rivals named". One bullet per file, one
repo-relative path each, because the verify extracts these bullets and compares them with the final
commit's own file list in both directions.

- `.planning/phases/19-discriminating-the-display-path/19-DECISION.md`
- `.planning/phases/19-discriminating-the-display-path/19-03-SUMMARY.md`
- `data/expected/important-gates.ids`
- `.planning/REQUIREMENTS.md`
- `.planning/ROADMAP.md`
- `.planning/STATE.md`
- `.planning/PROJECT.md`
- `data/discrepancies.json`
- `docs/stic/reference-notes.md`

`data/expected/discrepancies.ids` is deliberately **absent** from this list, and the absence is the
W1 limb's evidence rather than an omission: no new C id is minted, so the inventory does not move, and
its non-movement is what proves no rival row was added or dropped. Every profile file,
`data/expected/ivsn-literals.ids`, `data/stic/rules.json`, `data/golden/` and
`data/expected/pkg-install-files.ids` are absent for the same class of reason: nothing is decided, so
no digest, literal, rule statement, golden or installed file moves.

## Exit A enumeration

The exit in which a default is decided. One bullet per file, same rule. Exit A's list is Exit B's list
plus the files a decided value moves, and the per-field table below says which of them a given decision
would actually touch - the enumeration names them all by path so the final commit can be checked against
it in both directions, and the table is the prediction the measurement confirms or corrects.

- `.planning/phases/19-discriminating-the-display-path/19-DECISION.md`
- `.planning/phases/19-discriminating-the-display-path/19-03-SUMMARY.md`
- `data/expected/important-gates.ids`
- `.planning/REQUIREMENTS.md`
- `.planning/ROADMAP.md`
- `.planning/STATE.md`
- `.planning/PROJECT.md`
- `data/discrepancies.json`
- `docs/stic/reference-notes.md`
- `tests/stic/stic_disc.c`
- `data/expected/tests/stic.ids`
- `data/profiles/intv-2609-ntsc-family-p1.json`
- `data/expected/ivsn-literals.ids`
- `data/stic/rules.json`
- `tests/stic/stic_access.c`
- `tests/stic/stic_events.c`
- `tests/video/video_slot.c`
- `cmake/IvsSticTable.cmake`
- `data/golden/`

The three test files are the homes of the existing ids whose assertions a decided value moves:
`tests/stic/stic_access.c` (the `stic.window.*`, `stic.alias.*` and `stic.enable.seq.*` families),
`tests/stic/stic_events.c` (the latch and frame-start families) and `tests/video/video_slot.c`
(`video.slot.displayed-flag-follows-latch`). `cmake/IvsSticTable.cmake` is in the list for the gating
field only, because deciding that field the other way is the one case that needs the generator to accept
a second policy string. `data/golden/` is named as a directory because a golden's *name* is not knowable
before the measurement: each golden whose `digest.instance` moves is retargeted in place and named
individually in the record at execution time, `goldens=29` never changes, and the measured set is added
to this record in a commit of its own if the list has to grow.

### Per-field delta, predicted

Measured at this plan's start commit: `data/expected/important-gates.ids` holds 372 data lines under
122 distinct gate names, `data/expected/ivsn-literals.ids` holds five digest-carrying values,
`data/golden/` holds 29 files, and `data/expected/discrepancies.ids` holds 110 ids.

| field decided | profile value | rule statement that becomes false | existing ids whose assertions move | other files |
|---|---|---|---|---|
| 0, the register-window length | `stic.reg_window_cpu_cycles` 2000 to 2900 | STIC-R18 and STIC-R22 (the window's length and its close) | **predicted: none** - the ids assert relations to `p->reg_window_ticks` and to the 2900 alternate, so they hold at either setting; the measured failing set decides | `data/expected/ivsn-literals.ids` (all five), the two pinning ids |
| 1, the gating rule | `stic.access_gating_when_off` `apply` to the open-while-off policy | none directly; STIC-R15's "a read has no effect" is not what this field states | **predicted:** `stic.window.applies-when-display-off`, `stic.blocked.counters` | `cmake/IvsSticTable.cmake` (a second policy string), `data/expected/ivsn-literals.ids` (all five), the two pinning ids |
| 2, the `$0020` READ row | the row's effect `none` to `arm` | STIC-R15 | **predicted:** `stic.alias.read-0040` and the `stic.peek.*` family | `data/expected/ivsn-literals.ids` (all five), the two pinning ids |
| 3, the latch sustain rule | `stic.display_latch_cleared_each_frame` 1 to 0 | STIC-R13's sustain note | **predicted:** `stic.events.latch-cleared-at-frame-start`, `stic.enable.seq.latch-cleared-at-frame-start`, `video.slot.displayed-flag-follows-latch` | `data/expected/ivsn-literals.ids` (all five), two cut literals move (`cut-snapshot-sha256.hp-latch`, `cut-snapshot-sha256.hp-mob`), the two pinning ids, possibly a golden |

The predicted sets above are **predictions**, marked as such: the measured failing set in the appended
section is what would be retargeted, and a prediction that differs from it is recorded, never quietly
corrected. Price, stated in advance because "the measurement decides the exit" has a cost consequence:
the window length is the cheapest decision (its ids assert relations rather than numbers) and the latch
the most expensive (two existing ids, one video id and two cut literals move); the gating field is the
only one that also needs a generator change. `claims=29`, `provisional=28`, `faults=238`, `steps=38`
and `prefix-files=11` are invariants in both exits.

## The rule this record exists to make unbreakable

In one sentence: the prediction is not the decision - the measurement taken after this commit decides
which enumeration is honoured, neither enumeration is edited afterwards, and the final commit's file
list is compared against the blob this commit wrote, not against a list completed later.

<!-- Everything below this line is appended by plan 19-03's second task, after the sweep ran. No line above
     it was edited: `git diff <start-commit> -- 19-DECISION.md` contains no removed line. -->

## Measured matrix: the sixteen cells as measured

Taken after the enumerations above were committed, by `stic.disc.sweep-diagonal` in the `stic` suite on the
dev preset (`IVS_TEST_OUT_DIR=<tmp> ctest --preset dev -R '^stic$' --output-on-failure`, result file
`<tmp>/stic.txt`, grammar `ok <n> <id>`). Each guest's decisive stamps were measured from the guest itself,
then every cell re-drove that access sequence through the hand-built bus context with `c.stic` set to the
cell's stack-copied row. `G4`'s observable is a three-element vector, the displayed word of frames 0, 1
and 2.

| cell | W | G | R | L | G1 `blocked_writes` | G2 `blocked_writes` | G3 frame-0 `displayed` | G4 frames 0/1/2 |
|---|---|---|---|---|---|---|---|---|
| 0 | 0 | 0 | 0 | 0 | 1 | 1 | 0 | 1/0/0 |
| 1 | 1 | 0 | 0 | 0 | 0 | 1 | 0 | 1/0/0 |
| 2 | 0 | 1 | 0 | 0 | 1 | 0 | 0 | 1/0/0 |
| 3 | 1 | 1 | 0 | 0 | 0 | 0 | 0 | 1/0/0 |
| 4 | 0 | 0 | 1 | 0 | 1 | 1 | 1 | 1/0/0 |
| 5 | 1 | 0 | 1 | 0 | 0 | 1 | 1 | 1/0/0 |
| 6 | 0 | 1 | 1 | 0 | 1 | 0 | 1 | 1/0/0 |
| 7 | 1 | 1 | 1 | 0 | 0 | 0 | 1 | 1/0/0 |
| 8 | 0 | 0 | 0 | 1 | 1 | 1 | 0 | 1/1/1 |
| 9 | 1 | 0 | 0 | 1 | 0 | 1 | 0 | 1/1/1 |
| 10 | 0 | 1 | 0 | 1 | 1 | 0 | 0 | 1/1/1 |
| 11 | 1 | 1 | 0 | 1 | 0 | 0 | 0 | 1/1/1 |
| 12 | 0 | 0 | 1 | 1 | 1 | 1 | 1 | 1/1/1 |
| 13 | 1 | 0 | 1 | 1 | 0 | 1 | 1 | 1/1/1 |
| 14 | 0 | 1 | 1 | 1 | 1 | 0 | 1 | 1/1/1 |
| 15 | 1 | 1 | 1 | 1 | 0 | 0 | 1 | 1/1/1 |

Every cell agrees with the prediction, and the four guests' alternate-value masks are exactly the cells
with their own bit set:

```
# sweep-diagonal: G1 field 0 mask 0xAAAA default 1 alternate 0
# sweep-diagonal: G2 field 1 mask 0xCCCC default 1 alternate 0
# sweep-diagonal: G3 field 2 mask 0xF0F0 default 0 alternate 1
# sweep-diagonal: G4 field 3 mask 0xFF00 default 4 alternate 7
```

So the diagonal holds as one sentence over sixteen cells, and no prediction had to be corrected: each
guest moves under its own field and under no other, at the default of the remaining three. The diagonal is
falsifiable rather than vacuous - `stic.judge.disc-diagonal-two-fields` drives the same comparison on G1's
entangled reading and the comparison rejects it:

```
# judge: the diagonal rejected the entangled guest at cell 2 (bits 0x2), observed 1
```

Cell 2 is the gating field alone, which is exactly the coupling plan 19-01 measured when it made the
blocked counter G1's registered observable.

### Why the measurement is Exit B and not Exit A

The sweep settles **separability**, which is criterion 2, and it settles nothing else. A diagonal cell
shows that a field's two settings are told apart by an authored guest's registered observable; it cannot
show which setting the hardware used, because the row under test is a stack copy of the engine's own
generated table and an engine agreeing with itself is not an oracle (`AGENTS.md`: repeated hashes and
shared-core agreement are not independent evidence; D-51: an observation is not a hardware probe). All
four guests are diagonal, so no field yields evidence that prefers one of its settings, and the four
rivals stay `open` with their `default_policy` sentences and their `unknown-default-chosen` tags intact.

That is the reading of the plan's Exit A trigger this record uses, stated so a reader can check it rather
than infer it: "exactly one field discriminates" is read as "exactly one field yields evidence that
prefers one of its settings", not as "exactly one field's guest moves", because a four-diagonal matrix is
the criterion-2 success and would otherwise satisfy "one field discriminates" zero times or four times
depending on how the sentence is parsed. Nothing here is decided by fit, and a diagonal is not a decision.

Exit B taken - the sixteen cells are diagonal and no cell prefers a setting, so the record reads default
unchanged, observation appended, rivals named, which is `Complete` for VID-07 on its own stated terms.

The evidence the line above names is `stic.disc.sweep-diagonal` (`ok 110`) and
`stic.judge.disc-diagonal-two-fields` (`ok 111`) in the `stic` suite's own result file, on both presets
through the gate. No profile value, parameter digest, snapshot literal, rule statement, golden or
installed file moves with this exit, and `data/expected/discrepancies.ids` does not move either: no new C
id is minted.

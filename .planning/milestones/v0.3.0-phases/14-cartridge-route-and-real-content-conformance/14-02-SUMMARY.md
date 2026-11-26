---
phase: 14-cartridge-route-and-real-content-conformance
plan: 02
subsystem: CPU interrupt-enable policy (profile data → parameter digest → snapshot literals) and one named blocker record
tags: [profile-field, retarget, digest, snapshot-literals, discrepancy-log, live-pages]
status: complete
dependency-graph:
  requires:
    - data/profiles/intv-2609-ntsc-family-p1.json (the registry scalar the flip moves)
    - cmake/IvsMemmapTable.cmake (the `intff` registry scalar at :302-303 and the body-digest writer)
    - core/src/machine/machine_snapshot.c (the four table body digests and the 134-byte preimage)
    - data/expected/ivsn-literals.ids (the frozen oracle keyed by name)
    - data/cp1610/vectors/interrupt.json (39 vectors, 34 of whose derivations name the setting)
  provides:
    - the profile field at 1 with the tag still `unknown-default-chosen`
    - ivs_cp1610_config_default at 1, agreeing with the one registered profile
    - seven vectors carrying an explicit `config` key (the both-settings pin)
    - re-pinned parameter digest and four snapshot literals
    - C82's strobe observation and a STATE.md Deferred row naming its gate
  affects:
    - plan 14-04 (the CPU-06 completion flip and the five GUARDRAILS §2.4 rows)
    - any later phase that takes the tier-4 STIC register-window and strobe default (Phase 16 candidate)
tech-stack:
  added: []
  patterns:
    - a profile default moves in one commit with its tag, cite, digest, every literal and every expectation the behaviour moves (GUARDRAILS V12)
    - a re-pin of a frozen oracle is an in-place edit of the value line with the recipe and both digests in the commit message, never an appended duplicate key
    - a derivation that names a deleted default is rewritten by reading its own record, and the count of clauses that still name the setting is held fixed so nothing is deleted to make a gate green
    - a control whose premise a flip deletes is re-expressed as the flip's own negative pin, so it fails when the flip is reverted
key-files:
  created: []
  modified:
    - data/profiles/intv-2609-ntsc-family-p1.json
    - core/src/cpu/cp1610.c
    - data/cp1610/vectors/interrupt.json
    - data/expected/ivsn-literals.ids
    - data/cp1610/vector-review.json
    - data/expected/diag-mutations-scn.ids
    - tests/run/run_stub2.c
    - tests/stic/stic_irq.c
    - data/discrepancies.json
    - docs/stic/timing-notes.md
    - docs/diagnostics.md
    - docs/cp1610/second-reader-policies.md
    - docs/cp1610/stepper-conventions.md
    - docs/state/hard-points.md
    - .planning/STATE.md
decisions:
  - ivs_cp1610_config_default moves to 1 with the profile, so the library's public default stops contradicting the only registered profile silently; the five vectors that rested on it state config 0 explicitly
  - run.stub2.jd-as-j is re-expressed as an equality of the J half and the JD half of the same frame, because at the profile's value 1 a J and a JD there are indistinguishable (measured)
  - runner-process.mutant.SCN-STUB-JD-AS-J becomes a negative pin (`pass 0`), because the flip deletes the re-entry its mutation produced
  - the probe source tests/run/stub2-probe.asm is not changed: a re-arm in its 5004 routine discriminates nothing and breaks run.stub2.handoff (measured)
  - the interrupt vector second-read marker keeps its derived hash, the reviewer and the adjudication; only its file hash follows the file
metrics:
  duration: ~145 min
  completed: 2026-11-26
  tasks: 2
  files: 15
actuals:
  tokens: 44091
  tasks: 2
  commits: 2
plan_head_before: 57571a42bf5a300482a945e273a151550c7de5a8
plan_head_after: a030e7f1cbf6aa11d4368a683d6ad6d5cd76ead5
requirements: [CPU-06]
coverage:
  - id: D1
    description: "the profile field cpu.interrupt_accept_clears_intff is 1 with the tag still unknown-default-chosen, its cite rewritten and the C41 observation appended without closing the row"
    requirement: CPU-06
    verification:
      - kind: automated_ui
        ref: "tools/check/steps/records.sh (profile step) + tools/check/steps/discrepancies.sh: unknown-fields=47, discrepancies=110 open=101"
        status: pass
      - kind: unit
        ref: "ctest --preset dev -R '^(state|run|stic|cpu-vectors)$'"
        status: pass
    human_judgment: false
  - id: D2
    description: "both settings pinned explicitly: seven vectors carry a config key, cpu.UNIT-INT.accept-keeps-intff (0) and -clears-intff-config (1) unchanged in data, and the library default agrees with the profile"
    requirement: CPU-06
    verification:
      - kind: unit
        ref: "ctest --preset dev -R '^cpu-vectors$' and --preset release (identical id set)"
        status: pass
      - kind: other
        ref: "grep -c -F '\"config\": {' data/cp1610/vectors/interrupt.json == 7"
        status: pass
    human_judgment: false
  - id: D3
    description: "the parameter digest and the four snapshot literals are re-pinned by a method that reproduces the pre-flip digest, and the state suite's second-reader limb rebuilds the preimage from the specification"
    requirement: CPU-06
    verification:
      - kind: unit
        ref: "ctest --preset dev -R '^state$': state.literal.params-digest, state.literal.header-structure, state.read.describe-ok, state.save.canonical-stream-literal, state.restore.roundtrip-fixed-point, state.judge.header-literal-corrupt, state.cut.fixed-point-and-canonical"
        status: pass
      - kind: other
        ref: "cmake -DROOT=<tree> -DOUT=<tmp> -P cmake/Ivs{Memmap,Psg,Stic,Ctrl}Table.cmake with the field at 0 reproduces 89ad6507..c0522"
        status: pass
    human_judgment: false
  - id: D4
    description: "the 33 derivations that called 0 the profile default are rewritten, the setting is still named in 34 derivations, and no config/init/expect datum of the corpus moves"
    requirement: CPU-06
    verification:
      - kind: other
        ref: "grep -c -e 'the profile default, C41' -e 'is 0 by default (POL-INT-03' data/cp1610/vectors/interrupt.json == 0 (was 33)"
        status: pass
      - kind: other
        ref: "git diff -U0 data/cp1610/vectors/interrupt.json: zero removed non-derivation lines"
        status: pass
    human_judgment: false
  - id: D5
    description: "five live pages each carry an inline pointer at the sentence stating the deleted default and a dated appended amendment"
    requirement: CPU-06
    verification:
      - kind: other
        ref: "sh tools/check/steps/text.sh: -- text pass docs=64 claim-refs=26"
        status: pass
    human_judgment: false
  - id: D6
    description: "the display-enable strobe blocker is named in C82 with its own gate (D10-VID-08) and one STATE.md Deferred bullet, unfixed"
    verification:
      - kind: other
        ref: "grep -x -F 'D10-VID-08 | C82 | ...' data/expected/d10-measurements.ids and the C82 note in data/discrepancies.json"
        status: pass
    human_judgment: false
---

# Phase 14 Plan 02: the interrupt-enable default, flipped with every value it moves

`cpu.interrupt_accept_clears_intff` is 1 on the owner-local observation and still tagged
`unknown-default-chosen`; the library default agrees with the profile, both settings are pinned by
explicit `config` keys, and the parameter digest plus four snapshot literals were re-pinned by a
method that first reproduced the pre-flip digest. The next blocker the same run exposed — the
display-enable strobe that never latches — is in C82 with its gate and in STATE.md, named and not
fixed.

## Performance

- **Duration:** ~145 min
- **Started:** 2026-11-26T02:00:50Z
- **Completed:** 2026-11-26
- **Tasks:** 2
- **Files modified:** 15

## Task Commits

1. **Task 1: the flip, in one commit, with every value it moves** — `f795f2a` (fix, 14 files)
2. **Task 2: the next blocker, named with its own row and gate** — `a030e7f` (docs, 2 files)

`plan_head_before` 57571a4 → `plan_head_after` a030e7f is **2** commits
(`git rev-list --count 57571a4..a030e7f`). The metadata commit that carries this file is the third,
by design, so a verifier reading the count at the final commit reads 3 and the two figures do not
contradict each other.

## The nine-id set was real, and it was not the whole set: the measured failure set

Measured on this tree, dev preset rebuilt with the profile field at 1 and the C default also at 1,
before any retarget (`ctest --preset dev -R '^(state|run|stic|cpu-vectors)$'`): **14** failing ids.

| Suite | id | This plan's disposition |
|---|---|---|
| state | `state.literal.params-digest` | re-pin `params-digest-sha256` |
| state | `state.literal.header-structure` | collateral; green after the re-pin |
| state | `state.read.describe-ok` | collateral; green after the re-pin |
| state | `state.save.canonical-stream-literal` | re-pin `stepped-snapshot-sha256` |
| state | `state.restore.roundtrip-fixed-point` | collateral; green after that re-pin |
| state | `state.judge.header-literal-corrupt` | collateral; green after the params re-pin |
| state | `state.cut.fixed-point-and-canonical` | re-pin three `cut-snapshot-sha256.*` |
| run | `run.stub2.jd-as-j` | **re-expressed** (see the defect below) |
| stic | `stic.irq.level-held-reentry-c41-0` | re-targeted to the profile's value |
| cpu-vectors | `cpu.EIS.defers-intr`, `cpu.EIS.then-intrm-after-next`, `cpu.GSWD.intff-unchanged`, `cpu.J.je-then-intrm-immediate`, `cpu.UNIT-INT.intrm-intff-set-accepted` | the five that move only because `ivs_cp1610_config_default` moved with the profile; each gained an explicit `config` 0 |

**A fifteenth id appears only in the release preset:** `runner-process.mutant.SCN-STUB-JD-AS-J`.
`mutant_case` in `tests/runner-process-tap.sh:708-713` judges every mutant **statically** outside a
Release build (`mut_static`, `ran=0`) and only runs it when the preset configuration is `Release`.
A dev-only measurement therefore cannot see it, which is why the research's list of nine and the
plan's predicted list of nine-plus-five both miss it. Measured in release: `exit 0`, `outcome pass`,
where the row pinned `guest-fail 10` with `frame 3 displayed == yes got no`. It is retargeted in the
same commit as the flip (see the second defect below). **Measured set: 15 ids, of which the plan
predicted 14.**

## The re-pin, and the two-path derivation that makes it auditable

`data/expected/ivsn-literals.ids` is a frozen oracle keyed by name. The re-pin is an in-place edit of
five value lines (a recorded deviation from GUARDRAILS §2.10's append-only rule, with the file's own
convention as the reason: appending a second `params-digest-sha256` line would create a duplicate
key, which `ivsn_lit_hex` reads as the first occurrence and which no check would catch).

**Path 1 — the digest arithmetic, run twice.** The four generators
(`cmake/Ivs{Memmap,Psg,Stic,Ctrl}Table.cmake`) were run into a temp tree, once with the field at 0 and
once at 1, and their `*_body_digest` arrays read back:

```
field 0:  mm b16e3672c67dbb224dac624f6b7eb42c12f3b683e5ea2dc8917008256c172771
          psg f19494a38983ed30457bf115015857ae43c4861152348f9f362fe4d914b2f88d
          stic ad2585eedfaaaf099955e6da4db881be12dab2213b2ccc9cbf7508dd6e3a58bf
          ctrl 9b00c1c49ddde04e6c35eba7899a7af3bc1e83b1fa1230b40ebbceeff8bd7848

preimage = "IVSP" + be16(1) + mm + psg + stic + ctrl          (134 bytes)
sha256   = 89ad65073a3e357acbdd687f6a29016457864a1666b953e9327b7559dc0c0522   <- the pinned value
field 1:  mm c6e522ee0e42bebdb7db37858355511cade9ad29db6af05617b88608212413ed
          (psg, stic and ctrl unchanged)
sha256   = efba85009a57e91ad21060742dadc34949b976829525bb5e18d6b0897af1425c
```

The field-at-0 run **reproduces the literal the tree already pins**, `…c0522`, exactly. That equality
is the check that the method is right; a method that cannot reproduce the old value is not trusted
for the new one.

**Path 2 — the core itself.** With `params-digest-sha256` set to `efba8500…f1425c`, the state suite
goes green, and `state.literal.params-digest` does not merely compare against the pin: it rebuilds the
same 134-byte preimage from the specification in `tests/state/state_literal.c:164-200` and holds the
core's accessors to it. So the pin, the core and the specification agree, which is two paths and not
one pin read through one reader.

**The four snapshot literals**, taken from the writer's own diagnostics in the dev preset
(`state.save.canonical-stream-literal` and `state.cut.fixed-point-and-canonical` print them on
mismatch):

| Literal | Before | Measured after |
|---|---|---|
| `params-digest-sha256` | `89ad6507…c0522` | `efba85009a57e91ad21060742dadc34949b976829525bb5e18d6b0897af1425c` |
| `stepped-snapshot-sha256` | `087a15d3…755b6b` | `95dd98cbb5221c99f189173c54d72bde17409ea98ce05616c01a6b1d5a363bcc` |
| `cut-snapshot-sha256.hp-frame-mid` | `9dc4ae7e…68567f` | `82fd4438b78390cea19056722bb30cbe1759aae9f7150c288088262b9818478b` |
| `cut-snapshot-sha256.hp-latch` | `63507cb4…6cee1d` | `1eb9c2cd15f6a7da80d5b88138c04cf1db323bd278a60a823a7d13ea58f8662b` |
| `cut-snapshot-sha256.hp-mob` | `27e98b02…9bc768` | `ffdf3d0e4b8793e953cb3911f77bf2314267e32c7409d92d05bedc250eb9bc55` |

**Both presets agree.** `ctest --preset release -R '^(state|run|stic|cpu-vectors|runner-process)$'`
is green with the same literals, so the snapshot bytes are identical across the two builds. That is
the property the file's own comment block asks for.

Nothing else moved: `git diff 57571a4..f795f2a -- 'data/golden/*' data/memmap/*.literal.txt
data/expected/profile-fields.ids data/expected/digest-v*.ids` is empty, and no report digest line
moved (`runner`, `machine` and `runner-process` stayed green).

## The C default, and why it moved with the profile

`ivs_cp1610_config_default` went from `0u` to `1u` in the same commit. Leaving it at 0 would keep a
**public library default that contradicts the only registered profile**, with nothing in the gate
comparing the two (RESEARCH Pitfall 4): a standalone CPU caller would get a value the machine never
uses, and its comment could only be rewritten to admit the disagreement. Its comment now says what it
is — the default of the library surface, which the machine overrides from
`ivs_mm_profiles[]`, and which agrees with the one registered profile — instead of claiming to
mirror the profile by copying its numbers.

The measured cost of that decision is exactly the five `cpu-vectors` ids above, and the fix is five
explicit `"config": {"interrupt_accept_clears_intff": 0}` keys. `cmake/IvsVectorTable.cmake` sets
`config_mask` bit 0 exactly when a vector carries a `config` key, so the override reaches the harness
with no C change; `grep -c -F '"config": {'` now reads **7** (two pre-existing plus five new).

Neither `cpu.UNIT-INT.accept-keeps-intff` (0) nor `cpu.UNIT-INT.accept-clears-intff-config` (1) gained
a third pin: `git diff -U0 data/cp1610/vectors/interrupt.json` shows **zero removed non-derivation
lines**, so their `config`, `init`, `expect`, `bus`, `bus_basis`, `cite`, `covers`, `policy` and `row`
data is byte-identical to its pre-plan form. Only their `derivation` prose moved, because it called 0
"the profile default", which the flip made false.

## The derivation sweep: 33 → 0, and the count of clauses held at 34

`derivation` is prose, but a derivation that calls 0 the profile default is a false statement *inside
the oracle*, which is worse than a stale page. Measured at the pre-plan HEAD:

```
grep -c -e 'the profile default, C41' -e 'is 0 by default (POL-INT-03' data/cp1610/vectors/interrupt.json
33        (32 of the form "is 0 by default (POL-INT-03, C41)", 1 of the form "the profile default, C41")
grep -c '^ *"derivation": .*interrupt_accept_clears_intff' …                                    -> 34
```

After the sweep the stale-phrase count is **0** and the derivation count that still names
`interrupt_accept_clears_intff` is still **34**, so every one of the 33 clauses was *rewritten* and
none was deleted to make the gate green. The split, by reading each record rather than substituting
the phrase:

- **6 of the 33** carry a `config` key after this plan's five additions
  (`cpu.UNIT-INT.accept-keeps-intff` plus the five). Each clause now states the value *this vector's*
  `config` gives, says it is stated rather than inherited, and says the profile carries 1.
- **2 of the 33** assert `intff` in `expect` and carry no `config`: `cpu.DIS.defers-intr` (0, because
  DIS cleared it at step 1) and `cpu.UNIT-RESET.intr-during-reset` (0, because the reset entry cleared
  it). Each clause now names what left INTFF at that value and states that the acceptance rule does
  not move it, so the expectation holds under either setting. **No override was added to either**:
  their expectation is not a claim about the acceptance rule.
- **25 of the 33** assert no `intff` and carry no `config`. Each clause now states the setting is 1
  (the profile's value and the library default), that acceptance therefore clears the enable, that
  the vector's expectation is the same under either setting, and names the two ids that pin the
  not-cleared case. `cpu.UNIT-INT.entry-figure` keeps the more explicit form the plan specifies,
  because its `init.intff` 0 and inherited `expect.intff` are what makes "the same under either
  setting" worth spelling out — **and it gained no override**: adding one would set `config_mask`
  bit 0 on a vector whose expectation cannot distinguish the two values, which is a claim of coverage
  it does not have. Its `config_mask` is still clear.

The 34th derivation (`cpu.UNIT-INT.accept-clears-intff-config`) was not one of the 33 — its clause
read "(the other value C41 asks to test)", which states no default — and it now reads "(the value the
profile carries, stated explicitly by this vector, C41; its pair is pinned by
`cpu.UNIT-INT.accept-keeps-intff`)".

No `config`, `init`, `expect`, `bus`, `bus_basis`, `cite`, `covers`, `policy` or `row` line changed
anywhere in the file: `git diff -U0` shows only added `config` blocks and changed `derivation` lines.

## What `run.stub2.jd-as-j` now holds, and the measured entry counts

`run.stub2.jd-as-j` fails at `f.entries > 1u` because the flip deletes its premise ("a plain J is a
trap", D-10). Its purpose was the *consequence* of C41 = 0, not a number, so it is re-expressed. It
now drives two heap copies of the same image — the vector's own second decle `0352` (JD) and `0350`
(a plain J) — through the same single-unit frame loop, and holds the two frames to be equal, while
checking that the two images differ in that one word so the equality cannot be the vacuous case of a
patch that was never applied.

**Measured entry counts** (`rs2_run_frame`'s `entries` field, "frame 0" of the stub-2 stand-in):

| Profile field | JD half | J half |
|---|---|---|
| 1 (this plan) | 1 entry, `vec_intr` 0, R6 `0341`, marker `03C5`, counter `0001`, R7 `500D` | **1 entry, identical in every one of those fields** |
| 0 (the stand-in) | 1 entry | **179 entries** |

So the case is a live control for this flip: returning `cpu.interrupt_accept_clears_intff` to 0 makes
it fail on `fj.entries == 1u && fj.entries == fjd.entries`, and re-applying the flip makes it pass.

## What `stic.irq.level-held-reentry-c41-0` now asserts

Two halves in one id, so it fails if either stops holding:

- **(a) the profile's value governs the machine.** `si_machine_entries` builds its guest through the
  public API, and the machine takes its CPU configuration from the profile registry, so the profile's
  1 must give **exactly one** entry in the frame with a service routine that does not start with DIS,
  and the guest's own counter must read 1. Measured with the field at 0 (the stand-in): `251 entries
  in frame 0 with the profile's value (one expected), the guest counted 251` — the old observation,
  and the closed-form storm. (A) therefore fails if the profile's value stops reaching the machine.
- **(b) the not-cleared setting stays exercised by a stated value.** `si_rig_entries(0u, 0u, …)` — the
  stepper rig with the field explicitly 0 — must still give the closed form from the entry and
  instruction cycle counts (15 CPU cycles per period: 9 for the entry, 6 for the first instruction),
  measured `251` at the first entry's tick, and at least 100 so a collapse to one entry cannot pass as
  a smaller closed form.

`stic.irq.c41-both-values` keeps its four-way sweep of both settings crossed with a DIS-first and a
not DIS-first routine, unchanged. The suite's header comment at `:4-14` was amended for both ids.

## The four live pages, and the pointer each one carries

Five pages in total (the timing note plus the four the plan names). Each keeps its historical sentence
— the sentence describes the 0 setting, which stays exercised — and gets an inline pointer at that
sentence plus a dated appended amendment citing D-51 and C41 (GUARDRAILS §2.10):

| Page | The sentence pointed at | What the appended amendment states |
|---|---|---|
| `docs/stic/timing-notes.md` | the "With C41 = 0 … 15 CPU cycles per entry (9 for the entry, 6 for the first instruction) give about 253 entries" bullet | the profile now carries 1, so the consequence is conditional on the field being 0; the 15-cycle period and the closed form are unchanged and still held by the re-targeted STIC id |
| `docs/diagnostics.md` | "Why the vector is `JD` and not a plain `J`: … a plain `J` lets the CPU accept the held interrupt request again" **and** the sentence describing `run.stub2.jd-as-j` as showing more than one entry | the plain-`J` premise is deleted; what the control now holds; the measured 179-vs-1 stand-in; why no re-expression can make a J take more entries than a JD at the profile's value; the probe is unchanged |
| `docs/cp1610/second-reader-policies.md` | "`[param] interrupt_accept_clears_intff` defaults to 0, …" | the default is 1 from the profile and the library default; a vector that wants the not-cleared behaviour states `"config": {"interrupt_accept_clears_intff": 0}`; seven vectors carry a `config` key; no `config`/`init`/`expect` datum moved |
| `docs/cp1610/stepper-conventions.md` | "`interrupt_accept_clears_intff` defaults to 0, from the profile field …" | the profile field carries 1, `ivs_cp1610_config_default` also gives 1, and the 0 case is stated per vector; `config_mask` bit 0 is set exactly when a vector carries `config` |
| `docs/state/hard-points.md` | "Acceptance of an interrupt does not clear the interrupt enable in this profile (a pinned default, C41)." | the profile carries 1 so acceptance clears it; still a pinned default and still not a hardware finding; what moved with it (the parameter digest and the four snapshot literals) and what did not (cut ticks, predicates, row classifications) |

`sh tools/check/steps/text.sh` reports `-- text pass docs=64 claim-refs=26` with no
`text-claim-untagged`, so no amendment line carries an untagged claim word from
`tools/check/steps/text.sh:5-10`.

## The record: C41 amended, C82 named

**C41** keeps `status: "open"`, keeps the `unknown-default-chosen: ` prefix the schema requires, and
edits the sentence that stated the value (`… = 0, with vectors for both values` → `… = 1, with vectors
for both values`) because the log must not contradict the profile. `evidence` gains the sentence that
names the ids now exercising the observation (`run.stub2.jd-as-j`, `stic.irq.level-held-reentry-c41-0`,
`stic.irq.c41-both-values`, the both-settings vector pair). `note` gains the amendment: the field is
now 1 on that observation, the tag stays `unknown-default-chosen`, no claim is minted, the row stays
open because the hardware probe is still the resolver, both settings are now pinned explicitly rather
than inherited, the library default also returns 1, and the retarget list.

**C82** gains the 2026-11-26 observation from the same owner-local run: with the interrupt-enable
default corrected the program ran further and the picture stayed black, because the display-enable
strobe written at `$0020` never latched; the register-window rule and the strobe rule are the next
tier-4 default and are **not** resolved here. No new discrepancy id was added (a second row for one
finding would register one observation twice and move `discrepancies=110`/`open=101` for no new
evidence), and no new gate line was needed because `data/expected/d10-measurements.ids:46` already
carries `D10-VID-08 | C82 | display-latch persistence and strobe semantics, with strobes at swept
offsets | #stic-strobe`. `.planning/STATE.md` gains exactly one `- Deferred: the STIC register-window`
bullet naming the rule, C82, the gate `D10-VID-08` and the candidate phase (Phase 16's owner-route
work, with the milestone close deciding). The existing `D13` bullet is untouched.

## Decisions

1. **`ivs_cp1610_config_default` moves to 1 with the profile**, and the five vectors that rested on it
   state `config` 0 explicitly. Leaving it at 0 would keep the library's public default contradicting
   the only registered profile, with nothing in the gate comparing the two.
2. **`run.stub2.jd-as-j` is re-expressed as an equality of two halves of one frame**, not as an
   inequality, because at the profile's value 1 the two are indistinguishable and the inequality is
   unachievable (measured, §"What `run.stub2.jd-as-j` now holds").
3. **`runner-process.mutant.SCN-STUB-JD-AS-J` becomes a negative pin (`pass 0`)** — the flip deletes
   the re-entry its mutation produced — and it still fails with the old observation when the field
   returns to 0, which is what keeps SCN-01's scenario coverage live.
4. **`tests/run/stub2-probe.asm` is not changed.** The plan allowed adding a re-arm to its `5004`
   routine; measured, that gives both halves 47 entries (no discrimination) and breaks
   `run.stub2.handoff` (`f.entries == 1u` fails), so the probe stays as it is.
5. **The interrupt vector second-read marker keeps its `derived_sha256`, reviewer and adjudication**;
   only `file_sha256` follows the file, because `vector-review-stale` compares the current file hash
   and the second read was of the blinded derived content, which no prose change touches.

## Deviations from Plan

### 1. `run.stub2.jd-as-j`: the specified re-expression is unachievable, and here is the measurement

The plan asks the control to "require the J run to take strictly more entries than the JD run,
`vec_intr` to be 1 after the vector (a J does not disable, a JD does), and R6 above `0341`". Both of
the first two limbs are impossible at the profile's value 1, and the fallback the plan offers (a
re-arm in the probe's `5004` routine) does not restore them.

- **`vec_intr` is 0 in both halves, measured.** `vec_intr` is the enable after the first unit
  following the entry, which is the vector instruction itself. Acceptance has already cleared the
  enable by then, and the entry leaves `prev_interruptible` 0, so no test can intervene before the
  vector: a `J` leaves the clear in place and a `JD` repeats it. Measured: `JD run: entries 1 vec_r7
  5004 vec_intr 0 …` and `J run: entries 1 vec_r7 5004 vec_intr 0 …`, identical in every field.
- **"Strictly more entries" is impossible for the same reason**, so long as the vector is reached
  through the interrupt entry — which is the only way the vector address is reached.
- **The plan's fallback was measured and rejected.** Adding `EIS` to the probe's `5004` routine (a
  reversible scratch edit, reverted) gives `JD run: entries 47 …` and `J run: entries 47 …` — no
  discrimination at all, because the re-arm comes after the vector's clear in both halves — and it
  breaks `run.stub2.handoff`: `check failed at line 281: f.entries == 1u`. The probe source is
  therefore unchanged.

What is implemented instead is the honest statement of what the flip does, and it is a live control:
the two halves' frames are equal, one word apart, and returning the field to 0 restores 179-vs-1.
`docs/diagnostics.md`'s amendment records the same measurement on the page.

### 2. A fifteenth id the plan's set missed: `runner-process.mutant.SCN-STUB-JD-AS-J`

The research measured with `ctest --preset dev`, and `mutant_case` (`tests/runner-process-tap.sh:708`)
judges a mutant statically outside a Release build. In release the mutant now reads `exit 0`, `outcome
pass` against a row that pinned `guest-fail 10` with `frame 3 displayed == yes got no`. It is a
retarget of exactly the same premise as `run.stub2.jd-as-j`, so it got the same treatment: the row is
now `- pass 0` with its comment rewritten to say the mutation no longer breaks the scenario and to say
what it gives back when the field returns to 0 (measured: `assert=30 line=62 frame 3 displayed == yes
got no`, `guest-fail 10`). Its test id, its registry line and every count are unchanged.

**Recorded, not silently kept:** `data/expected/important-gates.ids:337` still names this id as one of
SCN-01's `scn-mutants` must-fail mechanisms, and the row now pins the *absence* of a failure. The line
is left in place because the plan's invariants include `gates=114 lines=352`, and SCN-01 keeps eleven
other failing mutant rows (`SCN-INPUT-PORT`, `-PROGRESS-STORE`, `-LAMP-COMPLEMENT`, `-AMP-VALUE`,
`-SOUND-UNGATED`, `-REARM-SHORT`, `-REARM-LONG`, `-STROBE-LATE`, `-NO-EIS`, `-HLT-IDLE`, `-ISR-TIE`),
so the requirement's must-fail coverage does not depend on it. A later plan should either drop that
line or repoint it at a case that still fails.

### 3. A file the plan did not list: `data/cp1610/vector-review.json`

`vector-review-stale` (`tools/check/kinds/vector-review.cmake:96-104`) compares the *current* file
hash of every reviewed vector file against the row's `file_sha256`, so any edit to
`data/cp1610/vectors/interrupt.json` — including prose — must move that hash or the gate refuses with
`vector-review-stale: interrupt`. Measured on the first gate run after the flip. `derived_sha256`,
`reviewer`, `vectors`, `agree`, `disagreements` and `resolutions` are unchanged: the second read was
of the blinded derived content, and no `init`, `program` or policy datum of the corpus moved.

### 4. The plan's baseline figures were stale, as 14-01 recorded

`tests-expected` is **5,008**, not the plan's 4,990: plan 14-01 added nine ids × two presets. The
figures that *are* invariants all hold and were measured at the final commit: `steps=38/38`,
`faults=238/238`, `goldens=29`, `controls=402/402`, `discrepancies=110 pending-resolvers=20
open=101 d10-items=47`, `profile-keys=154 unknown-fields=47 vectors=412 state-rows=88 state-cuts=18
tests-skipped=4`, `claims=28 provisional=27 tiers=8`, `gates=114 lines=352`.

### 5. REQUIREMENTS.md is left byte-identical (CPU-06 stays `Pending`)

The generic executor contract says to mark the plan's `requirements:` complete. Plan 14-01 recorded
why that cannot be done yet, and the reason holds unchanged: `important-gates.sh` refuses a `Complete`
requirement with no registry line, and `data/expected/important-gates.ids` has no `CPU-06` line yet —
that row and the five GUARDRAILS §2.4 rows land in plan 14-04's task 1 together with the flip. So
CPU-06's checkbox and its traceability row still read `Pending`, and the flip is 14-04's.

### Auto-fixed issues

**1. [Rule 3 - Blocking] `si_rig_entries` had to be declared before its new caller**
- **Found during:** Task 1, after the STIC re-target
- **Issue:** `stic.irq.level-held-reentry-c41-0` now states the field explicitly through the rig, whose
  definition sits further down the file; `-Werror` refused the implicit declaration.
- **Fix:** one forward declaration above `si_reentry_c41_0`, with the reason in a comment.
- **Files modified:** `tests/stic/stic_irq.c`
- **Committed in:** `f795f2a`

**2. [Rule 3 - Blocking] Non-ASCII em dashes in a C comment**
- **Found during:** the first gate run (`integrity-non-ascii: tests/run/run_stub2.c`)
- **Issue:** the rewritten comment block used U+2014.
- **Fix:** replaced with `--`; `grep -P '[^\x00-\x7F]'` over every touched file now finds nothing.
- **Files modified:** `tests/run/run_stub2.c`
- **Committed in:** `f795f2a`

---

**Total deviations:** 2 auto-fixed (2 blocking), 5 recorded plan defects.
**Impact on plan:** The flip itself landed exactly as specified. The two large deltas are both
*measurement* corrections, not scope changes: one plan-specified assertion is impossible at the
profile's value 1 (so the control pins the flip's real consequence instead), and the plan's failure
set was one id short because a Release-only mutant path is invisible to a dev-only measurement. Both
were measured rather than argued, and both left the id set, the counts and the requirement's coverage
intact.

## Issues Encountered

- The five extra `cpu-vectors` failures appear only once `ivs_cp1610_config_default` moves with the
  profile, which is the decision this plan had to make explicitly. Measured both ways: profile-only
  leaves them green; profile plus default breaks all five.
- The four snapshot literals were read from the writer's own diagnostics rather than from the research
  table, and all four matched what the research had measured — which is a useful cross-check, but the
  SUMMARY records the measured values, not the quoted ones.

## Verification at the final commit

```
sh   tools/gate.sh   exit 0
dash tools/gate.sh   exit 0
last lines byte-identical:
gate: PASS steps=38/38 controls=402/402 … discrepancies=110 pending-resolvers=20 open=101
d10-items=47 faults=238/238 … profile-keys=154 goldens=29 … gates=114 lines=352 …
tests-expected=5008 tests-executed=5004 tests-skipped=4 tests-missing=0 …
claims=28 provisional=27 … state-rows=88 state-cuts=18 … tags=10 tiers=8
```

- `ctest --preset dev -R '^(state|run|stic|cpu-vectors)$'` — green, 4/4
- `ctest --preset release -R '^(state|run|stic|cpu-vectors|runner-process)$'` — green, 5/5
- `grep -c -F '"tag": "unknown-default-chosen"' data/profiles/intv-2609-ntsc-family-p1.json` → 47
- `git ls-files | grep -E '[.]ivsn$'` → no match; no image byte and no operator digest anywhere
- `<automated>` of task 1 and task 2 both re-run on the final tree: exit 0

## Token delta actually measured

| Figure | Before this plan | After | Arithmetic |
|---|---|---|---|
| `tests-expected` / `tests-executed` | 5,008 / 5,004 | **5,008 / 5,004** | no id added or removed; `tests-skipped=4`, `tests-missing=0` |
| `vectors` | 412 | 412 | derivation prose only; no vector added |
| `state-rows` / `state-cuts` | 88 / 18 | 88 / 18 | unchanged |
| `discrepancies` / `open` | 110 / 101 | 110 / 101 | no id added, no status moved |
| `claims` / `provisional` / `tiers` | 28 / 27 / 8 | 28 / 27 / 8 | no claim minted |
| `gates` / `lines` | 114 / 352 | 114 / 352 | no registry row added, removed or repointed |
| `controls` / `faults` / `goldens` / `steps` | 402 / 238 / 29 / 38 | 402 / 238 / 29 / 38 | unchanged invariants |
| `profile-keys` / `unknown-fields` | 154 / 47 | 154 / 47 | the tag did not change |
| `tracked` / `files` | 2,743 | 2,743 | this SUMMARY.md makes the metadata commit read 2,744 |

**Actuals.** `tokens: 44091` is chars/4 over the realized diff of the two task commits
(`git diff 57571a4..a030e7f | wc -c` = 176,364), on the same scale as the plan's `estimate.tokens:
58000`; 202 lines added, 82 removed across 15 files. `commits: 2` is measured from the plan ledger.

## Known Stubs

None. Every value written is recomputed or measured, no placeholder was introduced, and no case was
skipped.

**Recorded for the verifier, not a stub:** `data/expected/important-gates.ids:337` names a case that
now pins the absence of a failure (deviation 2 above), with eleven other failing mutants still
carrying SCN-01. It is a registry accuracy item for a later plan, not a gap in coverage.

## Self-Check

- `data/profiles/intv-2609-ntsc-family-p1.json` field at 1, tag `unknown-default-chosen` — FOUND
- `core/src/cpu/cp1610.c` `ivs_cp1610_config_default` at `1u` — FOUND
- `data/cp1610/vectors/interrupt.json` seven `config` keys, zero stale derivations, 34 naming — FOUND
- `data/expected/ivsn-literals.ids` five re-pinned values, zero pre-flip digests — FOUND
- `data/discrepancies.json` C41 open with the prefix and 1; C82 note with the strobe observation — FOUND
- `.planning/STATE.md` exactly one `- Deferred: the STIC register-window` bullet — FOUND
- commit `f795f2a` — FOUND (ancestor of HEAD)
- commit `a030e7f` — FOUND (ancestor of HEAD)
- `sh tools/gate.sh` and `dash tools/gate.sh` — exit 0, last lines byte-identical

**Self-Check: PASSED**

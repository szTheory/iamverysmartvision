# Phase 19: Discriminating the Display Path — Research

**Researched:** 2026-11-27
**Domain:** STIC display-path parameters, profile-record mechanics, in-process guest authoring, the `W1` retarget chain
**Confidence:** HIGH on mechanism (every claim below was read out of the tree at `8786e20`), MEDIUM on the
per-field retarget *sizes* (measured by reading the tests that assert each rule, not by running the flips)

## Summary

Three findings change what the plan can be, and one of them changes the phase's shape.

**1. A parameter row cannot be selected per run.** `cmake/IvsSticTable.cmake` generates
`ivs_stic_params.inc` from every `data/profiles/*.json` at build time; there is one profile file; and the
machine rebuilds its bus context from the *profile index* on every unit
(`core/src/machine/machine_run.c:175`). So the 16 cells are **not** 16 ids, 16 runs or 16 scenarios, and they
are **not** 16 profile files (that would multiply the four generated tables and every `profile`-step count).
They are 16 **stack copies of `ivs_stic_params`**, exactly the mechanism `tests/stic/stic_profile.c:131` already
uses for `stic.profile.window-close`. This settles the discretion item CONTEXT left open ("How the 16 cells are
enumerated in the suite").

**2. Two of the four fields are not flippable today, and a third only looks flippable.** `stic.reg_window_cpu_cycles`
is real data (the REG_CLOSE event tick, `stic_params.h:58-59`). But:

- `stic.access_gating_when_off` **produces no parameter member at all**. `st_policy` validates the string and emits
  nothing (`cmake/IvsSticTable.cmake:194-201`, call at `:375`); the gate decision is a memsys flag
  (`core/src/machine/machine_bus.c:84`). Flipping "the field" is currently a no-op.
- The `$0020` READ row of `stic.access_effects` is a **row the engine refuses to reach**: `ivs_stic_effect`
  returns `IVS_STIC_EFFECT_NONE` for any READ whose address is not in `stic.mode_select_addrs`
  (`core/src/stic/stic_regs.c:32-34`), and `$0020` is not a member. Flipping that row is currently a no-op.
- The display-latch rule is **hardcoded**, not a field: `s->display_latch = 0u` at FRAME_START
  (`core/src/stic/stic_events.c:55`). It is not a row anywhere, which is why criterion 2 says it must become one
  first.

A sweep of the four fields as the tree stands today would be diagonal for G1 and flat for G2, G3 and G4. So the
phase's real work is: make the three unreachable settings *reachable as parameter-driven branches whose default
reproduces today's behaviour byte for byte*, then sweep. That is three small core branches plus one generator
change, not a rewrite — but it is core behaviour, and the plan must budget it explicitly.

**3. G2's observable is not a report key.** The report's key table is closed (`docs/runner.md` `## Report`) and
carries no blocked counter; `stic.blocked_writes` exists only as machine state
(`core/include/ivs/ivs_run.h:102`, exposed by `ivs_machine_stic_state`). G1, G3 and G4's `frame N displayed` *is*
a report line (`runner/report.c:604`) but is equally readable in-process from the slot header bit
(`core/include/ivs/ivs_machine.h:128`, written from `s->windows_active` at `core/src/stic/stic_frame.c:394`).
So all four observables are read at the machine-state seam, in the `stic` suite — which is where ROADMAP's cost
line already puts the ids.

**Primary recommendation:** put all four guests, their liveness guards and the 16-cell sweep in
`tests/stic/` as `stic.disc.*` cases over stack-copied parameter rows; add one 0/1 policy member per
unreachable field in `ivs_stic_params` (default = today's behaviour) with the branch that consults it; author
the guests with the suite's existing `sc_w_*` decle builders and *measure* each guest's access stamp with
`ivs-inspect trace` rather than deriving it; and write both exits' commit shapes before the sweep runs, with the
`W1` file list measured by the recipe in `f795f2a`'s commit message.

---

## Architectural Responsibility Map

| Capability | Primary Tier | Secondary Tier | Rationale |
|---|---|---|---|
| The four parameter rows (a cell of the sweep) | Core (`ivs_stic_params`, generated) | — | The STIC is a pure function of the row; a cell is a row, so the tier that owns a row is the tier that owns the cell |
| Guest program (a WRITE/READ at a chosen cycle) | Test tier (`tests/stic`, decle builders) | Core (CPU executes it) | The project authors guests as test data, never as committed binaries (`tests/run/stub2-probe.asm` precedent) |
| Access stamp measurement | Test tier via Phase 18's instrument (`ivs-inspect trace`) | — | The stamp is `unit_start + cycle x ticks_per_cpu_cycle + access_strobe_ticks`; only the trace prints it |
| `displayed` observable | Core state (`windows_active`) | Report (`frame N displayed` assert line) | The report line *is* the slot header bit; both read the same word |
| `stic.blocked_writes` observable | Core state (`ivs_stic_report`) | — | No report key exists and the key table is closed |
| The 16-cell matrix | Test tier | — | Only a test can build a row the machine cannot be handed |
| Decision record / retarget chain | Records (`data/expected/*`, `data/profiles/*`, `data/discrepancies.json`, prose) | Gate steps (read them) | A profile change is data; the gate is the reader that makes it a chain |

---

<user_constraints>
## User Constraints (from CONTEXT.md)

### Locked Decisions

**The four guests**
- **One guest per rival rule, authored as project-owned content**: a source assembled by the project's own `ivs_asm` plus the scenario that drives it, under the fixtures the repository already uses. No third-party content, no operator file, nothing rights-bearing — the guests are ours and are committed as sources, not as binaries.
- Each guest exposes **exactly one machine-state observable**, named by the criterion and read from the runner's report (never a pixel, never a hash of a frame):
  - **G1** a WRITE to `$0020` at a cycle inside the 2000-to-2900 gap → frame 0 `displayed`.
  - **G2** a WRITE outside both window candidates with the display latch clear → `stic.blocked_writes`.
  - **G3** a READ of `$0020` with no write → frame 0 `displayed`.
  - **G4** one WRITE never re-strobed → `displayed` for frames 0, 1 and 2.
- Each guest ships a **liveness guard with a committed stand-in that fails it**, in the same commit: a guest whose observable cannot move is a dead guest, and the stand-in is what proves the guard is not vacuous.

**The 16-cell sweep, and the latch row**
- The sweep is a **matrix of parameter rows**: four fields at two settings each, `2^4 = 16` cells, each cell a profile parameter row built explicitly (never a global edit), driven through the existing `stic` suite ids.
- **The latch rule becomes a row in `data/expected/profile-fields.ids` before it is swept.** That is a profile change and it drags the `W1` chain with it — the field row, its tag and cite, the parameter digest, and **every** snapshot literal in `data/expected/ivsn-literals.ids` — so the row and that chain land in **one commit**, before any sweep result is recorded. The field stays `unknown-default-chosen`.
- **Diagonality is asserted, not eyeballed**: a committed `mutant` row over the diagonal check is shown able to fail, so "each guest moves under its own field and no other" is falsifiable. A guest that moves under two fields is **re-derived**, not accepted with a note.

**The two exits**
- **Exit A** (a default is decided) requires: two ids, one per setting, each building the alternate parameter row explicitly; and **one commit** carrying the whole `W1` chain — the profile field with its tag and cite, `data/expected/profile-fields.ids`, the parameter digest in `data/expected/ivsn-literals.ids` and the id that holds it, **every** snapshot literal in that file, any moved golden retargeted in place, the moved `stic` inventory ids, the C-row `evidence` line in `data/discrepancies.json` and `data/expected/discrepancies.ids`, and the prose. The commit's file list must equal the enumeration written into the phase record **before** the commit; a mismatch is a **stop**, not a re-bless.
- **Exit B** (nothing discriminates) reads "default unchanged, observation appended, rivals named" and is `Complete` for VID-07 **on its own terms** — a result, not a failure.
- In **both** exits the four rival rows in `data/discrepancies.json` carry the appended observation, the field stays `unknown-default-chosen`, no claim cites the observation, `claims=29`, `provisional=28` and `goldens=29` are unchanged, and no record holds a digest, size, path or name of an operator file.
- **Which exit is taken is decided by the measurement, not chosen in advance.** The plan writes both exits' commit shape before the sweep runs, so neither can be improvised afterwards.

**The owner-local sweep (`19-02`)**
- It is a **throwaway probe and nobody's exit criterion**: it lives under `refs/` only, is bounded by a wall-clock limit and a run count, its output is refused outside `refs/` on the `tools/flatwrap` precedent, and it is `skip-ok` (exit 13, `missing-fixture`) in both test inventories.
- Its absence, or a run that ends inconclusive, leaves criteria 1 to 4 standing. **No criterion of this phase depends on a human act.**

### Claude's Discretion
- The guest source shape (address layout, cycle offsets within the stated gaps), the scenario files' names, and the module split of the sweep driver.
- How the 16 cells are enumerated in the suite (one id per cell versus one id that loops the matrix) — bounded by the criterion's "diagonal" assertion and the cost line.

### Deferred Ideas (OUT OF SCOPE)
- A fifth rival rule (for example a strobe that latches the *next* frame) — not one of the four the sources leave open; named here so a later reader does not add a guest to the sweep without a new source.
- Any pixel or frame-hash comparison — refused by the criterion and by D-51; the observables are machine state.
- Turning the owner-local sweep into a CI case — refused: its input is operator content (D11), so it stays `skip-ok` and bounded.
</user_constraints>

<phase_requirements>
## Phase Requirements

| ID | Description | Research Support |
|----|-------------|------------------|
| VID-07 | The four rival display-path rules are separated by four authored guests whose observations are diagonal across the 16-cell sweep, each with a liveness guard a committed stand-in can fail; then either the decided default is pinned by one test per setting with every dependent expectation moving in one commit, or the record reads "default unchanged, observation appended, rivals named"; the field stays `unknown-default-chosen` in either case | §1 (row mechanics + the measured `W1` file list), §2 (guest authoring and the three observables), §3 (which ids move per field), §4 (the C-row append), §6 (what fits in 1.64 cpu-s) |

---

## 1. How a parameter row works, and what the `W1` chain actually is

### 1a. The exact row format for a new field

`data/expected/profile-fields.ids` must equal the members of `fields` in
`data/profiles/intv-2609-ntsc-family-p1.json` — enforced by `members fields` + `inventory` in
`tools/check/schema/profile.txt`, and the count is printed as `profile-keys` by
`tools/check/steps/glossary.sh:99` and as `fields`/`unknown-fields` by `tools/check/kinds/profile.cmake:11,25-27`.

The row's key set is fixed and **`note` is optional, every other key is required**
`[VERIFIED: tools/check/schema/profile.txt]`:

```
keys value tag cite conflict resolving_evidence revisit_trigger decision note
when tag=unknown-default-chosen nonblank resolving_evidence
when tag=unknown-default-chosen nonblank revisit_trigger
enumfrom tag data/tiers.json source_authority_tags id
match conflict ^C[0-9]+$
enumfrom conflict data/discrepancies.json discrepancies id
```

So a new latch field must carry: a `value`, `tag: "unknown-default-chosen"`, a `cite` beginning `none-found:`
(the house style — see `stic.access_gating_when_off`'s cite), a `conflict` naming a **registered** C id, a
non-blank `resolving_evidence` and `revisit_trigger`, `decision: null`, and optionally a `note`. A `tier5-emulator-hint`
tag is refused (`profile-tier5-only`) and a `measured` tag is refused while D10 is unperformed
(`profile-measured-without-d10`) — both `[VERIFIED: tools/check/kinds/profile.cmake:15-22]`.

The three existing STIC rows the sweep names, verbatim from the inventory
`[VERIFIED: data/expected/profile-fields.ids:114,119,132]`:

```
stic.access_effects
stic.reg_window_cpu_cycles
stic.access_gating_when_off
```

and their profile records `[VERIFIED: data/profiles/intv-2609-ntsc-family-p1.json fields]`:

| field | `value` | `tag` | `conflict` |
|---|---|---|---|
| `stic.reg_window_cpu_cycles` | `2000` | `tier4-community` | `C1` |
| `stic.access_gating_when_off` | `"apply"` | `unknown-default-chosen` | `C94` |
| `stic.access_effects` | 4 records: `$0020 W arm`, `$0020 R none`, `$0021 W mode_fgbg`, `$0021 R mode_cs` | `tier4-community` | `C82` |

**Which C row the latch field cites.** C82's `topic` is "Display-enable strobe at `$0020`: read effect, deadline,
late strobe and **per-frame re-strobe**", and its `default_policy` reads "…the latch is cleared at every frame
start…" `[VERIFIED: data/discrepancies.json C82]`. So the latch field cites **C82**, and so does the `$0020` read
row. C1 carries the window length, C94 carries gating-when-off `[VERIFIED: data/discrepancies.json C1,C94]`.

**Naming constraints on the new field** (both are gate steps, not style):

- `tools/check/steps/glossary.sh:88-96` refuses a field path whose last `_`-segment contains `_window_`,
  `_offset_`, `_hold_`, `_duration_`, `_delay_`, `_period_`, `_latency_`, `_interval_` or `_timeout_` unless the
  segment ends in `_ticks` or `_cpu_cycles` (`glossary-unit-suffix`, emitted at
  `tools/check/steps/glossary.sh:90`). A name like
  `stic.display_latch_clear_each_frame` or `stic.display_latch_persists` is safe; a name containing
  `..._period` is not.
- `tools/check/steps/video.sh` requires every `fields` entry of a rule in `data/stic/rules.json` that is not
  `engine:<name>` to be a profile row **or** a line of `data/stic/fields-pending.ids`, and refuses a pending line
  whose field now exists (`video-pending-stale`). `data/stic/fields-pending.ids` currently has **zero data lines**
  (comments only) `[VERIFIED: data/stic/fields-pending.ids]`, so adding the field costs no pending-line deletion
  *unless* the plan also adds the field to a rule's `fields` list — in which case the row must exist in the same
  commit, which is what W1 already requires.

**Rule-oracle coupling the plan must decide.** `data/stic/rules.json` STIC-R13 states "A write to `$0020` inside the
register window sets the display latch **for the current frame**; the value is ignored" and its note says "The
sustain rule (the latch is cleared at every frame start) is C6 and C82"; STIC-R15 states flatly "A read of `$0020`
has no effect" `[VERIFIED: data/stic/rules.json STIC-R13,STIC-R15]`. If Exit A decides either of those two rules
the *other way*, the rule statement becomes false and must be amended (append-only note + inline pointer,
GUARDRAILS §2.12) in the same commit. `stic-rules=67` must not change: amending a statement is not adding a rule.

### 1b. Which literals move, and the commands that recompute them

`data/expected/ivsn-literals.ids` holds exactly five values that a profile-table change moves
`[VERIFIED: data/expected/ivsn-literals.ids]`:

```
params-digest-sha256 efba85009a57e91ad21060742dadc34949b976829525bb5e18d6b0897af1425c
stepped-snapshot-sha256 95dd98cbb5221c99f189173c54d72bde17409ea98ce05616c01a6b1d5a363bcc
cut-snapshot-sha256.hp-frame-mid 82fd4438b78390cea19056722bb30cbe1759aae9f7150c288088262b9818478b
cut-snapshot-sha256.hp-latch 1eb9c2cd15f6a7da80d5b88138c04cf1db323bd278a60a823a7d13ea58f8662b
cut-snapshot-sha256.hp-mob ffdf3d0e4b8793e953cb3911f77bf2314267e32c7409d92d05bedc250eb9bc55
```

The **whole** file moves, not the digest alone, because the container header carries the parameter digest at
offset 120 (`tests/state/state_literal.c:316-320`: `"the parameter digest differs from params-digest-sha256"`,
compared at `snap + 120u`), so every snapshot byte string changes. `header-bytes`, `section-bytes.*`,
`sections-total-bytes`, `total-bytes`, `section-offset.*`, `trailer-offset`, `header-prefix-hex` and
`schema-digest-sha256` do **not** move (no STATE row is added, no section resized).

**The recompute recipe is already written down in a commit message** — `f795f2a`, the Phase 14 profile flip, which
moved exactly these five values `[VERIFIED: git log -1 --format=%B f795f2a]`:

> Recipe: the four table body digests are read from `cmake/Ivs{Memmap,Psg,Stic,Ctrl}Table.cmake` run into a temp
> tree; the preimage is the ASCII `IVSP`, a big-endian u16 1 and the four digests in the order memory map, PSG,
> STIC, controller (134 bytes); sha256 of it. The same method with the field at 0 reproduces 89ad6507..c0522
> exactly, which is the check that the method is right … The four snapshot literals are the writer's own
> diagnostics in the dev preset.

The plan should reproduce that method **including the "reproduce the old value first" control**, because it is the
only way to prove the recipe rather than the number. Concretely:

```bash
# 1. the four body digests, from the generators run into a scratch tree
cmake -DROOT="$PWD" -DOUT=/tmp/g/memmap.inc -P cmake/IvsMemmapTable.cmake
cmake -DROOT="$PWD" -DOUT=/tmp/g/psg.inc    -P cmake/IvsPsgTable.cmake
cmake -DROOT="$PWD" -DOUT=/tmp/g/stic.inc   -P cmake/IvsSticTable.cmake
cmake -DROOT="$PWD" -DOUT=/tmp/g/ctrl.inc   -P cmake/IvsCtrlTable.cmake
# expected: four files, each ending in a `ivs_*_body_digest[...][32]` block whose bytes are the body digest
# 2. the 134-byte preimage and its SHA-256
python3 - <<'EOF'   # reads the four digests out of the .inc files, builds b"IVSP" + b"\x00\x01" + d0..d3
EOF
# expected: params-digest-sha256; with the tree unchanged it MUST equal efba8500..f1425c
# 3. the four snapshot literals: the state suite's own diagnostics
ctest --preset dev -R state --output-on-failure
# expected on a stale literal: the case names the mismatch (`the parameter digest differs from
# params-digest-sha256`, `the canonical stream differs from stepped-snapshot-sha256`, ...) and reports the
# observed value; copy that value in place
```

`state.literal.params-digest` (`tests/state/state_literal.c:463`) is the id that holds the digest and it *also*
rebuilds the preimage byte by byte from the specification (`tests/state/state_literal.c:160-200`), so the retarget
is held by two readers, not one.

### 1c. Can a parameter row be selected per run? **No.**

Three independent facts close this:

1. The table is **generated at build time** from `data/profiles/*.json` and included as `const` data
   (`cmake/IvsSticTable.cmake` header: "reads every `data/profiles/*.json` in sorted order … writes
   `ivs_stic_params.inc` to OUT (build tree only, never committed). The only includer is
   `core/src/stic/stic_params.c`").
2. There is **one** profile file: `data/profiles/intv-2609-ntsc-family-p1.json`.
3. The machine resolves its parameters **from the profile index on every unit**, so no caller can hand it a row
   `[VERIFIED: core/src/machine/machine_run.c:175]`:
   ```
   bus_ctx.stic = ivs_stic_params_get(im->profile); /* never NULL: the handle check covers the index (WR-04) */
   ```

The runner's only profile input is the scenario's `profile <id>` line (`docs/runner.md` `## Scenario grammar`),
and `ivs_machine_create`'s option struct takes `const char *profile_id` and nothing else
(`core/include/ivs/ivs_machine.h:85-89`). There is no parameter override anywhere on the public or internal path.

**Consequence for the plan.** The 16 cells are **16 stack copies of `ivs_stic_params`** inside the `stic` suite,
each perturbed in exactly one member (or member group) and driven through a hand-built `ivs_machine_bus_ctx` — the
mechanism `tests/stic/stic_profile.c` already uses and documents `[VERIFIED: tests/stic/stic_profile.c:1-8,131-137]`:

```c
  q = *p;
  q.event_tick[idx] += q.ticks_per_cpu_cycle;
  q.reg_window_ticks += q.ticks_per_cpu_cycle;
```

A guest *run* cannot be swept: `bus_ctx.stic` is rebuilt from the profile index inside
`ivs_machine_step_unit_trace_internal`, so the guest always sees the shipped row. The faithful structure is
therefore **guest supplies the stamp and the observable; the cell re-drives that same stamp under a perturbed
row** — which is what `stic.profile.window-close` already does with a bisection over stamps.

### 1d. The `W1` chain, measured

Files the chain touches, with what moves and why. `[VERIFIED: each path exists; the token counts are read from the
`18-VERIFICATION.md` gate line at `9a51d68` and must be re-measured at the plan's start commit]`

| # | File | What changes | Gate token that moves |
|---|---|---|---|
| 1 | `data/profiles/intv-2609-ntsc-family-p1.json` | +1 field object (value/tag/cite/conflict/resolving_evidence/revisit_trigger/decision/note) | `profile-keys`, `fields`, `unknown-fields` |
| 2 | `data/expected/profile-fields.ids` | +1 line, in the file's existing order (the file is the member order) | same three |
| 3 | `core/src/stic/stic_params.h` | +1 member for each new policy the sweep needs | none directly |
| 4 | `cmake/IvsSticTable.cmake` | emit the new member; accept the alternate policy string where a policy must be sweepable | none directly |
| 5 | `core/src/stic/stic_events.c` (latch), `core/src/machine/machine_bus.c` (gating), `core/src/stic/stic_regs.c` (read effect) | the parameter-driven branch; **default reproduces today's bytes** | none, if the default is behaviour-identical |
| 6 | `data/expected/ivsn-literals.ids` | all five values above | `state` step green |
| 7 | `data/expected/tests/stic.ids` | +N ids (4 guests + guards + sweep + pins) | `tests-expected` = ids x presets |
| 8 | `data/expected/tests/runner-process.ids` + `data/expected/runner-scenarios.ids` | only if `19-02` is a scenario (2 files, `skip-ok`) | `tests-expected`, `tests-skipped` |
| 9 | `data/discrepancies.json` | the `note` of C1 / C82 / C94 (+C6 or C83, see §4) | `discrepancies=110` unchanged |
| 10 | `data/expected/discrepancies.ids` | **unchanged** (no new C id) | `discrepancies=110` |
| 11 | `data/expected/important-gates.ids` | +1 `mutant` row for the diagonal check | `gates`, `lines` |
| 12 | `data/stic/rules.json` | STIC-R13 / STIC-R15 statement amendment if that rule was decided | `stic-rules=67` unchanged |
| 13 | `.planning/PROJECT.md` | one decision row (W1) | none |
| 14 | `docs/stic/reference-notes.md` | an appended note (W1) | `docs`/`doc-files` |
| 15 | any golden whose `digest.instance` moves | retargeted in place | `goldens=29` unchanged |
| 16 | `.planning/REQUIREMENTS.md`, `ROADMAP.md`, `STATE.md` | the VID-07 flip | none |

**Token delta to predict before the run** (arithmetic, not quotation — re-measure at the start commit):

- `profile-keys` 154 → 155, `fields` 154 → 155, `unknown-fields` 47 → 48, `records` 154 → 155
- `tests-expected` 5082 → 5082 + (new ids × 2 presets)
- `gates` 122 → 123 and `lines` 372 → 373 if the mutant row takes a **new** gate name; `lines` only if it reuses one
- `steps=38/38`, `faults=238`, `goldens=29`, `claims=29`, `provisional=28`, `prefix-files=11`, `controls=405/405`
  **unchanged** unless a control directory is added (CONTEXT allows at most +1, for the diagonal mutant)
- `discrepancies=110`, `handoffs=8`, `rights=11` unchanged

---

## 2. How a guest is authored and driven today

### 2a. The smallest authored guest that writes to `$0020` at a chosen cycle

Two mechanisms already exist and the plan should use the first.

**(i) The `stic` suite's decle builders** — `tests/stic/stic_progs.c:9-79`. The minimal "write `$0020` after a
delay" guest is three calls:

```c
  n = sc_w_mvii(w, n, 0u, 0x0000u);   /* MVII #0, R0          8 cycles  (02B8, 0000) */
  n = sc_w_delay(w, n, 149u);         /* MVII #149,R1; DECR R1; BNZE   = 15 x 149 + 6 cycles */
  n = sc_w_mvo(w, n, 0u, 0x0020u);    /* MVO R0, $0020        0240, 0020 */
  n = sc_w_loop(w, n);                /* B self               0220, 0001 */
```

`sc_w_delay`'s loop is `DECR R1` (6 cycles) + `BNZE` (9 taken, 7 not) over `MVII #count, R1` (8), i.e.
**`15 x count + 6` cycles** `[VERIFIED: tests/stic/stic_progs.c:65-72]`. With the profile's
`ticks_per_cpu_cycle = 4` and `frame_ticks = 59736` (14934 CPU cycles per frame)
`[VERIFIED: data/profiles/intv-2609-ntsc-family-p1.json ticks_per_cpu_cycle, cpu_cycles_per_scanline,
scanlines_per_frame]`, the 2000-to-2900 CPU-cycle gap is **ticks 8000 to 11600**, so `count = 149` lands near
cycle 2241 ≈ tick 8964 — inside the gap. **Do not use that arithmetic as the expected value**: measure it (§2b).

The same builder pattern already produces G4's guest: `sa_strobe(0, 1, 0x0020, ticks, want_latch, want_blocked)`
`[VERIFIED: tests/stic/stic_access.c:371-400]`, and G3's is the same with `sc_w_mvi` in place of `sc_w_mvo`.

**(ii) An `ivs_asm` source assembled at build time** — the `tests/run/stub2-probe.asm` precedent
`[VERIFIED: tests/CMakeLists.txt:148-157]`: the source is committed, `ivs_asm` writes a listing and
`ivs_pack --ivsc` a container into the build tree, and "Neither the listing nor the container is committed."
This satisfies CONTEXT's "committed as sources, not as binaries" if the plan prefers a textual guest. It is the
more expensive of the two (a `add_custom_command` chain, an image include, and a *third* home for the same words if
the sweep needs them in-process), and it buys nothing the decle builders do not already give the `stic` suite.

**Recommendation:** author the four guests with the decle builders, with the decle derivation written into the
comment exactly as `tests/stic/stic_cases.h:116-141` does for the VBL program (that comment block *is* the
project's guest-source convention in this suite). If the plan wants a textual source as well, it must say why —
the decle builders are the mechanism the ids will actually use.

### 2b. Relating the machine's cycle count to the guest's instruction timing

The stamp is computed in one place `[VERIFIED: core/src/machine/machine_bus.c:78-80]`:

```
uint64_t at = c->unit_start_tick + (uint64_t)a->offset_cpu_cycles * (uint64_t)c->ticks_per_cpu_cycle +
              (uint64_t)c->stic->access_strobe_ticks;
```

and the frozen page states the same formula for the trace's own `tick=` field: "`tick` is
`unit_start + cycle * ticks_per_cpu_cycle`, with the multiplier read from the profile's time base and never written
as a literal" `[CITED: docs/inspection.md ## The trace file]`. The trace prints, per access
`[VERIFIED: tools/inspect/inspect_trace.c:617]`:

```
tick=<n> kind=<kind> addr=<HHHH> value=<HHHH> wait=<n> cycle=<n> unit_start=<n> unit_end=<n>
```

with `kind` ∈ `fetch read write adar intak iab dw strobe-tci strobe-pcit`. So the guest's write cycle is
**measured**, not derived:

```bash
build/dev/tools/inspect/ivs-inspect trace <guest-scenario>.ivss --content-dir build/dev/images --out /tmp/g.ivst
grep 'kind=write addr=0020' /tmp/g.ivst
# expected: exactly one line; its tick= is the number the case pins, and (tick - unit_start) / 4 is the cycle
```

This is the phase's load-bearing use of Phase 18: the trace line that differs between the rivals is *this* line's
`tick` and the frame in which the latch is set, which is CONTEXT §"Rival rule pair separated" item 2. The trace
file itself is operator-local and never committed (GUARDRAILS §2.13); what lands in the tree is the **number**,
quoted in the case comment with the command that produced it.

### 2c. Are `displayed` and `stic.blocked_writes` report keys today?

| observable | report key? | where it lives |
|---|---|---|
| `frame <n> displayed` | **yes, as an assert clause** — `runner/report.c:604` prints `frame %llu displayed %s %s`, reached only through `assert frame <n> displayed ==\|!= yes\|no` (`docs/runner.md` grammar table; `runner/ivs_runner.h:90`), which additionally requires a `capture video <first> <count>` directive in the same scenario (`assert-without-capture` is refused at parse time) | the slot header bit `IVS_FRAME_F_DISPLAYED` (`core/include/ivs/ivs_machine.h:128`), written from `displayed = s->windows_active != 0u ? 1u : 0u` (`core/src/stic/stic_frame.c:394,417`) |
| `stic.blocked_writes` | **no** — the report's key table is closed and carries no STIC counter (`docs/runner.md ## Report`) | `ivs_stic_report.blocked_writes` (`core/include/ivs/ivs_run.h:102`), returned by `ivs_machine_stic_state` (`ivs_run.h:114`); internally `im->stic.blocked_writes` |

So the two are not symmetric, and the plan must not pretend they are. G1/G3/G4 can be judged from a report line
*or* from the slot header; G2 only from machine state. Since criterion 2 forces the sweep into the process, the
cleanest consistent choice is to read **all four** at the machine-state seam and state that the report line is a
*rendering* of the same word (`frame N displayed` prints the slot header bit, which is `windows_active`, which is
`display_latch != 0` at GMEM_CLOSE — `core/src/stic/stic_events.c:66`). The in-process read of the displayed flag
already exists and is proven: `vs_displayed_flag_follows_latch` reads `r.buf[24..28]` and asserts
`IVS_FRAME_F_DISPLAYED` on frame 0 and 0 on frame 1 `[VERIFIED: tests/video/video_slot.c:120-127]`.

**Do not add a report key for `blocked_writes`.** It would bump `ivsr 3`, change every golden's report body and
touch the closed key table for one phase's convenience.

### 2d. `skip-ok` and exit 13

Two files carry the marker, and both must (this is the "both inventories" of criterion 5):

- `data/expected/runner-scenarios.ids` — the row's 5th column: `run`, `skip-ok` or `release-only`
  `[VERIFIED: data/expected/runner-scenarios.ids header]`.
- `data/expected/tests/runner-process.ids` — the id line's trailing token:
  `runner-process.scenario.needs-user-exec skip-ok` `[VERIFIED: data/expected/tests/runner-process.ids:118]`.

The suite accepts exactly three markers (`run | skip-ok | release-only`) and, for `skip-ok`, judges the case like
any other and turns a *passing* judgement into a visible skip `[VERIFIED: tests/runner-process-tap.sh:657,685]`;
the gate then prints `-- SKIPPED <preset>/<suite>/<id>: <reason>` per id per preset
(`data/expected/tests/runner-process.ids:7`). Exit 13 is the `missing-fixture` outcome — produced by a scenario
naming `content ... user` when the operator supplies no image (`docs/runner.md` `## Outcomes and exit codes`), and
already exercised end to end by `needs-user-exec` (`tests/runner-process-tap.sh:1356-1363` asserts the report's
`outcome` is `missing-fixture`).

---

## 3. What the `stic` suite already asserts, and which ids move per field

The inventory `data/expected/tests/stic.ids` currently holds the four families the criterion names
`[VERIFIED: data/expected/tests/stic.ids:21-34,46-47]`:

```
stic.window.applies-when-display-off
stic.window.gmem-edges
stic.blocked.reg-read
stic.blocked.reg-write
stic.blocked.gram-read
stic.blocked.gram-write
stic.blocked.grom-read
stic.blocked.counters
stic.enable.seq.strobe-in-window
stic.enable.seq.strobe-outside-window-dropped
stic.enable.seq.second-strobe
stic.enable.seq.latch-cleared-at-frame-start
stic.enable.seq.reset-clears
stic.enable.seq.strobe-via-write-alias
stic.profile.window-close
stic.profile.window-open
```

plus `stic.events.latch-cleared-at-frame-start` (`stic.ids:9`), the three `stic.judge.*` self-controls
(`stic.ids:48-50`) and the three register ids `stic.reg.strobe-stores-nothing`, `stic.reg.width`, `stic.reg.masks`.
C82's `evidence` line names the six `stic.enable.seq.*` ids, which is exactly the six above
`[VERIFIED: data/discrepancies.json C82 evidence]`.

**Which ids move under each field** (read from the assertions; the plan must confirm by running the flip):

| field | flips | ids whose assertion breaks |
|---|---|---|
| `stic.reg_window_cpu_cycles` 2000 → 2900 | REG_CLOSE tick 8000 → 11600 | none by construction: `stic.profile.window-close` / `window-open` bisect against the params and assert *relations*; `stic.enable.seq.strobe-outside-window-dropped` uses `SC_DELAY_PAST_REG = 200` = `15 x 200 + 6 = 3006` cycles plus the leading `MVII` and the `MVO`'s own offset, so its stamp is ≈ 3023 cycles — still past 2900, but the **margin is only ≈ 123 cycles**, so a third candidate window above ~3000 would move this id and it must be re-checked if the alternate changes; `stic.window.applies-when-display-off` uses the same delay. **The risk is a golden or digest**, not these ids: an access in 2000..2900 that was dropped now lands, so any scenario with such an access moves. |
| `stic.access_gating_when_off` `apply` → open-when-off | gate decision | `stic.window.applies-when-display-off` (asserts `blocked_reads == 1` with the latch clear, `stic_access.c:209-219`) and every `stic.blocked.*` that relies on the latch being clear. `stic.blocked.counters` uses "past both windows" with no strobe, so it moves too. |
| the `$0020` READ row `none` → `arm` | read arms the latch | `stic.mode.read-outside-window-no-effect` (a read of `$0021`, unaffected if the relaxation is scoped to the register file), `stic.alias.read-0040` (`mode == 0` asserted; the alias is at `$0040`, so it must keep no effect), `stic.peek.mode-unchanged-0021` and `stic.peek.cpu-read-switches` (PEEK must stay inert — `machine_inspect.c` never applies an effect). C84's `default_policy` ("reads of `$0040-$007F` have no effect") is the constraint the relaxation must not break. |
| the latch rule clear-each-frame → persist | FRAME_START | `stic.events.latch-cleared-at-frame-start` (`stic_events.c:220-246`), `stic.enable.seq.latch-cleared-at-frame-start` (`stic_access.c:420-440`), `video` suite `vs_displayed_flag_follows_latch` (`video_slot.c:120-127`), and `cut-snapshot-sha256.hp-latch` / `hp-frame-mid` (the HP-LATCH cut is at tick 75456, past frame 0's end, with the latch set). Also every `demo-lamp-tone-*` `assert frame 0 displayed == no` **if** the persist policy lets a strobe survive into frame 0 — it does not, so those hold; their `digest.instance` may still move. |

**The engine facts behind rows 2 and 3** (these are the phase's real work):

- gating: `uint32_t open = c->m->mem.gate_open[gate];` and `if (open == 0u) { ivs_stic_count_blocked(...); }`
  `[VERIFIED: core/src/machine/machine_bus.c:84-87]`. There is no parameter in the decision.
- read effect: `if (access == IVS_STIC_ACCESS_READ && ivs_stic_is_mode_select_addr(p, addr) == 0u) { return
  IVS_STIC_EFFECT_NONE; }` `[VERIFIED: core/src/stic/stic_regs.c:32-34]`. `$0020` is not in
  `stic.mode_select_addrs` (which is `["$0021","$4021","$8021","$C021"]`
  `[VERIFIED: data/profiles/intv-2609-ntsc-family-p1.json stic.mode_select_addrs]`), so the row is unreachable.
- latch: `s->display_latch = 0u;` at `IVS_STIC_EV_FRAME_START` `[VERIFIED: core/src/stic/stic_events.c:55]`.
- the policy string produces nothing: `st_policy` only validates (`st_fail` when the value differs)
  `[VERIFIED: cmake/IvsSticTable.cmake:194-201]`, and `gating_off` is referenced at exactly one line, `:375`.

**A precedent worth copying for the "dormant member" shape:** `stic.alias_read_sensitive` *is* emitted
(`.alias_read_sensitive = ${alias_sens}u`, `cmake/IvsSticTable.cmake:752`) and *no* engine code reads it —
C84's evidence says so in as many words: "the value 0 of the field is only range-checked in the machine suite, not
exercised end to end" `[VERIFIED: data/discrepancies.json C84 evidence]`. A new policy member with a
behaviour-identical default is the same kind of object, and the machine suite's range check
(`tests/machine/machine_stic_params.c:223`) is where it gets its first registration.

---

## 4. The discrepancy rows

**Which four rows.** The criterion says "the four rival rows", and the four rival rules map onto **C1** (window
length 2000 vs 2900), **C94** (gating when the display is off), **C82** (read-vs-write strobe *and* per-frame
re-strobe) and — for the fourth — **C6** ("Display-enable strobe"), whose note already says "the strobe sustain
rule is the latch cleared at every frame start (C82)" `[VERIFIED: data/discrepancies.json C6,C1,C82,C94]`.
C83 (the window *mechanism*, frame-position schedule vs bus copy mode) is the mechanism row, not a rival rule;
C85 (power-on) and C91 (interrupt line) are neither. **Recommendation:** append to C1, C6, C82 and C94, and name
that set explicitly in the phase record so a reader does not have to re-derive it. If the plan prefers C1/C82/C83/C94,
say so and justify it — either set is defensible, but silence is not, because C82 alone carries two of the four
rivals.

**The append shape.** `note` is a plain string; append one dated sentence at the end, in the style C82's existing
observation uses ("Observation, 2026-11-26 (owner-local, the D11 route …). … The observation mints no claim.")
`[VERIFIED: data/discrepancies.json C82 note]`. Optionally extend `evidence` with the new id names — C82's evidence
already enumerates ids, so a new id that exercises the rule belongs there. Do **not** touch `status`
(all four are `open`), `default_policy`'s leading `unknown-default-chosen:` (the schema refuses its absence for
`open`, `tools/check/kinds/discrepancy.cmake:55`), `resolver` (must keep a `test:`/`measurement:`/`pending:`
prefix) or `id`.

**What the step enforces** `[VERIFIED: tools/check/schema/discrepancy.txt; tools/check/steps/discrepancies.sh]`:
the 8 required keys, `match id ^C[0-9]+$`, `match status ^(open|partial|resolved-primary|withdrawn)$`,
`match resolver ^(test|measurement|pending):.+`, the `unknown-default-chosen: ` prefix for `open`/`partial`, a
non-blank `evidence` for `resolved-primary`, and `inventory data/expected/discrepancies.ids` equality. **No new C
id** is minted, so `data/expected/discrepancies.ids` is unchanged and `discrepancies=110` does not move.

---

## 5. The owner-local sweep's precedent

`19-02` is a **helper script, not a gate step**, and the tree already has the exact shape twice:

- `tools/owner-run.sh` — "NOT a gate step: the gate never passes it an image and CI never runs it"; refuses when
  `CI` or `GITHUB_ACTIONS` is set; refuses any path "inside the work tree outside `refs/`"; prints
  `SKIPPED owner-run: no operator images (D11)` and exits **77** when the images are absent
  `[VERIFIED: tools/owner-run.sh:3,25,45-46]`. Note 77 is the ctest skip code and is the *script's* skip; the
  criterion's exit **13** is the *scenario's* `missing-fixture`, so `19-02` needs both limbs: the script's 77 for a
  bare invocation and a `skip-ok` scenario whose outcome is 13 for the suite case.
- `tools/flatwrap` — the output-path refusal `flatwrap-output-tree` ("the output %s is inside the work tree
  outside refs/"), whose rule is "1 when the physical parent directory of out lies inside the work tree and is not
  under `refs/`" `[VERIFIED: tools/flatwrap/flatwrap.c:158-196,349]`. `refs/` is gitignored (`/refs/`,
  `.gitignore:2`) and "a tracked path under `refs/` fails the `files` step"
  `[CITED: docs/owner-images.md:5,15,46-47]`.

So `19-02`'s containment is: a shell helper under `tools/`, `CI`/`GITHUB_ACTIONS` refused, every output path
resolved physically and refused unless under `refs/` (the `flatwrap` rule verbatim), a wall-clock bound
(`--max-ticks`/`timeout`), a run-count bound, `SKIPPED` + 77 when the operator image is absent, and a `skip-ok`
scenario row in **both** inventories reporting `missing-fixture`/13. Nothing it writes is committed, cited, or
read by a claim.

`data/expected/handoffs.ids` needs **no row**: D10-VID-08 is Phase 20's row (W6), and this project refuses a
handoff row for something a tool can check (18-VERIFICATION, "Handed off" section). The four-token row format is
`<id> <handed-off|not-performed> evidence: none` `[VERIFIED: data/expected/handoffs.ids header]`.

---

## 6. The cost, and what fits inside 1.64

Frozen unit prices `[CITED: .planning/research/v0.4.0-scoping-2-gate-cost.md §2.3]`: in-process id **0.065**,
process-launch id **0.10**, control **0.31**, in-step selftest **0.05**, registry row **0.005** — hosted cpu-s.

The frozen line for Phase 19 is `4 x 0.10 + 4 x 0.31 = 1.64`, i.e. **four process-launch ids plus four control
directories**. Two things in the tree disagree with that assumption, and the plan should record the deviation:

1. `controls` is **405/405** and CONTEXT says it stays unchanged unless the diagonal mutant needs a directory
   (then +1). Four new control directories would take it to 409/409, contradicting CONTEXT.
2. The ids land in the `stic` suite (ROADMAP's own cost sentence: "four discrimination ids and two pinning ids per
   decided field **inside the existing `stic` suite**"), which is **in-process**, not process-launch.

Arithmetic for the shape this research recommends:

| item | unit | count | cpu-s |
|---|---|---|---|
| 4 guest ids (`stic.disc.g1..g4`), each with its in-case stand-in limb | 0.065 | 4 | 0.26 |
| 1 sweep id (`stic.disc.sweep-diagonal`, loops the 16 cells) *or* 16 cell ids | 0.065 | 1 or 16 | 0.065 … 1.04 |
| 2 pinning ids per decided field (Exit A only) | 0.065 | 0…8 | 0 … 0.52 |
| 1 `mutant` registry row (the diagonal check) | 0.005 | 1 | 0.005 |
| 1 control directory, **only if** the diagonal mutant needs one | 0.31 | 0…1 | 0 … 0.31 |
| **total** | | | **0.33 … 2.14** |

So 1.64 fits comfortably *provided* the sweep is **one id that loops the matrix** (0.33 total with no control
directory, 0.64 with one). Sixteen cell ids plus eight pinning ids would reach 1.89 and would also be sixteen
places to keep the diagonal assertion honest. **Recommendation:** one `stic.disc.sweep-diagonal` id that builds all
16 rows and asserts the diagonal in one place, with a per-cell diagnostic line naming the cell that broke — that
is also the only shape where "a guest that moves under two fields" is detectable in one comparison rather than
inferred from 16 independent outcomes.

**The checkpoint.** Recomputed by arithmetic from `.planning/ci-runs.tsv` at `8786e20`, last five rows
`45839646456, 45841732895, 45842795129, 45843408874, 45850438592, 45850984597` — the last five are
`584, 580, 572, 530, 368` cpu-s (median **572**), `51, 51, 49, 49, 35` build-s (median **49**) and
`171, 165, 164, 154, 115` gate-s (median **164**) `[VERIFIED: .planning/ci-runs.tsv tail]`. Against the frozen
361/34/118 baseline, `discharged-within-allowance` is not reachable (572 > 361 + 1.64) and `lever-required` is not
reached (the priced addition is 1.64 of 21 and the phase's content lands at or under its priced count), so the
verdict is **`reprice` — a runner event, no lever invented**, the same verdict Phases 13–17 recorded. The plan
must recompute this at its own start commit (F2) and not quote this paragraph.

---

## Standard Stack

No new dependency. The phase adds no library, no tool and no build target: it uses the existing `stic` suite
executable, the existing `ivs_test` harness, the existing `ivs_asm`/`ivs_pack` chain if a textual guest is wanted,
and Phase 18's `ivs-inspect` for measurement only.

## Package Legitimacy Audit

Not applicable — the phase installs no external package on any ecosystem. No `npm`/`PyPI`/`crates` name is added to
any file. Nothing to gate.

---

## Don't Hand-Roll

| Problem | Don't Build | Use Instead | Why |
|---|---|---|---|
| Building a perturbed parameter row | A new profile JSON per cell, or a global default edit | `ivs_stic_params q = *p;` + one member change | The table is generated and `const`; a stack copy is the proven mechanism (`stic_profile.c:131`) and a profile file would multiply four generated tables |
| Measuring a guest's access cycle | Arithmetic over instruction cycles in a comment | `ivs-inspect trace` `tick=` on `kind=write addr=0020` | The stamp includes the access offset within the instruction and the strobe ticks; only the trace prints the composed number |
| Reading the displayed flag | A new report key, or a pixel probe | `ivs_machine_stic_state` / the slot header bit | The report line already renders that bit; a pixel is refused by D-51 |
| Reading blocked writes | A report key | `ivs_stic_report.blocked_writes` | The key table is closed and bumping `ivsr` moves every golden |
| The diagonal assertion | 16 hand-written expectations | One loop over the matrix with a per-cell diagnostic | The diagonal is a *relation between cells*; written as 16 literals it is 16 fits, not one discrimination |
| The owner-local probe | A gate step | A `refs/`-only helper on the `owner-run.sh` pattern | Its input is operator content (D11); a gate step would re-open `steps=38` |

---

## Common Pitfalls

### Pitfall 1: sweeping a field that produces no parameter
**What goes wrong:** the sweep flips `stic.access_gating_when_off` and every cell reads the same value; the
"diagonal" passes trivially and the phase claims discrimination it never had.
**Why:** `st_policy` validates and emits nothing (`IvsSticTable.cmake:194-201`); the gate is a memsys flag.
**Avoid:** for each of the four fields, first assert that the flipped row *changes the observation in the
single-field cell* before the 16-cell matrix is written. A field whose single-field cell does not move is not
sweepable, and the plan must add the member+branch or record the field as undiscriminable.
**Warning signs:** a cell whose two settings give byte-identical observations; a `mutant` row that cannot fail.

### Pitfall 2: assuming a guest run can be swept
**What goes wrong:** the plan writes "run the guest under each row" and the implementation discovers
`bus_ctx.stic` is rebuilt from the profile index per unit (`machine_run.c:175`), so no injection point exists.
**Avoid:** sweep by re-driving the guest's *measured stamp* through a hand-built `ivs_machine_bus_ctx` (the
`sa_bus`/`sp_blocked_at` mechanism), and say in the plan that the guest supplies the stamp, not the sweep.
**Warning signs:** a plan task that says "run the guest with the alternate profile".

### Pitfall 3: the mutant row cannot land until VID-07 is Complete
**What goes wrong:** the diagonal `mutant` row names VID-07, `important-gates.sh` **derives** its must-be-covered
set from the requirement records and refuses a row naming a `Pending` requirement — so the row and the requirement
flip cannot move separately. Phase 18 hit exactly this and split the commit (18-VERIFICATION O4).
**Avoid:** put the registry row and the VID-07 flip in the **same** commit, and say so in the enumeration. If the
phase takes Exit B, the requirement still flips to Complete (VID-07's own terms say so), so the same rule applies.
**Warning signs:** a plan that adds the row in an earlier wave.

### Pitfall 4: the parameter digest is retargeted but a snapshot literal is forgotten
**What goes wrong:** four of the five literals are updated, `ctest -R state` fails on the fifth, and the temptation
is to re-bless the file wholesale.
**Avoid:** change all five in one edit and prove the recipe by reproducing the *old* value first (the `f795f2a`
control). `state.literal.params-digest` rebuilds the preimage independently, so a wrong preimage fails twice.
**Warning signs:** a `state` step failure whose token is `the canonical stream differs from stepped-snapshot-sha256`
after the digest was already updated.

### Pitfall 5: a new field name trips the unit-suffix tripwire
**What goes wrong:** a name containing `_delay_`, `_period_`, `_interval_`, `_window_` etc. must end in `_ticks`
or `_cpu_cycles`, or `glossary-unit-suffix` fails the `glossary` step.
**Avoid:** pick a name with no time-unit segment (`stic.display_latch_persists`,
`stic.display_latch_clear_each_frame`).

### Pitfall 6: adding a rule `fields` entry without the field
**What goes wrong:** if the plan extends STIC-R13's `fields` list, `video.sh` requires the field to be a profile
row or a `fields-pending.ids` line in the same commit; a stale pending line then fails `video-pending-stale`.
**Avoid:** either don't touch the rule's `fields`, or add the row in the same commit (W1 already forces that).

### Pitfall 7: Exit A on the latch rule is the expensive one
**What goes wrong:** the phase decides "persist", then discovers `stic.display_latch` is a digest row
(`machine_digest.c:143`), the HP-LATCH cut literal moves, the `video` suite's displayed-flag id moves and possibly
several goldens' `digest.instance`. A window-length flip is far cheaper (its ids assert relations).
**Avoid:** measure the *failing set* for each candidate flip before choosing, and price the retarget into the
`W1` enumeration. This is the one place where "the measurement decides the exit" has a cost consequence the plan
should state in advance.

---

## Code Examples

### A parameter row as a cell (the sweep's unit of work)

```c
/* Source: tests/stic/stic_profile.c:118-137 (the existing one-field reversal), adapted to a named cell. */
static int sp_cell(const ivs_stic_params *p, ivs_stic_params *q, uint32_t cell)
{
  *q = *p;                                        /* the shipped row is never edited */
  if ((cell & 1u) != 0u) {                        /* field 1: the register window length */
    uint32_t idx = sp_index(p, IVS_STIC_EV_REG_CLOSE);
    if (idx >= p->event_count) {
      return 0;
    }
    q->event_tick[idx] = (uint32_t)2900u * q->ticks_per_cpu_cycle;   /* was 2000 x 4 = 8000 */
    q->reg_window_cycles = 2900u;
    q->reg_window_ticks = q->event_tick[idx];
  }
  if ((cell & 2u) != 0u) { q->access_gating_when_off = 0u; }         /* field 2: needs the member */
  if ((cell & 4u) != 0u) { /* field 3: the $0020 READ row's effect_code */ }
  if ((cell & 8u) != 0u) { q->display_latch_clears = 0u; }           /* field 4: needs the member */
  return 1;
}
```

### Driving the guest's stamp under a row (the only injection point that exists)

```c
/* Source: tests/stic/stic_access.c:45-60 (sa_bus) and tests/stic/stic_profile.c:47-60 (sp_blocked_at). */
static uint32_t at(const ivs_stic_params *q, uint64_t stamp, uint8_t kind, uint16_t addr, uint16_t data)
{
  struct ivs_machine *im = ivs_machine_from_handle_mut(x.m);
  ivs_machine_bus_ctx c;
  ivs_cp1610_access a;
  (void)ivs_machine_reset(x.m, IVS_RESET_POWER_ON);
  memset(&c, 0, sizeof c);
  c.m = im;
  c.unit_start_tick = stamp - q->access_strobe_ticks;   /* the stamp the guest measured */
  c.ticks_per_cpu_cycle = q->ticks_per_cpu_cycle;
  c.psg = ivs_psg_params_get(im->profile);
  c.stic = q;                                           /* the cell's row, not the profile's */
  c.sink = NULL;
  memset(&a, 0, sizeof a);
  a.kind = kind;                                        /* IVS_ACC_WRITE / IVS_ACC_READ */
  a.addr = addr;                                        /* 0x0020 */
  a.data = data;
  (void)ivs_machine_bus(&c, &a);
  return im->stic.display_latch;                        /* or blocked_writes, or windows_active */
}
```

### Reading the observable the report would print

```c
/* Source: core/src/stic/stic_frame.c:394,417 and tests/video/video_slot.c:120-127. */
/* frame N displayed == (windows_active at GMEM_CLOSE) == (display_latch != 0 at that instant) */
/* In-process, the same value is: ivs_machine_stic_state(m, &r); r.windows_active; or the slot header's
 * IVS_FRAME_F_DISPLAYED bit at offset 24+3 of the delivered slot. */
```

---

## Runtime State Inventory

Not a rename/refactor/migration phase. **None — verified by reading the phase's scope**: no stored data, live
service config, OS-registered state, secret or build artifact carries a renamed string. The only runtime state
the phase touches is the generated build-tree includes (`ivs_stic_params.inc` and the three sibling tables), which
are build outputs and are regenerated, never migrated.

---

## Environment Availability

| Dependency | Required By | Available | Version | Fallback |
|---|---|---|---|---|
| `cmake` ≥ 3.25 | generators, `ctest` | ✓ | as used by `cmake --preset dev` | — |
| a C17 compiler | the `stic` suite | ✓ | `cc=AppleClang` in the gate line | — |
| `ivs-inspect` (Phase 18) | measuring each guest's stamp | ✓ (build-only target, landed `18-02`) | present in `build/dev/tools/inspect/` | none — without it the stamps are derived arithmetic, which is a *fit*, and the phase would have to record that |
| `refs/` with operator images | `19-02` only | ✗ (owner-local) | — | `skip-ok` + exit 13; criterion 5 says its absence leaves criteria 1–4 standing |
| docker | the container gate | ✗ on this host (measured twice in 18-VERIFICATION: `docker info` rc 124) | — | a visible `not run` line, never a pass |

**Missing dependencies with no fallback:** none for criteria 1–4.

**Missing dependencies with fallback:** the operator images (`19-02` skips); docker (a `not run` line).

---

## Validation Architecture

### Test Framework

| Property | Value |
|---|---|
| Framework | the project's own `ivs_test` harness, run by CTest (`tests/CMakeLists.txt`, `ctest-runs=2`) |
| Config file | `CMakePresets.json` (`dev`, `release`); the `stic` suite is registered at `tests/CMakeLists.txt:318-323` (`IVS_STIC_TEST_SOURCES` at `:318`, `add_test(NAME stic ...)` at `:322`), and the same source list feeds the fault executables at `:574` |
| Quick run command | `ctest --preset dev -R stic --output-on-failure` |
| Full suite command | `sh tools/gate.sh` (and `dash tools/gate.sh`, last lines byte-identical) |

### Phase Requirements → Test Map

| Req ID | Behavior | Test Type | Automated Command | File Exists? |
|---|---|---|---|---|
| VID-07 | G1's observable moves iff `stic.reg_window_cpu_cycles` flips | unit (in-process) | `ctest --preset dev -R stic` | ❌ Wave 0 (new ids) |
| VID-07 | G2's moves iff `stic.access_gating_when_off` flips | unit | same | ❌ Wave 0 |
| VID-07 | G3's moves iff the `$0020` READ row flips | unit | same | ❌ Wave 0 |
| VID-07 | G4's moves iff the latch rule flips | unit | same | ❌ Wave 0 |
| VID-07 | the 16-cell sweep is diagonal; the mutant row can fail | unit + registry | same, plus `sh tools/check/steps/important-gates.sh` | ❌ Wave 0 |
| VID-07 | the `W1` chain moves in one commit | gate | `sh tools/gate.sh` (`state`, `profile`, `glossary`, `video`, `tests-inventory` steps) | ✅ existing steps |
| VID-07 | `19-02` is `skip-ok`, exit 13, `refs/`-only | smoke | `ctest --preset dev -R runner-process` | ❌ Wave 0 (scenario + two inventory lines) |

### Sampling Rate

- **Per task commit:** `ctest --preset dev -R stic`
- **Per wave merge:** `sh tools/gate.sh`
- **Phase gate:** both shells' last lines byte-identical, `-- FAIL` count 0, before `/gsd:verify-work`

### Wave 0 Gaps

- [ ] `tests/stic/stic_disc.c` — the four guests, their guards, the sweep, the pinning ids (new module, added to
      `IVS_STIC_TEST_SOURCES` at `tests/CMakeLists.txt:318` and declared as `ivs_stic_disc_cases[]` in
      `tests/stic/stic_cases.h` beside the eighteen existing family tables)
- [ ] `data/expected/tests/stic.ids` — the new id lines
- [ ] the three parameter members + their branches (`stic_params.h`, `IvsSticTable.cmake`, `stic_events.c`,
      `machine_bus.c`, `stic_regs.c`)
- [ ] `tests/machine/machine_stic_params.c` — the range check for each new member (the
      `alias_read_sensitive` precedent at `:223`)
- [ ] `data/expected/important-gates.ids` — one `mutant` row, landed with the VID-07 flip
- [ ] `data/expected/ivsn-literals.ids` — five values, recomputed by the `f795f2a` recipe

---

## Security Domain

`security_enforcement` is not disabled for this milestone. The phase's surface is small but not empty.

### Applicable ASVS Categories

| ASVS Category | Applies | Standard Control |
|---|---|---|
| V2 Authentication | no | — |
| V3 Session Management | no | — |
| V4 Access Control | no | — |
| V5 Input Validation | **yes** | the profile schema (`tools/check/schema/profile.txt`) and the generator's fail-closed readers (`st_fail`) are the only path a new field takes; a field with a wrong type, an unknown key or a missing `resolving_evidence` stops the build, not the run |
| V6 Cryptography | **yes (no new crypto)** | SHA-256 comes from the existing `ivs_sha256`; the parameter digest preimage framing is pinned by `state.literal.params-digest`'s independent rebuild. Never hand-roll |
| V12 Files & Resources | **yes** | `19-02`'s output path is resolved physically and refused unless under `refs/` (the `flatwrap` rule); the helper refuses `CI`/`GITHUB_ACTIONS` |
| V14 Configuration | **yes** | the new policy member's default must reproduce today's behaviour; a wrong default silently changes every frame |

### Known Threat Patterns for this stack

| Pattern | STRIDE | Standard Mitigation |
|---|---|---|
| An operator image path escaping into a tracked file | Information disclosure | the `refs/`-only rule + `tools/check/binary-ext.txt` (`ivsc` refused by name) + `/refs/` in `.gitignore` |
| A record naming an operator file's digest, size, path or name | Information disclosure | criterion 4's invariant; the `rights` and `claim` steps already refuse a claim citing such an artifact |
| A default changed without discrimination | Repudiation (of the evidence) | GUARDRAILS §2.6 + the diagonal mutant row |
| A silently vacuous check | Tampering | the liveness guard + committed stand-in per guest (§2.7) |

---

## Assumptions Log

| # | Claim | Section | Risk if Wrong |
|---|---|---|---|
| A1 | The four rival rows are C1, C6, C82 and C94 | §4 | A plan that appends to the wrong four rows leaves a rival unnamed; the criterion fails on a records technicality, not on the measurement |
| A2 | `stic.display_latch` appears in no golden's `digest.instance` for the current scenarios, so a persist-policy flip may move no golden | §3, Pitfall 7 | Exit A on field 4 costs a golden retarget nobody priced; mitigated by measuring the failing set first |
| A3 | The `stic` suite is the right home for all four ids (ROADMAP's cost line says so; G2 has no report key) | Summary, §2c | If the plan instead puts G1/G3/G4 in `runner-process` as scenarios, the sweep still has to be in-process and the phase acquires two homes for one observation |
| A4 | The three new policies can be added as 0/1 members whose default reproduces today's bytes | §1d, §3 | If a default cannot be made byte-identical, `goldens=29` or a digest literal moves for a reason that is not the decision — a stop |
| A5 | One id looping the 16 cells is cheaper and safer than 16 cell ids | §6 | If the criterion is read as demanding 16 ids, the price reaches 1.89 and `tests-expected` grows by 32 |
| A6 | `refs/` is present and gitignored on the target host (it exists here) | §5 | `19-02`'s fallback is the skip, which criterion 5 already permits |

---

## Open Questions

All three are **(RESOLVED)** at the plan set committed as `e148500`, and each names where it lands. A later
reader should treat them as settled decisions, not as open research.

1. **(RESOLVED) Does the latch field belong in `stic.power_on` or at the top level?**
   - What we know: `stic.power_on.display_latch` is the *power-on* value (`stic.power_on`, C85), a different
     question from the per-frame sustain rule (C6/C82).
   - What was unclear: whether the plan wants one field or two.
   - **Resolution: a new top-level field**, `stic.display_latch_cleared_each_frame`, because folding the sustain
     rule into the power-on record would make one row answer two conflicts. It lands as the profile row plus its
     inventory line plus the whole W1 retarget in `19-01-PLAN.md` Task 2 (step 4 and step 6), and is the field
     the G4 guest sweeps in `19-01-PLAN.md` Task 3.

2. **(RESOLVED) Should the sweep's observation be `display_latch`, `windows_active`, or the slot header bit?**
   - What we know: `displayed` = `windows_active` at GMEM_CLOSE = `display_latch != 0` at that instant
     (`stic_events.c:66`, `stic_frame.c:394`).
   - What was unclear: which of the three the criterion means by "frame 0 `displayed`".
   - **Resolution: all three, pinned as one word by one id.** `stic.disc.displayed-equivalence`
     (`19-01-PLAN.md` Task 1, step 6) asserts the equality in both polarities and names the slot header bit
     (`video.slot.displayed-flag-follows-latch`) and the runner's `frame N displayed` line as the other two
     readings; the guests read the machine state, and the sweep cells read the same machine-state call, so no
     plan has to choose between the three.

3. **(RESOLVED) Is `19-02` a scenario, a helper, or both?**
   - What we know: exit 13 is a *scenario outcome*; the `refs/`-only refusal is a *helper* rule; the two live in
     different files today (`owner-run.sh` + `needs-user-exec.ivss`).
   - **Resolution: both**, as `owner-run.sh` and `needs-user-exec` already are. `19-02-PLAN.md` Task 1 is the
     helper (`tools/disc-sweep.sh`, which carries the bound, the `refs/` rule and the `CI` refusal, and exits 77
     when the images are absent) and Task 2 is the `skip-ok` scenario that carries the visible skip and exit 13.
     The two codes are different on purpose and the helper's header says so.

---

## Sources

### Primary (HIGH confidence — read out of this tree at `8786e20`)
- `data/profiles/intv-2609-ntsc-family-p1.json`, `data/expected/profile-fields.ids:114,119,132`
- `tools/check/schema/profile.txt`, `tools/check/kinds/profile.cmake`, `tools/check/schema/discrepancy.txt`,
  `tools/check/kinds/discrepancy.cmake`
- `cmake/IvsSticTable.cmake:194-201,373-377,752`, `cmake/IvsMemmapTable.cmake:635-740`
- `core/src/stic/stic_params.h:58-59,90-99`, `core/src/stic/stic_regs.c:32-41`,
  `core/src/stic/stic_events.c:55,66`, `core/src/stic/stic_frame.c:394,417`,
  `core/src/machine/machine_bus.c:78-93`, `core/src/machine/machine_run.c:175`
- `core/include/ivs/ivs_machine.h:85-89,128`, `core/include/ivs/ivs_run.h:102,114`
- `runner/report.c:604`, `docs/runner.md` (`## Report`, `## Scenario grammar`, `## Outcomes and exit codes`)
- `tests/stic/stic_profile.c:1-8,118-137`, `tests/stic/stic_access.c:45-60,209-219,371-440`,
  `tests/stic/stic_progs.c:9-79`, `tests/stic/stic_cases.h:116-141`
- `tests/video/video_slot.c:120-127`, `tests/state/state_literal.c:160-200,316-320,463`
- `data/expected/tests/stic.ids:21-34,46-50`, `data/expected/tests/runner-process.ids:7,118`,
  `data/expected/runner-scenarios.ids`, `data/expected/ivsn-literals.ids`, `data/expected/handoffs.ids`
- `data/discrepancies.json` C1, C6, C82, C83, C84, C85, C94; `data/stic/rules.json` STIC-R13, STIC-R15, STIC-R20,
  STIC-R22
- `tools/owner-run.sh:3,25,45-46`, `tools/flatwrap/flatwrap.c:158-196,349`, `.gitignore:2`,
  `docs/owner-images.md:5,15,46-47`
- `tools/check/steps/important-gates.sh:157-164`, `tools/check/steps/glossary.sh:79-92,99`,
  `tools/check/steps/discrepancies.sh`
- `git log -1 --format=%B f795f2a` — the parameter-digest and snapshot-literal recipe
- `.planning/phases/18-the-inspection-surface/18-VERIFICATION.md` — the gate line, the `ivs-inspect` surface,
  observation O4 (the derived must-be-covered set)
- `docs/inspection.md` `## The trace file`

### Secondary (MEDIUM confidence)
- `.planning/research/v0.4.0-scoping-2-gate-cost.md` §2.3 — the frozen unit prices and Phase 19's 1.64 line
- `.planning/research/v0.4.0-scoping-1-phase-decomposition.md` §1.3-1.6 — the three plans and the throwaway probe

### Tertiary (LOW confidence — flagged for measurement)
- The per-field "which ids move" table in §3 is read from the assertions, not from running the flips. Every row
  of it is a prediction the plan must confirm by running the flip and recording the measured failing set, exactly
  as `f795f2a` did ("Measured failing set with the field at 1 before the retarget: …").

---

## Metadata

**Confidence breakdown:**
- Standard stack: **HIGH** — no new dependency; every mechanism named is already in the tree and cited by line
- Architecture: **HIGH** — the three facts that shape the phase (no per-run row, two fields unreachable, G2 has no
  report key) are each a two-line read of a named file
- Pitfalls: **HIGH** on 1–6 (each is a gate step read in full), **MEDIUM** on 7 (the cost of Exit A per field is
  predicted, not measured)
- Retarget sizes: **MEDIUM** — the token arithmetic is exact, the "which ids move" set is a reading

**Research date:** 2026-11-27
**Valid until:** 30 days (stable tree; re-read if `core/src/stic/`, `cmake/IvsSticTable.cmake` or
`data/profiles/` change)

## RESEARCH COMPLETE

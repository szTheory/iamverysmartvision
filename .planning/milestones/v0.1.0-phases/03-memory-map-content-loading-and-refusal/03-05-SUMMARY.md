---
phase: 03-memory-map-content-loading-and-refusal
plan: 05
subsystem: gate
tags: [memmap, literal-map, gate-step, controls, D-18, D-21, D-38, H11]

requires:
  - phase: 03-memory-map-content-loading-and-refusal
    provides: "plan 03-04: profile-driven memory system, discrepancies C60-C62, device vocabulary of the decode introspection"
provides:
  - "data/memmap/intv-2609-ntsc-family-p1.literal.txt: the sourced literal map (D-21), 12 read rows and 16 write rows, each view tiling 0000-FFFF with explicit undriven rows, 20 rows citing a source id and locator"
  - "Gate step memmap (tools/check/steps/memmap.sh + tools/check/memmap.cmake): one literal per profile, no orphan memmap file, row grammar, per-view tiling, src/locator/tag/conf provenance, tier-5 refusal by tag and by source tier"
  - "data/expected/gate-steps/machine.ids (memmap); steps=25/25"
  - "Ten controls memmap-literal-{missing,orphan,grammar,hole,overlap,no-cite,unknown-source,tier5,unknown-tag,unknown-conf} and data/expected/controls/memmap.ids; controls=212/212"
  - "Good-tree toy literal (one undriven row per view, conf=C1); the good-tree profile is unchanged (H11)"
affects: [03-06, 03-07, 03-11, 03-13]

actuals:
  tokens: 6450
  tasks: 2
  commits: 3
plan_head_before: ed349149190b7cc51fbb9cf12d2fab6938852229
plan_head_after: ac3057315213412d93b064a6c49e008758363343

tech-stack:
  added: []
  patterns:
    - "A checker whose gate_finish name equals the step name: the wrapper filters the checker's own pass line and prints it once, so gate.sh does not collect the stats twice"
    - "Positional row grammar checked by key sequence first (w impl mirror gate src tag [conf]), so a missing column gives one finding that names the keys found"
    - "A row whose view and range parse joins the tiling check even when another column is bad, so one defect reports one token"

key-files:
  created:
    - data/memmap/intv-2609-ntsc-family-p1.literal.txt
    - tools/check/memmap.cmake
    - tools/check/steps/memmap.sh
    - data/expected/gate-steps/machine.ids
    - data/expected/controls/memmap.ids
    - tools/check/controls/_good/tree/data/memmap/intv-2609-ntsc-family-p1.literal.txt
    - tools/check/controls/memmap-literal-*/ (10 control directories)
  modified: []

key-decisions:
  - "Mirror column definition: mirror=<n> when the row reaches a backing or register file of n words that is smaller than the row or shared with another row (an alias); - when the row is the sole full-extent home of its device. So 0040-007F and the write-only STIC aliases carry mirror=64, every GRAM row mirror=512, and scratchpad and sysram (backing larger than the decoded extent) carry -"
  - "Write-view width is the width a store keeps when read back; dropped stores (stub, undriven) are w=0"
  - "Device and gate vocabularies are closed in the checker (stic scratchpad psg sysram exec grom gram cart undriven; stic-regs gmem); a new device or gate needs a checker edit"
  - "The tier-5 refusal fires on the tag and also on a src whose data/sources.json authority_tier is tier5-emulator-hint (SRC-mame-intv), independent of whether the tree's tiers.json lists that tag"
  - "The checker reads profiles and memmap files from FILELIST (like vectors.cmake), not by globbing, so untracked-ignored files never count"
  - "GI 1980 locators use pdf pages (pdf = printed + 181): pdf-197 GROM and GRAM window, pdf-199 system RAM, pdf-202 cartridge ROM width; the wiki locator names the page and revision (Memory_Map.oldid-14847) because the registry URL of SRC-intv-wiki is a different page of the same wiki"
  - "MEM-01 stays open: the dump comparison against the runtime decode is plan 03-06"

patterns-established:
  - "data/memmap/<profile-id>.literal.txt is the only place a region is restated outside the profile, and it is written from sources only"

requirements-completed: []

coverage:
  - id: D1
    description: "Sourced literal map tiling both views (D-21, T-03-16)"
    requirement: MEM-01
    verification:
      - kind: integration
        ref: "sh and dash tools/gate.sh: -- memmap pass literal-rows=28 views=2"
        status: pass
    human_judgment: false
  - id: D2
    description: "Static literal rules each fail for their own token (T-03-15, T-03-16)"
    requirement: MEM-01
    verification:
      - kind: other
        ref: "sh tools/check/run-controls.sh: control ok for _good/memmap and all ten memmap-literal-* controls; controls=212/212"
        status: pass
    human_judgment: false

duration: 8min
completed: 2026-11-11
status: complete
---

# Phase 3 Plan 05: Sourced literal memory map and the memmap gate step Summary

**`data/memmap/intv-2609-ntsc-family-p1.literal.txt` is the D-21 oracle: 28 rows written from the recorded source readings (GI 1980 catalog pdf 197/199/202, the 2609 overview, wiki memory-map revision 14847), never from the profile. Each view tiles `0000-FFFF` exactly once. The new `memmap` gate step checks the literal of every profile for grammar, tiling and provenance and refuses tier-5 rows. Ten controls show that each rule fails for its own token. Final gate under sh and dash: `gate: PASS steps=25/25 controls=212/212 ... faults=79/79 ... literal-rows=28 views=2 ... tests-expected=1206 tests-executed=1206 tests-skipped=0 tests-missing=0`.**

## Performance

- **Duration:** about 8 min of execution (2026-11-11T05:53:40Z to 06:00:54Z), not counting the reading before the recorded start
- **Tasks:** 2 of 2
- **Files:** 40 in 3 commits (diff +549 / -0)
- **Gate time:** `gate-time total=18` to `20` (sh) and `18` to `19` (dash) on this host, against 19 to 24 at the end of 03-04; `step-time memmap 0`

## Accomplishments

- **Literal map (D-21, D-18).**
  - Header: row grammar, device vocabulary, width and mirror definitions, claim tier `source-expectation` (a pass shows transcription consistency, not hardware truth), the independence rule, and per-group source notes labelled OBS/INF.
  - Read view, 12 rows: STIC `0000-003F` stub gate stic-regs; STIC alias `0040-007F` stub mirror 64 gate stic-regs; undriven `0080-00FF`; scratchpad `0100-01EF` w=8; PSG `01F0-01FF` stub; sysram `0200-035F` w=16; undriven `0360-0FFF`; EXEC `1000-1FFF` w=10; undriven `2000-2FFF`; GROM `3000-37FF` w=8 gate gmem; GRAM `3800-3FFF` w=8 mirror 512 gate gmem; cart `4000-FFFF` w=10.
  - Write view, 16 rows: STIC `0000-003F`; undriven `0040-00FF`; scratchpad; PSG; sysram; undriven `0360-37FF` (EXEC and GROM space); GRAM `3800-3FFF`; STIC aliases `4000-403F`, `8000-803F`, `C000-C03F` (stub, mirror 64, gate stic-regs); GRAM aliases `7800-7FFF`, `B800-BFFF`, `F800-FFFF` (w=8, mirror 512, gate gmem); undriven rows between them.
  - Tags: tier1-primary-doc for sysram (pdf-199) and GROM (pdf-197); `inference` for GRAM `3800-3FFF` (window at pdf-197, decode of the whole window onto 512 words is inferred, C21); unknown-default-chosen for the cart span (width at pdf-202, span C62); tier4-community for the wiki rows (C11, C20, C21, C60, C61); every undriven row `src=none tag=unknown-default-chosen` with conf C5 (C21 for write `0040-00FF`).
- **Checker and step.** `tools/check/memmap.cmake` copies the `vectors.cmake` skeleton (required `-D` guard, `lib.cmake`, a per-token cap of 20 with exact totals, `gate_stat`/`gate_finish`). Tokens: `memmap-literal-{missing,orphan,grammar,hole,overlap,no-cite,unknown-source,tier5,unknown-tag,unknown-conf}`. Stats: `literal-rows=<n> views=<2 x profiles>`. `tools/check/steps/memmap.sh` is the POSIX wrapper; it prints `-- memmap pass literal-rows=28 views=2`.
- **Inventories.** `data/expected/gate-steps/machine.ids` (header `# Gate steps for Phase 3 (plans 03-05, 03-13).`, line `memmap`); `data/expected/controls/memmap.ids` (header `# Negative controls for the memmap step (plans 03-05, 03-06).`, ten names).
- **Good tree (H11).** Only the toy literal was added; the good-tree profile is untouched.

## Controls (Task 2)

Each control was run on its own copy of the good tree to confirm it fails only for its token:

| Control | Change | Failure printed |
|---------|--------|-----------------|
| memmap-literal-missing | MAKE delete the literal | `memmap-literal-missing: data/memmap/intv-2609-ntsc-family-p1.literal.txt (profile data/profiles/intv-2609-ntsc-family-p1.json)` |
| memmap-literal-orphan | MAKE write `data/memmap/other-profile.literal.txt` (FORBID missing) | `memmap-literal-orphan: data/memmap/other-profile.literal.txt ...` |
| memmap-literal-grammar | read row without `impl=` (FORBID hole) | `...:2 expected <view> <LO-HI> <device> then w impl mirror gate src tag [conf], found 9 columns with keys 'w mirror gate src tag conf'` |
| memmap-literal-hole | read row ends at `FFFE` | `... R FFFF-FFFF` |
| memmap-literal-overlap | read rows `0000-8000` and `8000-FFFF` (FORBID hole) | `... R 0000-8000 8000-FFFF` |
| memmap-literal-no-cite | `cart` row with `src=none` | `...:2 src=none needs an undriven row tagged unknown-default-chosen with conf=` |
| memmap-literal-unknown-source | `src=SRC-not-registered@x` | `...:2 source 'SRC-not-registered'` |
| memmap-literal-tier5 | `tag=tier5-emulator-hint` (FORBID unknown-tag) | `...:2 tag tier5-emulator-hint` |
| memmap-literal-unknown-tag | `tag=not-a-tag` | `...:2 tag 'not-a-tag'` |
| memmap-literal-unknown-conf | write row `conf=C9999` | `...:3 conf 'C9999'` |

A scratch probe on a copy of the real tree also showed the source-tier path: a row citing `SRC-mame-intv@x` gives `memmap-literal-tier5: ... source SRC-mame-intv is tier5-emulator-hint`.

## Task Commits

1. **Task 1 (tracer): literal map checked by the new memmap step in the real gate.** `18067c7` (feat). Tracer verified end to end before expansion: `sh tools/gate.sh` and `dash tools/gate.sh` both ended `gate: PASS steps=25/25 controls=202/202`, and `sh tools/gate.sh | grep -c -E '^-- memmap pass literal-rows=[0-9]+'` printed 1.
2. **Task 2: ten controls for the literal rules.** `6579240` (feat). The control runner ended `controls=212/212`; `grep -c -E 'control ok +memmap-literal-'` printed 10; the gate passed under sh and dash.
3. **Header note on the alias gates.** `ac30573` (docs). It labels the alias gate column as an inference (see the literal-versus-profile table below). The gate passed under sh and dash.

**Plan metadata:** recorded in the docs(03-05) completion commit.

## Acceptance evidence

| Check | Result |
|-------|--------|
| `grep -c -E '^R ' data/memmap/intv-2609-ntsc-family-p1.literal.txt` | 12 |
| `grep -c -E '^W ' data/memmap/intv-2609-ntsc-family-p1.literal.txt` | 16 |
| `grep -v '^#' <literal> \| grep -c -E 'src=SRC-[a-z0-9-]+@[^ ]'` | 20 |
| `sh tools/gate.sh \| grep -c -E '^-- memmap pass literal-rows=[0-9]+'` | 1 |
| `sh tools/check/run-controls.sh \| grep -c -E 'control ok +memmap-literal-'` | 10 |
| `grep -v '^#' data/expected/controls/memmap.ids \| grep -c .` | 10 |
| run-controls last line | `controls=212/212` (plan text: 210, see Deviations) |

## Literal reading versus the 03-04 profile

The profile was opened only after the literal was committed (independence rule). Structural differences that plan 03-06's dump comparison will show:

| Rows | Literal | Profile | Note for 03-06 |
|------|---------|---------|----------------|
| W `4000-403F`, `8000-803F`, `C000-C03F` | `gate=stic-regs` | `stic_alias_w{4,8,c}.gate = none` | The literal reading is an inference (an alias reaches the home registers, so it shares their access window), now labelled in the header. Under D-18 the literal overrides; 03-06 decides whether to change the profile gate citing the wiki, or to record that this inference is too weak and keep it as a C61/C21 note. |
| W `7800-7FFF`, `B800-BFFF`, `F800-FFFF` | `gate=gmem` | `gram_alias_w{7,b,f}.gate = none` | Same reasoning for graphics RAM. |

Not differences, but things the dump must reproduce:
- **Mirror column.** The profile `size_words` is the power-of-two backing size (scratchpad 256, sysram 512), not a mirror. The literal writes `mirror=-` for those rows and `mirror=64`/`512` only for aliases and for GRAM, whose 2K extent exceeds its 512-word backing. The dump has to use the literal's definition (stated in its header), not print `size_words`.
- **Device names.** The literal uses the physical names (`stic`, `scratchpad`, `psg`, `sysram`, `exec`, `grom`, `gram`, `cart`, `undriven`), as the plan's shared vocabulary requires, not the profile's device kinds (`rom10`, `ram8` and so on).
- Tags and conf ids differ in places (for example GRAM is tagged `inference` in the literal and `tier4-community` in the profile, GROM carries C5 only in the profile). These columns are stripped before the comparison.

## Decisions Made

See `key-decisions` in the frontmatter.

## Deviations from Plan

### Auto-fixed Issues

**1. [Rule 2 - Missing check] Two more rules, each with a control (controls 212, not 210)**
- **Found during:** Task 1, while writing the tier rule.
- **Issue:** The plan's token list has no token for a tag outside `data/tiers.json` or for a file in `data/memmap/` that belongs to no profile. The good tree's `tiers.json` does not list `tier5-emulator-hint`, so the tier-5 check must not depend on the tag being known, and an unknown tag needs its own finding. An orphan literal would otherwise go unchecked here, and plan 03-06 generates probes from every `data/memmap/*.literal.txt`.
- **Fix:** Added tokens `memmap-literal-unknown-tag` and `memmap-literal-orphan`, each with a control. The tier-5 check runs before the tag lookup.
- **Effect:** `controls=212/212` instead of 210. Plan 03-06's expected count becomes 215 (212 + 3).
- **Committed in:** `18067c7`, `6579240`.

**2. [Rule 1 - Bug] Duplicate pass line**
- **Issue:** `gate_finish(memmap)` prints `-- memmap pass ...`, the same text the wrapper prints, and `gate.sh` collects every such line, so the stats would have appeared twice in the summary.
- **Fix:** The wrapper filters the checker's pass line and prints it once.
- **Committed in:** `18067c7`.

**3. [Rule 1 - Bug] CMake `if()` evaluation order**
- **Issue:** `if(x MATCHES re AND (CMAKE_MATCH_1 ...))` evaluates the parenthesised group before `MATCHES`, so the gate column was read from the previous match. This was found on the first run against the real literal.
- **Fix:** The match and the vocabulary test are two statements.
- **Committed in:** `18067c7`.

**4. [Clarity] Grammar finding names the key sequence**
- **Issue:** With purely positional parsing, a row with one missing column reported every shifted column.
- **Fix:** The keys of columns 4 onward are compared with `w impl mirror gate src tag [conf]` first, and one finding names what was found.
- **Committed in:** `6579240`.

**5. [Data] Alias gate note**
- **Issue:** The alias gate column is an inference and differs from the profile (see the table above).
- **Fix:** One header line labels it [INF]. No row changed.
- **Committed in:** `ac30573`.

---

**Total deviations:** 5, all small. No existing token, test id, fault or control changed. Steps 25, faults 79 and tests 1206 are as the plan expects.

## Issues Encountered

None beyond the deviations.

## Open items

- **memmap-gen-* generator controls (from 03-04): not added here.** The `memmap` step is a pure checker of literal files on a text tree. Gate-level generator controls would need a profile with `memmap.*` fields in the control tree, which H11 rules out for the shared good-tree profile. The alternative, letting the step skip profiles without memmap fields, would fail open. The generator stays covered by its build-time fail-closed refusals (the 03-04 scratch probes) and by `GEN_MAP_MUTATION`. A later plan should add them, either in 03-06 or in a dedicated step with its own toy memmap profile overlay.
- **Unused generated-include lines** (from 03-03) are still not flagged. Out of scope here.
- **MEM-01 stays open** until plan 03-06 adds the dump comparison. MEM-01 was not marked complete.
- **Plan 03-06 counts.** Controls become 215 (see Deviation 1). The alias-gate difference above has to be resolved there.
- **Hosted runners.** This plan adds cmake-script and shell code only, checked under sh and dash on macOS. The first hosted run checks the Ubuntu shells.

## Known Stubs

None. The literal's `impl=stub` rows (STIC, PSG) describe the intended stub devices of D-19. They are not placeholders in this plan's output.

## Threat Flags

None. The threat register already covers the new surface:
- T-03-15: `src=`/`tag=`/`conf=` are required, tier-5 is refused by tag and by source tier, unknown sources are refused, and every rule has a control.
- T-03-16: exact tiling per view, with hole and overlap controls.
- T-03-17: the denylist step scanned the literal (`scanned=143`). It names no emulator and quotes no source text.

## User Setup Required

None.

## Next Phase Readiness

- Plan 03-06 can generate probes from the literal rows and compare a decode dump with the literal's structural columns: view, range, device, w, impl, mirror and gate.
- Plans 03-07 and 03-11 can read the cart row `R 4000-FFFF cart w=10` for the span.
- **Final gate:**
  - `sh tools/gate.sh`: `gate: PASS steps=25/25 controls=212/212 cc=AppleClang c-presets=2 fault-runs=79 ... faults=79/79 ... literal-rows=28 views=2 ... tests-expected=1206 tests-executed=1206 tests-skipped=0 tests-missing=0 ...` (`gate-time total=19`)
  - `dash tools/gate.sh`: the same summary line (`gate-time total=18`).

## Self-Check: PASSED

- Files: the literal map, `tools/check/memmap.cmake`, `tools/check/steps/memmap.sh`, `data/expected/gate-steps/machine.ids`, `data/expected/controls/memmap.ids` and the good-tree literal were found.
- Commits: `18067c7`, `6579240` and `ac30573` are ancestors of HEAD. The ledger count from `ed34914` is 3.
- Hygiene: no absolute home path and no "Intellivision" in any added line. `.planning/config.json` and `.planning/state.json` are still ` M` and in no commit of this plan.

---
phase: "18"
slug: "the-inspection-surface"
status: complete
nyquist_compliant: true
wave_0_complete: true
created: "2026-11-26"
---

# Phase 18 — Validation Strategy

> Per-phase validation contract for feedback sampling during execution.

---

## Test Infrastructure

| Property | Value |
|----------|-------|
| **Framework** | the project's own TAP shell suites, one `ctest` test per suite (`tests/CMakeLists.txt:2-4`), under the `dev` and `release` presets |
| **Config file** | `CMakePresets.json`; `tests/CMakeLists.txt` registers the suites; `data/expected/tests/*.ids` are the id inventories |
| **Quick run command** | `ctest --preset dev -R '^asm$'` (measured 7.60 s) or `ctest --preset dev -R '^runner-process$'` (measured 29.28 s) |
| **Full suite command** | `sh tools/gate.sh` and `dash tools/gate.sh` (the `GUARDRAILS-v0.4` §4 definition of done) |
| **Estimated runtime** | measured on this audit's tree, one run per shell, sequentially: `gate-time total=109` (`sh`) and `total=103` (`dash`), `gate-cpu user=218.47 sys=207.32` / `user=210.99 sys=184.91`, `gate-env ncpu=18 cc=AppleClang-21.0.0.21000101`. The earlier revision quoted 164 from the Phase 17 start commit; that figure is superseded by this measurement, not by a re-quote. `18-02`'s §2.4 repricing candidate (priced 2.96, recomputed 3.59, +21 %) is a records item for the milestone close and is unchanged here |

---

## Sampling Rate

- **After every task commit:** `ctest --preset dev -R '^asm$'` and `ctest --preset dev -R '^runner-process$'`
- **After every plan wave:** `sh tools/gate.sh` and `dash tools/gate.sh`, **sequentially**, both ending `gate: PASS` with the same summary line byte for byte (`cmp`)
- **Before `/gsd:verify-work`:** the full gate plus one `sh tools/gate-linux.sh`; copy `build/gate-linux.log` aside first (lesson 18), one container gate at a time
- **Max feedback latency:** 300 s — measured 109 s (`sh`) and 103 s (`dash`)

> Two gate runs must never share `build/`. This audit's first attempt started `sh` and `dash` concurrently and the
> `dash` run read a half-built tree: `-- FAIL c-build-failed: dev` and `-- FAIL c-tests-failed: release`, summary
> `gate: FAIL steps=36/38`. That was a harness race, not a tree defect; the two runs above were then taken
> sequentially and agree byte for byte. Recorded so a later reader does not mistake a concurrent run's `steps=36/38`
> for a regression.

---

## Per-Task Verification Map

The three plans ship **five tasks**, not ten: `18-02` is one tracer task (the target, the two passes, the
reconciliation, the output policy, the rights row and four `runner-process` ids), one expansion task (breakpoints,
the `step` surface, the token-coverage id and four stand-ins) and one task that closes the two build-only proofs and
the registry. The earlier revision of this table listed `18-02-T1..T6`, which no plan ever shipped.

**Every row below is `✅ green` because its command was run on this tree during this audit** (`V1`–`V5`, exact
commands and outputs in `## The commands this audit ran`). No row is green by inheritance from a SUMMARY, and no
row is marked green for a check that only *would* pass.

| Task ID | Plan | Wave | Requirement | Threat Ref | Secure Behavior | Test Type | Automated Command | File Exists | Status |
|---------|------|------|-------------|------------|-----------------|-----------|-------------------|-------------|--------|
| 18-01-T1 | 01 | 1 | TOOL-02, TOOL-09 | T-18-01, T-18-02 | the frozen document cannot be retro-fitted to the code, and it is written to conditions rather than to a scanner | verify | **V1** — `git log --diff-filter=A -- docs/inspection.md` yields one commit, that commit's only file is the page, the freeze version is a byte prefix of the page now, and `git merge-base --is-ancestor <freeze> <first tools/inspect commit>` succeeds. The page's own pins are read by `V2` (five refusal tokens, both directions) and `V5` (`## Disassembly`'s derivation, by the round trip) | ✅ | ✅ green |
| 18-02-T1 | 02 | 2 | TOOL-02, TOOL-09 | T-18-03, T-18-04, T-18-08 | the report is the library's bytes captured through `write_report` (no record is ever constructed), the trace reconciles with that text, and no output can enter the tree | build + suite + step | **V2** — `cmake --build --preset dev --target ivs_inspect`; `ctest --preset dev -R '^runner-process$'` → `ok 150`–`ok 153` (`inspect.trace-records`, `inspect.trace-digests`, `inspect.neg.inspect-output-tree`, `inspect.neg.inspect-output-exists`), `executed=161 skipped=2 failed=0`; the five-token both-directions comparison (`inspect.surface`, `ok 158`); `sh tools/check/steps/files.sh` → `-- files pass tracked=2836 binaries-listed=0 rights-pinned=4/4` (this step carries `rights_scan_naming`); `git ls-files \| grep -E '\.ivst$'` → empty | ✅ | ✅ green |
| 18-02-T2 | 02 | 2 | TOOL-02 | T-18-05, T-18-07 | a check that cannot pass vacuously, and a bounded trace that cannot silently truncate | suite | **V3** — `ctest --preset dev -R '^runner-process$'` → `ok 154`–`ok 162`: the breakpoint pair `(1, 0)` with the in-run and file-derived counts reconciling (`inspect.break-pair`), byte-identical digests and access records under a breakpoint (`inspect.break-noninterference`), the `step` unit count equal to the trace's (`inspect.step`), the token coverage in both directions (`inspect.surface`), and the four stand-ins `judge.trace-empty`, `judge.trace-repeat-tick`, `judge.trace-pokes-register`, `judge.break-always-hits`, each judged by the same function the real case uses. The fifth demonstration, `trace-target-instead-of-stop`, is a second limb **inside** `inspect.trace-state` (`ok 154`), so it adds no id | ✅ | ✅ green |
| 18-02-T3 | 02 | 2 | TOOL-02, TOOL-09 | T-18-06 | the tool cannot acquire an install rule or a release membership unnoticed, and a row cannot outrun its requirement flip | step | **V4** — `pkg-selftest=21/21` includes `inspect-absent` (`pkg-inventory-extra: bin/ivs-inspect`); `release-selftest=13/13` includes `inspect-unlisted-member` (the existing extra-member limb, whose negative limb proves the extension-only binary rule cannot see an extensionless member); `prefix-files=11` with `data/expected/pkg-install-files.ids` at 11 data lines and zero `install(` in `tools/inspect/CMakeLists.txt`; `important-gates` prints **`gates=122 lines=372`** with no finding and six Phase 18 rows; TOOL-02 and TOOL-09 `Complete` in both the checkbox and the traceability row | ✅ | ✅ green |
| 18-03-T1 | 03 | 3 | TOOL-02 | T-18-09, T-18-10, T-18-11 | the round trip is against the assembler, not against itself, and every `ivs_asm` call names a relative source path so the verdict is the words and not the usage rule | suite | **V5** — `ctest --preset dev -R '^asm$'` and `ctest --preset release -R '^asm$'`, each `executed=56 skipped=0 failed=0` with `ok 6`–`ok 11`: disassemble, re-assemble, words equal, `words > 0`, a two-word form and an immediate present, at least three distinct mnemonics, `inspect-dis-row` fail-closed, constant-mnemonic stand-in failing the byte equality, determinism asserted once | ✅ | ✅ green |

*Status: ⬜ pending · ✅ green · ❌ red · ⚠️ flaky*

**Continuity:** all five tasks have an automated verify, so no three consecutive tasks lack one and no row needs a
manual fallback. `18-01-T1` is a prose task and is still tool-checked: the ancestry proof, the byte-prefix freeze and
the page's own machine-read blocks are all commands, and the page's prose is then held by `V2`/`V5` against the code
that implements it.

**Corrections this audit made to the table** (each recomputed from the tree, never re-blessed):

1. `18-02-T3` predicted `important-gates` would print `gates=121 lines=371`. It reads **`gates=122 lines=372`**,
   because `18-03` added the sixth Phase 18 registry row under the new gate name `asm`. The `121`/`371` pair was the
   intermediate state at `d8eb593`; quoting it now would be quoting a superseded measurement.
2. `18-02-T1` named `sh tools/check/steps/rights.sh`. **No such step exists** (`ls tools/check/steps/*.sh` → 38 files,
   no `rights.sh`). The rights naming limb is `rights_scan_naming`, called from `tools/check/files.cmake:190` and so
   exercised by the **`files`** step. The row now names the step that actually runs the check.
3. The `## Test Infrastructure` runtime row quoted 164 from the Phase 17 start commit; it now carries this audit's
   own sequential measurements.

---

## The commands this audit ran

Run on the tree at the phase's final `HEAD`, in this order, after the two sequential gate runs. Every figure quoted
anywhere above comes from one of these.

### V1 — the freeze and its ancestry (`18-01-T1`)

```sh
F=$(git log --format=%H --diff-filter=A -- docs/inspection.md)
printf '%s\n' "$F" | wc -l                                  # 1
git show --name-only --format= "$F"                         # docs/inspection.md, nothing else
git ls-tree -r --name-only "$F" | grep -c '^tools/inspect/'  # 0
git merge-base --is-ancestor "$F" \
  "$(git log --format=%H --reverse -- tools/inspect/ | head -1)"   # rc 0
git show "$F:docs/inspection.md" > /tmp/ivs-frozen.md
head -c "$(wc -c < /tmp/ivs-frozen.md)" docs/inspection.md | cmp - /tmp/ivs-frozen.md   # rc 0
```

Measured: one add-commit `0627be925fd1fc449260c2d4bb8efe20363e8994`, that commit contains only `docs/inspection.md`,
zero `tools/inspect/` paths exist at it, it is an ancestor of the first `tools/inspect` commit
`5ed5610e7268fc1b7649ba269d98a42fb9ac195a`, and the frozen page (15,699 B) is a byte prefix of the page now
(21,194 B). `git log --format=%H -- docs/inspection.md` reads **2** commits: the freeze and `e936f8d`, a pure append
(77 insertions, 0 deletions) carrying the six dated `## Amendments`. That second commit is the append-only mechanism
the page's own `## Amendments` section requires, and it is why the criterion is a *prefix* comparison rather than a
one-commit rule; it is recorded as debt **D2** in `18-SECURITY.md` rather than re-blessed.

### V2 — the target, the tracer's ids and the naming limb (`18-02-T1`)

```sh
cmake --build --preset dev --target ivs_inspect            # ninja: no work to do (already built)
ctest --preset dev -R '^runner-process$' -V                # rc 0
#   ok 150 runner-process.inspect.trace-records
#   ok 151 runner-process.inspect.trace-digests
#   ok 152 runner-process.inspect.neg.inspect-output-tree
#   ok 153 runner-process.inspect.neg.inspect-output-exists
#   # suite=runner-process executed=161 skipped=2 failed=0
sed -n '/^## Refusal tokens/,/^## Liveness guards/p' docs/inspection.md \
  | grep -o -E 'inspect-[a-z-]+' | sort -u > /tmp/page.tokens
grep -h -o -E '"inspect-[a-z-]+"' tools/inspect/*.c | tr -d '"' | sort -u > /tmp/tool.tokens
comm -3 /tmp/page.tokens /tmp/tool.tokens                  # empty: identical, both directions
git ls-files | grep -E '\.ivst$' | wc -l                   # 0
# the files step, with the gate's own FILELIST contract (tracked + untracked-not-ignored, existing paths)
ROOT=$PWD CHECK_DIR=$PWD/tools/check FILELIST=$T/filelist sh tools/check/steps/files.sh
#   -- files pass tracked=2836 binaries-listed=0 rights-pinned=4/4
```

Both token sets are the same five names — `inspect-break-empty`, `inspect-dis-row`, `inspect-output-exists`,
`inspect-output-tree`, `inspect-reconcile` — so the page and the tool agree in both directions and `18-03`'s token is
no longer excluded.

### V3 — breakpoints, `step` and the stand-ins (`18-02-T2`)

The same `ctest --preset dev -R '^runner-process$'` run, read per id:

```
ok 154 runner-process.inspect.trace-state          (includes the target-instead-of-stop limb)
ok 155 runner-process.inspect.break-pair
ok 156 runner-process.inspect.break-noninterference
ok 157 runner-process.inspect.step
ok 158 runner-process.inspect.surface
ok 159 runner-process.judge.trace-empty
ok 160 runner-process.judge.trace-repeat-tick
ok 161 runner-process.judge.trace-pokes-register
ok 162 runner-process.judge.break-always-hits
```

Nine ids, thirteen committed inspection ids in `data/expected/tests/runner-process.ids` (`:280-292`), zero `not ok`.
The five stand-ins are all committed cases or committed limbs judged by the same helper the real case uses
(`ix_records`, `ix_tick_domain`, `ix_digests_equal`, `ix_break_pair`, `ix_end_tick`); each reports `judge-vacuous`
when its check accepts the stand-in, so a check that has gone blind fails rather than passes.

### V4 — the build-only proofs and the registry (`18-02-T3`)

```sh
grep -c 'install(' tools/inspect/CMakeLists.txt                    # 0
awk '!/^#/ && NF' data/expected/pkg-install-files.ids | wc -l      # 11
grep -c 'inspect-absent' data/expected/pkg-selftests.ids           # 1 (inventory) + 2 in pkg-install.sh
grep -c 'inspect-unlisted-member' data/expected/release-assets-selftests.ids  # 1 + 2 in release-assets.sh
awk '!/^#/ && NF && /TOOL-02|TOOL-09/' data/expected/important-gates.ids | wc -l  # 6
grep -n 'TOOL-02\|TOOL-09' .planning/REQUIREMENTS.md               # both `[x]`, both `| Phase 18 | Complete |`
```

and, from the sequential gate run (`sh tools/gate.sh`, then `dash tools/gate.sh`):

```
-- files pass tracked=2836 binaries-listed=0 rights-pinned=4/4
-- important-gates pass gates=122 lines=372
-- pkg-install pass prefix-files=11 ... pkg-selftest=21/21
-- release-assets pass release-selftest=13/13
```

### V5 — the disassembly round trip (`18-03-T1`)

```sh
ctest --preset dev     -R '^asm$' -V    # rc 0, # suite=asm executed=56 skipped=0 failed=0
ctest --preset release -R '^asm$' -V    # rc 0, # suite=asm executed=56 skipped=0 failed=0
#   ok 6  asm.dis.roundtrip-boot-stub
#   ok 7  asm.dis.two-word-form
#   ok 8  asm.dis.immediate
#   ok 9  asm.dis.words-nonvacuous
#   ok 10 asm.dis.deterministic
#   ok 11 asm.judge.dis-constant-mnemonic
```

The round trip is the three pairwise word dumps of the input listing, the emitted source and the committed twin
(22/22/22, `cmp` clean inside the case), with 22 words against 16 instruction lines, 7 distinct mnemonics and
`MVII #$0001, R0` present — so it cannot pass on a `DECLE`-everything emitter, and the stand-in proves that by
failing the word comparison with a diff.

---

## The measured state of the phase (recomputed, never quoted)

From the two sequential gate runs — last line of each, compared whole with `cmp`, **identical**:

```
gate: PASS steps=38/38 controls=405/405 ... claims=29 provisional=28 handoffs=8 ... faults=238/238
tracked=2836 ... doc-files=64 ... goldens=29 ... gates=122 lines=372 ... files=2836 listings=7
rights=11 rights-cleared=1 ... prefix-files=11 ... pkg-selftest=21/21 ... release-selftest=13/13
... tests-expected=5082 tests-executed=5078 tests-skipped=4 tests-missing=0 ... docs=65 ...
```

| token | this audit | |
|---|---|---|
| `steps` | `38/38` | ✓ |
| `controls` | `405/405` | ✓ |
| `faults` | `238/238` | ✓ |
| `goldens` | `29` | ✓ |
| `claims` | `29` | ✓ |
| `provisional` | `28` | ✓ |
| `prefix-files` | `11` | ✓ |
| `handoffs` | `8` | ✓ |
| `rights` | `11` | ✓ |
| `pkg-selftest` | `21/21` | ✓ |
| `release-selftest` | `13/13` | ✓ |
| `tests-expected` | `5082` | ✓ |
| `tests-executed` | `5078` (`tests-skipped=4`, `tests-missing=0`) | ✓ |
| `gates` | `122` | ✓ |
| `lines` | `372` | ✓ |
| `docs` | `65` (`doc-files=64`) | ✓ |

Nothing was added to reach these: no gate step, fault, golden, claim, installed file, handoff row or test framework.
The phase's committed ids, counted from the two inventories, are **19**: thirteen `runner-process` inspection ids
(`data/expected/tests/runner-process.ids:280-292`) and six `asm` ids (`data/expected/tests/asm.ids:33-38`). The
dispatch's "16 committed ids" does not recompute from the tree; the inventories do.

---

## Wave 0 Requirements

Every reference is present on the tree — checked item by item, not asserted:

- [x] `docs/inspection.md` — wave 1, alone, with the ancestry proof (W5), verified by **V1**. Its `## Refusal tokens`
      block is what `inspect.surface` reads (both directions, five tokens each side), and its `## Command surface`
      pins `--help`, which `inspect.surface` tests
- [x] `tools/inspect/` + `tools/inspect/CMakeLists.txt` — wave 2, with `inspect_dis.c` joining `IVS_INSPECT_SOURCES`
      in wave 3 (`grep -c inspect_dis.c tools/inspect/CMakeLists.txt` → 1)
- [x] `data/expected/tests/runner-process.ids` — 13 inspection ids (`:280-292`); `data/expected/tests/asm.ids` —
      6 `asm.dis.*` / `asm.judge.dis-*` ids (`:33-38`)
- [x] `data/rights/inspection-output.json`, `data/expected/rights.ids` (`:12` `inspection-output`), `docs/RIGHTS.md`
      row (`:46`) — one commit (`5ed5610`)
- [x] `tools/check/binary-ext.txt` (`:49` `ivst`, with its scope at `:15-17`), `.gitignore` (`:25` `*.ivst`)
- [x] `tools/check/controls/files-unlisted-binary-ivst/` (`CHECK files`, `EXPECT FAIL files-unlisted-binary:
      docs/run.ivst`, `MAKE write docs/run.ivst x`) and its line in `data/expected/controls/rights.ids` — the
      phase's **one** control
- [x] `tools/check/integrity.cmake` — `inspect` joins the tools alternation, so SPDX/ASCII coverage is not silently
      lost
- [x] `data/expected/pkg-selftests.ids` (`:38` `inspect-absent`) and `data/expected/release-assets-selftests.ids`
      (`:27` `inspect-unlisted-member`) — one case each
- [x] `data/expected/important-gates.ids` — four `runner-process mutant` rows, one `files control` row and one
      `asm mutant` row, six in total (`gates=122 lines=372`)

### Recorded deviation from `18-CONTEXT.md`'s control count

`18-CONTEXT.md`'s specificity 3 prices `controls` at **+1** and its specificity 1 parenthetical names both a
`files-unlisted-*` control and a `c-build` unlisted-source control. The plans ship **one** control,
`files-unlisted-binary-ivst`, and this file records the reading rather than leaving the difference implicit:

- `GUARDRAILS-v0.4` **W3** names a `files-unlisted-*` control for a trace artifact explicitly, and `18-02` adds it.
- The `c-build` unlisted-source limb is already covered by four existing per-tool controls, so a fifth would re-prove
  an existing mechanism for no new evidence (the same reasoning `tools/check/binary-ext.txt` records for `dll` and
  `bundle`).
- The result is `controls` **+1**, which is the number `18-CONTEXT.md` prices. `steps` stays 38 and `faults` stays
  238; the toy source list and the module line are edited in place, not added.

---

## Manual-Only Verifications

| Behavior | Requirement | Why Manual | Test Instructions |
|-------------|-------------|------------|-------------------|
| none | — | — | Every behavior this phase produces is judged by a committed id, a control, an in-step selftest or the gate's own token line. The one judgement a tool cannot make — whether the frozen document states the right contract — was made in the discuss record and is not re-asked. |

*No `data/expected/handoffs.ids` row is added: this project forbids a handoff row for something a tool can check, and
nothing in this phase qualifies.*

---

## Outstanding evidence (recorded as `not run`, never as a pass)

| Item | State | Measured reason | What would discharge it |
|------|-------|-----------------|-------------------------|
| `sh tools/gate-linux.sh` (the container gate, `GUARDRAILS-v0.4` §4) | **not run** | measured on this host, not assumed: `timeout 20 docker info` → **rc 124**, the client prints its own warnings and the `Server:` section never arrives; `timeout 120 sh tools/gate-linux.sh` → **rc 124** with `build/gate-linux.log` at **0 bytes**, so no image, container or volume was created and no `ivs-*` object exists to clean up; the hung `docker info --format` and `docker image ls` clients were terminated and no docker client is left running | one container gate on a host whose daemon answers, ending `gate: PASS` in `build/gate-linux.log` |

This is **outstanding evidence, not a pass**. It is a local pre-push helper and not a gate step (`tools/gate-linux.sh:3`),
so its absence weakens no id this phase added: every id above ran under the local gate on this host, and both suites
were re-run directly (`asm` 56/56 under both presets, `runner-process` 161 executed / 0 failed). `18-VERIFICATION.md`
and `18-SECURITY.md` (debt **D9**) record the same item with the same disposition.

---

## Validation Sign-Off

- [x] All tasks have `<automated>` verify or Wave 0 dependencies — five tasks, five automated commands (`V1`–`V5`),
      each run during this audit
- [x] Sampling continuity: no 3 consecutive tasks without automated verify — none of the five lacks one
- [x] Wave 0 covers all MISSING references — every Wave 0 item is present on the tree, checked item by item above
- [x] No watch-mode flags — none in either suite or in the gate
- [x] Feedback latency < 300s — measured 109 s (`sh`) and 103 s (`dash`)
- [x] `nyquist_compliant: true` set in frontmatter

**Approval:** complete — audited 2026-11-27 by the Nyquist auditor. Every row green because its command was run now;
one item handed off as outstanding evidence (the container gate) and recorded as `not run`, never as a pass.

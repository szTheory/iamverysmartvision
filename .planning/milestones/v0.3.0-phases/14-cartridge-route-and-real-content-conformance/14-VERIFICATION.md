---
phase: 14-cartridge-route-and-real-content-conformance
verified: 2026-11-26T04:34:23Z
status: passed
score: 11/11 must-haves verified
covered_files: [".planning/phases/14-cartridge-route-and-real-content-conformance/14-01-PLAN.md", ".planning/phases/14-cartridge-route-and-real-content-conformance/14-01-SUMMARY.md", ".planning/phases/14-cartridge-route-and-real-content-conformance/14-02-PLAN.md", ".planning/phases/14-cartridge-route-and-real-content-conformance/14-02-SUMMARY.md", ".planning/phases/14-cartridge-route-and-real-content-conformance/14-03-PLAN.md", ".planning/phases/14-cartridge-route-and-real-content-conformance/14-03-SUMMARY.md", ".planning/phases/14-cartridge-route-and-real-content-conformance/14-04-PLAN.md", ".planning/phases/14-cartridge-route-and-real-content-conformance/14-04-SUMMARY.md", "CMakeLists.txt", "core/src/cpu/cp1610.c", "data/cp1610/vector-review.json", "data/cp1610/vectors/interrupt.json", "data/discrepancies.json", "data/expected/controls/runner-hygiene.ids", "data/expected/diag-mutations-scn.ids", "data/expected/flat-refusals.ids", "data/expected/important-gates.ids", "data/expected/ivsn-literals.ids", "data/expected/tests/pack.ids", "data/expected/tests/runner-process.ids", "data/profiles/intv-2609-ntsc-family-p1.json", "docs/RIGHTS.md", "docs/cp1610/second-reader-policies.md", "docs/cp1610/stepper-conventions.md", "docs/diagnostics.md", "docs/formats/ivsn-v1.md", "docs/owner-images.md", "docs/packaging.md", "docs/state/hard-points.md", "docs/stic/timing-notes.md", "tests/CMakeLists.txt", "tests/pack-tap.sh", "tests/run/run_stub2.c", "tests/runner-process-tap.sh", "tests/stic/stic_irq.c", "tools/check/controls/_good/tree/.planning/REQUIREMENTS.md", "tools/check/controls/_good/tree/.planning/milestones/v0.1.0-REQUIREMENTS.md", "tools/check/controls/_good/tree/CMakeLists.txt", "tools/check/controls/_good/tree/data/expected/important-gates.ids", "tools/check/controls/c-build-unlisted-flatwrap-source/CHECK", "tools/check/controls/c-build-unlisted-flatwrap-source/EXPECT", "tools/check/controls/c-build-unlisted-flatwrap-source/overlay/tools/flatwrap/extra.c", "tools/check/controls/important-gates-bad-req-phase5/overlay/data/expected/important-gates.ids", "tools/check/controls/important-gates-bad-req-phase6/overlay/data/expected/important-gates.ids", "tools/check/controls/important-gates-complete-uncovered/overlay/.planning/REQUIREMENTS.md", "tools/check/controls/important-gates-req-uncovered-phase5/overlay/data/expected/important-gates.ids", "tools/check/controls/important-gates-req-uncovered-phase6/overlay/data/expected/important-gates.ids", "tools/check/controls/important-gates-req-uncovered-phase7/overlay/data/expected/important-gates.ids", "tools/check/controls/important-gates-uncovered-gate/overlay/data/expected/important-gates.ids", "tools/check/controls/important-gates-unknown-id-phase5/overlay/data/expected/important-gates.ids", "tools/check/controls/important-gates-unknown-id-phase6/overlay/data/expected/important-gates.ids", "tools/check/controls/important-gates-unknown-id/overlay/data/expected/important-gates.ids", "tools/check/integrity.cmake", "tools/check/steps/c-build.sh", "tools/check/steps/important-gates.sh", "tools/flatwrap/CMakeLists.txt", "tools/flatwrap/flatwrap.c", "tools/owner-run.sh"]
covered_digest: "v4:sha256:0fbca08e5679db37ec78be15c5bf2d359254b2aefad776b7d70c76837cb7578e"
behavior_unverified: 0
overrides_applied: 0
re_verification:
  previous_status: passed
  previous_score: 11/11 must-haves verified
  gaps_closed: []
  gaps_remaining: []
  regressions: []
---

# Phase 14: Cartridge Route and Real-Content Conformance Verification Report

**Phase Goal:** A person can wrap a flat cartridge image of their own, at an address they state, into
the project's cartridge container and run it through the owner-local route — with nothing parsed,
nothing described from another implementation and nothing committed — and the first blocker that run
exposes is fixed, pinned and retargeted rather than recorded and abandoned.
**Verified:** 2026-11-26T04:34:23Z at `55f26b5`
**Status:** passed
**Re-verification:** Yes — re-derived at full scope. The previous report (2026-11-26T04:18:15Z, at
`4ee92f8`) carried **no `gaps:` section**, so there was no gap-closure contract to narrow against: the
must-haves were re-established from the ROADMAP success criteria and the four plans' frontmatter and
every limb was re-checked, not spot-checked.

Everything below was re-derived on this tree in this session. No figure is quoted from a SUMMARY, a
VALIDATION, a SECURITY or the previous verification. Where a record and the tree disagree, the tree
wins and the disagreement is named.

**What changed since the previous report, and why it was re-run.** One commit, `55f26b5`, added the two
`--word-order` cases the previous report raised as its only WARNING, in two covered files
(`tests/pack-tap.sh`, `data/expected/tests/pack.ids`). That commit is the whole delta: `git diff --stat
4ee92f8..HEAD` is 99 insertions and 1 deletion across those two files, and `git diff --name-only
4ee92f8..HEAD -- '*.c' '*.h' 'CMakeLists.txt' '*.cmake'` is empty — no compiled file, no build file and
no gate step moved. The digest therefore went stale without any behavioural surface changing; the
re-derivation below was still done at full scope, and the previous pass's five INFO rows were each
re-checked against the tree rather than carried forward on trust.

## Goal Achievement

### Observable Truths

| # | Truth | Status | Evidence |
|---|-------|--------|----------|
| 1 | `tools/flatwrap` is build-only: no install rule, absent from `data/expected/pkg-install-files.ids`, named in `docs/packaging.md`'s "not installed" sentences, and a tracked `tools/flatwrap/*.c` the build does not compile is refused by a committed control | ✓ VERIFIED | `tools/flatwrap/CMakeLists.txt` has `add_executable` and no `install(`; `data/expected/pkg-install-files.ids` is 11 non-comment paths with no flatwrap, and the gate prints `prefix-files=11`; `docs/packaging.md:38` and `:121` both name `ivs_flatwrap` as build-only and not installed; control `c-build-unlisted-flatwrap-source` (`EXPECT` → `FAIL c-build-unlisted-source: tools/flatwrap/extra.c`) is inventoried at `data/expected/controls/runner-hygiene.ids:21` and runs inside `controls=402/402`; `tools/check/steps/c-build.sh` reads the written `ivs-flatwrap-sources.txt` |
| 2 | The tool states only a byte length, a word order and an address the operator gave it; it names no other tool's format and auto-detects nothing | ✓ VERIFIED | `flatwrap.c:21` publishes `Usage: ivs_flatwrap <flat> <base> <out> [--word-order hi-first\|lo-first]`; the only inputs are `argv[1..3]` plus that option (`:259-287`); no extension is inspected; `grep -rniE 'cfg\|\.bin\|rom\b\|jzintv\|mame\|intellicart' tools/flatwrap/` returns four hits, all substrings of ordinary prose ("from", "buffer", "byte count") and none a format name; success prints one 64-hex line only |
| 3 | Every malformed input is refused by a typed token pinned in `data/expected/flat-refusals.ids`, one case per token asserting the refusal, each beside a boundary positive | ✓ VERIFIED (one recorded deviation, below) | 8 pinned tokens, 8 `pack.flatwrap.neg.<token>` cases and 2 judges, **26/26 `ok`** live in this session (`ctest --preset dev -R '^pack$' -V`, `executed=26 skipped=0 failed=0`); `pack.judge.flatwrap-tokens` closes the three-way set (pin = source literals = observed) and went red in a mutation when a token stopped being observed. **Deviation:** the `flatwrap-words` pair is a token pair, not refuse→accept — measured live this session, 65,537 words → `flatwrap-words`, 65,536 words → `flatwrap-cross`; see "Criterion 2's word-count pair" |
| 4 | A wrapped image round-trips: it loads as `content cart user`, `slot.cart.sha256` equals the digest printed on stdout, and flipping one input byte moves both | ✓ VERIFIED | `runner-process.judge.flatwrap-roundtrip` `ok` live in this session; it asserts `slot.cart.loaded 1`, `slot.cart.status ok`, `slot.cart.length_bytes` == `wc -c` of the container, `slot.cart.sha256` == the printed digest, then flips input byte 1 and requires both the printed digest and the second report's value to differ; `exec: -` asserted in the same report |
| 5 | The owner-local route takes a cartridge as an **option** (`--cart <file>`), never as a third positional, and the lock keeps exactly two roles | ✓ VERIFIED | `tools/owner-run.sh:33-42` parses options then `[ "$#" -eq 2 ] \|\| die "$usage"`, so any third positional is a usage refusal; `--cart` with no value is a usage refusal; the lock loop accepts only `exec`/`grom` and a `cart` line is a bad line (`runner-process.owner-run.bad-lock-line` `ok`); `runner-process.owner-run.usage` `ok`; the pass case asserts `grep -c . refs/owner-images.sha256` == 2 |
| 6 | All six derived-output refusals fire for a scenario with `content cart user`, one stderr line each and no file, and the snapshot rule is the **absence** of a seventh refusal | ✓ VERIFIED | `runner-process.cart-user.refuse-derived` `ok` live: `--frame-ppm`, `--video-raw`, `--audio-raw`, `--psg-log-out`, `--audio-wav`, `--update-golden` each exit 14, stderr `cmp`-equal to the one expected line, no report, no file; the three-audio case names `--audio-raw` and leaves none of the three; `--roundtrip-at-ticks` is accepted, writes nothing, and the report is body-equal to the plain run apart from `host.calls` (with a `>= 2` guard that the hook actually cut) |
| 7 | The route's pass path is exercised on project-made files only, with `exec: -`, and a one-byte EXEC mutation proves the id can fail | ✓ VERIFIED | `runner-process.judge.owner-run-cart-pass` `ok` live: zero-filled 8192/2048 images plus a container the wrap tool wrote from a zero-filled flat file, `slot.exec.loaded 1`, `slot.cart.loaded 1`, `slot.cart.sha256` == printed digest, `exec: -`, `outcome guest-fail`, `exit.code 10`; then one byte of the EXEC flipped after `--record` → exit 14 with `load-refused role=exec status=IVS_E_DIGEST_MISMATCH` and `cpu.run_state unreset` |
| 8 | `docs/owner-images.md` no longer says the route has no cartridge role, and `docs/RIGHTS.md`'s flat-cartridge row names the derived material | ✓ VERIFIED | the sentence `The route has no cartridge role…` is gone; `docs/owner-images.md:3,19` name the optional cartridge, `:32-38` is a "### The cartridge role" section, `:57-58` carries the `--cart` form and the "not a third positional" sentence; `docs/RIGHTS.md:37` — `owner-local-only`, "the derived `IVSC v1` container the project's own wrap tool writes is written outside the work tree, and it is never committed, never a fixture and never a golden" |
| 9 | No tracked file carries a new binary extension, `tools/check/binary-ext.txt` and `.gitignore` are unchanged by this phase, and the gate ends `steps=38/38` with the predicted `tests-expected`/`tests-skipped` delta | ✓ VERIFIED (one recorded deviation, below) | `git diff --stat 0825c0e..HEAD -- tools/check/binary-ext.txt .gitignore` is empty; `git ls-files \| grep -E '\.(ivsc\|bin\|rom\|so\|dylib\|cfg\|ivsn)$'` is empty; every file this phase added is `.md`, `.c`, `.txt`, `.ids` or a control `CHECK`/`EXPECT`; gate `steps=38/38 faults=238/238 goldens=29 controls=402/402 tests-skipped=4 tests-missing=0`. **Deviation:** `tests-expected` is 5020, not the predicted 5014 — two legitimate review/test rounds added three case ids after the prediction was taken; chased below |
| 10 | The interrupt-enable default is resolved from the owner-local observation, a project-authored CPU test pins **both** settings, the value stays `unknown-default-chosen` with the observation in C41, and the profile parameter digest, every snapshot literal and the boot-stub-2 interrupt expectations move in one commit | ✓ VERIFIED | `data/profiles/…p1.json:174-182` — `value` 1, `tag` `unknown-default-chosen`, `conflict` `C41`, `decision` null, cite naming `SRC-gi-cp1600-um-1975` pdf 22 and pdf 56 as the pages that do *not* say it; both settings pinned **explicitly and behaviourally**: 7 vectors carry a `config` key, including `cpu.UNIT-INT.accept-clears-intff-config` (`{…: 1}`, `expect.intff 0`) and `cpu.UNIT-INT.accept-keeps-intff` (`{…: 0}`, no `intff` in `expect`), and the harness sets `config_mask` bit 0 exactly when the key is present; plus `stic.irq.level-held-reentry-c41-0` (machine at profile 1 held to one entry **and** the rig stated as 0 to the closed-form storm, in one case) and `stic.irq.c41-both-values`; five re-pinned literals present (`params-digest-sha256 efba8500…f1425c`, `stepped-snapshot 95dd98cb…`, three `cut-snapshot` values); `ivs_cp1610_config_default` is 1; C41 `open`, amended, with an inline pointer at the sentence the flip made wrong; all in `f795f2a` |
| 11 | The next blocker the same run exposes — the display-enable strobe never latches — is recorded with its own row and its own gate | ✓ VERIFIED | `data/discrepancies.json` C82 is `open`, its note records the observation ("the display-enable strobe written at `$0020` never latched"), names `D10-VID-08` and leaves `default_policy` unchanged; `data/expected/d10-measurements.ids:46` is `D10-VID-08 \| C82 \| display-latch persistence and strobe semantics, with strobes at swept offsets`; `docs/stic/d10-procedure.md:38` is the procedure; `.planning/STATE.md:646` is the Deferred row naming the gate and saying it is not fixed here — which is exactly what criterion 7 asks |

**Score:** 11/11 truths verified (0 present, behavior-unverified)

No truth is a stub, no artifact is orphaned, and every behavioural claim above was re-observed by
running the case in this session — `ctest --preset dev -R '^(pack|runner-process)$'`, exit 0, all 26
pack ids and all 138 runner-process ids `ok`, zero `not ok` — not read from a SUMMARY.

### The WARNING from the previous report is closed, and closed with evidence

The previous report's only WARNING was that `--word-order` was implemented but exercised by no case
(`grep -rn 'lo-first\|word-order' tests/` was empty). Commit `55f26b5` closes it with two ids:
`pack.flatwrap.word-order` and `pack.flatwrap.usage-word-order`, both `ok` live. The executor claimed
four mutations were shown red and reverted; **this verifier re-ran all four**, each against
`tools/flatwrap/flatwrap.c`, each restored with `git checkout` and confirmed byte-identical by `cmp`
against a pre-mutation copy, and each reproduced the claimed message exactly:

| Mutation | Case | Observed result | Executor's claim |
|---|---|---|---|
| `int lo_first = 1;` (default flipped to lo-first) | 25 | ✗ red — "the no-flag digest 2c638d6e… is not the explicit hi-first digest 6e79a717…, so hi-first is not the default" (plus collateral reds 16 and 23, below) | matches |
| invert the pairing ternary (`? a : b` / `? b : a`) | 25 | ✗ red — "the hi-first container word is 0201, expected 0102 from the input bytes 01 02" | matches |
| drop the swap (`hi = a; lo = b;` + `(void)lo_first;`) | 25 | ✗ red — "judge-vacuous: lo-first printed the hi-first digest 6e79a717…, so the two orders are one order", and it is the **only** failure | matches |
| accept any `--word-order` value | 26 | ✗ red — "an unknown value exited 0 and a missing value 2, expected 2 and 2", and it is the **only** failure | matches |

The two collateral reds under the default-flip mutation are not noise, they are the default's other
witnesses: `pack.flatwrap.neg.flatwrap-word-range` goes red because its asymmetric input reads as a
different word under lo-first, and `pack.judge.flatwrap-tokens` then goes red because
`flatwrap-word-range` is no longer observed by any case. A default that drifts is caught three ways.

The case is not vacuous in the other direction either: it asserts the no-flag digest **equals** the
explicit `hi-first` digest, the two orders' digests **differ**, the container word is `0102` for
`hi-first` and the byte-swap of it for `lo-first`, and the segment entry does not move — and the
bytes (`01 02`) were chosen so both orders are legal words ≤ `03FF`, so the pairing is isolated from
the word-range rule rather than passing for the wrong reason. `pack.flatwrap.usage-word-order` asserts
exit 2, nothing on stdout, exactly one stderr line, that line `grep -qxF`-equal to the tool's own
usage line (which carries no token), and no output file — so no ninth token was minted and
`data/expected/flat-refusals.ids` is untouched by the commit.

### Criterion 2's word-count pair — judged again, on a fresh measurement

Re-measured in this session against the built tool, not read:

```
65537 words (131074 bytes) at 4000 -> exit 1  ivs_flatwrap: flatwrap-words: the input is 131074 bytes, more than 65536 words
65536 words (131072 bytes) at 4000 -> exit 1  ivs_flatwrap: flatwrap-cross: 65536 words at 4000 end above 10000
49152 words (98304 bytes)  at 4000 -> exit 0  98368-byte container, entry 4000 c000
    0 bytes                       -> exit 1  ivs_flatwrap: flatwrap-empty: the input is empty, and a container segment holds at least one word
    3 bytes                       -> exit 1  ivs_flatwrap: flatwrap-length: the input is 3 bytes, and a word is two bytes
```

The delivered tool refuses 65,536 words with the **crossing** token, not the word-count token, and the
arithmetic is exact and is the reason: rule 4's floor is `base < 0x4000u` (`flatwrap.c:335`), rule 5
refuses `base + n_words > IVS_IVSC_ADDRESS_SPACE_WORDS` (`:339`), so the largest accepted count at the
lowest legal base is `0x10000 - 0x4000 = 49,152` words — which the tool accepts and which the entry
read back confirms (`4000 c000`, ending at `$FFFF`). There is no base at which 65,536 words is legal,
and a one-segment writer cannot encode it in a 16-bit `len_words` anyway
(`IVS_IVSC_MAX_SEGMENT_WORDS` is 65,535). The criterion's literal "65,537↔65,536 words" refuse→accept
pair is unsatisfiable.

**Does the pair still deliver what the criterion is for? Yes.** The criterion is for: every malformed
input refused by a typed token, each token beside a positive. Verified limb by limb:

- **Every malformed input has a typed token.** 8 tokens, and `pack.judge.flatwrap-tokens` proves the
  pin, the source's printable literals and the observed set are the same three-way set — a token the
  source could print but the pin lacks, or a pin with no case, fails the gate. That judge is not
  decorative: it went red in this session's default-flip mutation precisely because one token stopped
  being observed.
- **Each token has a case that asserts the refusal** (exit 1, exactly one `ivs_flatwrap: <token>: …`
  line, nothing on stdout, no output file): all 8 `ok`.
- **Each has a positive beside it:** 7 of 8 are refuse→accept pairs (empty→1 word/66 bytes,
  odd→even, `$0400`→`$03FF` read back from the container, base `$3FFF`→`$4000`, 49,153→49,152 words
  with entry `4000 C000` ending at `$FFFF`, in-tree→outside, existing→new). The words token's pair is
  a second typed refusal, and that is *stronger* than a positive here: 65,537→`words` and
  65,536→`cross` together pin the count rule's threshold at exactly 65,536 and prove the writer never
  emits a wrapped or zero `len_words`.
- **The real acceptance boundary is not missing:** it is `flatwrap-cross`'s positive — 49,152 words
  accepted, 98,368 bytes, entry read back as `4000 C000`, re-measured above.

The deviation is recorded in three places on the tree (the `flatwrap-words` case comment in
`tests/pack-tap.sh`, the tool's check-order header, and `14-01-SUMMARY.md`), with the arithmetic.

**Seven versus eight.** The pin grew append-only from seven to eight because the phase's own code
review measured a real defect (CR-01): a zero-byte input passed all seven rules and produced a 64-byte
container the project's own loader refuses (`IVS_IVSC_MIN_BYTES` 66, `len_words` ≥ 1). The
re-measurement above shows the new rule and its boundary positive both live (0 bytes refused, 2 bytes
accepted). Adding a refusal is a superset of "seven typed refusals", it is appended rather than
substituted, and `data/expected/flat-refusals.ids` still carries exactly 8 token lines.

### Criterion 6's both-settings pin — judged again, on a fresh parse

The plan's specified re-expression of `run.stub2.jd-as-j` (make the J half take more entries than the
JD half at the new profile value) is impossible: with acceptance clearing the enable, the vector
always executes with the enable clear, where `J` and `JD` are indistinguishable. It was delivered as
the flip's own negative pin instead — both frames equal at 1 (asserted unit-for-unit in
`tests/run/run_stub2.c`: 1 entry, `vec_r7 5004`, `vec_intr 0`, `vec_r6 0341`, marker `03C5`, counter
`0001`, `R7 500D`, `intr_enabled 0`), and 179-vs-1 at 0 (the pre-flip assertion, replaced at
`f795f2a`; the 179 figure is the commit's recorded measurement, not a live test at 0).

**Are both settings genuinely still exercised? Yes — three ways, and two of them live in the gate.**
Re-parsed from the tree this session:

1. `data/cp1610/vectors/interrupt.json` has 39 vectors, **7** of which carry a `config` key, each
   stating its own value: five at `0` (`intrm-intff-set-accepted`, `EIS.defers-intr`,
   `EIS.then-intrm-after-next`, `J.je-then-intrm-immediate`, `GSWD.intff-unchanged`),
   `cpu.UNIT-INT.accept-keeps-intff` at `0` with no `intff` in `expect`, and
   `cpu.UNIT-INT.accept-clears-intff-config` at `1` with `expect.intff 0`. Neither inherits the
   default: the harness sets `config_mask` bit 0 exactly when the key is present, so a core that
   ignored the field fails one of the two.
2. `stic.irq.level-held-reentry-c41-0` runs **both halves in one case**: the machine (profile 1) gives
   exactly one entry, and the stepper rig with the field stated as `0` gives the closed form of the
   held level. It fails if either half stops holding.
3. `stic.irq.c41-both-values` crosses both values with a DIS-first and a not-DIS-first routine.

**Is the tag still `unknown-default-chosen`? Yes.** `data/profiles/intv-2609-ntsc-family-p1.json:176`
— `"tag": "unknown-default-chosen"`, `"conflict": "C41"`, `"decision": null`, cite naming
`SRC-gi-cp1600-um-1975` pdf 22 and pdf 56 as the pages that do *not* state it, `resolving_evidence`
the D10 probe, `revisit_trigger` unchanged. C41 is still `open` with the hardware probe as its
resolver, its `default_policy` now reads "acceptance clears INTFF … = 1", and `claims=28
provisional=27` is unmoved, so the resolved value minted no claim. That is the point of the tag: an
owner-local observation is not a hardware probe.

### Criterion 9's predicted delta — the tokens that do not read as predicted

`14-04-PLAN.md` predicted `tests-expected=5014` (+24 over the phase start of 4990). The tree at `HEAD`
measures **5020**. The +6 is three case ids, each counted once per preset, each added after the
prediction was taken:

| Case | Commit | Why it was not in the prediction |
|---|---|---|
| `pack.flatwrap.neg.flatwrap-empty` (+2) | `bf3215d` | the phase's own code review measured CR-01: a zero-byte input produced a container the core refuses |
| `pack.flatwrap.word-order` (+2) | `55f26b5` | closes the previous verification's one WARNING: the tool's `--word-order` had no case |
| `pack.flatwrap.usage-word-order` (+2) | `55f26b5` | the other side of the same parser: a usage error is not a refusal, and no ninth token is minted |

`tests-skipped=4` (2 operator-content ids × 2 presets) and `tests-missing=0` are exactly as predicted,
and `steps=38/38`, `faults=238/238`, `goldens=29` and `controls=402/402` — the invariant limbs of the
criterion — hold exactly. No fault, golden, step or control was added or dropped. The plan's predicted
token is explicitly not re-blessed: `14-04-SUMMARY.md`'s amendment and `14-REVIEW.md` record the
earlier two-token drift, and this report records the third.

### Required Artifacts

| Artifact | Expected | Status | Details |
|---|---|---|---|
| `tools/flatwrap/flatwrap.c` | the wrap tool, states bytes and detects nothing | ✓ VERIFIED | 381 lines at the fix head; reads only its own container layout through `content/ivsc.h` and `common/ivs_sha256.h`; restates the 8 magic bytes rather than exporting a core symbol (defined/public symbol sets unmoved: `public-symbols=19 defined=141`); untouched by `55f26b5` |
| `tools/flatwrap/CMakeLists.txt` | `ivs_flatwrap`, no install rule, source list written for `c-build` | ✓ VERIFIED | `add_executable` + `ivs-flatwrap-sources.txt` writer; no `install(` |
| `data/expected/flat-refusals.ids` | the pinned typed surface, append-only | ✓ VERIFIED | 8 token lines + comment block; grew only by appending `flatwrap-empty`; untouched by `55f26b5` |
| `data/expected/important-gates.ids` | 5 rows, 3 gate names, under TOOL-07/SCN-03/CPU-06 | ✓ VERIFIED | `flatwrap control …:549`, `flatwrap mutant …:552`, `cart-role mutant …:555`, `cart-role mutant …:561`, `cpu06 fault ACCEPT_CLEARS_INTFF_IGNORED:565`; step prints `gates=117 lines=356` |
| `tools/check/controls/c-build-unlisted-flatwrap-source/` | `CHECK`, `EXPECT`, overlay `extra.c`, inventoried | ✓ VERIFIED | `EXPECT` = `FAIL c-build-unlisted-source: tools/flatwrap/extra.c`; inventoried at `runner-hygiene.ids:21`; inside `controls=402/402` |
| `data/profiles/intv-2609-ntsc-family-p1.json` | value 1, tag unchanged, observation cited | ✓ VERIFIED | `:174-182`, parsed this session |
| `data/expected/ivsn-literals.ids` | 5 re-pinned values | ✓ VERIFIED | `params-digest-sha256 efba8500…`, `stepped-snapshot-sha256 95dd98cb…`, three `cut-snapshot-sha256.*`; the `state` suite is green in the gate |
| `data/cp1610/vectors/interrupt.json` | explicit overrides for both settings | ✓ VERIFIED | 39 vectors, 7 with `config`; the both-settings pair is `accept-keeps-intff` (0) / `accept-clears-intff-config` (1) |
| `data/discrepancies.json` | C41 amended, C82 note appended | ✓ VERIFIED | C41 `open` with the observation and an inline pointer; C82 `open`, note names `D10-VID-08`; `discrepancies=110 open=101` unchanged |
| `tests/pack-tap.sh` | 8 token cases + 2 judges + boundary positives + the two option cases | ✓ VERIFIED | 26 pack ids, all `ok` live; the two new cases each shown red under two independent mutations |
| `data/expected/tests/pack.ids` | 26 ids | ✓ VERIFIED | 26 non-comment lines, matching the 26 cases the suite executed |
| `tests/runner-process-tap.sh` | 3 new cases, sixth argument | ✓ VERIFIED | `cart-user.refuse-derived`, `judge.flatwrap-roundtrip`, `judge.owner-run-cart-pass` all `ok`; a missing sixth argument is a finding, not a skip |
| `data/expected/tests/runner-process.ids` | 138 ids | ✓ VERIFIED | 138 non-comment lines; suite prints `executed=136 skipped=2 failed=0` (the 2 skips are the operator-content scenarios) |
| `tools/owner-run.sh` | `--cart` option, conditional template insertion | ✓ VERIFIED | option parsing, `[ "$#" -eq 2 ]`, `want` count asserted against the arguments |
| `docs/owner-images.md`, `docs/RIGHTS.md` | the cartridge role; the derived-material disposition | ✓ VERIFIED | `owner-images.md:3,19,32-38,57-58,71,75,91`; `RIGHTS.md:37` |
| 13 toy files | 39 added lines, zero deletions | ✓ VERIFIED | `git show --numstat 548e80d` over the 13 paths sums 39/0; `control-toy-drift` and `control-deviation-unused` both green in `controls=402/402` |
| `14-VALIDATION.md` | signed off | ✓ VERIFIED | `status: validated`, `nyquist_compliant: true`, `wave_0_complete: true`, nine per-task rows, ten ticked Wave 0 bullets, Approval line carrying the measured gate line, plus the appended amendment that corrects the three tokens the review moved |

### Key Link Verification

| From | To | Via | Status | Details |
|---|---|---|---|---|
| `tools/flatwrap/flatwrap.c` | `core/src/content/ivsc.h` | the prefix/entry/trailer/word-max/address-space constants | ✓ WIRED | included at `:79`; linked through `ivs_core_internal`, the target `tools/pack` already links |
| `tools/flatwrap/flatwrap.c` | `core/src/common/ivs_sha256.h` | the trailer digest and the printed whole-file digest | ✓ WIRED | `ivs_sha256_digest` in `fw_write`; `pack.judge.flatwrap-digest-live` checks both against `cmake -E sha256sum` (an implementation other than the core's) and shows one flipped byte moves both |
| `data/expected/flat-refusals.ids` | `tests/pack-tap.sh` | the token loop and the three-way coverage judge | ✓ WIRED | `pack.judge.flatwrap-tokens` `ok`, and red in a mutation when a token stopped being observed |
| `c-build-unlisted-flatwrap-source/EXPECT` | `data/expected/controls/runner-hygiene.ids` | a control resolves only when inventoried | ✓ WIRED | inventoried `:21`; counted inside `controls=402/402` |
| `tools/owner-run.sh` | `data/scenarios/owner-exec-title.ivss` | the marker line the `awk` step inserts after | ✓ WIRED | `awk` inserts the two `expect-sha256` lines, plus `content cart user` only when `--cart` is given; the count is asserted so the switch cannot silently invert; with no cartridge the copy is byte-identical to the pre-plan one |
| `tests/runner-process-tap.sh` | `runner/main.c` | the six derived-output refusals | ✓ WIRED | already role-generic (one loop over `IVS_SCN_ROLES` per output family), so criterion 4's runner half is proof, not new code; `cart-user.refuse-derived` `ok` |
| `runner/main.c` | the report's `slot.cart.sha256` | `--cart <file>` is the existing operator option | ✓ WIRED | the route only forwards a path; the equality is asserted in two cases |
| `data/expected/rights-pinned.ids` | `tools/owner-run.sh`, `docs/owner-images.md` | 4 `<token> <path>` pins | ✓ WIRED | `rights-pinned=4/4` in the gate; a stale pin is a failure, not a note |
| `data/profiles/…p1.json` | `data/expected/ivsn-literals.ids` | the registry scalar → `ivs_mm_profiles[]` → memmap body digest → 134-byte preimage → params digest | ✓ WIRED | the `state` suite holds the pin to the core and is green; the phase review rebuilt the preimage independently (T-14-08) |
| `core/src/cpu/cp1610.c` | `data/cp1610/vectors/interrupt.json` | `ivs_cp1610_config_default` is the harness start; `config_mask` bit 0 is set iff the vector carries `config` | ✓ WIRED | both-settings pair `ok` in `cpu-vectors` inside the gate |

### Data-Flow Trace (Level 4)

| Artifact | Data Variable | Source | Produces Real Data | Status |
|---|---|---|---|---|
| `tools/flatwrap` output → `ivs-run` report | `slot.cart.sha256`, `slot.cart.length_bytes` | the operator's flat file, read and paired by the tool, digested over the whole container; the loader re-digests the same bytes | Yes — `runner-process.judge.flatwrap-roundtrip` compares the report's value to the tool's printed value and to the file's own size, and proves it live by moving one input byte | ✓ FLOWING |
| `slot.cart.loaded` / `slot.cart.status` | the loader's verdict on the container | `content/ivsc.c` parse of the bytes the tool wrote | Yes — `loaded 1`, `status ok`, and a byte-flipped container is refused `IVS_E_MALFORMED`/`IVS_E_DIGEST_MISMATCH` | ✓ FLOWING |
| the route's temporary scenario | `content cart user` | the helper's `awk` insertion, conditional on `--cart` | Yes — the run's report shows `slot.cart.loaded 1` only when the option is given, and the count is asserted | ✓ FLOWING |
| the interrupt-enable field | `intff` after acceptance | the profile registry → `ivs_mm_profiles[]` → machine config | Yes — one entry at 1 (profile) and the closed-form storm at 0 (rig), in one case | ✓ FLOWING |
| the word order → the container's first word | `fw_word[0]` | the operator's `--word-order` value → the pairing ternary at `flatwrap.c:369-370` → the container bytes at offset 32 | Yes — read back from three containers this session (`0102`, `0201`, and the default equal to the explicit `hi-first`), and each of the four mutations moved or flattened it | ✓ FLOWING |

No static return, hardcoded literal or mock ends any of these chains.

### Behavioral Spot-Checks

| Behavior | Command | Result | Status |
|---|---|---|---|
| the gate is green under the hosted toolchain | `sh tools/gate.sh` | exit 0, no `^-- FAIL` line, `real 1m16.235s`; line below | ✓ PASS |
| the eight refusals, their pairs, the two judges and the two option cases | `ctest --preset dev -R '^pack$' -V` | `executed=26 skipped=0 failed=0`, all 26 `ok` | ✓ PASS |
| the cartridge role, the round trip and the route pass | `ctest --preset dev -R '^runner-process$' -V` | exit 0, `executed=136 skipped=2 failed=0`, all 138 ids `ok`, zero `not ok` | ✓ PASS |
| the word-order case can fail for its intended reason | 4 mutations of `flatwrap.c`, each rebuilt, run, reverted (`git checkout` + `cmp`) | 4/4 red with the executor's exact messages; case 25 under three mutations, case 26 under one | ✓ PASS |
| criterion 2's word-count pair | the built tool on 65,537 / 65,536 / 49,152 / 0 / 3-word inputs | `flatwrap-words` / `flatwrap-cross` / accepted 98,368 bytes entry `4000 c000` / `flatwrap-empty` / `flatwrap-length` | ✓ PASS |
| both settings are pinned explicitly | parse `data/cp1610/vectors/interrupt.json` | 39 vectors, 7 with `config`, the 0/1 pair present with the stated `expect` | ✓ PASS |
| no gate step, fault, golden or control was added or dropped | `ls tools/check/steps/*.sh \| wc -l`; `ls tools/check/controls \| grep -v '^_' \| wc -l` | 38 steps; 402 control directories, matching `steps=38/38 controls=402/402` | ✓ PASS |
| the derived output is not tracked | `git ls-files \| grep -E '\.(ivsc\|bin\|rom\|so\|dylib\|cfg\|ivsn)$'` | empty; `.gitignore:16` is `*.ivsc`, `tools/check/binary-ext.txt:5` is `ivsc` (refusal surface) | ✓ PASS |
| the phase's binary surfaces are untouched | `git diff --stat 0825c0e..HEAD -- tools/check/binary-ext.txt .gitignore` | empty | ✓ PASS |
| no debt marker in a phase-modified file | `grep -n -E 'TBD\|FIXME\|XXX'` over every non-`.planning` file the phase touched | no `TBD`/`FIXME`; the `XXX` hits are `mktemp …XXXXXX` templates and the prose `DECLE $XXXX` | ✓ PASS |
| the cost medians, recomputed | arithmetic over `.planning/ci-runs.tsv` (both the committed file and the worktree) | `cpu_s` median 363 committed / 366 with the two newer rows; `build_s` 35/35; `gate_s` 121/120 | ✓ PASS |
| the tree was left as it was found | `git status --porcelain`; `git diff --stat` | the two pre-existing worktree modifications only; no source file modified; `flatwrap.c` byte-identical to `HEAD` | ✓ PASS |

The gate line measured in this session (`sh tools/gate.sh`, exit 0):

```
gate: PASS steps=38/38 controls=402/402 api-calls=19 api-statuses=15 api-rows=61 pinned-defines=120 cc=AppleClang c-presets=2 fault-runs=238 core-imports=7 purity-selftest=7/7 ctest-runs=2 ci-runs=102 ci-uses=2 ci-workflows=2 ci-summary-selftest=11/11 claims=28 provisional=27 cp1610-rows=74 patterns=1024 unclaimed=0 ambiguous=0 coincidences=1 agree=71 resolved=3 vectors=412 source=291 policy=121 rows-pending=0 policies-pending=0 bus-figure-pages=35 blind-files=12 second-read=12/12 scanned=689 waived=0 discrepancies=110 pending-resolvers=20 open=101 d10-items=47 faults=238/238 tracked=2749 binaries-listed=0 rights-pinned=4/4 terms=9 doc-files=63 profile-keys=154 goldens=29 reports=30 host-dependent=3 release-only=2 mode=tree hygiene-files=2749 history-commits=1 gates=117 lines=356 input-rows=31 input-policies=11 input-profile-fields=31 files=2749 listings=7 listing-words=1386 manifests=11 effective-yes=8 effective-no=3 rights=10 rights-cleared=1 literal-rows=80 views=2 dump-lines=160 prefix-files=11 archive=members public-symbols=19 defined=141 hdr-alone=10/10 consumer-lines=164 cxx-link=ok pkg-selftest=19/19 records=154 fields=154 unknown-fields=47 psg-rules=21 psg-vectors=120 psg-policies=12 release-selftest=11/11 runner-files=18 cases=14 sources=13 refs-verified=3/4 state-rows=88 state-cuts=18 tests-expected=5020 tests-executed=5016 tests-skipped=4 tests-missing=0 docs=64 claim-refs=26 stic-rules=67 stic-rows=64 video-frames=26 video-hash-ties=18 video-frame-pairs=9 tags=10 tiers=8
```

Two tokens in that line come from the **worktree**, not from `HEAD`, and are named rather than
quietly quoted: `ci-runs=102` counts two rows appended to `.planning/ci-runs.tsv` in this worktree
(runs `45747709457`, `45751979661`, both real hosted runs of commits `2e27267` and `2c29a8c`) and
`tracked=2749 files=2749 hygiene-files=2749` counts this report file, which was untracked when the gate
ran (`tools/gate.sh:60` defines `FILELIST` as "tracked plus untracked-not-ignored"). At `HEAD` the same
line reads `ci-runs=100` and `tracked=2748`. Every other token — including all four of this phase's
invariant limbs and `gates=117 lines=356` — is a property of the tree, not of the worktree.

**The Ubuntu 24.04 container run was not repeated in this pass, deliberately, and the reason is a
measurement rather than an omission.** The previous pass re-took it at `4ee92f8` (`gate-linux: exit 0`,
`gate: PASS`, four documented host tokens). Since then exactly one commit landed, `55f26b5`, and
`git diff --name-only 4ee92f8..HEAD -- '*.c' '*.h' 'CMakeLists.txt' '*.cmake'` is **empty**: the delta is
two shell test cases and an `.ids` file. No compiled or build input changed, so there is nothing a
second compiler could disagree about. The four documented host differences (`cc`, `core-imports`,
`archive`, `refs-verified`) are unaffected by a shell test file.

### Probe Execution

The phase declares no `scripts/*/tests/probe-*.sh` and mentions no probe, PASS marker or stage marker
in any PLAN or SUMMARY; it is not a migration or CLI-tooling phase in the probe sense
(`find scripts -path '*/tests/probe-*.sh'` is empty, and `grep -R -n -E 'probe-[^[:space:]]+\.sh'
"$PHASE_DIR"/*-PLAN.md "$PHASE_DIR"/*-SUMMARY.md` returns nothing). Its runnable evidence is the two
suites and the gate above, which were run in their own processes.

**Step 7c: not applicable (no declared or conventional probe).**

### Requirements Coverage

| Requirement | Source Plan | Description | Status | Evidence |
|---|---|---|---|---|
| TOOL-07 | 14-01, 14-04 | wrap a flat image at a stated address with one build-only command; every malformed input refused by a typed token with a positive beside it; the wrapped image is derived material, never committed/golden/fixture/claim | ✓ SATISFIED | truths 1-3; 8 tokens + 2 judges + 2 option cases `ok`; `prefix-files=11`, `binaries-listed=0`; registry rows `flatwrap control c-build-unlisted-flatwrap-source` and `flatwrap mutant pack.judge.flatwrap-digest-live` |
| SCN-03 | 14-03, 14-04 | the owner-local route accepts a cartridge alongside EXEC and GROM; every derived-output refusal is extended to it; the pass path is exercised on project-made files without an owner image | ✓ SATISFIED | truths 4-8; 6 refusals + the snapshot rule + the pass path + the mutation, all `ok` live; registry rows `cart-role mutant runner-process.judge.flatwrap-roundtrip` and `…judge.owner-run-cart-pass` |
| CPU-06 | 14-02, 14-04 | the first real-content blocker resolved from an owner-local observation; both settings pinned; tag stays `unknown-default-chosen` with the observation in the log; profile digest, every snapshot literal and the boot-stub-2 expectations move in one commit | ✓ SATISFIED | truth 10; `f795f2a` moves the field, the C default, the 5 literals, the 7 vector `config` keys, the stub-2 re-expression, the STIC C41-0 retarget and the C41 row; registry row `cpu06 fault ACCEPT_CLEARS_INTFF_IGNORED` (a pre-existing fault, not a new one) |

**All three requirement IDs accounted for. No orphaned requirement.** `.planning/REQUIREMENTS.md`
marks all three `- [x]` with `Complete` traceability rows (`:108-110`); all three appear in the
`requirements:` frontmatter of plans 14-01/14-02/14-03 and again in 14-04; `grep -E "Phase 14"
REQUIREMENTS.md` maps exactly TOOL-07, SCN-03 and CPU-06 and no fourth id; each has a live registry row
under a gate name that resolves to an inventory of its declared kind (`important-gates` green in both
directions inside the gate).

### Anti-Patterns Found

The previous report's one WARNING is **closed** (see "The WARNING from the previous report is closed"
above) and is no longer listed. The five INFO rows were re-checked against the tree this pass; each is
still true, and none is a blocker.

| File | Line | Pattern | Severity | Impact |
|---|---|---|---|---|
| `data/expected/flat-refusals.ids` | 9-12 | the comment claims "every token has exactly one case … that asserts the refusal and its boundary positive together" | ℹ️ INFO | not true for `flatwrap-words`, whose case asserts the refusal and a second refusal; `tests/pack-tap.sh`'s own comment states the real shape correctly, so the pin's header over-claims by one token. Same class as UF-14-02 (a record that disagrees with the file it names) |
| `tools/flatwrap/flatwrap.c` | 93, 322 | rule 3's threshold is `FW_WORDS_MAX` = `IVS_IVSC_ADDRESS_SPACE_WORDS` (65,536) rather than the segment maximum (65,535), so a 65,536-word input would encode `len_words` as `0x0000` | ℹ️ INFO | unreachable today: rule 4's floor is `$4000`, so rule 5 refuses every 65,536-word input first — re-measured this session (65,536 → `flatwrap-cross`). The safety is a coupling between two rules rather than a stated bound |
| `tools/flatwrap/flatwrap.c` | 212 vs ~322 | one token, two detail texts (`… already exists` vs the bare path) | ℹ️ INFO | both correct, both `flatwrap-output-exists`; the cases assert the token prefix only, so a change is invisible. Deferred as review IN-01 / risk R-14-A with a reason |
| `tools/owner-run.sh` | 37, 49, 96 | `--cart ""` is silently treated as "no cartridge" | ℹ️ INFO | re-confirmed: the option takes `$2` with no non-empty check and the location loop `[ -n "$p" ] \|\| continue`s past it. The one silent branch of the helper; every other malformed argument is a usage refusal. Deferred as review IN-02 / risk R-14-B |
| `tools/flatwrap/flatwrap.c` | 177-179 | the `refs/` exception to the in-tree rule is not exercised by any committed case | ℹ️ INFO | a regression either way would leave the gate green; the exception is documented as deliberate (D-47) in the tool's header. Deferred as review IN-03 / risk R-14-C |

No `TBD`/`FIXME`/`XXX` debt marker exists in any file this phase touched. No placeholder return, empty
handler, hardcoded empty prop or `console.log`-only body exists in the phase's code. No blocker.

The two new cases were read for vacuity as well as re-run: `pack.flatwrap.word-order` cannot pass on a
tool that ignores the option (the two digests would be equal → `judge-vacuous`), cannot pass on a tool
whose default drifted (the no-flag digest would differ from the explicit `hi-first` one), and cannot
pass on a swapped pairing (the word assertion). `pack.flatwrap.usage-word-order` cannot pass on a tool
that accepts an unknown value (exit 0 ≠ 2) and its expected line is pinned by exact match, so a change
to the usage string is a failure rather than a silent drift.

### Advisory (New Scope, Unevidenced)

None. No new-scope finding was raised on this pass: the five INFO rows are the previous pass's, each
re-checked and each already recorded as deferred with its reason, and nothing in the two new cases or
the ids file produced a concern with no deterministic evidence.

### Human Verification Required

**None.** Every phase behaviour has an automated check that was re-run in this session — including the
two behaviours the previous report flagged as unexercised, which are now cases that this verifier
showed red under four mutations. The tier ceiling is `none` — the wrap and the round trip are
repeatability `[nc]`, and a resolved default is an observation, never a claim — so nothing is handed
off and no row carries `evidence: none`.

One thing this verifier did **not** re-perform, by design: the owner-local observation behind D-51
(the D11 run whose interrupt storm chose `1`). It needs the owner's own EXEC, GROM and cartridge, it is
recorded in C41 with `evidence` naming the route, and re-asking it is exactly what
`.planning/AUTONOMY.md`/`AGENTS.md` forbid. What criterion 6 asks a verifier to check is that the value
was resolved *from* that observation and that the record, the tag and the pins agree — verified above.

### Non-blocking Findings

Recorded so they are not lost, not accepted as fresh work in this phase:

1. **`--word-order` coverage is now closed, and the closure is proved.** The previous report's
   recommendation was "one `pack` case (`lo-first` swap read back from the container) plus the two
   usage refusals"; `55f26b5` delivered exactly that shape as two ids (the swap, the default and the
   entry in one case; both usage errors in the other), with zero new steps, faults, goldens or
   controls, and this verifier re-ran all four of its mutations. Nothing is owed here.
2. **Three INFO rows from the phase's own review are deferred with reasons** (IN-01/IN-02/IN-03 →
   risks R-14-A/B/C), and two auditor observations are recorded as R-14-D (the in-tree rule protects
   the directory, not a final symlink component; closed for reachable shapes by `stat` + `O_EXCL`) and
   R-14-E (the token-coverage judge is a lexical match on the file it guards; currently exact).
3. **Two record figures that were stale at the fix head are closed**: UF-14-01 (the live format page
   still quoted the pre-flip params digest) and UF-14-02 (the signed validation quoted the pre-CR-01
   `pack` id count) — both fixed at `2c29a8c`, re-verified in the tree
   (`docs/formats/ivsn-v1.md:245` carries `efba8500…`; `14-VALIDATION.md` reads 24, and the suite now
   executes 26).
4. **The `reprice` verdict's arithmetic was recomputed twice, and both windows give the same word.**
   From the **committed** `.planning/ci-runs.tsv` (100 rows), the last five `cpu_s` are 357 360 363 438
   368 → sorted 357 360 363 368 438 → median **363**; `build_s` 33 34 35 39 35 → median **35**;
   `gate_s` 121 120 119 135 122 → median **121**. `discharged-within-allowance` needs ≤ 361 + 1.535 =
   362.535, and 363 is 0.465 above it; `lever-required` needs > 21 cumulative (3.470) or > 25 % overrun
   (+18.1 %), neither holds. Including the **two newer rows in this worktree** (`45747709457` cpu 366,
   `45751979661` cpu 359), the last five are 363 438 368 366 359 → median **366**, `build_s` median 35,
   `gate_s` median 120 — and 366 is 3.465 above the allowance, so `reprice` is still the word the rule
   names. The verdict does not depend on which window is used.
5. **The gate line quoted in `14-04-SUMMARY.md`, `14-VALIDATION.md` and `.planning/STATE.md` is the
   line at the close commit, not at `HEAD`.** At `HEAD` four tokens differ from it, each chased: the
   three the amendment already names (`lines=357`→`356` from WR-03's deleted `pass 0` registry row,
   `tests-expected 5014/5010`→`5016/5012` from CR-01, `tracked 2745`→`2747` from the two record files)
   plus `tests-expected 5016/5012`→`5020/5016` from `55f26b5`'s two cases. `14-REVIEW.md` and
   `14-SECURITY.md` also still read `tracked=2747`, now three behind. No invariant moved.
6. **This pass's gate line carries two worktree tokens**, named in the spot-check section:
   `ci-runs=102` (two uncommitted rows in `.planning/ci-runs.tsv`) and `tracked=2749` (this report,
   untracked at measurement; `HEAD` is 2748). Neither is a tree property and neither is quoted as one.

### Gaps Summary

No gaps. Every one of the seven ROADMAP success criteria and every must-have truth is delivered in the
code, wired, and proved by a case that ran in this session. Three places where the delivered tree does
not match a criterion's literal wording are all arithmetic or measurement forced, are recorded on the
tree with their reasons, and are replaced by a stronger proof:

- criterion 2's "65,536 words accepted" cannot exist at any legal base (re-measured: 65,536 words →
  `flatwrap-cross`), and is replaced by a second typed refusal that pins the count threshold at exactly
  65,536 and proves no wrapped or zero `len_words` is ever emitted;
- criterion 9's predicted `tests-expected` is 5014 and measures 5020 because three legitimate case ids
  (one review fix, two coverage cases closing the previous pass's warning) were added after the
  prediction was taken; `steps`, `faults`, `goldens`, `controls` and `tests-skipped` are exact;
- criterion 6's specified re-expression of `run.stub2.jd-as-j` is impossible at the new profile value,
  and the flip's own negative pin plus three explicit witnesses carry the "both settings" requirement
  instead.

The one thing the previous report asked a reader to know — that the tool's stated word-order option was
implemented but never exercised — is no longer true, and the fix is proved rather than asserted: four
mutations, four reds, the executor's exact messages reproduced by this verifier, and the tree restored
byte-identical afterwards.

---

_Verified: 2026-11-26T04:34:23Z_
_Verifier: Claude (gsd-verifier)_

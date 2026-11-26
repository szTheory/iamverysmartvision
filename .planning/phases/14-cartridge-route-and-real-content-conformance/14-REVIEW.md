---
phase: 14-cartridge-route-and-real-content-conformance
reviewed: 2026-11-26T03:22:38Z
depth: standard
files_reviewed: 13
files_reviewed_list:
  - tools/flatwrap/flatwrap.c
  - tools/flatwrap/CMakeLists.txt
  - tools/owner-run.sh
  - runner/main.c
  - core/src/cpu/cp1610.c
  - data/profiles/intv-2609-ntsc-family-p1.json
  - data/expected/flat-refusals.ids
  - data/expected/ivsn-literals.ids
  - data/cp1610/vectors/interrupt.json
  - data/discrepancies.json
  - tools/check/integrity.cmake
  - docs/owner-images.md
  - docs/RIGHTS.md
findings:
  critical: 1
  warning: 3
  info: 3
  total: 7
  necessity: 0
status: issues_found
---

# Phase 14: Code Review Report

**Reviewed:** 2026-11-26T03:22:38Z
**Depth:** standard
**Files Reviewed:** 13
**Status:** issues_found

## Summary

The phase's three deliverables hold up under an adversarial pass: the wrap tool's seven refusals are ordered, typed and each fires before any byte is written; the exclusive create genuinely cannot truncate; the physical-parent compare survives `..`, a symlinked directory and a relative spelling; the 65,536-word pair is refused by the crossing token for the reason the SUMMARY states; the profile flip moved no `config`/`init`/`expect` datum, left the tag alone, and the parameter digest is reproducible by a second reader; the cartridge role is an option (never a third positional), the six derived-output refusals are role-generic, and the `exec:` label stays `-`. The gate re-run on this tree is green with exactly the predicted line (`steps=38/38 … tests-expected=5014 … gates=117 lines=357 tracked=2746`, zero `-- FAIL` lines).

One reachable input breaks the tool's central property, and three records still state things the phase changed. The wrap tool **accepts a zero-byte input and writes a container the project's own parser refuses** (measured below), with no rule and no case covering it. The C41 record still asserts the deleted default in three of its fields, the `important-gates` registry still claims a must-fail mechanism that now pins a pass, and the tool's two path probes block forever on a FIFO — a hazard the project's own `runner/hostio.c` was written to avoid.

What I verified rather than accepted, each against the tree:

- **The params digest is reproducible from the specification alone.** I rebuilt the 134-byte preimage (`"IVSP"` + big-endian `u16` 1 + the four body digests, in memory-map/PSG/STIC/controller order) from `build/dev/core/gen/*.inc` with my own script and got `efba85009a57e91ad21060742dadc34949b976829525bb5e18d6b0897af1425c`, exactly the re-pinned literal. `state.literal.params-digest` (`tests/state/state_literal.c:166-201`) independently rebuilds the same preimage, so the pin, the core and the spec agree by two paths.
- **No vector datum moved.** Parsed both revisions of `data/cp1610/vectors/interrupt.json` and diffed every field except `derivation`: 39 vectors, identical id set, exactly five added `config` keys and nothing else; `grep -c '"config": {'` is 7, matching the SUMMARY.
- **The 24 rewritten "same under either setting" clauses are true.** For every vector carrying that clause I checked its own record: `init.intff` is 0 or absent and `expect` has no `intff` in all 24 — no clause asserts a datum the vector does not hold.
- **The toy append is 13 files, 39 added lines, 0 deletions** (`git show --numstat 548e80d`), no `DEVIATION` file, and the five §2.4 rows resolve (the `important-gates` step passes in both directions).
- **Derived-material discipline holds elsewhere:** no tracked `.ivsc`/`.cfg`/`.ivsn`, `binary-ext.txt` and `.gitignore` untouched by the phase, no fixture manifest carries a `committed` flag at all, and the tool's only digest output is its one stdout line.
- **`run.stub2.jd-as-j` discriminates structurally.** I did not re-measure the 179-vs-1 figure (that needs the flip reverted, and this review is read-only); the assertion `fj.entries == 1u && fj.entries == fjd.entries` cannot hold at field 0, and the STIC suite's explicit-0 rig limb (`si_rig_entries(0u, 0u, …)` → 251 entries) is the independent witness that 0 produces the storm.

Two notes on scope, not findings. The config block lists `core/src/cp1610/cp1610.c`, which does not exist; the file the phase changed is `core/src/cpu/cp1610.c`, and that is what I reviewed. And the tool *can* write inside the work tree in one place by design — `<root>/refs/`, the same exception `tools/owner-run.sh` makes — so "the tool cannot write inside the work tree" is true only for every path outside `refs/`; `/refs/` is gitignored (`.gitignore:2`), so nothing it writes there can be committed.

The necessity rubric was read before classifying. No finding qualified: rule 2 (the word-count refusal) is subsumed by rule 4 for every accepted base but still earns its place as the read bound and as a published token, and rule 6's pre-probe duplicates the `wbx` EEXIST refusal on purpose because the check *order* is the contract. Both are real work, not guards against nothing.

## Critical Issues

### CR-01: A zero-byte input is accepted and produces a container the core refuses to load

**File:** `tools/flatwrap/flatwrap.c:282-292` (the rule 1/2 gate), `:300` (rule 4), `:199-200` (the entry write)
**Issue:** The tool's rule set has no lower bound on the word count. A 0-byte input is even (rule 1 passes), is not more than 65,536 words (rule 2 passes), passes the base range (rule 3), does not cross `$10000` because it adds nothing (rule 4), is not inside the tree (rule 5), does not exist (rule 6) and contains no word above `$03FF` (rule 7). `fw_write` then emits a 64-byte container whose segment entry is `addr 4000, len_words 0000`. That container is invalid twice over against the container the tool exists to write: `IVS_IVSC_MIN_BYTES` is 66 (prefix 24 + entry 8 + one word 2 + trailer 32) and `ivs_ivsc_parse` refuses `len_words == 0` outright (`core/src/content/ivsc.c:144-146`, `IVS_IVSC_R_SEGMENT_LENGTH`; and `docs/formats/ivsc-v1.md` states `len_words` is 1 to 65,535). The tool exits 0 and prints a digest, so the operator has no signal at all. (The measured `reason=1` below is `IVS_IVSC_R_TRUNCATED`: the parser's length rule fires on the 64-byte file before the segment-length rule can, so the container is refused for the smaller reason first.)

Measured on this tree:

```
$ : > empty.bin && build/dev/tools/flatwrap/ivs_flatwrap empty.bin 4000 empty.ivsc
ad669ef4f57f54140fe176f411713bbaf5cda173e061bcd4cf4adcc97f8fc2f4
rc=0        # 64 bytes; entry bytes 24..31 read 40 00 00 00 0a 00 00 00
$ build/release/runner/ivs-run --content-dir . --exec exec.bin --grom grom.bin \
      --cart empty.ivsc scn.ivss          # content exec/grom/cart user
outcome harness-fail
outcome.detail load-refused role=cart status=IVS_E_MALFORMED reason=1
slot.cart.loaded 0
```

The 14-01 test suite has no case for a 0-byte input, so the gate cannot see this: `fw_accepted` checks exit 0, one 64-hex line and an exact byte count, and the only case that proves a container *loads* is the 8,192-byte round trip. The phase's own "the load is the drift detector" sentence in the file header is exactly what fails here.
**Fix:** refuse an empty input as part of rule 1 (no new token, so the pinned seven and `pack.judge.flatwrap-tokens` stay intact), and extend the existing case with the refusal beside its 2-byte positive:

```c
  if (size == 0L) {
    fclose(f);
    return fw_refuse("flatwrap-length", "the input is empty, and a container segment holds at least one word");
  }
```

and in `tests/pack-tap.sh` under `flatwrap-length`, after the existing pair:

```sh
      : > "$tmp/fl.0"
      fw_refused flatwrap-length "$tmp/fw.out.ivsc" rm "$tmp/fl.0" 4000 "$tmp/fw.out.ivsc" || ok=0
```

Update the rule-1 wording in the header comment ("an odd byte count, or none") so the published order still describes what the tool does.

## Warnings

### WR-01: Both path probes open the file, so a FIFO hangs the tool forever

**File:** `tools/flatwrap/flatwrap.c:317` (the output existence probe), `:272` (the input open)
**Issue:** Rule 6 tests "does the output already exist" with `fopen(out, "rb")`, which blocks until a writer appears when the path is a FIFO; the input is opened with `fopen(flat, "rb")`, which blocks the same way. Both are operator-supplied paths, so both are reachable. Measured: `mkfifo myfifo; timeout 5 ivs_flatwrap two.bin 4000 myfifo` → exit 124; the same with the FIFO as the *input* → exit 124. The project already treats this as a real hazard for exactly this kind of path: `ivs_hostio_read` "opens without blocking, requires fstat to report a regular file" (`runner/hostio.c`, and the refusal comment at `runner/main.c:803-805`), so the tool is the one place in the tree that opens an operator path without that discipline. A refusal that never returns is worse than a wrong token: the operator's shell sits there and the CI-shaped timeout discipline of the suites does not apply to a build-only tool.
**Fix:** make the existence probe a `stat`/`lstat` (no open at all — it is also a smaller TOCTOU surface, since the `wbx` create is what actually enforces rule 6), and open the input with the non-blocking regular-file check the runner uses:

```c
#include <sys/stat.h>
...
  struct stat st;
  if (stat(out, &st) == 0) { fclose(f); snprintf(detail, sizeof detail, "the output %s already exists", out);
                             return fw_refuse("flatwrap-output-exists", detail); }
...
  int fd = open(flat, O_RDONLY | O_NONBLOCK | O_NOCTTY);
  if (fd < 0 || fstat(fd, &st) != 0 || !S_ISREG(st.st_mode)) { /* file error, exit 2 */ }
```

### WR-02: The C41 record still states the deleted default as current fact in three of its fields

**File:** `data/discrepancies.json:407` (`topic`), `:410` (`default_policy`), `:413` (`note`)
**Issue:** The profile now carries `cpu.interrupt_accept_clears_intff = 1`, but the row the phase names as the record of the observation says the opposite three times, in the present tense:

- `:407` topic — "Interrupt acceptance **does not clear** the interrupt enable flip-flop".
- `:410` default_policy — "unknown-default-chosen: interrupt acceptance **does not clear INTFF** … ; profile field cpu.interrupt_accept_clears_intff = **1**". One field, two contradictory statements: the policy sentence says the project's chosen default is the behaviour that 1 means *not* to have.
- `:413` note — opens with "Profile field cpu.interrupt_accept_clears_intff **holds 0 (false)** as a JSON number", which is simply false now, and the appended amendment (1,518 characters later in the same field, 2,786 characters long) says "the field is now 1". GUARDRAILS §2.10 asks a live document a person reads to get an *inline pointer* at the wrong sentence as well as an appended amendment; `:413` got only the amendment, and `:410` got its value edited without its sentence.

The `evidence` sentence at `:412` that says "Phase 6 final … carried by … = 0" is fine: it is explicitly historical. This finding is about the three present-tense statements. The phase's own criterion 3 rests on this row ("the observation in the discrepancy log"), and `AGENTS.md` is explicit that "a quoted figure that disagrees with the file is a defect to correct, not a fact to reuse".
**Fix:** correct the policy clause after the schema's required prefix, and give `topic` and `note` the inline pointer the other five amended pages got:

```json
"topic": "Interrupt acceptance and the interrupt enable flip-flop; JE takes effect before the boundary test [value chosen as 1: see the 2026-11-26 amendment in `note`, D-51, C41]",
"default_policy": "unknown-default-chosen: interrupt acceptance clears INTFF, and JE takes effect before the boundary test (POL-INT-03); profile field cpu.interrupt_accept_clears_intff = 1, with vectors for both values.",
"note": "Profile field cpu.interrupt_accept_clears_intff holds 1 (true) as a JSON number [amended 2026-11-26, D-51: this sentence read `holds 0 (false)` before the flip; the amendment at the end of this field records the observation and the retarget]. Plan 06-01 (D-08): ..."
```

The prefix `unknown-default-chosen: ` and `status: "open"` stay as they are (the schema's `when` hook and the tag rule both still hold).

### WR-03: A must-fail registry row now names a case that pins a pass

**File:** `data/expected/important-gates.ids:337` (with `data/expected/diag-mutations-scn.ids:90`)
**Issue:** `scn-mutants mutant runner-process.mutant.SCN-STUB-JD-AS-J SCN-01` registers this case as one of SCN-01's mechanisms "shown to fail for its intended reason". After 14-02 the inventory row reads `SCN-STUB-JD-AS-J demo-lamp-tone-pass 1005 0352 0350 - pass 0`, i.e. the case now passes only when the mutation breaks nothing. `tools/check/steps/important-gates.sh:184-194` resolves a row by looking the `mutant <id>` pair up in the inventories and never looks at the row's expected outcome, so the gate reports the mechanism as covered while the only thing it can now prove is the *absence* of a failure. That is the "a gate can pass vacuously" case the step's own header names as its reason to exist, and GUARDRAILS §2.7 forbids dropping a judge or a pinned-default test to keep a count.

The SUMMARY records this honestly and defers it ("A later plan should either drop that line or repoint it at a case that still fails"), and SCN-01 keeps eleven other failing mutant rows, so the requirement's coverage does not rest on this one. It is still a live inaccuracy in the registry the phase edited, so it should not leave the phase.
**Fix:** either delete line 337 (SCN-01 stays covered by the eleven failing rows, and `gates`/`lines` move to `116`/`356` with the record updated) or repoint it at a case that still fails for its intended reason. The project's own practice says a `pass 0` row is not a must-fail mechanism: the other three negative rows (`SCN-REARM-NEGATIVE` at `diag-mutations-scn.ids:92`, `STIC-PAD-NEGATIVE` at `diag-mutations-stic.ids:46`, `DELAY-COUNT-CHANGED` at `diag-mutations-ctrl-psg.ids:46`) are all deliberately absent from `data/expected/important-gates.ids`, and after 14-02 SCN-STUB-JD-AS-J is the only `pass 0` row that is in it. Whichever way the row goes, the counts move with it (`gates=117 lines=357` in the closing line and in `14-04-SUMMARY.md`).

## Info

### IN-01: The same refusal token carries two different detail texts

**File:** `tools/flatwrap/flatwrap.c:212` versus `:322`
**Issue:** Rule 6's probe branch prints `the output <path> already exists`; the `EEXIST` branch inside `fw_write` (the race the exclusive create catches) prints the bare path, because `fw_refuse` is called with `out` as the detail. Both are `flatwrap-output-exists` and both are correct behaviour, but the published line differs by which branch won, and `tests/pack-tap.sh` only checks the token prefix, so the difference is invisible.
**Fix:** pass the same formatted detail in the `EEXIST` branch (`snprintf(detail, sizeof detail, "the output %s already exists", out);`), or drop the `EEXIST` branch's token and let the probe be the only place the token is minted.

### IN-02: `--cart ""` is silently treated as "no cartridge"

**File:** `tools/owner-run.sh:37` (`--cart) … cart=$2`), `:49` (`[ -n "$p" ] || continue`), `:96` (`want=0`)
**Issue:** An empty `--cart` value passes argument parsing, is skipped by the location loop, and ends with `want=0`, so the helper runs with no cartridge and no `content cart user` line, and the operator gets a report that looks like a deliberate no-cartridge run. Every other malformed argument is a usage refusal with exit 2, so this is the one silent branch.
**Fix:** `--cart) [ "$#" -ge 2 ] || die "$usage"; [ -n "$2" ] || die "$usage"; cart=$2; shift 2 ;;` — an option that names a path must name one.

### IN-03: The `refs/` exception is the one way the tool may write inside the tree, and no case exercises it

**File:** `tools/flatwrap/flatwrap.c:177-179` (the `refs/` return), `tests/pack-tap.sh:306-323`
**Issue:** Rule 5 refuses an output whose physical parent is inside the work tree *except* under `<root>/refs`, and that exception is load-bearing: `tools/owner-run.sh` and `docs/owner-images.md` both put the container there, and the owner-local test writes its container into a scratch root's `refs/` (outside the real tree, so it never reaches this limb). The suite covers three refused spellings and one outside positive, so a regression in either direction is invisible: a widening of the exception (`refs2/`, a prefix that matches `/refs` without the slash) or a change that starts refusing `refs/` itself would leave the gate green. The exception is also the single place the phase's "never inside the work tree" claim is softened, which makes it the limb most worth pinning.
**Fix:** cheap and tree-safe: keep the test out of the real `refs/` but assert the *predicate* the exception encodes by exercising the accept path once against a path under `$root/refs` only when that directory already exists and is writable, or (better, and no tree write at all) split `fw_inside_tree`'s decision into a pure helper over two already-resolved strings and unit-test it from the `pack` suite with `$root/refs/x.ivsc`, `$root/refs2/x.ivsc` and `$root/x.ivsc` as literals. Record in the comment that the exception is deliberate (D-47, R-2) so a later reader does not read it as a hole.

---

## Dispositions

Fixed in one review round on branch `gsd/v0.3.0-phase-14-cartridge-route`, one commit per finding, each
naming its id. The three INFO rows are deferred with their reasons; nothing above was rewritten, so a
reader can still see what the reviewer measured.

| Finding | Disposition | Commit | What was done, and what was measured |
|---|---|---|---|
| CR-01 | fixed | `bf3215d` | A zero-byte input is refused as its own rule and its own token, `flatwrap-empty` (appended to `data/expected/flat-refusals.ids`, never substituted), with one word as the boundary positive: 66 bytes, entry `4000 0001`. The case `pack.flatwrap.neg.flatwrap-empty` and the rule-1 wording moved with it. Measured: the empty input is refused, the 2-byte input is accepted, `ctest -R '^pack'` green. |
| WR-01 | fixed | `4efc806` | The output existence probe is a `stat` (no open at all, and a smaller TOCTOU surface: the exclusive `wbx` create still enforces rule 7), and the input is opened `O_RDONLY\|O_NOCTTY\|O_NONBLOCK` and refused unless `fstat` reports a regular file, which is `runner/hostio.c`'s `ivs_hostio_read` technique. Measured on this tree, each under `timeout 5`: FIFO as output -> exit 1 `flatwrap-output-exists` (was exit 124); FIFO as input -> exit 2, `is not a regular file` (was exit 124); an ordinary 4-byte input still accepted (68-byte container, rc 0); an existing file still refused and left byte-identical; a dangling symlink still refused `flatwrap-output-exists` by the `wbx` create. `ctest -R '^pack'` green. No gate case was added: the hazard is a host-environment one, and the three artifacts GUARDRAILS 2.4 demands for a new check (a registry row, a liveness guard, a scratch stand-in control) would move `controls`, which this round may not do. |
| WR-02 | fixed | `94300ec` | Per GUARDRAILS 2.10, `topic` and `note` now carry an inline pointer at the sentence the flip made wrong, and `default_policy`'s clause after the schema's required prefix says what `1` means. The note states what it read before the flip (`holds 0 (false)`) and the 2,786-character amendment at the end of the field is untouched, so nothing was rewritten away. `C41` stays `open` (the hardware probe is still the resolver), the `unknown-default-chosen: ` prefix and `status` are unchanged, no other discrepancy was touched, and `discrepancies=110 open=101` is unchanged. |
| WR-03 | fixed | `fc25ce9` | Deleted, not repointed: every one of SCN-01's eleven still-failing mutant rows is already registered, so the only repoint targets were duplicates, and the project's own practice is that a `pass 0` row is not a registry line (the other three are deliberately absent). The case is not dropped: `SCN-STUB-JD-AS-J` stays a live row in `data/expected/diag-mutations-scn.ids` whose stated outcome the `scn-mutants` case judges. The block comment now names both negative rows and says why. Measured: `-- important-gates pass gates=117 lines=356`. |
| IN-01 | deferred | - | Two detail texts under one token, both correct; `tests/pack-tap.sh` asserts the token only, so a change here would be invisible to the gate while touching the published line in the same round that rewrote WR-01's probe. It belongs with a case that asserts the detail, not with a one-line edit nobody can see. |
| IN-02 | deferred | - | `--cart ""` is the one silent branch of `tools/owner-run.sh`, whose refusal, skip and identity cases share one case judge (SCN-02). Turning it into a usage refusal is a behaviour change in the owner route that needs its own case, and this round may not add a case id or control. Recorded for the next owner-route plan. |
| IN-03 | deferred | - | The recommended fix splits the `refs/` predicate into a pure helper and unit-tests it from the `pack` suite, which is a new test surface; this round's constraint is `controls=402`, no new gate step, fault or golden. The exception is already documented as deliberate (D-47, R-2) in the tool's header, so a later reader is not left with an unexplained hole. |

### WR-03's count arithmetic, corrected

The finding's Fix section predicted `gates`/`lines` would move to `116`/`356`. Measured, `gates` does not
move: the step's `gates` counts **distinct gate names** (`if (!($1 in gates)) ngates++`), and
`scn-mutants` keeps eleven rows, so only `lines` moves, `357` -> `356`. Recompute, never quote.

### The gate line at the fix head

`sh tools/gate.sh` exits 0 with no `^-- FAIL` line, `gate-time total=97`. Three tokens differ from the
line `14-04-SUMMARY.md` and `14-VALIDATION.md` quote, and each has a named cause: `lines=356` (WR-03's
deleted registry row), `tests-expected=5016 tests-executed=5012` (CR-01's `pack.flatwrap.neg.flatwrap-empty`,
added after the closing line was measured; the figure is two ids per case, one per preset), and
`tracked=2747 files=2747 hygiene-files=2747` (the two record files added after that measurement:
`14-04-SUMMARY.md` in `2e27267` and this review). `steps=38/38`, `faults=238`, `goldens=29` and
`controls=402/402` are unchanged, so no gate step, fault, golden or control was added or dropped.

```
gate: PASS steps=38/38 controls=402/402 api-calls=19 api-statuses=15 api-rows=61 pinned-defines=120 cc=AppleClang c-presets=2 fault-runs=238 core-imports=7 purity-selftest=7/7 ctest-runs=2 ci-runs=100 ci-uses=2 ci-workflows=2 ci-summary-selftest=11/11 claims=28 provisional=27 cp1610-rows=74 patterns=1024 unclaimed=0 ambiguous=0 coincidences=1 agree=71 resolved=3 vectors=412 source=291 policy=121 rows-pending=0 policies-pending=0 bus-figure-pages=35 blind-files=12 second-read=12/12 scanned=689 waived=0 discrepancies=110 pending-resolvers=20 open=101 d10-items=47 faults=238/238 tracked=2747 binaries-listed=0 rights-pinned=4/4 terms=9 doc-files=63 profile-keys=154 goldens=29 reports=30 host-dependent=3 release-only=2 mode=tree hygiene-files=2747 history-commits=1 gates=117 lines=356 input-rows=31 input-policies=11 input-profile-fields=31 files=2747 listings=7 listing-words=1386 manifests=11 effective-yes=8 effective-no=3 rights=10 rights-cleared=1 literal-rows=80 views=2 dump-lines=160 prefix-files=11 archive=members public-symbols=19 defined=141 hdr-alone=10/10 consumer-lines=164 cxx-link=ok pkg-selftest=19/19 records=154 fields=154 unknown-fields=47 psg-rules=21 psg-vectors=120 psg-policies=12 release-selftest=11/11 runner-files=18 cases=14 sources=13 refs-verified=3/4 state-rows=88 state-cuts=18 tests-expected=5016 tests-executed=5012 tests-skipped=4 tests-missing=0 docs=64 claim-refs=26 stic-rules=67 stic-rows=64 video-frames=26 video-hash-ties=18 video-frame-pairs=9 tags=10 tiers=8
```

---

_Reviewed: 2026-11-26T03:22:38Z_
_Reviewer: Claude (gsd-code-reviewer)_
_Depth: standard_
_Fixed: 2026-11-26 - CR-01 `bf3215d`, WR-01 `4efc806`, WR-02 `94300ec`, WR-03 `fc25ce9`; IN-01..03 deferred_
_Fixer: Claude (gsd-code-fixer)_

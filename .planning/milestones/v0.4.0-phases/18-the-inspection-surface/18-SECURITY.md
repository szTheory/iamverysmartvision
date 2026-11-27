---
phase: "18"
slug: "the-inspection-surface"
status: secured
# threats_open = count of OPEN threats at or above workflow.security_block_on severity (high).
threats_open: 0
asvs_level: 1
created: "2026-11-27"
audited: "2026-11-27T03:28Z"
auditor: "gsd-security-auditor (pi, subagent)"
---

# Phase 18 -- Security

> Per-phase security contract: threat register, accepted risks, and audit trail.

**Verdict: SECURED -- 12 register rows across three plans, 12 distinct threat ids, 12 closed, 0
open.** `block_on: high`, `asvs_level: 1`. Every `high` row (`T-18-01`, `T-18-03`, `T-18-04`,
`T-18-05`, `T-18-06`, `T-18-10`, `T-18-11`, `T-18-SC`) is closed by a command run in this session,
not by reading a SUMMARY; every `medium` row is closed the same way.

## Basis of this audit

This phase's threat surface is **evidence and rights integrity, not attack surface**. It adds one
build-only executable, one rights row, two name lists and sixteen test ids; it adds no core code
(D-52), no network, no install rule, no release membership, no dependency and no parser for input the
project does not already own. The risk it carries is therefore the inverted one: an output path that
escapes into the work tree or a tracked file, an artifact committed despite the rule, a rights row
that lands after the bytes or does not govern the naming limb, a "build-only" tool quietly acquiring
an install rule, and a check that passes vacuously.

Evidence classes, used consistently below:

- **code** -- `file:line` where the refusal, the equality or the exclusive create is emitted.
- **live** -- a command run in this session against the committed tree at `5565c7c`, the built
  `build/dev/tools/inspect/ivs-inspect`, or a `mktemp` directory outside the repository. No tracked
  file was modified; the two probe paths written under `build/` and `refs/` were removed and the tree
  ends clean (`git status --porcelain` empty).
- **suite** -- the committed id or committed control that re-demonstrates the mechanism on every gate
  run, whose own result I re-read from a run taken now.
- **record** -- a phase record's claim, cited only where the tree has already been measured against
  it; three such claims disagree with the tree and are listed under *Accepted debts*.

Final state measurement, taken on the tree this record is committed to (`5565c7c`), before this file
existed:

```
sh tools/gate.sh
  -> exit 0, `grep -c '^-- FAIL '` = 0
  -> gate: PASS steps=38/38 controls=405/405 ... gates=122 lines=372 ... rights=11 rights-cleared=1
     ... prefix-files=11 pkg-selftest=21/21 release-selftest=13/13 ... tests-expected=5082
     tests-executed=5078 tests-skipped=4 tests-missing=0 ... faults=238/238 goldens=29 claims=29
     provisional=28 handoffs=8 docs=65 tracked=2835
sh tools/hygiene.sh --tree            -> -- hygiene pass mode=tree hygiene-files=2835
ctest --preset dev -R 'runner-process|asm' -> 2/2 passed (asm 6.58 s, runner-process 29.48 s)
```

The invariants the brief froze recompute from the files rather than from the line: `tests-expected`
= `cat data/expected/tests/*.ids | grep -v '^#' | awk NF | wc -l` (2541) x 2 presets = **5082**;
`controls` = `ls -d tools/check/controls/*/ | grep -vc _good` = **405**; `steps` =
`ls tools/check/steps/*.sh | wc -l` = **38**; `rights` = `ls data/rights/*.json | wc -l` = **11**;
`handoffs` = **8**; `claims` = **29**. `gates=122` and `lines=372` are the registry's own counts and
are read off the run above. `tracked` moved 2834 -> 2835 against `18-VERIFICATION.md` because that
verification's own record was committed after its measurement; the arithmetic recomputes.

After this record was staged, the gate was run again on the tree that carries it: `sh tools/gate.sh`
-> exit 0, zero `^-- FAIL ` lines, with `files=2836 tracked=2836 hygiene-files=2836`. Those three
are the only moved tokens, they count `git ls-files`, and the one file they gained is this record --
so every invariant above still reads its frozen value on that run too.

**The container gate (`tools/gate-linux.sh`) was not run and is not claimed.** Measured now:
`docker info` produced no answer inside 15 s and the process had to be killed. No threat in this
register depends on the platform: every mechanism below is `realpath(3)`, `lstat(2)`,
`fopen("wbx")`, `fwrite`/`fflush`, a POSIX shell test, a CMake `file(WRITE)`, or a TAP case, all of
which behave identically on the Linux image; the two name lists are text. The absence is recorded as
**outstanding evidence, not a pass** (lesson 24).

## Trust Boundaries

Union of the three plans' boundaries.

| Boundary | Description | Data Crossing |
|----------|-------------|---------------|
| the plan's prose -> the frozen document | a sentence written after the code can quietly disagree with the page; the page must bind, so the ordering and the byte-prefix freeze are the control | one page, one ancestry proof |
| the document -> the gate's prose rules | a page edited to pass a scanner stops being a contract | claim words, claim refs, tier ids |
| the tool's output -> the work tree | an operator-local artifact is one careless path away from being a tracked file, and a trace or disassembly of rights-bearing content is derivative material | one `.ivst`/`.asm` path, three spellings |
| the two passes -> the report's numbers | the report and the trace come from different calls; without an equality the trace is a second truth | nine digests, one end tick |
| a stand-in -> the check that judges it | a check that accepts its own stand-in proves nothing | four committed stand-ins, one in-id limb |
| the build tree -> a release | a build-only target that acquires an install rule or a member rule becomes a shipped artifact unnoticed | eleven prefix paths, one archive member list |
| the registry -> the requirement record | a row naming a `Pending` requirement is refused and a `Complete` requirement with no row is refused | requirement ids, gate names |
| the listing -> the emitted source -> `ivs_asm` | the assembler is the oracle, so a source that assembles is not yet evidence - only the word equality is | 22 words, three pairwise dumps |
| a new source file -> the build's own list | a tool source outside `ivs-inspect-sources.txt` is a `c-build` finding, and the reverse is the same defect | three source names |

---

## Threat Register

*12 register rows across the three plans resolve to 12 distinct ids: `T-18-SC` is carried by all
three plans (one shared threat, verified once); `18-01` carries `T-18-01`, `T-18-02`, `T-18-SC`;
`18-02` carries `T-18-03`..`T-18-08` and `T-18-SC`; `18-03` carries `T-18-09`..`T-18-11` and
`T-18-SC`.*

| Threat ID | Plan(s) | Category | Severity | Disposition | Mitigation | Evidence | Status |
|-----------|---------|----------|----------|-------------|------------|----------|--------|
| T-18-01 | 18-01 | Tampering -- `docs/inspection.md` retro-fitted to whatever `tools/inspect/` does | high | mitigate | the page is committed in a commit of its own before any `tools/inspect/` path exists, and the freeze is a **byte prefix** rather than a whole-file equality, so every later correction is an append | **live** `git log --diff-filter=A -- docs/inspection.md` -> one commit, `0627be9`; `git show --stat 0627be9` -> `docs/inspection.md \| 253 +++`, 1 file changed; `git ls-tree -r --name-only 0627be9 \| grep -c 'tools/inspect/'` -> **0**; `git merge-base --is-ancestor 0627be9 5ed5610` -> exit **0** (the page precedes the first tool commit); `now.startswith(frozen)` on the 15,699-byte frozen page and the 21,194-byte page now -> **True**; `git show e936f8d --stat` -> 77 insertions, 0 deletions and `after.startswith(before)` -> **True**, so the only later commit is a pure append | closed |
| T-18-02 | 18-01 | Repudiation -- a page that passes the gate because a claim word or the time-base word was re-worded until the scanner was quiet | medium | mitigate | the page describes conditions rather than outcomes, mints no claim, cites no claim id, and the gate's own `text`/`glossary` tripwires are the check; a re-wording that changed a condition is visible in the diff of a two-commit page | **live** the step's own regex over the page -- `grep -a -n -i -w -E 'supports\|compatible\|passes\|accurate\|verified\|runs\|plays\|playable\|looks\|sounds' docs/inspection.md` -> **no line**, and the same over the frozen page -> no line; `grep -o -E '\[claim:[^]]*\]' docs/inspection.md` -> none; `grep -oE '\b(CLM\|EV\|CLAIM)-[0-9]+'` -> none; the run above carries `claims=29 provisional=28 claim-refs=27`, the frozen values, and `text`/`glossary` are inside `steps=38/38`. Deviation 2 of `18-01-SUMMARY.md` claims two `[nc]` waivers exist; the tree has **zero** (see accepted debt D1) -- the mitigation does not depend on them, because no claim word matches | closed |
| T-18-03 | 18-02 | Information disclosure -- a trace or disassembly dump written into the work tree, or committed as a golden, fixture or CI artifact | high | mitigate | the tree refusal runs before any create in three spellings with the `refs/` exception, `ivst` sits in the body of `tools/check/binary-ext.txt` and in `.gitignore`, the `files` step refuses it by name, and a committed control proves that refusal | **live** literal in-tree -> `ivs-inspect: inspect-output-tree: the output build/audit-probe.ivst is inside the work tree outside refs/`, rc 1, **no file**; `..` spelling (`tools/../build/...`) -> same token, rc 1, no file; absolute `..` spelling -> same token, rc 1, no file; a symlinked directory inside the tree -> same token, rc 1, no file; a `refsX/` sibling -> **refused** (the `/refs` and `/refs/` compares are exact); `refs/` accepted by a positive write -> rc 0, 9,669 B, `git check-ignore -v` -> `.gitignore:2:/refs/`, `git ls-files refs \| wc -l` -> 0, file removed; `grep -n ivst tools/check/binary-ext.txt .gitignore` -> `:49 ivst` (body) and `.gitignore:25 *.ivst`; `git ls-files \| grep -E '\.ivst$'` -> empty; `git diff --name-status 48470fa..HEAD \| grep -E '\.(asm\|ivst)$'` -> empty. **suite** `control ok files-unlisted-binary-ivst` in the gate run above (`controls=405/405`), whose `MAKE` writes `docs/run.ivst` and whose `EXPECT` is `FAIL files-unlisted-binary: docs/run.ivst` | closed |
| T-18-04 | 18-02 | Tampering -- a second report writer, which would make the `digest.*` equality unfalsifiable | high | mitigate | the report is the bytes `ivs_runner_execute` hands the tool's `write_report`, which captures them verbatim and forwards them to stdout; the tool constructs no record and calls no report writer; the nine values it reads back are compared to the traced instance's own `ivs_machine_state_digest` values, and a poked report must be refused | **live** `grep -rn ivs_runner_report_write tools/inspect/` -> **two header comments, zero call sites**; the capture is `inspect.c:457` wired at `:736`; the nine `digest.*` lines of the tool's report vs `ivs-run`'s for the same scenario -> `sed -n 's/^digest\.//p'` on both, 9 lines each, `cmp` **identical** (the whole-report diff is `output.audio.*` only, i.e. the host sink, not the machine); poked report (`digest.cpu` replaced by `digest.mem`'s value) -> `ivs-inspect: inspect-reconcile: the stored report's digest.cpu is not the one this run printed, so the two documents disagree`, rc **1**, stdout **0 B**, **no output file**. **suite** `ok 151 runner-process.inspect.trace-digests`, `ok 161 runner-process.judge.trace-pokes-register` in a run taken now | closed |
| T-18-05 | 18-02 | Repudiation -- a check that passes vacuously | high | mitigate | every id asserts a domain, a count, a witness pair or a byte equality; `records` must be above zero and **both** digest sets must be exactly nine lines before any comparison is allowed to be silent; each committed stand-in is judged by the same function the real case uses, and a stand-in the check accepts is reported `judge-vacuous` | **live** the four judge helpers, read from `tests/runner-process-tap.sh`: `ix_records` refuses `records=0`; `ix_digests_equal` refuses a side that is not exactly 9 lines **before** it `cmp`s; `ix_tick_domain` requires non-decreasing **and** more than one distinct tick; `ix_break_pair` pins `1 0` **and** reconciles the file-derived pair against the trace's own in-run counts; `self_control_run` returns `vacuous` when the check accepts the stand-in and `wrong-reason` unless the observed token set equals the declared set exactly; `ixt_parse_oracle` refuses a report missing any of the nine digests or any step's `now_ticks`, so an empty extraction cannot make a comparison silent. **suite** `sh tests/runner-process-tap.sh` on this tree -> `# suite=runner-process executed=161 skipped=2 failed=0`, zero `^not ok`, with `ok 150`..`ok 162` (all nine `inspect.*` and all four `judge.*`), including `ok 159 judge.trace-empty`, `ok 160 judge.trace-repeat-tick`, `ok 161 judge.trace-pokes-register`, `ok 162 judge.break-always-hits` | closed |
| T-18-06 | 18-02 | Tampering -- a build-only tool acquiring an install rule or a release membership unnoticed | high | mitigate | no `install(` in the target's CMake, the eleven-path inventory unchanged, a `pkg-install` selftest that plants `bin/ivs-inspect` in a copy of the prefix, a `release-assets` case that refuses it as an unlisted archive member through the existing extra-member limb whose negative limb shows the extension-only rule cannot see it, plus the `c-build` module line and the toy source list | **live** `grep -n 'install(' tools/inspect/CMakeLists.txt` -> **no match**; `cmake --install build/release --prefix <tmp>` -> rc 0, **11 files**, `find <prefix> -name '*inspect*'` -> **0**, and the eleven are the same `include/`, `lib/`, `share/doc/` paths; `awk '!/^#/ && NF' data/expected/pkg-install-files.ids \| wc -l` -> **11**; `grep -rn 'ivs_inspect\|ivs-inspect' data/expected/*.ids` outside the two selftest inventories and the test inventories -> **no match**. **code** `pkg-install.sh:683-692` (the `inspect-absent` case: plant `bin/ivs-inspect`, require `-- FAIL pkg-inventory-extra: bin/ivs-inspect` from the same `check_inventory`); `release-assets.sh:202-216` (the `inspect-unlisted-member` case and its negative limb); `c-build.sh:51` (the `tools/inspect/.*[.]c$ ivs-inspect-sources.txt` module line); `tools/check/controls/_good/tree/CMakeLists.txt:17` + `good.c` (the toy source list); `docs/packaging.md:38,122` (the "not installed" line naming the tool). **suite** the gate run above reads `prefix-files=11 pkg-selftest=21/21 release-selftest=13/13` | closed |
| T-18-07 | 18-02 | Denial of service -- an unbounded trace file, or one that silently truncates | medium | mitigate | the trace's length is bounded by the scenario's own caps and the run's target; the writer creates exclusively, fails on a short write or a failed flush, and removes what it created, so a partial file cannot be mistaken for a complete trace | **code** `inspect_trace.c:91` (`IXT_TRACE_MAX` = 64 MiB), `:295` (the grow path refuses past the bound), `:659`/`:690` (`the trace is larger than %u bytes, or could not be built`), `:556` (`bound-too-large step=`), `:714` (`fwrite(...) != buf.len`), `:718` (`fflush(out) != 0`), `:843` (the step report's own write+flush check); `inspect.c:361` (`fopen(out, "wbx")`, the exclusive create), `:768` (`fclose` failure is a write error), `:787-788` (`if (created != 0) remove(out)`). **live** the poked-report run above fails **after** the create and leaves **no output file**, which is the removal limb firing; the accepted run is 9,669 B with a single `end` record last | closed |
| T-18-08 | 18-02 | Elevation of privilege -- an output path that escapes the tree rule through `..` or a symlink | medium | mitigate | the parent directory is resolved physically with `realpath` and compared physically, so a spelling through `..` or a symlink gets the same answer as the literal path; the three spellings are all refused in the case and the `refs/` exception is proved by a positive write rather than by prose | **code** `inspect.c:299` (`ix_inside_tree`), `:307` (the root resolved), `:324` (the parent resolved), `:330-334` (the physical prefix compare, then the exact `/refs` or `/refs/` suffix), `:342` (`ix_open_output` decides tree -> exists -> exclusive create, in that order). **live** four escape spellings refused with the token and no file left (literal, relative `..`, absolute `..`, symlink-from-outside-in) and a `refsX/` sibling refused; the `refs/` exception accepted by a 9,669-byte write that was then removed; the dangling-symlink case refused with `inspect-output-exists`, so `lstat` and not `stat` is what decides | closed |
| T-18-09 | 18-03 | Tampering -- a second decoder table, which would make the disassembler a second truth about the instruction set | medium | mitigate | the mnemonic is the listing's own `ROW` column and the operand its own `ASM` column; the only knowledge in the file is how to cut the annotation and when to emit `ORG`; a row it does not know is refused with `inspect-dis-row` before any output exists, and the round trip through `ivs_asm` is the oracle for every form | **code** `inspect_dis.c:56-65` (`ix_dis_rows` is a **names-only** vocabulary of spellings `ivs_asm` accepts: no encoding, no operand shape, no word), `:291` (the refusal), `:198` (the membership test); `grep -n 'table' tools/inspect/inspect_dis.c` -> only the prose that says there is none. **live** a word row renamed to `NOSUCHROW` (line 52 of a copy, same line count as the original) -> `ivs-inspect: inspect-dis-row: the row NOSUCHROW at 1002 is not one of the mnemonics ivs_asm accepts`, rc **1**, stdout **0 B**, **no output file**; the registered listing `fixtures/FX-diag-stic-2/diag-stic-2.listing.txt` -> `inspect-dis-row: the row DATA at 512E ...`, rc 1, no file (fail-closed, as Amendment 4 states); the round trip below is the positive oracle | closed |
| T-18-10 | 18-03 | Repudiation -- a vacuous round trip (`DECLE` for everything, or a corpus of one-word instructions) | high | mitigate | four guards on one id set: the word dump must hold more lines than the source holds instruction lines, an immediate must be present, the distinct-mnemonic count must be at least three, and a committed constant-mnemonic stand-in must fail the byte equality with a diff | **live** the emitted source of `fixtures/FX-boot-stub/boot-stub.listing.txt`: **22** words vs **16** instruction lines (margin 6 against a required 2); `MVII #$0001, R0` and `MVII #$0340, R6` present, and the committed twin carries an immediate too; distinct mnemonics by the committed id's own extraction = **5** (`B J MVII MVO TSTR`) against a required 3; the round trip `dis -> ivs_asm -> ivs_pack --dump-words` equals the input listing's dump (`cmp` silent, 22 == 22); the constant-mnemonic stand-in (every instruction line -> `DECLE $0000`, `ORG` kept) assembles at **rc 0** to **16** words that **differ** from the input's 22, so the *word comparison* is the clause it defeats (Amendment 3's correction, verified). **suite** `ok 6 asm.dis.roundtrip-boot-stub`, `ok 7 asm.dis.two-word-form`, `ok 8 asm.dis.immediate`, `ok 9 asm.dis.words-nonvacuous`, `ok 10 asm.dis.deterministic`, `ok 11 asm.judge.dis-constant-mnemonic`, and `# suite=asm executed=56 skipped=0 failed=0` | closed |
| T-18-11 | 18-03 | Information disclosure -- a disassembly dump written into the work tree or committed | high | mitigate | `dis` uses the same output ladder `18-02` proved (tree refusal with the `refs/` exception, then exists, then exclusive create); `ivst` is already refused by name in two lists and by the `files` step; the verify asserts no `.ivst` is tracked; the emitted `.asm` lives only under the suite's temporary directory | **live** `dis ... --out build/audit-dis.asm` -> `inspect-output-tree`, rc 1, **no file**; `dis ... --out refs/audit-dis.asm` -> rc 0, 430 B, removed; `grep -n ix_open_output tools/inspect/inspect.c` -> `:712` (trace), `:1001` (break), `:1105` (**dis**), so all three subcommands share one ladder; `git ls-files \| grep -E '\.ivst$'` -> empty and `git diff --name-status 48470fa..HEAD \| grep -E '\.(asm\|ivst)$'` -> empty; `tests/asm-tap.sh:64-65` writes every emitted source under `mktemp -d` with `trap 'rm -rf "$tmp"' EXIT`. **suite** the `asm` run above, 56 executed, 0 failed | closed |
| T-18-SC | 18-01, 18-02, 18-03 | Tampering -- npm/pip/cargo installs | high | mitigate | no plan installs anything: they write C, CMake, shell, JSON, ids and Markdown. No package manager runs, so no `[ASSUMED]` or `[SUS]` package exists to gate | **live** `git diff --name-only 48470fa..HEAD` -> 53 paths, none a manifest or a lockfile (`grep -Ei '(package(-lock)?\.json\|requirements.*\.txt\|Pipfile\|pyproject\|Cargo\.(toml\|lock)\|go\.(mod\|sum)\|vcpkg\|conan\|\.lock)$'` -> no match); `grep -rn -E 'find_package\|FetchContent\|ExternalProject\|pkg_check' tools/inspect/` -> no match; no `third_party/` path and no lockfile is in the diff, and the gate ran green with `steps=38/38 controls=405/405`, so no dependency entered the build | closed |

*Status: closed / open / open -- below `block_on` threshold (non-blocking). Severity order:
critical > high > medium > low; only open threats at or above `high` count toward `threats_open`.
No row is `open`.*

---

## Unregistered flags

**None.** All three SUMMARY records declare `## Threat Flags` = *None*, and this audit found no new
attack surface that the registers do not already map: the phase adds one executable with no install
rule and no release membership (`T-18-06`), one output path class with one ladder (`T-18-03`,
`T-18-07`, `T-18-08`, `T-18-11`), one rights row (`T-18-03`), one new source in the build's own list
(`T-18-06`), and one new argument to an existing suite (`T-18-05`). The `tools/check/integrity.cmake`
scope widening (`:84`) puts `tools/inspect/` *inside* the SPDX/ASCII scan, which narrows surface
rather than opening it.

---

## Accepted debts (non-blocking, each with a named owner)

None of these is an open threat at ASVS L1: in every case the mechanism exists in the tree and the
command above shows it working. They are recorded so a later reader does not have to rediscover them,
and each names who closes it.

| # | Debt | Why it is not an open threat | Owner |
|---|------|-----------------------------|-------|
| **D1** | `18-01-SUMMARY.md` deviation 2 states that two page lines "carry a `[nc]` waiver". The tree has **zero** `[nc]` markers in either commit of the page (`git show 0627be9:docs/inspection.md \| grep -c '\[nc\]'` -> 0; the same at `e936f8d` -> 0), and the page matches no claim word under the step's own regex, so no waiver was ever needed. | It is a **records inaccuracy**, not a missing mitigation: `T-18-02`'s control is the tripwire itself, which passes with no waiver and no claim id. `claims=29 provisional=28 claim-refs=27` are unmoved. Correcting it is an appended note, per §2.12. | next touch of `18-01-SUMMARY.md` (records), or the milestone close |
| **D2** | `T-18-01`'s mitigation plan says `git log --format=%H -- docs/inspection.md` "must stay at one line for the phase". It reads **two**: `0627be9` (the freeze) and `e936f8d` (the six appended amendments). | The mitigation's substance holds and is measured: the page is a byte **prefix** of itself, so the second commit cannot have rewritten a frozen line (`after.startswith(before)` -> True, 77 insertions, 0 deletions). The page's own rule is append-only amendments, which is what §2.12 requires; a literal one-line reading would have forbidden the amendment mechanism the phase's other threats depend on. | the plan's letter, at the milestone close (recorded here rather than re-blessed) |
| **D3** | `T-18-10`'s mitigation plan says the four guards are each "asserted on the emitted source **and** on the committed twin". In the tree, only `asm.dis.immediate` asserts on both; `asm.dis.two-word-form` and `asm.dis.words-nonvacuous` assert on the emitted source, and the twin is covered by the round-trip id's `dumps_agree`. | The vacuity the threat names is defeated: the corpus margin (22 vs 16), the immediate and the 5 distinct mnemonics are all measured on the emitted source, the twin's 22 words are byte-identical to the emitted source's and to the input listing's, and the constant-mnemonic stand-in fails the word comparison. No combination of "emitter coincidence" survives. | a later `asm` suite touch, if the wording is to be met literally |
| **D4** | **O1** -- `docs/inspection.md` `## Liveness guards` guard 2 says the stand-in "is a tracing pass that pokes one register"; the committed stand-in is a **report** with `digest.cpu` replaced by `digest.mem`'s value. No dated amendment records the difference. | The second half of the sentence is exactly true (one domain, named in the refusal) and the committed stand-in is judged by the same function the real case uses, so the guard is live and demonstrated (`ok 161 runner-process.judge.trace-pokes-register`, and by hand). The mismatch is page **wording**, not a missing control. | an appended Amendment 7 on the next touch of `docs/inspection.md` |
| **D5** | **O2** -- the review's WR-06 asked for an `asm.dis` case asserting the fail-closed refusal on the registered `ivs-diag-stic-2` listing; the disposition took the amendment only, so the committed unknown-row limb uses a synthetic `NOSUCHROW`. | The behaviour is present and I reproduced it on the registered listing (`inspect-dis-row: the row DATA at 512E ...`, rc 1, no file). What is missing is a **regression net**, not a mitigation: `T-18-09`'s refusal is proved on both a synthetic and the real listing in this session. Adding the id would also move `tests-expected`, so it is not a change this audit may make. | the phase's follow-up / next `asm` suite touch |
| **D6** | **O3** -- `GUARDRAILS-v0.4` §2.4's phase-boundary checkpoint word (`discharged-within-allowance` / `reprice` / `lever-required`) is not recorded for phase 18; `18-02-SUMMARY.md` says only "carried to the milestone close as a repricing candidate". | It is a **records obligation**, not a threat mitigation: the arithmetic is checkable (priced 2.96, recomputed 3.59, +21 %, inside the 25 % repricing band) and no threat in this register depends on it. | the milestone close |
| **D7** | **O4** -- §2.7's "three things in one commit" was satisfied across two commits: the four stand-ins and guards in `e876c93`, their four registry rows and both requirement flips in `d8eb593`. | Mechanically forced and recorded: `important-gates.sh:157-164` **derives** the accepted set from the two requirement records and refuses a row naming a `Pending` requirement *and* a `Complete` requirement with no row, so rows and flips cannot move separately. Both commits say so and `18-02-SUMMARY.md` items 6 and 12 record it. Not security-relevant: the drift this rule prevents is exactly what the derivation makes impossible. | recorded; no action |
| **D8** | **O5** -- the tool's report differs from `ivs-run`'s on `output.audio.*` (`520/130` vs `0/0`, `audio.chain.check` `pass` vs `-`). | Not a weakness: the build-only instrument supplies no audio sink, and the nine `digest.*` lines - which include the AUDIO domain - are byte-identical (`cmp` silent, 9 vs 9). The page scopes equality 1 to the nine digest lines, which is exactly what is asserted. | recorded; no action |
| **D9** | The container gate (`tools/gate-linux.sh`) is **not run**: `docker info` gave no answer in 15 s and was killed. | Outstanding evidence, not a pass (lesson 24). No threat here depends on the platform: the mechanisms are `realpath`, `lstat`, `fopen("wbx")`, `fwrite`/`fflush`, POSIX shell tests, CMake `file(WRITE)` and TAP cases, all platform-identical; the two name lists are text. | the milestone close, when a docker host is available |

---

## The review's two open notes, judged at ASVS L1

- **WR-06's extra case** (now `D5`): the refusal works on the registered listing and on a synthetic
  row, both reproduced here; the missing item is a regression id, not a mitigation. **Not
  security-relevant at L1; accepted as an owned debt.**
- **IN-05's ordering** (the page named the artifact two commits before the rights row): judged
  **closed, not a debt**. The page commit `0627be9` names the artifact class but writes no bytes; the
  only trace writer (`tools/inspect/inspect_trace.c`) and the rights row (`data/rights/
  inspection-output.json`, `data/expected/rights.ids`, `docs/RIGHTS.md`, both name lists, the control)
  landed in the **same commit**, `5ed5610` (`git show --stat 5ed5610` -> 21 files including all seven
  rights paths and both tool sources), and `git log --format=%H -- tools/inspect/inspect_trace.c` ->
  `5ed5610` and `e876c93` only. So §2.13's "rights before bytes" is met at the granularity that
  matters: no byte could have been produced before the row existed.

---

## Audit trail

- **Loaded before any action:** the three `*-PLAN.md` `<threat_model>` blocks, the three
  `*-SUMMARY.md` records (deviations and `## Threat Flags`), `18-VERIFICATION.md`, `18-REVIEW.md`,
  `18-VALIDATION.md`, `docs/inspection.md` (frozen and current), `data/rights/inspection-output.json`,
  `docs/RIGHTS.md`, `tools/check/binary-ext.txt`, `.gitignore`, `GUARDRAILS-v0.4.md` (§2.1, §2.5,
  §2.7, §2.13, W3, W4), `AGENTS.md`.
- **Implementation files are read-only in this audit.** No tracked file was modified; the only writes
  outside `/tmp` were the two probe paths under `build/` and `refs/`, both refused or accepted and then
  removed, and this record.
- **Commands run now, not quoted:** the full `sh tools/gate.sh` (rc 0, zero `^-- FAIL `), `sh
  tools/hygiene.sh --tree`, `ctest --preset dev -R 'runner-process|asm'`, both suites run directly for
  their per-id `ok` lines, the behavioural probes against the built `ivs-inspect` (five output-path
  spellings, the `refs/` positive write, the exists and dangling-symlink refusals, the poked report,
  the `dis` round trip, the constant-mnemonic stand-in, the unknown-row refusal on two listings), one
  `cmake --install` to a scratch prefix, the token re-extraction in both directions, and the byte-level
  history checks on the frozen page.
- **Not judged by any tool:** whether the frozen page states the *right* contract. That judgement was
  made in `18-DISCUSS`/`18-RESEARCH` and is not re-asked (§6.5's "not re-asked" rule).
- **Not run:** `tools/gate-linux.sh` (`D9`). Recorded as outstanding evidence, never as a pass.

_No threat at or above `block_on: high` is open. `threats_open: 0`._

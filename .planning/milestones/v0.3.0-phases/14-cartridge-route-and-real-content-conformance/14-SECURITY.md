---
phase: "14"
slug: "cartridge-route-and-real-content-conformance"
status: secured
# threats_open = count of OPEN threats at or above workflow.security_block_on severity (high).
threats_open: 0
asvs_level: 1
created: "2026-11-26"
audited: "2026-11-26T05:05Z"
auditor: "gsd-security-auditor (pi, subagent)"
---

# Phase 14 -- Security

> Per-phase security contract: threat register, accepted risks, and audit trail.

**Verdict: SECURED -- 25 register rows across four plans, 22 distinct threat ids, 22 closed,
0 open.** `block_on: high`, `asvs_level: 1`. No row is `accept` or `transfer`; every row is
`mitigate` and closed. Two **unregistered flags** are recorded below: neither maps to a register
row, both are record-accuracy defects rather than control gaps, and neither is at or above `high`,
so neither counts toward `threats_open`.

## Basis of this audit

The phase adds a host-side tool that reads an operator's file and writes a derived artifact, and it
moves a machine behaviour. Its security property is **inverted** again: the risks are a refusal that
does not fire, an artifact that escapes into the tree, and a behaviour change whose records still
assert the old value. Every mitigation below was therefore checked by making it fire or by reading
the exact statement, not by reading the plan's claim. Evidence classes, used consistently:

- **code** -- `file:line` where the refusal, the bound or the assignment is emitted.
- **live** -- a command run in this session against the committed tree, a `/tmp` scratch tree, or the
  built binaries (`build/dev/tools/flatwrap/ivs_flatwrap`, `build/dev/runner/ivs-run`). **No source
  was modified.**
- **derivation** -- a value recomputed here by a path other than the one that produced it (the
  params digest was rebuilt in Python from the four generated body digests and the 134-byte
  preimage arithmetic).
- **control** -- the committed stand-in that makes the check fail for its own reasons, confirmed by
  `control ok <name>` inside `controls=402/402`.
- **review** -- a measurement `14-REVIEW.md` already took, cited only where re-running it needed the
  flip reverted or a mutation this audit may not make. Everything cheap was re-run anyway: the four
  review fixes, all eight refusals, the round trip, and the digest.

Final state measurement, one command, run before this file existed:

```
sh tools/gate.sh   ->  exit 0, zero `^-- FAIL` lines, gate-time total=97
gate: PASS steps=38/38 controls=402/402 ... fault-runs=238 core-imports=7 ... claims=28
provisional=27 ... discrepancies=110 pending-resolvers=20 open=101 ... faults=238/238
tracked=2747 binaries-listed=0 rights-pinned=4/4 terms=9 doc-files=63 profile-keys=154
goldens=29 ... gates=117 lines=356 ... prefix-files=11 public-symbols=19 defined=141
hdr-alone=10/10 ... pkg-selftest=19/19 ... tests-expected=5016 tests-executed=5012
tests-skipped=4 tests-missing=0
```

Per-step lines used as evidence: `-- c-build pass cc=AppleClang c-presets=2`;
`-- integrity pass files=2747`; `-- important-gates pass gates=117 lines=356`;
`-- files pass tracked=2747 binaries-listed=0 rights-pinned=4/4`;
`-- pkg-install pass prefix-files=11 ... pkg-selftest=19/19`; `control ok
c-build-unlisted-flatwrap-source`. The two review-round fixes that moved a token are at the values
the amendments predicted (`lines=356`, `tests-expected=5016`), so nothing was re-blessed.

Also run this session: `ctest --preset dev -R '^(state|pack)$'` -> 2/2 passed (the `state` literal
pins and the `pack` suite that mints the refusal cases); `build/gate/dev/pack.txt` -> `1..24`,
24 `ok`, 0 `not ok`; `git status --porcelain` empty at the end.

## Trust Boundaries

| Boundary | Description | Data Crossing |
|----------|-------------|---------------|
| operator's flat file -> the tool | untrusted bytes and an operator-stated address cross here; nothing is detected, everything is checked | a byte count, a base, a word order |
| operator's `<out>` path -> the filesystem | the tool creates exactly one file at a path the operator names, and must not create or destroy any other | a derived container, and nothing else |
| tool's stdout -> the runner's report | the printed digest is the identity an operator compares with `slot.cart.sha256` | a 64-hex digest of derived material |
| `tools/flatwrap/*.c` -> the build and the SPDX rule | a tracked source the build does not compile, or one outside the SPDX rule, is a gate hole | source-list membership, SPDX headers |
| profile data -> the core's CPU config | a profile scalar decides whether acceptance clears the enable; the machine must take it from the profile and not from the library default | one bit, one release latency, one wait limit |
| profile data -> the parameter digest -> the snapshot | the same scalar reaches four table body digests, one pinned digest and four pinned snapshot literals | four 32-byte body digests, one 134-byte preimage |
| an observation -> a claim | an owner-local run is evidence of behaviour on one build, never a hardware probe | a default, a tag, a discrepancy row |
| operator's cartridge -> the route -> the core loader | an untrusted container crosses here; the route forwards a path and the core decides legality | a container path, a digest |
| the route's temporary scenario copy -> the runner | a generated file decides which roles are user content, so a conditional insertion is a behaviour switch | `content cart user`, one line |
| the lock file -> the load | two digests the core checks; the cartridge is deliberately not in it | `exec`/`grom` digests only |
| a requirement's status -> the registry's must-be-covered set | the traceability table is parsed, so a status and a row that move apart in either direction is a finding | requirement ids, gate names |
| `.planning/ci-runs.tsv` -> the checkpoint verdict | the arithmetic input is the committed file; a quoted figure certifies a number nobody measured | `cpu_s`, `build_s`, `gate_s` |
| a gate log -> the record | the log is rewritten by every run, so a second run destroys the first run's evidence | one container run |

---

## Threat Register

*25 register rows across the four plans resolve to 22 distinct ids: `T-14-SC` is carried by all four
plans (one shared threat, verified once), and `T-14-01`..`T-14-21` are each carried by exactly one
plan (14-01: 01-06; 14-02: 07-11; 14-03: 12-16; 14-04: 17-21).*

| Threat ID | Plan(s) | Category | Component | Severity | Disposition | Mitigation | Evidence | Status |
|-----------|---------|----------|-----------|----------|-------------|------------|----------|--------|
| T-14-01 | 14-01 | Tampering | `tools/flatwrap/flatwrap.c`, size arithmetic | high | mitigate | the word count is decided from the exact byte count **before** any multiplication and before any read; the read buffer is bounded at 131,074 bytes; `len_words` is written only from a count that rule 5 has already proved encodable | code `tools/flatwrap/flatwrap.c:322-325` (`size/2 > FW_WORDS_MAX`, `FW_WORDS_MAX` = `IVS_IVSC_ADDRESS_SPACE_WORDS` at `:93`), buffer `fw_in[FW_INPUT_MAX=131074]` at `:90,:97`, entry write at `:217`; **live** a 131,074-byte input -> `flatwrap-words` exit 1 (never read); 65,536 words at `4000` -> `flatwrap-cross` exit 1, so `n_words` on the write path is <= 49,152 at `$4000` and `fw_put16` can never wrap to 0; 49,152 words at `4000` accepted, container 98,368 bytes, entry bytes `40 00 c0 00`; 49,153 words -> `flatwrap-cross`. The plan's literal 65,537<->65,536 "refuse vs accept" pair is a **token pair** by arithmetic (`IVS_IVSC_MAX_SEGMENT_WORDS` is 65,535 and `$4000 + 65,536 = $14000` crosses first); the deviation is recorded in the file header `:49-55` and pinned in `data/expected/tests/pack.ids:35-38` | closed |
| T-14-02 | 14-01 | Tampering / Elevation | the output-path rule | high | mitigate | the **parent directory** is resolved physically and compared against the configure-time tree root with the trailing separator required; `refs/` is the one documented exception and it is matched on the whole segment, not a prefix | code `:171` (`realpath` of the root), `:188` (`realpath` of the parent), `:190-195` (root compare with `phys[rootlen] == '/'`), `:196` (`strcmp(phys+rootlen,"/refs")` **or** `strncmp(...,"/refs/",6)` -- so `refs2/` and a bare `/refs`-prefix do not pass); **live** four spellings all -> `flatwrap-output-tree` exit 1: literal `$PWD/build/x.ivsc`, `$PWD/tools/../build/y.ivsc`, `./build/rel.ivsc` from inside the tree, and `$T/linkdir/s.ivsc` where `linkdir` is a symlink to `$PWD/build`; `$PWD/refs/audit-probe.ivsc` **accepted** (exit 0, 66 bytes) and removed again, and `refs/` is gitignored (`.gitignore:2`) so nothing written there can be committed | closed |
| T-14-03 | 14-01 | Tampering | the output file open | high | mitigate | exclusive create, never a truncating open; the probe is a `stat` and the create is what actually enforces the rule | code `:228` `fopen(out, "wbx")` (the portable `O_CREAT|O_EXCL` form), `:231` the `EEXIST` branch mints the same token, `:352` the `stat` probe; **live** a pre-existing file -> `flatwrap-output-exists` exit 1 and `cmp` byte-identical against a pre-run copy; a FIFO as the output -> the same token, exit 1 (not a hang) | closed |
| T-14-04 | 14-01 | Tampering | a failed write | medium | mitigate | write, check the result, `remove()` on failure and exit non-zero; every refusal is required to leave no output file | code `:236-245`: `ok = fwrite(...) == total`, `fclose` folded into `ok`, `remove(out)` then exit 2; **live** `tests/pack-tap.sh:208-209` asserts `[ ! -e "$f_out" ]` for every non-`keep` refusal, and all eight refusal cases pass (`build/gate/dev/pack.txt` 24 `ok`) | closed |
| T-14-05 | 14-01 | Information disclosure | the tool's output and its digest | high | mitigate | the output goes only to the path named and is refused inside the tree; the extension is already refused by the ignore file and the binary extension list; the tool's only digest output is its one stdout line; the phase commits no image byte and records no digest of an operator image | code `:245-250` (one `printf` loop over the 32 digest bytes, `\n`, nothing else); `:342-355` (tree + exists before the create); `.gitignore:16` `*.ivsc`, `tools/check/binary-ext.txt:5` `ivsc` (both byte-identical across the phase -- `git diff --name-only abc0ceb..HEAD -- tools/check/binary-ext.txt .gitignore` empty); **live** `git ls-files \| grep -cE '[.](ivsc\|cfg)$'` -> 0; `git status --porcelain --ignored=matching --untracked-files=all` -> 0 lines matching `[.]ivsc$`; `git diff --name-only abc0ceb..HEAD \| grep -c '^fixtures/'` -> 0, so no fixture was added or edited; `fixtures/FX-owner-exec/manifest.json` and `FX-owner-grom` are `"committed": false` with `"outputs": []` (no digest of an owner image anywhere) and `git ls-files refs/` is empty | closed |
| T-14-06 | 14-01 | Tampering | `tools/flatwrap/*.c` coverage | high | mitigate | the c-build module list, the source-list writer, the toy tree's seventh list, the unlisted-source control and the SPDX alternation all land in the tool's own commit | code `tools/check/steps/c-build.sh:51` (`'^tools/flatwrap/.*[.]c$ ivs-flatwrap-sources.txt'`); `tools/flatwrap/CMakeLists.txt:23-29` (the repo-relative writer); `tools/check/controls/_good/tree/CMakeLists.txt:16` (the seventh empty list); `tools/check/controls/c-build-unlisted-flatwrap-source/{CHECK,EXPECT}` + `overlay/tools/flatwrap/extra.c`; `data/expected/controls/runner-hygiene.ids:21`; `tools/check/integrity.cmake:77` (`^tools/(pack\|cart\|pcm\|asm\|bench\|flatwrap)/`); **live** gate prints `-- c-build pass cc=AppleClang c-presets=2`, `-- integrity pass files=2747`, `control ok c-build-unlisted-flatwrap-source`, `controls=402/402` | closed |
| T-14-07 | 14-02 | Repudiation | the C default vs the profile | high | mitigate | the default moves to 1 and its comment states that the machine overrides it from the profile; every vector that rests on the not-cleared setting states it explicitly, so no expectation inherits a value nobody stated | code `core/src/cpu/cp1610.c:231-237` (`out->interrupt_accept_clears_intff = 1u;` with the comment "the machine overrides every field below from the profile registry (ivs_mm_profiles[])" and the profile's value named), `:1233-1237` (`clears = cfg->interrupt_accept_clears_intff`, the fault override below it), `:1357` (`ivs_is_bit` validation); `data/profiles/intv-2609-ntsc-family-p1.json:174-180` (`"value": 1`, tag `unknown-default-chosen`); **live** `grep -c '"config": {' data/cp1610/vectors/interrupt.json` -> 7, and every one states the bit (six `0`, one `1`), so no vector inherits the default; `ctest -R '^(state\|pack)$'` green | closed |
| T-14-08 | 14-02 | Repudiation | the re-pinned literals | high | mitigate | the parameter digest is derived by a path other than the one that produced it, and that method is required to reproduce the **pre-flip** value before it is trusted for the new one; both digests go in the commit message | **derivation, re-run here**: rebuilt the 134-byte preimage (`IVSP` + big-endian `u16` 1 + the four body digests in memory-map/PSG/STIC/controller order) in Python from `build/dev/core/gen/ivs_{profiles,psg_params,stic_params,ctrl_lines}.inc` -> `c6e522ee..2413ed`, `f19494a3..b2f88d`, `ad2585ee..3a58bf`, `9b00c1c4..bd7848` -> **`efba85009a57e91ad21060742dadc34949b976829525bb5e18d6b0897af1425c`**, exactly the pin at `data/expected/ivsn-literals.ids:49`; commit `f795f2a`'s message carries both values (`89ad6507..c0522 -> efba8500..f1425c`), the four body digests, the recipe and the statement that the same method with the field at 0 reproduces `89ad6507..c0522`; **live** `ctest -R '^state$'` passes, so the pin and the core agree. *The one live page that still quotes the old value is recorded under Unregistered Flags.* | closed |
| T-14-09 | 14-02 | Tampering | `run.stub2.jd-as-j` | medium | mitigate | the control is re-expressed as a comparison against the JD run in the same frame, not edited to a number another id already asserts; the case fails if the J run stops taking the same number of entries | code `tests/run/run_stub2.c:299-345`: the JD half asserts `fjd.entries == 1u` (`:318`), the J half patches image word 1005 `0352 -> 0350` and **checks the mutation really landed** (`:335`), then asserts `fj.entries == 1u && fj.entries == fjd.entries` (`:341`) plus R6/R7/R7-tuple equality -- a structural comparison, not a copied literal; the old premise is recorded in the file's header `:17` and the measured 179-vs-1 is in `docs/diagnostics.md:313`; **live** gate green (`ctest-runs=2`, `tests-missing=0`) | closed |
| T-14-10 | 14-02 | Repudiation | the observation becoming a claim | high | mitigate | the tag stays `unknown-default-chosen`, C41 stays `open`, and the claim counts do not move: an observation moves a default, never a tier | `data/profiles/intv-2609-ntsc-family-p1.json:176` `"tag": "unknown-default-chosen"` with `:177` cite naming the owner-local observation (D-51) and saying no source gives it, `:179` `resolving_evidence` still the D10 hardware probe; `data/discrepancies.json` C41 `status = "open"`, `topic`/`default_policy`/`note` all carry the 2026-11-26 inline pointer (WR-02's fix at `94300ec`), note length 2,947 with the original sentence quoted as what it read before; **live** gate `claims=28 provisional=27`, `discrepancies=110 pending-resolvers=20 open=101`, `tags=10 tiers=8` -- unmoved | closed |
| T-14-11 | 14-02 | Tampering | the timing-notes consequence sentence | medium | mitigate | the live document gets an inline pointer at the wrong sentence **and** an appended amendment, so no reader takes the 250-entries-per-frame consequence as current at the profile's value | `docs/stic/timing-notes.md:105` carries `[Amended by A-14-02 at the end of this page (2026-11-26, D-51, C41): the profile now carries 1, so this consequence holds only with the field stated as 0.]` inline on the sentence itself, and `:113` opens `## Appended amendment A-14-02 ...`; the same two-limb pattern is measured on the four other live pages the flip made wrong -- `docs/cp1610/second-reader-policies.md:109` + `:171-175`, `docs/cp1610/stepper-conventions.md:63` + `:68`, `docs/state/hard-points.md:306`, `docs/diagnostics.md:94` + `:309`; **live** `grep -rn interrupt_accept_clears_intff --include=*.md .` outside `.planning/` returns only amended sites | closed |
| T-14-12 | 14-03 | Information disclosure | the six derived-output options with user content | high | mitigate | all six are asserted individually for a `content cart user` scenario: exit 14, one exact stderr line, no report, no file; the three audio options are also given together so the fixed order's first option is proved to be the one named | code `tests/runner-process-tap.sh:1729-1738` (`cu_refuse`) + `:1739-1744` (six calls): `cu_refuse` asserts `rc -eq 14`, `hung -eq 0`, empty stdout, `cmp` against the exact single stderr line, and `[ ! -e "$3" ]`; six calls (`--frame-ppm`, `--video-raw`, `--audio-raw`, `--psg-log-out`, `--audio-wav`, `--update-golden`); `:1747-1755` the audio trio asserts the `--audio-raw` line and all three absent; **live** gate green with `runner-process` 138 ids, 0 `not ok` | closed |
| T-14-13 | 14-03 | Tampering | the conditional `content cart user` insertion | high | mitigate | the helper asserts exactly one `content cart user` line when a cartridge is given and none when it is not, so the switch cannot silently invert | code `tools/owner-run.sh:93` (the `awk` inserts the line only `if (c != "")`), `:97-99` (`want=1` only when `$cart` is non-empty, then `grep -c -x 'content cart user'` compared with `-eq "$want"` and `die` on a mismatch); `docs/owner-images.md:38` states the same rule for the reader; **live** `runner-process.judge.owner-run-cart-pass` passes, and `14-03-SUMMARY.md:318` records the measured stand-in (an `awk` that always inserts is refused by the count assertion, exit 2) | closed |
| T-14-14 | 14-03 | Repudiation | the round trip's digest equality | high | mitigate | the equality is proved live by flipping one input byte and requiring both digests to change, and the second half fails if they are equal, so a constant comparison cannot pass | code `tests/runner-process-tap.sh:1786-1827` (flip the first word's low half `00 -> 01`, both digests must move) and `tests/pack-tap.sh:386-435` (`pack.judge.flatwrap-digest-live`: same input twice equal, `cmake -E sha256sum` of the whole file, trailer cut with `dd` and hashed, then the flipped input must differ, with the literal `judge-vacuous:` finding when it does not); **live** my own round trip: printed digest `23db1e49..d65857` = `slot.cart.sha256` = `cmake -E sha256sum` of the 8,256-byte container, `slot.cart.loaded 1`, `outcome pass`, `exec: -` | closed |
| T-14-15 | 14-03 | Tampering | the lock's role arm and the positional arity | medium | mitigate | `--cart` is an option, the lock keeps two roles, and the positional arity stays exactly two so a cartridge cannot be passed as a third positional | code `tools/owner-run.sh:37` (`--cart` in the option loop), `:42` `[ "$#" -eq 2 ] || die "$usage"` (a third positional is a usage refusal), `:62` the lock path is `refs/owner-images.sha256`; **live** `tests/runner-process-tap.sh:2082` `or_badlock 'owner-run: bad lock line: the role is not exec or grom' "$or_z  exec" "$or_z  cart"` -- a `cart` role line is still refused, and the case is `ok` in the 138-id suite; `:1977` `--cart` with no value is the usage refusal; `docs/owner-images.md:58` states "not a third positional" | closed |
| T-14-16 | 14-03 | Information disclosure | an operator image byte or digest reaching the tree | high | mitigate | the container and the images live in a scratch root outside the work tree, the scratch root's file listing is compared before and after, no tracked path carries the extension, and the report's `exec:` label stays `-` | code `tests/runner-process-tap.sh:1709-1717` (everything under this run's `mktemp -d` `$tmp` and its `$cuc` subdirectory), `:1768-1782` (`ls -A` before/after `cmp -s`, so the hook run may add no file), `:1814` and `:2167` the `exec: -` assertions; `tools/check/binary-ext.txt` and `.gitignore` byte-identical across the phase; **live** `git ls-files` carries no `.ivsc`/`.cfg`; the ignored-status listing carries no `.ivsc`; `fixtures/FX-owner-*` are `committed:false` with `outputs: []`; my own run's report shows `exec: -` | closed |
| T-14-17 | 14-04 | Repudiation | a requirement flipped without its row, or a row without its flip | high | mitigate | the three flips and the five rows land in one commit, and the step fires in both directions | `git show --stat 548e80d` carries `.planning/REQUIREMENTS.md` (the three checkboxes and the three traceability rows) **and** `data/expected/important-gates.ids` (+32 lines) in the same commit; the five rows are `data/expected/important-gates.ids:549` (`flatwrap control c-build-unlisted-flatwrap-source TOOL-07`), `:552` (`flatwrap mutant pack.judge.flatwrap-digest-live TOOL-07`), `:555` (`cart-role mutant runner-process.judge.flatwrap-roundtrip SCN-03`), `:561` (`cart-role mutant runner-process.judge.owner-run-cart-pass SCN-03`), `:565` (`cpu06 fault ACCEPT_CLEARS_INTFF_IGNORED CPU-06`); **live** `-- important-gates pass gates=117 lines=356` with `rows-pending=0 policies-pending=0`, and the two committed controls `important-gates-unknown-id` / `important-gates-complete-uncovered` are `ok` inside `controls=402/402` | closed |
| T-14-18 | 14-04 | Repudiation | the cost checkpoint | high | mitigate | the five `cpu_s`, `build_s` and `gate_s` values are printed beside the verdict with their sorted middle values, recomputed from the committed TSV; the frozen figures are named stale by measurement; the verdict is one of the three rule words with its arithmetic shown | `14-04-SUMMARY.md:230-265`: the five-row table, `cpu_s` sorted `357 360 363 368 438 -> median 363`, then all three rule words evaluated with their thresholds (`discharged-within-allowance` needs `<= 362.535`; measured 363 is 0.465 above, so it is unreachable whatever the token line did; `lever-required` needs cumulative spend past 21; `reprice` is what remains) -- one verdict, `reprice`, with the arithmetic; the frozen 361/34/118 are named stale by measurement in `14-01-PLAN.md`'s start reading; **live** `.planning/ci-runs.tsv` last five recomputed here: `cpu_s` median 363, `build_s` 35, `gate_s` 121 -- the same figures the record prints | closed |
| T-14-19 | 14-04 | Tampering | a differing gate token | medium | mitigate | every predicted token is compared against the fresh line, a differing token is chased to named commits and recorded, never re-blessed, and the invariants are stated to be where they were frozen | `14-04-SUMMARY.md:102-151` (105 tokens compared token-by-token; exactly four differ in the container run and all four are host tokens -- `cc=GNU`, `core-imports=5`, `archive=sha256`, `refs-verified=0/4`), `:417-431` (the three tokens that differ at the fix head, each with its named cause: `lines=356` = WR-03, `tests-expected=5016` = CR-01, `tracked=2747` = the two record files), `:424-431` (the plan's own predicted `lines=357` is explicitly **not** re-blessed); **live** my run reproduces the fix-head line exactly, including `steps=38/38 controls=402/402 faults=238/238 goldens=29 gates=117 lines=356 tests-expected=5016` | closed |
| T-14-20 | 14-04 | Repudiation | the container gate log | medium | mitigate | one container gate at a time, the log copied aside before anything else runs, and no resource left behind | `14-04-SUMMARY.md:94-100`: `sh tools/gate-linux.sh`, image tagged `ivs-gate-linux:14-04` and the tag removed, `docker run --rm`, `build/gate-linux.log` copied to `/tmp/g14linux.log` before anything else ran, `gate-linux: exit 0`; **live** `docker images --format '{{.Repository}}:{{.Tag}}' \| grep '^ivs-'` and `docker ps -a --format '{{.Names}}' \| grep '^ivs-'` both return nothing (rc 1) -- no image, container or volume named `ivs-*` survives | closed |
| T-14-21 | 14-04 | Tampering | toy coverage silently deleted | medium | mitigate | three appended lines per file across all ten registries and three rows per toy requirement record, zero deletions, no `DEVIATION`; both drift directions are refused | `git show --numstat 548e80d`: thirteen toy paths, every one `+3 -0` (30 registry lines) plus three requirement records at `+3 -0` (9 rows), **zero deletions** anywhere in the commit's toy set; **live** `find tools/check/controls -name DEVIATION` returns only the nine pre-existing declarations from earlier phases (none under any Phase 14 control), and `control-toy-drift` / `control-deviation-unused` are `ok` inside `controls=402/402` | closed |
| T-14-SC | 14-01, 14-02, 14-03, 14-04 | Tampering | npm/pip/cargo installs | high | mitigate | no package is installed by any of the four plans; the tool links two in-tree targets that `tools/pack` already links | `tools/flatwrap/CMakeLists.txt:20` `target_link_libraries(ivs_flatwrap PRIVATE ivs_core ivs_core_internal ivs_warnings ivs_san)` -- four in-tree targets, no `find_package`, no `FetchContent`, and `grep -c install` on that file returns only a comment (`:5`); **live** `git diff --name-only abc0ceb..HEAD` over package/lock/build-dependency paths returns only `.planning/REQUIREMENTS.md`-family files (no `package.json`, no lockfile, no `vcpkg`/`conan`, no `pyproject`); `core-imports=7` and `defined=141 public-symbols=19` unmoved, so no new core symbol was exported for the tool | closed |

*Status: closed / open / open -- below {block_on} threshold (non-blocking). Severity order:
critical > high > medium > low; only open threats at or above `high` count toward `threats_open`.*

---

## The four code-review findings, re-verified as fixed

All four were re-run rather than taken from `14-REVIEW.md`'s disposition line, because each one is
exactly the shape this phase exists to close: a refusal that does not fire, a probe that never
returns, and a record that still asserts the old value.

| Finding | Fix commit | Re-measured this session |
|---------|-----------|--------------------------|
| **CR-01** -- a zero-byte input was accepted and produced a 64-byte container the project's own parser refuses (`IVS_IVSC_MIN_BYTES` is 66 and `len_words` must be at least 1) | `bf3215d` | code `tools/flatwrap/flatwrap.c:313-316` refuses `flatwrap-empty` as **rule 1**, before any byte-count arithmetic, with the reason in the message; the token is **appended** to `data/expected/flat-refusals.ids:25` (the list is now eight, not seven) and minted into `pack.flatwrap.neg.flatwrap-empty` (`data/expected/tests/pack.ids:30`) with one word as its boundary positive. **live** `: > e.bin; ivs_flatwrap e.bin 4000 e.ivsc` -> `ivs_flatwrap: flatwrap-empty: ...` exit 1, no output file; the 2-byte positive -> exit 0, 66 bytes, entry bytes `40 00 00 01`; `build/gate/dev/pack.txt` -> `1..24`, 24 `ok`, 0 `not ok`; `pack.judge.flatwrap-tokens` still closes (the pin, the source literals and the observed set are the same eight). Counts moved exactly as the amendment said: `tests-expected=5016 tests-executed=5012` |
| **WR-01** -- both operator paths were opened with `fopen`, so a FIFO hung the tool forever | `4efc806` | code `:292` `open(flat, O_RDONLY\|O_NOCTTY\|O_NONBLOCK)`, `:297` `fstat` + `S_ISREG` refusal (the `runner/hostio.c` `ivs_hostio_read` technique), `:352` the existence probe is a `stat` with **no open at all**, and the `wbx` create at `:228` still enforces the rule. **live**, each under `timeout 5`: FIFO as output -> exit **1** `flatwrap-output-exists` (was exit 124); FIFO as input -> exit **2** `is not a regular file` (was exit 124); an ordinary 2-byte input still accepted; an existing file still refused and `cmp` byte-identical. No gate case was added (the hazard is a host-environment one and a §2.4 row would move `controls`), which is recorded in the review's disposition and left as residual R-14-B below |
| **WR-02** -- the C41 row still asserted the deleted default in three present-tense fields | `94300ec` | `data/discrepancies.json` C41 read back: `topic` carries `[value chosen as 1: see the 2026-11-26 amendment at the end of this entry's note, D-51, C41]`; `default_policy` keeps its schema prefix and now says "interrupt acceptance **clears** INTFF ... profile field ... = 1, with vectors for both values"; `note` opens "holds **1 (true)** ... [amended 2026-11-26, D-51: this sentence read 'holds 0 (false)' before the flip ...]" with the 2,947-character amendment intact. `status` is still `"open"`, the `unknown-default-chosen: ` prefix is still there, and `discrepancies=110 open=101` is unmoved. **live** the gate's `discrepancies` step passes |
| **WR-03** -- a `pass 0` row was registered as one of SCN-01's must-fail mechanisms | `fc25ce9` | `data/expected/important-gates.ids:326` now states the reason in the block comment and the row is **deleted**: `awk '!/^#/ && NF && /scn-mutants/' data/expected/important-gates.ids` -> **11** rows (every one a still-failing mutant), and `SCN-STUB-JD-AS-J` survives only as a comment word plus its live inventory row at `data/expected/diag-mutations-scn.ids` (2 occurrences, both the case's own definition and its stated outcome). **live** `-- important-gates pass gates=117 lines=356`: `lines` moved 357 -> 356 and `gates` did not move, because the step counts distinct gate names and `scn-mutants` keeps eleven rows -- the review's own Fix text predicted `116`/`356` and its corrected arithmetic is the right one |

---

## Unregistered Flags

Two findings surfaced during this audit that map to **no** register row. Neither is a control gap in
the sense the register declares, neither is at or above `block_on: high`, and neither is counted in
`threats_open`. They are recorded so they are not lost, not accepted.

| Flag | Severity | What is wrong | Evidence | Recommended action |
|------|----------|---------------|----------|--------------------|
| **UF-14-01** -- a live format page still asserts the pre-flip parameter digest as the current value | medium | `docs/formats/ivsn-v1.md` is a live page a reader consults for the container's specification. It states, in the present tense, "Its value for the registered profile is `89ad65073a3e357acbdd687f6a29016457864a1666b953e9327b7559dc0c0522`, pinned as `params-digest-sha256` in `data/expected/ivsn-literals.ids`" -- a sentence that is now false in both halves, since the pin reads `efba8500..f1425c`. The page was **not** touched by this phase (`git log --oneline -5 -- docs/formats/ivsn-v1.md` -> last commit `3fe5373`, Phase 8) and carries no inline pointer and no amendment, so the GUARDRAILS §2.10 two-limb treatment the phase applied to five other live pages was not applied to this one. T-14-11 declares only "the timing-notes consequence sentence", and T-14-08 declares only the derivation of the literals, so no row covers this site. Impact is a repudiation one: a reader or a later agent could quote a value the project's own pin contradicts, and no gate step compares a doc-quoted digest with a pin (the `state.literal.params-digest` test holds the **pin** to the core, not the page). | `docs/formats/ivsn-v1.md:245`; `data/expected/ivsn-literals.ids:49`; my independent rebuild of the preimage (see T-14-08) returning `efba8500..f1425c`; `ctest -R '^state$'` green, so the pin is the live one and the page is the stale one | Correct the sentence to `efba8500..f1425c` and give it the same inline pointer + appended amendment shape the other five pages got, in the next docs or records pass. Cheap, one page, no code and no gate token |
| **UF-14-02** -- a signed validation record still quotes the pre-CR-01 `pack` id count | low | `14-VALIDATION.md` states the `pack` suite's size as **23 ids** in two places, both written before CR-01's added case. The file now holds 24 and the suite runs 24. The review's amendment corrected `tests-expected` (`5014 -> 5016`) but not these two sentences, so the record still carries a figure that disagrees with the file it names -- the class `AGENTS.md` calls "a defect to correct, not a fact to reuse". | `14-VALIDATION.md:50` (Per-Task row `14-01-T2`: "`ctest --preset dev -R '^pack$'` -> 23 ids, no `not ok`" and "`data/expected/tests/pack.ids`, 23 ids") and `:68` (Wave 0 bullet: "(23 ids: 14 existing plus the nine; 23 `ok`, no `not ok`)"); `awk '!/^#/ && NF' data/expected/tests/pack.ids \| wc -l` -> **24**; `build/gate/dev/pack.txt` -> `1..24`, 24 `ok` | Amend the two sentences to 24 (14 existing + nine + CR-01's `flatwrap-empty`) in the same pass. No threat, no control and no token is involved |

---

## Accepted Risks Log

No threat in the register is `accept` or `transfer`; every row is `mitigate` and closed. The rows
below are **non-register residuals** -- they carry no threat id, are all below the `high` block
threshold, and are recorded here so they are not re-litigated as fresh findings. The first three are
`14-REVIEW.md`'s deferred INFO rows, whose deferral reasons this audit re-read and agreed with; the
last two are auditor observations the review did not make.

| Risk ID | Threat Ref | Severity | Rationale | Accepted By | Date |
|---------|------------|----------|-----------|-------------|------|
| R-14-A | -- (`14-REVIEW.md` IN-01) | low | One refusal token carries two different detail texts: the `stat` probe says `the output <path> already exists` and the `EEXIST` branch inside `fw_write` passes the bare path, so the published stderr line differs by which branch won. Both are correct behaviour, both are `flatwrap-output-exists`, and `tests/pack-tap.sh:206` asserts the token prefix only, so a change here is invisible to the gate. Fixing it edits the published line in the same round that rewrote the probe; it belongs with a case that asserts the detail, which is a new id. | owner-delegated autonomous operation (AGENTS.md); next change to the tool's published lines | 2026-11-26 |
| R-14-B | -- (`IN-02`) | low | `--cart ""` is silently treated as "no cartridge": the empty value passes argument parsing, is skipped by the location loop (`tools/owner-run.sh:49`), and leaves `want=0`, so the operator gets a report that looks like a deliberate no-cartridge run. Every other malformed argument is a usage refusal, so this is the one silent branch. Measured today: `--cart` with **no** value **is** refused (`:37`), so only the explicit empty string is affected. Turning it into a usage refusal is a behaviour change in the owner route that needs its own case id, and this round may not add one. | owner-delegated autonomous operation; next owner-route plan | 2026-11-26 |
| R-14-C | -- (`IN-03`) | low | The `refs/` exception is the one way the tool may write inside the work tree, and **no committed case exercises it** -- the suite covers three refused spellings and one outside positive. A regression in either direction (widening to `refs2/`, or a change that starts refusing `refs/` itself) would leave the gate green. This audit did exercise the predicate live (a `refs/` output accepted, then removed; four inside-tree spellings refused), so the limb is *shown* working but not *pinned*; pinning it needs either a write into the real `refs/` or a new pure-helper unit test, both of which are a new test surface and would move `controls`. The exception is documented as deliberate (D-47, R-2) in `tools/flatwrap/flatwrap.c:57-59` and `docs/owner-images.md:52`, so a later reader is not left with an unexplained hole. | owner-delegated autonomous operation; deferred with the review's reason | 2026-11-26 |
| R-14-D | -- (auditor observation) | low | The wrap tool's work-tree rule protects the **directory** the output lands in, not a final component that is a symlink. Measured and closed for the reachable shapes: `stat` follows a symlink, so an existing target is refused `flatwrap-output-exists`, and `O_CREAT\|O_EXCL` refuses a symlink whose target does not exist, so a dangling link cannot be used to create a file inside the tree. The residual is that this is enforced by the C library's `O_EXCL` semantics rather than by a stated rule of the tool's own, and no case asserts the dangling-link shape (`14-REVIEW.md` measured it by hand). | owner-delegated autonomous operation; recorded rather than narrowed | 2026-11-26 |
| R-14-E | -- (auditor observation) | low | The `flatwrap` token-coverage judge extracts the printable tokens by grepping the source for `"flatwrap-[a-z-]*"` literals (`tests/pack-tap.sh:373`), so a token the source builds from fragments, or one spelled outside that character class, would escape the diff. Measured today: the tool prints all eight from string literals of exactly that shape and `pack.judge.flatwrap-tokens` closes against both the pin and the observed set, so nothing is currently unguarded; the residual is that the detector is a lexical match on the file it guards. | owner-delegated autonomous operation; next change to the tool's token set | 2026-11-26 |

*Accepted risks do not resurface in future audit runs.*

---

## Security Audit Trail

| Audit Date | Threats Total | Closed | Open | Run By |
|------------|---------------|--------|------|--------|
| 2026-11-26 | 22 (25 register rows) | 22 | 0 | gsd-security-auditor (pi), ASVS L1, `block_on: high` |

**What was executed, not inferred.** `sh tools/gate.sh` (exit 0, zero `^-- FAIL`, `gate-time
total=97`); `ctest --preset dev -R '^(state|pack)$'` (2/2 passed) and the `pack` suite's own TAP file
(`1..24`, 24 `ok`); thirteen live invocations of `build/dev/tools/flatwrap/ivs_flatwrap` covering all
eight refusals and six boundary positives (empty, 1 byte, 2 bytes, `0400`, `03FF`, `3FFF`, `4000`,
65,537 words, 65,536 words, 49,153 words, 49,152 words, a literal in-tree path, a `..` spelling, a
relative spelling, a symlinked directory, the `refs/` accept, an existing file with a `cmp`
before/after, a FIFO as output, a FIFO as input); one end-to-end round trip through
`build/dev/runner/ivs-run` with `--cart` (`outcome pass`, `slot.cart.loaded 1`, the printed digest
equal to `slot.cart.sha256` and to `cmake -E sha256sum`, `exec: -`); one independent rebuild of the
134-byte parameter-digest preimage in Python from the four generated body digests; the C41 row read
back from `data/discrepancies.json`; the five `important-gates` rows and the eleven surviving
`scn-mutants` rows counted from the committed registry; `git show --numstat 548e80d` for the toy
append and the one-commit flip+row pair; `git log -1 --format=%B f795f2a` for the two digests and the
recipe; the tracked/ignored/tree scans for `.ivsc`, `.cfg`, `fixtures/`, `refs/` and package files;
and `docker images` / `docker ps -a` for leftover `ivs-*` resources.

**Nothing was written outside this file.** The tree was inspected read-only; the one file this audit
created outside the build tree was the `refs/` probe under the tool's own documented exception, and
it was removed in the same command block. `git status --porcelain` was empty before this file was
written; no branch was switched, no stash was made and nothing was committed.

---

## Sign-Off

- [x] All `<required_reading>` loaded before any analysis (four plans, four summaries, the review, the context, GUARDRAILS v0.3 and `AGENTS.md`)
- [x] Threat register extracted from the four plans' `<threat_model>` blocks: 25 rows, 22 distinct ids
- [x] Each threat verified by its disposition (`mitigate` x 22; no `accept` or `transfer` row)
- [x] Threat flags from the summaries incorporated -- `14-03-SUMMARY.md` and `14-04-SUMMARY.md` each carry a "Threat surface scan" section reporting no new boundary, which this audit confirmed independently; `14-01-SUMMARY.md` and `14-02-SUMMARY.md` carry none, and no new surface was found for them either
- [x] The two ids the §2.4 deviation does **not** register are recorded as owing no row, not silently missing: `14-03-SUMMARY.md`'s "Deferred Issues" names the two `cart-role mutant` rows as deferred to 14-04 (they landed at `important-gates.ids:555,561`), and `14-04-SUMMARY.md:186-194` names `pack.judge.flatwrap-tokens` and `runner-process.cart-user.refuse-derived` as ids that owe no §2.4 row, with the reason each one has no stand-in limb
- [x] The four review findings re-verified as fixed by re-running them, not by citing the disposition
- [x] Implementation files never modified; **no file written but this one**
- [x] `threats_open: 0` confirmed -- no open threat at or above `high`, and the two unregistered flags are both below it
- [x] Each mitigation is shown working by a command, a derivation or a `file:line`, not by the plan's claim

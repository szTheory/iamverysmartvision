---
phase: "15"
slug: "build-only-host-and-the-fake-frontend-harness"
status: secured
# threats_open = count of OPEN threats at or above workflow.security_block_on severity (high).
threats_open: 0
asvs_level: 1
created: "2026-11-26"
audited: "2026-11-26T08:07Z"
auditor: "gsd-security-auditor (pi, subagent)"
---

# Phase 15 -- Security

> Per-phase security contract: threat register, accepted risks, and audit trail.

**Verdict: SECURED -- 22 register rows across three plans, 20 distinct threat ids, 20 closed,
0 open.** `block_on: high`, `asvs_level: 1`, requirement `TOOL-08`. No row is `accept` or
`transfer`; every row is `mitigate` and closed. **No unregistered flag at or above `high`**: one
low-severity record-accuracy flag is recorded below (UF-15-01), it maps to no register row, and it
does not count toward `threats_open`.

The phase's security property is the one its own CONTEXT names: a boundary a third party's frontend
could load, and a body of evidence that must not be able to pass while the boundary is wrong. Six
risks are therefore inverted relative to ordinary phases -- an export surface that grows unnoticed, a
harness that passes vacuously, a load that leaves the machine half-committed, a host that reaches a
file it was not handed, a shared object that leaks into the package or a release, and an input
schedule that silently drops events. Each mitigation below was **made to fire** or the exact
statement was read, never the plan's claim.

## Basis of this audit

Evidence classes, used consistently:

- **code** -- `file:line` where the refusal, the bound or the assignment is emitted.
- **live** -- a command run in this session against the committed tree, a `/tmp` scratch tree, or the
  built binaries (`build/{dev,release}/host/libivs_retro.dylib`,
  `build/dev/tests/ivs_host_harness`). **No source was modified.**
- **control** -- the committed stand-in that makes the check fail for its own reason, re-run here
  either through the step it names or through the suite's own judge.
- **review** -- a measurement `15-REVIEW.md` took that cannot be re-taken without a mutation this
  audit may not make; cited only where re-running it would mean editing source. Everything cheap was
  re-run anyway: the export check in both directions, the import limb, the four harness modes, the
  empty-poll limb against the idle golden, the two refusal surfaces, and the whole gate.

Final state measurement, one command, run before this file existed:

```
sh tools/gate.sh   ->  exit 0, zero `^-- FAIL` lines, gate-time total=77
gate: PASS steps=38/38 controls=404/404 ... cc=AppleClang c-presets=2 fault-runs=238
core-imports=7 ... faults=238/238 tracked=2773 binaries-listed=0 rights-pinned=4/4 ...
gates=118 lines=361 ... prefix-files=11 archive=members public-symbols=19 defined=141
hdr-alone=10/10 consumer-lines=164 cxx-link=ok pkg-selftest=20/20 ...
release-selftest=12/12 runner-files=18 cases=14 ... tests-expected=5042
tests-executed=5038 tests-skipped=4 tests-missing=0 ... claim-refs=27
```

The token line is **exactly** the one the five post-review commits carry (`gates=118 lines=361
tests-expected=5042`, last stated by `d45f87f`; the merge commit `0fc4d33` is the tree measured
here). The two tokens the plan predicted (`lines=358`, `tests-expected=5036`) are superseded by the
review's three added ids and are chased commit by commit, not re-blessed -- see T-15-17.

Also run this session, each on the committed tree:

- `sh tests/runner-process-tap.sh ...` -> trailer `# suite=runner-process executed=147 skipped=2
  failed=0`, **zero** `not ok`, and all eleven host ids `ok` (126-133, 138-140).
- `sh tools/check/host-symbols.sh --allow-prefix __asan_ --allow-prefix __ubsan_
  --allow-prefix __sanitizer_ build/{dev,release}/host/libivs_retro.dylib` -> `-- host-symbols pass
  exported=9 imports=0`, exit 0, on **both presets**.
- `sh tools/check/steps/{files,integrity,text,hygiene,important-gates,release-assets,pkg-install}.sh`
  -> `files pass tracked=2773 binaries-listed=0 rights-pinned=4/4`;
  `integrity pass files=2773`; `text pass docs=64 claim-refs=27`; `hygiene pass mode=tree
  hygiene-files=2773`; `important-gates pass gates=118 lines=361`;
  `release-assets pass release-selftest=12/12`; `pkg-install pass prefix-files=11 ... pkg-selftest=20/20`.
- The four harness modes and the empty-poll run against both goldens (T-15-07, T-15-08, T-15-09,
  CR-01, WR-01 below).
- `docker ps -a` / `docker images` filtered on `ivs-` -> empty (T-15-18).
- The last five `.planning/ci-runs.tsv` rows recomputed in Python -> the checkpoint's figures (T-15-16).

## Trust Boundaries

| Boundary | Description | Data Crossing |
|----------|-------------|---------------|
| the harness process -> the shared object | `dlopen` crosses from a plain process into code holding the whole machine; the path must be CMake-provided, never environment-provided | one absolute build path |
| the shared object -> the host process | the export surface is the object's published contract; every name on it is an ABI promise | nine names, and only nine |
| the shared object -> the filesystem | the core takes bytes, never paths; an import of `fopen`/`open`/`read`/`stat` would be a second content route around the runner's caps | three byte spans, no path |
| `data/expected/host-symbols.ids` -> the linker and the checker | one contract file feeds both, so artifact and pin cannot drift apart silently | nine names |
| the machine core -> the frontend's evidence | the nine digests are the only evidence, so every limb that could let a constant output pass is a boundary | nine 32-byte digests |
| the harness -> the judge | the judge reads the harness's stdout; a stand-in that differs in two places would pass for the wrong reason | one token set |
| a tracked file -> the tree and the release | a shared object is a rights-bearing and executable artifact; it must be refused by name in both places | a path and an extension |
| the install prefix -> the package inventory | the eleven-path list is the only thing between a build artifact and a shipped file | eleven relative paths |
| `host/` -> the integrity and licence rules | a new top-level directory is invisible to directory-anchored rules unless the alternation is widened | SPDX headers, ASCII rule |
| a requirement's status -> the registry's must-be-covered set | the traceability table is parsed, so a status and a row that move apart in either direction is a finding | requirement ids, gate names |
| a claim row -> the profile, the artifact and the tier ceiling | a claim naming a field the profile lacks, a handoff-shaped artifact, or a tier above `provisional` is a finding | one claim, one tier |
| `.planning/ci-runs.tsv` -> the checkpoint verdict | the arithmetic input is the committed file; a quoted figure certifies a number nobody measured | `cpu_s`, `build_s`, `gate_s` |
| a gate log -> the record | the log is rewritten by every run, so a second run destroys the first run's evidence | one container run |

---

## Threat Register

*The table below carries **one row per distinct id** (20). The 22 register rows across the three plans resolve to those 20 ids: `T-15-SC` is carried by all
three plans (one shared threat, verified once), and `T-15-01`..`T-15-19` are each carried by exactly
one plan (15-01: 01-06; 15-02: 07-13; 15-03: 14-19).*

| Threat ID | Plan(s) | Category | Component | Severity | Disposition | Mitigation | Evidence | Status |
|-----------|---------|----------|-----------|----------|-------------|------------|----------|--------|
| T-15-01 | 15-01 | Elevation of privilege | `tests/host/ivs_host_harness.c` `dlopen` path | medium | mitigate | the path is `$<TARGET_FILE:ivs_retro_shim>` handed over as the suite's seventh argument; the harness never reads a path from the environment, and a missing argument is a `not ok` finding, never a skip | code `tests/CMakeLists.txt:754-757` (the seventh argument is the built object; the eighth is the frontend), `tests/host/ivs_host_harness.c:351-354` (`shim_path = argv[1]`, `dlopen(shim_path, RTLD_NOW\|RTLD_LOCAL)`), `tests/runner-process-tap.sh:143-145` (a shared object is required to exist, not to be executable) and `:2303-2307` (`[ -z "$shim" ]` / `[ -z "$harness" ]` -> `ok=0; why host-shim`/`host-harness`, a finding); **live** `grep -rn 'getenv\|environ' host/ tests/host/` -> nothing; `ivs_host_harness` with no arguments -> `usage: ...`, exit 1; with `/nonexistent/libivs_retro.dylib` -> `fail dlopen ...`, exit 1; with a nonexistent content dir -> `fail images.exec ...`, exit 1 (no skip, no vacuous pass) | closed |
| T-15-02 | 15-01, 15-02 | Tampering | the export surface of `libivs_retro` | high | mitigate | the linker's export list is **generated from** `data/expected/host-symbols.ids` (`-Wl,-exported_symbols_list` on Darwin, `-Wl,--version-script` with `local: *` on GNU), so an unlisted name cannot be exported; `tools/check/host-symbols.sh` compares the built set in **both** directions, and the case proves **both** directions live | code `host/CMakeLists.txt:17-50` (`file(STRINGS ... REGEX "^[A-Za-z_][A-Za-z0-9_]*$")` then the two linker branches), `tools/check/host-symbols.sh:131-142` (`comm -23` -> `host-symbols-missing`, `comm -13` -> `host-symbols-extra`, and the palette grep over the exported set), `tests/runner-process-tap.sh:2572-2622` (the appended-name limb **and** the dropped-name limb, each required by name); **live** `nm -P -g --defined-only` on `build/dev` and `build/release` -> exactly the nine names of `host-symbols.ids` in both presets; `host-symbols.sh` with **no ids argument** -> `-- host-symbols pass exported=9 imports=0` exit 0 on both (the default resolves to the same file the linker reads); a scratch ids copy with `ivs_not_defined` appended -> `-- FAIL host-symbols-missing: ivs_not_defined` exit 1; a scratch copy with `retro_run` deleted -> `-- FAIL host-symbols-extra: retro_run` exit 1; the real link command carries `-Xlinker -exported_symbols_list -Xlinker build/dev/host/host-exports.txt` (nine `_`-prefixed names) | closed |
| T-15-03 | 15-01 | Information disclosure | the shim's undefined symbols | high | mitigate | `nm -P -u` minus `core-symbols-allow.txt` minus `data/expected/host-imports.ids` minus every `--allow-prefix` must be empty, so `fopen`, `open`, `read` and `stat` are refused by name; the shim takes three byte spans and opens nothing | code `data/expected/host-imports.ids` (allocator plus four named GNU crt stubs -- **by name**, never an `_ITM_*`/`__gmon_*` prefix, so the contract never reviewed is not silently allowed), `tools/check/host-symbols.sh:147-158` (the subtraction and `host-symbols-import`), `host/ivs_retro_shim.c:10-12` (the file's own statement that it opens nothing and keeps no clock); **live** `nm -P -u build/release/host/libivs_retro.dylib` -> `__memcpy_chk __memmove_chk __stack_chk_fail __stack_chk_guard bzero free malloc memcpy memmove memset realloc` -- no file, clock, environment, thread, randomness, assert, abort or libm name; the dev object adds only `__asan_*`/`__ubsan_*`; the checker on the harness binary refuses `fclose`, `fopen`, `fread` by name (43 disallowed imports in total), which is the limb shown able to fire; both presets `imports=0` | closed |
| T-15-04 | 15-01, 15-02 | Tampering | a tracked `.so`/`.dylib` entering the tree or the release | high | mitigate | `host/` has no `install()` line, the object is built under `build/`, plan 15-02 adds `so dylib dll bundle` to `tools/check/binary-ext.txt` with two controls and the `pkg-install` selftest, and the phase's definition of done runs `git ls-files` for those extensions | code `host/CMakeLists.txt:52-53` ("NO INSTALL RULE HERE, EVER"), `tools/check/binary-ext.txt:47-50`, `tools/check/controls/files-unlisted-binary-{so,dylib}/{CHECK,EXPECT,MAKE}`; **live** `grep -rn 'install(' host/ tests/host/` -> nothing; `git ls-files \| grep -Ei '\.(so\|dylib\|dll\|bundle)$'` -> empty; the objects live at `build/{dev,release}/host/libivs_retro.dylib`; `files pass tracked=2773 binaries-listed=0`; both controls re-run on a copy of the toy tree -> `-- FAIL files-unlisted-binary: docs/save.so` and `... docs/save.dylib`, exit 1, exactly the intended token and no other | closed |
| T-15-05 | 15-01 | Denial of service | unbounded host memory in the shim | medium | mitigate | the shim sizes its three buffers from `ivs_info`'s documented per-frame formulas exactly as `runner/exec.c`'s `ex_plan_buffers` does; a `BUFFER_TOO_SMALL` from the core is a shim failure returning 0, never a truncation | code `host/ivs_retro_shim.c:486-500`: `slots = delta / nominal_frame_ticks + 1u`, `video_need = video_bytes_per_frame * (slots + video_slack_frames)`, `audio_need = audio_bytes_per_frame * slots`, `log_need = psg_log_record_bytes * psg_log_records_per_frame * slots`, `if (slots > SHIM_SLOTS_MAX) return 0`, then `shim_reserve` x3 and `return 0` on any refusal; `:72-91` `shim_reserve` refuses a request that cannot be represented and never truncates; `:518` `if (st != IVS_OK ...) return 0` -- and `BUFFER_TOO_SMALL` is the status `IVS_E_BUFFER_TOO_SMALL` (`core/include/ivs/ivs_status.h:30`), so it is refused; **live** the formula is byte-for-byte `runner/exec.c:952-985` (`per[0..2]` with `slots + video_slack_frames` on the video channel only), and the pass run's `frames 13` / `video.plane.bytes 198432` show the sizes are sufficient rather than guessed | closed |
| T-15-06 | 15-01 | Repudiation | a digest recomputed in `host/` or `tests/host/` | high | mitigate | `ivs_retro_digest` is one call to `ivs_machine_state_digest` and nothing else; the harness prints what it is given | code `host/ivs_retro_shim.c:533-543` -- guard on `machine`/`out32`, one `ivs_machine_state_digest(...)`, `return 1`, no other statement; `tests/host/ivs_host_harness.c:591-602` prints `api.digest(...)`'s 32 bytes and nothing else; **live** `grep -rn 'sha256\|SHA-256\|sha_256' host/ tests/host/` -> one hit, `host/libretro_shim.h:126`, a comment describing the entry; the pass run's nine lines are byte-equal to `data/golden/demo-lamp-tone-pass.digests` (checked field by field, all nine MATCH) | closed |
| T-15-SC | 15-01, 15-02, 15-03 | Tampering | npm/pip/cargo installs | high | mitigate | no package is installed and no header is vendored (D-48); the only new dependency is the platform's own `dlopen`, part of the C library on both gate platforms, and `data/rights/libretro-abi.json` is already `cleared` with `taken: nothing` | code `data/rights/libretro-abi.json` (`"disposition": "cleared"`, `"taken": "nothing"`, with the note that the project writes its own subset and vendors no header, stub or copy), `host/libretro_shim.h:1-15` (project-authored, *libretro-shaped, never compatible*), `tests/CMakeLists.txt:744` (the only new link input is `${CMAKE_DL_LIBS}`); **live** `git diff --name-only 78ab270..HEAD` over `package.json`, lockfiles, `Cargo.*`, `pyproject`, `go.mod`, `vcpkg`, `conan` -> nothing (only `.planning/REQUIREMENTS.md`-family files match the word); `find . -name 'libretro*.h' -not -path './build/*'` -> only `host/libretro_shim.h`, the project's own; `core-imports=7` and `public-symbols=19 defined=141` unmoved | closed |
| T-15-07 | 15-02 | Tampering | the digest equality judge | high | mitigate | three limbs, each able to fail: the observed-set and line-set equality, the empty-poll run whose `digest.input` differs while `digest.images` and every non-input line the idle golden carries (eight today) equal that golden's, and the two committed stand-ins (`host-vacuous` refused for `frames-zero`, `host-digest-diff` refused for `digest-diff`) | code `tests/runner-process-tap.sh:2293-2300` (`check_host` requires a positive frame count **before** anything is compared, then `check_digest_lines`), `:2234-2289` (`check_host_empty`, the three limbs), `:2703-2752` (the two stand-ins, each judged by the **same** `check_host` the real run is judged by); **live** the pass run -> all nine `digest.*` lines MATCH the pass golden and the non-`-` count is 9; `--empty-poll` -> `frames 13`, `digest.images 09679d87...` equal to the pass golden, `digest.input 27da2e1f...` different, and **all eight** non-input lines equal to `data/golden/demo-lamp-tone-idle.digests` (checked one by one, 8/8 MATCH); `ok 138 judge.host-vacuous`, `ok 139 judge.host-digest-diff` in the suite (the token sets are compared exactly, see WR-02) | closed |
| T-15-08 | 15-02 | Repudiation | the final tick of the drive | high | mitigate | `run(target_ticks)` and the harness's last call to exactly 806568; the empty-poll and pass runs share the drive shape, so a frame-only stop moves seven lines and `digest.cpu`/`digest.psglog` are asserted byte for byte | code `tests/host/ivs_host_harness.c:37` (`#define HARNESS_LAST_TARGET 806568u`), `:532-541` (one call per frame boundary, then the last call to `HARNESS_LAST_TARGET`, in **both** the pass and empty modes), `:128` (`int (*run)(uint64_t target_ticks)`); **live** the pass run's nine lines equal the golden, whose `step.4.target_ticks 806568` and `step.4.now_ticks 806568` (overshoot 0) are the mid-frame stop; `host/ivs_retro_shim.c:502` sets `q.flags = 0` with the comment that the target tick, not a frame flag, is the authority | closed |
| T-15-09 | 15-02 | Tampering | the input poll | medium | mitigate | the poll carries `at_tick` and the case requires the scenario's six absolute ticks and words; the shim submits only the unconsumed suffix, so `IVS_REASON_INPUT_BEFORE_FLOOR` cannot arise, and a frame-quantized poll fails the word assertion | code `tests/runner-process-tap.sh:2339-2360` (`host_scenario_words` derives the six `<at_tick> <c0> <c1>` triples from `data/scenarios/demo-lamp-tone-pass.ivss` + `data/expected/input-bits.ids`, never from a typed list) and `:2436-2441` (`cmp -s` against the harness's printed `input.words`); `host/ivs_retro_shim.c:511-514` (`q.events = shim.consumed < shim.sched_n ? &shim.sched[shim.consumed] : NULL`, `q.event_count = shim.sched_n - shim.consumed`), `:239-250` (the harness's poll refuses to deliver an event the machine has passed: `poll_backwards`); **live** the pass run prints `input.words 100000/220000/340000/460000/580000/700000` with the scenario's exact words, and the case compares them against the scenario-derived file, so any other tick or word fails; the golden's `input.events 6` / `input.consumed 6` agree | closed |
| T-15-10 | 15-02 | Tampering | a second palette or converter | high | mitigate | the shim compiles in `runner/frame_export.c` and links `tools/pcm/ivs_refpcm.c`; `host-symbols.sh`'s palette-word grep over the exported set and plan 15-01's digest grep are the static halves, and the pixel probe is the dynamic half | code `host/CMakeLists.txt:21-27` (`add_library(ivs_retro_shim SHARED ivs_retro_shim.c ${PROJECT_SOURCE_DIR}/runner/frame_export.c)` and `target_link_libraries(... ivs_core ivs_refpcm ...)`), `host/ivs_retro_shim.c:427` (`ivs_frameppm_palette`) and `:455` (`ivs_refpcm_feed`), `tools/check/host-symbols.sh:139-142` (`host-symbols-palette` over `palette\|rgb\|ppm\|framepreview`), `tests/host/ivs_host_harness.c:166-190` (every delivered pixel must equal one of the 17 pinned-distinct triples); **live** `grep -rn 'palette\|PALETTE' host/ tests/host/` -> only calls to `ivs_frameppm_palette` and comments, **no array**; no second converter name in `host/`; a synthetic `/tmp` object exporting `ivs_palette_dump` -> `-- FAIL host-symbols-palette: ivs_palette_dump` (the limb fires); the pass run's `pixel.probes ok 198432` equals `video.plane.bytes 198432`, so a copied-and-drifted palette would fail `host.frames` | closed |
| T-15-11 | 15-02 | Information disclosure | the object entering the installed package | high | mitigate | no `install()` rule; the eleven-path inventory; the `host-so-absent` selftest plants `lib/libivs_retro.so` in a copy of the prefix and requires `pkg-inventory-extra` by name; `prefix-files=11` in the closing line | code `host/CMakeLists.txt:52-53`, `data/expected/pkg-install-files.ids` (11 paths, none under `lib/libivs_retro`), `tools/check/steps/pkg-install.sh:670-679` (the planted `lib/libivs_retro.so` must produce `-- FAIL pkg-inventory-extra: lib/libivs_retro.so`), `data/expected/pkg-selftests.ids:33` (`host-so-absent`); **live** `pkg-install` re-run in full this session -> `-- pkg-install pass prefix-files=11 archive=members public-symbols=19 defined=141 hdr-alone=10/10 consumer-lines=164 cxx-link=ok pkg-selftest=20/20`, exit 0; the gate line carries the same `prefix-files=11` | closed |
| T-15-12 | 15-02 | Tampering | a shared object entering the tree or a release archive | high | mitigate | `so dylib dll bundle` in the one extension list read by the files step, the release member check and the hygiene rule; two inventoried controls; one `release-assets` case that refuses a `.so` member the list itself names; `binaries-listed=0` | code `tools/check/binary-ext.txt:47-50` and its header naming the three readers, `tools/check/steps/release-assets.sh:182-195` (the `so-member` case plants `lib/libivs_retro.so` in an archive **whose own list names it** and additionally requires that neither `extra-member` nor `missing-member` fired), `data/expected/release-assets-selftests.ids:21`; **live** `release-assets` re-run -> `-- release-assets pass release-selftest=12/12`, so `so-member` fired for the binary rule alone; `files pass ... binaries-listed=0`; `git ls-files` carries no shared object; the two controls each fire on the planted path | closed |
| T-15-13 | 15-02 | Repudiation | an untagged claim word in the new `docs/packaging.md` sentences | low | mitigate | the wording avoids all ten claim words (Pitfall 8) and the `text` step's `text-claim-untagged` finding is the tripwire; the closing gate line must be green | code `docs/packaging.md:39` (the host-shim paragraph: "build-only", "never installed and never shipped", "not a release member", "libretro-shaped", no claim word) and `:123` (the no-shared-object-in-a-release sentence); **live** `text pass docs=64 claim-refs=27` on the committed tree, and the full gate exit 0 with zero `-- FAIL`, so no `text-claim-untagged` fired | closed |
| T-15-14 | 15-03 | Repudiation | a requirement flipped without its row, or a row without its flip | high | mitigate | the flip and the rows land in one commit; `important-gates-unknown-id` and `important-gates-complete-uncovered` fire in both directions and the verify requires the measured `gates=118 lines=...` pass line | code `git show --stat d712f02` carries `.planning/REQUIREMENTS.md` (checkbox **and** traceability row) and `data/expected/important-gates.ids` (+11 lines: the `host` gate name and its rows) in the same commit; `data/expected/important-gates.ids:570,576,584,590,597` (five TOOL-08 rows: `judge.host-vacuous`, `judge.host-digest-diff`, `judge.host-schedule`, `judge.host-load-refused`, `judge.host-two-limb`), `.planning/REQUIREMENTS.md:38,111` (Complete in both places); **live** `important-gates pass gates=118 lines=361` with `rows-pending=0 policies-pending=0`; the two committed controls re-run on the toy tree -> `-- FAIL important-gates-unknown-id: fault NO_SUCH_FAULT` (a row naming an id no registry carries) and `-- FAIL important-gates-complete-uncovered: FND-99` (a Complete requirement with no row) -- both directions fire. The measured `lines` is **361**, not the plan's predicted 358: the review's three added rows moved it (`d712f02` -> `ccfc04b` 359 -> `567b790` 360 -> `d45f87f` 361), each stated in its own commit message, never re-blessed | closed |
| T-15-15 | 15-03 | Repudiation | a claim minted without evidence | high | mitigate | the claim is minted only after the host ids measure green (the suite trailer with zero `not ok` is the precondition, checked in the same task); the tier is the frozen ceiling `deterministic-repeatability`, the standing `provisional`, and the note states that the same core is compared with itself and that D10/D11 are unperformed; the recorded fallback is TOOL-08 `Deferred`, never a claim over a red case | code `data/claims.json` CL-0029 (`"tier": "deterministic-repeatability"`, `"standing": "provisional"`, `"artifact": "tests/host/ivs_host_harness.c"`, `depends_on` = four profile fields, and a note stating limits (1)-(6): never installed so `installed-host-behavior` does not apply, the same core compared with itself, D10/D11 unperformed, one build and one profile, the weakest-field rule, no shared object shipped); `data/expected/claims.ids` (the ledger row and the inventory line in one commit, `f9b7864`), `docs/evidence-tiers.md` (the `[claim:CL-0029]` tag), `CHANGELOG.md` (the table rendered by `claims-table.cmake`); **live** the suite trailer is `# suite=runner-process executed=147 skipped=2 failed=0` with **zero** `not ok`, and the eleven host ids are `ok`; the gate reads `claims=29 provisional=28` and `claim-refs=27`, so the minted claim is `provisional` and every reference resolves; GUARDRAILS V3's ceiling for Phase 15 is exactly this one `provisional` claim | closed |
| T-15-16 | 15-03 | Repudiation | the cost checkpoint | high | mitigate | the five `cpu_s`, `build_s` and `gate_s` values are printed beside the verdict with their sorted middle values, recomputed from the committed TSV; the frozen figures are named stale by measurement, and the verdict is one of the three rule words with its arithmetic shown | code `.planning/STATE.md:681` (the Phase 15 checkpoint bullet: the five values, all three medians, the thresholds and the single verdict `reprice`), `15-03-SUMMARY.md:282-323` (the same arithmetic with the five-row table); **live, recomputed here in Python** from the last five `.planning/ci-runs.tsv` rows (`45718553735`, `45720934719`, `45723269901`, `45747709457`, `45751979661`): `cpu_s` sorted `359 363 366 368 438` -> median **366**; `build_s` `34 34 35 35 39` -> **35**; `gate_s` `115 119 120 122 135` -> **120** -- identical to the record, run ids included. The rule arithmetic holds: `discharged-within-allowance` needs `<= 361 + 1.535 = 362.535`, and 366 is 3.465 above, so it is unreachable whatever the token line did; `lever-required` needs cumulative spend past 21 (it is 5.005); `reprice` is what remains | closed |
| T-15-17 | 15-03 | Tampering | a differing gate token | medium | mitigate | every predicted token is compared against the fresh line; a differing token is chased to named commits and recorded, never re-blessed, and the invariants (`steps`, `faults`, `goldens`) are stated to be where they were frozen | code `15-03-SUMMARY.md:151-185` (105 tokens compared one by one; exactly four differ in the container run, all four documented host tokens), `:119-150` (the container run's log handling, including the preserved pre-fix `gate: FAIL steps=37/38` log), `:298` (the frozen `361/34/118` named stale by measurement, by `+5/+1/+2`); the three token moves after the review are each in their own commit message -- `lines=359 tests-expected=5038` (`ccfc04b`), `360`/`5040` (`567b790`), `361`/`5042` (`d45f87f`), `lines=361` (`d57c132`); **live** my full gate reads `gates=118 lines=361 tests-expected=5042 tests-executed=5038`, exactly the post-review figure, and the invariants are where they were frozen (`steps=38/38`, `faults=238/238`, `goldens=29`); the container log's token diff against the macOS line is exactly four pairs (`archive=members\|sha256`, `cc=AppleClang\|GNU`, `core-imports=7\|5`, `refs-verified=3/4\|0/4`) and no other | closed |
| T-15-18 | 15-03 | Repudiation | the container gate log | medium | mitigate | one container gate at a time, the log copied before anything else runs, a `not run` line with its reason if docker is unavailable, and no `ivs-*` image or container left afterwards | code `15-03-SUMMARY.md:119-135` (one `sh tools/gate-linux.sh` against the tree carrying both portability fixes: `gate: PASS`, `gate-time total=279`, `gate-linux: exit 0`; the passing log preserved at `/tmp/g15linux.log` **and** the pre-fix FAIL log at `/tmp/g15linux-prefix-fail.log`, so neither is mistaken for the other; nothing re-run); **live** `build/gate-linux.log` is the tool's own output, headed `gate-linux: x86_64, gcc (Ubuntu 13.3.0-6ubuntu2~24.04.1) 13.3.0` with `gate: PASS`; `docker ps -a --format '{{.Names}}' \| grep ivs-` and `docker images ... \| grep ivs-` are both empty (rc 1); and the Ubuntu leg for the **post-review** tree is the hosted run on the merged commit -- `gh run list` -> run `45779277467` (`push`, `0fc4d33`, workflow `gate`, `conclusion: success`), whose workflow is `sh tools/gate.sh` on `ubuntu-24.04` | closed |
| T-15-19 | 15-03 | Tampering | toy coverage silently deleted | medium | mitigate | one appended line per file across all ten registries and one row per toy requirement record, zero deletions, no `DEVIATION`; `control-toy-drift` and `control-deviation-unused` are refused in both directions, and `TOOL-96` is checked free before the append | code `git show --stat d712f02` -> thirteen toy paths, every one a single insertion (`+1 -0`): `_good/tree/data/expected/important-gates.ids` plus nine control overlays (ten registries) and three toy requirement records; **live** `grep -rl TOOL-96 tools/check/controls --include=important-gates.ids \| wc -l` -> **10**; `grep -rn TOOL-96 tools/check/controls/_good/tree/.planning/REQUIREMENTS.md` -> `\| TOOL-96 \| Phase 15 \| Complete \|`; the toy tree's own `important-gates` step re-run here -> `-- important-gates pass gates=47 lines=50`, exactly the predicted figure; no `DEVIATION` file exists under any Phase 15 control, so both drift directions are satisfied by construction (`_good` and all ten overlays carry the same line) | closed |

*Status: closed / open / open -- below {block_on} threshold (non-blocking). Severity order:
critical > high > medium > low; only open threats at or above `high` count toward `threats_open`.*

**Severity distribution:** 13 `high`, 6 `medium`, 1 `low` (`T-15-13`). Every `high` row is closed
with live evidence; `threats_open: 0`.

---

## The five code-review findings, re-verified as fixed

`15-REVIEW.md` found **one critical and four warnings** against the code as first committed. Each is
exactly the shape this phase exists to close -- a refusal that does not fire, a load that half
commits, and an evidence mechanism that cannot detect the failure it claims -- so each was **re-run**,
not taken from the disposition line. The six INFO rows are deferred and appear under Accepted Risks.

| Finding | Fix commit | Re-measured this session |
|---------|-----------|--------------------------|
| **CR-01** -- the input schedule was a fixed 4096-entry array that never compacted and never refused, so past 4096 polled events the shim silently stopped polling and reported a successful run (the review measured `poll_calls=4096 run=1`, 904 events dropped with no finding) | `ccfc04b` | code `host/ivs_retro_shim.c:366-393` (`shim_drain_poll`): the applied prefix is `memmove`d down **before** every drain (`:368-376`), the poll going backwards is a refusal (`:384-386`), and a schedule still full after the loop is `return 0` (`:392`, "still full: the host has more to say than the shim can hold"), which `retro_run` turns into a refused run (`:483-485`). **live**, both limbs through the committed harness modes: `--schedule-full` (5000 events pending in **one** drain) -> `schedule.polled 4096`, `schedule.runs 1`, `schedule.runs.ok 0`, `schedule.frames 0` -- refused, nothing delivered, and `4096 < 5000` proves the refusal came from the bound while the host still had events left; `--schedule-long` (8000 events over two calls, 4000 at a time) -> `schedule.polled 8000`, `schedule.runs 2`, `schedule.runs.ok 2`, `schedule.frames 2` -- the cap is on **outstanding** events, so a lifetime cap would stop at 4096 here. Case `runner-process.judge.host-schedule` is `ok 131` and its row is `data/expected/important-gates.ids:584` |
| **WR-01** -- a `retro_load_game` refused part-way left EXEC and GROM committed, so a digest answered on a half-loaded machine and a retry with the correct images was refused `IVS_E_DUPLICATE_LOAD` forever (the review measured `load_bad_cart_first=0`, `digest_after_refused_load=1`, `load_good_retry=0`) | `567b790` | code `host/ivs_retro_shim.c:292-357`: a `committed` bitmask records **this call's own** kinds (`1` EXEC, `2` GROM, `4` cartridge) and `refuse:` unloads exactly those, cartridge first (`:335-353`); the failure paths after the power-on reset (info, refpcm) go to `refuse_powered:`, whose undo is `shim_rebuild_machine()` (`:269-289`) -- a fresh image-less instance over the same storage -- because the machine is POWERED there and `ivs_machine_unload` would refuse `BAD_STATE`. **live**, the committed `--refused-load` mode: `load.before.digest 1`, `load.refused 1`, `load.refused.digest 1`, `load.refused.unchanged 1` (the instance digest is **unmoved**, so nothing was committed), `load.refused.run 0` (nothing to run), `load.retry 1`, `load.retry.digest 1`, `load.retry.moved 1` (the same call with the right images succeeds and moves the digest, so a refusal cannot pass as a machine that is simply dead). Case `runner-process.judge.host-load-refused` is `ok 132`, row `important-gates.ids:590` |
| **WR-02** -- `self_control` accepted a stand-in that failed for its intended token **and others**, so a two-limb stand-in passed as a healthy one-limb control | `d45f87f` | code `tests/runner-process-tap.sh:2660-2683`: `_scwant` (the declared token set, sorted) is compared with `_schave` (the tokens actually emitted, sorted) and `ok` only on exact equality; anything else is `judge-wrong-reason` with the declared and observed sets printed. **live** the suite's two committed stand-ins are `ok 138 judge.host-vacuous` and `ok 139 judge.host-digest-diff`, and the guard **on** the guard is `ok 140 judge.host-two-limb`, which drives both directions: a two-limb stand-in must be `wrong-reason` (not `ok`), and a declaration claiming a limb the check never exercises must also be `wrong-reason`. Row `important-gates.ids:597` |
| **WR-03** -- `CL-0029` said the frontend loads the cartridge "with the zero graphics ROM and no EXEC image", while the harness reads `boot-stub-2.exec` (exactly 8192 bytes) and passes it as `imgs.exec`, loaded with `IVS_KIND_EXEC` | `d57c132` | code `data/claims.json` CL-0029 `statement` now reads "through the project boot stub contract 2 (the image boot-stub-2), the project's own lamp-tone.ivsc ... with the project's own zero graphics ROM and no EXEC image or GROM image of any console" -- the sibling wording, with the qualifier restored; `tests/host/ivs_host_harness.c:430-432` is the load the sentence describes. **live** `claims=29 provisional=28 claim-refs=27` unmoved, so the correction added no id and no reference |
| **WR-04** -- the `host-symbols-extra` direction had no committed stand-in, so half of the exact-set comparison was unproved | `cef9e57` | code `tests/runner-process-tap.sh:2608-2622`: a second scratch ids copy with an exported name (`retro_run`) **deleted** must be refused as `host-symbols-extra` **by that name**, with the same two limbs as the missing direction (refused at all, and refused for the right token). **live** both limbs reproduce by hand against the built object: appended name -> `-- FAIL host-symbols-missing: ivs_not_defined`; dropped name -> `-- FAIL host-symbols-extra: retro_run`, exit 1 each. The suite's `runner-process.host.exports` is `ok 133` |

**The two container-leg fixes cannot hide a real import** (the task's last named check). Both were
made before the passing container run and are re-verified here:

- **PIC on `ivs_refpcm`** -- `tools/pcm/CMakeLists.txt:22-23` sets `POSITION_INDEPENDENT_CODE ON`
  with the measured reason (an instrumented non-PIC member carries a relocation a GNU shared object
  cannot use). It cannot move the static package: `tools/pcm` has **no** `install()` rule
  (`grep -n 'install(' tools/pcm/CMakeLists.txt` -> nothing) and the package stays at
  `prefix-files=11`, measured live in this session.
- **The four ELF stubs** -- `data/expected/host-imports.ids` lists `_ITM_deregisterTMCloneTable`,
  `_ITM_registerTMCloneTable`, `__cxa_finalize`, `__gmon_start__` **by name**, explicitly *not* behind
  an `_ITM_*`/`__gmon_*` prefix, so the allowlist cannot admit an unreviewed spelling of the same
  family. A real import still fails: the checker's own subtraction is what refused `fopen`/`fread`/
  `fclose` on the harness binary (43 names), and both presets measure `imports=0`. The residual that
  the four are not *asserted* to be weak-undefined is R-15-E below -- low, and none of the four is a
  content route, clock, environment, thread, randomness, assert, abort or libm name.

The container leg that proves the Linux link is the 01:58 run (`gate: PASS`, `cc=GNU`), made eight
seconds before `85470b8` against the fixed working tree; the Ubuntu leg for the **post-review** tree
is the hosted `gate` run on the merged commit (`45779277467`, success).

---

## Unregistered Flags

One finding surfaced during this audit that maps to **no** register row. It is not a control gap,
it is below `block_on: high`, and it is not counted in `threats_open`. Recorded so it is not lost.

| Flag | Severity | What is wrong | Evidence | Recommended action |
|------|----------|---------------|----------|--------------------|
| **UF-15-01** -- a phase summary still says the claim's note cites the pre-review trailer | low | `15-03-SUMMARY.md` states the trailer as `executed=144 skipped=2 failed=0` and adds, in parentheses, that this is "the same trailer `CL-0029`'s note cites". The note was updated by the review round and now cites `executed=147`, which is what the suite actually prints. So the summary carries a figure that disagrees with the file it names and with a fresh run -- the class `AGENTS.md` calls "a defect to correct, not a fact to reuse". The **claim** is correct against measurement (verified live: `executed=147 skipped=2 failed=0`, zero `not ok`); the stale sentence is in the plan record, not the claim, so no mitigation is open. | `15-03-SUMMARY.md:117` and `:240`; `data/claims.json` CL-0029 `note` ("the suite trailer `# suite=runner-process executed=147 skipped=2 failed=0`"); my run of `tests/runner-process-tap.sh` -> `# suite=runner-process executed=147 skipped=2 failed=0`; the fix commit `d45f87f` states `executed=147` in its own message | Amend the two sentences to 147 (and drop the "same trailer" parenthetical, which the review round invalidated) in the next records pass. No threat, no control, no claim and no token is involved. |

---

## Accepted Risks Log

No threat in the register is `accept` or `transfer`; every row is `mitigate` and closed. The rows
below are **non-register residuals** -- they carry no threat id, are all below the `high` block
threshold, and are recorded here so they are not re-litigated as fresh findings. The first six are
`15-REVIEW.md`'s deferred INFO rows, whose deferral reasons this audit re-read and agreed with; the
seventh is an auditor observation the review did not make.

| Risk ID | Threat Ref | Severity | Rationale | Accepted By | Date |
|---------|------------|----------|-----------|-------------|------|
| R-15-A | -- (`15-REVIEW.md` IN-01) | low | `shim_deliver_video` trusts the slot geometry: `frames = bytes / video_slot_bytes` silently discards a remainder and the plane read at `slot + IVS_FRAME_HEADER_BYTES` does not check `video_slot_bytes >= IVS_FRAME_HEADER_BYTES + w*h`. Both hold for the one registered profile (`15296 = 32 + 159*96`), and the geometry is profile data the package pins, not host input; a profile change that broke it would fail `host.frames`'s `video.plane.bytes 198432` and `frame.geometry ok 13` first. A guard belongs with the next change to the slot contract, which is a core/profile change, not this phase's. | owner-delegated autonomous operation (AGENTS.md); next change to the video slot contract | 2026-11-26 |
| R-15-B | -- (`IN-02`) | low | `q.event_count = shim.sched_n - shim.consumed` is unsigned arithmetic over a count the **core** reports, and `shim.consumed += rep.input_consumed` is not clamped, so a core that reported more consumed events than it was given would underflow the next call's count. The core is the project's own code and `input.consumed` is bounded by the request it was handed (`core/src/machine/machine_bound.c`); the defect needs a core invariant break to reach. Clamping is a two-line hardening that should land with a case that can exercise it, and no committed case can today. | owner-delegated autonomous operation; with the next change to the run request | 2026-11-26 |
| R-15-C | -- (`IN-03`) | low | `SHIM_SLOTS_MAX` (`1 << 20`) bounds the slot **count**, not the bytes, so a hostile target tick could request ~34 GB of log plus ~16 GB of video before `realloc` is attempted. The refusal happens (allocation fails, `retro_run` returns 0) and nothing crashes, so the DoS threat T-15-05 is closed; the residual is that the first line of defence is the allocator rather than a named byte cap. A named `SHIM_BYTES_MAX` derived from a sane maximum call length is the right shape and needs its own refusal case. | owner-delegated autonomous operation; next change to the run bound | 2026-11-26 |
| R-15-D | -- (`IN-04`) | low | `host-symbols.sh`'s pass line prints `exported=` from the **pin's** line count (`$tmp/want`), not from the object's exported set (`$tmp/have`). The comparison itself is exact in both directions and fails correctly, so no check is weakened -- but the token a gate reader sees (`exported=9`) is the pin's count, not the artifact's, which is the "recompute, never quote" rule applied to a printed figure. This audit confirmed the artifact independently (`nm` -> exactly nine on both presets), so the number is currently true as well as printed. | owner-delegated autonomous operation; next change to the check's published line | 2026-11-26 |
| R-15-E | -- (`IN-05`) | low | The four ELF crt stubs are allowlisted by name with a comment stating each is `weak undefined`, and nothing verifies the weakness (`nm -P -u` does not distinguish weak from strong). The stated reason is therefore unchecked: if the shim ever acquired a **strong** dependency on `__cxa_finalize` or `__gmon_start__`, the allowlist would hide it. Low because none of the four is a content route, clock, environment, thread, randomness, assert, abort or libm name, and because the release object's undefined set was read this session and carries no such name. Asserting the weakness (`nm -P -u` prints `w`/`v` on ELF) is the fix and needs an ELF-only case. | owner-delegated autonomous operation; with the next change to `host-imports.ids` | 2026-11-26 |
| R-15-F | -- (`IN-06`) | low | `host/libretro_shim.h:110-115` promises "a refused or internal-failure run records nothing, never a truncated delivery", while `retro_run` advances `shim.consumed` and delivers video **before** `shim_deliver_audio`, which can still return 0 (`realloc` failure, or `IVS_REFPCM_E_LEVEL`). The caller then gets 0 with frames already handed over and events marked consumed. Reachable only on allocator failure or a core invariant break, and a truncated delivery fails `host.frames`'s `frames 13` and `pixel.probes == video.plane.bytes` rather than passing. Amending the header (or staging both streams) is the fix and needs a case that can force a delivery-stage failure. | owner-delegated autonomous operation; next change to the delivery contract | 2026-11-26 |
| R-15-G | -- (auditor observation) | low | The evidence that the **final** tree links and passes on GNU is a hosted CI run, not a committed log: `build/gate-linux.log` (01:58) predates the five review-fix commits, so its token line reads `lines=358` while the tree reads `361`, and `build/` is not tracked. This audit closed the gap by reading `gh run list`: run `45779277467` on the merged commit `0fc4d33` is the `gate` workflow (`sh tools/gate.sh` on `ubuntu-24.04`) with `conclusion: success`. The residual is that a future reader of the tree alone cannot see that Ubuntu run; the container leg for a post-review tree is not re-run locally (lesson 18: a second run destroys the first run's evidence), so this is recorded rather than fixed. | owner-delegated autonomous operation; next container leg | 2026-11-26 |

*Accepted risks do not resurface in future audit runs.*

---

## Security Audit Trail

| Audit Date | Threats Total | Closed | Open | Run By |
|------------|---------------|--------|------|--------|
| 2026-11-26 | 20 (22 register rows) | 20 | 0 | gsd-security-auditor (pi), ASVS L1, `block_on: high` |

**What was executed, not inferred.** `sh tools/gate.sh` (exit 0, zero `^-- FAIL`,
`gate-time total=77`); `sh tests/runner-process-tap.sh ...` (trailer `executed=147 skipped=2 failed=0`,
zero `not ok`, eleven host ids `ok`); `host-symbols.sh` on both presets and in both directions plus a
third run over the harness binary; `nm -P -g --defined-only` and `nm -P -u` on both presets; the four
harness modes (`--empty-poll`, `--schedule-full`, `--schedule-long`, `--refused-load`) and the no-arg /
bad-path / missing-content invocations; the empty-poll output compared line by line against **both**
goldens; the seven step scripts re-run individually; both `files-unlisted-binary-*` controls and both
`important-gates` direction controls on copies of the toy tree; a synthetic `/tmp` object exporting
`ivs_palette_dump` and importing `fopen` (both limbs refused by name, then removed); the last five
`.planning/ci-runs.tsv` rows recomputed in Python; `git show --stat d712f02` and the five fix commits;
`gh run list` for the merged commit's Ubuntu run; and `docker ps -a` / `docker images` for leftover
`ivs-*` resources.

**Nothing was written outside this file.** The tree was inspected read-only; the only files this
audit created outside `build/` were under `/tmp` (step filelists, harness outputs, the synthetic
object, which was removed in the same command block). `git status --porcelain` was empty before this
file was written; no branch was switched, no stash was made and nothing was committed.

---

## Sign-Off

- [x] All `<required_reading>` loaded before any analysis (three plans, three summaries, the review, the context, GUARDRAILS v0.3 and `AGENTS.md`)
- [x] Threat register extracted from the three plans' `<threat_model>` blocks: 22 rows, 20 distinct ids
- [x] Each threat verified by its disposition (`mitigate` x 20; no `accept` or `transfer` row)
- [x] Threat flags from the summaries incorporated -- `15-03-SUMMARY.md` carries a "Threat Flags" section reporting none, and its two container-leg files (`tools/pcm/CMakeLists.txt`, `data/expected/host-imports.ids`) are inside the register as T-15-03/T-15-05/T-15-11 territory, which this audit confirmed by reading both; `15-01-SUMMARY.md` and `15-02-SUMMARY.md` carry "Threat surface scan" sections reporting no new boundary, and no new surface was found for them either
- [x] The five review findings re-verified as fixed by re-running them, not by citing the disposition
- [x] The export set is exactly the nine pinned names in **both** presets, and **both** directions of the check are proved live (committed limb and by hand)
- [x] The two committed stand-ins each fail for their intended reason and no other (exact token-set comparison, guarded by `judge.host-two-limb`)
- [x] `retro_load_game`'s rollback leaves nothing committed on a refusal and a good retry succeeds (eight observations, both directions)
- [x] The input schedule refuses rather than truncating (CR-01, `ccfc04b`), proved at the bound and over a lifetime
- [x] The host opens no file but its arguments, holds no palette, and cannot write into the tree
- [x] The shared object is absent from the installed package and an archive, and the `release-assets` case fails when it is present
- [x] The two container-leg fixes (PIC on `ivs_refpcm`, the four ELF stubs listed explicitly) cannot hide a real import
- [x] Implementation files never modified; **no file written but this one**
- [x] `threats_open: 0` confirmed -- no open threat at or above `high`, and the one unregistered flag is below it
- [x] Each mitigation is shown working by a command, a `file:line` or a control, not by the plan's claim

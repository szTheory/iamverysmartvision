# Phase 4: Headless Runner and Installed Consumer (Slice 0) - Research

**Researched:** 2026-11-11
**Domain:** extending a C17 deterministic core (run API, observation, input, digest domains) with a POSIX runner, a hand-assembled diagnostic, a relocatable CMake package with an independent consumer, and a CI/gate that stays inside a 40 s hosted budget
**Confidence:** MEDIUM-HIGH (gate mechanics read from source and a baseline gate run this session: HIGH; hosted timing, gcc/ELF behaviour of new code and CMake 3.25 itself: MEDIUM; the listed `[ASSUMED]` items: LOW)

Scope note: the design is settled by D-01..D-47 in `04-CONTEXT.md` and the six `04-DISCUSS-*.md` files. This file adds only what reduces planning risk: which Phase 1-3 mechanisms must change and where (file:line), a 17-plan decomposition with sequencing, hazards specific to this codebase, the validation map, and the data-file edits. No web research was done and no source of another emulator was opened. Probes run this session on this host (Apple clang 21, CMake 4.4.3, Ninja 1.13.2, dash, macOS sh): a full baseline `sh tools/gate.sh`; `times` output formats; `ninja -t deps` with a space in the prefix.

<user_constraints>
## User Constraints (from CONTEXT.md)

### Locked Decisions
D-01 through D-47 in `.planning/phases/04-headless-runner-and-installed-consumer/04-CONTEXT.md` `<decisions>` are locked and incorporated by reference (a verbatim copy would drift; the planner reads that file first). One-line index:
- Run API and observation (API-02, API-04): D-01 one call `ivs_machine_run`, absolute master-tick target, ends at the first unit boundary `>= target`, no state between calls; D-02 request; D-03 report; D-04 stop reasons TARGET=1, CPU_HALTED=2, GUEST_FAULT=3, INTERNAL=4 pinned by `data/expected/api-stop-reasons.ids`; D-05 refusal order, no new status numbers, new reasons after the load-report reasons; D-06 zero-length real video/audio plus an internal env seam, layout and timebase appended to `ivs_info`; D-07 slicing invariance and named `RUN_*` faults; D-08 `ivs_machine_cpu_state` and `ivs_machine_peek`; D-09 frame and cycle bounds are runner sugar.
- Digests (RUN-05): D-10 domains 16/17/18 = VIDEO/AUDIO/INPUT, composite stays version 1 over 1..4; D-11 `*.expect` (original-diagnostic, independent-derivation) and `*.digests` (deterministic-repeatability, self-recorded) goldens with header checks.
- Input (D6, delegated): D-12 logical 32-bit word per PSG port; D-13 32-byte `ivs_input_event` array in the run request; D-14 apply at the first CPU-unit boundary whose start tick is `>= at_tick`, `input_consumed`, `input_floor_tick`, `controller_now[4]`; D-15 INPUT domain live with chain `h0 = SHA256("IVS-INPUT-LOG-V1")`; D-16 discrepancy records and tests.
- Runner (RUN-01, RUN-02): D-17 `runner/` layout and `ivs-run`; D-18 IVSS v1; D-19 report; D-20 exit codes 0/10/11/12/13/14; D-21 slices of at most `1<<22` ticks, `sig_atomic_t`, monotonic timer only in the runner; D-22 untrusted scenarios, caps; D-23 suites `runner` and `runner-process`; D-24 runner not installed, fault framework generalized to repo-relative owner TUs.
- Diagnostic (RUN-06, D4 delegated): D-25 hand-assembled annotated listings, no assembler dependency; D-26 listing grammar and `tools/check/listing.cmake`; D-27 arithmetic oracle, CL-0008 provisional; D-28 22-word boot stub; D-29 anti-HLE guards; D-30 `tools/pack`, `zero-grom`; D-31 `ivs-diag-cpu-1` (56 words at `$5000`); D-32 mutation controls.
- Packaging (PKG-01): D-33 one static export `ivs::core`; D-34 version single source in `ivs.h`; D-35 static only, `IVS_API` empty, floor 3.25; D-36 consumer in `tests/installed-consumer/`; D-37 gate step `pkg-install`; D-38 symbol and header rules; D-39 consumer exercise and golden; D-40 ten in-step selftests.
- CI (PKG-03, PKG-04, RUN-04; D12 delegated): D-41 keep one job, no matrix, no macOS leg; D-42 `tools/check/ci-summary.sh` through an EXIT trap; D-43 structural no-artifact/no-cache; D-44 `gate-env`/`gate-cpu` lines, `tools/ci-record.sh`, `.planning/ci-runs.tsv`, `ci-log` step; D-45 budget rule (apply a lever when the median of the last 3 hosted gate-times exceeds 40 s); D-46 important-gates registry and step; D-47 branch protection recommended, release design recorded.

### Claude's Discretion (verbatim)
Plan ordering, exact struct layouts and field names, the final fault and control id lists, TAP suite splitting, whether `tools/pack` is C or sh-plus-C, and file naming under `runner/` and `tests/`.

### Deferred Ideas (OUT OF SCOPE, verbatim)
- Shared library, `IVS_API` export macro and exact `nm -D` symbol equality; an installable runner component; project mini-assembler; real controller electrical table and PSG reads (Phase 5); frame stop and real video/audio (Phases 6-7); release workflow; macOS leg; ccache; two-job split.
</user_constraints>

<phase_requirements>
## Phase Requirements

| ID | Description | Research Support |
|----|-------------|------------------|
| API-02 | Bounded run by absolute target, overshoot, slicing invariance | Run loop and ladder (Plan 05); step seam `core/src/machine/machine_run.c:84`; slicing test against a unit-by-unit reference (Plans 05, 07, 09) |
| API-04 | Output buffers in caller memory; undersized refused before any state change | Env seam and zero-length layout (Plan 06); effect-table rows with whole-storage `memcmp` oracle |
| RUN-01 | Scripted scenario on fresh isolated state, reports time, stop reason, outputs, assertions | IVSS parser/executor (Plan 10), process suite (Plan 11), `ivs_machine_cpu_state` and `ivs_machine_peek` (Plan 04), input schedule through D-13..D-15 (Plan 07) |
| RUN-02 | Five distinct outcomes | Outcome mapping as a pure function with named faults (Plans 02, 10), one demonstrating scenario each (Plan 11) |
| RUN-04 | Each important gate has a named-fault control that fails for its intended reason | Fault framework generalization (Plan 02), registry and step (Plan 15), mutants for the consumer (Plan 14) |
| RUN-05 | Per-domain digests, repeated runs match, goldens carry tier and oracle | Observation rows and domains 16..18 (Plan 03), input chain (Plan 07), golden step (Plan 12) |
| RUN-06 | Original diagnostic from reset via a project boot path, independent predicate | Listing checker, `tools/pack`, boot stub (Plan 08), diagnostic and hand derivation (Plan 09) |
| PKG-01 | Independent consumer against the installed, relocated package | Install/export and structural proof (Plan 13), consumer and selftests (Plan 14) |
| PKG-03 | CI shows skips visibly, never caches or uploads | `ci-summary.sh`, action allowlist in `ci-parity` (Plans 01, 16) |
| PKG-04 | CI critical path recorded per run | `gate-env`, `gate-cpu`, `step-time`, `ci-runs.tsv`, `ci-log` (Plan 01), budget rule (Plan 16) |
</phase_requirements>

## Project Constraints (from CLAUDE.md / AGENTS.md / AUTONOMY.md)

- Prefix `ivs_`; never "intellivision" in a package, product or repo name (hard stop). C17 + CMake (D1). No copying from MAME, jzIntv, libasm or any assembler; behaviour agreement with another emulator is not evidence. "Another copy-paste is better than another dependency" (AUTONOMY section 2): the packer, the TAP shell suites, the listing checker and the summary helper are all in-repo.
- Rights: no committed `.ivsc`, `.bin`, `.rom` or system image (`.gitignore` lists `*.bin *.rom *.ivsc *.int *.cfg`); `tools/pack` writes images only into the build tree; the boot stub and `zero-grom` are labelled "not an EXEC" / "not a GROM"; goldens never hold a `user` image digest. A hash is identity, not rights.
- Shift-left: zero human UAT. Nothing in this phase is handed to the owner except D10 (hardware measurement) and D11 (real EXEC/GROM runs), both already outside the automated exit, plus the branch-protection repository setting (orchestrator-assist, blocks nothing). Executors never call `gh`; the orchestrator runs `tools/ci-record.sh` once per hosted run.
- `sh tools/gate.sh` is the single local and CI command (`ci-parity`); measure before changing CI shape; the hosted budget is 40 s with a lever rule (D-45).
- Docs rule: claims in `README.md` and `docs/**/*.md` use the words supports, compatible, passes, accurate, verified, runs only with `[claim:CL-NNNN]` or `[nc]` on the same line (`tools/check/steps/text.sh:30-40`); the bare word clock/clocks fails `glossary-bare-clock` unless the line says input clock, STIC clock, PSG chip clock or `[nc]` (`tools/check/steps/glossary.sh:53-55`); emulator project names need `[cite-only]` (`tools/check/denylist-cite.txt`: `jzintv`); author-name strings in `tools/check/denylist.txt` are never waivable outside `.planning/`, `.claude/`, `docs/provenance/`, `tools/check/`. New docs (`docs/runner.md`, `docs/diagnostics.md`, `docs/packaging.md`) must obey all three.
- Hygiene: no home path, `/private/tmp/<name>`, `/tmp/claude-*`, tilde path or secret shape in any tracked file (`tools/hygiene.sh`), so scratch paths never appear in scripts, goldens or docs.

## Summary

Phase 4 is a second pass over the Phase 3 machinery, not new machinery. Every mechanism it needs exists and has a copy-able precedent: fault loop (`c-faults` plus `faults-verdict`), pure verdict steps driven by controls on a text-only `_good/tree`, TAP suites with exact inventories, in-step selftests (`purity-selftest`), `cmake -P` checkers (`vectors.cmake`, `api-effects.cmake`, `memmap.cmake`), build-tree products judged by producer/consumer step pairs. Five seams are single-assumption today and must be opened before the plans that need them: (1) the fault framework (`tests/faults.cmake`, `faults-verdict`, `c-build` listing) knows only `core/`-relative owner TUs; (2) `tools/check/integrity.cmake` applies the SPDX and ASCII rules only to `core|include|tests|tools|cmake|data|fixtures` paths, so a top-level `runner/` directory escapes both; (3) `tools/check/api-effects.cmake` pins only `IVS_E_*` numbers, so stop reasons and input bits need a generic header-define pin; (4) `tools/check/run-controls.sh` can drive only pure steps on a toy tree, so every new step that has controls needs toy fixtures in `_good/tree`, and every step that needs a built tree (`pkg-install`) must use in-step selftests; (5) the instance and digest table (`core/src/machine/machine.h:26,52`, `machine_digest.c:39`) are pinned by a static size assert and a coverage test that accepts only domains 1..4.

Three findings change what the planner writes. (a) The CONTEXT says "pin a Phase 3 composite literal first": it already exists (`tests/machine/machine_digest.c:214-217`, `md_fresh_composite`), but it covers only a fresh instance; Plan 03 must add a stepped-state literal set (loaded plus reset plus N units) recorded from the unmodified Phase 3 code before the digest table is touched. (b) 04-DISCUSS-runapi 4.5 applies input events "at the top of the loop before the exit check"; D-14 and 04-DISCUSS-ctrl say "at the first CPU-unit boundary whose start tick is `>= at_tick`" and define the floor as start of the last begun unit plus 1. Only the second form is self-consistent (applying at exit would let a later call submit an event that precedes an already-applied one). Follow D-14: apply immediately before beginning each unit, never at exit. (c) Gate cost: the baseline gate on this host, under load, is 26 steps, 231 controls, 119 fault runs, `gate-time total=80` with `controls` 52 s overlapped (step times in Pitfall 16). Phase 4 adds about 25-30 faults, 45 controls and four heavier steps; the estimate in D-45 (+5-8 s hosted) is plausible, and the budget will be tested, so the measuring lines land first (Plan 01).

**Primary recommendation:** 17 plans in five tracks. Track E (CI measurement, Plan 01) and the fault-framework generalization with a runner tracer (Plan 02) go first; Track A (API: digest rows, observation, run, buffers, input; Plans 03-07) is strictly sequential because all five edit `machine.h`, the digest table, `data/api/api-effects.json` and `data/expected/tests/run.ids`; Track B (listing checker, packer, stub, diagnostic; Plans 08-09), Track C (runner library, executable, goldens; Plans 10-12) and Track D (install and consumer; Plans 13-14) follow their API prerequisites; Plans 15-17 audit, record and close. Choke points: 02, 03, 05, 07, 10, 11, 13/14.

## Architectural Responsibility Map

| Capability | Primary Tier | Secondary Tier | Rationale |
|------------|-------------|----------------|-----------|
| Bounded run loop, overshoot, stop reasons, event application, observation chains | Core `core/src/machine/` (pure, instance-held state only) | Public header `core/include/ivs/ivs_run.h` | Determinism and slicing invariance require all resumable state inside the instance (D-01, D-07); no clock, no allocation, no callback |
| Output layout and timebase facts | Core (`ivs_info` append, profile fields) | Host adapters read them | Layout lives with the machine so hosts do not keep tables (D-06) |
| Scenario parsing, assertions, outcome taxonomy, report text, exit codes | `runner/` library over the public API only | Host vtable (fake in tests) | Runner is the first real consumer of the public API; its failures are harness-level, not machine-level (D-17, D-20) |
| File reads, monotonic timer, signals | `runner/hostio.c` (POSIX, only file with `_POSIX_C_SOURCE`) | operator command line | Keeps wall time and filesystem out of the core and the library (D-21, D-23) |
| Image construction (boot stub, cartridge container, zero GROM) | `tools/pack` in the build tree | Runner synthesizes `zero-grom` | The core stays image-free; nothing binary is committed (D-30) |
| Diagnostic source of truth | `fixtures/FX-*/…listing.txt` (hand-assembled) | `tools/check/listing.cmake` (static check) | The listing is the source; no assembler is a dependency (D-25, D-26) |
| Expected values for the predicate | `data/golden/*.expect` with a hand derivation | none | Independent of the emulator by construction (D-27) |
| Install/export proof | Gate step `pkg-install` (producer and checker in one, in-step selftests) | `tests/installed-consumer/` project | Needs configure/build/install, so it cannot be a control-driven pure step (D-37, D-40) |
| CI summary and measurement | `gate.sh` EXIT trap plus `tools/check/ci-summary.sh`; orchestrator-only `tools/ci-record.sh` | `.planning/ci-runs.tsv` | Queue time is not observable in-job (D-44) |

## Standard Stack

No external package is installed by this phase (copy-paste over dependency); the Package Legitimacy Audit is not applicable.

| Tool | Local (this host) | Hosted ubuntu-24.04 | Note |
|------|-------------------|---------------------|------|
| CMake | 4.4.3 [VERIFIED: `cmake --version`] | 3.28.3 [CITED: `.planning/CI-EFFICIENCY.md` Phase 3 section: "gcc 13.3, cmake 3.28.3"] | Floor 3.25 is declared and untested itself (D-35); everything used is at or below 3.25: `file(STRINGS)` before `project()`, `cmake --install --prefix` (3.15), `write_basic_package_version_file ... ARCH_INDEPENDENT` (3.14), `SameMinorVersion` (3.11), `NO_SYSTEM_FROM_IMPORTED` [CITED: 04-DISCUSS-pkg 4.2 item 12] |
| Ninja | 1.13.2 [VERIFIED] | 1.13.2 [CITED: 02-RESEARCH via 03-RESEARCH] | `ninja -t deps` prints a path with a space unescaped, 4-space indent [VERIFIED: probe this session on macOS clang, prefix directory `pfx dir`] |
| C compiler | Apple clang 21 [VERIFIED: gate `cc=AppleClang`] | gcc 13.3 [CITED: CI-EFFICIENCY] | Strict `-std=c17`, extensions off: POSIX names vanish on glibc without `_POSIX_C_SOURCE` |
| sh | macOS sh, dash, bash 5.2 | dash 0.5.12 [ASSUMED] | `times` prints `0m0.000000s 0m0.000000s` under dash and `0m0.002s 0m0.005s` under macOS sh [VERIFIED: probe this session]; one awk pattern `([0-9]+)m([0-9.]+)s` reads both |
| docker | `/usr/local/bin/docker` [VERIFIED: `which docker`] | n/a | `sh tools/gate-linux.sh` (local only): ubuntu:24.04, gcc 13, dash, cmake 3.28; about 3 min under amd64 emulation [CITED: CI-EFFICIENCY] |
| `nm`, `ar`, `cmake -E sha256sum`, `string(SHA256)` | present | present | Independent SHA-256 oracles at zero dependency cost (`tools/check/steps/tests-inventory.sh:163` precedent) |

**Version verification:** the CMake and compiler rows are from commands run this session or the recorded hosted runs; no registry package is involved.

## Architecture Patterns

### System flow (conceptual)

```
IVSS scenario (data/scenarios/*.ivss, untrusted inert text)
   |  ivs-run --content-dir <build>/images [--exec/--grom/--cart for user content]
   v
runner lib: parse (caps, charset) -> create instance (aligned storage) -> load images -> expect-load
   |   content boot-stub: image bytes built by tools/pack from fixtures/FX-boot-stub listing
   |   content zero-grom: 2048 zero bytes synthesized in the runner (labelled "not a GROM")
   v
reset (power on) -> loop over steps: slices of <= 1<<22 ticks ------------------+
   |   ivs_machine_run(target, flags, input events suffix, buffers)             | cancel flag /
   |      validate whole request (ladder) -> loop { apply events whose at_tick  | wall timer checked
   |      <= next unit start; step one unit; stop on fault / halt+flag / target }| between slices
   |   report: stop_reason, now, overshoot, input_consumed/floor, controller_now|
   v                                                                            |
checkpoint asserts via ivs_machine_cpu_state / ivs_machine_peek (read-only) <---+
   v
report (key value lines, ivsr 1 ... report.end n) + exit code 0/10/11/12/13/14
   v
gate: runner-process TAP (both presets) -> build/gate/<preset>/ -> pure steps: golden (header rules,
      body vs golden, dev == release), tests-inventory, faults-verdict, important-gates

install: release tree -> cmake --install --prefix A -> mv A "B dir" -> rm A -> exact file inventory,
      header cmp, path-leak grep, symbol rules, headers alone (C17, C++17) -> consumer copied outside
      the repo, find_package(ivs 0.1 CONFIG REQUIRED), build, run -> stdout vs pkg-consumer.golden
      and digests vs the runner golden for the same scenario -> ten in-step selftests
```

### Mechanisms Phase 4 must extend (read this session; the planner edits exactly these)

| # | Mechanism | Where (file:line) | Change required | Plan |
|---|-----------|-------------------|-----------------|------|
| 1 | Step inventory equality (steps on disk == union of `data/expected/gate-steps/*.ids`); each step must print `-- <name> pass ...` | `tools/gate.sh:66-81` (inventory), `:85-101` (run loop, `gate-step-no-pass-line`) | New steps `ci-summary` (selftest), `ci-log`, `listing`, `golden`, `pkg-install`, `important-gates`, `runner-scan`: add one inventory file (for example `data/expected/gate-steps/phase4.ids`) in the same plan as each script | 01, 02, 08, 12, 13, 15 |
| 2 | Summary line and tokens | `tools/gate.sh:128-129` (`gate-time`, `gate: %s steps=... controls=... %s`) | Add `gate-env ncpu=<n> cc=<id>` and `gate-cpu user=<s> sys=<s>` lines (not tokens, same class as `step-time`, `tools/gate.sh:20`); copy the `bad()` lines, `step_time` lines and the final line to files under `$GATE_TMP` for the EXIT-trap helper | 01 |
| 3 | EXIT trap | `tools/gate.sh:11` (`trap '... rm -rf "$GATE_TMP"' EXIT`; line 12 traps HUP INT TERM) | Run `sh "$CHECK_DIR/ci-summary.sh" "$GATE_TMP" >/dev/null 2>&1 \|\| :` before `rm -rf`, only when `GITHUB_STEP_SUMMARY` is non-empty; prints nothing to stdout; early `exit 1` paths still summarize because the trap runs | 01 |
| 4 | Control runner contract: a control's `CHECK` must be `steps/<name>.sh`; the step runs with `ROOT=<tree>`, `IVS_GATE_DIR=<tree>/build/gate`; every distinct `CHECK` step must pass on the pristine `_good/tree` | `tools/check/run-controls.sh:149-153` (`run_step`), `:155-165` (`good_one`), `:167-200` (`ctl_one`), `:242-252` (positive control) | Each new step with controls needs toy fixtures committed under `tools/check/controls/_good/tree/` in the same plan (toy listing, toy golden, toy `ci-runs.tsv`, toy `important-gates.ids` plus a toy `data/expected/controls/*.ids`); `ci-summary.sh` is not a step, so its checks are in-step selftests, not controls | 01, 08, 12, 15 |
| 5 | Control inventory exact | `data/expected/controls/*.ids` vs `tools/check/controls/*/` (`run-controls.sh:224-241`) | Every new control directory needs a line in a controls inventory file in the same commit | all |
| 6 | Fault loop: owner TU found only among `IVS_CORE_SOURCES` (core-relative); plain copies named by `NAME_WE` stem | `tests/faults.cmake:26-30` (reads `IVS_CORE_SOURCES`), `:45-52` (plain copies), `:77-122` (`ivs_fault_exe`), `:213-223` (owner scan: "must appear in exactly one core TU") | Generalize to a repo-relative owner list (core TUs plus `runner/` library TUs), configure-time check that stems are unique across both, a per-suite module list (suite `runner` links plain copies of core and runner lib, never `main.c`/`hostio.c`); plain copies of runner TUs get only `core/include` on their include path, so a runner TU that includes core internals fails to compile | 02 |
| 7 | Fault sites verdict scans only core | `tools/check/steps/faults-verdict.sh:131` (`grep -E '^core/src/.*[.](c\|h)$' "$FILELIST"`) | Scan `^(core/src\|runner)/.*[.](c\|h)$`; add controls for an orphan site in `runner/` and a listed id with no site in `runner/` | 02 |
| 8 | Suite registry for fault executables | `tests/CMakeLists.txt:169` (`function(ivs_fault_suite suite)`), `:211-245` (registrations: cpu-vectors, machine, memmap, ivsc) | Register suites `run` and `runner` (`SOURCES`, `INCLUDE_DIRS`, `GENERATED`); a fault id lives in exactly one file (`tests/faults.cmake:172-176`, "listed in ... and ...") | 02, 05 |
| 9 | Unlisted-source guard | `tools/check/steps/c-build.sh:39` (`grep -E '^core/.*[.]c$' "$FILELIST"` vs `build/dev/ivs-core-sources.txt`) | Add `^runner/.*[.]c$` and `^tools/pack/.*[.]c$` against `ivs-runner-sources.txt` and `ivs-pack-sources.txt` written by CMake (same `file(WRITE ...)` plus `file(COPY_FILE ... ONLY_IF_DIFFERENT)` idiom as `core/CMakeLists.txt:70-72`) | 02, 08 |
| 10 | Text-hygiene rules by directory | `tools/check/integrity.cmake:60` (ASCII for `data\|fixtures\|tools\|.github\|core\|include\|tests\|cmake`), `:65-67` (SPDX line for `tools/**.sh\|cmake`, `core\|include\|tests/**.c\|h\|sh\|cmake`) | `runner/**` and `tools/pack/*.c` are in neither list: add `runner` to both patterns and `tools/.*[.](c\|h)` to the SPDX pattern, with a control each (`integrity-missing-spdx` on a `runner/` file) | 02 |
| 11 | API symbol and status pin | `tools/check/api-effects.cmake:55-65` (`IVS_OK`/`IVS_E_*` defines vs `data/expected/api-statuses.ids`), `:131-146` (one-line `IVS_API ` declarations), `:175-198` (inventories) | Add a generic header-define pin section: `IVS_STOP_*` vs `data/expected/api-stop-reasons.ids`, `IVS_CTRL_*` vs `data/expected/input-bits.ids` (use literal `(1u << n)` defines, not a function-like macro, which the regex cannot read), and recommended `IVS_REASON_*` vs `data/expected/api-reasons.ids`; new tokens with one control each | 05, 07 |
| 12 | Effect table rows | `data/api/api-effects.json` (10 calls, 27 rows today: gate `api-calls=10 api-rows=27`); `ivs_machine_state_digest` row says "a domain other than 0..4 (the reserved 16..18 included)" at `:304` | Add calls `ivs_machine_run`, `ivs_machine_cpu_state`, `ivs_machine_peek`, `ivs_run_stop_name`; amend the digest row and retarget test `machine.refuse.digest.domain-16` (`tests/machine/machine_digest.c:572`, `data/expected/tests/machine.ids:113`) to a still-invalid domain (for example 19) | 03, 04, 05, 06, 07 |
| 13 | Header self-containment suite enumerates `core/include/ivs/*.h` | `tests/headers-tap.sh:40-47`; `data/expected/tests/headers.ids` (6 ids today) | A new public header `ivs_run.h` adds ids `headers.c17-ivs-run` and `headers.cxx-ivs-run skip-ok`; give the script an optional include-dir argument for the installed-header pass (D-38) | 04 (or 05), 13 |
| 14 | Instance size and digest field table | `core/src/machine/machine.h:26` (`#define IVS_MACHINE_BYTES 113456u`), `:52-53` (`_Static_assert(sizeof(struct ivs_machine) == IVS_MACHINE_BYTES, ...)`), `core/src/machine/machine_digest.c:39` (`static const ivs_digest_field ivs_digest_fields[] = {`) | Add one observation struct, its digest rows (domains 16, 17, 18), update the constant and layout comment; see Pitfall 1 for the alignment hole | 03 |
| 15 | Digest refuses 16..18 today | `core/src/machine/machine_digest.c:237` (`domain < IVS_DIGEST_CPU \|\| domain > IVS_DIGEST_INSTANCE`), `:263` (`domain > IVS_DIGEST_INSTANCE`); macros `IVS_DIGEST_RESERVED_*` in `core/include/ivs/ivs_machine.h` | Accept 16..18 (rename the macros, same numbers); the per-domain loop already filters on `f->domain == domain` (`:197`) | 03 |
| 16 | Digest tests accept domains 1..4 only | `tests/machine/machine_digest.c:199` (`d <= 4u`), `:251` (`per_domain[5]`), `:257` (`f->domain < 1u \|\| f->domain > 4u`), `:308`, `:324`, `:363`, `:475` | Generalize to the domain list {1,2,3,4,16,17,18}; for 16..18 flipping a row must change only that domain and NOT the composite (composite stays v1 over 1..4), so `md_changed_only(..., check_comp)` needs the inverse assertion for those domains | 03 |
| 17 | Reset and the step seam | `core/src/machine/machine_run.c:38-47` (`ivs_run_power_on`), `:84-124` (`ivs_machine_step_unit_internal`), `:31-33` (`STATIC_CARRY` site) | Reset clears the observation rows; the run loop calls the step seam one unit at a time; the seam stays internal | 03, 05 |
| 18 | Profile registry generator | `cmake/IvsMemmapTable.cmake:240-262` (registry scalars: `mm_uint`, `mm_hex`; `ticks_per_cpu_cycle` at `:258-262`); `data/expected/profile-fields.ids` (must equal the members of `fields`) | Emit `scanlines_per_frame`, `cpu_cycles_per_scanline`, `crystal_hz` num/den, `ticks_per_stic_clock`, `ticks_per_psg_clock` and the new `input.controllers` into `ivs_profile_desc` (`core/src/memmap/memsys.h`); add `input.controllers` to the real profile and `profile-fields.ids` (the `_good/tree` profile is a 2-field toy, no change there) | 05, 07 |
| 19 | CMake: OBJECT libs plus `$<TARGET_OBJECTS:>` into a STATIC archive; absolute PUBLIC include dir | `core/CMakeLists.txt:46-61` (loop), `:59-61` (`add_library(ivs_core STATIC ...)`, `target_include_directories(ivs_core PUBLIC "${CMAKE_CURRENT_SOURCE_DIR}/include")`) | Keep the loop; export through `install(TARGETS ivs_core EXPORT ...)` with `$<BUILD_INTERFACE:...>`/`$<INSTALL_INTERFACE:include>`, `EXPORT_NAME core`, `POSITION_INDEPENDENT_CODE ON` on the objects, `add_library(ivs::core ALIAS ivs_core)`; a sanitized tree refuses to install | 13 |
| 20 | Presets | `CMakePresets.json` (`base` hidden, `dev`, `release`; `release` is `-O2 -DNDEBUG`, `IVS_WERROR ON`) | Add `CMAKE_INSTALL_LIBDIR=lib` to the `base` cache variables | 13 |
| 21 | ctest registration and SKIP code | `tests/CMakeLists.txt` (`add_test`, `SKIP_RETURN_CODE 77` on every suite); `c-tests.sh:38-46` runs both presets and writes `ctest-list.txt` | Add `add_test` for suites `run`, `runner`, `runner-process`; ids in `data/expected/tests/<suite>.ids`; skip-ok ids for `needs-user-exec` and the C++ header cases | 05, 10, 11 |
| 22 | `core-symbols.sh` | `tools/check/core-symbols.sh:5-35` (usage, `--forbid-writable-data`, `norm`) and `tools/check/core-symbols-allow.txt` (memcpy, memmove, memset, memcmp, `__memcpy_chk`, `__memmove_chk`, `__memset_chk`, `__stack_chk_fail`, `__stack_chk_guard`, bzero, `_GLOBAL_OFFSET_TABLE_`) | `pkg-install` reuses it on the installed archive; new core code must keep to this allowlist (no `strlen`, no `printf`, no `abs`) | 13 |

### Recommended structure (additions)

```
core/include/ivs/ivs_run.h          public: request/report/event/cpu-state types, IVS_RUN_*, IVS_STOP_*, IVS_CTRL_*; ivs.h includes it
core/src/machine/machine_bound.c    ivs_machine_run, ivs_run_stop_name, env seam (new TU; stems unique)
core/src/machine/machine_input.c    event validation/apply, input chain, controller words
core/src/machine/machine_inspect.c  ivs_machine_cpu_state, ivs_machine_peek
runner/{CMakeLists.txt,scenario.c,exec.c,report.c,outcome.c,hostio.c,main.c,*.h}   lib = all but main.c/hostio.c
tools/pack/pack.c                   boot-stub EXEC image, IVSC container, listing words dump (build tree only)
tools/check/{listing.cmake,steps/{listing,golden,pkg-install,important-gates,ci-log,ci-summary,runner-scan}.sh,ci-summary.sh}
tools/ci-record.sh                  orchestrator-only
fixtures/FX-boot-stub/{manifest.json,boot-stub.listing.txt}
fixtures/FX-diag-cpu-1/{manifest.json,diag-cpu-1.listing.txt}
data/scenarios/*.ivss  data/golden/*.{expect,digests}  data/expected/{api-stop-reasons,input-bits,api-reasons,runner-scenarios,runner-outcomes,important-gates,pkg-install-files,pkg-selftests,pkg-consumer-calls,listings,bootstub-rows}.ids
tests/{run,runner}/*.c  tests/runner-process-tap.sh  tests/installed-consumer/{CMakeLists.txt,main.c,main.cpp}
docs/{runner.md,diagnostics.md,packaging.md}
```
Core `.c` stems must stay unique across `core/src` and `runner/` because both object naming (`core/CMakeLists.txt:47-48`, `ivs_core_obj_<stem>`) and fault plain copies (`tests/faults.cmake:47`, `ivs_fault_plain_<stem>`) use `NAME_WE`.

### Pattern 1: validate-then-commit for the run call
`ivs_machine_run` copies the Phase 3 ladder discipline (`core/src/machine/machine_load.c:2-37`): a pure validate stage over `const` inputs fills a small stack plan (target, flags, event count, buffer spans, env), then a commit loop that cannot refuse. All argument classes (D-05 order) are decided before the first unit; the only in-loop refusal is the internal step status, which maps to stop reason INTERNAL with state at the last whole boundary.

### Pattern 2: produce/consume split for every build-tree judgement
Producers are ctest suites writing under `$IVS_TEST_OUT_DIR` (set per preset by `c-tests.sh:38`); consumers are pure text steps reading only `build/gate/` and `data/` so controls can drive them with overlays. Apply to: runner reports and goldens (`runner-process` suite produces `build/gate/<preset>/runner/<id>.report`; the pure `golden` step compares dev with release, body with golden, headers with the tier vocabulary).

### Pattern 3: in-step selftests for steps a control cannot drive
`c-purity` (`tools/check/steps/c-purity.sh:61,364`, `purity-selftest=n/n`) is the template: mutations built in `$tmp`, each must fail with the intended token, a missing case counts as a failure. Use for `pkg-install` (D-40, ten mutants), `ci-summary` (backtick stripping, unwritable `GITHUB_STEP_SUMMARY` keeps the exit status, empty-skips renders "none", FAIL lines capped at 40) and the `ci-log` parser.

### Pattern 4: fail-closed generators and checkers
`cmake -P` with `gate_fail` tokens and per-token caps (`tools/check/vectors.cmake:77-83` `vfail`, `api-effects.cmake` `afail`); `tools/check/listing.cmake` copies `decle_leaves` (`vectors.cmake:323-364`) and `program_word` (`:367-385`) and the row-label check at `:600-618`.

### Anti-patterns
- Applying events at call exit or at call start instead of before each unit (breaks slicing invariance and the floor semantics).
- A second listing decoder that is "also checked against itself": `tools/pack` is the only image decoder; `listing.cmake` checks text grammar only; the independent comparison is pack's word dump against `cmake`'s own parse of the same listing (a TAP id), never pack against the core.
- A pure step reading `build/` outputs not under `build/gate/` (controls cannot supply them).
- Fault sites in `hostio.c` or `main.c` (no in-process suite can reach them; use a shell-judge control instead).
- A golden that a gate step regenerates; only `ivs-run --update-golden`, run by a human or an orchestrator command, rewrites one (D-11).

## Don't Hand-Roll

| Problem | Don't Build | Use Instead | Why |
|---------|-------------|-------------|-----|
| TAP harness, exact inventories, nonce, stale detection | a new harness | `tests/harness/ivs_test.[ch]`, `tests-inventory` (`tools/check/steps/tests-inventory.sh:34-148`), the `headers-tap.sh` shell TAP shape for `runner-process` | exact id sets, plan and trailer counts exist |
| Negative controls | a new mechanism | `tools/check/run-controls.sh` overlays on `_good/tree` | 4 lanes, inventory exact |
| Named faults | a new loop | `data/expected/faults/*.ids` + generalized `tests/faults.cmake` + `faults-verdict` | per-fault intended-reason check exists |
| Independent SHA-256 | a second C implementation | `cmake -E sha256sum`, `string(SHA256)` | different implementation, zero dependency |
| Producer/judge for build products | ad-hoc greps in one step | the `c-faults`/`faults-verdict`, `c-tests`/`tests-inventory` split | controls can drive the judge |
| Listing grammar check | parse listings in the packer | `listing.cmake` copying `vectors.cmake` rules | one rule set, one set of controls |
| Package files | `configure_package_config_file` | a hand-written two-line `ivsConfig.cmake` plus `write_basic_package_version_file` | no paths to resolve (D-33) |
| Exit-code and signal plumbing | threads, SIGALRM | slices of `run()`, `signal()` plus a `volatile sig_atomic_t`, an in-process hook (`--self-signal-after-slices`) | deterministic, no sleeps (D-21) |
| JSON in C | a JSON parser | IVSS line format; `cmake -P` for JSON data | smaller fuzz surface (D-18) |

**Key insight:** every piece of verification machinery Phase 4 needs is a variation on something that already passed review in Phase 2 or 3. The risk is not invention; it is the number of places a variation must be registered at once (inventory, toy tree, controls list, gate-steps list, api table) and the cost of getting one wrong on a hosted-only toolchain.

## Plan Decomposition (17 plans; each ends gate-green; tracer first)

Conventions for every plan: it lands its code, its inventory lines, its toy-tree fixtures, its controls and its faults in one commit series and ends with `sh tools/gate.sh` green; a plan that adds a step also adds the step to `data/expected/gate-steps/`; a plan that adds a test id adds it to `data/expected/tests/<suite>.ids`; a plan that adds a public `IVS_API` line adds its `api-effects.json` call and rows. Fault ids below are proposals (final names belong to the plans); every fault needs a must-fail id, an allowed-prefix column and a must-pass id (`data/expected/faults/machine.ids:1-30` shows the form).

| # | Plan | Reqs | Depends on | Core deliverables | New faults / controls (proposal) | Gate impact |
|---|------|------|------------|-------------------|----------------------------------|-------------|
| 01 | CI measurement and parity hardening | PKG-03, PKG-04 | none | `gate-env`, `gate-cpu` lines (POSIX `times`); `tools/check/ci-summary.sh` through the EXIT trap (headline tokens, "Skips and missing" table, 8 slowest steps, first 40 FAIL lines in a fence, backticks stripped); step `ci-summary` (selftest of the helper); step `ci-log` over `.planning/ci-runs.tsv` (exact header, 13 numeric fields, unique run ids, non-decreasing dates); `tools/ci-record.sh <run-id>` (orchestrator-only, two `gh` calls); `ci-parity` extension: `actions/checkout` is the only allowed `uses:`, tokens for `secrets.`, `strategy:`, `runs-on` other than `ubuntu-24.04`; `gate.yml` `timeout-minutes` 10 to 5; toy `ci-runs.tsv` in `_good/tree` | controls: `ci-parity-action-cache`, `-action-upload`, `-secrets`, `-matrix`, `-runs-on`, `ci-log-header`, `-fields`, `-duplicate`, `-order` (9); selftest cases for the helper (no controls: it is not a step) | under 0.5 s; lands first so every later hosted push records `gate-env`/`gate-cpu` |
| 02 | Fault framework generalization and runner tracer | RUN-04, RUN-02 | none | `tests/faults.cmake` repo-relative owner list and per-suite module list; `faults-verdict.sh:131` scan extended to `runner`; `c-build.sh:39` unlisted-source guard for `runner/` and `tools/pack/`; `integrity.cmake:60,65-67` extended to `runner/` and `tools/**.c`; `runner/` skeleton: `outcome.c` (pure outcome precedence and exit-code mapping, D-20), `ivs_runner_lib` target linking only `ivs_core`; suite `runner` (first ids: outcome table, exit-code map, precedence); step `runner-scan` (pure awk: every `runner/*.c` except `hostio.c` and `main.c` includes only ISO C headers, `ivs/*.h` and runner-local headers; no `_POSIX_C_SOURCE` outside `hostio.c`) | faults `OUTCOME_GUEST_AS_HARNESS`, `OUTCOME_TIMEOUT_AS_GUEST`, `OUTCOME_MISSING_FIXTURE_EXIT0`, `OUTCOME_CANCEL_AS_TIMEOUT` (suite `runner`, `data/expected/faults/runner.ids`); controls: `faults-orphan-site-runner`, `faults-missing-site-runner`, `integrity-missing-spdx-runner`, `integrity-non-ascii-runner`, `runner-scan-posix-header`, `runner-scan-internal-include` (the `c-build` unlisted-source guard reads a build-tree list, so it has no control; the plan proves it once with a throwaway untracked `runner/*.c`) | +1-2 s c-build (new TU, four fault exes); choke point 1 |
| 03 | Observation rows and digest domains 16-18 | RUN-05 | 02 | FIRST task: record a stepped-state literal set from the unmodified Phase 3 code (composite and four domain digests after load stand-ins A, reset, and 100 units; labelled repeatability only) next to `md_fresh_composite`; then one observation struct before `slot[]` (VIDEO chain+count, AUDIO chain+count, INPUT words[4], count, floor, chain), 4-byte explicit pad row, new `IVS_MACHINE_BYTES`, digest rows with domains 16/17/18, macros renamed `IVS_DIGEST_VIDEO/AUDIO/INPUT`, `ivs_machine_state_digest` accepts 16..18, reset clears the rows, `docs/time-base.md` one line | faults `OBS_NOT_CLEARED_ON_RESET`, `DIGEST_DOMAIN_SWAPPED` (maps 16 to 17); retargeted row `machine.refuse.digest.domain-19` | choke point 2: touches every `IVS_MACHINE_BYTES` user (tests only use the macro) |
| 04 | Guest observation | API-02 (support), RUN-01 | 03 | `core/include/ivs/ivs_run.h` (types so far: `ivs_cpu_state_report`), `ivs_machine_cpu_state(const ivs_machine *, ivs_cpu_state_report *)`, `ivs_machine_peek(const ivs_machine *, uint16_t addr, uint32_t count, uint16_t *out)` over `ivs_memsys_read(const ivs_memsys *, ...)` (`core/src/memmap/memsys.h`, already const); effect rows (BAD_ARG, BAD_HANDLE, OUT_OF_RANGE) with whole-storage `memcmp`; headers ids; suite `run` created (`data/expected/tests/run.ids`, `tests/run/`) | faults `PEEK_ADVANCES_TIME`, `PEEK_GATE_IGNORED`; peek is allowed OFF and POWERED | small |
| 05 | Run API | API-02, RUN-05 (slicing) | 04 | `ivs_machine_run` (`machine_bound.c`), `ivs_run_stop_name`, request/report types, stop reasons pinned by `data/expected/api-stop-reasons.ids` and the new generic pin check in `api-effects.cmake`, run-refusal reasons 34+ (one append-only allocation, in the header), `ivs_info` appended layout and timebase fields, profile registry timebase fields (nominal frame ticks derived, "nominal, not a frame event"), effect rows for run and stop name, slicing-invariance family with five slicing patterns against a unit-by-unit reference over HLT and guest-fault stand-ins (D-07), overshoot assertions | faults `RUN_BUDGET_CARRY`, `RUN_STOP_BEFORE_OVERSHOOT`, `RUN_TARGET_UNCHECKED`, `RUN_FAULT_STEPS`; controls `api-stop-renumbered`, `api-stop-unpinned` | choke point 3: largest ladder; about 60 test ids |
| 06 | Output buffers (API-04) | API-04 | 05 | `ivs_machine_run_env_internal(m, req, rep, env)` (declared only in `machine/machine.h`); public call is a wrapper with the profile's env; real layout all zero; refusals BUFFER_TOO_SMALL before any step with whole-storage `memcmp` and untouched caller buffers (poison pattern), exact fit OK, NULL-with-capacity, overlap with the instance and between buffers (A-08 style `uintptr_t` differences); a gate-visible assertion that the real layout is all-zero | faults `RUN_CAPACITY_AFTER_STEP`, `OUTBUF_PARTIAL_WRITE`; test that both entry points give equal whole-storage results for equal env | medium |
| 07 | Input plumbing and the INPUT domain (D6) | RUN-01, RUN-05 | 06 | `IVS_CTRL_*` literal-bit defines pinned by `data/expected/input-bits.ids`; `ivs_input_event` (32 bytes); request fields; apply before each unit with `at_tick <= unit start`; `input_consumed`, `input_floor_tick`, `controller_now[4]`; whole-array validation before any write with failing index and reason; INPUT chain `h(n+1) = SHA256(h(n) \|\| be64(at_tick) \|\| be32 words)`, `h0 = SHA256("IVS-INPUT-LOG-V1")`; profile `input.controllers`; independent chain oracle through `cmake -E sha256sum`; slicing fuzz with events inside the overshoot window; guest-invisibility test; discrepancy edits (C14 decided, C15 recorded, new entries with D10 resolvers) | faults `INPUT_APPLY_AT_RUN_START`, `INPUT_LOG_SLICE_RELATIVE_TICK`, `INPUT_NO_FLOOR_CHECK`, `INPUT_STATIC_CARRY`, `RUN_EVENT_AT_TARGET_ONLY`, `RESET_KEEPS_INPUT_LOG`; controls `api-input-bit-duplicate`, `api-input-bit-unpinned` | choke point 4; last plan that edits public headers |
| 08 | Listing checker, `tools/pack`, boot stub | RUN-06 | 05 (run + observation for the path test) | `tools/check/listing.cmake` and step `listing` (grammar `<ADDR> <hex4> <bit groups> <ROW\|-> <asm>`, hex equals bits, row is a leaf of the decle, address continuity, header keys, header/allowed-row allowlist `data/expected/bootstub-rows.ids`, 22-word and 16-nonzero-word guards, vector words present at `reset.r7` and `interrupt.iab_value`); `data/expected/listings.ids`; `tools/pack/pack.c` (EXEC image, IVSC container, `--dump-words`); CMake target `ivs_pack` and build-tree images; `fixtures/FX-boot-stub/` (manifest committed listing, MIT); `docs/diagnostics.md` (no-routines, no-services review rule); suite `run` path tests: R7 at unit starts 1000, 1010, 1011, 1013, 5000 and one INTR pulse trap test; pack-vs-cmake word-dump TAP id and IVSC trailer vs `cmake -E sha256sum` | controls: one per listing token (about 10), including a `JSR` row, 40 words and a moved vector; fault-free (content defects are controls) | `listing` about 0.3 s; two-preset build of `ivs_pack` |
| 09 | Original diagnostic `ivs-diag-cpu-1` | RUN-06 | 08 | `fixtures/FX-diag-cpu-1/diag-cpu-1.listing.txt` (56 words at `$5000`, 04-DISCUSS-diag section 5), manifest, hand derivation (arithmetic, GSWD layout pdf 56, stack direction C17, cycle table) committed as `docs/diagnostics.md` section and as the `# derivation:` header of `data/golden/diag-cpu-1.expect`; predicate (10 memory words, R0-R6, stop reason halted; R7, flags and time excluded); API-level test of the whole chain in suite `run`; slicing test re-run on the real diagnostic (D-07); claim CL-0008 (tier `original-diagnostic`, provisional) with shared-misreading risk stated; discrepancies for the `$5000` entry convention and `$0340` stack; optional libasm cross-check recorded only as an executed result, never gating | the four program mutations are TAP ids in Plan 11's process suite; here: `diag-fixture-mutated-decle` control (listing hash vs manifest) | small |
| 10 | Runner library | RUN-01, RUN-02 | 07, 02 | IVSS v1 parser (caps, charset, hand integer parser, line-numbered reason tokens), executor (storage alignment by hand, load, `expect-load`, reset, slices, `stop target\|halted\|faulted`, asserts over `ivs_machine_cpu_state`/`peek`, input lines with `at_tick=` and `at_cycle=`, names from `input-bits.ids`), report writer (key order, `ivsr 1` ... `report.end n`), host vtable (`read_file`, `now_ms`, `cancel_requested`); suite `runner`: parser table (about 40 rows), seeded fuzz under ASan/UBSan, slice invariance with a fake host, fake-clock timeout and cancel | faults `RUNNER_ASSERT_ALWAYS_PASS`, `RUNNER_EXIT_ZERO_ON_GUEST_FAIL`, `RUNNER_OVERSHOOT_NOT_REPORTED`, `RUNNER_DOTDOT_ACCEPTED`, `RUNNER_READ_UNBOUNDED`, `RUNNER_CANCEL_IGNORED`, `RUNNER_WALL_AS_GUEST_FAIL`, `RUNNER_MISSING_AS_HARNESS`, `RUNNER_KEY_ORDER_SWAPPED`, `RUNNER_SLICE_DROPS_REMAINDER`, `RUNNER_STOP_EXPECT_IGNORED` | choke point 5 (size about 1,000 lines); gcc-hazard review |
| 11 | `ivs-run` executable and process suite | RUN-01, RUN-02 | 10, 09 | `hostio.c` (only `_POSIX_C_SOURCE 200809L`; `fopen` plus `fstat` on the open descriptor, regular files only, cap+1 reads; `clock_gettime(CLOCK_MONOTONIC)`; `signal()`), `main.c` (`--content-dir --exec --grom --cart --max-ticks --wall-timeout-ms --self-signal-after-slices --slice-ticks --update-golden --help`), `tests/runner-process-tap.sh` (suite `runner-process`), scenarios and `data/expected/runner-scenarios.ids` (one demonstrating scenario per outcome, plus the four diagnostic program mutants, `needs-user-exec` skip-ok), `data/expected/runner-outcomes.ids`, judge self-controls as TAP ids (fake `ivs-run` shell scripts returning a wrong exit code, a mutated body, a missing `report.end`, an outcome/exit disagreement); `docs/runner.md`; run `tools/gate-linux.sh` before the first push | judge controls are TAP ids, no framework change | about 1-3 s (two presets) |
| 12 | Goldens and the `golden` step | RUN-05 | 11 | pure step `golden`: header rules (`# tier:` in `data/expected/evidence-tiers.ids`, non-empty `# oracle:`, `self-recorded` forces `deterministic-repeatability`, `domains` lines `observed\|none`, no `none` domain listed as observed), `.expect` assert set equals the scenario's assert lines, body equals report, dev report equals release report byte for byte, `runner-outcome-uncovered`, repeat-run equality (the process suite runs each digest scenario twice); `data/golden/*.digests` written by `--update-golden` only; toy golden and toy report in `_good/tree` | controls: `golden-header-missing-tier`, `-missing-oracle`, `-self-recorded-wrong-tier`, `-empty-domain-observed`, `-mismatch`, `-expect-drift`, `-dev-release-diff`, `-user-scenario-tracked` | about 0.2 s |
| 13 | Install/export and `pkg-install` structure | PKG-01 | 07 | root `project(ivs VERSION <parsed> LANGUAGES C)` with the version read from `ivs.h` before `project()`; `install(TARGETS ... EXPORT ivsTargets)`, `ivs::core` alias and `EXPORT_NAME core`, include dirs split with generator expressions, hand-written `ivsConfig.cmake`, version file `SameMinorVersion ARCH_INDEPENDENT`, LICENSE under the doc dir, `POSITION_INDEPENDENT_CODE ON`, sanitized tree refuses install, `CMAKE_INSTALL_LIBDIR=lib` in the base preset; step `pkg-install` part 1: install from `build/release`, `mv` into a path with a space, delete the original, exact `data/expected/pkg-install-files.ids`, header `cmp`, archive sha256 equals the build-tree archive, no `core/` or `build/` string in installed files or archive, symbol rules (every `IVS_API` name defined; set equals the `api-effects.json` call names; every external definition `^ivs_`), headers alone (C17, C++17) through `headers-tap.sh` with an include-dir argument; selftests 1-8 of D-40 | selftest ids `data/expected/pkg-selftests.ids` (exact); no controls (step needs a built tree) | 1-2 s; choke point 6; run `tools/gate-linux.sh` before pushing |
| 14 | Installed consumer | PKG-01 | 13, 09, 12 | `tests/installed-consumer/` (copied outside the repo at step time), `find_package(ivs 0.1 CONFIG REQUIRED)`, strict flags, `NO_SYSTEM_FROM_IMPORTED ON`, registries off, path env unset in a subshell; create, load stub plus diagnostic (images from `ivs_pack` output passed as arguments), refusals with digest unchanged, reset, bounded run, per-domain digests, second fresh instance equal; stdout vs `data/expected/pkg-consumer.golden` ("regression pin, emulator is oracle"); digests equal the runner golden of the same scenario; `data/expected/pkg-consumer-calls.ids` (names the consumer references, `nm -u` on the object); `ninja -t deps` positive-presence and `ivs_DIR` under the moved prefix; direct C++ link of a tiny `main.cpp` (visible SKIP without a C++ compiler); selftests 9-10; `docs/packaging.md`; consumer exercises the INPUT domain with two events across two slices | mutants are the ten selftests | 2-3 s total for `pkg-install`; run `tools/gate-linux.sh` |
| 15 | RUN-04 registry and audit | RUN-04 | 12, 14 | `data/expected/important-gates.ids` (`<gate> <fault\|control\|mutant> <id> <REQ>`), step `important-gates` (each id exists in the matching inventory: faults files, controls inventory, `pkg-selftests.ids`; every Phase 4 requirement has a line; toy fixtures in `_good/tree` including a toy `data/expected/controls/*.ids`); audit of every Phase 4 gate against D-46's three criteria; fill gaps found | controls `important-gates-unknown-id`, `important-gates-uncovered-gate` | under 0.2 s |
| 16 | CI budget rule, branch protection and release design records | PKG-03, PKG-04 | 01, hosted data | `.planning/CI-EFFICIENCY.md` (budget rule D-45, levers in order, measured Phase 4 rows), `.planning/AUTONOMY.md` branch-protection recommendation with the ruleset JSON shape and "orchestrator-assist, blocks nothing", release design (`release.yml` on `v*`, re-run gate, `git archive`, install tarball, `SHA256SUMS`, allowlist), PROJECT.md Decision Register rows D4, D6, D12; first `ci-runs.tsv` rows from the orchestrator | none | none |
| 17 | Close-out | all | all | `docs/` pass (claims-word and bare-clock rules), REQUIREMENTS and ROADMAP traceability, discrepancy and claim finalization (Data File Updates), STATE, final `tools/gate-linux.sh`, measured gate-time and the D-45 decision, SC1-SC5 verification table | none | none |

Suggested waves (file-set disjointness): W1 {01, 02}; W2 {03}; W3 {04}; W4 {05}; W5 {06, 08}; W6 {07, 09}; W7 {10}; W8 {11, 13}; W9 {12, 14}; W10 {15, 16}; W11 {17}. Plans 03-07 are strictly sequential (shared `machine.h`, digest table, `api-effects.json`, `run.ids`, `tests/CMakeLists.txt`). Plans 08 and 06 may overlap (disjoint). Plan 09 may overlap 07 only if each uses its own TU in `tests/run/` and the id-file edits are merged by the orchestrator; otherwise sequence 07 then 09.

**Sequencing constraints (hard):** 02 before any `runner/` code and before every runner fault; 03 first task (pin stepped-state literals) before the digest table is touched; 03 before 07 (INPUT rows) and before 05 (reset clearing); API (04-07) before the runner (10); 08 (`tools/pack`, listing step) before 09 (diagnostic) and before 11 (images for scenarios); 13 after the public header set is final (07), because `data/expected/pkg-install-files.ids` and the headers-alone pass list every installed header; 14 after 09 and 12 (needs the diagnostic and the runner golden); 15 after every plan that adds an important gate; 16 after at least three hosted gate-time rows exist.

**Likely choke points:** 02 (generalization breaks every Phase 2/3 fault if wrong; verify with the full existing 119-fault run), 03 (digest and layout), 05 (ladder and slicing), 07 (largest refusal matrix plus an independent oracle), 10-11 (runner size, first POSIX code, first hosted-only risk), 13-14 (CMake 3.25 to 4.x, relocation, `ar`/`nm` portability). Run `sh tools/gate-linux.sh` before the first push of Plans 02, 10, 11, 13, 14 and 17 (about 3 min each under emulation).

## Common Pitfalls

### Pitfall 1: the observation struct opens an alignment hole the digest coverage test reports
**What goes wrong:** `struct ivs_machine` places `mem` at offset 56 and `mem` is 113,228 bytes (`core/src/memmap/memsys.h` layout comment: 76 header + 113,152 backing), so the first byte after `mem` is at offset 113,284, which is 4 mod 8. Any `uint64_t` member of a new struct placed there makes the compiler insert 4 implicit padding bytes; `md_coverage` (`tests/machine/machine_digest.c:245-290`) requires every instance byte covered exactly once, so it fails with "byte N covered 0 times" (and `sizeof` is not what the static assert says).
**Why it happens:** the Phase 3 layout has only 4-byte members after `mem` (`ivs_machine_slot` is all `uint32_t`/`uint8_t`, `core/src/machine/machine.h:29-37`).
**How to avoid:** put an explicit `uint32_t reserved` (a `PAD` row, never serialized) before the first `uint64_t`, or lead the struct with the four `uint32_t` words so the following `uint64_t` is 8-aligned; recompute the size: 24 + 32 + 113,228 + obs + 168 (slots) rounded up to 16 [INF: with a 144-byte observation struct the instance is 113,600 bytes; the static assert settles it]. Place the struct BEFORE `slot[]`: `IVS_FAULT_DIGEST_SKIPS_IMAGE_TABLE` skips every row with `f->offset >= offsetof(struct ivs_machine, slot)` (`machine_digest.c:203-205`), which would silently also skip rows placed after the slots and change that fault's reach.
**Warning signs:** `machine.digest.coverage` fails; `_Static_assert` text "instance size changed".

### Pitfall 2: composite must NOT change when a 16..18 row changes
**What goes wrong:** `md_changed_only` (`tests/machine/machine_digest.c:344-380`) asserts the composite differs after a change; for domains 16..18 the composite is deliberately unchanged (D-10), so the unmodified test fails, or worse, an implementer "fixes" it by adding the new domains to the composite and invalidates every Phase 3 literal and golden.
**How to avoid:** Plan 03 pins the stepped-state literals first, generalizes the tests with an explicit per-domain "in composite" flag, and asserts the inverse for 16..18; the whole-storage `memcmp` oracle already covers the hidden state. The slicing test must therefore compare the seven domain digests, not the composite alone.

### Pitfall 3: input event timing differs between two DISCUSS files
**What goes wrong:** 04-DISCUSS-runapi 4.5 reads `loop { apply events with tick <= now; if exit_condition break; step }`; D-14 reads "first CPU-unit boundary whose start tick is `>= at_tick`" and `input_floor_tick` = start of the last begun unit + 1. With apply-at-exit, an event applied at the exit boundary can be followed in the next call by an event with an earlier tick that is still `>=` the floor, applied out of tick order.
**How to avoid:** apply only immediately before beginning a unit; at exit nothing is applied; `controller_now` is the state in force for the last begun unit; the floor is state in the instance (INPUT row), so it survives across calls; reset clears it. Test: events with ticks inside the overshoot window, equal ticks (array order kept), and a tick exactly at a slice target.

### Pitfall 4: strict `-std=c17` hides POSIX names on glibc; clang does not
**What goes wrong:** `clock_gettime`, `fstat`, `CLOCK_MONOTONIC` compile on Apple clang with extensions off but fail on gcc/glibc unless `_POSIX_C_SOURCE` is defined before the first include; this is the hosted-only class that cost two CI round trips in Phase 3 [CITED: CI-EFFICIENCY Phase 3 section].
**How to avoid:** `hostio.c` alone defines `_POSIX_C_SOURCE 200809L` first; `runner-scan` (Plan 02) fails any other runner file that includes a non-ISO header; `tools/gate-linux.sh` before pushing Plans 10-11.

### Pitfall 5: gcc evaluates call arguments in an unspecified order
**What goes wrong:** a test or helper whose arguments depend on each other's side effects reads a stale value under gcc and ASan aborts (Phase 3: `tests/machine/machine_refuse.c`, [CITED: CI-EFFICIENCY, section "hosted-only failures"]). Run-API tests build requests and read reports in the same expression easily.
**How to avoid:** one call per statement when an argument reads state another argument changes; no `f(g(&x), x.len)`; `-Wsequence-point`-class review in the code review checklist for Plans 05-07, 10.

### Pitfall 6: `-fPIC` and the ELF `core-symbols` allowlist
**What goes wrong:** `POSITION_INDEPENDENT_CODE ON` (D-34) changes gcc output: address-taken functions import `_GLOBAL_OFFSET_TABLE_` (already allowlisted in `core-symbols-allow.txt`). `c-purity` compiles its own archives `-fno-pie` on purpose (`c-purity.sh:45-46` comment) so it is unaffected, but the ctest suite `core-symbols` and the new `pkg-install` symbol rule check the shipped PIC archive, where a new import appears only on the hosted toolchain.
**How to avoid:** run `tools/gate-linux.sh` after Plan 13; keep new core code free of libc beyond the allowlist (no `strlen`, no `abs`, no `printf`); the `pkg-install` symbol rule re-runs `core-symbols.sh` on the installed archive.

### Pitfall 7: a step that needs a built tree cannot be driven by controls
**What goes wrong:** `run-controls.sh` copies `_good/tree` (text only) and runs the step with `IVS_GATE_DIR=<tree>/build/gate` (`run-controls.sh:149-153`); there is no compiler output, no `build/release`. A control for `pkg-install` would need install plus configure plus build per control (about 30 s for ten, [CITED: 04-DISCUSS-pkg section 5]).
**How to avoid:** `pkg-install` uses in-step selftests with a stat `pkg-selftest=<ok>/<total>` and an exact inventory `data/expected/pkg-selftests.ids`, so a missing case is a failure; `important-gates` treats those ids as `mutant` mechanism entries. `ci-summary.sh` is not a step at all (`tools/check/ci-summary.sh`), so its checks live in the `ci-summary` selftest step.

### Pitfall 8: `ninja -t deps` and paths with a space
**What goes wrong:** an awk that splits on whitespace truncates `.../reloc pfx/include/ivs/ivs.h`; the positive-presence check then passes or fails for the wrong reason.
**How to avoid:** read lines with a fixed 4-space prefix strip (`sed 's/^    //'`); `ninja -t deps` printed the path unescaped with the indent [VERIFIED: probe this session]; assert presence of `ivs.h`, `ivs_machine.h`, `ivs_status.h` and `ivs_run.h` under the moved prefix; compare physical paths with `pwd -P` (macOS `/var` vs `/private/var`).

### Pitfall 9: `times` placement and `$?` after an EXIT trap
**What goes wrong:** `times` reports children CPU only for waited-for children; reading it before `wait "$ctl_pid"` omits the controls; and an EXIT trap that ends with a failing command must not change the gate's exit status (POSIX keeps the status unless the trap calls `exit`, shells differ in practice).
**How to avoid:** call `times` after the controls `wait` and before the final lines (`tools/gate.sh:104-111`, the `wait "$ctl_pid"` block); the helper runs as `... || :`; the selftest case `ci-summary-unwritable-keeps-exit` runs the helper with an unwritable `GITHUB_STEP_SUMMARY` and asserts the surrounding shell's status is unchanged.

### Pitfall 10: goldens and reports can leak private text
**What goes wrong:** a report echoing a scenario path, an environment value or wall time would put a home path into a tracked golden (`tools/hygiene.sh` home/temp patterns) or make dev and release reports differ.
**How to avoid:** report keys come from a closed table; echoed tokens are charset-restricted; no wall time (`host.calls` is the only host-dependent key and is excluded from comparisons); scripts never write `/private/tmp/...` or tilde paths into tracked files (use `${TMPDIR:-/tmp}` and `mktemp -d`, as `tools/gate.sh:9` does).

### Pitfall 11: the process suite cannot compare dev and release in one ctest run
**What goes wrong:** `c-tests.sh` builds and tests `dev` then `release` sequentially (`c-tests.sh:38-46`); during the dev run the release binary may not exist.
**How to avoid:** each preset's run writes its reports under `$IVS_TEST_OUT_DIR/runner/`; the pure `golden` step (Plan 12) compares the two sets, as `tests-inventory` judges TAP output after the fact.

### Pitfall 12: the `machine` suite has a case-table cap and every fault re-runs the whole suite
**What goes wrong:** `IVS_MACHINE_MAX_CASES 512u` (`tests/machine/machine_main.c`) bounds the table families; the machine inventory already has 381 ids, and each of the 28 machine faults runs all of them (`tests/faults.cmake` builds one executable per fault from the suite sources).
**How to avoid:** put every run, observation, input and buffer test in the new suite `run` (`tests/run/`), registered with `ivs_fault_suite(run ...)`; only the digest-table and reset changes of Plan 03 touch the `machine` suite.

### Pitfall 13: pointer-bearing structs and size assertions
**What goes wrong:** `ivs_run_request` carries `const ivs_input_event *`, `uint8_t *video`, `uint8_t *audio`; `sizeof` differs on ILP32. Phase 3 static-asserts sizes only of pointer-free structs (`_Static_assert(sizeof(ivs_load_options) == 40u, ...)`, `machine_load.c:47`).
**How to avoid:** assert `offsetof` of the leading pointer-free prefix and `sizeof(...) <= IVS_STRUCT_SIZE_MAX`; mark any 32-bit-size row skip-ok the way the `len-wrap` rows are; do not promise a v1 size in docs.

### Pitfall 14: `tests/headers-tap.sh` and the installed-header pass share ids
**What goes wrong:** the installed headers are the same files as `core/include/ivs/*.h`; running the same TAP ids against the prefix would duplicate ids in the inventory.
**How to avoid:** give `headers-tap.sh` an optional second argument (include dir); the gate's `pkg-install` calls it with a temp output dir and consumes the TAP text itself; the ctest suite keeps its six-plus-two ids.

### Pitfall 15: ROM word masking of the stub and the zero GROM
**What goes wrong:** the profile marks GROM required at reset (`system_images.grom` value `"required": 1`), so reset refuses without one; and EXEC words above `0x3FF` are refused at load (`IVS_REASON_IMAGE_WORD`).
**How to avoid:** the runner and consumer load a 2048-byte zero GROM labelled "not a GROM" (D-30); every stub word is at most `0x3FF` (the listing checker enforces 10 bits); HLT stops only after the next instruction, so every stub/diagnostic ending carries a TSTR follower (D-29; zero fill is not a trap).

### Pitfall 16: gate wall time and the control runner
**What goes wrong:** baseline on this host (load average about 20, so inflated): `gate-time total=80`, `controls` 52 s overlapped with the steps, `faults-verdict` 11 s, `hygiene-history` 10 s, `c-faults` 10 s, `c-build` 10 s, `c-purity` 8 s, `cp1610-vectors` 7 s, `c-tests` 6 s (26 steps, 231 controls, 119 fault runs) [VERIFIED: baseline `sh tools/gate.sh` run this session]. Hosted, `controls` (13-24 s) overlap steps (about 25-30 s) with `controls-wait 0` [CITED: CI-EFFICIENCY]; +45 controls is about +20 percent on the overlapped path, which stays below the steps' critical path but narrows the margin.
**How to avoid:** prefer controls and in-step selftests over faults where the defect is in a script or CMake (a control costs about 0.1-0.4 s CPU, a fault one compile plus link plus a suite run); keep the new steps pure and cheap (`listing`, `golden`, `ci-log`, `important-gates` are awk/`cmake -P` over small files); `pkg-install` reuses `build/release` (no third build); do not add `sleep` or wall-time waits to any suite (D-21 hooks). The first lever when D-45 fires and `step-time` names the step: background `c-build` for the independent steps only if `gate-cpu` shows idle cores; else less control CPU.

## Code Examples

### Run loop (sketch; D-01, D-14, runapi 4.5 amended)
```c
/* Source: derived from D-01/D-14 and core/src/machine/machine_run.c:84-124 (step seam). Validation (D-05 ladder)
 * has already run; this is the commit stage, which cannot refuse. Exit tests come first, so an event is
 * applied only when a unit is really about to begin. */
while (im->time_ticks < target) {
  ivs_cp1610_result r;
  uint64_t start = im->time_ticks;
  if (cpu_faulted(im)) break;                                        /* a FAULTED CPU is never stepped */
  if ((flags & IVS_RUN_F_STOP_ON_HALT) != 0u && cpu_halted(im)) break;
  while (consumed < count && ev[consumed].at_tick <= start) {        /* array order, equal ticks keep it */
    ivs_input_apply(im, &ev[consumed]);                              /* words in force, count + 1, chain; no early return */
    consumed++;
  }
  im->input_floor_ticks = start + 1u;                                /* start of the unit now beginning, plus 1 */
  if (ivs_machine_step_unit_internal(m, &r) != IVS_OK || im->time_ticks == start) { stop = IVS_STOP_INTERNAL; break; }
}
/* stop reason is a pure function of exit state: fault, then halt (with the flag), then target */
```
A HALTED CPU without the flag still begins one-cycle idle units, so events keep being applied while it idles; with the flag the call stops before applying anything. The reference run (`run(now + 1)` per unit) goes through the same code, so every slicing sees the same boundary sequence. The plan names the fault that breaks this order (`INPUT_APPLY_AT_RUN_START`, `RUN_EVENT_AT_TARGET_ONLY`).

### Fault generalization (`tests/faults.cmake`, sketch)
```cmake
# Source: tests/faults.cmake:26-30, 213-223 (current core-relative form)
# 1. Owner list: repo-relative paths from every module that defines faults.
get_directory_property(ivs_core_src   DIRECTORY "${PROJECT_SOURCE_DIR}/core"   DEFINITION IVS_CORE_SOURCES)
get_directory_property(ivs_runner_src DIRECTORY "${PROJECT_SOURCE_DIR}/runner" DEFINITION IVS_RUNNER_SOURCES)
set(ivs_fault_owner_sources "")
foreach(s IN LISTS ivs_core_src)   list(APPEND ivs_fault_owner_sources "core/${s}")   endforeach()
foreach(s IN LISTS ivs_runner_src) list(APPEND ivs_fault_owner_sources "runner/${s}") endforeach()
# 2. Stems (NAME_WE) must be unique across the combined list: fail at configure time.
# 3. Owner scan: file(READ "${PROJECT_SOURCE_DIR}/${src}") over the combined list; exactly one match.
# 4. ivs_fault_exe: link plain copies of every TU of the modules the suite names (core always;
#    runner library TUs for suite runner); the owner TU is rebuilt with IVS_FAULT_<ID>; runner TUs
#    get only core/include on their include path.
```

### Listing line and its checks (D-26; 04-DISCUSS-diag section 3)
```
5000 02B8 10 1011 1000  MVII  MVII #$03FF,R0     <- addr, hex4, 10 bits in groups, row id, asm text
5001 03FF 11 1111 1111  -     imm                 <- operand word: row "-"
```
Checks copied from `vectors.cmake`: `program_word` (hex equals the binary groups, `:367-385`), `decle_leaves` (the row label is a leaf of the decle and the most specific row, `:323-364`, applied as at `:600-618` to the first decle, or the second after SDBD), plus listing-only rules: address continuity, 10-bit words, header keys, row allowlist for the stub, word-count and non-zero-word guards, vector words present at the profile's `reset.r7` ($1000) and `interrupt.iab_value` ($1004).

### `times` parsing under dash and macOS sh (verified formats)
```sh
# Source: probe this session. dash: "0m0.000000s 0m0.000000s"; macOS sh: "0m0.002s 0m0.005s".
# Line 2 of `times` is the children's user and system time.
times | sed -n 2p | awk '{ for (i = 1; i <= 2; i++) { split($i, a, /m|s/); t[i] = a[1] * 60 + a[2] }
  printf "gate-cpu user=%.2f sys=%.2f\n", t[1], t[2] }'
```

### Package config skeleton (D-33, D-34)
```cmake
# Source: 04-DISCUSS-pkg 4.1 (spike observed on macOS and Linux). Before project():
file(STRINGS core/include/ivs/ivs.h _v REGEX "^#define IVS_API_VERSION_(MAJOR|MINOR|PATCH) ")   # parse, then project(ivs VERSION ...)
set_target_properties(ivs_core PROPERTIES EXPORT_NAME core)
add_library(ivs::core ALIAS ivs_core)
target_include_directories(ivs_core PUBLIC "$<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>" "$<INSTALL_INTERFACE:include>")
install(TARGETS ivs_core EXPORT ivsTargets ARCHIVE DESTINATION lib)      # no INCLUDES DESTINATION: it duplicates the include entry
install(DIRECTORY include/ivs DESTINATION include FILES_MATCHING PATTERN "*.h")
install(EXPORT ivsTargets NAMESPACE ivs:: DESTINATION lib/cmake/ivs)
# hand-written ivsConfig.cmake: include("${CMAKE_CURRENT_LIST_DIR}/ivsTargets.cmake")
# never export(PACKAGE) or export(EXPORT): a user registry entry would let a consumer find the BUILD tree
```

## State of the Art

| Old approach (Phase 3) | Phase 4 approach | Impact |
|------------------------|------------------|--------|
| Public API is lifecycle, load and digest only; stepping is an internal seam | One bounded `ivs_machine_run` plus read-only observation | hosts and the runner need nothing internal |
| Digest domains 1..4, 16..18 refused | 16..18 live, composite v1 unchanged | per-domain reports without invalidating Phase 3 literals |
| Fault owners are core TUs only | Repo-relative owners (core and runner) | runner faults are first-class |
| No install rules | One static export `ivs::core` proven in a relocated prefix | PKG-01 |
| CI is a log | Job summary, measurement lines, structural bans | PKG-03, PKG-04 |

**Deprecated/outdated:** the CONTEXT/DISCUSS statement "pin a Phase 3 composite literal first" is satisfied for the fresh state by `md_fresh_composite`; only the stepped-state pin is new work. The runapi proposal of a 16-byte `{tick, channel, value}` event and of an "electrical per channel" D6 is superseded (CONTEXT Reconciliation).

## Runtime State Inventory

Not a rename or migration phase; omitted. The one persistent-state hazard is the instance layout change (Pitfall 1), covered by the whole-storage `memcmp` tests.

## Environment Availability

| Dependency | Required By | Available | Version | Fallback |
|------------|------------|-----------|---------|----------|
| CMake | everything | yes | 4.4.3 local, 3.28.3 hosted | none needed |
| Ninja | build, `ninja -t deps` | yes | 1.13.2 | none |
| C compiler (clang local, gcc hosted) | build | yes | Apple clang 21 / gcc 13.3 | none |
| C++ compiler | headers C++17 pass, direct C++ link | yes locally (`c++`); hosted `g++` present on the image | not recorded | visible `# SKIP no C++ compiler` (ids already skip-ok in `headers.ids`; add the same for the new C++ ids) |
| dash | POSIX-sh compatibility | yes locally | 0.5.x | run `sh tools/gate-linux.sh` |
| docker | `tools/gate-linux.sh` | yes | local only | skip (local aid, never a gate dependency) |
| `gh` | `tools/ci-record.sh` | orchestrator only | n/a | executors never call it |
| libasm / any assembler | optional cross-check (D-25) | not required; not installed [ASSUMED] | n/a | record "not run" visibly; never gating |
| owner EXEC/GROM, hardware | D10, D11 | not available | n/a | mark evidence missing, keep going |

**Missing dependencies with no fallback:** none.

## Validation Architecture

### Test Framework
| Property | Value |
|----------|-------|
| Framework | In-repo harness (`tests/harness/ivs_test.[ch]`, TAP-like, `--only <prefix>`, `--list`) driven by ctest; shell TAP suites (`tests/headers-tap.sh` shape); `cmake -P` and `sh` gate steps; named-fault executables; negative controls |
| Config file | `tests/CMakeLists.txt`, `CMakePresets.json`, `data/expected/tests/<suite>.ids`, `data/expected/faults/*.ids`, `data/expected/controls/*.ids` |
| Quick run command | `cmake --build --preset dev && build/dev/tests/ivs_tests_run --only run.slice.` (and `ivs_tests_runner`, `ivs_tests_machine --only machine.digest.`) |
| Suite command | `ctest --preset dev -R '^(run\|runner\|runner-process\|machine)$'` |
| Full suite command | `sh tools/gate.sh` (the single local/CI command); before a push of Plans 02, 10, 11, 13, 14, 17: `sh tools/gate-linux.sh` |

### Success criteria to automated checks (zero human UAT)
| SC | Claim | Automated evidence (all in `sh tools/gate.sh`) |
|----|-------|---------------------------------------------|
| SC1 | A scripted scenario reports machine time, stop reason, outputs and assertions; the diagnostic reaches an independently derived predicate; boot path labelled "project boot stub, not an EXEC" | suite `runner-process` scenario `diag-boot-pass` exits 0 with the report lines `exec: project boot stub, not an EXEC sha256=...`; the report keys (time, stop reason, assertions, `output.video.bytes`, `output.audio.samples`) are checked by the `golden` step against `data/golden/diag-cpu-1.digests`; the predicate comes from `data/golden/diag-cpu-1.expect` (header `# oracle: independent-derivation`, `# derivation:` with GI manual page cites); the four program-mutant scenarios (ADDI to SUBI, BNZE to BZE, PSHR to MVO@, INCR to DECR) must exit 10 naming the clause; `listing` step proves listing hex equals bits and the row labels; `tests-inventory` proves the suites ran in both presets |
| SC2 | Five distinct outcomes, one scenario each; every important gate has a named-fault control that fails for its intended reason | `data/expected/runner-scenarios.ids` lines `<id> <exit> <outcome>` judged by `runner-process` (exit codes 0, 10, 11, 12, 13, 14, `skip-ok` only for `needs-user-exec`); `runner-outcome-uncovered` token; faults `OUTCOME_*` and `RUNNER_*` through `c-faults` + `faults-verdict` (must-fail, allowed prefix, must-pass); `important-gates` step proves each gate has a registry line and each id exists |
| SC3 | Absolute target with overshoot; slicing gives the same per-domain digests; repeats match; goldens carry tier and oracle; undersized buffer refused before any state change | suite `run`: `run.slice.*` (five slicing patterns vs unit-by-unit reference, seven domain digests at sampled ends, whole-storage `memcmp`, summed counters), `run.overshoot.*`, `run.refuse.*` and `machine.refuse.run.*` rows (BUFFER_TOO_SMALL with whole-storage `memcmp` and poison-pattern buffers), `input.slice.*` including events inside the overshoot window; process suite runs each digest scenario twice and with `--slice-ticks` variants; `golden` step header rules and dev == release; faults `RUN_*`, `INPUT_*`, `OUTBUF_*` |
| SC4 | A separate consumer built only against the installed package in a relocated prefix runs load, refusal, run and digest | step `pkg-install`: install, `mv` to a path with a space, delete the original, exact file inventory, header `cmp`, archive sha256, no path leaks, symbol rules, `ninja -t deps` positive presence, `ivs_DIR` under the moved prefix, consumer stdout vs `data/expected/pkg-consumer.golden`, consumer digests equal the runner golden, ten selftests `pkg-selftest=10/10` |
| SC5 | CI runs the same fast gate on every PR, shows skipped and missing items, never caches or uploads images, records durations | `ci-parity` (single `run: sh tools/gate.sh`, action allowlist, no `secrets.`, `strategy:`, other `runs-on`) with controls; `ci-summary` selftest (skips table, "none" when zero, FAIL lines fenced, backticks stripped, unwritable path keeps the exit status); `gate-env`/`gate-cpu`/`step-time` lines present in the gate output (checked by the selftest on a synthetic run and by the first hosted log, recorded by the orchestrator into `.planning/ci-runs.tsv` which `ci-log` validates); `files`/`manifests`/`hygiene` keep rights-bearing bytes out |

### Phase Requirements to Test Map
| Req | Behavior | Test type | Automated command | File exists? |
|-----|----------|-----------|-------------------|--------------|
| API-02 | target ends at first boundary `>= target`; overshoot; no state between calls; slicing invariance | unit + sweep | `ctest --preset dev -R '^run$'` (`run.slice.*`, `run.overshoot.*`, `run.stop.*`, `run.refuse.*`) | no, Wave 0 (Plans 04-05) |
| API-02 | observation is read-only | unit + faults | `build/dev/tests/ivs_tests_run --only run.peek.` and `run.cpu-state.` | no (Plan 04) |
| API-04 | refusal before any state change; exact fit OK; zero-length real layout | unit | `build/dev/tests/ivs_tests_run --only run.buffers.` | no (Plan 06) |
| RUN-01 | scenario end to end, input schedule, assertions | in-process + process | `ctest --preset dev -R '^(runner\|runner-process)$'` | no (Plans 10-11) |
| RUN-02 | five outcomes | unit + process | same; `data/expected/runner-scenarios.ids` | no (Plans 02, 10, 11) |
| RUN-04 | each important gate has a fault, control or mutant | static | step `important-gates`; `sh tools/gate.sh` | no (Plan 15) |
| RUN-05 | seven domain digests; repeats match; golden headers | unit + step | `ctest --preset dev -R '^machine$'` (`machine.digest.*`), process suite, step `golden` | partial (domains 1..4 exist) (Plans 03, 07, 12) |
| RUN-06 | listing check; stub path; diagnostic predicate; mutants | step + unit + process | `sh tools/gate.sh` (`listing`), `run.stub.*`, `run.diag.*`, process mutants | no (Plans 08-09, 11) |
| PKG-01 | relocated installed consumer | step with selftests | `sh tools/gate.sh` (`pkg-install`) | no (Plans 13-14) |
| PKG-03 | skips visible; no cache/upload | step + controls | `ci-parity`, `ci-summary` | partial (`ci-parity` exists) (Plans 01, 16) |
| PKG-04 | durations recorded | step | `ci-log`; `gate-env`/`gate-cpu` lines | no (Plan 01) |

### Sampling Rate
- **Per task commit:** the suite command for the touched suite (about 1-5 s) plus `sh tools/check/steps/<step>.sh` equivalents through `sh tools/gate.sh` when a step changed.
- **Per plan merge:** `sh tools/gate.sh` green (steps, controls, faults).
- **Before pushing Plans 02, 10, 11, 13, 14, 17:** `sh tools/gate-linux.sh` green (dash, gcc 13, glibc, cmake 3.28).
- **Phase gate:** full gate green on the PR; hosted gate-time recorded by the orchestrator; D-45 rule evaluated.

### Wave 0 Gaps (files that do not exist yet; each plan creates its own)
- [ ] `tests/run/*.c`, `data/expected/tests/run.ids`, `data/expected/faults/run.ids` (Plan 04-05)
- [ ] `runner/*`, `tests/runner/*.c`, `data/expected/tests/runner.ids`, `data/expected/faults/runner.ids` (Plan 02, 10)
- [ ] `tests/runner-process-tap.sh`, `data/expected/tests/runner-process.ids`, `data/expected/runner-scenarios.ids`, `data/scenarios/*.ivss` (Plan 11)
- [ ] `tools/check/listing.cmake`, `tools/check/steps/{listing,golden,pkg-install,important-gates,ci-log,ci-summary,runner-scan}.sh` and `data/expected/gate-steps/phase4.ids` (Plans 01, 02, 08, 12, 13, 15)
- [ ] toy fixtures in `tools/check/controls/_good/tree/` for each new controlled step (toy listing, golden, report, `ci-runs.tsv`, important-gates, controls inventory)
- No framework install is needed.

## Data File Updates (discrepancy log, profile fields, claims, inventories)

| File | Edit | Plan |
|------|------|------|
| `data/discrepancies.json` + `data/expected/discrepancies.ids` | C14: keep `status` within the schema's four values (`open\|partial\|resolved-primary\|withdrawn`, `tools/check/schema/discrepancy.txt`); a modelling decision is not a primary resolution, so keep `open`, rewrite `default_policy` as `unknown-default-chosen: logical switch state per controller (D6, delegated decision)` and point `note` at D6; C15: record the modelling choice (top buttons one code), stays `open`; new entries from C63 (next free after C62): disc internals and two positions closed together; unplugged connector and upper-byte read; read of a port the PSG drives as output; which connector is R14 (wiki only); input application at CPU-unit start granularity (an architecture decision, not a hardware claim); the `$5000` entry convention and `$0340` stack of the boot stub (project conventions, no source). Each `resolver` matches `^(test\|measurement\|pending):.+` (D10 items use `measurement:`); open entries state `unknown-default-chosen:` | 07, 09 |
| `data/profiles/intv-2609-ntsc-family-p1.json` + `data/expected/profile-fields.ids` | add `input.controllers` (value 2; suggested tag `tier2-first-party-adjacent`, cite `SRC-bsr-2609`, `decision` D6); nothing else in Phase 4 (the port-map, line-model and mask fields wait for Phase 5 so the inventory carries no unused unknowns); `cmake/IvsMemmapTable.cmake` emits the new registry fields (Table row 18) | 05, 07 |
| `data/claims.json` + `data/expected/claims.ids` | CL-0008 (next free; the ledger holds CL-0001..CL-0007): tier `original-diagnostic`, `standing` provisional, `artifact` a path that exists (`docs/diagnostics.md`), `depends_on` the profile fields the predicate rests on (reset.r7, cartridge.slots, ...); statement limited to "the diagnostic reaches the predicate whose expected values were derived by hand from the cited manual pages"; the note states the shared-misreading risk and that R7, flags at halt and machine time are excluded. Optional CL-0009 (tier `installed-host-behavior`, provisional) for PKG-01, and no claim for digest repeatability beyond tier `deterministic-repeatability`. Any doc line using supports/compatible/passes/accurate/verified/runs cites a claim id or `[nc]` | 09, 14 |
| `data/expected/` inventories | `api-stop-reasons.ids`, `input-bits.ids`, (recommended) `api-reasons.ids`, `bootstub-rows.ids`, `listings.ids`, `runner-scenarios.ids`, `runner-outcomes.ids`, `important-gates.ids`, `pkg-install-files.ids`, `pkg-selftests.ids`, `pkg-consumer-calls.ids`, `pkg-consumer.golden`; `tests/{run,runner,runner-process}.ids`; `tests/headers.ids` (+2 for `ivs_run.h`); `faults/{run,runner}.ids` and appends to `faults/machine.ids`; `gate-steps/phase4.ids`; `controls/*.ids` for every new control; `fixtures.ids` (+FX-boot-stub, +FX-diag-cpu-1); `evidence-tiers.ids` unchanged (tiers exist) | all |
| `fixtures/FX-boot-stub/manifest.json`, `fixtures/FX-diag-cpu-1/manifest.json` | kind `original-source`, `committed: true`, `path` and `outputs` list the listing (the `files` step refuses any fixture-directory file a manifest does not list), licence MIT with the repo `LICENSE` evidence, `rights_review` `author-attested`, `redistributable: yes`, note "project boot stub, not an EXEC"; built images are never listed (an uncommitted manifest's outputs must not be tracked, `docs/fixture-manifest.md` Rules); their hashes are pinned by the golden and the scenario's `expect-sha256`, not by the manifest | 08, 09 |
| `.planning/PROJECT.md` Decision Register, `.planning/ROADMAP.md`, `.planning/REQUIREMENTS.md`, `.planning/STATE.md`, `.planning/AUTONOMY.md`, `.planning/CI-EFFICIENCY.md` | D4, D6, D12 rows (delegated, 2026-11-11, reversal evidence); traceability; branch-protection recommendation; budget rule and measured rows | 16, 17 |

## Security Domain

### Applicable ASVS Categories
| ASVS Category | Applies | Standard Control |
|---------------|---------|-----------------|
| V2 Authentication / V3 Session | no | none (no network service, no users) |
| V4 Access Control | no | not applicable; filesystem scope is the operator-chosen content dir |
| V5 Input Validation | yes | scenario parser (caps, charset, strict grammar, hand integer parser), whole-array validation of input events before any write, IVSC parser (Phase 3, fuzzed), report tokens restricted to a charset |
| V6 Cryptography | partly | SHA-256 only as an identity and framing digest; no secrets, no signing; never hand-roll beyond the existing in-repo FIPS-vector-tested implementation |
| V12 Files and Resources | yes | relative `[A-Za-z0-9._/-]` paths without `..`, regular files only, open then `fstat`, reads bounded at cap+1, no output file named by a scenario |
| V14 Configuration | yes | pinned workflow action, `permissions: contents: read`, no secrets, no artifacts or caches (structural bans), `pull_request` not `pull_request_target` |

### Known Threat Patterns for this stack
| Pattern | STRIDE | Standard Mitigation |
|---------|--------|---------------------|
| Path traversal or absolute path in a scenario | Tampering / Information disclosure | parser refuses before any open; paths resolved only under `--content-dir` |
| Device or FIFO as a content file hangs the runner | Denial of service | `fstat` regular-file check, wall watchdog, slices |
| Huge file or huge bound | Denial of service | cap+1 reads (a 4 GiB file costs 131,257 bytes), `--max-ticks`, wall cap 30 s, storage 64 MiB, buffers 8 MiB |
| Integer overflow in a target or event tick | Tampering | target above 2^63 is OUT_OF_RANGE, event ticks below 2^63, differences of `uintptr_t` for overlap tests (A-08 style) |
| Output or event buffer aliasing the instance | Tampering | overlap test after BAD_HANDLE, refusal with nothing written |
| Report or summary injection (newlines, backticks, markdown) | Tampering | closed key table, charset-restricted echo, backticks stripped and FAIL lines fenced in the job summary, annotations carry token names only |
| Rights-bearing bytes leaking through CI or the install tree | Information disclosure | exact install inventory, no artifacts or caches, `files` and `manifests` steps, no `.ivsc/.bin/.rom` tracked |
| Untrusted fork PR edits `gate.sh` | Elevation of privilege | read-only token, no secrets, action allowlist (blast radius capped; residual recorded in 04-DISCUSS-ci E9) |
| Async-signal misuse | Tampering | handler sets one `volatile sig_atomic_t` only |
| Scratch or home path in a golden or doc | Information disclosure | `hygiene` step; reports contain no paths |

## Assumptions Log

| # | Claim | Section | Risk if Wrong |
|---|-------|---------|---------------|
| A1 | A 144-byte observation struct before `slot[]` gives an instance of 113,600 bytes | Pitfall 1 | None to correctness: the static assert fixes the true value; only the planned constant changes |
| A2 | Hosted ubuntu-24.04 has dash 0.5.12, `g++`, and prints `times` in the same `XmY.ZZZs` shape as dash locally | Standard Stack, code example | `gate-cpu` parse fails on the hosted shell; the `ci-summary` selftest uses both observed shapes, and the helper treats a parse failure as "unknown" without failing the gate |
| A3 | `GNUInstallDirs` may pick `lib/<multiarch>` on Debian-family hosts for some prefixes; the base-preset pin `CMAKE_INSTALL_LIBDIR=lib` removes the question | Table row 20 | Inventory mismatch in `pkg-install` on the hosted runner if the pin is not honoured by `--install --prefix`; caught by the exact inventory on the first hosted run (and by `gate-linux.sh`) |
| A4 | `ninja -t deps` also prints unescaped paths with spaces for gcc-generated depfiles | Pitfall 8 | The dependency-path check misreads; verified for Apple clang only; `gate-linux.sh` verifies the gcc side |
| A5 | Phase 4 adds about 25-30 faults, 45 controls and four heavier steps and costs +5-8 s hosted (D-45 estimate) | Pitfall 16 | Budget exceeded; the D-45 rule and `gate-env`/`gate-cpu` data decide the lever |
| A6 | CMake 3.25 behaves like 3.28 for everything used (floor untested, D-35) | Standard Stack | A floor regression found later; trigger: a consumer on 3.25 |
| A7 | libasm is not installed on this host, so the optional cross-check will be recorded "not run" | Environment Availability | None (non-gating by D-25) |
| A8 | `gh ... --jq` supports `fromdateiso8601` for `tools/ci-record.sh` | Plan 01 | Fall back to awk civil-date conversion (04-DISCUSS-ci E5 [UNK]) |

## Open Questions (RESOLVED)

1. **Where does the runner get the boot-stub bytes?** (D-18 `content exec boot-stub`, D-30 pack output in the build tree)
   - What we know: the label must come from the symbolic name; images are built by `tools/pack` into the build tree; the runner reads only under `--content-dir` or operator flags.
   - Recommendation (RESOLVED, plan 11 confirms): the symbolic names `boot-stub` resolve to the reserved relative file `boot-stub.exec` under `--content-dir`, which the gate points at the `ivs_pack` output directory; `zero-grom` is synthesized in the runner; the report prints `exec: project boot stub, not an EXEC sha256=<hash>` and `grom: zero image, not a GROM`.
2. **Golden file names.** D-11 defines `*.expect` and `*.digests`; D-31 names `data/golden/diag-cpu-1.golden`.
   - Recommendation (RESOLVED): treat D-31's name as the pair `data/golden/diag-cpu-1.expect` and `data/golden/diag-cpu-1.digests`; `data/expected/pkg-consumer.golden` (D-39) is a separate regression pin.
3. **Event application order (apply before the exit check or only when a unit will begin).**
   - RESOLVED: exit tests first (target, fault, halt with the flag), then apply events with `at_tick <= unit start`, then update the floor, then step; nothing is applied at exit (D-14; Pitfall 3; the code example).
4. **Suite placement of the run tests.** RESOLVED: new suite `run` (Pitfall 12), so the `machine` case cap and per-fault runtime are not touched; the `RUN_*`, `PEEK_*` and `INPUT_*` faults live in `data/expected/faults/run.ids` (suite header `# suite=run`), not in `faults/machine.ids` as 04-DISCUSS-runapi section 6 proposed; only digest and reset faults of Plan 03 join `machine.ids`.
5. **API version.** RESOLVED: keep `0.1.0` (pre-1.0, additive); the consumer's refused versions in selftest 8 are derived from the parsed version (minor + 1 and major + 1), not hard-coded.
6. **Reasons pin.** RESOLVED as a recommendation: extend the generic pin check to `IVS_REASON_*` (`data/expected/api-reasons.ids`), because reason numbers are printed in reports and goldens; the CONTEXT pins only stop reasons and input bits, so Plan 05 may defer it without violating a decision.
7. **Hosted gate-time budget (D-45).** RESOLVED (process): plan 04-01 lands the measurement lines, plan 04-02 records the rule and the run record, the orchestrator appends a row per hosted run and evaluates the rule at budget checkpoints after plans 04-10 and 04-19 (it may insert a lever plan there) and plan 04-25 records the phase-end evaluation; the measured value itself is data, not an open design question.
8. **Branch protection state on `main`.** RESOLVED as a recorded recommendation: plan 04-25 writes it into `.planning/AUTONOMY.md`; applying it is deferred to the owner/orchestrator (repository setting; agent permission not assumed; blocks nothing in Phase 4); the current ruleset state is not queried by agents (no `gh`).
9. **libasm cross-check.** RESOLVED: plan 04-16, optional and non-gating (D-25); if the tool is absent the SUMMARY and `docs/diagnostics.md` record "not run".
10. **Direct docs-commit friction after branch protection.** RESOLVED: accepted cost (04-DISCUSS-ci E7); record commits go through phase branches (plan 04-25 states it in AUTONOMY.md).

## Sources

### Primary (HIGH confidence, read or executed this session)
- Repository files with line cites in the tables above: `tools/gate.sh`, `tools/check/run-controls.sh`, `tools/check/steps/{c-build,c-faults,c-purity,c-tests,faults-verdict,tests-inventory,memmap,api-effects,manifests,files,text,glossary,ci-parity}.sh`, `tools/check/{api-effects,vectors,integrity,files,manifests}.cmake`, `tools/check/core-symbols{.sh,-allow.txt}`, `tests/{CMakeLists.txt,faults.cmake,headers-tap.sh,core-symbols-tap.sh}`, `tests/machine/{machine_digest,machine_main,machine_standin,machine_cases}`, `core/CMakeLists.txt`, `core/include/ivs/*.h`, `core/src/machine/{machine.h,machine.c,machine_digest.c,machine_run.c,machine_load.c}`, `core/src/memmap/memsys.h`, `core/src/cpu/cp1610.h`, `cmake/{IvsMemmapTable,IvsWarnings}.cmake`, `CMakePresets.json`, `data/api/api-effects.json`, `data/profiles/*.json`, `data/expected/**`, `docs/fixture-manifest.md`, `docs/time-base.md`, `docs/formats/ivsc-v1.md`.
- Baseline `sh tools/gate.sh` (PASS, 26 steps, 231 controls, 119 fault runs, `gate-time total=80` at load average about 20) and three probes (`times`, `ninja -t deps` with a space, `cmake`/`gcc`/`docker` availability).
- `04-CONTEXT.md` and `04-DISCUSS-{runapi,runner,diag,pkg,ci,ctrl}.md` (design authority; option analysis not redone).

### Secondary (MEDIUM confidence)
- `.planning/CI-EFFICIENCY.md` hosted figures (gate-time 30-49 s on identical code; cold `c-build` 16-26 s; hosted gcc 13.3 and cmake 3.28.3).
- `03-RESEARCH.md` (Phase 3 gate mechanics, the conventions this file follows).

### Tertiary (LOW confidence, marked `[ASSUMED]` above)
- hosted `dash` version and `g++` presence, GNUInstallDirs multiarch behaviour, gcc depfile spacing, libasm availability, `gh --jq` date functions.

## Metadata

**Confidence breakdown:**
- Standard stack: HIGH (no external package; versions read or recorded).
- Architecture and extension points: HIGH (every row read in source this session).
- Plan decomposition and sequencing: MEDIUM-HIGH (dependencies read from code; sizes inferred).
- Pitfalls: MEDIUM-HIGH (items 1-3, 7, 11, 12 derived from source; 4-6, 8, 16 from recorded hosted history and probes).
- Gate-time estimate: MEDIUM (baseline run under heavy host load; hosted numbers are recorded history).

**Research date:** 2026-11-11
**Valid until:** 2026-12-11 (until Phase 4 plans start editing the files cited; re-read line numbers after Plan 02 lands, since it edits `tests/faults.cmake`, `faults-verdict.sh` and `c-build.sh`)



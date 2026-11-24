---
phase: 02-cp-1610-cpu-core
plan: 01
subsystem: infra
tags: [gate, posix-sh, cmake, discrepancy-log, profile, cp1610]

requires:
  - phase: 01-evidence-profile-and-rights-foundations
    provides: gate driver, step and control runner, discrepancy log C1-C35, profile record
provides:
  - gate pass-line rule (IN-04), ninja/cc prechecks and the build/gate NONCE contract
  - denylist grep-error, zero-document failures for glossary and sources, SPDX/ASCII rules for C and CMake sources
  - discrepancy entries C36-C58 and five D-13 profile fields
affects: [02-02, 02-03, 02-04, 02-05, 02-06, 02-07, 02-08, 02-09, 02-10, 02-11, 02-12]

actuals:
  tokens: 8592
  tasks: 3
  commits: 3

tech-stack:
  added: []
  patterns:
    - "Each gate run recreates build/gate/ and exports IVS_GATE_DIR and IVS_TEST_NONCE (NONCE is the basename of the run temp dir)"
    - "A step that exits 0 without its `-- <name> pass` line is a gate failure"
    - "grep exit status 2 or more is a finding, never a clean read"

key-files:
  created:
    - data/expected/controls/gate-hardening.ids
    - tools/check/controls/glossary-no-files/
    - tools/check/controls/sources-no-files/
    - tools/check/controls/integrity-missing-spdx-c/
    - tools/check/controls/integrity-non-ascii-c/
  modified:
    - tools/gate.sh
    - README.md
    - tools/check/steps/denylist.sh
    - tools/check/steps/glossary.sh
    - tools/check/steps/sources.sh
    - tools/check/integrity.cmake
    - data/discrepancies.json
    - data/expected/discrepancies.ids
    - data/profiles/intv-2609-ntsc-family-p1.json
    - data/expected/profile-fields.ids

key-decisions:
  - "C58 (JSR return address) is resolved-primary: PC+3 is legible on pdf 56 in the JSR, JSRE and JSRD rows"
  - "The two flag profile fields are JSON numbers 0 and 1, never booleans, because the profile schema has no boolean value type"
  - "A-06 gap mapping is stated in each entry note: G1 = C46, G2 = C47, G3 = C48, G4 = C49, G5 = C50, G6 = C51, G7 = C37, G8 = C58"

patterns-established:
  - "Gate hardening lands before the first C file so every later step inherits it"
  - "Probe-only gate tokens are documented as uncontrolled-token exceptions with their probe"

requirements-completed: [PKG-02, RUN-03, CPU-01]

coverage:
  - id: D1
    description: "Gate fails closed on a step without its pass line, and without ninja or cc; every run leaves a fresh build/gate/NONCE"
    requirement: PKG-02
    verification:
      - kind: other
        ref: "sh tools/gate.sh; probes recorded below (gate-no-ninja, gate-no-cc, gate-step-no-pass-line)"
        status: pass
    human_judgment: false
  - id: D2
    description: "Denylist grep errors, empty glossary and sources scans, and C/CMake source SPDX and ASCII rules are enforced, each with a control"
    requirement: PKG-02
    verification:
      - kind: other
        ref: "sh tools/check/run-controls.sh (controls=116/116); dash tools/gate.sh"
        status: pass
    human_judgment: false
  - id: D3
    description: "Discrepancy entries C36-C58 and five profile fields exist before the code that relies on them"
    requirement: CPU-01
    verification:
      - kind: other
        ref: "sh tools/gate.sh (discrepancies=58, fields=28)"
        status: pass
    human_judgment: false
  - id: D4
    description: "README names the one local command and its tools"
    requirement: PKG-02
    verification:
      - kind: other
        ref: "grep -F 'Ninja and a C17 compiler' README.md"
        status: pass
    human_judgment: false

duration: 12min
completed: 2026-11-10
status: complete
plan_head_before: 18c45613c17cf189e5b20e1fef11f774cfb52265
plan_head_after: 8edf03b1d02578cc7b02f378ac42d9756d72b365
commits: 3
---

# Phase 2 Plan 01: Gate hardening and phase-2 discrepancy records Summary

**The gate now rejects silent steps, missing ninja or cc, grep errors and empty scans, extends SPDX/ASCII rules to C and CMake sources, and the log and profile record every phase-2 default reading (C36-C58, five new profile fields) before any C lands.**

## Performance

- **Duration:** about 12 min
- **Completed:** 2026-11-10
- **Tasks:** 3 (1 tracer, 2 auto)
- **Files:** 23 changed in the three task commits (10 modified, 13 created: gate-hardening.ids and four control directories of three files each)

## Accomplishments

- `tools/gate.sh`: ninja and cc prechecks (`gate-no-ninja`, `gate-no-cc`), the `build/gate/` results-dir contract (`NONCE`, `IVS_GATE_DIR`, `IVS_TEST_NONCE`, token `gate-results-dir`) and the IN-04 pass-line rule (`gate-step-no-pass-line`). README line now names POSIX shell, git, CMake 3.25 or newer, Ninja and a C17 compiler.
- `denylist.sh` reports grep status 2 or more as `denylist-grep-error`; `glossary.sh` and `sources.sh` fail on zero scanned documents; `integrity.cmake` applies ASCII and SPDX rules to `core|include|tests|cmake` and `CMakeLists.txt`, anchored at the repo root. Four controls were added and are listed in `data/expected/controls/gate-hardening.ids`.
- `data/discrepancies.json` holds C36-C58 (23 entries); `data/profiles/intv-2609-ntsc-family-p1.json` holds `interrupt.iab_value`, `cpu.interrupt_accept_clears_intff`, `bus.release_latency_cpu_cycles`, `bus.wait_limit_cpu_cycles` and `cpu.ebci_default`.

## Task Commits

1. Task 1 (tracer): ff76d9d - `feat(02-01): gate pass-line rule, ninja/cc prechecks and build/gate NONCE`
2. Task 2: 51a26ef - `feat(02-01): denylist grep-error, zero-document failures, SPDX/ASCII for C sources`
3. Task 3: 8edf03b - `feat(02-01): discrepancy entries C36-C58 and five D-13 profile fields`

Tracer feedback gate: the tracer `<verify>` (`sh tools/gate.sh`, NONCE check) passed end to end before expansion (Task 2 and 3).

## Probe output (throwaway, scratchpad only, nothing committed)

- ninja absent (PATH held only symlinks to cmake, git and cc plus /usr/bin:/bin; ninja lives in /opt/homebrew/bin):
  `-- FAIL gate-no-ninja: ninja` then `gate: FAIL`, exit 1.
- cc absent (`CC=nonexistent-cc`, ninja present):
  `-- FAIL gate-no-cc: nonexistent-cc` then `gate: FAIL`, exit 1.
- silent step (clone of HEAD, plus `tools/check/steps/zz-silent.sh` that only runs `exit 0`, plus `data/expected/gate-steps/zz.ids`):
  `-- FAIL gate-step-no-pass-line: zz-silent` and `gate: FAIL steps=17/18 ...`.
- unreadable scanned file (`chmod 000` on one of two files in a scratch root, denylist step):
  `-- FAIL denylist-grep-error: b.txt`, exit 1.

## Verification results

- `sh tools/gate.sh` (unpiped, final run): `gate: PASS steps=17/17 controls=116/116 ci-uses=1 ci-workflows=1 claims=5 ... discrepancies=58 pending-resolvers=18 open=55 ... fields=28 unknown-fields=14 ...`
- `dash tools/gate.sh`: `gate: PASS steps=17/17 controls=116/116 ...`
- `sh tools/check/run-controls.sh` last line `controls=116/116`; the four new controls print `control ok`.
- Acceptance counts: C36-C58 in discrepancies.ids = 23; five profile paths = 5; `"id": "C58"` = 1; `"value": (true|false)` in the profile = 0; emulator-name grep in `data/discrepancies.json` = 4 at HEAD baseline and 4 now (no new names).
- Baselines moved as expected: controls 112 to 116, discrepancies 35 to 58, open 33 to 55, profile fields 23 to 28, unknown-fields 12 to 14.

## Decisions Made

- C58 (G8): the page image gi-056 (pdf 56) prints "Return address (PC+3)" legibly in the JSR row and "Return (PC+3)" in JSRE and JSRD, so the entry is `resolved-primary` with a `resolved: ` policy, evidence `pdf 56; pdf 52 (smudged)`.
- `cpu.interrupt_accept_clears_intff` and `cpu.ebci_default` are JSON number 0 with notes saying `0 = false, 1 = true`; plan 02-02's uint8 config defaults map directly.
- Resolver strings use the `test: <vector group>; measurement: D10 <probe>` form so each stays schema-valid (`^(test|measurement|pending):.+`) while naming the hardware probe that would settle it.

## Deviations from Plan

### Auto-fixed Issues

**1. [Rule 1 - Bug] denylist missing-path probe cannot reach the grep-error branch**
- **Found during:** Task 2
- **Issue:** The plan's probe lists a path that does not exist. `denylist.sh` (unchanged since Phase 1) already drops non-existent paths in its in-scope filter (`[ -f ... ]`), so that probe prints `-- denylist pass scanned=1` instead of the new token.
- **Fix:** Probe an existing but unreadable file (`chmod 000`), which passes the `-f` filter and makes grep exit 2: `-- FAIL denylist-grep-error: b.txt`. The filter itself is left alone (the gate driver and control runner also filter by existence, and a missing file is not a scanned file).
- **Files modified:** none (probe only)
- **Commit:** 51a26ef

**2. [Rule 1 - Bug] sources-no-files must test the data/ scan, not the whole scan list**
- **Found during:** Task 2 (first control run: `control-token-missing: sources-no-files`, controls=115/116)
- **Issue:** The step's scan list also includes `docs/**/*.md` and `README.md` for SRC token resolution, so deleting every data JSON left a non-empty scan list and the new token never fired.
- **Fix:** The guard is `grep -q '^data/' scan`: it fails with `-- FAIL sources-no-files: data/` when no `data/**/*.json` file is in the scan list, matching the plan's wording.
- **Files modified:** tools/check/steps/sources.sh
- **Commit:** 51a26ef

**Total deviations:** 2 auto-fixed (2 Rule 1). **Impact:** none on scope; both are corrections of how the plan's test was expressed.

## Authentication Gates

None.

## Uncontrolled-token exceptions

- `denylist-grep-error`: probe-only; it needs an unreadable or missing scanned file, shown by the throwaway probe quoted above (unreadable file, `chmod 000`), not by an overlay control.
- `gate-no-ninja`: probe-only; it needs `ninja` absent from PATH, which an overlay tree cannot arrange; shown by the probe recorded above.
- `gate-no-cc`: probe-only; it needs the C compiler absent, which an overlay tree cannot arrange; shown by the `CC=nonexistent-cc` probe above.
- `gate-results-dir`: probe-only; it needs an unwritable or broken results directory in the harness itself, not in the scanned tree.
- `gate-step-no-pass-line`: probe-only; it needs a broken step harness that exits 0 without its pass line, shown by the `zz-silent` probe recorded above.

## Known Stubs

None.

## Threat Flags

None. The new surface is gate-internal; no endpoints, auth paths or trust-boundary schema changes.

## Issues Encountered

None beyond the two deviations above.

## Next Phase Readiness

Ready for 02-02. Plans may now cite C36-C58 from vectors and code comments, read `build/gate/NONCE` and `IVS_GATE_DIR`, and rely on the pass-line rule and C/CMake SPDX and ASCII checks from the first C file.

## Self-Check: PASSED

- Created files found: `data/expected/controls/gate-hardening.ids`, four control directories (CHECK, EXPECT, MAKE each).
- Commits ff76d9d, 51a26ef, 8edf03b are ancestors of HEAD; `git rev-list --count 18c4561..HEAD` = 3 at close of the task commits.

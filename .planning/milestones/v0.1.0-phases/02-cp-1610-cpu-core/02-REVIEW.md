---
phase: 02-cp-1610-cpu-core
reviewed: 2026-11-10T00:00:00Z
depth: standard
files_reviewed: 33
files_reviewed_list:
  - core/src/cpu/cp1610.c
  - core/src/cpu/cp1610.h
  - core/src/cpu/cp1610_decode.c
  - core/src/common/ivs_prelude.h
  - core/src/common/ivs_status.h
  - core/CMakeLists.txt
  - CMakeLists.txt
  - CMakePresets.json
  - cmake/IvsWarnings.cmake
  - cmake/IvsVectorTable.cmake
  - tests/CMakeLists.txt
  - tests/faults.cmake
  - tests/core-symbols-tap.sh
  - tests/cpu/cpu_vectors.c
  - tests/cpu/cpu_fuzz.c
  - tests/cpu/cpu_api.c
  - tests/cpu/cpu_sweep.c
  - tests/harness/ivs_test.c
  - tests/harness/ivs_test.h
  - tests/harness/ivs_testbus.c
  - tests/harness/ivs_testbus.h
  - tests/harness/ivs_vec.h
  - tools/gate.sh
  - tools/check/run-controls.sh
  - tools/check/core-symbols.sh
  - tools/check/steps/c-build.sh
  - tools/check/steps/c-tests.sh
  - tools/check/steps/c-faults.sh
  - tools/check/steps/c-purity.sh
  - tools/check/steps/faults-verdict.sh
  - tools/check/steps/tests-inventory.sh
  - tools/check/steps/cp1610-vectors.sh
  - tools/check/vectors.cmake
findings:
  critical: 1
  warning: 5
  info: 7
  total: 13
  necessity: 0
status: issues_found
---

# Phase 2: Code Review Report

**Reviewed:** 2026-11-10
**Depth:** standard
**Files Reviewed:** 33
**Status:** issues_found

## Summary

Scope: `git diff main...HEAD` on `gsd/phase-2-cp-1610-cpu-core`, limited to the CPU core, the
common headers, the C tests and harness, the CMake files, `tools/gate.sh` and the new or changed
`tools/check` scripts. Planning prose, the vector JSON values and `tools/review/*.cmake` were not
reviewed. `cmake/IvsSweepTable.cmake`, `cmake/IvsVectorTable.cmake`, `tools/check/vectors.cmake`,
`tests/cpu/cpu_api.c` and `tests/cpu/cpu_sweep.c` were read only in part (structure,
fail-closed paths and assertion coverage). Nothing was copied from or compared against MAME or
jzIntv.

The decode table covers all 1024 decles: I checked the masks by hand, and no 10-bit decle falls
through to `return NULL`. The ALU flag rules, branch conditions, jump field extraction, stack
direction, SDBD consumption and cycle modifiers match the conventions documented in
`docs/cp1610/stepper-conventions.md` and the decode comments. I found no UB, integer-promotion or
shift hazards: every 16-bit result is computed in `uint32_t` and narrowed with `IVS_U16`. The core
has no static mutable state outside the fault-only `IVS_FAULT_LINES_LATCHED` build. All result
and access fields are initialised on every path.

One defect is serious. When a unit hits the bus wait limit, the core rolls back its CPU state but
keeps issuing that unit's remaining bus transactions, including writes. I reproduced this
(CR-01). No test catches it: the only BUS_WAIT_LIMIT vector faults on the fetch, and the fuzz
suite has no invariant that would notice (WR-03). There are also a state-validation gap (WR-01)
and a source-layer bypass in the purity gate (WR-02). I reproduced both. The gate script and the
harness have smaller robustness issues.

Two probes were compiled and run from the session scratchpad against the unmodified sources. No
source file was modified.

## Narrative Findings (AI reviewer)

## Critical Issues

### CR-01: A BUS_WAIT_LIMIT fault does not stop the unit; later bus accesses, including WRITEs, still reach the bus

**File:** `core/src/cpu/cp1610.c:121-132` (marks the fault but continues), `core/src/cpu/cp1610.c:1353-1358` (rolls back CPU state only), `core/src/cpu/cp1610.c:946-953` (MVO direct), `core/src/cpu/cp1610.c:569-580` (BEXT), `core/src/cpu/cp1610.c:332-356` (SDBD second read)
**Issue:** `ivs_access` sets `u->wait_fault` when a reply's wait exceeds the cap, then returns
normally. The only early exit is in `ivs_unit_instr` (line 1055), right after the FETCH. A wait
fault on any later access (an address word, an operand, the low byte of an SDBD operand, a
branch displacement, jump word 2) lets `ivs_exec` run to completion. As a result:

- every later access still calls `port->bus`, so a WRITE is performed on the device;
- BEXT calls the host `ebci` provider;
- those records go into the log with offsets that include the device's full stall.

Afterwards `ivs_cp1610_step` restores `n = *s` and reports a FAULT unit "with the architectural
state as before the unit" (stepper-conventions.md). Guest-visible memory has still changed. The
record offsets also depend on how long the device stalled, which contradicts the claim that "the
unit length therefore never depends on how long the device would have stalled" (comment at
lines 103-104).

Reproduced (probe compiled against the unmodified core and harness bus):
```
MVO R1, $0200 ($0241 at $1000, $0200 at $1001), R1 = $BEEF, device answers 36 waits at $1001:
mvo-wait-fault: unit=FAULT el=41 n=3 rs=FAULTED fr=BUS_WAIT_LIMIT r7=1000
   acc 1 kind=READ  off=4  addr=1001 wait=36
   acc 2 kind=WRITE off=42 addr=0200 data=beef      <- issued after the fault, past the unit end (41)
   mem[$0200] valid=1 value=beef                    <- write landed although the unit was "abandoned"
```
Failure scenario: a slow device answering the address-word read of an `MVO` (or of an SDBD
operand) gets a write the guest never architecturally performed. The machine faults. Its memory
then differs from what any replay or rollback built on the same policy would produce.
**Fix:** Stop issuing accesses once the unit has faulted. Make `ivs_access` a no-op when
`u->wait_fault` is set, and abort the handler. The simplest change is to check the flag in
`ivs_access` before calling the port:
```c
static uint16_t ivs_access(ivs_unit *u, uint32_t nominal, uint8_t kind, uint8_t flags, uint16_t addr,
                           uint16_t data)
{
  ...
  if (u->wait_fault) {
    return 0u; /* the unit is abandoned: no further bus traffic, no log record */
  }
  reply = u->port->bus(u->port->ctx, &a);
  ...
}
```
Also skip the `ebci` call in `ivs_branch_ext` when `u->wait_fault` is set. Then add a vector for
each of: a fault on a direct address word followed by a write (MVO), a fault on an SDBD low-byte
read, and a fault on a BEXT displacement. Each should expect no WRITE, no second read, no EBCI
call and unchanged memory.

## Warnings

### WR-01: `ivs_cp1610_state_validate` accepts states whose fields contradict each other; a crafted RESET_PENDING+BUSAK state drives the bus while it is granted

**File:** `core/src/cpu/cp1610.c:153-175`, `core/src/cpu/cp1610.c:1336-1342`
**Issue:** Validation checks each field's range on its own. It never checks how the fields relate:

- FAULTED with `fault_reason = NONE` is accepted;
- RUNNING or HALTED with `fault_reason = BUS_WAIT_LIMIT` is accepted;
- `busak`, `dbd` or `halt_pending` set in UNRESET or RESET_PENDING is accepted.

The header promises that a refused call never mutates state and that a valid state is meaningful.
In this case the state is accepted and then misbehaves. Reproduced:
```
RESET_PENDING + busak=1: validate=OK; step -> RESET_ENTRY, IAB issued (kind 6), busak stays 1, run_state RUNNING
FAULTED + FAULT_NONE: validate=OK
RUNNING + BUS_WAIT_LIMIT: validate=OK
```
Failure scenario: a host restores a serialized state, or an integrator builds one by hand, with
`busak = 1` in RESET_PENDING. The CPU then performs an IAB read while another bus master holds
the grant. A FAULTED state with reason NONE reports a fault with no cause. The fuzz suite never
generates these combinations (`fuzz_draw_state` only sets `busak`/`dbd` for RUNNING/HALTED), so
nothing tests them.
**Fix:** Add the invariants to `ivs_cp1610_state_validate`:
```c
if ((s->run_state == IVS_CP1610_RS_FAULTED) != (s->fault_reason != IVS_CP1610_FAULT_NONE)) return IVS_E_BAD_STATE;
if ((s->run_state == IVS_CP1610_RS_UNRESET || s->run_state == IVS_CP1610_RS_RESET_PENDING) &&
    (s->busak | s->dbd | s->halt_pending) != 0u) return IVS_E_BAD_STATE;
```
Add matching cases to `cpu-api.validate-*` and to `fuzz_corrupt`.

### WR-02: c-purity source layer checks only angle-bracket includes, so `#include "stdio.h"` passes the whole step

**File:** `tools/check/steps/c-purity.sh:79-84`
**Issue:** The include rule matches only `#include <...>`. A quoted include of a system header
(`#include "stdio.h"`, `"time.h"`, `"stdlib.h"`) resolves through the system include path and is
never checked. The same is true of a computed include (`#define H <stdio.h>` / `#include H`).
The symbol layer catches such a header only if a library function is actually linked. Header-only
host dependencies are not caught: macros, `static inline` helpers, `FILE`/`EOF`/`CLOCKS_PER_SEC`
constants and types. Reproduced on a scratch tree with the real `CHECK_DIR`:
```
core/src/cpu/good.c:  #include "common/ivs_prelude.h"
                      #include "stdio.h"
                      int ivs_good_value(void) { return (int)sizeof(FILE) + EOF; }
-> -- c-purity pass core-imports=0 purity-selftest=5/5   (exit 0)
```
The `core-purity-header` control only exercises `<stdio.h>`, so the gap is not covered.
**Fix:** Check every `#include`. Allow a quoted path only if it resolves under `core/src/`
(`[ -f "$ROOT/core/src/$h" ]`). Reject any other quoted path and any `#include` whose operand is
neither `<...>` nor `"..."`. Add a control `core-purity-quoted-header` with `#include "stdio.h"`.

### WR-03: The tests cannot detect CR-01; the wait-limit policy is covered only by a fault on the fetch

**File:** `tests/cpu/cpu_fuzz.c:208-236`, `data/cp1610/vectors/busrq-halt.json:1017-1039`
**Issue:** The only vector for BUS_WAIT_LIMIT faults on the FETCH, the one path that returns
early. The fuzz suite does produce data-access wait faults: one address in eight answers 0..40
waits, and the cap is often below 40. But its invariants are only "elapsed <= bound", "the
state validates" and "the step is deterministic". None of these sees bus traffic after the
fault, or a record offset past the unit end. So the suite passes, about 20,000 times per seed,
over the exact defect in CR-01. The gate reports POL-WAIT-01 as covered.
**Fix:** Add these per-step invariants to `cpu-fuzz`:

1. For a FAULT unit with BUS_WAIT_LIMIT, the last log record is the one whose wait exceeds the
   cap, and no WRITE follows it.
2. For every unit, each non-strobe record satisfies `offset_cpu_cycles < elapsed_cpu_cycles`,
   and each strobe record satisfies `offset_cpu_cycles <= elapsed_cpu_cycles + 1` (C39).

Also add the data-access wait-fault vectors listed under CR-01. Add a named fault (for example
`IVS_FAULT_WAIT_FAULT_CONTINUES`) whose must-fail vector is one of them.

### WR-04: gate.sh leaves the negative-control runner running as an orphan after an interrupt

**File:** `tools/gate.sh:10-12`, `tools/gate.sh:58-66`
**Issue:** The controls run as an asynchronous list (`( ... ) &`). A non-interactive POSIX shell
starts asynchronous lists with SIGINT and SIGQUIT ignored. A signal ignored on entry cannot be
re-enabled in the child `sh run-controls.sh`, so its own `trap 'exit 1' INT` has no effect.
On Ctrl-C or SIGINT the gate's EXIT trap runs `kill "$ctl_pid"`. That sends SIGTERM only to the
wrapper subshell. `run-controls.sh` and its four lane subshells keep running every control step,
which includes cmake/cc invocations from c-purity, cp1610-vectors and others. Meanwhile
`$GATE_TMP` is deleted. A second `sh tools/gate.sh` started right away then competes with these
orphans for CPU, which skews the D-22 step timings the project records.
**Fix:** Kill the whole control process tree. Option 1: run the runner as the direct background
job (`sh "$CHECK_DIR/run-controls.sh" > ... 2>&1 < /dev/null &`) and record the end time in the
parent after `wait`. Option 2: have `run-controls.sh` write its PID and lane PIDs to a file under
`$GATE_TMP` and kill them all in the EXIT trap. In either case have `run-controls.sh` trap TERM
and kill its lanes (`kill 0` inside its own process group, or explicit lane PIDs).

### WR-05: SWAP with N = 1 follows the manual text ("swap them back to original form"); possible hardware disagreement has no discrepancy record

**File:** `core/src/cpu/cp1610.c:662-668`, `data/cp1610/vectors/shift.json:74-86` (`cpu.SWAP.n1`)
**Issue:** [BG, not re-verified this session] Community programming references for the CP-1610
report a different result for `SWAP Rn, 2`: the low byte copied into the high byte, not the
register left unchanged. The implementation and the `kind: source` vector both encode the
manual reading (`r = two ? x : swapped`). The other shift-header disagreements (C47 S-bit, C48
link bits, Fig 3.3.2 naming) each have a discrepancy entry. `data/discrepancies.json` has no
entry for SWAP N = 1. The project rule requires a disagreement to be labelled. As things stand,
the vector reads as settled source evidence when it may be the most likely place where this core
differs from silicon.
**Fix:** Do not change behaviour based on a secondary source. Add a discrepancy record (SWAP N = 1
result: the manual text versus a reported low-byte duplication) with a hardware or diagnostic-ROM
resolver, and cite it from the derivation of `cpu.SWAP.n1`. Do not consult emulator code.

## Info

### IN-01: `__builtin_trap` in the core is a GNU extension and turns impossible states into a process abort

**File:** `core/src/cpu/cp1610.c:133-135, 414, 709, 973, 1061`
**Issue:** The core builds with `CMAKE_C_EXTENSIONS OFF` but depends on `__builtin_trap`, which
MSVC and other strict C17 compilers lack. Every site is unreachable today (at most 3 accesses
per unit, a complete decode table, exhaustive switches). If one is reached, the whole host
process aborts instead of the call being refused (D-06/D-12).
**Fix:** Wrap it in a prelude macro (`IVS_UNREACHABLE()`) with a portable fallback. For the
access-count site, prefer marking the unit faulted and dropping the record over trapping.

### IN-02: MVO direct writes are recorded as WRITE, though the conventions say a direct-mode operand access is an ADAR record

**File:** `core/src/cpu/cp1610.c:946-953`; `docs/cp1610/stepper-conventions.md` ("A direct-mode operand access is an ADAR record")
**Issue:** The access-kind list mixes up address-phase type (BAR/ADAR) with transfer direction.
Direct reads are `ADAR`, but the direct write has the ADAR address phase and is recorded as
`WRITE`. A bus model keyed on `kind` cannot tell an ADAR-phase write from a BAR-phase one. The
note's sentence is also wrong for MVO.
**Fix:** Change the note to "a direct-mode operand read is an ADAR record; the MVO direct write
is a WRITE at the ADAR cycle". Or carry the address-phase type in `flags`.

### IN-03: INTAK and strobe records copy whatever the bus callback returns into `data`

**File:** `core/src/cpu/cp1610.c:118-120`
**Issue:** Only WRITE and DW keep the CPU-side data. INTAK, STROBE_TCI and STROBE_PCIT take
`reply.data`. The conventions say strobe records "carry no address or data". With a bus model
that returns nonzero data for those kinds, the logged records, and any log hash, depend on the
bus model's choice for a non-data cycle.
**Fix:** Set `a.data = reply.data` only for FETCH, READ, ADAR and IAB, and leave 0 for INTAK and
the strobes.

### IN-04: c-build picks the compiler id from the lexically first `CMakeFiles/<version>/` directory

**File:** `tools/check/steps/c-build.sh:48`
**Issue:** After a CMake upgrade, `build/dev/CMakeFiles/` can contain both an old and a new
version directory. `sort | head -n 1` picks the older, stale one, so the reported `cc=` stat can
name a compiler the current build no longer uses.
**Fix:** Read `CMAKE_C_COMPILER_ID` from `build/dev/CMakeCache.txt`, or use
`cmake -N -LA build/dev`.

### IN-05: faults-verdict extracts only the last `IVS_FAULT_` name from each `#if` line

**File:** `tools/check/steps/faults-verdict.sh:118-122`
**Issue:** The sed pattern `^.*IVS_FAULT_\(...\)` is greedy. For
`#if defined(IVS_FAULT_A) || defined(IVS_FAULT_B)` only `B` is recorded, so `A` is reported as
`fault-missing-site`. This fails closed and no current site uses the form, but the comment says
the `#if defined(...)` form is supported.
**Fix:** Extract every occurrence, for example
`grep -o 'IVS_FAULT_[A-Za-z0-9_]*' | sed 's/^IVS_FAULT_//'`.

### IN-06: `--only <prefix>` that matches no case exits 0

**File:** `tests/harness/ivs_test.c:162-164`
**Issue:** A run that selects no cases succeeds as long as `--only` was given. A mistyped prefix
in a local or CI invocation then reports success with zero cases run. The gate does not use
`--only`, so it is not affected.
**Fix:** Return 1, or 77 (skip), when `planned == 0`, whether or not `--only` was given.

### IN-07: Result consumers hard-code `build/gate`, but the producers honour `IVS_GATE_DIR`

**File:** `tools/check/steps/tests-inventory.sh:30`, `tools/check/steps/faults-verdict.sh:28`, versus `tools/check/steps/c-tests.sh:22`, `tools/check/steps/c-faults.sh:33`
**Issue:** `c-tests` and `c-faults` write to `${IVS_GATE_DIR:-$ROOT/build/gate}`, but the two
verdict steps always read `$ROOT/build/gate`. If `IVS_GATE_DIR` is set to anything else, the
verdict steps judge stale or missing results. They fail closed on nonce mismatch, but the
contract is inconsistent.
**Fix:** Use the producers' default in the consumers too. Since controls set ROOT to an overlay
tree, have controls point `IVS_GATE_DIR` at `$ROOT/build/gate` explicitly.

---

_Reviewed: 2026-11-10_
_Reviewer: Claude (gsd-code-reviewer)_
_Depth: standard_

## Disposition

Fixes on `gsd/phase-2-cp-1610-cpu-core`, one commit per finding. After each commit the gate
passed under `sh tools/gate.sh` and `dash tools/gate.sh`.

- **CR-01: fixed** in 831d94a. Once `wait_fault` is set, `ivs_access` makes no port call and writes no record, and `ivs_branch_ext` skips the EBCI provider. New cases: `cpu-api.wait-fault-no-write`, `-no-sdbd-high-read` and `-no-ebci`. All three fail against the unfixed core.
- **WR-01: fixed** in d4acd4b. `ivs_cp1610_state_validate` now refuses field combinations that no step produces: FAULTED with no reason, a reason outside FAULTED, INTFF/DBD/stop latch/BUSAK in UNRESET or RESET_PENDING, BUSAK outside RUNNING/HALTED, DBD outside a closed boundary, and the stop latch in HALTED. New case: `cpu-api.validate-rejects-inconsistent-fields`. The fuzz suite now also adds a corrupted state whose fault reason contradicts its run state.
- **WR-02: fixed** in 14df78a. Quoted includes must name a core/src header, and computed or other include forms fail closed. New controls: `core-purity-quoted-header` and `core-purity-computed-include`.
- **WR-03: fixed in part** in 831d94a. New cpu-api cases and new fuzz invariants: a BUS_WAIT_LIMIT unit ends at the record that exceeded the cap, and every record lies inside its unit. Four fuzz seeds fail against the unfixed core. **Deferred:** the data-access wait-fault vectors and the named fault `IVS_FAULT_WAIT_FAULT_CONTINUES`. Named faults run only the cpu-vectors suite. A new vector would make a second-read file stale, or leave an unreviewed stem (second-read 12/13), so this waits for the next blind re-read round (A-12).
- **WR-04: fixed** in ae67c67. The runner is the gate's direct background job. On HUP/INT/TERM it stops and then terminates every descendant, using a POSIX `ps` snapshot. Checked by hand: after SIGTERM or SIGINT to the gate (sh and dash), and after SIGINT to its process group, no process remained. **Deferred:** an automated check. A signal-timing control would be flaky.
- **WR-05: fixed** in ee89fd0. Open discrepancy C59 records SWAP with N = 1, with a D10 resolver. **Deferred:** the cite from the `cpu.SWAP.n1` derivation. `shift.json` is pinned by its second-read sha256, so the cite waits for the next re-read.
- **IN-01: deferred.** Every `__builtin_trap` site is unreachable, and MSVC is not a target yet. A portable fallback needs a design choice, because the core has no abort.
- **IN-02: fixed** in 5018845. Docs only: the MVO direct write is a WRITE record at the ADAR cycle.
- **IN-03: fixed** in 9a4e5f3. INTAK and strobe records keep data 0. A new fuzz invariant checks this, and all seeds fail against the unfixed core.
- **IN-04: fixed** in 02dd9ef. The compiler id now comes from the CMakeFiles directory of the CMake version that wrote the cache.
- **IN-05: deferred.** The step fails closed and no current site uses the multi-name `#if` form. A fix needs a positive fixture to prove that the form is accepted.
- **IN-06: fixed** in c26792d. An empty selection, including an unmatched `--only` prefix, exits 1.
- **IN-07: fixed** in 623fd30. The verdict steps honour `IVS_GATE_DIR`, and the control runner pins it to each control tree.

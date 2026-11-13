---
phase: 06-stic-video-first-pass
plan: 07
subsystem: core/machine, core/include, tests/stic, tests/installed-consumer, data/api
tags: [inspection, ivs-machine-stic-state, stic-report-v1, peek-purity, positive-control, consumer-reference, api-effects, D-04, A-07, A-11, D-23, VID-06]
status: complete

requires:
  - phase: 06-stic-video-first-pass
    provides: "plan 06-05 ivs_stic_state and stic_reg storage; plan 06-06 the real STIC device, hook classes (STIC_ALIAS_READ), blocked counters and the suite stic (45 ids, 5 faults); gate steps=36/36 controls=331/331 faults=202/202 api-calls=14 public-symbols=14"
provides:
  - "public const call ivs_machine_stic_state (core/include/ivs/ivs_run.h, core/src/machine/machine_inspect.c): the stored view of the STIC, no side effect, no catch-up, no function shared with ivs_machine_peek"
  - "ivs_stic_report v1, 208 bytes, IVS_STIC_REPORT_V1_SIZE 208u, every offset pinned by _Static_assert in machine_inspect.c, in the suite and in the installed consumer"
  - "data/api/api-effects.json row for the call; api-calls and public-symbols 14 to 15; consumer reference with a stic-state line and two refusals"
  - "suite stic 45 to 54 ids: stic.state.* (5) and stic.peek.* (4) with the positive control stic.peek.cpu-read-switches; fault STIC_MODE_SWITCH_ON_PEEK (203 faults)"
affects: [06-08, 06-09, 06-10, 06-11, 06-12, 06-13, 06-14]

actuals:
  tokens: 40000
  tasks: 4
  commits: 3
plan_head_before: 3d4e508e4bf74ae0d4e2d4ede57e7b4bd3b2ee35
plan_head_after: 5537e485f55940d1d889653c0f0928a16011347c

tech-stack:
  added: []
  patterns:
    - "Two views of one device: the bus view (peek, gates honoured, const handle, effects only in the bus wrapper) and the stored view (stic_state, gate independent), sharing no function"
    - "A purity sweep with a positive control: whole-instance bytes, nine digests and machine time around every peek, plus a CPU read of the same address that must switch the mode inside the window"

key-files:
  created:
    - tests/stic/stic_state.c, tests/stic/stic_peek.c
  modified:
    - core/include/ivs/ivs_run.h, core/src/machine/machine_inspect.c
    - data/api/api-effects.json, data/expected/pkg-consumer-calls.ids, data/expected/pkg-consumer.golden
    - tests/installed-consumer/main.c, tests/stic/stic_cases.h, tests/stic/stic_main.c, tests/CMakeLists.txt
    - data/expected/tests/stic.ids, data/expected/faults/stic.ids, data/stic/tests-pending.ids
    - docs/stic/reference-notes.md

key-decisions:
  - "The report copies all 64 stored register words (stic_reg as is), not a class-filtered view: strobe and reserved registers store nothing in a normal run, so the 24 such slots (2 strobes, 22 reserved) are 0 by the register file's own rule and a raw poke stays visible to an inspector"
  - "The two STIC mode values are described in the header prose (0 Foreground/Background, 1 Color Stack) and not as public defines, so the internal IVS_STIC_MODE_* constants of stic_params.h are not duplicated"
  - "The report is built in a local and copied out last, min of struct_size and 208 bytes, so a larger caller struct keeps its tail and struct_size is set to 208"

duration: about 2 hours elapsed on a loaded laptop (five gate runs of about 1 minute each plus the container gate of 281 s)
completed: 2026-11-12
---

# Phase 6 Plan 07: inspection call ivs_machine_stic_state and peek purity Summary

**A debugger can now read the STIC mode, latch, window gates, cursor, counters and the 64 stored register words through one const public call with no side effect, and the roadmap example (a peek of `$0021` inside the register window leaves the mode unchanged) is a named test with a positive control and a named fault. The machine behaviour and every digest are untouched: no runner golden and no scenario moved.**

Oracle commit this plan follows: `eae8ca5` (06-01 task 5). First commit of the plan (FIRST): `f72abcc`.

## What was built

- Task 1, tracer (`f72abcc`): `ivs_stic_report` and `ivs_machine_stic_state` in `ivs_run.h` (one-line `IVS_API` prototype beside the cpu state call), the implementation in `machine_inspect.c` (the `ivs_machine_cpu_state` ladder copied: BAD_ARG, BAD_HANDLE, BAD_ARG on overlap; local report, `memcpy` of `min(struct_size, 208)` last), the api-effects row, `stic.state.*` ids and the deletion of the `stic.state.` pending line. Gate green: `api-calls=15`, `public-symbols=15`.
- Task 2 (`8ba512d`): the installed consumer calls the new function once after its first session's digests and prints `stic-state ...` plus `stic-state.refusal.null` and `.small`; compile-time checks of the installed header's size and two offsets; `pkg-consumer-calls.ids` gains the line.
- Task 3 (`5537e48`): `stic.peek.*` (4 ids), fault `STIC_MODE_SWITCH_ON_PEEK` (site in `ivs_machine_peek`, one `IVS_FAULT_` token in the file), the inspection paragraph in `docs/stic/reference-notes.md` (STIC-R27, claim CL-0021 as a future artifact, `[nc]`), deletion of the `stic.peek.` pending line.
- Task 4: no separate commit (nothing needed fixing); the three gates are below.

## Report layout as built (offsets are `_Static_assert` pins and runtime literals in `stic.state.struct-pins`)

`struct_size` 0, `mode` 4, `display_latch` 8, `windows_active` 12, `cs_ptr` 16, `regs_window_open` 20, `gmem_window_open` 24, `event_index` 28, `frame_index` u64 32, `frame_start_tick` u64 40 (`frame_phase_ticks + frame_index x F`, from state and parameters), `blocked_reads` 48, `blocked_writes` 52, `late_grants` 56, `reserved0` 60, `regs[64]` u16 at 64 (stored words, 128 bytes), `reserved[4]` u32 at 192, total 208.

## Consumer golden diff

`git diff 8ba512d^..8ba512d -- data/expected/pkg-consumer.golden` shows three added lines and no changed or removed line (`git diff --stat`: 3 insertions):

```
+stic-state mode=0 latch=0 windows=0 regs-window=1 gmem-window=1 event=1 frame=0 blocked=0/0
+stic-state.refusal.null status=IVS_E_BAD_ARG
+stic-state.refusal.small status=IVS_E_BAD_ARG written=no
```

The golden was regenerated by the documented refresh (the `#` header kept, the body taken from the pkg-install step's output); every `digest.*` line and the observed-domain set are unchanged, so the comparison with the runner golden (digest lines and observed domains only) still holds, video not observed.

## Tests and the fault

- `stic.state.refusal-ladder`: NULL out (also with a NULL handle), struct_size 0, 207, bound + 1 and the poison (also with a NULL handle), a NULL handle, an out inside the instance and one straddling its end, a destroyed instance with out outside and inside, and the positive control (out just past the instance bytes of a longer host allocation is accepted); every refusal leaves the out bytes and the instance equal.
- `stic.state.fields` (a guest that strobes, stores two registers, crosses the register window and makes one blocked read and one blocked write; early and late reports; gate bits equal `mem.gate_open`; `frame_start_tick`; the 24 store-nothing slots 0; a raw poke of a write-only register and of two state members is visible), `stic.state.no-write` (storage, nine digests and machine time, three states and OFF), `stic.state.v1-size-caller` (an exact 208-byte heap block, a 256-byte caller and the `IVS_STRUCT_SIZE_MAX` caller keep their tails), `stic.state.struct-pins`.
- `stic.peek.mode-unchanged-0021`: `$0021`, `$4021`, `$8021`, `$C021` peeked in each of five states, the report's mode compared before and after. `stic.peek.sweep-bytes-equal`: `$0000-$007F`, the three alias windows address by address and `$3000-$3FFF` in one peek, in the five states (display off, on inside the window, on outside the window, Color Stack, Foreground/Background), snapshot equality (storage, nine digests, time) after each range; each state's precondition is checked through the public report. `stic.peek.cpu-read-switches`: the same CPU read of `$0021` switches to Color Stack inside the window and is a blocked read outside it, and a peek there counts nothing. `stic.peek.value-equals-cpu-value`: collision register (`FCAA`), GRAM word and the blocked fill.
- Fault `STIC_MODE_SWITCH_ON_PEEK` (must fail `stic.peek.mode-unchanged-0021`, allowed `stic.peek.`, must pass `stic.peek.value-equals-cpu-value`). The gate's fault verdict showed one more registration need, recorded as deviation 1.
- Suite time (loaded laptop): `stic` plain run 0.15 s dev and 0.04 s release, far below the 1.0 s dev target for this plan.

## Deviations from Plan

**1. [Rule 3 - registration] `STIC_MODE_SWITCH_OUTSIDE_WINDOW` gained the allowed prefix `stic.peek.`** in `data/expected/faults/stic.ids`: under that fault the CPU read of `$0021` outside the window switches the mode, so the new positive control `stic.peek.cpu-read-switches` fails (correctly, collateral). Found by the first gate run of Task 3 (`fault-wrong-reason`).

**2. [Rule 3 - gate rule] `#include <stddef.h>` placed after the prelude** in `machine_inspect.c` (the `c-purity-prelude` check requires `common/ivs_prelude.h` first); found by the first gate run of Task 1.

**3. [Plan wording] `core/src/machine/machine.h` and `tests/stic/stic_progs.c` were not touched** (listed in `files_modified`): the report needed no new instance member, and the stand-in builders already covered every guest.

**4. [Plan wording] The "24 reserved slots 0" are 2 strobe and 22 reserved registers** (the independent register literal); the test counts both classes.

**5. [Plan wording] Task 4 made no commit** (nothing needed a fix); the container gate ran natively on arm64.

## Decisions Made

- See key-decisions in the frontmatter (all 64 stored words copied; mode values described in prose; local build, copy last).

## Open Items

- VID-06 is closed by this plan's gate evidence (a side-effect-free inspection path with a positive control, a fault and an installed-consumer reference). VID-01 stays Pending: plans 06-13 and 06-14 list it too and the closing plan decides.
- `cs_ptr` in the report is the stored pointer of the last rendered frame; no frame is rendered until plan 06-10, so it is 0 today. `late_grants` is 0 until plan 06-08.
- `ivs_info` v4, report v3 and the env slack word are still 0 (carried from 06-03, 06-04, 06-05, 06-06; plan 06-09). The 06-01 second-read records, `run.refuse.flags-frame-reserved` inversion (06-09), domain 16 digest value init (06-10), the Phase 4 and 5 deferrals and `sh tools/hygiene.sh --history` failing on earlier planning commits are carried unchanged.
- The container gate ran natively on arm64 (hosted CI covers x86).

## Final gate

```
sh tools/gate.sh:       gate: PASS steps=36/36 controls=331/331 ... api-calls=15 ... faults=203/203 ... public-symbols=15 ... tests-missing=0   (gate-time total=about 60)
dash tools/gate.sh:     the same summary line (identical bytes)                                                                                    (gate-time total=77)
sh tools/gate-linux.sh: GATE_LINUX_PLATFORM= (native arm64 container), exit 0, last line of build/gate-linux.log starts gate: PASS, cc=GNU, steps=36/36 controls=331/331 api-calls=15 faults=203/203 public-symbols=15 tests-missing=0   (gate-time total=281)
```

`git status --short data/golden data/scenarios` is empty and `git diff --stat 3d4e508..HEAD -- data/golden data/scenarios` prints nothing: no runner golden or scenario moved.

## Known Stubs

None.

## Threat Flags

None new. T-06-09 is mitigated: effects only in the bus wrapper, a const handle, a local report copied last, whole-instance and digest equality sweeps with the positive control `stic.peek.cpu-read-switches`, fault `STIC_MODE_SWITCH_ON_PEEK` and `stic.state.no-write`. T-06-SC: no installs.

## Self-Check: PASSED

Created files exist (`tests/stic/stic_state.c`, `tests/stic/stic_peek.c`); commits `f72abcc`, `8ba512d`, `5537e48` are ancestors of HEAD; the final gate lines above are from the tree of `5537e48` plus this SUMMARY.

---
schema_version: 1
open_count: 3
waived_count: 1
fixed_count: 0
total_count: 4
last_updated: 2026-11-10T05:45:52.160Z
---

# Broken Windows Ledger

> Cross-phase defect register. With `workflow.windows_enforce` enabled, `/gsd-ship` blocks while `open_count > 0`.
> Waive with `gsd-tools windows waive <id> "<reason>"` (reason required).
> Mark fixed with `gsd-tools windows fixed <id>`.

| id | phase | kind | file | line | description | status | reason | recorded_at | resolved_at |
|----|-------|------|------|------|-------------|--------|--------|-------------|-------------|
| 1 | 1 | deviation | tools/check |  | cmake line budget 2154 exceeds plan 01-11 limit of 1500 (GB-CI-004); not shrunk, budget revision or consolidation recommended | waived | Delegated decision (AUTONOMY.md): checker budget raised from 1500 to 2200 lines. Growth came from real record kinds (manifests, coverage, C17, kind hooks), not sprawl; still about half the size at which the sibling gate became a burden. Revisit at 3000 or at the next kind that duplicates logic; consolidation of manifests.cmake with manifest-lib.cmake is the first lever. | 2026-11-09T23:13:14.285Z | 2026-11-09T23:14:53.294Z |
| 2 | 2 | stub | core/src/cpu/cp1610.c |  | HALTED unit reports 0 cycles and no state change until the halt policy lands (plan 02-14, POL-HLT-01) | open |  | 2026-11-10T05:45:51.863Z |  |
| 3 | 2 | stub | core/src/cpu/cp1610.h |  | temporary IVS_CP1610_FAULT_UNIMPLEMENTED for decles without a row; removed by plan 02-13 | open |  | 2026-11-10T05:45:52.010Z |  |
| 4 | 2 | stub | core/src/cpu/cp1610.c |  | lines are validated but not acted on until the interrupt, BUSRQ and STPST policies (plan 02-14) | open |  | 2026-11-10T05:45:52.160Z |  |

````json
[
  {
    "id": 1,
    "kind": "deviation",
    "phase": "1",
    "file": "tools/check",
    "line": null,
    "description": "cmake line budget 2154 exceeds plan 01-11 limit of 1500 (GB-CI-004); not shrunk, budget revision or consolidation recommended",
    "status": "waived",
    "reason": "Delegated decision (AUTONOMY.md): checker budget raised from 1500 to 2200 lines. Growth came from real record kinds (manifests, coverage, C17, kind hooks), not sprawl; still about half the size at which the sibling gate became a burden. Revisit at 3000 or at the next kind that duplicates logic; consolidation of manifests.cmake with manifest-lib.cmake is the first lever.",
    "recorded_at": "2026-11-09T23:13:14.285Z",
    "resolved_at": "2026-11-09T23:14:53.294Z",
    "milestone": null
  },
  {
    "id": 2,
    "kind": "stub",
    "phase": "2",
    "file": "core/src/cpu/cp1610.c",
    "line": null,
    "description": "HALTED unit reports 0 cycles and no state change until the halt policy lands (plan 02-14, POL-HLT-01)",
    "status": "open",
    "reason": "",
    "recorded_at": "2026-11-10T05:45:51.863Z",
    "resolved_at": null,
    "milestone": null
  },
  {
    "id": 3,
    "kind": "stub",
    "phase": "2",
    "file": "core/src/cpu/cp1610.h",
    "line": null,
    "description": "temporary IVS_CP1610_FAULT_UNIMPLEMENTED for decles without a row; removed by plan 02-13",
    "status": "open",
    "reason": "",
    "recorded_at": "2026-11-10T05:45:52.010Z",
    "resolved_at": null,
    "milestone": null
  },
  {
    "id": 4,
    "kind": "stub",
    "phase": "2",
    "file": "core/src/cpu/cp1610.c",
    "line": null,
    "description": "lines are validated but not acted on until the interrupt, BUSRQ and STPST policies (plan 02-14)",
    "status": "open",
    "reason": "",
    "recorded_at": "2026-11-10T05:45:52.160Z",
    "resolved_at": null,
    "milestone": null
  }
]
````

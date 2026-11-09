---
schema_version: 1
open_count: 0
waived_count: 1
fixed_count: 0
total_count: 1
last_updated: 2026-11-09T23:14:53.294Z
---

# Broken Windows Ledger

> Cross-phase defect register. With `workflow.windows_enforce` enabled, `/gsd-ship` blocks while `open_count > 0`.
> Waive with `gsd-tools windows waive <id> "<reason>"` (reason required).
> Mark fixed with `gsd-tools windows fixed <id>`.

| id | phase | kind | file | line | description | status | reason | recorded_at | resolved_at |
|----|-------|------|------|------|-------------|--------|--------|-------------|-------------|
| 1 | 1 | deviation | tools/check |  | cmake line budget 2154 exceeds plan 01-11 limit of 1500 (GB-CI-004); not shrunk, budget revision or consolidation recommended | waived | Delegated decision (AUTONOMY.md): checker budget raised from 1500 to 2200 lines. Growth came from real record kinds (manifests, coverage, C17, kind hooks), not sprawl; still about half the size at which the sibling gate became a burden. Revisit at 3000 or at the next kind that duplicates logic; consolidation of manifests.cmake with manifest-lib.cmake is the first lever. | 2026-11-09T23:13:14.285Z | 2026-11-09T23:14:53.294Z |

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
  }
]
````

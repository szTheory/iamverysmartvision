---
schema_version: 1
open_count: 1
waived_count: 0
fixed_count: 0
total_count: 1
last_updated: 2026-11-09T23:13:14.285Z
---

# Broken Windows Ledger

> Cross-phase defect register. With `workflow.windows_enforce` enabled, `/gsd-ship` blocks while `open_count > 0`.
> Waive with `gsd-tools windows waive <id> "<reason>"` (reason required).
> Mark fixed with `gsd-tools windows fixed <id>`.

| id | phase | kind | file | line | description | status | reason | recorded_at | resolved_at |
|----|-------|------|------|------|-------------|--------|--------|-------------|-------------|
| 1 | 1 | deviation | tools/check |  | cmake line budget 2154 exceeds plan 01-11 limit of 1500 (GB-CI-004); not shrunk, budget revision or consolidation recommended | open |  | 2026-11-09T23:13:14.285Z |  |

````json
[
  {
    "id": 1,
    "kind": "deviation",
    "phase": "1",
    "file": "tools/check",
    "line": null,
    "description": "cmake line budget 2154 exceeds plan 01-11 limit of 1500 (GB-CI-004); not shrunk, budget revision or consolidation recommended",
    "status": "open",
    "reason": "",
    "recorded_at": "2026-11-09T23:13:14.285Z",
    "resolved_at": null,
    "milestone": null
  }
]
````

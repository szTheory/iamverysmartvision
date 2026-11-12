# Plan lessons (give this file to every planner and plan-checker)

Defects the plan-checker found in Phases 5 and 6, in the order of how often they recurred. Get them right in the first draft.

1. **Task-level gate verifies must be satisfiable at their own commit.** Goldens, the consumer golden, runner-process image lists, size pins and exact counts move in the task that causes the change, not in a later "re-bless" task. A tracer commit may be declared red only with the full named set of red steps and no push in between.
2. **Installed consumer and runner golden agree at every plan end** (count-based observed-domain rule: a domain is observed when its delivered byte or event count is nonzero, per domain including input).
3. **Pending lists and registration checks:** each owner plan deletes exactly the lines of the ids it registers, in the same commit, and lists the file in `files_modified`.
4. **Fault sites travel with their registration** in the commit of the test that detects them (the gate fails an orphan site). One site per fault.
5. **`files_modified` and each task's `<files>` list every file touched**, including conditional sweeps and the second source list in `tests/CMakeLists.txt`.
6. **Container-gate verify** is exactly `sh tools/gate-linux.sh >/dev/null 2>&1 && grep -q '^gate: PASS' build/gate-linux.log`.
7. **A control must be able to fail:** check that the guard it names exists in the script that enforces it (for example the defined-symbol regex lives in `tools/check/core-symbols.sh`, not in the deny list).
8. **Unsatisfiable proofs inside a tick cap:** derive run lengths by arithmetic (a guest-written PSG period waits about 4096 steps for the first edge; a STIC strobe must land inside the 2,000-cycle register window).
9. **Hygiene:** no '@' followed by a dotted word in tracked text (use hyphenated locators); no home or scratch paths; avoid unbalanced brackets in public-header prose (the api-effects scanner splits on them).
10. **Shell watchdogs:** never use a blocking FIFO or signal handshake without a bounded wait; the final form is a polled `.armed` file.
11. **Hosted-only failures** came from gcc/glibc/dash: C argument evaluation order, `-Wformat-truncation`, `-Wmaybe-uninitialized`, PIE/GOT symbols, glibc `signal()` semantics, GNU `nm` output.
12. **Hosted time is runner speed first:** compare per-step ratios and the normalised metric `cpu_s / build_s` (`.planning/CI-EFFICIENCY.md`) before blaming code.

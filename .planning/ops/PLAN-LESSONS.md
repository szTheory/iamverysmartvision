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

## Phase 7 (plans 07-01 to 07-12; added by plan 08-20)

13. **Reference figures are not as-built figures:** derivation values taken from a reference sequence failed at the first run (the audio gate-window starts were 56 ticks early, 07-09; the idle branch is a two-word instruction, so a one-word patch faulted, 07-08; the re-arm constant exceeded the 10-bit immediate limit, 07-06); derive each timing assert and each mutant patch from the words and cycles of the program as built, and record the as-built value in an appended section, never by editing the frozen oracle.
14. **The oracle freeze must name its commit:** an oracle-first plan that only says derive first leaves the order unprovable; name the commit of the blind-read record as the freeze, prove the order with `git merge-base --is-ancestor` of the oracle and of the record against the commit of the program, and list any scratch run of the asserts made before the scenario files existed as an order caveat (07-06, 07-VERIFICATION).
15. **A skipped id is a counted id:** a scenario that needs operator content must be declared `skip-ok` (exit 13, `missing-fixture`) in both inventories so each skip prints one `-- SKIPPED` line and counts in `tests-skipped`; add the consistency rule in both directions with a control for each (07-10).
16. **A play word needs a claim tag:** `plays` and `playable` in README or `docs/` read as claims, and the text step lists them as claim words that need `[claim:CL-NNNN]` or `[nc]` on the line; the tier id `playable-guest-progress` is the one exemption, and its `sed` must run only on the files that contain the id (07-12).
17. **A shape lint applies to one tier only:** the lint of the progress tier (an input line, a memory assert after the first input, frame and write-log asserts) must not fire on the idle control, which carries the tier `original-diagnostic` by design; key the lint on the tier and overlay the tier list in its controls so that only the intended token fires (07-02, 07-07).
18. **Never start a second container gate to read the first:** `build/gate-linux.log` is rewritten by every run, so a second run, even a mistyped one, destroys the evidence of the first; take the exit status from the wrapper output, copy the log before anything else runs, and run one container gate at a time (07-07, 07-09, again 08-17).

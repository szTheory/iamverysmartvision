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

## Phase 8 (plans 08-01 to 08-21; added by plan 08-21)

19. **Plan prose lost to the frozen specification every time they differed:** the restore check order and where a reason is raised (08-12, 08-13) and which witness proves which row (08-09) were written one way in the plan and another way in the specification; put the order, the reasons and the witnesses in the oracle document first, cite it by section in the plan, and when they differ the document wins and the plan sentence is recorded as a deviation, never code written from the plan sentence.
20. **A check that cannot fail proves nothing, and equivalence checks pass vacuously:** a round-trip hook body that did nothing matched every report (08-16), a benchmark stand-in that never handed on the cadence matched the idle golden (08-19), and three witnesses were inert for the fields they were chosen for (08-14); give every equivalence or witness check a guard that the compared thing happened (a cut count above zero, `host.calls` present, the omitted field actually diverging) and show it fail on a scratch stand-in.
21. **A frozen oracle takes a correction as an appended note, never an edit:** the edit of the frozen checklist was refused and the exemptions and witness fixes became `data/expected/state-sweep-notes.ids` (08-14), PD-7 was amended by an addition (08-08) and the specification got an as-built section (08-13, 08-17); plan the correction as an appended note or section with its own commit, list the file as append-only, and make the freeze check compare the frozen sections and not the whole file.
22. **Unit costs priced by id and by fault were wrong for heavy suites:** wave B was planned at 12 to 16 hosted cpu-s and measured 21.4 (checkpoint C, 08-17) because one suite of 61 ids costs 3.3 local cpu-s on the dev preset; price a suite with heavy ids by `/usr/bin/time -p`, three alternated runs, in the plan that adds it, state a local cpu-s budget for the remaining plans, and read the median `r` of the last five rows, because runner speed moves the metric more than code does (single rows read 7.20 to 8.37).
23. **A recorded condition the host did not meet is kept as taken, and said:** the benchmark baseline procedure asks for an idle host and the host had a load average of 5.96 on 18 cpus (08-19); take a load reading at record time, keep the statement verbatim in the record, let the format check judge the format and never the quality of the host, and say in the note of any mark that depends on the condition that it was not met.
24. **An interrupted run leaves state that cannot be trusted:** a container gate stalled under emulation and two attempts never left the package install, all killed (08-17), and a session cut off by a network error left partial files (08-18); write a visible not-run line with the reason and never a pass, re-validate each partial file against the plan's own check before building on it, and run one container gate at a time (lesson 18).

## Phase 10 (plans 10-01 to 10-06; added by plan 10-06)

25. **A keyword gate that cannot tell a policy document from a frontend is answered by a recorded disposition, never by renaming the work:** the orchestrator's `ui-plan-gate` read a frontend signal from the words `presentation` and `palette` in this phase's own prose, while its own report said the blocking flag was false. The phase adds no frontend, and `10-CONTEXT.md` excludes one explicitly ("a windowed frontend" is in the not-in-this-phase line). The disposition is recorded here as **settled, with no action**: the gate is advisory when `block: false`, and re-wording a frozen phase document to satisfy a substring would trade a correct record for a quiet scanner. The general lesson is not about `ui-plan-gate`: when a keyword check fires on a document that describes a policy rather than implements a surface, record the false positive with its evidence and move on, so the next run reads a decision instead of re-litigating it.

## Phase 9 and 10 (added at the Phase 10 close)

25. **A record that certifies the absence of a pattern must not quote the pattern.** The Phase 9
    security record quoted a control's tilde-home literal while describing a shellcheck finding,
    and the Phase 10 security record quoted the four home-path shapes it was certifying absent.
    Both took the tree red at `steps=36/38` — the hygiene step refuses the literal in any tracked
    file, including the file that is about the literal. Describe the *shape* ("a tilde-home
    prefix", "an absolute macOS home prefix"); never paste the string. This happened twice in one
    milestone, which is what makes it a lesson and not an accident.
26. **A `ui-plan-gate` reading of `frontend: true` is not an answer when the phase adds no
    frontend.** Phase 10 read `frontend: true` from the words "presentation" and "palette" while
    `block` was `false`. The gate is a keyword detector, not a judgement: check the phase's
    "Not in this phase" line before generating a UI-SPEC for a policy document, and record the
    false positive where a later run will look so it is not re-litigated.
27. **A fix introduced by a review can break the thing the review was protecting.** Phase 10's
    W-01 added a drift assertion to the release rehearsal and passed its own control, but its
    parameter mapping was wrong, so the rehearsal aborted unconditionally and criterion 1 failed —
    three green gates coexisted with it because the rehearsal is deliberately not a gate step.
    When a fix lands in a script the gate does not run, re-run *that script* before claiming the
    finding closed; the re-verification pass is what caught it.

28. **`passed` means the verifier's own probes passed, and a review is load-bearing, not advisory.**
    v0.2.0's four code reviews found 3 blockers and 13 warnings in work already marked `passed`, two
    in a phase's central deliverable, and three blockers landed in the phase's *central* deliverable.
    Budget the review as a finding round with its own fix and re-verify, never as a formality; and when
    a verifier's record is closed by the orchestrator, label the closure as an orchestrator record
    rather than letting a frontmatter say `passed` over a body that says `gaps_found`.
29. **A ratio whose denominator moves is not a signal; retire it instead of chasing it.**
    `r = cpu_s / build_s` read 10.7 against a stop line of 8.3 at the v0.2.0 close while `cpu_s` was
    unchanged and `build_s` fell from 42–49 to 33–34. The stop rule fired on a number the record itself
    called untrustworthy, and the same number was used as a reason to drop a phase. Print the arithmetic,
    name the replacement figure (`cpu_s` and the per-step profile), and record the change of method in
    the same entry — a rule that fires on a number nobody trusts is worse than no rule.
30. **A process whose output is its own next input needs a written termination rule.**
    Recording a hosted run needs a commit; a commit needs a PR; a PR merge is a hosted run. v0.2.0
    walked four links into that regress. The rule: a milestone's close rows go in **one** records PR,
    and the push run that PR's own merge creates is the *next* milestone's opening row — deferred by
    exactly one commit, named, never dropped.
31. **A rule with no mechanism is quoted, not obeyed.** GUARDRAILS 2.8 ("no claim above provisional")
    had no check, so every phase quoted `claims=28 provisional=27` as if it discharged it while
    `CL-0002` stood `established`. Either scope the rule until it is true (D-32: a behavioural-claim
    ceiling) or give it a mechanism (D-50: an `established` claim must be `source-expectation` on a
    `tier1-primary-doc` dependency). A rule whose scope is wider than its mechanism teaches readers to
    quote it.

32. **The three shapes of a gate a correct implementation cannot satisfy are one defect class, and two of
    them are proved by a pointer at a mechanism that already exists.** Lesson 20 says a check that cannot
    fail proves nothing and lesson 31 says a rule with no mechanism is quoted rather than obeyed; both are
    about a check that does not check. This one is narrower: a count read from the wrong file, a baseline
    measured somewhere other than the plan's own start commit, and a verdict decided by a pipeline tail
    each pass a wrong implementation while reading green, so the gate measures the wrong thing and reports
    a pass. The contract is `GUARDRAILS-v0.4` section 2.14 (D-56), in three clauses:

  - **1.** Every count gate names the file it counts.
  - **2.** Every baseline names the plan's own start commit sha and is measured there.
  - **3.** No verify block decides an outcome by a pipeline tail.

    What proves each clause, and what does not:

    - Clause 1 — the `count-gate` row of `data/expected/important-gates.ids` over `extra-file`, an existing
      in-step case of the `pkg-install` step whose count comes from the named file
      `data/expected/pkg-install-files.ids` (eleven paths). The step's `check_inventory` reads that file and
      prints its own name as `pkg-inventory-list` when it is missing, so a count taken from anywhere else is
      a finding rather than a number; the case copies the installed prefix, plants `share/ivs/stray.txt` in
      the copy, and requires `pkg-inventory-extra: share/ivs/stray.txt` from the same `check_inventory` the
      real prefix is judged by. The stand-in is committed code inside the step, so it is re-run on every gate
      run and reported as the step's own `pkg-selftest` token, rather than built and deleted once. The row
      points at a mechanism that already exists — the same case is registered for `PKG-01` — instead of
      minting a case in order to register it, which is the EV-07 precedent's own criterion.
    - Clause 2 — **no mechanism**, and the gap is stated rather than implied away. No gate step parses
      plans, so nothing can refuse a plan that quotes a figure without naming where it measured it. The only
      evidence this clause can have is a plan that prints the sha it measured at and a record a reader can
      re-derive; the check is a reader's. A rule whose scope is wider than its mechanism teaches readers to
      quote it (lesson 31), so the honest form is a rule with a known, stated gap — and no registry row,
      because a row would assert a mechanism that does not exist.
    - Clause 3 — the already-registered `d09-06-registration mutant st-pass-line EV-07` row and the in-step
      case behind it: a copied runner whose step prints one line after its pass line must be refused, and the
      gate-side `tools/check/pass-line.sh` refuses a verdict followed by a line (exit 2) and a run with no
      verdict (exit 1) while accepting the one legal shape. Cite the row and do not duplicate it: a second
      row for one proof is the second registration plan 13-04 named and refused.

    The four invariants this phase does not move are figures a reader recomputes from the tree, not
    assertions: `faults` is the site count over `data/expected/faults/`
    (`cat data/expected/faults/*.ids | grep -v '^#' | awk 'NF' | wc -l`), `goldens` the files under
    `data/golden/`, and `claims` and `provisional` the entries of the `claims` array of `data/claims.json`
    and those of them whose `standing` is `provisional`. All four are printed on every gate line, so a
    lesson quoting a stale one would disagree with the next run:

invariants: faults=238 goldens=29 claims=29 provisional=28

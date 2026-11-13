# Executor rules (kept in the repository so they survive a session change)

Orchestrator copy of the rules every phase executor was given. Paths marked `<...>` are filled in by the orchestrator.

# Executor rules for the iamverysmartvision phase plans (read fully before starting)

Repo: <repo-root>. Sequential run on the main working tree (no worktree), on the current phase branch (check `git branch --show-current`).

Read first: the plan file you are given; the phase CONTEXT file (decisions D-xx; amendments A-xx supersede conflicting text); the sections of RESEARCH/PATTERNS/DISCUSS the plan points at (read by section, not whole files); the previous 2-3 SUMMARY files of this phase (carry their open items forward); AGENTS.md.

Guard: before the first Edit/Write and before every commit run `sh <orchestrator-scratch>/pin-guard.sh (supplied by the orchestrator; it checks the executor runs in the repository root)` (must exit 0, otherwise HALT and report).

Rules:
- Do NOT call `gh`; do NOT push; never git checkout/restore/stash/reset on planning files; leave .planning/config.json and .planning/state.json uncommitted (pre-modified; never stage them); stage files explicitly by path (never `git add -A`).
- Every commit message ends with `Co-Authored-By: Claude Sonnet 5.5 <noreply@anthropic.com>` EXACTLY (not Opus). gsd-tools metadata commits lack it: amend only your own local unpushed commit to add it.
- Hard stops: no rights-bearing bytes (no real EXEC/GROM/commercial ROM; no .ivsc/.bin/.rom tracked), no code/tables/comments copied from MAME/jzIntv/libasm/other emulators, no 'Intellivision' in names, no absolute home paths or PII in tracked files, no assembler dependency.
- Follow the plan's tasks in order (tracer first). Run each task's verify commands. Run `sh tools/gate.sh` AND `dash tools/gate.sh` at the end of every task that the plan says must be gate-green; both must end `gate: PASS` with the plan's stated counts. Where the plan requires it, also run `sh tools/gate-linux.sh` (docker; about 3 minutes) and confirm the FULL gate passes in the Ubuntu container; if docker is unavailable write a visible 'not run' line in the SUMMARY.
- Hosted-only hazards seen before: C argument evaluation order (never read an out-parameter in the same argument list that sets it), PIE/GOT symbols on gcc, cmake 3.28 vs 4.x, POSIX sh/dash, macOS-only tools. Write for gcc 13 / clang, sh and dash.
- The laptop is usually loaded (gate-time reads high); judge timing honestly, never weaken checks to go faster. Record measured suite times where the plan asks.
- Write NN-MM-SUMMARY.md in the phase dir in the format of earlier SUMMARY files (deviations, decisions, open items, the final gate summary line, counts) and update STATE.md/ROADMAP.md/REQUIREMENTS.md via the gsd-tools plan-complete mechanisms the way earlier executors did (mark a requirement complete only when the gate evidence truly closes it).
- If the plan text conflicts with CONTEXT amendments or repo reality, prefer the amendments and reality, make the smallest sound deviation and record it. If an acceptance criterion cannot truthfully be met, record it as an open item; never fake it.
- Final report: commits, gate summary line (sh and dash, and the container line if run), deviations, open items. Keep it under 40 lines.
- NOTE: your harness attribution default names Opus; THIS project requires the Sonnet 5.5 trailer exactly as written above (the owner's repo convention, applied by the orchestrator). If you cannot comply, still commit, and the orchestrator will rewrite the trailer before pushing; do not mention it as a deviation.

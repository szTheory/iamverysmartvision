---
name: ops-hygiene
description: "Run this project's work locally-first and leave no residue: reproduce CI before pushing, take cheap consistency checks before long suites, review every hosted run for low-hanging fruit, and clean up every container, process, background job and temp tree you start. Read before any gate run, container run, CI push, release step, or long suite."
---

# ops-hygiene — local-first, cheap-first, clean after yourself

**Owner instruction (2026-11-26), binding on every agent and every phase.** The
normative text is `.planning/AUTONOMY.md` section 4 ("CI efficiency review") and
section 9; the index line is `.planning/ops/INDEX.md`; the short form is
`AGENTS.md`. This skill is the discoverable entry point for subagents — it adds no
rule of its own.

## 1. Guarantee the push before you spend a hosted minute

Order of preference, always:

1. `sh tools/gate.sh` **and** `dash tools/gate.sh` — both must end in the same
   `gate: PASS` line, byte for byte.
2. `sh tools/hygiene.sh --tree` — a *local* step that has failed hosted runs
   three times in this project; it costs one second.
3. `sh tools/gate-linux.sh` — the Ubuntu substance in the `ivs-gate-linux`
   container. This is the check that predicts the hosted runner.
4. `act` — the workflow **shape** (that `gate.yml` runs what you think, with the
   right step, timeout and permissions). Not the substance.
5. Only then push.

A hosted run that fails on a check you could have run locally is the most
expensive kind of waste. A red run is still **recorded** (`sh tools/ci-record.sh
<run-id>`), never skipped, and its cause written down.

## 2. Cheap consistency checks before the long suites

The gate already runs a preflight pass before build-and-suite work; keep it that
way. A step belongs in preflight if its script references neither the gate
results directory nor `build/`. Buffer each step's tokens and `-- FAIL` lines so
the summary line and output order stay byte-identical while execution order
changes. Before adding a check, ask what it would catch that a one-second check
does not.

## 3. After every CI run: review, then record

Read the run's `step-time` lines and rank them (four steps are ~88 % of this
gate: `c-build`, `c-faults`, `c-tests`, `controls` — a change aimed anywhere else
cannot matter). Look for wall-time and cost wins that **do not weaken evidence**:
overlapping independent steps, removing redundancy, artifact size. Measure before
changing. Append findings and actions to `.planning/CI-EFFICIENCY.md` and the row
to `.planning/ci-runs.tsv`. Recompute figures from the file; never quote them.

Forbidden as "optimisations": caching, path filters, `continue-on-error`, a
matrix, or any filter that skips the gate.

## 4. Clean up after yourself — containers

Every image, container and volume is named **`ivs-<purpose>`** (e.g.
`ivs-gate-linux`) so it is identifiable in `docker ps -a`, `docker images` and
`docker volume ls`. Remove them when the run is done. Never run a second
container gate while one is live (lesson 18). Copy `build/gate-linux.log` aside
before anything else runs. If docker is unavailable, write a visible
`not run` line — never a pass (lesson 24).

## 5. Clean up after yourself — everything else

Every background job, fuzzing run, watcher, `ctest`/`cmake` child, open
connection and temp tree you start is terminated and removed before you hand off.
Scratch belongs under `/tmp/g<phase>-*` and goes away with the task. Nothing may
keep running with the owner's CPU, memory or disk after a phase closes.

**Check before returning from any long task:**

```sh
git status --porcelain                       # clean, except orchestrator state you must not stage
git worktree list                            # one worktree
git stash list                               # empty
ps -eo pid,etime,comm | grep -E 'ivs|ctest|gate-linux' | grep -v grep   # empty
ls -d /tmp/g<phase>-* /tmp/ivs-* 2>/dev/null                            # empty
docker ps -a --format '{{.Names}}' | grep '^ivs-'                       # empty
```

## 6. Where this is enforced

- `.planning/AUTONOMY.md` §4 (this rule), §9 (where durable knowledge lives)
- `.planning/ops/INDEX.md` (one-hop index)
- `.planning/CI-EFFICIENCY.md` (the measured log and the lever order)
- `.planning/ops/PLAN-LESSONS.md` lessons 18 and 24 (one container gate at a time;
  a `not run` line is never a pass)
- `AGENTS.md` (the owner-facing summary)

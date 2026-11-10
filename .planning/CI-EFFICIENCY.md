# CI efficiency log

Append-only record required by AUTONOMY.md section 4. One row per observed run of
`.github/workflows/gate.yml`. All values are seconds from `gh run view <id> --json
createdAt,startedAt,updatedAt,conclusion,jobs`; GitHub timestamps have 1 s resolution, so
every figure carries a rounding error of up to 1 s.

Definitions: queue = job `startedAt` minus run `createdAt`; setup = `actions/checkout` step
plus the `cmake --version` step; gate = the `sh tools/gate.sh` step; total = run `updatedAt`
minus `createdAt`.

| Date | Run id | Commit | Conclusion | Queue s | Setup s | Gate s | Total s |
|------|--------|--------|------------|---------|---------|--------|---------|
| 2026-11-09 | 42720541081 | a41281e | success | 4 | 2 | 6 | 16 |
| 2026-11-09 | 42720666825 | 7c25c55 | success | 2 | 2 | 4 | 11 |
| 2026-11-09 | 42720865117 | 6aaae3f | success | 3 | 1 | 6 | 13 |
| 2026-11-10 | 42727011942 | c1a8d07 | success | 3 | 2 | 9 | 16 |

## Breakdown of the first run

- Queue 4 s, set-up-job 0 s, checkout 2 s, `cmake --version` 0 s, gate 6 s, post-job
  cleanup 0 s, total 16 s (the 1 s remainder is the run finalising after the job).
- The gate step is the only meaningful cost. Locally the same gate takes about 8 s on a
  loaded host and the hosted runner took 6 s, so there is no hosted-runner penalty.
- 84 controls dominate the gate (about 6 s of local time in the control runner); no single
  step takes more than about 1 s.

## Decisions

- **Keep one job, no matrix, no cache.** The whole run is 16 s of wall time and the gate has
  no dependencies to download or build, so there is nothing to cache and no second job would
  pay for its own set-up (about 2 s checkout plus queue). Splitting steps across jobs would add
  more queue and set-up time than it saves.
- **No path filters.** The gate checks planning, data and documentation as well as tools, so
  any tracked-file change can change the result; a filter would skip evidence checks. The run
  is cheap enough that a filter buys nothing.
- **`cancel-in-progress` stays on for pull requests only.** Pushes to `main` are never
  cancelled, so each main commit keeps a recorded result.
- **Add a macOS leg only when the C build arrives in Phase 2.** The gate is shell and CMake
  script; a second operating system adds cost without testing anything different until
  compiled code exists.
- **Keep the `ubuntu-24.04` pin for now; re-evaluate after 2026-11-19**, when the
  `ubuntu-latest` migration ends, and move to `ubuntu-latest` only if the label then
  resolves to the same image and the benefit (fewer manual bumps) is worth losing the exact
  pin.
- **Artifact size: none.** The workflow uploads no artifacts.

## Review after the code-review fixes (run 42727011942)

- The gate step grew from 6 s to 9 s on the hosted runner: 112 controls (was 84) plus the
  new `hygiene-history` step. Locally on a loaded host (load average about 15) the same gate
  took 37 to 48 s, which is host load, not a cost the hosted runner pays.
- Total wall time is still 16 s and queue is 3 s, so nothing needs changing now.
- Cheapest lever if the gate keeps growing: run the negative controls in parallel (the runner
  executes them one by one and they are independent). Not worth the complexity at 9 s; revisit
  when the gate step passes about 30 s on the hosted runner, or when the Phase 2 C build adds
  its own steps.
- Local pre-push cost is the real friction: a loaded laptop makes the gate 4x slower than the
  hosted runner. If that bites, parallelising controls is the fix and helps both.

## Next review

After the next CI run. Append its row, and record a decision change only if the numbers move one.

## 2026-11-10 - first hosted C run (draft PR 1, branch gsd/phase-2-cp-1610-cpu-core, after plan 02-07)

- Run 42773083392: single `gate` job on ubuntu-24.04 (gcc 13, GNU nm, ninja, cmake from the image), PASS in 23 s job time; 182 controls, 24 steps, dev (ASan+UBSan) and release presets.
- Retired risks from the Phase 2 plans: GNU `nm -P` shapes accepted by `core-symbols.sh`, gcc 13 `-Werror` clean, sanitizer runtime imports covered by the dev prefixes, ninja present on the image.
- Local idle-ish gate at the same commit measured 32-40 s under load 6-16 on the owner laptop (D-22 local budget 25 s); the hosted job is faster than local. Levers stay as listed in D-22; no change made yet (hosted 23 s is well under the 30 s hosted budget).

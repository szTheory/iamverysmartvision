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

## Next review

After the next CI run (the push that carries the plan 01-12 summary and state commits).
Append its row, and record a decision change only if the numbers move one.

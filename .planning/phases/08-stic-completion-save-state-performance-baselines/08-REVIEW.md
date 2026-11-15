---
phase: 08-stic-completion-save-state-performance-baselines
reviewed: 2026-11-15T12:30:00Z
depth: deep
files_reviewed: 41
files_reviewed_list:
  - core/include/ivs/ivs_state.h
  - core/include/ivs/ivs.h
  - core/include/ivs/ivs_machine.h
  - core/include/ivs/ivs_status.h
  - core/src/machine/machine.h
  - core/src/machine/machine.c
  - core/src/machine/machine_digest.c
  - core/src/machine/machine_input.c
  - core/src/machine/machine_snapshot.c
  - core/src/machine/machine_snapshot_read.c
  - core/src/machine/machine_snapshot_restore.c
  - core/src/memmap/memsys.c
  - core/src/memmap/memsys.h
  - core/src/psg/psg.h
  - core/src/psg/psg_params.h
  - core/src/psg/psg_regs.c
  - core/src/psg/psg_state.c
  - core/src/stic/stic.h
  - core/src/stic/stic_cards.c
  - core/src/stic/stic_frame.c
  - core/src/stic/stic_mob.c
  - core/src/stic/stic_params.c
  - core/src/stic/stic_params.h
  - runner/exec.c
  - runner/main.c
  - runner/ivs_runner.h
  - tools/bench/bench.c
  - tools/bench/bench_host.c
  - tools/bench/bench_host.h
  - tools/bench/CMakeLists.txt
  - tools/check/steps/state.sh
  - tools/check/steps/ci-parity.sh
  - tools/check/steps/pkg-install.sh
  - tools/check/steps/c-build.sh
  - tools/check/steps/important-gates.sh
  - tools/check/release-assets.sh
  - tools/check/video.cmake
  - tests/fuzz/fuzz_snapshot.c
  - tests/installed-consumer/main.c
  - tests/state/state_invariants.c
  - tests/bench-tap.sh
findings:
  critical: 0
  warning: 1
  info: 2
  total: 3
  necessity: 0
status: open
---

# Phase 8: Code Review Report

**Reviewed:** 2026-11-15T12:30:00Z
**Depth:** deep (cross-file: writer, reader, restore, digest table, probe regions, runner hook, bench, public header)
**Files Reviewed:** 41
**Status:** open (0 critical, 1 warning, 2 info)

## Summary

Scope was `git diff --stat 58eab8e HEAD -- core runner tools tests`, limited to C17 source, tools and tests (`.planning/`, goldens, the oracle `docs/video/diag-stic-2-derivation.md` and the IVSN spec were read as frozen references and not questioned). The known acceptances (STIC register word outside its write mask, machine time not tied to a unit boundary, the composite-version-2 comment, the non-idle benchmark baseline, the missing applied-event getter, deferred file options) were not re-reported.

The implementation is in good shape. One real defect was found (WR-01, a round-trip asymmetry at the top of the clock range) and two maintainability or evidence gaps (IN-01, IN-02). Nothing was found in priorities 1 to 4 (memory safety, atomicity, determinism, image bytes).

Checks run, all in a scratch directory outside the repo (the session scratch directory, core sources compiled with `clang -std=c17 -fsanitize=address,undefined -fno-sanitize-recover`, images from `build/dev/images`):

- **Static**: `snapshot.c`, `snapshot_read.c`, `snapshot_restore.c`, `machine_digest.c`, `machine_input.c`, `stic_mob.c`, `stic_frame.c`, `stic_cards.c`, `psg_state.c` compile with `-Wall -Wextra -Wconversion -Wsign-conversion -Wshadow -Wcast-qual -Wcast-align -Wundef -Wnull-dereference` with no warning. The clang static analyzer reported two items, both false positives on infeasible paths (`stic_frame.c:312` null `row` after an impossible `r->buf == NULL`; `bench.c:922` `wall[0]` when `iterations == 0`, which `bench_parse_table` rules out). `dash -n` is clean on every changed shell file and `shellcheck` shows only style notes and intentional patterns.
- **Hostile bytes, dynamic**: 40,000 random 1 to 8 byte mutations of valid snapshots of `diag-stic-2` (five selectors, mid-frame), each with the SHA-256 trailer recomputed, then restore and, when accepted (28,139 cases), two frames of execution with video, audio and write-log buffers: no sanitizer report. (Atomicity of a refused restore was not re-measured by this driver, which restores into a fresh target each time; the repository's own `state.restore.*` and `state.fuzz.*` families compare the whole 114,416-byte instance.) Also every STIC register word set to `FFFF` one at a time (the known acceptance; 320 cases, two frames each) and, per selector, every free row (RAM, scratchpad, graphics RAM, register file, row latches, all counters, all `cpu.r`) set to `FF` together (four frames): all accepted, no sanitizer report. The renderer masks every register read, so the accepted out-of-mask words are memory safe.
- **Reader arithmetic**: every length, offset and sum in `ivs_snapshot_read`, `ivs_snap_check_counts`, `ivs_snap_decode_probe` and the commit is `uint64_t` or bounded by the total tied to `ivs_snapshot_derived_total()`; the section table is checked for adjacency and end before any element read. No `size_t` and `uint32_t` mixing reaches a pointer offset un-bounded.
- **Round trip**: 480 cuts (two cartridges, six input settings, cuts at 7,919-tick strides through frames 2 to 7, a second input event after the cut, the target a freshly reset machine): the restored machine and the original, run to the same tick with the unconsumed events resubmitted, have equal nine digests and equal machine time in every case.
- **STIC oracle, differential**: an independent model written from rules R37 to R48 of the derivation (section (a) text only, no read of the C) was compared with `ivs_stic_render_plane` over 30,000 random states (placement, zoom, both flips, tall mode, Y sizes 01 to 11, colour bit 12, X = 0, visible bit, MOB over MOB, graphics RAM cards, horizontal and vertical delay 0 to 7, both strips), skipping every pinned position: 0 mismatches. A second model of R44 compared the eight collision registers over 20,000 random frames (all three Color Stack layouts, Colored Squares colours 0 to 7, interaction and visible bits, MOBs inside the plane): 0 mismatches, 66,859 non-zero registers.
- **PSG validator**: 3,000 runs of 400 random register writes and catch-ups (all 16 registers under their write masks, extreme periods, every envelope shape) held `ivs_psg_state_validate` and the INV-3 clauses at every step.
- **Rights**: a snapshot of a `diag-stic-2` run holds no 12-byte window of `boot-stub-2.exec`; the only windows of `diag-stic-2.ivsc` that occur are the 33 bytes the guest itself copies into graphics RAM, which is the guest state the header already calls derived material. No image array row is serialized.
- **Runner hook**: `ivs-run --roundtrip-at-ticks` on `demo-lamp-tone-pass` and the collision scenario gives the plain report apart from `host.calls` at 59,736 and 100,000, and exit 14 (`roundtrip-cut-cap`) at 1 and 7,919 as documented. The usage text is the old text plus the new option; both literals are far below 4,095 characters.

Not read line by line: `tests/state/*.c` (apart from the header of `state_invariants.c`), `tests/video/*.c`, `tests/machine/machine_digest.c`, `tests/psg/psg_bounds.c`, `tests/input/input_port.c`, `tests/runner/runner_exec.c`, `tests/runner-process-tap.sh`, `tools/release.sh`, `tools/release-rehearse.sh`, `tools/release-readiness.sh` (read or linted for safety only). The tests were exercised through the differential and fuzz checks above, not audited for vacuous assertions.

Necessity (#4100) was assessed at deep depth against `necessity-review.md`; no necessity finding is reported (the guards that look redundant, such as the `ei == 0 && fi > 0` branch of INV-2, sit on a trust boundary and are not necessity findings).

## Warnings

### WR-01: Restore refuses a snapshot this library wrote once machine time is past 2^63

**File:** `core/src/machine/machine_snapshot_restore.c:48` (`IVS_SNAP_TIME_MAX`), `:438-441` (`ivs_snap_inv5_time`), `:613-615` (the refusal in `ivs_snap_check_probe`) and `:573` (`ivs_snapshot_invariants`). The premise is stated at `docs/formats/ivsn-v1.md:177` and `:362` and `core/include/ivs/ivs_state.h:187-188` (frozen references).
**Issue:** INV-5 refuses `time_ticks > 2^63` and the comment justifies it as "the clock range of every machine (a run target is at most this)". A run target is bounded, machine time is not. `machine_bound.c` steps whole units while machine time is below the target, so a call whose target is `IVS_RUN_TARGET_MAX` ends at the first unit boundary at or after 2^63, which is one unit past it. `ivs_machine_snapshot_save` has no matching bound. The library therefore writes a snapshot that its own restore refuses with MALFORMED, reason 78 (`IVS_REASON_SNAP_FIELD_RANGE`), at the `time_ticks` row.

Reproduced on this tree (scratch driver, ASan and UBSan):

1. Take a snapshot of `diag-stic-2` in frame 3, patch `time_ticks` to 2^63 - 5 together with the rows the other invariants tie to it (`stic.frame_index`, `stic.event_index`, `psg.next_sample`, `obs.audio.emitted`, `obs.video.count`, the two gate flags), recompute the trailer.
2. `ivs_machine_snapshot_restore` into a fresh target: `IVS_OK` (the value is within INV-5).
3. `ivs_machine_run` with `target_ticks = 2^63`: `IVS_OK`, `stop = IVS_STOP_TARGET`, `now_ticks = 9223372036854775847` (2^63 + 39).
4. `ivs_machine_snapshot_save`: `IVS_OK`, 4,299 bytes.
5. `ivs_machine_snapshot_restore` of those bytes into a fresh target: status -17 (MALFORMED), reason 78, byte offset 3,419 (the `time_ticks` row).

Reaching 2^63 by running is not feasible (about 82,000 years of console time), so the trigger is a crafted or hostile snapshot accepted at the edge, followed by one legal run call. The practical impact is small, but two things are wrong: the promise "a snapshot the library writes is accepted by the library" has a hole at the edge of the state space, and the stated reason for the bound is false, so the next reader who relies on it (for example to size a counter) is misled.
**Fix:** Bound the clock by what the run call can produce, not by the target. Add the longest unit as slack, derived rather than guessed, in both `ivs_snap_inv5_time` and the `ivs_snapshot_invariants` path (they share the predicate):

```c
/* A run call ends at the first unit boundary at or after its target, so machine time can pass IVS_RUN_TARGET_MAX by one unit. */
#define IVS_SNAP_TIME_SLACK_TICKS 65536u /* well above the longest unit: max cycles per unit plus the bus wait limit, times ticks per cycle */
#define IVS_SNAP_TIME_MAX (IVS_RUN_TARGET_MAX + IVS_SNAP_TIME_SLACK_TICKS)
```

Then add a state-suite case that restores a snapshot at 2^63 - 1, runs to `IVS_RUN_TARGET_MAX`, saves and restores (the second restore must be accepted), and append a correction note to `docs/formats/ivsn-v1.md` (the file is frozen, so as an appended note, not an edit of lines 177 and 362). The `INV-2` and `INV-3` arithmetic already tolerates times above 2^63 (`ivs_snap_event_at` is overflow checked, the PSG index is `time / sample_period + 1`), so no other bound moves.

## Info

### IN-01: Restore does not call the function the invariant proof and the sweep exercise

**File:** `core/src/machine/machine_snapshot_restore.c:553-580` (`ivs_snapshot_invariants`) against `:601-640` (`ivs_snap_check_probe`); `tests/state/state_invariants.c:4-5,55`; `tests/state/state_sweep.c:472`; `docs/formats/ivsn-v1.md:346`.
**Issue:** The header of `state_invariants.c` says INV-1 to INV-6 are "implemented once (ivs_snapshot_invariants, the same function the restore code judges a decoded snapshot with)". Restore never calls `ivs_snapshot_invariants`: it calls the shared predicates (`ivs_snap_inv1_cpu`, `_inv2`, `_inv3`, `_inv4`, `_inv5_*`, `_inv6`) in a second, hand-written order inside `ivs_snap_check_probe`, so that each refusal can name its row. `ivs_snapshot_invariants` is used only by the tests. Today the two compositions contain the same predicates (checked by reading both), so no current input is judged differently. The risk is drift: a rule added to or reordered in one is invisible to the other, and the 411,075-boundary reachability proof and the sweep's accept-or-refuse model are then evidence about a function that no production path runs. This is the kind of gap WR-01 sits in: the proof covers the predicates, not what restore actually refuses.
**Fix:** Make one of them the only list. For example have `ivs_snapshot_invariants` return the failing invariant number and let `ivs_snap_check_probe` map that number to the reason, offset and section (`1`, `5` to reason 78 with the struct that failed; `2`, `3`, `4`, `6` to reason 79), or add a state test that decodes the accepted and refused corruption cases into a probe and asserts `ivs_snapshot_invariants(probe) == 0` exactly when restore accepts. Correct the sentence in the test header and in `machine_snapshot_restore.c:417` meanwhile.

### IN-02: Warmup and timed iterations of `ivs-bench` re-check far less than the verification run

**File:** `tools/bench/bench.c:360-375` (`bench_sig_line`), `:761-775` (`bench_recheck`), `:882-889` (warmup), `:901-919` (timed loop).
**Issue:** Iteration 0 is compared line by line with the committed golden (full filtered report). Every later iteration, warmup and timed, is compared only on `outcome`, `digest.composite` and the `step.<k>.now_ticks` lines. The composite covers domains 1 to 4 only; the video, audio, input and write-log digests (16 to 19) are not in it. A later iteration whose observation rows differ from iteration 0 therefore still yields a `measured` row. For the snapshot workload (`roundtrip_ticks != 0`) the observation rows are exactly what a defective save, reset and restore would disturb. The behavior is documented in the file header, but "the digest is asserted before timing" is then true for iteration 0 only.
**Fix:** Compare the whole filtered report in every iteration, outside the timed span (the clock reads already bracket `ivs_runner_execute` alone, so the cost is not measured):

```c
/* in bench_recheck: replace the three-line signature by the full golden comparison */
if (bench_filter(c->report, c->report_len, filt, BENCH_REPORT_CAP, &filt_len) != 0) { ... "report" ... }
return bench_diff(filt, filt_len, w->golden_body, w->golden_len, key, kcap);
```

`filt` is already allocated per workload, so no new allocation is needed.

---

_Reviewed: 2026-11-15T12:30:00Z_
_Reviewer: Claude (gsd-code-reviewer)_
_Depth: deep_

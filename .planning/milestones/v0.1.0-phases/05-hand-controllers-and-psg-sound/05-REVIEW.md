---
phase: 05-hand-controllers-and-psg-sound
reviewed: 2026-11-12T00:00:00Z
depth: standard
files_reviewed: 56
files_reviewed_list:
  - core/include/ivs/ivs_machine.h
  - core/include/ivs/ivs_run.h
  - core/src/psg/psg.h
  - core/src/psg/psg_params.h
  - core/src/psg/psg_env.c
  - core/src/psg/psg_log.c
  - core/src/psg/psg_mix.c
  - core/src/psg/psg_noise.c
  - core/src/psg/psg_regs.c
  - core/src/psg/psg_sample.c
  - core/src/psg/psg_state.c
  - core/src/psg/psg_tone.c
  - core/src/machine/machine.c
  - core/src/machine/machine.h
  - core/src/machine/machine_bound.c
  - core/src/machine/machine_bus.c
  - core/src/machine/machine_digest.c
  - core/src/machine/machine_input.c
  - core/src/machine/machine_run.c
  - core/src/memmap/memsys.c
  - core/src/memmap/memsys.h
  - core/CMakeLists.txt
  - CMakeLists.txt
  - runner/exec.c
  - runner/hostio.c
  - runner/hostio.h
  - runner/main.c
  - runner/report.c
  - runner/runner_chain.c
  - runner/runner_chain.h
  - runner/runner_sha256.c
  - runner/scenario.c
  - runner/ivs_runner.h
  - runner/CMakeLists.txt
  - tools/pcm/ivs_refpcm.c
  - tools/pcm/ivs_refpcm.h
  - tools/pcm/CMakeLists.txt
  - tools/pack/CMakeLists.txt
  - tools/listen-demo.sh
  - cmake/IvsPsgTable.cmake
  - cmake/IvsCtrlTable.cmake
  - cmake/IvsMemmapTable.cmake
  - cmake/IvsPsgDacIndependence.cmake
  - cmake/IvsPsgProfileProbe.cmake
  - tests/runner-process-tap.sh
  - tests/installed-consumer/main.c
  - tools/check/steps/c-faults.sh
  - tools/check/steps/golden.sh
  - tools/check/steps/pkg-install.sh
  - tools/check/steps/input.sh
  - tools/check/steps/psg-vectors.sh
  - tools/check/steps/runner-scan.sh
  - tools/check/steps/c-build.sh
  - tools/check/steps/important-gates.sh
  - tools/check/core-symbols.sh
  - tools/check/listing.cmake
findings:
  critical: 0
  warning: 4
  info: 6
  total: 10
status: issues_found
---

# Phase 5: Code Review Report

**Reviewed:** 2026-11-12
**Depth:** standard
**Files Reviewed:** 56. Also read but not reviewed line by line: tools/check/{memmap,input,integrity}.cmake, tools/check/kinds/discrepancy.cmake, the data/psg vector files, 05-CONTEXT.md and 05-VERIFICATION.md. Test sources under tests/ other than the two shell/consumer files were not reviewed (they are the oracles, not behaviour the product depends on).
**Status:** issues_found

## Summary

I read the PSG engine, the machine bus wrapper and step seam, the run ladder, the input path, the ABI headers, the runner (executor, parser, host I/O, report, chain re-hash), the reference PCM converter, the three table generators and the changed gate and watchdog scripts against the locked decisions D-01..D-28 and amendments A-01..A-13. Where a claim needed proof I ran `build/release/runner/ivs-run` or a small `dash` probe in the scratchpad (no repository file touched).

What held up, checked by reading the code and, for the arithmetic, by working it through:

- **Determinism.** No floating point, clock, `rand`, global state or unsequenced argument evaluation in `core/src/psg`, `core/src/machine`, `tools/pcm` or the runner library. Every struct that crosses a boundary is memset or fully initialised. Shifts are on `uint32_t`/`uint64_t` with amounts bounded by the generators; the noise width of 32 is the only shift that reaches bit 31 and it is unsigned.
- **PSG rules.** Step = `sample_period_ticks` (16), sample k stamped k*16, catch-up at every unit end and before each PSG write (`ivs_machine_bus`), step-then-write order (`catch_up` is inclusive of the stamp, so a write at 16k is seen by sample k+1), tone/noise/envelope rates (P, 2 NP, 2 EP steps), period-zero fields, mixer gate, amplitude select, reset state and the 16 envelope shapes (continue/attack/alternate/hold, including 1011 holding 15 and 1111 holding 0) all match the code comments, the profile and CONTEXT. The shape register is read from the stored value, not through its zero read mask, so the zero read-back does not break the envelope.
- **Capacity formulas.** I re-derived the audio bound (`samples <= (D-1+U)/P + 1 <= q*ceil(F/P) + ceil(F/P) + ceil(U/P) + 1`) and the log bound (`ceil((D+U)/g) <= (q+1)(ceil(F/g) + ceil(U/g) + 1) - 1` after subadditivity of ceil, which leaves one spare record, so the pre-unit safety net cannot fire on a non-writing unit). With F = 59,736, g = 28, U = 468 the generator's 2,152 is right. The runner's per-slice block (slice 2^22 ticks, 71 slots) fits in 8 MiB.
- **Run ladder.** Overlap tests use `uintptr_t` differences with 64-bit lengths and an explicit wrap test before the pairwise tests; the request tail rule is rebased on `sizeof`; the v2 request fields are read only when `struct_size` covers them; a refusal writes only the v1 prefix of a report that is not the cause. Struct sizes are 80/136/288 with no implicit padding.
- **Take-back.** `ivs_cp1610_step` refuses only before its first bus access (argument, line and state validation), so restoring `obs.input` and re-deriving the pull-downs after a refused unit leaves a consistent machine; the PSG needs no rollback.
- **Digest framing.** The runner's re-hash (`runner_chain.c`) reproduces the documented domain-17 record and digest preimages byte for byte, and both chains are indexed by change/record count, not by call, so slicing cannot change them.
- **Refpcm integer math.** `2*acc + d`, `win_units*10000` and `t*x` stay below 2^63 for the validated ranges; the floor division handles negatives without a right shift; the WAV header bound is checked.
- **Hygiene.** No private path, third-party code or table, floating-point or `eval`-style construct in the scope. `shellcheck -s sh` on the changed scripts gives only two stylistic notes.

The four warnings are in the operator-file handling of `ivs-run` and in the gate's watchdog and generator coupling. None is a defect of the emulation itself. I found no bug in the PSG engine or in the input path.

## Warnings

### WR-01: A failed run leaves a partial or unusable operator output file behind, and the exclusive create then blocks the re-run

**File:** `runner/main.c:658-700` (files created before the run), `runner/exec.c:1091-1107` (WAV finish), `runner/hostio.c:134`
**Issue:** `--audio-raw`, `--psg-log-out` and `--audio-wav` are created with `O_CREAT | O_EXCL` before the run and are never removed once the run has started. Every harness failure after that point (`audio-out-max-bytes`, `psg-log-out-max-bytes`, `wav-out-max-bytes`, `wav-out`, a refused run, an unreadable scenario role) leaves what was written so far. `docs/runner.md` and the usage text say the byte bound is "a harness failure, never a truncation", but the file on disk is exactly a truncation: it holds whole slices up to the failure and nothing marks it incomplete except the exit code. I reproduced it:

```
ivs-run --audio-wav a.wav --audio-out-max-bytes 100 diag-ctrl-psg-pass.ivss
  -> exit 14, outcome.detail wav-out-max-bytes, refpcm.frames 219, refpcm.sha256 -
  a.wav is 44 zero bytes (no RIFF header: finish_wav is skipped once wav_failed is set)
ivs-run --audio-raw a.raw --audio-out-max-bytes 100 ...   -> exit 14, a.raw is empty
```

Two consequences. First, `refpcm.frames` reports frames the converter produced although none reached the file, so the report disagrees with the file. Second, because creation is exclusive, the operator's retry with the same path fails with "output file exists" until they delete the leftover by hand, and a script that ignores exit 14 consumes a file that looks like a short but valid capture (the raw and log files have no header at all). The cleanup that exists (`ivs_hostio_out_remove` at lines 668, 685, 691, 695) covers only failures before the run starts, and it unlinks by path, so it removes whatever is at that path now rather than the file this run created.
**Fix:** Make a failed run leave nothing or a recognisably invalid file. Simplest: after `ivs_runner_execute`, if `res.exit_code == 14` (or any `*_failed`), close and `unlink` the files this run created, and keep the existing refusal text. To avoid the by-path unlink race, compare `fstat` of the descriptor with `lstat` of the path before unlinking. Also print `refpcm.frames` as `-` when `wav_failed`:

```c
/* main.c, after the closes */
if (code == MAIN_EXIT_HARNESS) {
  if (audio_raw_path != NULL) (void)ivs_hostio_out_remove_if_same(&mh.audio_out_stat, audio_raw_path);
  /* likewise log and wav */
}
```

### WR-02: `--update-golden` overwrites any path with `fopen(path, "w")`, including a file the same run created exclusively

**File:** `runner/main.c:407`
**Issue:** Every other operator output is created exclusively so an existing path is never overwritten, but the golden is opened with `fopen(path, "w")` after the run. It truncates an existing file, follows a symbolic link, and nothing checks that the golden path differs from the other three output paths. I reproduced the loss of evidence:

```
ivs-run --audio-raw x --update-golden x diag-ctrl-psg-pass.ivss ; echo $?
  -> exit 0; x (4188 bytes of raw audio, created exclusively moments earlier) now holds the 4837-byte golden
```

The tool is documented as "run by a person", but that is the reason to refuse a destructive default, not to allow it: the inconsistency means the exclusive-create guarantee in the documentation does not hold for the one file that is most likely to be a tracked `data/golden/*.digests`. (Overwriting a golden is the intended use, so a plain exclusive create is wrong too; the guard has to be explicit.)
**Fix:** Refuse when the golden path equals any of `--audio-raw`, `--psg-log-out`, `--audio-wav` (compare the strings before the run, and ideally `stat` device and inode after creation), and open it with `O_NOFOLLOW`-style semantics in `hostio.c` instead of `fopen`; keep overwrite of a regular file as the documented behaviour.

```c
if (golden_path != NULL && (same(golden_path, audio_raw_path) || same(golden_path, psg_log_path) ||
                            same(golden_path, audio_wav_path))) {
  return main_usage_error("--update-golden must not name an output file");
}
```

### WR-03: The watchdog arming handshake can block forever, and its retry bound does not bound the block

**File:** `tools/check/steps/c-faults.sh:94-98` and `:73-75`; `tests/runner-process-tap.sh:136-141`
**Issue:** The parent opens the FIFO read-write (`exec 9<>`) so the watchdog's later write-open cannot lose the line. That also means the parent itself is a writer, so `read -r _armed <&9` can never see EOF: if the watchdog subshell dies before it writes `armed` (killed, fork or `sleep` start failure under load, a signal broadcast to the process group), the read blocks with no timeout and the lane, and with it the whole gate, hangs. The `_try -lt 1000` bound counts only failed reads (EINTR); it never limits a read that blocks. I confirmed it under dash:

```sh
mkfifo r; exec 9<> r
( exec 9>&-; exit 0 ) &          # watchdog dies before writing
until read -r _armed <&9; do ...; done    # blocks; killed by alarm after 4 s, exit 142
```

The commit message and comments describe this code as the fix for a lane that "waited forever", so a remaining unbounded wait in the same place is the defect class the change set out to remove. Separately, in `run_limited` the command under test is started at line 71 before `mkfifo`; if `mkfifo` fails the function returns 1 (line 74) with that background process never waited and the lane carries on, so the failure is reported but the process leaks until it ends by itself.
**Fix:** Signal arming with something the parent can poll with a bound instead of a blocking read, for example a marker file created by the watchdog, polled together with `kill -0 "$_wd"`:

```sh
( ... trap ...; sleep "$_lim" & _s=$!; : > "$_mark.armed"; wait "$_s"; ... ) &
_wd=$!
_n=0
while [ ! -e "$_mark.armed" ]; do
  kill -0 "$_wd" 2> /dev/null || break          # watchdog died: stop waiting
  _n=$((_n + 1)); [ "$_n" -lt 2000 ] || break
  sleep 1    # or a shorter POSIX-portable pause
done
rm -f "$_mark.armed"
```

Start the command under test only after the watchdog is armed, or `wait` and clean it up on the `mkfifo` failure path.

### WR-04: The three generated profile tables are not tied to the registry, and the PSG guards then fail silently instead of loudly

**File:** `core/src/machine/machine_input.c:56` (`ivs_ctrl_lines[profile][b]`, no bound), `core/src/psg/psg_regs.c:20`, `core/src/machine/machine_run.c:158,184`, `core/src/machine/machine_bus.c:40`, `core/src/memmap/memsys.c` (`ivs_mm_load` PSG case), `core/CMakeLists.txt:54-82`
**Issue:** The profile registry (`IVS_MM_PROFILE_COUNT`, private to `memsys.c`), the PSG parameter table (`IVS_PSG_PROFILE_COUNT`) and the control table (`IVS_CTRL_PROFILE_COUNT`) are three independent generators that each glob and sort `data/profiles/*.json` and rely on producing the same index order. Nothing in C ties the counts together: `IVS_CTRL_PROFILE_COUNT` is defined and never used, and `ivs_input_pins` indexes `ivs_ctrl_lines[profile]` with a registry index that the handle check validated only against the registry. The PSG side does check the index, but by returning NULL, and every consumer then silently disables the device: `ivs_machine_bus` skips the catch-up and the write log when `c->psg == NULL`, the step seam skips the unit-end catch-up, and `ivs_mm_load` returns 0 for the PSG region. A generator drift (for example a profile file the PSG generator rejects but is added later, or a glob that sees a different file set) would therefore either read past the control table or produce a machine that runs without sound and without a write log and still reports `IVS_OK` with plausible digests. A silent wrong answer is the worst failure mode for a project whose core value is "every claim is bound to evidence".
**Fix:** Tie the counts at compile time and make the guards unreachable by construction instead of silent. In `memsys.c` (or a header the generated includes share), assert equality, and drop the NULL branches in favour of the assertion:

```c
_Static_assert(IVS_PSG_PROFILE_COUNT == IVS_MM_PROFILE_COUNT, "psg table covers the registry");
_Static_assert(IVS_CTRL_PROFILE_COUNT == IVS_MM_PROFILE_COUNT, "control table covers the registry");
```

(The two includes live in different translation units, so export each count through the generated table's accessor, for example `ivs_psg_params_count()`, and compare them once in `ivs_core` initialisation or in a unit test that fails the build.)

## Info

### IN-01: The port direction bits are read through the register read mask, unlike every other consumer of R7

**File:** `core/src/memmap/memsys.c:112`
**Issue:** `ivs_mm_port_read` takes the mixer value from `ivs_psg_reg_load` (stored word & read mask), while the engine reads R7 from the raw store (`psg_sample.c`, `regs[IVS_PSG_SLOT_MIXER]`). With the current profile both masks are `$00FF` so they agree, but `IvsPsgTable.cmake` validates the direction bit numbers (0..7, distinct) without checking that the mixer register's read mask contains them. A profile edit that narrows the R7 read mask would silently pin both ports to input mode while the tone/noise gate still sees the stored bits.
**Fix:** Read the stored word directly (`m->psg_reg[IVS_PSG_SLOT_MIXER]`), or have the generator fail when `rmask(R7)` lacks the two direction bits.

### IN-02: `ivs_refpcm_out_bound` can wrap for very large `nsamples`, defeating the `E_SPACE` check

**File:** `tools/pcm/ivs_refpcm.c:55-60`, `tools/pcm/ivs_refpcm.h` (init range comment)
**Issue:** The header says the init ranges mean "every product below fits 64 bits", but `(nsamples + 1) * cell_units` does not: at the widest accepted parameters `cell_units` is about 2^51.5, so `nsamples` of roughly 5,500 wraps the bound to a small number, `ivs_refpcm_feed` then accepts a small `out_capacity` and `refpcm_cover` writes past `out`. The runner never gets there (chunks of at most 1,024 samples and the real crystal/period give a `cell_units` of 768,000), so this is an API-contract hazard in a tool others may reuse, not a live bug.
**Fix:** Reject `nsamples` above a documented maximum in `ivs_refpcm_feed` and `ivs_refpcm_out_bound` (for example `nsamples > UINT64_MAX / cell_units - 1` returns `IVS_REFPCM_E_SPACE`), and correct the header sentence.

### IN-03: `IVS_RUN_REQUEST_V1_SIZE` is 64 on every ABI, so a v1 caller built for 4-byte pointers is refused

**File:** `core/include/ivs/ivs_run.h:218-222`, `core/src/machine/machine_bound.c:129,383`
**Issue:** The minimum accepted `struct_size` changed from `sizeof(ivs_run_request)` (portable) to the constant 64, which is the LP64 size. On i386 the v1 struct is 52 bytes (4-byte pointers, 4-byte `uint64_t` alignment), so a caller built against the Phase 4 header passes 52 and is refused `RUN_STRUCT_SIZE`; the header comment admits "other pointer widths do not pin it" but the check pins it anyway, and the static asserts are guarded to LP64 only. D-19 chose the constant deliberately, so this is a documented portability regression rather than a slip; it matters only if an ILP32 host is ever in scope.
**Fix:** Either state in `ivs_run.h` and the API effects file that the ABI is LP64-only and add a `#error` or `_Static_assert` for other pointer widths, or define the v1 size as `offsetof(ivs_run_request, psg_log)` so it follows the layout.

### IN-04: Comments left from the plan-by-plan build-up now describe the opposite of the code

**File:** `core/src/psg/psg.h:9`, `core/src/machine/machine_run.c:54,154`, `core/src/machine/machine.h:196`, `core/src/memmap/memsys.h:106,208`
**Issue:** "nothing advances it, so every member is 0 and stays 0 until plan 05-06", "Storage only until plans 05-04..05-06", "nothing reads it yet" and "Nothing reads the masks before plan 05-05" were true when written; the bus wrapper, the catch-up, the log and the input path now read all of it. A reader following the comments would conclude the PSG state is inert.
**Fix:** Rewrite those sentences to describe the final behaviour (or delete them); the per-plan history belongs in the SUMMARY files.

### IN-05: Operator output files are created world-readable (mode 0644) although they derive from operator-supplied content

**File:** `runner/hostio.c:134`
**Issue:** `--audio-raw`, `--psg-log-out` and `--audio-wav` of a `content ... user` run are derived from the owner's rights-bearing EXEC/GROM/cartridge. `AGENTS.md` and the `tools/listen-demo.sh` rule keep such derived audio out of shared records; a 0644 file in a shared directory (subject to the umask) is a weaker default than the policy implies.
**Fix:** Create with mode 0600 and let the operator widen it; the data are not meant to be shared.

### IN-06: Two defensive branches that cannot run

**File:** `core/src/psg/psg_sample.c:111`, `core/src/machine/machine.c:333`
**Issue:** `if (period == 0u) return;` in `ivs_psg_catch_up` and `if (period != 0u)` in `ivs_machine_profile_env` defend against a zero sample period. `IvsPsgTable.cmake` fails the build unless `psg.sample_period_ticks == ticks_per_psg_clock * psg.clocks_per_step` with both factors nonzero, and `IvsMemmapTable.cmake` rejects a zero `ticks_per_psg_clock`, so no registered profile can reach either branch and no test drives it. (This is dead code, not a necessity finding: the code does not run.) The silent early return is also the kind of guard WR-04 argues against, since a zero period would hide a broken profile rather than report it.
**Fix:** Remove the branches and keep the generator check as the single owner of the invariant, or turn the first into an assertion.

---

_Reviewed: 2026-11-12_
_Reviewer: Claude (gsd-code-reviewer)_
_Depth: standard_

## Review disposition

Applied 2026-11-12 on branch `gsd/phase-5-hand-controllers-and-psg-sound`, one atomic commit per finding. Test and fault counts moved from `tests-expected=3772 faults=191` (plan 05-12) to `tests-expected=3778 faults=193` (each case runs once per preset, so +2 ids per added case); `steps=35/35` and `controls=316/316` are unchanged.

| Finding | Disposition | Commit | Test or control |
|---|---|---|---|
| WR-01 failed run leaves output files | Fixed. `ivs-run` removes the `--audio-raw`, `--psg-log-out` and `--audio-wav` files it created whenever the run ends harness-fail (exit 14, whatever the cause), comparing the device and inode recorded at creation (`ivs_hostio_out_remove_if_same`) so a replaced file is never touched; the retry with the same path works. `refpcm.frames` and `refpcm.sha256` are both `-` when no finished file remains. Stated in `docs/runner.md` and the usage text. Other outcomes keep their complete files. | 796688e | `runner-process.operator.failed-run` (new: bound hit on each file and on all three, nothing left, retry succeeds, a budget timeout keeps its 252-byte file; red before the fix); `runner.exec.wav-failures` now also requires `refpcm.frames -` |
| WR-02 `--update-golden` over an output path | Fixed. The same string is refused before anything is created; the same file under another spelling or link is refused once the outputs exist (stat identity, links followed as `fopen` would) and the files just created are removed. Exit 14, one stderr line, no report. `O_NOFOLLOW` on the golden itself was not added (see carried residual 2). | 81662c7 | `runner-process.operator.golden-distinct` (new: same string and `./` spelling for each of the three options, nothing left, a distinct golden works and leaves the raw file intact; red before the fix) |
| WR-03 watchdog FIFO handshake can hang | Fixed in both places. The watchdog sets its TERM trap, starts its sleep, then creates `<mark>.armed`; the parent checks at once, then polls every 0.01 s for at most 500 polls and never past `kill -0` of the watchdog; a dead or silent watchdog is a finding (`c-faults-run: <ID> (watchdog not armed)`; `watchdog-unarmed` in runner-process, with the run stopped), never a hang. The mkfifo path and its command leak are gone. The dash early-TERM fix is kept (trap before arming). | 123a32f | Existing self-tests `watchdog self-test (fire/quiet)` and `runner-process.watchdog` unchanged and passing; new c-faults self-test cases `(arm)`, `(dead)`, `(stall)` and the same two in runner-process.watchdog (a watchdog that dies before arming, one that never arms: bounded failure). Loop of 220 to 300 start/stop pairs of the real functions with 8 to 12 busy loops (load average 36): macOS sh, dash, bash and Ubuntu 24.04 dash and bash, 0 fired, 0 unarmed, target alive |
| WR-04 tables not tied to the registry | Fixed. A registry index is usable only below the smallest of `ivs_profile_count`, `ivs_psg_params_count` and `ivs_input_profile_count`: the lookup (UNKNOWN_PROFILE), the handle check and `ivs_memsys_init` refuse anything else, and the NULL guards that silently disabled the PSG (bus wrapper, step seam, memsys) are removed. | a4395c4 | `machine.create.tables-cover-registry` (new) and named faults `PSG_TABLE_SHORT` and `CTRL_TABLE_SHORT` (new, each fails it; `machine.api.version` must still pass) |
| IN-01 port direction bits read through the R7 read mask | Deferred. With the one registered profile both masks are `$00FF`, so behaviour is identical; the fix (read the stored word, or make the generator check that rmask(R7) holds the direction bits) belongs with the next profile that changes the R7 masks, where it can be tested against real data. | n/a | n/a |
| IN-02 `ivs_refpcm_out_bound` wrap | Fixed (small). The bound saturates to `SIZE_MAX` and `ivs_refpcm_feed` refuses it as `IVS_REFPCM_E_SPACE` before reading or writing; the header sentence is corrected. | fef5487 | `runner.refpcm.refusals-consume-nothing` extended with the wide-parameter checks (red before the fix); no new id |
| IN-03 `IVS_RUN_REQUEST_V1_SIZE` is 64 on every ABI | Deferred, behaviour unchanged. D-19 chose the constant deliberately, no ILP32 host is currently in scope, and a `#error` or layout-derived size would change a pinned ABI statement. Revisit if a 32-bit host is ever scoped. | n/a | n/a |
| IN-04 stale plan-by-plan comments | Fixed. `psg.h`, `machine_run.c`, `machine.h`, `memsys.h` now describe the final behaviour. Comment-only. The public header `ivs_run.h` line 120 (`until plan 05-08/05-09`) was outside the finding and is left. | 17e8745 | n/a (comments; gate unchanged) |
| IN-05 operator outputs created 0644 | Deferred, behaviour unchanged. Mode 0600 is a policy choice for the owner (it affects the listening handoff and tooling that reads the files back); recorded again as carried residual below. | n/a | n/a |
| IN-06 two unreachable defensive branches | Deferred, no change. The two `period == 0` branches (`psg_sample.c`, `machine.c`) are unreachable by construction (the generator refuses a zero period) and do no harm; removing them is cosmetic and was kept out of this fix set, which only changes behaviour where a finding is a behaviour bug. The generator stays the single owner of the invariant. | n/a | n/a |

### Non-blocking residuals of 05-SECURITY.md

Carried unless noted.

1. Failed runs leave created output files: **resolved** by WR-01 (files removed on harness failure, `refpcm.frames` and `refpcm.sha256` `-`).
2. Operator-path trust: `--update-golden` still uses `fopen(path, "w")` (follows symlinks, truncates a regular file by design) and content-directory reads still follow symlinks (no `O_NOFOLLOW`). **Partly addressed** by WR-02 (the golden can no longer name an output file); the rest is carried.
3. Watchdog orphan after a `SIGKILL` of the harness (test-only): carried. WR-03 does not change it.
4. No named control for the measured-tier claim rule (`claim-measured-without-d10`): carried (suggested control `claims-measured-without-d10`).
5. TOCTOU on the event array is documented, not prevented: carried.
6. `refs-verified=3/4` cache verifier: carried (informational).
7. Two test skips guarded by `tests-inventory-unexpected-skip`: carried.
8. Public-history scratch strings from `04-SECURITY.md`: carried.
9. Unregistered flags: operator outputs created 0644 (this review's IN-05): carried; raw output extensions `*.raw` and log text files are not in `.gitignore`: carried.

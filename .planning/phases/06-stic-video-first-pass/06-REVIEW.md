---
phase: 06-stic-video-first-pass
reviewed: 2026-11-13T00:00:00Z
depth: standard
files_reviewed: 38
files_reviewed_list:
  - core/include/ivs/ivs_machine.h
  - core/include/ivs/ivs_run.h
  - core/src/stic/stic.h
  - core/src/stic/stic_params.h
  - core/src/stic/stic_params.c
  - core/src/stic/stic_state.c
  - core/src/stic/stic_regs.c
  - core/src/stic/stic_events.c
  - core/src/stic/stic_frame.c
  - core/src/stic/stic_lines.c
  - core/src/stic/stic_cards.c
  - core/src/machine/machine.c
  - core/src/machine/machine.h
  - core/src/machine/machine_bound.c
  - core/src/machine/machine_bus.c
  - core/src/machine/machine_digest.c
  - core/src/machine/machine_inspect.c
  - core/src/machine/machine_run.c
  - core/src/memmap/memsys.c
  - core/src/memmap/memsys.h
  - core/src/psg/psg.h
  - core/src/psg/psg_state.c
  - cmake/IvsSticTable.cmake
  - cmake/IvsMemmapTable.cmake
  - cmake/IvsVideoFrames.cmake
  - runner/main.c
  - runner/exec.c
  - runner/hostio.c
  - runner/hostio.h
  - runner/report.c
  - runner/runner_vchain.c
  - runner/frame_export.c
  - runner/scenario.c
  - runner/ivs_runner.h
  - tests/installed-consumer/main.c
  - tests/runner-process-tap.sh
  - tools/check/listing.cmake
  - tools/check/video.cmake
findings:
  critical: 0
  warning: 3
  info: 5
  total: 8
  necessity: 0
status: issues_found
---

# Phase 6: Code Review Report

**Reviewed:** 2026-11-13
**Depth:** standard
**Files Reviewed:** 38. Also read, not line by line: tools/check/steps/{golden,important-gates,pkg-install,video}.sh, tools/check/core-symbols.sh and its deny list, tools/pack/CMakeLists.txt, tests/expect-inc.cmake, tests/faults.cmake, fixtures/FX-diag-stic-1/diag-stic-1.listing.txt, the Phase 5 REVIEW disposition, 06-CONTEXT.md D-17 and PLAN-LESSONS.md. The test suites under tests/stic, tests/video, tests/run and tests/runner were sampled (video_buffers.c, video_judge.c) and not reviewed in full.
**Status:** issues_found

## Summary

I read the STIC engine (state, register effects, event timeline, line levels, card renderer, frame writer and chain), the machine seam and bus wrapper, the run ladder and its capacity law, the public header growth (info v4, report v3, `IVS_RUN_F_STOP_ON_FRAME`, `ivs_machine_stic_state`), the runner (`ivsr 3`, chain re-hash, `capture video`, frame asserts, operator output files), the installed consumer, and the two checkers that changed (SDBD rule, video oracle). Where a claim needed proof I ran `build/release/runner/ivs-run` and a scratch copy of the listing checker (scratchpad only, no repository file touched).

The core is in better shape than the tooling around it. I looked for integer-promotion and UB problems in the 14/16-bit paths, out-of-range indexing of the row latches and frame slots, capacity and slack arithmetic, slice dependence and chain framing, and found none: every shift is on `uint32_t`, `row_latch` and `cap_vid` indexing is bounded at each use, the slack law holds for slices from 1 tick to the 4,194,304 maximum (27 slice sizes around every frame, window and halt boundary all gave `outcome pass` and a byte-identical `digest.composite` and `digest.video`), the core's domain-16 framing matches the runner's recomputation, and the exclusive-create, identity-checked removal and `--update-golden` alias refusal behave as documented (I tried the same string, `./` spelling, a symlink, two options on one path and a bound hit; none left a file or touched an existing one).

What I did find: a checker that is weaker than the CPU it guards, an operator file path (`--update-golden`) that the Phase 5 cleanup rule never reached and that Phase 6 made easier to hit, and a family of silent drops when a profile's geometry is outside what the hard-coded renderer can draw, with nothing in the generator to stop it.

## Warnings

### WR-01: The listing checker accepts an `SDBD` before any non-branch instruction, but the CPU faults on every target except six immediates and five `@` forms

**File:** `tools/check/listing.cmake:71-72,458-466,490-495` (rule), `core/src/cpu/cp1610.c:1046-1067,1087-1088` (CPU truth)
**Issue:** The new rule refuses an `SDBD` before a branch or jump, a doubled `SDBD`, a missing or extra operand and an operand byte above `00FF`, then states "every other non-branch form after SDBD is accepted unchanged". The CPU's own refusal (`ivs_sdbd_target`, POL-SDBD-02) is the opposite shape: with DBD set, only `MVII ADDI SUBI CMPI ANDI XORI` and `MVI@ ADD@ SUB@ CMP@ AND@ XOR@` with mode 1..5 are targets, and anything else (`MVO`, `MVO@`, `MVI` direct, `NOP`, `INCR`, `HLT`, `PSHR`, a register row, `MVI@` with mode 0 or 6) ends the guest in `FAULTED` with `UNSUPPORTED_SDBD_TARGET`. So a listing the gate calls valid can be a program that faults on its first instruction after the prefix. Reproduced on a scratch copy of the tree: in `FX-diag-stic-1` I replaced the `MVII #$5065, R4` triple at 501D-501F with `MVO@ R0,R5` and two `INCR R0` (so `SDBD` at 501C now prefixes `MVO@`):

```
cmake -DROOT=<scratch> -DFILELIST=<listings> -DCHECK_DIR=tools/check -P tools/check/listing.cmake
-- listing pass listings=4 listing-words=300
```

The branch exclusion shows the intent was to catch exactly this class (a prefix before something that cannot take it); it stops at the rows that happen to be branches. Three guest diagnostics are hand-assembled with no assembler, so this checker is the only static defence.
**Fix:** Invert the list: keep `SDBD_IMMEDIATE_ROWS` and add `SDBD_INDIRECT_ROWS` (`MVI@ ADD@ SUB@ CMP@ AND@ XOR@`), and refuse every other row after an `SDBD` as `listing-sdbd`. The mode field (bits 5..3 of the decle, 1..5 allowed) is available in `hv` at that point:

```cmake
set(SDBD_INDIRECT_ROWS MVI@ ADD@ SUB@ CMP@ AND@ XOR@)
if(this_target AND NOT row IN_LIST SDBD_IMMEDIATE_ROWS)
  if(NOT row IN_LIST SDBD_INDIRECT_ROWS)
    lfail(listing-sdbd "${at} SDBD before ${row}, which is not an SDBD target")
  else()
    math(EXPR mmm "(${hv} >> 3) & 7")
    if(mmm LESS 1 OR mmm GREATER 5)
      lfail(listing-sdbd "${at} SDBD before ${row} with mode ${mmm} (only 1..5 are SDBD targets)")
    endif()
  endif()
endif()
```

Add a control per refused family (`listing-sdbd-target-mvo`, `-target-nop`) next to the three existing ones; remove "every other non-branch form" from `docs/diagnostics.md`.

### WR-02: `--update-golden` writes a golden for a run that ended harness-fail, truncating a good file, and the Phase 5 "failed run leaves no operator file" rule stops short of it

**File:** `runner/main.c:384-441` (`main_write_golden`), `runner/main.c:816-831`
**Issue:** After a run, `main` tests `fflush`/`ferror`/`res.report_write_failed` and then writes the golden with `fopen(path, "w")` regardless of the outcome. `main_write_golden` refuses only `cancelled` and a `wall` timeout. Every other harness-fail outcome (and `missing-fixture`) is recorded as a golden, and the existing file at the path is truncated first. Phase 6 added several new operator-controlled ways to a harness-fail (`video-out-max-bytes`, `frame-ppm-out-max-bytes`, `frame-ppm-convert`, `video-chain`), so a single bad flag now replaces a tracked golden with a failure record. Reproduced:

```
$ echo precious > g.digests
$ ivs-run --video-raw v.raw --video-out-max-bytes 100 --update-golden g.digests diag-stic-fgbg-pass.ivss
ivs-run: --video-raw: the run failed, the incomplete output file was removed
$ head -12 g.digests
# domains: cpu=observed mem=observed images=observed instance=observed video=observed ...
scenario.id diag-stic-fgbg-pass
outcome harness-fail
outcome.detail video-out-max-bytes
exit.code 14
```

The run's own `v.raw` was removed as a failed-run file, while the golden (the one file most likely to be a tracked `data/golden/*.digests`) was both written and destroyed, and says `video=observed` for a run that was cut short. Phase 5 carried "`--update-golden` truncates a regular file by design" as a residual; that rationale covers overwriting a good golden with a new good one, not with a harness failure.
**Fix:** Treat the golden as a result only of a run that produced a complete, deterministic report: refuse in `main_write_golden` when `outcome` is `harness-fail` or `missing-fixture` (the same place the `cancelled` and `wall ` refusals live), and write to a sibling temporary file renamed over the target on success so a failed write cannot leave a half golden either:

```c
v = main_report_value(buf, len, "outcome", &vn);
if ((vn == 9u && memcmp(v, "cancelled", 9u) == 0) || (vn == 12u && memcmp(v, "harness-fail", 12u) == 0) ||
    (vn == 15u && memcmp(v, "missing-fixture", 15u) == 0)) {
  return main_fail("golden refused: host-dependent or failed outcome");
}
```

Add a case to `runner-process.operator.video-raw-failed-run` that passes `--update-golden` over an existing file and requires it unchanged.

### WR-03: A profile geometry outside what the hard-coded renderer can draw passes the generator, and the core then silently produces no frame or blank frames

**File:** `cmake/IvsSticTable.cmake:352-399` (checks), `core/src/stic/stic_frame.c:66-78,127-137,184-186`, `core/src/stic/stic.h:34-40,98-103`
**Issue:** The renderer, the row latch and the working buffers are fixed: `IVS_STIC_ROW_WORDS 20`, `IVS_STIC_LATCH_ROWS 12`, 8 x 8 cards, `IVS_STIC_CARDROW_MAX 2048`, `IVS_STIC_PLANE_MAX 15360`. The generator accepts `video.card_columns`, `video.card_rows` and each card dimension up to 255 and checks only that the width fits the object grid and `backtab.words == columns x rows`. Nothing ties the geometry to what the core can render. A profile that passes the generator then behaves as follows, all silently:
- `card_columns != 20` or card size `!= 8 x 8` (e.g. 24 columns): `ivs_stic_render_card_row` takes the "a geometry the card decoder does not draw" branch and fills every position with `video_blank_value`, so each display-on frame is delivered, hashed and chained as a wholly blank picture.
- `card_columns x card_w x card_h > 2048` (e.g. 33 columns): `ivs_stic_frame_deliver` hits its early `return` at line 136 and produces nothing: no slot, no chain step, no `count` advance, no `video_frames` increment, while `ivs_stic_catch_up` still advances `frame_index`. The report's frame index and the machine's frame index then diverge, `IVS_RUN_F_STOP_ON_FRAME` never stops, and `ivs_machine_info` still advertises a video layout and a slot size.
- `card_rows > 12`: rows 12 and above are drawn blank (`cr < IVS_STIC_LATCH_ROWS` fails) with no finding.
- In the same function the mid-frame `break` at line 184-186 (after the header has been hashed and the first rows copied) would leave a delivered slot whose tail is whatever the caller's buffer held while `video_written` and `count` still advance, so the chain and the delivered bytes disagree. It is unreachable only if the first guard holds.

The comments say "never reached by a registry profile", but the only thing that makes that true is that there is one profile. The generator's own header promises it is "fail closed" and already refuses unimplemented policies by name; the geometry is an implemented-only value too.
**Fix:** Make the generator the single owner of the constraint, in the same style as the other policy checks:

```cmake
set(ST_CTX "${rel}: video.card_columns")
if(NOT cards_x EQUAL 20 OR NOT cards_y EQUAL 12 OR NOT card_w EQUAL 8 OR NOT card_h EQUAL 8)
  st_fail("the card renderer implements only 20 x 12 cards of 8 x 8 pixels (got ${cards_x} x ${cards_y} of ${card_w} x ${card_h})")
endif()
if(rows GREATER cards_y)
  st_fail("stic.bus.rows ${rows} is above video.card_rows ${cards_y}")
endif()
```

and keep the two runtime guards, but make a refused geometry an `INTERNAL` stop (set `sink->video_overflow`) instead of a silent skip.

## Info

### IN-01: `--frame-ppm` is refused for `content ... user`, `--video-raw` is not, and a run that delivers no frame exits 0 with an empty file

**File:** `runner/main.c:647-656`, `runner/exec.c:1054-1060,1162-1180`, `docs/runner.md:39`
**Issue:** The stated rule (T-06-17) is that a frame rendered from an owner image is derived art of rights-bearing content. `--video-raw` of the same run writes the same pixels as index bytes, 15,296 bytes per frame with a self-describing header, and is allowed (mode 0600 only). The block is therefore a policy statement, not a control: converting the raw file to a PPM is a few lines. Separately, when no frame was delivered (a run that ends before tick 59,736, like `diag-boot-pass`) `--frame-ppm` and `--video-raw` leave a 0-byte file and the run is `pass`, exit 0, `video.chain.check pass`. I reproduced both. That is documented and tested (`video-raw-empty-run`), but a script that asked for a preview gets success and nothing to view.
**Fix:** Decide the policy once: either refuse `--video-raw` for user content as well, or say in `docs/runner.md` that the block is advisory. For the empty case, consider a one-line stderr note ("no frame was delivered, the file is empty") without changing the exit code.

### IN-02: Profile-owned values are hard-coded in the core, so the generated parameters can disagree with what the renderer reads

**File:** `core/src/stic/stic_frame.c:28-31`, `core/src/memmap/memsys.c:463-466`, `core/src/stic/stic.h:34-35`
**Issue:** `IVS_STIC_REG_BORDER 0x2C` and `IVS_STIC_REG_STACK0 0x28` are register addresses; `ivs_memsys_stic_backtab_word_internal` returns `sysram[i]` for `i < 240` whatever `stic.backtab.lo` and `words` say; the display-off fill is "border" because the generator refuses anything else. `stic_regs.c` states "no register address literal appears here", `stic_params.h` says the core holds no profile constant (D-02), and the hook class table is generated from `stic.backtab`, so moving `lo` would move which accesses catch the STIC up but not which words the row fetch reads. `backtab_lo`, `backtab_words`, `bus_rows`, `video_colors`, `video_grid_*`, `reg_class` and `video_display_off_fill` are published and read by no core source (tests read some of them).
**Fix:** Either read the values from `ivs_stic_params` (`p->backtab_words` for the bound and a generated sysram offset for the base) or have the generator refuse a profile whose `stic.backtab.lo` is not the sysram base and whose regmap roles are not `border` at `$2C` and `stack` at `$28..$2B`, so the duplication cannot drift silently.

### IN-03: The runner's slot, PPM and chain-literal sizes are copies of one profile's numbers

**File:** `runner/ivs_runner.h:356-359`, `runner/exec.c:1052-1060`, `tools/check/video.cmake:473`
**Issue:** `IVS_RUNNER_VIDEO_SLOT_MAX 15296` (32 + 159 x 96), `IVS_RUNNER_PPM_MAX 46000` and the checker's `want_tick = (n+1) x 4 x 57 x 262` restate the geometry and timebase. With a wider or taller frame the capture path fails loudly (`video-slot-too-large`), but the `--frame-ppm` path does not: `x->vid_slot_bytes <= IVS_RUNNER_VIDEO_SLOT_MAX` is a silent condition on `have_last_slot`, so the file is left empty and the run passes. The capacity of the capture store is also charged to every scenario with any `capture` directive (`ex_create`, `need_tail`), 244,736 bytes more than a psglog-only capture needs.
**Fix:** Fail with `video-slot-too-large` (or `frame-ppm-convert`) when a requested export cannot be held, instead of skipping it; derive the checker's tick from the profile or compare it with `data/video/geometry.literal.txt`.

### IN-04: `video.chain.check` prints `pass` for a run that delivered no frame

**File:** `runner/exec.c:1141-1158`
**Issue:** With a video buffer on every slice and zero frames the runner's chain is h0 and the core's digest is the digest of h0 with count 0, so they match and the report says `video.chain.check pass`. The check compared two empty chains. It is correct as arithmetic, but a reader (and the golden, which keeps the line while video is observed, so only when bytes are nonzero) takes `pass` as "frames were verified".
**Fix:** Print `-` when `x->vchain.frames == 0`, as `audio.chain.check` does for a run with nothing to check, or document `pass` as "consistent, possibly empty".

### IN-05: Small diagnostics and a dead store

**File:** `runner/main.c:258-270`, `tools/check/listing.cmake:441-444`, `core/src/machine/machine_run.c:182`
**Issue:** (a) `--video-raw x --frame-ppm x` (or any two outputs on one path) reports "output file exists, refusing to overwrite it" for the second option although the path was free before this invocation; the first file is discarded correctly, only the message is misleading. (b) `listing-sdbd` is also reported for a stray `-` line that follows an unrelated instruction two places after an `SDBD` target (`prev_target OR after_target` is a proxy for "just after a target"), where the operand-count token is the accurate one. (c) `uint64_t sample = start;` is overwritten on every non-fault build (clang analyzer: dead store), and `start - lag` is computed unsigned before the `start >= lag` guard that makes it unused.
**Fix:** (a) check `main_same_path` among the five outputs before creating anything and print "names the same path as ..."; (b) track the exact count of lines since the target instead of two flags; (c) initialise `sample` inside the `#ifdef` branch and compute `start - lag` only under `start >= lag`.

---

_Reviewed: 2026-11-13_
_Reviewer: Claude (gsd-code-reviewer)_
_Depth: standard_

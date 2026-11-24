# Phase 6: STIC Video First Pass - Research

**Researched:** 2026-11-12
**Domain:** growing a C17 deterministic core with a second time-driven device (the STIC): a register file behind access windows, a frame timeline that drives the CPU's INTRM and BUSRQ lines, a raw palette-index framebuffer with its own digest chain, a frame-stop run flag, ABI growth, runner growth, a third original diagnostic, and the evidence machinery (goldens, inventories, registries, gate) that carries all of it
**Confidence:** MEDIUM-HIGH (code facts, struct arithmetic, derived constants and the CPU stepper's behaviour under a driven BUSRQ/INTRM schedule were read or executed this session: HIGH; hosted timing, gcc-only behaviour of code that does not exist yet, and every hardware-facing number: MEDIUM or LOW, and those numbers are policy by decision D-01/D-05/D-09)

Scope note: the design is locked by D-01..D-24 and the Reconciliation in `06-CONTEXT.md`; the five `06-DISCUSS-*.md` files are the audit trail. No hardware or chip question was re-researched, no web search was made, no library documentation was consulted (the phase installs no package). This file grounds the decisions in the CURRENT code and tooling: which file and mechanism changes for each decision (file:line), whether the CONTEXT assumptions about the code are true (several are not, see the check table), a decomposition check of D-23 with a recommended re-cut, ranked pitfalls, and the validation map (`06-VALIDATION.md`). Probes run this session on this host (Apple clang 21, CMake 4.4.3, Ninja 1.13.2, dash, Docker 29.5.2 with `ubuntu:24.04` cached): `sh tools/gate.sh` on the unmodified tree (baseline below); a throwaway C harness, kept outside the repository and not committed, that drives the real `ivs_cp1610_step` (compiled from `core/src/cpu/cp1610.c` and `cp1610_decode.c`) with schedule-driven `busrq` and `intrm` levels sampled at `start - 4` ticks; a layout harness that proves the struct arithmetic with static asserts; a throughput and SHA-256 micro-benchmark against `build/release/core/libivs_core.a`. No repository file other than the two phase documents was written.

<user_constraints>
## User Constraints (from CONTEXT.md)

### Locked Decisions
D-01 through D-24 and the Reconciliation section of `.planning/phases/06-stic-video-first-pass/06-CONTEXT.md` `<decisions>` are locked and incorporated by reference (a verbatim copy would drift; the planner reads that file first, as Phase 5 research did). One-line index:
- Evidence posture: D-01 page-reading verdict FIRST in oracle plan 06-01 (no C); D-02 oracle-first, hand-derived literal frames (about 8), policy numbers proven by perturbation and accounting identities, never expected values; D-03 STIC pinned AY-3-8900-1 NTSC, `stic.part` tier-tagged.
- Device (VID-01, VID-06): D-04 `stic14` device kind, generated `stic.regmap`, `ivs_stic_state`, all side effects in the machine bus wrapper, one new const call `ivs_machine_stic_state()`; D-05 windows are a pure function of the access stamp (registers 2,000 cycles, GROM/GRAM 3,780), gates flipped at catch-up, `stic.access_gating_when_off = apply`; D-06 blocked policy (drop / `$FFFF`, counted), display latch set by a `$0020` write inside the window and cleared every frame start, persistent Color Stack mode switched by a `$0021` read, stack pointer reset each frame.
- Timeline (VID-02): D-07 frame `F = 59,736` ticks constant, `S_k = frame_phase_ticks + k x F`, frame index and event cursor are state, correct `docs/time-base.md` first; D-08 INTRM level 3,791 cycles from `S_k`, `bus.line_sample_lag_cpu_cycles = 1`, C41 stays 0, ISRs DIS-first; D-09 13 bus windows (first 3,791, pitch 912, hold 114), the stall is the stepper's own BUS_GRANT sequence; D-10 catch-up at every unit end and before each STIC access, `U = 468` and `g = 28` re-proved through the full machine; D-11 row-granular fetch (BACKTAB latched per window), frame-granular render at `E_k`.
- Frame, buffer, API (VID-03): D-12 D8 = 159 x 96, width = `19 x 8 + video.last_card_column_px`; D-13 one byte per position, `0xFF` not displayed, 32-byte header plus plane, `IVS_OUTPUT_FORMAT_INDEX8_FRAMES_V1`; D-14 frame completes at the first unit boundary `>= E_k`, `IVS_RUN_F_STOP_ON_FRAME = 2`, `IVS_STOP_FRAME = 5`, report v3, `ivs_info` v4, `video_*` counters live; D-15 video capacity `slot x (D/F + 2)` with env member `video_slack_frames = 1`, fault `VIDEO_CAP_PLUS_ONE`; D-16 domain 16 per-frame chain over the delivered slot bytes, composite 2 -> 3, STIC registers in MEM, `ivs_stic_state` in INSTANCE; D-17 runner `ivsr 3`, `capture video`, `assert frame`, `video.chain.check`, operator-only `--video-raw`.
- Cards (VID-04): D-18 pure renderer, FG/BG and Color Stack, Colored Squares with a documented cut line, named-fault set; D-19 no GROM in CI, glyphs from GRAM through the raw-poke seam and one guest fixture, literal expected frames.
- Verification: D-20 third diagnostic `ivs-diag-stic-1` (stub contract 1 unchanged, no ISR, mode by controller input at tick 0, listing-checker SDBD rule); D-21 suites `stic` and `video`, caps 28 faults and 25 controls for the phase and 5 tasks, 8 faults, 25 controls per plan, a small lever pass early (`-g0` on fault targets, normalised metric); D-22 claims CL-0016.., one allocation table (next free discrepancy id C81), `d10-measurements.ids` extended; D-23 eleven-plan outline; D-24 hard stops (D10, D11 not performed, datasheets stay in the gitignored cache).

### Claude's Discretion
CONTEXT has no separate section; D-14 leaves "the exact offsets and sizes" of report v3 and `ivs_info` v4 to the plan ("pinned by the plan with 64-bit pointer math"), D-23 leaves the cut ("at most 5 tasks, 8 faults and 25 controls per plan"), and D-04/D-18 leave struct and field names to the plans (names from `06-DISCUSS-regs.md` section 5/6 and `06-DISCUSS-cards.md` where they exist). The planner may re-cut plans on a file-set collision or a cap overrun (verify note V6-23), choose the final fault and control ids, and choose where the operator-side frame export helper lives (Open Question 3).

### Deferred Ideas (OUT OF SCOPE, verbatim from CONTEXT)
- MOBs, collision, delay registers, border extension, mid-frame register effects, per-row BACKTAB beyond the latch (Phase 8); a project ISR and sustained display in the boot stub (Phase 7); window-copy-mode event machine if hardware evidence supports it; tight video capacity formula; PAL timing; presentation (palette, scaling, PNG) beyond the operator-side export; a mini-assembler (D4 trigger, expected Phase 7); `-O0` fault objects or cached fault builds (owner decision, D12).
- Phase 4 review deferrals IN-04, IN-06, IN-09..IN-11 and Phase 5 deferrals IN-01, IN-03, IN-05, IN-06 stay open.
</user_constraints>

<phase_requirements>
## Phase Requirements

| ID | Description | Research Support |
|----|-------------|------------------|
| VID-01 | STIC register access, display-enable handshake and VBLANK access windows follow profile timing constants, blocked accesses follow the documented policy | New device kind `stic14` and generated table (mechanisms 3-5), wrapper hook and stamp (mechanism 1), gate flips at catch-up (mechanism 2), windows closed-form from `S_k` (derived constants), memmap literal flip (mechanism 4 and Pitfall 9), suite `stic` families `stic.reg.*`, `stic.window.*`, `stic.blocked.*`, `stic.enable.seq.*`, step `video` |
| VID-02 | VBLANK interrupt and BUSRQ/BUSAK bus-steal at profile-defined machine times; effect on CPU progress tested | `lines.intrm`/`lines.busrq` are hard-wired 0 today (machine_run.c:150); the stepper's grant/hold/release and open-boundary rules executed under a driven schedule (derived constants: resume at `w + 468` ticks, 13,452 to 13,524 executed cycles per display frame, 253 nested entries with C41 = 0); `stic.irq.*`, `stic.busrq.*`, `stic.tile.*`, `stic.steal.*`, `stic.bounds.*` |
| VID-03 | Raw palette-index framebuffer at a defined machine-time boundary with profile geometry (D8) | Video row and domain 16 exist (machine_digest.c:109-110), report counters exist but are never assigned (machine_bound.c:637-650), `ivs_info` has `video_format`/`video_bytes_per_frame` at 228/240 (ivs_machine.h:162-164), run flag 2 exists as reserved (ivs_run.h:92); struct arithmetic (report v3 160, `ivs_info` v4 320 candidate, instance 114,416 candidate), capacity corner executed, runner sizing coupling (Pitfall 7), `video.*` families |
| VID-04 | FG/BG and Color Stack render hand-derived index frames from known BACKTAB/GRAM contents, no real GROM in CI | BACKTAB is the first 240 words of `sysram` (no named region), GRAM is `m->gram[512]` through the raw-poke seam `ivs_memsys_poke_raw_internal` (memsys.h:215), GROM zero image path, pure renderer through dedicated accessors, literal frames and the pure step `video`, suite `video` card families |
| VID-06 | Side-effect-free inspection path for STIC/device state | `ivs_machine_peek` is `ivs_memsys_read` over the const memory system (machine_inspect.c:98-100), so effects must live in the wrapper (D-04); new const call joins the API effect table (14 -> 15 calls) and the installed consumer's exact symbol list (Pitfall 11); whole-instance byte-equality sweeps plus the positive control |
</phase_requirements>

## Project Constraints (from CLAUDE.md / AGENTS.md / AUTONOMY.md)

- Prefix `ivs_`; never "intellivision" in a package, product or repo name (hard stop). C17 + CMake (D1). No copying from MAME, jzIntv, libasm or any assembler or other emulator; agreement with another emulator is not evidence. "Another copy-paste is better than another dependency" (AUTONOMY section 2): no external package is installed by this phase.
- Rights: datasheet PDFs, wiki images and fetched pages stay in the gitignored `refs/` cache [VERIFIED: `git check-ignore refs` prints `refs`]; `data/sources.json` entries carry `rights: unknown`; literal files hold derived hexadecimal facts and short paraphrases only. No committed `.ivsc`, `.bin`, `.rom`, `.wav`, `.ppm`; `tools/check/binary-ext.txt` ends with `wav` and does NOT list `ppm`, and `.gitignore` line 18 is `*.wav` with no `*.ppm` or raw-video entry [VERIFIED: `tail -3 tools/check/binary-ext.txt`, `grep -n "wav\|ppm\|raw" .gitignore`], so Phase 6 must add both (Pitfall 16).
- Shift-left: zero human UAT. D10 (hardware) and D11 (owner EXEC/GROM) are not performed; evidence needing them is recorded missing and handed off (D-22, D-24). Executors never call `gh`; the orchestrator runs `tools/ci-record.sh`.
- `sh tools/gate.sh` is the single local and CI command; it must stay green under `sh`, `dash tools/gate.sh` and `sh tools/gate-linux.sh` at every plan end that adds C or a new shell construct. Measure before changing CI shape.
- Docs rule: README and `docs/**/*.md` are scanned by `text` and `glossary` (claim words need `[claim:CL-NNNN]` or `[nc]`; the bare word clock fails unless the line says input clock, STIC clock, PSG chip clock or `[nc]`); `.planning/` is not scanned by either. The two Phase 6 documents therefore carry no claim-word obligations, but `docs/time-base.md`, `docs/runner.md`, `docs/diagnostics.md` and the new `docs/stic/`, `docs/video/` files do.
- Profile-field names: a last segment containing a whole token window/offset/hold/duration/delay/period/latency/interval/timeout must end `_ticks` or `_cpu_cycles` (tools/check/steps/glossary.sh, Phase 5 research); `stic.reg_window_cpu_cycles`, `stic.gmem_window_cpu_cycles`, `stic.irq_low_cpu_cycles`, `stic.bus.hold_cpu_cycles`, `bus.line_sample_lag_cpu_cycles`, `stic.frame_phase_ticks`, `stic.access_strobe_ticks` comply.
- Hygiene (tools/hygiene.sh): no absolute home path, no scratch or temp-directory path form (the macOS private temp tree, the per-session scratch directories, tilde-relative paths), no secret shape, and no non-noreply email shape (tools/hygiene.sh:44 holds the exact rule: local part, at sign, dotted domain). A name-at-name-dot-word string in a tracked doc fails, and so does a doc that quotes a forbidden path: write CP-1610 indirect mnemonics as "MVO-indirect", never use the at sign in prose, and never spell out a scratch path. Scripts use `mktemp -d`; docs use `<repo-root>`. The gate's `hygiene` step scans untracked-not-ignored files too, so a research file that quotes a forbidden form fails the gate (this very file did once and was corrected).

## Summary

Phase 6 makes the STIC the second time-driven device and the first one that changes CPU progress. The PSG precedent (a machine-level bus wrapper, a per-unit context, catch-up at every unit end, a per-call sink, rows tagged INSTANCE/MEM, a generator-fed parameter struct, a region flip in the device plan) carries over almost unchanged; what is new is that the device now drives the CPU's `lines` (the stepper is untouched), that frame completion produces 15,296-byte slots, and that the digest, the run ladder and the runner all gain a real video channel. The design is sound against the code. The plan risk is, as in Phase 5, registration breadth (profile fields, a fourth generated table, inventories, key lists, toy trees, goldens), plus three couplings the CONTEXT does not mention: the runner's buffer sizing, the runner's permanent `STOP_ON_HALT`, and the stub-versus-real rule of the memmap generator.

Eight findings change what the planner writes. (a) The two hard-wired lines are exactly where CONTEXT says (machine_run.c:150) and the stepper behaves as the timing note states: executed against the real `ivs_cp1610_step`, a BUSRQ window starting at tick `w` always resumes the CPU at `w + 468` ticks (117 cycles) whatever the grant phase, a window is granted only at an OPEN boundary (a stream of non-interruptible instructions such as MVO is never stalled and never takes the interrupt), and with C41 = 0 and INTFF held set an ISR that is not DIS-first is re-entered 253 times per frame. (b) The memmap generator forbids a physical device that is half stub and half real (IvsMemmapTable.cmake:479-492), so the regs note's "the read alias `$0040-$007F` stays `impl=stub`" cannot be built as written: flip all STIC regions to the real device (recommended) or change the generator. (c) The "test that refuses the reserved flag" is one test id (`run.refuse.flags-frame-reserved`) but the flag value is also used as a generic unsupported flag at five more call sites in three test files, plus the effect table text. (d) The runner is not isolated to plan 06-09: `ex_plan_buffers` sizes video as per-frame bytes times `slice/F + 1` (exec.c:694-707) and `ivs-run` always supplies an 8 MiB block (main.c:674, 743-744), so the moment the core publishes a real video layout every runner scenario needs the D-15 slack, or it harness-fails with BUFFER_TOO_SMALL. (e) The runner always sets `IVS_RUN_F_STOP_ON_HALT` (exec.c:824), so the third diagnostic must not execute HLT before the first frame end plus completion (cycle 14,934 plus stalls), or no frame exists to assert; and D-05/D-06 give it hard cycle budgets (display strobe within 2,000 cycles of tick 0, GRAM writes within 3,780) that the verify note's "no cycle counting" outline missed. (f) The PSG bounds survive a driven BUSRQ by monotonicity and by measurement: grant, hold and release units are at most 2 cycles (8 ticks), the densest writer (non-interruptible MVO-indirect) writes 1,659 times per frame with the display on or off, against W_log = 2,152. (g) The video `+1` capacity corner is real and reachable with a unit of only 8 ticks (now = F - 4, D = 2F - 1, overshoot 44 ticks completes 3 frames against a formula of 2); the runner's per-slice total with `slot x (D/F + 2)` is 4,615,244 bytes against the 8 MiB cap. (h) Struct arithmetic: report v2 136 -> v3 160 (three u64 at 136, 144, 152), `ivs_info` v3 288 -> v4 320 with the candidate field list below (312 with the minimal list), `ivs_run_env` stays 32 bytes if `video_slack_frames` takes the unused `reserved_env` word, `ivs_memsys` 113,264 -> 113,392 with `stic_reg[64]`, and the instance 113,760 -> 114,416 with a 528-byte STIC state (multiple of 16, `obs` stays 8-aligned).

Gate cost: baseline measured this session `gate: PASS steps=35/35 controls=316/316 faults=193/193 tests-expected=3778 tests-executed=3776 tests-skipped=2 goldens=7 gates=48 lines=175 claims=15 discrepancies=80 d10-items=32`, `gate-time total=42`, `gate-cpu user=70.61 sys=60.13` on 18 cores (controls 15 s, c-faults 12 s, c-tests 8 s the largest steps). The new engine is cheap (release 9.3 million units per second; one frame slot of SHA-256 is 101 microseconds release, 126 microseconds under ASan and UBSan, so 71 slots per default runner slice cost 9 ms); the cost is registrations, reruns and the cold build, exactly as in Phase 5, whose estimate was exceeded by a factor of 2 to 2.6 (verify note V6-11).

**Primary recommendation:** cut D-23 into 14 plan slots in 14 sequential waves (06-03 and 06-04 each split, see Plan Decomposition); oracle plan first and C-free (it also carries the listing SDBD rule and the `docs/time-base.md` correction); pin the composite v2 literals first in the plan that bumps to v3; implement the COMPLETE sixteen-event frame table and cursor in the register plan so no later plan moves a display-off golden again; flip every STIC region to the real device; give the video channel a field in the existing per-call sink so the 34 `ivs_psg_sink_init` test sites keep compiling; change `ex_plan_buffers` in the framebuffer plan, not the runner plan; keep the new call `ivs_machine_stic_state` in its own small plan because it touches eight registries.

## Architectural Responsibility Map

| Capability | Primary Tier | Secondary Tier | Rationale |
|------------|-------------|----------------|-----------|
| STIC register file, masks, power-on state | Profile data (`data/profiles`) plus a generator (`cmake/IvsSticTable.cmake`, new) feeding `core/src/stic/` | `core/src/memmap/` (device kind `stic14`, backing, pure read) | Same split as `psg.regmap` and `ivs_psg_params`; the memory system stays pure and clock-free so `ivs_machine_peek` equals a CPU read minus effects |
| Access windows, handshake, blocked counters, mode switch | Machine bus wrapper (`machine_bus.c`) over the STIC engine | Gate flags in `ivs_memsys` (derived cache, digest MEM) | The wrapper is the only place with unit start, offset and the sink (Phase 5 Pattern 1); effects never enter `ivs_memsys_read` (D-04) |
| Frame timeline, INTRM/BUSRQ line levels, row latches | New `core/src/stic/` TUs (events, lines) owned by the machine, parameters passed explicitly | Step seam (`machine_run.c`) calls the line function and the catch-up | Lines are pure functions of (state, tick); the CPU stepper is unchanged and the grant/hold/release sequence is its own |
| Card to pixel rendering | Pure function in `core/src/stic/stic_cards.c` | Dedicated memsys accessors (backing, no gates) | No state, no I/O; renders at `E_k` from registers, GRAM/GROM and the row latches |
| Frame slot, chain, frame stop, capacity | `machine_bound.c` run ladder and loop, `machine.c` env/info | Per-call sink (stack struct, no pointer kept) | Slicing invariance requires the chain to be per frame and the sink to hold no instance state |
| Inspection | `machine_inspect.c` (`ivs_machine_stic_state`, const) | `ivs_machine_peek` unchanged | Stored registers, mode, latch, counters, frame cursor are what the bus view cannot show |
| Report, IVSS grammar, goldens, operator outputs | `runner/` library over the public API only | `tools/check/steps/golden.sh` (second copy of the key list), operator helper outside the core | Runner is the first consumer; key lists exist in six places today (mechanism 12) |
| Rule/literal/frame consistency | Pure gate step `video` over text files (register literal, geometry, cards, frames) | In-process suites `stic`, `video` | Pure steps cannot read build outputs; cross-checks against generated tables belong to suites |
| Installed-package proof | `pkg-install` + `tests/installed-consumer/` | runner golden agreement | Growth of request, report and info can only be wrong in the installed header |

## Standard Stack

No external package is installed by this phase; the Package Legitimacy Audit is not applicable and `gsd-tools query package-legitimacy check` was not run (nothing to check).

| Tool | Local (this host) | Hosted ubuntu-24.04 | Note |
|------|-------------------|---------------------|------|
| CMake | 4.4.3 [VERIFIED: `cmake --version`] | 3.28.3 [CITED: `.planning/CI-EFFICIENCY.md` Phase 3 section, as in Phase 5 research] | Floor 3.25; new generators use `file(STRINGS)`/`string(JSON)` only inside `cmake -P` generators, not in new gate steps |
| Ninja | 1.13.2 [VERIFIED: `ninja --version`] | 1.13.2 [CITED: 04-RESEARCH.md] | |
| C compiler | Apple clang 21.0.0 under both `cc` and `gcc` [VERIFIED: `cc --version`; `/usr/bin/gcc` exists] | gcc 13.3 [CITED: CI-EFFICIENCY.md] | No native gcc locally: gcc behaviour only through the container |
| sh / dash | macOS sh (bash 3.2 mode) and `/bin/dash` [VERIFIED: `sh --version`, `dash -c`] | dash | New shell must be dash-safe, no `[[`, no `pushd`; the c-faults watchdog lesson (polled marker, never a blocking FIFO) applies to any new waiting construct |
| docker | daemon up, Docker 29.5.2, image `ubuntu:24.04` present [VERIFIED: `docker version`, `docker image ls`] | n/a | `sh tools/gate-linux.sh`, recorded 151 s total, 3 to 5 minutes under amd64 emulation [CITED: CI-EFFICIENCY.md Phase 5 checkpoint 2] |
| PDF tools | `pdftotext` 26.04.0, `pdftoppm`, `tesseract` 5.5.0, `qpdf` present; `mutool` absent [VERIFIED: `pdftotext -v`, `tesseract --version`, `which`] | n/a | For plan 06-01 page reading; the text layer of pdf 191 prints the AY-3-8900 feature list, 159 x 96, 20H x 12V and the 20th column of 7 [VERIFIED: `pdftotext -f 191 -l 193 -layout refs/SRC-gi-1980-catalog.pdf`] |
| Cache | `refs/` holds `SRC-gi-1980-catalog.pdf`, `SRC-gi-1982-catalog.pdf`, `SRC-gi-cp1600-um-1975.pdf`, `pages/`, `ctrl/`, `psg/`; the 1980 catalog's sha256 starts `70094d12e9512edf`, equal to the `data/sources.json` entry [VERIFIED: `shasum -a 256`, `grep`] | n/a | Unlike Phase 5 (which had to fetch the catalogs), both catalogs are already cached and hash-registered: plan 06-01 needs no network |
| python3, shellcheck | 3.14.4, 0.11.0 [VERIFIED] | n/a | Throwaway authoring and probes only; no python in any gate step (the gate is sh and cmake) |

**Version verification:** every row is from a command run this session or a recorded hosted run; no registry package is involved.

## Architecture Patterns

### System flow (conceptual)

```
IVSS scenario (inert text: input, run-to, capture video, assert frame/pixel/hash) --> runner exec (slices <= 1<<22 ticks, STOP_ON_HALT always)
   |  per slice: ivs_machine_run(target, events suffix, video+audio+psg_log buffers from one 8 MiB block)
   v
ladder (stages 1..4b/4c..9, refuse before any state change)  [flag 2: effective target T' = min(target, E_next) before stage 7]
   v
loop while time < target:                                      [machine_bound.c:556]
   exit tests -> apply due input events -> STEP SEAM (machine_run.c):
        lines = stic_lines(params, stic_state, start - lag*tpc)      (was: memset 0, machine_run.c:150)
        ivs_cp1610_step(cpu, cfg, lines, port = ivs_machine_bus, ctx{unit start, tpc, sink})
            CPU access (offset_cpu_cycles) -> wrapper:
               class = hook(addr, view)  [PSG | STIC regs | GROM/GRAM | BACKTAB write | none]
               STIC class: stic_catch_up(stamp) -> gates/latch/frame state current; effect ($0020 strobe, $0021 read switch);
                           blocked? count; then ivs_memsys_bus (the only store)
        time += elapsed * tpc ; psg_catch_up(now) ; stic_catch_up(now)  [events <= now in tick order; FRAME_END renders+delivers a slot]
   v
report: stop reason (INTERNAL, GUEST_FAULT, CPU_HALTED, FRAME, TARGET), video_frames_completed, video_bytes_written,
        video_first_index, last/next_frame_end_ticks, audio and log counters
   v
runner: per slot re-hash of the video chain (video.chain.check), pixel/hash asserts, ivsr 3, golden (key list x2)
```

### Mechanisms Phase 6 must extend (read this session; the planner edits exactly these)

| # | Mechanism | Where (file:line) | Change required | Plan |
|---|-----------|-------------------|-----------------|------|
| 1 | Machine bus wrapper: only a WRITE or DW that the WRITE view decodes to the PSG phys is hooked (`ivs_memsys_describe(.., IVS_MEMSYS_VIEW_W, ..)` then `d.phys == IVS_MEMSYS_PHYS_PSG && c->sink != NULL`); every other access goes straight to `ivs_memsys_bus`; stamp = `unit_start_tick + offset * ticks_per_cpu_cycle + psg->write_strobe_ticks` | machine_bus.c:27-65 (describe at :34-35, stamp at :37-38) | Add a STIC hook class for ALL access kinds (reads and fetches have gate and mode-switch effects): catch the STIC up to the access stamp, evaluate the gate, count a blocked access, apply the `$0020` strobe and the `$0021` read switch, then call `ivs_memsys_bus`. Detect the class in O(1) from a generated per-16-word granule table (read and write views), not by `ivs_memsys_describe` (it scans every region for the alias test, `ivs_mm_is_alias`, memsys.c:ivs_mm_is_alias loop); a read of `$4021`/`$8021`/`$C021` decodes to the CARTRIDGE in the read view, so the mode-switch read hook uses the WRITE view's decode (or an explicit list). BACKTAB writes (sysram `$0200-$02EF`, DW included) also catch up first | 06-04b, 06-05 |
| 2 | Step seam: lines hard-wired 0 (`memset(&lines, 0, sizeof lines);`), CPU step with the wrapper context, time advance, PSG catch-up to the new machine time at every unit end | machine_run.c:150, :167, :183-185 (header comment :21-22 "inactive lines") | Replace the memset by `ivs_stic_lines(params, &im->stic, start)` (sample tick `start - 4 * lag`, clamped at 0); add `ivs_stic_catch_up(.., im->time_ticks, sink)` AFTER the PSG catch-up (fixed order PSG then STIC); extend `ivs_run_power_on` (machine_run.c:44-77) with the STIC state, `stic_reg` zero, gates open, video chain h0; extend the `ivs_machine_bus_ctx` (machine.h:199) with the STIC params pointer | 06-04b, 06-05 |
| 3 | Memory system: gates `IVS_MEMSYS_GATE_NONE 0u`, `_STIC_REGS 1u`, `_GMEM 2u`, `_GATES 3u`; device kinds end at `IVS_MEMSYS_DEVICE_STUB 5u`, PSG8 = 6, `IVS_MEMSYS_DEVICE_KINDS 7u`; `ivs_memsys` is 113,264 bytes with `psg_reg[16]`, `port_pulldown[2]`, `psg_reserved[2]` last; reads are `const` and pure; `ivs_mm_load`/`ivs_mm_store` default case "stub devices hold no state" | memsys.h:54-62, :92-111 (struct), :113 `_Static_assert(sizeof(ivs_memsys) == 113264u, "ivs_memsys must be 113,264 bytes (machine.h layout)");`; memsys.c ivs_mm_load/ivs_mm_store switch | Add `IVS_MEMSYS_DEVICE_STIC14 7u` (KINDS 8); `uint16_t stic_reg[64]` (+128 bytes -> 113,392, still a multiple of 16); load/store cases for phys STIC (per-register masks from the generated table, as `ivs_psg_reg_load`/`_store`); raw-poke case; dedicated renderer accessors reading backing with no gate and no `*_loaded` test (`ivs_memsys_backtab_word_internal`, `_gram_byte_`, `_grom_byte_`); a gate setter already exists (`ivs_memsys_set_gate_internal`, memsys.h:191) | 06-04a (rows), 06-04b (device) |
| 4 | Generator rules: `set(MM_DEVICES undriven rom10 grom8 ram8 ram16 stub psg8)`, `set(MM_STUB_PHYS stic)`, a stub device needs "impl stub and a stub physical device (stic)", a backed device needs impl real and a physical device NOT in `MM_STUB_PHYS`, and one physical device has one size and one device kind | cmake/IvsMemmapTable.cmake:155, :164, :479-492 | Make STIC a backed physical device (`MM_CAP_stic IVS_MEMSYS_STIC_WORDS`), remove it from `MM_STUB_PHYS`, add `stic14` to `MM_DEVICES`. Because phys is the region-name prefix, `stic_regs`, `stic_alias_read` and the three write aliases are ONE physical device: all must become `impl real` with device `stic14`, size 64 (the read alias mirrors the register file; a write-only register reads `$FFFF` through the device fill, so a peek of the alias is unchanged for write-only registers). `memmap.device.stub` then has no user (keep the record for the ECS stub) | 06-04b |
| 5 | Profile: 109 fields, none `video.*`, only `stic.part` ("AY-3-8900-1", tier1), `ticks_per_stic_clock` 1, `region_video` "NTSC", `scanlines_per_frame` 262, `cpu_cycles_per_scanline` 57, `bus.release_latency_cpu_cycles` 2, `bus.wait_limit_cpu_cycles` 35, `cpu.interrupt_accept_clears_intff` 0 (C41), `power_on_fill.stic` zero, `psg.min_write_gap_ticks` 28, `psg.max_unit_ticks` 468, `psg.log_records_per_frame` 2152; regions `memmap.region.stic_regs` (RW, `$0000-$003F`, device stub, gate stic-regs), `stic_alias_read` (R `$0040-$007F`), `stic_alias_w4/w8/wc` (W `$4000/$8000/$C000`) | data/profiles/intv-2609-ntsc-family-p1.json (listing of all 109 keys printed this session) | Add `stic.*` and `video.*` fields (names in the CONTEXT: `stic.reg_window_cpu_cycles` 2000, `stic.gmem_window_cpu_cycles` 3780, `stic.irq_low_cpu_cycles` 3791, `stic.bus.count` 13, `.first_start` 3791, `.pitch` 912, `.hold_cpu_cycles` 114, `stic.frame_phase_ticks` 0, `stic.access_strobe_ticks` 0, `stic.access_gating_when_off` apply, `stic.regmap` (64 rows), `stic.backtab` span, `bus.line_sample_lag_cpu_cycles` 1, `video.last_card_column_px` 7, `video.display_off_fill`, power-on mode), each with tag, cite, conflict id from the 06-01 allocation table; change the five STIC regions and add `memmap.device.stic14`; `profile-fields.ids`. Fields are added by the plan whose generator consumes them | 06-03b, 06-04b |
| 6 | Fourth generated table is NOT tied to the registry yet: `ivs_machine_profile_covered` is the smallest of three counts | machine.c:50-54 `return (uint32_t)(index < ivs_profile_count() && index < ivs_psg_params_count() && index < ivs_input_profile_count());` | Add `ivs_stic_params_count()` as a fourth term and extend `machine.create.tables-cover-registry` (Phase 5 review WR-04); named fault `STIC_TABLE_SHORT` | 06-03b |
| 7 | Instance layout: `IVS_MACHINE_BYTES 113760u`; mem at 56, psg at 113,320 (72 bytes), obs at 113,392 (200 bytes), slots at 113,592; asserts `offsetof(struct ivs_machine, obs) == 113392u`, `sizeof == IVS_MACHINE_BYTES`; the digest tail assert `IVS_MACHINE_SLOTS_END == sizeof` | machine.h:48, :14-37 (layout comment), :115-119 (offset and size asserts); machine_digest.c:`IVS_MACHINE_SLOTS_END` | Derive the layout on paper first (below): candidate `stic` state of 528 bytes between `psg` and `obs`, instance 114,416. The STIC rows go BEFORE `slot[]` (a named fault skips every row at or after `slot`) | 06-04a |
| 8 | Digest table: rows in instance order; `mem.gate_open` is a 4-byte MEM row; the VIDEO domain rows are only `obs.video.chain` (32 bytes) and `obs.video.count` (8); composite is `IVS_DIGEST_COMPOSITE_VERSION 2u` over domains 1..4; `ivs_digest_domain_known` already accepts 16..19 | machine_digest.c:82, :109-110, :239 (`ivs_digest_domain_known`), `comp_version` in `ivs_machine_state_digest`; machine.h:141 | `mem.stic_reg` (MEM), `stic.*` rows (INSTANCE), keep VIDEO rows (the chain value changes, the row set does not); version 3; hand-framed domain-2 and domain-4 literal streams extended in `tests/machine/machine_digest.c`; a `IVS_DF_STIC_ROWS` omission macro for the named fault | 06-04a |
| 9 | Video observation row is zero at create and reset except nothing; the INPUT row is initialised by `ivs_input_init` at create and reset | machine.c:251, machine_run.c:75, machine_input.c:95 | Initialise `obs.video.chain` to `SHA-256("IVS-VIDEO-LOG-V1")` at BOTH create and reset (same two call sites; a fresh instance and a reset instance must agree) | 06-06 |
| 10 | Per-call sink is `ivs_psg_sink` (audio buf/cap/written/samples/overflow, log channel, fresh flag), initialised by `ivs_psg_sink_init` at 34 test call sites and by the run loop; the seam takes `ivs_psg_sink *sink` and the bus ctx carries it | psg.h:`ivs_psg_sink`, machine.h (`ivs_machine_step_unit_sink_internal` declaration), machine_bound.c:531-542 (sink init) and :594 (the call) | Add the video channel (buf, cap, written, frames, overflow, header state) to the SAME struct, zero meaning the counting channel, so no existing call site changes (it is a psg-named struct carrying a second device's channel; a rename is churn for no gain, record it in the header comment) | 06-06 |
| 11 | Run ladder: flags `if ((req->flags & ~IVS_RUN_F_STOP_ON_HALT) != 0u)` refuses flag 2; stage 7 capacity `ivs_bound_capacity_reason(req, env, req->target_ticks, im->time_ticks)`; loop `while (im->time_ticks < plan.target)`; stop is `ivs_bound_stop(im, plan.flags, plan.target)`; report assigned `r.audio_bytes_written = sink.written;` and never `video_*`; `memcpy(rep, &r, n)` | machine_bound.c:399, :437, :556, :639, :650, :668; recheck for the fault `RUN_CAPACITY_AFTER_STEP` :623 | Accept bit 2; compute `T' = min(target, E_next)` once after stage 6 and use it for stage 7 (a flagged call with `IVS_RUN_TARGET_MAX` must not demand astronomical capacity) and for the loop; stop order INTERNAL, GUEST_FAULT, CPU_HALTED, FRAME, TARGET with `IVS_STOP_FRAME` (name in `ivs_run_stop_name`, machine_bound.c:689-696 area); fill `video_*` fields; the three new report fields | 06-03a (constants only), 06-06 (behaviour) |
| 12 | Public constants and structs: `IVS_RUN_F_STOP_ON_HALT 1u`, `IVS_RUN_F_RESERVED_FRAME 2u /* reserved for a frame stop; UNSUPPORTED until Phase 6 */`, stops 1..4, `IVS_RUN_REPORT_V1_SIZE 112u`, `video_frames_completed`/`video_bytes_written` at 267-268 (offsets 56 and 64), `psg_log_first_index` last at offset 128 (`sizeof 136`); `IVS_OUTPUT_FORMAT_NONE 0u`, `_PSG_LEVELS_V1 1u`; `IVS_INFO_V1_SIZE 216u`; info `video_format` and `video_bytes_per_frame` | ivs_run.h:91-92, :103-106, :238, :267-268, :277; ivs_machine.h:110-111, :116, :162-164 | Rename the flag to `IVS_RUN_F_STOP_ON_FRAME 2u` (still refused until 06-06), append `IVS_STOP_FRAME 5u` (pin line in `api-stop-reasons.ids`), `IVS_OUTPUT_FORMAT_INDEX8_FRAMES_V1 2u` (not a pinned class), report v3 fields at 136/144/152 (sizeof 160), `ivs_info` v4 tail from 288; the `ivs_run.h` capacity paragraph (lines 108-137 area) must be amended for video (`+2`), and `nominal_frame_ticks` is now a frame event length, not "nominal" | 06-03a |
| 13 | Env and info producers | machine.h:250-263 `ivs_run_env` (video_format, video_bytes_per_frame, audio_format, audio_bytes_per_frame, u64 nominal_frame_ticks, psg_log_bytes_per_frame, `uint32_t reserved_env; /* 0 */`); machine.c:339-376 `ivs_machine_profile_env` (`env->video_format = IVS_OUTPUT_FORMAT_NONE;`), :303-330 info assignments; static asserts machine.c:30-45 | `video_slack_frames` takes the `reserved_env` slot (no other reader: `grep reserved_env` finds only the declaration), sizeof stays 32; video layout real in 06-06 (format 2, bytes per frame = slot bytes 15,296, slack 1); info v4 filled from profile; asserts extended | 06-03a (zero), 06-06 (real) |
| 14 | New public const call: `ivs_machine_cpu_state` and `ivs_machine_peek` live in machine_inspect.c and declare in ivs_run.h; the effect table lists 14 calls; the installed consumer's undefined-symbol list must equal `data/expected/pkg-consumer-calls.ids` exactly | machine_inspect.c:57-118; data/api/api-effects.json (calls array: ivs_api_version ... ivs_run_stop_name); data/expected/pkg-consumer-calls.ids; tests/installed-consumer/main.c; gate counters `api-calls=14`, `public-symbols=14` | `ivs_machine_stic_state(const ivs_machine *, ivs_stic_report *)` with the `cpu_state` refusal ladder; effect row with a non-skip test id; consumer reference; counters 15; `ivs_stic_report` pinned size | 06-04c |
| 15 | Runner: report keys exist for `digest.video` and `output.video.bytes` (report.c:117, :121); `ivsr 2` first line; golden key list in `main_golden_keep` (live domain flags `MAIN_LIVE 4`, video keys `digest.video` only) and in golden.sh (:26-43, :115-123, :153-177, :264-274); buffers: `ex_plan_buffers` `per[0] = info->video_bytes_per_frame;` times `slots = slice_ticks / nominal_frame_ticks + 1`; `q.flags = IVS_RUN_F_STOP_ON_HALT;` always; `x->rec->video_bytes += rep.video_bytes_written;`; main.c allocates and passes the 8 MiB block | runner/report.c:117-121; runner/main.c:363-447, :674, :743-744; runner/exec.c:694-707, :824, :853; tools/check/steps/golden.sh; tests/runner-process-tap.sh:204; toy reports under tools/check/controls (7 files) | `ex_plan_buffers`: video need = slot bytes x (slice/F + 1 + slack) BEFORE the core publishes a real video layout (06-06); everything else (new keys, `ivsr 3`, capture, chain re-hash, operator output) in 06-09 across ALL key-list places: report.c table, ivs_runner.h (:36-100 grammar comment, `digest[9][32]` :492), main.c golden list, golden.sh, docs/runner.md, tests/runner/runner_exec.c, runner-process-tap.sh, the 7 toy reports | 06-06 (sizing), 06-09 |
| 16 | Tests that assert the old layout or stub: `run.refuse.flags-frame-reserved` (run_bound.c:943, :1222), the reserved flag used as a generic bad flag at run_bound.c:1085, :1109, :1146, run_input.c:876, run_buffers.c:757; `tests/run/run_info.c:106` (`a.video_format == IVS_OUTPUT_FORMAT_NONE && a.video_bytes_per_frame == 0u`); `ro_peek_undriven` expects fill at `$0000-$0001` and a 64-word window at `$0070` (run_observe.c:576-590); memmap rows `{"stic", 0u, 0x0000u, 0xFFFFu, 0u}` (memmap_rows.c:282) and the descriptor check (memmap_main.c:541); installed consumer prints `video_format=%u` (main.c:391-394) so `pkg-consumer.golden` moves when video becomes real | as listed | Keep the six bad-flag call sites meaningful by moving them to an unused bit (`4u`), invert the one dedicated test in 06-06; update the info/peek/memmap tests in the plans that flip the layout and the stub; each one is a reviewed change named in the SUMMARY | 06-04b, 06-06 |
| 17 | Fault framework and registries: fault TUs unique by stem across core and runner; `IVS_CORE_SOURCES` list; `ivs_core_gen` waits for every generated include; `core-generated.ids` | tests/faults.cmake:18-57; core/CMakeLists.txt:12-24, :34-81 | New TUs `core/src/stic/stic_params.c` (single includer of `gen/ivs_stic_params.inc`), `stic_regs.c`, `stic_events.c`, `stic_lines.c`, `stic_cards.c`, `stic_frame.c` (stems unique, listed, each small so a fault recompiles one file); a fourth generator rule and `core-generated.ids` line; `fault-orphan-site`: a site is registered in the commit that adds it | 06-03b onward |
| 18 | Gate registries that name requirements or counts | tools/check/steps/important-gates.sh:70 (`split("API-02 API-04 ... AUD-01 AUD-02", reqs, " ")`); data/expected/important-gates.ids; gate-steps; controls; tests; faults; `docs/` | Add `VID-01 VID-02 VID-03 VID-04 VID-06` to the step's list and its three "uncovered, unknown id, bad requirement" controls (as plan 05-12 did for Phase 5); one new gate step `video` with its controls and toy tree | 06-01 (step), 06-11 (registry) |

### Derived constants (executed and computed this session)

| Quantity | Value | Basis |
|----------|-------|-------|
| Frame | `F = 4 x 57 x 262 = 59,736` ticks = 14,934 cycles; 59.9227 Hz at 3,579,545 Hz | [VERIFIED: executed arithmetic; profile fields `ticks_per_cpu_cycle` 4, `cpu_cycles_per_scanline` 57, `scanlines_per_frame` 262] |
| Per-frame event table (cycles from `S_k`; ticks = 4x) | FRAME_START 0; REG_CLOSE 2,000 (8,000 ticks); GMEM_CLOSE 3,780 (15,120); INTRM level ends 3,791 (15,164); window i starts `3,791 + 912 i`; ROW_FETCH(i) at `+57` (cycle 3,848 + 912 i, ticks 15,392 + 3,648 i), i = 0..11; window i ends `+114`; window 12 spans 14,735..14,849 (58,940..59,396 ticks); FRAME_END 14,934 (59,736). The last window ends 85 cycles before the frame end; a 14th regular window would start at 15,647, outside the frame | [VERIFIED: executed arithmetic from CONTEXT D-07..D-09 values] |
| Closed form of one stall | Window start `w`, hold `H`, release latency `L = 2`: the CPU's next unit starts at `w + 4 (H + 1 + L) = w + 468` ticks (117 cycles) whatever boundary granted; executed against the real stepper with the sampling rule `level(start - 4)`: "grant at w+12 ticks, resume at w+468 ticks" in every window of every frame (first window w+24) | [VERIFIED: executed throwaway harness driving `ivs_cp1610_step`] |
| Line predicates for the oracle | INTRM seen at a boundary starting at `b` iff `S_k + 4 <= b <= S_k + 4 x 3,791`; BUSRQ seen iff `w + 4 <= b < w + 4 H + 4` (sampling lag 1 cycle) | [VERIFIED: derived from the sampling rule and checked by the harness windows] |
| Executed instruction cycles per display-on frame (steady state, real stepper, display strobed every frame) | uniform NOP or INCR (6 cycles): 13,452 executed + 1,482 in grant units = 14,934, 13 windows granted, 3,711 units; MVII (8): 13,480 + 1,454, 3,126 units; MVI (10): 13,480 + 1,454, 2,789 units; J (12): 13,524 + 1,410, 2,524 units; display off: 14,934, 2,489 units (NOP). Display-on units are +49 percent | [VERIFIED: executed harness]. CONTEXT's "about 13,518" is inside the 13,452..13,524 spread (0.5 percent); the usable count is an OUTPUT of the guest's instruction mix, never a constant |
| Non-interruptible streams | A stream of MVO (11 cycles, `interruptible` 0): 14,927..14,938 executed, 0 grant cycles, 0 windows granted with the display on; the CPU is neither stalled nor interrupted. EIS, DIS, MVO family, shifts, SDBD, TCI, CLRC, SETC, HLT are non-interruptible in the decode table | [VERIFIED: executed harness; core/src/cpu/cp1610_decode.c:36-38 `IVS_ROW("SDBD", ... 0u ...)`, `IVS_ROW("EIS", 0x3FFu, 0x002u, 0u, 1u, 4u, 0u, IVS_CP1610_OP_EIS)`, `IVS_ROW("DIS", ...)`, `IVS_ROW("MVO", 0x3F8u, 0x240u, 0u, 2u, 11u, 0u, IVS_CP1610_OP_MVO)`] |
| INTRM re-entry with C41 = 0 | NOP loop with INTFF held set: 253 interrupt entries per frame (INTRM level 3,791 cycles / (9-cycle entry + 6-cycle instruction)); display on or off. `cpu.interrupt_accept_clears_intff` is 0 | [VERIFIED: executed harness; profile `cpu.interrupt_accept_clears_intff` value 0] |
| U and g under a driven BUSRQ | Grant, hold and release units are at most 2 cycles (8 ticks), the interrupt entry is 9 cycles (36 ticks, `IVS_INT_ENTRY_CYCLES 9u`, cp1610.c:60); both far below `U = 468`. A stall only lengthens the time between two writes, so `g = 28` (the minimum gap) and `W_log = ceil(F/g) + ceil(U/g) + 1 = 2,134 + 17 + 1 = 2,152` (profile `psg.log_records_per_frame`) still bound; densest reachable writer (MVO-indirect to a PSG register, non-interruptible, never stalled): 1,659 writes per frame with the display on AND off | [VERIFIED: executed harness counting WRITE accesses to `$01F0-$01FF` per frame by access stamp; profile fields] |
| Video slot | header 32 + 159 x 96 = 15,296 bytes; at 160 columns 15,392 | [VERIFIED: arithmetic] |
| Capacity corner of the `+1` law | A call from `now` to `target` completes frames whose `E_k` lie in `(now, end]`; with `r = now mod F`, `d = target - now`, overshoot `ov < U`, the count can reach `floor(d/F) + 2` when `r + (d mod F) + ov >= 2F`. Concrete: `now = F - 4`, `d = 2F - 1`, `ov = 44` ticks (a 11-cycle unit) completes 3 frames against `d/F + 1 = 2`. Random sampling (400,000 trials) found 0 violations at U = 48 and 4 at U = 468, so the CONTEXT's "61 of 400,000" is a property of its own sampler; the corner exists and is narrow, which is why a property test must construct it, not sample it | [VERIFIED: executed arithmetic and sampler] |
| Runner per-slice buffer total with `slot x (slice/F + 2)` | slice `1 << 22` = 4,194,304 ticks: video 72 x 15,296 = 1,101,312; audio 71 x 15,060 = 1,069,260; log 71 x 34,432 = 2,444,672; total 4,615,244 bytes (55 percent of `IVS_RUNNER_BUFFERS_MAX` = 8 MiB) | [VERIFIED: arithmetic; ivs_runner.h `IVS_RUNNER_BUFFERS_MAX (8u * 1024u * 1024u)`] |
| Struct arithmetic (64-bit pointers; every new member is u32 or u64, so identical on 32-bit) | `ivs_run_report` v3: offsets 136, 144, 152 (`video_first_index`, `last_frame_end_ticks`, `next_frame_end_ticks`), sizeof 160. `ivs_info` v4 candidate: 288 `frame_phase_ticks` u64, 296 `video_width`, 300 `video_height`, 304 `video_slot_bytes`, 308 `video_header_bytes`, 312 `video_slack_frames`, 316 `video_row_lines`, sizeof 320 (the minimal list without the last two is 312; the frame note's 152/312 figures disagree with CONTEXT's three u64 = 160). `ivs_run_env` 32 bytes with the slack in `reserved_env`. `ivs_memsys` 113,264 + 128 = 113,392. Candidate STIC state: u64 frame_index, u32 event_index, display_latch, windows_active, mode, cs_ptr, blocked_reads, blocked_writes, late_grants, `uint16_t row_latch[12][20]` (480), `uint32_t reserved[2]` = 528 bytes, 8-aligned. Instance = 24 + 32 + 113,392 + 72 + 528 + 200 + 168 = 114,416 = 16 x 7,151; `stic` starts at 113,520 (0 mod 8) so `obs` stays 8-aligned | [VERIFIED: layout harness with static asserts for all of these, compiled clean] |
| Cost anchors | one frame slot of SHA-256 (15,296 bytes): 101.4 microseconds (`-O2`), 126.3 (`-O1` with ASan and UBSan); stepping 9.3 million units per second release (14,043 emulated frames in 2.6 s); `ivs_memsys_describe` 3.0 nanoseconds per call release | [VERIFIED: executed micro-benchmarks against the release archive] |

### CONTEXT assumption check (against the code)

| CONTEXT or note statement | Verdict | Evidence |
|---------------------------|---------|----------|
| `lines.intrm` and `lines.busrq` are hard-wired 0 in machine_run.c | TRUE | machine_run.c:150 `memset(&lines, 0, sizeof lines);`; header :21-22 "inactive lines (Phase 3 has no device that drives INTR, INTRM, BUSRQ or STPST)" |
| The stepper's BUS_GRANT and interrupt-entry mechanics behave as T-2/T-3 describe | TRUE, with one consequence the CONTEXT underplays | cp1610.c:1148-1175 (conventions comment), `ivs_step_running` (:1271, grant test :1291) (grant only at an OPEN boundary, `grant = open && l->busrq != 0u`), `ivs_unit_grant` (cp1610.c:1247; 1 cycle; release `latency` = `cfg->release_latency_cpu_cycles`), executed: resume at `w + 468`; a closed boundary defers or drops the grant (nothing is latched: `IVS_FAULT_LINES_LATCHED` is a fault, cp1610.c:1200). Consequence: guests made only of non-interruptible rows are never stalled and never interrupted |
| Line sampling lag is a machine-layer rule | TRUE that it is NOT implemented | no `line_sample_lag` anywhere in code or profile; POL-LINE-01 exists only as text (data/discrepancies.json:380, data/cp1610/vectors/interrupt.json); `bus.line_sample_lag_cpu_cycles` is a new profile field and desc member |
| `IVS_MEMSYS_GATE_STIC_REGS` and `_GMEM` exist and are in the digest as `mem.gate_open` | TRUE | memsys.h:60-61; machine_digest.c:82 `IVS_DF("mem.gate_open", mem.gate_open, 1u, 4u, IVS_DIGEST_MEM, IVS_DIGEST_KIND_UINT)`; both opened by `ivs_memsys_init` (memsys.c:80-81) and by power-on (machine_run.c:62-63) |
| How the STIC stub behaves today | TRUE: reads `$FFFF`, writes dropped | profile `memmap.device.stub` value `{"bits": 0, "rmask": "$0000", "fill": "$FFFF", "store": "drop"}`; `ivs_mm_load` default "stub devices hold no state"; a closed gate reads the undriven fill (`$FFFF`); peek is `ivs_memsys_read` so `$0000-$007F` peeks `$FFFF` (run_observe.c:579 asserts it) |
| `ivs_machine_peek` for the STIC region | TRUE: gates honoured, no log, no time | machine_inspect.c:96 `uint16_t v = ivs_memsys_read(&im->mem, a);`; the only mutation in the file sits inside `IVS_FAULT_PEEK_*` blocks |
| The memory map has BACKTAB, GRAM and GROM regions | PARTIAL | No BACKTAB region: it is `sysram[0..239]` (`memmap.region.sysram` `$0200-$035F`, ram16, 16-bit words kept whole). GRAM region `$3800-$3FFF` RW with write aliases `$7800/$B800/$F800`, device ram8 (`rmask $00FF`, keeps value and `$00FF`), backing `uint16_t gram[512]`, gate gmem, 4 mirrors. GROM region `$3000-$37FF` read-only, `grom[2048]` byte-valued words, reads the undriven fill until an image is loaded (`ivs_mm_present`). Raw poke seam: `ivs_memsys_poke_raw_internal(ivs_memsys *m, uint16_t addr, uint16_t value)` (memsys.h:215) |
| Existing video layout fields in profile and `ivs_info` | `ivs_info`: TRUE; profile: none | `ivs_info` has `video_format` and `video_bytes_per_frame` (ivs_machine.h:162,164, offsets 228 and 240) and the nominal frame; the profile has no `video.*` or `stic.*` field besides `stic.part` and `ticks_per_stic_clock` (all 109 keys listed this session) |
| `video_frames_completed` and `video_bytes_written` are wired | PARTIAL | fields exist (ivs_run.h:267-268) and the runner already accumulates `rep.video_bytes_written` (exec.c:853) but the core never assigns them (machine_bound.c:637-650 sets `audio_*`, `psg_log_*`, input fields from a zeroed `r`) |
| `IVS_RUN_F_RESERVED_FRAME` exists and "the Phase 4 test that refused it is inverted" | TRUE but UNDERSTATED | ivs_run.h:92; refused at machine_bound.c:399; ONE dedicated test `run.refuse.flags-frame-reserved` (run_bound.c:943, registered :1222, data/expected/tests/run.ids:61) plus five uses as a generic unsupported flag (run_bound.c:1085, :1109, :1146; run_input.c:876; run_buffers.c:757) and the effect-table text (api-effects.json:396, :429) |
| D-15: "the Phase 4 `+1` formula is violated in a corner (61 of 400,000 runs)" | TRUE (corner real), figure not reproduced | see derived constants: constructed case violates it with a 44-tick overshoot |
| D-10: "the Phase 4/5 audio and log capacity numbers and goldens stay bit-identical" | TRUE for capacities and for `digest.audio`, `digest.psglog`, `digest.cpu`, `digest.images`, `digest.input`, `step.*`, `cpu.*` of the four `.digests` goldens; `digest.mem`, `digest.instance`, `digest.composite` move (D-16 says so) | all four `.digests` end below the first frame end (`step.1.now_ticks` 2,084, 16,752, 2,084 and 1,020/2,084, 404 versus `F = 59,736`) with `video=none` and `output.video.bytes 0`, so `digest.video` is not in them; the longest (diag-ctrl-psg-pass, 16,752) ends after both windows close (8,000 and 15,120 ticks), so its `mem.gate_open` bytes change |
| D-14: frame content "fixed by events <= E_k" | AMBIGUOUS against the PSG convention | the PSG rule is catch-up inclusive of the stamp, then the write (psg.h `ivs_psg_write` comment, machine_bus.c:7-11): a write stamped EXACTLY `E_k` lands in frame k+1. State it as "writes stamped below `E_k`" and test E-1, E, E+1 |
| D-11: "12 row latches (13 x 20 words state, about 520 bytes)" | INCONSISTENT | 12 rows x 20 words = 240 words = 480 bytes; 13 x 20 = 260 words = 520 bytes. Window 12 fetches nothing (D-09), so 12 latches; the candidate layout above uses 480 |
| Regs note: "the read alias `$0040-$007F` stays `impl=stub`"; D-04 "region flip from stub to real in the device plan" | FALSE as a pair | IvsMemmapTable.cmake:479-492: a stub device needs a stub physical device and a backed device a non-stub one; the phys is the name prefix, so a half-real STIC cannot be generated. Flip all five STIC regions (recommended) |
| Regs note: `armed/active/copy` state and the INTAK/BUSAK copy-mode machine | SUPERSEDED | D-05/D-06 keep only the display latch and pure windows; the state struct is the one derived above |
| Hooks "derived from the memory system's own decode" (timing T-5.3) | INCOMPLETE for alias READS | `$4021` etc. read-decode to the cartridge span (memmap literal `R 4000-FFFF cart`), the STIC alias exists only in the WRITE view (`W 4000-403F stic`): use the write-view decode for the mode-switch read hook |
| Frame note: publish `video_bytes_per_frame = 2 x slot` and keep the capacity law | SUPERSEDED by D-15 | env member `video_slack_frames`, law `slot x (D/F + 2)`; `ivs_info` v4 must publish the slack (or document `+2`) so a host and the runner size a buffer from `ivs_info` alone |
| Verify note V6-9: "the setup needs no interrupt and no cycle counting" | FALSE under D-05/D-06 | windows apply with the display off and the latch is cleared each frame: the strobe must land within 2,000 cycles of tick 0 and GRAM writes within 3,780; the runner halts at HLT (exec.c:824) so the guest must run past cycle 14,934 plus stalls before HLT |
| "Runner changes belong to 06-09" (D-23) | FALSE for buffer sizing | exec.c:694-707 plus main.c:674, 743-744: the runner already requests every channel with a nonzero layout; see Pitfall 7 |
| "`ivs_machine_stic_state` joins the api-effects registry" | TRUE and costs eight files | 14 calls today (`api-calls=14` in the baseline gate line), installed consumer `nm -u` list equality, `public-symbols=14` |
| Phase 4/5 fixtures use no EIS and never strobe the display, so their unit sequence is identical | TRUE | no EIS decle in any fixture listing (`grep ' EIS'` over fixtures/*/*.listing.txt is empty; the boot stub holds DIS, listing line 66) and no test program in tests/machine/machine_standin.c uses EIS; with INTFF 0 the INTRM level is never accepted and with the latch clear no BUSRQ window exists |

### Recommended structure (additions)

```
core/src/stic/stic_params.h      explicit ivs_stic_params struct (one per profile), passed to every function
core/src/stic/stic_params.c      single includer of gen/ivs_stic_params.inc; ivs_stic_params_get / _count
core/src/stic/stic_regs.c        register load/store masks from the generated regmap; effects table
core/src/stic/stic_events.c      16-event frame table, cursor, catch-up (gates, latch clear, row latches, FRAME_END)
core/src/stic/stic_lines.c       ivs_stic_lines(params, state, tick): INTRM and BUSRQ levels, pure
core/src/stic/stic_cards.c       pure card renderer (registers, glyph source, row latches -> index plane, 20 cards wide, cropped)
core/src/stic/stic_frame.c       slot header, plane copy, chain update, sink delivery, display-off fill
core/src/machine/machine_bus.c   hook class + STIC catch-up and effects (extended, not a new file)
cmake/IvsSticTable.cmake         stic.* and video.* objects, regmap -> ivs_stic_params[profile]; fail-closed shape checks; -DMUTATE for GEN_ faults
data/stic/*.literal.txt data/stic/rules.json data/stic/vectors/*.txt data/video/{geometry,cards}.literal.txt data/video/frames/*.frame
data/expected/{video-frames,stic-rules,d10-measurements}.ids tests/{stic,video}/*.c tools/check/steps/video.sh
docs/stic/reference-notes.md docs/video/{frame-derivations,diag-stic-1-derivation}.md
fixtures/FX-diag-stic-1/{manifest.json,diag-stic-1.listing.txt} data/scenarios/diag-stic-*.ivss data/golden/diag-stic-*.{expect,digests}
runner/frame_export.[ch] (or a shell helper)   operator-side PPM export; see Open Question 3
```

### Pattern 1: line function and catch-up are pure of the host's slicing

**What:** `ivs_stic_lines(params, state, tick)` returns `{intrm, busrq}` as a pure function of the frame start and the stored latch decision; it never advances state. The step seam calls it with `tick = start - lag * ticks_per_cpu_cycle` (0 if negative). INTRM is `rel < irq_low * tpc` with `rel = tick - S_k`; BUSRQ is `windows_active && (rel - first_start*tpc) mod pitch < hold*tpc` for window index below `bus.count`. A tick before `S_k` (the sample point of the first unit of a frame) is in the PREVIOUS frame, where both levels are 0 (the last window ends 85 cycles early and INTRM ends at 3,791), so the function needs no previous-frame state. `windows_active` is decided at the event "window 0 start" from the latch (a write stamped exactly at window 0's start lands after the decision: step first, then act).
**Why:** the event cursor, not a modulo, is the state (D-07); lines are not events (timing T-5.2), so slicing cannot move them.
**Ordering rule:** per unit: lines from the STIC at the unit start; CPU step through the wrapper (each hooked access catches the STIC up to its own stamp first); time advances; PSG catch-up; STIC catch-up to the unit end.

### Pattern 2: hook class from a generated granule table, not `ivs_memsys_describe`

The wrapper runs on every access of every unit (3 to 4 per unit; 9.3 million units per second release). `ivs_memsys_describe` costs 3.0 ns release but scans all regions for the alias test; under ASan and UBSan it is an order of magnitude slower. Generate two 4096-entry granule tables (read view, write view, one class byte per 16 words) next to `ivs_mm_rd_region`/`ivs_mm_wr_region` (the same granule index, `addr >> 4`), classes NONE, PSG, STIC_REGS, GMEM, BACKTAB; the BACKTAB class is a sub-granule span (`$0200-$02EF` is 15 granules of 16 words, aligned), so granule resolution is exact. One lookup per access, no region scan. The existing PSG test (`describe` on WRITE/DW) can stay or migrate to the same table in the same plan.

### Pattern 3: complete event table from the start

Implement all 16 events per frame (FRAME_START, REG_CLOSE, GMEM_CLOSE, ROW_FETCH 0..11, FRAME_END) and the cursor semantics in the register plan (06-04b), with the row-fetch and frame-end handlers inert until 06-05 and 06-06 (a ROW_FETCH of a frame whose `windows_active` is 0 changes nothing). Then the `digest.instance` of every display-off golden stops moving after 06-04b, and later plans re-bless only goldens that exercise the display (the new STIC diagnostic goldens).

### Pattern 4: effective target for a flagged run

```c
/* written for this project; sketch of the ladder change, no third-party code */
uint64_t eff = req->target_ticks;
if ((req->flags & IVS_RUN_F_STOP_ON_FRAME) != 0u) {
  uint64_t e = ivs_stic_next_frame_end(params, &im->stic); /* S_k + F of the frame in progress, from state, not now / F */
  if (e < eff) { eff = e; }
}
/* stage 7 capacity uses eff (video: slot * (D/F + 2) with D = eff - now), the loop runs to eff,
 * overshoot stays against req->target_ticks, stop reason FRAME when the flag is set and a frame completed */
```

### Pattern 5: produce/consume split for the new gate step

One pure step `video` (text formats, no `string(JSON)`): register literal grammar and 64-row tiling, geometry literal, cards literal, frame grammar, per-card pixel-count conservation, hash tie to the `.expect`, page-reading verdict keys present. Cross-checks against generated tables belong to suites (`stic.table.matches-literal`, `video.geom.profile-derives-159x96`), because pure steps cannot read build outputs (Phase 4 anti-pattern).

### Anti-patterns

- Putting the STIC registers, the mode or the latch in the memory system's READ path (`ivs_memsys_read` must stay `const` and pure; a peek of `$0021` must be byte-identical before and after).
- Deriving the frame index from `now / F` ("never derived by modulo" is a statement about state, even though `frame_phase_ticks = 0` makes it numerically equal today; the named fault `STIC_FRAME_BY_MODULO` is the test).
- Rendering lazily at the end of a call or at a call's target (the PSG A-02 rule: catch up to a unit boundary or an access stamp, never to a call target).
- Hashing a struct by memory (slot header and chain framing are written byte by byte, big-endian).
- Letting the video chain depend on whether a buffer exists (the audio row's lesson: the same code runs against a counting sink).
- Magic numbers in the render path (159, 96, 160, 192, 240, 15,296): geometry comes from derived profile data; goldens are indices.
- Reading the CPU view in the renderer (gates and `*_loaded` flags would leak into pixels): dedicated backing accessors only.

## Don't Hand-Roll

| Problem | Don't Build | Use Instead | Why |
|---------|-------------|-------------|-----|
| SHA-256 (video chain) | a second implementation | core: `core/src/common/ivs_sha256.c` (incremental, 101 us per slot); runner: its own `ivs_rsha256` (`runner/runner_sha256.c`, streaming); oracle: `cmake -E sha256sum` over a `printf`-framed byte string | already FIPS-vector tested; the oracle is a different implementation (copy-paste rule: the runner keeps its own) |
| Expected frames | a second renderer, a generator inside the repo's tests, model output | hand-derived literal text (`data/video/frames/*.frame`), conservation and hash-tie checks in the pure step | the whole point of VID-04 is independence; shared-core agreement is not evidence (AGENTS.md) |
| Event tables and windows | hand-typed arrays in C | generated from profile fields by `IvsSticTable.cmake` (fail closed), parameters passed as a struct | a perturbation test needs a stack copy of the struct, not a registry seam (Phase 5 Pattern 3) |
| Profile to C tables | a JSON parser in the core | `cmake -P` generators in the `IvsPsgTable.cmake`/`IvsCtrlTable.cmake` style | no JSON parser may enter the core |
| Named faults | a new loop | `data/expected/faults/<suite>.ids` + `ivs_fault_suite` + `faults-verdict` | per-fault intended-reason check exists |
| Negative controls | a new mechanism | `tools/check/run-controls.sh` overlays on `_good/tree` | exact inventories exist |
| Slicing oracle | a second run implementation | `run(now + 1)` per unit as the reference plus relational checks, and a schedule-computable oracle where the program is computable | as Phase 5 D-05 |
| Window, grant and interrupt semantics | a re-implementation in the machine layer | the stepper's existing BUS_GRANT/INT_ENTRY units, driven only through `lines` | the Phase 2 vectors (`busrq-halt.json`, 22 vectors) already pin them with named faults |
| PNG export | a deflate/CRC writer | PPM (P6) written by an operator-side helper, or nothing | a viewable artifact is optional and never evidence; PNG needs a CRC table and zlib framing for no gate value |
| Operator output cleanup | by-path `unlink` after a failed run | the Phase 5 post-review behaviour (`ivs_hostio_out_remove_if_same`, same-file refusal for `--update-golden`) copied for `--video-raw` | review WR-01 and WR-02 found the pre-review forms losing evidence |

**Key insight:** the new risk is again registration breadth and seam order, not algorithms; the engine (events, lines, render, slot writer) is about 600 lines of integer code.

## Plan Decomposition (re-cut check of D-23)

File-set collisions found in the CONTEXT outline: `ivs_run.h`, `ivs_machine.h`, `machine.h`, `machine_bound.c`, `machine.c`, `machine_digest.c`, the profile JSON, `profile-fields.ids`, `api-effects.json`, `data/expected/tests/*.ids`, `tests/CMakeLists.txt`, `faults.cmake`/`core-generated.ids`, `important-gates.ids`, the goldens and `pkg-consumer.golden`. Over-cap or over-coupled plans in the outline: 06-03 (ABI plus profile plus a fourth generated table, 25 or more files); 06-04 (device kind, generator change, memmap flip, register file, windows, handshake, inspection, a new public call with eight registries, composite v3, re-bless, suite creation, 8 faults); 06-06 (core framebuffer, a runner sizing change D-23 puts in 06-09, five-file test inversion); 06-09 (grammar, exec, key lists in more than ten places, operator outputs, golden). Recommended cut: 14 plan slots, 14 sequential waves (plus the conditional lever), every plan ends gate-green under `sh` and `dash`, and under `sh tools/gate-linux.sh` where marked.

| # | Plan | Reqs | Depends on | Core deliverables (summary) | Faults / controls (est.) | Tasks | Gate before push |
|---|------|------|------------|-----------------------------|--------------------------|-------|------------------|
| 06-01 | Sources and oracles (no C) | all | none | page-image reading verdict block (`registers-printed` .. `timing-figure-readable`), `docs/stic/reference-notes.md`, `data/stic/rules.json`, register literal (64 rows), geometry, cards and frame literals, hand derivations (`docs/video/*`), ONE allocation table (discrepancies from C81, claims from CL-0016, `d10-measurements.ids`), step `video` with controls and toy tree, listing SDBD rule with controls, `docs/time-base.md` correction (lines 31 and 72-73), blind second-read requests, `.gitignore` and `binary-ext.txt` additions | 0 / about 14 | 5 | sh, dash |
| 06-02 | Lever pass and the normalised metric | none | none (file-disjoint) | `-g0` on fault targets (tests/faults.cmake, after the preset's `-O1 -g`), record `cpu_s / build_s` in CI-EFFICIENCY.md, orchestrator re-baseline request, `phase6-start` record | 0 / 0 | 2 | sh, dash, linux |
| 06-03a | ABI growth, behaviour-neutral | VID-03 (support) | 06-01 | `IVS_RUN_F_STOP_ON_FRAME` rename (still refused), `IVS_STOP_FRAME` + pin + `ivs_run_stop_name`, report v3 fields (written 0) and pins, `ivs_info` v4 fields (zero), `IVS_OUTPUT_FORMAT_INDEX8_FRAMES_V1`, env `video_slack_frames`, installed consumer size line, `run.struct.*` extension, header text amendments (capacity paragraph, nominal frame) | 1 / 3 | 4 | sh, dash, linux |
| 06-03b | Profile growth and the STIC table | VID-01, VID-03 (support) | 06-01 | `stic.*`, `video.*`, `bus.line_sample_lag_cpu_cycles` fields with tag/cite/conflict, `IvsSticTable.cmake`, `ivs_stic_params` TU and struct, `core/CMakeLists.txt` and `core-generated.ids`, fourth-table registry tie, tests for the table and the shape checks | 2 / 3 | 5 | sh, dash, linux |
| 06-04a | State and digest foundation; composite v3; golden re-bless 1 | VID-01, VID-06 (support) | 06-03a, 06-03b | FIRST: pin the v2 literals (`digest-v2-literals.ids`, `md_fresh_composite`, `md_stepped`, the four goldens); then `stic_reg[64]`, the 528-byte STIC state, rows, `IVS_MACHINE_BYTES`, composite 3, reset/create clears, hand-framed streams, golden re-bless 1 (classified), `pkg-consumer.golden` | 3 / 0 | 4 | sh, dash, linux |
| 06-04b | Register file, windows, handshake, memmap flip, suite `stic` | VID-01 | 06-04a | device kind `stic14`, generator change (all STIC regions real), regmap masks, hook table, wrapper catch-up and effects, the 16-event table and cursor (FRAME_START, REG_CLOSE, GMEM_CLOSE live; the rest inert), blocked policy and counters, `$0020` strobe, `$0021` read switch, memmap literal split + probe + descriptor row, suite `stic` created, golden re-bless 2 (diag-ctrl-psg-pass `mem` and `instance` lines only) | 5 / 2 | 5 | sh, dash, linux |
| 06-04c | Inspection call | VID-06 | 06-04b | `ivs_machine_stic_state` + `ivs_stic_report`, effect row and tests, installed consumer, `pkg-consumer-calls.ids`, counters, peek purity sweeps with the positive control pair | 1 / 1 | 4 | sh, dash, linux |
| 06-05 | Timeline lines, VBLANK interrupt, bus steal, bounds re-proof | VID-02 | 06-04c | `ivs_stic_lines`, seam wiring (machine_run.c:150), ROW_FETCH latch, `windows_active` decision, tiling and accounting identities, count per frame in {13,14}, slicing, fuzz, `stic.bounds.*` through the full machine, stale-statement updates in profile notes and `psg_bounds.c` comment, generator second refusal (release unit and entry within U) | 6 / 2 | 5 | sh, dash, linux |
| 06-06 | Framebuffer, buffers, domain 16, frame stop, runner sizing | VID-03 | 06-05 | video channel in the sink, slot writer, chain (h0 at create and reset), capacity with slack and T', flag accepted, FRAME stop, report/info live, `ex_plan_buffers` slack, test inversions (run_bound, run_input, run_buffers, run_info), suite `video` created, display-off frames only | 5 / 2 | 5 | sh, dash, linux |
| 06-07 | Foreground/Background card rendering | VID-04 | 06-06 | renderer, dedicated accessors, BACKTAB row latches into the plane, 16 colours, frames F-A1..A5, bit-role probe sweep | 3 / 2 | 4 | sh, dash, linux |
| 06-08 | Color Stack and inspection completion | VID-04, VID-06 | 06-07 | stack advance/wrap/reset, mode switch end to end, Colored Squares cut line, frames F-B1..B3, cross-mode relations, sweeps | 3 / 1 | 4 | sh, dash, linux |
| 06-09 | Runner and scenarios (`ivsr 3`) | VID-03, VID-04 | 06-08 | `capture video`, frame/pixel/hash asserts, per-slot chain re-hash, new keys in every list, `ivsr 3`, toy reports, golden grammar, operator `--video-raw` with the Phase 5 cleanup rules, helper; split into a/b if the task count exceeds 5 | 2 / 4 | 5 | sh, dash, linux |
| 06-10 | Diagnostic and end to end | VID-01..04 | 06-09 | `ivs-diag-stic-1`, manifest, listings/fixtures ids, scenarios, six mutants, installed-consumer replay, `pkg-consumer.golden`, new goldens | 0 / 2 | 5 | sh, dash, linux |
| 06-11 | Close-out | all | all | claims, discrepancy updates, important-gates registry (+5 requirement ids, +3 controls), D-45 and normalised-metric evaluation, VERIFICATION three tables, Decision Register notes (D3, D4, D8) | 0 / 3 | 3 | sh, dash, linux |
| 06-L1 | Conditional lever | | trigger only | next lever of verify note V6-12 | | | |

Fault budget: 1 + 2 + 3 + 5 + 1 + 6 + 5 + 3 + 3 + 2 = 31 as estimated per plan; the phase cap is 28 (D-21), so three of the lowest-value faults become judge self-controls. The five notes propose well over 28 distinct named faults (the cards note alone says 19); rank by determinism seams first (catch-up at target, hook completeness, alias hook, event order, frame by modulo, buffer-dependent state, capacity plus one, T' capacity, chain framing, mode switch on peek) over card-bit faults (a probe sweep plus judge self-controls cover bit roles).

Wave suggestion: one plan per wave in the table order. 06-02 is file-disjoint and may run first or in parallel with 06-01 only if the orchestrator wants wall time; the sequential default stands.

**Sequencing constraints (hard):** C-id and claim allocation (06-01) before any profile field cites them; the v2 literal pin is the FIRST task of 06-04a; 06-03a before any plan that writes a grown struct field; 06-03b before 06-04b (generator and table); 06-04a before 06-04b (layout); 06-04b before 06-05 (the lines need the latch and windows); 06-05 before 06-06 (frames are timeline events); the runner's `ex_plan_buffers` change lands in the same plan that publishes the real video layout (06-06); 06-06 before 06-07 before 06-08; 06-09 before 06-10; every fault site is registered in the commit that adds it (`fault-orphan-site`); the oracle commit precedes the implementation commit and each SUMMARY names it (the verifier reads `git log`).

## Common Pitfalls (ranked)

### Pitfall 1: golden and literal churn, and what the classification can honestly say (determinism evidence)
**What goes wrong:** every plan that adds a digest row or evolves STIC state moves `digest.mem`, `digest.instance` and `digest.composite` of every golden, and `pkg-consumer.golden` follows; unclassified drift hides a real regression.
**Why:** the baseline `.digests` goldens all end before the first frame end (now_ticks 2,084; 16,752; 2,084/1,020; 404, against 59,736) with `video=none`, so `digest.video` is NOT in them; the one that ends at 16,752 ticks is past both windows (8,000 and 15,120 ticks), so its `mem.gate_open` bytes change in 06-04b.
**How to avoid:** pin the v2 literals first (new file `data/expected/digest-v2-literals.ids`, modelled on `digest-v1-literals.ids`: `test.fresh-composite`, `test.stepped.0..4`, `golden.<id>.digest.*`), then bump. Classification rule for 06-04a: ONLY `digest.mem`, `digest.instance`, `digest.composite` lines move, in all four `.digests`; `digest.cpu`, `digest.images`, `digest.audio`, `digest.input`, `digest.psglog`, every `step.*`, `slot.*`, `cpu.*`, `input.*` line byte-identical. 06-04b: only diag-ctrl-psg-pass `digest.mem`, `digest.instance`, `digest.composite` (gate bytes and cursor), the three short goldens unchanged; if Pattern 3 is followed, 06-05 and 06-06 move NO existing `.digests` (compare against the literal pin). Never hand-edit a golden; regenerate with `ivs-run --update-golden` and read the diff. New goldens (06-10) are new files with `video=observed`.

### Pitfall 2: layout, alignment and the 72-versus-64 lesson
**What goes wrong:** Phase 5's PSG state was planned at 64 bytes and had to be 72 so the instance stays a multiple of 16 with no tail padding (05-03 deviation 1); the digest file asserts `IVS_MACHINE_SLOTS_END == sizeof` and `obs` must stay 8-aligned.
**How to avoid:** derive on paper first, with the static-assert harness this research used: memsys +128 -> 113,392 (a multiple of 16), `stic` at 113,520 (0 mod 8), STIC state a multiple of 16 (528 in the candidate), total 114,416 = 16 x 7,151; keep every member u32/u64 or u16 arrays with explicit reserved words (PAD rows in the digest table); update the layout comment block (machine.h:14-37), `IVS_MACHINE_BYTES`, the three offset asserts and the digest field table in the same commit; a `uint64_t` in the state that is not on an 8-byte offset opens a hole that `md_coverage` reports.

### Pitfall 3: hook completeness and the mode-switch alias read
**What goes wrong:** an access that can observe or change an input of an event but is not hooked reads or writes stale gates, latch or BACKTAB (the row latch reads sysram at its own tick). The read-view decode of `$4021`, `$8021`, `$C021` is the cartridge, so a hook keyed on the read view misses the mode switch; a DW (interrupt-entry write) whose R6 points into BACKTAB is a write too.
**How to avoid:** hook table from both views (Pattern 2); `stic.hook.completeness` over all 65,536 addresses x 3 access classes against the generated table; positive control `stic.alias.read-4021-switches-mode`; faults `STIC_ALIAS_NOT_HOOKED`, `STIC_BACKTAB_WRITE_NOT_HOOKED`, `STIC_CATCHUP_AT_TARGET`, `STIC_EVENT_ORDER_START_BEFORE_END`. Blocked-access counting lives in the wrapper (the memory system's read is const), so the wrapper must compute "blocked" from the same gate state the memory system reads.

### Pitfall 4: the frame tie rule and the straddling unit
**What goes wrong:** inclusive catch-up means a write stamped exactly `E_k` is not in frame k; a unit that straddles `E_k` can contain accesses on both sides; D-14's wording ("events `<= E_k`") reads the other way.
**How to avoid:** one sentence in the plan and the docs: "a frame contains the accesses stamped below `E_k`"; tests `video.boundary.event-minus-1/.event/.event-plus-1` using observed unit starts (never predicted from the cycle table, Phase 5 Pitfall 9), a store swept over every cycle of an instruction across `E_k` (the C67 pattern), fault `VIDEO_FRAME_AT_CALL_END`.

### Pitfall 5: late grants, non-interruptible streams and vacuous accounting tests
**What goes wrong:** a guest whose loop is made only of non-interruptible rows (the MVO family, EIS, DIS, shifts) is never stalled and never interrupted; an accounting test that assumes 13 stalls per display frame passes or fails depending on the stand-in's mix. Executed: 13,452 to 13,524 executed cycles for interruptible mixes, 14,927 to 14,938 for an MVO stream.
**How to avoid:** `stic.steal.accounting` states the identity per window (`resume - w = 468`, stalled cycles = `(resume - grant)/4`), uses an interruptible-heavy stand-in, and asserts the late-grant counter equals the number of windows without a grant; a separate vector pins the MVO stream (0 grants, `late_grants` 13 per frame). Interrupt vectors: the boot stub's `$1004` is an unexpected-interrupt trap that halts (stub contract 1), so any stand-in that executes EIS must provide its own handler or be judged to halt there.

### Pitfall 6: the runner always stops at HLT; the diagnostic's cycle budgets
**What goes wrong:** exec.c:824 sets `IVS_RUN_F_STOP_ON_HALT` on every slice, so a diagnostic that halts before the first frame end (cycle 14,934 plus about 13 x 116 stolen) produces no frame; the display latch is cleared every frame start, so the strobe must land in frame 0; registers only inside 2,000 cycles and GRAM/GROM inside 3,780 cycles of tick 0.
**How to avoid:** the 06-01 derivation fixes a cycle budget table before the listing exists (reset entry and boot stub about 50 cycles; each MVII + MVO pair 19 cycles; MVO-indirect loops 24 per word); the program strobes `$0020` early, copies GRAM (4 patterns, 32 words) inside the GMEM window, fills BACKTAB rows ahead of their latch ticks (row i at cycle 3,848 + 912 i), then runs a counted delay loop past cycle 15,000 (about 1,000 iterations of DECR plus a taken branch) before HLT and the three followers; the scenario's `run-to ... stop halted` then ends with frame 0 completed. Mutants must not lengthen a loop unboundedly (budget timeout is the deterministic bound).

### Pitfall 7: the runner and the 8 MiB block
**What goes wrong:** `ivs-run` always passes an 8 MiB block (main.c:674, 743-744) and `ex_plan_buffers` cuts it by `per[i] x (slice/F + 1)`. When the core publishes video (06-06), per-frame video bytes become 15,296 and the law becomes `slot x (D/F + 2)`: a runner that keeps `+1` has capacity for 71 slots against a requirement of 72 and EVERY scenario harness-fails BUFFER_TOO_SMALL; with `--slice-ticks 1` the requirement is 2 slots, the plan gives 1.
**How to avoid:** change `ex_plan_buffers` in 06-06 to read the slack from `ivs_info` v4 (`video_slack_frames`) and size `slot x (slice/F + 1 + slack)`; total 4,615,244 bytes at the default slice; a runner test with slice 1, slice 7 and the default; `video.chain.check` and captures remain 06-09.

### Pitfall 8: the reserved flag is a test fixture in six places (one dedicated test plus five generic uses)
**What goes wrong:** renaming or accepting flag 2 breaks tests that use it as "an arbitrary unsupported flag" (run_bound.c:1085, :1109, :1146; run_input.c:876; run_buffers.c:757) and the api-effects text; a plan that only inverts the one named test goes red.
**How to avoid:** 06-03a keeps refusing flag 2 under its new name and moves the five generic uses to a bit that stays unsupported (`4u`); 06-06 inverts `run.refuse.flags-frame-reserved` into positive `run.flags.stop-on-frame-*` ids as a reviewed change recorded in the SUMMARY, edits the effect-table row (api-effects.json:396 and :429) and the `ivs_run.h` flag comment.

### Pitfall 9: the memmap generator, probe and the stub tests
**What goes wrong:** a half-real STIC cannot be generated (mechanism 4); the behavioural width probe merges adjacent equal tuples and will see per-register widths, write-only registers and the two side-effect registers (`$0020` stores nothing, `$0021` is a mode switch); `ro_peek_undriven`, `memmap_rows.c:282` and `memmap_main.c:541` encode the stub.
**How to avoid:** flip all five regions in one plan with the literal split by width, a declared exception class for registers whose access is a strobe (an `x=` name added to `tools/check/memmap.cmake`'s accepted set, as `x=reads-zero` and `x=pins` were in Phase 5), the rule table rows and descriptor row updated, and `memmap.row.*` ids renamed in `data/expected/tests/memmap.ids`; CL-0006 artifact refreshed.

### Pitfall 10: four generated tables, one registry
**What goes wrong:** a fourth generated table (`ivs_stic_params`) that is not tied to `ivs_profile_count` silently disables the device for a new profile (Phase 5 review WR-04).
**How to avoid:** add the fourth term to `ivs_machine_profile_covered` in the plan that creates the table (06-03b), extend `machine.create.tables-cover-registry`, remove NULL guards in the new code (assert by construction), fault `STIC_TABLE_SHORT`; `ivs_core_gen` DEPENDS and `core-generated.ids` gain the new include or `c-purity` and fault builds miss it.

### Pitfall 11: a new public symbol touches eight registries
**What goes wrong:** `ivs_machine_stic_state` changes `api-calls` and `public-symbols` (14 to 15), `data/api/api-effects.json` (a refusal row needs a test id that is not skip-ok), the installed consumer's undefined-symbol list (equality with `data/expected/pkg-consumer-calls.ids`), the header (`ivs_run.h`), `core-symbols` (no new import expected), `headers-tap`, the consumer golden, and a pinned size for `ivs_stic_report`.
**How to avoid:** its own small plan (06-04c) with the cpu_state refusal ladder copied (BAD_ARG for out NULL or a bad struct_size, BAD_HANDLE, BAD_ARG for overlap), the report built in a local and copied last, valid OFF and POWERED, a pin of the v1 size, and the positive control for VID-06 in the same plan.

### Pitfall 12: gcc versus clang in code that does not exist yet
**What goes wrong:** `-Wall -Wextra -Wpedantic -Wshadow -Wconversion -Wsign-conversion ... -Werror` is on in both presets; 14-bit fields, 4-bit colour numbers and 3-bit pointers under `int` promotion, unspecified argument evaluation order in test helpers and table loops, gcc-only `-Wmaybe-uninitialized` on a `goto done` path (05-08) and `-Wformat-truncation` on runner text (05-09) each cost a hosted round trip.
**How to avoid:** all STIC and render arithmetic in `uint32_t` with explicit masks and the `IVS_U16` narrowing helper (cp1610.h:24); pixel coordinates unsigned; one side-effecting call per statement; initialise every variable read after a `goto done`; narrow format buffers; no float (`#pragma GCC poison float double` is in force); run `sh tools/gate-linux.sh` at every marked plan.

### Pitfall 13: the sink and the 34 test call sites
**What goes wrong:** introducing a new sink type or a new seam signature forces edits in `tests/machine/machine_reset.c` (10 sites), `tests/psg/*` (8 sites) and 34 `ivs_psg_sink_init` calls, in a plan that is meant to change behaviour, not tests.
**How to avoid:** extend `ivs_psg_sink` (zero video channel = counting) and keep `ivs_machine_step_unit_internal`, `_sink_internal` and `_trace_internal` signatures; record the oddity of a psg-named struct carrying video in the header comment; one fault `VIDEO_STATE_DEPENDS_ON_BUFFER` (copy of `AUDIO_STATE_DEPENDS_ON_BUFFER`).

### Pitfall 14: gate cost is dominated by reruns, not the engine; budgets are already over their triggers
**What goes wrong:** per measured Phase 5 units a fault costs 0.137 cpu-s per run locally (about 0.5 hosted with its build), a control 0.185 cpu-s on macOS (0.04 native Linux; a control that reaches the C symbol layer about 2.2 hosted), a test id 1.5 to 3.5 ms; Phase 5's estimate (+17..28) was exceeded 2 to 2.6 times (realised about +55). The last-5 hosted medians (gate 106, cpu 336) are over the triggers (90, 280) while the speed-normalised `cpu_s / build_s` is stable (6.4 to 7.4 since Phase 5), and the local baseline this session is 42 s and 131 cpu-s.
**How to avoid:** plan 06-02 first (`-g0` on fault targets only, the metric recorded beside the raw triggers); judge self-controls and controls before faults; at most one control per plan that reaches the C symbol layer; suite plain-run targets 0.3 s release and 1.5 s dev (video tests run few frames: one 71-slot slice is about 9 ms of SHA); keep fuzz at 100,000 ticks per seed and slice-1 replays for one scenario; checkpoints after 06-04b and 06-06 with the stop rule of V6-12; the per-note cost estimates (+5..9 cards, +1..2 regs) are floors.

### Pitfall 15: the time-base document states the opposite of the design
**What goes wrong:** `docs/time-base.md` line 31 ("A frame is a timeline event and is never derived by modulo of the master-tick count") and lines 72-73 ("A frame is never derived by modulo, because stalls and the display-enable state change how many CPU cycles a frame holds") give a rationale that D-07 contradicts: the frame is a constant 59,736 ticks and stalls change how many CYCLES THE CPU EXECUTES in it.
**How to avoid:** correct it in 06-01 before any STIC code (D-07): state the event-and-cursor design, the constant length, `frame_phase_ticks`, and that a frame is never derived by modulo as a rule about state, not about numerical equality. The glossary and text steps read this file: claim words need tags, the bare word clock needs a qualifier.

### Pitfall 16: hygiene and rights in the oracle plan
**What goes wrong:** datasheet text pasted into a literal or note; page images or a PPM committed; an email-shaped string (a mnemonic with the at sign next to a name and a dot) in a tracked doc; a scratch path in a script; `.ppm` and raw-video files outside `.gitignore` (the Phase 5 residual that `*.raw` was never ignored).
**How to avoid:** literals and notes are paraphrase plus page locators; PDFs only under `refs/`; `data/sources.json` entries with sha256, size, `local_path`, `rights: unknown`; add `ppm` to `tools/check/binary-ext.txt` and `*.ppm` plus the raw-video extension to `.gitignore` in 06-01 (or 06-09 for the operator output); the operator output default mode 0600 is the carried IN-05 residual, decide it deliberately for `--video-raw`; no at sign in prose.

### Pitfall 17: a nonzero `frame_phase_ticks` and the pre-frame interval
**What goes wrong:** with `frame_phase_ticks > 0` the interval `[0, S_0)` belongs to no frame: gates, latch and lines are undefined there, and `video_first_index`/`next_frame_end_ticks` need a defined first value.
**How to avoid:** have the generator refuse a nonzero phase until the plan defines the pre-frame behaviour (or define it as "frame 0 starts at 0 and the phase only shifts later frames"); publish `frame_phase_ticks` in `ivs_info` v4 as D-14 says; test with `0` and a perturbed profile copy built on the stack.

### Pitfall 18: the Phase 5 lessons that still bite
**What goes wrong:** a golden update placed in the wrong plan keeps `golden` red for several tasks (05-09 deviation 1); fault sites travel with registrations (`fault-orphan-site`); a watchdog or FIFO construct without a bounded wait hangs the gate (05-12); a vacuous generator (a regex that dropped a match group) passes every vector.
**How to avoid:** classify and regenerate goldens in the plan that moves them; register each fault in the commit of its detecting test; no waiting construct without a bound; every new generator and judge gets a judge self-control that fails for the intended reason.

## Code Examples

### Hook skeleton (written for this project; derived from machine_bus.c:27-65)
```c
/* hook class from the generated granule tables: O(1), both views; no region scan */
uint32_t cls = ivs_memsys_hook_class_internal(&c->m->mem, a->kind, a->addr);
if (cls != IVS_HOOK_NONE && c->sink != NULL) {
  uint64_t stamp = c->unit_start_tick + (uint64_t)a->offset_cpu_cycles * (uint64_t)c->ticks_per_cpu_cycle +
                   (uint64_t)c->stic->access_strobe_ticks;
  ivs_stic_catch_up(c->stic, &c->m->stic, &c->m->mem, &c->m->obs.video, stamp, c->sink);
  /* then: gate decision from c->m->mem.gate_open, blocked counter, $0020 strobe / $0021 read switch effects */
}
reply = ivs_memsys_bus(&c->m->mem, a); /* the only store; a blocked write is dropped by the closed gate */
```

### Slot framing (D-13, D-16; byte by byte, big-endian)
```
slot = be64(tick = E_k) || be64(index) || be32(width) || be32(height) || be32(flags) || be32(0) || plane[width * height]
chain(n+1) = SHA-256( chain(n) || slot );  chain(0) = SHA-256("IVS-VIDEO-LOG-V1")
```

### Independent oracle for the framing (authoring-time construction; no emulator involved)
```
a display-disabled frame (all 0xFF plane) with a hand-written header printed with printf and hashed with
cmake -E sha256sum gives the expected chain step; the runner recomputes the same value from delivered bytes
```

### Struct pins (compile-time, as the PSG plans did)
```c
_Static_assert(offsetof(ivs_run_report, video_first_index) == 136u, "report v3 starts at 136");
_Static_assert(sizeof(ivs_run_report) == 160u, "ivs_run_report is 160 bytes (v3)");
_Static_assert(offsetof(ivs_info, frame_phase_ticks) == 288u, "info v4 tail starts at 288");
_Static_assert(sizeof(ivs_info) == 320u, "ivs_info is 320 bytes (v4 candidate; the plan pins the final list)");
```

## State of the Art

| Phase 5 | Phase 6 | Impact |
|---------|---------|--------|
| `lines` hard-wired 0; no device raises an interrupt or stalls the bus | STIC drives INTRM and BUSRQ through the stepper's own units | `psg.bounds.*` re-proved through the full machine; stale "until Phase 6" statements updated |
| Video layout NONE, report counters never assigned | real slots, per-frame chain, frame stop | composite 3, `ivsr 3`, capacity law `+2` for video |
| Reserved run flag 2 | `IVS_RUN_F_STOP_ON_FRAME` | one inverted test, five generic uses moved |
| STIC region a stub (`$FFFF`, drops writes) | `stic14` device with masks, windows, effects | memmap literal, probe and three test files change |
| Two original diagnostics | three (the third carries SDBD) | listing checker gains the SDBD rule; D4 recorded, not crossed |

## Environment Availability

| Dependency | Required By | Available | Version | Fallback |
|------------|------------|-----------|---------|----------|
| CMake, Ninja, cc | everything | yes | 4.4.3, 1.13.2, Apple clang 21 | none needed |
| gcc (native) | gcc-only behaviour | no (Apple clang under both names) | n/a | `sh tools/gate-linux.sh` |
| dash | POSIX-sh parity | yes | `/bin/dash` | none needed |
| docker + `ubuntu:24.04` | gate-linux | yes (daemon up, image cached) | Docker 29.5.2 | skip only for record-only plans (06-01) |
| `pdftotext`, `pdftoppm`, `tesseract`, `qpdf` | plan 06-01 page-image reading (pdf 191-193, 199; 1982 equivalents) | yes | 26.04.0, -, 5.5.0, - | none needed; `mutool` absent and not needed |
| Cached catalogs under `refs/` | plan 06-01 | yes (1980, 1982, CP-1600 manual, hash-registered) | sha256 prefix `70094d12` for the 1980 catalog | if a page cannot be read, record "not read" and keep `unknown-default-chosen` |
| `gh` | `tools/ci-record.sh` | orchestrator only | n/a | executors never call it |
| Owner hardware, EXEC/GROM | D10, D11 | not available | n/a | record missing, hand off |

**Missing dependencies with no fallback:** none.

## Validation Architecture

Full map, ids and sampling: `06-VALIDATION.md`. Summary:

- Framework: in-repo harness (`tests/harness/ivs_test.[ch]`) under ctest; new suites `stic` and `video` (own executables, ids files, fault files); shell TAP (`runner-process`); one new pure step `video`; extended steps `listing` (SDBD), `golden`, `api-effects`, `pkg-install`, `important-gates`, `memmap`; named faults; negative controls; in-suite judge self-controls.
- Quick command per task: `cmake --build --preset dev && build/dev/tests/ivs_tests_<suite> --only <prefix>`; per plan: `sh tools/gate.sh` and `dash tools/gate.sh`; marked plans add `sh tools/gate-linux.sh`.
- Independent oracles: structure printed on the pages (geometry, counts, SR1/SR2/SR3, 13 or 14 requests per frame); hand-derived literal frames with per-card popcount conservation; closed forms (resume at `w + 468`, tiling, count per frame); slicing invariance; whole-instance byte equality for inspection with a positive control; `cmake -E sha256sum` over printf-framed bytes; hand-derived diagnostic predicate. Policy numbers are never expected values: profile perturbation and accounting identities only. Self-recorded goldens are repeatability only.

## Data File Updates (discrepancy log, profile fields, claims, inventories)

| File | Edit | Plan |
|------|------|------|
| `data/discrepancies.json` + `data/expected/discrepancies.ids` | next free id is C81 [VERIFIED: baseline gate `discrepancies=80`]; ONE allocation table in 06-01 maps the register note's C81..C89, the timing note's C81..C86 and the card note's CV-1..CV-9 to unique ids; update C1, C2, C3, C4, C10 (open -> partial, text names `video.last_card_column_px`), C21, C38 (promoted to a field), C41; the profile checker requires a conflict to exist before a field cites it | 06-01 |
| `data/profiles/intv-2609-ntsc-family-p1.json` + `data/expected/profile-fields.ids` | the Phase 6 fields (mechanism 5); `unknown-fields` count grows by about 14; retag nothing silently | 06-03b, 06-04b |
| `data/memmap/intv-2609-ntsc-family-p1.literal.txt` | split the five STIC regions by width, `impl=real`, new `x=` exception name for strobe registers; CL-0006 artifact refreshed | 06-04b |
| `data/claims.json` + `claims.ids` | next free CL-0016 [VERIFIED: baseline `claims=15`]; CL-0016.. provisional, weakest-field rule, none at measured-hardware | 06-10, 06-11 |
| `data/sources.json` + `sources.ids` | entries for the wiki STIC and BACKTAB revisions, the BSR 2609 page, the 1982 catalog pages: `capture: hashed` with `local_path` under `refs/` or `reference-only`, `rights: unknown` | 06-01 |
| `data/expected/` | new: `digest-v2-literals.ids`, `video-frames.ids`, `stic-rules.ids`, `tests/{stic,video}.ids`, `faults/{stic,video}.ids`, `gate-steps/video.ids`, `controls/video.ids`, `diag-mutations-stic.ids`, `runner-scenarios.ids` (+4), `listings.ids` (+1), `fixtures.ids` (+1), `d10-measurements.ids` (+9 D10-VID items and one D11 item), `important-gates.ids`, `pkg-consumer.golden`; changed: `api-stop-reasons.ids` (+`IVS_STOP_FRAME 5`), `core-generated.ids`, `pkg-consumer-calls.ids` (+1), `tests/{run,memmap,machine,runner,runner-process}.ids`, `faults/{run,machine,runner,psg}.ids`, `profile-fields.ids` | all |
| `tools/check/controls/_good/tree/` | toy register literal, frame and geometry files for step `video`; seven toy reports gain the `ivsr 3` header and keys in 06-09 | 06-01, 06-09 |
| `.planning/PROJECT.md` Decision Register | D8 (delegated, D-12), D3 note, D4 note; `.planning/CI-EFFICIENCY.md` checkpoint entries | 06-11 |
| `.gitignore`, `tools/check/binary-ext.txt` | `ppm` and the raw-video extension | 06-01 |

## Security Domain

| ASVS category | Applies | Standard control |
|---------------|---------|------------------|
| V5 Input validation | yes | scenario parser caps and charset (IVSS stays inert text; no directive names a file), whole-array event validation, request/report struct-size rules, buffer overlap tests with `uintptr_t` differences for the video buffer (existing stage 4b), frame header consistency re-checked by the runner |
| V12 Files and resources | yes | operator-only `--video-raw` and the export helper written only in `hostio.c`, exclusive create, byte cap, cleanup on failure by device/inode identity, `--update-golden` refused when it names an output file (Phase 5 WR-01/WR-02 behaviour copied, not the pre-review form) |
| V6 Cryptography | partly | SHA-256 only as identity and framing digest; the core's and the runner's implementations stay separate; no new primitive |
| V2, V3, V4 | no | no network service or users |
| V14 Configuration | yes | workflow unchanged (no cache, artifact, action); `ci-parity` bans stay |

| Threat | STRIDE | Mitigation |
|--------|--------|------------|
| Video buffer aliasing the instance, report, events, audio or log | Tampering | existing overlap stage covers video; tests with poison sentinels, nothing written on refusal (`VIDEO_OUTBUF_PARTIAL_WRITE`) |
| Undersized video buffer silently drops a frame | Repudiation | capacity checked before the first unit with the `+2` law; INTERNAL stop as never-expected safety net; no partial slot |
| Huge target with the frame flag demands astronomical capacity | Denial of service | capacity computed on `T'`; 64-bit guards of `ivs_bound_fits` retained |
| Rights-bearing frames from a user EXEC/GROM leaving the machine | Information disclosure | operator-local artifact, never committed, helper refuses `content ... user` scenarios (`look-demo` rule of the frame note), `.ppm` ignored and binary-listed |
| A scenario names an output path or a palette | Elevation | IVSS grammar has no such directive; operator flags only |
| Home or scratch path in a tracked file | Information disclosure | `hygiene` step; `<repo-root>` placeholders |

## Assumptions Log

| # | Claim | Section | Risk if Wrong |
|---|-------|---------|---------------|
| A1 | gcc 13 `-Wconversion` and `-Wmaybe-uninitialized` behaviour on new narrowing patterns matches past findings (classes only; no new code exists yet) [ASSUMED] | Pitfall 12 | an extra hosted round trip; mitigated by `gate-linux.sh` |
| A2 | The candidate `ivs_info` v4 field list (312 or 320 bytes) and the 528-byte STIC state are acceptable names and sizes [ASSUMED: candidates, the arithmetic is verified] | Derived constants | none to correctness (static asserts fix the real numbers); the plans pin the final list |
| A3 | Cost of the new suites and faults follows Phase 5's measured unit costs times its realised 2 to 2.6 overrun [ASSUMED from verify note V6-11 and the Phase 5 close-out] | Pitfall 14 | gate budget triggers fire earlier; checkpoints and the lever order are pre-committed |
| A4 | The pages (pdf 191-193) print no register map or window figure at text-layer level; the timing figure on 5-12/9-45 is unread [CITED: 06-DISCUSS-verify.md section 1; the text-layer features page was re-read this session, the images were not] | Summary | 06-01 verdict changes a tier, not the plan structure (D-01) |
| A5 | Hosted `dash`, cmake 3.28 and gcc 13 behave as in Phase 5 [ASSUMED] | Environment | hosted-only failure class, caught by gate-linux |
| A6 | A PPM export helper in `runner/` (all four runner guards free) is preferable to a new `tools/` directory [ASSUMED, Open Question 3] | Don't Hand-Roll | a `tools/` helper needs the integrity, c-build and runner-scan guards extended (Phase 5 Open Question 3) |

## Open Questions (RESOLVED by recommendation)

1. **Does the STIC read alias stay a stub?** Recommendation: no; flip all five regions to `stic14` (mechanism 4). The alternative (generator change allowing mixed physical devices) buys nothing and adds a rule to `IvsMemmapTable.cmake` and its controls.
2. **Where does the video channel live in the sink?** Recommendation: the existing `ivs_psg_sink` (Pitfall 13); renaming is a separate cleanup.
3. **Where does the operator frame export live?** Recommendation: `runner/` (as a small static TU linked only into `ivs-run`), PPM only; a `tools/` helper needs three guard extensions (integrity anchor, c-build unlisted-source, runner-scan allowance) and cannot own a named fault.
4. **12 or 13 row latches?** Recommendation: 12 (480 bytes); window 12 fetches nothing in Phase 6 (D-09).
5. **Tie rule at `E_k`?** Recommendation: accesses stamped below `E_k` belong to frame k (the PSG convention); record it as a discrepancy with its reversal.
6. **Which `ivs_info` v4 fields?** Recommendation: the 320-byte candidate (includes `video_slack_frames`, which a host needs to size a buffer, and `video_row_lines`); the plan may drop `video_row_lines` for 312.
7. **Event cursor representation.** Recommendation: `event_index` 0..15 plus `frame_index`; the next event tick is computed from the generated table (no stored tick), so a profile perturbation cannot leave a stale stored tick.
8. **Hook table versus `describe`.** Recommendation: generated granule tables (Pattern 2); migrate the PSG test too, in the plan that adds the table.

## Sources

### Primary (HIGH confidence, read or executed this session)
- Repository files with line cites above: `core/include/ivs/{ivs_run,ivs_machine,ivs}.h`, `core/src/machine/{machine.h,machine.c,machine_bound.c,machine_bus.c,machine_digest.c,machine_inspect.c,machine_run.c,machine_input.c}`, `core/src/memmap/{memsys.h,memsys.c}`, `core/src/psg/psg.h`, `core/src/cpu/{cp1610.h,cp1610.c,cp1610_decode.c}`, `core/CMakeLists.txt`, `cmake/IvsMemmapTable.cmake`, `data/profiles/intv-2609-ntsc-family-p1.json`, `data/memmap/intv-2609-ntsc-family-p1.literal.txt`, `data/api/api-effects.json`, `data/expected/**`, `data/golden/*.digests`, `data/scenarios`, `data/cp1610/vectors/busrq-halt.json`, `fixtures/*/*.listing.txt`, `tools/gate.sh`, `tools/gate-linux.sh`, `tools/hygiene.sh`, `tools/check/{api-effects.cmake,listing.cmake,binary-ext.txt,core-symbols-allow.txt}`, `tools/check/steps/{golden,important-gates}.sh`, `tests/{CMakeLists.txt,faults.cmake}`, `tests/run/*.c`, `tests/machine/machine_standin.c`, `tests/memmap/*.c`, `tests/psg/psg_bounds.c`, `runner/{ivs_runner.h,exec.c,report.c,main.c,runner_chain.h}`, `docs/time-base.md`, `docs/runner.md`, `.planning/{AGENTS.md via root,AUTONOMY.md,CI-EFFICIENCY.md,ci-runs.tsv,ROADMAP.md,REQUIREMENTS.md,config.json}`, Phase 5 `05-RESEARCH.md`, `05-VALIDATION.md`, `05-REVIEW.md`, `05-VERIFICATION.md`, the 05 SUMMARY deviation sections, `06-CONTEXT.md`, `06-DISCUSS-{regs,timing,frame,cards,verify}.md`.
- Executed: `sh tools/gate.sh` baseline (PASS, counts above, `gate-time total=42`); throwaway CP-1610 schedule harness (closed form, per-mix cycles, MVO stream, 253 entries, write density); layout harness (static asserts); release throughput, SHA-256 and `ivs_memsys_describe` micro-benchmarks; tool presence checks; `pdftotext` of pdf 191-193 of the cached 1980 catalog.

### Secondary (MEDIUM confidence)
- `.planning/CI-EFFICIENCY.md` hosted figures and Phase 5 unit costs; `.planning/ci-runs.tsv` last-5 medians as quoted by the verify note (gate 106, cpu 336).

### Tertiary (LOW confidence, tagged `[ASSUMED]` above)
- hosted gcc warnings for unwritten code, hosted tool versions, the cost overrun factor.

## Metadata

**Confidence breakdown:**
- Code mechanisms and file:line cites: HIGH (every cite read this session).
- Derived constants (windows, stall closed form, executed cycles, capacity corner, struct arithmetic): HIGH (executed or compiled).
- Plan decomposition: MEDIUM-HIGH (dependencies read from code; task sizes inferred).
- Pitfalls: MEDIUM-HIGH (1 to 11 from source and execution; 12, 14, 18 from recorded history).
- Gate cost: MEDIUM (measured baseline plus Phase 5 unit costs; the phase overrun factor is an assumption).

**Research date:** 2026-11-12
**Valid until:** 2026-12-12, or until plan 06-03a edits `ivs_run.h`, `ivs_machine.h` and `machine_bound.c` (re-read the line numbers then).

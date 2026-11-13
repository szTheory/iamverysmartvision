# Phase 6, decision point: raw framebuffer surface (VID-03) and D8 (geometry, C10)

Status: discuss output (fan-out protocol), not yet adopted. Labels: [OBS] read or ran in this repo or in a cited document in this session, [INF] inference, [REC] recommendation, [UNK] unknown, [BG] background knowledge not re-fetched.

Inputs read [OBS]: AGENTS.md, AUTONOMY.md sections 1-3, PROJECT.md (Decision Register; D2, D3, D7 decided, D8 open), REQUIREMENTS.md (VID-01..06, API-02, API-04, RUN-*), ROADMAP.md (Phase 6, Phase 8), `docs/time-base.md`, `docs/runner.md`, `data/discrepancies.json` (C1-C7, C10, C21, C60 read; last id is C80), `data/profiles/intv-2609-ntsc-family-p1.json` (no video layout fields exist yet; `scanlines_per_frame` 262, `cpu_cycles_per_scanline` 57, `ticks_per_stic_clock` 1, `memmap.region.stic_regs` stub), `core/include/ivs/ivs_run.h` and `ivs_machine.h`, `core/src/machine/{machine.h,machine.c,machine_run.c,machine_bound.c,machine_digest.c}`, the Phase 4 context and `04-DISCUSS-runapi.md` (via `05-DISCUSS-audio.md`), the Phase 5 context and `05-DISCUSS-audio.md`, `.planning/research/{PITFALLS,SUMMARY,FEATURES}.md` (Pitfalls 9 and 10), `runner/*.c` and `tests/installed-consumer/main.c` (how video is wired today). Primary pages read from the gitignored local cache (nothing copied, only facts and the arithmetic below are used): GI 1980 catalog pdf 191 (printed 5-10) and pdf 193 (printed 5-12, rendered and viewed); GI 1982 catalog pdf 462 (printed 9-43) and pdf 464 (same figure). No emulator source was opened (AGENTS.md). Emulator lessons below are [BG] and generic.

No timing note existed in the phase directory when this was written. Every dependency on it is stated as a constraint in F-5, not as an assumption about its numbers.

## 1. Facts that constrain the design

- [OBS] AY-3-8900-1 (NTSC) features, GI 1980 printed 5-10 and GI 1982 printed 9-43, identical text: "8 coordinate addressable foreground objects on a grid of 168H by 104V for AY-3-8900 or 167H by 105V for AY-3-8900-1 of which 159 x 96 are visible positions"; "background display on a matrix of 20H x 12V using 8 x 8 picture elements"; "twelve rows by twenty columns of which 19 are composed of 8 x 8 picture elements and the 20th 7 x 8"; background memory 240 x 14-bit words; "16 digitally selectable colors"; clock 3.579545 MHz.
- [INF] Arithmetic on the page: 19 x 8 + 7 = 159 and 12 x 8 = 96. The printed visible window is exactly the extent of the background matrix, so "159" is not a stray typo: two statements on the same page agree. The object grid minus the visible window is 8 columns by 9 rows (NTSC) and 9 by 8 (PAL, 168 x 104); the grid is the coordinate space of the objects, with off-screen positions, not a larger picture.
- [OBS] Page 5-12 figure (viewed): SR1 pulses once per frame; SR2 pulses a bit over a dozen times per frame; one SR2 period expands into LINE 1 ... LINE 16 each carrying a burst of SR3 pulses. The text says the first line of a row is fetched by SR2+SR3 and "the following fifteen lines to complete the 8 x 8 array are fetched by SR3 alone". [INF] So one card row is 16 TV lines for 8 pixel rows: each display row is 2 TV lines high; 12 x 16 = 192 active lines (the research notes quote 192 active lines of 45 us from the 1977 GIMINI text, not re-read here, [UNK] by me).
- [OBS] Same page: "SR1 indicates that STIC time is complete and that the AY-3-8900-1 has switched to the CPU controlled mode", and the CPU control timing figure shows CPU time before graphic time. [INF] SR1 is the end of the display period and the start of CPU (VBLANK) time. After it the CPU may touch the STIC registers and graphics memory; before it (during graphic time) the STIC owns the graphics bus (BAR/DTB/DWS) and the CPU control bus is ignored ("only validated if the AY-3-8900/8900-1 is in the CPU controlled mode").
- [OBS] `docs/time-base.md`: frames are timeline events and never a modulo; `ivs_info.nominal_frame_ticks` = 4 x 57 x 262 = 59,736 is nominal. [INF] A line is 57 x 4 = 228 master ticks; 159 visible pixels is 159 of 228 ticks if one pixel is one STIC clock (`ticks_per_stic_clock` 1). That ratio is an observation for Phase 8 timing, not a decision input.
- [OBS] Output seam today (`machine_bound.c`, `ivs_run.h`): the request already has `video`/`video_capacity` (offsets 16 and 24, v1); capacity is checked before the first unit as `bytes_per_frame * (max(target - now, 0) / nominal_frame_ticks + 1)` against `ivs_run_env.video_bytes_per_frame`, which `ivs_machine_profile_env` sets to 0 with format NONE ("no frame is fabricated"); NULL + capacity 0 means channel not requested (Phase 5 D-17, amended with "adopt the same rule for video in Phase 6"); refusal reasons RUN_BUFFER_NULL 42, RUN_BUFFER_OVERLAP 43, RUN_BUFFER_SMALL 41 already cover video; `IVS_RUN_F_RESERVED_FRAME = 2` is UNSUPPORTED until Phase 6; `ivs_run_report` v2 is 136 bytes with `video_frames_completed` (offset 56) and `video_bytes_written` (64) already present (written 0); `ivs_info` is 288 bytes with `video_format` (228), `video_bytes_per_frame` (240); output formats are NONE 0 and PSG_LEVELS_V1 1.
- [OBS] Digest row for domain 16 already exists in the instance (`obs.video`: `chain[32]` + `count u64`, 40 bytes, zero until a producer exists; the domain is outside the composite, which is v2 over domains 1..4). Phase 4 D-10 chain rule: `SHA256("IVSR" || domain || chain || u64 index || u32 len || record)`; Phase 5 D-18 warns the index must never be a per-call index.
- [OBS] Phase 5 audio proved the bare frame-slot formula is not an upper bound for a continuous stream (a target one tick short of a frame end plus the overshoot of the last unit); `psg.max_unit_ticks` U = 468 and the generator refuses a frame <= U. The runner cuts one output block of at most 8 MiB per slice; with audio and log that is 71 slots x (15,060 + 34,432) = 3.51 MB at the default 2^22-tick slice (matches the "about 3.5 MB" in `docs/runner.md`).
- [OBS] CI has no EXEC and no real GROM: scenarios use the boot stub and `zero-grom` (2048 zero bytes made by the runner). The core checks image shape only (D-27). Card patterns in CI therefore come from GRAM that the guest writes, or from synthetic project-authored GROM bytes.
- [UNK] Which 7 of the 8 pixels of the 20th card column are shown (the GI page does not say; the research notes carry a community statement that the right pixel is not displayed); the bit order of a card pattern byte (not printed); what the video output shows when the display is not enabled; whether the border (outside the 159 x 96 window) is guest-visible per line; whether the STIC1A (not in this profile) shows a 160th column.

## 2. Options

### 2.1 Raw framebuffer geometry (D8)

| # | Option | Pros | Cons | Verdict |
|---|--------|------|------|---------|
| G1 | The visible-positions window, 159 x 96, one byte per position, rows are display rows | Exactly the tier-1 "visible positions"; no invented pixels; width derives from card geometry (19 x 8 + 7); smallest; MOB clipping (Phase 8) needs no more | Odd width; a host that wants 160 must pad; the 160-column reading needs a re-bless on reversal | Recommended |
| G2 | 160 x 96 with the 160th column a "not displayed" marker | Both readings expressible (the C10 default text); fixed 160 stride | Ships a column the primary page calls invisible; every host must know to crop; the marker lives forever in every slot and hash | Rejected as default; the reversal path is one field (F-2) |
| G3 | The 167 x 105 object grid including never-visible positions | Whole coordinate space | About 12 percent more bytes, 8 columns and 9 rows that can never hold a pixel of picture; collisions in hidden positions are internal STIC state, not pixels | Rejected |
| G4 | 192 TV lines (rows doubled) | Matches the TV line count | Contains no information the 96 rows lack (all STIC output is 1 row = 2 lines); doubles every slot; scaling is host policy | Rejected; the 2-line fact is published as metadata (`video_row_lines`) |
| G5 | Window plus a border margin of N pixels | Border colour per line becomes pixels | N is unknown (TV overscan, 45 us active line); invents geometry | Rejected now; reversal path when per-line border matters (Phase 8) |

### 2.2 Frame boundary (what "complete" means)

| # | Option | Verdict |
|---|--------|---------|
| B1 | End of the display period (SR1: STIC time complete, CPU time begins) | Recommended: the picture the STIC just finished showing; all render inputs the CPU cannot change during display are final; the guest's VBLANK-time edits for the next frame have not happened yet |
| B2 | End of the 262-line TV frame | Rejected: it falls after the CPU's VBLANK-time work, so the render would read the next frame's BACKTAB/GRAM and label it as this frame |
| B3 | Start of the display period (latch, then show) | Equivalent exactness for registers and GRAM; delivers a picture before it is displayed; rejected as less natural for A/V sync and for the "guest cannot react to a frame it has not seen" property; same approximation set |
| B4 | The call target (render when the host stops) | Rejected: slicing-dependent by construction (named fault) |

### 2.3 Pixel format and slot framing

| # | Option | Verdict |
|---|--------|---------|
| P1 | One byte per pixel holding the STIC colour code 0..15, a core-defined value for "not displayed", row-major, no padding, plus a self-describing 32-byte slot header (tick, index, size, flags) | Recommended |
| P2 | Same plane without a header; ticks and index only in the report | Rejected: per-frame ticks of a multi-frame call are lost, the digest chain could not be recomputed from the delivered bytes alone (the audio `audio.chain.check` pattern), dumps are not self-describing |
| P3 | RGB or RGBA | Rejected: bakes the unknown NTSC colour values into evidence (host presentation policy, PITFALLS 10); 3 to 4 times the bytes |
| P4 | Two pixels per byte (4-bit) | Rejected: saves 7.6 KB per slot, costs every consumer an unpack and the blank marker a nibble |
| P5 | The core keeps the last frame and a read call returns it | Rejected: contradicts API-04 ("written into caller-owned buffers") and the Phase 4 request shape, adds 15 KB of instance state, loses frames in multi-frame calls |

### 2.4 Rendering granularity (surface consequences, PIT-9)

| # | Option | Instance state | Verdict |
|---|--------|----------------|---------|
| R-F | Frame-granular: render at the boundary from the state as of that tick | None for pixels (O(1) timeline state) | Recommended for the first pass, with a named approximation set |
| R-R | Row-granular: latch a card row at each row fetch, build the frame across calls | A frame-in-progress plane (15,264 B) plus row latches in the instance, covered by a digest; composite bump | Reversal path; the surface (slot, chain, flags, capacity) does not change |

### 2.5 Digest accumulation for frames

| # | Option | Verdict |
|---|--------|---------|
| H1 | Chain over frame events, one chain step per completed frame over the whole slot (header and plane), index = frames completed since reset | Recommended: frames are discrete state-determined events, so the chain is call independent; recomputable from delivered bytes |
| H2 | Hash only the plane | Rejected for the chain (loses the tick and index); kept as a separate `plane` hash used by asserts (F-9) |
| H3 | Per-call framing | Rejected (Phase 5 D-18) |
| H4 | Hash only frames a buffer received | Rejected (digest would depend on buffer presence) |

## 3. Role lenses (condensed)

- Video architect: the picture is a function of render inputs at one tick; everything the CPU can change during display is frozen by the access windows (VID-01), except BACKTAB (system RAM), so a frame-granular render at SR1 is exact for registers and graphics memory and approximate only for BACKTAB words written during active display (and the STIC-internal per-row effects, Phase 8). Name that set, test it, tag it, limit the claim. Row-granular is the reversal; the surface was designed so that it is not a format change.
- Hardware/EE: use only what the pages print: 159 x 96, 20 x 12, 19 x 8 + 7, 16 colours, SR1 meaning. Everything else (bit order, which 7 pixels, blank output, border) is a tagged policy with a D10 resolver. The 2-lines-per-row fact is read off the figure and arithmetic and tagged inference.
- API/integrator: caller-owned slot buffer, capacity law identical in shape to audio and the log (one formula for all three channels, the runner and consumer already implement it), NULL + 0 = not requested, FRAME stop so a host never computes frame lengths (D-09), geometry from `ivs_info`, never constants.
- QA: oracles must be independent. Expected frames are derived from card definitions by hand (asymmetric patterns so mirror, transpose and off-by-one errors cannot hide), stored width-independently (20 cards = 160 columns; the profile crops), the chain framing is checked by an authoring-time `printf` + `cmake -E sha256sum` construction of a display-disabled frame (all 0xFF plane) with a hand-derived header, geometry is checked against a literal written from the page. Slicing, buffer-presence and refusal invariants as in Phases 4 and 5.
- Security/rights: no EXEC or GROM bytes in any committed expected frame (project-authored GRAM patterns and a synthetic GROM only); frames rendered from user content are derived art and stay operator-local (the helper refuses `content ... user`); every write into the caller buffer is bounded by a capacity proof plus a sink with an overflow flag; width and height come from generator-checked constants.
- DevOps/CI: render plus SHA-256 of a 15 KB slot per frame is about 1 MB/s of emulated time, well below the PSG's per-sample cost [INF, unmeasured]; scenarios of 3 to 6 frames; one new suite; no workflow change; the Phase 5 measurement checkpoint rule applies before the next ABI-heavy plan.
- Product: the owner can run one command and see a PNG of a card test pattern (operator-side, labelled debug palette, not chip truth); no accuracy claim follows from looking.
- DX: a host steps one frame per call with a flag, gets index and tick in the slot header, never hard-codes 159; hosts that need 160 or RGBA do it in their adapter.

## 4. Decisions

Ids F-1.. are proposals for the Phase 6 CONTEXT; the orchestrator renumbers. Proofs are in the text of each decision.

### F-1 Geometry (D8): the raw framebuffer is the 159 x 96 visible-positions window

- The raw framebuffer of profile `intv-2609-ntsc-family-p1` is 159 columns by 96 rows, origin top-left (column 0 is the leftmost visible position, row 0 the top visible row), rows are display rows of the STIC (one row = 8-pixel-card row index x 8 + pixel row = one object-grid vertical unit), not TV lines. [OBS] page figures, [INF] arithmetic 19 x 8 + 7 = 159, 12 x 8 = 96.
- Not in the buffer: the 8 hidden grid columns and 9 hidden rows (the 167 x 105 object grid), any border margin, blanking intervals. Border pixels that fall inside the window (border-extension effects, delay-exposed area) are emitted as the border colour index when they exist (Phase 8). The surrounding border area is host presentation.
- Each row is displayed for 2 TV lines (tag `inference`); this is published as metadata (`video_row_lines`, F-8) and nothing in the core scales or doubles rows (host policy).
- Palette, scaling, aspect correction, RGB values and border painting are outside the core and outside every installed artifact (the D7 pattern).

### F-2 Profile fields, derivation, and the one-field reversal of C10

New profile fields (generator `IvsVideoTable.cmake`, fail-closed like `IvsCtrlTable.cmake`; each with tag, cite, conflict, resolver, trigger as the schema requires). Names are proposals:

| Field | Value | Tag | Cite |
|-------|-------|-----|------|
| `video.card_columns` | 20 | tier1-primary-doc | GI 1980 printed 5-10, GI 1982 printed 9-43 |
| `video.card_rows` | 12 | tier1-primary-doc | same |
| `video.card_size_px` | {"w": 8, "h": 8} | tier1-primary-doc | same ("8 x 8 picture elements") |
| `video.last_card_column_px` | 7 | tier1-primary-doc (the 20th column is 7 x 8) | same; conflict C10; **the one-field reversal** |
| `video.last_card_column_drop` | "right" | unknown-default-chosen | the page says only 7 x 8; a community statement in the research notes says the right pixel is not displayed; resolver D10 (draw a pattern in card column 19 and capture) |
| `video.object_grid` | {"w": 167, "h": 105} | tier1-primary-doc | same; consumed by the generator sanity check only (visible window <= grid) and by Phase 8 |
| `video.colors` | 16 | tier1-primary-doc | "16 digitally selectable colors" |
| `video.row_lines` | 2 | inference | printed 5-12 figure (LINE 1..LINE 16 per card row) and the 8-row card |
| `video.blank_value` | 255 | unknown-default-chosen | policy: the value emitted when nothing is displayed (F-3, C-new-2) |

- Derived by the generator (never a profile input): `width = (card_columns - 1) x card_w + last_card_column_px = 159`, `height = card_rows x card_h = 96`, `stride = width`, `plane_bytes = 15,264`, slot and per-frame figures (F-4, F-7). Generator checks: `1 <= last_card_column_px <= card_w`, `width <= grid.w`, `height <= grid.h`, `blank_value >= video.colors`, `2 x max_unit_ticks < nominal_frame_ticks`.
- One-field reversal: set `video.last_card_column_px` to 8. Width becomes 160, stride 160, plane 15,360, slot 15,392, per-frame reservation 30,784. Nothing else is edited: info, capacity, runner buffers, the consumer and the geometry checks read the derived figures. What must change on reversal is only data that records pixels: runner goldens (`.digests`, `.expect` frame asserts) are re-blessed as one classified diff (a column added, digests moved), and the reversal is a new claim revision, not a silent edit. The independent expected frames do NOT change, because they are stored 20 cards wide (F-15) and cropped by the profile.
- C10 update (status `open` -> `partial`, not resolved): decided by D8 on tier-1 grounds; the 160-column reading (Blue Sky nominal grid, STIC1A, "192v x 160h" community text) stays recorded; default policy line rewritten to name `video.last_card_column_px = 7`; resolver unchanged in kind (D10 capture, owner-gated). Related new unknowns get their own entries (provisional ids C81.. ; the orchestrator owns one allocation table, as in Phase 5 D-12): which 7 pixels (`last_card_column_drop`), blank output of a disabled display, frame boundary tie rule, rendering-granularity approximation set, border outside the window, window origin inside the 167 x 105 grid (Phase 8), frame spacing is nominal-minimum. D10 items proposed: `D10-VID-01` card column 19 pixel 7 (159 vs 160), `D10-VID-02` which 7 pixels, `D10-VID-03` output while the display is not enabled, `D10-VID-04` relation of the VBLANK interrupt to the end of the picture, `D10-VID-05` effect of a BACKTAB write during active display. All marked missing; no measured-tier claim.

### F-3 Pixel values and what the core emits for each region

- Value space (pinned by `data/expected/video-pixels.ids`, append-only): `0..15` the STIC 4-bit colour code of the position (primary and pastel sets are an interpretation of the code, never expressed in the core); `0xFF` = `IVS_PIXEL_BLANK`, "not displayed" (the display was not enabled for that frame, F-11); `16..254` reserved, never emitted, a consumer treats them as invalid. `0xFF` rather than 16 so a 256-entry host palette cannot read out of range and a 16-entry one fails loudly in tests.
- Card pixels: the foreground or background colour code per the card mode rules (FG/BG and Color Stack are the render agent's decisions; this surface only fixes that the result is a colour code 0..15). With no MOBs (Phase 8) every one of the 15,264 positions is covered by the 20 x 12 matrix, so the first pass never emits a "gap" value.
- Border inside the window: the border colour code (Phase 8; none in the first pass). Border outside the window: not emitted.
- Blanking inside the window: none while the display is enabled. Display not enabled for the frame: every position `0xFF` and the slot flag `displayed` clear (F-11).
- "GROM card in CI" (no real GROM): the zero GROM image gives all-zero card patterns, so GROM-numbered cards render solid background; that exercises the fetch path only. Foreground shapes come from guest-written GRAM or a synthetic project-authored GROM (F-15).

### F-4 Output format id and slot layout

- `IVS_OUTPUT_FORMAT_INDEX8_FRAMES_V1 2u` appended after `PSG_LEVELS_V1 1u` (0 stays NONE). Published by the profile as `video_format`.
- One frame slot = 32-byte header followed by the pixel plane, all multi-byte header fields written byte by byte big-endian (host independent, like the PSG log). Plane: `height` rows of `stride` bytes, row-major, top row first, left to right, `stride == width`, no padding bytes (padding would need defined values and a hash rule; hosts that need an aligned pitch copy row by row using `video_stride_bytes`). Slot bytes for this profile: 32 + 159 x 96 = 15,296 (= 64 x 239, plane 15,264 = 32 x 477, so the plane starts and ends 32-aligned).
- Header (32 bytes):

  | Offset | Size | Field | Meaning |
  |--------|------|-------|---------|
  | 0 | 8 | `frame_tick` | the master tick T_k at which the frame completed (F-5), exact, not the call's exit time |
  | 8 | 8 | `frame_index` | k, frames completed since reset before this one (0 for the first); continuous across calls |
  | 16 | 2 | `width` | 159 |
  | 18 | 2 | `height` | 96 |
  | 20 | 1 | `flags` | bit 0 `IVS_FRAME_F_DISPLAYED` (the display was enabled for this frame); bits 1..7 reserved, written 0 |
  | 21 | 11 | `reserved` | written 0 |

  Reserved bytes are assigned only by appending a flag bit that says a field is meaningful; a consumer ignores unknown flag bits. A change to header size, field order or pixel meaning is a new format id.
- `#define IVS_FRAME_HEADER_BYTES 32u`, `IVS_FRAME_F_DISPLAYED 1u`, `IVS_PIXEL_BLANK 0xFFu`, `IVS_PIXEL_COLOR_MAX 15u` in `ivs_machine.h` next to the format list; the constants are pinned by an ids file and a consumer compile-time check.

### F-5 The frame boundary: a pure function of machine state at a declared tick (my constraint on the timing note)

- Frame k completes at master tick T_k, the STIC timeline event "end of display period" (the SR1 instant of the primary pages: STIC time complete, CPU time begins). The timing note owns where T_0 is and the constant spacing; this decision owns what the event must satisfy:
  1. T_k is derived from STIC timeline state held in the instance (digested), never from a call target, a slice boundary or a buffer. Frames are timeline events; no modulo of machine time anywhere.
  2. The frame is the picture displayed during the display period that ends at T_k. Its pixels are R(state as of T_k) together with latches set earlier in the same frame (for example "display enabled for this frame"). It must never see an effect of the VBLANK-time CPU work that follows T_k (B2 is rejected for exactly this reason).
  3. "State as of T_k" is made exact by one catch-up rule, the PSG discipline of Phase 5 A-02: the STIC is caught up (frame rendered if T_k <= the stamp) before any bus access stamped s that can change a render input is applied (STIC register write or side-effecting read, BACKTAB and other system RAM words, GRAM), and at the end of every unit; never to the call target. A write stamped exactly T_k is not seen by frame k (step first, then write; same tie rule as the PSG, tag `unknown-default-chosen`, resolver D10). A unit that straddles T_k therefore renders at the first of: a hooked write stamped >= T_k, the unit end. Both give the same pixels because no hooked access lies between them.
  4. Rendering writes only: the sink (caller buffer or counting), `obs.video` (chain, count) and STIC timeline state (next boundary, display latch advance). It never changes a render input. What it changes is identical with and without a buffer.
  5. Spacing: `nominal_frame_ticks` is published and generator-asserted as a lower bound on the distance between two boundaries (equal to the spacing in the first profile if the timing note keeps the STIC free-running). Constant spacing is a timing-note outcome; the surface (header tick, capacity law) needs only the bound.
  6. [REC, not required] if the timing note raises the VBLANK interrupt at T_k, a host and a guest see the frame and the interrupt at the same machine time, which gives tests a clean statement: the frame is complete exactly when the guest could first react to it.
- How a host obtains it: a run whose execution crosses T_k delivers the frame (slot appended to the video buffer, `video_frames_completed` counts it, digest chain advances); a host may bound a run by `target_ticks` or ask for the frame stop (F-6). There is no "render now" call in v1 (deferred idea: a const, side-effect-free `ivs_machine_render_preview`, useful to debuggers; it must not exist before the surface is proven).

### F-6 Frame stop: `IVS_RUN_F_FRAME_STOP` and `IVS_STOP_FRAME`

- Flag value 2 (today `IVS_RUN_F_RESERVED_FRAME`, UNSUPPORTED): renamed `IVS_RUN_F_FRAME_STOP`. With it a run ends at the first unit boundary at which this call has completed at least one frame.
- New stop reason `IVS_STOP_FRAME 5` appended to `data/expected/api-stop-reasons.ids` and `ivs_run_stop_name`. The reason stays a pure function of exit state, checked in this order: INTERNAL, GUEST_FAULT, CPU_HALTED (with its flag), FRAME (with its flag, a frame completed in this call), TARGET.
- The flag counts frames completed by this call, so a call that starts exactly at a boundary does not stop at once (no stuck loop). The target still bounds the call: a call whose target is reached before any frame completes ends TARGET with zero frames.
- Slicing: the flag changes where calls end, not what the machine does; any sequence of calls gives the same final state (tested like every other slicing pattern).
- At most one frame per flagged call: a unit is at most U ticks and a frame spacing is at least F, and the generator asserts 2U < F; the flagged capacity rule (F-7) relies on it.
- Why a flag at all: D-09 forbids deriving frame length from the nominal figure; without the flag a host steps frames by target arithmetic (`now + nominal_frame_ticks`, which works only while spacing is constant) and must size for two slots. Cost: one condition in the loop, one stop reason, one test family. Reversal: leave the flag reserved and let hosts use targets (F-7 keeps the capacity law correct either way).

### F-7 Buffer contract, capacity law, refusal

- A video buffer is optional per call exactly like audio (Phase 5 D-17): NULL with capacity 0 means "not requested": no capacity check, no byte written, and the machine state, `obs.video` and every digest are byte-identical to a run with a buffer (named fault `VIDEO_STATE_DEPENDS_ON_BUFFER`). A nonzero capacity must meet the requirement, else `BUFFER_TOO_SMALL` (`RUN_BUFFER_SMALL`, existing reason 41) before the first unit with nothing changed and no byte written; NULL with nonzero capacity is `RUN_BUFFER_NULL` 42; overlap with the instance, other buffers, the report or the event array is `RUN_BUFFER_OVERLAP` 43. No new refusal reasons. Every refusal precedes any state change.
- Frames are delivered whole or not at all: a slot is never partially written; bytes after `video_bytes_written` are untouched; each call writes from byte 0 of the buffer.
- Capacity law (the Phase 4 shape, unchanged): `required = video_bytes_per_frame * (max(target - now, 0) / nominal_frame_ticks + 1)`, with `video_bytes_per_frame = 2 x slot_bytes = 30,592` published in `ivs_info` (the slack is absorbed per slot, as in AU-05; `video_slot_bytes` is the parse unit).
- Proof [INF]. Let F be the minimum spacing, U the maximum ticks of one unit, D = target - now. If D = 0 no unit runs and no frame completes. Otherwise the last unit starts below the target and ends at E <= target + U - 1; the frames completed by the call are those with T_k in (now, E], at most `floor((E - now) / F) + 1 <= floor((D + U - 1) / F) + 1`. Because U - 1 < F, `floor((D + U - 1) / F) <= floor(D / F) + 1`, so at most `floor(D / F) + 2` frames. The law grants `2 x (floor(D / F) + 1) = floor(D / F) + 1 + floor(D / F) + 1 >= floor(D / F) + 2` slots. The bare one-slot formula would fail for D mod F >= F - U + 1 (a target a few ticks short of a boundary plus the last unit's overshoot completes two frames), the same defect Phase 5 found for audio; a factor of 2 is the least integer slack that works for D < F. With U = 468 and F = 59,736 the generator check 2U < F holds with a wide margin.
- Flagged runs: with `IVS_RUN_F_FRAME_STOP` the call completes at most one frame (F-6), so `required = video_slot_bytes = 15,296`; the validator uses `min(formula, 1 slot)` when the flag is set. A capacity of `slot_bytes - 1` is refused; exactly `slot_bytes` is accepted.
- Defense in depth (never expected to fire): before a unit begins, when a buffer is requested and the next boundary lies within U ticks of the machine time, the loop checks that one more slot fits; if not it ends with `IVS_STOP_INTERNAL` at that whole boundary (state consistent, a harness failure), the existing pattern of the write log. Named fault shows it fires when the reservation is cut.
- Sizing numbers [INF]: default runner slice 2^22 ticks is 71 slots = 2,172,032 bytes of video; with audio 1,069,260 and log 2,444,672 the block is 5,685,964 bytes (5.42 MiB), inside the 8 MiB runner cap. A host that single-steps with the flag needs 15,296 bytes.
- Overflow of the multiplication is guarded exactly as `ivs_bound_fits` does (64-bit, `frames > UINT64_MAX / bytes_per_frame`).

### F-8 Report and info growth (append-only, A-02)

`ivs_info` v4, appended after the 288-byte v3 (written only when `struct_size` covers it; `IVS_INFO_V1_SIZE` stays 216):

| Offset | Field | Value on the NTSC profile |
|--------|-------|---------------------------|
| 288 | `video_width` u32 | 159 |
| 292 | `video_height` u32 | 96 |
| 296 | `video_stride_bytes` u32 | 159 |
| 300 | `video_header_bytes` u32 | 32 |
| 304 | `video_slot_bytes` u32 | 15,296 |
| 308 | `video_row_lines` u32 | 2 |

sizeof(ivs_info) becomes 312 (a multiple of 8). Existing fields change meaning only here: `video_format` = `IVS_OUTPUT_FORMAT_INDEX8_FRAMES_V1`, `video_bytes_per_frame` (offset 240) = 30,592 (the reservation per frame slot, not the parse unit), `nominal_frame_ticks` documented as the minimum spacing of two boundaries. A host never hard-codes 159, 96 or 15,296.

`ivs_run_report` v3, appended after the 136-byte v2 (written only when `rep->struct_size` covers it; `IVS_RUN_REPORT_V1_SIZE` stays 112):

| Offset | Field | Meaning |
|--------|-------|---------|
| 136 | `video_first_index` u64 | frames completed before this call (the index of the first frame this call completes); the slots of consecutive calls concatenate to the stream of one call |
| 144 | `video_last_frame_tick` u64 | T_k of the latest completed frame in the instance, 0 before any (T_0 > 0); useful to buffer-less digest runs |

sizeof(ivs_run_report) becomes 152. Existing fields: `video_frames_completed` = frames completed by this call (counted whether or not a buffer was requested, so a host can see that it omitted the buffer); `video_bytes_written` = slot bytes delivered into the caller buffer (0 when not requested), `video_bytes_written == frames x video_slot_bytes` when requested. The request struct is unchanged (the video fields are already v1); the `static_assert` pins in `machine_bound.c` and the pinned-size constants gain the v3 lines.

### F-9 Digest domain 16 (VIDEO), plane hash, composite and instance state

- Domain 16 goes live at domain version 1 (first producer; the Phase 4 zero row had no producer). Rows are the existing `obs.video.chain[32]` and `obs.video.count u64`: no new instance bytes for the digest.
- Chain, one step per completed frame, `slot = header || plane` (15,296 bytes), `index` = `frame_index` = count before the increment:
  `chain0 = 32 zero bytes`; `chain' = SHA-256('I' 'V' 'S' 'R' || 16 || chain || be64(index) || be32(slot_bytes) || slot)`; `count' = count + 1`.
  Domain digest: `SHA-256('I' 'V' 'S' 'D' || 16 || be16(1) || be32(32) || chain || be64(count))`. Documented in `ivs_machine.h` beside domain 17, so a host can recompute it from the delivered slots without core state.
- Why it is slicing invariant: each step is over a frame whose bytes are a pure function of state at T_k and whose index is the global count; no per-call value enters. The core renders every frame whether or not a buffer was given (row by row into a 159-byte row buffer plus a streaming SHA-256 context on the stack, copied to the caller buffer when requested), so the chain never depends on a buffer. No hash context or plane lives in the instance (F-10).
- Plane hash: SHA-256 of the 15,264 plane bytes alone is the hash used by scenario asserts and goldens (`assert frame <n> hash`), because an independent author can build a plane with shell tools and hash the file with `cmake -E sha256sum`; the chain is the engine-side digest, the plane hash is the oracle-side one. They are different values on purpose and both are documented.
- The VIDEO domain stays outside the composite (D-10). The STIC device state (register file in MEM, timeline and latches in INSTANCE) is guest-visible and joins domains 2 and 4, so the composite goes to version 3 by a deliberate bump as in Phase 5 D-18: pin the v2 composite literals first (`digest-v1-literals.ids` pattern), then classify the golden change (only composite, mem and instance lines may differ). That bump belongs to the STIC state plan; this surface adds nothing to it.
- Instance size: `IVS_MACHINE_BYTES` changes only for STIC state; this decision adds no bytes (R-F keeps no pixel storage). If R-R is ever adopted the frame-in-progress plane is a new digested row set (a domain 16 preimage change, version 2) and a composite bump.

### F-10 Rendering granularity: frame-granular at the boundary, approximation named

- First pass renders the whole frame at T_k from the state as of T_k (R-F). Exact, given the access windows, for STIC registers and graphics memory; approximate for the set A = { a BACKTAB word written by the CPU during active display (the render shows the value at T_k for every row, hardware shows the value at that row's fetch), a late bus acknowledgement that repeats or shifts a row, any per-row STIC-internal effect }. Each member is a profile-tagged policy (`unknown-default-chosen`, C-new-3), has a fixture that documents the difference ("documented and may change"), and bounds the claim: "card rendering is frame-granular at the end-of-display boundary", never "row accurate".
- The surface is renderer agnostic by construction: slot format, header, chain, flags, capacity law and report fields do not change if a row-granular model replaces the renderer; only the instance state set (a frame-in-progress plane and row latches, digested) and the composite version do.
- Reversal trigger: any fixture or D10 capture showing a mid-display BACKTAB edit effect a guest depends on, or Phase 8 vertical delay, border and MOB effects that need per-row state; Phase 7's scenario is a CPU-time-update program, so it needs none of this.

### F-11 Display not enabled, and the first frames

- A frame completes at every boundary whether or not the display was enabled for it (frames are timeline events; skipping them would tie the frame count to guest state and break A/V sync). When the display was not enabled the slot has `flags` bit 0 clear and every pixel is `0xFF`. This is a policy (`video.blank_value`, `unknown-default-chosen`, C-new-2, D10-VID-03): what a real unit shows then (black, border colour, sync only) is not in the primary pages read. Hosts choose how to present a not-displayed frame.
- Reset clears `obs.video` (count 0, chain zero) and the STIC timeline; the first frame completes at T_0 with index 0 and, with no guest code having enabled the display, is a blank frame. This makes the first frame of every run a deterministic, independently derivable observation (F-15 oracle).

### F-12 Runner (ivsr 3, scenario syntax, options, goldens)

- Report format `ivsr 3` (key table closed, fixed order, so added keys are a format change; bump once for the whole phase, coordinate with the STIC-state keys): new keys after `output.video.bytes`: `output.video.frames` (frames completed, from the report counters), `video.geometry` (`159x96`, from `ivs_info`), `video.row_lines`, `video.frame.last_tick` (`-` before any), `video.chain.check` (`pass`, `fail`, or `-` when a video buffer was not delivered on every slice). `digest.video` already exists. A failed check is `harness-fail` with detail `video-chain`. The runner recomputes the chain from the slots it received, verifying also header consistency (index continuity, width and height equal to `ivs_info`, reserved bytes zero, pixel values only 0..15 or 0xFF, `displayed` consistent with the pixels, ticks strictly increasing and at least `nominal_frame_ticks` apart).
- IVSS v1 additions (line based, keywords append-only, no directive names a file or a palette):
  - `capture video <first> <count>`: keep delivered frames with index `first .. first + count - 1` (count at most 16, about 245 KB); outside the window frames are not kept, so there is no overflow class, and a frame assert outside the captured window is a harness failure `frame-not-captured` (never vacuous).
  - `assert frame <n> pixel <x> <y> ==|!= <0..15|blank>`; `assert frame <n> rect <x0> <y0> <x1> <y1> ==|!= <0..15|blank>` (every pixel of the inclusive rectangle); `assert frame <n> hash ==|!= <64 hex>` (plane hash, F-9); `assert frame <n> tick ==|!= <ticks>`; `assert frame <n> displayed ==|!= yes|no`; `assert video frames ==|!= <n>` (frames completed so far).
  - `run-to <n> frame-ends [stop frame]`: run with the frame stop until n frames have completed since reset (stop defaults to `frame`); the existing `run-to <n> frames` stays the nominal conversion (D-09). This gives scenarios the exact post-frame unit boundary for register and memory checkpoints.
  - Caps: a capture window of at most 16 frames per scenario; assert caps unchanged; a scenario with `capture video` or a frame assert run without a video buffer is a harness failure `stream-without-buffer target=video`, never a vacuous pass.
- Goldens: the `.digests` key list gains `digest.video`, `output.video.frames`, `output.video.bytes`, `video.geometry`, `video.frame.last_tick`; header rule as audio: domain `video=observed` iff the profile video format is not NONE. Frame asserts live in `.expect` (tier `original-diagnostic`, oracle `independent-derivation`); digests are `deterministic-repeatability`, `self-recorded`. A reversal of C10 is a classified diff of exactly these files.
- Operator-only options (never scenario controlled, written only in `hostio.c`, exclusive create, byte cap): `--video-raw <file>` (delivered slots concatenated, a self-describing stream because every slot carries a header), `--video-out-max-bytes <n>` (default 268,435,456; exceeding it is a harness failure `video-out-max-bytes`, files removed as for the audio options), and the PNG option of F-13. At 15,296 bytes a minute of emulated video is about 55 MB, which is why the cap is explicit.
- Budget note for the runner block: the video channel adds 2,172,032 bytes to the default slice (F-7); a slice-ticks value is unchanged.

### F-13 Operator-side picture output (outside the core, D7 pattern)

- `tools/frame/ivs_refframe.h/.c`: pure, integer, no I/O, no allocation, no globals; converts one raw slot plane to an indexed PNG (colour type 3, a 17-entry PLTE of a project-chosen debug palette, 0xFF the 17th entry, uncompressed "stored" deflate blocks, CRC-32 and Adler-32 written by hand, about 120 lines) with an integer scale of x k horizontally and x 2k vertically (the row-lines fact; nearest neighbour; presentation only). Linked only into the runner's `--frame-png <file> [--frame-index <n>]` (default: the last frame) and its own tests. Labelled "debug palette, not NTSC, not chip truth", never a golden, never a claim, never CI-uploaded, never committed. A cheaper fallback if the PNG writer exceeds the budget is binary PPM (P6) with the same palette.
- Guards, as Phase 5 A-07: the converter sits outside runner-scan, the c-build unlisted-source guard and the install inventory, which are extended deliberately; the symbol deny list gains `refframe`, `png`, `ppm`, `palette`; `pkg-install-files.ids` proves the install prefix has none. No palette or RGB value exists in `ivs_core` or in any installed artifact.
- `tools/look-demo.sh` (local helper, not a gate step): builds `dev`, runs the card diagnostic with `--frame-png <scratch>/cards.png`, prints size and `sha256`; it refuses `content ... user` scenarios (frames rendered from a real GROM are derived art of rights-bearing content). This is the one owner handoff for subjective look (does the card test pattern look plausible), recorded as a note, never a tier.

### F-14 Installed consumer and replay

- `tests/installed-consumer` requests a video buffer sized from `ivs_info` (`video_bytes_per_frame` x slots, never hard-coded) on the card scenario, with the frame stop on a second pass, and prints `digest.video`, the report counters, the parsed first-slot header and a few literal pixels and rectangles taken from the hand derivation; the gate compares the digest line with the runner golden for the same scenario (D-39 pattern). The consumer has no SHA-256 of its own and keeps none (copy-paste rule: the chain recomputation stays in the runner); it checks `video_bytes_written == frames x video_slot_bytes`, index continuity, ticks strictly increasing and the literals. `pkg-consumer-calls.ids` is unchanged (no new function); the header constants are compile-time checked from the installed header.

### F-15 What "no real GROM in CI" means for buffer-content tests, and the oracle design

- Inputs for content tests: (a) guest programs that write GRAM cards and BACKTAB words during CPU time (hand-assembled decle listings, D4 stands; the Phase 7 trigger for a project assembler is already recorded) with the boot stub and `zero-grom`; (b) a synthetic project-authored GROM (2048 bytes from a documented formula, a build output, listed in the fixture manifest as `redistributable: yes`, labelled "synthetic, not a GROM") used only in-process by `tests/video` to exercise card numbers that index GROM; never a real image, never committed binary.
- Expected frames are independent of the renderer in two ways: structurally (a frame is 20 x 12 cards, each card an 8 x 8 pattern with two colours; the expected plane is built by a throwaway authoring script, or by hand for the smallest cases, directly from the card definitions) and by file shape: `data/video/expected/<id>.frame.txt` holds one hex digit per pixel, 96 lines of 160 characters (20 cards x 8), and the test crops to the profile width. Asymmetric glyphs (an "F" or an arrow, a one-pixel marker in each corner of each card region) are mandatory so mirroring, transposition and off-by-one errors cannot cancel; the 20th column fixture puts a marker in pixel 6 and in pixel 7 of card 19 so both clip choices produce a different, hand-derivable result.
- The plane hash for scenario asserts is the SHA-256 of the same file's cropped bytes built with shell tools (the Phase 4 A-08 technique), compared with the core's frame in the runner and in-process.
- The first frame of a display-disabled boot is the cheapest end-to-end oracle of the whole surface: plane = 15,264 bytes of 0xFF (`head -c`, `tr`), header from `printf` with T_0 taken from the timing note's literal derivation, chain0 and chain1 by `cmake -E sha256sum`; it proves header, plane, framing and index without any renderer.
- Evidence tiers: geometry claim `source-expectation` (tier 1 pages), standing `provisional`; card render claims `original-diagnostic`, `provisional`; the surface (slicing, buffer presence, capacity) `deterministic-repeatability` plus `installed-host-behavior`; no `measured-hardware`. Shared-core agreement is not an oracle: a hash recorded from this core is repeatability only. Bit order, clip side and blank output are tagged defaults and named in the claim text.

### F-16 Test inventory, named faults, controls (proposal for the surface; the render and timing agents add their own)

- Suite `video` (surface part), about 40 ids: `video.geometry.profile-derives-159x96`, `.literal-matches-page` (profile vs `data/video/geometry.literal.txt` written from the pages), `.reversal-one-field` (generator with 8 gives 160 and the derived figures follow), `.info-matches-profile`; `video.slot.header-bytes` (hand-written header), `.plane-stride`, `.blank-frame-all-ff`; `video.boundary.frame-tick-header`, `.frame-pure-of-state` (state at T_k reproduces the frame in a twin instance), `.write-straddling-boundary` (a store whose write stamp falls before, at and after T_k, swept over every cycle of the instruction: the Phase 5 C67 sweep pattern), `.tie-rule`; `video.slice.*` (slicing 1, 7, 4096, random, 2^22; sequence of FRAME-stop calls; whole-instance memcmp at every boundary; summed frame counts; concatenated slot streams equal one call byte for byte); `video.buffer.not-requested-equals-requested` (whole-instance memcmp, digests equal), `.exact-fit`, `.undersized-refused` (poison sentinel, nothing written), `.null-nonzero`, `.overlap-*`, `.flag-capacity-one-slot`, `.worst-case-two-frames` (a call crafted so a target a few ticks short of a boundary completes two frames: needs 2 slots, so the 2x law is exercised, not just stated), `.capacity-overflow-guard`; `video.chain.framing-oracle` (printf + cmake sha), `.chain-recompute-from-slots`, `.index-continuity`, `.reset-clears`; `video.stop.frame-reason-order`, `.no-stuck-loop-at-boundary`, `.frame-flag-with-halt`; `video.fuzz.*` (extend the run fuzz seeds with video buffer sizes and the flag).
- Named faults (at most 8 per plan, a file each): `VIDEO_FRAME_AT_CALL_END` (render at the end of the call instead of T_k), `VIDEO_STATE_DEPENDS_ON_BUFFER`, `VIDEO_INDEX_PER_CALL`, `VIDEO_SLOT_RESERVE_ONE` (reservation cut to one slot), `VIDEO_OUTBUF_PARTIAL_WRITE` (a refused run writes a byte into the video buffer), `VIDEO_BLANK_AS_ZERO` (disabled frame filled with colour 0), `VIDEO_CHAIN_OMITS_HEADER` (tick not hashed), `VIDEO_WIDTH_160_HARDCODED` (a consumer path hard-codes a width), `VIDEO_FRAME_STOP_LATE` (flag ignored until the target), `VIDEO_RENDER_SKIPS_WRITE_HOOK` (render only at unit ends).
- Controls (gate fails when the fault is applied): the faults above each map to the matching family; the geometry literal and the info check each have a mutation control (`-DMUTATE=` hook as in the controller generator).

## 5. Proposed Decision Register text (D8)

| Gate | Decision | Would reverse it |
|------|----------|------------------|
| D8 | The raw framebuffer of `intv-2609-ntsc-family-p1` is the 159 x 96 visible-positions window of the AY-3-8900-1 (GI 1980 printed 5-10, GI 1982 printed 9-43: 159 x 96 visible of a 167 x 105 object grid; 19 x 8 + 7 = 159), rows are display rows (2 TV lines each, a tagged inference published as metadata), one byte per position holding the 4-bit STIC colour code 0..15 or 0xFF for not displayed; palette, scaling, aspect, border painting and PCM-like conversions are outside the core. A frame completes at the end of the display period (SR1) and is delivered as a 32-byte-header slot with its tick and index, into a caller-owned buffer (NULL + 0 = not requested), with a frame-stop run flag; the VIDEO digest is a per-frame chain over slots, slicing invariant. The width derives from one profile field (`video.last_card_column_px`, 7); C10 becomes `partial`. | A tier 1 to 3 source, a measurement (D10-VID-01 and -02) or a profile variant (STIC1A) showing a visible 160th column or the other 7-pixel clip: set the field to 8 (and the clip field) and re-bless as one classified diff; evidence that per-line border or MOB-in-border effects need pixels outside the window: a margin or a new format id; a mid-display BACKTAB effect a guest depends on: row-granular rendering behind the same surface |

Also record: C10 `status: partial` (not resolved-primary: the primary page supports 159, the alternatives remain recorded); seven new discrepancy entries (F-2) and five D10 ids; `docs/video.md` states the surface; `docs/time-base.md` glossary gains **Raw framebuffer**, **Frame slot**, **Frame boundary** (end of display period), **Display row**, and its Frame entry gains the statement that the boundary is a STIC timeline event whose tick is in the slot header.

## 6. Footguns and anti-patterns avoided [BG unless noted]

- Magic 160, 192 and 240 in the framebuffer path, RGB goldens and edge "fixes" applied by editing the golden (PITFALLS 10): geometry comes from derived profile data, goldens are indices, expected frames are stored 20 cards wide and cropped.
- Rendering after the VBLANK-time CPU work (a "late render" that shows the next frame's edits): the boundary is the end of display, with an exact catch-up rule.
- Render-on-host-demand or render-at-call-end: slicing dependent by construction; the render happens at T_k or never.
- A frame count tied to guest state (skipping frames while the display is off): frames are timeline events; the flag says whether they displayed anything.
- Pixel buffers whose content depends on whether the host supplied a buffer: the core always renders and hashes, the buffer is a sink.
- Hard-coded host assumptions that a call equals a frame (the libretro habit): a call is a time bound; the frame stop is opt-in.
- Baking palette, aspect and scaling into the core: palette and scaling are host policy; the row-lines fact is metadata only.
- Per-call framing in the chain: avoided (global index, state-determined records).
- Unbounded retention: nothing retained in the instance; the runner caps captures and output files.
- Copying tables, comments or constants from other emulators: none used; every figure is a page figure, arithmetic on one, or a tagged default.

## 7. Adversarial pass (against this recommendation)

1. "159 is a datasheet quirk; real STIC output shows 160." The page states 159 x 96 visible and the 20th column 7 x 8; the two statements agree arithmetically; the STIC1A and community statements are about another part or are unsourced. The cost of being wrong is one field and a classified re-bless; the claim is `provisional`; D10-VID-01 resolves it.
2. "The 32-byte header is over-engineering; hosts want a bare plane." The plane starts at a fixed 32-aligned offset published in `ivs_info`, so a host copies from it; the header is what lets per-frame ticks survive multi-frame calls and lets the chain be recomputed from delivered bytes. Reversal: a plane-only format id with ticks in a side channel.
3. "Frame-granular rendering will mis-render games that edit BACKTAB during display." True and named (set A); first-pass fixtures and Phase 7 update in CPU time; the surface does not change if it is replaced; the claim wording prevents overclaiming.
4. "A 2x reservation wastes memory." 30,592 bytes per slot, 2.2 MB per default runner slice, 15,296 bytes when single-stepping with the flag; proof in F-7 shows 1x is wrong near a boundary. A tick-tight law publishing the exact slot count is deferred until a host objects.
5. "Hashing every frame even without a buffer costs wall time." About 0.9 MB per emulated second against 224k PSG samples per second [INF, unmeasured]; the Phase 5 measurement checkpoint applies; if it is not negligible, the row-by-row streaming hash is already the cheap form.
6. "The chain is not comparable with a hash of a dumped video file." The plane hash is; both framings are documented, and the runner proves the delivered bytes reproduce the core's chain on every run that delivers video.
7. "Blank frames for a disabled display are fake output." They keep frame index and tick continuous and carry a flag; hosts may skip presenting them. The alternative (no frame) ties timeline events to guest state.
8. "FRAME stop adds API surface for nothing." It is one flag the reserved bit already named, one stop reason and the only way for a host to step frames without frame-length arithmetic; reversal is cheap.
9. "A frame completing inside a unit must see a write stamped after T_k in the same unit." That is why the catch-up hooks sit before state-changing accesses, not only at unit ends; `video.boundary.write-straddling-boundary` sweeps a store across T_k over every cycle and `VIDEO_RENDER_SKIPS_WRITE_HOOK` must fail it.
10. "U may grow with STIC bus-request stalls, breaking 2U < F and the one-frame-per-flagged-call claim." Then the generator refuses the profile (it fails closed); the stall model must re-derive `psg.max_unit_ticks` (Phase 5 flagged this); 114 CPU cycles of hold is 456 ticks, still far below F.
11. "Expected frames from the author's own understanding share the author's misreading." Acknowledged: bit order and clip side are tagged defaults; asymmetric glyphs and corner markers catch transposition, mirroring and off-by-one errors but cannot tell a mirrored bit order from the true one; only D10 can.

## 8. Reversal triggers

| Decision | Revisit when |
|----------|--------------|
| F-1/F-2 159 columns, one-field reversal | D10-VID-01/02 or a tier 1 to 3 source shows 160 or the other clip; a STIC1A profile is requested (new profile id, not an edit) |
| F-1 no margin | Per-line border colour or MOB-in-border effects need pixels outside the window: margin fields or a new format id (Phase 8) |
| F-3 0xFF blank | A real unit's disabled-display output is measured (then a defined colour or a flag-only rule) |
| F-4 header in slot | A host needs bare planes only: new format id with ticks in a side channel |
| F-5 boundary at SR1, tie rule | A schematic or capture shows the VBLANK interrupt or access windows relate differently to the end of the picture; the timing note changes T_k |
| F-6 frame stop | Hosts do not use it: remove; or hosts want N-frame stops: `frame_count` field in the request (append) |
| F-7 2x reservation | A host objects to buffer size: publish the tick-tight slot-count law in `ivs_info` |
| F-9 plane + chain digests | A consumer needs the chain to equal a plain hash of a dump: add a second named domain |
| F-10 frame-granular render | A fixture or capture shows per-row effects: R-R behind the same surface, composite bump |
| F-12 ivsr 3 | Not reversible without a goldens refresh; bump once per phase |
| F-13 PNG | Cost over budget: PPM; no one uses it: drop it |

## 9. Open items and cross-dependencies

1. Timing note: T_0, spacing, whether the STIC frame is free running, whether the VBLANK interrupt coincides with T_k, how display-enable is latched; the stall model must re-derive `psg.max_unit_ticks` U and `psg.min_write_gap_ticks` g (audio slack `s` and `W_log`) because BUSRQ/BUSAK stalls change unit lengths. This surface only needs 2U < F and F as a lower bound.
2. Render rules (other agent): FG/BG and Color Stack card word decoding, colour stack pointer reset and advance, bit order of a card pattern byte, card numbering for GROM and GRAM, which BACKTAB words feed which card; each must be tagged and have a vector. Whatever they decide must emit colour codes 0..15 only.
3. Composite version 3 for the STIC state, with the v2 literals pinned first; one `ivsr` bump for the phase; one discrepancy allocation table starting at C81; plan 06-01 owns the `d10-measurements.ids` additions.
4. Side-effect-free STIC inspection (VID-06) is separate from this surface; a host that wants border colour or mode state reads it through that path, not through the slot header.
5. The origin of the 159 x 96 window inside the 167 x 105 grid and the clip side matter to MOBs and collisions (Phase 8); recorded as unknowns, not as profile fields, until a test reads them.
6. Whether `1 pixel = 1 STIC clock` (159 of 228 ticks per line) holds is an inference for Phase 8 line timing; no Phase 6 decision depends on it.
7. `tools/frame` joins four guards (runner-scan, c-build unlisted-source guard, integrity anchor, fault modules) deliberately, as Phase 5 A-07 found for `tools/pcm`; the runner-scan file-write confinement must allow the new exclusive-create options only in `hostio.c`.
8. Rename `IVS_RUN_F_RESERVED_FRAME`: grep tests, the consumer and docs for the old name in the same plan (the flag was UNSUPPORTED, so no caller depends on it succeeding).
9. Plan shape suggestion mirroring Phase 5: an ABI/profile growth plan first and behaviour-neutral (format id, info v4 and report v3 fields written 0, flag and stop reason constants, ids files, video layout still NONE), then the timeline and state plans, then the surface plan that flips the layout to real (slot writer, chain, capacity law, flag, runner keys), then card render plans, runner and fixture, installed-consumer replay, close-out. Per-plan caps from Phase 5 stand.

## 10. Three riskiest unknowns

1. Whether 159 (and which 7 pixels, and bit order) is what a 2609 actually draws: tier 1 pages support the count, nothing in hand supports the clip side or the bit order, and the first expected frames will encode defaults (C10 family, D10-VID-01/02).
2. The frame boundary against the guest-visible timeline: SR1 as the end of the display period is read from a figure and the access-window consequences, but the relation of the VBLANK interrupt and the stall model to T_k, the display-enable latch, and a constant frame spacing are not primary-sourced; a wrong boundary moves every frame tick and digest and the "frame is exact for CPU-time updates" claim (C1-C4, C7, new entries).
3. Frame-granular rendering versus the guest's real update pattern: the approximation set A (BACKTAB edits during display, late bus acknowledgement) is tolerable for the Phase 7 scenario only if its program updates during CPU time; if a first real guest depends on mid-frame edits the renderer and the instance state change (composite bump), though the surface does not.

# STIC timeline notes

Project-owned notes for the frame timeline of the first profile (`intv-2609-ntsc-family-p1`): the numbers of the Phase 6 default timeline, the arithmetic behind each, how far each is backed, and the closed forms the later tests are derived from.
Facts and short paraphrases only; no page text and no page image is committed.
Labels: [OBS] read on a page or in a project record, [INF] derived by arithmetic or inference, [UNK] not printed on any page read.
Every figure below that is not printed on a manufacturer page is a default (`unknown-default-chosen`), tagged by its discrepancy id, and is never the expected value of a test of the same number (D-02): later tests prove the machine follows the profile field, not that the number is right.

## Inputs

Three profile fields carry the frame: `ticks_per_cpu_cycle` = 4, `cpu_cycles_per_scanline` = 57, `scanlines_per_frame` = 262.
The last two are tier 4 and open (C7); the first is the project time-base rule (`docs/time-base.md`).
Every other figure on this page is derived from these three or is a default named in the table of the evidence section.

## Arithmetic

| Quantity | Derivation | Value |
|----------|------------|-------|
| One scanline | 4 x 57 | 228 ticks |
| Frame length F | 228 x 262 = 4 x 57 x 262 | 59,736 ticks |
| Frame length in CPU cycles | 57 x 262 | 14,934 cycles (x 4 = 59,736 ticks) |
| Frame rate | 3,579,545 / 59,736 | about 59.9227 Hz (one STIC clock per tick) |
| One CPU cycle | 4 ticks | about 1.1175 microseconds |
| Register window | 2,000 x 4 | 8,000 ticks |
| Graphics-memory window | 3,780 x 4 | 15,120 ticks |
| Interrupt level width | 3,791 x 4 | 15,164 ticks |
| Bus window pitch | 16 lines x 57 = 912 cycles | 3,648 ticks |
| Bus hold H | 114 x 4 | 456 ticks |
| Row latch offset | 57 x 4 | 228 ticks after the window start |

## Timeline of frame k

Cycles count from `S_k`, the start of frame k; `S_k = stic.frame_phase_ticks + k x F` with phase 0 (C90), and `E_k = S_(k+1)`.
Windows are half-open: a time `t` is inside a window that opens at `a` and closes at `b` when `a <= t < b`.

| Event | Cycle | Tick from S_k | Mechanism |
|-------|-------|---------------|-----------|
| FRAME_START: INTRM level asserted, display latch cleared, both access windows open | 0 | 0 | timeline event |
| REG_CLOSE: register window closes | 2,000 | 8,000 | timeline event, gate STIC_REGS |
| GMEM_CLOSE: graphics-memory window closes | 3,780 | 15,120 | timeline event, gate GMEM |
| INTRM level released | 3,791 | 15,164 | pure function of the frame position |
| Bus window i starts, i = 0..12, only if the display latch is set | 3,791 + 912 i | 15,164 + 3,648 i | pure function (BUSRQ level) |
| ROW_FETCH(i), i = 0..11 | 3,848 + 912 i | 15,392 + 3,648 i | timeline event |
| Bus window i ends | 3,905 + 912 i | 15,620 + 3,648 i | pure function |
| FRAME_END then FRAME_START of the next frame | 14,934 | 59,736 | timeline events, FRAME_END first |

Derived rows: `3,791 + 57 = 3,848` and `15,164 + 228 = 15,392`; window i ends at `3,791 + 912 i + 114 = 3,905 + 912 i` cycles, which is `15,164 + 3,648 i + 456 = 15,620 + 3,648 i` ticks.
Row 11 latches at cycle 3,848 + 10,032 = 13,880 (tick 55,520).
Window 12 starts at cycle 3,791 + 10,944 = 14,735 (tick 58,940) and ends at cycle 14,849 (tick 59,396); it steals time and fetches nothing in Phase 6.
The frame ends 14,934 - 14,849 = 85 cycles (340 ticks) after the last window.
A fourteenth regular window at the same pitch would start at cycle 3,791 + 13 x 912 = 15,647, outside the frame of 14,934 cycles, so "fourteen" needs a different window placement (Phase 8, the picture offset) and is not modelled here.

## Closed forms

- Resume time of a granted stall (C52, C4): the CPU is granted for a window that starts at tick `w` at some open boundary no later than `w + 4 H`; the hold units walk to the first boundary at which the request is no longer seen, `w + 4 H + 4`, a release unit of `L` = 2 cycles follows, and the next CPU unit starts at `w + 4 (H + 1 + L)` = `w + 4 x 117` = `w + 468` ticks, which is 117 cycles after the window start whatever the grant phase.
- Level sampling (POL-LINE-01, C38, lag 1 cycle = 4 ticks): a unit that starts at tick `b` sees the levels of tick `b - 4`.
  INTRM is seen at a boundary `b` of frame k exactly when `S_k + 4 <= b <= S_k + 4 x 3,791` (the boundary at cycle 3,791 still sees it, the next does not).
  BUSRQ of a window starting at `w` is seen exactly when `w + 4 <= b < w + 4 H + 4` = `w + 460`.
- Access windows (A-06, C1, C2): an access stamped `a` is inside the register window when `0 <= a - S_k < 4 x 2,000` and inside the graphics-memory window when `0 <= a - S_k < 4 x 3,780`; the stamp is the unit start plus the access offset times 4 ticks (`stic.access_strobe_ticks` = 0).
- Tie rule (A-06, C104): FRAME_END(k) is processed before an access stamped exactly `E_k`, so that access lands in frame k + 1; frame k contains the accesses stamped strictly below `E_k`.
- Frame completion (D-14): frame k completes at the first unit boundary at or after `E_k`; the overshoot is at most U - 1 ticks.
- Capacity (D-15, A-08): the corner is `now = E_k - 4` with `D = target - now = F - 1`; the unit that starts below the target can end up to U - 1 = 467 ticks past it, so two frames complete in one call of F - 1 ticks.
  The frames completed in one call are at most `ceil((D + U - 1) / F)` for D >= 1, which is at most `floor(D / F) + 2` because U < F; the Phase 4 form `D / F + 1` is wrong in that corner.
- Bounds that do not move (D-10, A-09): `U` = (12 + 3 x 35) x 4 = 468 ticks, `g` = (9 - 4 + 2) x 4 = 28 ticks, and `W_log` = ceil(59,736 / 28) + ceil(468 / 28) + 1 = 2,134 + 17 + 1 = 2,152.
  A stall adds grant, hold and release units of 1 or 2 cycles; none is longer than the interrupt entry of 9 cycles, so the largest unit stays U.
- Executed cycles per display frame are an output of the instruction mix (about 13,430 to 13,530 for uniformly interruptible streams) and 14,934 with the display off; they are never a constant and never a profile field (C95).

## Evidence for each number

| Number | Value | Backing | Id |
|--------|-------|---------|----|
| STIC input clock | 3.579545 MHz, one STIC clock per tick | tier 1: the pages print it for the NTSC part (pdf 191 and 192; 1982 pdf 462 and 463) | C9 |
| Visible grid, card matrix | 167 x 105 grid, 159 x 96 visible, 20 x 12 cards, 19 columns of 8 plus one of 7 | tier 1 (pdf 191, 462) | C10 |
| Rows per frame requests | 13 or 14 bus requests per frame | tier 1 (pdf 191, 462); 13 is a default | C92 |
| Scanlines per frame, cycles per line | 262, 57 | tier 4, one lineage | C7 |
| Register window | 2,000 cycles (alternate 2,900) | tier 4 | C1 |
| Graphics-memory window | 3,780 cycles (alternate 3,796) | tier 4 | C2 |
| Interrupt level width | 3,791 cycles (alternates 2,907 and 2,920) | tier 4 | C3, C91 |
| Bus hold, acknowledge deadline | 114 cycles (alternates 110, and 57 and 44 as a pair), deadline about 57 | tier 4 | C4 |
| Bus window start and pitch | 3,791 and 912 = 16 x 57 | 16 lines per card row is tier 1 text plus inference; the start is a default | C92 |
| Line sampling lag | 1 cycle | default | C38 |
| Release latency | 2 cycles | inference | C52 |
| Row latch instant | 57 cycles after the window start | default | C93 |
| Frame phase at power-on | 0 | default | C90 |
| Usable cycles per display frame | about 13,518 and 13,572 | tier 4 aggregate, plausibility record only | C95 |

The 1980 and 1982 timing figure (printed 5-12 and 9-45) shows the sync signals qualitatively and prints numbers only for the bus protocol (see the verdict in `docs/stic/reference-notes.md`); no figure of the table above is read from it.

Cross-checks [INF]: 192 active lines are 12 rows x 16 lines, occupying 192 x 57 = 10,944 cycles; the remaining 70 lines are 3,990 cycles, and the interrupt level width of 3,791 cycles is 66.5 lines.
A scanline of 228 STIC clocks is about 63.695 microseconds.
None of these upgrades a figure above tier 4 or inference.

## Alternative hypothesis (D-05)

The manufacturer page of the system RAM describes a bus copy mode entered when the interrupt acknowledge is decoded and left at the first bus acknowledge, and the STIC page says its CPU control bus is ignored outside CPU mode.
That mechanism can explain a register window and a graphics-memory window without two constants.
It is recorded as the alternative to the pure frame-position schedule above; reversal is an owner measurement (D10-VID-06) or a primary page that prints the window events (C83).

## Measured through the full machine (plan 06-08)

These are outputs of the implemented lines and the test guests, recorded so a later change can be compared; none is an expected constant (D-02).

- Executed instruction cycles per frame depend on the guest's instruction mix. With the display on, a loop of one instruction length keeps the CPU out of 13 bus windows (grant, hold and release cycles are stolen) and out of one interrupt entry per frame; the executed cycles of a steady frame of the 12-cycle loop lie within one percent of the community aggregate 13,518 (plausibility only, C95). With the display off the CPU keeps all 14,934 cycles of the frame.
- A window starting at tick w resumes the CPU at w + 4 (H + 1 + L) ticks whichever instruction was interrupted (468 ticks with H 114 and L 2); the grant happens at the first open boundary inside the window, so a stream of non-interruptible stores is never stalled and its 13 windows count as late grants.
- With C41 = 0 a service routine that does not start with DIS is entered again at every open boundary while the interrupt level is held: 15 CPU cycles per entry (9 for the entry, 6 for the first instruction) give about 253 entries in the level of 3,791 cycles. Every stand-in service routine of Phase 6 starts with DIS.
- The densest reachable PSG writer (back-to-back non-interruptible indirect stores) makes 1,660 register writes in a span of one frame with the display on and with it off; the log bound is 2,152. The shortest gap between two writes of instruction units is 36 ticks; the 28 ticks of the published bound need an interrupt-entry write and are proven at the stepper level.
- The longest unit seen through the full machine over all 1,024 decles, with and without the SDBD prefix, with the interrupt level held and a window open, is 48 ticks; grant, hold and release units are at most 8 ticks. U stays 468.

## Not modelled in Phase 6

Mid-frame register effects, the picture offset that yields a fourteenth window, per-line fetches, the second half of the interrupt (acknowledge-driven release), and any PAL timing (D-03).

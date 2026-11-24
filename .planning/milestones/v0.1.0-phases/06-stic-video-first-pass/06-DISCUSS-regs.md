# Phase 6 discuss note: STIC register and access model (VID-01, VID-06 partly; D3 STIC pin)

**Role:** research agent in the fan-out discuss protocol (`.planning/AUTONOMY.md` section 2). Scope is one decision point: the STIC register file and its CPU access model. Out of scope here and owned by sibling notes: the VBLANK/BUSRQ/BUSAK timeline (VID-02), raw framebuffer geometry (D8, VID-03), card rendering (VID-04). Where this note needs something from them it states an interface (section 9).
**Labels:** [OBS] read on a page or in a source, [INF] inference, [REC] recommendation, [UNK] not printed or not found. Basis labels for rules: `primary-literal`, `primary-derived`, `community`, `default-unknown`, `project-rule` (a design rule of this project, not a hardware claim).
**Rights:** the GI catalogs were read from the gitignored cache (`<repo-root>/refs/`) and are cited by pdf and printed page only; nothing from a scan, from MAME or from jzIntv is copied. The community documents were read for facts (addresses, widths, behaviours) that are restated in my own words; no table, comment or code is transcribed. Paths in this file are `<repo-root>` and `<scratch>`.

---

## 0. One-screen summary

1. **The primary STIC pages do not print a register map.** The STIC section is three pages (1980: printed 5-10 to 5-12, pdf 191 to 193; 1982: printed 9-43 to 9-45, pdf 462 to 464) and it is a pin, bus-protocol and electrical data sheet. No CPU register address, bit field, display-enable strobe, color stack, delay register or access-window length is printed. Every register-level fact is therefore tier 4 (one community lineage) or [UNK], exactly as the existing `memmap.region.stic_regs` profile entry already says.
2. **But the primary pages do print the access mechanism**, and it is better evidence than the community window constants. The RA-3-9600 page (1980 printed 5-18, pdf 199; 1982 printed 9-51, pdf 470) describes a **bus copy mode**: entered when the system RAM decodes the CPU's INTAK response, left at the first BUSAK negative edge; while it lasts the RAM copies the low 14 bits of the CPU bus onto the 14-bit graphics bus. The STIC page (printed 5-10) says the STIC's CPU control bus is validated only in CPU-controlled mode and is ignored otherwise. Together these explain, in one mechanism, every community observation (windows exist, no INTAK means no access, a blank display gives unlimited access, a missed strobe blanks the display).
3. **Recommendation [REC]:** model the STIC register file as a real device on the existing memory system (device kind `stic14`, per-register masks from a generated `stic.regmap`, PSG precedent), keep all side effects in the machine bus wrapper (so `ivs_memsys_read` and `ivs_machine_peek` stay pure), derive the access gates from a small event-driven state machine (INTAK enters copy mode, first BUSAK leaves it, plus a V-anchored STIC register window and a blank-display override), and add one new const inspection call that returns the stored register file and mode state, which the bus view cannot show.
4. **D3 STIC pin:** AY-3-8900-1, NTSC, re-read and confirmed on the pages (S-1). PAL AY-3-8900 and STIC1A are out of scope.
5. Almost every register-level rule is `community` or `default-unknown`, so every claim that depends on it stays `provisional` (weakest-field rule). The vectors are therefore mostly `kind=policy`, with a small `primary-literal`/`primary-derived` core (section 6).

---

## 1. What the primary pages actually print (page-reading verdict)

Read at 150 dpi renders of pdf 190, 191, 193, 198, 199, 200 of the 1980 catalog and the pdftotext of pdf 461 to 464 and 470 of the 1982 catalog (the 1982 STIC pages carry the same text and figures; spelling and page furniture differ only).

```
registers-printed: no
register-bit-fields-printed: no
display-enable-handshake-printed: no
color-stack-printed: no
window-lengths-printed: no            (the timing figure is not to scale and has no cycle numbers)
bus-width-printed: yes                (14 bit bus, pins S00..S013)
cpu-control-validity-printed: yes     (ignored unless in CPU controlled mode)
bus-copy-mode-printed: yes            (RA-3-9600: entry on INTAK, exit at first BUSAK)
frame-sync-signals-printed: yes       (SR1 once per frame, SR2 13 or 14 per frame, SR3 per character)
electrical-timing-printed: yes        (ns figures only; no frame-level or cycle-level figures)
```

Per claim, page and what it prints [OBS]:

| # | Claim | Page | What the page prints |
|---|-------|------|----------------------|
| P1 | Part and clock | 1980 5-10 (pdf 191), 5-11 (pdf 192); 1982 9-43, 9-44 | AY-3-8900 for CCIR, AY-3-8900-1 for NTSC; "4.000MHz clock for AY-3-8900 and ... 3.579545MHz clock for AY-3-8900-1"; the electrical table repeats "3.579545 for AY-3-8900-1, both externally adjusted" |
| P2 | Object grid | same | "168H by 104V for AY-3-8900 or 167H by 105V for AY-3-8900-1 of which 159 x 96 are visible positions"; 8 objects; half height, y zoom, x zoom, 8 or 16 lines |
| P3 | Background | same | 20H x 12V matrix, 19 columns of 8x8 and the 20th 7x8; 16 colors; "240 by 14 bit words" of external memory for the character control codes; graphic pattern memory is 8 bits wide |
| P4 | Bus width | same | "time sharing a bidirectional 14 bit bus"; pins S00 to S013 on the diagram |
| P5 | Frame sync | same | SR1 "occurs once per frame ... indicates that STIC time is complete and that the AY-3-8900-1 has switched to the CPU controlled mode"; SR2 "issued thirteen or fourteen times per picture frame depending on picture offset"; SR3 pulses per character and "also issued during the CPU controlled mode in response to BAR, ADAR or DW to enable an external device onto the 14 bit BUS" |
| P6 | CPU control validity | same | "The CPU control BUS is only validated if the AY-3-8900/8900-1 is in the CPU controlled mode, otherwise it is ignored." |
| P7 | Timing figure | 1980 5-12 (pdf 193); 1982 9-45 | SR1 pulse, then a "CPU TIME" bracket, then "GRAPHIC TIME" with SR2 pulses (thirteen or so) and SR3 bursts for lines 1 to 16; not to scale, no numbers. The CPU control timing figure is the CP1610 bus protocol (4 time slots per cycle) with ns set-up figures (tas 200, tao 30, tws 100, two 30, tda 140) |
| P8 | RA-3-9600 contents | 1980 5-18 (pdf 199); 1982 9-51 (pdf 470) | 352 x 16 words at 512 to 863 with "the graphics descriptors using the first 240 words. The graphics use only the lower 14 bits of each word leaving the two most significant bits available for user storage"; a 20-word x 14-bit current line buffer |
| P9 | Bus copy mode | same | "Mode 1: On decoding an interrupt the RAM is enabled into a bus copy mode ... copies the lower fourteen bits of the CPU bus onto the graphics bus ... direction ... always from the CPU towards the graphics except during a bus reversal condition ... when the CPU requests a read from an external graphics address on the 14 bit bus"; "Mode 2 is selected when the CPU issues BUSAK"; "At the end of active picture the STIC issues an interrupt request to the CPU. The RA-3-9600 tests for the INTAK response ... and uses this signal as an entry control for a copy mode between the two buses. The end of the copy mode is controlled by the first BUSAK negative edge."; footnote "INTAK equivalent BC1, BC2, BDIR = 1" |
| P10 | GROM view | 1980 5-16 (pdf 197), 5-17 (pdf 198) | after SR1 pulsed negative the GROM sits at 12288 to 14335 with external memory at 14336 to 16383; after BUSAK it sits at 0 to 2047 (the STIC's own view); the 5-17 figure shows the position returning to 12 to 14K after each BUSAK pulse, so it describes GROM addressing, not a CPU-access gate |
| P11 | RA-3-9600 clock | 1980 5-19 (pdf 200) | "Clock Input Freq. phi2 ... 1.79545MHz" (image read, not OCR). That is the crystal divided by 2 [INF], a tier 1 data point for the C8 input-clock chain (the CPU's own input clock stays [INF]) |
| P12 | Pulled-up bus | not on a GI page | [UNK] on tier 1. The pull-up reading is the wiki's (C5) |

What is **not** printed anywhere I read: any CPU-visible STIC register address; which register reads return data; the strobe addresses; the length in cycles of any access window; whether SR1 coincides with or follows the interrupt request; what the CPU reads while access is blocked; the power-on state of the STIC.

Tier 2 (Blue Sky Rangers 2609 page, read in full, `SRC-bsr-2609`): confirms 240 BACKTAB locations in system RAM, a "bus arbitration controller" for GROM/GRAM in the RA-3-9600, 160 x 96 nominal and 159 x 96 displayed, 2 TV lines per pixel; says the STIC is "set to black out the video display while the instructions execute" during EXEC boot (so a blank display with CPU activity is a normal state). It prints no register address.

Tier 4 read for facts (one lineage, `SRC-intv-wiki` STIC page oldid 15008 and Memory Map oldid 14847; `SRC-community-emu-docs` `programming/stic.txt`, `interrupts.txt`, `memory_map.txt`, reference-only): register address ranges, the `$0020`/`$0021` roles, the aliases, the approximate windows, the blank-display statement, INTAK dependence, bit-width diagrams (see section 4).

---

## 2. Decisions

### S-1. D3 STIC part pin (confirmed, no change)

`stic.part = AY-3-8900-1`, NTSC, input clock 3.579545 MHz, object grid 167H x 105V, 159 x 96 visible positions (P1, P2). Tag stays `tier1-primary-doc`; the profile notes "research-era reading, not re-read" are replaced by "re-read in Phase 6: 1980 pdf 191, 192; 1982 pdf 462, 463". The PAL AY-3-8900 (4.000 MHz, 168H x 104V) is out of scope (REQUIREMENTS), as is the STIC1A. Add one tier 1 field `stic.bus_bits = 14` (P4, P8). `ticks_per_stic_clock = 1` stays tier 1 for the 3.579545 MHz input; the equality of that input with the crystal is the C9 nominal-crystal assumption, not new evidence. [REC]

### S-2. Layering (PSG precedent)

- The **register file** is guest-visible storage and lives in `ivs_memsys` as `uint16_t stic_reg[64]` (words `$0000` to `$003F`; the 24 reserved slots stay 0). Reads and stores go through one formula with a per-register mask from a generated table `stic.regmap` (same shape as `psg.regmap`, generated by a new fail-closed `cmake/IvsSticTable.cmake` with a `-DMUTATE=` hook). New device kind `IVS_MEMSYS_DEVICE_STIC14` (7; kinds become 8) replaces the `stub` device on `memmap.region.stic_regs` and on the three write aliases. The read alias `$0040-$007F` stays a stub.
- The **device state** lives in the machine as a small flat struct `ivs_stic_state` (mode, armed, active, copy, frame/event ticks, blocked counters; section 5). It is inside the instance and the composite digest.
- **All side effects live in `ivs_machine_bus`** (the Phase 5 wrapper), evaluated at the access stamp (unit start tick + access offset times ticks per CPU cycle), before `ivs_memsys_bus` applies the masked store. `ivs_memsys_read`, `ivs_memsys_bus`'s read path and therefore `ivs_machine_peek` never change state. This is what makes success criterion 5 true by construction, and a named fault proves a test can see the opposite.
- Rationale against the alternative "put the mode in memsys and switch it in memsys_read": the memmap probe drives a bare `ivs_memsys` and `peek` calls `ivs_memsys_read` on a const instance; a side effect there would make both lie. [REC]

### S-3. The register file ($0000-$003F), widths and masks

The CPU bus word is 16 bits; the STIC has 14 data pins (P4), so bits 15..14 can never be driven by the STIC and can never be stored by it [INF from P4, P8]. Per register, the write mask is what the register stores (bits above it are dropped; they are unobservable through the bus because reads of these registers do not return them), the read mask is what a bus read returns from the store; the remaining read bits come from the device fill, which is `memmap.undriven_read` `$FFFF` (C5), by the Phase 3 width formula `read = (stored & rmask) | (fill & ~rmask)`.

| Addr | Role | wmask | rmask | Stored | Read value (gate open) | Phase | Basis | Source |
|------|------|-------|-------|--------|------------------------|-------|-------|--------|
| `$00-$07` | MOB X (bits 0..7 position, 8..9 flags, 10 x-size; bits 11..13 unknown) | `$07FF` | `$0000` | yes | `$FFFF` | store only (VID-05, Phase 8) | community | stic.txt X diagram; wiki STIC |
| `$08-$0F` | MOB Y (0..6 position, 7 yres, 8..9 y size, 10..11 flip) | `$0FFF` | `$0000` | yes | `$FFFF` | store only | community | same |
| `$10-$17` | MOB attribute (14 bits: color, card, grom/gram, priority) | `$3FFF` | `$0000` | yes | `$FFFF` | store only | community | same |
| `$18-$1F` | MOB collision (10 bits: MOB0..7, background, border) | `$03FF` | `$03FF` | yes | `$FC00` OR stored | store; sticky/set rules Phase 8 | community | same; readable because ISRs read it |
| `$20` | display-enable strobe | `$0000` | `$0000` | no | `$FFFF` | Phase 6 | community | wiki "Active Display Strobe"; interrupts.txt |
| `$21` | mode-select strobe | `$0000` | `$0000` | no | `$FFFF` (read also switches mode) | Phase 6 | community | wiki, stic.txt |
| `$22-$27` | reserved | `$0000` | `$0000` | no | `$FFFF` | none | default-unknown | wiki "Unused/Unknown" |
| `$28-$2B` | color stack 0..3 | `$000F` | `$0000` | yes | `$FFFF` | Phase 6 (consumed by color stack rendering) | community | stic.txt, wiki |
| `$2C` | border color | `$000F` | `$0000` | yes | `$FFFF` | store; use depends on D8 | community | same |
| `$2D-$2F` | reserved | `$0000` | `$0000` | no | `$FFFF` | none | default-unknown | |
| `$30` | horizontal delay | `$0007` | `$0000` | yes | `$FFFF` | store only (Phase 8) | community + [BG] width | stic.txt names the register; the 3-bit width is [BG] |
| `$31` | vertical delay | `$0007` | `$0000` | yes | `$FFFF` | store only | same | |
| `$32` | border extension (bit 0 left, bit 1 top) | `$0003` | `$0000` | yes | `$FFFF` | store only | community | stic.txt |
| `$33-$3F` | reserved | `$0000` | `$0000` | no | `$FFFF` | none | default-unknown | |

Honest reading of the table:

- The **read-back column is [UNK] for every row but the collision registers.** The tier 4 documents list `$0000-$003F` as readable ("RW" in the wiki's table, "VBLANK 1" in the docs) without saying what a read returns; for the aliases both sources state the CPU sees no data. I default to the existing stub behaviour (undriven fill `$FFFF`, C60 default) for everything except the collision registers, which the docs say ISRs read. This is one field per register (`rmask`) to change and is the single most likely default to be overturned by D10; it is also the safest choice, because it claims nothing about stored values. [REC, `default-unknown`]
- **Stored widths** above the used bits are dropped. The unknown bits (for example X bits 11..13, shown as "??" in the community diagram) are unobservable through the bus, so dropping them cannot change a guest result; the choice only matters to the inspection report and the digest, and is tier-tagged. [REC]
- The collision row reads `$FC00` OR stored because bits above the 10 collision bits are undriven and take the pull-up fill; that upper-bit value is [INF] (P4 plus C5), not an observation.
- `$0020` and `$0021` store nothing. The mode lives in `ivs_stic_state`, not in a register word.

Memory-map literal impact (Phase 5 precedent): `data/memmap/<profile>.literal.txt` rows for `$0000-$003F` and the three write aliases (`$4000`, `$8000`, `$C000` blocks) are split by width and flip from `impl=stub` to `impl=real`, with `x=reads-zero` on the write-only rows (the same exception class Phase 5 introduced) and a new declared exception `x=strobe` on the `$20/$21` rows (no value stored, the access has a machine-level effect). The independent literal `data/stic/<profile>.regmap.literal.txt` (64 rows, hex address, role, wmask, rmask, source locator, tag, conflict) is written from the sources before the profile field. The read alias `$0040-$007F` stays `impl=stub`.

### S-4. Which accesses have side effects, and the side-effect-free path

Side-effect table (profile field `stic.access_effects`, tier-tagged, one row per register and access class; a "read" is an access of kind READ or FETCH, because the STIC sees only the bus control codes and cannot tell a fetch from a data read; a "write" is WRITE or DW; ADAR, BAR, INTAK, IAB and strobes have no effect except INTAK in S-5):

| Reg | Write | Read |
|-----|-------|------|
| `$0020` | arm the display for the frame (value ignored), subject to S-5 rule W-3 | none ([UNK] whether a read also strobes; default none, C82) |
| `$0021` | mode := Foreground/Background | mode := Color Stack (returns `$FFFF`) |
| every other register | masked store | none (collision clear-on-read is [UNK], Phase 8) |

Effects happen only if the STIC register gate is open at the stamp (S-5); a blocked access has no effect, which is P6 read literally.

Aliases: a write to `$4000/$8000/$C000` plus `0..$3F` behaves like a write to the register (community: aliases are sensitive to both reads and writes), and a **read** of exactly `$4021`, `$8021`, `$C021` switches the mode even though the read view of those addresses decodes to cartridge space and the CPU receives the cartridge value (or the fill). The effect decode is therefore an explicit profile list `stic.mode_select_addrs = [$0021, $4021, $8021, $C021]` (tag `community`, source: wiki Memory Map oldid 14847 names exactly these), independent of the read-view region. `$0061` and the rest of `$0040-$007F` have no effect (default-unknown; the wiki says the range is "STIC-responsive, but read only" with no effect named, C84). A one-field switch `stic.alias_read_sensitive` (default 1) disables the alias read effect; see adversarial item A-5.

**The side-effect-free inspection path (VID-06, success criterion 5):**

1. `ivs_machine_peek` keeps its Phase 4 contract: "what a CPU READ returns now" over the const memory system, with gates honoured, no log, no time, no state. Because every side effect is in `ivs_machine_bus`, peeking `$0021`, `$4021`, `$8021`, `$C021` (any number of times, any gate state) cannot change the mode. Peek of a write-only register returns `$FFFF`, which is the truthful bus view.
2. A **new const call** returns what the bus cannot show: `ivs_machine_stic_state(const ivs_machine *m, ivs_stic_report *out)` [REC]. Versioned report, `struct_size` first, the cpu_state refusal ladder (BAD_ARG for out NULL or struct_size outside bounds, BAD_HANDLE, BAD_ARG for overlap with the instance), built in a local and copied out last, valid OFF and POWERED. Contents v1: `regs[64]` (stored words, gate-independent), `mode` (0 FGBG, 1 Color Stack), `armed`, `active`, `copy`, `regs_window_open`, `gmem_window_open` (evaluated at machine time now), `frame_start_tick`, `blocked_reads`, `blocked_writes` (saturating u32), and reserved words. It advances no time and writes nothing; a digest taken before and after is equal. The reason for a second call rather than a peek flag: the gate-ignoring view (stored registers behind a closed window) is exactly the named fault `PEEK_GATE_IGNORED` for peek, so the two views must not share a function.
3. Positive controls: a guest `MVI $0021` (in an open window) does switch the mode; a peek of the same address does not. Named fault `PEEK_SWITCHES_MODE` puts the effect into the read path and the test must fail.

### S-5. Access windows and the display-enable handshake (VID-01)

Mechanism [REC, tags per rule in section 4]:

State: `copy` (bus copy mode), `armed` (a strobe was accepted this frame), `active` (graphic time is running this frame), `frame_start_tick` (V).
Events, supplied by the machine timeline (VID-02 owns when they happen; this note owns what they do):

- **V** (end of active picture, the VBLANK interrupt request): `active := 0`, `frame_start_tick := tick`.
- **INTAK** (the CPU's interrupt acknowledge access, seen in `ivs_machine_bus`): `copy := 1` (P9: entry on INTAK).
- **G** (start of graphic time, the first BUSAK negative edge of the frame): if `armed` then `active := 1`, `armed := 0`, `copy := 0` (P9: exit at first BUSAK). If not armed: nothing (blank frame, no bus steals, `copy` unchanged).

Gate predicates, pure functions of (state, tick), with `blank := !(armed || active)`:

- W-1: `stic_regs_open(t) = blank || (copy && (t - frame_start_tick) < regs_window_ticks)`
- W-2: `gmem_open(t) = blank || copy`
- W-3: a write to `$0020` arms the display only if `(t - frame_start_tick) < handshake_ticks`; a later strobe is accepted by the register decode (nothing to store) and does nothing.

Blocked-access policy: a read of a closed region returns the undriven fill (`memmap.undriven_read`, `$FFFF`; C5) with no side effect; a write is dropped; nothing faults; `blocked_reads`/`blocked_writes` are incremented (state, digest-covered). This reuses the Phase 3 closed-gate behaviour (read gives fill, write dropped) unchanged.

Ordering: events with `tick <= stamp` are applied before the access at `stamp` (the Phase 5 "step first, then write" rule); the access at the event tick sees the event. At every unit end the STIC is caught up to machine time and the memsys gate flags are refreshed from the predicates; the wrapper refreshes them again at each access stamp before dispatch. The flags in `ivs_memsys` are therefore a **derived cache**; the single function `ivs_stic_refresh_gates` is the only writer besides power-on, and a named fault `STALE_GATE_AT_UNIT_END` proves a stale flag makes peek differ from a CPU read at the same tick.

Parameters (profile, section 3): `stic.window.regs_cpu_cycles` default 2000 and `stic.window.handshake_cpu_cycles` default equal to it (C1: shorter documented figure as the conservative bound), converted once at load to master ticks (4 per CPU cycle). No GMEM constant is needed in the predicate: the GROM/GRAM window ends at the first BUSAK, which VID-02 places at the C2 figure (shorter bound 3780 CPU cycles after V); see interface I-1.

Behaviours this produces, each a vector (section 6): with the display never armed (power-on, EXEC-style boot) every access is open (matches the community "unlimited access while blanked" and the BSR black-out during boot); a strobe within the first 2000 CPU cycles after V arms the next graphic time; the window then closes at `regs_window_ticks` for the STIC registers and at the first BUSAK for GROM/GRAM; a guest that never takes the interrupt loses access after the first active frame (copy stays 0) and its display goes blank one frame later (it could not strobe), after which access reopens (blank); a late strobe does not arm. The last two behaviours are consequences of the mechanism and agree with the community statements; they are not independently observed [INF].

Persistence: the mode is **persistent until the next `$21` access** (wiki and docs agree), not re-armed per frame. Power-on state: registers 0 (`power_on_fill.stic`, existing), mode FGBG (the zero state of the mode bit), `armed 0`, `active 0`, `copy 1`, counters 0 (all `default-unknown`).

What is deliberately not modelled: register decay if interrupts are not taken (a single-source community claim, C88 logged), the INTAK requirement while blank beyond the predicate above, STIC1A differences.

### S-6. Digest, snapshot and instance growth

`stic_reg[64]` (128 bytes) joins the MEM rows and `ivs_stic_state` joins the INSTANCE rows, so guest-visible STIC state cannot differ with equal composites: a deliberate **composite version bump to v3** with the Phase 5 v2 literals pinned first and the golden change classified (only composite, instance and mem lines may differ), as Phase 5 did for v2. Reset restores the power-on state above. `sizeof(ivs_memsys)` and `IVS_MACHINE_BYTES` grow; the layout `_Static_assert`s and the digest field table are updated in the same plan. Phase 8 snapshots inherit the rows.

### S-7. Side finding on the time base (C8)

P11: the RA-3-9600 prints a 1.79545 MHz clock input. Add it to C8's evidence as a tier 1 data point that a console clock of crystal divided by 2 exists; the CPU input clock itself stays [INF]. No profile change.

---

## 3. Profile data schema (new fields; every field carries tag, cite, conflict, resolving_evidence, revisit_trigger)

| Field | Value (NTSC p1) | Tag | Conflict |
|-------|-----------------|-----|----------|
| `stic.part` | `AY-3-8900-1` (existing) | tier1-primary-doc | none |
| `stic.bus_bits` | 14 | tier1-primary-doc | none |
| `stic.regmap` | 64 objects `{addr, name, class, wmask, rmask, phase}` per section 3, `class` in {`stored`, `strobe`, `collision`, `reserved`} | community (registers), default-unknown (read masks, reserved) | C60, C81 |
| `stic.access_effects` | rows `{addr, access: R\|W, effect: arm\|mode_fgbg\|mode_cs\|none}` per S-4 | community | C82 |
| `stic.mode_select_addrs` | `[$0021,$4021,$8021,$C021]` | community | C84 |
| `stic.alias_read_sensitive` | 1 | community | C84 |
| `stic.write_alias_bases` | `[$4000,$8000,$C000]` | community (existing regions) | C61 |
| `stic.window.model` | `copy+regs-window` (informational enum; reversal path `constant`) | project-rule | C83 |
| `stic.window.regs_cpu_cycles` | 2000 | tier4-community (shorter figure) | C1 |
| `stic.window.handshake_cpu_cycles` | 2000 | default-unknown | C82 |
| `stic.copy.enter_on` | `intak` | primary-derived (P9) tagged `inference` | C83 |
| `stic.copy.leave_on` | `first_busak` | primary-derived (P9) tagged `inference` | C83 |
| `stic.blank_unlimited` | 1 | tier4-community | C85 |
| `stic.blocked_read_value` | `memmap.undriven_read` (reference, no new constant) | default-unknown | C5 |
| `stic.power_on` | `{regs: zero, mode: fgbg, copy: 1, armed: 0, active: 0}` | default-unknown | C85 |
| `memmap.device.stic14` | `{bits: 14, rmask: "$3FFF", fill: "$FFFF", store: "keep"}` (per-register masks refine it) | default-unknown | C60 |

Tag discipline: `tier1-primary-doc` only for P1 to P4 facts; the two `copy.*` rows are `inference` because the page describes the RA-3-9600's behaviour and the application to STIC register access is an inference (S-5 rule W-1 rests on it). Unit rule from `docs/time-base.md`: window fields end in `_cpu_cycles` (or `_ticks`) and are converted at load; no bare unit word in docs.

---

## 4. Rule inventory (`data/stic/rules.json`, ids STIC-R01..; basis, source, fields, vector families)

| Id | Rule | Basis | Source | Fields / engine | Vectors |
|----|------|-------|--------|-----------------|---------|
| STIC-R01 | STIC is AY-3-8900-1, 3.579545 MHz, 167H x 105V grid, 159 x 96 visible | primary-literal | 1980 5-10, 5-11; 1982 9-43, 9-44 | `stic.part`, `ticks_per_stic_clock` | stic.profile.* |
| STIC-R02 | STIC data bus is 14 bits; bus bits 15..14 are not driven by it | primary-literal | 5-10 (pins S00..S013, "14 bit BUS") | `stic.bus_bits`, `memmap.device.stic14` | stic.reg.width |
| STIC-R03 | Graphics words use the low 14 bits; the top two bits of a system-RAM word are user storage (16 bits round-trip in BACKTAB RAM) | primary-literal | 5-18 | existing `ram16` | stic.bt.roundtrip |
| STIC-R04 | STIC ignores CPU control codes unless in CPU controlled mode | primary-literal | 5-10 | gate predicates | stic.blocked.* |
| STIC-R05 | A blocked STIC write is dropped and has no effect | primary-derived (R04) | 5-10 | `stic_regs_open` | stic.blocked.w |
| STIC-R06 | Bus copy mode is entered on INTAK and left at the first BUSAK edge | primary-literal | 5-18; 9-51 | `stic.copy.*` | stic.copy.* |
| STIC-R07 | A STIC register write travels over the copy path, so it needs copy mode (or a blank display) | primary-derived (R06, R04) | 5-18, 5-10 | W-1 | stic.win.* |
| STIC-R08 | GROM/GRAM CPU access needs copy mode (or a blank display) | primary-derived (R06) plus community | 5-16, 5-18; docs | W-2 | stic.win.gmem |
| STIC-R09 | Register address roles `$00-$32` per section 3 | community | wiki STIC oldid 15008; stic.txt | `stic.regmap` | stic.reg.map |
| STIC-R10 | Stored width per register | community / default-unknown | stic.txt diagrams; [BG] | `stic.regmap` wmask | stic.reg.rw |
| STIC-R11 | Collision registers are readable (10 bits); other registers read the undriven fill | community (collision), default-unknown (others) | docs; C60 | `stic.regmap` rmask | stic.reg.rd |
| STIC-R12 | Reserved registers drop writes and read the fill | default-unknown | wiki "Unused/Unknown" | `stic.regmap` | stic.reg.rsv |
| STIC-R13 | `$0020` write arms the display; value ignored | community | wiki, interrupts.txt | `stic.access_effects` | stic.fx.strobe |
| STIC-R14 | `$0021` write selects FGBG, read selects Color Stack; persists until the next access | community | wiki STIC, stic.txt | `stic.access_effects` | stic.fx.mode |
| STIC-R15 | A `$0020` read has no effect | default-unknown | none found | `stic.access_effects` | stic.fx.strobe-read |
| STIC-R16 | Aliases `$4000/$8000/$C000` + reg write the register; read of `$4021/$8021/$C021` switches the mode; the read returns no STIC data | community | wiki Memory Map oldid 14847; memory_map.txt | `stic.mode_select_addrs`, regions | stic.alias.* |
| STIC-R17 | `$0040-$007F` reads have no effect | default-unknown | wiki marks it read-only, effect unnamed | stub region | stic.alias.r40 |
| STIC-R18 | STIC register window is about 2000 CPU cycles after V | community (shorter bound) | docs; wiki says about 2900 | `regs_cpu_cycles` | stic.win.regs |
| STIC-R19 | GROM/GRAM window ends at the first BUSAK, about 3780 CPU cycles after V | community + primary-derived (R06) | docs; wiki about 3796 | interface I-1 | stic.win.gmem |
| STIC-R20 | A blank display gives unlimited STIC and GROM/GRAM access | community | interrupts.txt, stic.txt | `stic.blank_unlimited` | stic.win.blank |
| STIC-R21 | Without INTAK there is no access during an active frame | primary-derived (R06) plus community | 5-18; interrupts.txt | copy flag | stic.copy.nointak |
| STIC-R22 | A strobe within the handshake deadline arms the next graphic time; a late one does not | community / default-unknown | interrupts.txt | `handshake_cpu_cycles` | stic.hs.* |
| STIC-R23 | Unarmed frame = blank frame: no graphic time, no bus steal | community | interrupts.txt, BSR | engine | stic.hs.blank |
| STIC-R24 | Blocked read returns the undriven fill, no side effect; blocked write dropped; counters increment | default-unknown (value), project-rule (counters) | C5 | existing gate + `ivs_stic_state` | stic.blocked.* |
| STIC-R25 | Power-on state (regs 0, mode FGBG, copy 1, unarmed) | default-unknown | none | `stic.power_on` | stic.power.* |
| STIC-R26 | Events at tick <= stamp apply before the access | project-rule | Phase 5 precedent | engine | stic.order.* |
| STIC-R27 | Peek is the bus view, pure; the state report is the stored view, pure | project-rule | VID-06 | `ivs_machine_peek`, `ivs_machine_stic_state` | stic.peek.* |
| STIC-R28 | Phase 6 renders only Color Stack colors; MOB, delay, border-extension registers are stored, not applied | project-rule | REQUIREMENTS VID-04/05 | `phase` column | stic.scope.* |

Gate step (`stic-vectors`, mirroring `psg-vectors`): every rule has a field or engine marker, at least one vector, and a source or an `unknown-default-chosen` tag with resolver and trigger; a rule with basis `default-unknown` must name its discrepancy.

---

## 5. State struct (`ivs_stic_state`, fixed width, no pointers, explicit pad)

```
u64 frame_start_tick     /* V, last vblank event */
u64 last_event_tick      /* catch-up position (events applied up to here) */
u8  mode                 /* 0 FGBG, 1 Color Stack */
u8  armed, active, copy  /* 0 or 1 */
u8  reserved[4]          /* 0 */
u32 blocked_reads, blocked_writes   /* saturating */
```

Roughly 32 bytes, 8-aligned; the exact total is set by the layout plan (Phase 5 computed offsets in the machine.h comment; do the same). Mode and the color-stack read position are separate: the color stack's current position during rendering is renderer state (VID-04's note).

---

## 6. Vector plan (oracle-first, non-circular)

Plan 06-01 (or the equivalent first plan) contains no C, as Phase 5 plan 05-01 did: it writes `docs/stic/reference-notes.md` with the verdict block of section 1, `data/stic/rules.json`, `data/stic/<profile>.regmap.literal.txt` (64 rows from the sources, written before the profile field), line-based vectors under `data/stic/vectors/*.txt`, `data/expected/stic-rules.ids`, the `stic-vectors` and `stic` gate steps with controls, the discrepancy allocation, `d10-measurements.ids` rows, and blind second-read requests. The profile fields and generator come after, and the memmap literal flip lands with the implementation plan (Phase 5 amendment A-05: the generator rejects `impl=real` rows before the rule and descriptor rows exist).

Independent expected values the pages actually give (the non-circular core):

1. **Width.** 14 pins (P4) and "two most significant bits available for user storage" (P8) give literal vectors: a 16-bit word written to a BACKTAB address (`$0200-$02EF`, system RAM, always open) reads back all 16 bits; a value with bits 15..14 set written to a stored STIC register reads back (through the inspection report) with those bits absent. `primary-literal`.
2. **Copy-mode ordering** (P9): INTAK then access sees the window; an access after the first BUSAK sees it closed (active frame); no INTAK in an active frame leaves it closed. The expected values follow from the page's sentence structure, not from any constant. `primary-derived` (the page does not say STIC registers use the copy path, R07 is an inference).
3. **Ignored when not in CPU mode** (P6): a write in graphic time changes nothing. `primary-literal` for the principle.
4. **Unit consistency** (P1, P11, existing D2): window ticks equal CPU cycles times 4. A named fault `WINDOW_CYCLES_AS_TICKS` fails it. This checks arithmetic, not hardware.
5. Everything else (register roles, `$21` read versus write, aliases, handshake deadline, blank-unlimited, read-back values, power-on) is **`kind=policy`** with the community source and discrepancy named. The boundary vectors (access at `V + regs_window_ticks - 1` open, at `V + regs_window_ticks` closed) are consequences of the chosen default and are labelled "consistency with the chosen default, not evidence".

Families (indicative counts): `stic.reg.map` (profile table equals literal, 64), `stic.reg.rw` and `stic.reg.rd` (about 40: per class masks, readback through report and bus), `stic.reg.rsv` (6), `stic.fx.*` (about 16: strobe, mode write/read, `$0020` read, fetch of `$0021`, DW, ADAR no effect), `stic.alias.*` (about 12), `stic.win.*` and `stic.copy.*` (about 28: boundaries in ticks, blank, never-INTAK, INTAK latency), `stic.hs.*` (about 12: strobe in time, late, none, two per frame, strobe blocked), `stic.blocked.*` (about 10), `stic.order.*` (6: event at stamp, stamp-1), `stic.power.*` (6), `stic.peek.*` (about 10: `$0021` peek no effect in every gate state, peek equals CPU read at the same tick, report equals regs), `stic.scope.*` (4). About 150 vectors; each plan adds its own inventory lines.

Slicing invariance (API-03 style): the same program run as one call versus one-unit calls versus `--slice-ticks 1` and `7` gives identical STIC state, report and composite; the events-before-access rule is what makes this true, so the vectors are mandatory, not optional.

Original diagnostic (D4 note): a guest program that (a) strobes `$0020` in the ISR, (b) reads `$0021`, (c) writes one color-stack register and one GRAM word inside the window and tries them again after it, and (d) stores the observed read-backs in system RAM. It needs a branch and a poll loop; hand-assembled per D4. The D4 mini-assembler trigger (a third diagnostic or labels/macros) is likely reached in this phase or the next; record it, do not decide it here.

Named faults (indicative, at most 8 per plan per the Phase 5 caps): `PEEK_SWITCHES_MODE`, `READ_EFFECT_WHEN_BLOCKED`, `WRITE_STORED_WHEN_BLOCKED`, `STALE_GATE_AT_UNIT_END`, `EVENT_AFTER_ACCESS` (off by one at the event tick), `MODE_READ_FGBG` (swapped sense), `STROBE_VALUE_DEPENDENT`, `ALIAS_READ_NO_EFFECT`, `LATE_STROBE_ARMS`, `COPY_NOT_CLEARED_AT_BUSAK`, `NO_INTAK_REQUIREMENT`, `WINDOW_CYCLES_AS_TICKS`, `STIC_STATE_NOT_RESET`, `REG_STORED_AT_16_BITS`, `BLOCKED_READ_ZERO`. Each needs a green positive control in the same suite.

Evidence tiers for claims (Phase 5 pattern): register model claims `source-expectation`, `provisional`; guest handshake claim `original-diagnostic`, `provisional`; slicing invariance `deterministic-repeatability`; none `measured-hardware`. The weakest-field rule keeps all of them provisional.

---

## 7. Discrepancy ids to open (provisional numbering; orchestrator allocates one table across all Phase 6 notes)

The next free id is C81 (log ends at C80). Provisional ids below must be reconciled with the timing, geometry and render notes in ONE allocation table (the Phase 5 lesson).

| Prov. id | Topic | Default | Resolver |
|----------|-------|---------|----------|
| C81 | STIC register read-back: which registers return data, which bits, collision upper bits (extends C60) | write-only except collision, fill `$FFFF`, collision `$FC00` OR stored | D10 read-back of all 64 addresses; a tier 1 to 3 STIC register description |
| C82 | `$0020` semantics: read effect, handshake deadline, value ignored, late strobe, whether `$21` and `$20` roles are as the wiki says (extends C6) | per S-4 and S-5 W-3 | D10 diagnostic with strobes at swept offsets |
| C83 | Access mechanism: copy mode on INTAK/first BUSAK plus a V-anchored register window versus constant windows (links C1, C2, C3) | S-5 | D10 edge probes; a tier 1 to 3 STIC time description |
| C84 | Alias sensitivity: read of `$4021/$8021/$C021` switches mode; `$0040-$007F` and `$0061`; alias writes reach registers (extends C61, C21) | S-4 | D10 read of aliases with a known mode |
| C85 | Blank display: unlimited access, power-on `copy`, power-on mode | S-5 | D10 power-on probe |
| C86 | Stored widths and unknown bits (X 11..13, Y/A upper, delay widths) | S-3 | D10 write-all-ones read-back where readable; schematic |
| C87 | Collision register clear-on-read, sticky set, self bit (Phase 8, VID-05) | stored only in Phase 6 | D10 |
| C88 | Register decay when interrupts are not taken (single-source community claim) | not modelled | D10 long-run probe |
| C89 | Ordering of the interrupt request versus SR1 versus V (the pages print neither relation) | V is the interrupt-request tick | timing note; D10 |

D10 inventory rows (to `data/expected/d10-measurements.ids`, one per resolver above): `D10-STIC-01..` read-back sweep, strobe-offset sweep, window-edge probe, alias read probe, power-on probe, decay probe. All stay "missing, handed off" (hard stop).

Updates to existing entries: C1/C2 gain "S-5 uses 2000 as `regs_cpu_cycles`; GROM/GRAM end is the first BUSAK"; C5 gains "STIC blocked reads use the same fill"; C60 gains "STIC registers: see C81"; C61 gains "alias read-sensitivity of `$21` addresses: C84"; C6 gains "see C82"; C8 gains P11.

---

## 8. Role lenses (what each found; the decisions above fold these in)

- **Emulator architect.** Event-before-access catch-up (the PSG model) is the only scheme that stays slicing-invariant with a lazily stepped device. Two classic traps: (a) deciding the window by a frame-modulo counter, which breaks with stalls and a blank display (time-base doc already forbids it); (b) an "in vblank" boolean toggled by the frame loop and read by the CPU, which makes registers depend on call boundaries. Lessons from other emulators are only discrepancy hints here (no code used): they model the windows as constants, which this note keeps as parameters but does not make the mechanism.
- **Hardware/EE.** The primary pages describe two cooperating pieces: the STIC's own CPU/graphic mode (P5, P6) and the RA-3-9600's copy mode (P9). The register window is the conjunction; GROM/GRAM only needs the copy mode, which explains why GRAM stays open longer than the registers (3780 versus 2000) without a second constant. [INF, not measured.] SR1's relation to the interrupt request is not printed (C89).
- **QA.** The vector set is intentionally honest about circularity: a boundary test that reads back the chosen constant is labelled consistency, not evidence. The strong tests are structural (ordering, mask, purity) plus slicing invariance and named faults.
- **Security/rights.** No scan content committed; community documents are GPL or wiki text read for facts only; the register literal cites wiki oldid 15008 and 14847 (the wiki, not the GPL docs, is the primary community citation for addresses and the mode strobes). Bit widths come from diagram facts in a GPL document: restated as masks, not transcribed; if that is judged too close, the second source is a blind re-derivation from the wiki's MOB pages (login-gated today) or a D10 measurement, and the width rows revert to `default-unknown`.
- **DevOps/CI.** About 150 vectors, a table-driven literal and generator add roughly one gate step; they are text and table checks, cheap. Expect a +1 to +2 cpu-s class change; measure after the first implementation plan against the CI-EFFICIENCY triggers, as Phase 5 did.
- **API/integrator.** Peek keeps its meaning; one new const report call, versioned; no new run-call field. The report is how a host or debugger sees write-only state. A host never needs to know about copy mode to use the core.
- **Product.** The first visible scenario (Phase 7) needs only: strobe, mode select, color stack, GRAM write in window. Everything else is stored but unapplied and says so in the rule table (STIC-R28), so no claim overreaches.
- **DX.** `blocked_reads/blocked_writes` counters answer the most common STIC-programming question (why was my GRAM write dropped) with one report call. The generated `stic.regmap` makes a width change a one-line profile edit.

---

## 9. Interfaces to the sibling decisions

| Id | Needs from | Statement |
|----|-----------|-----------|
| I-1 | VID-02 timeline | Provide events V (vblank, tick), G (first BUSAK of the frame, tick) as functions of machine time and display state; the first BUSAK must not precede `V + regs_window_ticks` and should sit at the C2 bound (3780 CPU cycles after V) for GROM/GRAM; an invariant test asserts G offset >= regs window. Until VID-02 lands, G is a profile constant with the same value. |
| I-2 | VID-02 | The machine drives INTRM at V; this note's INTAK handling reads the CPU's INTAK access only and does not depend on how the request is generated. A guest that masks interrupts simply never produces INTAK. |
| I-3 | D8 / VID-03 | Border color (`$2C`) is a stored register exposed to the renderer; whether it enters the raw frame depends on D8. |
| I-4 | VID-04 | The renderer reads `mode`, `stic_reg[0x28..0x2C]` and BACKTAB (system RAM, always accessible; the STIC reads it through bus steals, not through a gate) at render time. Color stack position is renderer state. |
| I-5 | VID-02 | Blank frames (unarmed) have no BUSRQ events; an active frame has 13 or 14 (P5, `SR2`); how many and when is VID-02's. |

---

## 10. Open items, honest limits

- The register-level model is almost entirely tier 4 (one lineage: wiki plus the community docs) plus defaults. No decision above is evidence of hardware behaviour. Success criterion 1 is satisfiable as "windows are profile parameters with tier tags and disputed values logged `unknown-default-chosen`", which is what this gives.
- Not obtained: any STIC register description from tier 1 to 3; the wiki's MOB and timing pages (login-gated); a schematic of how the STIC and the RA-3-9600 are wired (INTAK, BUSAK, SR1 paths).
- The window mechanism (S-5) is an inference from P6 and P9 that happens to fit the community observations. It is the part most likely to change when D10 evidence exists.
- Blind second reads: the register literal (1 reader), the rule/basis labels of section 4 (1 reader, projection only, no emulator material).

## 11. Reversal triggers

| Decision | Reversal evidence | Cost |
|----------|-------------------|------|
| S-3 read masks | D10 read-back showing registers return stored data | one `rmask` edit per register, vectors flip |
| S-3 widths | schematic or D10 | `wmask` edit |
| S-4 alias read effect | a cartridge that legitimately maps data at `$8021`/`$C021` working on hardware, or D10 | `stic.alias_read_sensitive` = 0 |
| S-5 mechanism (copy mode + V window) | D10 edge probes showing a pure constant window from INTAK or from V, or a different STIC/RA split | change predicates W-1..W-3; `stic.window.model` documents the alternative; state struct stays |
| S-5 handshake deadline | probes of a late strobe | `handshake_cpu_cycles` or rule W-3 |
| S-5 power-on `copy`/mode | power-on probe | `stic.power_on` |
| S-2 layering | a need for STIC reads that depend on STIC time | move the effect, not the register file |
| S-6 composite v3 | none expected | one-way once goldens are re-blessed |
| Window constants 2000 / 3780 | D10, a tier 1 to 3 number | profile edit |

## 12. Adversarial pass

- **A-1: "The copy-mode mechanism is an invention; use constants."** Fair: P9 describes the 9600's behaviour toward the graphics bus; that STIC register writes use it is an inference (R07 says so). Counter: the constant-window model cannot explain the INTAK dependence or the blank-display override, so it needs extra special cases anyway; the mechanism adds only two booleans and keeps both constants as parameters. If D10 contradicts it, only predicates change (section 11). Kept, with `stic.window.model` recording the alternative.
- **A-2: "Too many events for a first pass; VID-02 not final."** The interface is three events (V, INTAK, G) and two predicates; with G as a profile constant the model runs before the BUSRQ model exists. The cost of not modelling INTAK is a guest-visible difference only for programs that mask interrupts, which our diagnostics do not.
- **A-3: "Reads of write-only registers returning `$FFFF` may break a guest doing read-modify-write."** Real EXEC games keep shadow copies precisely because of this; a guest depending on a read-back would be depending on an unknown. The default claims nothing, and the report call shows the stored value to humans.
- **A-4: "Dropping unknown bits changes nothing observable, so why store masks at all?"** For the digest, snapshots and the inspection report to be stable across a later width discovery; cheap, table-driven.
- **A-5: "Read-sensitive aliases are hazardous for legitimate ROM data at `$8021`."** The wiki itself warns cartridges to avoid those ranges; the core mimics the hazard because both community sources describe it. The one-field flag limits the blast radius; the loader could later warn on a cartridge segment covering `$4021/$8021/$C021` (not decided here, a candidate Phase 7 item).
- **A-6: "Peek and report duplicate each other."** They answer different questions (bus view with gates versus stored view without); sharing a function is exactly the PEEK_GATE_IGNORED fault class. Kept separate.
- **A-7: "Composite v3 is churn."** Without it STIC state is invisible to equal-digest checks, which Phase 5 already judged unacceptable for guest-visible state.
- **A-8: "Late strobe not arming contradicts blank-unlimited."** W-3 is a separate rule with its own discrepancy (C82); the community forum anecdote quoted in interrupts.txt (strobe issued too late, display missing) is the only evidence and fits W-3; a probe is cheap. Default stays.
- **A-9: "The page-reading verdict is mostly negative; is a decision even warranted?"** Yes: the project needs a deterministic, tier-tagged default, and the negative verdict is itself the finding that keeps every dependent claim provisional.
- **Shared-misreading risk:** the vectors and the engine are written by the same project; blind second reads of the literal and rule labels are requested, and no emulator code was used.

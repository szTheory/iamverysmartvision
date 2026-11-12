# PSG reference notes

Project-owned notes for the sound chip of the 2609 family: what the manufacturer pages for the AY-3-8910/8912 print, what they leave out, and how the Phase 5 rules, vectors and allocation ids are tied to them.
Facts and short paraphrases only; no page text is copied and no page image is committed (the pages live in the gitignored local cache, D-28).
Labels: [OBS] read from a page image in plan 05-01, [INF] inferred from such readings, [UNK] not printed on the pages read.
The part in the console is named `AY-3-8914-class` by the Phase 5 decision D-07; the pages read here describe the AY-3-8910/8912, so every use of them for the console part is an inference and the matching rules are tagged `inference` (C11, C69).

## Source and method

Source copies: `SRC-gi-1980-catalog` (pdf 204-210, printed 5-23..5-29) and `SRC-gi-1982-catalog` (pdf 299-305, the same text).
Both catalogs were fetched again on 2026-11-12 into the gitignored cache and their size and sha256 equal the values pinned in `data/sources.json` [OBS].
The pages were rendered at 150 and 300 dpi into the cache and read as images; the text layer was used only to find pages, because it drops overbars and damages the figures.
The third-party reprint `SRC-gi-ay8910-manual` could not be obtained (see its registry note), so nothing in this directory is cited to it.
A different 8-page scan of an AY-3-8910 sheet was reachable and was not used: it is not the reprint and has no text layer.
No emulator source or table was used for any value.

## Page-reading verdict

These four lines answer, for the printed pages 5-23..5-29 only, whether the page prints the thing named.
They decide which Phase 5 vectors may be `kind=source` and which fall back to `kind=policy` (D-23).

equations-printed: no
shapes-printed: yes
amplitude-printed: no
dac-labels-printed: no

Per answer:

- `equations-printed: no`. The page for tone, noise and envelope period control (pdf 207, printed 5-26) gives the divide chain in words: the input clock is counted down by 16 and then by the 12-bit tone period; by 16 and then by the 5-bit noise period; by 256 and then by the 16-bit envelope period, and the envelope generator then counts down by 16 more to make a 16-state cycle [OBS]. The register bit figures on the same page show the field widths (TP11..TP0, NP 5 bits, EP15..EP0) [OBS]. No page prints a closed-form frequency equation, the lowest and highest period values, a worked envelope example, or a statement that the tone square wave has 50 percent duty; those items are [UNK] here. The 1982 catalog (pdf 302) has the same words.
- `shapes-printed: yes`. Fig. 3 (pdf 208, printed 5-27) draws the envelope output for the ten classes of R15 bits B3..B0 (continue, attack, alternate, hold) as waveforms, Fig. 4 shows two cycles of the `1010` class in staircase form, and Fig. 5 labels the sixteen levels 0..15 with the end value repeated at the turning point [OBS]. The one-sentence definitions of hold and alternate are not printed on these pages, so the sixteen shapes are read from the figure, not from text.
- `amplitude-printed: no`. The amplitude register diagram (pdf 207) shows bit B4 as "amplitude mode" and B3..B0 as the 4-bit fixed level, and the operation table on the same page says the registers select fixed or envelope-variable amplitudes [OBS]. No sentence states which value of the mode bit selects the envelope, and none states how the 4-bit envelope output maps to the converter input when it does [UNK].
- `dac-labels-printed: no`. The converter paragraph (pdf 208, printed 5-27) says the conversion is in logarithmic steps over a normalised 0 to 1 volt range, and Fig. 5 labels only the 1 volt top and the 0 bottom against the sixteen states [OBS]. No intermediate voltage label or table is printed on these pages [UNK].

Other readings that bear on the rules (not part of the four-line verdict):

- mixer-sense: the heading word `Enable` of register R7 and the labels `Noise Enable` and `Tone Enable` carry an overbar on the 300 dpi render (pdf 207), which is how the page marks active low; the page has no sentence saying a 0 bit enables [OBS]. The `Input Enable` cell of bits B7 and B6 shows no overbar I could resolve at 300 dpi in this run, so the direction sense of the port bits stays an inference (C73) [UNK].
- ports: pdf 205 (5-24) says every port pin has an on-chip pull-up, so a port in input mode reads high and the recommended way to scan switches is to ground the input bit; the port registers R16 and R17 are only a store between the data bus and the pins (pdf 208) [OBS].
- reset: pdf 205 (printed 5-24) says a logic 0 on the reset pin resets all registers to 0; it says nothing about generator state beyond the registers [OBS] (C77).
- bus: pdf 205 shows the sixteen registers as a 16-word block of a 1,024-word space selected by address bits A9 and A8 and the high data/address bits, and states that each register is readable [OBS].
- input clock: pdf 209 (5-28) lists the clock input as 1.0 to 2.0 MHz with 25 to 75 percent duty [OBS]. The console's value is an inference (C76).
- 8914: the string 8914 does not appear on pages 5-23..5-29 or in the pages 299-305 of the 1982 catalog [OBS]; every rule below that is not labelled otherwise is an 8910 reading applied to the 8914-class part by inference.

Consequences for the vectors (applied in the vector files):

- Rate vectors (tone, noise, envelope spacing) are derived from the printed divide chain and field widths. They stay `kind=source`, cite `SRC-gi-1980-catalog@pdf207`, and the `derive` words name the chain. Their rules carry basis `8910-primary-derived` (inference), and the duty and toggle granularity step is a project inference.
- Shape vectors are `kind=source` against Fig. 3 (shapes-printed: yes).
- Vectors that need the amplitude mode bit sense, the amplitude-to-converter mapping, the DAC voltage labels or the period ranges stated as a sentence are `kind=policy` with `unknown-default-chosen` basis (D-23 fallback), and C12 stays open; the printed 0..15 labels of Fig. 5 fix only the sixteen state count.
- The worked 0.5 Hz envelope example, the closed-form equations and the DAC voltage labels exist, if at all, only in the reprint that could not be obtained; no vector cites them.

## Register numbering

The data sheet numbers its registers in OCTAL: R0..R7, then R10, R11, R12 (decimal 8, 9, 10), R13 and R14 (decimal 11, 12), R15 (decimal 13), R16 and R17 (decimal 14, 15) [OBS, the page tables use R0..R5, R6, R7, R10..R12, R13..R15, R16..R17].
R10, R11 and R12 are the amplitude registers of channels A, B and C; R13 and R14 are the envelope period fine and coarse bytes; R15 is the envelope shape; R16 and R17 are the stores of ports A and B [OBS].
The Intellivision register map is a permutation of these, recorded in `data/psg/intv-2609-ntsc-family-p1.regmap.literal.txt` (tier 4, C70); every row carries both numberings, and the decimal and octal readings of one name are never mixed (C80).

## Rule table

The rules of `data/psg/rules.json`, one row per rule; the exact vector lists are in that file and are checked in both directions by the `psg-vectors` step.
Basis names how the page set backs the rule: `8910-primary-literal` is printed on pages 5-23..5-29 of the 1980 catalog, `8910-primary-derived` is derived from printed text by inference, `8914-community` rests on the community page (tier 4, one lineage), `default-unknown` has no usable source and carries a resolver and a trigger in its note.
The last column says whether a difference between the AY-3-8910 and the 8914-class part is known for the rule: nothing documents the 8914 beyond the community page, so where the column says unknown the rule is an 8910 reading applied by inference (C11, C69).

| Rule | Statement | Basis | Source tier | Vectors | Discrepancy | 8914 delta known |
|------|-----------|-------|-------------|---------|-------------|------------------|
| PSG-R01 | Sixteen registers, each readable and writable at its address. | 8910-primary-literal | tier 1 (AY-3-8910 pages) | reg.rewrite | C70 | no |
| PSG-R02 | The register permutation into 01F0..01FF: role per address as in the register-address literal. | 8914-community | tier 4 | psg.reg.* (7 rows) | C70 | yes, tier 4 only |
| PSG-R03 | Write and read masks: stores keep value AND the write mask, reads give the store AND the read mask with bits 15..8 zero. | 8914-community | tier 4 | psg.reg.* (19 rows) | C60 | yes, tier 4 only (C60, C75) |
| PSG-R04 | A read of any PSG register has no side effect. | default-unknown | none or inference | reg.read-twice-same | C70 | unknown |
| PSG-R05 | Tone frequency: the input clock divided by 16 and then by the 12-bit period, a square wave at 50 percent duty: half-period P steps (16 P master ticks). | 8910-primary-derived | tier 1 pages plus inference | psg.tone.* (13 rows) | C69 | unknown (C76 for the clock) |
| PSG-R06 | Tone period field is 12 bits, so the largest period is 4095. | 8910-primary-derived | tier 1 pages plus inference | tone.rate-p4095 | C69 | unknown |
| PSG-R07 | Period 0 acts as the range top: tone 4096 and envelope 65536 (community), noise 32 (default by analogy). | 8914-community | tier 4 | tone.period-zero, noise.period-zero, env.period-zero | C12 | yes, tier 4 only (noise zero unknown) |
| PSG-R08 | Tone counter structure: a down-reload counter, a period write takes effect at the next reload. | default-unknown | none or inference | tone.next-reload | C12 | unknown |
| PSG-R09 | Noise rate: the input clock divided by 16 and then by the 5-bit period; the register shifts every 2 NP steps. | 8910-primary-derived | tier 1 pages plus inference | psg.noise.rate-np*, first-shift, no-shift (7 rows) | C69 | unknown |
| PSG-R10 | Noise register polynomial, seed and output sense: right-shifting Galois, width 17, mask 12000 hex, seed 1, output bit 0. | default-unknown | none or inference | noise.lfsr-maximal-period, noise.lfsr-alt-period, noise.lfsr-first-bits, noise.level-seq-np1 | C71 | unknown (no source) |
| PSG-R11 | Envelope rate: the input clock divided by 256 and then by the 16-bit period gives the cycle, one state per 16 EP chip clocks, so one step per 2 EP PSG steps. | 8910-primary-derived | tier 1 pages plus inference | env.rate-ep1, env.rate-ep2, env.rate-ep3, env.rate-ep15625 | C72 | unknown (step reading is inference) |
| PSG-R12 | The sixteen envelope shapes of R15 bits B3..B0 (continue, attack, alternate, hold) with sixteen states per cycle. | 8910-primary-literal | tier 1 (AY-3-8910 pages) | psg.env.shape-0..15 | C72 | unknown for the 8914 |
| PSG-R13 | A write to the shape register restarts the envelope. | 8914-community | tier 4 | env.restart-on-shape-write | C72 | yes, tier 4 only |
| PSG-R14 | Mixer polarity and bit order: R7 bits 0..2 disable tone A..C, bits 3..5 disable noise A..C, 1 = disabled. | 8910-primary-literal | tier 1 (AY-3-8910 pages) | mix.tone-only-1, mix.tone-only-0, mix.noise-only-1, mix.noise-only-0, mix.channel-isolated | C69 | unknown for the 8914 |
| PSG-R15 | Tone and noise combine by AND after the disable bits are applied; a channel with both disabled is a constant 1. | 8910-primary-derived | tier 1 pages plus inference | mix.truth-table, mix.and-tone0, mix.and-noise0, mix.and-both0, mix.both-disabled-const1 | C69 | tier 4 only |
| PSG-R16 | Amplitude: bits 3..0 of the amplitude register are the 4-bit fixed level. | 8910-primary-literal | tier 1 (AY-3-8910 pages) | amp.fixed-15, amp.fixed-0, amp.fixed-masks-upper | C70 | unknown (8914 field width unverified) |
| PSG-R17 | Amplitude select bits 5..4: 0 fixed, 1 envelope shifted right by 2, 2 envelope shifted right by 1, 3 envelope. | 8914-community | tier 4 | amp.env-mode, amp.env-shift2, amp.env-shift1, ampenv.env-through-amp, ampenv.shift2 | C12 | yes, tier 4 only (6-bit variant) |
| PSG-R18 | Step scheduling and write ordering: a write stamped at a step boundary lands after that step (step first, then write). | default-unknown | none or inference | ampenv.write-after-step-n, ampenv.write-at-0-visible | C78 | not a hardware rule |
| PSG-R19 | DAC levels: sixteen output levels of a logarithmic converter between 0 and 1 volt. | default-unknown | none or inference | psg.dac.* (8 rows) | C12 | unknown (no printed labels) |
| PSG-R20 | Port reads: input mode follows the pins with an on-chip pull-up idle of FF, output mode returns the store, a write stores the latch. | default-unknown | none or inference | psg.port.* (10 rows) | C65 | unknown |
| PSG-R21 | Reset: all registers 0, both ports input; generator state at its defaults. | 8910-primary-literal | tier 1 (AY-3-8910 pages) | reset.regs-zero, reset.ports-input-after-reset, reseng.silent-after-reset, reseng.envelope-held-zero | C77 | unknown for generator state |

Vector totals: 120 vectors in 11 files, 95 `kind=source` and 25 `kind=policy` (12 policy ids in `data/expected/psg-policies.ids`).
Every policy row and every row of a rule whose basis is `default-unknown` is a default chosen under the stated discrepancy, never a hardware claim.
The DAC rows are all policy: the per-level labels are not on the pages read (page-reading verdict), and their values come from a planning-note reading of a reprint that could not be obtained, so they are not independent.
Expected values were checked with a second, throwaway model written after the vectors (a shared-misreading risk, not an independent implementation); blind second reads of every vector file were run afterwards and are recorded in `docs/psg/second-read-record-*.md` (request: `docs/psg/second-read-request.md`; tier `source-expectation, second-read`, same model family and same pages, so a transcription cross-check and not evidence about any console).

## Allocation table

ONE table for all Phase 5 ids (D-12). A profile field, a rule or a claim cites an id from here and never invents one; ids are never reused.
The discrepancy records C69..C80 exist in `data/discrepancies.json` and `data/expected/discrepancies.ids` as of plan 05-01, so a later `conflict` reference resolves.

| Id | Topic | Profile field or contract |
|----|-------|---------------------------|
| C69 | data sheet applicability to the 8914-class part, and the chip step | `psg.clocks_per_step` |
| C70 | register permutation and the R8 and R10 amplitude layouts | `psg.regmap` |
| C71 | noise LFSR width, polynomial, seed and sense | `psg.noise.lfsr` |
| C72 | envelope step rate and restart on shape write | engine, rules PSG-R11 and PSG-R13 |
| C73 | port direction bit order of R7 | `psg.mixer.port_dir_bits` |
| C74 | decode aliasing and mirrors | `psg.mirrors` |
| C75 | read-back of R10 and of the shape register | read masks of `psg.regmap` |
| C76 | PSG chip clock = crystal/2, and its phase | `ticks_per_psg_clock`, `psg.clock_phase_ticks` |
| C77 | power-on and reset state beyond zero, and whether the PSG reset follows the system reset | `psg.reset` |
| C78 | write-effect instant, step-first-then-write tie rule, sample alignment (a project contract like C67, not a hardware claim) | `psg.write_strobe_ticks`, engine |
| C79 | raw 16-bit write value in the log versus the 8-bit register | log format, write masks |
| C80 | octal versus decimal register numbering across the sources | every register table |

Existing ids that Phase 5 updates and does not reopen: C11, C12 (partial), C14, C15 (partial), C16, C23 (partial), C60, C63, C64, C65, C66, C67.
The next free discrepancy id after this table is C81.

Claim ids reserved for later plans (the records are written by plans 05-11 and 05-12, because a claim needs an existing artifact, D-26); all are provisional:

| Claim id | Scope |
|----------|-------|
| CL-0009 | single-control table (31 controls) |
| CL-0010 | control combinations |
| CL-0011 | slicing-invariant input |
| CL-0012 | PSG rule vectors |
| CL-0013 | write log and samples |
| CL-0014 | the diagnostic fixture `ivs-diag-ctrl-psg-1` |
| CL-0015 | installed-package replay |

The next free claim id is CL-0016.

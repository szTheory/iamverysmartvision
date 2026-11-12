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

Filled in by Task 4 of plan 05-01.

## Allocation table

Filled in by Task 2 of plan 05-01.

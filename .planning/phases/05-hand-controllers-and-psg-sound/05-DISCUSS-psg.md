# Phase 5 discuss: Decision point P, PSG chip rules (AUD-01; C11, C12)

Status: recommendation (fan-out protocol, `.planning/AUTONOMY.md` section 2). Labels: [OBS] read or run in this session, [INF] inference, [REC] recommendation, [UNK] unknown, [BG] background not re-fetched.
Inputs read [OBS]: AGENTS.md, AUTONOMY.md s1-3, PROJECT.md Decision Register (D2, D3, D6), REQUIREMENTS (AUD-01, AUD-02, INP-01..03), ROADMAP Phase 5, `docs/time-base.md`, `docs/evidence-tiers.md`, `data/tiers.json`, `data/sources.json`, the profile record `data/profiles/intv-2609-ntsc-family-p1.json` and its schema (`tools/check/schema/profile.txt`), `data/discrepancies.json` (C8, C9, C11, C12, C14, C60, C63-C67), `data/memmap/*.literal.txt`, `core/src/memmap/memsys.h`, `core/src/cpu/cp1610.h` (bus access record), `core/src/machine/machine_run.c` (unit scheduling).
Primary documents read in this session [OBS], rendered to page images where the text layer was damaged, kept in an uncommitted scratch directory:
- GI Microelectronics Data Catalog 1980 (`SRC-gi-1980-catalog`, sha256 matches the registry entry), AY-3-8910/8912 section, pdf 204-210 (printed 5-23 to 5-29).
- A third-party reprint of the full GI AY-3-8910/8912 Programmable Sound Generator Data Manual (55 pages; sha256 `3fbc9b40c7736ab79270e274bb6688f2d9f0dab693d7eb8060d6d8d29cd73deb`; hosted on a public file CDN; the cover says "reprinted with permission" and that some pages are excluded). Not yet in `data/sources.json`; proposed id `SRC-gi-ay8910-manual` below.
- The community wiki PSG page (`SRC-intv-wiki`, oldid 14940, last edited 2011-09-28) and three of its register sub-pages (current revision at fetch time, id not captured), and the community emulator documentation file `doc/programming/psg.txt` (`SRC-community-emu-docs`, GPL doc tree: facts only, nothing copied).
No emulator source code was opened. Emulator behaviour appears below only as a flagged lead (tier 5) where it explains a discrepancy.

## 1. The decision in one paragraph

[REC] Model the PSG as "the GI AY-3-8910 functional core, as documented by GI, plus the Intellivision register permutation and the two-bit envelope select of the AY-3-8914-class part, as documented by community sources". Every rule gets an id (PSG-Rnn), a stated basis (`8910-primary-literal`, `8910-primary-derived`, `8914-community`, `default-unknown`), a profile field carrying a source-authority tag, and a vector. No rule is promoted to `tier1-primary-doc` for the 8914-class part, because no 8914 datasheet exists in any source found; the 8910 text is carried as `inference` ("sibling datasheet applied to the named part") and the claims that rest on it are therefore `provisional` by the weakest-field rule, which is the honest standing. The engine is a pure, integer-only module with one fixed time quantum, the PSG step (8 chip clocks = 16 master ticks = 4 CPU cycles on the NTSC profile). Behaviours with no primary source (period 0, noise LFSR, DAC levels, envelope-select map, read-back of unused bits, port read in output mode, register permutation) are profile data with tags and discrepancy ids. The raw-audio evidence is the per-step triple of 4-bit level indices, which does not depend on the DAC table.

## 2. What the evidence supports (and does not)

### 2.1 Strong: the GI AY-3-8910/8912 text and figures [OBS, tier 1 for the 8910]

- Register numbers in GI's text are OCTAL (R0..R17). The reprint's cover note says so explicitly ("R10 is really 8"). R10, R11, R12 are the amplitude registers; R13, R14 are envelope period fine and coarse; R15 is envelope shape; R16, R17 are the I/O port stores. Misreading octal as decimal is a classic footgun and the register audit table (section 6) carries both numberings.
- Tone: f_T = f_clock / (16 x TP), TP 12 bits, "lowest period value 1, highest 4095" (reprint p.16-17; catalog printed 5-26). The statement "count the input clock down by 16, then by TP" gives a square wave of frequency f_clock/(16 TP) [INF, with the printed equation]. A 50 percent duty square wave of that frequency toggles every 8 x TP chip clocks. [INF] That fixes the toggle granularity at 8 chip clocks, not 16.
- Noise: f_N = f_clock / (16 x NP), NP 5 bits, "lowest 1, highest 31" (reprint p.18). The text defines a noise "frequency", not a shift-register clock or a polynomial.
- Envelope: f_E = f_clock / (256 x EP), EP 16 bits, "lowest 1, highest 65535" (reprint p.22). The worked example is f_E = 0.5 Hz at 2 MHz giving EP = 15625 (p.23). Fig. 3 and Fig. 5 label EP as "the envelope period (duration of one cycle)". So f_E is the cycle (16-state ramp) frequency, and one envelope step is 16 x EP chip clocks. [INF] The sentence "further counts down by 16 producing a 16-state pattern" is consistent: 256 = 16 (prescaler, same as tone) x 16 (states). The alternative reading (step = 256 x EP clocks) contradicts the figure captions and the worked example, so it is rejected, and kept as named fault F3.
- Envelope shape R15 bits: B3 Continue, B2 Attack, B1 Alternate, B0 Hold. Definitions (reprint p.23-24): Hold limits to one cycle holding the last count (0 or 15); Alternate reverses direction each cycle; "when both Hold and Alternate are ones the envelope counter is reset to its initial count before holding"; Attack=1 counts 0 to 15, Attack=0 counts 15 to 0; Continue=0 means the generator "resets to 0000 after one cycle and holds". Fig. 3 draws all 10 shape classes (00xx, 01xx, 1000..1111). Fig. 9 (DAC output) shows a doubled step at the extremes of the triangle (labels "0 0" at the bottom), so each cycle has 16 states with end values repeated across a cycle boundary.
- Amplitude (R10-R12 octal): B4 = M (envelope enable), B3..B0 = fixed level; B7..B5 unused. Disabling noise and tone does not silence a channel (it outputs a constant); only a zero amplitude does (R7 note, reprint p.19).
- Mixer/enable R7: 0 = enabled, 1 = disabled (truth tables, reprint p.19: noise bits B5 B4 B3 = C B A, tone bits B2 B1 B0 = C B A). I/O direction: B7 = port B, B6 = port A, 0 = input, 1 = output (the figure's "I/O Port: B A" under B7 B6 and the I/O Port Truth Table). Reset pin low resets all registers to 0 (catalog printed 5-24; reprint p.11).
- DAC: "logarithmic steps, normalized 0 to 1 V" (text) and Fig. 9 axis labels (reprint p.27 at 300 dpi): level 15 = 1 V, 14 = .707 V, 13 = .5 V, 12 = .303 V, 11 = .25 V, 10 = .1515 V, 9 = .125 V; levels below 9 are not labelled; level 0 sits on the 0 V axis. Note .303 and .1515 are not on a pure 3 dB (square root of 2) ladder (which would give .3536 and .1768). Either the figure is drawn from measurement or it contains an error; unresolved [UNK].
- Clock: the manual's own application circuit divides a 3.579545 MHz colour-burst crystal by two to get 1.7897725 MHz for the PSG (reprint p.29, Fig. 15), inside the specified 1.0-2.0 MHz range (catalog printed 5-28). [OBS] This is application advice, not the 2609 board, so the chip clock stays an inference for the profile.
- Bus: the PSG decodes BDIR/BC2/BC1 from the CP1600 directly (INTAK/ADAR latch address, DTB read, DWS write, NACT/IAB inactive). The 16 registers occupy a 16-word block of a 1,024-word space. Chip select = address bits A9, A8 (fixed to code 01) plus DA7..DA4 (mask-programmable, default 0000); the low four DA bits pick the register; the latched address stays valid until a new valid address (reprint bus section, page not recorded; catalog printed 5-24). Data is 8 bits (DA7..DA0). Each I/O pin has an on-chip pull-up so an input port reads high unless grounded; in input mode the port register "follows the signals applied to the pins" and a read transfers it (reprint p.26).
- 8900 system tie: the 1980 catalog's GIMINI 8900 system overview lists the AY-3-8910 as the optional sound generator of the CP1610/STIC set (printed 5-9). [OBS] This is the only tier 1 link between the console family and an AY part. It does not mention the 8914.

### 2.2 Weak: everything specific to the Intellivision's AY-3-8914-class part [OBS, tier 4]

- The permutation of registers into $01F0-$01FF, the two-bit envelope select and the per-register read masks come from two community sources that share a lineage (the community emulator doc file and the wiki PSG page; the wiki lists "R0..R15 consecutive at $01F0..$01FF" and says the AY-3-8910 "uses different register numbers"). The emulator doc file additionally gives the octal equivalent of each address, which is the only place a mapping to the GI octal numbering is stated. Independent weak corroboration of "the 8914 is an 8910 with shuffled control registers": the sdiy.info wiki and a tracker-software doc ("different register format", "4-level envelope volume per channel"). These are secondary and likely derive from the same lineage. [OBS]
- No AY-3-8914 datasheet turned up: the 1980 catalog text layer has no "8914" (grepped in this session; the 1977 and 1982 catalogs were grepped in the research era per the source registry), the reprinted manual has none, and a few web searches found none. [OBS] The roadmap's research flag stands: the 8914 is never directly documented here.
- Discrepancies inside tier 4 and against tier 1 (each becomes a numbered entry, section 5.3):
  - Port direction bits: the GI figure has B7 = port B and B6 = port A; the emulator doc file states b7 = port A, b6 = port B. [OBS]
  - R10 (shape): the wiki lists data bits $0000 ("write only"); the GI 8910 text says all 16 registers are readable. [OBS]
  - Period zero: the wiki says tone period 0 acts as 4096 and envelope period 0 as $20000 (that is 2 x 65536 PSG-step units); the GI text defines the range as 1..4095 and 1..65535 and does not define 0; the usual practice of other emulators for the 8910 is that 0 acts as 1 (tier 5 lead, recalled, not re-read). [BG]
  - Noise generator: the wiki gives a shift formula whose feedback mask is $14000 (Galois, shift right). I computed [OBS, executed]: that recurrence has a cycle length of 114,681, not maximal. The mask $12000 (bits 13 and 16) has the maximal cycle 131,071 and its output is a cyclic shift of a Fibonacci register that XORs bits 0 and 3 into bit 16, which is the form most often quoted for the AY family (search results, tier 5 leads, no die analysis found). So the wiki mask is either a typo or a real difference. Neither is primary. The seed is unknown (one list post says all zeros is impossible to observe because the generator runs from power-up) [OBS via search].
  - Envelope amplitude select: ee = 01 shifts the envelope level right by 2, 10 right by 1, 11 passes it through, 00 selects the fixed level. The same two sources agree. The doc says AY-3-8916/8917 implement one select bit and read back $1F after writing $3F. [OBS, tier 4]
- Unused-bit reads: the wiki "Off Bits" column makes bits above the register width read 0 including the whole upper byte. No tier 1 to 3 source (C60 stays open); on a real 16-bit bus the upper byte is simply not driven by the PSG and its value depends on pull-ups. [INF]

### 2.3 Consequence for time-base.md [OBS, a correction to record]

`docs/time-base.md` defines "PSG step: 16 PSG chip clocks [INF] ... 32 master ticks". With the datasheet equation f_T = f_clock/(16 TP) and a 50 percent duty cycle, the tone output toggles every 8 x TP chip clocks, so the natural integer quantum is 8 chip clocks = 16 master ticks (with `ticks_per_psg_clock` = 2). A 16-clock step would need half-integer toggle intervals. The community formula quoted in the same glossary entry, F = crystal / (32 P), is the same statement: half-period 16 P master ticks. Phase 5 must correct the glossary entry (and `data/expected/glossary-terms.ids` if the term text changes) to "8 chip clocks, 16 master ticks, 4 CPU cycles on NTSC" and stop saying 32. This is the only already-written statement that the evidence contradicts.

## 3. Options considered

| | A. Full 8910 emulation with the 8914 as a thin register-remap | B. Engine hard-codes 8914 community rules | C (chosen). Pure integer engine; every non-datasheet behaviour is profile data; datasheet-literal rules are engine rules labelled by id |
|---|---|---|---|
| What changes when the 8914 turns out different | engine edit | engine edit | profile edit plus vectors |
| Honesty about evidence | implies the 8910 sheet is the 8914 sheet | implies the wiki is a sheet | each rule has basis, tag, discrepancy |
| Cost | small | small | one table, 17 profile fields, a rule inventory |
| Fits roadmap SC4 ("not hard-coded constants") | no | no | yes |

Sub-options, decided in section 4: tone counter structure (down-reload vs up-compare), LFSR form (Fibonacci vs Galois), DAC (printed levels vs idealised 3 dB), sample payload (level indices vs weighted PCM), I/O port latch (separate vs shared).

## 4. Numbered decisions

P-1 [REC] Part identity and basis. The PSG part stays `AY-3-8914-class, Intellivision register map` (tag `tier4-community`, existing field `psg.part`). Add a rule inventory `data/psg/rules.json` (ids `PSG-R01..`), each with: statement, `basis` in {`8910-primary-literal`, `8910-primary-derived`, `8914-community`, `default-unknown`}, source locators (SRC id plus pdf or printed page), the profile field(s) it reads, the vector ids that cover it, and the discrepancy id. The gate checks that every rule has a profile field or a literal engine marker, at least one vector, and a source or an `unknown-default-chosen` tag with resolver and trigger. This is the "audited rule by rule" deliverable of the research flag, in a form the gate can enforce. Copy the pattern of `data/cp1610/*` and `tools/check/schema/*.txt`; no new tooling idea is needed.

P-2 [REC] Register file (rule PSG-R01..R04). Sixteen registers at `$01F0..$01FF`, one 16-bit bus word per register, only the low byte driven. Profile field `psg.regmap` (tag `tier4-community`, conflict new C70): an array of 16 records `{addr, name, gi_octal, wmask, rmask}`:

| addr | name | GI octal (8910 doc) | wmask | rmask |
|---|---|---|---|---|
| $01F0 | chA period fine | R0 | $00FF | $00FF |
| $01F1 | chB period fine | R2 | $00FF | $00FF |
| $01F2 | chC period fine | R4 | $00FF | $00FF |
| $01F3 | envelope period fine | R13 | $00FF | $00FF |
| $01F4 | chA period coarse | R1 | $000F | $000F |
| $01F5 | chB period coarse | R3 | $000F | $000F |
| $01F6 | chC period coarse | R5 | $000F | $000F |
| $01F7 | envelope period coarse | R14 | $00FF | $00FF |
| $01F8 | mixer and port direction | R7 | $00FF | $00FF |
| $01F9 | noise period | R6 | $001F | $001F |
| $01FA | envelope shape | R15 | $000F | $0000 (C75) |
| $01FB-$01FD | chA, chB, chC amplitude | R10, R11, R12 | $003F | $003F |
| $01FE | port A (right connector per tier 4, C66) | R16 | $00FF | $00FF, see P-9 |
| $01FF | port B | R17 | $00FF | $00FF, see P-9 |

Stores keep `value & wmask`; reads give `reg & rmask` with bits 15..8 zero (tier 4 "Off bits", conflict C60; this replaces the stub's $FFFF only for the PSG region once the stub becomes a device). Reads of any PSG register have no side effects. R10 reads 0 under the tier 4 claim and the 8910 text disagrees (C75); the default follows the part-specific source and the 8910 reading is a one-line profile change (`rmask` $000F). The 16 words are the whole decode; no mirrors (field `psg.mirrors` = `none`, tag `unknown-default-chosen`, conflict new C74: the 8910 chip select sees only A9, A8 and DA7..DA4, so on a bus whose upper address bits are not decoded externally the part may alias; no source documents the 2609 decode).

P-3 [REC] Bus protocol seen by the CP-1610. The model is memory-mapped: one logical read per READ access and one logical write per WRITE/DW access at the address phase. The BDIR/BC1/BC2 encodings, the address latch and its "stays valid" property are not modelled as state because every CP-1610 memory access begins with an address cycle, so the latch is always reloaded; this is an [INF] with a stated limit: it is exact for a CP-1610 master and wrong for a master that does back-to-back data cycles without re-addressing (the ECS keyboard scan does not do that). The existing `ivs_memsys_bus` dispatch is not tick-aware, so the machine layer must pass the access time to the PSG device: write tick = unit start tick + `offset_cpu_cycles` x `ticks_per_cpu_cycle` (the BAR cycle start; the data strobe is later in the same CPU cycle, sub-cycle placement is [UNK] and recorded as `psg.write_stamp`, tag `delegated-decision`, a contract not a hardware claim, like C67). ADAR-kind reads are treated as ordinary side-effect-free reads.

P-4 [REC] Time chain (rules PSG-R05..R07, field `psg.clocks_per_step` = 8, tag `inference`, derived from the printed equation f_T = f_clock/(16 TP) and 50 percent duty). Derived at profile load: `ticks_per_psg_step = ticks_per_psg_clock x psg.clocks_per_step` = 2 x 8 = 16 master ticks, which is exactly 4 CPU cycles on NTSC. The existing field `ticks_per_psg_clock` = 2 should be retagged from `unknown-default-chosen` (backed only by an emulator hint) to `inference`, citing three non-emulator facts: the community formula F = crystal/(32 P) (tier 4), the GI manual's colour-burst/2 application circuit (reprint p.29), and the 1.0-2.0 MHz clock range (catalog printed 5-28); the tier 5 hint stays in `note` only. Add discrepancy C76 (PSG chip clock derivation) and keep the D10 tone-capture resolver. The PSG state machine advances in whole steps at ticks that are multiples of `ticks_per_psg_step` (phase `psg.clock_phase_ticks` = 0, `unknown-default-chosen`, counted from reset). One step is the finest time at which any PSG output can change. Per step:
- each tone channel: down counter reloaded from the effective period; toggles when it expires, so the half-period is exactly P steps (16 P master ticks) for P >= 1 [INF from the equation];
- noise: LFSR shifts every 2 x NP steps (NP effective) [INF: f_N = f_clock/(16 NP) read as the shift rate, because the text defines the noise frequency by the same divider as the tone frequency];
- envelope: one envelope step every 2 x EP steps (EP effective) [INF: 16 EP clocks per step, primary-supported, section 2.1].
All counts are integers, so no fractional accumulator exists in the core. Samples per frame are 3,733.5 (59,736 / 16), which is why frames are never PSG-aligned (consistent with D2's "a frame is never derived by modulo").

P-5 [REC] Tone generator (PSG-R05, PSG-R08). Counter model: `down-reload` (a counter loaded with the effective period, decremented each step, toggle and reload on expiry; a write to a period register changes the value used at the next reload and does not disturb the running count). Rationale: it is the only model that makes the tier 4 "period 0 acts as 4096" claim natural (a 12-bit down counter wraps), and in steady state with P >= 1 it equals the up-compare model, so the two diverge only for period writes during a half-cycle and for P = 0. The model name is recorded in the profile as `psg.counter_model` (tag `unknown-default-chosen`, folded into C12). Do not build both models speculatively. Tone output flip-flop is 0 after reset and keeps toggling while the channel's tone is disabled (the mixer gates it, the counter does not stop), standard behaviour of the family [BG].
Period zero (profile, three separate fields so each carries its own tag): `psg.period_zero.tone` = 4096 (tier4, C12), `psg.period_zero.noise` = 32 (`unknown-default-chosen`: the same wrap rule extended by analogy; no source), `psg.period_zero.envelope` = 65536 (tier4, C12). The effective period is `value == 0 ? period_zero.X : value`. The alternative "0 acts as 1" is expressible by editing three numbers and is the reversal for a measured part.

P-6 [REC] Noise generator (PSG-R09, field `psg.noise.lfsr`, tag `unknown-default-chosen`, conflict new C71). One engine form: a right-shifting Galois register, `if (s & 1) s = (s >> 1) ^ mask; else s >>= 1`, output bit `out_bit`, width 17. Profile value `{width: 17, galois_mask: "$12000", seed: "$00001", out_bit: 0, invert: 0}`. Rationale: one code path covers both candidate polynomials (the wiki's $14000 is a mask edit), the default has the maximal period (131,071, checked), and the output stream equals a Fibonacci XOR-of-bits-0-and-3 register up to a cyclic offset, so a reader coming from either form can compare. Seed 1 is a nonzero convention (zero locks the register); the real power-up state is unobservable [UNK]. The noise bit, when 1, passes the noise side of the mixer. `psg.noise.steps_per_np` = 2 (tag `inference`, P-4). Reset reloads the seed (inference).

P-7 [REC] Envelope generator (PSG-R10..R12). Engine rule written from the GI text and Fig. 3/9 (basis `8910-primary-derived`): state (level 0..15, direction, held flag, cycle count); at start level = Attack ? 0 : 15 and direction accordingly; one cycle is 16 states; at the end of a cycle: Continue = 0 gives level 0 held; Continue = 1 and Hold = 1 gives, with Alternate = 0, the last level held, with Alternate = 1 the initial level of the finished cycle held (so 1011 holds 15 and 1111 holds 0); Continue = 1, Hold = 0 gives, with Alternate = 0, a restart at the initial level, with Alternate = 1 a direction reversal. A write to the shape register restarts the generator (level, direction, step counter reloaded from the effective period; tier 4 "envelope is reset each time R10 is overwritten", rule PSG-R13, field `psg.envelope.shape_bits` = `{continue: 3, attack: 2, alternate: 1, hold: 0, restart_on_write: 1}`, tag `inference`, conflicts C70, C72). Step spacing `psg.envelope.steps_per_ep` = 2 (tag `inference`). Period writes do not restart the generator. After reset the generator holds level 0 until the first shape write (field `psg.reset`, `unknown-default-chosen`): a guest that selects the envelope before ever writing the shape register gets silence. The 16-step table (not 32: the 32-step envelope is the YM2149 and is named in C12 as a different part) is the engine's constant, because it is the 8910 datasheet's literal behaviour; `steps = 16` appears as a vector precondition, not a profile field.

P-8 [REC] Mixer and amplitude (PSG-R14..R17). Mixer bits (field `psg.mixer.bits`, tag `inference`: 8910 figure applied, plus the community map putting the same bits in R8): enable bit polarity 0 = enabled, tone bits 0..2 = A..C, noise bits 3..5 = A..C. Port direction bits (field `psg.mixer.port_dir_bits`, tag `unknown-default-chosen`, conflict new C73): the GI figure says B6 = port A, B7 = port B; the community doc says the reverse; default is the primary figure, and the reversal is a profile edit. Combination: `chan = (tone_out | tone_dis) & (noise_out | noise_dis)`; a channel with both disabled is a constant 1. [INF] The AND form is standard but the GI text does not print it; the wiki's equation (noise_enable OR noise_out) AND (tone_enable OR tone_out) with "enable" meaning the disable bit is identical. Amplitude word: fixed level = bits 3..0, envelope select `ee` = bits 5..4 (field `psg.amp.env_select`, tag `tier4-community`, conflict C12): ee 0 selects the fixed level, 1 gives envelope >> 2, 2 gives envelope >> 1, 3 gives the envelope as is; effective level = 4-bit index; the shift is applied to the index before the DAC lookup. The channel's sample is `chan ? effective_level : 0` (index 0 is the 0 V level, so a gated-off channel and a level-0 channel are indistinguishable in the sample, which is the physical truth).

P-9 [REC] I/O ports R14, R15 (the controller path, shared with Decision point INP). Each port is 8 bits. Input mode: the read returns the external pin byte, which the controller model supplies as "bits driven low" and which idles at $FF because of the on-chip pull-ups (GI text; tag `inference`, matches the community statement that an idle controller reads all ones, C64). Output mode: the read returns the stored latch (C65, `unknown-default-chosen`). A write always updates the latch; in input mode the latch is not visible on the pins (the GI text says the register "follows the pins" in input mode, which would overwrite a written value; the separate-latch default is the weaker commitment and affects only the ECS-style scan case). Field `psg.port` = `{idle_level: "$FF", input_read: "pins", output_read: "latch", write_in_input_mode: "latch-only"}`, tag `unknown-default-chosen`, conflicts C64, C65. The core owns only `pins = ext_low_bits ^ 0xFF`; the control-to-byte function belongs to the INP decision. Hand-off interface: the machine layer sets one 8-bit "driven low" mask per port at CPU-unit boundaries (C67), so the PSG never sees a switch.

P-10 [REC] Reset. The PSG register file resets to 0 (the datasheet reset pin), matching the existing `power_on_fill.psg` = zero. The generator state resets as: tone counters empty and reloaded on the first step, tone outputs 0, LFSR = seed, envelope held at 0, ports in input mode with latch 0. Machine reset resets the PSG (the 2609 reset wiring is [UNK], recorded in `psg.reset`).

P-11 [REC] DAC levels (field `psg.dac.levels`, tag `inference`, conflict C12). Store 16 unsigned integers in units of 1e-8 of full scale. Default derived from the GI Fig. 9 axis labels, no emulator table involved: levels 15..9 are the printed voltages; levels 8..1 continue the printed pattern of halving every two levels (level n = level n+2 divided by 2, exact in these units); level 0 = 0.

| n | 15 | 14 | 13 | 12 | 11 | 10 | 9 | 8 |
|---|---|---|---|---|---|---|---|---|
| value | 100000000 | 70700000 | 50000000 | 30300000 | 25000000 | 15150000 | 12500000 | 7575000 |

| n | 7 | 6 | 5 | 4 | 3 | 2 | 1 | 0 |
|---|---|---|---|---|---|---|---|---|
| value | 6250000 | 3787500 | 3125000 | 1893750 | 1562500 | 946875 | 781250 | 0 |

[OBS, executed arithmetic] The sum of three channels is at most 300,000,000, which fits `uint32_t`. Per-entry basis is written in the field `note` (9..15 printed; 0 from the text; 1..8 extrapolated, `inference`). A second named table "pure 3 dB ladder" is documented as the alternative and is not shipped, to keep dead data out. The DAC table is not part of the raw audio evidence (P-12); it is used only by the optional weighting helper, so a wrong table cannot invalidate raw audio goldens.

P-12 [REC] Raw audio and log shape (AUD-02 interface, a recommendation for the plan; D7 stays with its own decision point). Register-write log record: `{tick (u64), reg index 0..15, written word (u16, as sent by the CPU before masking)}`, appended when the write is applied, ordered by tick then by access order. Chip-rate sample: one `uint16_t` per PSG step, `level_C << 8 | level_B << 4 | level_A`, each the post-mixer effective level index of P-8. Sample k is stamped at master tick k x `ticks_per_psg_step`; the exact rational sample rate for hosts is `crystal_hz / ticks_per_psg_step`, which equals 223,721.5625 Hz on the NTSC profile and is published as data (`ticks_per_psg_step` in `ivs_info`), never as a float. PCM conversion, weighting by the DAC table and resampling live in the runner or a host adapter. The core API stays caller-buffer, bounded, and slicing-invariant (same bytes whether one call or many).

P-13 [REC] Scheduling and slicing invariance (rule PSG-R18). The PSG is advanced lazily (Option B catch-up). To apply a write at tick w, first process every step with boundary tick b <= w, then apply the write. At the end of a run the PSG is advanced to the run's `now_tick` over boundaries strictly below it; a boundary exactly at `now_tick` belongs to the next run. This is consistent for any slicing because a run always stops at a CPU-unit boundary. The PSG state and its log position enter the machine digest and snapshot (the digest framing is an existing Phase 4 concern; the plan must extend it and the run-record goldens that include it).

## 5. Profile-data schema (follow the repo patterns)

### 5.1 New profile fields

All follow `tools/check/schema/profile.txt` exactly (keys `value tag cite conflict resolving_evidence revisit_trigger decision note`), are added to `data/expected/profile-fields.ids`, and respect the unit-suffix rule of `docs/time-base.md` (no field path ends in `period`, `offset`, `hold`, `delay`, `window` and so on; the one tick-valued field ends `_ticks`).

| field | value (summary) | tag | conflict | note |
|---|---|---|---|---|
| `psg.regmap` | 16 records, P-2 | tier4-community | C70 (new), C60 | single lineage, no tier 1 to 3 |
| `psg.mirrors` | "none" | unknown-default-chosen | C74 (new) | resolver: D10 decode probe |
| `psg.clocks_per_step` | 8 | inference | C69 (new) | derived from f_T = f_clock/(16 TP) |
| `psg.counter_model` | "down-reload" | unknown-default-chosen | C12 | steady-state equal to up-compare |
| `psg.period_zero.tone` | 4096 | tier4-community | C12 | |
| `psg.period_zero.noise` | 32 | unknown-default-chosen | C12 | analogy only |
| `psg.period_zero.envelope` | 65536 | tier4-community | C12 | wiki states $20000 PSG-step units = 2 x 65536 |
| `psg.noise.steps_per_np` | 2 | inference | C69 | f_N = f_clock/(16 NP), 8-clock step |
| `psg.noise.lfsr` | {17, $12000, $00001, 0, 0} | unknown-default-chosen | C71 (new) | candidate B mask $14000 |
| `psg.envelope.steps_per_ep` | 2 | inference | C72 (new) | 16 EP clocks per step |
| `psg.envelope.shape_bits` | {3,2,1,0,restart 1} | inference | C70, C72 | bit roles from 8910 text, 8914 R10 per tier 4 |
| `psg.amp.env_select` | {fixed 3..0, select 5..4, 0:fixed 1:>>2 2:>>1 3:>>0} | tier4-community | C12 | 6-bit variant; 8916/8917 differ |
| `psg.dac.levels` | 16 integers, P-11 | inference | C12 | per-entry basis in note |
| `psg.mixer.bits` | {polarity 0=enabled, tone 0..2, noise 3..5} | inference | C69 | |
| `psg.mixer.port_dir_bits` | {A: 6, B: 7, output: 1} | unknown-default-chosen | C73 (new) | sources disagree |
| `psg.port` | {$FF, pins, latch, latch-only} | unknown-default-chosen | C64, C65 | |
| `psg.reset` | {regs zero, lfsr seed, tone 0, env held 0, ports input} | unknown-default-chosen | C69 | |
| `psg.clock_phase_ticks` | 0 | unknown-default-chosen | C76 (new) | |
| `psg.write_stamp` | {basis: bar_cycle_start, ties: steps_at_or_before_write_first} | delegated-decision | none | architecture contract, decision id to be assigned in CONTEXT.md |

Existing fields touched: `ticks_per_psg_clock` retag to `inference` (P-4, new conflict C76); `memmap.region.psg_regs` stays `tier4-community` and its `impl` moves from `stub` to `real` with a new device kind (the memmap literal file row `R 01F0-01FF psg` and its write-view row change `impl` and `w`; the per-register masks live in `psg.regmap`, not in the uniform `rmask` of the device record, so the memsys formula needs a PSG-specific read); `memmap.device.stub` note drops PSG from its scope; `psg.part` cite gains the new source ids; `power_on_fill.psg` unchanged.
The tag rule "a field may not carry `tier5-emulator-hint`" is respected: no field is backed by an emulator alone; the emulator hint on `ticks_per_psg_clock` and the LFSR/period-zero leads live in `note`.

### 5.2 New source registry entries (`data/sources.json`, never the files themselves)

- `SRC-gi-ay8910-manual`: GI AY-3-8910/8912 Programmable Sound Generator Data Manual, third-party reprint copy, 55 pages, sha256 as above, rights `unknown`, `authority_tier` `tier2-first-party-adjacent` (an edited reproduction: the cover page adds notes and omits pages, by the project's own definition in `docs/evidence-tiers.md`), `local_path` under the gitignored `refs/`. It is the only copy of the printed equations and the worked examples; where the catalog (tier 1) prints the same text and figures, cite the catalog page.
- New locators for existing ids: `SRC-gi-1980-catalog` pdf 204-210 (printed 5-23..5-29), `SRC-intv-wiki` "PSG.oldid-14940", `SRC-community-emu-docs` `doc/programming/psg.txt` (record the commit).
- Cited pages: catalog pdf 207 (register tables, shape bits), pdf 208 (Fig. 3, 5, 9 shapes and DAC), pdf 205 (bus decode, pull-ups); reprint pdf pages 11, 16-19, 22-24, 26-27, 29.

### 5.3 Discrepancy entries (`data/discrepancies.json`, ids never reused; next free id at the time of writing is C69, and the orchestrator must allocate across the parallel discuss files)

- Update C11: evidence now records the 1980 catalog's 8900 system listing of the AY-3-8910 (printed 5-9) and the community "8910 with shuffled registers" statements; status stays `open`.
- Update C12: `partial`; sources now include the printed ranges 1..4095, 1..65535, 1..31 (period 0 undefined), the 16-state envelope with doubled extremes, and the Fig. 9 labels; default policy lists the three period-zero fields, the LFSR field, the DAC field and the envelope-select field.
- C69 (new) applicability of the AY-3-8910 datasheet to the 8914-class part: rule-by-rule table in `data/psg/rules.json`; resolver `measurement: D10` or `pending: an 8914 datasheet`.
- C70 (new) register permutation, R8 and R10 bit layouts: single lineage.
- C71 (new) noise LFSR polynomial (mask $12000 vs $14000), seed and output sense.
- C72 (new) envelope step rate reading (step = 16 EP clocks, the 256 divisor sentence) and restart-on-shape-write.
- C73 (new) port direction bit order (GI figure vs community doc).
- C74 (new) PSG decode aliasing and mirrors (A9, A8, DA7..DA4 chip select).
- C75 (new) R10 read-back (GI: readable; community: write-only).
- C76 (new) PSG chip clock = crystal / 2 (GI application circuit, community formula, emulator hint) and its phase.
Existing C60, C64, C65, C66, C67 are referenced, not reopened.

## 6. Rule-by-rule audit (the table that `data/psg/rules.json` formalises)

| rule | statement | basis | source | 8914 delta known? |
|---|---|---|---|---|
| R01 | 16 registers, read/write | 8910-primary-literal | catalog 5-24 | no |
| R02 | register permutation into $01F0..$01FF | 8914-community | wiki; community doc | yes, tier 4 |
| R03 | masks, upper byte reads 0 | 8914-community | wiki "Data/Off bits" | C60, C75 |
| R04 | no side effects on reads | default-unknown (inferred from "readable registers") | catalog 5-24 | no |
| R05 | tone f = clk/(16 TP), 50 percent duty | 8910-primary-derived | reprint p.16-17 | clock: C76 |
| R06 | tone range 1..4095 | 8910-primary-literal | reprint p.16 | |
| R07 | tone period 0 = 4096 | 8914-community | wiki | C12 |
| R08 | tone counter structure | default-unknown | none | C12 |
| R09 | noise rate clk/(16 NP) | 8910-primary-derived | reprint p.18 | |
| R10 | noise LFSR polynomial, seed, sense | default-unknown | conflicting tier 4/5 | C71 |
| R11 | envelope f_E = clk/(256 EP), step = 16 EP clocks | 8910-primary-derived | reprint p.22-23, Fig. 3, 9 | C72 |
| R12 | shape bits and 16-state patterns | 8910-primary-literal | reprint p.23-24, Fig. 3, 9 | |
| R13 | restart on shape write | 8914-community | wiki R10 page | |
| R14 | R7 polarity and bit order | 8910-primary-literal | reprint p.19 | |
| R15 | tone/noise AND combine, both-off constant | 8910-primary-derived + tier 4 | reprint p.19 note; wiki | |
| R16 | amplitude: 4-bit fixed | 8910-primary-literal | reprint p.20 | |
| R17 | 2-bit envelope select and shifts | 8914-community | wiki R11-R13; community doc | C12 |
| R18 | step scheduling and write ordering | architecture (delegated decision) | D2 | |
| R19 | DAC levels | 8910-primary-derived (9..15), inference (1..8) | reprint p.27 | C12 |
| R20 | I/O ports: pull-up idle, direction bits, output read | mixed | reprint p.26; wiki | C64, C65, C73 |
| R21 | reset state | 8910-primary-literal (registers) + default-unknown | catalog 5-24 | |

## 7. Per-rule test-vector plan (non-circular)

Rule of independence: an expected value is derived from (a) a printed equation evaluated in the test itself as integer arithmetic over master ticks, (b) a hand-transcribed figure or truth table written from the source into a literal file in the style of `data/memmap/*.literal.txt` (written from the source only, never generated from the profile or the engine, gate-checked for consistency), or (c) a structural property that a wrong parameter breaks (cycle length). The engine's own tables never produce expected values. Vector kinds mirror `data/cp1610/vectors`: `kind: source` (tier `source-expectation`, cite with page, derivation text) and `kind: policy` (a policy id from `data/expected/psg-policies.ids`, no tier, expected value stated as "what the profile says"). Claims on `kind: source` vectors that depend on `inference` or `tier4-community` fields are `provisional` by the weakest-field rule. Emulator names stay out of vector text (the existing lint).

| rule | vector idea | expected value derivation |
|---|---|---|
| R01-R04 | write $FFFF to each of 16 registers, read back; write per-bit patterns | from the literal register table (source file lists mask per register) |
| R05, R06 | channel A tone only, fixed level 15; P = 1, 2, 3, 255, 256, 4095; record output edges as master ticks | edge spacing is exactly 16 x P ticks (equation f_T = f_clock/(16 TP), duty 50 percent, crystal = 2 x chip clock); toggle count in a window is a closed-form integer; independent of the engine |
| R07 | P = 0 | policy vector: equals the profile's `psg.period_zero.tone` spacing; also run once with the field edited (unit-level engine API takes a params struct) to prove it is not hard-coded |
| R08 | period write mid-half-cycle | policy vector, `counter_model` named |
| R09 | noise period NP = 1, 2, 31; count LFSR shifts in a window | shift spacing 32 x NP ticks (equation f_N, step arithmetic) |
| R10 | cycle-length property: stepping the LFSR from the seed returns to the seed after exactly 131071 steps (mask $12000) and 114681 (mask $14000), plus first 40 output bits from a hand derivation | property is derived from polynomial theory, independent of the engine's constants; the first-40-bits literal is checked by an independent tiny script run once and the output committed (label `independent-implementation-comparison` only if a second author writes it, otherwise `source-expectation` is not claimed and the vector is `policy`) |
| R11 | EP = 1, 2, 3, 15625; shape 1000 (repeat decay); record level changes | step spacing 32 x EP ticks; the worked example (f_E = 0.5 Hz, 2 MHz, EP = 15625) converts to 15,625 x 16 x 16 chip clocks per 16-state cycle, equal to 4,000,000 clocks |
| R12 | all 16 shape values, first 36 steps each | hand-derived sequences from the text and Fig. 3/9, e.g. 1010: 15..0, 0..15, 15..0 with the doubled extremes; 1011: 15..0 then 15 held; 1101: 0..15 then 15 held; 1111: 0..15 then 0 held; 00xx: 15..0 then 0 held; 01xx: 0..15 then 0 held (derived above in this session with a throwaway script and checked by eye against the figure; the committed literal is hand-written) |
| R13 | write the shape twice with the same value mid-cycle | policy vector |
| R14, R15 | R7 all 8 noise and 8 tone truth-table rows; the 4 I/O direction rows | the printed truth tables, transcribed literally |
| R15 | both disabled gives constant 1; tone x noise AND | rule text; truth table |
| R16, R17 | fixed levels 0..15 select DAC index; ee 0..3 shift 0, 2, 1, 0 | R16 source; R17 policy-tier-4 |
| R19 | table vs printed labels 15..9; 0 | source vector for the 7 printed levels; policy vector for 1..8 (the halving rule) |
| R18 | the same CPU program (MVO writes to $01F0.. at known cycles) run in one call and in many slices | register-write log ticks equal start + offset x 4 computed from the program's cycle counts (the existing CP-1610 cycle tables), digests equal across slicing |
| R20 | port read in input mode with a given driven-low mask; direction flip | policy vectors, wired to the INP vectors |
| R21 | reset then read all 16 | all zero reads; samples after reset are silent (level index 0 everywhere) |

Named-fault controls (each must make its vector fail, per the existing control framework): F1 treat GI octal numbers as hex (R10 lands at $01FA); F2 make the PSG step 16 chip clocks (frequency half of the equation; this is exactly the current glossary mistake, so it is a real regression guard); F3 envelope step 256 x EP clocks; F4 drop "reset to initial before holding" (1011 holds 0); F5 invert enable polarity; F6 swap LFSR mask for the other candidate while the profile says the default; F7 DAC lookup off by one index; F8 stamp writes at the end of the unit; F9 R10 reads back its stored value under the default; F10 input-mode port read returns the latch.

CI cost [REC]: all PSG vectors are integer, deterministic, and run at module level in milliseconds; one new gate step (`psg-rules`: rules.json schema, rule-to-field-to-vector coverage, literal files consistency) joins the existing structural-check family. No new CI job, no new dependency; follow `AUTONOMY.md` section 4 (measure before touching CI).

## 8. Lens notes

- Emulator and audio architect: the step quantum of 16 master ticks makes the PSG entirely integer, so the catch-up scheduler needs no fractional state. 3,733.5 steps per frame is a feature: tests prove time is a tick count, not a frame count. Anti-patterns avoided: floating-point mixing in the core, hidden resampling, a 32-step envelope borrowed from the YM2149, treating the envelope as "clock/256 per step", sampling at the host rate, per-frame audio buffers. Lesson from other cores: sample-exactness depends on the counter structure and LFSR seed, which is why both are visible profile data and why the raw path is level indices.
- Hardware/EE: the chip clock of the 2609 PSG is inferred (crystal/2); the printed 1.0-2.0 MHz range admits 1.79 MHz and the GI application note uses exactly that. The DAC is a 4-bit log DAC, three channels summed in an analog node of unknown loading, so the weighting is a model; only level indices are claimed. The upper data byte is not driven by the PSG; its value is a bus property, not a PSG property.
- QA: the gate checks rule coverage, not just vectors. Vectors that rest on tier 4 are labelled policy, not source. The period-zero, LFSR and DAC fields have a "parametric proof" vector that edits the params struct and observes the effect, which defeats a hard-coded constant that happens to equal the profile default.
- Security and rights: datasheet PDFs stay in the gitignored cache; only locators, hashes and short derivations are committed; the register table is a factual mapping, re-expressed as data; the LFSR is a generic Galois shift written from scratch; no table or comment from any emulator is used. The reprint copy's host is a public file CDN with no stated licence, so its registry entry says `rights: unknown` and nothing is redistributed. No path or personal data in committed text.
- DevOps and CI: one structural gate step, no matrix growth, no cache; the cost estimate is seconds. The plan should measure the step before declaring it cheap (per `.planning/CI-EFFICIENCY.md`).
- API and integrator: expose `ticks_per_psg_step` and the log and sample record layouts in the public header; keep the DAC weighting as a separate pure function so integrators with their own mixer can ignore it. The register-write log is the natural interface for chip-tune players and for LibFuturo-side adapters (a later contract, D13).
- Product: what users hear is determined by pitch and envelope timing, which are datasheet-equation rules; the unknowns (noise timbre phase, period-0 edge cases, DAC curve) are second-order. The strongest end-to-end evidence needs the owner's EXEC (D11), which is a hard stop; the substitute is an original diagnostic that writes the register map and checks read-back, which only proves self-consistency.
- DX: one `psg-rules` inventory, one literal file per figure, octal-and-hex columns in the audit table, and engine rule names (`PSG-R12`) used in function comments so a reader jumps from code to datasheet page.

## 9. Adversarial pass against the recommendation

1. "The whole register map could be wrong." It rests on one lineage. Mitigation: it is a data table; a wrong table changes the profile and the literal file, not the engine. A guest that works with the real EXEC (D11) or a hardware read-back (D10) would settle it. Risk accepted and called out as the top unknown.
2. "Retagging `ticks_per_psg_clock` to `inference` is overclaiming." The facts are two tier 2/4 documents plus the manual's range; the tag is `inference`, not tier 1, and the weakest-field rule keeps dependent claims provisional.
3. "The 8-chip-clock step contradicts an already-merged document." True; section 2.3 says so. The glossary text was explicitly marked "to be checked in Phase 5". The correction is cheap now (no code uses it) and expensive after goldens exist, so it is a hard sequencing requirement for the first plan.
4. "Envelope step 16 EP clocks could still be wrong." The primary figure captions and the worked example agree with it and so does the tier 4 "multiplied by two" statement. The residual ambiguity is only the sentence order in the text. Named fault F3 keeps the other reading testable. Confidence: high for the 8910.
5. "Seven DAC levels from a figure whose labels are not a clean ladder." True, and the figure may be drawn schematically. That is why the table is `inference`, is outside the raw evidence path, and can be replaced by a measured table with no engine change.
6. "Two LFSR candidates, only one engine path." Correct by design; but the Galois-versus-Fibonacci equivalence is a computed fact (checked in this session), so the claim that the default is the common form is an inference about other documents' intent, not about hardware.
7. "Profile bloat: 19 new fields." They are the minimal grouping at which a field has a single tag; merging fields would hide provenance. The gate cost is an inventory file.
8. "Raw samples as level triples are not 'audio'." They are integer chip-rate samples (AUD-02) and exactly what the DAC would consume; the weighted sum is one pure function away. If the plan prefers a weighted sample as the primary record, it can store both in one 32-bit word (12 bits of indices plus nothing else is enough; the weighted sum needs 29 bits), at the cost of binding evidence to the unverified table. Not recommended.
9. "Modelling no address latch is wrong for the ECS." It would be wrong for an ECS PSG scanned with non-addressing cycles; ECS is out of the first profile (PER-01). The limit is stated in P-3.
10. "R10 reads 0 follows a source that contradicts the datasheet." The part-specific source wins by policy, the contradiction is a numbered discrepancy (C75), and the reversal is one mask.

## 10. Reversal triggers

- An AY-3-8914 (or 8916/8917) datasheet, die analysis or hardware read-back that changes the register permutation, bit layouts, period-zero behaviour, envelope-select map or noise polynomial: edit profile and literal files, retag, re-run vectors, no engine change expected.
- A board schematic or a measured tone capture (D10) that shows a PSG clock other than crystal / 2: change `ticks_per_psg_clock`; if the new clock is not a multiple of 8 chip clocks per toggle granule, `psg.clocks_per_step` and D2's "integer divisor of the crystal" reversal trigger apply.
- A measured DAC curve: replace `psg.dac.levels`; raw audio goldens do not change.
- Evidence that the tone counter is up-compare (period 0 acts as 1, immediate period effect): change `counter_model` to a second engine branch; the steady-state vectors stay valid.
- An owner decision to support the 5-bit variants (8916/8917) or the ECS PSG: a new profile id; `psg.amp.env_select` and the decode become profile-specific.
- A consumer that needs host-rate PCM inside the core: that is D7, not this decision.

## 11. Open items and hand-offs

1. Correct the glossary "PSG step" entry in `docs/time-base.md` (8 chip clocks, 16 master ticks, 4 CPU cycles on NTSC) before any PSG code lands; check `data/expected/glossary-terms.ids`.
2. Register the new source and locators (section 5.2); update C11 and C12 and add C69..C76 (ids to be allocated by the orchestrator across the Phase 5 discuss files); add the profile fields and regenerate the profile header.
3. Memmap: the stub becomes a PSG device with per-register masks; the memmap literal rows and the gate's row grammar (`w=` is uniform per row) need a `psg` exception or a per-register note; decide in the plan whether `w=8` plus the register table is acceptable for the consistency check.
4. Make the bus path tick-aware (the access record carries only a CPU-cycle offset; the machine computes ticks) and carry the PSG state in digest and snapshot.
5. Interface with the controller decision: the PSG consumes one "driven low" byte per port at unit boundaries; the control-to-byte function and C63..C66 stay there.
6. D7 (reference PCM location) is unaffected: raw evidence is level indices and the write log.
7. [UNK] Which R7/R8 bit order the 8914 uses for the port direction bits, whether the PSG aliases at other addresses, and whether R10 is readable: all three are cheap to settle with an original diagnostic on owner hardware (D10), which is a hard stop here.
8. [UNK] Whether the sdiy.info "A9 expected input may be different" remark means the 8914 selects at a different A9/A8 code; irrelevant to the CPU-visible $01F0 mapping but relevant to mirrors (C74).

## 12. The three riskiest unknowns

1. The AY-3-8914 register map, bit layouts and part-specific behaviours (R8 layout, R10 write-only, 6-bit amplitude with the two-bit select) rest on one community lineage with no datasheet; a mistake affects every PSG access and the controller ports at $01FE/$01FF.
2. The sub-step behaviour that sample-exact goldens depend on: tone counter structure and period 0, noise polynomial and seed, envelope restart rules and the PSG clock phase relative to reset; frequencies are datasheet-backed, phases are not.
3. Bus-visible oddities a probing guest can see: upper-byte and unused-bit reads, R10 read-back, output-mode port reads, and chip-select aliasing of the 16-word block, all currently `unknown-default-chosen` with no tier 1 to 3 source.

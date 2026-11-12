# Phase 5 discuss: Decision point C, controller read path and electrical model (INP-01..03, API-03)

Status: recommendation (fan-out protocol, `.planning/AUTONOMY.md` section 2). Labels: [OBS] read or run in this session, [INF] inference, [REC] recommendation, [UNK] unknown, [BG] background not re-fetched (never used as evidence).
Inputs read [OBS]: AGENTS.md, AUTONOMY.md s1-3, PROJECT.md Decision Register, REQUIREMENTS (INP-01..03, API-03, AUD-01/02), ROADMAP Phase 5, 04-CONTEXT.md (D-12..D-16, A-04, A-05), 04-DISCUSS-ctrl.md, `core/src/machine/machine_input.c`, `machine_run.c`, `machine_bound.c`, `machine_digest.c`, `core/src/memmap/memsys.{h,c}`, `core/include/ivs/ivs_run.h`, the profile JSON, `data/discrepancies.json` (C11, C12, C14-C16, C63-C67), `docs/evidence-tiers.md`, `docs/runner.md`, `cmake/IvsMemmapTable.cmake`, `data/memmap/*.literal.txt`, `tests/run/run_fuzz.c`, `data/scenarios/diag-input-pass.ivss`.
No emulator source was opened. Other emulators are used at most as a "find discrepancies" hint, and none was needed.
Primary and community sources were fetched fresh in this session into a scratch directory (never committed; hashes in section 9).

## 1. The decision in one paragraph

[REC] Keep D-12..D-15 exactly (logical switch words in, timestamped level samples, unit-start application). Phase 5 adds the part D-15 deferred: a profile-data table that maps every control bit to the set of PSG port lines it pulls to ground, the rule `pulldown(word) = OR of line_mask[bit] over the set bits` (ideal ground switches on lines that have on-chip pull-ups), and a read path where the PSG ports R14 ($01FE, port 0) and R15 ($01FF, port 1) return `~pulldown` in the low 8 bits while the port direction bit in R7 says input, the stored latch value while it says output, and `$00` in the upper byte (a labelled default). The derived pull-down masks live in the memory system next to the STIC/GROM gate flags, are written only through an internal setter called when an input event is applied (the same seam pattern as `ivs_memsys_set_gate_internal`), and are part of the MEM digest. Every one of the 31 table entries is its own profile field with a source-authority tag (`tier4-community` for all 31, with the cross-source agreement recorded), the combination rule is tagged `inference`, and every combination class that no source covers is a named policy row tagged `unknown-default-chosen`. API-03 is already satisfied by the Phase 4 mechanism; Phase 5 must keep its proof honest now that input becomes guest-visible (the fuzz boundary oracle can no longer assume the schedule does not change instruction timing).

## 2. What the evidence supports and does not (section 5 of the request)

### 2.1 New finding: the C16 "one LOW-confidence source" picture is stronger than recorded [OBS]

I fetched three controller sources and checked them against each other by script (throwaway, scratch only):

1. A pin-level table (Jay Tilton, "Intellivision Controller Pinouts", dated 06/28/98, captured in a 2006 web archive of Deathskull Labs): for each of the 16 disc directions D1..D16, 12 keys and 3 side buttons, which of connector pins 1,2,3,4,6,7,8,9 are pulled low; pin 5 is ground; "all other pins are normally high unless a controller input pulls them low". The same page says the top two side buttons are functionally identical. [OBS, tier 4]
2. The ConsoleMods wiki page "Intellivision:Connector Pinouts" (oldid 23186, last edited 26 Feb 2023, read through a web-archive capture because the live host answers 403 to curl): the identical matrix row for row, plus the pin-to-PSG-bit map `pin 4=B0, 3=B1, 2=B2, 1=B3, 9=B4, 8=B5, 7=B6, 6=B7` (B = port B, "1P"; A = port A, "2P"), and "all controller pins except GND are connected to the data input pins on the AY-3-8914 ... each direction and button shorts a different combination of these pins to GND". That page says the controller port connector exists only on the Intellivision II and Sears Super Video Arcade (removable controllers). [OBS, tier 4; same matrix as source 1, so treat both as ONE lineage]
3. The community wiki Hand Controller page, revision 14808, which the project already cites as SRC-intv-wiki, but whose key-table IMAGE (`Hand_controller.png`) had never been read. I read it. It lists, for the left controller drawing, the value whose set bits are the bits that get CLEARED on a read: keys 1..9 = 81 41 21 / 82 42 22 / 84 44 24, Clear 88, 0 48, Enter 28; side buttons top 0xA0 (both the left and right top button show 0xA0), lower left 0x60, lower right 0xC0; disc from north clockwise 04 14 16 06 02 12 13 03 01 11 19 09 08 18 1C 0C. [OBS, tier 4]

Script check [OBS, executed]: converting source 1 pins to bits with source 2's map reproduces ALL 31 values of source 3, with disc numbering D1 = north increasing clockwise (so the "orientation unknown" gap I expected is closed at tier 4), and reproduces the two numeric witnesses in the wiki prose: key 1 alone reads `$7E`, keys 1 and 9 together read `$5A`. The wiki's four decodability statements (a single keypad press is decodable; a side button combined with a disc input is decodable; a keypad press combined with a side button or a disc input is not uniquely decodable; two side buttons are not uniquely decodable) are all reproduced by the union rule: within the set {idle, one key, one disc, one side, disc + side} no two states give the same mask (0 collisions, 12 + 48 states checked); every one of the 36 key + side states, 180 of the 192 key + disc states and all 3 side + side states collide with another state. None of the 31 single-control masks is a duplicate, so the "apparent duplicate pin" in C16 is a transcription artefact of the earlier extraction, not a property of the table (I cannot exclude that the earlier source had a real typo).

Honest limits [INF]: sources 1 and 2 are one lineage; source 3 is a software-level table whose editor handle is "Mr z", and it plausibly shares lineage with emulator documentation, so I do not count its agreement as independent hardware evidence. What the agreement does give is a transcription cross-check (31 values, 8 pin-to-bit entries, 2 numeric witnesses, 4 prose statements, all consistent) and an orientation answer. Tier stays 4, standing stays `provisional`, no `measured-hardware` claim (D10).

### 2.2 Tier 1: what the AY-3-8910 datasheet says about the ports [OBS]

From the GI 1980 catalog (SRC-gi-1980-catalog, sha256 recorded in sources.json, re-fetched and re-hashed: identical), AY-3-8910/8912 section:
- pdf 205 (printed 5-24): "Each pin is provided with an on-chip pull-up resistor, so that when in the 'input' mode, all pins will read normally high. Therefore, the recommended method for scanning external switches, for example, would be to ground the input bit." Also "the I/O ports are TTL-compatible and are provided with internal pull-ups on each pin".
- pdf 205 (5-24): "applying a logic 0 (ground) to the Reset pin will reset all registers to 0".
- pdf 205 (5-24) and pdf 207 (5-26): registers are numbered in OCTAL (R0 to R17); "16 registers"; the I/O data registers are R16 and R17 (octal), that is decimal 14 and 15, which is why the Intellivision addresses are $01FE and $01FF. The envelope registers "R13 to R15" are decimal 11 to 13. [Footgun for the PSG decision: every register number in the datasheet is octal. Decision text must say "R14/R15 decimal" or "R16/R17 octal" and never mix them.]
- pdf 207 (5-26): R7 bits B7 and B6 carry the label "Input Enable" with an OVERBAR (verified on a 300 dpi render), next to "I/O Port: B A". Overbar means active low, so bit = 0 enables input and bit = 1 selects output; B7 is port B, B6 is port A. Port A is IOA = R16 octal = decimal 14 = $01FE; port B is $01FF. Text extraction loses the overbar, so a reader of the text layer would get the sense wrong.
- pdf 208 (5-27): R16 and R17 are "intermediate data storage registers between the PSG/CPU data bus and the two I/O ports"; nothing is said about what a read returns in output mode, or about a port driven as output while a switch grounds the pin. [UNK, C65]
- pdf 205 (5-24): "each of the 16 registers is also readable so that the microprocessor can determine present states or stored data values". This is the only tier-1 sentence supporting "read of a stored register returns the stored value", used for the output-mode default.
- No "8914" appears in the catalog text (C11 stands). All of the above is the 8910 and is applied to the 8914-class part by inference (tag `inference`, cite C11).

### 2.3 Other sources and what they add [OBS]

- Blue Sky Rangers 2609 overview (SRC-bsr-2609, re-fetched, sha256 identical to the recorded value): each controller is "hooked to an 8-bit input port (physically the input ports of the sound chip)"; the disc outputs a 5-bit code, the action buttons a 3-bit code, the keypad an 8-bit code; the top action keys are wired together (three separate action buttons); "the keypad duplicates the disc controller and the action keys, so you can use the disc and buttons OR the keypad, but not both at the same time". These are programmer-level statements and agree with the table: disc uses bits 0-4, side buttons are two-of-three codes on bits 5-7, a key is one of bits 0-3 plus one of bits 5-7.
- Wiki code snippet: `MVI $01FE,R0 ; right controller`, `AND $01FF,R0 ; left controller`, `XORI #$FF,R0`, `BEQ no_input`. [INF] This snippet only works if the UPPER byte of both reads is zero (XORI #$FF flips only the low byte, so an upper byte of $FF would leave R0 nonzero). It is weak (illustrative code, tier 4), but it is the only evidence about the upper byte and it points at `$00`.
- Both wiki and ConsoleMods agree that pin level semantics are active-low with idle all ones; ConsoleMods says port A (R14, $01FE) is the second controller ("2P") and port B ($01FF) the first ("1P"), the wiki says $01FE is "right" and $01FF is "left". Nothing here fixes which physical connector is which; C66 stays open and the core keeps indexing by port.
- Not obtained: the iFixit and ConsoleMods Sylvania service manuals (HTML viewer / HTTP 403), the AtariAge controller-matrix threads (HTTP 403), the Intellivision II service manual (no response). No schematic of the 2609 controller was found. [UNK] Recorded as missing evidence, not as absence.

### 2.4 Unknown and not obtainable without D10

(a) upper byte of the 16-bit read; (b) read of a port in output mode, and contention when a switch grounds a driven pin; (c) whether the AY-3-8914 matches the 8910 on port behaviour at all; (d) whether any disc position pair can close at once, and what lies between positions; (e) which physical connector feeds R14; (f) a schematic-level confirmation that each control is "grounds a pin set" rather than "bridges pins to a common node"; (g) reset state of the PSG on the 2609 (is the RESET pin tied to system reset). All carry D10 resolvers. No `measured` claim is made anywhere in this recommendation.

## 3. Options

### 3.1 Combination rule (point 2)

| | R1. union of grounded lines (chosen) | R2. priority resolution in core (keypad over disc, first-wins, etc.) | R3. analog/resistive contact model | R4. bridge model (a closed switch connects its pins to each other, wired-AND on merged nets) |
|---|---|---|---|---|
| Rule | `byte = NOT(OR of line masks of closed switches)` | one source wins | contact resistance divider | merged net level = AND of driven members |
| Evidence fit | all 31 values, 2 numeric witnesses, 4 prose statements reproduce (section 2.1); matches the datasheet's "ground the input bit" scan method | contradicts the wiki ("all corresponding bits clear") | none, no data | needs a driven line to differ from the pull-up level; in input mode nothing drives a line low except ground, so a bridge cannot produce the observed lows; only matters in output mode |
| Where host policy lives | host (SOCD, keypad exclusivity) | core, invisible | core | core |
| Cost | one OR loop over 31 bits | table of priorities, unsupported by any source | unbounded | needs a net model |
| Reversal evidence | schematic or D10 shows a non-union result | n/a | n/a | D10 output-mode measurement (C65) |
Verdict [REC]: R1. R4 is not rejected as false, it is deferred: it differs from R1 only when a port is driven as output (C65), and that case is already a labelled policy row.

### 3.2 Where the derived byte lives and how it reaches the guest (point 3)

| | P1. pins cached in the memory system, set by an internal setter, digested in MEM (chosen) | P2. overlay at machine level: a machine bus wrapper reads `obs.input.words` directly | P3. replace the memory-system read for PSG ports by a callback |
|---|---|---|---|
| Single source of truth | words in force (INPUT row) plus a derived cache | words only | words only |
| Peek and CPU read agree | by construction (both read memsys) | needs every read path (bus port, peek, any future debugger) routed through the wrapper; one missed path is a bug | callbacks are forbidden by the core's rules |
| Digest | pins are in the MEM domain, so the composite (domains 1-4) sees them; the INPUT row stays in domain 18 | the composite does not cover domain 18, so guest-visible state would be outside it: needs a composite version bump and a Phase 3 literal change | n/a |
| Take-back on a refused step (IN-01 restore in `machine_bound.c`) | must re-derive pins from the restored words (one call, testable) | free | n/a |
| Pattern fit | same as `ivs_memsys_set_gate_internal` (machine writes device-input state) | new pattern | breaks no-callback rule |
| Failure mode | cache desync: guarded by an invariant test `pins == f(words)` after every unit in the fuzz | forgotten routing | n/a |
Verdict [REC]: P1. P2 is a legitimate alternative if the owner prefers no duplicated state; its price is a composite bump and a funnel that peek and the CPU port must both use.

### 3.3 Table storage (point 1)

| | T1. one profile field per control, generated const table (chosen) | T2. three table-valued fields (keypad, disc, side) | T3. hard-coded C table with a literal-file test |
|---|---|---|---|
| Per-value tier tag (ROADMAP SC1 requires "each value carries a tier tag") | yes, tag/cite/conflict/resolving_evidence/revisit_trigger per field, like `memmap.region.*` | one tag per table, so a corrected single row cannot be tagged apart | tags live in comments |
| Violates "tier-tagged PROFILE DATA, not hard-coded" | no | no | yes |
| Cost | +31 ids in `data/expected/profile-fields.ids`, one generator of about 150 lines copying the helpers of `IvsMemmapTable.cmake` | smaller | smallest |
Verdict [REC]: T1. A separate generator `cmake/IvsCtrlTable.cmake` (copy of the `mm_type`/`mm_keys`/`mm_fail` helpers, about 60 lines copied, no shared module) rather than growing the 580-line memmap generator; its `.inc` is included only by the machine-side input module, keeping the memmap `.inc` single-includer rule intact.

## 4. Recommendation (numbered decisions)

### P5-C1. The control-to-byte table (point 1)

Core bit names are the existing `IVS_CTRL_*` (pinned by `data/expected/input-bits.ids`). The line mask is the set of port bits pulled LOW; the port byte in input mode is `0xFF XOR mask`. Pins are the D-sub reference numbering of the source; bits come from the pin-to-bit map (pin 4=b0, 3=b1, 2=b2, 1=b3, 9=b4, 8=b5, 7=b6, 6=b7).

| Control (core bit) | Source label | Pins low | Line mask | Port byte | Wiki image value |
|---|---|---|---|---|---|
| KEY_1 | K1 | 4,6 | 81 | 7E | 81 |
| KEY_2 | K2 | 4,7 | 41 | BE | 41 |
| KEY_3 | K3 | 4,8 | 21 | DE | 21 |
| KEY_4 | K4 | 3,6 | 82 | 7D | 82 |
| KEY_5 | K5 | 3,7 | 42 | BD | 42 |
| KEY_6 | K6 | 3,8 | 22 | DD | 22 |
| KEY_7 | K7 | 2,6 | 84 | 7B | 84 |
| KEY_8 | K8 | 2,7 | 44 | BB | 44 |
| KEY_9 | K9 | 2,8 | 24 | DB | 24 |
| KEY_CLEAR | Clear | 1,6 | 88 | 77 | 88 |
| KEY_0 | K0 | 1,7 | 48 | B7 | 48 |
| KEY_ENTER | Enter | 1,8 | 28 | D7 | 28 |
| DISC_N | D1 | 2 | 04 | FB | 04 |
| DISC_NNE | D2 | 2,9 | 14 | EB | 14 |
| DISC_NE | D3 | 2,3,9 | 16 | E9 | 16 |
| DISC_ENE | D4 | 2,3 | 06 | F9 | 06 |
| DISC_E | D5 | 3 | 02 | FD | 02 |
| DISC_ESE | D6 | 3,9 | 12 | ED | 12 |
| DISC_SE | D7 | 3,4,9 | 13 | EC | 13 |
| DISC_SSE | D8 | 3,4 | 03 | FC | 03 |
| DISC_S | D9 | 4 | 01 | FE | 01 |
| DISC_SSW | D10 | 4,9 | 11 | EE | 11 |
| DISC_SW | D11 | 1,4,9 | 19 | E6 | 19 |
| DISC_WSW | D12 | 1,4 | 09 | F6 | 09 |
| DISC_W | D13 | 1 | 08 | F7 | 08 |
| DISC_WNW | D14 | 1,9 | 18 | E7 | 18 |
| DISC_NW | D15 | 1,2,9 | 1C | E3 | 1C |
| DISC_NNW | D16 | 1,2 | 0C | F3 | 0C |
| SIDE_TOP | S1 (both top buttons) | 6,8 | A0 | 5F | A0 |
| SIDE_LOWER_LEFT | S2 | 7,8 | 60 | 9F | 60 |
| SIDE_LOWER_RIGHT | S3 | 6,7 | C0 | 3F | C0 |

The table is identical for port 0 and port 1 (both controllers are the same device; wiki and pin table agree). The wiki-image column is the independent-lineage cross-check; the generator and the literal file store the line mask, the literal file additionally stores pins and the wiki value so the three-way consistency is checkable.

Profile data [REC]:
- 31 fields `input.control.key_1` ... `input.control.side_lower_right`, value `{"lines": "$81"}` (object, so a later `bridge` extension does not change the schema), tag `tier4-community`, cite `SRC-tilton-intv-pinouts` + `SRC-consolemods-intv-pinouts` + `SRC-intv-wiki` (image `Hand_controller.png`), conflict `C16`, resolving_evidence "schematic, service manual, or D10 controller reader diagnostic", revisit_trigger "any of those, or a value disagreeing with the guest-visible EXEC decode when owner EXEC runs are available (D11)".
- `input.line_model` = `"ground-union"`, tag `inference`, cite the datasheet pdf 205 (5-24) sentence plus the wiki statements, conflict `C16`, `C63`.
- `input.line_idle` = `"$FF"` (the byte read with no switch closed, which is also what an unconnected port reads under the pull-up model), tag `tier4-community` (wiki: "returns all 1's in the lower 8 bits when no key is pressed") with the datasheet as corroboration for the sibling part.
- `input.port_output_read` = `"latch"` (read of a port whose R7 direction bit selects output returns the stored R14/R15 value; switches are ignored), tag `unknown-default-chosen`, conflict `C65`.
- Direction sense lives with the PSG fields (decision point for AUD): `psg.io_direction` = {R7 bit 6 port A (= R14, port 0), bit 7 port B (= R15, port 1), 1 = output}, tag `inference` (8910 datasheet pdf 207, 5-26, overbar "Input Enable"; the part is C11).
- Upper byte: no new field. Add the device kind `memmap.device.psg8` = `{"bits": 8, "rmask": "$00FF", "fill": "$0000", "store": "keep"}` tagged `unknown-default-chosen` (C64), and re-point the region `psg_regs` to it (`impl: real`). The existing width formula `read = (stored & rmask) | (fill & ~rmask)` then yields the upper byte from profile data. Flipping the default is a one-field change. Coordinate with the AUD decision: this edit belongs to whoever makes the PSG region real.
- Generator `cmake/IvsCtrlTable.cmake` fails closed: the set of `input.control.*` names must equal the `IVS_CTRL_*` names in `input-bits.ids` (missing and unknown both fail), each `lines` is `$XX` with 01..FF, `input.line_model` must be in the generator's known list, a profile with `input.controllers` > 0 must carry the table. Output `ivs_ctrl_lines[profile][31]` (u8, index = bit number) in a build-tree `.inc` included only by the machine-side input module. `-DMUTATE=<control>` flips one mask bit for the named-fault control `GEN_CTRL_MUTATION`.
- Independence literal `data/input/intv-2609-ntsc-family-p1.literal.txt`, same rule as `data/memmap/*.literal.txt`: written from sources only, never derived from the profile or generator. Row grammar `<NAME> pins=<n,n> cleared=<HH> wiki=<HH> byte=<HH> src=<SRC-id>@<locator> tag=<tag> conf=C16`, plus a header holding the pin-to-bit map and the wiki prose statements. A gate probe checks: pins mapped through the header equal `cleared` equal `wiki` (source cross-check), and the generated profile table equals `cleared` (profile vs source).

Rights [INF, not legal advice]: the literal file records 31 hexadecimal facts with citations; it copies no page layout, no image and no overlay. The fetched pages and the wiki image stay in the gitignored cache or scratch (AUTONOMY section 1, third-party material); `data/sources.json` gets entries with `rights: unknown`, `capture: hashed` and a local path under `refs/`.

### P5-C2. The combination rule and its tier tags (point 2)

Rule [REC]: for a controller word `w` on port `p`, `pulldown = OR over set bits b of ivs_ctrl_lines[profile][b]`; `pins = 0xFF XOR pulldown` (low 8 bits). The core never resolves a combination: any subset of the 31 bits is accepted, in any combination. SOCD and keypad exclusivity are host policy.

Result classes and the tag each carries in `data/expected/input-policies.ids` (precedent: `cp1610-policies.ids`) and in the claim ledger:

| Class | Example | Result | Tag of the result |
|---|---|---|---|
| Single control | KEY_1 -> $7E | table value | `tier4-community` (provisional) |
| Key + key | KEY_1 + KEY_9 -> $5A | union | `tier4-community`: the wiki gives the number and the rule |
| Disc + side | any of 48 | union, injective in the intended space (checked) | `tier4-community` (wiki decodability statement), the union itself `inference` |
| Key + disc, key + side, side + side | KEY_5 + SIDE_TOP | union, guest-ambiguous (180/192, 36/36, 3/3) | `inference` (wiki says "not uniquely decodable"; the core adds nothing) |
| Disc + disc, adjacent | N + NNE | union (here = a valid code, NNE) | `unknown-default-chosen` (C63) |
| Disc + disc, other (56 pair unions equal a valid position, others are not any position) | N + E -> ENE code, E + W | union | `unknown-default-chosen` (C63) |
| Opposite directions | N + S -> $FA, E + W -> $F5 | union, a code no valid position produces | `unknown-default-chosen` (C63); hosts resolve SOCD before the core |
| Three or more | any | union | `unknown-default-chosen` |
| Bit 31, words 2-3, flags | | UNSUPPORTED refusal (unchanged D-14) | n/a |

The wiki prose statements are the independent expectation for the class rows (section 6 INP-02). A result that is only the model's definition (opposite directions) is listed as a policy row and is never reported as evidence.

### P5-C3. How the byte reaches the guest (point 3)

1. Addresses: `$01FE` = PSG R14 (decimal; R16 octal; IOA) = port index 0; `$01FF` = R15 (IOB) = port index 1. Index is by port, never player; C66 remains.
2. Memory-system state: add to `ivs_memsys` a PSG block owned by the AUD point (register file) plus `uint8_t port_pulldown[2]`, written ONLY by `ivs_memsys_set_port_pulldown_internal(m, port, mask)`. Digest rows `mem.psg_port_pulldown` in the MEM domain (the composite, domains 1-4, therefore covers guest-visible controller state; no composite version bump). The MEM digest changes once in Phase 5 anyway because the PSG register file joins it; pin that literal change deliberately in the same plan.
3. Machine: `machine_input.c::ivs_input_apply` (the only place words change) calls a new internal `ivs_input_derive_pulldown(profile, word)` and the memsys setter for each of the 2 connected ports, after updating `words[]` and the chain. `ivs_run_power_on` zeroes `port_pulldown` and the PSG file (R7 = 0, so both ports are input at reset, consistent with the datasheet's "reset sets all registers to 0", whose tie to the 2609 reset line is [UNK] and is covered by the existing `power_on_fill.psg = zero` unknown-default). The IN-01 take-back in `machine_bound.c` restores `obs.input` from `input_before`; it must also re-derive `port_pulldown` from the restored words (extend `run.stop.internal-input-restored`).
4. Read: `ivs_mm_region_read` for phys PSG at word offset 14 or 15: if R7 bit (6 for port 0, 7 for port 1) is 0 (input), low byte = `0xFF XOR port_pulldown[p]`, upper bits from the device record fill; if 1 (output), low byte = stored register value (R14/R15 latch). All other PSG offsets are the AUD point's. Reads are pure (a `const` read of state), so peek, the CPU read and the ADAR strobe stay side-effect-free, which keeps the Phase 4 observation contract (`ivs_machine_peek` returns "what a CPU read returns now"). Consequence: a host and the runner can observe the derived byte with `ivs_machine_peek(0x01FE)`/`assert mem 01FE == 00FB` and no new API or report key is needed.
5. Writes to R14/R15 store the latch whatever the direction (datasheet: data store registers); in input mode they do not change the read value.
6. Unplugged and other port: the 2609-family p1 profile has two hard-wired controllers (`input.controllers = 2`), and the matrix source states removable controllers exist only on the later models, so "unplugged" is outside p1's hardware. Under the pull-up model an unplugged port and an idle controller both read $FF in the low byte [INF]; a profile with `input.controllers < 2` (future) leaves the missing port at pulldown 0. This halves C64: the unplugged case reduces to the idle case by inference, and only the upper byte stays unknown. Words at port index >= `input.controllers` are already refused (UNSUPPORTED) if nonzero.
7. Upper byte: `$00` by the `memmap.device.psg8` fill, tagged `unknown-default-chosen` (C64), with the wiki snippet as weak support (section 2.3). The AY datasheet's DA7..DA0 is an 8-bit bus; what the 16-bit CPU bus shows for the other 8 bits is the open question.
8. Granularity: C67 unchanged: an event applies at the first CPU-unit boundary with start >= at_tick; every guest read in a unit sees the value in force at that unit's start. The Phase 5 PSG read test is the C67 resolver at instruction granularity (section 6).

### P5-C4. API-03 (point 4)

Confirmed [OBS from code and header, plus INF]: the mechanism of D-13/D-14/A-04 meets "applied at an explicit machine-time boundary declared by the API; same script, same results regardless of host timing":
- the boundary is declared in `ivs_run.h` (first unit start >= at_tick) and tested first in a run (exit test, then apply only when a unit will begin), so where the host slices cannot decide where an event lands;
- the core retains nothing between calls (events come in the request; `input_consumed` is the cursor), the floor refuses late events, and the log chain hashes the SCRIPT tick, so the INPUT digest equals the accepted script for any slicing;
- the runner already slices (`--slice-ticks`, resubmits the unconsumed suffix) and documents that every key but `host.calls` is slicing independent.

What must change in Phase 5 [REC]: the existing proof is weaker than it looks because Phase 4's fuzz twin derives the unit boundaries `b[]` by stepping WITHOUT the schedule ("input is guest-invisible in Phase 4, so the boundaries do not depend on the schedule", `run_fuzz.c`). In Phase 5 a guest that reads a port and branches changes instruction timing, so the boundary list depends on the schedule. Required:
- Fuzz content B: an original stand-in (project-authored decle literals, no rights-bearing bytes) that polls `$01FE` and `$01FF` in a loop of CONSTANT cycle count per iteration, storing each pair into a RAM log. Timing is schedule independent, so the independent oracle survives: expected log entry j = `0xFF XOR OR(table over the words in force at the start tick of the unit that performs the read)`, computed in the test from the schedule, the recorded unit starts and the literal table, never from the core's own port code.
- Fuzz content C: a data-dependent loop (branch on the byte, different cycle counts per path). Boundaries now depend on the schedule, so the oracle is relational: the 1-unit-per-call reference (`run(now + 1)` with the suffix resubmitted, the definition of the semantics) versus random slicings, comparing at every reference boundary: consumed count, `controller_now`, floor, CPU, MEM and INPUT digests, then final storage byte for byte. Equality with the reference is the property; the oracle for the reference itself is content B.
- Both contents run the same seeds as `run.fuzz.seed-01..08` (extend the generator; keep the xorshift seed list so no wall clock or host randomness is involved); events at ticks on a boundary, strictly inside a unit, and past the horizon, with equal ticks, as in Phase 4; plus the invariant `port_pulldown == derive(words)` after every call.
- Runner: no IVSS format change is needed (the `input` directive already carries port words, `assert mem 01FE == 00FB` already observes the derived byte, `--slice-ticks` exists). Add one scenario `diag-ctrl-1` (the new original diagnostic, section 6) and extend the existing determinism test (second run, `--slice-ticks 1`, `7`, other working directory give the same report apart from `host.calls`) to cover it. Do NOT add report keys for the derived bytes now: `ivsr 1` is a closed key table and every golden carries it; peek covers the need. Add one documentation line to `docs/runner.md`: the `c0`/`c1` words are indexed by PSG port.
- Installed consumer: extend the replay of the two-event scenario to print the peeked `$01FE` after the first event (expected `0x007E` for KEY_1) and compare to a literal (tier `installed-host-behavior`), so the packaged artifact proves the read path from outside.
- Docs: record in the header comment of `ivs_run.h` that the controller words are now guest-visible (replace "Nothing in the machine reads the words in Phase 4") and invert `input.guest-invisible-phase4` deliberately (named in 04-DISCUSS-ctrl section 7: the golden change is reviewed, not accidental).
- C67: add the instruction-granularity test: with the poll instruction starting at tick s, an event at at_tick = s is visible to that read, at s + 1 is visible only to the next unit's read, and at s - 1 is visible (applied before the unit). This resolves the Phase 4 note that C67's resolver is a "Phase 5 PSG port-read test".

### P5-C5. Evidence tiers and claims (point 5)

- INP-01 single-control table: claim tier `source-expectation`, standing `provisional` (weakest-field rule: all dependencies `tier4-community`/`inference`), artifact the literal file and the ctest log; depends_on `input.control.*`.
- INP-01 guest read path: claim tier `original-diagnostic` (the new guest diagnostic reads the bytes), `provisional`; the expected values come from the literal, so this proves the core's path implements the table, not that hardware does.
- INP-02 combination rule: `source-expectation` for the wiki witnesses and the four statements; `deterministic-repeatability` for the property tests; policy rows are recorded in the ledger text as `unknown-default-chosen`, no claim.
- API-03: `deterministic-repeatability` (slicing equivalence) plus `installed-host-behavior` (consumer replay). Neither says anything about hardware accuracy.
- No claim uses `measured-hardware`; the claims step already fails that tier without D10.
- Discrepancies: C14 stays `open` (rewrite default_policy to point at the generated table); C15 -> `partial` (four buttons, three codes, both top buttons read $A0 in two tier-4 sources; schematic still missing); C16 stays `open` but its `sources` text is rewritten to the three-source finding of section 2.1 (the "duplicate pin" remark is dropped, or marked "not reproduced"); C63 default_policy gains the `disc + disc` policy rows; C64 split: unplugged reduces to idle by inference (note), upper byte stays open with default `$00`; C65 default_policy states "output mode reads the latch"; C66 unchanged with the two-source naming mismatch noted; C67 gets the Phase 5 test as evidence. New discrepancy candidates (ids at write time, next free after C68): (i) octal vs decimal register numbering across the datasheet and the project text; (ii) 8914 versus 8910 port behaviour (extends C11); (iii) whether the PSG reset line follows system reset.
- New sources in `data/sources.json`: `SRC-tilton-intv-pinouts` (web archive of the Deathskull Labs page, author Jay Tilton, dated 28 June 1998), `SRC-consolemods-intv-pinouts` (page oldid 23186, read through a web-archive capture; live host 403), and a locator `Hand_controller.png@oldid-14808` under `SRC-intv-wiki`. Rights `unknown`, authority `tier4-community`, hashes in section 9.

## 5. Lens notes

- Emulator architect: the model is switch state -> line network -> pin level, with the PSG direction register selecting pin versus latch. All three stages are data or a pure function. Keeping the derived pulldown in the same struct as the gate flags matches how STIC bus effects will reach the memory system in Phase 6. No per-access visibility; unit-start granularity is kept (C67).
- Hardware/EE: ground-switch plus pull-up is the datasheet's own recommended scan method, and the wiki prose reproduces under it. What the model cannot say is contention (a driven-high pin shorted by a closed switch) and the 8914's differences. Both are labelled. Octal register numbering is the main EE footgun (section 2.2).
- QA: independence is protected three ways: the literal file is source-only and never generated from the profile; two source lineages are compared at authoring time; the wiki's prose statements are an oracle for the combination model that the table did not use as input. All class rows are exhaustive over 31 controls x 2 ports; combination behaviour is a property over random subsets with the oracle computed from the literal in test code.
- Security/rights: no overlay art, no device names, no image or page copy in the repo; the new sources are cache-only; the literal file is facts with citations. The hygiene gate rule against home paths applies to the literal's `src=` locators (use SRC ids, never local paths).
- DevOps/CI: cost is small. The generator runs at configure time (milliseconds). The 62 single-control checks run as 6 ctest cases, each looping; property tests are seeded and bounded. No new CI job, no new gate step if the literal probe joins the existing memmap probe step (inventory file `data/expected/gate-steps` stays at its current size unless the planner prefers a distinct `ctrl-table` step; either is a one-line choice). Estimated added gate time [INF] well under 2 s of the 45 s hosted gate; measure per CI-EFFICIENCY.md.
- API/integrator: nothing in the public ABI changes. The 32-byte event, the 31 bit macros, `controller_now` and peek are the whole interface. A LibFuturo adapter still maps a 32-bit mask into controller words; host-side SOCD and keypad exclusivity stay in the host. The note for integrators: the byte a guest sees for KEY_5 + SIDE_TOP is not decodable by design; hosts should not send keypad plus disc.
- Product/DX: the runner scenario syntax already speaks names (`c0=KEY_1+DISC_N`). A diagnostic failure should say which of the 62 expectations differed, so each loop reports `<port> <control> expected <HH> got <HH>`.
- Lessons from other emulators and ecosystems [BG/INF, no source opened]: frame-sampled pad masks tie input to a host frame and lose keypad combinations; edge events with host timestamps are non-deterministic across hosts; emulators that keep the table as hard-coded 8-bit codes make every later correction a code change and carry no per-row evidence. Agreement with such emulators would not be independent evidence anyway (AGENTS.md).

## 6. Requirement-to-test mapping (suggested, names are proposals for the planner)

New suite `input` (`tests/input/`), seeded, no wall clock, no rights-bearing bytes; the existing `run` suite keeps API-03 cases.

| Req | Test (ctest name) | Oracle and tier |
|---|---|---|
| INP-01 | `input.table.literal-vs-sources`: pins mapped through the header map equal `cleared` equal `wiki` for all 31 rows | source cross-check, `source-expectation` |
| INP-01 | `input.table.profile-vs-literal`: generated table equals the literal; control GEN_CTRL_MUTATION must fail it | source-expectation |
| INP-01 | `input.read.keys.port0` / `.port1`, `input.read.disc.port0` / `.port1`, `input.read.side.port0` / `.port1`: set one bit on one port, assert peek of `$01FE`/`$01FF` equals the literal byte, the other port reads `$00FF`, upper byte `$00` | expected from literal, `source-expectation` |
| INP-01 | `input.read.idle-and-reset`: both ports `$00FF` after reset and after events then reset | |
| INP-01 | `input.guest.reads-table`: guest diagnostic `diag-ctrl-1` (below) stores both port reads for a scripted sequence of single controls; predicates in `data/golden/diag-ctrl-1.expect` | `original-diagnostic` |
| INP-02 | `input.combo.wiki-witnesses`: KEY_1 -> `$007E`, KEY_1 + KEY_9 -> `$005A` | wiki numbers, `source-expectation` |
| INP-02 | `input.combo.union-property`: seeded random subsets of the 31 bits on both ports; byte equals `0xFF XOR OR(literal masks)` computed in test code | model definition, `deterministic-repeatability` (labelled "defines the model", not hardware) |
| INP-02 | `input.combo.wiki-decodability`: 0 collisions in {idle, key, disc, side, disc + side}; key + side 36/36, side + side 3/3, key + disc 180/192 ambiguous | derived from the four wiki statements, `source-expectation` |
| INP-02 | `input.combo.policy-rows`: opposite pairs, adjacent discs, the 56 disc pair unions equal to a position, three-way; each row listed in `data/expected/input-policies.ids` with tag `unknown-default-chosen` | policy, no claim |
| INP-03 | `input.bits.inventory` (exists), `input.refuse.*` (exist), plus `input.core.no-host-names`: symbol/grep check that the core headers contain no device, remap or overlay vocabulary (extends the API-05 style check) | structural |
| INP-03 | `input.port.reads-are-pure`: peek and CPU read leave every digest unchanged | |
| port path | `input.bus.direction-input-reads-pins`, `input.bus.direction-output-reads-latch`, `input.bus.per-port-direction`, `input.bus.upper-byte`, `input.bus.write-latch-in-input-mode`, `input.bus.peek-equals-cpu-read` | R7 sense from datasheet 5-26; output readback `unknown-default-chosen` |
| port path | `input.digest.mem-covers-pins`: instances differing only in a controller word have different MEM and composite digests; same schedule, same digests | hidden-state rule |
| port path | `input.takeback.restores-pins`: extends `run.stop.internal-input-restored` | |
| API-03 | `run.fuzz.input-visible-seed-01..08` (contents B and C), `run.fuzz.input-visible-covered` (non-vacuity: an event inside an overshoot window changed a read, a read changed a branch, one event pending at the end) | independent expectation from content B, reference slicing for C |
| API-03 | `run.input.granularity-c67`: events at s - 1, s, s + 1 around the polling instruction | resolves C67 |
| API-03 | `runner.determinism.diag-ctrl-1`: reports at default, `--slice-ticks 1` and `7` equal except `host.calls` | |
| API-03 | installed consumer: peeked `$01FE` after KEY_1 equals `0x007E` | `installed-host-behavior` |
| rights | existing `fixtures` and hygiene gates accept `FX-diag-ctrl-1` with manifest source, recipe, hashes, `redistributable: yes` | |

Guest diagnostic: `diag-ctrl-1`, hand-assembled annotated decle listing with the Phase 4 listing checker (D4: assembler-free; this is the second diagnostic, below the "more than 3" trigger). It writes R7 for each direction case, reads `$01FE`/`$01FF`, stores to system RAM. Scenario `data/scenarios/diag-ctrl-1.ivss` applies single controls and chosen combinations at tick boundaries between known instructions.

Named faults (prefer controls where they prove the same thing, A-06): `INPUT_PINS_NOT_UPDATED` (apply skips the setter), `CTRL_PORTS_SWAPPED`, `CTRL_UNION_AS_AND` (AND of masks), `CTRL_NO_INVERSION` (reads the active-high mask), `CTRL_DIRECTION_SENSE_FLIPPED`, `INPUT_PINS_NOT_RESTORED_ON_TAKEBACK`, `INPUT_PINS_NOT_RESET`, `GEN_CTRL_MUTATION`. Each needs its control under `tools/check/controls/` and an `expected/faults` row naming the intended failing test.

## 7. Anti-patterns and footguns

1. Treating the two matching sources as independent evidence. They are one pin lineage plus one software-level lineage; both are tier 4.
2. Octal register numbers read as decimal (R16/R17 octal are $01FE/$01FF; R13-R15 envelope registers are decimal 11-13).
3. Reading "Input Enable" without the overbar (text extraction loses it): bit 1 means output.
4. A raw-byte override or electrical event field "for debugging": two sources of truth. Reversal path is a flag-gated event field, not a second default path (D-12).
5. Putting the derived byte in the INPUT row only: the composite digest would not see guest-visible state (hidden-state rule).
6. A cache without a take-back: the refused-step restore must re-derive the pins.
7. Using player names (left/right/1P/2P) in the core: sources disagree on which port is which; C66.
8. A keypad "either/or" rule in the core: the BSR sentence is a programmer-level simplification; the electrical result of both is the union.
9. Per-access input visibility inside an instruction: not justified by any evidence and breaks the one-place-to-change property.
10. Generating the literal file from the profile (or the reverse): destroys the independence the file exists for.

## 8. Adversarial pass against my own recommendation

1. "Two tier-4 lineages agreeing is noise." Partly true. The agreement still catches transcription errors in a table that would otherwise rest on one extraction, and it fixed the disc orientation. Defence: all 31 fields stay tier 4, claims `provisional`, D10 stays the only path to better.
2. "The union rule is baked in as the physical model but only the observed single-control values and wiki prose support it." Correct, and the rule is a profile field (`input.line_model`) with tag `inference`; any other model would need the generator and the derive function to branch on it, so the branch exists (generator refuses unknown models). Reversal evidence is a schematic or D10 pair measurements.
3. "Per-field profile entries are heavy." 31 ids is the same shape as the 30 memmap region fields and gives the per-row tags the roadmap success criterion demands. A table-valued field would not.
4. "The pins cache duplicates the words." Yes, deliberately, for peek/CPU single-path purity and no composite bump; guarded by an invariant test and the take-back test. If the owner prefers zero duplication, P2 is available at the price of a composite version 2 and a read funnel.
5. "Output-mode read returns the latch" may be wrong (the part may return the pin). It matters only if a guest reads a port it set to output, which the 2609 EXEC does not need for controllers [UNK]; it is one labelled field with a D10 resolver. The same applies to R7 sense on the 8914: if the 8914 differs, the datasheet-based default flips one field.
6. "Upper byte `$00` rests on a code snippet." Agreed; it is the weakest guest-visible default and the first thing to measure. The consequence of being wrong in one direction is guests that mask the byte do not notice, and guests that do not mask misbehave in the idle test; the failure would show immediately with an owner EXEC run (D11), so detection is cheap, only evidence is expensive.
7. "Unit-start granularity vs hardware." Unchanged from Phase 4 (C67): contract, not claim, with a profile-flag reversal path; Phase 5 now has a test that pins it.
8. "Why not make the host send bytes, since the guest only sees bytes?" That scatters the unverified table across hosts and logs and makes INP-01 untestable in the core; the Phase 4 decision stands and nothing found here contradicts it. Conversely nothing here shows that bridging or active electronics exist, so the reversal trigger of D6 (output not a function of switch state plus port drive) is not met; the only unmodelled interaction is contention, which is expressible later because the port drive is an input of the read function.
9. "ECS keyboard scanning uses ports as outputs." Out of scope (PROJECT.md). The read function takes the direction and latch as inputs, so a keyboard profile can add bridging later without changing the event format.

## 9. Evidence log (what I fetched, so the planner can cite and re-fetch)

| Source | Locator | Result |
|---|---|---|
| GI 1980 data catalog (SRC-gi-1980-catalog) | pdf 204-208 = printed 5-23..5-27 | re-fetched, sha256 matches the recorded `70094d12...9a09`; pull-up, direction, R16/R17 and reset sentences read as above; no hand-controller section found by text search (the 8900 pages do not mention controllers) |
| Blue Sky Rangers 2609 page (SRC-bsr-2609) | HAND CONTROLLERS section | re-fetched, sha256 matches `3d779159...4819`; sentences quoted above |
| Wiki Hand Controller, oldid 14808 | page text and image `Hand_controller.png` | page html sha256 `263b132f...3989` (14,861 bytes); image sha256 `f789cc5a...1d36` (7,498 bytes, 283 x 443 grayscale); image read locally only; the full 31-value table was read from it |
| Deathskull Labs, Intellivision Controller Pinouts (J. Tilton, 06/28/98), web archive 2006-05 | `intvcont.html` | sha256 `c93d5014...ce17` (10,837 bytes); chart parsed from the raw HTML cells |
| ConsoleMods wiki Intellivision:Connector Pinouts, oldid 23186 | web-archive capture (live 403) | sha256 `984d548c...d6c` (36,791 bytes); pin-to-bit map and identical matrix |
| Not obtained | iFixit 2609 service manual viewer, Sylvania service manual (403), Intellivision II service manual (no response), AtariAge controller-matrix threads (403) | missing evidence, no tier-3 source yet |
| Script checks | scratch only (not committed) | 31/31 values match; K1 = `$7E`, K1 + K9 = `$5A`; decodability statements reproduced; no duplicate single masks |
Full hashes are truncated here for readability; the planner records the complete digests in `data/sources.json` when the sources are added.

## 10. Reversal triggers

- A schematic, service manual or D10 measurement shows a pin set different from the table, a non-union combination, or contact structure that makes the output not a function of switch state plus port drive: change profile fields (table) or add a `bridge` extension; D6 reversal only for the last case.
- An owner EXEC run (D11) shows the guest decodes a different byte for any control or reads a different upper byte: flip the table row or `memmap.device.psg8.fill`.
- A 8914 datasheet or die analysis shows port behaviour unlike the 8910 (direction sense, pull-ups, output readback): replace the `inference` tags with the primary cite and change the affected field.
- A consumer needs raw port injection: flag-gated event field (D-12 reversal), no second default path.
- Per-access visibility is shown to matter (a guest reads the controller twice within a unit): refine C67 with a profile flag; the fuzz and C67 test change together.
- The owner prefers no cached state: switch to P2 with composite version 2.

## 11. Open items for the orchestrator

1. Coordinate with the PSG point(s): PSG register file location (memsys), R7 direction sense and decimal/octal wording, `memmap.device.psg8` and the `psg_regs` region switch from `stub` to `real`, the single MEM digest change in Phase 5, power-on state of R7.
2. Add the three new source ids and decide whether the literal probe joins the existing memmap probe step or gets its own gate step (inventory edit either way).
3. Decide the diagnostic fixture id and whether `diag-ctrl-1` shares the Phase 4 boot stub entry convention (it should).
4. Rewrite C14/C15/C16 text and add the new discrepancy rows (ids at write time).
5. Owner-facing D10 checklist (hard stop, evidence marked missing): all 31 controls singly on each port, pair classes (key + key, key + disc, key + side, disc + side, side + side, adjacent discs, opposite discs), idle byte and upper byte, read with a port set to output while a switch is closed, which connector feeds `$01FE`, reset state of R7.

## 12. Top unknowns, ranked

1. Upper byte of the 16-bit port read (C64): default `$00`, weak tier-4 inference.
2. Output-direction readback and contention, plus whether the AY-3-8914 matches the 8910 on direction sense (C65, C11): tier-1 text only for the sibling part.
3. The whole table is tier 4 and effectively one lineage; no schematic or service manual obtained (C16, C15); D10 is the only path to `measured`.

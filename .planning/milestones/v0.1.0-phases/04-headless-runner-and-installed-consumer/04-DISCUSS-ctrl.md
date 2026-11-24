# Phase 4 discuss: Decision point F, D6 controller core-input representation (C14)

Status: recommendation (fan-out protocol, `.planning/AUTONOMY.md` section 2). Labels: [OBS] read or run in this session, [INF] inference, [REC] recommendation, [UNK] unknown, [BG] background not re-fetched.
Inputs read [OBS]: AGENTS.md, AUTONOMY.md s1-3, PROJECT.md Decision Register, REQUIREMENTS (API-02/03/04, RUN-05, INP-01..03), ROADMAP Phase 4/5, `core/include/ivs/*.h`, `core/src/machine/{machine.h,machine_run.c}`, `docs/time-base.md`, profile JSON, `data/discrepancies.json` (C14, C15, C16), research ARCHITECTURE 4.1-4.3 and FEATURES 1.4 (dated input, not decisions), 03-CONTEXT.
No emulator source opened. Controller facts below are tier-2/4 leads from the already-recorded research (BSR 2609 page, intellivision.us wiki oldid=14808, one archived pin page); none was re-fetched here and none is tier 1.

## 1. The decision in one paragraph

[REC] The single core input representation is the logical switch state of each hand controller: a fixed-size packed word per controller with one bit per physical control surface (12 keypad keys, 16 disc positions, 3 side-button codes = 31 bits). The core turns it into 8 active-low port lines through profile data (Phase 5). The electrical byte is a derived, read-only observation, never an input. This is "both layers" only in the sense that the derived byte is observable; there is exactly one input path and no raw-byte override. Input is delivered as a caller-owned, time-sorted array of level samples inside the run request, each effective at the first CPU-unit boundary at or after its machine-time tick. There is no stateful "set input, applies on next run" call, because that is host-slicing dependent and violates API-03.

## 2. What the evidence supports (and does not)

- [OBS, tier 2/4] Each controller feeds an 8-bit port that is the PSG I/O port (R14 at $01FE, R15 at $01FF); idle reads all ones in the low 8 bits; pressed controls clear bits; wiki: key 1 gives $7E, keys 1+9 give $5A; some combinations are not uniquely decodable (keypad+side/disc, two side buttons). BSR: disc 5-bit code, action 3-bit code, keypad 8-bit code; the two top buttons are wired together.
- [OBS, tier 4, LOW] One archived page says each control grounds a combination of two pins (S1 = pins 6+8, K1 = pins 4+6, D9 = pin 4 only), one apparent duplicate row (C16).
- [INF] Everything known is compatible with "byte = NOT(OR over closed switches of that switch's line mask)", but nothing proves switches ground lines independently rather than bridge two lines. If they bridge, a port that the PSG drives as an output interacts with them. Only a switch-level input can express that later; a byte-level input cannot.
- [INF] The guest only ever sees bytes, so for the machine's behaviour the byte is sufficient information. The reasons to keep the switch state as the input are ownership of the unverified mapping, replay stability, and the port-direction case, not information loss at the CPU.
- [UNK] Disc internals (is every one of 16 positions a single closed contact set, can two adjacent positions close together), unplugged-port read, upper byte of the 16-bit read, which connector is R14. C15 (4 physical buttons, 3 codes) and C16 stay open.

## 3. Options

| | A. Logical lossless only | B. Electrical bytes per port | C. Both: logical in, core derives bytes, bytes also observable (chosen, with no byte override) |
|---|---|---|---|
| Input shape | 31 control-surface bits per controller (or 12-bit keypad + disc enum + 3 side = 20 bits) | 2 x 8-bit active-low values (16 bits; 32 with ECS) | Same as A; plus a read-only derived port-byte observation |
| Where control-to-line mapping lives | core, profile data | every host or the runner | core, profile data |
| Fixing C16 later | edit one profile table; old replay logs stay valid | every host and every recorded log changes | same as A |
| Port-direction / bridging effects (Phase 5, ECS keyboard scan) | expressible (function of switch state and port drive) | not expressible; byte frozen before the PSG writes R7/latch | expressible |
| Lossless record/replay of intent | yes | no: $5A does not say which keys; ambiguous aliases collapse | yes |
| Host burden | must know the 31 glyph bits only | must know pin matrix (unverified) | same as A |
| INP-01/02/03 | INP-01 and 02 need the core table anyway; INP-03 satisfied | INP-01 "each produce the profile's documented read value" is untestable in the core if the host supplies the value; INP-03 weak | satisfied |
| LibFuturo 32-bit mask fit | lossy subset only via adapter | fits (16 bits) | adapter maps mask to logical, lossy subset labelled |
| Risk | wrong table is wrong for all hosts | wrong table is wrong in every host separately and in logs | same as A, plus the observation lets tests see the byte |
| Anti-pattern flagged | none | owner rule (PROJECT.md: electrical modelling core-side, host mapping separate) is violated | adding a raw-byte override creates two sources of truth: rejected |

Variant inside A, disc encoding: enum (0 = none, 16 directions) vs one-hot 16 bits.
- Enum: 5 bits, opposite or double directions are unrepresentable, invalid codes need a refusal. INP-02's "opposite directions" becomes "cannot occur".
- One-hot (chosen) [REC]: every state is representable, no decode or range refusal, matches "set of closed switches", and a profile table of 16 masks is trivially extendable if C16 shows two adjacent positions can close together. Cost: 3 more bits, and combos that real hardware cannot produce must be given a documented default (union), tagged `unknown-default-chosen`.
- Hosts with a stick or d-pad pick one bit; any opposite-direction resolution (SOCD) is host policy, outside the core.

Variant inside delivery, three ways to carry time:
1. Stateful `set_input(state)` applied at the start of the next run: rejected. Where a host slices decides when input lands, so the same script gives different results (violates API-03; this was the earlier research recommendation, 4.1 item 4, and is superseded here).
2. Bounded queue in the instance (`input_at(T, state)` copies into fixed storage): rejected. The queue is hidden state; the digest either covers it (then digest depends on how the host submitted) or omits it (hidden state, save-state hazard); the capacity is a footgun.
3. Caller-owned sorted event array in the run request, core retains nothing (chosen). Stateless between calls except the state in force, the event count, a chain hash and the input floor.

Variant: edge events (press/release) vs level samples. Level samples chosen: idempotent, a lost or repeated event cannot desynchronize, lossless, trivially digestable.

## 4. Lens notes

- Emulator architect: with CPU-unit scheduling (D2, Option B), the unit is the quantum. Event effect = "visible to every guest access made by a unit that starts at or after T". Granularity is at most one instruction (at most about 12 cycles = 48 ticks of a 59,736-tick frame); a real controller is a mechanical device read at frame rate, so this loses nothing observable. Phase 5 may refine to per-access visibility only if the slicing test keeps passing and the contract text changes (see risks).
- Hardware/EE: model is switch state to line network; the byte also depends on PSG port direction and output latch (R7 direction bits, [BG] AY-3-891x class). Phase 5 must define the byte as f(switch state, port drive), with outputs-driven reads a labelled default. [UNK] whether idle upper bits of the 16-bit read are 0 or open bus (C-new, below).
- QA: all properties are testable now without a PSG: validation, floor, ordering, slicing invariance, digest framing, isolation. The unit-start rule gives one crisp oracle for "when did it land" (the log).
- Security/rights: input is plain integers; no overlays, device names or artwork in the core (overlays are third-party artwork, FEATURES 1.4). Validate the whole array before the first write; refuse spans overlapping instance storage (A-06 rule).
- DevOps/CI: cost is small. One new ids inventory, a handful of ctest cases (seeded, no wall clock), 3 to 4 named faults. Budget impact estimate [INF] under 1 s of the 30-49 s hosted gate; measure per CI-EFFICIENCY.md.
- API/integrator: one concept. A host builds events from whatever it has (SDL pad, keyboard, TAS file); the core never sees a device. The host needs only the bit macros. For LibFuturo (32-bit mask) an adapter maps up to 31 mask bits to 31 control bits, so the lossy-subset story needs no core change.
- Product/DX: runner scenario files list events in readable names that map one-to-one to header macros; hex words are also accepted for exact replay. Debuggability: the run report and `ivs_info` show the controller words in force and the derived bytes (Phase 5).
- Lessons from other ecosystems [BG/INF, reasoning from published project docs only]: emulators that take "pressed buttons" per frame and sample once per frame make record/replay frame-granular and tie input to a host frame; those that take edge events with host timestamps are non-deterministic across hosts. A level sample stamped with machine time avoids both. Libretro-style 32-bit joypad masks lose multi-key keypad states, which is exactly the reason not to adopt a 32-bit "buttons" word as the core contract.

## 5. Adversarial pass against the recommendation

1. "The core table rests on one LOW source." True, but it is a profile table with a tier tag per row (C16 policy), and a correction changes one file and no input log or host. Option B scatters the same unverified table into every host and every recorded log. Mitigation stays: no measured-tier claim before D10.
2. "One-hot admits states the hardware cannot produce, so tests assert fiction." Mitigated: such states get a single documented default (union) tagged `unknown-default-chosen`, listed in the INP-02 test as a policy row, never as evidence. The enum alternative hides the same uncertainty instead of documenting it.
3. "The byte is all the guest sees, so logical state is over-engineering." Counter: port-direction and bridging cases (ECS keyboard scanning uses a port as output [BG]) need switch-level state; reversing later would be an ABI break and invalidate logs.
4. "Unit-start quantization is less accurate than per-access." It is coarser than hardware but deterministic, slicing invariant and one place to change. Reversal path exists (contract text plus profile flag), cost is a golden update, and games poll once per frame.
5. "Both layers invites a debug byte override." Rejected on purpose. Anything non-switch-reachable (cable fault, floating port) enters later as a flag bit in the event record that is UNSUPPORTED now, not as a second input path.
6. "Refusing events before the floor breaks hosts that deliver late." That is the point of API-03: a late event must be an error, not a silent timing difference. A complete sorted script never trips it.
7. "Machine-time unit mismatch": API-02 text says cycles; D2 (docs/time-base.md) fixes the ABI unit as master ticks. [REC] events use `at_tick` in master ticks like `ivs_info.time_ticks`; the runner accepts `at_cycle` and converts with the profile ratio (CPU-cycle helper stays in the runner per D2). If point-decision A (run API) picks a different unit, only this field's unit changes.

## 6. Exact reserved/implemented API surface for Phase 4 [REC]

Implement real plumbing now (logic is small); defer only the control-to-byte function and PSG port reads to Phase 5. Reason: a call that is accepted and ignored changes meaning in Phase 5; real logical semantics now means Phase 5 adds an observer, not a behaviour change. Fallback if planning wants minimal: events validate and are consumed but the guest never sees them; this is the same thing, because Phase 4 has no PSG.

Header additions (names are proposals; numbers are append-only once merged):

```c
/* Controller control-surface bits (one controller word, uint32_t). 31 used, bit 31 reserved. */
#define IVS_CTRL_KEY_1 (1u << 0)   /* ... KEY_9 bit 8; KEY_CLEAR 9, KEY_0 10, KEY_ENTER 11 */
#define IVS_CTRL_DISC(n) (1u << (12 + (n)))   /* n 0..15, clockwise from north, 22.5 degree steps */
#define IVS_CTRL_SIDE_TOP (1u << 28)          /* both top buttons: one code (C15) */
#define IVS_CTRL_SIDE_LOWER_LEFT (1u << 29)
#define IVS_CTRL_SIDE_LOWER_RIGHT (1u << 30)
#define IVS_CTRL_RESERVED_MASK 0x80000000u    /* nonzero: UNSUPPORTED */
#define IVS_INPUT_CONTROLLER_WORDS 4u         /* v1 profile: 2 connected; words 2,3 reserved for ECS ports */

typedef struct ivs_input_event {   /* array element: fixed v1 record, no struct_size; growth = new request field */
  uint64_t at_tick;                /* master ticks, same unit as ivs_info.time_ticks; < 2^63 */
  uint32_t flags;                  /* must be 0 (UNSUPPORTED otherwise) */
  uint32_t reserved;               /* must be 0 */
  uint32_t controller[IVS_INPUT_CONTROLLER_WORDS]; /* the full level state from at_tick on; index = port (0 = R14, 1 = R15) */
} ivs_input_event;                 /* 32 bytes */
```
Controller index means the PSG port (0 = R14/$01FE, 1 = R15/$01FF), never "player", "left" or "right"; which connector is which is a host/profile fact [UNK, wiki only].

Run request fields (the container struct belongs to decision point A/API-02; these are the input fields it must carry): `const ivs_input_event *input_events; uint32_t input_event_count;`. Run report fields: `uint32_t input_consumed; uint32_t input_reason; uint32_t input_event_index; uint64_t input_floor_tick; uint32_t controller_now[4];`.

Semantics (all of these become D-xx text):
1. Validation precedes any write and covers the whole array: count > 0 with NULL events is BAD_ARG; array overlapping instance storage is BAD_ARG; `at_tick` not nondecreasing is BAD_ARG; `at_tick` >= 2^63 or below the input floor is BAD_ARG; nonzero event flags/reserved, reserved control bits, or a nonzero word at an index at or above the profile's controller count is UNSUPPORTED. The failing index and a reason go in the run report. No new status codes (-6..-15 stay free); reasons extend the append-only `IVS_REASON_*` space (proposal: 40 UNSORTED, 41 BEFORE_FLOOR, 42 TICK_RANGE, 43 CONTROL_RESERVED, 44 CONTROLLER_COUNT, 45 EVENT_FLAGS, 46 NULL_EVENTS, 47 EVENT_OVERLAP).
2. Application: before each CPU unit starting at tick s, every not-yet-applied event with `at_tick <= s` is applied in order (last wins as state in force). Events with `at_tick` greater than the start of the last unit run stay unconsumed; `input_consumed` reports how many were applied. The host resubmits the unconsumed suffix in the next slice. Nothing is retained between calls.
3. Input floor: `input_floor_tick` = (start tick of the most recently begun unit) + 1, or 0 before any unit has run. Events below it are refused. A complete script resubmitted from `input_consumed` never trips it.
4. Reset (power cycle) sets all controller words to 0 (idle), count 0, chain to its initial value, floor 0, and the host must resubmit; input is not part of load/unload (BAD_STATE rules unchanged).
5. Input log domain (RUN-05): make `IVS_DIGEST_RESERVED_INPUT` (18) live as `IVS_DIGEST_INPUT`. Rows in the existing field-table framing ("IVSD" || domain || version || rows): controller words in force (4 x u32), applied-event count (u64), chain hash (32 bytes). Chain: `h0 = SHA-256("IVS-INPUT-LOG-V1")`, `h(n+1) = SHA-256(h(n) || be64(at_tick) || be32(c0) || be32(c1) || be32(c2) || be32(c3))`, recording the script tick (not a unit-relative tick), so the log equals the accepted script and is slicing invariant. Same chain is computable by an independent script (see tests). Composite domain 0 must start covering 16..18 when they go live: one composite version bump (`IVS_DIGEST_COMPOSITE_VERSION` 2) with golden updates, done once in Phase 4 (hidden-state rule: state that changes future behaviour must be in the composite).
6. Not provided in Phase 4 (and why): no `set_input`; no pull callback (violates no-callbacks rule); no "frame" unit (a frame is a timeline event, never derived by modulo; `at_frame` waits for the STIC frame event, Phase 6); no raw-byte path.
7. Instance cost: 16 bytes controller words + 8 count + 32 chain + 8 floor, fixed in the instance, no allocation; update the "instance size changed" static assert and digest field table deliberately.

Runner scenario input syntax [REC, small]: one line per event, `input at_tick=<n>|at_cycle=<n> c0=<names|0xHEX> c1=<names|0xHEX>`, names like `key1,key9,disc:N,disc:ENE,side:top`. The name table is checked against the header by an inventory (`data/expected/input-bits.ids`: name, bit number) so the runner cannot drift from the header. The runner reports effective input via the log chain and a text input log (script ticks), never wall time.

## 7. Phase 5 follow-up (not Phase 4 work)

- Profile keys (tag per key, `unknown-default-chosen` or tier 4 until D10): `input.controllers` (2; add in Phase 4), `input.port_map` (controller 0 to R14), `input.line_model` (`switch-union-active-low`), 31 per-control 8-bit line masks per controller, `input.idle_byte` (0xFF, tier 4 wiki), upper-byte policy, unplugged policy. Phase 4 adds only `input.controllers`; the table and others wait for INP-01 so profile-fields.ids does not carry unused unknowns.
- Pure internal function `ivs_input_port_byte(profile, switch_word, port_drive) -> byte`, then PSG R14/R15 reads call it with the state in force; readback of an output-driven port is an explicit labelled policy.
- INP-01 tests table-driven from a hand-authored file (`data/expected/` literals from the wiki/pin page, tier recorded per row; expected values are written from sources, not from the implementation). INP-02: pairwise policy rows and the union property. Port reads also appear in the CPU/MEM digest, so the Phase 4 "input is guest invisible" test is deliberately inverted at that point (golden change reviewed).
- Optional later: `at_frame` through a core frame event; ECS words 2 and 3; a debug output of derived bytes in the run report (`port_now[4]`).

## 8. Tests writable now (Phase 4), with named faults

1. `input.bits.inventory`: static assertions that the 31 bits are distinct and disjoint from `IVS_CTRL_RESERVED_MASK`; `input-bits.ids` equality; negative control: a duplicated bit number fails the step.
2. `input.refuse.*` (rows in the API effects table, each asserting status, reason, failing index, and instance digest unchanged): NULL events with count, unsorted, tick >= 2^63, tick below floor, reserved control bit, word 2 nonzero on the 2-controller profile, nonzero event flags, overlap with instance storage, bad run request not mutating the input state.
3. `input.slice.invariance`: seeded generator produces schedules and random slice targets (including targets that land inside an instruction's overshoot window and events with `at_tick` inside that window); single run versus many slices give equal domain 18 digest, `controller_now`, applied count, and CPU/MEM digests at equal final tick. Also two interleaved instances with different schedules equal their sequential runs.
4. `input.log.independent-oracle`: expected chain value for a literal 3-event schedule computed by a script using only `cmake -E sha256sum`/printf (independent of core SHA code), compared with the core. This tests framing only; tier is "self-consistency", labelled so in the golden header.
5. `input.guest-invisible-phase4`: with the stub PSG, any schedule leaves CPU/MEM/IMAGES digests equal to the no-input run (documents the Phase 4 state; flips in Phase 5).
6. Reset: events applied, reset, state back to idle, count 0, floor 0, digest equals fresh power-on.
7. Runner scenario test with an input schedule (RUN-01), installed-consumer step that submits two events across two slices and prints the domain 18 digest compared to a literal.
8. Named faults (RUN-04) with the intended-reason assertion: `INPUT_APPLY_AT_RUN_START` (applies pending at slice start; fails slicing test), `INPUT_LOG_SLICE_RELATIVE_TICK` (fails slicing and independent oracle), `INPUT_NO_FLOOR_CHECK` (fails refusal row), `INPUT_STATIC_CARRY` (a static holds the last state; fails isolation). Each needs a control under `tools/check/controls/` and an `expected/faults` entry per the existing fault inventory.

## 9. Evidence and discrepancy entries to record

- C14: status to `decided (D6, delegated, owner authorization 2026-11-09)`: logical switch-state input, core-side electrical function, one input path. Resolver for the underlying wiring stays pending (C16). Reversal trigger: schematic/measurement showing the controller output cannot be modelled as a function of switch state plus port drive, or a first-class consumer requiring raw bytes.
- C15: record the modelling choice (three codes; top pair share one bit; a host maps either top button to `IVS_CTRL_SIDE_TOP`); still open pending a schematic.
- C16: unchanged, plus: the table is the only place the pin matrix lives.
- New entries [REC]: (a) disc internals and whether two positions can be closed together (one-hot default = union); (b) unplugged-connector and upper-byte read; (c) read of a port the PSG drives as output; (d) which connector is R14 (wiki only); (e) granularity decision (CPU-unit start) as an architecture decision with a profile-flag reversal path, not a hardware claim.
- Decision Register row D6: "Logical switch state per controller (31 control-surface bits, one word per PSG port), timestamped level samples in the run request, derived electrical bytes in-core (Phase 5); reverse if: schematic/measurement invalidates switch-state modelling, or an owner requires raw-byte injection." Also record that the earlier research recommendation (apply at next run) is superseded.

## 10. What needs D10 hardware measurement (hard stop, mark missing)

Owner console plus a project-authored reader diagnostic: all 31 controls singly on each port (8-bit read), selected pairs (key+key, disc+key, disc+side, side+side, adjacent discs if mechanically possible), idle value of the upper byte, unplugged port read, read with the PSG port direction set to output, left/right-to-port mapping. Until then every table row carries tier 4 or `unknown-default-chosen`, and no measured-tier claim is made (ROADMAP D10).

## 11. Risks, reversal, blockers

Top risks:
1. The controller table rests on LOW tier-4 evidence; a wrong union rule would propagate to guest results. Contained by profile data, tier tags, and D10.
2. The unit-start application granularity is a contract choice that may differ from hardware by up to one instruction; refining it later changes goldens.
3. Run API shape (decision point A) may place the event array elsewhere or use a different time unit; the input semantics here are written to survive that, but the planner must reconcile the two D-xx sets, especially the ticks-versus-cycles wording of API-02.

What would reverse it: evidence that line states cannot be derived from a switch set (e.g. active electronics in the controller), an owner or consumer need for raw port injection (then add a flag-gated event field, still no second default path), or a LibFuturo shared-contract change that fixes a wider mask.

Blocking [UNK]: none for planning. Open but non-blocking: the exact numbering and ordering of disc positions and keypad bits is a header convention (clockwise from north, key order 1 to 9, Clear, 0, Enter [BG: keypad layout]) that should be confirmed against the owner's controller photo or a manual when the INP-01 table is authored.

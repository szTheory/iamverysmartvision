# Phase 8 discuss, decision point S-2: canonical state serializer, snapshot container, hidden-state checklist and hard-point tests (STA-01, STA-02; success criteria 2 and 3)

Research agent B. Read-only on the repo apart from this file. Nothing was built or run; every "tested" word below is a design intent, not a result.
Labels: [OBS] read in the tree at branch `gsd/phase-8-stic-completion-save-state-performance` (HEAD 58eab8e + records), [INF] inference, [REC] recommendation, [UNK] unknown or to verify at planning, [BG] background knowledge.

## 1. Recommendation on one screen

The instance is already the whole resumable state, and the digest field table already covers every byte of it once. So the cheapest correct design is: **one field table, one walker, two outputs** (a hash sink and a snapshot sink), with a per-row class that says what a snapshot does with the row.

- S2-1 [REC] Reuse `ivs_digest_fields[]` and `ivs_digest_domain()` (machine_digest.c) as THE serializer. Add one per-row `snap` class in the unused `uint16_t reserved` of `ivs_digest_field` (no struct growth): STATE (serialize), IMAGE_BEARING (`mem.exec/grom/cart`, never serialized), BOUND (image slot rows, `mem.exec_loaded/grom_loaded/cart_count/cart_seg`, `mem.profile`, `profile`: not serialized, checked against the live instance), PAD/MAGIC (never serialized, zeroed on restore). A behaviour-neutral refactor first (digest literals must not move), as plan 06-03 did for the ABI.
- S2-2 [REC] Wire order is **big-endian**, not little-endian: it is the project's single wire order (IVSC v1, the digest rows, the video slot header; `docs/formats/ivsc-v1.md` "Conventions"). The snapshot payload bytes are byte-for-byte the digest row encoding, so "the same serializer for snapshots and digests" is literally true. (The brief suggested little-endian; that would fork the project's one byte order and the digest code.)
- S2-3 [REC] Container "IVSN v1" (new, project-owned): 8-byte magic with a high-bit first byte, u16 format version, u16 flags (0), u32 total_bytes, u32 schema version, 32-byte **schema digest** (SHA-256 over the field table's name/width/count/domain/kind/class, so a state field added without a deliberate bump is refused and the pinned literal in a test breaks), profile id string plus a **profile parameter digest**, the three image slot identities (loaded, SHA-256), a section table for domains 1, 2, 4 and 16..19 (offset, length, full-domain SHA-256 as `ivs_machine_state_digest` gives it), the composite digest, then the sections, then a 32-byte SHA-256 trailer over everything before it (the IVSC trailer rule). About 4.3 KB [INF, to derive on paper in the oracle plan]; the core never allocates.
- S2-4 [REC] Four public calls in a new header `ivs/ivs_state.h`: `ivs_machine_snapshot_size`, `ivs_machine_snapshot_save` (caller buffer, bounded, writes nothing on refusal), `ivs_machine_snapshot_restore` (validates ALL, then commits with non-failing copies; whole-storage memcmp unchanged on refusal) and a stateless `ivs_snapshot_describe` (bytes to report, no instance; lets a host learn the profile and image digests to load before restoring, and gives the fuzzer an instance-free target). One report struct with the `struct_size` rule, reasons appended to the `IVS_REASON_*` list from 58.
- S2-5 [REC] Status codes: reuse BAD_ARG, BAD_STATE, BAD_HANDLE, BUFFER_TOO_SMALL, UNSUPPORTED (version, flags, schema), MALFORMED, OVERSIZED, DIGEST_MISMATCH (trailer or image digest), UNKNOWN_PROFILE, MISSING_SYSTEM_IMAGE. Add exactly one status, `IVS_E_STATE_MISMATCH (-24)` (well-formed snapshot that does not belong to this instance: profile parameter digest or a slot differs), inside the reserved -24..-31 content range; the reason in the report says which.
- S2-6 [REC] Restore into any valid instance of the same profile whose three slots equal the snapshot's identities (an OFF instance with images loaded is the normal fresh-consumer case). Restore never touches slots or image arrays. It sets phase POWERED from the payload. Snapshot is refused (BAD_STATE) while OFF. Snapshot points are unit boundaries only; that is exactly the set of points `ivs_machine_run` can stop at, and the CPU keeps no state inside a unit [OBS], so nothing "mid-instruction" exists to be lost.
- S2-7 [REC] Validation before mutation reuses the validators the code already advertises for this purpose (`ivs_cp1610_state_validate`, `ivs_stic_state_validate`: "a Phase 8 snapshot reload refuses a state this function does not accept") on locally decoded copies (32 B and 528 B, no big stack frame), plus a short list of **provably-true derived invariants** (section 3.4). Every invariant needs a witness test that all reachable states satisfy it, so a wrong invariant fails the round-trip suite instead of refusing real snapshots in the field.
- S2-8 [REC] STA-02 tests compare against an uninterrupted run with **table-independent oracles**: whole-instance `memcmp` (restored into a poisoned target, not a fresh one), the delivered video/audio/log streams concatenated across the cut, and all nine digests. The hidden-state checklist is a machine-checked `data/expected/state-checklist.ids` (one line per field-table row: class, witness cut, test id) so a row cannot exist without a line and a witness. Negative controls are mostly **runtime row-omission sweeps built from `ivs_machine_digest_field()`** (restore, then put row r back to its poison bytes, require divergence at a named cut), which cost no fault rebuild; only about 9 compile-time named faults for the serializer's own logic.
- S2-9 [REC] Hard points (section 5) are found by a **witness scan** over the internal step seam (predicates on `cpu.dbd`, `cpu.busak`, `cpu.halt_pending`, `stic.event_index`/`row_latch`, `psg` counters) and pinned as machine ticks that a witness test re-proves, so a scenario literal cannot drift. Plus a designed "latch-vs-current" guest program (stores to a fetched BACKTAB row after its fetch) that defeats an implementation that recomputes `row_latch` instead of restoring it.
- S2-10 [REC] Host surface: `ivs-run --roundtrip-at-ticks <n>` (in-process test hook, same style as `--slice-ticks`; no file), existing scenarios replayed with it must reproduce their goldens; installed consumer gets a fifth replay pass (Lamp Tone: cut mid-frame, restore into a poisoned fresh machine, streams and digests equal). File persistence stays host policy: optional last plan adds exclusive-create `--snapshot-out`/`--snapshot-in` to the runner only, refused for `content ... user`. The core does no file IO and chooses no path, name, slot count or autosave.
- S2-11 [REC] Rights: the snapshot embeds no image array and the writer is tested for it (no 16-byte window of EXEC/GROM/cartridge content appears in the bytes). But a snapshot of a real-EXEC run still holds GRAM and system RAM that the EXEC filled (GROM-derived glyph data [INF]); treat it as derived rights-bearing material like `--video-raw`: ivs-run refuses snapshot output for `content user`, `.ivsn` is added to `tools/check/binary-ext.txt` and `.gitignore`, `docs/owner-images.md` gets a line.
- S2-12 [REC] Scope: about 7 plans, one new suite `state` (about 80 ids per preset), one new fault file (about 9 faults), about 8 pure controls. Estimated hosted addition 12 to 16 cpu-s, delta r about +0.3 to +0.4 (section 7). That is inside the 8.3 trigger but not inside the 7.8 budget from a 7.78 median, so a checkpoint line after the serializer wave, release-only row sweeps and an explicit lever list are part of the plan. Order: STIC state layout (VID-05) before the serializer's schema is pinned.

## 2. Evidence A: the current digest code, state layout and API conventions

### 2.1 What the digest serializes today [OBS: core/src/machine/machine_digest.c, machine.h]

- `struct ivs_machine` is one flat, pointer-free struct of **114,416 bytes** (`IVS_MACHINE_BYTES`), 16-aligned, no tail padding, guarded by `_Static_assert` on the size and on the offsets of `psg` (113,448), `stic` (113,520), `obs` (114,048), `slot` (114,248). Any layout change forces a review of the digest table.
- One table `ivs_digest_fields[]`: 74 literal rows plus three `IVS_DF_SLOT` macros of 7 rows each (about 95 rows). Each row: name, `offsetof`, element width (1/2/4/8), count, domain, kind (UINT, PROFILE, MAGIC, PAD). Fields are read natively element by element (`memcpy` of one scalar) and written **most significant byte first** by shifts; a row with count > 1 is preceded by `u32 count`; PROFILE rows write the id string (`u32 len || ASCII`) not the registry index. MAGIC and PAD rows are never serialized. A test proves every byte of the struct lies in exactly one row (header of tests/machine/machine_digest.c: "Coverage").
- Domains: 1 CPU, 2 MEM, 3 IMAGES, 4 INSTANCE (composite version 3 = SHA-256 over `"IVSSTATE" || be16 3 || (id, digest)x4`), and observation domains 16 VIDEO, 17 AUDIO, 18 INPUT, 19 PSGLOG (outside the composite). Domain preimage is `"IVSD" || u8 domain || be16 IVS_DIGEST_DOMAIN_VERSION(1) || rows`.
- The sink (`ivs_dsink`) is a 256-byte stage that feeds SHA-256 and, for the test seam `ivs_machine_digest_preimage(m, domain, buf, cap)`, a caller buffer. No function pointer, no allocation. `ivs_machine_digest_field_count/field(i)` already expose the table to tests.
- Existing named faults on this code: DIGEST_SKIPS_RAM, DIGEST_SKIPS_IMAGE_TABLE, DIGEST_LITTLE_ENDIAN, DIGEST_INCLUDES_PADDING, DIGEST_DOMAIN_SWAPPED, DIGEST_PSG_ROWS_OMITTED, DIGEST_STIC_ROWS_OMITTED, DIGEST_COMPOSITE_VERSION_STALE, DOMAIN_ID_DROPPED [OBS]. Literal pins: `data/expected/digest-v1-literals.ids`, `digest-v2-literals.ids`, hand-framed streams in machine_digest.c tests.
- [INF] Size split: images are 110,592 B of the 114,416 (`exec` 8,192 + `grom` 4,096 + `cart` 98,304), so the non-image state is about 3,824 B raw. A snapshot is about 3.7 KB payload plus about 0.6 KB header and 32 B trailer.

### 2.2 Why "instance = whole state" holds [OBS]

- `ivs_machine_run` header: "The call keeps no state between calls (no budget, deficit or pointer): the whole instance is the resumable state, and any slicing of one target into several calls gives the same machine." tests/run/run_slice.c proves it against a seam-stepped twin including whole-storage memcmp.
- Run ends only at a **unit boundary** (`ivs_cp1610_step` executes exactly one unit: instruction, interrupt entry, reset entry, grant/hold/release, halted idle, fault, not-reset). The CPU struct is 32 B with no pointers; lines (INTR, INTRM, BUSRQ, STPST) are inputs of one step and "nothing is latched" (cp1610.c comments); the STIC lines are a pure function of `(stic state, tick)` sampled `line_sample_lag_ticks` before unit start (`ivs_stic_lines`).
- The PSG and the STIC are caught up to the unit end at every unit end in the step seam (fixed order PSG then STIC), "never to a call target" [OBS machine_run.c], so no lazily-pending device work crosses a boundary.
- No hidden globals: the only file-scope statics in the core are inside `#ifdef IVS_FAULT_*` (STATIC_CARRY, INPUT_STATIC_CARRY, PSG_STATIC_CARRY, RUN_BUDGET_CARRY) [OBS, all four read]. `c-purity` and the core-symbols check exist for this [OBS tools/check/core-symbols.sh].
- Existing validators meant for this phase: `ivs_cp1610_state_validate` (also run at the start of every step, so a restored bad CPU would be refused one unit late if not caught earlier) and `ivs_stic_state_validate` ("a Phase 8 snapshot reload refuses a state this function does not accept") [OBS]. There is no PSG validator [OBS psg_state.c has only reset].
- Caller-owned, outside the instance: video/audio/psg_log buffers (cursors live in the obs rows: `obs.video.count`, `obs.audio.emitted`, `obs.psglog.count`, which feed `video_first_index`, `audio_first_index`, `psg_log_first_index`), the input event array (host resubmits the suffix; applied count and floor are in `obs.input`).

### 2.3 Public API and ABI conventions the new calls must follow [OBS: ivs_machine.h, ivs_run.h, ivs_status.h]

- Struct rule: every struct starts with `uint32_t struct_size`; in structs: below v1 size or above `IVS_STRUCT_SIZE_MAX` (4096) is BAD_ARG, larger accepted only if the unknown tail is zero (else UNSUPPORTED); out structs: below v1 is BAD_ARG with nothing written, else write `min(caller, own)` bytes and set `struct_size` to the count. Growth is append-only with `*_V1_SIZE` constants pinned (IVS_INFO_V1_SIZE 216, IVS_RUN_REPORT_V1_SIZE 112, IVS_RUN_REQUEST_V1_SIZE 64).
- Out parameters must not overlap the instance (`ivs_machine_out_overlaps`, `ivs_machine_span_overlaps`: uintptr differences only); with a NULL or invalid handle the call is BAD_HANDLE wherever the out lies; BAD_ARG classes are checked before BAD_HANDLE for the call-specific argument errors; a refusal writes only its out parameters.
- No allocator, clock, path, callback; multiple instances are independent, one instance must be serialized by the host.
- Statuses frozen and append-only: 0, -1..-5, -16..-23 used; -6..-15 unassigned; -24..-31 reserved for content codes; pinned in `data/expected/api-statuses.ids`. Reasons: `IVS_REASON_*` 0..57 pinned in `data/expected/api-reasons.ids`; "numbers are append-only".
- Every public function needs a row in `data/api/api-effects.json` (precedence, one row per refusal status with state effect and the test ids that assert it), checked by `tools/check/api-effects.cmake` against the header, `api-statuses.ids` and the test inventories; the installed consumer's undefined-symbol list is pinned in `data/expected/pkg-consumer-calls.ids`.
- Bounded execution: calls are O(table) not O(guest). The save/restore cost is a fixed 3.7 KB walk plus SHA-256 over about 4.3 KB (plus domain digests, which hash the 110 KB of image rows once per save; about a few hundred microseconds [INF]).

## 3. Evidence B: the written hidden-state checklist (derived by reading the structs)

Classes: ARCH = architectural state, serialized, restored byte for byte. DER = derived from other state but stored; serialized, and the derivation is re-checked on restore (so neither a lost field nor a recomputed-instead-of-restored field can hide). BIND = identity of loaded content; not serialized, compared with the live instance. IMG = image-derived backing array, read-only after load, never serialized (rights). PAD/ID = never serialized, restore zeroes pads. HOST = outside the core by design.
"Witness" names a cut (section 5) at which the field is non-default and affects the continuation or an observable (digest, report, stream). "Control" is the negative control: `ROW(r)` = the runtime row-omission sweep (restore, then put row r back to its poison bytes, require divergence from the uninterrupted run at the witness cut within the continuation window); `FAULT:x` = named compile-time fault.

### 3.1 CPU (domain 1; `ivs_cp1610`, 32 B) [OBS]

| Item | Class | Why it can hide | Witness cut | Control |
|------|-------|-----------------|-------------|---------|
| `r[0..7]` (R6 SP, R7 PC) | ARCH | PC/SP trivially change the run | any | ROW |
| `status` flags C OV Z S (bits 4..7) | ARCH | only a branch or GSWD reads them | cut between a flag-setting op and its BEQ/BC | ROW (flags differ from a zero default) |
| `intff` | ARCH | only matters when INTRM arrives at an open boundary | cut after EIS, before the next frame's INTRM | ROW |
| `dbd` (SDBD prefix pending) | ARCH | the next instruction decodes differently (extra decle, +2 cycles, UNSUPPORTED target fault) | **HP-SDBD**: after the SDBD unit | ROW; FAULT |
| `prev_interruptible` | ARCH | decides whether the next boundary tests lines (grant, interrupt) or fetches | **HP-OPEN**: after an interruptible instruction with INTRM/BUSRQ due; **HP-CLOSED**: after MVO/entry | ROW |
| `run_state` (UNRESET, RESET_PENDING, RUNNING, HALTED, FAULTED) | ARCH | RESET_PENDING means "take the reset entry next" | **HP-RESETPEND** (right after power-on reset, before any unit), **HP-HALTED**, **HP-FAULTED** | ROW |
| `halt_pending` (HLT/STPST latch) | ARCH | HLT sets it, the stop happens at the next interruptible boundary | **HP-HLT**: after the HLT unit | ROW |
| `fault_reason` | ARCH | sticky; every later call stops GUEST_FAULT at once | HP-FAULTED | ROW |
| `busak` | ARCH | a held grant is a unit of its own; release needs BUSRQ low | **HP-STEAL** (section 5) | ROW |
| `reserved[8]` | PAD | stale garbage would break whole-instance memcmp | poisoned target | restore zeroes; test memcmp |
| Absent by design: cycle counter, pending-instruction microstate, latched lines | n/a | [OBS] CPU keeps no tick counter; a unit is atomic; "nothing is latched" | n/a | witness test that `ivs_cp1610` has no other member (`_Static_assert(sizeof == 32)` already) |

### 3.2 Machine, memory, images [OBS: machine.h, memsys.h]

| Item | Class | Notes | Witness / control |
|------|-------|-------|-------------------|
| `magic`, `profile` index | ID | handle validity; profile travels as id string + parameter digest in the header | refusal rows |
| `phase` | ARCH | must be POWERED in a snapshot; restore sets POWERED | validator |
| `reserved` | PAD | | memcmp |
| `time_ticks` | ARCH | machine time in master ticks; all device cursors are checked against it | ROW at every cut |
| `mem.gate_open[4]` | DER | the STIC register gate and graphics gate follow the timeline cursor (`FRAME_START` opens both, `REG_CLOSE`, `GMEM_CLOSE` close); index 0 always 1, index 3 always 0 | serialized; validator recomputes from `(frame_index, event_index)` and the table; cuts inside the closed windows |
| `mem.sysram[512]` (352-word extent), `mem.scratchpad[256]`, `mem.gram[512]` | ARCH | full backing is serialized (the digest does too), not only the extent | cut after the guest stored a pattern; GRAM written by guest and read by the renderer |
| `mem.psg_reg[16]` | ARCH | masked stores; guest-visible | PSG cut |
| `mem.port_pulldown[2]` | DER | equals `derive(obs.input.words)` ("the invariant port_pulldown == derive(words) holds after every call" [OBS machine.h]) | key held across the cut; validator calls a pure `ivs_input_derive` |
| `mem.psg_reserved[2]` | PAD | | memcmp |
| `mem.stic_reg[64]` | ARCH | the stored register words, including collision registers (class COLLISION) and the MOB, border, delay registers VID-05 will exercise; read masks apply on the CPU view, not here | cuts with MOBs/collisions set (after VID-05) |
| `mem.exec`, `mem.grom`, `mem.cart` | IMG | read-only after load (ROM devices drop stores [INF: store policy `keep` per device, memsys.c; verify per profile at planning]); 110,592 B; **never serialized** | rights test: no image bytes in the snapshot |
| `mem.exec_loaded`, `grom_loaded`, `cart_count`, `cart_seg[16]`, `slot[3]` records | BIND | the committed cartridge segment table and slot records are the only mutable decode state, but only a load changes them; compared via domain-3 digest and the header slot identities | slot-mismatch refusals |
| `mem.profile` | ID | PROFILE kind | |

### 3.3 PSG and STIC device state, observation rows [OBS: psg.h, stic.h, machine.h]

| Item | Class | Why it can hide | Witness cut | Control |
|------|-------|-----------------|-------------|---------|
| `psg.next_sample` (u64) | DER (equals `time_ticks / sample_period_ticks + 1` after a unit end) | a restored value off by one shifts every later sample stamp | any | validator + ROW |
| `psg.tone_counter[3]`, `tone_out[3]` | ARCH | mid-period phase is not derivable once periods were rewritten by the guest | **HP-PSG-MID**: periods changed twice before the cut, cut mid-period | ROW |
| `psg.noise_counter`, `noise_lfsr` | ARCH | lfsr history | after many noise steps | ROW |
| `psg.env_counter`, `env_level`, `env_dir`, `env_held`, `env_cycles` | ARCH | envelope mid-cycle, `env_held` set at reset until first shape write | **HP-ENV-MID** (shape with decay, cut mid-envelope) and held-at-reset cut | ROW |
| `psg.reserved[3]` | PAD | | memcmp |
| `stic.frame_index` (u64), `event_index` | ARCH | the timeline cursor is state "never derived from time by modulo" (D-07) | **HP-FRAME-MID** (section 5) | ROW; existing fault STIC_FRAME_BY_MODULO shows the hazard |
| `stic.display_latch`, `windows_active` | ARCH | latch decided inside the register window, windows decided at `GMEM_CLOSE` | cut between register window and first window | ROW |
| `stic.mode`, `cs_ptr` | ARCH (cs_ptr observable only through the STIC report and the digest, because each render restarts at 0) | | after a Color Stack frame | ROW (digest/report divergence, not pixels) |
| `stic.blocked_reads`, `blocked_writes`, `late_grants`, `grants_in_frame` | ARCH (saturating counters) | `grants_in_frame` feeds the FRAME_END late-grant count | **HP-STEAL** mid frame | ROW |
| `stic.reserved0` | PAD | | |
| `stic.row_latch[12][20]` | ARCH | latched BACKTAB copy: a guest store after a row's fetch must NOT change that frame's row; recomputing it from BACKTAB at restore is wrong | **HP-LATCH** (section 5.2) | ROW + the designed program |
| Phase 8 growth (VID-05 MOB/collision/delay hidden state, if any) | ARCH | [UNK] whether VID-05 needs state beyond `stic_reg` and `ivs_stic_state` | wait for the VID-05 layout, then add rows | the `_Static_assert(sizeof)` and "every byte in exactly one row" tests force a decision |
| `obs.video.chain/count` | ARCH (history; not derivable) | the chain is the digest of every delivered slot | cut after frame k, check the slot k+1 chain | ROW; `video.count == stic.frame_index` is a candidate derived check [INF, verify] |
| `obs.audio.chain/count/emitted/prev_levels` (+ reserved) | ARCH (`emitted == psg.next_sample - 1` DER) | the chain continues across the cut and the runner's own recomputation must still verify | audio change after the cut | ROW |
| `obs.input.words[4]`, `count`, `floor_tick`, `chain` | ARCH | floor rejects events below it; count tells the host where its schedule resumes | held key + events on both sides of the cut | ROW |
| `obs.psglog.chain/count` | ARCH | | writes on both sides | ROW |

### 3.4 Derived invariants checked on restore (all must be provably true of reachable states; each needs a witness "reachable states satisfy it" test)

1. `cpu` passes `ivs_cp1610_state_validate`; `stic` passes `ivs_stic_state_validate` and additionally `event_index < p->event_count`.
2. STIC cursor consistency: the next event tick `phase + frame_index*F + event_tick[event_index]` is greater than `time_ticks` (all events at or below the unit end were applied), and the event before it is at or below it (or it is the first event of frame 0). `gate_open` equals the replay of the table up to `event_index`.
3. PSG: `next_sample == time_ticks / sample_period + 1`; `obs.audio.emitted == next_sample - 1`; `tone_out`, `env_dir`, `env_held` in {0,1}; `env_level <= 15`, `env_cycles <= 15`; `prev_levels` bytes <= 15; `noise_lfsr` within the register width; stored PSG registers equal themselves under the write mask.
4. Input: words at index >= `input.controllers` are 0, bit 31 clear; `port_pulldown == derive(words)`; `floor_tick <= time_ticks`.
5. `phase == POWERED`, `time_ticks <= 2^63`, reserved/pad words absent from the stream (a canonical stream has no pad bytes at all).
6. Candidate (verify at planning): `obs.video.count == stic.frame_index` (frames chained equal frames completed on the machine path); `obs.psglog.count` and `obs.input.count` have no derivable relation and are free.
- Counters, chains, RAM words and `row_latch` words are free values; the check there is only the trailer checksum.

## 4. Evidence C: the container design ("IVSN v1")

### 4.1 Layout (all integers big-endian; fixed positions; no padding anywhere; names are working names)

| Offset | Size | Field | Rule |
|--------|------|-------|------|
| 0 | 8 | magic | `89 49 56 53 4E 0D 0A 1A` ("IVSN"; "IVSS" is the scenario format, so not reused). High-bit first byte, CRLF and SUB catch 7-bit and text-mode transfers (IVSC rationale) |
| 8 | 2 | format version | 1 |
| 10 | 2 | flags | 0 (any bit refused) |
| 12 | 4 | total_bytes | whole container incl. trailer; exact, must equal the size this build derives |
| 16 | 4 | schema version | = `IVS_SNAPSHOT_SCHEMA_VERSION`; bumped with the composite digest version |
| 20 | 4 | section count | 7 (domains 1, 2, 4, 16, 17, 18, 19) |
| 24 | 32 | schema digest | SHA-256 over table descriptors (name, width, count, domain, kind, class) in table order; never offsets (host layout) |
| 56 | 64 | profile id | NUL-padded, <= 63 bytes (`IVS_PROFILE_ID_MAX_BYTES`); rest zero |
| 120 | 32 | profile parameter digest | see 4.3 |
| 152 | 3 x 40 | image slots | per slot: u32 loaded, 32-byte SHA-256 of the container bytes supplied to the load (`ivs_image_report.sha256`), u32 length_bytes (diagnostic only) |
| 272 | 32 | composite digest | `ivs_machine_state_digest(COMPOSITE)` at save time |
| 304 | 7 x 44 | section table | domain u8, 3 zero bytes, u32 offset, u32 length, 32-byte full-domain digest |
| 612 | n | sections | each section = the domain's preimage with rows of class != STATE skipped: `"IVSD" || domain || be16 domain_version || rows` in table order |
| total-32 | 32 | trailer | SHA-256 of every byte before it |

Domain 3 (IMAGES) has no section: it is entirely BIND. Total about 612 + 3.7 KB + 32 [INF; derive exactly in the oracle plan and pin].
Why not a flat `memcpy` of the struct: layout, padding and host byte order would leak (the digest design rejects exactly this, D-35 [OBS]); the instance also holds 110 KB of image arrays.
Why per-section domain versions: a domain's row set can change without the others (composite versions 1 to 3 already did); the snapshot then refuses by schema digest and says which section.

### 4.2 Sink and encoding (copy-paste, no library)

- Add to `ivs_dsink` a mode (`HASH`, `BUFFER`, `BOTH`) and a row filter `want_class`; the existing `ivs_digest_domain()` loop gets one `if (k->only_state && f->snap != STATE) continue;`. The digest path passes `only_state = 0`: bytes and hashes unchanged (the digest literals are the tripwire).
- Decode is the mirror: for each STATE row, read `count` (if > 1, and check it equals the table's count: a snapshot never changes a row's shape) then elements as BE shifts into a local struct or straight into the instance. All reads are bounds-checked against the section length first (length-lies refused before any read of the byte).
- Decode into locals for the two device structs that have validators (CPU 32 B, STIC 528 B, PSG state 72 B) and validate there; arrays (sysram, gram, row_latch) are validated by range only and copied in the commit phase.

### 4.3 Profile identity

- [OBS] `ivs_profile_desc.id` is the only identity today, and the profile file has changed behaviour within one id across phases (profile fields were added in Phases 5 and 6, D8 is "reversible by one profile field"), so the id alone does not bind parameters.
- [REC] Add to the generated registry a **profile parameter digest** per profile: the cmake generator (`cmake/IvsMemmapTable.cmake`, `IvsPsgTable.cmake`, `IvsSticTable.cmake`, `IvsCtrlTable.cmake`) already emits the canonical C tables; hash that emitted text (`string(SHA256 ...)`) over the table bodies, not over provenance comments or the JSON file, so tag/cite edits do not invalidate snapshots and checkout line endings do not matter. [UNK] whether the four generators' output is byte-stable across platforms (it is cmake string output, so LF; confirm at planning). Cost: one generated constant per profile and one `.inc` consumer; no runtime hashing of tables.
- Snapshots are **exact-build state, not an archival format** (pre-1.0, no migration): a code change that alters behaviour but not parameters yields a state-compatible snapshot whose future may differ; the snapshot only claims equivalence for the same build and profile digest [INF]. Say so in `docs/formats/ivsn-v1.md`.

### 4.4 Check order (first failing class wins; same philosophy as IVSC "Check order")

Save: BAD_ARG (buf NULL with cap>0, out_written NULL, out or buf overlapping the instance or each other) -> BAD_HANDLE -> BAD_STATE (OFF) -> BUFFER_TOO_SMALL (cap below the exact size; nothing written) -> write.
Restore (nothing is read beyond `len`, nothing in the instance is written until step 10):
1. BAD_ARG: NULL buf with len>0, rep struct_size rules, buffer overlapping the instance (the commit would corrupt its own input) or the report; BAD_HANDLE; BAD_STATE is **not** raised for POWERED targets (rewind is legal).
2. Length: `len > IVS_SNAPSHOT_MAX_BYTES` -> OVERSIZED (before any byte is read); `len` below the smallest possible -> MALFORMED (TRUNCATED).
3. Magic (first differing byte reported), then format version (UNSUPPORTED), then flags (UNSUPPORTED), schema version and schema digest (UNSUPPORTED: "another schema"), `total_bytes` against `len` (lies above: MALFORMED TRUNCATED; below: MALFORMED TRAILING) and against the derived size (SIZE_MISMATCH), section count.
4. Section table: known domains only, strictly ascending, offsets/lengths inside the payload, adjacent, no gaps or overlaps, sum equals payload (all MALFORMED with section index).
5. Trailer SHA-256 over all bytes before it (DIGEST_MISMATCH). Placed after structure, before anything that interprets content, like IVSC step 7; version and schema are checked before it because a future version may change the trailer.
6. Profile: id registered (UNKNOWN_PROFILE) and equals the instance's (STATE_MISMATCH reason PROFILE); parameter digest equals the build's (STATE_MISMATCH reason PROFILE_PARAMS).
7. Images: for each slot, loaded flag and SHA-256 equal the live slot (a snapshot slot loaded but the instance empty: MISSING_SYSTEM_IMAGE; both loaded, digest differs or loaded only on the other side: STATE_MISMATCH with the kind in the report), then the live domain-3 digest equals the recorded one (covers `cart_seg` structure).
8. Section content: per-row `count` equals the table's; decode device structs into locals; run the validators and the derived invariants of 3.4 (MALFORMED, reason FIELD_RANGE or INVARIANT with domain and row index).
9. Canonical-form checks: pad/reserved never present (they are not in the stream), no trailing bytes inside a section.
10. Commit: `memset` the non-image, non-slot region of the instance to zero (so pads and unreachable bytes are canonical), then store every decoded value with non-failing copies; set `magic`/`profile`/slots untouched. Nothing after step 9 can fail. [INF] One `memset` plus table-driven stores keeps "restored instance bytes depend only on the snapshot and the loaded images".
- Test seam for atomicity: a refused restore leaves the whole instance byte-identical (whole-storage memcmp, the project's refusal oracle SP4 [OBS api-effects "none" effect]); the named fault RESTORE_MUTATES_BEFORE_VALIDATE (commit CPU, then validate STIC) proves the oracle can fail.

### 4.5 Forward and backward compatibility policy

- Format version bumps only for container layout changes; schema version/digest for state-field changes. Restore accepts exactly one (format, schema) pair per build. No migration code before 1.0 (every earlier phase bumped the composite digest version deliberately, A-03/D-16, and pinned old literals as history).
- Unknown future version: UNSUPPORTED with the version in the report; never a best-effort load.
- `ivs_snapshot_describe` works for any version it can parse up to the version field, so a host can say "made by a newer core".
- A pinned **literal vector**: header bytes hand-written independently of the code and the SHA-256 of a stepped project-authored boot stub run, labelled "repeatability only" like `digest-v2-literals.ids`; the schema digest literal is pinned so a field change cannot slip past review.

### 4.6 API sketch (public, `ivs/ivs_state.h`, included from `ivs/ivs.h`)

```c
#define IVS_SNAPSHOT_FORMAT_VERSION 1u
#define IVS_SNAPSHOT_MAX_BYTES 16384u      /* length bound refused before any read */
#define IVS_SNAPSHOT_REPORT_V1_SIZE <pinned>u
typedef struct ivs_snapshot_report {       /* out */
  uint32_t struct_size, reason, format_version, schema_version;
  uint64_t total_bytes, byte_offset;       /* where the defect was seen */
  uint32_t section_index, kind;            /* IVS_SECTION_NONE / IVS_SEGMENT_NONE style sentinels */
  char     profile_id[IVS_PROFILE_ID_MAX_BYTES];
  uint8_t  profile_params_sha256[32], composite_sha256[32];
  ivs_image_report slots[3];               /* loaded, sha256; reused type */
} ivs_snapshot_report;
IVS_API ivs_status ivs_machine_snapshot_size(const ivs_machine *m, uint64_t *out_bytes);
IVS_API ivs_status ivs_machine_snapshot_save(const ivs_machine *m, uint8_t *buf, uint64_t cap, uint64_t *out_written);
IVS_API ivs_status ivs_machine_snapshot_restore(ivs_machine *m, const uint8_t *buf, uint64_t len, ivs_snapshot_report *rep);
IVS_API ivs_status ivs_snapshot_describe(const uint8_t *buf, uint64_t len, ivs_snapshot_report *rep);
```
- 64-bit lengths like the run call (`uint64_t` capacities), `uintptr_t` overlap arithmetic, `rep` may be NULL on restore (as the load report). Reasons appended from 58: BAD magic/version/flags/schema/length forms, TRAILER, SECTION_TABLE, PROFILE, PROFILE_PARAMS, SLOT_KIND, FIELD_RANGE, INVARIANT, ARG_* forms. Exact list pinned in `data/expected/api-reasons.ids` by the plan.
- Size is constant per (profile, build) [INF]; the query exists so a host never hard-codes it and so `BUFFER_TOO_SMALL` has a number to report.

## 5. Evidence D: hard-point tests

### 5.1 Reaching the points deterministically [OBS: step seam, run API, existing test machinery]

- The internal step seam `ivs_machine_step_unit_internal` and `ivs_machine_run(now + 1)` ("runs exactly one unit", proven equal by `run.single-step.equals-seam`) give unit-by-unit control. Tests in `ivs_core_internal` can read `struct ivs_machine` directly; the public `ivs_cpu_state_report` has no `dbd`/`busak`/`halt_pending` (36 bytes: regs, flags, intff, run state, fault), so a **public-API-only** consumer cannot locate HP-SDBD or HP-STEAL by inspection alone. [REC] do not grow the public report; instead (a) a core witness test finds each point with the seam and records `(program id, tick)`, (b) the runner/consumer cut at those pinned ticks, and (c) the witness test is the independent proof that the pinned tick is the hard point (a literal that drifts fails there, not silently).
- Programs: hand-assembled decle literals in test sources (precedent: run_slice.c `rs_prog_*`, stic_progs.c) or `ivs_asm` fixtures (tools/asm, Phase 7) for the two diagnostic-style programs below; the Lamp Tone original program (`data/scenarios/demo-lamp-tone-*`) already runs video, audio, input and 30 PSG writes across 13 frames and is the consumer workload.
- Cheap: a frame is 59,736 master ticks [OBS consumer comment]; reaching frame 3 is a few thousand units.

### 5.2 The cut set (each cut = a witness predicate on the instance after a unit, plus a pinned tick)

| Cut | Predicate after the unit [OBS fields] | What it proves |
|-----|----------------------------------|----------------|
| HP-SDBD | `cpu.dbd == 1` (the SDBD unit just ran; next unit is the extended instruction), program `SDBD; MVII #imm, Rn` with a bus access that distinguishes 1 vs 2 decles | `dbd` restored; and a second cut variant at `dbd == 1 && stic.windows_active == 1` (SDBD closes the boundary: no grant may start) |
| HP-STEAL-A | `cpu.busak == 1` first grant unit of a window (`unit_kind == BUS_GRANT`) | grant held across the cut; release then happens on the restored side with `release_latency` cycles |
| HP-STEAL-B | `cpu.busak == 1` and `stic.grants_in_frame >= 1` mid-hold | `grants_in_frame`, `late_grants` accounting across the cut |
| HP-STEAL-C | the unit after a release (`events & BUS_RELEASED` just happened, `prev_interruptible` still 1) | "step after the release runs the interrupt test, not a fetch" (POL-BUS-01) with `prev_interruptible` restored |
| HP-OPEN / HP-CLOSED | `prev_interruptible` 1/0 at INTRM time (frame start, `irq_low_ticks` window) with `intff` 1 | interrupt acceptance vs fetch after restore |
| HP-INT-ENTRY | `unit_kind == INT_ENTRY` just ran (R6 pushed, `prev_interruptible` 0) | interrupt entry unit not replayed or skipped |
| HP-HLT | `cpu.halt_pending == 1` with `run_state == RUNNING` (HLT unit just ran) | stop latch; next instruction still runs (C56) |
| HP-HALTED | `run_state == HALTED` with a grant window due (`stic_busrq.halted-grants` exists) | grants while halted survive |
| HP-FAULTED | `run_state == FAULTED` | sticky fault, later call stops GUEST_FAULT immediately |
| HP-RESETPEND | immediately after `ivs_machine_reset`, before any unit (also "restore then run" from a fresh reset state) | RESET_PENDING and IAB reset vector path |
| HP-FRAME-MID | `stic.frame_index >= 1`, `display_latch == 1`, `windows_active == 1`, `event_index` after ROW_FETCH 5 and before FRAME_END (the 16-entry table, `stic_events.c`) | cursor, latch, half-fetched `row_latch`, closed gates, frame completes after restore with the same slot bytes |
| HP-LATCH | as HP-FRAME-MID, plus the guest rewrites BACKTAB word of an already fetched row after the fetch and before FRAME_END (stores stamped before `ROW_FETCH` are seen, after are not, A-06) | rendered pixels use the saved latch, not current BACKTAB: kills "recompute `row_latch` on restore" |
| HP-CS | after a Color Stack frame (`cs_ptr` nonzero), cut mid next frame | `cs_ptr` and mode (digest/report only) |
| HP-PSG-MID | tone periods rewritten twice, cut with `tone_counter` nonzero and `tone_out` mixed; noise advanced | counters not derivable from time |
| HP-ENV-MID | envelope shape with decay, `env_cycles` mid, `env_held == 0`; plus the `env_held == 1` post-reset cut | envelope generator |
| HP-INPUT | a controller word held (`port_pulldown != 0`), one event applied, one pending in the host array; `floor_tick > 0` | pulldown/words/floor; host resubmits suffix from `obs.input.count` |
| HP-LOG | cut after a PSG write whose stamp is inside the last unit (`obs.psglog.count > 0`) | log chain continues; `psg_log_first_index` equals pre-cut count |
| HP-MOB (after VID-05) | MOBs, collisions, delay registers set, cut mid-frame | new STIC state, whichever form VID-05 gives it |

### 5.3 The comparison (STA-02 oracle; independent of the shared table)

For each cut c and program P, with E the end tick (reference end, as run_slice.c fixes E from the program's own stop):
1. REF: fresh instance, run 0 -> E through `ivs_machine_run` with video, audio, log buffers sized from `ivs_info`; keep streams `S_ref` and final instance bytes `B_ref`, nine digests `D_ref`.
2. SUB: fresh instance A, run 0 -> c (seam-stepped to the predicate tick, or `run(c)` with the tick from the witness), `snapshot_save`. Poisoned target T: create, load the same images, then **fill every non-image, non-slot byte with 0xA5** using the field table (offset/width/count from `ivs_machine_digest_field`) and set `magic`, profile; restore into T. Check immediately: `memcmp(T, A) == 0` over the whole instance (equal pads too, because restore zeroes them) and all nine digests equal.
3. Continue T c -> E with fresh stream buffers resubmitting the input suffix; check: `S1 || S2 == S_ref` byte for byte for video, audio and log; `report.*_first_index` of leg 2 equal the leg-1 counts; `memcmp(T, B_ref) == 0`; digests equal `D_ref`; `ivs_machine_stic_state` report equal.
4. Also: `save(T)` immediately after restore equals the snapshot bytes (canonical form), and snapshot -> restore -> snapshot is a fixed point at every cut.
- Poisoned target is essential: a fresh target holds zeros, and every field that is 0 at the cut would pass whether restored or not (see 7.3).
- Equality of digests alone is not the oracle: digest and snapshot share one table, so a row missing from the table is missing from both. The independent oracles are the whole-instance byte compare, the streams, and the existing "every byte in exactly one row" coverage test.

### 5.4 Negative controls (named at the cut that must catch them)

- ROW sweep (runtime, no fault build): for each of the about 95 rows r (arrays count once, large arrays are also sampled at first/last element) and for each cut that the checklist lists as its witness: restore correctly, then copy r's poison bytes back, continue to E; the result must **differ** from REF in digest, report or stream. A row that does not diverge at any listed cut is a checklist defect: either its witness is weak or it is dead state; dead rows are listed in an explicit exemption list with a reason (e.g. unreachable backing tail beyond the 352-word extent), checked by the gate like `cp1610-flag-exempt.ids`. The cut each row must diverge at is named in `data/expected/state-checklist.ids`, so a failure message is "row stic.row_latch survived omission at HP-LATCH".
- FAULT list (compile-time, `data/expected/faults/state.ids`, one site each): SNAPSHOT_LITTLE_ENDIAN (literal vector), SNAPSHOT_PAD_SERIALIZED (canonical re-encode), SNAPSHOT_IMAGE_BYTES_EMBEDDED (rights window test), SNAPSHOT_STIC_ROWS_OMITTED (HP-FRAME-MID), RESTORE_MUTATES_BEFORE_VALIDATE (atomicity), RESTORE_TRAILER_UNCHECKED, RESTORE_PROFILE_UNCHECKED, RESTORE_IMAGE_DIGEST_UNCHECKED, RESTORE_LENGTH_TRUSTED (reads past `len`, ASan), RESTORE_OBS_NOT_RESTORED (chains continue from the target's). Existing DIGEST_*_ROWS_OMITTED faults already model "row group omitted"; if the walker is shared they may also fail state ids, so the state suite must not be in their allowed-prefix lists unless the verdict says otherwise [UNK, check faults-verdict semantics at planning].
- Hard-point sensitivity self-controls (pure, gate controls under tools/check/controls): checklist line missing for a table row; witness id not in the inventory; exemption without reason; schema digest literal stale.

## 6. Evidence E: corruption and fuzz plan

Same style as the IVSC family: a seeded deterministic family in the gate, plus an optional libFuzzer target outside the gate [OBS: `ivsc.fuzz.*`, `tests/fuzz/fuzz_ivsc.c` (IVS_ENABLE_LIBFUZZER, resealed trailer, canonical re-encode, poisoned out)].

### 6.1 Seeded family `state.fuzz.*` (xorshift with a fixed seed, no host time)

- Exhaustive and cheap (snapshot is about 4.3 KB): every truncation length 0..N (refused, defined status, instance memcmp equal); every **single-byte flip** (N cases, expect refusal: header fields by their own reason, payload by DIGEST_MISMATCH); every single-byte flip **with the trailer resealed** (reaches the validators: outcome is either a refusal with a defined (status, reason) pair or an acceptance whose `save()` re-encodes to exactly the flipped bytes); every single-bit flip of the header; `total_bytes` lies (0, 1, below, above, 0xFFFFFFFF, above the buffer); section table lies (offset/length wraparound, overlap, gap, unknown/duplicate domain, count 0/8/0xFFFF); version 0/2/0xFFFF; flags each bit; extra bytes after the trailer; buffer one byte short; the profile id with an embedded NUL, no NUL, 64 bytes, non-ASCII, unregistered (UNKNOWN_PROFILE); profile params digest flipped (STATE_MISMATCH); each slot digest flipped, loaded flag flipped, slot empty in the target (MISSING_SYSTEM_IMAGE).
- Semantic lies with a valid trailer: each derived invariant of 3.4 violated one at a time (next_sample off by one, `gate_open` against the cursor, pulldown not equal to derive(words), floor above time, `event_index` 16, `dbd` with `prev_interruptible` 1, FAULTED with reason 0, run_state 9).
- Invariants checked on every case [OBS pattern]: status is one of the documented set and matches the reason; a refusal leaves the **whole instance bytes** equal and writes only the report; a poisoned report is untouched where the call says so; an acceptance leaves state equal to the decode of the stream and `save` re-encodes canonically (so no payload byte is ignored: restore never silently drops a byte); two attempts give identical results.
- Same on the instance-free `ivs_snapshot_describe` (no instance, so it runs on a heap buffer of exactly the input size under ASan to catch over-reads).
- Different-handle cases: restore into a destroyed handle, NULL handle, an instance whose stored profile index is damaged (BAD_HANDLE), buffer overlapping the instance (BAD_ARG, nothing read first), report overlapping the instance.
- libFuzzer target `tests/fuzz/fuzz_snapshot.c` (not in the gate, same recipe as fuzz_ivsc: Homebrew LLVM or Linux clang; Apple clang lacks the runtime [OBS CLAUDE.md]): reseal, restore into a live instance, require the same invariants, abort on violation.
- Cost: with 4.3 KB inputs the three sweeps are about 4,300 + 4,300 + 4,300 restore attempts at about 20 to 100 microseconds each under ASan [INF]: about 1 to 2 cpu-s dev, about 0.2 release. Bounded; keep it in dev (sanitized) and sample the header/table cases only in release if the budget needs it.

## 7. Evidence F: scope, CI cost, rights, ABI risk and the adversarial pass

### 7.1 Plan breakdown (about 7 plans; the Phase 6 and 7 style: oracle first with no C, behaviour-neutral refactor before behaviour)

| # | Plan | Content | Gate delta |
|---|------|---------|-----------|
| S1 | Oracle, no C | `docs/state/hidden-state-checklist.md` + `data/expected/state-checklist.ids` (every row: class, witness cut, test id), `docs/formats/ivsn-v1.md` (layout, check order, reasons, size derivation on paper, hand-written header literal), cut-set definitions, rights note, forward policy; 3 pure controls (row without line, witness without test, exemption without reason) | controls +3 |
| S2 | Behaviour-neutral walker refactor | `snap` class per row, sink modes, pure `ivs_input_derive`, `psg` validate function, `ivs_stic_state_validate` extended with the event-count check; digest literals and all `machine.digest.*` unchanged; coverage test for classes; no public change | tests +6 |
| S3 | Serializer and `describe`/`size`/`save`, header, schema digest, profile parameter digest in the generators, literal vector | new header, api-effects rows, reasons/status append, `pkg-consumer-calls.ids` untouched until S7 | tests +20, faults +4, controls +2 |
| S4 | Restore | staged validation, commit, refusal matrix (about 45 refusal rows), atomicity oracle | tests +30, faults +6, controls +1 |
| S5 | Hard-point suite (STA-02) | witness scan, programs, 17 cuts, round-trip equivalence, ROW sweep, poisoned target, stream concatenation; exemption list | tests +20 (cuts are data-driven rows inside a few cases) |
| S6 | Corruption family and libFuzzer target | `state.fuzz.*`, `fuzz_snapshot.c` | tests +12 |
| S7 | Hosts and close | `--roundtrip-at-ticks`, runner-scenarios lines, installed consumer fifth pass, consumer golden lines, docs/runner.md, `.ivsn` binary-ext, claims CL (provisional, tier deterministic-repeatability, profile + build), traceability, CI checkpoint, optional file options | process cases +8 |

- Dependency: S2 should follow the VID-05 plan(s) that change `ivs_stic_state` or `stic_reg` use, otherwise the schema digest, the sizes and the literal vector are re-derived. S1 can run in parallel with VID-05 (rows for MOB/collision state are added when the layout lands). [REC]
- Fan-out with other decision points: S-1 (STIC completion) decides whether VID-05 adds device state; S-3 (benchmarks, PRF-01) can use `snapshot_restore` as a cheap "reset to cut" for steady-state workloads, but only if the digest-before-throughput rule in PRF-01 is kept (restore must be proven equal first).

### 7.2 CI cost against the arbiter `r = cpu_s / build_s` (budget 7.8, revisit 8.3) [OBS .planning/CI-EFFICIENCY.md]

- Last five hosted rows: median `r` 7.78 (7.53, 8.02, 8.17, 7.73, 7.78); newest `build_s` 60 is a slow-runner reading, previous 37 to 41; hosted cpu is about 1.675 x local cpu (Phase 7 factor).
- Unit costs measured by the project: per named fault about 0.5 hosted cpu-s (build 0.72 + run 0.27 container cpu-s x 0.51); per process case up to 0.7 hosted cpu-s at the stated upper bound (measured an order of magnitude lower in Phase 7); per pure control about 0.07 local cpu-s.
- Estimate [INF, upper-biased]: 9 to 10 faults x 0.5 = 5; about 6 hosted cpu-s for ids (80 ids x 2 presets, with the sweeps in 6.1 and 5.4 about 2 to 4 local cpu-s in dev and under 1 in release, times 1.675); 8 process cases x 0.1 to 0.7 = 1 to 5; 8 controls x 0.12 = 1. Total **about 12 to 16 hosted cpu-s**, divided by `build_s` 37 to 41 gives **delta r about +0.3 to +0.4**, projected median about 8.1 to 8.2: under 8.3, over 7.8. VID-05 fixtures and PRF-01 add on top of this and are other points' costs; the shared margin is thin.
- Mitigations designed in (not dropping evidence): (a) ROW sweep and the 17-cut matrix run in the release preset only (precedent: lever (2) "mutant inventories on the release preset only", with a visible skip line); the sanitized preset keeps the corruption family, atomicity and one smoke round-trip, because decode bugs need ASan; (b) cuts are rows in one case per family, not one process or one ctest id each; (c) at most 9 faults, none duplicated by a runtime sweep (the sweeps already cover row omission); (d) one `checkpointB-08` line after S4 using the D-34 rule, and the stop rule stands: median above 8.3 means a lever plan from the orchestrator before more tests; (e) levers if needed, in order: fold two faults into one table-driven fault, then the Phase 7 list (3) and the owner-level options.
- Not CI-significant: `ivs_snapshot_describe`/restore at a fixed 4 KB; the consumer pass reuses `build/release` and an existing replay (no new cold build), like Phase 4's rule.

### 7.3 Adversarial pass: what passes a round trip but is wrong

1. **Zero coincidence.** Restoring into a fresh instance passes for any field that is zero (or default) at the cut. Fix: restore into a poisoned target and require whole-instance memcmp (5.3). A second layer: the ROW sweep proves each row's witness cut has a nonzero, behaviour-relevant value.
2. **Recomputed instead of restored.** A restore that recomputes `row_latch` from BACKTAB, `next_sample` from time, `gate_open` from the cursor, `port_pulldown` from words, or `emitted` from `next_sample` passes tests whenever the derived value equals the stored one. The derived fields are exactly where it can differ (HP-LATCH differs by design; the others differ only if some path updates one without the other). Fix: serialize them, check the derivation as an invariant on restore AND at save (a save-side assert test that every reachable cut satisfies it), so a drift is a visible failure not a silent re-sync.
3. **Shared-table blind spot.** The digest and the snapshot use one row table; a row added to the struct and not to the table is invisible to both, so digest-before == digest-after passes. Independent oracles: whole-instance memcmp (table-free), the existing "every byte of `struct ivs_machine` in exactly one row" test, the `_Static_assert(sizeof == 114,416)` tripwire, and the schema-digest literal pin.
4. **Equal digests, unequal pads.** Digest ignores pads and magic; a restore that leaves stale pad bytes passes digest checks but breaks memcmp; a restore that serializes pads (DIGEST_INCLUDES_PADDING analog) breaks canonical re-encode. Both covered (5.3 step 4, fault SNAPSHOT_PAD_SERIALIZED).
5. **Restore equals save-time digest but not the future.** A snapshot taken between a grant and its release restores `busak` only if `cpu.busak` is serialized; the digest would still match for a state with the wrong `busak` only if the row were missing. Covered by HP-STEAL plus ROW(busak). Likewise `prev_interruptible` after a grant (A-02: a grant never changes it).
6. **Host-side hidden state.** The input event array and the output buffers are host state: a host that restores but resubmits events from index 0 sees BAD_ARG `INPUT_BEFORE_FLOOR` for events below `floor_tick` (good) or silently re-applies equal-tick events (below the floor are refused, at/above are re-applied). Document: resubmit from `obs.input.count` (cumulative) ; runner hook does it and the consumer pass proves it.
7. **Same-build bias.** Round-trips on one binary prove consistency, not accuracy or cross-build stability (AGENTS.md: repeated hashes establish repeatability only). The claim written for STA-01/02 is tier deterministic-repeatability, profile + build, and says nothing about hardware or about snapshots across versions. Cross-preset identity (dev vs release produce identical snapshot bytes; golden SHA-256 pinned) is tested; cross-endianness is by construction plus the literal vector and the LITTLE_ENDIAN fault, not by a big-endian host run [UNK: no BE CI host].
8. **Validators too strict.** A wrong derived invariant refuses legitimate snapshots. Fix: every invariant has a "reachable states satisfy it" test over all cuts and over the diagnostic and Lamp Tone programs (and over the existing seeded run fuzz, which can call the invariant checker at every unit [INF]).
9. **Saturation and wrap.** Counters saturate at 0xFFFFFFFF (`blocked_*`, `late_grants`); `time_ticks` wraps nowhere reachable. A hand-built snapshot with saturated counters must round-trip (canonical decode of u32 max), tested in the fuzz family.
10. **Restore changes the profile index or the slots.** It must not: whole-instance memcmp outside the restored region equals before; rights test: no image bytes in the output.
11. **Time/IO leakage.** Nothing in save/restore may read a clock or the file system; core-symbols deny list already forbids imports; the new TUs go through the same checker.

### 7.4 Rights and persistence policy [OBS AGENTS.md, AUTONOMY.md, runner docs]

- Snapshots must not embed image bytes (`mem.exec/grom/cart` are IMG; only SHA-256 identities and the load status are carried). Hash establishes identity, not rights; the existing reports already expose slot SHA-256 values.
- A snapshot of a run with owner EXEC/GROM contains GRAM and system RAM that those images wrote (glyph data copied out of GROM into GRAM is the likely case [INF]). Same handling as `--audio-raw` and `--frame-ppm` for `content ... user` (T-07-29, D-31): the runner refuses snapshot output for such scenarios; no repository path, golden or CI artifact ever holds one; `.ivsn` joins `binary-ext.txt` and `.gitignore` (the IVSC rule), and the hygiene/binscan steps then treat any tracked `.ivsn` as a rights problem. D11 (owner real-EXEC runs) stays untouched.
- Persistence policy (open question 8) is split: the **core** is policy-free (bytes in, bytes out; no name, slot, location, compression, encryption, autosave, battery RAM). The **runner** gets an operator-supplied explicit path only (exclusive create, mode 0600, like `--video-raw`; read through `ivs_hostio_read` for `--snapshot-in`) and only in the optional last plan. Battery-backed cartridge RAM has no Intellivision profile yet (PROJECT.md line 103) and is out of scope; the format does not forbid a later section.
- [REC] Record as a delegated decision in the Decision Register: "persistence policy: core exposes snapshot bytes only; the runner owns explicit-path files; no default save location; exact-build, non-archival format", reversible by adding sections and bumping the schema.

### 7.5 ABI risk (api-calls / api-statuses / reasons inventories)

- New public surface: 4 functions, 1 header, 1 status, about 12 reasons, 1 report struct. Touch list: `ivs_state.h`, `ivs.h` include, `data/api/api-effects.json` (4 calls, every refusal row with test ids), `data/expected/api-statuses.ids` (+ `IVS_E_STATE_MISMATCH -24`), `api-reasons.ids`, `pkg-install-files.ids` (installed header), `pkg-consumer-calls.ids` (consumer uses 3 of 4), `tests/headers-tap.sh` (C and C++ header self-compile), `core-symbols-allow.txt` (new TU imports only memcpy/memset/memcmp style), docs. Controls already exist for undocumented call, dead status, reason unpinned [OBS tools/check/controls/api-*], so omissions fail the gate.
- `IVS_API_VERSION` 0.1.0 is pre-1.0 "no ABI stability promise" [OBS ivs.h]; adding calls is additive; bumping minor to 0.2.0 is a records decision, not required by the rules [UNK, orchestrator].
- The one irreversible choice is the status number and the reason numbers (frozen, append-only). The format itself is reversible (pre-1.0, exact-build).
- Risk if STA lands before STIC state settles: the schema digest changes with VID-05, but nothing public does (rows are internal) - cheap; only literals and sizes move.

## 8. Unknowns and checks for planning

- [UNK] Whether VID-05 (MOB/collision/delay) adds `ivs_stic_state` members or only uses `stic_reg` (decides the schema pin and HP-MOB).
- [UNK] The exact payload size and the section list (derive by hand in S1; section count 7 assumes IMAGES has no section).
- [UNK] Whether the four profile generators emit byte-stable text across platforms (profile parameter digest).
- [UNK] Store policy of every device (`keep` flags) to confirm the IMG arrays are read-only for every profile; if a future profile has writable cartridge RAM, `mem.cart` becomes ARCH for that profile and the class must be per-profile (a coverage test on `keep` guards this).
- [UNK] Semantics of `faults-verdict` for existing DIGEST_* faults once the walker is shared (allowed prefixes).
- [UNK] Whether a second registered profile can exist in tests for a true profile-mismatch case (only `intv-2609-ntsc-family-p1` is registered; plan: test mismatch by editing the id/digest fields of a resealed snapshot).
- [UNK] `video.count == stic.frame_index` and similar candidate invariants: confirm in code when writing the checker.
- [INF] The decision to reject snapshot while OFF could be relaxed (OFF snapshot = nothing but identity); no consumer needs it.
- Not covered here (other decision points): VID-05 research on collisions/priority/delay, benchmarks (PRF-01), the release assessment (REL-01).

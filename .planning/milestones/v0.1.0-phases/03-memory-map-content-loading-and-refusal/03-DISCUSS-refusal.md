# Phase 3 discuss: DECISION POINT E, refusal matrix, atomicity and state digest

Covers MEM-04, API-06, success criteria 3 and 5 (isolation half). Labels: [OBS] read or ran this session, [INF] inference, [REC] recommendation, [UNK] unknown, [BG] background not re-fetched. Nothing here copies or opens emulator source. No web use was needed.

## 0. Inputs read and assumptions

[OBS] Phase 2 already fixed: `ivs_status` is an int, 0 OK, negative = caller refusal, "a refused call never mutates state" (D-06); `-Wcast-qual` is on (D-19); core has no alloc, no libc I/O, `string.h` allowed (c-purity); CPU state is a 32-byte pointer-free struct serialized "per field, little-endian, never by memcpy" (D-03); test harness is TAP with an exact id inventory in `data/expected/tests/<suite>.ids`; named faults are `#ifdef IVS_FAULT_<ID>` sites with `data/expected/faults/cpu.ids` (`<ID> <must-fail> <prefixes> <must-pass>`); `c-faults` and `faults-verdict` hardcode `faults/cpu.ids` and `faults.cmake` builds the fault exe from `cpu/cpu_vectors.c` only. Gate steps sort alphabetically, so `c-*` runs before `tests-*`.
[OBS] Hosted gate 31 s (run 42942719332); `.planning/CI-EFFICIENCY.md` says revisit only if a run exceeds 40 s or a phase adds a step over 5 s.
[OBS] Portable-C SHA-256 on this machine (Apple clang, arm64), 128 KB: 0.53 ms at -O2, 0.66 ms at -O1 with ASan+UBSan (scratch probe, `abc` matched `shasum`).
Assumptions made (other decision points own them; only wording of rows changes if they differ):
- A1 Caller-provided instance storage (D-07 forbids core alloc): `ivs_machine_size()`, create into caller storage, destroy zeroizes. If the lifecycle agent picks allocator callbacks, only the create-row effect text changes ("no allocation survives").
- A2 Content is loaded per image, one call per typed image (EXEC, GROM, CART container), no bundle call. System images have fixed expected sizes; the cartridge container has segments (MEM-05 owns the format).
- A3 Phase 3 has an internal machine step (CPU through the memory-map bus) so isolation tests can step; Phase 4 wraps it in `run(target_tick)`.

## 1. Lens summary (what each lens forced)

- Architect: validation must be a pure function of (instance, input); commit must be a copy that cannot fail. Instance must be a pointer-free POD so copy, relocation and later snapshots are trivial.
- Hardware/EE: nothing in refusal depends on hardware. The only coupling is [UNK] upper-6-bit ROM read behavior (C21): refuse decles above 0x3FF in a ROM image instead of masking, so the stored and the digested bytes never diverge from the supplied bytes.
- QA: oracle-free properties beat hand-written expected states: "refused implies byte-identical storage" and "accepted then unload implies fresh digest" hold for every mutation of every fixture.
- Security/rights: every length goes through widened unsigned arithmetic before any use; counts bounded before loops; no read past the supplied length (ASan covers it); no real image bytes anywhere (stand-ins only).
- DevOps/CI: one new test executable and suite, one new pure sh/awk step, no new job, no cache.
- API/integrator: distinct code only where a host would react differently; no hidden "last error" slot; every function documents its error effects in one table.
- Product: a host should see "unload then reload" as the one idiom for changing content.
- DX: on a digest mismatch the test prints per-domain digests, so the failing domain is visible.

## 2. Atomicity options

| Option | Pros | Cons / footguns |
|---|---|---|
| A. Validate-then-commit: const validate builds a stack plan, void commit copies | Smallest cost (no extra copy); `const ivs_machine *` on validate is compiler-enforced, and `-Wcast-qual` flags casting it away; commit is memcpy/table writes only | Atomicity rests on "commit cannot fail", a convention; a later fallible step in commit would reopen the bug class; source buffer must not change between validate and commit (single-thread contract) |
| B. Stage into a full copy of the machine, swap on success | Obviously atomic for any mutation order | Machine is tens to hundreds of KB: stack blowup or a second caller-provided scratch region; copies on every load; hides sloppy mutation-before-check rather than removing it |
| C. Mutate, keep an undo log, roll back | Handles incremental parsers | Rollback bugs, partial states visible if the process stops, hardest to test; rejected |
| D. Replace-on-duplicate with atomic swap | Friendlier to hosts | Failed replace must restore old image (more rows), history-dependent state; rejected, use explicit unload |

[REC] A, with three structural guards: (1) every `*_validate`/`*_plan` function takes `const ivs_machine *`; (2) commit functions return `void` and contain no early return, no callback, no loop bounded by input data except over a plan already checked; (3) no "last error" or counter is ever written into the instance (a refusal counter would itself break digest equality). The test oracle is behavioral (memcmp of raw storage plus digest), so a violation of 1 to 3 is caught by tests even if the convention rots. Lessons [BG]: libretro's load entry returns a bare bool and leaves "half-initialized after failed load" as a recurring host bug class; transactional stores (journal then commit) validate first for the same reason. A bundle call would need plan-aware validation across images; deferred (see reversal).

## 3. Recommended decisions (to become D-xx)

- E1 Error codes, append-only after `IVS_E_BAD_ARG -1`, `IVS_E_BAD_STATE -2`: UNKNOWN_PROFILE -3, BUFFER_SMALL -4, MALFORMED -5, TOO_LARGE -6, RANGE -7, OVERLAP -8, DUPLICATE -9, NO_SYSTEM_IMAGE -10. Names and values are pinned by `data/expected/status-codes.ids`; the gate fails on a rename, a renumber or an unpinned `IVS_E_*`. BUFFER_SMALL is also what API-04 (Phase 4) needs, so it is introduced here by create.
- E2 Fixed validation precedence for `load_image`; the first failing class wins, within a class the lowest segment index wins: (1) BAD_ARG: null handle, bad magic, unknown kind, nonzero reserved flags, null data with length above 0; (2) BAD_STATE: instance already powered (load-after-run); (3) DUPLICATE: that kind is already loaded; (4) TOO_LARGE: length above the kind's bound, checked in `uint64_t` before any other use of length, then segment count above the bound is MALFORMED; (5) MALFORMED: zero length, wrong exact size for a fixed-size kind, bad header or version, reserved nonzero, field inconsistency, trailing or missing bytes, decle above 0x3FF; (6) RANGE: any segment outside its allowed window or address+length overflow, for all segments; (7) OVERLAP: any two segments intersecting; (8) commit. Rule of thumb: args, then instance state, then size, then bytes, then placement.
- E3 Missing system image is refused by `reset` (the power-on transition), not by load: loads are order-independent. `reset` with EXEC or GROM absent returns NO_SYSTEM_IMAGE before touching anything. Cartridge absent is legal. This satisfies SC3 because the digest is compared across the refused call. [REC] over refusing a cart load until system images exist (order coupling and an unload cascade).
- E4 Load-after-run: any load while `powered` is BAD_STATE. `powered` is set only by a successful `reset`. `unload` returns the instance to the exact post-create state (profile kept, all content and device state cleared, powered 0) through the same internal init path as create, so "digest(unload) equals digest(fresh)" is a test.
- E5 Duplicate typed load: DUPLICATE, no replace. Zero length: MALFORMED regardless of pointer (a null pointer with length above 0 is BAD_ARG). Unknown profile: only `create` can return UNKNOWN_PROFILE; storage untouched. Use after destroy: destroy zeroizes storage; a second destroy or any call returns BAD_STATE (best effort, magic only; not a claim about arbitrary garbage).
- E6 Effect vocabulary for API-06: `none` (raw storage byte-identical and digest equal; for create, the caller's storage is untouched) or `partial:<token>` with the token listed in `data/expected/api-partial-effects.ids` (empty in Phase 3: every Phase 3 refusal is `none`).
- E7 State digest: SHA-256 (FIPS 180-4), one primitive shared with MEM-03 image digests, in `core/src/common/ivs_sha256.c` (copy-paste, streaming init/update/final, no dependency). Per-domain digests with a composite (section 5). Public `ivs_machine_digest(const ivs_machine *, domain, out[32])`.
- E8 Test oracle for "unchanged": both `memcmp` of the whole instance storage (strict; sees padding and excluded bytes, so scratch use of the instance is caught) and digest equality (public, domain-localized). Every row runs against each prior state in its mask: fresh, system-loaded, full (system plus cart), powered. Init zeroes the whole storage first.
- E9 One test executable and one ctest suite `machine` (ids prefixed `machine.refuse.`, `machine.iso.`, `machine.digest.`, `machine.sha256.`, `machine.sweep.`), run in dev and release presets like the CPU suites.
- E10 API-06 table `data/api/state-effects.tsv` plus a pure gate step `tests-api-effects` that cross-checks it against ids, TAP results and observed (function, code) pairs (section 6).
- E11 Named-fault controls for mutation-before-refusal, shared state and digest defects (section 7), by generalizing `c-faults`, `faults-verdict` and `faults.cmake` to every `data/expected/faults/*.ids`.
- E12 Isolation evidence is behavioral plus structural: interleaved-versus-sequential equality, byte-identical storage from differently poisoned storage, memcpy relocation, and a `core-symbols` rule that the core archive defines no writable data symbols. No threaded test in the gate.

## 4. Refusal matrix (rows of the `machine` suite)

Id form `machine.refuse.<op>.<kind>.<slug>`; cells in `data/expected/refusal-cells.ids` are `<op>.<kind>.<class>` and each needs at least one row (checked by `tests-api-effects`). Effect is `none` for every row. P = prior-state mask (F fresh, S system loaded, C full, W powered).

| Row slug (op) | Input | Code | P |
|---|---|---|---|
| create.null-storage / .null-out | NULL arg | BAD_ARG | n/a |
| create.misaligned | storage not aligned for the instance struct | BAD_ARG | n/a |
| create.small-storage | size one less than `ivs_machine_size()` | BUFFER_SMALL | n/a |
| create.unknown-profile | id string not in table, and a near-miss id (one char off, trailing space) | UNKNOWN_PROFILE | n/a (poisoned storage compared bytewise) |
| load.null-machine / .bad-magic | NULL, zeroized instance | BAD_ARG / BAD_STATE | n/a |
| load.bad-kind / .reserved-flags | kind 0, 4, 255; flags nonzero | BAD_ARG | F S C |
| load.null-data | NULL with length 8 | BAD_ARG | F S |
| load.after-power (each kind) | valid image, powered | BAD_STATE | W |
| load.duplicate (each kind) | valid image, kind present | DUPLICATE | S C |
| load.zero-length (each kind) | length 0 with non-null and with null pointer | MALFORMED | F S |
| load.exec/grom.undersized, .oversized, .off-by-one-high | exact size minus 1, plus 1, far above | MALFORMED / TOO_LARGE | F S |
| load.len-wrap | length 2^32 + small (64-bit only, `skip-ok` on 32-bit) | TOO_LARGE | F |
| load.cart.truncated-every-prefix | container cut at header, mid-descriptor, mid-segment | MALFORMED | F S C |
| load.cart.bad-magic / .bad-version / .reserved | header faults | MALFORMED | F S |
| load.cart.trailing-bytes | valid plus 1 byte | MALFORMED | F S |
| load.cart.decle-high-bits | decle 0x400 and 0xFFFF in the last word | MALFORMED | F S |
| load.cart.too-many-segments | count above bound, huge count with tiny data | MALFORMED | F |
| load.cart.range-start / .range-end / .addr-wrap | segment outside windows, ends past the end, address+length wraps 16 bits | RANGE | F S C |
| load.cart.overlap-exact / -partial / -adjacent-ok | equal, one-word overlap; adjacent must be OK (positive control) | OVERLAP / OK | F S C |
| prec.* | a double violation per adjacent pair of the E2 list, e.g. powered and malformed gives BAD_STATE; malformed and range gives MALFORMED; range and overlap gives RANGE; duplicate and oversized gives DUPLICATE | the earlier class | as needed |
| reset.no-exec / .no-grom / .neither | image missing | NO_SYSTEM_IMAGE | F, S minus one |
| reset.no-cart-ok | EXEC and GROM only | OK (positive) | S |
| unload.null / destroy.null / destroy.twice | | BAD_ARG / BAD_ARG / BAD_STATE | n/a |
| digest.bad-domain / .null-out | domain 0, 99 | BAD_ARG | any |
| step.unpowered (if A3 holds) | step before reset | BAD_STATE | F S C |
| exec.digest-mismatch (conditional) | only if D5 chooses strict identity | new code IDENTITY (-11) | F |

[INF] About 45 to 60 ids. Accepted images also get checks: after OK the reported length and SHA-256 equal the supplied bytes' (literal vectors), and `unload` returns to the fresh digest.

Sweeps (oracle-free, ids `machine.sweep.*`): for each cart fixture, every truncation length 0..len and every single-byte change to 0x00 and 0xFF; each load must be OK or one allowed refusal code; a refusal implies raw-storage equality (memcmp only, no SHA, about 300 loads); an OK implies `unload` equals the fresh digest. One seeded random-mutation id may reuse the cpu-fuzz style. Label as robustness, not accuracy.

## 5. State digest and canonical serialization

Covers all machine-observable state, serialized from named fields, never by struct memcpy. [REC]
- Domains: 1 CPU (R0-R7, status, intff, dbd, prev_interruptible, run_state, halt_pending, fault_reason, busak; the zero reserved bytes are not serialized); 2 MEM (per device in descriptor order: system RAM words, GRAM bytes, STIC and PSG register files when they exist, EXEC, GROM and cartridge storage as stored); 3 IMAGES (per slot: kind, present, length u32, SHA-256, segment count and each segment's address and length); 4 INSTANCE (profile id as length-prefixed ASCII, powered, state version; the master tick joins here when the scheduler lands). Pointers, padding, the magic, host addresses and any history counter are never serialized (and no such counter may exist).
- Composite STATE = SHA-256("IVSSTATE" || u16 version || for domains 1..4 in order: u8 id || 32-byte domain digest). A new device or time domain adds a domain id and bumps the version; existing domain digests stay stable. Reserved ids for RUN-05: 16 video, 17 audio, 18 input log (outputs and logs, not part of STATE) using the same framing.
- Domain preimage: "IVSD" || u8 domain id || u16 digest-format version || fields in table order. Integers fixed width little-endian written by shifts. Booleans u8. Enums as documented numeric values, never compiler enums. Fixed-schema fields carry no length; every variable-length list has a u32 count first, so the stream is unambiguous. The profile id inside domain 4 means the same bytes under two profiles never collide.
- A single field table (`name, offset, width, count, domain`) drives the serializer. Guards: (a) a bitmap check that every byte of `ivs_machine` is covered by exactly one table row or declared padding, plus `_Static_assert(sizeof)` against the table total, so a new struct field cannot be forgotten silently; (b) the table dump is compared with the checked-in `data/expected/digest-fields.ids` (same mechanism as the MEM-01 decode dump; that decision point picks the mechanism) so a schema change forces a version bump and a golden refresh.
- Independent anchors (not self-consistency): SHA-256 literals for 0, 1, 55, 56, 63, 64, 65, 119, 120, 128 byte inputs (padding boundaries) plus the two FIPS 180-4 sample messages, expected values taken from FIPS and from `cmake -E sha256sum` at authoring time; a hand-written literal byte stream for a tiny synthetic state per domain, compared byte for byte through a sink callback (production sink = hash update, test sink = capture buffer); the digest of that stream is checked against the same literal. A fresh-instance composite per profile is a literal in the test, tier "repeatability only" (promote to a RUN-05 golden file in Phase 4).
- Sensitivity sweep (`machine.digest.sensitivity`): for each table row flip the first and the last byte of a copy; the digest must change in exactly the row's domain and the composite must change; padding flips change nothing. About 60 digests, tens of ms.
- Cost [OBS-based INF]: 128 KB state about 0.5 ms, so roughly 200 digests per run is about 0.1 s dev. Per-step trace digests use domain 1 only (32 bytes).

## 6. Test architecture and gate plug-in

Suite `machine` (one exe `ivs_tests_machine`, ctest name `machine`, TAP at `build/gate/<preset>/machine.txt`, inventory `data/expected/tests/machine.ids`). The existing `tests-inventory` step already enforces exact ids, nonce, plan, no duplicates and ctest list, at no new cost.
- Table-driven rows: each row is a C struct `{id, op, prior_mask, builder, expected_code}` in `tests/machine/`; builders synthesize bytes from the committed text fixtures of MEM-05 or mutate them (truncate, set byte, override length); no binary is committed. A macro `REFUSE_CALL(fn, expected, call)` snapshots the instance, makes the call, asserts the code, memcmp and digest equality, runs `ivs_machine_validate`, and appends `# pair <fn> <CODE> none` to a pairs file `build/gate/<preset>/machine.pairs` (this needs a small harness addition: a second output stream with the same nonce header).
- Isolation (`machine.iso.*`): A and B, same profile, different stand-in EXEC/GROM/cart, in separately aligned storage. (1) Sequential: A for N units, then B; record composite and CPU digests every 8 units. (2) Fresh pair stepped interleaved by a fixed LCG schedule (chunks 1 to 7), B created first; the checkpoint digests at equal step counts must match (1). (3) Destroy A mid-way, finish B, still equal. (4) A load into B, successful or refused, leaves A's digest unchanged. (5) Poison A's storage with 0xA5 and B's with 0x5A before create, same content, same steps: raw storage must be byte-identical (proves init covers every byte; poor man's MSan on Apple clang). (6) `memcpy` an instance to new storage, step both: equal (no self-pointers, no hidden global). No instance id from a global counter.
- Static evidence of no shared mutable state: `core-symbols.sh` gains a writable-data rule (`nm -P` types d D b B C) with token `core-symbols-writable`, on the -O2 archives c-purity already builds, with `-fno-pic` so ELF const tables stay in rodata. [UNK] ELF `.data.rel.ro` and Mach-O `s` behavior on the hosted gcc; verify on the first hosted run, else fall back to an allowlist of reviewed names. [OBS] today's release archive on macOS shows only T, t, U, s.
- API-06 table `data/api/state-effects.tsv`, one line per (function, code): `<function> <CODE> <effect> <test-id>[,<test-id>...] <note>`. Step `tests-api-effects` (pure sh/awk, named so it sorts after `c-tests`) checks: every `IVS_E_*` in `ivs_status.h` has a line (value pinned by `status-codes.ids`); every listed test id is in the machine ids and passed in both presets; the set of (function, code, effect) in the table equals the set observed in `machine.pairs` (a documented pair no test hits, or a tested pair nobody documented, both fail); every cell of `refusal-cells.ids` has a row; every public `ivs_status` function in `core/include/ivs/*.h` has at least one line or an explicit `-` entry. Tokens: `api-effects-code-untested`, `-pair-unobserved`, `-pair-undocumented`, `-effect-mismatch`, `-test-missing`, `-test-failed`, `-cell-uncovered`, `-code-renumbered`, `-pairs-missing`, `-function-undocumented`.
- Controls: about 10 `tools/check/controls/api-effects-*` overlays on `_good/tree` plus 2 for `core-symbols-writable`, listed in `data/expected/controls/api-effects.ids`.
- Gate cost [INF, not measured]: new core TUs (about 6) plus one test exe in two presets, plus fault copies, about +2 to 4 s on the cold hosted tree (c-build is 18 s there); `tests-api-effects` and `core-symbols` change at most 0.3 s each; about 12 more controls overlap the steps (controls-wait is 0 today); about 14 more fault exes add about 1 s to `c-faults`. Expect 34 to 36 s hosted, over the 30 s step budget and under the 40 s revisit trigger. Offsets: one exe not three, memcmp instead of SHA in sweeps, no new job or cache, no threaded test. [REC] re-baseline the budget to 40 s in D-xx and measure on the first hosted run.

## 7. Named-fault controls (`data/expected/faults/machine.ids`, suite `machine`)

Each is a site of at most 3 lines; must-fail row, allowed prefix and a must-pass sensitivity row as in `cpu.ids`. Over-reads are not fault-testable (fault exes link no sanitizer by design); they are covered by ASan in the normal dev-preset run, using exact-size heap buffers for boundary rows (documented exemption).
- Mutation before refusal: LOAD_LEN_BEFORE_CHECK (image length recorded first); LOAD_COPY_THEN_OVERLAP (segment 1 copied before overlap is found); LOAD_CLEAR_THEN_VALIDATE (destination window wiped first); LOAD_TABLE_BEFORE_VALIDATE (image table entry written first); RESET_CLEARS_BEFORE_IMAGE_CHECK; CREATE_MAGIC_BEFORE_PROFILE_CHECK; DUPLICATE_REPLACES.
- Precedence and typing: CHECK_ORDER_RANGE_FIRST; CODE_OVERLAP_AS_RANGE; STATE_CHECK_OMITTED (load after power succeeds); LEN_TRUNCATED_32 (wrap row); ROM_DECLE_MASKED_NOT_REFUSED.
- Shared state: STATIC_CARRY (a file-scope carry of the previous call's length, caught by interleaving); UNLOAD_KEEPS_CART; INIT_SKIPS_MEMSET (poisoned-storage equality).
- Digest: DIGEST_SKIPS_RAM; DIGEST_SKIPS_IMAGE_TABLE; DIGEST_BIG_ENDIAN (literal stream); DIGEST_INCLUDES_PADDING (relocation or poison test); DOMAIN_ID_DROPPED (profile or domain collision).
Infra change [INF]: `faults.cmake`, `c-faults.sh` and `faults-verdict.sh` loop over `data/expected/faults/*.ids`, taking the suite header per file, mapping suite to its test source, and treating the union of ids as the allowed site set (orphan and missing-site checks must use the union). About 20 to 40 lines of sh and cmake; existing `faults-*` controls guard the loop.

## 8. Adversarial pass

- "Commit cannot fail is only a convention." True; mitigated by const validate, void commit, the memcmp oracle and seven mutation faults. If one still escapes, add the missing row (the Phase 2 sufficiency rule).
- "Per-call atomicity is not per-bundle atomicity": a host loading EXEC, GROM and cart sees the first two persist if the cart is refused. Documented idiom is `unload` on any failure; the API-06 table states the effect per call. Reversal: add `load_set` using the same plan machinery.
- "memcmp of raw storage over-constrains the implementation": intended; it is what proves no scratch use and no padding writes. It does not constrain the digest, which stays logical.
- "SHA-256 is overkill": measured 0.5 ms per 128 KB; it also serves MEM-03 and RUN-05, so no second primitive. If per-unit full-state digests become a need, add a cheap rolling hash for traces only.
- "Pairs and table are bookkeeping tests can game": the macro makes the call itself and logs the observed status, so a pair cannot be claimed without being returned.
- "Refusing at reset instead of load hides the missing image from the loader": the host gets it at the first moment it matters, and hosts that load in any order work. Reversal: a profile or D5 outcome that wants early refusal can add a cart-load check as a new precedence class.
- "No threaded test": structural (no writable globals), behavioral (interleave, relocation, poison) evidence is stronger per cost than a TSan-less thread run [INF]. A one-off local TSan probe is allowed as throwaway, not in the gate.

## 9. Top risks, reversal, blockers

Top risks: (1) hosted gate lands above 40 s (levers in order: drop the second-preset run of `machine.sweep.*`, run fault exes in parallel with `xargs -P`, background the C build); (2) the writable-data symbol rule gives ELF false positives, so the isolation proof leans on behavior; (3) the digest field table drifts from the struct (guard a and b in section 5), or the MEM-01 dump mechanism chosen elsewhere is incompatible with the field-table dump.
Would reverse it: a host requirement for bundle atomicity (add `load_set`); an allocator-based lifecycle that makes instances non-POD (then staging in section 2 option B becomes more attractive and the memcmp oracle becomes digest-only); evidence that per-unit full-state digests are needed (rolling hash).
[UNK] blocking planning: none strictly. Coupled to other decision points: whether D5 makes EXEC identity strict (adds code IDENTITY -11 and rows); the final cartridge container and its segment and window bounds (rows are parametrized by them); whether a machine step exists in Phase 3 (A3; if not, isolation steps the CPU against the map bus through a test driver).

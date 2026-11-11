---
phase: 03-memory-map-content-loading-and-refusal
asvs_level: 1
block_on: high
status: secured
threats_total: 51
threats_closed: 51
threats_open: 0
audited: 2026-11-11
audited_commit: 684e729
---

# Phase 3 Security Verification

Phase 3 adds the first public machine API. The host supplies the instance storage (create, destroy, info, state digest), typed content loading (raw EXEC and GROM images, IVSC v1 cartridge containers), unload, power-on reset, a profile-generated memory system and an in-repo SHA-256. The attack surface is host-controlled input crossing that API: instance storage (alignment, size, aliasing, lifetime), load buffers and lengths (overflow, 32-bit `size_t`, wrap, overlap with the instance), untrusted IVSC bytes (counts, lengths, offsets, trailer digest), out-parameters, `struct_size` forward-compatible tails, the reset and load state machine, and the digest code. It also covers the gate's evidence chain and the public repository.

## Verification performed

- **Code reading:** every core file in the diff: `core/src/machine/{machine,machine_load,machine_run,machine_digest}.c`, `core/src/content/ivsc.c`, `core/src/common/ivs_sha256.c`, `core/src/memmap/memsys.c` and `core/include/ivs/*.h`.
- **Fault wiring:** all 47 Phase 3 named faults (`data/expected/faults/{ivsc,machine,memmap,sha256}.ids`) have an `IVS_FAULT_*` site in core, or a generator site for `GEN_MAP_MUTATION` (`tests/CMakeLists.txt:237`, `cmake/IvsMemmapTable.cmake:34`). Each one's must-fail target id is in `data/expected/tests/*.ids`.
- **Full local gate:** `sh tools/gate.sh` at `684e729` printed `gate: PASS steps=26/26 controls=224/224 ... faults=115/115 ... tests-expected=2094 tests-executed=2094 tests-skipped=0`, with `gate-time total=27`.
- **Sanitized build:** a separate ASan+UBSan build (`-O1 -g`, `IVS_SAN=ON`, outside the repo). The ivsc, machine, memmap and headers suites pass with no sanitizer report.
- **Adversarial parser and load probe, under ASan+UBSan (auditor-written, not committed):**
  - Inputs: 200,000 inputs mixing valid-structure containers (1 to 16 segments, correct trailer), random bytes, bit flips and truncations, each copied into an exact-size heap buffer and fed to `ivs_ivsc_parse` and `ivs_machine_load(KIND_CARTRIDGE)`.
  - Results: 27,455 accepted and 172,545 refused. No sanitizer report, and no refused load changed the composite state digest.
  - Lying lengths: 131,257, `SIZE_MAX` and 2^40 against a 1-byte buffer each return OVERSIZED without reading the buffer.
- **API probe, under ASan+UBSan:**
  - Create refusals: misaligned storage gives BAD_ARG, an out-pointer inside the storage gives BAD_ARG, storage one byte short gives BUFFER_TOO_SMALL, an unterminated 64-byte id in an exact-size buffer gives BAD_ARG, and a nonzero options tail gives UNSUPPORTED. In every case the storage is byte-identical afterwards.
  - Input-span overlap: EXEC input spans at offsets -8191, -1, 0, 1 and N-1 relative to the instance give BAD_ARG with reason 28. Spans at -8192 and N are accepted.
  - Out-parameter aliasing: a report, info or digest out-parameter inside the instance gives BAD_ARG with the instance unchanged.
  - Stale handle: a call after destroy gives BAD_HANDLE.

## Hard constraints (whole branch diff vs main, 63 commits)

| Constraint | Result | Evidence |
|---|---|---|
| No rights-bearing material | PASS | No `.ivsc/.rom/.bin/.int/.itv/.cfg/.pdf` tracked or present in any branch commit. No added file has NUL bytes or a non-text MIME type. Fixtures are project-authored text JSON (`fixtures/FX-ivsc-vectors/manifest.json`, `redistributable: yes`, MIT). Machine stand-ins are generated in test code (`tests/machine/machine_standin.c:1-15`). `files` step: `binaries-listed=0` |
| `refs/` ignored and untracked | PASS | `.gitignore:2 /refs/`; `git ls-files refs` is empty |
| `*.ivsc` ignored and binary by name | PASS | `.gitignore:16`; `tools/check/binary-ext.txt` (`ivsc`) |
| No PII, credentials, tokens or home paths | PASS | No `/Users/`, `/home/`, `/private/tmp/claude`, `/var/folders/`, `ghp_`, `github_pat_`, `sk-ant-`, `AKIA`, `xox?-` or private-key hits in tracked files, in `git log -p main..HEAD` (every intermediate commit) or in commit messages. Every author and committer is `szTheory@users.noreply.github.com`. `hygiene` passes |
| No copying from other emulators or their format docs | PASS | No MAME, jzIntv, Zbiciak, libasm, as1600, FRASM, `.cfg`/`[mapping]`/CFGVAR or LUIGI text in added code, data or docs. SHA-256 is written from FIPS 180-4 (`ivs_sha256.c:1-12`). The IVSC format is project-specified (`docs/formats/ivsc-v1.md`, `ivsc.c:2-4`). `denylist` passes with `scanned=185 waived=0` |
| "Intellivision" not used as a name | PASS | No path, target, symbol, format or package name contains it. The project is `project(ivs LANGUAGES C)` and the format is IVSC |
| CI workflow least-privilege and SHA-pinned | PASS (unchanged) | `git diff main...HEAD -- .github` is empty. `ci-parity` passes with `ci-uses=1 ci-workflows=1` |
| `tools/gate-linux.sh` | PASS with a WARNING (see Unregistered flags) | No `-v`/`--mount`, no `-e`/`--env-file`. Streams `git ls-files -co --exclude-standard` as tar over stdin (`:30-37`, `:55`); `.git` and history are not sent (the container runs `git init`, `:48`). Output goes to the ignored `build/`. Not referenced by `.github/workflows/gate.yml` or `tools/gate.sh` |
| Guest and host boundary bounded | PASS | Every parser loop is bounded by a checked count (`ivsc.c:123`, `:139`, `:176`, `:218-220`). The cartridge scan is bounded by 16 (`memsys.c:192`). Backing indices are masked to power-of-two sizes (`memsys.c:93`), with generated capacity asserts (`build/*/core/gen/ivs_profiles.inc` `_Static_assert`s, emitted by `IvsMemmapTable.cmake:296-446`). The probes above are clean |

## Threat verification

| Threat ID | Category | Severity | Disposition | Status | Evidence |
|---|---|---|---|---|---|
| T-03-01 | Spoofing | medium | mitigate | CLOSED | Per-file suite header and union site checks in `tools/check/steps/faults-verdict.sh`. Controls `faults-suite-missing`, `faults-second-not-detected` and `faults-second-missing-site` in `tools/check/controls/`. Gate `controls=224/224` |
| T-03-02 | Tampering | medium | mitigate | CLOSED | `fault-duplicate-id` in `faults-verdict.sh`, duplicate refusal in `tests/faults.cmake` and `c-faults.sh`. Control `faults-duplicate-id` |
| T-03-03 | Repudiation | low | mitigate | CLOSED | `tests/faults.cmake:15` (stated rule), `:52`, `:58`, `:70`, `:87`, `:114`, `:131`, `:141`: fault targets link `ivs_warnings` only, never `ivs_san` |
| T-03-04 | Spoofing | high | mitigate | CLOSED | FIPS literals in `data/sha256/vectors.txt`, re-derived with `string(SHA256)` (`cmake/IvsShaVectors.cmake:127-129`, fails closed). Six faults: `SHA_K_WRONG`, `SHA_ROTATE_WRONG`, `SHA_LENGTH_LE`, `SHA_PAD_BOUNDARY`, `SHA_FINAL_BLOCK_DROPPED`, `SHA_INIT_NO_RESET` (`ivs_sha256.c:67`, `:102`, `:131`, `:183`, `:199`, `:203`). Cases `machine.sha256.split-every-offset` and `ctx-copy-snapshot` |
| T-03-05 | Tampering | medium | mitigate | CLOSED | `uint64_t total_bytes` (`ivs_sha256.h:27`, `ivs_sha256.c:150`, `:181`). Cases `pat-0055/56/57/63/64/65/119/120/127/128/129` are present in `data/expected/tests/machine.ids`. UBSan in dev (`cmake/IvsWarnings.cmake:26-30`). Faults `SHA_PAD_BOUNDARY` and `SHA_LENGTH_LE` |
| T-03-06 | Elevation of privilege | low | mitigate | CLOSED | `tools/check/core-symbols.sh` with `core-symbols-allow.txt`. The only Phase 3 addition is `_GLOBAL_OFFSET_TABLE_` (linker-defined GOT base, no host state; commit `0bea375`). `c-purity` passes with `core-imports=4` |
| T-03-07 | Tampering | high | mitigate | CLOSED | `data/expected/core-generated.ids` (`ivs_profiles.inc cmake/IvsMemmapTable.cmake`). Generator run plus scan in `tools/check/steps/c-purity.sh:78-110`. Controls `core-purity-generated-unlisted`, `-scan` and `-missing` |
| T-03-08 | Information disclosure | medium | mitigate | CLOSED | `core/CMakeLists.txt:61`: PUBLIC include is `core/include` only. Internals go through the INTERFACE `ivs_core_internal` (`:63-64`), linked only by tests |
| T-03-09 | Elevation of privilege | high | mitigate | CLOSED | `core-symbols.sh --forbid-writable-data` (b/B/d/D/c/C) on `-fno-pie` archives (`c-purity.sh:221`, `:240`). Control `core-purity-static-mutable`. Behavioural `machine.iso.*` (T-03-41) |
| T-03-10 | Denial of service | medium | mitigate | CLOSED | `-fno-pie` in `c-purity.sh:221`. The writable type set excludes s/S/r/R (`core-symbols.sh` header). Hosted run confirmed (post-verification fix `0bea375` for gcc PIE GOT) |
| T-03-11 | Tampering | high | mitigate | CLOSED | `cmake/IvsMemmapTable.cmake`: exact key sets (`:58-74`), granule and span (`:351`), power-of-two (`:424`), overlap (`:502`), nonzero power-on fill refused (`:237`), capacity asserts (`:296`, `:354`, `:446`). Fault `GEN_MAP_MUTATION` (`tests/CMakeLists.txt:237`) |
| T-03-12 | Information disclosure | high | mitigate | CLOSED | `memsys.c:93`: index `(addr - lo) & (size_words - 1)`. Backing sizes in `memsys.h:29-34`. Generated `_Static_assert(size <= cap)` per backing. Cartridge reads only inside committed segments, which the load's span check confines to the span (`machine_load.c:95-110`, `:161-165`; install guard `memsys.c:323`). Faults `MEM_MIRROR_MASK` and `MEM_CART_END_INCLUSIVE`. ASan clean |
| T-03-13 | Spoofing | medium | mitigate | CLOSED | Undriven fill `$FFFF` (`data/profiles/intv-2609-ntsc-family-p1.json:301`). Unloaded EXEC/GROM and closed gates read the fill (`memsys.c:155-179`). Fault `MEM_UNDRIVEN_ZERO` (`memsys.c:84`). Cases `memmap.unloaded.*` |
| T-03-14 | Repudiation | low | mitigate | CLOSED | Tag, cite and conflict on every profile field (`profile` step passes with `records=51 fields=51`). C60-C62 recorded |
| T-03-15 | Repudiation | medium | mitigate | CLOSED | `tools/check/memmap.cmake:11-28` (`src=`/`tag=`/`conf=` grammar, tier-5 refused). Controls `memmap-literal-no-cite`, `-tier5`, `-unknown-source`, `-unknown-tag`, `-unknown-conf` and `-grammar` |
| T-03-16 | Tampering | medium | mitigate | CLOSED | Exact per-view tiling in `memmap.cmake`. Controls `memmap-literal-hole`, `-overlap`, `-orphan` and `-missing` |
| T-03-17 | Information disclosure | medium | mitigate | CLOSED | `tools/check/steps/denylist.sh:34-41` scans every tracked file outside `.planning/` and `tools/check/`, including `data/`. Passes with `scanned=185 waived=0` |
| T-03-18 | Spoofing | medium | mitigate | CLOSED | Nonce header bound to `build/gate/NONCE` (`memmap.cmake:33`, `:395-396`). Both presets are compared. Controls `memmap-dump-stale`, `-diff` and `-missing` |
| T-03-19 | Tampering | medium | mitigate | CLOSED | Probes generated from the literal (`cmake/IvsMemmapProbes.cmake`). Five width and gate faults: `MEM_ROM_NO_MASK`, `MEM_RAM8_NO_TRUNC`, `MEM_RAM16_TRUNC`, `MEM_ROM_WRITABLE`, `MEM_GATE_IGNORED` (`memsys.c:171`, `:229`, `:234`, `:243`, `:164`) |
| T-03-20 | Repudiation | low | mitigate | CLOSED | `data/claims.json` CL-0006: tier `source-expectation`, standing `provisional`, statement names its behavioural and declarative columns |
| T-03-21 | Tampering | high | mitigate | CLOSED | `ivsc.c:80-86`: `uint64_t` length and OVERSIZED before any byte is read. Total cross-checks at `:109-118`. Wrap at `:156`. Table pre-check at `:129`. Exact size at `:164`. Cases B06, B08, N09, N10, N11 and N34. Probe: lying lengths up to 2^64-1 give OVERSIZED |
| T-03-22 | Information disclosure | high | mitigate | CLOSED | Entry reads are bounded by the `:129` pre-check. Word offsets are computed from checked counts only (`:161`) under exact equality with `total == len` (`:164`, `:113-118`). Exact-size heap buffers under ASan in dev. Cases N01, N02, N10 and N30. Probe with 200k inputs is ASan clean |
| T-03-23 | Spoofing | medium | mitigate | CLOSED | Trailer digest is checked before word range (`ivsc.c:195-211` before `:213-225`), compared in constant time over 32 bytes. Expected trailers come from `cmake -E sha256sum` (`tools/cart/manifest-outputs.sh`, `cmake/IvsCartTable.cmake`). Cases N27, N28, N29 and N31. Fault `IVSC_SKIP_DIGEST` |
| T-03-24 | Information disclosure | medium | mitigate | CLOSED | Text JSON fixtures only. Manifest `fixtures/FX-ivsc-vectors/manifest.json` (effective yes, author-attested). `files` and `hygiene` steps pass |
| T-03-25 | Denial of service | medium | mitigate | CLOSED | Every loop is bounded by a checked count. Seeded fuzz `ivsc.fuzz.seed-01..08` and `reasons-covered` under ASan/UBSan in dev (`tests/ivsc/ivsc_fuzz.c`). Optional libFuzzer target `tests/fuzz/fuzz_ivsc.c` (corpus under the ignored `build/`) |
| T-03-26 | Tampering | medium | mitigate | CLOSED | `tests/ivsc/ivsc_fuzz.c:280-303`: determinism, input unwritten, refusal leaves the view poisoned, acceptance leaves the report poisoned |
| T-03-27 | Spoofing | medium | mitigate | CLOSED | Six faults in `data/expected/faults/ivsc.ids`: `IVSC_SKIP_DIGEST`, `_END_OFF_BY_ONE`, `_ACCEPT_WORD_400`, `_COUNT_17`, `_TRUST_TOTAL_BYTES`, `_ADJACENT_AS_OVERLAP` (sites `ivsc.c:120`, `:133`, `:165`, `:181`, `:205`, `:215`) |
| T-03-28 | Information disclosure | low | mitigate | CLOSED | `.gitignore:16` `*.ivsc`; `binary-ext.txt` `ivsc` |
| T-03-29 | Tampering | high | mitigate | CLOSED | `machine.c:146-194` validates everything before the first write at `:201`. Poisoned-storage rows (`tests/machine/machine_refuse.c:7-13`). Fault `CREATE_MAGIC_BEFORE_PROFILE_CHECK` (`machine.c:184`). API probe: storage byte-identical on every refusal |
| T-03-30 | Information disclosure | medium | mitigate | CLOSED | `machine.c:67-80`: loop bounded by 64, no string functions (`core-purity-strlen` control). The probe's unterminated 64-byte heap id gives BAD_ARG, ASan clean |
| T-03-31 | Spoofing | medium | mitigate | CLOSED | `machine.c:214-227` zero-scrubs on destroy. Magic check at `:29-43`. Fault `DESTROY_NO_SCRUB`. Probe: stale handle gives BAD_HANDLE |
| T-03-32 | Repudiation | medium | mitigate | CLOSED | Field table covering every instance byte (`machine_digest.c` `ivs_digest_fields`, size pinned by `machine.h:52`). Four faults: `DIGEST_SKIPS_RAM`, `DIGEST_SKIPS_IMAGE_TABLE`, `DIGEST_LITTLE_ENDIAN`, `DIGEST_INCLUDES_PADDING` (plus `DOMAIN_ID_DROPPED`) |
| T-03-33 | Tampering | high | mitigate | CLOSED | `machine_load.c:184` `ivs_load_validate(const ivs_machine *...)`. `:279` `static void ivs_load_commit`, copy-only. Commit is reached only on `IVS_OK` (`:341-345`). Probe: digest unchanged across 172k refusals |
| T-03-34 | Tampering | medium | mitigate | CLOSED | Stage 6 length bound (`machine_load.c:249`) before stage 7 overlap (`:254`). Wrap-proof `uintptr_t` difference test (`machine.c:45-56`). Matrix rows `*.input-inside`, `overlap-exact/partial/adjacent-ok`, `prec.oversized-over-overlap`, `prec.wrap-over-overlap`. Fault `OVERLAP_BEFORE_LENGTH` |
| T-03-35 | Spoofing | medium | mitigate | CLOSED | Reported digest is the parser's whole-file SHA-256 (`ivsc.c:199-200`, `machine_load.c:174`), never the trailer. Fault `CART_REPORT_TRAILER_DIGEST` (`:175`). `expect_sha256` compared over all 32 bytes (`:264-271`) |
| T-03-36 | Information disclosure | medium | mitigate | CLOSED | Stand-in bytes generated in `tests/machine/machine_standin.c`. No `fopen` or network in the machine or ivsc tests. `files` and `hygiene` pass |
| T-03-37 | Tampering | high | mitigate | CLOSED | Whole-storage `memcmp` plus digest per row (`machine_refuse.c:7-13`, `machine_sweep.c:92-93`). Faults `LOAD_LEN_BEFORE_CHECK`, `LOAD_TABLE_BEFORE_VALIDATE`, `CART_COPY_BEFORE_VALIDATE`, `DUPLICATE_REPLACES` (`machine_load.c:328`, `:333`, `:155`, `:231`) |
| T-03-38 | Tampering | high | mitigate | CLOSED | `uint64_t n = len` compared before any read (`machine_load.c:189`, `:249`). Rows `len-wrap-heap` and `len-wrap-inside` (`machine_refuse.c:606-614`, `:961-963`; skip-ok on 32-bit `size_t`). Fault `LEN_TRUNCATED_32` (`machine_load.c:325`). Row `oversized-far-inside` |
| T-03-39 | Spoofing | medium | mitigate | CLOSED | 14 `machine.prec.*` pairwise rows. Faults `CHECK_ORDER_MALFORMED_FIRST` (`machine_load.c:238`) and `ROM_DECLE_MASKED_NOT_REFUSED` (`:120`) |
| T-03-40 | Tampering | low | mitigate | CLOSED | Span-relative fixture addresses resolved from the literal (`cmake/IvsCartTable.cmake`). Case `ivsc.X01-span-matches-map` |
| T-03-41 | Information disclosure | high | mitigate | CLOSED | Five `machine.iso.*` cases (cross-load, destroy-mid-run, interleaved-equals-sequential, memcpy-relocation, poisoned-storage-identical). Fault `STATIC_CARRY` (`machine_run.c:30-34`). Writable-data rule (T-03-09) passes for every core file |
| T-03-42 | Tampering | medium | mitigate | CLOSED | `machine_run.c:52-82`: kind, handle, EXEC then GROM checks before `ivs_run_power_on`. Fault `RESET_CLEARS_BEFORE_IMAGE_CHECK` (`:69`). Rows `machine.refuse.reset.*` |
| T-03-43 | Tampering | medium | mitigate | CLOSED | Load stage 4 (`machine_load.c:222-228`) and unload (`:364`) refuse when POWERED. 25 `.w` rows. Fault `STATE_CHECK_OMITTED` (`:223`) |
| T-03-44 | Tampering | medium | mitigate | CLOSED | Seven `machine.sweep.*` cases compare instance storage with `memcmp` and digest (`tests/machine/machine_sweep.c:92-93`) |
| T-03-45 | Denial of service | low | mitigate | CLOSED | `tests/headers-tap.sh`: C17 `-pedantic -Werror` and C++17 where present, with a visible SKIP otherwise. `data/expected/tests/headers.ids` |
| T-03-46 | Repudiation | medium | mitigate | CLOSED | `tools/check/api-effects.cmake:14-17`, `:113-115`. Controls `api-status-renumbered`, `-unpinned` and `-reserved`. Pins in `data/expected/api-statuses.ids` |
| T-03-47 | Repudiation | medium | mitigate | CLOSED | `api-effects.cmake:23-25`. Controls `api-dead-status`, `api-refusal-no-test` and `api-effect-unregistered`. `api-effects` passes with `api-rows=27` |
| T-03-48 | Information disclosure | low | mitigate | CLOSED | `api-effects.cmake:19-20`. Controls `api-call-undocumented` and `api-call-undeclared`. `api-calls=10` |
| T-03-49 | Repudiation | medium | mitigate | CLOSED | `03-14-SUMMARY.md`: fresh gate run (`:88`), SC and requirement evidence table (`:108`), `## Gaps` (`:131`), `## Honest caveats` (`:200`), `## Accepted risks / caveats` (`:226`) |
| T-03-50 | Denial of service | low | mitigate | CLOSED | `03-14-SUMMARY.md:179`: re-baseline 30 s to 40 s with a 50 s trigger, no check or token dropped (gate `controls=224/224`, `faults=115/115`). Local `gate-time total=27` |
| T-03-SC | Tampering (supply chain) | low | accept | CLOSED (accepted; amended 2026-11-11 to cover the local-only container apt install of tools/gate-linux.sh) | Accepted in each of the 14 plans' `<threat_model>`. Confirmed: no `FetchContent`, `ExternalProject`, `find_package`, lockfile, package manifest or vendored tree added. The optional libFuzzer path uses an existing LLVM toolchain. See the gate-linux.sh flag below for the one apt install, which is local only |

Summary: 50 numbered threats plus T-03-SC = 51 distinct ids; 51 closed, 0 open. The numbered threats are 12 high, 29 medium and 9 low. No `transfer` dispositions are declared.

## Unregistered flags

| Flag | Classification | Detail |
|---|---|---|
| `tools/gate-linux.sh` (commit `59d0286`, after 03-14 verification) | WARNING (unregistered_flag), does not block | Local, opt-in pre-push helper. It runs `apt-get install` of cmake, ninja, gcc, g++, dash, git, ca-certificates, python3 and coreutils inside a throwaway `docker run --rm` container, from an image pulled by tag (`ubuntu:24.04`, `GATE_LINUX_IMAGE` overridable), not by digest. That contradicts plan 03-14's T-03-SC text, "no package installs". Mitigating facts: not in CI (`.github/workflows/gate.yml` unchanged, not called by `tools/gate.sh`); no host mount and no env passthrough; host `.git` and history are not sent; the log goes to the ignored `build/`. Exposure: the tar carries untracked-not-ignored files, so a stray local file not covered by `.gitignore` would enter the local container (`.env` is ignored). Suggested action: amend T-03-SC in this file to record the container-only apt install as accepted, and optionally pin the image by digest |

All SUMMARY.md `## Threat Flags` sections (03-03 to 03-14) read "None" or map to existing ids. 03-01 and 03-02 have no section.

## Accepted risks log

| ID | Risk | Rationale | Revisit trigger |
|---|---|---|---|
| T-03-SC | Supply chain via package installs | No dependency is installed or vendored. The build uses cmake, ninja, cc, nm and ar. CI uses one SHA-pinned action. The local-only `tools/gate-linux.sh` installs Ubuntu apt packages inside a disposable container and never on the host or in CI | Any dependency, FetchContent or vendored source; wiring gate-linux.sh into CI |
| 03-14 caveats | The open control and test holes listed in `03-14-SUMMARY.md` § Accepted risks / caveats (no `memmap-gen-*` controls, no digest-fields golden, `len-wrap` rows only on 64-bit `size_t`, no named fault for IVSC total-below-length, single-profile case ids, and others) | Each has a compensating check named in that section. None is a missing mitigation of a registered threat | Per item, as recorded there |

## Residual risks (non-blocking, below `block_on: high`)

1. **Forged or corrupted handle.** A handle is valid on a magic match only. `ivs_load_validate` uses `ivs_profile_get(im->profile)` without a NULL check (`machine_load.c:211`, then `p->` in stages 8 and 9), and `memsys.c` indexes `ivs_mm_profiles[m->profile]` unchecked. Reset and step do check (`machine_run.c:65-68`, `:105-108`). This is reachable only if the host overwrites its own instance's profile word while leaving the magic intact, which breaks the D-01 storage contract. Suggest a defensive `p == NULL` → BAD_HANDLE in load for consistency.
2. **Caller pointer contracts are not checkable in C.** The core reads `opts` up to `opts->struct_size` bytes, the profile id up to 64 bytes and the input up to `len` bytes. Misaligned out-struct pointers (`ivs_info *`, `ivs_load_report *`) are caller UB: UBSan flags the core's member access when a probe passes one. Only storage alignment is refused. Document alignment for out structs in `ivs_machine.h`, or accept.
3. **32-bit `size_t` path unexercised.** The `len-wrap` rows are skip-ok and no 32-bit CI host exists (already accepted in 03-14).
4. **Relocated copies are valid handles by design.** `machine.iso.memcpy-relocation` copies an instance with `memcpy` and the copy is accepted as a valid handle. A host must not keep stale copies of live storage and expect them to be refused.
5. **`hygiene-history` scans only `history-commits=1` locally.** This audit independently scanned every commit in `main..HEAD` (patches and messages) and found no hits.

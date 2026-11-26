---
phase: 15-build-only-host-and-the-fake-frontend-harness
reviewed: 2026-11-26T18:40:00Z
depth: standard
files_reviewed: 20
files_reviewed_list:
  - host/ivs_retro_shim.c
  - host/libretro_shim.h
  - host/CMakeLists.txt
  - tests/host/ivs_host_harness.c
  - tools/check/host-symbols.sh
  - data/expected/host-symbols.ids
  - data/expected/host-imports.ids
  - tools/pcm/CMakeLists.txt
  - tools/check/binary-ext.txt
  - tools/check/integrity.cmake
  - docs/packaging.md
  - data/claims.json
  - tests/runner-process-tap.sh
  - tests/CMakeLists.txt
  - tools/check/steps/pkg-install.sh
  - tools/check/steps/release-assets.sh
  - tools/check/controls/files-unlisted-binary-so/CHECK
  - tools/check/controls/files-unlisted-binary-so/EXPECT
  - tools/check/controls/files-unlisted-binary-dylib/CHECK
  - tools/check/controls/files-unlisted-binary-dylib/EXPECT
findings:
  critical: 1
  warning: 4
  info: 6
  total: 11
  necessity: 0
status: issues_found
---

# Phase 15: Code Review Report

**Reviewed:** 2026-11-26T18:40:00Z
**Depth:** standard
**Files Reviewed:** 20
**Status:** issues_found

## Summary

The seam is real and most of it is proved live rather than asserted. I re-ran the evidence on this
host instead of reading the summaries:

- **Export contract.** `nm -P -g --defined-only` on both `build/dev` and `build/release` yields
  exactly the nine names of `data/expected/host-symbols.ids`;
  `tools/check/host-symbols.sh --allow-prefix __asan_ … build/{dev,release}/host/libivs_retro.dylib`
  prints `-- host-symbols pass exported=9 imports=0`, exit 0, **with no ids argument** — the default
  resolves to `$ROOT/data/expected/host-symbols.ids`, which is also the file `host/CMakeLists.txt`
  turns into the linker list, so artifact and pin cannot drift. I proved the import limb is live by
  pointing the same checker at `build/dev/tests/ivs_host_harness`: it refused `fopen`, `fread`,
  `fclose`, `dlsym`, `dlopen`, `exit` and `__stdoutp` by name, so "the host opens no file" is a
  static fact and not a promise. I proved the `host-symbols-extra` direction is live by feeding the
  checker an ids copy with `retro_run` deleted: `-- FAIL host-symbols-extra: retro_run`, exit 1.
- **The harness is not vacuous.** `build/dev/tests/ivs_host_harness build/dev/host/libivs_retro.dylib
  build/dev/images` reproduces all nine lines byte for byte from the object (`frames 13`,
  `video.plane.bytes 198432`, `audio.frames 10815`, `pixel.probes ok 198432`,
  `unload.digest.refused 1`). `--empty-poll` gives `frames 13`, `digest.images` equal to the pass
  golden and `digest.input 27da2e1f…`, so the empty-poll limb is a genuinely different observed set
  and not a second copy of the pass run. The pixel probe is what actually enforces "no palette of its
  own": it checks every delivered pixel against the 17 triples of `runner/frame_export.c`, so a copied
  and drifted palette would fail `host.frames` even though it would be invisible to the export scan.
- **Packaging.** `binary-ext.txt` carries `so dylib dll bundle`; the two committed controls plant a
  tracked `.so` and `.dylib` and both expect `files-unlisted-binary`; `pkg-install`'s `host-so-absent`
  plants `lib/libivs_retro.so` in a copy of the prefix and requires `pkg-inventory-extra`;
  `release-assets`' `so-member` plants one in an archive **whose own list names it** and additionally
  requires that neither `extra-member` nor `missing-member` fired, which is what makes the binary rule
  the load-bearing one. `host/CMakeLists.txt` has no `install(` and the eleven-path inventory is
  untouched.
- **The two container-leg fixes are right.** `POSITION_INDEPENDENT_CODE ON` on `ivs_refpcm` is the
  documented D-34 reason and cannot move the static package: `tools/pcm` has no install rule, so
  `prefix-files` stays 11. The four ELF stubs are real crt symbols, they are allowlisted by name
  rather than by a `_ITM_*` prefix (which is the stronger choice), and the release object's undefined
  set carries no sanitizer name at all (`nm -P -u build/release/host/libivs_retro.dylib` → `__memcpy_chk`,
  `__stack_chk_fail`, `__stack_chk_guard`, `bzero`, `free`, `malloc`, `memcpy`, `memmove`, `memset`,
  `realloc`), so a plain frontend can `dlopen` the release object. The dev object needs the ASan
  runtime first, which `host/CMakeLists.txt` states.

Two defects survive that the committed evidence cannot see, and both are on paths the six-event
harness never walks. **CR-01**: the input schedule is a fixed 4096-entry array that is never
compacted and never refuses; past 4096 polled events the shim silently stops polling and reports a
successful run. I proved it (a probe host returning 5000 events: `poll_calls=4096`, `run=1`, 904
events dropped with no finding). **WR-01**: a `retro_load_game` that is refused part-way (a malformed
cartridge) leaves EXEC and GROM committed, so `ivs_retro_digest` answers on a half-loaded machine and
a retry with the correct images is refused forever until `retro_deinit` + `retro_init`. I proved both
limbs.

Three findings are about the evidence machinery itself rather than the artifact: `self_control`
accepts a stand-in that fails for its intended token **and others** (WR-02), the `host-symbols-extra`
direction has no committed stand-in (WR-04), and `CL-0029`'s statement says "no EXEC image" while the
harness loads `boot-stub-2.exec` as the EXEC kind (WR-03).

Scope note: the review config names `runner/host_harness.c`; the file does not exist. The fake
frontend is `tests/host/ivs_host_harness.c`, which I reviewed in its place (and `tests/CMakeLists.txt`
confirms it). No file outside the listed scope was modified; nothing was committed.

The necessity pass (`$HOME/.pi/agent/gsd-core/references/necessity-review.md`) found no
repository-provable impossible-state guard, already-defended-elsewhere duplicate, cargo-culted
pattern, phantom requirement or uninstantiated generality in this scope, so `necessity: 0`. The
defensive bounds in the shim (`SIZE_MAX` overflow checks, the `nominal_frame_ticks == 0` division
guard) all defend values that arrive from another layer at runtime, which the rubric's trust-boundary
negative space excludes.

## Critical Issues

### CR-01: The input schedule silently truncates at 4096 events and never reclaims consumed slots

**File:** `host/ivs_retro_shim.c:31`, `:47-49`, `:307-324` (`shim_drain_poll`), `:445`
**Issue:** `sched` is a fixed `ivs_input_event sched[SHIM_EVENTS_MAX]` (4096) and `sched_n` only ever
grows: `shim_drain_poll` loops `while (shim.sched_n < SHIM_EVENTS_MAX)` and then **returns 1**, which
`retro_run` treats as success. Two consequences, both silent:

1. When the poll has more than 4096 pending changes, the surplus is dropped with no refusal, no
   counter and no stderr — the run reports success and produces a different `digest.input` than the
   script asked for. That is the exact failure mode the file's own `shim_reserve` comment refuses for
   buffers ("never a truncation").
2. `sched_n` is reset only in `retro_load_game` (`:293-294`). Consumed slots are never reclaimed, so
   the cap is on the **lifetime** count of input changes, not the outstanding ones: any frontend that
   returns one event per frame (a plausible reading of "the next change at or after now_ticks" — a
   frontend may simply report the current state) hits the cap in ~68 s at 60 Hz, after which every
   later event is dropped forever.

Proved on this host with a throwaway probe (built with `-fsanitize=address,undefined` against
`build/dev/host/libivs_retro.dylib`, since the dev object requires the ASan runtime first): a poll
returning 5000 strictly increasing events printed `poll_calls=4096  frames=33  run=1`. The shim
stopped polling and reported a successful run.
**Fix:** compact the consumed prefix before appending, and refuse rather than truncate:
```c
static int shim_drain_poll(uint64_t now_ticks)
{
  /* Reclaim the applied prefix so the cap is on OUTSTANDING events, not lifetime ones. */
  if (shim.consumed > 0u) {
    if (shim.consumed < shim.sched_n) {
      memmove(&shim.sched[0], &shim.sched[shim.consumed],
              (size_t)(shim.sched_n - shim.consumed) * sizeof shim.sched[0]);
      shim.sched_n -= shim.consumed;
    } else {
      shim.sched_n = 0u;
    }
    shim.consumed = 0u;
  }
  while (shim.sched_n < SHIM_EVENTS_MAX) {
    ivs_retro_input ev;
    memset(&ev, 0, sizeof ev);
    if (shim.host.input_poll(shim.host.ctx, now_ticks, &ev) != 1) {
      return 1;                       /* script exhausted */
    }
    if (shim.sched_n > 0u && ev.at_tick < shim.sched[shim.sched_n - 1u].at_tick) {
      return 0;                       /* poll went backwards */
    }
    memset(&shim.sched[shim.sched_n], 0, sizeof shim.sched[0]);
    shim.sched[shim.sched_n].at_tick = ev.at_tick;
    memcpy(shim.sched[shim.sched_n].controller, ev.controller, sizeof ev.controller);
    shim.sched_n++;
  }
  return 0;                           /* still full: the host has more to say than we can hold */
}
```
A committed case is needed with it: the harness's six events cannot reach the cap, so nothing in the
phase's evidence would notice a regression here.

## Warnings

### WR-01: `retro_load_game` leaves the machine half-loaded on a mid-way refusal

**File:** `host/ivs_retro_shim.c:263-306`
**Issue:** The three `ivs_machine_load` calls and `ivs_machine_reset` are committed one at a time and
every failure path is a bare `return 0`. `ivs_machine_load` is validate-then-commit per kind
(`core/src/machine/machine_load.c:339-387`), so a refusal of GROM or the cartridge leaves the earlier
kinds installed. Measured on this host with a probe (a 4-byte cartridge, everything else valid):
`load_bad_cart_first=0`, then `digest_after_refused_load=1` (a digest is answered on a half-loaded
machine) and `load_good_retry=0` (the correct images are now refused `IVS_E_DUPLICATE_LOAD`
forever). Only `retro_deinit` + `retro_init` recovers, which the header does not say. `retro_run`
does refuse (`run_after_refused_load=0`), so the machine cannot execute half-loaded — the damage is
the false digest and the permanently unusable object. The header's own words are "the core's typed
refusal is the boundary and nothing is loaded twice" (`host/libretro_shim.h:105`); the implementation
does not keep that.
**Fix:** roll the committed kinds back on every failure path (and on the reset/info/refpcm failures
after them), e.g. a single `goto refuse` that runs
`(void)ivs_machine_unload(shim.machine, IVS_KIND_CARTRIDGE); (void)ivs_machine_unload(shim.machine, IVS_KIND_GROM);
(void)ivs_machine_unload(shim.machine, IVS_KIND_EXEC);` before `return 0`, so a refused load leaves
the machine exactly as it was and a retry is meaningful. Add a `runner-process` limb that requires a
retry after a refused load to succeed.

### WR-02: `self_control` accepts a stand-in that fails for its intended token *and* others

**File:** `tests/runner-process-tap.sh:2555-2570`
**Issue:** `self_control` fails a control only when the check **accepted** the fake (`judge-vacuous`)
or when the intended token is **absent** (`judge-wrong-reason`). When the intended token is present it
sets `ok=1` and stops looking, even if `$tmp/why` carries further findings — `why()` appends without
dedup (`:275`). So a stand-in that differs from a passing output in two limbs passes as a healthy
control, which is the opposite of the property 15-02's own key decision and `CL-0029`'s note assert
("a stand-in … differ[s] from a passing output in one limb only, so a rejection for any other token is
judge-wrong-reason"). Both committed host stand-ins happen to emit exactly one token today
(`host-vacuous` returns at `frames-zero` before any digest comparison; `host-digest-diff` emits only
`digest-diff`), so the evidence is not currently wrong — the mechanism just cannot detect the failure
it claims to detect.
**Fix:** require the intended token and nothing else:
```sh
elif grep -q -e "^$_tok: " "$tmp/why" \
     && [ "$(sed -n 's/^\([a-z0-9-]*\):.*/\1/p' "$tmp/why" | sort -u | wc -l)" -eq 1 ]; then
  ok=1
```
(or compare `sort -u` of the token set against `printf '%s\n' "$_tok"`).

### WR-03: `CL-0029` states "no EXEC image" while the harness loads `boot-stub-2.exec` as the EXEC kind

**File:** `data/claims.json:707`
**Issue:** The claim says the frontend "loads the project's own `lamp-tone.ivsc` (built from the
project's own sources, with the zero graphics ROM and no EXEC image)". The harness reads
`boot-stub-2.exec`, requires it to be exactly 8192 bytes, and passes it as `imgs.exec`
(`tests/host/ivs_host_harness.c:376-400`), and `retro_load_game` loads it with `IVS_KIND_EXEC`. The
project's convention makes "no EXEC image" mean "no console EXEC ROM" — but the sibling claims say
so explicitly ("the project boot stub contract 2 … and no EXEC image or GROM image of any console",
`data/claims.json:448`, `:595`), and this one drops both the qualifier and the boot stub. Read
literally, a minted claim misstates the inputs of its own artifact.
**Fix:** match the sibling wording: "through the project boot stub contract 2 (`boot-stub-2`), the
project's own zero graphics ROM and no EXEC or GROM image of any console, loads the project's own
`lamp-tone.ivsc`…".

### WR-04: The `host-symbols-extra` direction has no stand-in, so half of the exact-set comparison is unproved

**File:** `tests/runner-process-tap.sh:2508-2526`
**Issue:** The `host.exports` case proves only the *missing* direction live (it appends
`ivs_not_defined` to a scratch ids copy and requires `-- FAIL host-symbols-missing`). The *extra*
direction — an exported name the list does not carry — is never shown able to fail, and `AGENTS.md`
requires a stand-in proving each new equivalence guard can fail. The direction is reachable and it
works: `tools/check/host-symbols.sh` against an ids copy with `retro_run` deleted printed
`-- FAIL host-symbols-extra: retro_run`, exit 1. So the gap is in the case, not in the checker.
**Fix:** add a second limb in the same case, using the ids copy the case already builds:
```sh
awk '/^#/ { next } NF { print $1 }' "$root/data/expected/host-symbols.ids" | grep -v -x -F retro_run > "$tmp/host-short.ids"
if CHECK_DIR="$root/tools/check" ROOT="$root" sh "$check_sym" --allow-prefix __asan_ --allow-prefix __ubsan_ \
  --allow-prefix __sanitizer_ "$shim" "$tmp/host-short.ids" > "$tmp/runs/host-symbols-short.out" 2>&1; then
  ok=0; why host-exports-live 'the checker accepted a list missing a name the object exports'
elif ! grep -q -x -F -e '-- FAIL host-symbols-extra: retro_run' "$tmp/runs/host-symbols-short.out"; then
  ok=0; why host-exports-live "not refused by the dropped name: $(grep -m 1 -e 'FAIL' "$tmp/runs/host-symbols-short.out")"
fi
```

## Info

### IN-01: `shim_deliver_video` trusts the slot geometry and drops a partial slot

**File:** `host/ivs_retro_shim.c:337-357`
**Issue:** `frames = bytes / video_slot_bytes` silently discards a remainder, and the plane read
`plane = slot + IVS_FRAME_HEADER_BYTES` then reads `w * h` bytes without checking
`video_slot_bytes >= IVS_FRAME_HEADER_BYTES + w*h`. Both hold today (15296 = 32 + 159·96) but they are
asserted nowhere, so a profile whose slot geometry changed would read past the last slot of
`shim.video` instead of refusing.
**Fix:** before the loop, `if (shim.info.video_slot_bytes < IVS_FRAME_HEADER_BYTES + npix || bytes % shim.info.video_slot_bytes != 0u) return 0;`.

### IN-02: `q.event_count` can underflow if the core reports more consumed events than it was given

**File:** `host/ivs_retro_shim.c:444-452`
**Issue:** `q.event_count = shim.sched_n - shim.consumed` is unsigned arithmetic over a count the core
reports. If `rep.input_consumed` ever exceeded the remaining count, the next call passes a huge
`event_count` (with `q.events == NULL` once `consumed >= sched_n`), which the core would read.
**Fix:** clamp after the run: `shim.consumed += rep.input_consumed; if (shim.consumed > shim.sched_n) shim.consumed = shim.sched_n;` and refuse the run when `rep.input_consumed > q.event_count`.

### IN-03: `SHIM_SLOTS_MAX` bounds slot count, not bytes, so a hostile target can request ~36 GB

**File:** `host/ivs_retro_shim.c:36`, `:418-431`
**Issue:** `slots` is capped at `1 << 20`, but `log_need = 16 · 2152 · slots` reaches ~34 GB and
`video_need` ~16 GB before `realloc` is even attempted. The refusal happens (allocation fails), so
this is not a crash, but the guard the comment promises ("a hostile target cannot wrap the capacity
arithmetic") is only about wrapping, and the first line of defence is the allocator.
**Fix:** cap the reservation itself, e.g. refuse when `log_need + audio_need + video_need` exceeds a
named `SHIM_BYTES_MAX`, and lower `SHIM_SLOTS_MAX` to a figure derived from a sane maximum call
length rather than from what fits in a `uint64_t`.

### IN-04: `exported=` in the pass line counts the ids file, not the object

**File:** `tools/check/host-symbols.sh:163`
**Issue:** `printf … "$(grep -c . "$tmp/want")"` prints the number of names **expected**, not the
number the object defines (`$tmp/have`). The comparison still fails correctly in both directions, but
the token a reader of the gate line sees is the pin's count, so `exported=9` is not evidence of what
the artifact exported. That is the "recompute, never quote" rule applied to a printed figure.
**Fix:** print `"$(grep -c . "$tmp/have")"` (or both, `exported=<have> expected=<want>`).

### IN-05: The four ELF stub names are allowlisted with no check that they are weak-undefined

**File:** `data/expected/host-imports.ids:22-31`
**Issue:** The comment states each name "is `weak undefined`, so a build without them still links",
and that property is what makes listing them safe. Nothing verifies it: `nm -P -u` does not
distinguish weak from strong, so if the shim ever acquired a **strong** dependency on
`__cxa_finalize` or `__gmon_start__` (a Linux `ld` links a shared object with undefined strong
symbols unless `-z defs` is passed), the allowlist would hide it. Low risk — none of the four is a
content route, clock, environment, thread or libm name — but the stated reason is unchecked.
**Fix:** either assert the weakness (`nm -P -u` prints `w`/`v` in the second field on ELF; refuse a
strong `U` for these four names) or drop the "weak" justification from the comment and record them as
accepted crt stubs.

### IN-06: `retro_run` can deliver a truncated stream, which the header says it never does

**File:** `host/libretro_shim.h:108-112`, `host/ivs_retro_shim.c:446-456`
**Issue:** The header promises "a refused or internal-failure run records nothing, never a truncated
delivery". In the implementation `shim.consumed` is advanced **before** delivery, and
`shim_deliver_video` runs (calling `video_refresh` per frame) before `shim_deliver_audio`, which can
still return 0 (a `realloc` failure, or `IVS_REFPCM_E_LEVEL` if the core ever emitted a level byte
above 15). The caller then gets 0 with some frames already handed over and the events already marked
consumed, so the call cannot be retried. Reachable only on allocator failure or a core invariant
break.
**Fix:** either amend the header to state that the frames completed before a delivery-stage failure
have been delivered, or stage the audio result first and deliver both streams only after every stage
succeeded.

---

_Reviewed: 2026-11-26T18:40:00Z_
_Reviewer: Claude (gsd-code-reviewer)_
_Depth: standard_

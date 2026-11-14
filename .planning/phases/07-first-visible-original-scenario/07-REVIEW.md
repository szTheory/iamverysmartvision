---
phase: 07-first-visible-original-scenario
reviewed: 2026-11-13T21:50:00Z
depth: standard
files_reviewed: 25
files_reviewed_list:
  - tools/asm/asm.c
  - tools/asm/CMakeLists.txt
  - runner/exec.c
  - runner/main.c
  - runner/report.c
  - runner/scenario.c
  - runner/ivs_runner.h
  - tests/runner/runner_exec.c
  - tests/runner/runner_fuzz.c
  - tests/runner/runner_parse.c
  - tests/run/run_cases.h
  - tests/run/run_main.c
  - tests/run/run_stub2.c
  - tests/run/stub2-probe.asm
  - tests/installed-consumer/main.c
  - tools/owner-run.sh
  - tests/asm-tap.sh
  - tests/runner-process-tap.sh
  - tools/check/steps/c-build.sh
  - tools/check/steps/golden.sh
  - tools/check/steps/important-gates.sh
  - tools/check/steps/pkg-install.sh
  - tools/check/steps/tests-inventory.sh
  - tools/check/steps/text.sh
  - tools/check/steps/video.sh
findings:
  critical: 0
  warning: 2
  info: 2
  total: 4
status: fixed
fixed: 2026-11-13
fixes:
  WR-01: 4fa3214
  WR-02: b9d0c7b
  IN-01: 34a656a
  IN-02: 4fa3214
---

# Phase 7: Code Review Report

**Reviewed:** 2026-11-13T21:50:00Z
**Depth:** standard (asm.c read in full and exercised; owner-run.sh read in full and exercised)
**Files Reviewed:** 25
**Status:** fixed (all four findings, see Fix notes)

## Summary

Scope was the diff of `gsd/phase-7-first-visible-original-scenario` against the merge base, limited to `tools/asm`, `runner`, `tests`, `tools/owner-run.sh` and `tools/check/steps`. `.planning/`, `data/`, `fixtures/` and `docs/` were not reviewed, and the frozen hand-derived oracle was not questioned. The two deferred items (owner-run symlink-in-refs gap, trust-on-first-use `--record`) and the audio-window deviation were not re-reported.

No Critical issue was found. The assembler core is sound. Checks run, all in a scratch directory outside the repo build trees:

- `tools/asm/asm.c` built with `clang -std=c17 -Wall -Wextra -Wconversion -Wshadow -Wstrict-prototypes -Wformat=2 -fsanitize=address,undefined`: no warning.
- 6,000 byte-mutated sources (twins, fixtures, positive cases) and 1,500 mutated pairs files were run under ASan+UBSan: no crash, hang, or sanitizer report, and every exit status was 0 or 1.
- The encoding table, branch displacement formulas, the SDBD immediate order (low byte word first), the JSR/JE/JD second decle and the boundary cases (word at FFFF, branch at FFFE, 256 and 257 symbols, 31 and 32 character names) were exercised by hand and match the comments and the manual pages cited in the source.
- `dash -n` is clean on every changed shell file and `shellcheck -s sh` reports nothing beyond intentional patterns. `owner-run.sh`, `text.sh`, `golden.sh` and `tests-inventory.sh` use no bashism and nothing that needs GNU userland.
- No absolute home path and no binary file enter the diff.
- `runner/main.c`: the new `--audio-raw`, `--psg-log-out` and `--audio-wav` refusal runs after parse and before any storage or output file is created. It frees `scn` on the refusal path and names the options in the documented fixed order.

Two real defects remain (one silent miscompile in the assembler, one fail-open path in the owner-image guard) and two minor ones.

Necessity (#4100) was assessed at standard depth; no necessity finding is reported.

## Warnings

### WR-01: A label between `SDBD` and its instruction silently binds past the prefix

**File:** `tools/asm/asm.c:797-808` (label definition in `as_stmt`), `tools/asm/asm.c:824-825` (the SDBD target rule it bypasses)
**Issue:** `as_stmt` defines a label at the current `pc` without looking at `as_g.sdbd`. The target rule at line 824 refuses `ORG`, `DECLE`, `EQU`, a second `SDBD` and non-immediate rows after an `SDBD`, but a label on the target line (or alone on its own line) is not refused. The label then points at the instruction after the prefix, not at the `SDBD`. Reproduced (assembles, exit 0):

```
        ORG $5000
        SDBD
loop:   MVII #$1234, R0
        DECR R1
        BNZE loop
```
The listing has `# sym: loop 5001` and `BNZE $5001 -> 5001`. After the first pass, every later iteration enters at `MVII` without the prefix. R0 takes only the low-byte word (`0034`) and the high-byte word `0012` then executes as an instruction. The result is silent wrong code that no `ivs_asm` refusal, listing check or word comparison flags, because the generated words are exactly what the source says. A label alone on the line after `SDBD` has the same effect. The same label-before-the-pc-moves pattern applies to `lbl: ORG $xxxx`, which binds to the address before the fill gap.
**Fix:** refuse in pass 1 before `as_define`. The existing `asm-sdbd-target` token needs no new pinned token, and `tests/asm/neg` plus `data/expected/asm-tokens.ids` stay in step.

```c
if (pass == 1u && !as_g.have_org) {
  as_fail("asm-org", "ORG must come before the first label: ", label);
} else if (pass == 1u && as_g.sdbd) {
  as_fail("asm-sdbd-target", "a label may not sit between SDBD and its instruction: ", label);
} else if (pass == 1u) {
  as_define(label, as_g.pc, 1);
}
```
Add one `asm.var.*` case, for example `sdbd-label|asm-sdbd-target|        SDBD\nlbl:    MVII  #$1234, R0`, to `tests/asm-tap.sh`.

### WR-02: The owner-run work-tree guard fails open when `cd -P` rejects the directory

**File:** `tools/owner-run.sh:36-38` (also `:19` for the root lookup)
**Issue:** the guard computes the physical directory with `d=$(cd -P "${d:-/}" 2> /dev/null && pwd -P) || d=` and then matches `$d/` against `$root/`. Any `cd` failure leaves `d` empty, which matches no pattern, so the image passes the "inside the work tree outside refs/" check. One concrete trigger is a relative image path whose directory begins with `-`: `cd -P "-x"` is parsed as an option. Confirmed in `/bin/sh` (bash), dash and bash on this host:

```
p=-x/e.bin; d=${p%/*}; d=$(cd -P "${d:-/}" 2>/dev/null && pwd -P) || d=   # d is empty
cd -P -- "-x"                                                             # resolves
```
So `sh tools/owner-run.sh -x/e.bin -x/g.bin` run from inside the work tree is accepted and the owner images stay in the tree, outside the ignored `refs/`. The option loop stops at the first non-option, so `-x/e.bin` reaches this code. An unsearchable directory fails open the same way, although the later `sized` check then usually refuses. This is the kind of careless in-tree placement the guard exists to refuse (T-07-26).
**Fix:** end option parsing for `cd` and make a `cd` failure a refusal, not a pass.

```sh
d=$(cd -P -- "${d:-/}" 2> /dev/null && pwd -P) || continue   # missing directory: sized refuses it below
```
Keep the `|| d=` fall-through only for the documented "directory does not exist" case, which `sized` already turns into the exact-size refusal. If any `-` leading name should be refused outright, add `case $p in -*) die "bad image path: $p" ;; esac` before the loop. The same `--` applies to `root=$(cd -P "$here/.." ...)` on line 19, where `here` comes from `$0`.

## Info

### IN-01: `ivs-run --help` does not mention the new audio-output refusal

**File:** `runner/main.c:80-93` (`main_usage`)
**Issue:** `--video-raw` states "refused for `content <role> user`", and `--update-golden` states the same for `user` content. The three options that gained the refusal in this phase (`--audio-raw`, `--psg-log-out`, `--audio-wav`, refusal at `runner/main.c:707-720`) say nothing about it. The one-line `ivs-run: --audio-raw refused: content user` is then the first an owner learns of the rule. `docs/runner.md` was updated, the in-binary help was not.
**Fix:** add "refused for `content <role> user`" to the `--audio-raw` and `--audio-wav` paragraphs, and a note to `--psg-log-out`.

### IN-02: A long listing path is refused with the usage text for the source path

**File:** `tools/asm/asm.c:1089-1094`
**Issue:** `n > AS_MAX_PATH` is applied to the listing path as well as the source path, only because `tmp[AS_MAX_PATH + 8u]` is sized from it. The listing prints only the source path (the header comment says so), so the 200 byte cap on the output path is purely a buffer limit. With a long `TMPDIR` (the asm suite builds `$TMPDIR/asm-tap.XXXXXX/…` paths) every `asm.*` TAP case fails with the usage banner, which names only the source path rules. The failure is unrelated to the real cause and easy to misread.
**Fix:** print a distinct line for the listing path, for example `ivs_asm: listing path above 200 bytes`, or size `tmp` from `strlen(argv[2]) + 5u` on the heap.

## Fix notes

All four findings fixed on 2026-11-13; the gate was green under sh and dash on the fixed tree (identical summary line, `controls=357/357`, `tests-expected=4628`, 0 FAIL lines) and in the Ubuntu container.

- **WR-01 fixed (4fa3214).** Pass 1 of `as_stmt` refuses a label defined while an `SDBD` is pending with `asm-sdbd-target` ("a label may not sit between SDBD and its instruction"), before `as_define`, exactly as suggested. A label on the `SDBD` line itself (`init:   SDBD`, used by the Lamp Tone program) is still accepted and no committed listing changed (the asm.regen cases and the manifests step confirm it). Two refusal cases pin it: `asm.var.sdbd-label` (label on the target line) and `asm.var.sdbd-label-alone` (label alone on its own line), so `data/expected/tests/asm.ids` has two more ids (`tests-expected` 4624 to 4628, two presets). `asm.c` changed, so the recipe digests of `fixtures/FX-boot-stub-2/manifest.json` and `fixtures/FX-demo-lamp-tone-1/manifest.json` were refreshed in the same commit (outputs untouched); `docs/diagnostics.md` names the rule. Not changed: the `lbl: ORG $xxxx` remark in the finding (the label binds to the address before the fill gap, which is the documented ORG behaviour and was not part of the requested fix).
- **WR-02 fixed (b9d0c7b).** `cd -P --` at all three lookups (repository root, image directory, TMPDIR copy). A directory that cannot be entered now `continue`s to the exact-size check, which refuses the image (no file can be read in a directory that cannot be entered), instead of leaving `d` empty and passing. Added beyond the finding: `unset CDPATH`, because a set CDPATH made `cd` resolve (and print) a different directory for a relative path, the same fail-open class. `runner-process.owner-run.in-tree-path` gained the `-x/e.bin` case run from inside the scratch root (the old script answered "no lock" there, the fixed one names the in-tree refusal) and `wrong-size` gained an image in a missing directory; no new test ids.
- **IN-01 fixed (34a656a).** `ivs-run --help` now says "refused for `content <role> user`" under `--audio-raw` and `--audio-wav` and notes it under `--psg-log-out`. No help golden exists (only the `outcomes (` section is compared with `docs/runner.md`); the usage literal is 4054 characters, under the 4095 portable limit.
- **IN-02 fixed (4fa3214).** A listing path above 200 bytes with a valid source path prints `ivs_asm: the listing path is longer than 200 bytes` and exits 1; the usage banner remains for every other bad argument. No expected-output file changed (the message carries no token).

---

_Reviewed: 2026-11-13T21:50:00Z_
_Reviewer: Claude (gsd-code-reviewer)_
_Depth: standard_

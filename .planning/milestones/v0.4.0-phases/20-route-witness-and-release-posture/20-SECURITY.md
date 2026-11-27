---
phase: "20"
slug: "route-witness-and-release-posture"
status: secured
# threats_open = count of OPEN threats at or above workflow.security_block_on severity (high).
threats_open: 0
asvs_level: 1
created: "2026-11-27"
audited: "2026-11-27T07:55Z"
auditor: "gsd-security-auditor (pi, subagent)"
---

# Phase 20 -- Security

> Per-phase security contract: threat register, accepted risks, and audit trail.

**Verdict: SECURED -- 14 register rows across two plans, 13 distinct threat ids, 13 closed, 0 open.**
`block_on: high`, `asvs_level: 1`. Every row in the register is `high` except two `medium` rows
(`T-20-05`, `T-20-06`); all thirteen are closed by a command run in this session against the committed
tree, not by reading a SUMMARY. The two plans each carry a `T-20-SC` row: it is one threat id and is
counted once.

## Basis of this audit

This phase's threat surface is **evidence integrity, not attack surface**. It adds no code: two document
sections, one appended release note, one added bullet, one ledger row, one anchor sentence and one
recomputed release line. It adds no core sink, no network, no install rule, no release membership, no
dependency, no parser, and no test id. The risks it carries are therefore the inverted ones the two plans
named: an operator section whose command lines do not execute as written, a frozen page silently rewritten
by the section that extends it, an instrument output escaping `refs/` into the work tree, a naming limb
widening the rights ledger, a command surface that grows a spelling the frozen block does not name, a
tripwire that cannot fail, a packet that leaks an operator file's identity, a claim that acquires a
citation to the handoff row, a ledger row landing where it is judged inconsistently, a comparison row that
drifts from Phase 19's measurement, an append-only record rewritten, and a release line quoted rather than
recomputed.

Evidence classes, used consistently below:

- **live** -- a command run in this session against the committed tree. The only files written were the
  four `refs/op-*` outputs the page itself tells an operator to remove (removed), two throwaway archives
  under `/tmp` (removed), and one scratch control tree under `/tmp` (removed). No tracked file was modified
  by this audit.
- **commit** -- a `git diff`/`git show` measurement against the phase start commit `57f9b31` or the page's
  freeze commit `0627be92`.
- **suite** -- the committed control or step re-run now, whose output I read from this run.

Final state measurement, taken on the tree this record is committed to, before this file existed:

```
sh tools/gate.sh          -> rc 0, `grep -c '^-- FAIL '` = 0, `gate: PASS`
sh tools/hygiene.sh --tree -> rc 0, `-- hygiene pass mode=tree hygiene-files=2860`
git ls-files | grep -E '\.(ivsc|bin|rom|so|dylib|cfg|ivsn|ivst)$' -> 0
```

The gate's own line, with the phase's invariants recomputed from the files that produce them rather than
quoted:

```
steps=38/38 controls=405/405 faults=238/238 goldens=29 claims=29 provisional=28 handoffs=9
gates=123 lines=378 prefix-files=11 rights-pinned=4/4 tests-expected=5114 tests-executed=5108
binaries-listed=0 release-selftest=13/13 pkg-selftest=21/21 d10-items=47
```

Cross-checked directly, each read from the file that produces it:

```
ls tools/check/steps/*.sh | wc -l                                   -> 38
ls -d tools/check/controls/*/ | grep -v _good | wc -l               -> 405
ls data/golden | wc -l                                              -> 29
grep -v '^#' data/expected/handoffs.ids | awk NF | wc -l            -> 9
grep -v '^#' data/expected/rights-pinned.ids | awk NF | wc -l       -> 4
grep -v '^#' data/expected/d10-measurements.ids | awk NF | wc -l    -> 47
ROOT=$PWD CHECK_DIR=$PWD/tools/check sh tools/check/steps/claims.sh -> claims=29 provisional=28 handoffs=9
```

Every one matches the line, so no figure below is a quotation. The phase's one move is `handoffs` 8 -> 9;
`tracked`/`hygiene-files` read 2860 because the verifier's `20-VERIFICATION.md` is present and untracked on
the tree the gate ran on, and this record adds one more tracked file after it.

**The container gate (`tools/gate-linux.sh`) was not run and is not claimed.** Measured now:
`timeout 10 docker info` -> rc 124 (no answer), and `build/gate-linux.log` is a 0-byte file dated
2026-11-26, so no `gate: PASS` line exists to read. No `ivs-*` image, container or volume was created.
Recorded as **outstanding evidence, never a pass** (lesson 24). No threat below depends on it: every
mechanism is a markdown assertion, a `tar -tzf` reading, a POSIX shell predicate or a CMake step that runs
identically on the Linux image, and every id cited below ran under the local gate on this host.

## Trust Boundaries

Union of the two plans' boundaries; no boundary is added by this audit.

| Boundary | Description | Data crossing |
|---|---|---|
| the page to the person driving the tool | a command line that needs a local tweak is a defect the reader cannot fix; the page is the interface, so a wrong line is a wrong interface | thirteen command lines, their option tokens, the shapes they print |
| the appended section to the frozen contract | the page is its own oracle: an appended section that silently redefines a frozen sentence would supersede the contract it extends | `## Command surface`, `## The trace file`, the five refusal tokens |
| the operator to the work tree | the tool writes files derived from whatever the scenario loaded; a path inside the tree but outside `refs/` puts a rights-bearing artifact where a staging command can reach it | trace records, a break report, a disassembly, a report text file |
| the naming limb to the rights ledger | the four pinned pairs are the whole permitted surface; one new spelling widens what a rights scan must accept, and one lost pair makes the pin stale | `exec-image`, `grom-image` and the page that may name them |
| the packet to the owner's private files | the packet is a shared document; anything it carried about an operator file -- a digest, a size, a path, a name -- would leave the owner's machine | trace field names, profile keys, report clauses |
| the packet to the claim ledger | a row is a handoff, never evidence; the moment a claim can cite it, an unanswered question becomes a cited artifact | `data/expected/handoffs.ids`, `data/claims.json` |
| the ledger to the gate | the row set and the disposition set are pinned in a script and judged against six trees; a set that moves in one place takes the control run red or green for the wrong reason | `HANDOFF_IDS`, `HANDOFF_DISP`, five control-tree ledgers |
| the release text to what ships | a line written from a record rather than from the artifacts describes an earlier tree; the archives and the checks are the only things that can be recomputed | eleven install members, 2859 source members, five zero-counts |

## Threat Register

| Threat ID | Category | Component | Severity | Disposition | Mitigation (as built) | Status |
|-----------|----------|-----------|----------|-------------|-----------------------|--------|
| T-20-01 | Tampering | `docs/inspection.md`'s appended `## The operator route` | high | mitigate | all 13 lines extracted from the one `sh` fence tag and executed verbatim, rc 0, every stated shape asserted on the output; a constructed wrong line rejected by the same check | **closed** |
| T-20-02 | Tampering | the frozen sections of `docs/inspection.md` | high | mitigate | the section is the page's last `## ` heading; `git diff --numstat 0627be92..HEAD` = `188 0`, zero removed lines; freeze commit an ancestor of `HEAD` | **closed** |
| T-20-03 | Information disclosure | `refs/`, the four output files, the work tree | high | mitigate | all three `--out` paths (and the shell redirect) under `refs/`; both refusal tokens re-demonstrated live; zero tracked `.ivst`/`refs/` paths; the four files removed, `refs/` restored to its 13 pre-existing entries | **closed** |
| T-20-04 | Tampering | `data/expected/rights-pinned.ids` and `rights_scan_naming` | high | mitigate | pair count exactly 4, both `docs/owner-images.md` pairs present by exact text, zero added lines carrying either pinned spelling, the two rights files untouched by the phase | **closed** |
| T-20-05 | Spoofing | the operator section's command surface | medium | mitigate | the eight `ivs-inspect` option tokens used are a subset of the frozen block's ten; the three non-inspect tokens are exactly `--build`/`--preset`/`--target`; all four subcommands named in the frozen block | **closed** |
| T-20-06 | Repudiation | the claim-word tripwire and the additions-only predicate | medium | mitigate | both re-run now on constructed wrong examples: a claim word flagged (1 vs 0 on the correct line), a one-line deletion seen (removed=1), a wrong command line rejected (rc 2) | **closed** |
| T-20-07 | Information disclosure | `docs/handoffs.md`'s packet section | high | mitigate | the 45-line packet carries 0 `refs/` paths, 0 64-hex strings, 0 content extensions, 0 pinned rights spellings, and states the no-leak rule in its own words | **closed** |
| T-20-08 | Elevation of privilege | `data/claims.json` and the claim-artifact predicate | high | mitigate | 0 claims name `handoffs.ids`, 0 name `D10-VID-08`, `claims=29 provisional=28`; the committed control refuses `CL-0005` by `claim-handoff-artifact` alone, and a repointed copy stops being refused while seven others stay refused | **closed** |
| T-20-09 | Repudiation | `data/expected/handoffs.ids`, the two pinned sets in `claims.sh`, the five control-tree ledgers | high | mitigate | one commit `e10c45f` carries the row, both pinned lists and all five trees; `handoffs=9`; each of the five trees holds the id once; the three grammar controls are untouched by the phase | **closed** |
| T-20-10 | Repudiation | the four comparison rows and the status line | high | mitigate | each row carries its guest id, its two candidate values, its witness and its mask, compared programmatically against Phase 19's committed measurement; the status line carries all four masks and does not carry the forbidden reading; constructed wrong mask and wrong pair both rejected | **closed** |
| T-20-11 | Tampering | `.planning/ops/RELEASE-OPS.md` (i)/(i.1), `docs/packaging.md` | high | mitigate | `(i.2)` sits between `(i.1)` and `(j)`; the phase diff is `43 0` and `1 0` with zero removed lines, so the earlier notes are quoted rather than edited, and `(i.2)` says what it supersedes | **closed** |
| T-20-12 | Repudiation | the release-decision line | high | mitigate | a fresh `tools/release.sh --no-gate` archive pair, then every clause re-derived: 2859 source members = `git ls-files`, 11 install members identical to the pinned inventory in both directions, five prefix counts and five zero-counts; constructed wrong member list and clause-less line both rejected | **closed** |
| T-20-SC | Tampering | npm/pip/cargo installs (both plans) | high | mitigate | not applicable and asserted as such: the phase's 15 changed files are markdown, two `.ids` files and one shell script; no manifest or lockfile moved and no package manager is named in any of them | **closed** |

*Status: open - closed - open below `block_on` threshold (non-blocking).*
*Severity: critical > high > medium > low -- only open threats at or above `block_on` count toward `threats_open`.*
*`T-20-SC` appears as a row in both plans' registers; it is one threat id and is counted once.*

## Per-threat verification, with the command that closes it

### T-20-01 (high) -- an operator section whose lines do not execute as written

*Mitigation as built.* The section is seven `sh` blocks holding thirteen command lines, each line free of
continuation and placeholder; the same task's verify extracts them with one `awk` on the single fence tag and
runs them with `sh -e`, so a line needing a local tweak exits non-zero instead of being proofread.

*Command and result (live).*

```
awk '/^## The operator route/{s=1} s && /^```sh$/{f=1;next} s && /^```$/{f=0;next} f && NF' docs/inspection.md > /tmp/a20-ops.sh
grep -c . /tmp/a20-ops.sh   -> 13
sh -e /tmp/a20-ops.sh       -> rc 0
```

The stated shapes, asserted on what that run printed:

```
sed -n '1p;$p' refs/op-trace.report   -> ivsr 3 / report.end 78
grep -c . refs/op-trace.report        -> 79          (report.end = line count - 1 = 78)
sed -n '1p' refs/op-trace.ivst        -> tick=4 kind=iab addr=0000 value=1000 wait=0 cycle=1 unit_start=0 unit_end=20
sed -n '$p' refs/op-trace.ivst        -> end tick=2084 records=112 units=70 steps=1 break_pc=1000 break_hits=1 other_pc=1002 other_hits=0
grep -c '^tick=' refs/op-trace.ivst   -> 112         (equals the end record's records=)
step record end=                      -> 2084        (equals the end record's tick= and report step.1.now_ticks)
awk '/^break\./{n++} END{print n}' refs/op-break.txt -> 7  (seven pinned keys, break.hits 1, break.other.hits 0)
sed -n '1p' refs/op-dis.asm           -> # id: boot-stub; 8 header keys; ORG $1000; last line `J $5000`
step invocation printed               -> 13 lines, step.units 70 .. step.mem.0200 7FFF
```

The `mkdir -p refs` line is load-bearing, and the page states the consequence it prevents. Re-demonstrated
live from a directory inside the work tree that has no `refs/`:

```
cd tools && ../build/dev/tools/inspect/ivs-inspect trace ../data/scenarios/diag-boot-pass.ivss \
  --content-dir ../build/dev/images --out refs/a20-nodir.ivst
  -> rc 2, "ivs-inspect: cannot resolve the output directory refs", no file created
```

The negative limb: a constructed section whose line is
`build/dev/tools/inspect/ivs-inspect nosuchsubcommand` is extracted to a one-line file and **rejected**
(rc 2), so the check cannot pass vacuously. A differently tagged fence (` ```bash `) extracts to **0** lines
and would have executed successfully while asserting nothing -- the reason the tag is pinned.

The owner-local half of the same route was checked too: `docs/owner-images.md` names the target
`ivs_inspect` and the path `build/release/tools/inspect/ivs-inspect`, that binary is executable, and it
traces the project's own scenario to an `end` record (`end tick=2084 records=112 units=70 steps=1
break_pc=none break_hits=0 other_pc=none other_hits=0`), so the page names a path that works rather than a
plausible one.

### T-20-02 (high) -- a frozen sentence moved by the section that extends it

*Mitigation as built.* The section is appended after `## Amendments` and after all six notes, and is the
page's last `## ` heading; the diff from the freeze commit is asserted to remove no line.

*Command and result (commit).*

```
git merge-base --is-ancestor 0627be92 HEAD                     -> ok
grep -n '^## ' docs/inspection.md | tail -2                    -> 255:## Amendments / 332:## The operator route
git diff --numstat 0627be92..HEAD -- docs/inspection.md        -> 188  0
git diff 0627be92..HEAD -- docs/inspection.md | grep -c '^-[^-]' -> 0
```

The position assertion is what makes an insertion mid-page a failure rather than a style note: the section
line number equals the line number of the last `## ` heading.

### T-20-03 (high) -- an instrument output escaping `refs/` into the work tree

*Mitigation as built.* Every `--out` in the section lies under `refs/`, the section states the two frozen
tokens as the reason, and the four files are removed after the run.

*Command and result (live).*

```
grep -o -E '\-\-out [^ ]+' /tmp/a20-ops.sh   -> refs/op-trace.ivst / refs/op-break.txt / refs/op-dis.asm
                                                (the fourth output is `> refs/op-trace.report`)
--out build/a20-probe.ivst                   -> rc 1, "inspect-output-tree: the output build/a20-probe.ivst
                                                is inside the work tree outside refs/", no file left behind
--out refs/op-trace.ivst (second run)        -> rc 1, "inspect-output-exists: the output refs/op-trace.ivst already exists"
git ls-files | grep -cE '\.ivst$|(^|/)refs/' -> 0
rm -f refs/op-trace.ivst refs/op-trace.report refs/op-break.txt refs/op-dis.asm
ls refs | grep -c '^op-'                     -> 0 ; ls refs | wc -l -> 13 (the pre-existing owner directory, intact)
git status --porcelain                       -> only `?? .planning/phases/20-.../20-VERIFICATION.md`
```

The single porcelain entry is the verifier's own untracked record, present before this audit ran; nothing
this audit wrote is tracked or left behind. The containment paragraph is at `docs/inspection.md:429-432`
and cites `inspect-output-tree` (`:200`) and `inspect-output-exists` (`:205`), both still frozen.

### T-20-04 (high) -- the naming limb widening the rights ledger

*Mitigation as built.* The pair count is asserted at exactly four, both `docs/owner-images.md` pairs are
asserted present by exact text, and no added line carries either pinned spelling.

*Command and result (live + commit).*

```
grep -v '^#' data/expected/rights-pinned.ids | awk NF | wc -l        -> 4
grep -x -F 'exec-image docs/owner-images.md' data/expected/rights-pinned.ids -> present (:17)
grep -x -F 'grom-image docs/owner-images.md' data/expected/rights-pinned.ids -> present (:19)
git diff --name-only 57f9b31..HEAD -- data/expected/rights-pinned.ids data/rights data/expected/rights.ids -> (none)
git diff 57f9b31..HEAD -- docs/inspection.md docs/owner-images.md | grep '^+'
  | grep -c -F -e 'exec-image' -e 'grom-image'                       -> 0
gate line                                                           -> rights-pinned=4/4
```

The limb widened by one page (the instrument section, which names the target, the path and the containment)
and by **zero** rights tokens: the section uses the project's own scenario and needs no owner file.

### T-20-05 (medium) -- a command surface that grows an unnamed spelling

*Mitigation as built.* Every subcommand and option token used is compared, as a set, against the frozen
`## Command surface` block; the three cmake tokens the build lines carry are held to an explicit allow-list
rather than being folded into that comparison.

*Command and result (live).*

```
frozen block's option tokens           -> --break --content-dir --help --other --out --pc --peek --report --trace --units
section's ivs-inspect option tokens    -> --break --content-dir --other --out --pc --peek --trace --units
comm -13 frozen section                -> (empty)
section's non-inspect option tokens    -> --build --preset --target   ; comm -13 against the allow-list -> (empty)
subcommands used -> break 1 / dis 1 / step 1 / trace 1, each present in the frozen block
```

The scoping narrows one comparison instead of opening a hole: a fourth option token outside an
`ivs-inspect` invocation fails the same limb, which is exactly what the constructed `--werror` negative in
the plan's verify block asserts.

### T-20-06 (medium) -- a tripwire that cannot fail

*Mitigation as built.* Both predicates are shown to fail on a constructed wrong example in the same task
that relies on them (lesson 20, lesson 34), and both were re-run here.

*Command and result (live).*

```
claim-word tripwire on `the tool runs as written`      -> 1 match  (flagged)
claim-word tripwire on `the tool executes as written`  -> 0 matches (accepted)
additions-only predicate on a one-line deletion of `docs/inspection.md` -> removed=1 (seen)
extraction-and-execute on a constructed wrong section  -> rejected, rc 2
```

The tripwire's own list and its two exemptions (`[nc]`, `[claim:CL-NNNN]`) live in
`tools/check/steps/text.sh` and are unchanged by the phase; the phase's added lines carry zero claim words.

### T-20-07 (high) -- the packet leaking an operator file's identity

*Mitigation as built.* The packet is written from trace field names, profile keys and report clauses only,
and its `### What this packet does not carry` paragraph says why: a hash establishes identity and not
rights.

*Command and result (live).*

```
awk '/^## D10-VID-08/{s=1} /^## D11:/{s=0} s' docs/handoffs.md > /tmp/a20-packet.md   -> 45 lines
grep -cE '(^|[^a-z])refs/' /tmp/a20-packet.md                 -> 0
grep -cE '[0-9a-fA-F]{64}' /tmp/a20-packet.md                 -> 0
grep -cE '\.(ivsc|bin|grom|img|rom|exec|ivst)\b'              -> 0
grep -cF -e 'exec-image' -e 'grom-image'                      -> 0
grep -nE '[A-Za-z0-9_./-]+\.(md|ids|json|txt|ivss)'           -> four project records only:
   data/expected/d10-measurements.ids, data/expected/handoffs.ids, docs/inspection.md, docs/stic/d10-procedure.md
```

The four paths are the project's own records, cited as the packet's provenance and its own procedure. No
operator file's digest, size, path or name appears anywhere in the section, and the no-leak paragraph is at
`docs/handoffs.md` line 40 of the section.

### T-20-08 (high) -- a claim acquiring a citation to the handoff row

*Mitigation as built.* No claim names the ledger or the row id, and the claim-artifact predicate's refusal
is demonstrated rather than promised, in both directions, on the committed control.

*Command and result (live).*

```
grep -c 'handoffs.ids' data/claims.json   -> 0
grep -c 'D10-VID-08'   data/claims.json   -> 0
python3 -c 'json' data/claims.json        -> 29 claims
ROOT=$PWD CHECK_DIR=$PWD/tools/check sh tools/check/steps/claims.sh
  -> -- claims pass claims=29 provisional=28 handoffs=9   (rc 0)
```

The committed stand-in, run from a copy of `_good/tree` with the control's overlay applied:

```
-- FAIL claim-handoff-artifact: CL-0001
-- FAIL claim-handoff-artifact: CL-0002
-- FAIL claim-dangling-path: CL-0003
-- FAIL claim-dangling-path: CL-0004
-- FAIL claim-handoff-artifact: CL-0005      <- refused by the artifact predicate, not by a dangling path
-- FAIL claim-handoff-artifact: CL-0006
-- FAIL claim-handoff-artifact: CL-0007
-- FAIL claim-dangling-path: CL-0008
```

After repointing `CL-0005` to `data/expected/profile-fields.ids` in the same copy, the run prints
`-- FAIL claim-type: CL-0005/artifact` instead, **no** `claim-handoff-artifact` finding for `CL-0005`, and
the other seven unchanged -- so the refusal is neither absent nor blanket. The control is registered as
`claims control claims-handoff-artifact EV-05` (`data/expected/important-gates.ids:468`) and its `EXPECT`
names all eight ids; no new control, id or registry row was minted for this proof.

### T-20-09 (high) -- the ledger row landing where it is judged inconsistently

*Mitigation as built.* One commit carries the row, both pinned lists and all five control-tree ledgers.

*Command and result (commit + live).*

```
git show --name-status --format='%h' e10c45f
  A 20-02-SUMMARY.md
  M data/expected/handoffs.ids
  M docs/handoffs.md
  M docs/stic/d10-procedure.md
  M tools/check/controls/_good/tree/data/expected/handoffs.ids
  M tools/check/controls/claims-handoff-disposition/overlay/data/expected/handoffs.ids
  M tools/check/controls/claims-handoff-id-duplicate/overlay/data/expected/handoffs.ids
  M tools/check/controls/claims-handoff-id-extra/overlay/data/expected/handoffs.ids
  M tools/check/controls/claims-handoff-id-missing/overlay/data/expected/handoffs.ids
  M tools/check/steps/claims.sh
grep -v '^#' data/expected/handoffs.ids | awk NF | wc -l                      -> 9
grep -c 'D10-VID-08' <each of the five control-tree ledgers>                  -> 1, 1, 1, 1, 1
grep -n 'D10-VID-08' tools/check/steps/claims.sh                             -> :39 HANDOFF_IDS, :43 HANDOFF_DISP
git diff --numstat 57f9b31..HEAD -- tools/check/controls/claims-handoff-row \
    tools/check/controls/claims-handoff-unreadable tools/check/controls/claims-handoff-missing -> (empty)
claims step                                                                  -> handoffs=9, zero FAIL lines
```

The three grammar controls that exit before the id-set pass are untouched, so the row cannot land in a tree
where the row set and the disposition set are judged by different rules.

### T-20-10 (high) -- a comparison row drifting from Phase 19's measurement

*Mitigation as built.* Each row is read for its guest id, its two candidate values and its mask, and the
mask is compared against the value `stic.disc.sweep-diagonal` committed in Phase 19's own record rather
than against a re-typed literal.

*Command and result (live).*

```
19-03-SUMMARY.md:  G1 mask 0xAAAA default 1 alternate 0 | G2 0xCCCC 1/0 | G3 0xF0F0 0/1 | G4 0xFF00 4/7
packet rows:       G1 5/5 fields present, G2 5/5, G3 4/4, G4 5/5  (id, field, two candidate values,
                                                                  observable pair, mask)
masks in the packet -> 0xAAAA x2, 0xCCCC x2, 0xF0F0 x2, 0xFF00 x2  (row + status line)
status line         -> names all four masks and ends `none of them is an answer`, row stays not-performed
grep -ci 'unproduced by instrument' /tmp/a20-packet.md -> 0
```

The negative limb: replacing `0xAAAA` with `0x5555` in G1's row makes the mask comparison **false**, and
replacing `2000` then `2900` with `2000` then `2901` makes the value comparison **false**, while the
untouched row is accepted -- so the predicate is not one that accepts anything. Bands and stamps match
Phase 19's record (`G1` inside `8000 < tick < 11600`, `G2` inside `11600 < tick < 15120`, stamps 9004 and
12064), and the row's observable/witness split is the reading `20-VERIFICATION.md` judged explicitly.

### T-20-11 (high) -- an append-only record rewritten

*Mitigation as built.* The note is appended inside section (i) and the diff is asserted to remove no line of
either file; the earlier notes are quoted rather than edited.

*Command and result (commit).*

```
git diff --numstat 57f9b31..HEAD -- .planning/ops/RELEASE-OPS.md docs/packaging.md -> 43  0 / 1  0
git diff 57f9b31..HEAD -- .planning/ops/RELEASE-OPS.md | grep -c '^-[^-]'         -> 0
git diff 57f9b31..HEAD -- docs/packaging.md            | grep -c '^-[^-]'         -> 0
grep -n '^### (i' .planning/ops/RELEASE-OPS.md -> 114: (i.1) / 177: (i.2) ; `## (j)` at 220
grep -cF '<the seven-clause close line>' .planning/ops/RELEASE-OPS.md            -> 1
```

`(i.2)` sits between `(i.1)` and `(j)`, states that neither (i) nor (i.1) is edited, and states what it
supersedes; the earlier notes and their superseded wordings survive verbatim.

### T-20-12 (high) -- the release line quoted rather than recomputed

*Mitigation as built.* A fresh archive pair is written by `tools/release.sh --no-gate`, and every clause is
re-derived from those archives and from the checks that ran on this tree.

*Command and result (live).*

```
rm -rf /tmp/a20-rel && sh tools/release.sh --no-gate --out /tmp/a20-rel   -> rc 0, 1.5s
  release-assets source ok members=2859
  release-assets install ok members=11
  consumer output equals data/expected/pkg-consumer.golden (164 lines)
```

Then, from those archives only:

| clause | the reading | value |
|---|---|---|
| the release ships the static package | install archive members vs `pkg-install-files.ids`, prefix stripped, both directions | 11 vs 11, `comm` empty in both directions |
| the source archive | source members vs `git ls-files` | 2859 vs 2859; every tracked path present, nothing extra |
| the source archive carries the build-only sources | the five prefix greps | `tools/inspect/` 7, `tools/pack/` 3, `tools/asm/` 9, `tools/flatwrap/` 5, `host/` 6 (see R-20-01) |
| no shared object | `\.(so\|dylib\|dll\|bundle)$` | 0 in both archives; `binaries-listed=0`; `release-selftest=13/13` |
| no built host shim | `libivs_retro` | 0 in both archives; `pkg-selftest=21/21` with `host-so-absent` |
| no built inspection tool | `ivs-inspect` | 0 in both archives; `inspect-unlisted-member` inside the thirteen cases; no `bin/` in the install archive |
| no trace or disassembly output | `(^\|/)refs/`, `\.ivst$` | 0 and 0 in both archives; `ivst` is in `tools/check/binary-ext.txt:49` with control `files-unlisted-binary-ivst` (`important-gates.ids:673`); no tracked path |
| no content | content extensions | 0 and 0 in both archives, 0 tracked; `prefix-files=11` |

The negative limb: a constructed member list carrying `lib/libivs_retro.so` and `refs/op-trace.ivst` is
caught by all four zero-count predicates (1, 1, 1, 1), and a constructed line with the `no content` clause
removed fails the clause predicate (0) while the untouched line passes (1). The four figures the note cites
from the gate line were read from this session's run, not from the SUMMARY: `prefix-files=11`,
`binaries-listed=0`, `release-selftest=13/13`, `pkg-selftest=21/21`.

### T-20-SC (high) -- a dependency introduced

*Mitigation as built / measured.* Not applicable, asserted as such:

```
git diff --name-only 57f9b31..HEAD     -> 15 files: 12 markdown, two `.ids`, one shell script (`claims.sh`)
git log 57f9b31..HEAD -- '*package.json' '*package-lock.json' 'requirements*.txt' 'Cargo.toml' \
    'Cargo.lock' 'pyproject.toml' 'vcpkg.json' 'conanfile*'   -> (empty)
grep -cE 'npm|pip |cargo|yarn|pnpm|requirements\.txt' <each changed file> -> 0 for every file
```

No package manager is named in any file this phase touches, no manifest moved, and `package-legitimacy`
has nothing to gate.

## Accepted Risks Log

No threat is accepted or transferred. The rows below are **open non-blocking debt carried from this audit
and the verification** -- neither is a threat mitigation gap, and neither counts toward `threats_open`.
Each is recorded with the reason it is not one, so it is not re-litigated.

| Risk ID | Threat Ref | Rationale | Accepted By | Date |
|---------|------------|-----------|-------------|------|
| R-20-01 | T-20-12 (adjacent, not open) | The five prefix counts in `RELEASE-OPS.md` `(i.2)` and in `20-02-SUMMARY.md` were read with `grep -c -F '/tools/inspect/'` on the **raw** `tar -tzf` member list. That command also matches nested paths under the control fixtures, so it counts 7/3/9/5/6 where the archive root holds 4/2/2/2/3 files (plus one directory entry each) under those prefixes: the extra matches are `tools/check/controls/_good/tree/tools/inspect/good.c`, `.../_good/tree/tools/asm/good.c`, `.../c-build-unlisted-asm-source/overlay/tools/asm/extra.c`, `.../manifests-recipe-source-digest/overlay/tools/asm/asm.c`, `tests/host/ivs_host_harness.c` and their directory entries. The clause's substance is unaffected and was re-derived here: the archive root does carry `tools/inspect/{CMakeLists.txt,inspect.c,inspect_dis.c,inspect_trace.c}`, `tools/asm/{CMakeLists.txt,asm.c}`, `tools/pack/{CMakeLists.txt,pack.c}`, `tools/flatwrap/{CMakeLists.txt,flatwrap.c}` and `host/{CMakeLists.txt,ivs_retro_shim.c,libretro_shim.h}`. The values are reproducible with the recorded command, so this is a label precision issue ("members" counting nested matches), not a mitigation gap. Remedy is owed to the next note that recomputes those five prefixes: name the pattern as anchored, `grep -c -F -e '^<prefix>/'`, and state the file and directory counts separately. §2.12 makes `(i.2)` append-only, so the correction belongs in a further appended note, never an edit. | gsd-security-auditor | 2026-11-27 |
| R-20-02 | none (evidence-accuracy wording) | Both SUMMARYs say the gate run printed "zero `-- FAIL` lines", while `grep -c -- '-- FAIL'` on the log returns 2 on each run: both are the **observed text of a designed negative control** (`runner-selftest proof control-pass-line-not-last` and `proof runner-selftest-missing`), printed by the step's own `proof` observations. The gate exits 0 and prints `gate: PASS` with `grep -c '^-- FAIL '` = 0, so the substance -- no gate failure -- holds and no threat mitigation reads this sentence. The precise phrasing is "no gate failure", and `20-VERIFICATION.md` already records the same reading as ℹ️ Info. | gsd-security-auditor | 2026-11-27 |

## Unregistered threat flags

None. Neither SUMMARY carries a `## Threat Flags` section (`grep -i threat 20-0*-SUMMARY.md` -> no match),
and every artifact the phase added maps to a declared threat id above: the operator section and the
instrument section to `T-20-01`/`T-20-02`/`T-20-05`/`T-20-06`, the `refs/` containment to `T-20-03`, the
naming limb to `T-20-04`, the packet and its anchor to `T-20-07`/`T-20-10`, the ledger row and its pinned
sets to `T-20-08`/`T-20-09`, and the release note, the packaging bullet and the recomputed line to
`T-20-11`/`T-20-12`. The phase adds no executable, so there is no new attack surface to register.

## Outstanding evidence (recorded as not run, never as a pass)

| Item | State | Measured reason | What would discharge it |
|------|-------|-----------------|--------------------------|
| `sh tools/gate-linux.sh` (container gate, `GUARDRAILS-v0.4` §4) | **not run** | `timeout 10 docker info` -> rc 124 (no answer); `build/gate-linux.log` is 0 bytes, so no `gate: PASS` line exists to read. No `ivs-*` image, container or volume was created. | one container gate on a host whose daemon answers, ending `gate: PASS` in `build/gate-linux.log` |

No threat in this register depends on that item, and no id cited above is weakened by it: every mechanism is
a markdown assertion, a `tar -tzf` reading, a POSIX shell predicate or a CMake step, all of which behave
identically on the Ubuntu image, and every id cited ran under the local gate on this host.

## Cost checkpoint (recomputed, never quoted)

Last five rows of `.planning/ci-runs.tsv` (118 rows), by arithmetic:

```
cpu_s   {584, 580, 572, 530, 368}  -> median 572
build_s {51, 51, 49, 49, 35}       -> median 49
gate_s  {171, 165, 164, 154, 115}  -> median 164
```

Phase 20 is priced 0.02 against the 21 hosted cpu-s allowance and adds no in-process id, process-launch id or
registry row, so the verdict is **`discharged-within-allowance`**.

## Security Audit Trail

| Audit Date | Threats Total | Closed | Open | Run By |
|------------|---------------|--------|------|--------|
| 2026-11-27 | 13 | 13 | 0 | gsd-security-auditor (pi, subagent), ASVS L1, `block_on: high` |

## Sign-Off

- [x] All threats have a disposition (`mitigate`; none accepted, none transferred)
- [x] Accepted risks documented in the Accepted Risks Log (none accepted; two non-blocking debt rows with reasons)
- [x] `threats_open: 0` confirmed -- no open threat at or above `high`
- [x] `status: secured` set in frontmatter
- [x] No implementation file was modified by this audit; no gate step, fault, golden, claim, installed file
      or handoff row was added (`steps=38/38`, `controls=405/405`, `faults=238/238`, `goldens=29`,
      `claims=29`, `provisional=28`, `prefix-files=11`, `gates=123`, `lines=378`, `tests-expected=5114`;
      `handoffs=9` is the phase's one move, 8 -> 9, not this audit's)
- [x] Nothing this audit wrote is tracked or left behind: the four `refs/op-*` outputs and the throwaway
      archives were removed, `refs/` holds its 13 pre-existing entries, and the only porcelain entry is the
      verifier's untracked `20-VERIFICATION.md`

**Approval:** verified 2026-11-27

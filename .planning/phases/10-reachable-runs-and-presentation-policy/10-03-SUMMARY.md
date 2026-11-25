---
phase: 10-reachable-runs-and-presentation-policy
plan: "03"
subsystem: evidence-records
tags: [ev-05, handoffs, ledger, ids-grammar, docs]
status: complete
requirements: [EV-05]
dependency_graph:
  requires: ["10-01", "10-02"]
  provides: ["data/expected/handoffs.ids (the five-row human-only ledger)", "docs/handoffs.md (the H7-1/H7-2 packet)"]
  affects: ["10-05 (the claim hook's artifact refusal points at this ledger and must guard on its existence)"]
tech-stack:
  added: []
  patterns: ["four-token ids grammar in the state-cuts.ids style", "absence-means-empty declared in the file header, not through an allow-empty directive"]
key-files:
  created:
    - data/expected/handoffs.ids
    - docs/handoffs.md
  modified: []
decisions:
  - "The ledger header states the no-verdict rule as 'never a done item and never blank' and names the absence directive without the reserved literal, because this plan's own verify refuses the verdict vocabulary and the allow-empty literal anywhere in the file. Meaning preserved, tokens avoided; recorded here as a Rule 3 deviation."
  - "H8-3 is recorded as the Phase 8 close record's id for the same two acts as H7-1 and H7-2 plus a look at the STIC pictures, so five rows are not five human acts."
  - "The packet cites data/expected/d10-measurements.ids by path only; no row count appears in either new file (AGENTS.md: recompute, never quote)."
metrics:
  duration: "~24 min"
  completed: "2026-11-25"
  tasks: 2
  files: 2
estimate:
  tokens: 55000
  tasks: 2
actuals:
  tokens: 2574
  tasks: 2
  commits: 2
plan_head_before: 658a4221ed7a39c376e2a5893c781db01603f373
plan_head_after: 2cf4dd6d53027f23e141d05b768ad0405f2d9cf0
---

# Phase 10 Plan 03: Handoff ledger and packet Summary

A five-row ledger (`data/expected/handoffs.ids`) records every human-only item with the literal `evidence: none`, and one packet (`docs/handoffs.md`) gives the owner the exact commands, sizes, modes and digest lines for H7-1 and H7-2 so neither has to be reconstructed from a phase record.

## The five rows, verbatim

```
H7-1 handed-off evidence: none
H7-2 handed-off evidence: none
H8-3 handed-off evidence: none
D10 not-performed evidence: none
D11 not-performed evidence: none
```

Shape check (the plan's `awk`, run on the committed file): silent, exit 0 — five rows, four tokens each, every disposition in `{handed-off, not-performed}`, every row ending `evidence: none`, all five ids present.
Every row's last two tokens are the literal words `evidence: none`, so no row can express a tier, and there is no third disposition vocabulary for a row to hide in.

## What the header declares

- The grammar, `<id> <disposition> evidence: none`, with each of the four tokens named.
- That the id is the one the project's own records use (an H-id of a phase or close record, or a D-id owner gate).
- That the disposition is `handed-off` or `not-performed` and nothing else.
- That a **missing** file means an empty set, not a missing inventory, because this file is not read through `read_ids` (`tools/check/lib.cmake:75-101` appends whole lines and is only usable for single-column inventories). **Plan 10-05's reader must therefore guard on existence** (`if(EXISTS ...)`), the way the claim profile file is guarded, and must not use `read_ids`.
- That the file carries no allow-empty directive, because emptiness is not legal for this ledger.
- Its reader: the claim hook's artifact refusal (plan 10-05) and later milestones that cite a row id instead of re-asking.

## H8-3: five rows, fewer acts

H8-3 is the Phase 8 close record's id (`08-MILESTONE-CLOSE.md`, section K) for the same two acts as H7-1 and H7-2 plus a look at the STIC pictures of a `diag-stic2-` scenario, so one sitting discharges H8-3 and both Phase 7 rows. The row count is five; the number of distinct human acts is smaller, and a later milestone must not book a third act on the strength of the count. The relationship is stated in the ledger header and again in the packet.

## Reversibility rating

This file is rated **costly** to change (CONTEXT, the handoff-ledger decision). A later milestone cites a row id instead of re-asking the owner, so changing a row's meaning silently re-opens or closes a question that was already settled — the defect the ledger exists to prevent.

## The packet: measured identities, with the command that produced each

Every artefact was deleted afterwards; the packet quotes sizes, modes and hashes and ships no `.wav` and no `.ppm`.

| Item | Command | Measured |
|---|---|---|
| H7-1 | `sh tools/listen-demo.sh demo-lamp-tone-pass` | `build/listen/demo-lamp-tone-pass.wav`, **21674 bytes**, mode `0644`, `refpcm.sha256 e3b6638887947d64f7fa7d2be5687aff0eca85981b0ea79cfcfd3c74d057f1ad`, exit 0 |
| H7-2 (idle) | `build/release/runner/ivs-run --content-dir build/release/images --frame-ppm build/h7-2-idle.ppm data/scenarios/demo-lamp-tone-idle.ivss` | **45806 bytes**, mode `0600`, header `P6` / `159 96` / `255`, exit 0, `report.end 89` |
| H7-2 (pass) | the same with `demo-lamp-tone-pass.ivss` | **45806 bytes**, mode `0600`, header `P6` / `159 96` / `255`, exit 0, `report.end 130` |
| H8-3 picture limb | the same with `diag-stic2-mob-pass.ivss` | **45806 bytes**, mode `0600`, exit 0, `report.end 81` |
| D11 skip | `sh tools/owner-run.sh refs/exec.bin refs/grom.bin` (neither present) | `SKIPPED owner-run: no operator images (D11)`, exit **77** |

The frame-12 hash the packet quotes as the only deterministic identity of the H7-2 picture is the committed golden's own line (`data/golden/demo-lamp-tone-pass.expect:81`): `assert frame 12 hash == a7b9889b136cf47b2586def7f992c7caaf19c451e043a9a676b2f838bd91c2d7`. `--frame-ppm` adds no report key, so the golden is what ties the exported file to a pinned frame. The packet also states that the preview palette is arbitrary and is **not STIC colour data**.

`docs/handoffs.md` is a document, not a gate step: the gate never opens it, no test reads it, and no `data/claims.json` record points at it. Its first two paragraphs say so, and that a yes-or-no answer is a handoff **result** recorded as `evidence: none` either way, never a claim and never a tier.

## `[nc]` waivers

None were needed. The packet avoids the eight claim words the `text` step already scans for and the two (`looks`, `sounds`) that plan 10-05 adds, describing a judgement as a judgement ("what they hear", "whether the picture is the one the program intends"). Zero `text-claim-untagged` findings; `claim-refs=26` unchanged; `claims=28`, `provisional=27` unchanged.

## Deviations from Plan

### Auto-fixed Issues

**1. [Rule 3 - Blocking conflict] The header's verdict wording vs this task's own scanner**
- **Found during:** Task 1
- **Issue:** the must-haves ask the header to state that a handoff "is never a pass and never blank" and that there is "no `#allow-empty` line", while the same task's verify refuses the literal tokens `pass` and `#allow-empty` anywhere in the file. Both cannot hold literally.
- **Fix:** the file states the same meaning as "a handoff is never a done item and never blank" and "This file carries no allow-empty directive, because emptiness is not legal for this ledger", and names the verdict vocabulary as refused on purpose. The verify now passes and the meaning is unchanged; this is the only wording concession.
- **Files modified:** `data/expected/handoffs.ids`
- **Commit:** 841d6e6

### Budget deviation (approved, recorded)

`sh tools/gate.sh` and `dash tools/gate.sh` were each run **once at the end of the plan**, not per task (the per-task gate predicates in the plan were satisfied by the cheap steps instead: `files`, `hygiene`, `text`, `denylist`, `glossary`, `vocab` on the same FILELIST). The container gate was not run.

## Token deltas

- `docs` 62 → **63**, `doc-files` 61 → **62** (the one new `docs/*.md`); `data/expected/handoffs.ids` is not a document and moves no document count.
- `files` = `tracked` = `hygiene-files` = **2584** after task 2 (task 1: +1 over 2582; task 2: +1 over 2583; each task's delta against its own baseline is exactly 1), and **2585** at the final record commit, which adds this SUMMARY file (the two record files, `STATE.md` and `ROADMAP.md`, were already tracked). `files == tracked == hygiene-files` holds at both points.
- Unmoved: `steps=38/38`, `controls=386/386`, `claims=28`, `provisional=27`, `gates=107`, `lines=330`, `claim-refs=26`, `prefix-files=11`, `goldens=29`, `cases=13`, `faults=235/235`, `tests-expected/executed/skipped/missing=4958/4954/4/0`.

## Gate lines

```
sh:    gate: PASS steps=38/38 controls=386/386 ... claims=28 provisional=27 ... gates=107 lines=330 ... doc-files=62 ... tracked=2584 ... files=2584 ... prefix-files=11 ... cases=13 ... tests-expected=4958 tests-executed=4954 tests-skipped=4 tests-missing=0 docs=63 claim-refs=26
dash:  gate: PASS steps=38/38 controls=386/386 claims=28 provisional=27 tracked=2584 doc-files=62 hygiene-files=2584 gates=107 lines=330 files=2584 prefix-files=11 docs=63 claim-refs=26
sh at the final record commit:
gate: PASS steps=38/38 controls=386/386 claims=28 provisional=27 tracked=2585 doc-files=62 hygiene-files=2585 gates=107 lines=330 files=2585 prefix-files=11 docs=63 claim-refs=26
```

Both shells PASS; the only token that moves between the content HEAD and the final record commit is `files`/`tracked`/`hygiene-files` 2584 to 2585, caused by this SUMMARY file, and every absolute-and-frozen token is unchanged.

## Commits

- `841d6e6` docs(10): record the five human-only items as a handoff ledger (task 1)
- `2cf4dd6` docs(10): add the handoff packet for the five ledger rows (task 2)
- `36c7981` docs(10): complete the handoff ledger and packet plan (this file)
- the record commit `docs(10): record plan 10-03 completion in STATE and ROADMAP` (its hash is this plan's last commit, so it is named rather than quoted: a hash written into a file cannot survive the commit that writes it)

The frontmatter's `commits: 2` counts the two task commits the ledger measured at SUMMARY-write time (`plan_head_before..plan_head_after`); the two record commits follow this file and are outside every task's delta.

## EV-05 stays Pending (deliberate)

The plan frontmatter lists `requirements: [EV-05]`, and `requirements mark-complete EV-05` was applied and then reverted. EV-05's criterion has three clauses and this plan supplies two of them: the ledger and the packet. The third - "a claim may not cite a handoff artifact" - is plan 10-05, and the claim-word list extension is 10-05/10-06. Marking EV-05 Complete here would misstate the record and would also turn the gate red: `important-gates-excluded.ids` excludes EV-01..EV-04 but not EV-05, so a Complete EV-05 with no registry row trips `important-gates-complete-uncovered` (verified green after the revert). EV-05 is completed at the phase close, by the plan that lands the claim refusal.

## Self-Check: PASSED

- `data/expected/handoffs.ids` exists (34 lines, 5 rows) — FOUND
- `docs/handoffs.md` exists (94 lines) — FOUND
- `841d6e6` and `2cf4dd6` are ancestors of HEAD — FOUND
- No `.wav`, `.ppm` or image in the tree — confirmed (`git status --porcelain -- '*.wav' '*.ppm' '*.png'` empty)

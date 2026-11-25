# Phase 12 entry gate — the rights-review decision line

**Committed before any third-party file was read.** This document is the decision
the roadmap requires (`### Phase 12`, **Entry gate**), and it is written from the
project's own research records, not from any external format description.

**Decision: `degraded to FMT-01 only` — and, on the cost ground below, Phase 12 is
dropped from v0.2.0 under the pre-authorized three-phase fallback.**

Two independent grounds, either of which is sufficient. Both are recorded because a
later milestone will want to know which one to re-test.

---

## Ground 1 — the rights review cannot clear the importer, and this was known before the phase

The milestone's degradation condition reads, verbatim: *"the mapping is only
expressible as a table copied from a GPL source"* (`.planning/MILESTONE-CONTEXT.md`
§2, Phase 12, **Degradation**).

The project's own research already answered it:

| Record | What it says |
|---|---|
| `.planning/research/PITFALLS.md`, Pitfall 4 | "Doc says the BIN+CFG source is 'close to a spec' (jzIntv's `bincfg`). `[COMM S26]` **That is a GPL source; do not use it as the format spec**" |
| `.planning/research/FEATURES.md`, TS6 | "`.bin+.cfg` is the de-facto library format but its `.cfg` is 'annoying to parse' and 'poorly documented' **per its own maintainer**" |
| `.planning/research/PITFALLS.md` | "the plain ROM format cannot express ECS/Mattel bank switching" |

So the position is not "the review has not been done"; it is **"the review has a
recorded answer, and the answer is no"**. A per-fact table written today would have
one candidate source for its facts, and that source is under a licence this project
forbids as a derivation route (`FND-05`, and `AGENTS.md`'s "no code, tables or
constants copied from jzIntv, MAME or other emulators").

**FMT-04 (the `.bin`/`.cfg` importer) is therefore deferred, not degraded, and not
partial.** It cannot be entered at all without either a permissively-described
primary source or an independent description of the format — and no such source has
been identified in any of the project's research. Its condition for reversal is
stated below.

## Ground 2 — the recorded cost level is above the stop line, and Phase 12 is the phase designed to absorb that

The roadmap says Phase 12 "is last and conditional so a cost spike is absorbed by
dropping it rather than discovered at the close", and the milestone pre-authorizes a
three-phase fallback.

The recorded reading is uncomfortable and is stated rather than smoothed: the last
five hosted `r` values are 10.441, 10.559, 11.327, 11.351 and 11.673, **median
11.327**, against a budget of 7.8 and a stop line of 8.3. `.planning/CI-EFFICIENCY.md`
records why that number cannot be read as growth — the same trees read 7.4–8.4 hours
earlier, `cpu_s` moved 354 → 572 with no gate content added, and the Phase 9 diff
*removed* about 0.80 local cpu-s. But the recorded rule is a policy stop, not an
evidence claim: it fires on the arithmetic, and the arithmetic is above the line.

Either way the conclusion is the same, and that is why both grounds are recorded: a
phase that would add gate content on a reading the project cannot defend is exactly
what the fallback exists for.

---

## What this means for the requirements, recorded as the roadmap requires

The roadmap's criterion 5 is explicit: *"if the phase is dropped … FMT-01 and FMT-04
are recorded as partial or deferred with the condition in `.planning/REQUIREMENTS.md`
traceability — never silently lost and never marked `Complete`."*

- **FMT-04** — **deferred**, not partial. It was never entered. Reversal condition:
  a permissively-described primary source, or an independent description of the
  `.bin`/`.cfg` mapping that is not derived from a GPL implementation.
- **FMT-01 (narrowed)** — **deferred**. Its own half is rights-clean (a bank-mode
  model in profile data with typed refusals, exercised by **generated original
  fixtures** built from tracked `.asm`/`.cfgsrc` sources), so it is not blocked on
  rights. It is deferred on the second ground, and its reversal condition is a
  re-derived cost baseline: the next milestone that opens the registry baseline may
  take it as its own phase.

Neither is marked `Complete`. Neither is silently dropped: both keep their rows in
`.planning/REQUIREMENTS.md` with the status `Deferred` and this document as the
pointer.

## What was not done, and must not be read as done

- No third-party file was read, downloaded, quoted or paraphrased.
- No `.cfg` description was written, and `docs/formats/cfg-import-v1.md` does not
  exist.
- No importer, no `cfg-*` token, no `data/expected/cfg-refusals.ids`.
- No bank-mode profile field, no fixture, no fault.
- `tools/check/binary-ext.txt` and `.gitignore` are unchanged.

## The reversal, in one line each

| Requirement | Reopens when |
|---|---|
| FMT-04 | a permissively-described primary source or an independent `.cfg` description exists, **and** a per-mode rights review can be written fact by fact with a source id per row |
| FMT-01 | a later milestone re-derives the registry baseline (`lines=337`) and prices the phase against a cleared cost reading |

---

*Written 2026-11-25 by the autonomous run, before any third-party file was read, as
the roadmap's entry gate requires. It is an appended record: corrections go below
this line with their own date, never into the text above.*

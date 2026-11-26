---
phase: 16
plan: 05
status: complete
requirements: [PKG-06]
tags: [PKG-06, close, two-shell, container, cost-checkpoint, validation-signoff]
---

# 16-05 — The phase close: two shells, one container, one verdict, the signed record

## What this plan did

Measured **one** line on one tree under `sh` and `dash` and once inside the
Ubuntu 24.04 container, reconciled every token the phase's five plans predicted,
recorded exactly one checkpoint verdict, signed `16-VALIDATION.md`, and stated
what the milestone can and cannot now claim.

## The measured line

```
gate: PASS steps=38/38 controls=404/404 ... claims=29 provisional=28 handoffs=8 ...
      faults=238/238 goldens=29 gates=119 lines=364 pkg-selftest=20/20
      release-selftest=12/12 tests-expected=5042 tests-executed=5038
      tests-skipped=4 tests-missing=0 prefix-files=11 tracked=2787
```

- **Two shells:** `sh tools/gate.sh` and `dash tools/gate.sh` both exit 0 and
  their final lines are **byte-identical** (`cmp` clean, 1,520 bytes each).
  `gate-time total=104` (`sh`) / `204` (`dash`) on a host that was not idle.
- **Container:** `sh tools/gate-linux.sh` ran **once**, `gate-linux: exit 0`,
  `gate-time total=793` under amd64 emulation. Exactly four documented host
  tokens differ and nothing else: `cc` AppleClang→GNU, `core-imports` 7→5,
  `archive` members→sha256, `refs-verified` 3/4→0/4. No `ivs-` container, image
  or volume survives.
- `sh tools/hygiene.sh --tree` printed its pass line and exited 0;
  `git ls-files` holds no refused binary extension.

## Reconciliation

Every token the five plans predicted is where it was measured. The only moved
tokens are the phase's four entitled growth gates: `gates` 118 → 119,
`lines` 361 → 364, `handoffs` 5 → 8, and the good toy tree 47/50 → 48/51.
`steps`, `faults`, `goldens`, `controls`, `prefix-files`, `tests-skipped` are
exactly where they were.

## Cost checkpoint — one verdict: `reprice`

Recomputed by arithmetic at this commit from the last five `.planning/ci-runs.tsv`
rows (`45723269901`, `45747709457`, `45751979661`, `45778926664`, `45779277467`),
never quoted:

| series | five values | median | frozen |
|---|---|---|---|
| `cpu_s` | 368 366 359 441 352 | **366** | 361 |
| `build_s` | 35 34 34 38 33 | **34** | 34 |
| `gate_s` | 122 115 120 145 115 | **120** | 118 |

- `discharged-within-allowance` is unreachable: 366 > 361 + 0.4 = 361.4, by 4.6.
- `lever-required` is not reached: the cumulative priced total is
  **5.2 of 21** (2.0 + 1.3 + 1.5 + 0.4), and this phase's actual addition is
  **0.015** against the 0.4 it priced — 0.385 *under*.
- The median moved more than the priced addition while the token line is exactly
  where the plans predicted, which is the rule's own definition of `reprice`: a
  runner event, no lever invented. `steps=38/38` is an invariant, not a target.

## The lever (plan 16-01), measured

`gate-time total` **79 → 72**; the pair's wall time 37 s → 24 s (13 s given back);
`gate-cpu` sums 228.23 → 260.77, inside the stated band; the summary token line
byte-identical at 1,509 bytes and each `step.out.<name>` buffer compared. It buys
wall time and Actions minutes, not ratio — no allowance credit is claimed.

## What the milestone can now claim

**One claim, `CL-0029`**, tier `deterministic-repeatability`, standing
`provisional`, and `CL-0002` remains the only `established` claim: the same core driven through a shared-object host reproduces the
runner's nine digest lines on a project cartridge. It compares the core with
itself — never an independent oracle — and no claim rests on a handoff, a
`build/` path, a `.wav`, a `.ppm`, a `.ivsn` or a bench `.tsv`.

## What it cannot claim, and does not

- **No real EXEC, GROM or commercial cartridge has been shown to boot, play, look
  or sound right.** That is D11, still `not-performed`, and the route's `exec:`
  label stays `-` so no run reads as a real-EXEC result.
- **No hardware measurement exists** (D10, 47 items handed off). No claim reaches
  the `measured-hardware` tier, and none may: that tier needs D10 performed.
- **The owner's questions are handoff rows, `evidence: none`, cited by no claim and
  by no criterion.** The ledger holds eight: `D10` (physical measurement, 47 items,
  `not-performed`), `D11` (the owner's own EXEC/GROM/cartridge run, `not-performed`),
  `H7-1`, `H7-2` and `H8-3` from earlier milestones (`handed-off`), and this phase's
  three — `H16-1` a real frontend loads the shim, `H16-2` an owner image's picture
  and sound look right, `H16-3` the owner's flat image needs no word-order guess.
  None is evidence; none is a criterion; none is re-asked.
- **Nothing ships but the static package and the source archive** — no shared
  object, no host shim, no content byte. The release-decision line says so in
  `RELEASE-OPS.md` §(i).

## Growth gates, and the four tokens this phase owns

The phase's four entitled growth gates are `gates` 118 → 119, `lines` 361 → 364,
`handoffs` 5 → 8 and the good toy tree 47/50 → 48/51; nothing else moved. The four
tokens this phase itself owns are `handoffs=8` (the ledger's counted row count),
`gates=119`/`lines=364` (the three `release-posture` rows and their re-derived
baseline), and `prefix-files=11` (the package the posture refuses to grow).

## Deviations

Nine plan defects were recorded across the phase's five plans, each with its
measured reason and waived in `.planning/WINDOWS.md` rather than worked around
(two `hygiene.sh --tree` silence claims, a host-specific `cpu` floor, a
non-quotable cold-build line, a verify grepping a token its own action did not
name, a pin format the same task contradicted, vestigial guard annotations, a
`H16-` grep no change could empty, and an optimistic frozen §5 hosted estimate
recorded as ~144 s rather than 115 s).

This plan's own close was completed by the orchestrator after the executing
agent was stopped at the 30-minute dispatch wall having finished every
measurement but before writing this SUMMARY and signing the validation record;
the measurements above are the executor's, quoted from the log it left.

## Hosted row

`not-recorded` at this commit — the phase is not pushed yet. The orchestrator
records it from `tools/ci-record.sh <run-id>` after the push, deferred by exactly
one commit and never dropped.

# Phase 15: Build-Only Host and the Fake-Frontend Harness - Discussion Log

> **Audit trail only.** Do not use as input to planning, research, or execution agents.
> Decisions are captured in CONTEXT.md — this log preserves the alternatives considered.

**Date:** 2026-11-26
**Phase:** 15-build-only-host-and-the-fake-frontend-harness
**Areas discussed:** The host is build-only and the ABI subset is project-authored; The harness is the evidence, never the window; Packaging names the shared object as a non-member; Gate cost and evidence discipline

---

## The host is build-only and the ABI subset is project-authored

| Option | Description | Selected |
|--------|-------------|----------|
| A project-authored subset stating only what is implemented (D-48) | Eight entries, *libretro-shaped*, never compatible; nothing vendored | ✓ |
| Vendor an exact libretro header | 200 symbols, a licence to read and record first | |
| Claim libretro compatibility | No evidence, and a claim the tier ceiling forbids | |

**User's choice:** none asked — frozen by D-48 and scoping R-3.
**Notes:** the shared target is the only place `IVS_API` is defined; the package stays static and eleven files.

---

## The harness is the evidence, never the window

| Option | Description | Selected |
|--------|-------------|----------|
| A headless `dlopen` harness reproducing the nine digest lines | Three vacuity guards, two stand-ins | ✓ |
| A windowed demo frontend | Cannot be CI evidence; a CI blind spot | |
| Trust the shim because it compiles | No comparison at all | |

**User's choice:** none asked — the frozen verification strategy fixes the shape and names the two stand-ins.

---

## Packaging names the shared object as a non-member

| Option | Description | Selected |
|--------|-------------|----------|
| `so`/`dylib`/`dll`/`bundle` in `binary-ext.txt`, two controls, one selftest, docs line | The release ships the static package and the source archive | ✓ |
| Ship the shared object | Moves `pkg-install-files.ids` and `docs/packaging.md` in one commit — refused by D-49 | |

**User's choice:** none asked — D-49 and GUARDRAILS V6 freeze the retarget set.

---

## Gate cost and evidence discipline

| Option | Description | Selected |
|--------|-------------|----------|
| Recompute the last-five medians from `.planning/ci-runs.tsv` | One recorded verdict, arithmetic beside it | ✓ |
| Add a measurement run | Spends hosted minutes to re-derive a committed figure | |

**User's choice:** none asked — GUARDRAILS §2.3 freezes the checkpoint rule.

---

## Decided autonomously

[auto] [The host] — Q: "Which eight entries does the subset declare?" → Selected: "api_version, init, deinit, set_environment, get_system_av_info, load_game, run, unload_game, plus a harness-only ivs_retro_digest" (reversible technical — one header and one resolver list)
[auto] [The harness] — Q: "Which cartridge does the harness load?" → Selected: "the project's own diag-cpu-1.ivsc with the boot stub and zero GROM" (reversible technical — a project fixture)
[auto] [The harness] — Q: "How is 'no palette of its own' proved?" → Selected: "the shim links runner/frame_export.c and a symbol scan refuses an ivs_ or palette symbol in the export surface" (reversible technical — one id plus one symbol list)
[auto] [Packaging] — Q: "Does the shared object need a fourth files control per extension?" → Selected: "two controls over one extension list, matching the tools/bench precedent" (reversible technical — a control count)
[auto] [Packaging] — Q: "Where does 'no host artifact ships' live?" → Selected: "one release-assets case plus the RELEASE-OPS.md line" (reversible technical — record edits)
[auto] [Cost] — Q: "Does the checkpoint add a measurement run?" → Selected: "no — recompute from the committed .planning/ci-runs.tsv rows" (reversible technical — the file is the arithmetic input)

## Claude's Discretion

None.

## Deferred Ideas

- A real frontend loading the shim: a handoff row, never a criterion.
- Any vendored libretro header: refused unless a real frontend needs one.
- Shipping the shared object: refused; the release ships the static package and the source archive.
- A window, an audio device, a capture option for owner content: out of scope.

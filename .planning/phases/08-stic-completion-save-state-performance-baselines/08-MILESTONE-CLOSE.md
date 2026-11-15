# Phase 8 milestone close record, stage 1

**What this record is.** The close record of the first milestone (Phases 1 to 8) for the exact tested tree of the Phase 8 branch, in the eleven sections A to K of `.planning/phases/08-stic-completion-save-state-performance-baselines/08-DISCUSS-release.md` section 2 (D-27). It is **stage 1** of REL-01 (D-28): everything that can be computed before the merge. It was written by the executor of plan 08-21; it is **not an independent verifier run**, and the executor is the same model family that wrote the derivations, the programs and the core. [nc]

**What is generated and what is written.** Sections A, F, G and J are the pasted output of `sh tools/release-readiness.sh --allow-dirty --history` (the claim table inside it comes from `tools/check/claims-table.cmake` over `data/claims.json`), run on the head named in section A. `--allow-dirty` is used because three tracked files of the working tree differ from the head by design (`.planning/ci-runs.tsv`, `.planning/config.json`, `.planning/state.json`: the orchestrator's files, which no executor stages) and the record files of this plan were not yet committed. Every other section is written by hand from the SUMMARY files, the verification record and the repository, and says so. One generated line was edited: the tree-scan line of section J (see there). Nothing generated was typed. [nc]

**What the placeholder means.** `<stage2>` marks every fact that exists only after the merge of the Phase 8 pull request: the Phase 8 merge commit, the push run on it, the open pull request and issue counts, the ruleset read, and the release decision. Those are filled by the orchestrator's `gsd/records-m1-close` pull request after the merge (D-28, `.planning/ops/RELEASE-OPS.md`, item b). An executor writes none of them. REL-01 stays Pending in `.planning/REQUIREMENTS.md` until then. [nc]

**Release status at the time of writing: not started, paused by the owner's decision after Phase 8.** This statement is relayed to the executor by the orchestrator, which reports the owner's decision to run `/gsd-pause-work` after Phase 8 instead of continuing into the milestone audit, the milestone completion, the cleanup and the first release; the executor cannot check it independently. Consequently no tag, no draft and no release exists, no push or `gh` call was made by this plan, and nothing in this record says that anything was published. [nc]

**Labels.** Every figure is labelled local laptop, container (Ubuntu 24.04, emulated amd64) or hosted, and the three are never mixed. Every sentence that is not a quote of a gate line and that states a result carries a claim tag (`[claim:CL-NNNN]`) or `[nc]` (not a claim); the record is not scanned by the gate, so this was checked by reading. No sentence here is stronger than provisional: 27 of 28 claims are provisional, the one established claim is CL-0002, and no claim is at the measured-hardware tier. [nc]

## A. Identity

- commit: 407d668dc4d6a930798e6fff85106fa8e4f9dc8c
- commit date (UTC): 2026-11-15T15:53:58Z
- working tree: dirty (tracked files differ from HEAD; --allow-dirty); the ids below are those of HEAD
- tree core: 78927759e0f0c43f9b8ce2409fc90c171efc0ccf
- tree runner: 04f38a0a3ec71e938276cb5cb141a611f5450110
- tree tools: 5771148bfc623e1a433765eee7477ad16daf1326
- tree data: ec05601c37e186fc3cadfec1efd636acc6aae0c9
- tree docs: e1b81bd6efb01c9d9d9e195a0e3fc21dc7dc21e9
- tree fixtures: 55170e1141ffbdbe514a04bf47491d4c5b5ff0c8
- tree tests: 51ef9b88c6e79986f72d404371fa0b6a10d14efa


**Reading of section A.** [nc]

- Milestone label: the first milestone, Phases 1 to 8. It is a GSD planning label, not a software version and never a git tag (D-30, D-31).
- Software version: 0.1.0, from the three version macros of `core/include/ivs/ivs.h`; the installed package reports the same version (D-30).
- Artifact commit: `<stage2>` (the Phase 8 merge commit on `main`; a tag, when one is made, goes on that commit and not on a records commit, D-29).
- The commit line above is the head the script ran on: `407d668`, the head of plan 08-20 (every plan through 08-20, nothing of plan 08-21). Plan 08-21 changes only files under `.planning/` (its `files_modified` list), so the seven tree identities above are the identities of the final tree of this plan; the SUMMARY of plan 08-21 records the re-check made after its last commit.
- The line "dirty" lists the three tracked files that differ from the head by design, named at the top of this record.


## B. Main green

generated or written in Task 2

## C. Open pull requests and issues

generated or written in Task 2

## D. Rolling roadmap and lessons

generated or written in Task 2

## E. Requirement coverage

generated or written in Task 2

## F. Claim table

| Claim | Tier | Standing | Artifact |
|---|---|---|---|
| CL-0001 | source-expectation | provisional | docs/cp1610/reference-notes.md |
| CL-0002 | source-expectation | established | data/profiles/intv-2609-ntsc-family-p1.json |
| CL-0003 | source-expectation | provisional | data/profiles/intv-2609-ntsc-family-p1.json |
| CL-0004 | source-expectation | provisional | docs/cp1610/reference-notes.md |
| CL-0005 | source-expectation | provisional | data/cp1610/opcode-families.json |
| CL-0006 | source-expectation | provisional | data/memmap/intv-2609-ntsc-family-p1.literal.txt |
| CL-0007 | source-expectation | provisional | docs/formats/ivsc-v1.md |
| CL-0008 | original-diagnostic | provisional | fixtures/FX-diag-cpu-1/diag-cpu-1.listing.txt |
| CL-0009 | source-expectation | provisional | data/input/intv-2609-ntsc-family-p1.literal.txt |
| CL-0010 | source-expectation | provisional | data/input/intv-2609-ntsc-family-p1.literal.txt |
| CL-0011 | deterministic-repeatability | provisional | docs/input/controller-notes.md |
| CL-0012 | source-expectation | provisional | data/psg/rules.json |
| CL-0013 | deterministic-repeatability | provisional | docs/psg/reference-notes.md |
| CL-0014 | original-diagnostic | provisional | fixtures/FX-diag-ctrl-psg-1/diag-ctrl-psg-1.listing.txt |
| CL-0015 | installed-host-behavior | provisional | tests/installed-consumer/main.c |
| CL-0016 | source-expectation | provisional | data/stic/intv-2609-ntsc-family-p1.regmap.literal.txt |
| CL-0017 | deterministic-repeatability | provisional | docs/stic/timing-notes.md |
| CL-0018 | source-expectation | provisional | data/video/geometry.literal.txt |
| CL-0019 | source-expectation | provisional | docs/video/frame-derivations.md |
| CL-0020 | original-diagnostic | provisional | fixtures/FX-diag-stic-1/diag-stic-1.listing.txt |
| CL-0021 | deterministic-repeatability | provisional | docs/stic/reference-notes.md |
| CL-0022 | installed-host-behavior | provisional | tests/installed-consumer/main.c |
| CL-0023 | playable-guest-progress | provisional | data/golden/demo-lamp-tone-pass.expect |
| CL-0024 | installed-host-behavior | provisional | tests/installed-consumer/main.c |
| CL-0025 | source-expectation | provisional | tests/video/video_mob.c |
| CL-0026 | source-expectation | provisional | tests/video/video_collision.c |
| CL-0027 | original-diagnostic | provisional | fixtures/FX-diag-stic-2/diag-stic-2.listing.txt |
| CL-0028 | deterministic-repeatability | provisional | tests/state/state_cut.c |

Claims: 28 in total, 27 provisional, 1 established; none at tier measured-hardware.

By tier: source-expectation=15 original-diagnostic=4 deterministic-repeatability=5 installed-host-behavior=3 playable-guest-progress=1


**Reading of section F.** The script refuses with exit 1 when any claim stands at the measured-hardware tier, and it refused none: the totals line reads 28 claims, 27 provisional, 1 established, none at the measured-hardware tier, because no measurement on a console was made (D10 is not performed). [nc] The one established claim is CL-0002 (the printed STIC part and crystal figures of the 1980 data catalog); every other claim is provisional by the weakest-field rule. [claim:CL-0002] The four claims added by Phase 8 are CL-0025 (MOB, border and delay pictures), CL-0026 (collision registers), CL-0027 (the original STIC diagnostic program) and CL-0028 (snapshot, restore and run equals the uninterrupted run at 18 named hard points, one build, one profile); each is provisional and each carries an exclusion list in `data/claims.json`. [claim:CL-0025] [claim:CL-0026] [claim:CL-0027] [claim:CL-0028] The table above is the table of the `CHANGELOG.md` section `[0.1.0]`: the gate holds the two equal (the drift check of the `release-assets` step).


## G. Rights and package audit

- pinned install list (data/expected/pkg-install-files.ids): 11 files
  - include/ivs/ivs.h
  - include/ivs/ivs_machine.h
  - include/ivs/ivs_run.h
  - include/ivs/ivs_state.h
  - include/ivs/ivs_status.h
  - lib/cmake/ivs/ivsConfig.cmake
  - lib/cmake/ivs/ivsConfigVersion.cmake
  - lib/cmake/ivs/ivsTargets-release.cmake
  - lib/cmake/ivs/ivsTargets.cmake
  - lib/libivs_core.a
  - share/doc/ivs/LICENSE
- tracked binaries by name (git ls-files against tools/check/binary-ext.txt): 0
- fixture census (data/expected/fixtures.ids): 11 manifests, redistributable yes 8, no 2, unknown or other 1, missing 0
- name check (tracked paths holding the word Intellivision in any case): 0


**Reading of section G.** [nc]

- The install list is the eleven files of `data/expected/pkg-install-files.ids`: five headers, the static library, four CMake package files and the licence. Earlier Phase 8 text said ten; the count became eleven when `ivs_state.h` joined in plan 08-11.
- The count of tracked file names with a binary extension of `tools/check/binary-ext.txt` is 0. The check of the release archives themselves is content-based (`tools/check/release-assets.sh`, run by the gate on a synthetic archive and by `tools/release.sh` on the real ones), because the archive names end in the listed `gz` and `tar` extensions.
- Fixture census: 11 manifests; redistributable yes 8; no 2 (the two owner-image manifests, `committed: false`, no byte and no digest in the repository); other 1 (a sample manifest with unknown rights, treated as no).
- Not installed and not shipped: no EXEC, GROM, commercial ROM, manual, overlay or image; no runner, assembler or pack tool; no binary for any other system. A hash establishes identity, not rights, and no EXEC or GROM digest is recorded anywhere (`.planning/PROJECT.md`, row D11).
- Licence and notices: MIT, with the non-affiliation and working-name statements of `README.md` repeated in `CHANGELOG.md`; the working name is not trademark-cleared (owner item).
- Dependencies: none at runtime. The core is ISO C17 with no library; the gate needs CMake, a C compiler and POSIX shell.
- The per-file SHA-256 of the installed files is not printed here: the archive is build-specific, and the release run writes `SHA256SUMS`, which the draft check P8 reads (stage 2).


## H. Qualification

generated or written in Task 2

## I. Authority

generated or written in Task 2

## J. Hygiene

- tree scan (sh tools/hygiene.sh --tree), exit 0: mode=tree hygiene-files=2487
- history scan (sh tools/hygiene.sh --history), exit 1; findings compared by (token, short sha) with data/expected/hygiene-history-pinned.ids:
  - FAIL (accepted by recorded disposition, D-33): hygiene-message-home-path 955c13fa02
  - FAIL (accepted by recorded disposition, D-33): hygiene-history-home-path eaa832e3d3
  - FAIL (accepted by recorded disposition, D-33): hygiene-history-home-path 92c4dc7d37
  - FAIL (accepted by recorded disposition, D-33): hygiene-history-home-path b78af4fb9f
  - FAIL (accepted by recorded disposition, D-33): hygiene-history-home-path cccdfefdce
  - FAIL (accepted by recorded disposition, D-33): hygiene-history-home-path 345c48dd7f
  - history findings: 6 (accepted 6, new 0)
- disposition (D-33): accepted without a rewrite as a delegated decision; the baseline file is not moved and no rule is widened; a new finding fails this check; the owner may veto, a rewrite being an owner-only stop


**Disposition of the history findings (D-33).** The full history scan reported six findings and the scan itself exited 1; the readiness script exited 0 because each of the six is on the pinned list `data/expected/hygiene-history-pinned.ids` and is printed with its recorded disposition. The findings are accepted by a delegated decision; they are not cleared, and this record does not say that the history check passed, because it did not. [nc]

- The decision: accepted without a rewrite; pinned by (token, short sha) pair; any finding that is not on the list fails the release check (the script exits 1); the owner may veto the disposition and choose a rewrite, which is an owner-only hard stop (`.planning/AUTONOMY.md` section 1) and would not purge the pull-request refs that GitHub keeps.
- The matched text of each of the six is the shape of a scratch-directory path, not a path with user, host or session content (read by grep at planning time and recorded in `08-DISCUSS-release.md` section 7.3). The sixth pair names the reachable commit of the message finding; the Phase 4 and 7 records name another commit with the same subject that lies on no branch.
- Not touched by this plan: the baseline file `tools/check/hygiene-baseline.txt` (`git diff` of it against its last commit is empty) and every rule of `tools/hygiene.sh`. The baseline is not moved and no rule is widened.
- The scan ran in the background on the local laptop (load average 8 to 17), started 2026-11-15T15:57:58Z on the head `407d668`, took 253 s of wall time and ended with the script status 0. Plan 08-21 adds only `.planning/` text and commit messages after that head; the tree scan covers its text at every task, and the SUMMARY records a second history scan on the final head.
- Edit of one generated line: the tree-scan line is printed above without the two words that start its pass verdict, because the verify of the plan requires that this record never carry those words next to the history result; the scan's exit status 0 and its counters are unchanged (`mode=tree hygiene-files=2487`).
- The sections G and J together carry the checks the rest of the hygiene list of `.planning/AUTONOMY.md` section 6 asks for: tracked binaries (G), private paths and secrets (the tree scan), author emails (the history scan's identity check, which is part of the same run).

## K. Decision

generated or written in Task 2

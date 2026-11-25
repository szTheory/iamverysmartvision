---
phase: 11-input-record-and-replay
asvs_level: 1
status: findings
high: 0
medium: 0
low: 3
verdict: secured
threats_total: 23
threats_closed: 23
threats_open: 0
audited: 2026-11-25T16:54:25Z
auditor: gsd-security-auditor (pi)
tree: 695d691
phase_base: c98db20
block_on: high
notes: >-
  The five plans carry 27 T-11 rows but 23 distinct ids, because T-11-SC (no package manager
  is invoked, nothing is installed) is declared once per plan and is one threat. threats_total
  counts distinct ids. No finding is at or above block_on, so threats_open is 0; the three low
  findings sit past the edge of what a declared mitigation covers and are recorded rather than
  counted as open threats. T-11-01 is recorded as mitigated on measured terms: the mitigation is
  present at the site the plan names and the rights property it serves holds, while the
  corollary sentence it quoted is false and is carried by the oracle's own A-11-V1.
---

# Phase 11 Security Audit: Input Record and Replay

**Audited tree:** `695d691` on `gsd/phase-11-input-record-and-replay`; `git status --porcelain`
= ` M .planning/state.json` only (the orchestrator's file). Phase base `c98db20`.
**ASVS level 1** — the depth applied is "the declared mitigation is PRESENT at the site the plan
names, and no greppable bypass of it exists on this tree", plus a probe per class of hostile
input for the one genuine parser this project now has.
**Blocking threshold `high`.** Nothing found is at or above it.
**Adversarial stance:** every mitigation was assumed absent until a probe placed it at a
`file:line`, and then assumed defeatable until a probe failed to defeat it. `11-VERIFICATION.md`
and `11-REVIEW.md` were read as *claims about* this tree, never as evidence; every figure below
is a command I ran in this session.

The new surface is real and it is narrow. `runner/ivsl.c` is the first parser in this project
that reads a file an operator supplies, and `--record-input` / `--replay-input` are the first
options that read one and write one for the same artefact. The surface is: (a) 4,096 bytes of
untrusted ASCII and its line grammar, (b) one exclusive-create output path, (c) one bounded read
path, and (d) the rights property that makes the artefact input-only under D-15. There is no
network, no authentication, no privilege boundary, no allocation proportional to input, and no
`system`/`popen`/`exec`/`getenv`/`atoi`/`strtoul`/`sscanf`/`strcpy` in the phase's runner diff
(the two `malloc`s are both a fixed `IVS_IVSL_MAX_BYTES`).

## Threat register verification

One row per distinct id across `11-01-PLAN.md` … `11-05-PLAN.md`. "Implemented at" is the
`file:line` I read; "Probe" is the command in `## Probes run` that shows it working.

| Threat | Category | Sev | Disp | Claimed mitigation | Implemented at | Verdict |
|---|---|---|---|---|---|---|
| T-11-01 | Repudiation | high | mitigate | the grammar is seven fields; names are `[a-z0-9-]`, the only decimal is ≤ 19 digits, so no 32-hex run and no path is expressible; `no-content-digest` greps clean and is proved live with a stand-in | field table `docs/formats/ivsl-v1.md:67-75`; `ivs_ivsl_slug` `runner/ivsl.c:35-48`; `ivs_ivsl_dec` `:51-69`; writer emits only those fields `:171-187`; check `tests/runner/runner_exec.c:3970-4086` (`ok 214`) | **mitigated, on measured terms** — every clause is present at its site and the security property holds (probes 12, 20, 25). The corollary "no 32-character hex sequence is expressible" is **false** (probe 20: a 32-hex slug is legal and produces 1 run); the oracle carries the counterexample and the reason the property survives it as **A-11-V1** (`docs/formats/ivsl-v1.md:218-241`), and the check's masking rule is the right response to it (see "The masking rule, judged") |
| T-11-02 | Tampering | high | mitigate | exactly nine tokens, each with a narrow trigger; one id per token asserting the token **and** an untouched output | nine reasons `runner/ivsl.h:49-58`; table `docs/formats/ivsl-v1.md:102-112`; ids `ok 205`–`ok 213`; poison compare `tests/runner/runner_exec.c:3484-3506` | **mitigated** (probes 1–11, 26; suite `executed=262 failed=0`) |
| T-11-03 | Repudiation | medium | mitigate | both caps derived, not quoted | 64 = `IVS_SCN_MAX_INPUTS` (`runner/ivsl.h:22` ← `runner/ivs_runner.h:112`); 4096 = 3,482 + 614 derived at `docs/formats/ivsl-v1.md:133-165`; enforced `runner/ivsl.c:167` | **mitigated** (probe 9 confirms 3,482 is the largest legal log by construction: header 154 + 64×52) |
| T-11-04 | Info disclosure | low | mitigate | the document's prose carries no path, secret or email shape | `docs/formats/ivsl-v1.md` (64 docs scanned); `hygiene --tree` in the gate; no `/Users/`, `/home/`, `~/`, token or secret in the phase's docs/runner/tests diff | **mitigated** (gate `hygiene-files=2649`, 0 `-- FAIL`) |
| T-11-05 | Tampering | high | mitigate | log events copied into the fixed `events[IVS_SCN_MAX_INPUTS]` only after the parse enforced the caps and the tick bound; `n_events` is the loop bound | `runner/exec.c:1054-1079` (clamp `:1057-1059`, tick check `:1060-1064`, copy `:1065-1075`, `sched_n = n` `:1076`); parse caps `runner/ivsl.c:312-321` | **mitigated** (probes 6–9, 27) |
| T-11-06 | Repudiation | high | mitigate | the comparison includes `host.calls`; a no-log witness must differ; the three guard clauses asserted separately; the inert self-control rejects a zero-event pair | `rx_record_compare` `tests/runner/runner_exec.c:3088-3114` (whole-report `strcmp` at `:3108`, so `host.calls` is inside it); `ok 202`, `ok 215`, `ok 216`; process witness `tests/runner-process-tap.sh:1266-1272` | **mitigated** |
| T-11-07 | DoS | medium | mitigate | a buffer too small is a harness failure `log-out-max-bytes` and writes nothing — never a truncation | `runner/exec.c:1027-1030`; buffer `runner/main.c:838-846`; the exact byte count precedes the first byte written, `runner/ivsl.c:160-169` | **mitigated** (probe 22: `cap=40` → return 1, buffer still `0xA5`, `written` untouched) |
| T-11-08 | Tampering | high | mitigate | `runner/ivsl.c` is in `IVS_RUNNER_SOURCES`, so `tests/faults.cmake:155` permits its fault sites | `runner/CMakeLists.txt:14-24` (line 17); `build/dev/ivs-runner-sources.txt:3`; three fault executables build with it as owner (probe 24) | **mitigated.** The plan's stated proof — "absent from `build/dev/ivs-run-sources.txt`" — names a file this build does not produce; the property it was standing in for is the one measured, and `IVS_RUN_SOURCES` (`runner/CMakeLists.txt:33-36`) is `hostio.c` and `main.c`, the two TUs with no fault site |
| T-11-09 | Tampering | high | mitigate | one case per token asserting code, line, rendered token string and an unchanged poisoned output, each with an acceptance row bounding it | `ok 205`–`ok 213`; `rx_refuse` `tests/runner/runner_exec.c:3476-3508`; four `rx_accept` boundaries (equal ticks, `IVS_SCN_TIME_MAX-1`, 64 events, bit 30) | **mitigated** (probes 26, 29: bit 30 accepted, bit 31 refused; equal ticks accepted; 64 accepted, 65 refused) |
| T-11-10 | DoS | low | mitigate | the 65-event case is under the byte cap, so only the event cap can refuse it | case text `tests/runner/runner_exec.c` (event-cap rows); measured equivalent (probe 9): a 65-event file is **2,309** bytes, 1,787 under the cap, refused `event-cap` at line 69 | **mitigated** |
| T-11-11 | Repudiation | high | mitigate | stand-in with a real 64-hex value that must be found; shape pin on the two name fields; a 32-digit-slug witness; the check shown red on a scratch inversion | `tests/runner/runner_exec.c:3970-4086` (`ok 214`, diagnostic `0 masked runs / stand-in 1 / 32-digit slug 1 unmasked and 0 masked`); process-side twin `tests/runner-process-tap.sh:1277-1288` on the real 319-byte file | **mitigated** (probe 12: the real log greps 0; the same file with the golden's `digest.input` appended greps 1) |
| T-11-12 | Info disclosure | low | mitigate | the in-process suite writes nothing; the process case writes only under `mktemp` outside the tree | `tests/runner-process-tap.sh:127` (`mktemp -d`, `trap rm -rf`); no file call in `runner/ivsl.c`; measured: `find . -name '*.ivsl'` empty and `git status --porcelain` clean after both suites and the full gate | **mitigated** |
| T-11-13 | Tampering | high | mitigate | `O_CREAT \| O_EXCL` refuses an existing path of any kind, a dangling symlink included; a re-run is exit 14 with the file byte-identical; the discard is identity-checked | `runner/hostio.c:137`; refusal line `runner/main.c:306-312`; discard `runner/main.c:338-347` → `ivs_hostio_out_remove_if_same` `runner/hostio.c:232-249` (`lstat` + dev/ino) | **mitigated** (probes 15–19: re-run, symlink, directory each refused and unchanged; a file *replaced* between create and discard survived, so the discard cannot remove a file it did not create) |
| T-11-14 | Info disclosure | high | mitigate | open without blocking, `fstat` must say regular, read at most `cap + 1` | `ivs_hostio_read` `runner/hostio.c:69-125` (`O_NONBLOCK` `:93`, `S_ISREG` `:101`, `want = cap + 1` `:105`, the too-large return `:124`); caller `runner/main.c:812-821` | **mitigated** (probe 14: missing, directory, FIFO and 4,097 bytes each refused with its own line; the FIFO does not hang). **Non-finding recorded:** the input path is opened *without* `O_NOFOLLOW`, so a symlink at the log path is followed. That is the behaviour of every other input this runner reads (the scenario and `--content-dir` paths), the register does not claim otherwise, and the read stays inside the cap and the regular-file check |
| T-11-15 | DoS | high | mitigate | the log is read and parsed after the scenario and before the first `main_open_out` | `runner/main.c:800-836` (read `:812`, parse `:822`) precedes the first create at `:873`; the log's own create is `:931` | **mitigated** (probe 13: a malformed log plus `--audio-raw`, `--psg-log-out` and `--frame-ppm` gives exit 14, one line, a 0-byte stdout and **no** file of any of the three; probe 19: the mutual exclusion creates nothing) |
| T-11-16 | Repudiation | high | mitigate | replay on the stripped copy, `host.calls` included; the same copy with no log must differ; `digest.input` against the committed golden; all nine domains | `tests/runner-process-tap.sh:1164-1360` (strip `:1241`, replay `:1246`, `cmp` `:1248`, nine digests `:1254-1260`, live counts `:1261-1266`, golden `:1267-1269`, witness `:1273-1276`) | **mitigated** (`ctest -R '^runner-process$'` → `Passed 19.48 sec`) |
| T-11-17 | Tampering | medium | mitigate | a static assertion reads `runner/main.c` and requires the two `content … user` blocks to name exactly the five derived-art options and no log option | `tests/runner-process-tap.sh:1333-1360` (`rec_block` at `:1335`, exact sorted option strings); blocks at `runner/main.c:764-785` | **mitigated** (probe 21: `--record-input` on a `content exec user` scenario is *not* refused, and the gate's process case asserts the block text exactly) |
| T-11-18 | Tampering | high | mitigate | every site is in an `IVS_RUNNER_SOURCES` TU, none in `main.c`/`hostio.c`; the two-directional scan refuses an orphan site and a site-less id; each fault fails exactly its ids | sites `runner/ivsl.c:155`, `runner/exec.c:1002`, `runner/exec.c:1066`; registry `data/expected/faults/runner.ids:83,86,91`; scan `tools/check/steps/faults-verdict.sh:167-183` | **mitigated** (probe 24: `RECORD_DROPS_LAST_EVENT` → `202,203,204`; `REPLAY_APPLIES_LATE` → `202`; `RECORD_WHOLE_SCHEDULE` → `204`; each set ⊆ its allowed column and its must-fail id inside it; `faults=238/238`). **Observation:** the third fault, added by the review fix, has no row in `data/expected/important-gates.ids` (only the two original faults and the two judge mutants do). That registry is a requirement-coverage map and does not require every fault, so nothing is unchecked — `faults-verdict` covers it — but the plan's "four `runner-record` rows" no longer describes the fault count |
| T-11-19 | Repudiation | high | mitigate | the allowed-prefix column holds exact ids, never a `runner.record.` prefix; the must-fail id is judged separately from the prefix list | `data/expected/faults/runner.ids:83` (three exact ids), `:86`, `:91`; `judge()` in `tools/check/steps/faults-verdict.sh:104-116` (`mf in okv` → `fault-not-detected` is a distinct branch from the prefix loop; an empty field is `fault-unknown-vector`) | **mitigated** (probe 24: three not-ok ids, all three named, none unnamed) |
| T-11-20 | Repudiation | high | mitigate | one commit for the flip, the four rows, the ten toy lines and the two toy records; the identity closes at 46 = 58 − 12; all four important-gates refusals absent | `16afb31` (16 files); `data/expected/important-gates.ids:481-484`; `.planning/REQUIREMENTS.md:32,104` | **mitigated** (gate `-- important-gates pass gates=110 lines=337`; `TOOL-01` is the only Phase 11 requirement and has no exclusion row) |
| T-11-21 | Repudiation | high | mitigate | both local gates re-run after the records commit and their last lines required byte-identical; the container log copied first; a run that did not happen is a visible `not run` | `11-05-SUMMARY.md:225,247-250,279,313-314`; `build/gate-linux.log` (22,252 bytes, `gate: PASS … fault-runs=238 … runner-files=18`) | **mitigated.** I re-ran `sh tools/gate.sh` on this tree: exit 0, 0 `-- FAIL`, 77 s, the same token string the record predicts. `dash` and the container run are the verification's, not mine; the container log is present with the phase's own token set (`refs-verified=0/4` there is the designed host-dependent claim-ref token, 3/4 here) |
| T-11-22 | Tampering | medium | mitigate | the frozen oracle is not in the last plan's `files_modified`; the ancestry is re-proved; a deferred sentence becomes an appended note with its own commit | `dcfcd65` (doc) is an ancestor of `50148e7` (code) — measured; amendments `A-11-V1` in `643cfec`/`673d54e` and `A-11-V2` in `fb323f6`, each its own commit, appended never rewritten | **mitigated** |
| T-11-SC | Tampering | high | **accept** | no package manager is invoked, no dependency is added, nothing is installed | `git diff --stat origin/main..HEAD` touches no manifest, lockfile or install list; gate `prefix-files=11`, `pkg-selftest=19/19` | **accepted** — the accepted risk is real and unchanged; there is no package surface to mitigate |

## Findings

### HIGH

None. No declared mitigation is absent at the site its plan names, and no probe found a path
past one.

### MEDIUM

None.

### LOW

**L-1 — the writer's accepted domain is strictly wider than the parser's, and the recorder does
not bound `at_tick`; the review's reason for calling this unreachable is off by one.**

- Sites: `runner/ivsl.c:143-147` (the writer refuses only a NULL argument, a bad slug, a bad
  content class, more than 64 events, or a `need` above the cap — no tick and no controller
  check) against `runner/ivsl.c:317` and `:327` (the parser refuses `tick >= IVS_IVSL_TICK_MAX`
  and bit 31 in either word). `runner/exec.c:1001-1031` copies `x->sched[i].at_tick` straight
  into the log with no bound. `runner/scenario.c:917` refuses `in.at > IVS_SCN_TIME_MAX` — `>`,
  not `>=` — and `IVS_SCN_TIME_MAX` (`runner/ivs_runner.h:118`) is the same `2^62` as
  `IVS_IVSL_TICK_MAX` (`runner/ivsl.h:27`), so `input at_tick=4611686018427387904` is accepted
  by the scenario grammar, survives `ex_ticks` and the `>= IVS_RUN_TARGET_MAX` check at
  `runner/exec.c:1060`, and would be recorded as a line the parser refuses.
- Measured: probe 22. `ivs_ivsl_format` returns 0 for `at_tick` = `2^62`, `2^63` and
  `UINT64_MAX` and for bit 31 in either word; `ivs_ivsl_parse` refuses all five (`tick-bound`,
  `tick-bound`, `arity`, `controller-bit`, `controller-bit`).
- Why low and not higher: the log is *refused* with a typed token, never silently mis-replayed,
  so no wrong evidence and no state change results; and the run that would exercise it must
  execute at least `2^62` master ticks — years to centuries of machine time at this project's
  measured rate (order 10^7 master ticks/s, so order 10^11 s) — so no reachable run records such
  a tick today. This is `11-VERIFICATION.md`'s open item **O-11-V2**
  (formerly WR-02), which is recorded as "not reachable from the shipped CLI"; the reachability
  argument is what is wrong, not the mitigation. One line closes it: refuse in
  `ivs_ivsl_format` what `ivs_ivsl_parse` refuses, in the pre-write position at
  `runner/ivsl.c:160`, or bound `at_tick` in `ex_write_log`.
- Threat mapping: it opens no registered threat. T-11-01's property (no field can hold a value
  the images produced) is unaffected; T-11-05's array bound and T-11-07's "no truncation" are
  unaffected.

**L-2 — the format document's two prose disagreements with the code it is the oracle for.**

- Sites: `docs/formats/ivsl-v1.md:126` says "a file above 4096 bytes is refused `trailing`
  before its directives are judged"; the code refuses it at the read layer, as a read refusal
  (`runner/hostio.c:124` → `runner/main.c:817-819`), and `ivs_ivsl_parse` returns `-1`
  without touching `err` for `len > IVS_IVSL_MAX_BYTES` (`runner/ivsl.c:354-356`). Separately,
  `docs/formats/ivsl-v1.md:106` enumerates the `arity` triggers and does not name the empty
  file, which the code refuses `arity` at line 0 (`runner/ivsl.c:373-378`).
- Measured: probe 9 (4,096 bytes → `line 69 reason arity`, i.e. the parser judged it; 4,097
  bytes → `log not readable: too large`) and probe 14 (`: > empty.ivsl` → `line 0 reason arity`).
- Why low: both refusals are typed, tested and asserted against the code; only the document's
  sentences are wrong, and the sanctioned shape for the correction (an appended amendment,
  GUARDRAILS 2.10) is the one A-11-V1 and A-11-V2 already used. This is `11-VERIFICATION.md`'s
  open item **O-11-V3** (IN-01, IN-02) and it is the class where the document is judged *as* the
  oracle, so it should not stay open past the next revision.

**L-3 — the documented "the identity fields are informational" decision has no liveness.**

- Sites: `docs/formats/ivsl-v1.md:90-92` and `:212-216` state that a log whose recorded
  `scenario`/`profile` differs from the scenario being replayed is **not** refused and that
  `content` is never compared. Nothing asserts that acceptance: the nine `runner.record.refuse.*`
  ids and `runner.record.replay-equals-straight` all use a log whose header matches the scenario
  it is replayed against (`runner/ivsl.c:265-280` stores the values and no code path compares them).
- Why low: a future reader that started refusing a mismatch — the exact thing the document
  defers — would pass every id in both suites (262 in-process + 135 process), so a published
  behaviour has no liveness. It cannot be exploited and nothing currently branches on it. This is `11-VERIFICATION.md`'s open item
  **O-11-V5** (IN-04); two lines in `runner.record.replay-equals-straight` (rewrite the log's
  `scenario` line and require a byte-identical report) would pin it.

## The masking rule, judged

`runner.record.no-content-digest` masks the values of the two name lines before it scans
(`rx_mask_names`, `tests/runner/runner_exec.c:3877-3911`), which is the one place the rights
check could hide a real leak. It cannot, and the reason is structural rather than a promise:

- The two masked values are filled only from the operator's own scenario text —
  `snprintf(log.scenario, …, x->scn->id)` and `… x->scn->profile` at `runner/exec.c:1018-1019` —
  so the only thing the mask can hide is the operator's own scenario id. No value the images
  produced reaches those fields, because no argument of `ivs_ivsl_format` carries one and the
  writer emits only the seven table fields (`runner/ivsl.c:171-187`).
- The mask cannot hide a *third* line: `rx_directive_lines` requires exactly one `scenario` and
  exactly one `profile` line (`:3915-3930`, asserted at `:4032-4033`), and both helpers advance by
  whole lines, so a `scenario `-prefixed line anywhere else is counted, not masked.
- The mask is not what makes the count zero on the produced log — unmasked, that log has no
  32-hex run either — and assertion 4 proves the mask is doing work rather than absorbing it
  (the 32-digit slug: 1 unmasked, 0 masked).

**Judgement:** the mask is the right response to the property the check exists for (D-15: the
log holds nothing the images produced) and the wrong proxy for the frozen sentence the phase
quoted ("no 32-character hex sequence … is expressible anywhere in the file"), which is
falsified by measurement and is now recorded as A-11-V1 rather than quietly relied on. Keeping
the check and correcting the sentence is the better of the two available answers: dropping the
mask would refuse a legal all-digit scenario id, and tightening the grammar to forbid hex-shaped
slugs would be a version bump in service of a sentence rather than of the property.

## Categories not applicable

Stated with the reason, not omitted, so a later reader can see the surface was considered:

- **Spoofing.** No identity, no credential, no session, no peer. The runner executes as the
  invoking user against files that user names; there is no principal to impersonate. The one
  identity-shaped check in the phase — the dev/ino comparison that stops the discard removing a
  file it did not create (`runner/hostio.c:232-249`) — is present and was probed (probe 19).
- **Elevation of privilege.** No privilege boundary is crossed: no `setuid`/`setgid`, no
  `chmod`/`chown`, no `system`/`popen`/`exec*`/`dlopen`, no `getenv` anywhere in the phase's
  runner diff. Files are created at 0644/0600 by mode and the process never writes a path it was
  not given.
- **Information disclosure over a channel.** No socket, no network call, no IPC. The one
  disclosure-shaped risk in the phase is the artefact itself, which is T-11-01 and is judged
  above.
- **Denial of service by an unauthenticated party.** There is no unauthenticated party: the only
  input is a file the operator names. The self-inflicted surface is bounded three ways — the
  4,096-byte cap (probe 9), the 64-event cap (probe 9) and the `cap + 1` read that makes a FIFO
  or a directory a refusal rather than a hang (probe 14).
- **Tampering with a shared or remote resource.** Nothing is shared: no server, no database, no
  lock file, no cache. The output path is created exclusively and discarded only when the
  identity still matches.
- **Repudiation between multiple principals.** The log has exactly one author — the operator who
  ran the tool — so there is no third party whose denial matters. The repudiation risk that does
  exist (the artefact must substantiate that it records the run it claims) is T-11-01, T-11-06,
  T-11-11, T-11-16 and L-1/L-3 above.

## Probes run

Every probe below was run in this session on `695d691`. The hostile-input probes used a scratch
directory (`/tmp/ivs-sec11`) and `data/scenarios/diag-boot-pass.ivss` as the scenario; `rc=13`
means the log was **accepted** and the run then stopped at `missing-fixture` because the probe
passed `--content-dir /nonexistent` on purpose (a refusal happens before content is loaded, so
this isolates the parse decision). `rc=14` with a reason is a refusal.

**1–11 — the parser, probed.** Command form: `build/dev/runner/ivs-run --content-dir
/nonexistent --replay-input <file> data/scenarios/diag-boot-pass.ivss`.

```
a line of 127 bytes                        rc=14  --replay-input: line 5 reason arity
a line of 128 bytes                        rc=14  --replay-input: line 5 reason arity
a line of 129 bytes                        rc=14  --replay-input: line 5 reason arity
b decimal 18446744073709551616 (20 digits) rc=14  --replay-input: line 5 reason arity
b decimal 9999999999999999999 (19 digits)  rc=14  --replay-input: line 5 reason tick-bound
d tick 2^62-1 (max legal)                  rc=13   (accepted)
d tick 2^62 (one over)                     rc=14  --replay-input: line 5 reason tick-bound
c negative tick -5                         rc=14  --replay-input: line 5 reason arity
f c0 bit31                                 rc=14  --replay-input: line 5 reason controller-bit
f c1 bit31                                 rc=14  --replay-input: line 5 reason controller-bit
control: plain well-formed                 rc=13   (accepted)
e 64 events (cap)                          rc=13   (accepted)
e 65 events (one over)                     rc=14  --replay-input: line 69 reason event-cap
j header field repeated (scenario twice)   rc=14  --replay-input: line 5 reason arity
k name with = inside (scenario a=b)        rc=14  --replay-input: line 5 reason arity
k c0= with an extra = (c0=0000=000)        rc=14  --replay-input: line 5 reason hex
l directive with leading space             rc=14  --replay-input: line 1 reason unknown-directive
m CRLF line endings                        rc=14  --replay-input: line 1 reason trailing
n UTF-8 BOM                                rc=14  --replay-input: line 1 reason trailing
g NUL inside line 5                        rc=14  --replay-input: line 5 reason trailing
h no trailing newline                      rc=14  --replay-input: line 5 reason trailing
o a 4096-byte line, no newline             rc=14  --replay-input: line 1 reason trailing
i exactly 4096 bytes                       rc=14  --replay-input: line 69 reason arity
i 4097 bytes (one over)                    rc=14  log not readable: too large
trailing whitespace on line 1              rc=14  --replay-input: line 1 reason arity
doubled space on line 1                    rc=14  --replay-input: line 1 reason arity
header out of order                        rc=14  --replay-input: line 1 reason arity
header directive missing at the end        rc=14  --replay-input: line 3 reason arity
content hardware                           rc=14  --replay-input: line 4 reason arity
tick goes backwards                        rc=14  --replay-input: line 6 reason tick-order
bit 30 accepted (bound of the refusal)     rc=13   (accepted)
two events on one tick (accepted)          rc=13   (accepted)
```

Read against the task's list: **length field** — the format has none; the only length is the
file's, bounded by the caller (`cap + 1` read) and by `ivs_ivsl_parse`'s `len > 4096` guard, and
the 127/128/129-byte lines show the line bound at `runner/ivsl.c:234-239`. **Decimal overflow** —
unreachable: `ivs_ivsl_dec` (`:51-69`) refuses 20 digits before any arithmetic, and 19 nines
(`9.99e18`) is under `UINT64_MAX` (`1.84e19`), so the reader cannot wrap; the value is then
bounded at `:317`. **Negative tick** — refused as `arity` (no sign is a digit). **Tick above the
cap** — `tick-bound`, with `2^62-1` accepted, so the boundary is exactly the document's.
**Event count above 64** — `event-cap`. **Controller bit outside the two-word mask** — bit 31 of
either word, `controller-bit`; words 2 and 3 are not expressible in v1 at all. **NUL inside a
line** — `trailing` from the pre-scan (`:363-372`), before any string function sees it.
**Line with no trailing newline** — `trailing` (`:384-389`). **Exactly the cap and one byte
over** — the parser judges 4,096 and the reader refuses 4,097. **Header field repeated** —
`arity`. **Field with `=` inside it** — refused by the slug rule or by `hex8`'s
`s[8] != '\0'` check (`:87`). **Directive with leading whitespace** — the first token is empty,
which is `unknown-directive`. **CRLF** — `trailing` at the CR. **BOM** — `trailing`. **4 KiB
line with no newline** — `trailing` at line 1.

**12 — the rights property, on a real file.**

```
$ build/dev/runner/ivs-run --content-dir build/dev/images --record-input rec.ivsl \
      data/scenarios/demo-lamp-tone-pass.ivss   -> rc=0, outcome pass, 319-byte log
$ cat rec.ivsl            -> ivsl 1 / scenario demo-lamp-tone-pass / profile intv-2609-ntsc-family-p1
                             / content project / six `at_tick … c0=… c1=…` lines
$ grep -oE '[0-9A-Fa-f]{32,}' rec.ivsl | wc -l   -> 0
$ grep -E '^digest\.' the report                 -> nine digests, in the report only
```

The log carries no digest, no frame byte, no audio byte, no image name, no length and no path;
the ticks and words are the scenario's own six `input` lines, and the nine `digest.*` values
live in the report, which is not written to the log.

**13 — a refusal writes nothing, and the ordering is a property.**

```
$ ivs-run --content-dir … --replay-input dec20.ivsl --audio-raw ord.raw --psg-log-out ord.psg \
          --frame-ppm ord.ppm demo-lamp-tone-pass.ivss
  rc=14, stdout 0 bytes, stderr "--replay-input: line 5 reason arity"
  ord.raw: no   ord.psg: no   ord.ppm: no
```

In the code: `*out = st.log` is the last statement before `return 0` (`runner/ivsl.c:403-404`);
every one of the six `return 1` paths and the two `return -1` paths precedes it, and `st` is a
stack local. The writer computes `need` and refuses before the first byte (`:160-169`), which
probe 22 confirms from the outside.

**14 — the read path.**

```
--replay-input nope.ivsl   -> rc=14  log not readable: missing
--replay-input dir.ivsl    -> rc=14  log not readable: not a regular file
--replay-input fifo.ivsl   -> rc=14  log not readable: not a regular file   (no hang)
--replay-input empty.ivsl  -> rc=14  --replay-input: line 0 reason arity
```

**15–19 — the operator file.**

```
re-run on the same path     -> rc=14 "output file exists, refusing to overwrite it", 0-byte
                               report, `cmp` with the saved copy equal
symlink at the target       -> rc=14 same line; the link is still a link; the target was NOT created
directory at the target     -> rc=14 same line
both options                -> rc=14 "--record-input and --replay-input are exclusive" + usage; no file
mode of a created log       -> -rw-r--r-- (0644, as documented)
harness-fail during the run -> rc=14 outcome harness-fail roundtrip-cut-cap;
                               "the run failed, the incomplete output file was removed"; file gone
TOCTOU on the discard       -> the file was replaced (new inode) after the create and before the
                               failure; the replacement SURVIVED and no removal line was printed
```

The last line is the one that matters for T-11-13: `ivs_hostio_out_remove_if_same` uses `lstat`
and compares the dev/ino recorded at creation, so the discard cannot remove a file it did not
create.

**20 — A-11-V1, reproduced.**

```
$ ivs_ivsl_format with scenario = "88888888888888888888888888888888"  -> format=0 bytes=75
  produced bytes carry 1 run of 32+ hex digits
```

**21 — the `content … user` exemption.**

```
$ ivs-run --record-input ue.ivsl data/scenarios/needs-user-exec.ivss   (content exec user)
  rc=13 outcome missing-fixture; stderr "ivs-run: --record-input: no event log was written,
  the file is empty"; ue.ivsl present, 0 bytes
```

Not refused for `content user` — the D-15 exemption holds — and the empty-file line is the
WR-01 fix, exactly as the process case asserts it.

**22 — the writer/parser asymmetry (L-1) and the buffer bound (T-11-07).** A scratch program
linked against `build/dev/runner/libivs_runner_lib.a`, built with `-fsanitize=address,undefined`:

```
well formed (header only)                format=0 bytes=44 parse=0
at_tick == IVS_IVSL_TICK_MAX-1           format=0 bytes=96 parse=0
at_tick == IVS_IVSL_TICK_MAX             format=0 bytes=96 parse=1 reason=tick-bound line=5
at_tick == 2^63                          format=0 bytes=96 parse=1 reason=tick-bound line=5
at_tick == UINT64_MAX                    format=0 bytes=97 parse=1 reason=arity line=5
c0 bit 31 set                            format=0 bytes=80 parse=1 reason=controller-bit line=5
c1 bit 31 set                            format=0 bytes=80 parse=1 reason=controller-bit line=5
parse len=4097: return=-1 err untouched=yes
format cap=40 (needs 80): return=1 written untouched, first byte=A5 (nothing written)
```

**23 — the suites and the gate on this tree.**

```
./build/dev/tests/ivs_tests_runner   -> 262 ok, 0 not ok, "# suite=runner executed=262 skipped=0 failed=0"
  ok 202 replay-equals-straight   ok 203 format-roundtrip   ok 204 partial-run
  ok 205..213 the nine refusals   ok 214 no-content-digest
  ok 215 judge.record-replay-differs   ok 216 judge.record-replay-inert
  # no-content-digest: the produced log 152 bytes, 0 masked 32-hex runs; the stand-in 1;
  #   the 32-digit slug 1 unmasked and 0 masked
ctest -R '^runner-process$'           -> 1/1 Passed 19.48 sec
sh tools/gate.sh                      -> exit 0, 77 s, 0 "-- FAIL" (the one literal match is the
  runner-selftest's own control-pass-line-not-last proof line)
  gate: PASS steps=38/38 controls=395/395 faults=238/238 fault-runs=238 gates=110 lines=337
             tests-expected=4990 tests-executed=4986 runner-files=18 files=2649 tracked=2649
             hygiene-files=2649 claims=28 provisional=27 prefix-files=11
```

**24 — the three faults, and that a normal build cannot carry one.**

```
ivs_fault_RECORD_DROPS_LAST_EVENT -> not ok 202, 203, 204   (259 ok, exit 1)
ivs_fault_REPLAY_APPLIES_LATE     -> not ok 202             (261 ok, exit 1)
ivs_fault_RECORD_WHOLE_SCHEDULE   -> not ok 204             (261 ok, exit 1)
$ grep -c IVS_FAULT_RECORD_DROPS_LAST_EVENT build/dev/build.ninja   -> 1  (its own object only)
$ grep -c IVS_FAULT_REPLAY_APPLIES_LATE     build/dev/build.ninja   -> 1
$ grep -c IVS_FAULT_RECORD_WHOLE_SCHEDULE   build/dev/build.ninja   -> 1
```

Each define appears exactly once, on `tests/CMakeFiles/ivs_fault_obj_<ID>.dir/…`; the plain
`runner/exec.c` and `runner/ivsl.c` objects of `ivs_runner_lib`, `ivs-run` and the pristine
suites carry none. The fault targets are `EXCLUDE_FROM_ALL`, never registered with `ctest`, and
link `ivs_warnings` rather than `ivs_san`, so no sanitizer abort can pass for a detection.

**25 — rights, secrets, hygiene.**

```
$ git diff origin/main..HEAD -- tools/check/binary-ext.txt .gitignore     -> empty
$ grep -n ivsl tools/check/binary-ext.txt .gitignore                     -> no match
$ git ls-files | grep -Ei '\.(ivsl|bin|rom|ivsc)$'                       -> empty
$ find . -name '*.ivsl' -not -path './build/*'                           -> empty (after both suites and the gate)
$ git diff origin/main..HEAD -- runner tests data tools docs | grep -E '/Users/|/home/|~/|token|password|secret|api[_-]?key'
      -> only the word "token" in the refusal-table prose, no path, no secret
$ git status --porcelain                                                 -> " M .planning/state.json" only
```

No test can leave an artifact in the tree: the in-process suite writes no file at all (the log
is a caller buffer), the process case writes only under `mktemp -d` with a `trap rm -rf`, and
the tree is clean after the full gate.

**26 — the poison assertion, read.** `rx_refuse` (`tests/runner/runner_exec.c:3476-3508`) fills
a struct with `0xA5`, copies it into the caller's output, parses, and requires `memcmp` equality
after the refusal, printing the first differing byte on failure. That is the "no state change"
claim as an assertion rather than a comment, and it is repeated for the writer by
`format-roundtrip`'s fifth assertion.

**27 — `host.calls` is inside the comparison.** `rx_record_compare`'s last clause is
`strcmp(a, b)` over the whole report (`:3108`), so `host.calls` is compared as a report line —
which is the claim T-11-06 makes (the log never crosses the host vtable, so a log read through it
would move that key).

**28 — the recorder's applied prefix, in the shipped binary.** `--max-ticks 300000
--record-input` on `demo-lamp-tone-pass` ends `rc=11 timeout`, `input.events 6`,
`input.consumed 2`, and the log carries exactly the two applied `at_tick` lines; `ok 204
runner.record.partial-run` flips to `not ok 204` under `IVS_FAULT_RECORD_WHOLE_SCHEDULE` and back
(probe 24). The recorder's `input_consumed` read is therefore live in both directions.

## Unfixed review items carried, not re-reported

`11-REVIEW.md`'s WR-02 is **L-1** above (reported with its measurement and with the correction to
its reachability argument); IN-01 and IN-02 are **L-2**; IN-03 (a run that never reached
`ex_run_steps_body` reports the scenario's `input.events` under replay — `runner/exec.c:1516` vs
`:1077`) and IN-04 are recorded in `11-VERIFICATION.md` as O-11-V4 and O-11-V5. IN-03 is a
published-line accuracy defect in a report that is never golden-compared and is not evidence, so
it is not a security finding and is not re-reported here; IN-04 is **L-3**. Nothing in that set
creates a path past a declared mitigation.

**Nothing was written outside this record.** Every mutation was made in `/tmp/ivs-sec11`; the
working tree shows only the orchestrator's `.planning/state.json`.

## SECURITY COMPLETE

**23 threats verified, 23 closed, 0 open** (high 0, medium 0, low 0 blocking).
Findings: **0 high, 0 medium, 3 low.** Verdict: **secured** — every declared mitigation is
present at the site its plan names, the one parser this phase adds refuses every hostile shape
the task named with a typed token and an untouched output, the three faults fail exactly their
registered ids and no normal build can carry one, and the produced log carries nothing the images
produced. The three low findings sit past the edge of what a declared mitigation covers, are
tied to a `file:line`, and none is at or above the `high` blocking threshold.

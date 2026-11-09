# Emulator engineering DNA for iamverysmartvision

Prepared 2026-11-09 as self-contained input to the GSD kickoff. It combines
read-only observations of current sibling projects, LibFuturo contract notes
and Intellivision research leads. It is not a console specification, accepted
architecture, requirements or implementation report. No sibling builds,
tests, CI, games or hardware were run.

## Transferable emulator lessons

### Contracts and architecture

- Keep machine instances explicit. Define content, input, output-buffer,
  reset/unload, error, execution-bound and concurrency ownership. A context
  shaped API does not prove independent instances or safe reentrancy.
- Keep one canonical core. Separate CPU/device behavior from host file access,
  wall-clock pacing, physical controller discovery, A/V presentation and
  persistence policy. A headless consumer exposes core issues without a full UI.
- Use explicit versioning, size/compatibility rules and refusal behavior when
  they solve a real compatibility need. Test old prefixes, malformed requests,
  capacity boundaries and whether failed operations preserve promised state.
  Do not copy another emulator's exact C layout, status numbers or tick unit.
- Prove installed artifacts with a clean independent consumer before claiming
  package/integration success. Source-tree tests miss relocation,
  generated-file and runtime-loading failures.

### Tests and accuracy

- Define evidence tiers separately: source expectation, public vector, original
  diagnostic, independent implementation comparison, measured hardware,
  deterministic repeatability, playable guest progress and installed-host
  behavior. One tier does not imply the next.
- Use small authored diagnostics with independent expected results. Record
  fixture source, recipe, digest, notices, license and permission scope. A
  checksum identifies bytes; it does not grant rights.
- Drive deterministic input at an explicit machine/time boundary and use fresh,
  isolated machine/save state. Check guest semantics as well as execution
  progress, raw video and raw audio. Same-core hashes show consistency, not
  correctness.
- Give important gates exact inventories and known-fault controls. Fail visibly
  when required tests are missing, skipped, duplicated or consuming stale
  output. Distinguish harness failure, guest failure, timeout, cancellation and
  missing fixtures.
- Add API refusal/lifecycle cases only as the surface exists: malformed or
  oversized content, unsupported profile, short outputs, reset/unload, multiple
  instances and state/save refusal behavior.
- Keep runner, module, frontend and physical-device evidence separate. UI tests
  do not qualify the core; an installed adapter does not qualify accuracy.

### Performance, CI and workflow

- Start with one documented local command aligned to the fast PR gate. Give
  each required CI job a distinct failure class. Stage broad matrices, fuzzing,
  soak and performance distributions until value and cost are understood.
- Measure queue, setup, compile and test critical path before adding caches,
  concurrency or broad matrices. Avoid repeated work only when a clean
  comparison proves savings and invalidation/failure behavior is tested.
- Benchmark a named workload/build/configuration and hold correct outputs
  constant. Keep throughput, end-to-end latency/resource use and CI time
  separate. Stop when profiles cannot attribute cost to the suspected code.
- At milestone discussion, reread current project state and relevant sibling/API
  evidence. Fan out across only roles that can change the decision: Intellivision
  hardware, CPU/bus, graphics/video, sound, input, API/host, QA,
  security/licensing, DevOps/performance and product. Compare options, failure
  modes and cost; include an adversarial pass, then stop when more research
  stops changing the recommendation/evidence plan.
- Close milestones with main green, PR triage, updated rolling roadmap/lessons
  and a release target. Release the exact tested artifact only when package
  contents, fixture rights, qualification and authority are ready. GSD
  milestones and software versions are different.
- Routine reversible work can proceed autonomously. Automation does not accept
  unresolved rights, authorize external publication/hardware testing or replace
  a material owner decision.

## Cross-project synthesis

Recent work across several console cores and a shared integration API reinforces
these transferable methods:

- Exact test inventories, explicit named-fault controls, fixture provenance and
  independent expected values make verification easier to trust.
- A test card, synthetic session, or successful source-tree build proves only
  that bounded behavior. It does not establish console accuracy, gameplay,
  frontend input or installed-package behavior.
- Bind qualification to the exact source/build/package/content identity and
  route that was tested. Keep runner, module, frontend and actual-device
  evidence distinct.
- Preserve failed measurement attempts and source attribution. Do not repeat an
  expensive performance run unless its workload, input, tool and question are
  concrete and the attempt can answer them.
- Measure the full CI critical path early. Avoid letting documentation,
  redundant builds or setup consume workers needed by distinct functional
  platform jobs. Change concurrency, timeouts or caches only from attributable
  evidence.
- Use project-owned current state to identify unfinished work and existing
  stops. Do not transplant another project's gate, API, thresholds, fixture or
  active stop into this emulator.
- A shared C interface can be a candidate without being a stable ABI or a
  qualified route. Confirm the exact new core profile and independent consumer
  before proposing any shared-contract change.

These are method-level conclusions, not fresh test results from this review.
No sibling build, test or CI run was performed.

## Intellivision-specific research leads

Accessed 2026-11-09. Sources have different authority. Check primary manuals
and component data sheets before freezing timing/register requirements.

| Source | Useful observations | Authority and limits |
|---|---|---|
| [MAME Intellivision driver](https://github.com/mamedev/mame/blob/master/src/mame/mattel/intv.cpp) | Current device/memory configurations for base Intellivision, Intellivision II, ECS and IntelliVoice combinations; separate EXEC/GROM/GRAM and cartridge maps; lists Sears Super Video Arcade and an unreleased keyboard component. | Emulator source, not hardware oracle. Useful family checklist, not proof every configuration is accurate/complete. Review MAME license before copying code. |
| [Mattel Model 2609 service manual scan](https://www.ifixit.com/Document/vyUeXhXOo1tXKF3W/Intellivision_Service_Manual.pdf) | Describes shop diagnostics with the MTE-201 test cartridge and a follow-up game check; one service case includes a 90-minute burn-in. | Historical manufacturer manual scan hosted by a third party; completeness and reuse rights need review. Describes service procedure, not ROM redistribution rights or an automated oracle. |
| [Master Component 2609 overview from Blue Sky Rangers](https://history.blueskyrangers.com/hardware/2609.html) | Historical in-house technical material gives CP1610 clock/microcycle and ROM word-storage notes plus STIC display/card descriptions. | Useful first-party-adjacent source hosted by a historical Intellivision team. Triangulate clock and pixel interpretations with GI/Mattel documents and tests; nominal versus visible dimensions need precision. |
| [General Instrument 1977 data catalog](https://www.bitsavers.org/components/gi/_dataBooks/1977_GI_Data_Catalog.pdf) | Primary component catalog likely useful for CP1610 and GI chip docs. | Broad catalog, not by itself a console board configuration or proof of timing. Find exact part sections and cross-check schematics. |
| [Mattel Intellivision owner-manual scan](https://www.videogameconsolelibrary.com/images/Manuals/79_Mattel_Intellivision-Manual.pdf) | Describes two attached hand controllers, 12-key keypad, 16-direction disc, side action buttons and physical overlays. | Third-party hosted manual scan; useful for user-facing input design, not all electrical combinations or software semantics. |
| [Hand-controller technical notes](https://wiki.intellivision.us/index.php/Hand_Controller) | Reports active-low inputs and combinations that software cannot uniquely decode. | Community reference. Verify with schematics, software or hardware; test ambiguous simultaneous inputs. |
| [jzIntv/FujiNet source fork](https://github.com/FujiNetWIFI/jzintv) | Existing emulator code/build is a comparison point for components and tooling; README notes old upstream basis and that copyrighted EXEC/GROM/game ROMs are excluded. | Historical implementation, not independent oracle. GPL-2.0-or-later; do not copy or link without deliberate license review. |

The bounded search found the historical MTE-201 diagnostic but did not establish
a complete, permissively redistributable Intellivision conformance ROM suite.
This is an evidence limit, not proof none exists. Use MTE-201 to discover
diagnostic categories; do not include its ROM/assets until permission and exact
use are documented. Prefer original source-built diagnostics with literal
expected outcomes and independent checks.

## Open questions for GSD

1. Which Master Component profile is the initial baseline? Name model/revision,
   region/timing, clone compatibility, startup/reset and cartridge arrangement.
   Keep Intellivision II and Sears model explicit.
2. What authoritative sources establish CP1610 instructions, microcycle timing,
   interrupt/reset behavior and nonuniform 10-bit ROM storage/mapping? Which
   vectors have clear license and independent expected results?
3. What is the STIC's actual visible geometry/card behavior, borders, colors,
   MOB/sprite limits, collisions, interrupts and CPU access interaction? Reconcile
   nominal dimensions with visible framebuffer and timing.
4. What AY-3-8914/related sound behavior exists in each selected variant:
   register map, channel/noise behavior, clocking, output and sample cadence?
   Separate chip behavior from host PCM conversion.
5. What electrical and decoded input does each controller expose? Cover keys,
   disc, side actions, both players and simultaneous combinations. Distinguish
   hardware signal from host remapping and printed overlays.
6. Which cartridge formats/maps matter first? How are EXEC/GROM/system images
   supplied, if at all? Record source, digest, notices, license and use terms for
   each fixture.
7. Which acceptance path is needed: headless A/V, installed consumer, real
   controller/host or physical console proof? Attach each claim to its route.
8. Does a future LibFuturo adapter represent content, input sampling, execution
   granularity, video/audio and persistence without semantic loss? Prove a real
   consumer before proposing shared-contract changes.

## Testing and user experience

- Model the controller as a multi-signal device, not a joystick plus independent
  buttons. Keep electrical reads separate from host mapping. Add overlay
  metadata only if a user/host need justifies it.
- Record controller port, exact signal set and sample boundary. Test every
  single keypad key, disc direction and action; both players; selected
  simultaneous-key cases; ambiguous inputs only after the electrical model is
  sourced.
- Keep CPU/vector, STIC/frame, sound/PCM, controller, cartridge load and
  playable-progress claims as distinct evidence classes with named oracles and
  failure controls.
- Observe raw color/index output and audio events/samples before host scaling or
  audio-device output. Verify presentation at an installed host boundary.
- Keep a deterministic regression with a clear guest predicate, such as an
  original program reaching an expected register/memory state or a licensed
  homebrew reaching a known progress marker. A title screen or shared-core hash
  is limited evidence.
- Put long loops, broad content databases, fuzzers and performance distributions
  in scheduled/release jobs only when their incremental value justifies cost.

## Dependency, license and release posture

A flat dependency tree and original diagnostics fit owner preference. Add a
library, ROM set or copied subsystem only for concrete benefit and with
maintenance ownership. Treat source license, third-party notices, fixture-use
rights and release-artifact rights separately. Do not infer public visibility or
a license from another project. Target a release at each milestone when exact
tested artifacts, provenance, rights, qualification and authority are ready.

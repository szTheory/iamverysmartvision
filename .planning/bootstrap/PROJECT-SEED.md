# Project idea: iamverysmartvision

## What to build

Start a new emulator project for the Mattel Intellivision, using the
owner-selected working name iamverysmartvision. Build an accurate, reproducible
and useful emulator whose behavior can be tested and integrated into
well-designed hosts. Start from current lessons across the owner's emulator
projects and LibFuturo, and automate useful feedback so development stays fast
without overstating what evidence proves.

This is kickoff input, not a final requirement list, hardware specification,
API/ABI, license, release promise or fixed first milestone. Use GSD to create
the formal project context, evidence-based requirements and a rolling roadmap.
Keep accepted owner direction, recommendations, unknowns and GSD-generated
decisions distinct.

## North Star

- Emulate clearly named Intellivision hardware and content profiles faithfully.
  Tie support claims to exact configuration and evidence rather than an
  unqualified compatibility label.
- Keep machine execution deterministic and observable. Test CPU/device
  behavior, controller inputs, guest progress, raw video, raw audio and host
  integration as separate but connected evidence.
- Aim for high runtime performance without compromising correct behavior.
  Establish representative workloads and correctness baselines before
  optimizing measured bottlenecks.
- Keep machine behavior independent from host clocks, filesystems, physical
  controller discovery, windows, audio/video presentation and persistence
  policy.
- Make building, debugging, testing, packaging and integration repeatable with
  a small dependency surface and minimal manual steps.
- Expand capability through verified vertical slices while preserving the
  broader quality goal. A first profile is delivery sequencing, not a scope
  ceiling.

## Recommended discovery direction

Evaluate a small, explicitly named Master Component profile with an original
diagnostic fixture and an independent headless consumer as an early end-to-end
slice. The 2609 family is a plausible starting point, but is not selected
hardware scope. Keep one playable or homebrew scenario visible early so the
project does not become research-only. Ensure any chosen fixture is authored
or cleared for the intended use, has an explicit expected result and runs
repeatably.

This sequencing suggestion must be checked against source quality, user value
and testability. It does not settle exact revision/region, controller mapping,
boot/firmware handling, peripheral scope, video/audio timing, language, API,
runtime host, test ROM, game or release plan.

## Product and hardware scope to resolve

“Intellivision” covers a family of machines and accessories. Separate baseline
console behavior from optional or later systems, including:

- Original Master Component and clones such as Sears Super Video Arcade.
- Intellivision II and compatibility differences.
- EXEC, GROM, GRAM, cartridge ROM/RAM and unusual cartridge memory maps.
- Original hand controllers: 12-key keypad, directional disc, side action
  buttons, two-player wiring and game-specific printed overlays.
- IntelliVoice, ECS, keyboard components, and pass-through peripherals. MAME's
  driver includes configurations beyond the original console, including an
  unreleased keyboard component; their existence does not imply required
  support.
- Regional video timing, reset/boot behavior, sound generation and differences
  among GI chip revisions or console variants.

Choose a small baseline explicitly. Do not silently promise a clone, add-on,
keyboard or speech device because an existing emulator exposes it. A first
slice is not a permanent limit on future systems.

## Core, API and integration

Keep one canonical machine implementation and a narrow, headless way to drive
it. Define ownership and lifetime for loaded content, controller samples,
outputs, reset/unload, errors/refusal effects, bounded execution and instances.
Keep host files, wall-clock pacing, physical controls and presentation outside
the core.

LibFuturo is a candidate integration, not an accepted dependency or ready
profile. Its current experimental C API includes instance lifecycle, one
content byte span plus a configuration identifier, a 32-bit logical-button
mask sampled at advance start, bounded execution, caller-owned video/audio
buffers and profile-defined persistence. It has no Intellivision profile.
Before adopting or extending it, confirm fit for input combinations/timing,
content plus system images, variable frame/output behavior, audio,
memory-backed saves and the exact core API. A source build or adapter agreement
alone is not hardware accuracy or a qualified installed consumer.

## Development and verification intent

- Start with exact CPU/device tests, a small original fixture, a deterministic
  headless runner and a clean installed consumer. Keep source tests and
  package behavior as separate evidence.
- Use deterministic input at an explicit machine/time boundary and fresh
  isolated state. Check guest semantics as well as execution progress,
  controller effects, raw framebuffer and raw audio samples. Seek independent
  expected results where possible.
- Check positive, boundary, malformed and negative-control cases. Verify that
  important gates fail for their intended named fault. Make skips, missing
  artifacts and stale output visible.
- Treat the two multidimensional hand controllers as first-class machine input,
  not a generic gamepad. Test keypad keys, disc directions and combinations,
  both players, opposite inputs and ambiguous electrical combinations under
  the selected hardware contract. Keep physical device discovery, accessibility
  and remapping in the host layer.
- Build a fixture manifest with source, recipe, identity, license, notices and
  allowed use. Do not assume the historical MTE-201 service diagnostic or any
  EXEC/GROM image can be redistributed.
- Keep one documented local command aligned with fast PR checks. Put only
  distinct recurring failure classes in fast CI. Measure queue/setup/build/test
  time before changing caches, concurrency or matrix breadth; preserve quality
  while reducing demonstrated bottlenecks.
- Benchmark identified deterministic workloads only when correctness output is
  stable. Separate emulator throughput, end-to-end latency and CI wall time.
- At every CI run, inspect measured, low-risk critical-path improvements. At
  milestone close, keep main green, triage PRs, update rolling roadmap/lessons
  and target a release of the exact tested artifact when rights, package
  readiness, qualification and authority permit it.

## Choices for GSD to resolve or keep open

- Exact first console and variant profile; region and TV timing.
- CPU, graphics, audio and memory/timing architecture, including interaction
  boundaries and required raster/audio precision.
- Cartridge loading, configuration identity, unusual memory maps and how
  system images are supplied, if at all.
- Controller electrical representation and logical host mapping; overlays as
  optional metadata versus an emulator feature.
- Whether IntelliVoice, ECS, keyboard, Intellivision II and clone variants are
  first-release, later-roadmap or out of scope.
- Language/build system, public API/ABI, error/ownership/versioning policy,
  debugger/headless runner, save-state and battery-backed persistence.
- Initial original/cleared fixture and independent oracle; first consumer and
  supported runtime/platform.
- Dependency policy, new-code license, repository visibility, release
  artifacts and quality/performance goals.

Prefer reversible, evidence-supported defaults for ordinary choices. Ask the
owner only when a choice materially changes direction, rights, authority, risk,
cost or an irreversible action. Public release, license, third-party content,
external qualification and actual hardware testing remain explicit gates.

## Research input

Read .planning/bootstrap/EMULATOR-DNA.md before finalizing scope. It is dated
research: follow technical sources directly, check revisions and licenses,
and seek stronger evidence whenever implementation depends on a detail. Claims
from another emulator do not establish Intellivision behavior.

# iamverysmartvision bootstrap

Updated 2026-11-09. This is pre-GSD setup for a new Intellivision emulator.
The target directory was empty at review time: no code, Git repository, GSD
config or formal project/requirements/roadmap/state artifacts existed.

## Read before kickoff

1. Root AGENTS.md for owner direction and durable engineering policy.
2. .planning/bootstrap/PROJECT-SEED.md for the GSD idea document.
3. .planning/bootstrap/EMULATOR-DNA.md for transferable emulator lessons,
   Intellivision research leads and open design questions.
4. Current .planning/config.json for workflow defaults.

## Start the new project

In a fresh LLM session whose working directory is this project, run:

    $gsd-new-project --auto @.planning/bootstrap/PROJECT-SEED.md

Use the installed session's exact syntax if it differs. The owner requested
routine GSD work to be autonomous; the config enables auto-advance and the seed
distinguishes reversible defaults from material choices. GSD should create and
validate formal project, requirements, roadmap and state files. Do not start
$gsd-plan-phase 1 or implementation in this kickoff. Return the generated
artifacts and next useful command to the owner.

## GSD defaults

The config preserves the established emulator-project GSD defaults for
research, plan-check, verifier, Nyquist, code review, pattern mapping,
parallelization and security. workflow.auto_advance is enabled.
Routine reversible work can proceed autonomously; automation must not accept a
material scope, rights, external-publication or physical-qualification
decision. Batch unavoidable human judgment at phase end where possible.

## Current boundary

The project name iamverysmartvision and Intellivision domain were chosen by the
owner. The name is a working name, not trademark clearance. This pack does not
choose an implementation language, ABI, license, release visibility,
hardware/expansion coverage, firmware policy or first playable title/test
fixture. It contains no ROMs, system ROMs, saves or binaries. The owner is next:
start the fresh GSD session and run the command above.

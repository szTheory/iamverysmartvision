# Presentation and palette policy

This file is the frozen policy for how this project presents a machine to a
person, and for what it may and may not say about colour. It has six clauses,
numbered and fixed in this order. They are a rule, not evidence: nothing here
mints a claim, carries a source tier, or adds a row to `data/claims.json`.

The clauses are short on purpose. Each one ends in the rule it imposes rather
than in an argument for it, so a later change either keeps the clause or
contradicts it in the open.

1. The core publishes palette **indices** only. The published plane is an index
   frame: width times height bytes, row-major, each byte a colour number 0..15 or
   the blank value, with no RGB triple anywhere under `core/src`. A picture is
   produced by a host, not by the machine model, so the core's output is indices
   and nothing else.

2. Exactly one operator preview palette exists, and every fact about it here is
   measured rather than asserted. `runner/frame_export.c:12` holds the single
   `static const uint8_t fp_palette[...]` definition and `runner/frame_export.h:28`
   declares `ivs_frameppm_palette`; no other palette array exists under `runner/`.
   Its label is the literal string `not STIC colour data` (`runner/frame_export.h:4`,
   printed in the `--frame-ppm` help text at `runner/main.c:113`). It is never
   installed: `data/expected/pkg-install-files.ids` lists 11 paths and no runner
   file. It is never in a golden, a digest, an assert or a claim: a search for the
   palette word over `data/golden/` returns no file, and no `artifact` value in
   `data/claims.json` ends in `.ppm`. Its sixteen hues plus one entry for the blank
   value were chosen by this project only so the indices can be told apart on a
   screen; they are not STIC colour data and they state nothing about any display.

3. The project makes **no colour claim** for any profile - not for the first
   profile and not for a future one. In practice that forbids three things: a
   rendered image offered as evidence of the machine, a golden that carries colour
   literals instead of indices, and a colour table inside the core. The rule is
   absolute for this project, so a colour value has no home here at all until
   clause 6 is satisfied.

4. A host chooses the palette, the scaling and the aspect. The same index frame
   may therefore appear at different sizes and different shapes on two hosts, and
   that difference is not a defect and is not evidence about either host or about
   the machine.

5. `index frame` names the published plane, and the words a person uses for a
   picture or a sound are never evidence of hardware behaviour. `looks like`,
   `sounds like`, `the picture is washed out`, `render`, `picture` and their
   relatives describe a host's presentation and a viewer's judgement, never the
   machine; the evidence is the index plane, the digest chain and the hand-derived
   frames. This line is a policy statement and makes no claim. [nc]

6. A real colour table would be a profile field carrying a **source tier**.
   Naming what a colour value is derived from - a document, a measurement, a
   photographed screen with its conditions - is the only route by which a colour
   value could ever enter this project, and until such a field exists with such a
   tier there is nothing for a reader to cite.

## Enforcement of clause 3

Clause 3 is not aspirational; one refusal already makes it mechanical, and this
policy cites that refusal instead of building a second one.

`tools/check/core-symbols.sh` carries a second defined-symbol rule. Its label
block is at `tools/check/core-symbols.sh:38-41`, and the rule itself - the match
and the failure it raises - is at `tools/check/core-symbols.sh:163-173`: any
symbol the core archive *defines* whose name contains `palette`, `rgb`, `ppm` or
`framepreview` in any letter case is refused with
`FAIL core-symbols-palette-defined: <symbol> in <file>`. What the rule sees is a
defined symbol read from `nm` over the archive, a file-local `static` included,
not a comment and not an identifier in prose; so it constrains code that could
emit colour and says nothing about a document that discusses it. The step that
invokes it is `c-purity` (`tools/check/steps/c-purity.sh`), over the freshly
compiled `-O0` and `-O2` core archives.

The liveness guard for that rule is committed rather than assumed.
`tools/check/controls/core-symbols-palette-defined/` holds three files: `CHECK`
naming the `c-purity` step, `EXPECT` holding the single token
`FAIL core-symbols-palette-defined:`, and an overlay `core/src/cpu/good.c` that
defines one such symbol. The control therefore fails for its intended reason on
demand, and a reader can execute it rather than trust this paragraph.

The ordering here is recorded honestly, because it is the reverse of the usual
one. This rule pre-dates this policy: it was written in plan 06-09 for decisions
D-12 and D-13, before TOOL-04 existed in its present form. This document
therefore freezes the *meaning* of a refusal that already existed; it does not
create the check, and a second mechanism for one property was refused
deliberately (amendment A-02). That is an inversion of GUARDRAILS 2.9's
oracle-first order rather than an application of it, and the proof is an
ancestry in the direction the history actually has:
`git merge-base --is-ancestor <the commit that added the control> <the commit
that added this file>`. A sentence claiming the policy came first would be a
false oracle-first claim, so no such sentence is written here.

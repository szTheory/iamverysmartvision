# Time base (control tree)

This is the known-good time-base document for the control tree.

**Master tick**: one period of the crystal, and the canonical machine-time unit.
_Avoid_: calling it "the clock" [nc].

**CPU cycle**: the unit of every manual cycle count, which is a fixed number of master ticks on the profile.
_Avoid_: calling it "a clock" [nc].

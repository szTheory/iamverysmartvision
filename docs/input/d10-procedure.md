# Owner evidence procedure (D10 and D11)

This page tells the owner what to press, what to record and in which format, for every item of `data/expected/d10-measurements.ids`.
Nothing here has been done: the project records these items as missing evidence and hands them off, and no claim uses the measured-hardware tier (D10, D28).
A run with an owner system image (EXEC and GROM) is D11 and is also not performed; the images are rights-bearing and stay local.
The inventory is checked in both directions by the `discrepancies` step: every open discrepancy whose resolver is a measurement has at least one line, and every line names a real open one.

## Record format

A later ingest step will validate one text file per item, named by the item id (`D10-INP-01.txt`), with these lines in this order:

```
item: <id from the inventory>
date: <YYYY-MM-DD>
console: <model, revision, region>
controllers: <model and condition of each>
method: <one line: tool or cartridge reader used, no file paths>
result: <values, one `name=value` pair per line, hexadecimal for bytes>
note: <anything unusual>
```

Record raw values and never the expected value; the project compares them with its tables afterwards.
Files with owner data stay in the owner's own storage until the owner decides what to publish.

## Reader cartridge

The reading program (a small original diagnostic that polls the two ports and prints the bytes on screen) is not written yet. It is deferred beyond Phase 7: no Phase 7 requirement needs it, and it is revisited when the D10 evidence is collected. The original program of Phase 7, Lamp Tone, is a scenario program and not a reader cartridge.
Until it exists the controller items can only be done with equipment that reads the port pins directly.
Everything below describes what the program or the equipment must record.

## System probes

Items `D10-SYS-01..12` belong to earlier phases (video and bus timing, memory decode, cartridge decode, crystal, reset).
Each names a discrepancy whose entry in `data/discrepancies.json` carries the exact resolver text.

## Controller readings

### controller-bytes

For each of the 31 controls on each port, one control pressed alone, record the full 16-bit value read at the port address and the pins read low.
The control names are those of `data/expected/input-bits.ids`; the expected table is not shown to the person taking the reading.

### port-naming

Press one key on each physical controller connector and record which port address changes.

### port-idle-and-unplugged

Record the full 16-bit read of each port with an idle controller attached and with no controller attached.
The upper eight bits are the point of this item.

### output-read

Set each port to output through the direction bits of the mixer register, write two known values, and read the port back with every switch open and again with one switch closed.

### disc-closures

Press two neighbouring disc positions together where the disc allows it, then two opposite directions, then three controls together (a disc direction with two side buttons); record each value read.

## Sound readings

The sound items need a capture of the analog outputs or of the register write and the resulting level at a known time; the program writes the values named here and the owner records what is heard or captured.

### psg-tone

Write a known tone period, enable tone only on one channel with a fixed amplitude, and record the output frequency at three periods (small, middle, large).

### psg-noise

Enable noise only on one channel at the largest and smallest noise period and record the pulse pattern over at least 2,000 shifts.

### psg-envelope

For each of the sixteen shape values record the envelope against time at one period, and once rewrite the shape register in the middle of a cycle and record whether the cycle restarts.

### psg-dac

For each of the sixteen amplitude values record the relative output level with tone and noise disabled.

### psg-period-zero

Write period 0 to a tone, to noise and to the envelope in turn and record the resulting frequency.

### psg-registers

Write a distinct value to each of the sixteen register addresses with the outputs disabled and read each address back; then write all ones to the amplitude and shape registers and read them back.

### psg-reset

Record the registers and the first thousand samples (or the audible state) just after power-on, and again after a console reset.

### psg-timing

Record the frequency at the PSG chip clock pin or a tone against a video-timing reference, and the crystal frequency.

### psg-address-map

Read the words around the register block and the same offsets one block of 1,024 words away, and record any that answer.

### psg-direction

Set each direction bit of the mixer register in turn and record whether the port still follows a closed switch.

## Owner system image

### owner-exec

Items `D11-*`: run the owner's own system image against the controller and sound items above and record what the real startup program reads; the image is never committed and no result here is published without the owner's decision.
The owner-local route, its three identity checks and what it does not do are on `docs/owner-images.md`.

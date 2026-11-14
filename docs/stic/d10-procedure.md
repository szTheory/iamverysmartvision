# Owner evidence procedure for the STIC (D10 and D11)

This page tells the owner what to run, what to capture and in which format, for the items `D10-VID-01..14` and `D11-EXEC-03` of `data/expected/d10-measurements.ids`.
Nothing here has been done: the project records these items as missing evidence and hands them off, and no claim uses the measured-hardware tier (D10, D24 of Phase 6).
A run with an owner system image is D11 and is also not performed; the images are rights-bearing and stay local.
No Phase 6 scenario uses an owner image.
The general record format and the owner-evidence rules are those of `docs/input/d10-procedure.md`; this page adds the video items.

## Record format

One text file per item, named by the item id (`D10-VID-04.txt`), with the lines `item`, `date`, `console`, `method`, `result` and `note` in the order of `docs/input/d10-procedure.md`.
Record raw values (register values in hexadecimal, cycle counts in decimal, captured pixel colors as the numbers 0..15 of the project table or as a photo the owner keeps) and never the expected value.

## Reader cartridge

The program that exercises the STIC on hardware (a small original cartridge that strobes the display at chosen cycle offsets, draws known cards and prints values) is not written yet. It is deferred beyond Phase 7: no Phase 7 requirement needs it, and it is revisited when the D10 evidence is collected. The original program of Phase 7, Lamp Tone, is a scenario program and not a reader cartridge.
Until it exists these items can only be done with equipment that reads the buses directly.
Everything below describes what the program or the equipment must record.

## Geometry

### stic-geometry

Items `D10-VID-01` and `D10-VID-02`: draw a card whose pattern has a marker in pixel 0, pixel 6 and pixel 7 in the last card column and in the first column, capture one frame, and record whether a picture column exists to the right of pixel 6, which of the eight pixel columns is missing when only seven are displayed, and the width in pixels of the visible area.

## Display state

### stic-display-off

Item `D10-VID-03`: with the display strobe never written, record what the picture output is (a solid color, which color, the border color) for at least two frames, and whether the value changes when the border color register is written.

### stic-power-on

Item `D10-VID-09`: right after power-on and again after a console reset, read the 64 STIC addresses and record the values; record the card mode that a Foreground/Background word and a Color Stack word produce before any mode access, and whether the display is shown.

### stic-strobe

Item `D10-VID-08`: write the display strobe at a sweep of cycle offsets after the vertical-blank interrupt and record, for each offset, whether the frame is displayed; then write it once and record whether the next frames are displayed without another write; also read the strobe address and record whether the read arms the display.

## Frame edge and timing

### stic-frame-edge

Item `D10-VID-04`: record the cycle count from the interrupt to the last picture line and to the first picture line of the next frame, with the interrupt routine counting cycles from its first instruction.

### stic-window-probes

Item `D10-VID-06`: at a sweep of cycle offsets after the interrupt, read and write a register, a graphics RAM word and a graphics ROM word, and record for each access whether it worked; record the offset at which each region stops and starts working, and whether the result changes with the display strobed or not.

### stic-bus-windows

Item `D10-VID-07`: with the display on, record the number of times per frame the bus is requested, the spacing between requests in cycles and the number of cycles each request holds, from a trace of the bus request line or from a counting loop.

### stic-backtab-write

Item `D10-VID-05`: write one BACKTAB word at chosen cycle offsets inside the picture and record, for each offset, which card rows show the old and which the new word in the same frame and in the next.

## Moving objects, collision, delay and border extension

These five items belong to Phase 8 (the STIC completion). Each is the owner hand-off of one pinned default of `docs/video/diag-stic-2-derivation.md` section (i); none is done, no result exists and no claim rests on any of them.

### stic-collision

Item `D10-VID-10`: with the display on and two or more objects set up, record the collision registers in the next interrupt after a frame in which the objects overlap, after a read, after a write of zero and after a write of another value; record whether a read changes the register, and the value read after a write of the bit of the register's own object. Repeat with one object at X coordinate 0, with an object at the plane edge and partly outside the plane, with two objects overlapping outside the plane and with the display not strobed for the frame, and record which bits are set.

### stic-mob-priority

Item `D10-VID-11`: draw one object with the priority bit set over a card with set pixels and over a card with only background pixels, capture the frame and record, per pixel, whether the object or the card shows; repeat with the priority bit clear.

### stic-delay-area

Item `D10-VID-12`: with the border extension bits clear, set the horizontal delay and the vertical delay to each of 1 to 7 and record the colors of the exposed columns and rows, whether an object placed there is shown and the delay value at which the register stops following the written value; also count the bus requests and the usable cycles per frame at vertical delay 0 and above, and, with two overlapping objects and a nonzero delay, record whether the collision bits follow the delayed or the undelayed positions.

### stic-border-extension

Item `D10-VID-13`: with each extension bit set alone and both set, record the number of columns and of rows that show the border color, with delay 0 and with a nonzero delay, and whether an object under a strip is shown.

### stic-half-height

Item `D10-VID-14`: draw one object with the Y size bits clear and another in tall mode with the Y size bits clear, using a card whose rows differ, capture the picture line by line and record which pattern row each television line shows.

## Owner system image

### owner-exec

Item `D11-EXEC-03`: run the owner's own system image through the project STIC with the display on and record whether its title screen is drawn as on the console (cards, colors, the 159 visible columns); the image is never committed and no result here is published without the owner's decision.
The owner-local route, its three identity checks and what it does not do are on `docs/owner-images.md`.

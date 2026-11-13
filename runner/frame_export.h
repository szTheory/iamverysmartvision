/* SPDX-License-Identifier: MIT */
/* Operator-side frame export (plan 06-12, D-17): one published plane (width x height index bytes, row-major, each a color number
 * 0..15 or IVS_PIXEL_BLANK) as the bytes of a binary PPM (P6, maxval 255), for an operator who wants to look at a frame
 * (ivs-run --frame-ppm). The palette is the operator preview palette, not STIC colour data: sixteen hues chosen by this project
 * so the sixteen indices and the blank value can be told apart on screen (one extra entry for the blank value); it was not
 * taken from any other emulator and it states nothing about the colours of any display. The exported file is never evidence: no
 * golden, claim, digest, CI upload or test reads it, and a viewer's judgement is never a test (the evidence is the index
 * plane, the chain and the hand-derived frames). Pure functions: no I/O, no allocation, no floats, no globals. */
#ifndef IVS_RUNNER_FRAME_EXPORT_H
#define IVS_RUNNER_FRAME_EXPORT_H

#include <stddef.h>
#include <stdint.h>

#define IVS_FRAMEPPM_OK 0
#define IVS_FRAMEPPM_E_ARG 1   /* a NULL pointer or a zero or too large width or height */
#define IVS_FRAMEPPM_E_SPACE 2 /* the output capacity is below ivs_frameppm_bound */
#define IVS_FRAMEPPM_E_PIXEL 3 /* a position that is neither a color 0..15 nor the blank value */

/* Entries of the preview palette: indices 0..15, then the blank value. */
#define IVS_FRAMEPPM_PALETTE_ENTRIES 17u

/* The exact byte count of the PPM of a width x height plane (`P6`, the size line, `255`, then three bytes per position), 0 for a
 * zero width or height, SIZE_MAX when it does not fit. */
size_t ivs_frameppm_bound(uint32_t width, uint32_t height);

/* The three bytes (R, G, B) of palette entry i (0..15 a color index, 16 the blank value); 0 and *rgb untouched when i is out of range. */
int ivs_frameppm_palette(uint32_t i, uint8_t rgb[3]);

/* Writes the PPM of plane into out (cap bytes) and the byte count into *written. Returns IVS_FRAMEPPM_OK or an error and writes
 * nothing past the first position it refuses. */
int ivs_frameppm_convert(const uint8_t *plane, uint32_t width, uint32_t height, uint8_t *out, size_t cap, size_t *written);

#endif

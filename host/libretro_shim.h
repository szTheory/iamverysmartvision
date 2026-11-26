/* SPDX-License-Identifier: MIT */
/* The host shim's ABI subset (plan 15-01, TOOL-08, D-48, D-35): the entries a frontend may call on
 * the build-only shared object libivs_retro. This header is PROJECT-AUTHORED and LIBRETRO-SHAPED,
 * NEVER COMPATIBLE. The eight names below are shaped after the well-known frontend API so the shape
 * is recognisable to anyone who has written a frontend; nothing was vendored from that project - no
 * header, no prose, no constant table (D-48; rights row `libretro-abi`, disposition cleared, taken:
 * nothing). Do not read this header as an offer of compatibility with any frontend.
 * Two signatures are project-shaped, and this header says so at each one:
 *   - load_game takes three byte spans, because the core needs three images (EXEC, GROM, cartridge)
 *     and the host opens no file but the ones named on its command line;
 *   - run takes an absolute tick target, because the core's digest of the input log serialises the
 *     tick the run ended at, so a frame-counted stop cannot reproduce a run that ends mid-frame.
 * The object is build-only: it is not installed, not shipped and not a release member (D-35), so this
 * header is a build-tree contract and never an installed one. IVS_API is defined for the shim target
 * alone (host/CMakeLists.txt); the installed headers keep it empty and the package stays static.
 * Every declared function is here so -Wmissing-prototypes passes: the header's second job.
 * Standard headers only; constants are #defines (no enums, bool or bitfields). */
#ifndef IVS_HOST_LIBRETRO_SHIM_H
#define IVS_HOST_LIBRETRO_SHIM_H

#include <stddef.h>
#include <stdint.h>

#include "ivs/ivs.h"

#ifdef __cplusplus
extern "C" {
#endif

/* The version of this SHAPE (not of the core: ivs_api_version() answers that). */
#define IVS_RETRO_SHAPE_VERSION 1u

/* The three images the core needs, as byte spans the caller has already read: EXEC exactly 8192 bytes,
 * GROM exactly 2048 bytes, a cartridge container at most 131256 bytes. Project-shaped: the frontend
 * opens the files, and the core takes bytes and refuses a malformed span with its own typed reasons,
 * so the shim has no second content route and opens nothing. */
typedef struct ivs_retro_images {
  const uint8_t *exec;
  size_t exec_len;
  const uint8_t *grom;
  size_t grom_len;
  const uint8_t *cart;
  size_t cart_len;
} ivs_retro_images;

/* One input change: the full logical state of every controller from at_tick on, indexed by PSG port
 * (0 = R14/$01FE, 1 = R15/$01FF), the core's own logical words. at_tick is absolute machine time in
 * master ticks; the core applies an event at the first CPU-unit boundary at or after it. */
typedef struct ivs_retro_input {
  uint64_t at_tick;
  uint32_t controller[4];
} ivs_retro_input;

/* The host's context and its three callbacks.
 *   video_refresh  one completed frame as XRGB pixels (0x00RRGGBB) at pitch_bytes per row, w positions
 *                  by h rows;
 *   audio_batch    mono signed 16-bit PCM frames at the sample rate retro_get_system_av_info declares;
 *   input_poll     1 and *ev filled with the next change at or after now_ticks, or 0 when the script is
 *                  exhausted. It is the only input channel: the core keeps no queue and has no setter.
 * A host missing any of the three is refused by retro_set_environment, not ignored. */
typedef struct ivs_retro_host {
  void *ctx;
  void (*video_refresh)(void *ctx, const uint32_t *xrgb, unsigned w, unsigned h, size_t pitch_bytes);
  void (*audio_batch)(void *ctx, const int16_t *pcm, size_t frames);
  int (*input_poll)(void *ctx, uint64_t now_ticks, ivs_retro_input *ev);
} ivs_retro_host;

/* What a frontend needs to place a frame: the geometry of a frame slot, its aspect ratio and frame
 * rate, the audio sample rate, and the two tick fields a frontend needs to compute a frame boundary
 * (frame_phase_ticks the phase of the first frame after reset, nominal_frame_ticks the exact spacing of
 * two frame events). The last two are project-shaped: they exist so a frontend can ask for a target
 * tick instead of counting frames. The sample rate is the rate this shim delivers through its PCM
 * converter, not the core's native level rate. */
typedef struct ivs_retro_av_info {
  unsigned base_width;
  unsigned base_height;
  unsigned max_width;
  unsigned max_height;
  double aspect_ratio;
  double fps;
  double sample_rate;
  uint64_t frame_phase_ticks;
  uint64_t nominal_frame_ticks;
} ivs_retro_av_info;

/* The shape's version: IVS_RETRO_SHAPE_VERSION. */
IVS_API unsigned retro_api_version(void);

/* Creates the instance storage (aligned by hand: the core takes no allocator, D-01) and the machine.
 * Call it once, then load, run and unload. A failed creation leaves every later call refusing. */
IVS_API void retro_init(void);

/* Destroys the machine if one is loaded and frees every buffer this object owns. */
IVS_API void retro_deinit(void);

/* Installs the host. Returns 1 when accepted and 0 when refused (a NULL host, or a host missing any of
 * the three callbacks) - a refused host is refused, never silently ignored. */
IVS_API int retro_set_environment(const ivs_retro_host *host);

/* Fills *out from the instance: geometry, aspect ratio, frame rate and sample rate, plus the two tick
 * fields. A zeroed struct when no instance exists. */
IVS_API void retro_get_system_av_info(ivs_retro_av_info *out);

/* Loads the three spans into the core and powers the machine on. Returns 1 on success and 0 on a
 * refusal: the core's typed refusal is the boundary and nothing is loaded twice. A refused load is
 * rolled back - the kinds that call committed are unloaded again (or, when the refusal came after the
 * power-on reset, the instance is rebuilt image-less) - so a refusal leaves the machine exactly as it
 * was, no digest answers a state no load produced, and a retry with the right images is meaningful. */
IVS_API int retro_load_game(const ivs_retro_images *imgs);

/* Runs to the absolute target tick and delivers every frame completed on the way to video_refresh and
 * the samples those frames carried to audio_batch. Returns 1 when at least one frame was delivered and
 * 0 otherwise, a refusal included: a refused or internal-failure run records nothing, never a
 * truncated delivery.
 * The input schedule between calls is bounded (4096 outstanding events, the shim's own cap and not the
 * core's 65536): the applied prefix is reclaimed before every call, so the bound is on outstanding
 * events and a long run cannot lose input, and a poll that still has more to say when the schedule
 * fills is REFUSED - the call returns 0 and delivers nothing - rather than truncated, because a full
 * schedule cannot be told from a truncated one without consuming an event the shim could not store. */
IVS_API int retro_run(uint64_t target_ticks);

/* Tears the machine down. After it ivs_retro_digest refuses, so a frontend that only pretends to
 * unload cannot pass. */
IVS_API void retro_unload_game(void);

/* The harness-only entry: writes the SHA-256 of the canonical state of one IVS_DIGEST_* domain into
 * out32. It is one call to ivs_machine_state_digest and computes nothing itself - a second producer in
 * the host would hide a core change. Returns 1 on success, 0 when no machine is loaded or the domain is
 * refused. */
IVS_API int ivs_retro_digest(uint32_t domain, uint8_t out32[32]);

#ifdef __cplusplus
}
#endif

#endif

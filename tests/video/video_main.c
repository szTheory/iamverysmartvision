/* SPDX-License-Identifier: MIT */
/* Suite video (VID-03, plan 06-09): concatenates the fixed cases of every family declared in video_cases.h into one indexed suite. Ids
 * are unique across the families (the harness rejects a duplicate before running anything). Its named faults are
 * data/expected/faults/video.ids; their sites are in core/src/stic and core/src/machine. The TAP header carries one `# source <path>
 * <sha256>` line per data file the families read (the geometry and chain-framing literals). */
#include <stdio.h>
#include <string.h>

#include "ivs_test.h"
#include "video_cases.h"

#define VIDEO_MAX_CASES 256u
#define VIDEO_MAX_SECTIONS 16u

typedef struct video_section {
  const ivs_test_case *fixed;
  size_t n_fixed;
} video_section;

static video_section video_sections[VIDEO_MAX_SECTIONS];
static size_t video_n_sections;
static size_t video_total;

static void video_add(const ivs_test_case *fixed, size_t n_fixed)
{
  video_section *s = &video_sections[video_n_sections++];
  s->fixed = fixed;
  s->n_fixed = n_fixed;
  video_total += n_fixed;
}

static const ivs_test_case *video_find(size_t i)
{
  size_t k;
  for (k = 0u; k < video_n_sections; k++) {
    const video_section *s = &video_sections[k];
    if (i < s->n_fixed) {
      return &s->fixed[i];
    }
    i -= s->n_fixed;
  }
  return NULL;
}

static const char *video_case_id(const void *data, size_t i)
{
  const ivs_test_case *c = video_find(i);
  (void)data;
  return c != NULL ? c->id : "";
}

static int video_case_run(const void *data, size_t i)
{
  const ivs_test_case *c = video_find(i);
  (void)data;
  return c != NULL ? c->fn() : IVS_TEST_FAIL;
}

/* The `# source` lines: the geometry and chain-framing literals (video_geom.c), the frame and card literals (video_fgbg.c), then the moving-object
 * literals and the profile (video_mob.c). */
static ivs_test_source video_sources[32];
static void video_collect_sources(ivs_test_suite *suite)
{
  size_t n1 = 0u;
  size_t n2 = 0u;
  size_t n3 = 0u;
  const ivs_test_source *a = vl_source_table(&n1);
  const ivs_test_source *b = vf_source_table(&n2);
  const ivs_test_source *c = vm_source_table(&n3);
  size_t i;
  size_t n = 0u;
  for (i = 0u; i < n1 && n < 32u; i++) {
    video_sources[n++] = a[i];
  }
  for (i = 0u; i < n2 && n < 32u; i++) {
    video_sources[n++] = b[i];
  }
  for (i = 0u; i < n3 && n < 32u; i++) {
    video_sources[n++] = c[i];
  }
  suite->sources = video_sources;
  suite->n_sources = n;
}

int main(int argc, char **argv)
{
  ivs_test_suite suite;
  video_add(ivs_video_geom_cases, ivs_video_geom_count);
  video_add(ivs_video_slot_cases, ivs_video_slot_count);
  video_add(ivs_video_buffers_cases, ivs_video_buffers_count);
  video_add(ivs_video_stop_cases, ivs_video_stop_count);
  video_add(ivs_video_slice_cases, ivs_video_slice_count);
  video_add(ivs_video_fgbg_cases, ivs_video_fgbg_count);
  video_add(ivs_video_card_cases, ivs_video_card_count);
  video_add(ivs_video_probe_cases, ivs_video_probe_count);
  video_add(ivs_video_meta_cases, ivs_video_meta_count);
  video_add(ivs_video_judge_cases, ivs_video_judge_count);
  video_add(ivs_video_cstack_cases, ivs_video_cstack_count);
  video_add(ivs_video_mob_cases, ivs_video_mob_count);
  if (video_total > VIDEO_MAX_CASES) {
    fprintf(stderr, "video: more than %u cases; raise VIDEO_MAX_CASES\n", VIDEO_MAX_CASES);
    return 1;
  }
  memset(&suite, 0, sizeof suite);
  suite.name = "video";
  suite.count = video_total;
  suite.id = video_case_id;
  suite.run = video_case_run;
  video_collect_sources(&suite);
  return ivs_test_main(argc, argv, &suite);
}

/* SPDX-License-Identifier: MIT */
/* Suite psg (AUD-01, plan 05-04): concatenates the fixed cases of every family declared in
 * psg_cases.h and the vector cases of the generated tables into one indexed suite. Ids are unique
 * across the families (the harness rejects a duplicate before running anything). Its named faults
 * are data/expected/faults/psg.ids; their sites are in core/src/psg and core/src/machine. The TAP
 * header carries one `# source <path> <sha256>` line per data file (vector files and the
 * register-address literal).
 * Plan 05-06 turns the two-family concatenation into a section table: each section has fixed cases,
 * optionally the vectors of one generated family, and the data files it derives from. */
#include <stdio.h>
#include <string.h>

#include "ivs_test.h"
#include "psg_cases.h"

#define PSG_MAX_CASES 640u
#define PSG_MAX_SECTIONS 24u
#define PSG_MAX_SOURCES 24u

typedef struct psg_section {
  const ivs_test_case *fixed;
  size_t n_fixed;
  size_t (*n_vec)(void);
  const char *(*vec_id)(size_t);
  int (*vec_run)(size_t);
  const ivs_test_source *(*sources)(size_t *n);
} psg_section;

/* Order: family reg (fixed, then its vectors reg and reset), port, tone, mix, amp, then (plan 05-07) noise,
 * env, ampenv, reseng, dac and the profile perturbation cases, then the fixed families of the audio path
 * (sample, slice, chain, buffers, bounds, plan 05-06). */
static psg_section psg_sections[PSG_MAX_SECTIONS];
static size_t psg_n_sections;
static size_t psg_total;

static size_t psg_nv(const psg_section *s)
{
  return s->n_vec != NULL ? s->n_vec() : 0u;
}

static void psg_add(const ivs_test_case *fixed, size_t n_fixed, size_t (*n_vec)(void), const char *(*vec_id)(size_t),
                    int (*vec_run)(size_t), const ivs_test_source *(*sources)(size_t *n))
{
  psg_section *s = &psg_sections[psg_n_sections++];
  s->fixed = fixed;
  s->n_fixed = n_fixed;
  s->n_vec = n_vec;
  s->vec_id = vec_id;
  s->vec_run = vec_run;
  s->sources = sources;
  psg_total += n_fixed + psg_nv(s);
}

static const psg_section *psg_find(size_t *i)
{
  size_t k;
  for (k = 0u; k < psg_n_sections; k++) {
    const psg_section *s = &psg_sections[k];
    size_t n = s->n_fixed + psg_nv(s);
    if (*i < n) {
      return s;
    }
    *i -= n;
  }
  return NULL;
}

static const char *psg_case_id(const void *data, size_t i)
{
  const psg_section *s = psg_find(&i);
  (void)data;
  if (s == NULL) {
    return "";
  }
  return i < s->n_fixed ? s->fixed[i].id : s->vec_id(i - s->n_fixed);
}

static int psg_case_run(const void *data, size_t i)
{
  const psg_section *s = psg_find(&i);
  (void)data;
  if (s == NULL) {
    return IVS_TEST_FAIL;
  }
  return i < s->n_fixed ? s->fixed[i].fn() : s->vec_run(i - s->n_fixed);
}

int main(int argc, char **argv)
{
  ivs_test_suite suite;
  static ivs_test_source sources[PSG_MAX_SOURCES];
  size_t n_src = 0u;
  size_t k;
  /* The reg family's vector list is two files (reg, reset): its id function covers both. */
  psg_add(ivs_psg_reg_cases, ivs_psg_reg_count, ivs_psg_reg_vector_count, ivs_psg_reg_vector_id,
          ivs_psg_reg_vector_run, ivs_psg_reg_sources);
  psg_add(ivs_psg_port_cases, ivs_psg_port_count, ivs_psg_port_vector_count, ivs_psg_port_vector_id,
          ivs_psg_port_vector_run, ivs_psg_port_sources);
  psg_add(ivs_psg_tone_cases, ivs_psg_tone_count, ivs_psg_tone_vector_count, ivs_psg_tone_vector_id,
          ivs_psg_tone_vector_run, ivs_psg_tone_sources);
  psg_add(ivs_psg_mix_cases, ivs_psg_mix_count, ivs_psg_mix_vector_count, ivs_psg_mix_vector_id,
          ivs_psg_mix_vector_run, ivs_psg_mix_sources);
  psg_add(ivs_psg_amp_cases, ivs_psg_amp_count, ivs_psg_amp_vector_count, ivs_psg_amp_vector_id,
          ivs_psg_amp_vector_run, ivs_psg_amp_sources);
  psg_add(ivs_psg_noise_cases, ivs_psg_noise_count, ivs_psg_noise_vector_count, ivs_psg_noise_vector_id,
          ivs_psg_noise_vector_run, ivs_psg_noise_sources);
  psg_add(ivs_psg_env_cases, ivs_psg_env_count, ivs_psg_env_vector_count, ivs_psg_env_vector_id,
          ivs_psg_env_vector_run, ivs_psg_env_sources);
  psg_add(ivs_psg_ampenv_cases, ivs_psg_ampenv_count, ivs_psg_ampenv_vector_count, ivs_psg_ampenv_vector_id,
          ivs_psg_ampenv_vector_run, ivs_psg_ampenv_sources);
  psg_add(ivs_psg_reseng_cases, ivs_psg_reseng_count, ivs_psg_reseng_vector_count, ivs_psg_reseng_vector_id,
          ivs_psg_reseng_vector_run, ivs_psg_reseng_sources);
  psg_add(ivs_psg_dac_cases, ivs_psg_dac_count, ivs_psg_dac_vector_count, ivs_psg_dac_vector_id,
          ivs_psg_dac_vector_run, ivs_psg_dac_sources);
  psg_add(ivs_psg_profile_cases, ivs_psg_profile_count, NULL, NULL, NULL, NULL);
  psg_add(ivs_psg_sample_cases, ivs_psg_sample_count, NULL, NULL, NULL, NULL);
  psg_add(ivs_psg_slice_cases, ivs_psg_slice_count, NULL, NULL, NULL, NULL);
  psg_add(ivs_psg_chain_cases, ivs_psg_chain_count, NULL, NULL, NULL, NULL);
  psg_add(ivs_psg_bounds_cases, ivs_psg_bounds_count, NULL, NULL, NULL, NULL);
  psg_add(ivs_psg_buffers_cases, ivs_psg_buffers_count, NULL, NULL, NULL, NULL);
  psg_add(ivs_psg_log_cases, ivs_psg_log_count, NULL, NULL, NULL, NULL);
  if (psg_total > PSG_MAX_CASES) {
    fprintf(stderr, "psg: more than %u cases; raise PSG_MAX_CASES\n", PSG_MAX_CASES);
    return 1;
  }
  memset(&suite, 0, sizeof suite);
  suite.name = "psg";
  suite.count = psg_total;
  suite.id = psg_case_id;
  suite.run = psg_case_run;
  for (k = 0u; k < psg_n_sections; k++) {
    size_t n = 0u;
    const ivs_test_source *src = psg_sections[k].sources != NULL ? psg_sections[k].sources(&n) : NULL;
    if (n_src + n > PSG_MAX_SOURCES) {
      fprintf(stderr, "psg: more than %u sources\n", PSG_MAX_SOURCES);
      return 1;
    }
    if (n != 0u) {
      memcpy(sources + n_src, src, n * sizeof sources[0]);
      n_src += n;
    }
  }
  suite.sources = sources;
  suite.n_sources = n_src;
  return ivs_test_main(argc, argv, &suite);
}

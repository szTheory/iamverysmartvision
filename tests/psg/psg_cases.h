/* SPDX-License-Identifier: MIT */
/* Suite psg (AUD-01, plan 05-04): the PSG register file, then (plans 05-05..05-08) the ports, tone,
 * mixer, noise, envelope, sample stream and write log. One executable; each family file defines its
 * case table here declared and psg_main.c concatenates them into one suite. Expected values come
 * from the sourced data files only: the vector files data/psg/vectors (one text file per family) and the register-address
 * literal data/psg/<profile>.regmap.literal.txt, turned into C includes by cmake/IvsPsgVectors.cmake
 * (which reads neither the profile nor the core), never from the generated parameter tables or from
 * a core output. */
#ifndef IVS_PSG_CASES_H
#define IVS_PSG_CASES_H

#include <stddef.h>
#include <stdint.h>

#include "ivs_test.h"

/* One vector (field order of the generator's initializers, cmake/IvsPsgVectors.cmake): writes are
 * applied in order through the memory-system bus, then every read is compared with the register
 * store of its address. steps is the run length of the engine families (0 here). */
typedef struct ivs_pv_write {
  uint16_t addr;
  uint16_t value;
  uint32_t step;
} ivs_pv_write;

typedef struct ivs_pv_read {
  uint16_t addr;
  uint16_t value;
} ivs_pv_read;

typedef struct ivs_pv {
  const char *id;
  const char *rule;
  uint32_t source; /* 1: kind source, 0: kind policy */
  const ivs_pv_write *writes;
  uint32_t n_writes;
  const ivs_pv_read *reads;
  uint32_t n_reads;
  uint32_t steps;
  int32_t pin_port;  /* -1: none; 0 or 1: the controller port whose pins are driven low (family port) */
  uint32_t pin_mask; /* the driven-low mask HH of that port */
} ivs_pv;

/* One row of the register-address literal: address, write mask, read mask. */
typedef struct ivs_pv_lit {
  uint16_t addr;
  uint16_t wmask;
  uint16_t rmask;
} ivs_pv_lit;

/* Family reg (plan 05-04, psg_reg.c): the register file, the reset and the bus wrapper. The
 * fixed cases (hand-written ids) and the vector cases (one per vector of regs.txt and reset.txt). */
extern const ivs_test_case ivs_psg_reg_cases[];
extern const size_t ivs_psg_reg_count;
size_t ivs_psg_reg_vector_count(void);
const char *ivs_psg_reg_vector_id(size_t i);
int ivs_psg_reg_vector_run(size_t i);
/* The data files the family derives from, with their SHA-256 (re-checked by tests-inventory). */
const ivs_test_source *ivs_psg_reg_sources(size_t *n);

/* Family port (plan 05-05, psg_port.c): the ports $01FE and $01FF as a bus read returns them. Fixed
 * cases (the judge self-control) and the vector cases of port.txt, evaluated through the memory-system
 * bus with the pins of the vector's pin key applied before the reads. */
extern const ivs_test_case ivs_psg_port_cases[];
extern const size_t ivs_psg_port_count;
size_t ivs_psg_port_vector_count(void);
const char *ivs_psg_port_vector_id(size_t i);
int ivs_psg_port_vector_run(size_t i);
const ivs_test_source *ivs_psg_port_sources(size_t *n);

/* Engine vectors (plan 05-06, cmake/IvsPsgVectors.cmake families tone, mix, amp): the writes run through the
 * module-level PSG write function at their step (stamp step x 16 master ticks), the engine runs steps chip
 * steps, and one expectation of the kind below is judged on the sample stream or the pure function. */
#define IVS_PE_EDGE_SPACING 1u /* level changes of channel chan at exactly multiples of n steps */
#define IVS_PE_TOGGLES 2u      /* n level changes of channel chan in the first window steps */
#define IVS_PE_LEVEL_SEQ 3u    /* the levels of channel chan at steps 1.. equal seq */
#define IVS_PE_GATE 4u         /* ivs_psg_mix_channel(tone b, noise c, r7 a, ch chan) == n */
#define IVS_PE_INDEX 5u        /* ivs_psg_amp_index(amp a, env b) == n */
/* Plan 05-07 (families noise, env, dac, ampenv, reseng): */
#define IVS_PE_SHIFTS 6u  /* n noise register shifts in the first window steps */
#define IVS_PE_PERIOD 7u  /* the noise register returns to its seed after exactly n shifts */
#define IVS_PE_SILENT 8u  /* every level of the channels in the bit mask a (A 1, B 2, C 4) is 0 for all steps */
#define IVS_PE_TABLE 9u   /* pairs tkeys[i] = tvals[i]: key 0..15 a DAC level, 16 the entry count, 17 three channels at level 15 */
/* Channel number 3 of IVS_PE_LEVEL_SEQ is N: the noise output bit after each shift from the seed (no running time). */
#define IVS_PE_CHAN_N 3u

typedef struct ivs_pe {
  const char *id;
  const char *rule;
  uint32_t source;
  const ivs_pv_write *writes;
  uint32_t n_writes;
  uint32_t steps;
  uint32_t kind;
  uint32_t chan;
  uint32_t n;
  uint32_t window;
  uint32_t a;
  uint32_t b;
  uint32_t c;
  const uint8_t *seq;
  uint32_t n_seq;
  uint32_t base_first;   /* 1: the level before step 1 is the level of step 1 (an envelope), 0: it is 0 (a tone) */
  uint32_t stride;       /* IVS_PE_LEVEL_SEQ: 0 one level per step; else seq[j] is the sample at step stride x j (step 1 for j 0) */
  const uint32_t *tkeys; /* IVS_PE_TABLE keys and values */
  const uint32_t *tvals;
  uint32_t n_tab;
} ivs_pe;

/* Judges v (quiet suppresses the diagnostics): 1 when its expectation holds on a fresh PSG. Shared by
 * the families tone (psg_tone.c), mix and amp (psg_mix.c). */
int ivs_pe_eval(const ivs_pe *v, int quiet);

/* Family tone (plan 05-06, psg_tone.c): the 16 vectors of tone.txt and the fixed cases (the guest tracer). */
extern const ivs_test_case ivs_psg_tone_cases[];
extern const size_t ivs_psg_tone_count;
size_t ivs_psg_tone_vector_count(void);
const char *ivs_psg_tone_vector_id(size_t i);
int ivs_psg_tone_vector_run(size_t i);
const ivs_test_source *ivs_psg_tone_sources(size_t *n);

/* Families mix and amp (plan 05-06, psg_mix.c). */
extern const ivs_test_case ivs_psg_mix_cases[];
extern const size_t ivs_psg_mix_count;
size_t ivs_psg_mix_vector_count(void);
const char *ivs_psg_mix_vector_id(size_t i);
int ivs_psg_mix_vector_run(size_t i);
const ivs_test_source *ivs_psg_mix_sources(size_t *n);
extern const ivs_test_case ivs_psg_amp_cases[];
extern const size_t ivs_psg_amp_count;
size_t ivs_psg_amp_vector_count(void);
const char *ivs_psg_amp_vector_id(size_t i);
int ivs_psg_amp_vector_run(size_t i);
const ivs_test_source *ivs_psg_amp_sources(size_t *n);

/* Family noise (plan 05-07, psg_noise.c): the 11 vectors of noise.txt and the fixed cases (the LFSR through the
 * mixer, the reset seed). Family env (psg_env.c): the vectors of env.txt, the fixed amplitude integration and the
 * restart/phase cases. Family ampenv and reseng (psg_env.c): the vectors of ampenv.txt and reset-engine.txt. */
extern const ivs_test_case ivs_psg_noise_cases[];
extern const size_t ivs_psg_noise_count;
size_t ivs_psg_noise_vector_count(void);
const char *ivs_psg_noise_vector_id(size_t i);
int ivs_psg_noise_vector_run(size_t i);
const ivs_test_source *ivs_psg_noise_sources(size_t *n);
extern const ivs_test_case ivs_psg_env_cases[];
extern const size_t ivs_psg_env_count;
size_t ivs_psg_env_vector_count(void);
const char *ivs_psg_env_vector_id(size_t i);
int ivs_psg_env_vector_run(size_t i);
const ivs_test_source *ivs_psg_env_sources(size_t *n);
extern const ivs_test_case ivs_psg_ampenv_cases[];
extern const size_t ivs_psg_ampenv_count;
size_t ivs_psg_ampenv_vector_count(void);
const char *ivs_psg_ampenv_vector_id(size_t i);
int ivs_psg_ampenv_vector_run(size_t i);
const ivs_test_source *ivs_psg_ampenv_sources(size_t *n);
extern const ivs_test_case ivs_psg_reseng_cases[];
extern const size_t ivs_psg_reseng_count;
size_t ivs_psg_reseng_vector_count(void);
const char *ivs_psg_reseng_vector_id(size_t i);
int ivs_psg_reseng_vector_run(size_t i);
const ivs_test_source *ivs_psg_reseng_sources(size_t *n);

/* Family dac (plan 05-07, psg_dac.c): the profile DAC table the core never reads, compared with dac.txt through a
 * test-side generated include (cmake/IvsPsgProfileProbe.cmake). */
extern const ivs_test_case ivs_psg_dac_cases[];
extern const size_t ivs_psg_dac_count;
size_t ivs_psg_dac_vector_count(void);
const char *ivs_psg_dac_vector_id(size_t i);
int ivs_psg_dac_vector_run(size_t i);
const ivs_test_source *ivs_psg_dac_sources(size_t *n);

/* Family profile (plan 05-07, psg_profile.c): perturbation of every tagged rule parameter. Fixed cases only. */
extern const ivs_test_case ivs_psg_profile_cases[];
extern const size_t ivs_psg_profile_count;

/* Family bounds (plan 05-06, psg_bounds.c): U, g and the audio slack derived from the CPU model. */
extern const ivs_test_case ivs_psg_bounds_cases[];
extern const size_t ivs_psg_bounds_count;

/* Family buffers (plan 05-06, psg_buffers.c): the audio buffer rules of the public run call. */
extern const ivs_test_case ivs_psg_buffers_cases[];
extern const size_t ivs_psg_buffers_count;

/* Families sample, slice and chain (plan 05-06, psg_sample.c, psg_slice.c, psg_chain.c): the chip-rate sample
 * stream, its independence of slicing and the AUDIO digest chain. Fixed cases only. */
extern const ivs_test_case ivs_psg_sample_cases[];
extern const size_t ivs_psg_sample_count;
extern const ivs_test_case ivs_psg_slice_cases[];
extern const size_t ivs_psg_slice_count;
extern const ivs_test_case ivs_psg_chain_cases[];
extern const size_t ivs_psg_chain_count;

#endif

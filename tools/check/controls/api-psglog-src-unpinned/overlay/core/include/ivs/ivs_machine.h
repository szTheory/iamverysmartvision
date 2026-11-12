/* SPDX-License-Identifier: MIT */
/* Toy machine header of the known-good control tree (plan 05-02): the two write-log source numbers,
 * pinned by the toy data/expected/psg-log-source.ids, so the generic define pin of the api-effects
 * step has a header for the IVS_PSGLOG_SRC_ class (the run header is overlaid by several controls
 * and stays the one toy header they change). */
#ifndef IVS_MACHINE_H
#define IVS_MACHINE_H

#define IVS_PSGLOG_SRC_CPU_WRITE 1u
#define IVS_PSGLOG_SRC_CPU_DW 2u
#define IVS_PSGLOG_SRC_CPU_EXTRA 3u

#endif

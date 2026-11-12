# SPDX-License-Identifier: MIT
# tools/pcm (D7 decided as D-21, plan 05-10): ivs_refpcm, the pure integer reference PCM converter
# ("reference conversion v1, converter repeatability only"). An internal STATIC library linked ONLY by
# the runner library (the operator-side --audio-wav) and by suite runner; it has NO install rule and no
# core dependency (no PCM, resampling, filtering, DAC-table use or WAV code exists in ivs_core or in any
# installed artifact; core-symbols.sh and the exact install inventory prove it). Like tools/pack, the
# explicit source list is written repo-relative to the build tree (ivs-pcm-sources.txt) so the c-build
# step finds a tracked tools/pcm/*.c that is not compiled. IVS_REFPCM_INCLUDE_DIR is the directory of
# the header, for the fault framework's plain copies of the runner TUs and the runner suite.

set(IVS_PCM_SOURCES
  ivs_refpcm.c
)

add_library(ivs_refpcm STATIC ${IVS_PCM_SOURCES})
set_target_properties(ivs_refpcm PROPERTIES INTERPROCEDURAL_OPTIMIZATION OFF LINKER_LANGUAGE C)
target_include_directories(ivs_refpcm PUBLIC "${CMAKE_CURRENT_SOURCE_DIR}")
target_link_libraries(ivs_refpcm PRIVATE ivs_warnings ivs_san)

set(listed "")
foreach(src IN LISTS IVS_PCM_SOURCES)
  string(APPEND listed "tools/pcm/${src}\n")
endforeach()
file(WRITE "${CMAKE_BINARY_DIR}/ivs-pcm-sources.txt.tmp" "${listed}")
file(COPY_FILE "${CMAKE_BINARY_DIR}/ivs-pcm-sources.txt.tmp" "${CMAKE_BINARY_DIR}/ivs-pcm-sources.txt"
  ONLY_IF_DIFFERENT)

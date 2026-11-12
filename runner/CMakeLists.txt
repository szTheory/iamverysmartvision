# SPDX-License-Identifier: MIT
# The headless runner (D-17, plan 04-03): the library ivs_runner_lib holds every runner TU except
# the thin main.c and hostio.c, from an explicit source list (no file(GLOB)). It links only the
# public ivs_core, never ivs_core_internal, so a runner TU cannot reach core/src.
# IVS_RUNNER_SOURCES (relative to runner/, alphabetical, library TUs only) is read by
# tests/faults.cmake for the named-fault owner scan and plain copies (D-24); file stems must stay
# unique across core/src and runner/ (object and plain-copy names use the stem; faults.cmake checks
# it at configure time). The executable ivs-run (target ivs_run, plan 04-19) adds main.c and
# hostio.c (IVS_RUN_SOURCES, never in the fault framework: no in-process suite reaches them); it is
# built in both presets and never installed (D-24). The repo-relative list of every runner TU
# (both lists) is written to the build tree like ivs-core-sources.txt, for c-build's unlisted-source
# guard.

set(IVS_RUNNER_SOURCES
  exec.c
  outcome.c
  report.c
  scenario.c
)

add_library(ivs_runner_lib STATIC ${IVS_RUNNER_SOURCES})
set_target_properties(ivs_runner_lib PROPERTIES INTERPROCEDURAL_OPTIMIZATION OFF LINKER_LANGUAGE C)
target_include_directories(ivs_runner_lib PUBLIC "${CMAKE_CURRENT_SOURCE_DIR}")
target_link_libraries(ivs_runner_lib PUBLIC ivs_core PRIVATE ivs_warnings ivs_san)

set(IVS_RUN_SOURCES
  hostio.c
  main.c
)
add_executable(ivs_run ${IVS_RUN_SOURCES})
set_target_properties(ivs_run PROPERTIES OUTPUT_NAME ivs-run INTERPROCEDURAL_OPTIMIZATION OFF)
target_link_libraries(ivs_run PRIVATE ivs_runner_lib ivs_core ivs_warnings ivs_san)

set(listed "")
foreach(src IN LISTS IVS_RUNNER_SOURCES IVS_RUN_SOURCES)
  string(APPEND listed "runner/${src}\n")
endforeach()
file(WRITE "${CMAKE_BINARY_DIR}/ivs-runner-sources.txt.tmp" "${listed}")
file(COPY_FILE "${CMAKE_BINARY_DIR}/ivs-runner-sources.txt.tmp" "${CMAKE_BINARY_DIR}/ivs-runner-sources.txt"
  ONLY_IF_DIFFERENT)

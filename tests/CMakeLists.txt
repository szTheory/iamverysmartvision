# SPDX-License-Identifier: MIT
# Test suites (D-17, D-20). One ctest test per suite, registered unconditionally; every
# add_test gets SKIP_RETURN_CODE 77 (exit 77 means every case of the suite was skipped).
# The vector table is generated at build time from the JSON files listed in
# data/expected/cp1610-vector-files.ids into this build directory (D-14).

set(IVS_VEC_FILES_IDS "${PROJECT_SOURCE_DIR}/data/expected/cp1610-vector-files.ids")
set(IVS_VEC_SKIPS_IDS "${PROJECT_SOURCE_DIR}/data/expected/cp1610-skips.ids")
set(IVS_VEC_GENERATOR "${PROJECT_SOURCE_DIR}/cmake/IvsVectorTable.cmake")
set(IVS_VEC_INC "${CMAKE_CURRENT_BINARY_DIR}/ivs_vectors.inc")

file(STRINGS "${IVS_VEC_FILES_IDS}" ivs_vec_lines)
set(ivs_vec_json "")
foreach(line IN LISTS ivs_vec_lines)
  string(STRIP "${line}" line)
  if(line STREQUAL "" OR line MATCHES "^#")
    continue()
  endif()
  list(APPEND ivs_vec_json "${PROJECT_SOURCE_DIR}/data/cp1610/vectors/${line}.json")
endforeach()
set_property(DIRECTORY APPEND PROPERTY CMAKE_CONFIGURE_DEPENDS "${IVS_VEC_FILES_IDS}" "${IVS_VEC_SKIPS_IDS}")

add_custom_command(
  OUTPUT "${IVS_VEC_INC}"
  COMMAND "${CMAKE_COMMAND}" "-DROOT=${PROJECT_SOURCE_DIR}" "-DFILES=${IVS_VEC_FILES_IDS}"
          "-DSKIPS=${IVS_VEC_SKIPS_IDS}" "-DOUT=${IVS_VEC_INC}" -P "${IVS_VEC_GENERATOR}"
  DEPENDS ${ivs_vec_json} "${IVS_VEC_GENERATOR}" "${IVS_VEC_FILES_IDS}" "${IVS_VEC_SKIPS_IDS}"
  COMMENT "Generating the CP-1610 vector table"
  VERBATIM)

add_library(ivs_test_harness STATIC harness/ivs_test.c harness/ivs_testbus.c)
target_include_directories(ivs_test_harness PUBLIC "${CMAKE_CURRENT_SOURCE_DIR}/harness")
target_link_libraries(ivs_test_harness PUBLIC ivs_core PRIVATE ivs_warnings ivs_san)

add_executable(ivs_tests_cpu_vectors cpu/cpu_vectors.c "${IVS_VEC_INC}")
target_include_directories(ivs_tests_cpu_vectors PRIVATE "${CMAKE_CURRENT_BINARY_DIR}")
target_link_libraries(ivs_tests_cpu_vectors PRIVATE ivs_test_harness ivs_core ivs_warnings ivs_san)
add_test(NAME cpu-vectors COMMAND ivs_tests_cpu_vectors)
set_tests_properties(cpu-vectors PROPERTIES SKIP_RETURN_CODE 77)

# Suite cpu-sweep (D-16, plan 02-13): the decode sweep against the Phase 1 family table, whose
# columns cmake/IvsSweepTable.cmake turns into ivs_sweep.inc in this build directory.
set(IVS_SWEEP_FAMILIES "${PROJECT_SOURCE_DIR}/data/cp1610/opcode-families.json")
set(IVS_SWEEP_GENERATOR "${PROJECT_SOURCE_DIR}/cmake/IvsSweepTable.cmake")
set(IVS_SWEEP_INC "${CMAKE_CURRENT_BINARY_DIR}/ivs_sweep.inc")
add_custom_command(
  OUTPUT "${IVS_SWEEP_INC}"
  COMMAND "${CMAKE_COMMAND}" "-DROOT=${PROJECT_SOURCE_DIR}" "-DOUT=${IVS_SWEEP_INC}" -P "${IVS_SWEEP_GENERATOR}"
  DEPENDS "${IVS_SWEEP_FAMILIES}" "${IVS_SWEEP_GENERATOR}"
  COMMENT "Generating the CP-1610 decode sweep table"
  VERBATIM)
add_executable(ivs_tests_cpu_sweep cpu/cpu_sweep.c "${IVS_SWEEP_INC}")
target_include_directories(ivs_tests_cpu_sweep PRIVATE "${CMAKE_CURRENT_BINARY_DIR}")
target_link_libraries(ivs_tests_cpu_sweep PRIVATE ivs_test_harness ivs_core ivs_warnings ivs_san)
add_test(NAME cpu-sweep COMMAND ivs_tests_cpu_sweep)
set_tests_properties(cpu-sweep PROPERTIES SKIP_RETURN_CODE 77)

add_executable(ivs_tests_cpu_api cpu/cpu_api.c)
target_link_libraries(ivs_tests_cpu_api PRIVATE ivs_test_harness ivs_core ivs_warnings ivs_san)
add_test(NAME cpu-api COMMAND ivs_tests_cpu_api)
set_tests_properties(cpu-api PROPERTIES SKIP_RETURN_CODE 77)

# Suite machine (D-15, plans 03-02..03-12): every machine-level case family in one executable.
add_executable(ivs_tests_machine machine/machine_main.c machine/machine_sha256.c)
target_link_libraries(ivs_tests_machine PRIVATE ivs_test_harness ivs_core ivs_warnings ivs_san)
add_test(NAME machine COMMAND ivs_tests_machine)
set_tests_properties(machine PROPERTIES SKIP_RETURN_CODE 77)

# Suite cpu-fuzz (D-12, plan 02-15): the seeded robustness run; dev links the sanitizers.
add_executable(ivs_tests_cpu_fuzz cpu/cpu_fuzz.c)
target_link_libraries(ivs_tests_cpu_fuzz PRIVATE ivs_test_harness ivs_core ivs_warnings ivs_san)
add_test(NAME cpu-fuzz COMMAND ivs_tests_cpu_fuzz)
set_tests_properties(cpu-fuzz PROPERTIES SKIP_RETURN_CODE 77)

# Suite harness: self-test of the harness library alone (compiled in directly, no core).
add_executable(ivs_tests_harness harness/harness_selftest.c harness/ivs_test.c)
target_link_libraries(ivs_tests_harness PRIVATE ivs_warnings ivs_san)
add_test(NAME harness COMMAND ivs_tests_harness)
set_tests_properties(harness PROPERTIES SKIP_RETURN_CODE 77)

# Fault-suite registry (D-37, plan 03-01): each suite named by a `# suite=<name>` header of a
# data/expected/faults/*.ids file is described once here; tests/faults.cmake reads it to build
# that suite's fault executables from its own test sources with plain flags.
# ivs_fault_suite(<suite> SOURCES <test .c> INCLUDE_DIRS <dirs> GENERATED <generated files>)
function(ivs_fault_suite suite)
  cmake_parse_arguments(PARSE_ARGV 1 arg "" "" "SOURCES;INCLUDE_DIRS;GENERATED")
  if(NOT suite MATCHES "^[a-z0-9][a-z0-9-]*$" OR NOT arg_SOURCES OR arg_UNPARSED_ARGUMENTS)
    message(FATAL_ERROR "ivs_fault_suite(${suite}): bad suite name or arguments")
  endif()
  if(suite IN_LIST IVS_FAULT_SUITES)
    message(FATAL_ERROR "ivs_fault_suite(${suite}): registered twice")
  endif()
  set(IVS_FAULT_SUITES ${IVS_FAULT_SUITES} "${suite}" PARENT_SCOPE)
  set(IVS_FAULT_SUITE_${suite}_SOURCES "${arg_SOURCES}" PARENT_SCOPE)
  set(IVS_FAULT_SUITE_${suite}_INCLUDE_DIRS "${arg_INCLUDE_DIRS}" PARENT_SCOPE)
  set(IVS_FAULT_SUITE_${suite}_GENERATED "${arg_GENERATED}" PARENT_SCOPE)
endfunction()
# Generator-path controls (GEN_<NAME> fault ids, no source site): the generator script writes
# <OUTPUT> with one mutation (-DMUTATE=<must-fail id>, -DOUT=<fault dir>/<OUTPUT> are added by
# tests/faults.cmake) and the suite's test sources are recompiled against it.
# ivs_fault_gen(<GEN_ID> SUITE <suite> OUTPUT <generated basename> GENERATOR <script>
#               ARGS <-D arguments> DEPENDS <files>)
function(ivs_fault_gen id)
  cmake_parse_arguments(PARSE_ARGV 1 arg "" "SUITE;OUTPUT;GENERATOR" "ARGS;DEPENDS")
  if(NOT id MATCHES "^GEN_[A-Z0-9_]+$" OR NOT arg_SUITE OR NOT arg_OUTPUT OR NOT arg_GENERATOR
     OR arg_UNPARSED_ARGUMENTS)
    message(FATAL_ERROR "ivs_fault_gen(${id}): bad id or arguments")
  endif()
  if(id IN_LIST IVS_FAULT_GENS)
    message(FATAL_ERROR "ivs_fault_gen(${id}): registered twice")
  endif()
  set(IVS_FAULT_GENS ${IVS_FAULT_GENS} "${id}" PARENT_SCOPE)
  set(IVS_FAULT_GEN_${id}_SUITE "${arg_SUITE}" PARENT_SCOPE)
  set(IVS_FAULT_GEN_${id}_OUTPUT "${arg_OUTPUT}" PARENT_SCOPE)
  set(IVS_FAULT_GEN_${id}_GENERATOR "${arg_GENERATOR}" PARENT_SCOPE)
  set(IVS_FAULT_GEN_${id}_ARGS "${arg_ARGS}" PARENT_SCOPE)
  set(IVS_FAULT_GEN_${id}_DEPENDS "${arg_DEPENDS}" PARENT_SCOPE)
endfunction()
set(IVS_FAULT_SUITES "")
set(IVS_FAULT_GENS "")

ivs_fault_suite(cpu-vectors
  SOURCES cpu/cpu_vectors.c
  INCLUDE_DIRS "${CMAKE_CURRENT_BINARY_DIR}"
  GENERATED "${IVS_VEC_INC}")
ivs_fault_gen(GEN_EXPECT_MUTATION SUITE cpu-vectors OUTPUT ivs_vectors.inc
  GENERATOR "${IVS_VEC_GENERATOR}"
  ARGS "-DROOT=${PROJECT_SOURCE_DIR}" "-DFILES=${IVS_VEC_FILES_IDS}" "-DSKIPS=${IVS_VEC_SKIPS_IDS}"
  DEPENDS ${ivs_vec_json} "${IVS_VEC_GENERATOR}" "${IVS_VEC_FILES_IDS}" "${IVS_VEC_SKIPS_IDS}")

ivs_fault_suite(machine
  SOURCES machine/machine_main.c machine/machine_sha256.c)

# Named-fault control executables (D-18): dev tree only, never registered with ctest.
if(IVS_FAULT_CONTROLS)
  include("${CMAKE_CURRENT_SOURCE_DIR}/faults.cmake")
endif()

# Suite core-symbols (API-05, D-21): the exact shipped ivs_core archive of this preset against
# the reviewed allowlist. The release archive gets the strict check; only an instrumented build
# (IVS_SAN, the dev preset) passes the sanitizer runtime prefixes, set here by the build option
# and never hard-coded in the script.
set(IVS_SYMBOL_PREFIX_ARGS "")
if(IVS_SAN)
  # ASan and UBSan instrumentation imports their runtime entry points (__asan_*, __ubsan_*,
  # __sanitizer_*); Darwin's extra leading underscore is stripped by the script.
  list(APPEND IVS_SYMBOL_PREFIX_ARGS
    --allow-prefix __asan_ --allow-prefix __ubsan_ --allow-prefix __sanitizer_)
endif()
add_test(NAME core-symbols
  COMMAND sh "${CMAKE_SOURCE_DIR}/tests/core-symbols-tap.sh" "$<TARGET_FILE:ivs_core>"
          ${IVS_SYMBOL_PREFIX_ARGS})
set_tests_properties(core-symbols PROPERTIES SKIP_RETURN_CODE 77
  ENVIRONMENT "NM=${CMAKE_NM};AR=${CMAKE_AR}")

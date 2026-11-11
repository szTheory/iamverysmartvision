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
# ivs_core exports only core/include (D-06); tests reach core/src internals (cpu/cp1610.h,
# common/ivs_sha256.h) through ivs_core_internal, linked here and by every test executable.
target_link_libraries(ivs_test_harness PUBLIC ivs_core ivs_core_internal PRIVATE ivs_warnings ivs_san)

add_executable(ivs_tests_cpu_vectors cpu/cpu_vectors.c "${IVS_VEC_INC}")
target_include_directories(ivs_tests_cpu_vectors PRIVATE "${CMAKE_CURRENT_BINARY_DIR}")
target_link_libraries(ivs_tests_cpu_vectors PRIVATE ivs_test_harness ivs_core ivs_core_internal ivs_warnings ivs_san)
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
target_link_libraries(ivs_tests_cpu_sweep PRIVATE ivs_test_harness ivs_core ivs_core_internal ivs_warnings ivs_san)
add_test(NAME cpu-sweep COMMAND ivs_tests_cpu_sweep)
set_tests_properties(cpu-sweep PROPERTIES SKIP_RETURN_CODE 77)

add_executable(ivs_tests_cpu_api cpu/cpu_api.c)
target_link_libraries(ivs_tests_cpu_api PRIVATE ivs_test_harness ivs_core ivs_core_internal ivs_warnings ivs_san)
add_test(NAME cpu-api COMMAND ivs_tests_cpu_api)
set_tests_properties(cpu-api PROPERTIES SKIP_RETURN_CODE 77)

# Suite machine (D-15, plans 03-02..03-12): every machine-level case family in one executable.
# Family sha256 reads ivs_sha_vectors.inc, generated from data/sha256/vectors.txt by
# cmake/IvsShaVectors.cmake, which re-derives every literal digest with string(SHA256) and
# stops the build on a mismatch (D-28).
set(IVS_SHA_VECTORS "${PROJECT_SOURCE_DIR}/data/sha256/vectors.txt")
set(IVS_SHA_GENERATOR "${PROJECT_SOURCE_DIR}/cmake/IvsShaVectors.cmake")
set(IVS_SHA_INC "${CMAKE_CURRENT_BINARY_DIR}/ivs_sha_vectors.inc")
set_property(DIRECTORY APPEND PROPERTY CMAKE_CONFIGURE_DEPENDS "${IVS_SHA_VECTORS}")
add_custom_command(
  OUTPUT "${IVS_SHA_INC}"
  COMMAND "${CMAKE_COMMAND}" "-DROOT=${PROJECT_SOURCE_DIR}" "-DOUT=${IVS_SHA_INC}" -P "${IVS_SHA_GENERATOR}"
  DEPENDS "${IVS_SHA_VECTORS}" "${IVS_SHA_GENERATOR}"
  COMMENT "Generating the SHA-256 vector table (string(SHA256) oracle)"
  VERBATIM)
add_executable(ivs_tests_machine machine/machine_main.c machine/machine_sha256.c "${IVS_SHA_INC}")
target_include_directories(ivs_tests_machine PRIVATE "${CMAKE_CURRENT_BINARY_DIR}")
target_link_libraries(ivs_tests_machine PRIVATE ivs_test_harness ivs_core ivs_core_internal ivs_warnings ivs_san)
add_test(NAME machine COMMAND ivs_tests_machine)
set_tests_properties(machine PROPERTIES SKIP_RETURN_CODE 77)

# Suite memmap (D-16..D-20, plan 03-04): the profile-driven memory system through its bus
# function; expected values come from the suite's own rule table, never from the generated
# descriptors. Its literal-map family (memmap_rows.c, plan 03-06) reads ivs_memmap_probes.inc,
# generated by cmake/IvsMemmapProbes.cmake from the sourced literal maps data/memmap/*.literal.txt
# only (never from data/profiles), and writes memmap-dump.txt for the memmap gate step (H6).
set(IVS_MMPROBE_GENERATOR "${PROJECT_SOURCE_DIR}/cmake/IvsMemmapProbes.cmake")
set(IVS_MMPROBE_INC "${CMAKE_CURRENT_BINARY_DIR}/ivs_memmap_probes.inc")
file(GLOB IVS_MMPROBE_LITERALS CONFIGURE_DEPENDS "${PROJECT_SOURCE_DIR}/data/memmap/*.literal.txt")
add_custom_command(
  OUTPUT "${IVS_MMPROBE_INC}"
  COMMAND "${CMAKE_COMMAND}" "-DROOT=${PROJECT_SOURCE_DIR}" "-DOUT=${IVS_MMPROBE_INC}" -P "${IVS_MMPROBE_GENERATOR}"
  DEPENDS ${IVS_MMPROBE_LITERALS} "${IVS_MMPROBE_GENERATOR}"
  COMMENT "Generating the literal memory-map probe table"
  VERBATIM)
add_executable(ivs_tests_memmap memmap/memmap_main.c memmap/memmap_rows.c "${IVS_MMPROBE_INC}")
target_include_directories(ivs_tests_memmap PRIVATE "${CMAKE_CURRENT_BINARY_DIR}")
target_link_libraries(ivs_tests_memmap PRIVATE ivs_test_harness ivs_core ivs_core_internal ivs_warnings ivs_san)
add_test(NAME memmap COMMAND ivs_tests_memmap)
set_tests_properties(memmap PROPERTIES SKIP_RETURN_CODE 77)

# Suite ivsc (D-29..D-34, plan 03-07): the IVSC v1 layer-1 parser against the project-authored
# fixtures data/cart/<stem>.json listed in data/expected/cart-fixture-files.ids, which
# cmake/IvsCartTable.cmake turns into ivs_cart_table.inc in this build directory. The generator
# also reads the literal maps (span rule) and checks every fixture id against the suite's ids.
set(IVS_CART_FILES_IDS "${PROJECT_SOURCE_DIR}/data/expected/cart-fixture-files.ids")
set(IVS_CART_IDS "${PROJECT_SOURCE_DIR}/data/expected/tests/ivsc.ids")
set(IVS_CART_GENERATOR "${PROJECT_SOURCE_DIR}/cmake/IvsCartTable.cmake")
set(IVS_CART_INC "${CMAKE_CURRENT_BINARY_DIR}/ivs_cart_table.inc")
file(GLOB IVS_CART_JSON CONFIGURE_DEPENDS "${PROJECT_SOURCE_DIR}/data/cart/*")
set_property(DIRECTORY APPEND PROPERTY CMAKE_CONFIGURE_DEPENDS "${IVS_CART_FILES_IDS}")
add_custom_command(
  OUTPUT "${IVS_CART_INC}"
  COMMAND "${CMAKE_COMMAND}" "-DROOT=${PROJECT_SOURCE_DIR}" "-DFILES=${IVS_CART_FILES_IDS}"
          "-DIDS=${IVS_CART_IDS}" "-DOUT=${IVS_CART_INC}" -P "${IVS_CART_GENERATOR}"
  DEPENDS ${IVS_CART_JSON} ${IVS_MMPROBE_LITERALS} "${IVS_CART_GENERATOR}" "${IVS_CART_FILES_IDS}"
          "${IVS_CART_IDS}"
  COMMENT "Generating the IVSC fixture table"
  VERBATIM)
add_executable(ivs_tests_ivsc ivsc/ivsc_main.c ivsc/ivsc_fuzz.c "${IVS_CART_INC}")
target_include_directories(ivs_tests_ivsc PRIVATE "${CMAKE_CURRENT_BINARY_DIR}")
target_link_libraries(ivs_tests_ivsc PRIVATE ivs_test_harness ivs_core ivs_core_internal ivs_warnings ivs_san)
add_test(NAME ivsc COMMAND ivs_tests_ivsc)
set_tests_properties(ivsc PROPERTIES SKIP_RETURN_CODE 77)

# Suite cpu-fuzz (D-12, plan 02-15): the seeded robustness run; dev links the sanitizers.
add_executable(ivs_tests_cpu_fuzz cpu/cpu_fuzz.c)
target_link_libraries(ivs_tests_cpu_fuzz PRIVATE ivs_test_harness ivs_core ivs_core_internal ivs_warnings ivs_san)
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
# CONSUMER core:<core source> (H2 e, plan 03-04): the generated file is included by that core TU
# instead of a test source, so the fault executable compiles that TU as its owner copy with the
# fault directory first on its include path; the other core TUs are plain copies and the suite's
# test sources are compiled as usual. An -DMUTATE= in ARGS replaces the default must-fail id.
# ivs_fault_gen(<GEN_ID> SUITE <suite> OUTPUT <generated basename> GENERATOR <script>
#               ARGS <-D arguments> DEPENDS <files> [CONSUMER core:<core source>])
function(ivs_fault_gen id)
  cmake_parse_arguments(PARSE_ARGV 1 arg "" "SUITE;OUTPUT;GENERATOR;CONSUMER" "ARGS;DEPENDS")
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
  set(IVS_FAULT_GEN_${id}_CONSUMER "${arg_CONSUMER}" PARENT_SCOPE)
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
  SOURCES machine/machine_main.c machine/machine_sha256.c
  INCLUDE_DIRS "${CMAKE_CURRENT_BINARY_DIR}"
  GENERATED "${IVS_SHA_INC}")

ivs_fault_suite(memmap
  SOURCES memmap/memmap_main.c memmap/memmap_rows.c
  INCLUDE_DIRS "${CMAKE_CURRENT_BINARY_DIR}"
  GENERATED "${IVS_MMPROBE_INC}")
# GEN_MAP_MUTATION: the memory-map generator moves region sysram's lo up one 16-word granule
# after validation, and memsys.c is rebuilt against that table (plan 03-04).
get_directory_property(IVS_MEMMAP_GENERATOR DIRECTORY "${PROJECT_SOURCE_DIR}/core"
  DEFINITION IVS_MEMMAP_GENERATOR)
get_directory_property(IVS_PROFILE_FILES DIRECTORY "${PROJECT_SOURCE_DIR}/core"
  DEFINITION IVS_PROFILE_FILES)
ivs_fault_gen(GEN_MAP_MUTATION SUITE memmap OUTPUT ivs_profiles.inc
  GENERATOR "${IVS_MEMMAP_GENERATOR}"
  ARGS "-DROOT=${PROJECT_SOURCE_DIR}" "-DMUTATE=sysram"
  DEPENDS ${IVS_PROFILE_FILES} "${IVS_MEMMAP_GENERATOR}"
  CONSUMER core:src/memmap/memsys.c)

# Suite ivsc: its named faults are data/expected/faults/ivsc.ids (plan 03-08); the fuzz family
# (ivsc/ivsc_fuzz.c) runs under every fault with the fixtures.
ivs_fault_suite(ivsc
  SOURCES ivsc/ivsc_main.c ivsc/ivsc_fuzz.c
  INCLUDE_DIRS "${CMAKE_CURRENT_BINARY_DIR}"
  GENERATED "${IVS_CART_INC}")

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

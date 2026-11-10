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

add_executable(ivs_tests_cpu_api cpu/cpu_api.c)
target_link_libraries(ivs_tests_cpu_api PRIVATE ivs_test_harness ivs_core ivs_warnings ivs_san)
add_test(NAME cpu-api COMMAND ivs_tests_cpu_api)
set_tests_properties(cpu-api PROPERTIES SKIP_RETURN_CODE 77)

# Suite harness: self-test of the harness library alone (compiled in directly, no core).
add_executable(ivs_tests_harness harness/harness_selftest.c harness/ivs_test.c)
target_link_libraries(ivs_tests_harness PRIVATE ivs_warnings ivs_san)
add_test(NAME harness COMMAND ivs_tests_harness)
set_tests_properties(harness PROPERTIES SKIP_RETURN_CODE 77)

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

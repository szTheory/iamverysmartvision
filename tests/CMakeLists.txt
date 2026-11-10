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

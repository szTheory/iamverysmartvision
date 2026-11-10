# SPDX-License-Identifier: MIT
# Named-fault control executables (D-18, A-09); included from tests/CMakeLists.txt only when
# IVS_FAULT_CONTROLS is ON (dev preset). Inventory: data/expected/faults/cpu.ids, one line
# `<FAULT_ID> <must-fail> <allowed-prefixes> <must-pass>` per fault.
# For a site fault <ID>, exactly one core TU must hold `IVS_FAULT_<ID>`; ivs_fault_<ID> is that
# TU compiled with IVS_FAULT_<ID> defined, the other core TUs and the harness from plain
# (non-sanitized) object copies, and the normal vector table. A GEN_ id has no site: it links
# the plain core with a vector table the generator writes with one mutation.
# Fault executables link ivs_warnings only, never ivs_san, so no sanitizer abort can pass for a
# detection. They are EXCLUDE_FROM_ALL (aggregate target ivs_faults) and never registered with
# ctest; the c-faults gate step runs them and faults-verdict judges their results.

set(IVS_FAULT_IDS "${PROJECT_SOURCE_DIR}/data/expected/faults/cpu.ids")
get_directory_property(ivs_fault_core_sources DIRECTORY "${PROJECT_SOURCE_DIR}/core"
  DEFINITION IVS_CORE_SOURCES)
if(NOT ivs_fault_core_sources)
  message(FATAL_ERROR "faults: IVS_CORE_SOURCES is empty")
endif()
set_property(DIRECTORY APPEND PROPERTY CMAKE_CONFIGURE_DEPENDS "${IVS_FAULT_IDS}")

add_custom_target(ivs_faults)

# Plain object copies of every core TU and of the harness library.
foreach(src IN LISTS ivs_fault_core_sources)
  get_filename_component(stem "${src}" NAME_WE)
  set_property(DIRECTORY APPEND PROPERTY CMAKE_CONFIGURE_DEPENDS "${PROJECT_SOURCE_DIR}/core/${src}")
  add_library(ivs_fault_plain_${stem} OBJECT EXCLUDE_FROM_ALL "${PROJECT_SOURCE_DIR}/core/${src}")
  target_include_directories(ivs_fault_plain_${stem} PRIVATE "${PROJECT_SOURCE_DIR}/core/src")
  target_link_libraries(ivs_fault_plain_${stem} PRIVATE ivs_warnings)
  set_target_properties(ivs_fault_plain_${stem} PROPERTIES INTERPROCEDURAL_OPTIMIZATION OFF)
endforeach()
add_library(ivs_fault_plain_harness OBJECT EXCLUDE_FROM_ALL harness/ivs_test.c harness/ivs_testbus.c)
target_include_directories(ivs_fault_plain_harness PRIVATE
  "${CMAKE_CURRENT_SOURCE_DIR}/harness" "${PROJECT_SOURCE_DIR}/core/src")
target_link_libraries(ivs_fault_plain_harness PRIVATE ivs_warnings)

# ivs_fault_exe(<ID> <vector-inc-dir> <vector-inc> <owner-src or empty>)
function(ivs_fault_exe id inc_dir inc owner)
  set(objs "$<TARGET_OBJECTS:ivs_fault_plain_harness>")
  foreach(src IN LISTS ivs_fault_core_sources)
    get_filename_component(stem "${src}" NAME_WE)
    if(src STREQUAL owner)
      add_library(ivs_fault_obj_${id} OBJECT EXCLUDE_FROM_ALL "${PROJECT_SOURCE_DIR}/core/${src}")
      target_include_directories(ivs_fault_obj_${id} PRIVATE "${PROJECT_SOURCE_DIR}/core/src")
      target_compile_definitions(ivs_fault_obj_${id} PRIVATE "IVS_FAULT_${id}")
      target_link_libraries(ivs_fault_obj_${id} PRIVATE ivs_warnings)
      set_target_properties(ivs_fault_obj_${id} PROPERTIES INTERPROCEDURAL_OPTIMIZATION OFF)
      list(APPEND objs "$<TARGET_OBJECTS:ivs_fault_obj_${id}>")
    else()
      list(APPEND objs "$<TARGET_OBJECTS:ivs_fault_plain_${stem}>")
    endif()
  endforeach()
  add_executable(ivs_fault_${id} EXCLUDE_FROM_ALL cpu/cpu_vectors.c "${inc}" ${objs})
  target_include_directories(ivs_fault_${id} PRIVATE "${inc_dir}"
    "${CMAKE_CURRENT_SOURCE_DIR}/harness" "${PROJECT_SOURCE_DIR}/core/src")
  target_link_libraries(ivs_fault_${id} PRIVATE ivs_warnings)
  set_target_properties(ivs_fault_${id} PROPERTIES INTERPROCEDURAL_OPTIMIZATION OFF)
  add_dependencies(ivs_faults ivs_fault_${id})
endfunction()

file(STRINGS "${IVS_FAULT_IDS}" ivs_fault_lines)
foreach(line IN LISTS ivs_fault_lines)
  string(STRIP "${line}" line)
  if(line STREQUAL "" OR line MATCHES "^#")
    continue()
  endif()
  if(NOT line MATCHES "^([A-Z0-9_]+)[ \t]+([^ \t]+)[ \t]+([^ \t]+)[ \t]+([^ \t]+)$")
    message(FATAL_ERROR "faults: bad line in data/expected/faults/cpu.ids: '${line}'")
  endif()
  set(id "${CMAKE_MATCH_1}")
  set(must_fail "${CMAKE_MATCH_2}")
  if(id MATCHES "^GEN_")
    if(NOT id STREQUAL "GEN_EXPECT_MUTATION")
      message(FATAL_ERROR "fault ${id}: no generator control of that name")
    endif()
    set(dir "${CMAKE_CURRENT_BINARY_DIR}/fault-${id}")
    add_custom_command(
      OUTPUT "${dir}/ivs_vectors.inc"
      COMMAND "${CMAKE_COMMAND}" "-DROOT=${PROJECT_SOURCE_DIR}" "-DFILES=${IVS_VEC_FILES_IDS}"
              "-DSKIPS=${IVS_VEC_SKIPS_IDS}" "-DOUT=${dir}/ivs_vectors.inc" "-DMUTATE=${must_fail}"
              -P "${IVS_VEC_GENERATOR}"
      DEPENDS ${ivs_vec_json} "${IVS_VEC_GENERATOR}" "${IVS_VEC_FILES_IDS}" "${IVS_VEC_SKIPS_IDS}"
      COMMENT "Generating the mutated vector table for fault ${id}"
      VERBATIM)
    ivs_fault_exe(${id} "${dir}" "${dir}/ivs_vectors.inc" "")
    continue()
  endif()
  set(owner "")
  foreach(src IN LISTS ivs_fault_core_sources)
    file(READ "${PROJECT_SOURCE_DIR}/core/${src}" text)
    if(text MATCHES "IVS_FAULT_${id}[^A-Za-z0-9_]")
      list(APPEND owner "${src}")
    endif()
  endforeach()
  list(LENGTH owner n_owner)
  if(NOT n_owner EQUAL 1)
    message(FATAL_ERROR "fault ${id}: IVS_FAULT_${id} must appear in exactly one core TU (found ${n_owner}: ${owner})")
  endif()
  ivs_fault_exe(${id} "${CMAKE_CURRENT_BINARY_DIR}" "${IVS_VEC_INC}" "${owner}")
endforeach()

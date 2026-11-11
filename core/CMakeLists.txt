# SPDX-License-Identifier: MIT
# The machine core: an explicit source list (no file(GLOB)), one OBJECT library per source,
# and the STATIC library ivs_core built from those objects (D-19). The repo-relative source
# list is written to the build tree so the c-build step can find a tracked core source that
# is not compiled (token c-build-unlisted-source).
# Include split (D-06, plan 03-03): every core object sees core/include (public headers, ivs/*.h)
# and core/src (internals) PRIVATE; ivs_core exports only core/include, so a consumer that links
# it sees only ivs/*.h. Tests reach core/src internals through the interface target
# ivs_core_internal.

set(IVS_CORE_SOURCES
  src/common/ivs_sha256.c
  src/content/ivsc.c
  src/cpu/cp1610.c
  src/cpu/cp1610_decode.c
  src/memmap/memsys.c
)

# Generated core include (D-16, H2 a, plan 03-04): the memory map, device records and profile
# registry, written from every data/profiles/*.json by cmake/IvsMemmapTable.cmake into
# gen/ivs_profiles.inc in this build directory. memsys.c is its only includer and is listed in
# data/expected/core-generated.ids with its generator, so c-purity regenerates and checks it.
# tests/faults.cmake reads IVS_CORE_GEN_DIR and IVS_CORE_GEN_TARGET to compile the plain and
# owner copies of every core TU against the same directory.
set(IVS_CORE_GEN_DIR "${CMAKE_CURRENT_BINARY_DIR}/gen")
set(IVS_PROFILES_INC "${IVS_CORE_GEN_DIR}/ivs_profiles.inc")
set(IVS_MEMMAP_GENERATOR "${PROJECT_SOURCE_DIR}/cmake/IvsMemmapTable.cmake")
file(GLOB IVS_PROFILE_FILES CONFIGURE_DEPENDS "${PROJECT_SOURCE_DIR}/data/profiles/*.json")
list(SORT IVS_PROFILE_FILES)
add_custom_command(
  OUTPUT "${IVS_PROFILES_INC}"
  COMMAND "${CMAKE_COMMAND}" -E make_directory "${IVS_CORE_GEN_DIR}"
  COMMAND "${CMAKE_COMMAND}" "-DROOT=${PROJECT_SOURCE_DIR}" "-DOUT=${IVS_PROFILES_INC}"
          -P "${IVS_MEMMAP_GENERATOR}"
  DEPENDS ${IVS_PROFILE_FILES} "${IVS_MEMMAP_GENERATOR}"
  COMMENT "Generating the memory map and profile registry"
  VERBATIM)
add_custom_target(ivs_core_gen DEPENDS "${IVS_PROFILES_INC}")
set(IVS_CORE_GEN_TARGET ivs_core_gen)

set(ivs_core_objects "")
foreach(src IN LISTS IVS_CORE_SOURCES)
  get_filename_component(stem "${src}" NAME_WE)
  set(obj "ivs_core_obj_${stem}")
  add_library(${obj} OBJECT "${src}")
  target_include_directories(${obj} PRIVATE
    "${CMAKE_CURRENT_SOURCE_DIR}/include" "${CMAKE_CURRENT_SOURCE_DIR}/src")
  target_include_directories(${obj} PRIVATE "${IVS_CORE_GEN_DIR}")
  add_dependencies(${obj} ivs_core_gen)
  target_link_libraries(${obj} PRIVATE ivs_warnings ivs_san)
  set_target_properties(${obj} PROPERTIES INTERPROCEDURAL_OPTIMIZATION OFF)
  list(APPEND ivs_core_objects "$<TARGET_OBJECTS:${obj}>")
endforeach()

add_library(ivs_core STATIC ${ivs_core_objects})
set_target_properties(ivs_core PROPERTIES INTERPROCEDURAL_OPTIMIZATION OFF LINKER_LANGUAGE C)
target_include_directories(ivs_core PUBLIC "${CMAKE_CURRENT_SOURCE_DIR}/include")

add_library(ivs_core_internal INTERFACE)
target_include_directories(ivs_core_internal INTERFACE "${CMAKE_CURRENT_SOURCE_DIR}/src")

set(listed "")
foreach(src IN LISTS IVS_CORE_SOURCES)
  string(APPEND listed "core/${src}\n")
endforeach()
file(WRITE "${CMAKE_BINARY_DIR}/ivs-core-sources.txt.tmp" "${listed}")
file(COPY_FILE "${CMAKE_BINARY_DIR}/ivs-core-sources.txt.tmp" "${CMAKE_BINARY_DIR}/ivs-core-sources.txt"
  ONLY_IF_DIFFERENT)

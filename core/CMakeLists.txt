# SPDX-License-Identifier: MIT
# The machine core: an explicit source list (no file(GLOB)), one OBJECT library per source,
# and the STATIC library ivs_core built from those objects (D-19). The repo-relative source
# list is written to the build tree so the c-build step can find a tracked core source that
# is not compiled (token c-build-unlisted-source).

set(IVS_CORE_SOURCES
  src/cpu/cp1610.c
  src/cpu/cp1610_decode.c
)

set(ivs_core_objects "")
foreach(src IN LISTS IVS_CORE_SOURCES)
  get_filename_component(stem "${src}" NAME_WE)
  set(obj "ivs_core_obj_${stem}")
  add_library(${obj} OBJECT "${src}")
  target_include_directories(${obj} PRIVATE "${CMAKE_CURRENT_SOURCE_DIR}/src")
  target_link_libraries(${obj} PRIVATE ivs_warnings ivs_san)
  set_target_properties(${obj} PROPERTIES INTERPROCEDURAL_OPTIMIZATION OFF)
  list(APPEND ivs_core_objects "$<TARGET_OBJECTS:${obj}>")
endforeach()

add_library(ivs_core STATIC ${ivs_core_objects})
set_target_properties(ivs_core PROPERTIES INTERPROCEDURAL_OPTIMIZATION OFF LINKER_LANGUAGE C)
target_include_directories(ivs_core PUBLIC "${CMAKE_CURRENT_SOURCE_DIR}/src")

set(listed "")
foreach(src IN LISTS IVS_CORE_SOURCES)
  string(APPEND listed "core/${src}\n")
endforeach()
file(WRITE "${CMAKE_BINARY_DIR}/ivs-core-sources.txt.tmp" "${listed}")
file(COPY_FILE "${CMAKE_BINARY_DIR}/ivs-core-sources.txt.tmp" "${CMAKE_BINARY_DIR}/ivs-core-sources.txt"
  ONLY_IF_DIFFERENT)

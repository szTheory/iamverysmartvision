# SPDX-License-Identifier: MIT
# tools/flatwrap (Phase 14, plan 14-01, D-47, TOOL-07): the target ivs_flatwrap and the executable ivs_flatwrap
# (the underscore convention ivs_pack, ivs_asm and ivs_bench follow; the source list below is the hyphenated
# ivs-flatwrap-sources.txt), the wrap tool.
# It is a development tool: no install rule, never shipped, built in both presets like ivs_pack. It reads one
# flat cartridge image, a base address and a word order, all stated by the operator, and writes one IVSC v1
# container; it detects nothing and parses no other container. It links ivs_core for the SHA-256 and
# ivs_core_internal for the container layout (core/src/content/ivsc.h), the same two targets tools/pack links,
# so the layout has one definition. IVS_FLATWRAP_TREE_ROOT is the configure-time work-tree root the output rule
# compares against, resolved physically at run time. IVS_FLATWRAP_SOURCES (relative to tools/flatwrap/) is written
# repo-relative to the build tree (ivs-flatwrap-sources.txt) like the lists of the other tools, so the c-build
# step finds a tracked tools/flatwrap/*.c that is not compiled.

set(IVS_FLATWRAP_SOURCES
  flatwrap.c
)

add_executable(ivs_flatwrap ${IVS_FLATWRAP_SOURCES})
set_target_properties(ivs_flatwrap PROPERTIES INTERPROCEDURAL_OPTIMIZATION OFF LINKER_LANGUAGE C)
target_link_libraries(ivs_flatwrap PRIVATE ivs_core ivs_core_internal ivs_warnings ivs_san)
target_compile_definitions(ivs_flatwrap PRIVATE "IVS_FLATWRAP_TREE_ROOT=\"${PROJECT_SOURCE_DIR}\"")

set(listed "")
foreach(src IN LISTS IVS_FLATWRAP_SOURCES)
  string(APPEND listed "tools/flatwrap/${src}\n")
endforeach()
file(WRITE "${CMAKE_BINARY_DIR}/ivs-flatwrap-sources.txt.tmp" "${listed}")
file(COPY_FILE "${CMAKE_BINARY_DIR}/ivs-flatwrap-sources.txt.tmp" "${CMAKE_BINARY_DIR}/ivs-flatwrap-sources.txt"
  ONLY_IF_DIFFERENT)

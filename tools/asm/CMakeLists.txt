# SPDX-License-Identifier: MIT
# tools/asm (D4 re-evaluated as D-16, D-17, plan 07-03): ivs_asm, the project mini-assembler. It is a
# development tool, build-only, never installed or shipped (D-17): it has no install rule, no
# dependency and links nothing from ivs_core (the core knows no assembler). It reads one source file
# and writes one listing in the grammar of the committed listings. Like tools/pack and tools/pcm, the
# explicit source list is written repo-relative to the build tree (ivs-asm-sources.txt) so the c-build
# step finds a tracked tools/asm/*.c that is not compiled.

set(IVS_ASM_SOURCES
  asm.c
)

add_executable(ivs_asm ${IVS_ASM_SOURCES})
set_target_properties(ivs_asm PROPERTIES INTERPROCEDURAL_OPTIMIZATION OFF LINKER_LANGUAGE C)
target_link_libraries(ivs_asm PRIVATE ivs_warnings ivs_san)

set(listed "")
foreach(src IN LISTS IVS_ASM_SOURCES)
  string(APPEND listed "tools/asm/${src}\n")
endforeach()
file(WRITE "${CMAKE_BINARY_DIR}/ivs-asm-sources.txt.tmp" "${listed}")
file(COPY_FILE "${CMAKE_BINARY_DIR}/ivs-asm-sources.txt.tmp" "${CMAKE_BINARY_DIR}/ivs-asm-sources.txt"
  ONLY_IF_DIFFERENT)

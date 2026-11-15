# SPDX-License-Identifier: MIT
# tools/bench (D-21 to D-26, plan 08-18): ivs_bench (executable ivs-bench), the benchmark tool. It is a development tool: no install
# rule, never shipped, built in both presets like ivs_pack. It calls ivs_runner_execute in process (it links ivs_runner_lib and ivs_core)
# and holds the only clock reads of the repository for a measurement, in bench_host.c; bench.c is ISO C. No timing exists in the core,
# in runner/, in ivs-run or in any flag of ivs-run, and the gate runs the tool only with --verify-only (tests/bench-tap.sh), which reads
# no clock. IVS_BENCH_SOURCES (relative to tools/bench/) is written repo-relative to the build tree like the lists of the other tools,
# so the c-build step finds a tracked tools/bench/*.c that is not compiled.
# Provenance of a number (the build_id, optimized and sanitizers columns of its row) comes from here: the compiler id and version, the
# build type and a digest of the compile flags of that build type; optimized is 1 for a build type that optimizes; sanitizers is 1 when
# the preset builds with ASan and UBSan (IVS_SAN).

set(IVS_BENCH_SOURCES
  bench.c
  bench_host.c
)

add_executable(ivs_bench ${IVS_BENCH_SOURCES})
set_target_properties(ivs_bench PROPERTIES OUTPUT_NAME ivs-bench INTERPROCEDURAL_OPTIMIZATION OFF LINKER_LANGUAGE C)
target_link_libraries(ivs_bench PRIVATE ivs_runner_lib ivs_core ivs_warnings ivs_san)

if(CMAKE_BUILD_TYPE)
  set(IVS_BENCH_BUILD_TYPE "${CMAKE_BUILD_TYPE}")
else()
  set(IVS_BENCH_BUILD_TYPE "none")
endif()
string(TOUPPER "${IVS_BENCH_BUILD_TYPE}" IVS_BENCH_BUILD_TYPE_UPPER)
string(SHA256 IVS_BENCH_FLAGS_DIGEST "${CMAKE_C_FLAGS} ${CMAKE_C_FLAGS_${IVS_BENCH_BUILD_TYPE_UPPER}}")
string(SUBSTRING "${IVS_BENCH_FLAGS_DIGEST}" 0 12 IVS_BENCH_FLAGS_DIGEST)
set(IVS_BENCH_BUILD_ID "${CMAKE_C_COMPILER_ID}-${CMAKE_C_COMPILER_VERSION}-${IVS_BENCH_BUILD_TYPE}-${IVS_BENCH_FLAGS_DIGEST}")
if(IVS_BENCH_BUILD_TYPE_UPPER MATCHES "^(RELEASE|RELWITHDEBINFO|MINSIZEREL)$")
  set(IVS_BENCH_OPTIMIZED 1)
else()
  set(IVS_BENCH_OPTIMIZED 0)
endif()
if(IVS_SAN)
  set(IVS_BENCH_SANITIZERS 1)
else()
  set(IVS_BENCH_SANITIZERS 0)
endif()
target_compile_definitions(ivs_bench PRIVATE
  "IVS_BENCH_BUILD_ID=\"${IVS_BENCH_BUILD_ID}\""
  "IVS_BENCH_OPTIMIZED=${IVS_BENCH_OPTIMIZED}"
  "IVS_BENCH_SANITIZERS=${IVS_BENCH_SANITIZERS}")

set(listed "")
foreach(src IN LISTS IVS_BENCH_SOURCES)
  string(APPEND listed "tools/bench/${src}\n")
endforeach()
file(WRITE "${CMAKE_BINARY_DIR}/ivs-bench-sources.txt.tmp" "${listed}")
file(COPY_FILE "${CMAKE_BINARY_DIR}/ivs-bench-sources.txt.tmp" "${CMAKE_BINARY_DIR}/ivs-bench-sources.txt"
  ONLY_IF_DIFFERENT)

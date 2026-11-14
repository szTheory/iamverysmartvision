# SPDX-License-Identifier: MIT
# Toy project of the control tree (plan 05-10): the c-build step configures and builds the dev and release
# presets and compares the tracked C sources with the lists the build writes. This project writes the four
# lists (ivs-core-sources.txt, ivs-runner-sources.txt, ivs-pack-sources.txt, ivs-pcm-sources.txt,
# ivs-asm-sources.txt, plan 07-03) for the toy sources and has the target ivs_faults the dev tree builds; it compiles nothing.
cmake_minimum_required(VERSION 3.25)
project(toy LANGUAGES C)
file(WRITE "${CMAKE_BINARY_DIR}/ivs-core-sources.txt" "core/src/cpu/good.c\n")
file(WRITE "${CMAKE_BINARY_DIR}/ivs-runner-sources.txt" "runner/good.c\nrunner/hostio.c\n")
file(WRITE "${CMAKE_BINARY_DIR}/ivs-pack-sources.txt" "")
file(WRITE "${CMAKE_BINARY_DIR}/ivs-pcm-sources.txt" "tools/pcm/good.c\n")
file(WRITE "${CMAKE_BINARY_DIR}/ivs-asm-sources.txt" "tools/asm/good.c\n")
add_custom_target(ivs_faults)

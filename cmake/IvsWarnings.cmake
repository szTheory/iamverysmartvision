# SPDX-License-Identifier: MIT
# Interface targets carrying compile flags per target, never through global flags (A-09):
#   ivs_warnings  the D-19 warning list, -ffp-contract=off and hidden visibility;
#                 -Werror only when IVS_WERROR is ON.
#   ivs_san       ASan + UBSan (compile and link) only when IVS_SAN is ON, so a target that
#                 does not link it (a fault-control copy, later) stays plain.

add_library(ivs_warnings INTERFACE)
if(CMAKE_C_COMPILER_ID MATCHES "Clang|GNU")
  target_compile_options(ivs_warnings INTERFACE
    -Wall -Wextra -Wpedantic -Wshadow -Wconversion -Wsign-conversion
    -Wstrict-prototypes -Wmissing-prototypes -Wvla -Wundef -Wcast-qual
    -Wimplicit-fallthrough -Wdouble-promotion -Werror=date-time
    -ffp-contract=off -fvisibility=hidden)
  if(CMAKE_C_COMPILER_ID MATCHES "Clang")
    target_compile_options(ivs_warnings INTERFACE -Wmissing-variable-declarations)
  endif()
  if(CMAKE_C_COMPILER_ID STREQUAL "GNU")
    target_compile_options(ivs_warnings INTERFACE -Wlogical-op -Wduplicated-cond)
  endif()
  if(IVS_WERROR)
    target_compile_options(ivs_warnings INTERFACE -Werror)
  endif()
endif()

add_library(ivs_san INTERFACE)
if(IVS_SAN)
  target_compile_options(ivs_san INTERFACE
    -fsanitize=address,undefined -fno-sanitize-recover=all -fno-omit-frame-pointer)
  target_link_options(ivs_san INTERFACE -fsanitize=address,undefined -fno-sanitize-recover=all)
endif()

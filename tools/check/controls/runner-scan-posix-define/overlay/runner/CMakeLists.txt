# SPDX-License-Identifier: MIT
# runner-scan negative control (plan 04-04): a compile definition would bypass hostio.c.
target_compile_definitions(ivs_runner_lib PRIVATE _POSIX_C_SOURCE=200809L)

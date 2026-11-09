#!/bin/sh
# SPDX-License-Identifier: MIT
# Step profile: the intv-2609-ntsc-family-p1 record (kind profile).
LC_ALL=C; export LC_ALL
: "${ROOT:?ROOT is not set}"
: "${CHECK_DIR:?CHECK_DIR is not set}"
cmake -DROOT="$ROOT" -DKIND=profile -P "$CHECK_DIR/records.cmake" 2>&1

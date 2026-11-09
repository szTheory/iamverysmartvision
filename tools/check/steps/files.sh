#!/bin/sh
# SPDX-License-Identifier: MIT
# Step files: tracked-file rights check (refs/, .env, unlisted binaries, unlisted fixture files).
LC_ALL=C; export LC_ALL
: "${ROOT:?ROOT is not set}"
: "${CHECK_DIR:?CHECK_DIR is not set}"
: "${FILELIST:?FILELIST is not set}"
cmake -DROOT="$ROOT" -DFILELIST="$FILELIST" -P "$CHECK_DIR/files.cmake" 2>&1

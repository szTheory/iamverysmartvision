#!/bin/sh
# SPDX-License-Identifier: MIT
# Step integrity: path charset, empty files, CRLF, BOM, ASCII-only records, SPDX lines.
LC_ALL=C; export LC_ALL
: "${ROOT:?ROOT is not set}"
: "${CHECK_DIR:?CHECK_DIR is not set}"
: "${FILELIST:?FILELIST is not set}"
cmake -DROOT="$ROOT" -DFILELIST="$FILELIST" -P "$CHECK_DIR/integrity.cmake" 2>&1

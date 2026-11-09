#!/bin/sh
# SPDX-License-Identifier: MIT
# Step hygiene: public-repo hygiene over the file list (home paths, secrets, foreign emails).
LC_ALL=C; export LC_ALL
: "${ROOT:?ROOT is not set}"
: "${CHECK_DIR:?CHECK_DIR is not set}"
: "${FILELIST:?FILELIST is not set}"
export ROOT FILELIST
sh "$CHECK_DIR/../hygiene.sh" --tree 2>&1

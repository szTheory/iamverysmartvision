#!/bin/sh
# SPDX-License-Identifier: MIT
# Step manifests: fail-closed fixture manifests (docs/fixture-manifest.md, schema_version 1).
LC_ALL=C; export LC_ALL
: "${ROOT:?ROOT is not set}"
: "${CHECK_DIR:?CHECK_DIR is not set}"
: "${FILELIST:?FILELIST is not set}"
cmake -DROOT="$ROOT" -DFILELIST="$FILELIST" -P "$CHECK_DIR/manifests.cmake" 2>&1

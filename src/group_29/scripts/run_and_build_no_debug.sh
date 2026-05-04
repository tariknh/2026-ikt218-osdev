#!/usr/bin/env bash
set -e

WORKSPACE="/workspaces/2026-ikt218-osdev"
SRC_DIR="$WORKSPACE/src/group_29"
BUILD_DIR="$WORKSPACE/build/group_29"

"$SRC_DIR/scripts/build.sh"

echo "Launching QEMU without debugger... close the QEMU window or press Ctrl+C in this terminal to stop."
/bin/bash "$SRC_DIR/scripts/run_no_debug.sh" \
  "$BUILD_DIR/kernel.iso" \
  "$BUILD_DIR/fs.img"
  

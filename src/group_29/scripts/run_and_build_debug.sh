#!/usr/bin/env bash
set -e

WORKSPACE="/workspaces/2026-ikt218-osdev"
SRC_DIR="$WORKSPACE/src/group_29"
BUILD_DIR="$WORKSPACE/build/group_29"

"$SRC_DIR/scripts/build.sh"

echo "Launching QEMU in debug mode... In VS Code, open Run and Debug and click the green 'Qemu Debug' button to attach."
/bin/bash "$SRC_DIR/scripts/start_qemu.sh" \
  "$BUILD_DIR/kernel.iso" \
  "$BUILD_DIR/fs.img"

#!/usr/bin/env bash
set -euo pipefail

SCRIPT_DIR="$(cd -- "$(dirname -- "${BASH_SOURCE[0]}")" && pwd)"
PROJECT_DIR="$(cd -- "$SCRIPT_DIR/../../.." && pwd)"

KERNEL_PATH="${1:-$PROJECT_DIR/build/13_lucky/kernel.iso}"
DISK_PATH="${2:-$PROJECT_DIR/build/13_lucky/disk.iso}"

if [[ ! -f "$KERNEL_PATH" ]]; then
    echo "Missing kernel image: $KERNEL_PATH"
    echo "Build the uiaos-create-image target first."
    exit 1
fi

if [[ ! -f "$DISK_PATH" ]]; then
    echo "Missing disk image: $DISK_PATH"
    echo "Build the uiaos-create-image target first."
    exit 1
fi

if command -v pactl >/dev/null 2>&1; then
    pactl set-sink-volume @DEFAULT_SINK@ 5% || true
fi

echo "Starting QEMU"
qemu-system-i386 \
  -cdrom "$KERNEL_PATH" -boot d \
  -drive file="$DISK_PATH",media=cdrom,format=raw \
  -m 64 \
  -display gtk \
  -serial pty

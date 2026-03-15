#!/bin/bash
set -eu

KERNEL_PATH=${1:-build/kernel.iso}
DISK_PATH=${2:-build/disk.iso}
DEBUG=${DEBUG:-0}
MODE=${MODE:-gui}

if [ ! -f "$KERNEL_PATH" ]; then
    echo "Missing kernel image: $KERNEL_PATH"
    exit 1
fi

if [ ! -f "$DISK_PATH" ]; then
    echo "Missing disk image: $DISK_PATH"
    exit 1
fi

if pgrep -f "qemu-system-i386" >/dev/null; then
    echo "Another qemu-system-i386 process is already running."
    echo "Stop it first: pkill -f qemu-system-i386"
    exit 1
fi

VIDEO_OPTS="-display gtk"
if [ "$MODE" = "headless" ]; then
    VIDEO_OPTS="-display none"
elif [ "$MODE" = "curses" ]; then
    VIDEO_OPTS="-display curses"
elif [ "$MODE" = "gui" ]; then
    if [ -z "${DISPLAY:-}" ]; then
        echo "MODE=gui requested, but DISPLAY is not set."
        echo "Use MODE=headless for terminal output only."
        exit 1
    fi
else
    echo "Invalid MODE: $MODE (use 'gui', 'curses', or 'headless')"
    exit 1
fi

QEMU_BASE_OPTS="-m 64 -boot d -cdrom $KERNEL_PATH -drive file=$DISK_PATH,format=raw,if=ide $VIDEO_OPTS -serial stdio"

if [ "$DEBUG" = "1" ]; then
    echo "Starting QEMU in debug mode (gdb on tcp::1234, CPU paused)..."
    qemu-system-i386 -S -gdb tcp::1234 $QEMU_BASE_OPTS
else
    echo "Starting QEMU..."
    qemu-system-i386 $QEMU_BASE_OPTS
fi

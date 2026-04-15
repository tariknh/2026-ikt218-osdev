#!/bin/bash
set -eu

KERNEL_PATH=${1:-build/kernel.iso}
DISK_PATH=${2:-build/disk.iso}
DEBUG=${DEBUG:-0}
MODE=${MODE:-gui}
AUDIO_BACKEND=${AUDIO_BACKEND:-auto}
AUDIO_OUTPUT=${AUDIO_OUTPUT:-build/pcspk.wav}

# Prefer WSLg PulseAudio socket when available.
if [ -z "${PULSE_SERVER:-}" ] && [ -S "/mnt/wslg/PulseServer" ]; then
    PULSE_SERVER="/mnt/wslg/PulseServer"
    export PULSE_SERVER
fi

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

if [ "$MODE" != "headless" ] && [ "$MODE" != "curses" ] && [ "$MODE" != "gui" ]; then
    echo "Invalid MODE: $MODE (use 'gui', 'curses', or 'headless')"
    exit 1
fi

if [ "$MODE" = "gui" ] && [ -z "${DISPLAY:-}" ]; then
    DISPLAY="host.docker.internal:0.0"
    export DISPLAY
    echo "DISPLAY was not set. Falling back to DISPLAY=${DISPLAY}."
    echo "If GUI fails, start an X server on host (VcXsrv/X410) or configure WSLg mount."
fi

QEMU_CMD=(
    qemu-system-i386
    -m 64
    -boot d
    -cdrom "$KERNEL_PATH"
    -drive "file=$DISK_PATH,format=raw,if=ide"
)

if [ "$MODE" = "headless" ]; then
    QEMU_CMD+=( -display none )
elif [ "$MODE" = "curses" ]; then
    QEMU_CMD+=( -display curses )
else
    QEMU_CMD+=( -display gtk )
fi

if [ "$AUDIO_BACKEND" = "auto" ]; then
    if [ -n "${PULSE_SERVER:-}" ]; then
        AUDIO_BACKEND="pa"
    else
        AUDIO_BACKEND="wav"
    fi
fi

if [ "$AUDIO_BACKEND" = "pa" ]; then
    if [ -z "${PULSE_SERVER:-}" ]; then
        echo "AUDIO_BACKEND=pa requested but PULSE_SERVER is not set."
        echo "Set PULSE_SERVER or use AUDIO_BACKEND=wav."
        exit 1
    fi
    echo "Using PulseAudio server: ${PULSE_SERVER}"
    QEMU_CMD+=( -audiodev "pa,id=snd0,server=${PULSE_SERVER}" -machine "pcspk-audiodev=snd0" )
elif [ "$AUDIO_BACKEND" = "wav" ]; then
    echo "Recording PC speaker audio to: ${AUDIO_OUTPUT}"
    QEMU_CMD+=( -audiodev "wav,id=snd0,path=${AUDIO_OUTPUT}" -machine "pcspk-audiodev=snd0" )
elif [ "$AUDIO_BACKEND" = "none" ]; then
    echo "Audio disabled (AUDIO_BACKEND=none)."
else
    echo "Invalid AUDIO_BACKEND: $AUDIO_BACKEND (use 'auto', 'pa', 'wav', or 'none')"
    exit 1
fi

QEMU_CMD+=( -serial stdio )

if [ "$DEBUG" = "1" ]; then
    echo "Starting QEMU in debug mode (gdb on tcp::1234, CPU paused)..."
    QEMU_CMD+=( -S -gdb tcp::1234 )
else
    echo "Starting QEMU..."
fi

"${QEMU_CMD[@]}"

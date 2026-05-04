#!/bin/sh
set -eu

SCRIPT_DIR="$(CDPATH= cd -- "$(dirname -- "$0")" && pwd)"
PROJECT_ROOT="$(CDPATH= cd -- "$SCRIPT_DIR/../../.." && pwd)"
ISO="$PROJECT_ROOT/build/solo_alex94/kernel.iso"

if [ ! -f "$ISO" ]; then
    echo "ERROR: kernel.iso was not found:"
    echo "  $ISO"
    echo ""
    echo "Build first with:"
    echo "  cmake --build build/solo_alex94 --target uiaos-create-image -j"
    exit 1
fi

IS_CONTAINER=0
if [ -f /.dockerenv ] || [ -f /run/.containerenv ]; then
    IS_CONTAINER=1
fi

OS_NAME="$(uname -s)"

QEMU_COMMON="-boot d -cdrom $ISO -m 64"
QEMU_DISPLAY="-display gtk,zoom-to-fit=on"

print_header() {
    echo "=== IRQ Rush: Kernel Panic ==="
    echo "Project root: $PROJECT_ROOT"
    echo "ISO:          $ISO"
    echo "Host OS seen from this shell: $OS_NAME"
    echo "Running in container: $IS_CONTAINER"
    echo ""
}

run_qemu() {
    AUDIODEV="$1"
    echo "Trying QEMU audio backend:"
    echo "  $AUDIODEV"
    echo ""
    # shellcheck disable=SC2086
    exec qemu-system-i386 $QEMU_COMMON $QEMU_DISPLAY \
        -audiodev "$AUDIODEV" \
        -machine pcspk-audiodev=snd0
}

run_no_audio() {
    echo "WARNING: No working audio backend was found."
    echo "Starting QEMU without sound so the game can still be tested visually."
    echo ""
    # shellcheck disable=SC2086
    exec qemu-system-i386 $QEMU_COMMON $QEMU_DISPLAY
}

print_header

if [ "$OS_NAME" = "Darwin" ]; then
    run_qemu "coreaudio,id=snd0,out.buffer-length=2000,timer-period=1000"
fi

if [ -S /mnt/wslg/PulseServer ]; then
    export PULSE_SERVER="unix:/mnt/wslg/PulseServer"
    echo "Detected WSLg PulseAudio socket:"
    echo "  $PULSE_SERVER"
    run_qemu "pa,id=snd0,server=$PULSE_SERVER,out.buffer-length=2000,timer-period=1000"
fi

if [ "$IS_CONTAINER" -eq 1 ]; then
    SERVER="${PULSE_SERVER:-tcp:host.docker.internal:4713}"
    export PULSE_SERVER="$SERVER"
    export SDL_AUDIODRIVER="pulseaudio"

    echo "Container audio route:"
    echo "  $PULSE_SERVER"
    echo ""
    echo "If this fails, start the host audio bridge first:"
    echo "  Windows host:"
    echo "    powershell -ExecutionPolicy Bypass -File src\\solo_alex94\\scripts\\run_assignment6_host_audio_windows.ps1"
    echo "  macOS host:"
    echo "    bash src/solo_alex94/scripts/run_assignment6_host_audio_macos.sh"
    echo ""

    run_qemu "pa,id=snd0,server=$PULSE_SERVER,out.buffer-length=2000,timer-period=1000"
fi

if [ -n "${PULSE_SERVER:-}" ]; then
    run_qemu "pa,id=snd0,server=$PULSE_SERVER,out.buffer-length=2000,timer-period=1000"
fi

# Native Linux fallback attempts.
qemu-system-i386 $QEMU_COMMON $QEMU_DISPLAY \
    -audiodev pa,id=snd0,out.buffer-length=2000,timer-period=1000 \
    -machine pcspk-audiodev=snd0 && exit 0

qemu-system-i386 $QEMU_COMMON $QEMU_DISPLAY \
    -audiodev pipewire,id=snd0,out.buffer-length=2000,timer-period=1000 \
    -machine pcspk-audiodev=snd0 && exit 0

qemu-system-i386 $QEMU_COMMON $QEMU_DISPLAY \
    -audiodev sdl,id=snd0,out.buffer-length=2000,timer-period=1000 \
    -machine pcspk-audiodev=snd0 && exit 0

run_no_audio

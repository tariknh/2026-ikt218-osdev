#!/bin/sh
set -eu

echo "=== IRQ Rush host audio bridge for macOS ==="
echo "This starts PulseAudio TCP on port 4713 so the Linux devcontainer can play QEMU sound."
echo "Leave this terminal open while QEMU runs inside the devcontainer."
echo ""

if ! command -v pulseaudio >/dev/null 2>&1; then
    echo "ERROR: pulseaudio is not installed on macOS."
    echo "Install it with:"
    echo "  brew install pulseaudio"
    exit 1
fi

if ! command -v pactl >/dev/null 2>&1; then
    echo "ERROR: pactl was not found. It should come with pulseaudio."
    exit 1
fi

pulseaudio --check >/dev/null 2>&1 || pulseaudio --daemonize=yes --exit-idle-time=-1

pactl load-module module-native-protocol-tcp auth-anonymous=1 port=4713 >/dev/null 2>&1 || true

echo "PulseAudio TCP bridge should now be available on host.docker.internal:4713"
echo "Press Ctrl+C only after you are done running QEMU."

while true; do sleep 3600; done

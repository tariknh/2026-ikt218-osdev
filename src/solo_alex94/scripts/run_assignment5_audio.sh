#!/bin/sh
set -eu

ISO="./build/solo_alex94/kernel.iso"
DISK="./build/solo_alex94/disk.iso"

if [ -z "${PULSE_SERVER:-}" ] && [ -S /mnt/wslg/PulseServer ]; then
  export PULSE_SERVER="unix:/mnt/wslg/PulseServer"
fi

echo "Using PULSE_SERVER=${PULSE_SERVER:-<not set>}"

qemu-system-i386 \
  -boot d \
  -cdrom "$ISO" \
  -drive file="$DISK",format=raw,if=ide,index=1 \
  -m 64 \
  -audiodev sdl,id=snd0 \
  -machine pcspk-audiodev=snd0

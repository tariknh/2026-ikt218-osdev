#!/bin/sh
set -eu

ISO="./build/solo_alex94/kernel.iso"
DISK="./build/solo_alex94/disk.iso"

qemu-system-i386 \
  -boot d \
  -cdrom "$ISO" \
  -drive file="$DISK",format=raw,if=ide,index=1 \
  -m 64 \
  -audiodev none,id=snd0 \
  -machine pcspk-audiodev=snd0

#!/bin/bash

KERNEL_PATH=$1
DISK_PATH=$2

if [ -z "$KERNEL_PATH" ]; then
    KERNEL_PATH="kernel.iso"
fi

echo "Starting QEMU..."

qemu-system-i386 \
    -cdrom "$KERNEL_PATH" \
    -m 64 \
    -display gtk \
    -audiodev dsound,id=snd0 \
    -machine pcspk-audiodev=snd0

echo "QEMU stopped."


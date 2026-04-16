#!/bin/bash

BUILD_DIR="/workspaces/2026-ikt218-osdev-4/src/OSDev4/build"

echo "==> Stopping any running QEMU..."
kill -9 $(pgrep qemu) 2>/dev/null
sleep 1

echo "==> Building kernel..."
cd "$BUILD_DIR"
make && make uiaos-create-image

if [ $? -ne 0 ]; then
    echo "Build failed!"
    exit 1
fi

echo "==> Starting QEMU..."
export DISPLAY=host.docker.internal:0
qemu-system-i386 -cdrom kernel.iso -m 64

#!/bin/bash
KERNEL_PATH=$1
DISK_PATH=$2

export DISPLAY=host.docker.internal:0

echo "Starting QEMU"
qemu-system-i386 -cdrom $KERNEL_PATH -m 64 -audiodev wav,id=wav1,path=/tmp/pcspk_output.wav -machine pcspk-audiodev=wav1 &
QEMU_PID=$!

cleanup() {
    echo "Stopping QEMU..."
    kill $QEMU_PID 2>/dev/null
    exit 0
}

trap cleanup SIGINT SIGTERM

echo "Waiting for gdb to start..."
sleep 3
echo "gdb started."

echo "Monitoring gdb connection..."
wait $QEMU_PID
cleanup

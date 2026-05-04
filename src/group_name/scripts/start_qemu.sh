#!/bin/bash
KERNEL_PATH=$1
DISK_PATH=$2

# Start QEMU in the background
echo "Starting QEMU"
qemu-system-i386 -S -gdb tcp::1234 \
  -cdrom "$KERNEL_PATH" -boot d \
  -drive file="$DISK_PATH",media=cdrom,format=raw \
  -m 64 \
  -audiodev pa,id=pa1,server=tcp:host.docker.internal,out.buffer-length=40000 \
  -machine pcspk-audiodev=pa1 \
  -display gtk \
  -serial pty &
QEMU_PID=$!

# Function to check if gdb is running
is_gdb_running() {
    pgrep -f "gdb-multiarch" > /dev/null
}

# Function to handle termination signals
cleanup() {
    echo "Stopping QEMU..."
    kill $QEMU_PID
    exit 0
}

# Trap SIGINT and SIGTERM signals
trap cleanup SIGINT SIGTERM

# Wait for gdb to start
echo "Waiting for gdb to start..."
while ! is_gdb_running; do
    sleep 1
done
echo "gdb started."

# Wait for gdb to stop
echo "Monitoring gdb connection..."
while is_gdb_running; do
    sleep 1
done

# Cleanup after gdb stops
cleanup

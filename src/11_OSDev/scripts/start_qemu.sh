#!/bin/bash
KERNEL_PATH=$1
DISK_PATH=$2

# Start QEMU in the background
echo "Starting QEMU"
qemu-system-i386 -S -gdb tcp::1234 -cdrom $KERNEL_PATH -drive file=$DISK_PATH,format=raw,index=0,media=disk -m 64 -boot d -serial pty -audiodev pa,id=snd0,server=docker.for.mac.host.internal:4713 -machine pc,pcspk-audiodev=snd0 &
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

# Wait for gdb to start (with timeout of 30 seconds)
echo "Waiting for gdb to start..."
TIMEOUT=30
ELAPSED=0
while ! is_gdb_running && [ $ELAPSED -lt $TIMEOUT ]; do
    sleep 1
    ELAPSED=$((ELAPSED + 1))
done

if is_gdb_running; then
    echo "gdb started."
    # Wait for gdb to stop
    echo "Monitoring gdb connection..."
    while is_gdb_running; do
        sleep 1
    done
else
    echo "GDB did not connect within $TIMEOUT seconds, keeping QEMU running..."
    # Keep QEMU running indefinitely
    while true; do
        sleep 1
        if ! kill -0 $QEMU_PID 2>/dev/null; then
            break
        fi
    done
fi

# Cleanup after gdb stops or QEMU exits
cleanup

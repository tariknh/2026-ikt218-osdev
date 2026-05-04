#!/bin/bash
KERNEL_PATH=$1
DISK_PATH=$2

# Start QEMU in the background
echo "Starting QEMU"
echo "Starting QEMU"


PULSE_SERVER=unix:/mnt/wslg/PulseServer 

export PULSE_SERVER=unix:/mnt/wslg/PulseServer

qemu-system-i386 -S -gdb tcp::1234 -boot d \
  -drive file=$KERNEL_PATH,format=raw,media=cdrom \
  -drive file=$DISK_PATH,format=raw,media=disk \
  -m 256 \
  -rtc base=localtime,clock=host,driftfix=slew \
  -audiodev pa,id=snd0,server=$PULSE_SERVER,out.frequency=48000,out.buffer-length=120000,out.latency=60000 \
  -machine pcspk-audiodev=snd0 \
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

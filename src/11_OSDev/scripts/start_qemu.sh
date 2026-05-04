#!/bin/bash
KERNEL_PATH=$1
DISK_PATH=$2

detect_pulse_server() {
    if getent hosts host.docker.internal &>/dev/null; then
        echo "host.docker.internal:4713"
        return
    fi

    BRIDGE_IP=$(ip route show default 2>/dev/null | awk '/default/ {print $3}' | head -1)
    if [ -n "$BRIDGE_IP" ]; then
        echo "${BRIDGE_IP}:4713"
        return
    fi

    echo "localhost:4713"
}

PULSE_SERVER=$(detect_pulse_server)
echo "Starting QEMU (PulseAudio server: $PULSE_SERVER)"
qemu-system-i386 -S -gdb tcp::1234 -cdrom $KERNEL_PATH -drive file=$DISK_PATH,format=raw,index=0,media=disk -m 64 -boot d -serial pty -audiodev pa,id=snd0,server=$PULSE_SERVER -machine pc,pcspk-audiodev=snd0 &
QEMU_PID=$!

# check if gdb is running
is_gdb_running() {
    pgrep -f "gdb-multiarch" > /dev/null
}

# handle termination signals
cleanup() {
    echo "Stopping QEMU..."
    kill $QEMU_PID
    exit 0
}


trap cleanup SIGINT SIGTERM


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

cleanup

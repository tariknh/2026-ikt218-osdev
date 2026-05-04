$ErrorActionPreference = "Stop"

Write-Host "=== IRQ Rush host audio bridge for Windows + WSLg ==="
Write-Host "This exposes the WSLg PulseAudio socket to Docker/devcontainer on TCP port 4713."
Write-Host "Leave this running while QEMU runs inside the devcontainer."
Write-Host ""

$Wsl = Get-Command "wsl.exe" -ErrorAction SilentlyContinue
if (!$Wsl) {
    Write-Host "ERROR: wsl.exe was not found. This bridge requires WSLg."
    exit 1
}

$PythonProxy = @'
import os
import socket
import threading

LISTEN_HOST = "0.0.0.0"
LISTEN_PORT = 4713
PULSE_SOCKET = "/mnt/wslg/PulseServer"

if not os.path.exists(PULSE_SOCKET):
    raise SystemExit(f"ERROR: {PULSE_SOCKET} does not exist. WSLg audio is not available.")

def pipe(src, dst):
    try:
        while True:
            data = src.recv(65536)
            if not data:
                break
            dst.sendall(data)
    except Exception:
        pass
    finally:
        try:
            src.close()
        except Exception:
            pass
        try:
            dst.close()
        except Exception:
            pass

def handle(client):
    server = socket.socket(socket.AF_UNIX, socket.SOCK_STREAM)
    server.connect(PULSE_SOCKET)
    threading.Thread(target=pipe, args=(client, server), daemon=True).start()
    threading.Thread(target=pipe, args=(server, client), daemon=True).start()

listener = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
listener.setsockopt(socket.SOL_SOCKET, socket.SO_REUSEADDR, 1)
listener.bind((LISTEN_HOST, LISTEN_PORT))
listener.listen(32)
print(f"WSLg PulseAudio bridge listening on {LISTEN_HOST}:{LISTEN_PORT} -> {PULSE_SOCKET}", flush=True)

while True:
    client, addr = listener.accept()
    threading.Thread(target=handle, args=(client,), daemon=True).start()
'@

$Encoded = [Convert]::ToBase64String([Text.Encoding]::UTF8.GetBytes($PythonProxy))

$WslCommand = @"
set -e
pkill -f wslg_pulse_tcp_bridge.py 2>/dev/null || true
printf '%s' '$Encoded' | base64 -d > /tmp/wslg_pulse_tcp_bridge.py
python3 /tmp/wslg_pulse_tcp_bridge.py
"@

wsl.exe -- bash -lc $WslCommand

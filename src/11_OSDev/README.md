# UiAOS — University of Agder Operating System

A bare-metal x86 kernel built for IKT218 Advanced Operating Systems.  
Runs in QEMU inside a Docker devcontainer.

---

## Prerequisites

### Both platforms
- [Docker Desktop](https://www.docker.com/products/docker-desktop/)
- [Visual Studio Code](https://code.visualstudio.com/)
- VS Code extension: **Dev Containers** (`ms-vscode-remote.remote-containers`)

### macOS only (for PC Speaker audio)
```bash
brew install pulseaudio
```

### Windows only (for PC Speaker audio)
Download and install [PulseAudio for Windows](https://pgaskin.net/pulseaudio-win32/).  
Extract it somewhere permanent, e.g. `C:\pulseaudio`.

---

## Getting Started

### 1. Open in devcontainer

Clone the repo, open it in VS Code, then when prompted click  
**"Reopen in Container"** — or press `Ctrl+Shift+P` → `Dev Containers: Reopen in Container`.

Wait for the container to build (first time takes a few minutes).

### 2. Build the kernel

Inside the devcontainer terminal:

```bash
cd src/11_OSDev
cmake -B build
cmake --build build
```

The kernel image is produced at `build/kernel.iso`.

### 3. Start audio (required for the music player)

Do this **on your host machine** (outside the container), before launching QEMU.

**macOS:**
```bash
pulseaudio --start
pactl load-module module-native-protocol-tcp auth-anonymous=1
```

> After the first time you only need `pulseaudio --start` — the TCP module loads
> automatically if you followed the permanent setup below.

**Windows:**  
Open `C:\pulseaudio\bin\pulseaudio.exe` (run as normal user, not admin).  
Then in the same folder open `default.pa` in a text editor and make sure this line
is present and uncommented:
```
load-module module-native-protocol-tcp auth-anonymous=1
```

### 4. Launch QEMU

Run this from inside the devcontainer terminal:

```bash
bash scripts/start_qemu.sh
```

The QEMU window will open. The OS boots directly into the main menu.

---

## Using the OS

| Key | Action |
|-----|--------|
| `2` | Assignment 2 — GDT & VGA Terminal |
| `3` | Assignment 3 — Interrupts & Keyboard |
| `4` | Assignment 4 — Memory Management & PIT |
| `5` | Assignment 5 — PC Speaker Music Player |
| `6` | Assignment 6 — System Monitor |
| `q` | Return to main menu from any demo |

---

## Permanent audio setup (macOS)

So you only need to run `pulseaudio --start` each session:

```bash
sudo nano /opt/homebrew/etc/pulse/default.pa
```

Find this line and uncomment it (remove the `#`):
```
load-module module-native-protocol-tcp auth-anonymous=1
```

Save and close. PulseAudio will now load the TCP module automatically on start.

---

## Troubleshooting

**QEMU window doesn't open**  
Make sure Docker Desktop is running and you are inside the devcontainer terminal.

**No sound from the music player**  
- macOS: Run `pulseaudio --start` on the host, then restart QEMU.  
- Windows: Make sure `pulseaudio.exe` is running before launching QEMU.  
- Check that `pactl info` (macOS) shows the server is running.

**Build errors**  
Make sure you are in the devcontainer (bottom-left of VS Code shows the container name).  
Try a clean build:
```bash
rm -rf build && cmake -B build && cmake --build build
```

**QEMU mouse is captured**  
Press `Ctrl+Alt+G` (macOS) or `Ctrl+Alt` (Windows) to release the mouse.

---

## Project structure

```
src/11_OSDev/
├── src/          # Kernel source (C, C++, NASM)
├── include/      # Header files
├── scripts/      # QEMU launch script
└── CMakeLists.txt
```
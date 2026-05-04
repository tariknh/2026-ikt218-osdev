#!/usr/bin/env bash
set -euo pipefail

cmake --build /workspaces/2026-ikt218-osdev/build/33_faton
cmake --build /workspaces/2026-ikt218-osdev/build/33_faton --target uiaos-create-image

exec qemu-system-i386 \
    -cdrom /workspaces/2026-ikt218-osdev/build/33_faton/kernel.iso \
    -m 64 \
    -display gtk \
    -audiodev sdl,id=audio0 \
    -machine pcspk-audiodev=audio0
#!/usr/bin/env bash
set -e

WORKSPACE="/workspaces/2026-ikt218-osdev"
SRC_DIR="$WORKSPACE/src/group_29"
BUILD_DIR="$WORKSPACE/build/group_29"

export CC=/usr/local/bin/i686-elf-gcc
export CXX=/usr/local/bin/i686-elf-g++

cmake -S "$SRC_DIR" -B "$BUILD_DIR"
cmake --build "$BUILD_DIR" --target uiaos-create-image
#!/bin/bash
set -euo pipefail

SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
REPO_ROOT="$(cd "${SCRIPT_DIR}/../../.." && pwd)"

SOURCE_DIR="src/28_lazarOS"
BUILD_DIR="build"

MODE="${MODE:-curses}"
DEBUG="${DEBUG:-0}"

RUN_QEMU=1
FORCE_CONFIGURE=0
FORCE_CLEAN=0

usage() {
    cat <<'EOF'
Usage: bash src/28_lazarOS/scripts/dev_cycle.sh [options]

Options:
  --clean         Remove build/ before configuring
  --configure     Force CMake configure step
  --no-run        Build artifacts, but do not launch QEMU
  --mode <mode>   QEMU mode: gui | curses | headless (default: curses)
  --debug         Start QEMU with GDB stub (tcp::1234)
  -h, --help      Show this help

Environment overrides:
  MODE=<mode>     Same as --mode
  DEBUG=1         Same as --debug

Examples:
  bash src/28_lazarOS/scripts/dev_cycle.sh
  bash src/28_lazarOS/scripts/dev_cycle.sh --clean --configure
  MODE=headless bash src/28_lazarOS/scripts/dev_cycle.sh --no-run
  DEBUG=1 bash src/28_lazarOS/scripts/dev_cycle.sh --mode curses
EOF
}

while [[ $# -gt 0 ]]; do
    case "$1" in
        --clean)
            FORCE_CLEAN=1
            ;;
        --configure)
            FORCE_CONFIGURE=1
            ;;
        --no-run)
            RUN_QEMU=0
            ;;
        --mode)
            if [[ $# -lt 2 ]]; then
                echo "Missing value for --mode"
                usage
                exit 1
            fi
            MODE="$2"
            shift
            ;;
        --debug)
            DEBUG=1
            ;;
        -h|--help)
            usage
            exit 0
            ;;
        *)
            echo "Unknown option: $1"
            usage
            exit 1
            ;;
    esac
    shift
done

cd "$REPO_ROOT"
echo "Repo root: $REPO_ROOT"

if [[ "$FORCE_CLEAN" -eq 1 ]]; then
    echo "Cleaning build directory..."
    rm -rf "$BUILD_DIR"
fi

NEED_CONFIGURE=0
if [[ "$FORCE_CONFIGURE" -eq 1 ]]; then
    NEED_CONFIGURE=1
elif [[ ! -f "$BUILD_DIR/CMakeCache.txt" ]]; then
    NEED_CONFIGURE=1
fi

if [[ "$NEED_CONFIGURE" -eq 1 ]]; then
    echo "Configuring CMake..."
    cmake -S "$SOURCE_DIR" -B "$BUILD_DIR" \
        -DCMAKE_C_COMPILER=/usr/local/bin/i686-elf-gcc \
        -DCMAKE_CXX_COMPILER=/usr/local/bin/i686-elf-g++ \
        -DCMAKE_TRY_COMPILE_TARGET_TYPE=STATIC_LIBRARY
fi

echo "Building kernel..."
cmake --build "$BUILD_DIR" -j

echo "Building bootable image..."
cmake --build "$BUILD_DIR" --target uiaos-create-image -j

if [[ "$RUN_QEMU" -eq 1 ]]; then
    echo "Starting QEMU (MODE=$MODE, DEBUG=$DEBUG)..."
    MODE="$MODE" DEBUG="$DEBUG" bash "$SOURCE_DIR/scripts/start_qemu.sh" "$BUILD_DIR/kernel.iso" "$BUILD_DIR/disk.iso"
else
    echo "Skipping QEMU launch (--no-run)."
    echo "Built files:"
    echo "  $BUILD_DIR/kernel.iso"
    echo "  $BUILD_DIR/disk.iso"
fi
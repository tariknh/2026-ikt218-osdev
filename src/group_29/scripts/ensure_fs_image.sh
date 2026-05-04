#!/usr/bin/env bash
set -e

DISK_PATH=$1

if [ -z "$DISK_PATH" ]; then
  echo "Usage: $0 <fs-image-path>" >&2
  exit 1
fi

mkdir -p "$(dirname "$DISK_PATH")"

if [ ! -f "$DISK_PATH" ]; then
  dd if=/dev/zero of="$DISK_PATH" bs=1M count=8 status=none
  echo "Created persistent filesystem image: $DISK_PATH"
fi

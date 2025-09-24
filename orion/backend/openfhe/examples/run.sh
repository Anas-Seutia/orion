#!/bin/bash

# Run script for OpenFHE examples

set -e  # Exit on error

SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
BUILD_DIR="$SCRIPT_DIR/build"

if [ ! -f "$BUILD_DIR/simple-real-numbers" ]; then
    echo "Example not built yet. Building first..."
    "$SCRIPT_DIR/build.sh"
fi

echo "Running simple-real-numbers example:"
echo "======================================"
cd "$BUILD_DIR"
./simple-real-numbers
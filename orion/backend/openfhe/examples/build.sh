#!/bin/bash

# Build script for OpenFHE examples

set -e  # Exit on error

SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
BUILD_DIR="$SCRIPT_DIR/build"

echo "Building OpenFHE examples..."

# Create build directory if it doesn't exist
mkdir -p "$BUILD_DIR"

# Change to build directory
cd "$BUILD_DIR"

# Run cmake and make
echo "Configuring with CMake..."
cmake ..

echo "Building..."
make

echo ""
echo "Build complete! Available executables:"
ls -la simple-real-numbers

echo ""
echo "To run the example:"
echo "  cd $BUILD_DIR && ./simple-real-numbers"

echo ""
echo "Or use the run script:"
echo "  $SCRIPT_DIR/run.sh"
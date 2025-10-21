#!/bin/bash
# Build script for cross-compiling DoorMod for aarch64

# Remove old build directory to ensure clean build
rm -rf build

# Create build directory
mkdir -p build
cd build

# Configure CMake with the aarch64 toolchain
cmake -DCMAKE_TOOLCHAIN_FILE=../toolchain-aarch64.cmake ..

# Build the project
cmake --build .

echo ""
echo "Build complete! Binary is at: build/app/doorMod"
echo "To deploy, copy build/app/doorMod to your target device."

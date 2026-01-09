#!/bin/bash

# Android SO Injector Build Script
# Builds for all Android ABIs

set -e

echo "Android SO Injector - Build Script"
echo "==================================="

# Check if NDK is set
if [ -z "$ANDROID_NDK" ]; then
    echo "Error: ANDROID_NDK environment variable not set"
    echo "Please set it to your Android NDK path"
    echo "Example: export ANDROID_NDK=/path/to/ndk"
    exit 1
fi

echo "Using NDK: $ANDROID_NDK"

# ABIs to build
ABIS=("armeabi-v7a" "arm64-v8a" "x86" "x86_64")

# Create build directory
mkdir -p build
cd build

for ABI in "${ABIS[@]}"
do
    echo ""
    echo "Building for ABI: $ABI"
    echo "------------------------"
    
    mkdir -p "$ABI"
    cd "$ABI"
    
    cmake ../.. \
        -G Ninja \
        -DANDROID_ABI="$ABI" \
        -DANDROID_PLATFORM=android-21 \
        -DCMAKE_TOOLCHAIN_FILE="$ANDROID_NDK/build/cmake/android.toolchain.cmake" \
        -DCMAKE_BUILD_TYPE=Release
    
    cmake --build .
    
    cd ..
done

echo ""
echo "Build completed successfully!"
echo "Binaries are in build/<ABI>/ directories"

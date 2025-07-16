#!/bin/bash

echo "======================================="
echo "   Connect Messenger - Build Script"
echo "======================================="
echo

BUILD_TYPE="Release"

if [ "$1" = "debug" ]; then
    BUILD_TYPE="Debug"
    echo "Building in Debug mode..."
else
    echo "Building in Release mode..."
fi

cd "$(dirname "$0")"

echo
echo "[1/4] Installing dependencies..."

# Check if we're on Ubuntu/Debian
if command -v apt-get &> /dev/null; then
    echo "Installing system dependencies..."
    sudo apt-get update
    sudo apt-get install -y build-essential cmake pkg-config libssl-dev libsqlite3-dev libsodium-dev
    
    # Try Qt6 first, then Qt5
    if ! sudo apt-get install -y qt6-base-dev qt6-websockets-dev qt6-multimedia-dev qt6-multimediawidgets-dev; then
        echo "Qt6 not available, trying Qt5..."
        sudo apt-get install -y qtbase5-dev qtwebsockets5-dev qtmultimedia5-dev qtmultimediawidgets5-dev
    fi
fi

echo
echo "[2/4] Creating build directory..."
mkdir -p build
cd build

echo
echo "[3/4] Configuring CMake..."
cmake .. -DCMAKE_BUILD_TYPE=$BUILD_TYPE

if [ $? -ne 0 ]; then
    echo
    echo "CMake configuration failed!"
    exit 1
fi

echo
echo "[4/4] Building project..."
make -j$(nproc)

if [ $? -ne 0 ]; then
    echo
    echo "Build failed!"
    exit 1
fi

echo
echo "======================================="
echo "    Build completed successfully!"
echo "======================================="
echo
echo "Server executable: build/ConnectServer"
echo "Client executable: build/ConnectClient"
echo
echo "You can now run:"
echo "  ./start_server.sh    # Start the server"
echo "  ./start_client.sh    # Start the client"
echo 
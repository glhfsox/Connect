#!/bin/bash

# Connect Messenger Build Script

# Colors for output
RED='\033[0;31m'
GREEN='\033[0;32m'
BLUE='\033[0;34m'
YELLOW='\033[1;33m'
NC='\033[0m' # No Color

echo -e "${BLUE}======================================="
echo -e "   Connect Messenger - Build Script"
echo -e "=======================================${NC}"
echo

BUILD_TYPE="Release"

if [ "$1" == "debug" ]; then
    BUILD_TYPE="Debug"
    echo -e "${YELLOW}Building in Debug mode...${NC}"
else
    echo -e "${GREEN}Building in Release mode...${NC}"
fi

# Get script directory
SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
cd "$SCRIPT_DIR"

echo
echo -e "${BLUE}[1/4] Installing system dependencies...${NC}"

# Check for required packages
REQUIRED_PACKAGES="build-essential cmake pkg-config libssl-dev libsqlite3-dev libsodium-dev"

if command -v apt-get &> /dev/null; then
    echo "Installing packages with apt-get..."
    
    # Try Qt6 first
    if apt-cache search qt6-base-dev | grep -q qt6-base-dev; then
        QT_PACKAGES="qt6-base-dev qt6-websockets-dev qt6-multimedia-dev"
        echo "Using Qt6 packages"
    else
        QT_PACKAGES="qtbase5-dev qtwebsockets5-dev qtmultimedia5-dev"
        echo "Using Qt5 packages"
    fi
    
    sudo apt-get update
    sudo apt-get install -y $REQUIRED_PACKAGES $QT_PACKAGES
elif command -v yum &> /dev/null; then
    echo "Installing packages with yum..."
    sudo yum install -y gcc gcc-c++ cmake pkgconfig openssl-devel sqlite-devel libsodium-devel qt5-qtbase-devel qt5-qtwebsockets-devel qt5-qtmultimedia-devel
elif command -v pacman &> /dev/null; then
    echo "Installing packages with pacman..."
    sudo pacman -S --needed base-devel cmake pkgconf openssl sqlite libsodium qt6-base qt6-websockets qt6-multimedia
else
    echo -e "${YELLOW}Warning: Could not detect package manager. Please install dependencies manually.${NC}"
fi

echo
echo -e "${BLUE}[2/4] Creating build directory...${NC}"
if [ ! -d "build" ]; then
    mkdir build
fi
cd build

echo
echo -e "${BLUE}[3/4] Configuring CMake...${NC}"
cmake .. -DCMAKE_BUILD_TYPE=$BUILD_TYPE

if [ $? -ne 0 ]; then
    echo -e "${RED}CMake configuration failed!${NC}"
    exit 1
fi

echo
echo -e "${BLUE}[4/4] Building project...${NC}"
make -j$(nproc)

if [ $? -ne 0 ]; then
    echo -e "${RED}Build failed!${NC}"
    exit 1
fi

echo
echo -e "${GREEN}======================================="
echo -e "    Build completed successfully!"
echo -e "=======================================${NC}"
echo
echo -e "${YELLOW}Server executable: build/ConnectServer${NC}"
if [ -f "ConnectClient" ]; then
    echo -e "${YELLOW}Client executable: build/ConnectClient${NC}"
fi
echo
echo -e "${GREEN}You can now run ./start_server.sh to start the server${NC}"
echo 
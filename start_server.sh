#!/bin/bash

# Connect Messenger Server Startup Script

# Colors for output
RED='\033[0;31m'
GREEN='\033[0;32m'
BLUE='\033[0;34m'
YELLOW='\033[1;33m'
NC='\033[0m' # No Color

echo -e "${BLUE}======================================="
echo -e "      Connect Messenger Server"
echo -e "=======================================${NC}"
echo

# Get script directory
SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
cd "$SCRIPT_DIR"

# Check for server executable
if [ -f "build/ConnectServer" ]; then
    echo -e "${GREEN}Starting server from build directory...${NC}"
    ./build/ConnectServer
elif [ -f "ConnectServer" ]; then
    echo -e "${GREEN}Starting server...${NC}"
    ./ConnectServer
else
    echo -e "${RED}Error: ConnectServer not found!${NC}"
    echo -e "${YELLOW}Please build the project first:${NC}"
    echo "  mkdir build && cd build"
    echo "  cmake .."
    echo "  make -j\$(nproc)"
    exit 1
fi

echo -e "${YELLOW}Server stopped. Press any key to exit...${NC}"
read -n 1 -s 
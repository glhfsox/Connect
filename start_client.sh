#!/bin/bash

echo "======================================="
echo "      Connect Messenger Client"
echo "======================================="
echo

cd "$(dirname "$0")"

if [ -f "build/ConnectClient" ]; then
    echo "Starting client from build directory..."
    ./build/ConnectClient
elif [ -f "ConnectClient" ]; then
    echo "Starting client..."
    ./ConnectClient
else
    echo "Error: ConnectClient not found!"
    echo "Please build the project first: ./build_project.sh"
    exit 1
fi 
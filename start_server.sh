#!/bin/bash

echo "======================================="
echo "      Connect Messenger Server"
echo "======================================="
echo

cd "$(dirname "$0")"

if [ -f "build/ConnectServer" ]; then
    echo "Starting server from build directory..."
    ./build/ConnectServer
elif [ -f "ConnectServer" ]; then
    echo "Starting server..."
    ./ConnectServer
else
    echo "Error: ConnectServer not found!"
    echo "Please build the project first: ./build_project.sh"
    exit 1
fi 
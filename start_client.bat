@echo off
title Connect Messenger Client
color 0B

echo =======================================
echo      Connect Messenger Client
echo =======================================
echo.

cd /d "%~dp0"

if exist "build\Release\ConnectClient.exe" (
    echo Starting client from Release build...
    build\Release\ConnectClient.exe
) else if exist "build\Debug\ConnectClient.exe" (
    echo Starting client from Debug build...
    build\Debug\ConnectClient.exe
) else if exist "ConnectClient.exe" (
    echo Starting client...
    ConnectClient.exe
) else (
    echo Error: ConnectClient.exe not found!
    echo Please build the project first.
    pause
    exit /b 1
)

pause 
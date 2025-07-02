@echo off
title Connect Messenger Server
color 0A

echo =======================================
echo      Connect Messenger Server
echo =======================================
echo.

cd /d "%~dp0"

if exist "build\Release\ConnectServer.exe" (
    echo Starting server from Release build...
    build\Release\ConnectServer.exe
) else if exist "build\Debug\ConnectServer.exe" (
    echo Starting server from Debug build...
    build\Debug\ConnectServer.exe
) else if exist "ConnectServer.exe" (
    echo Starting server...
    ConnectServer.exe
) else (
    echo Error: ConnectServer.exe not found!
    echo Please build the project first.
    pause
    exit /b 1
)

pause 
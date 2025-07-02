@echo off
title Connect Messenger - Build Script
color 0E

echo =======================================
echo   Connect Messenger - Build Script
echo =======================================
echo.

set BUILD_TYPE=Release

if "%1"=="debug" (
    set BUILD_TYPE=Debug
    echo Building in Debug mode...
) else (
    echo Building in Release mode...
)

cd /d "%~dp0"

echo.
echo [1/4] Installing vcpkg dependencies...
if not exist "vcpkg\vcpkg.exe" (
    echo Building vcpkg...
    cd vcpkg
    call bootstrap-vcpkg.bat
    cd ..
)

echo.
echo [2/4] Installing packages...
vcpkg\vcpkg.exe install --triplet=x64-windows

echo.
echo [3/4] Configuring CMake...
if not exist "build" mkdir build
cd build

cmake .. -DCMAKE_TOOLCHAIN_FILE=../vcpkg/scripts/buildsystems/vcpkg.cmake -G "Visual Studio 17 2022" -A x64 -DCMAKE_BUILD_TYPE=%BUILD_TYPE%

if errorlevel 1 (
    echo.
    echo CMake configuration failed!
    pause
    exit /b 1
)

echo.
echo [4/4] Building project...
cmake --build . --config %BUILD_TYPE% --parallel

if errorlevel 1 (
    echo.
    echo Build failed!
    pause
    exit /b 1
)

echo.
echo =======================================
echo    Build completed successfully!
echo =======================================
echo.
echo Server executable: build\%BUILD_TYPE%\ConnectServer.exe
if exist "%BUILD_TYPE%\ConnectClient.exe" (
    echo Client executable: build\%BUILD_TYPE%\ConnectClient.exe
)
echo.
echo You can now run start_server.bat to start the server
echo.
pause 
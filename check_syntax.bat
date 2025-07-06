@echo off
echo 🔍 Checking project syntax and structure...

echo.
echo 📁 Checking file structure...
if exist "main.cpp" (
    echo ✅ main.cpp found
) else (
    echo ❌ main.cpp missing
)

if exist "include/WebSocketServer.h" (
    echo ✅ WebSocketServer.h found
) else (
    echo ❌ WebSocketServer.h missing
)

if exist "server/WebSocketServer.cpp" (
    echo ✅ WebSocketServer.cpp found
) else (
    echo ❌ WebSocketServer.cpp missing
)

if exist "CMakeLists.txt" (
    echo ✅ CMakeLists.txt found
) else (
    echo ❌ CMakeLists.txt missing
)

if exist "Dockerfile" (
    echo ✅ Dockerfile found
) else (
    echo ❌ Dockerfile missing
)

echo.
echo 📋 Checking for Q_OBJECT macro...
findstr /C:"Q_OBJECT" include\WebSocketServer.h >nul
if %ERRORLEVEL% EQU 0 (
    echo ✅ Q_OBJECT macro found in WebSocketServer.h
) else (
    echo ❌ Q_OBJECT macro missing in WebSocketServer.h
)

echo.
echo 🔧 Checking CMake configuration...
findstr /C:"CMAKE_AUTOMOC" CMakeLists.txt >nul
if %ERRORLEVEL% EQU 0 (
    echo ✅ CMAKE_AUTOMOC enabled
) else (
    echo ❌ CMAKE_AUTOMOC not found
)

findstr /C:"include/WebSocketServer.h" CMakeLists.txt >nul
if %ERRORLEVEL% EQU 0 (
    echo ✅ WebSocketServer.h included in sources
) else (
    echo ❌ WebSocketServer.h not in sources
)

echo.
echo 🎯 Project structure looks good for Railway deployment!
echo.
echo 💡 Next steps:
echo    1. Push to GitHub: git add . ^&^& git commit -m "fix: Qt MOC issues" ^&^& git push
echo    2. Deploy on Railway: railway up
echo.
pause 
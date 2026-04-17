@echo off
setlocal

echo Building Bird Scene for Windows...
echo.

where cmake >nul 2>nul
if errorlevel 1 (
    echo CMake was not found in PATH.
    echo Install CMake first, then run this script again.
    exit /b 1
)

if not exist build mkdir build
cmake -S . -B build
if errorlevel 1 exit /b 1

cmake --build build --config Release
if errorlevel 1 exit /b 1

echo.
echo Build finished.
echo If your generator created a Release folder, the executable is usually in:
echo   build\Release\bird_scene.exe
echo Otherwise check:
echo   build\bird_scene.exe
echo.
echo Keep the models folder next to the repo or executable when sharing the project.

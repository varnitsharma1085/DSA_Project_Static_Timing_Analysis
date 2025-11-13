@echo off
echo Building Static Timing Analysis Tool...

if not exist "bin" mkdir bin

echo Compiling...
g++ -std=c++17 -Wall -O2 src/main.cpp -o bin/sta.exe

if %ERRORLEVEL% EQU 0 (
    echo Build successful!
    echo Executable: bin\sta.exe
) else (
    echo Build failed!
)

echo.
echo Press Enter to exit...
pause >nul

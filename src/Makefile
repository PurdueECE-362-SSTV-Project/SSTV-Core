@echo off
REM Build script for SSTV Decoder Testbench (Windows)

setlocal enabledelayedexpansion

REM Check for command line argument
if "%1"=="" goto :build
if "%1"=="clean" goto :clean
if "%1"=="run" goto :run
if "%1"=="rebuild" goto :rebuild
if "%1"=="help" goto :help
echo Unknown command: %1
goto :help

:build
echo.
echo ========================================
echo Building SSTV Decoder Testbench
echo ========================================
echo.

REM Create directories
if not exist build mkdir build
if not exist mock_pico\pico mkdir mock_pico\pico
if not exist mock_pico\hardware mkdir mock_pico\hardware
if not exist lib\kissfft-master mkdir lib\kissfft-master

REM Create mock headers
echo Creating mock Pico SDK headers...

echo // Mock pico/stdlib.h > mock_pico\pico\stdlib.h
echo #ifndef PICO_STDLIB_H >> mock_pico\pico\stdlib.h
echo #define PICO_STDLIB_H >> mock_pico\pico\stdlib.h
echo #include ^<stdint.h^> >> mock_pico\pico\stdlib.h
echo #include ^<stdbool.h^> >> mock_pico\pico\stdlib.h
echo typedef uint32_t uint; >> mock_pico\pico\stdlib.h
echo #endif >> mock_pico\pico\stdlib.h

echo // Mock hardware/xosc.h > mock_pico\hardware\xosc.h
echo #ifndef HARDWARE_XOSC_H >> mock_pico\hardware\xosc.h
echo #define HARDWARE_XOSC_H >> mock_pico\hardware\xosc.h
echo #endif >> mock_pico\hardware\xosc.h

echo // Mock pico/multicore.h > mock_pico\pico\multicore.h
echo #ifndef PICO_MULTICORE_H >> mock_pico\pico\multicore.h
echo #define PICO_MULTICORE_H >> mock_pico\pico\multicore.h
echo #endif >> mock_pico\pico\multicore.h

echo // Mock kiss_fft.h > lib\kissfft-master\kiss_fft.h
echo #ifndef KISS_FFT_H >> lib\kissfft-master\kiss_fft.h
echo #define KISS_FFT_H >> lib\kissfft-master\kiss_fft.h
echo #endif >> lib\kissfft-master\kiss_fft.h

echo // Mock kiss_fftr.h > lib\kissfft-master\kiss_fftr.h
echo #ifndef KISS_FFTR_H >> lib\kissfft-master\kiss_fftr.h
echo #define KISS_FFTR_H >> lib\kissfft-master\kiss_fftr.h
echo #endif >> lib\kissfft-master\kiss_fftr.h

echo.
echo Compiling testbench...

REM Compile with gcc (MinGW)
gcc -Wall -Wextra -g -O0 -std=c11 -DTESTBENCH_MODE -I./mock_pico -o build\sstv_testbench.exe claude.c

if %ERRORLEVEL% EQU 0 (
    echo.
    echo ========================================
    echo Build successful!
    echo Executable: build\sstv_testbench.exe
    echo ========================================
    echo.
) else (
    echo.
    echo ========================================
    echo Build FAILED!
    echo ========================================
    echo.
    exit /b 1
)
goto :end

:run
call :build
if %ERRORLEVEL% EQU 0 (
    echo.
    echo ========================================
    echo Running SSTV Decoder Testbench
    echo ========================================
    echo.
    build\sstv_testbench.exe
)
goto :end

:clean
echo.
echo Cleaning build artifacts...
if exist build rmdir /s /q build
if exist mock_pico rmdir /s /q mock_pico
if exist lib\kissfft-master\kiss_fft.h del lib\kissfft-master\kiss_fft.h
if exist lib\kissfft-master\kiss_fftr.h del lib\kissfft-master\kiss_fftr.h
echo Clean complete!
echo.
goto :end

:rebuild
call :clean
call :build
goto :end

:help
echo.
echo ========================================
echo SSTV Decoder Testbench Build Script
echo ========================================
echo.
echo Usage: build.bat [command]
echo.
echo Commands:
echo   (none)   - Build the testbench
echo   run      - Build and run the testbench
echo   clean    - Remove build artifacts
echo   rebuild  - Clean and rebuild
echo   help     - Show this help message
echo.
echo Requirements:
echo   - GCC (MinGW) must be installed and in PATH
echo   - Run from the src directory
echo.
goto :end

:end
endlocal
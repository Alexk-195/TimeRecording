@echo off
echo ============================================
echo TimeRecording Compilation Script
echo ============================================
echo.

:: Enable delayed variable expansion for better error handling
setlocal EnableDelayedExpansion

:: Check if we're in the right directory
if not exist "main.cpp" (
    echo ERROR: main.cpp not found in current directory!
    echo Please make sure you're running this script from the correct folder.
    echo.
    pause
    exit /b 1
)

:: Display compiler version
echo Step 2: Checking compiler version...
cl 2>&1 | findstr "Version"
echo.

:: Attempt compilation
echo Step 3: Compiling with Visual Studio...
echo Command: cl /EHsc /utf-8 /O2 *.cpp /Fe:TimeRecording.exe /link user32.lib gdi32.lib comctl32.lib shell32.lib
echo.
cl /EHsc /utf-8 /O2 *.cpp /Fe:TimeRecording.exe /link user32.lib gdi32.lib comctl32.lib shell32.lib

set COMPILE_RESULT=%ERRORLEVEL%
echo.
echo Compilation finished with exit code: %COMPILE_RESULT%
echo.

if %COMPILE_RESULT% EQU 0 (
    echo ============================================
    echo COMPILATION SUCCESSFUL!
    echo ============================================
    echo.
    if exist "TimeRecording.exe" (
        echo Executable created: TimeRecording.exe
        for %%I in (TimeRecording.exe) do echo File size: %%~zI bytes
        echo.
    ) else (
        echo WARNING: Compilation reported success but executable not found!
    )
    echo To add to Windows startup:
    echo 1. Press Win+R, type 'shell:startup', press Enter
    echo 2. Copy TimeRecording.exe to the opened folder
    echo.
) else (
    echo ============================================
    echo COMPILATION FAILED!
    echo ============================================
    echo.
    echo Exit code: %COMPILE_RESULT%
    echo.
    echo TROUBLESHOOTING:
    echo 1. Make sure you're running from Developer Command Prompt
    echo 2. Check that main.cpp exists and is readable
    echo 3. Try running: compile_auto.bat (attempts to find VS automatically)
    echo 4. Try MinGW instead: compile_mingw.bat
    echo.
    echo If the error mentions missing headers or libraries:
    echo - Reinstall Visual Studio Build Tools with C++ workload
    echo - Make sure Windows SDK is installed
    echo.
)

echo.
echo Press any key to exit...
pause >nul
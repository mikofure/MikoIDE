@echo off
echo Building MikoIDE Installer...

REM Check if NSIS is installed
where makensis >nul 2>&1
if %ERRORLEVEL% NEQ 0 (
    echo ERROR: NSIS (makensis) not found in PATH
    echo Please install NSIS from https://nsis.sourceforge.io/
    pause
    exit /b 1
)

REM Create build directory structure
if not exist "build" mkdir build
if not exist "build\installer" mkdir build\installer

REM Copy installer script
copy "MikoIDE-Installer.nsi" "build\installer\"

REM Copy application files to build directory
echo Copying application files...
xcopy "MikoIDE.exe" "build\installer\" /Y
xcopy "*.dll" "build\installer\" /Y
xcopy "*.pak" "build\installer\" /Y
xcopy "*.dat" "build\installer\" /Y
xcopy "*.lib" "build\installer\" /Y
xcopy "*.pdb" "build\installer\" /Y
xcopy "*.bin" "build\installer\" /Y
xcopy "*.json" "build\installer\" /Y

REM Copy directories
xcopy "assets" "build\installer\assets\" /E /I /Y
xcopy "locales" "build\installer\locales\" /E /I /Y

REM Create a simple license file if it doesn't exist
if not exist "build\installer\License.txt" (
    echo MikoIDE License Agreement > "build\installer\License.txt"
    echo. >> "build\installer\License.txt"
    echo This software is provided "as is" without warranty. >> "build\installer\License.txt"
    echo By installing this software, you agree to use it responsibly. >> "build\installer\License.txt"
)

REM Build the installer
echo Building installer...
cd build\installer
makensis MikoIDE-Installer.nsi

if %ERRORLEVEL% EQU 0 (
    echo.
    echo SUCCESS: Installer built successfully!
    echo Output: build\installer\MikoIDE-Setup.exe
    echo.
    pause
) else (
    echo.
    echo ERROR: Failed to build installer
    pause
    exit /b 1
)

cd ..\..
echo Done!
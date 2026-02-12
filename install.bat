@echo off
REM Exit on error (similar to set -e)
setlocal enabledelayedexpansion

REM Compile the program

REM Try to initialize Visual Studio build environment if not already initialized.
REM Adjust the path below to match your VS edition/version if necessary.
if not defined VisualStudioVersion (
    if exist "%ProgramFiles(x86)%\Microsoft Visual Studio\2022\Community\Common7\Tools\VsDevCmd.bat" (
        call "%ProgramFiles(x86)%\Microsoft Visual Studio\2022\Community\Common7\Tools\VsDevCmd.bat"
    ) else if exist "%ProgramFiles(x86)%\Microsoft Visual Studio\2019\Community\Common7\Tools\VsDevCmd.bat" (
        call "%ProgramFiles(x86)%\Microsoft Visual Studio\2019\Community\Common7\Tools\VsDevCmd.bat"
    ) else (
        echo Could not find VsDevCmd.bat. Please run this from a "Developer Command Prompt for VS" or update the path in this script.
        endlocal
        exit /b 1
    )
)

REM Now compile
cl /nologo boc_init.c /Fe:"boc_init.exe"
if errorlevel 1 (
    echo Build failed.
    endlocal
    exit /b %errorlevel%
)

REM Create directory if it doesn't exist
if not exist C:\boc (
    mkdir C:\boc
    if errorlevel 1 exit /b %errorlevel%
)

REM Copy header file
copy boc.h C:\boc\
if errorlevel 1 exit /b %errorlevel%

REM Move executable
move boc_init.exe C:\boc\
if errorlevel 1 exit /b %errorlevel%

endlocal
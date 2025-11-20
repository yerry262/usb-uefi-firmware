@echo off
REM filepath: c:\Users\jerry\Downloads\USB proj\usb-uefi-firmware\tools\build_scripts\build.bat
REM USB UEFI Firmware Build Script for Windows
REM Supports both development and production builds

setlocal enabledelayedexpansion

REM Configuration
set PROJECT_NAME=USBUefiFirmware
set SCRIPT_DIR=%~dp0
set PROJECT_ROOT=%SCRIPT_DIR%..\..
set BUILD_DIR=%PROJECT_ROOT%\build

REM Default values
set TARGET=DEBUG
set COMMAND=build
set VERBOSE=false
set USB_DRIVE=

REM Parse arguments
:parse_args
if "%~1"=="" goto end_parse
if "%~1"=="build" (
    set COMMAND=build
    if "%~2"=="DEBUG" set TARGET=DEBUG & shift
    if "%~2"=="RELEASE" set TARGET=RELEASE & shift
    shift
    goto parse_args
)
if "%~1"=="clean" set COMMAND=clean & shift & goto parse_args
if "%~1"=="package" set COMMAND=package & shift & goto parse_args
if "%~1"=="test" set COMMAND=test & shift & goto parse_args
if "%~1"=="install" (
    set COMMAND=install
    set USB_DRIVE=%~2
    shift & shift
    goto parse_args
)
if "%~1"=="check-hw" set COMMAND=check-hw & shift & goto parse_args
if "%~1"=="edk2-setup" set COMMAND=edk2-setup & shift & goto parse_args
if "%~1"=="all" set COMMAND=all & shift & goto parse_args
if "%~1"=="-h" goto show_usage
if "%~1"=="--help" goto show_usage
if "%~1"=="-v" set VERBOSE=true & shift & goto parse_args
if "%~1"=="--verbose" set VERBOSE=true & shift & goto parse_args
echo ERROR: Unknown option: %~1
goto show_usage
:end_parse

echo USB UEFI Firmware Build Script v1.0
echo Project: %PROJECT_NAME%
echo Target: %TARGET%
echo Command: %COMMAND%
echo.

REM Execute command
if "%COMMAND%"=="build" goto build
if "%COMMAND%"=="clean" goto clean
if "%COMMAND%"=="package" goto package
if "%COMMAND%"=="test" goto test
if "%COMMAND%"=="install" goto install
if "%COMMAND%"=="check-hw" goto check_hw
if "%COMMAND%"=="edk2-setup" goto edk2_setup
if "%COMMAND%"=="all" goto all
goto show_usage

:build
echo [STEP] Building %PROJECT_NAME% (%TARGET%)...
call :check_prerequisites
if errorlevel 1 exit /b 1

cd /d "%PROJECT_ROOT%"
if not exist "%BUILD_DIR%" mkdir "%BUILD_DIR%"
if not exist "%BUILD_DIR%\obj" mkdir "%BUILD_DIR%\obj"
if not exist "%BUILD_DIR%\bin" mkdir "%BUILD_DIR%\bin"

make TARGET=%TARGET% TARGET_ARCH=X64 all
if errorlevel 1 (
    echo [ERROR] Build failed
    exit /b 1
)
echo [INFO] Build completed successfully
goto end

:clean
echo [STEP] Cleaning build directory...
if exist "%BUILD_DIR%" (
    rmdir /s /q "%BUILD_DIR%"
    echo [INFO] Build directory cleaned
)
goto end

:package
echo [STEP] Creating package...
call :build
if errorlevel 1 exit /b 1

set PACKAGE_DIR=%BUILD_DIR%\%PROJECT_NAME%-1.0.0
if not exist "%PACKAGE_DIR%" mkdir "%PACKAGE_DIR%"
if not exist "%PACKAGE_DIR%\EFI\BOOT" mkdir "%PACKAGE_DIR%\EFI\BOOT"

copy "%BUILD_DIR%\bin\%PROJECT_NAME%.efi" "%PACKAGE_DIR%\BOOTX64.EFI"
copy "%PACKAGE_DIR%\BOOTX64.EFI" "%PACKAGE_DIR%\EFI\BOOT\"
copy "%PROJECT_ROOT%\README.md" "%PACKAGE_DIR%\"

echo [INFO] Package created in %PACKAGE_DIR%
goto end

:test
echo [STEP] Testing with QEMU...
call :build
if errorlevel 1 exit /b 1

if not exist "%BUILD_DIR%\OVMF.fd" (
    echo [INFO] Downloading OVMF firmware...
    powershell -Command "(New-Object System.Net.WebClient).DownloadFile('https://github.com/clearlinux/common/raw/master/OVMF.fd', '%BUILD_DIR%\OVMF.fd')"
)

set TEST_DIR=%BUILD_DIR%\qemu_test
if not exist "%TEST_DIR%\EFI\BOOT" mkdir "%TEST_DIR%\EFI\BOOT"
copy "%BUILD_DIR%\bin\%PROJECT_NAME%.efi" "%TEST_DIR%\EFI\BOOT\BOOTX64.EFI"

echo [INFO] Starting QEMU...
qemu-system-x86_64 -pflash "%BUILD_DIR%\OVMF.fd" -hda "fat:rw:%TEST_DIR%" -net none -serial stdio -m 512
goto end

:install
if "%USB_DRIVE%"=="" (
    echo [ERROR] USB drive not specified
    echo Usage: %0 install E:
    exit /b 1
)
echo [STEP] Installing to USB drive %USB_DRIVE%...

if not exist "%USB_DRIVE%\EFI\BOOT" mkdir "%USB_DRIVE%\EFI\BOOT"
if exist "%BUILD_DIR%\bin\%PROJECT_NAME%.efi" (
    copy "%BUILD_DIR%\bin\%PROJECT_NAME%.efi" "%USB_DRIVE%\EFI\BOOT\BOOTX64.EFI"
    copy "%PROJECT_ROOT%\README.md" "%USB_DRIVE%\"
    echo [INFO] Installation completed
) else (
    echo [ERROR] Build output not found. Run 'build' first.
    exit /b 1
)
goto end

:check_hw
echo [STEP] Checking hardware compatibility...
echo [INFO] Windows hardware detection not fully implemented
echo [INFO] Please verify manually:
echo   - AMD AM5 motherboard
echo   - Ryzen 7000/9000 series CPU  
echo   - UEFI firmware with USB boot support
echo   - USB 3.0+ controller
goto end

:edk2_setup
echo [STEP] Creating EDK2 package files...
echo # USB UEFI Firmware Package > "%PROJECT_ROOT%\%PROJECT_NAME%.dsc"
echo [Defines] >> "%PROJECT_ROOT%\%PROJECT_NAME%.dsc"
echo   PLATFORM_NAME = %PROJECT_NAME% >> "%PROJECT_ROOT%\%PROJECT_NAME%.dsc"
echo   PLATFORM_GUID = 12345678-1234-1234-1234-123456789abc >> "%PROJECT_ROOT%\%PROJECT_NAME%.dsc"
echo   PLATFORM_VERSION = 1.0 >> "%PROJECT_ROOT%\%PROJECT_NAME%.dsc"
echo   DSC_SPECIFICATION = 0x00010005 >> "%PROJECT_ROOT%\%PROJECT_NAME%.dsc"
echo   OUTPUT_DIRECTORY = Build/%PROJECT_NAME% >> "%PROJECT_ROOT%\%PROJECT_NAME%.dsc"
echo   SUPPORTED_ARCHITECTURES = X64 >> "%PROJECT_ROOT%\%PROJECT_NAME%.dsc"
echo   BUILD_TARGETS = DEBUG^|RELEASE >> "%PROJECT_ROOT%\%PROJECT_NAME%.dsc"
echo [INFO] EDK2 package created
goto end

:all
call :clean
call :build
call :package
goto end

:check_prerequisites
echo [STEP] Checking prerequisites...

REM Check for EDK2
if "%EDK2_PATH%"=="" (
    if exist "C:\edk2" (
        set EDK2_PATH=C:\edk2
    ) else (
        echo [ERROR] EDK2_PATH not set and C:\edk2 not found
        exit /b 1
    )
)

if not exist "%EDK2_PATH%" (
    echo [ERROR] EDK2 not found at %EDK2_PATH%
    exit /b 1
)

REM Check for Visual Studio
if "%VS_PATH%"=="" (
    set VS_PATH=C:\Program Files ^(x86^)\Microsoft Visual Studio\2019\BuildTools
)

REM Check for required tools
where cl.exe >nul 2>&1
if errorlevel 1 (
    echo [ERROR] Visual Studio compiler not found
    exit /b 1
)

where nmake.exe >nul 2>&1
if errorlevel 1 (
    echo [ERROR] NMAKE not found
    exit /b 1
)

echo [INFO] Prerequisites check passed
exit /b 0

:show_usage
echo Usage: %0 [OPTIONS] [COMMAND]
echo.
echo Commands:
echo   build [DEBUG^|RELEASE]  - Build the project (default: DEBUG)
echo   clean                  - Clean build directory
echo   package               - Create release package
echo   test                  - Test with QEMU
echo   install DRIVE_LETTER  - Install to USB drive (e.g., E:)
echo   check-hw              - Check hardware compatibility
echo   edk2-setup            - Create EDK2 package files
echo   all                   - Clean, build, and package
echo.
echo Options:
echo   -h, --help            - Show this help
echo   -v, --verbose         - Verbose output
echo.
echo Environment variables:
echo   EDK2_PATH             - Path to EDK2 installation
echo   VS_PATH               - Path to Visual Studio
echo.
echo Examples:
echo   %0 build DEBUG        - Build debug version
echo   %0 install E:         - Install to E: drive
echo   %0 test               - Build and test with QEMU
goto end

:end
echo.
echo Script completed.
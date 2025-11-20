#
# USB UEFI Firmware Interface - Makefile
# Supports both Windows (Visual Studio) and Linux (GCC) builds
#

# Project Information
PROJECT_NAME = USBUefiFirmware
VERSION_MAJOR = 1
VERSION_MINOR = 0
VERSION_PATCH = 0
VERSION_BUILD = 1

# Detect OS
ifeq ($(OS),Windows_NT)
    DETECTED_OS := Windows
    SHELL := cmd.exe
    PATH_SEP := \\
    EXE_EXT := .exe
    LIB_EXT := .lib
    OBJ_EXT := .obj
    MKDIR := mkdir
    RMDIR := rmdir /s /q
    RM := del /f /q
else
    DETECTED_OS := $(shell uname -s)
    PATH_SEP := /
    EXE_EXT :=
    LIB_EXT := .a
    OBJ_EXT := .o
    MKDIR := mkdir -p
    RMDIR := rm -rf
    RM := rm -f
endif

# Directories
ROOT_DIR := .
SRC_DIR := $(ROOT_DIR)$(PATH_SEP)src
INCLUDE_DIR := $(ROOT_DIR)$(PATH_SEP)include
BUILD_DIR := $(ROOT_DIR)$(PATH_SEP)build
OBJ_DIR := $(BUILD_DIR)$(PATH_SEP)obj
BIN_DIR := $(BUILD_DIR)$(PATH_SEP)bin
TOOLS_DIR := $(ROOT_DIR)$(PATH_SEP)tools

# EDK2 Configuration
ifeq ($(DETECTED_OS),Windows)
    EDK2_PATH ?= C:$(PATH_SEP)edk2
    VS_PATH ?= C:$(PATH_SEP)Program Files (x86)$(PATH_SEP)Microsoft Visual Studio$(PATH_SEP)2019$(PATH_SEP)BuildTools
    NASM_PATH ?= C:$(PATH_SEP)nasm
else
    EDK2_PATH ?= $(HOME)/edk2
    NASM_PATH ?= /usr/bin
endif

WORKSPACE := $(EDK2_PATH)
PACKAGES_PATH := $(EDK2_PATH):$(ROOT_DIR)
TARGET := DEBUG
TARGET_ARCH := X64
TOOL_CHAIN_TAG := VS2019

ifeq ($(DETECTED_OS),Linux)
    TOOL_CHAIN_TAG := GCC5
endif

# Compiler Settings
ifeq ($(DETECTED_OS),Windows)
    CC := cl.exe
    LINK := link.exe
    CFLAGS := /c /Zi /Od /W4 /WX- /wd4201 /wd4214 /wd4996
    CFLAGS += /D"_CRT_SECURE_NO_WARNINGS" /D"WIN32" /D"_WINDOWS" /D"DEBUG"
    INCLUDES := /I"$(EDK2_PATH)$(PATH_SEP)MdePkg$(PATH_SEP)Include"
    INCLUDES += /I"$(EDK2_PATH)$(PATH_SEP)MdePkg$(PATH_SEP)Include$(PATH_SEP)X64"
    INCLUDES += /I"$(INCLUDE_DIR)"
    LINKFLAGS := /DEBUG /SUBSYSTEM:EFI_APPLICATION /ENTRY:UefiMain
else
    CC := gcc
    LINK := ld
    CFLAGS := -c -g -O0 -Wall -Wextra -Werror
    CFLAGS += -fno-stack-protector -fno-strict-aliasing -fshort-wchar
    CFLAGS += -mno-red-zone -maccumulate-outgoing-args
    INCLUDES := -I$(EDK2_PATH)/MdePkg/Include
    INCLUDES += -I$(EDK2_PATH)/MdePkg/Include/X64
    INCLUDES += -I$(INCLUDE_DIR)
    LINKFLAGS := -nostdlib -shared -Bsymbolic -pie --gc-sections
endif

# Source Files
MAIN_SOURCES := $(SRC_DIR)$(PATH_SEP)main.c

USB_SOURCES := $(SRC_DIR)$(PATH_SEP)usb$(PATH_SEP)usb_driver.c

UEFI_SOURCES := $(SRC_DIR)$(PATH_SEP)uefi$(PATH_SEP)uefi_interface.c
UEFI_SOURCES += $(SRC_DIR)$(PATH_SEP)uefi$(PATH_SEP)boot_services.c

FIRMWARE_SOURCES := $(SRC_DIR)$(PATH_SEP)firmware$(PATH_SEP)firmware_loader.c
FIRMWARE_SOURCES += $(SRC_DIR)$(PATH_SEP)firmware$(PATH_SEP)flash_manager.c

DEBUG_SOURCES := $(SRC_DIR)$(PATH_SEP)debug_utils.c

ALL_SOURCES := $(MAIN_SOURCES) $(USB_SOURCES) $(UEFI_SOURCES) $(FIRMWARE_SOURCES) $(DEBUG_SOURCES)

# Object Files
MAIN_OBJECTS := $(patsubst $(SRC_DIR)$(PATH_SEP)%.c,$(OBJ_DIR)$(PATH_SEP)%$(OBJ_EXT),$(MAIN_SOURCES))
USB_OBJECTS := $(patsubst $(SRC_DIR)$(PATH_SEP)%.c,$(OBJ_DIR)$(PATH_SEP)%$(OBJ_EXT),$(USB_SOURCES))
UEFI_OBJECTS := $(patsubst $(SRC_DIR)$(PATH_SEP)%.c,$(OBJ_DIR)$(PATH_SEP)%$(OBJ_EXT),$(UEFI_SOURCES))
FIRMWARE_OBJECTS := $(patsubst $(SRC_DIR)$(PATH_SEP)%.c,$(OBJ_DIR)$(PATH_SEP)%$(OBJ_EXT),$(FIRMWARE_SOURCES))
DEBUG_OBJECTS := $(patsubst $(SRC_DIR)$(PATH_SEP)%.c,$(OBJ_DIR)$(PATH_SEP)%$(OBJ_EXT),$(DEBUG_SOURCES))

ALL_OBJECTS := $(MAIN_OBJECTS) $(USB_OBJECTS) $(UEFI_OBJECTS) $(FIRMWARE_OBJECTS) $(DEBUG_OBJECTS)

# Target Files
TARGET_EFI := $(BIN_DIR)$(PATH_SEP)$(PROJECT_NAME).efi
TARGET_DEBUG := $(BIN_DIR)$(PATH_SEP)$(PROJECT_NAME).debug

# Default target
.PHONY: all
all: setup build

# Setup build environment
.PHONY: setup
setup:
	@echo Setting up build environment for $(DETECTED_OS)...
ifeq ($(DETECTED_OS),Windows)
		@if not exist "$(BUILD_DIR)" $(MKDIR) "$(BUILD_DIR)"
		@if not exist "$(OBJ_DIR)" $(MKDIR) "$(OBJ_DIR)"
		@if not exist "$(OBJ_DIR)$(PATH_SEP)usb" $(MKDIR) "$(OBJ_DIR)$(PATH_SEP)usb"
		@if not exist "$(OBJ_DIR)$(PATH_SEP)uefi" $(MKDIR) "$(OBJ_DIR)$(PATH_SEP)uefi"
		@if not exist "$(OBJ_DIR)$(PATH_SEP)firmware" $(MKDIR) "$(OBJ_DIR)$(PATH_SEP)firmware"
		@if not exist "$(BIN_DIR)" $(MKDIR) "$(BIN_DIR)"
	else
		@$(MKDIR) $(BUILD_DIR)
		@$(MKDIR) $(OBJ_DIR)
		@$(MKDIR) $(OBJ_DIR)/usb
		@$(MKDIR) $(OBJ_DIR)/uefi
		@$(MKDIR) $(OBJ_DIR)/firmware
		@$(MKDIR) $(BIN_DIR)
	endif
	@echo Build directories created successfully

# Build target
.PHONY: build
build: $(TARGET_EFI)
	@echo Build completed successfully

# Link EFI application
$(TARGET_EFI): $(ALL_OBJECTS)
	@echo Linking EFI application...
ifeq ($(DETECTED_OS),Windows)
		$(LINK) $(LINKFLAGS) /OUT:"$@" $(ALL_OBJECTS)
	else
		$(LINK) $(LINKFLAGS) -o $@ $(ALL_OBJECTS) -L$(EDK2_PATH)/BaseTools/Bin/Linux-x86_64 -lCommon
	endif
	@echo EFI application created: $@

# Compile main source
$(OBJ_DIR)$(PATH_SEP)main$(OBJ_EXT): $(SRC_DIR)$(PATH_SEP)main.c
	@echo Compiling main.c...
	$(CC) $(CFLAGS) $(INCLUDES) /Fo"$@" "$<"

# Compile USB sources
$(OBJ_DIR)$(PATH_SEP)usb$(PATH_SEP)usb_driver$(OBJ_EXT): $(SRC_DIR)$(PATH_SEP)usb$(PATH_SEP)usb_driver.c
	@echo Compiling usb_driver.c...
	$(CC) $(CFLAGS) $(INCLUDES) /Fo"$@" "$<"

# Compile UEFI sources
$(OBJ_DIR)$(PATH_SEP)uefi$(PATH_SEP)uefi_interface$(OBJ_EXT): $(SRC_DIR)$(PATH_SEP)uefi$(PATH_SEP)uefi_interface.c
	@echo Compiling uefi_interface.c...
	$(CC) $(CFLAGS) $(INCLUDES) /Fo"$@" "$<"

$(OBJ_DIR)$(PATH_SEP)uefi$(PATH_SEP)boot_services$(OBJ_EXT): $(SRC_DIR)$(PATH_SEP)uefi$(PATH_SEP)boot_services.c
	@echo Compiling boot_services.c...
	$(CC) $(CFLAGS) $(INCLUDES) /Fo"$@" "$<"

# Compile firmware sources
$(OBJ_DIR)$(PATH_SEP)firmware$(PATH_SEP)firmware_loader$(OBJ_EXT): $(SRC_DIR)$(PATH_SEP)firmware$(PATH_SEP)firmware_loader.c
	@echo Compiling firmware_loader.c...
	$(CC) $(CFLAGS) $(INCLUDES) /Fo"$@" "$<"

$(OBJ_DIR)$(PATH_SEP)firmware$(PATH_SEP)flash_manager$(OBJ_EXT): $(SRC_DIR)$(PATH_SEP)firmware$(PATH_SEP)flash_manager.c
	@echo Compiling flash_manager.c...
	$(CC) $(CFLAGS) $(INCLUDES) /Fo"$@" "$<"

# Compile debug utilities
$(OBJ_DIR)$(PATH_SEP)debug_utils$(OBJ_EXT): $(SRC_DIR)$(PATH_SEP)debug_utils.c
	@echo Compiling debug_utils.c...
	$(CC) $(CFLAGS) $(INCLUDES) /Fo"$@" "$<"

# EDK2 Build (Alternative build method)
.PHONY: edk2-build
edk2-build: edk2-setup
	@echo Building with EDK2 build system...
ifeq ($(DETECTED_OS),Windows)
		cd /d "$(EDK2_PATH)" && edksetup.bat && build -p $(PROJECT_NAME)/$(PROJECT_NAME).dsc -a $(TARGET_ARCH) -t $(TOOL_CHAIN_TAG) -b $(TARGET)
	else
		cd $(EDK2_PATH) && . edksetup.sh && build -p $(PROJECT_NAME)/$(PROJECT_NAME).dsc -a $(TARGET_ARCH) -t $(TOOL_CHAIN_TAG) -b $(TARGET)
	endif

.PHONY: edk2-setup
edk2-setup:
	@echo Setting up EDK2 environment...
	@echo Creating package description file...
	@echo "# $(PROJECT_NAME).dsc" > $(ROOT_DIR)$(PATH_SEP)$(PROJECT_NAME).dsc
	@echo "[Defines]" >> $(ROOT_DIR)$(PATH_SEP)$(PROJECT_NAME).dsc
	@echo "  PLATFORM_NAME = $(PROJECT_NAME)" >> $(ROOT_DIR)$(PATH_SEP)$(PROJECT_NAME).dsc
	@echo "  PLATFORM_GUID = 12345678-1234-1234-1234-123456789abc" >> $(ROOT_DIR)$(PATH_SEP)$(PROJECT_NAME).dsc
	@echo "  PLATFORM_VERSION = $(VERSION_MAJOR).$(VERSION_MINOR)" >> $(ROOT_DIR)$(PATH_SEP)$(PROJECT_NAME).dsc
	@echo "  DSC_SPECIFICATION = 0x00010005" >> $(ROOT_DIR)$(PATH_SEP)$(PROJECT_NAME).dsc
	@echo "  OUTPUT_DIRECTORY = Build/$(PROJECT_NAME)" >> $(ROOT_DIR)$(PATH_SEP)$(PROJECT_NAME).dsc
	@echo "  SUPPORTED_ARCHITECTURES = X64" >> $(ROOT_DIR)$(PATH_SEP)$(PROJECT_NAME).dsc
	@echo "  BUILD_TARGETS = DEBUG|RELEASE" >> $(ROOT_DIR)$(PATH_SEP)$(PROJECT_NAME).dsc

# Clean targets
.PHONY: clean
clean:
	@echo Cleaning build files...
ifeq ($(DETECTED_OS),Windows)
		@if exist "$(BUILD_DIR)" $(RMDIR) "$(BUILD_DIR)"
	else
		@$(RMDIR) $(BUILD_DIR)
	endif
	@echo Clean completed

.PHONY: distclean
distclean: clean
	@echo Performing distribution clean...
	@$(RM) *.log *.tmp
	@echo Distribution clean completed

# Install target (copy to USB/test location)
.PHONY: install
install: $(TARGET_EFI)
	@echo Installing EFI application...
ifeq ($(DETECTED_OS),Windows)
		@if defined USB_DRIVE (copy /Y "$(TARGET_EFI)" "%USB_DRIVE%\EFI\BOOT\BOOTX64.EFI") else (echo USB_DRIVE not defined)
	else
		@if [ -n "$(USB_MOUNT)" ]; then cp $(TARGET_EFI) $(USB_MOUNT)/EFI/BOOT/BOOTX64.EFI; else echo "USB_MOUNT not defined"; fi
	endif
	@echo Installation completed

# Test targets
.PHONY: test
test: build
	@echo Running tests...
	@echo No tests defined yet

.PHONY: qemu-test
qemu-test: build
	@echo Testing with QEMU...
	qemu-system-x86_64 -pflash OVMF.fd -hda fat:rw:$(BIN_DIR) -net none -serial stdio

# Debug targets
.PHONY: debug
debug: build
	@echo Starting debug session...
	@echo Debug target not implemented yet

# Version information
.PHONY: version
version:
	@echo $(PROJECT_NAME) Version $(VERSION_MAJOR).$(VERSION_MINOR).$(VERSION_PATCH).$(VERSION_BUILD)
	@echo Build for $(DETECTED_OS) $(TARGET_ARCH)
	@echo Toolchain: $(TOOL_CHAIN_TAG)

# Help
.PHONY: help
help:
	@echo USB UEFI Firmware Interface - Build System
	@echo ==========================================
	@echo Available targets:
	@echo   all         - Setup and build (default)
	@echo   setup       - Create build directories
	@echo   build       - Compile and link
	@echo   edk2-build  - Build using EDK2 system
	@echo   clean       - Remove build files
	@echo   distclean   - Full clean including logs
	@echo   install     - Install to USB drive
	@echo   test        - Run tests
	@echo   qemu-test   - Test with QEMU emulator
	@echo   debug       - Start debug session
	@echo   version     - Show version information
	@echo   help        - Show this help
	@echo
	@echo Environment variables:
	@echo   EDK2_PATH   - Path to EDK2 installation
	@echo   USB_DRIVE   - USB drive letter (Windows)
	@echo   USB_MOUNT   - USB mount point (Linux)
	@echo   TARGET      - DEBUG or RELEASE
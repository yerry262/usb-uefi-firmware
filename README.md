# USB UEFI Firmware Interface

A comprehensive UEFI firmware debugging interface designed for AMD AM5/Ryzen 7000 and 9000 series processors. This project provides a bootable USB-based debugging environment for firmware development, system debugging, and hardware analysis.

## Table of Contents

- [Overview](#overview)
- [Features](#features)
- [System Requirements](#system-requirements)
- [Installation](#installation)
- [Building from Source](#building-from-source)
- [Usage](#usage)
- [Architecture](#architecture)
- [Configuration](#configuration)
- [Troubleshooting](#troubleshooting)
- [Contributing](#contributing)
- [License](#license)

## Overview

This project implements a UEFI application that runs directly from USB storage, providing a low-level debugging interface for AMD AM5 desktop systems. It's specifically designed for PhD research and professional firmware development work.

### Key Components

- **USB Driver Interface**: Direct USB hardware communication
- **UEFI System Integration**: Full UEFI boot services utilization  
- **Firmware Management**: Flash memory operations and firmware loading
- **Debug Utilities**: Comprehensive logging and debugging tools
- **AMD-Specific Support**: Optimized for Ryzen 7000/9000 series

## Features

### Hardware Support
- ✅ AMD AM5 Socket (LGA 1718)
- ✅ AMD Ryzen 7000 Series (Zen 4)
- ✅ AMD Ryzen 9000 Series (Zen 5)
- ✅ USB 3.0/3.1/3.2 Controllers (xHCI)
- ✅ Legacy USB 2.0 Support (EHCI)
- ✅ DDR5 Memory Systems
- ✅ PCIe 5.0 Support

### Software Features
- 🔧 Interactive Command Interface
- 🔧 USB Device Detection and Enumeration
- 🔧 Memory Inspection and Modification
- 🔧 Flash Memory Operations
- 🔧 System Information Gathering
- 🔧 Debug Logging with Multiple Levels
- 🔧 Firmware Validation and Loading
- 🔧 Cross-Platform Build System

### Debug Capabilities
- Real-time USB device monitoring
- Memory map analysis
- Flash memory read/write/erase operations
- System configuration inspection
- Hardware register access
- Performance measurement tools

## System Requirements

### Target Hardware
- **Motherboard**: AMD AM5 chipset (B650, X670, X670E)
- **CPU**: AMD Ryzen 7000 or 9000 series
- **Memory**: DDR5 (minimum 8GB recommended)
- **Storage**: USB 3.0+ drive (minimum 1GB)
- **BIOS**: UEFI firmware with USB boot support

### Development Environment

#### Windows
- Visual Studio 2019 or newer
- Windows 10/11 SDK
- EDK2 (UEFI Development Kit)
- NASM Assembler
- Git for Windows

#### Linux
- GCC 5.0 or newer
- Make utility
- EDK2 (UEFI Development Kit)
- NASM Assembler
- Git

## Installation

### Quick Start (Pre-built Binary)

1. **Download Release**
   ```bash
   # Download the latest release from GitHub
   wget https://github.com/your-repo/usb-uefi-firmware/releases/latest/USBUefiFirmware.efi
   ```

2. **Prepare USB Drive**
   ```bash
   # Format USB drive as FAT32
   # Create EFI directory structure
   mkdir -p /mnt/usb/EFI/BOOT
   cp USBUefiFirmware.efi /mnt/usb/EFI/BOOT/BOOTX64.EFI
   ```

3. **Configure BIOS**
   - Enter BIOS/UEFI setup
   - Disable Secure Boot (if enabled)
   - Set USB drive as first boot priority
   - Save and restart

### Manual USB Setup

For manual USB preparation:

```bash
# Linux
sudo mkfs.fat -F 32 /dev/sdX1  # Replace X with your USB drive
sudo mkdir -p /mnt/usb
sudo mount /dev/sdX1 /mnt/usb
sudo mkdir -p /mnt/usb/EFI/BOOT
sudo cp build/bin/USBUefiFirmware.efi /mnt/usb/EFI/BOOT/BOOTX64.EFI
sudo umount /mnt/usb
```

```cmd
# Windows
format X: /FS:FAT32 /Q  # Replace X with your USB drive
mkdir X:\EFI\BOOT
copy build\bin\USBUefiFirmware.efi X:\EFI\BOOT\BOOTX64.EFI
```

## Getting Started (quick)

If you've just cloned the repository and want to build and test on Windows, the following quick steps will get you started. These assume you have set up the EDK2 environment and the required toolchain as described in the `Building from Source` section.

Open a PowerShell window (Run as Administrator when required):

```powershell
cd 'C:\Users\jerry\Downloads\USB proj\usb-uefi-firmware'
# Use your project's build steps; common example:
make
# Or run the project's Windows build script in an appropriate shell if needed.
# e.g., run the helper script if you prefer (may require WSL/MSYS):
# .\tools\build_scripts\build-windows.sh
```

Notes:
- Ensure `EDK2` include paths and tools are on your PATH and that `edksetup.bat` (Windows) or `. edksetup.sh` (Linux) has been run.
- If your editor reports missing UEFI headers (e.g., `Uefi.h`), configure the IDE includePath to point to your EDK2 include directories.
- For first-time builds, prefer a DEBUG build to aid troubleshooting: `make TARGET=DEBUG`.


## Building from Source

### Prerequisites Setup

#### Windows Setup
```cmd
# Install EDK2
git clone https://github.com/tianocore/edk2.git C:\edk2
cd C:\edk2
git submodule update --init
edksetup.bat

# Install NASM
# Download from https://www.nasm.us/
# Add to PATH environment variable
```

#### Linux Setup
```bash
# Install dependencies
sudo apt update
sudo apt install build-essential git nasm uuid-dev

# Install EDK2
git clone https://github.com/tianocore/edk2.git ~/edk2
cd ~/edk2
git submodule update --init
. edksetup.sh
```

### Building the Project

#### Standard Build
```bash
# Clone the repository
git clone https://github.com/your-repo/usb-uefi-firmware.git
cd usb-uefi-firmware

# Build using Make
make all

# Or build for specific target
make TARGET=RELEASE build
```

#### EDK2 Integration Build
```bash
# Setup EDK2 environment
export EDK2_PATH=~/edk2  # Linux
set EDK2_PATH=C:\edk2    # Windows

# Build with EDK2
make edk2-build
```

### Build Options

| Variable | Description | Default |
|----------|-------------|---------|
| `EDK2_PATH` | Path to EDK2 installation | `~/edk2` (Linux), `C:\edk2` (Windows) |
| `TARGET` | Build target (DEBUG/RELEASE) | `DEBUG` |
| `TARGET_ARCH` | Architecture (X64/IA32) | `X64` |
| `TOOL_CHAIN_TAG` | Compiler toolchain | `GCC5` (Linux), `VS2019` (Windows) |

### Installation to USB
```bash
# Set USB drive path
export USB_MOUNT=/mnt/usb    # Linux
set USB_DRIVE=F:             # Windows

# Install to USB
make install
```

## Usage

### Basic Operation

1. **Boot from USB**
   - Insert prepared USB drive
   - Power on system
   - Press F8/F12/Del to access boot menu
   - Select USB drive

2. **Command Interface**
   ```
   USB UEFI Firmware Interface v1.0.0
   ===================================
   Press any key for commands...
   
   Commands available:
   h/H/?  - Show help
   u/U    - USB device status  
   f/F    - Firmware information
   s/S    - System information
   d/D    - Debug level control
   r/R    - Reset USB subsystem
   q/Q    - Quit application
   ```

### Advanced Usage

#### USB Device Analysis
```
Command: u
USB Device Status:
  Initialized: YES
  Devices found: 3
  Device 0: VID=0x1234, PID=0x5678
  Device 1: VID=0xABCD, PID=0xEF01
  Device 2: VID=0x8765, PID=0x4321
```

#### System Information
```
Command: s
System Information:
  UEFI Version: 2.80
  Firmware Vendor: American Megatrends
  CPU Vendor: AuthenticAMD
  CPU Family: AMD Ryzen
  Total Memory: 32768 MB
  Available Memory: 28672 MB
```

#### Debug Level Control
```
Command: d
Debug Level: 0x0000000F, Categories: 0x000001F0
```

## Architecture

### Project Structure
```
usb-uefi-firmware/
├── src/
│   ├── main.c                 # Application entry point
│   ├── usb/
│   │   ├── usb_driver.c       # USB hardware interface
│   │   ├── usb_driver.h
│   │   ├── usb_protocol.h     # USB protocol definitions
│   ├── uefi/
│   │   ├── uefi_interface.c   # UEFI system interface
│   │   ├── uefi_interface.h
│   │   └── boot_services.h    # UEFI boot services
│   └── firmware/
│       ├── firmware_loader.c  # Firmware management
│       ├── firmware_loader.h
│       └── flash_manager.c    # Flash operations
├── include/
│   ├── common.h               # Common definitions
│   ├── config.h               # Configuration constants
│   └── debug_utils.h          # Debug utilities
├── tools/
│   ├── build_scripts/
│   └── flash_tools/
├── docs/
├── Makefile
└── README.md
```

### Component Interaction
```
┌─────────────┐    ┌─────────────┐    ┌─────────────┐
│   main.c    │────│ UEFI Interface │────│ Boot Services │
└─────────────┘    └─────────────┘    └─────────────┘
       │                   │                   │
       ├─────────────────────────────────────────┐
       │                                         │
┌─────────────┐    ┌─────────────┐    ┌─────────────┐
│ USB Driver  │    │ Firmware    │    │ Debug Utils │
│             │    │ Loader      │    │             │
└─────────────┘    └─────────────┘    └─────────────┘
       │                   │                   │
┌─────────────┐    ┌─────────────┐              │
│ USB Protocol│    │ Flash Mgr   │              │
└─────────────┘    └─────────────┘              │
                                                 │
                   ┌─────────────┐              │
                   │ Hardware    │──────────────┘
                   │ Layer       │
                   └─────────────┘
```

## Configuration

### Build Configuration

Edit `include/config.h` to customize build options:

```c
// USB Driver Configuration
#define MAX_USB_DEVICES             32
#define USB_TRANSFER_TIMEOUT        5000

// AMD Ryzen/AM5 Specific
#define SUPPORT_AMD_RYZEN_7000      TRUE
#define SUPPORT_AMD_RYZEN_9000      TRUE
#define ENABLE_AMD_USB_QUIRKS       TRUE

// Debug Configuration
#ifdef DEBUG_BUILD
    #define ENABLE_USB_DEBUG        TRUE
    #define ENABLE_VERBOSE_LOGGING  TRUE
#endif

// Hardware Specific Settings
#define AM5_USB_BASE_ADDRESS        0xFED80000
#define MAX_USB_PORTS               10
```

### Runtime Configuration

Debug levels can be controlled at runtime:

```c
// Debug Categories
#define DEBUG_CAT_USB               0x00000010
#define DEBUG_CAT_FIRMWARE          0x00000020
#define DEBUG_CAT_UEFI              0x00000040
#define DEBUG_CAT_BOOT              0x00000080
```

## Troubleshooting

### Common Issues

#### Boot Issues
**Problem**: System doesn't boot from USB
**Solutions**:
- Verify USB drive is formatted as FAT32
- Ensure EFI file is named `BOOTX64.EFI`
- Check BIOS boot order settings
- Disable Secure Boot if enabled

**Problem**: Application crashes on startup
**Solutions**:
- Check debug output via serial console
- Verify UEFI firmware compatibility
- Try building in DEBUG mode
- Check memory requirements

#### USB Detection Issues
**Problem**: USB devices not detected
**Solutions**:
- Verify USB controller support (xHCI/EHCI)
- Check AMD chipset drivers
- Try different USB ports
- Enable legacy USB support in BIOS

#### Build Issues
**Problem**: Compilation fails
**Solutions**:
- Verify EDK2 installation and setup
- Check compiler toolchain installation
- Ensure all dependencies are installed
- Clean build directory and retry

### Debug Information

#### Serial Console Output
Connect a USB-to-serial adapter to capture debug output:
- Baud rate: 115200
- Data bits: 8
- Stop bits: 1
- Parity: None

#### Memory Requirements
- Minimum: 4MB available memory
- Recommended: 8MB+ for full debugging features
- Flash operations may require additional memory

#### QEMU Testing
For development testing without physical hardware:
```bash
# Test with QEMU
make qemu-test

# Or manually:
qemu-system-x86_64 -pflash OVMF.fd -hda fat:rw:build/bin -serial stdio
```

### Getting Help

- **GitHub Issues**: Report bugs and feature requests
- **Documentation**: Check `docs/` directory for detailed information
- **Debug Logs**: Include debug output when reporting issues
- **System Info**: Provide motherboard model, CPU, and BIOS version

## Contributing

We welcome contributions from the community! Please read our contributing guidelines:

### Development Setup
1. Fork the repository
2. Create a feature branch
3. Set up development environment
4. Make your changes
5. Test thoroughly
6. Submit a pull request

### Code Style
- Follow existing code formatting
- Use descriptive variable names
- Add comments for complex logic
- Include debug output for new features

### Testing
- Test on multiple AMD AM5 systems
- Verify both DEBUG and RELEASE builds
- Test USB device compatibility
- Document any hardware-specific issues

## License

This project is licensed under the BSD 3-Clause License - see the [LICENSE](LICENSE) file for details.

### Third-Party Dependencies
- **EDK2**: BSD 2-Clause License
- **TianoCore**: Multiple licenses (see EDK2 documentation)

## Acknowledgments

- TianoCore EDK2 project for UEFI development framework
- AMD for processor documentation and support
- Open source community for tools and libraries
- PhD research advisor and committee members

---

**Disclaimer**: This software is for educational and research purposes. Use at your own risk. The authors are not responsible for any hardware damage or data loss.

**Contact**: For questions related to this PhD research project, please open an issue on GitHub.
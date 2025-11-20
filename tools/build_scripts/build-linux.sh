#!/bin/bash

# USB UEFI Firmware Build Script
# Supports both development and production builds

set -e

# Configuration
PROJECT_NAME="USBUefiFirmware"
SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
PROJECT_ROOT="$(dirname "$(dirname "$SCRIPT_DIR")")"
BUILD_DIR="$PROJECT_ROOT/build"

# Colors for output
RED='\033[0;31m'
GREEN='\033[0;32m'
YELLOW='\033[1;33m'
BLUE='\033[0;34m'
NC='\033[0m' # No Color

# Functions
log_info() {
    echo -e "${GREEN}[INFO]${NC} $1"
}

log_warn() {
    echo -e "${YELLOW}[WARN]${NC} $1"
}

log_error() {
    echo -e "${RED}[ERROR]${NC} $1"
}

log_step() {
    echo -e "${BLUE}[STEP]${NC} $1"
}

# Check prerequisites
check_prerequisites() {
    log_step "Checking prerequisites..."
    
    # Check for EDK2
    if [ -z "$EDK2_PATH" ]; then
        if [ -d "$HOME/edk2" ]; then
            export EDK2_PATH="$HOME/edk2"
        else
            log_error "EDK2_PATH not set and ~/edk2 not found"
            exit 1
        fi
    fi
    
    if [ ! -d "$EDK2_PATH" ]; then
        log_error "EDK2 not found at $EDK2_PATH"
        exit 1
    fi
    
    # Check for required tools
    command -v gcc >/dev/null 2>&1 || { log_error "GCC not found"; exit 1; }
    command -v make >/dev/null 2>&1 || { log_error "Make not found"; exit 1; }
    command -v nasm >/dev/null 2>&1 || { log_error "NASM not found"; exit 1; }
    
    log_info "Prerequisites check passed"
}

# Setup build environment
setup_environment() {
    log_step "Setting up build environment..."
    
    cd "$EDK2_PATH"
    source edksetup.sh >/dev/null 2>&1
    
    # Set environment variables
    export WORKSPACE="$EDK2_PATH"
    export PACKAGES_PATH="$EDK2_PATH:$PROJECT_ROOT"
    
    cd "$PROJECT_ROOT"
    log_info "Build environment ready"
}

# Clean build
clean_build() {
    log_step "Cleaning build directory..."
    
    if [ -d "$BUILD_DIR" ]; then
        rm -rf "$BUILD_DIR"
        log_info "Build directory cleaned"
    fi
}

# Build project
build_project() {
    local target=${1:-DEBUG}
    local arch=${2:-X64}
    
    log_step "Building $PROJECT_NAME ($target, $arch)..."
    
    cd "$PROJECT_ROOT"
    
    # Create build directories
    mkdir -p "$BUILD_DIR/obj" "$BUILD_DIR/bin"
    
    # Build using make
    make TARGET="$target" TARGET_ARCH="$arch" all
    
    if [ $? -eq 0 ]; then
        log_info "Build completed successfully"
    else
        log_error "Build failed"
        exit 1
    fi
}

# Package for distribution
package_release() {
    log_step "Packaging release..."
    
    local version="1.0.0"
    local package_name="${PROJECT_NAME}-${version}"
    local package_dir="$BUILD_DIR/$package_name"
    
    mkdir -p "$package_dir"
    
    # Copy binaries
    cp "$BUILD_DIR/bin/${PROJECT_NAME}.efi" "$package_dir/BOOTX64.EFI"
    
    # Copy documentation
    cp "$PROJECT_ROOT/README.md" "$package_dir/"
    cp "$PROJECT_ROOT/LICENSE" "$package_dir/" 2>/dev/null || true
    
    # Create package structure
    mkdir -p "$package_dir/EFI/BOOT"
    cp "$package_dir/BOOTX64.EFI" "$package_dir/EFI/BOOT/"
    
    # Create archive
    cd "$BUILD_DIR"
    tar -czf "${package_name}.tar.gz" "$package_name"
    
    log_info "Package created: $BUILD_DIR/${package_name}.tar.gz"
}

# Test with QEMU
test_qemu() {
    log_step "Testing with QEMU..."
    
    if ! command -v qemu-system-x86_64 >/dev/null 2>&1; then
        log_warn "QEMU not found, skipping test"
        return
    fi
    
    # Download OVMF if not present
    if [ ! -f "$BUILD_DIR/OVMF.fd" ]; then
        log_info "Downloading OVMF firmware..."
        wget -O "$BUILD_DIR/OVMF.fd" "https://github.com/clearlinux/common/raw/master/OVMF.fd"
    fi
    
    # Create test directory structure
    local test_dir="$BUILD_DIR/qemu_test"
    mkdir -p "$test_dir/EFI/BOOT"
    cp "$BUILD_DIR/bin/${PROJECT_NAME}.efi" "$test_dir/EFI/BOOT/BOOTX64.EFI"
    
    # Run QEMU
    log_info "Starting QEMU (press Ctrl+Alt+G to release mouse, Ctrl+Alt+2 for console)"
    qemu-system-x86_64 \
        -pflash "$BUILD_DIR/OVMF.fd" \
        -hda "fat:rw:$test_dir" \
        -net none \
        -serial stdio \
        -m 512
}

# Install to USB drive
install_usb() {
    local usb_path="$1"
    
    if [ -z "$usb_path" ]; then
        log_error "USB mount path not specified"
        log_info "Usage: $0 install /path/to/usb/mount"
        exit 1
    fi
    
    if [ ! -d "$usb_path" ]; then
        log_error "USB mount path does not exist: $usb_path"
        exit 1
    fi
    
    log_step "Installing to USB drive at $usb_path..."
    
    # Create EFI directory structure
    mkdir -p "$usb_path/EFI/BOOT"
    
    # Copy EFI application
    if [ -f "$BUILD_DIR/bin/${PROJECT_NAME}.efi" ]; then
        cp "$BUILD_DIR/bin/${PROJECT_NAME}.efi" "$usb_path/EFI/BOOT/BOOTX64.EFI"
        log_info "Installed BOOTX64.EFI to USB drive"
    else
        log_error "Build output not found. Run 'build' first."
        exit 1
    fi
    
    # Copy documentation
    cp "$PROJECT_ROOT/README.md" "$usb_path/"
    
    log_info "Installation to USB completed"
}

# Run unit tests
run_tests() {
    log_step "Running unit tests..."
    
    # For now, just check if project compiles
    log_info "Compilation test..."
    build_project "DEBUG"
    
    # Add more tests here as they're developed
    log_info "All tests passed"
}

# AMD-specific hardware checks
check_amd_compatibility() {
    log_step "Checking AMD AM5 compatibility..."
    
    # Check for AMD CPU
    if [ -f "/proc/cpuinfo" ]; then
        if grep -q "AuthenticAMD" /proc/cpuinfo; then
            log_info "AMD processor detected"
            
            # Check for Zen 4/5 architecture
            if grep -q "model.*: 19" /proc/cpuinfo; then
                log_info "Zen 3+ architecture detected (compatible)"
            elif grep -q "model.*: 25" /proc/cpuinfo; then
                log_info "Zen 4 architecture detected (fully supported)"
            else
                log_warn "CPU model may not be fully supported"
            fi
        else
            log_warn "Non-AMD processor detected"
        fi
    else
        log_info "Cannot detect CPU (not on Linux)"
    fi
    
    # Check for USB 3.0+ controllers
    if command -v lspci >/dev/null 2>&1; then
        if lspci | grep -i "usb.*3\." >/dev/null; then
            log_info "USB 3.0+ controller detected"
        else
            log_warn "USB 3.0+ controller not detected"
        fi
    fi
}

# Create EDK2 package description
create_edk2_package() {
    log_step "Creating EDK2 package description..."
    
    local dsc_file="$PROJECT_ROOT/${PROJECT_NAME}.dsc"
    
    cat > "$dsc_file" << 'EOF'
#
# USB UEFI Firmware Package Description
#

[Defines]
  PLATFORM_NAME                  = USBUefiFirmware
  PLATFORM_GUID                  = 12345678-1234-1234-1234-123456789abc
  PLATFORM_VERSION               = 1.0
  DSC_SPECIFICATION              = 0x00010005
  OUTPUT_DIRECTORY               = Build/USBUefiFirmware
  SUPPORTED_ARCHITECTURES        = X64
  BUILD_TARGETS                  = DEBUG|RELEASE
  SKUID_IDENTIFIER               = DEFAULT

[LibraryClasses]
  UefiApplicationEntryPoint|MdePkg/Library/UefiApplicationEntryPoint/UefiApplicationEntryPoint.inf
  UefiBootServicesTableLib|MdePkg/Library/UefiBootServicesTableLib/UefiBootServicesTableLib.inf
  UefiRuntimeServicesTableLib|MdePkg/Library/UefiRuntimeServicesTableLib/UefiRuntimeServicesTableLib.inf
  UefiLib|MdePkg/Library/UefiLib/UefiLib.inf
  BaseLib|MdePkg/Library/BaseLib/BaseLib.inf
  BaseMemoryLib|MdePkg/Library/BaseMemoryLib/BaseMemoryLib.inf
  MemoryAllocationLib|MdePkg/Library/UefiMemoryAllocationLib/UefiMemoryAllocationLib.inf
  DebugLib|MdePkg/Library/BaseDebugLibNull/BaseDebugLibNull.inf
  PrintLib|MdePkg/Library/BasePrintLib/BasePrintLib.inf

[Components]
  USBUefiFirmware/USBUefiFirmware.inf

EOF
    
    log_info "EDK2 package description created: $dsc_file"
}

# Show usage
show_usage() {
    echo "Usage: $0 [OPTIONS] [COMMAND]"
    echo ""
    echo "Commands:"
    echo "  build [DEBUG|RELEASE]  - Build the project (default: DEBUG)"
    echo "  clean                  - Clean build directory"
    echo "  package               - Create release package"
    echo "  test                  - Test with QEMU"
    echo "  install USB_PATH      - Install to USB drive"
    echo "  check-hw              - Check AMD hardware compatibility"
    echo "  edk2-setup            - Create EDK2 package files"
    echo "  unit-test             - Run unit tests"
    echo "  all                   - Clean, build, and package"
    echo ""
    echo "Options:"
    echo "  -h, --help            - Show this help"
    echo "  -v, --verbose         - Verbose output"
    echo ""
    echo "Environment variables:"
    echo "  EDK2_PATH             - Path to EDK2 installation"
    echo ""
    echo "Examples:"
    echo "  $0 build DEBUG        - Build debug version"
    echo "  $0 test               - Build and test with QEMU"
    echo "  $0 install /mnt/usb   - Install to USB drive"
    echo "  $0 check-hw           - Check hardware compatibility"
}

# Main script
main() {
    local command="build"
    local target="DEBUG"
    local verbose=false
    local usb_path=""
    
    # Parse arguments
    while [[ $# -gt 0 ]]; do
        case $1 in
            build)
                command="build"
                if [[ $# -gt 1 && "$2" =~ ^(DEBUG|RELEASE)$ ]]; then
                    target="$2"
                    shift
                fi
                shift
                ;;
            clean)
                command="clean"
                shift
                ;;
            package)
                command="package"
                shift
                ;;
            test)
                command="test"
                shift
                ;;
            install)
                command="install"
                if [[ $# -gt 1 ]]; then
                    usb_path="$2"
                    shift
                fi
                shift
                ;;
            check-hw)
                command="check-hw"
                shift
                ;;
            edk2-setup)
                command="edk2-setup"
                shift
                ;;
            unit-test)
                command="unit-test"
                shift
                ;;
            all)
                command="all"
                shift
                ;;
            -h|--help)
                show_usage
                exit 0
                ;;
            -v|--verbose)
                verbose=true
                set -x
                shift
                ;;
            *)
                log_error "Unknown option: $1"
                show_usage
                exit 1
                ;;
        esac
    done
    
    log_info "USB UEFI Firmware Build Script v1.0"
    log_info "Project: $PROJECT_NAME"
    log_info "Target: $target"
    log_info "Command: $command"
    
    case $command in
        build)
            check_prerequisites
            setup_environment
            build_project "$target"
            ;;
        clean)
            clean_build
            ;;
        package)
            check_prerequisites
            setup_environment
            build_project "RELEASE"
            package_release
            ;;
        test)
            check_prerequisites
            setup_environment
            build_project "DEBUG"
            test_qemu
            ;;
        install)
            install_usb "$usb_path"
            ;;
        check-hw)
            check_amd_compatibility
            ;;
        edk2-setup)
            create_edk2_package
            ;;
        unit-test)
            check_prerequisites
            setup_environment
            run_tests
            ;;
        all)
            check_prerequisites
            setup_environment
            clean_build
            build_project "RELEASE"
            package_release
            ;;
        *)
            show_usage
            exit 1
            ;;
    esac
    
    log_info "Script completed successfully"
}

# Make script executable
chmod +x "$0" 2>/dev/null || true

# Run main function
main "$@"
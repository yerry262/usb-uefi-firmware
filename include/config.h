#ifndef _CONFIG_H_
#define _CONFIG_H_

//
// USB Driver Configuration
//
#define MAX_USB_DEVICES             32
#define USB_TRANSFER_TIMEOUT        5000    // 5 seconds in milliseconds
#define USB_MAX_PACKET_SIZE         64
#define USB_CONTROL_TIMEOUT         1000    // 1 second
#define USB_BULK_TIMEOUT            3000    // 3 seconds
#define USB_INTERRUPT_TIMEOUT       100     // 100ms

//
// Memory Configuration
//
#define DEFAULT_BUFFER_SIZE         4096
#define MAX_BUFFER_SIZE             65536
#define MEMORY_ALIGNMENT            16

//
// Debug Configuration
//
#ifdef DEBUG_BUILD
    #define ENABLE_USB_DEBUG        TRUE
    #define ENABLE_VERBOSE_LOGGING  TRUE
    #define DEBUG_BUFFER_SIZE       1024
#else
    #define ENABLE_USB_DEBUG        FALSE
    #define ENABLE_VERBOSE_LOGGING  FALSE
    #define DEBUG_BUFFER_SIZE       256
#endif

//
// Firmware Configuration
//
#define FIRMWARE_VERSION_MAJOR      1
#define FIRMWARE_VERSION_MINOR      0
#define FIRMWARE_VERSION_PATCH      0
#define FIRMWARE_BUILD_NUMBER       1

#define FIRMWARE_NAME               L"USB UEFI Firmware"
#define FIRMWARE_VENDOR             L"Research Project"
#define FIRMWARE_COPYRIGHT          L"(C) 2025 PhD Project"

//
// AMD Ryzen/AM5 Specific Configuration
//
#define SUPPORT_AMD_RYZEN_7000      TRUE
#define SUPPORT_AMD_RYZEN_9000      TRUE
#define ENABLE_AMD_USB_QUIRKS       TRUE

//
// USB Controller Types
//
#define SUPPORT_XHCI                TRUE
#define SUPPORT_EHCI                TRUE
#define SUPPORT_OHCI                FALSE
#define SUPPORT_UHCI                FALSE

//
// Network Configuration (for remote access)
//
#define ENABLE_NETWORK_STACK        TRUE
#define DEFAULT_SSH_PORT            22
#define MAX_NETWORK_CONNECTIONS     5

//
// Boot Configuration
//
#define AUTO_BOOT_TIMEOUT           10      // seconds
#define ENABLE_SHELL_ACCESS         TRUE
#define ENABLE_REMOTE_DEBUG         TRUE

//
// Hardware Specific Settings
//
#define AM5_USB_BASE_ADDRESS        0xFED80000
#define MAX_USB_PORTS               10
#define USB_POWER_MANAGEMENT        TRUE

//
// Security Configuration
//
#define ENABLE_SECURE_BOOT          FALSE   // Disabled for debugging
#define REQUIRE_AUTHENTICATION      FALSE   // Disabled for debugging
#define ENABLE_AUDIT_LOGGING        TRUE

//
// Performance Configuration
//
#define ENABLE_DMA_TRANSFERS        TRUE
#define USE_INTERRUPT_DRIVEN_IO     TRUE
#define ENABLE_USB3_SUPERSPEED      TRUE

#endif // _CONFIG_H_
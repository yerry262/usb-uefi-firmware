// uefi_interface.h
// This header file declares the UEFI interface functions and structures.

// Include guard to prevent multiple inclusions
#ifndef _UEFI_INTERFACE_H_
#define _UEFI_INTERFACE_H_

// Include necessary headers
#include <Uefi.h>
#include <Library/UefiLib.h>
#include "boot_services.h"
#include "../../include/common.h"

// Function declarations

/**
 * Initialize UEFI interface
 * @return EFI_STATUS - Success or error code
 */
EFI_STATUS
EFIAPI
uefi_interface_init(
    VOID
    );

/**
 * Get system information
 * @param SystemInfo - Pointer to receive system info
 * @return EFI_STATUS - Success or error code
 */
EFI_STATUS
EFIAPI
uefi_get_system_info(
    OUT UEFI_SYSTEM_INFO *SystemInfo
    );

/**
 * Display UEFI interface status
 * @return EFI_STATUS - Success or error code
 */
EFI_STATUS
EFIAPI
uefi_interface_status(
    VOID
    );

/**
 * Cleanup UEFI interface
 * @return EFI_STATUS - Success or error code
 */
EFI_STATUS
EFIAPI
uefi_interface_cleanup(
    VOID
    );

// Structure definitions

/**
 * @brief UEFI interface structure.
 * 
 * This structure holds information related to the UEFI interface, including
 * pointers to the system table and boot services.
 */
typedef struct {
    EFI_SYSTEM_TABLE *SystemTable;    // Pointer to the UEFI system table
    EFI_BOOT_SERVICES *BootServices;  // Boot services pointer
} UEFIInterface;

// System Information Structure
//
typedef struct {
    UINT16 UefiMajorVersion;
    UINT16 UefiMinorVersion;
    CHAR16 FirmwareVendor[64];
    UINT32 FirmwareRevision;
    
    CHAR16 CpuVendor[32];
    CHAR16 CpuFamily[64];
    UINT32 CpuCores;
    UINT32 CpuThreads;
    
    UINT64 TotalMemory;
    UINT64 AvailableMemory;
    UINT64 ReservedMemory;
    BOOLEAN SecureBootEnabled;
    BOOLEAN TpmPresent;
} UEFI_SYSTEM_INFO;

#endif // _UEFI_INTERFACE_H_
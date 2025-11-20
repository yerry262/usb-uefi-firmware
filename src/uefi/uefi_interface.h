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
 * Get basic memory information (totals from cached system info)
 */
EFI_STATUS
EFIAPI
uefi_get_memory_info(
    OUT UINT64 *TotalMemory,
    OUT UINT64 *AvailableMemory
    );

/**
 * Detect whether the current platform is AMD Ryzen (AM5 family)
 */
EFI_STATUS
EFIAPI
uefi_detect_amd_platform(
    OUT BOOLEAN *IsAmdRyzen
    );

/**
 * Get CPU information (vendor, family string, and core count)
 */
EFI_STATUS
EFIAPI
uefi_get_cpu_info(
    OUT CHAR16 *CpuVendor,
    OUT CHAR16 *CpuFamily,
    OUT UINT32 *CpuCores
    );

/**
 * Check Secure Boot status (via UEFI variable SecureBoot)
 */
EFI_STATUS
EFIAPI
uefi_check_secure_boot(
    OUT BOOLEAN *SecureBootEnabled
    );

/**
 * Check TPM presence (TCG2 protocol availability)
 */
EFI_STATUS
EFIAPI
uefi_check_tpm(
    OUT BOOLEAN *TpmPresent
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
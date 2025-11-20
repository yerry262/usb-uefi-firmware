/**
 * @file uefi_interface.c
 * @brief UEFI-specific implementations and services
 */

#include <Uefi.h>
#include <Library/UefiLib.h>
#include <Library/MemoryAllocationLib.h>
#include <Library/BaseLib.h>
#include <Library/UefiBootServicesTableLib.h>
#include <Library/UefiRuntimeServicesTableLib.h>
#include <Library/DevicePathLib.h>
#include <Protocol/LoadedImage.h>
#include <Protocol/SimpleFileSystem.h>
#include <Protocol/BlockIo.h>

#include "uefi_interface.h"
#include "boot_services.h"
#include "../include/common.h"
#include "../include/config.h"
#include "../include/debug_utils.h"

//
// Static variables
//
STATIC BOOLEAN mUefiInitialized = FALSE;
STATIC EFI_LOADED_IMAGE_PROTOCOL *mLoadedImage = NULL;
STATIC UEFI_SYSTEM_INFO mSystemInfo;

/**
 * Initialize UEFI interface and gather system information
 * @return EFI_STATUS - Success or error code
 */
EFI_STATUS
EFIAPI
uefi_interface_init(VOID)
{
    EFI_STATUS Status;
    
    DBG_ENTER();
    
    if (mUefiInitialized) {
        DBG_EXIT_STATUS(EFI_ALREADY_STARTED);
        return EFI_ALREADY_STARTED;
    }
    
    // Get loaded image protocol
    Status = gBS->OpenProtocol(
        gImageHandle,
        &gEfiLoadedImageProtocolGuid,
        (VOID **)&mLoadedImage,
        gImageHandle,
        NULL,
        EFI_OPEN_PROTOCOL_GET_PROTOCOL
    );
    CHECK_STATUS(Status, "Failed to get LoadedImage protocol");
    
    // Gather system information
    Status = GatherSystemInformation();
    CHECK_STATUS(Status, "Failed to gather system information");
    
    // Initialize boot services wrappers
    Status = InitializeBootServicesWrappers();
    CHECK_STATUS(Status, "Failed to initialize boot services");
    
    mUefiInitialized = TRUE;
    
    LOG_INFO("UEFI interface initialized successfully\n");
    LOG_INFO("UEFI Version: %d.%02d\n", 
             gST->Hdr.Revision >> 16, (gST->Hdr.Revision & 0xFFFF) / 10);
    
    DBG_EXIT_STATUS(EFI_SUCCESS);
    return EFI_SUCCESS;
}

/**
 * Gather comprehensive system information
 * @return EFI_STATUS - Success or error code
 */
STATIC
EFI_STATUS
GatherSystemInformation(VOID)
{
    EFI_STATUS Status;
    UINTN MapSize;
    UINTN MapKey;
    UINTN DescriptorSize;
    UINT32 DescriptorVersion;
    EFI_MEMORY_DESCRIPTOR *MemoryMap;
    
    DBG_ENTER();
    
    // Initialize system info structure
    ZeroMemory(&mSystemInfo, sizeof(SYSTEM_INFO));
    
    // Get UEFI version
    mSystemInfo.UefiMajorVersion = (UINT16)(gST->Hdr.Revision >> 16);
    mSystemInfo.UefiMinorVersion = (UINT16)((gST->Hdr.Revision & 0xFFFF) / 10);
    
    // Get firmware vendor
    if (gST->FirmwareVendor) {
        StrCpyS(mSystemInfo.FirmwareVendor, 
               sizeof(mSystemInfo.FirmwareVendor) / sizeof(CHAR16),
               gST->FirmwareVendor);
    }
    
    mSystemInfo.FirmwareRevision = gST->FirmwareRevision;
    
    // Get memory information
    MapSize = 0;
    Status = gBS->GetMemoryMap(&MapSize, NULL, &MapKey, &DescriptorSize, &DescriptorVersion);
    if (Status == EFI_BUFFER_TOO_SMALL) {
        MemoryMap = AllocatePool(MapSize);
        if (MemoryMap != NULL) {
            Status = gBS->GetMemoryMap(&MapSize, MemoryMap, &MapKey, &DescriptorSize, &DescriptorVersion);
            if (!EFI_ERROR(Status)) {
                CalculateMemoryStatistics(MemoryMap, MapSize, DescriptorSize);
            }
            FreePool(MemoryMap);
        }
    }
    
    // Detect CPU information (AMD Ryzen specific)
    DetectCpuInformation();
    
    LOG_INFO("System information gathered successfully\n");
    
    DBG_EXIT_STATUS(EFI_SUCCESS);
    return EFI_SUCCESS;
}

/**
 * Calculate memory statistics from memory map
 * @param MemoryMap - UEFI memory map
 * @param MapSize - Size of memory map
 * @param DescriptorSize - Size of each descriptor
 */
STATIC
VOID
CalculateMemoryStatistics(
    IN EFI_MEMORY_DESCRIPTOR *MemoryMap,
    IN UINTN MapSize,
    IN UINTN DescriptorSize
)
{
    EFI_MEMORY_DESCRIPTOR *Desc;
    UINTN NumEntries;
    UINTN Index;
    
    NumEntries = MapSize / DescriptorSize;
    Desc = MemoryMap;
    
    mSystemInfo.TotalMemory = 0;
    mSystemInfo.AvailableMemory = 0;
    mSystemInfo.ReservedMemory = 0;
    
    for (Index = 0; Index < NumEntries; Index++) {
        UINT64 MemorySize = Desc->NumberOfPages * EFI_PAGE_SIZE;
        
        mSystemInfo.TotalMemory += MemorySize;
        
        switch (Desc->Type) {
            case EfiConventionalMemory:
            case EfiBootServicesCode:
            case EfiBootServicesData:
                mSystemInfo.AvailableMemory += MemorySize;
                break;
                
            case EfiReservedMemoryType:
            case EfiRuntimeServicesCode:
            case EfiRuntimeServicesData:
            case EfiUnusableMemory:
            case EfiACPIReclaimMemory:
            case EfiACPIMemoryNVS:
                mSystemInfo.ReservedMemory += MemorySize;
                break;
                
            default:
                break;
        }
        
        Desc = (EFI_MEMORY_DESCRIPTOR *)((UINT8 *)Desc + DescriptorSize);
    }
    
    LOG_INFO("Memory Statistics:\n");
    LOG_INFO("  Total: %ld MB\n", mSystemInfo.TotalMemory / (1024 * 1024));
    LOG_INFO("  Available: %ld MB\n", mSystemInfo.AvailableMemory / (1024 * 1024));
    LOG_INFO("  Reserved: %ld MB\n", mSystemInfo.ReservedMemory / (1024 * 1024));
}

/**
 * Detect CPU information (AMD Ryzen specific)
 */
STATIC
VOID
DetectCpuInformation(VOID)
{
    // This would typically use CPUID instruction
    // For now, we'll set default values for AM5/Ryzen
    
    StrCpyS(mSystemInfo.CpuVendor, 
           sizeof(mSystemInfo.CpuVendor) / sizeof(CHAR16),
           L"AuthenticAMD");
    
    StrCpyS(mSystemInfo.CpuFamily, 
           sizeof(mSystemInfo.CpuFamily) / sizeof(CHAR16),
           L"AMD Ryzen");
    
    mSystemInfo.CpuCores = 8;  // Default for testing
    mSystemInfo.CpuThreads = 16; // Default for testing
    
    LOG_INFO("CPU Information:\n");
    LOG_INFO("  Vendor: %s\n", mSystemInfo.CpuVendor);
    LOG_INFO("  Family: %s\n", mSystemInfo.CpuFamily);
    LOG_INFO("  Cores: %d\n", mSystemInfo.CpuCores);
    LOG_INFO("  Threads: %d\n", mSystemInfo.CpuThreads);
}

/**
 * Get current system information
 * @param SystemInfo - Pointer to receive system info
 * @return EFI_STATUS - Success or error code
 */
EFI_STATUS
EFIAPI
uefi_get_system_info(
    OUT SYSTEM_INFO *SystemInfo
)
{
    if (SystemInfo == NULL || !mUefiInitialized) {
        return EFI_INVALID_PARAMETER;
    }
    
    CopyMemory(SystemInfo, &mSystemInfo, sizeof(SYSTEM_INFO));
    return EFI_SUCCESS;
}

/**
 * Display UEFI interface status
 * @return EFI_STATUS - Success or error code
 */
EFI_STATUS
EFIAPI
uefi_interface_status(VOID)
{
    if (!mUefiInitialized) {
        Print(L"UEFI Interface: Not initialized\n");
        return EFI_NOT_READY;
    }
    
    Print(L"UEFI Interface Status:\n");
    Print(L"  UEFI Version: %d.%02d\n", 
          mSystemInfo.UefiMajorVersion, mSystemInfo.UefiMinorVersion);
    Print(L"  Firmware Vendor: %s\n", mSystemInfo.FirmwareVendor);
    Print(L"  Firmware Revision: 0x%08X\n", mSystemInfo.FirmwareRevision);
    Print(L"  CPU Vendor: %s\n", mSystemInfo.CpuVendor);
    Print(L"  CPU Family: %s\n", mSystemInfo.CpuFamily);
    Print(L"  CPU Cores: %d\n", mSystemInfo.CpuCores);
    Print(L"  Total Memory: %ld MB\n", mSystemInfo.TotalMemory / (1024 * 1024));
    Print(L"  Available Memory: %ld MB\n", mSystemInfo.AvailableMemory / (1024 * 1024));
    
    return EFI_SUCCESS;
}

/**
 * Cleanup UEFI interface resources
 * @return EFI_STATUS - Success or error code
 */
EFI_STATUS
EFIAPI
uefi_interface_cleanup(VOID)
{
    DBG_ENTER();
    
    if (!mUefiInitialized) {
        DBG_EXIT_STATUS(EFI_NOT_READY);
        return EFI_NOT_READY;
    }
    
    // Close protocols
    if (mLoadedImage != NULL) {
        gBS->CloseProtocol(
            gImageHandle,
            &gEfiLoadedImageProtocolGuid,
            gImageHandle,
            NULL
        );
        mLoadedImage = NULL;
    }
    
    mUefiInitialized = FALSE;
    
    LOG_INFO("UEFI interface cleanup complete\n");
    
    DBG_EXIT_STATUS(EFI_SUCCESS);
    return EFI_SUCCESS;
}
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
// For SecureBoot UEFI variable
#include <Guid/GlobalVariable.h>
// For TPM 2.0 protocol detection
#include <Protocol/Tcg2Protocol.h>

#include "uefi_interface.h"
#include "boot_services.h"
#include "../../include/common.h"
#include "../../include/config.h"
#include "../../include/debug_utils.h"

//
// Forward declarations for internal functions
//
STATIC EFI_STATUS GatherSystemInformation(VOID);
STATIC VOID CalculateMemoryStatistics(IN EFI_MEMORY_DESCRIPTOR *MemoryMap, IN UINTN MapSize, IN UINTN DescriptorSize);
STATIC VOID DetectCpuInformation(VOID);

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
    ZeroMemory(&mSystemInfo, sizeof(UEFI_SYSTEM_INFO));
    
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
    
    // Detect CPU information (CPUID-based)
    DetectCpuInformation();

    // Cache security features (best-effort; do not fail init on errors)
    {
        EFI_STATUS SbStatus;
        EFI_STATUS TpmStatus;
        BOOLEAN SbEnabled = FALSE;
        BOOLEAN TpmPresent = FALSE;
        SbStatus = uefi_check_secure_boot(&SbEnabled);
        if (EFI_ERROR(SbStatus)) {
            SbEnabled = FALSE;
        }
        mSystemInfo.SecureBootEnabled = SbEnabled;

        TpmStatus = uefi_check_tpm(&TpmPresent);
        if (EFI_ERROR(TpmStatus)) {
            TpmPresent = FALSE;
        }
        mSystemInfo.TpmPresent = TpmPresent;
    }
    
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
    UINT32 Eax, Ebx, Ecx, Edx;
    CHAR8  VendorAscii[13];
    CHAR8  BrandAscii[49];
    UINTN  i;
    UINT32 MaxExtLeaf = 0;
    UINT32 Cores = 0;

    // Query vendor string (leaf 0)
    AsmCpuid(0, &Eax, &Ebx, &Ecx, &Edx);
    *(UINT32 *)&VendorAscii[0]  = Ebx; // 'GenuineIntel' or 'AuthenticAMD'
    *(UINT32 *)&VendorAscii[4]  = Edx;
    *(UINT32 *)&VendorAscii[8]  = Ecx;
    VendorAscii[12] = '\0';

    // Copy vendor to Unicode
    for (i = 0; i < 12 && VendorAscii[i] != '\0' && i < (sizeof(mSystemInfo.CpuVendor)/sizeof(CHAR16)) - 1; i++) {
        mSystemInfo.CpuVendor[i] = (CHAR16)VendorAscii[i];
    }
    mSystemInfo.CpuVendor[i] = L'\0';

    // Brand string via extended leaves 0x80000002..4 if available
    AsmCpuid(0x80000000, &MaxExtLeaf, &Ebx, &Ecx, &Edx);
    if (MaxExtLeaf >= 0x80000004) {
        UINT32 *BrandU32 = (UINT32 *)BrandAscii;
        AsmCpuid(0x80000002, &BrandU32[0], &BrandU32[1], &BrandU32[2], &BrandU32[3]);
        AsmCpuid(0x80000003, &BrandU32[4], &BrandU32[5], &BrandU32[6], &BrandU32[7]);
        AsmCpuid(0x80000004, &BrandU32[8], &BrandU32[9], &BrandU32[10], &BrandU32[11]);
        BrandAscii[48] = '\0';
    } else {
        // Fallback: reuse vendor as family hint
        AsciiStrCpyS(BrandAscii, sizeof(BrandAscii), (CHAR8 *)VendorAscii);
    }

    // Copy brand to Unicode (trim leading spaces)
    CHAR8 *BrandSrc = BrandAscii;
    while (*BrandSrc == ' ') BrandSrc++;
    for (i = 0; BrandSrc[i] != '\0' && i < (sizeof(mSystemInfo.CpuFamily)/sizeof(CHAR16)) - 1; i++) {
        mSystemInfo.CpuFamily[i] = (CHAR16)BrandSrc[i];
    }
    mSystemInfo.CpuFamily[i] = L'\0';

    // Determine core count using leaf 0x4 (EAX[31:26] + 1) if supported
    // This is an approximation; topology leaves may be better but this is sufficient here.
    AsmCpuid(4, &Eax, &Ebx, &Ecx, &Edx);
    Cores = ((Eax >> 26) & 0x3F) + 1; // if leaf not valid, result might be 1
    if (Cores == 0) {
        Cores = 1;
    }
    mSystemInfo.CpuCores = Cores;
    // Threads are unknown without SMT topology; assume at least one per core
    mSystemInfo.CpuThreads = Cores;

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
    OUT UEFI_SYSTEM_INFO *SystemInfo
)
{
    if (SystemInfo == NULL || !mUefiInitialized) {
        return EFI_INVALID_PARAMETER;
    }
    
    CopyMemory(SystemInfo, &mSystemInfo, sizeof(UEFI_SYSTEM_INFO));
    return EFI_SUCCESS;
}

/**
 * Get basic memory information
 */
EFI_STATUS
EFIAPI
uefi_get_memory_info(
    OUT UINT64 *TotalMemory,
    OUT UINT64 *AvailableMemory
)
{
    if (TotalMemory == NULL || AvailableMemory == NULL) {
        return EFI_INVALID_PARAMETER;
    }
    if (!mUefiInitialized) {
        return EFI_NOT_READY;
    }
    *TotalMemory = mSystemInfo.TotalMemory;
    *AvailableMemory = mSystemInfo.AvailableMemory;
    return EFI_SUCCESS;
}

/**
 * Detect AMD Ryzen platform (simple heuristic based on vendor string)
 */
EFI_STATUS
EFIAPI
uefi_detect_amd_platform(
    OUT BOOLEAN *IsAmdRyzen
)
{
    if (IsAmdRyzen == NULL) {
        return EFI_INVALID_PARAMETER;
    }
    if (!mUefiInitialized) {
        return EFI_NOT_READY;
    }
    *IsAmdRyzen = (StrStr(mSystemInfo.CpuVendor, L"AMD") != NULL) ? TRUE : FALSE;
    return EFI_SUCCESS;
}

/**
 * Get CPU information
 */
EFI_STATUS
EFIAPI
uefi_get_cpu_info(
    OUT CHAR16 *CpuVendor,
    OUT CHAR16 *CpuFamily,
    OUT UINT32 *CpuCores
)
{
    if (CpuVendor == NULL || CpuFamily == NULL || CpuCores == NULL) {
        return EFI_INVALID_PARAMETER;
    }
    if (!mUefiInitialized) {
        return EFI_NOT_READY;
    }

    StrCpyS(CpuVendor, 32, mSystemInfo.CpuVendor);
    StrCpyS(CpuFamily, 64, mSystemInfo.CpuFamily);
    *CpuCores = mSystemInfo.CpuCores;
    return EFI_SUCCESS;
}

/**
 * Check Secure Boot status via SecureBoot variable
 */
EFI_STATUS
EFIAPI
uefi_check_secure_boot(
    OUT BOOLEAN *SecureBootEnabled
)
{
    EFI_STATUS Status;
    UINT8 SecureBoot = 0;
    UINTN DataSize = sizeof(SecureBoot);
    UINT32 Attributes = 0;

    if (SecureBootEnabled == NULL) {
        return EFI_INVALID_PARAMETER;
    }
    if (gRT == NULL) {
        return EFI_NOT_READY;
    }

    Status = gRT->GetVariable(
        L"SecureBoot",
        &gEfiGlobalVariableGuid,
        &Attributes,
        &DataSize,
        &SecureBoot
    );
    if (EFI_ERROR(Status)) {
        return Status;
    }
    *SecureBootEnabled = (SecureBoot != 0);
    // Cache
    mSystemInfo.SecureBootEnabled = *SecureBootEnabled;
    return EFI_SUCCESS;
}

/**
 * Check TPM presence via TCG2 protocol
 */
EFI_STATUS
EFIAPI
uefi_check_tpm(
    OUT BOOLEAN *TpmPresent
)
{
    EFI_STATUS Status;
    EFI_TCG2_PROTOCOL *Tcg2 = NULL;

    if (TpmPresent == NULL) {
        return EFI_INVALID_PARAMETER;
    }
    if (gBS == NULL) {
        return EFI_NOT_READY;
    }

    Status = gBS->LocateProtocol(&gEfiTcg2ProtocolGuid, NULL, (VOID **)&Tcg2);
    if (EFI_ERROR(Status) || Tcg2 == NULL) {
        *TpmPresent = FALSE;
        // Cache negative result as well to avoid repeated lookups later in the session
        mSystemInfo.TpmPresent = FALSE;
        return Status;
    }
    *TpmPresent = TRUE;
    // Cache
    mSystemInfo.TpmPresent = TRUE;
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
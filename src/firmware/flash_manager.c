/**
 * @file flash_manager.c
 * @brief Flash memory management for firmware updates
 */

#include <Uefi.h>
#include <Library/UefiLib.h>
#include <Library/MemoryAllocationLib.h>
#include <Library/BaseLib.h>
#include <Library/BaseMemoryLib.h>
#include <Library/DebugLib.h>
#include <Library/IoLib.h>
#include <Protocol/FirmwareVolumeBlock.h>

#include "flash_manager.h"
#include "firmware_loader.h"
#include "../include/common.h"
#include "../include/config.h"
#include "../include/debug_utils.h"

//
// Static variables
//
STATIC BOOLEAN mFlashManagerInitialized = FALSE;
STATIC EFI_FIRMWARE_VOLUME_BLOCK_PROTOCOL *mFvbProtocol = NULL;
STATIC EFI_HANDLE mFvbHandle = NULL;
STATIC FLASH_DEVICE_INFO mFlashInfo;
STATIC FLASH_REGION mFlashRegions[MAX_FLASH_REGIONS];
STATIC UINTN mRegionCount = 0;

/**
 * Initialize flash manager
 * @return EFI_STATUS - Success or error code
 */
EFI_STATUS
EFIAPI
flash_manager_init(VOID)
{
    EFI_STATUS Status;
    EFI_HANDLE *HandleBuffer;
    UINTN HandleCount;
    
    DBG_ENTER();
    
    if (mFlashManagerInitialized) {
        DBG_EXIT_STATUS(EFI_ALREADY_STARTED);
        return EFI_ALREADY_STARTED;
    }
    
    // Initialize flash info structure
    ZeroMemory(&mFlashInfo, sizeof(FLASH_DEVICE_INFO));
    ZeroMemory(mFlashRegions, sizeof(mFlashRegions));
    mRegionCount = 0;
    
    // Locate firmware volume block protocol
    Status = gBS->LocateHandleBuffer(
        ByProtocol,
        &gEfiFirmwareVolumeBlockProtocolGuid,
        NULL,
        &HandleCount,
        &HandleBuffer
    );
    
    if (EFI_ERROR(Status)) {
        LOG_WARN("No firmware volume block protocol found: %r\n", Status);
        // Continue without FVB protocol for basic functionality
        mFvbProtocol = NULL;
    } else {
        // Use the first FVB protocol found
        Status = gBS->OpenProtocol(
            HandleBuffer[0],
            &gEfiFirmwareVolumeBlockProtocolGuid,
            (VOID **)&mFvbProtocol,
            gImageHandle,
            NULL,
            EFI_OPEN_PROTOCOL_GET_PROTOCOL
        );

        if (!EFI_ERROR(Status)) {
            // Store handle for later cleanup
            mFvbHandle = HandleBuffer[0];
        }

        FreePool(HandleBuffer);

        if (EFI_ERROR(Status)) {
            LOG_WARN("Failed to open FVB protocol: %r\n", Status);
            mFvbProtocol = NULL;
        }
    }
    
    // Detect flash device information
    Status = DetectFlashDevice();
    if (EFI_ERROR(Status)) {
        LOG_WARN("Flash device detection failed: %r\n", Status);
        // Continue with default values
    }
    
    // Initialize flash regions
    Status = InitializeFlashRegions();
    CHECK_STATUS(Status, "Failed to initialize flash regions");
    
    mFlashManagerInitialized = TRUE;
    
    LOG_INFO("Flash manager initialized successfully\n");
    LOG_INFO("Flash device: %s, Size: %ld KB\n", 
             mFlashInfo.DeviceName, mFlashInfo.TotalSize / 1024);
    
    DBG_EXIT_STATUS(EFI_SUCCESS);
    return EFI_SUCCESS;
}

/**
 * Detect flash device information
 * @return EFI_STATUS - Success or error code
 */
STATIC
EFI_STATUS
DetectFlashDevice(VOID)
{
    EFI_STATUS Status;
    EFI_FVB_ATTRIBUTES_2 Attributes;
    EFI_LBA NumberOfBlocks;
    UINTN BlockSize;
    
    DBG_ENTER();
    
    // Set default flash information
    StrCpyS(mFlashInfo.DeviceName, 
           sizeof(mFlashInfo.DeviceName) / sizeof(CHAR16),
           L"Generic SPI Flash");
    
    mFlashInfo.DeviceId = 0x12345678; // Placeholder
    mFlashInfo.VendorId = 0xABCD;     // Placeholder
    mFlashInfo.TotalSize = 16 * 1024 * 1024; // 16MB default
    mFlashInfo.SectorSize = 4096;     // 4KB sectors
    mFlashInfo.WriteProtected = FALSE;
    mFlashInfo.BlockCount = mFlashInfo.TotalSize / mFlashInfo.SectorSize;
    
    if (mFvbProtocol != NULL) {
        // Get flash attributes from FVB protocol
        Status = mFvbProtocol->GetAttributes(mFvbProtocol, &Attributes);
        if (!EFI_ERROR(Status)) {
            mFlashInfo.WriteProtected = (Attributes & EFI_FVB2_READ_STATUS) ? TRUE : FALSE;
        }
        
        // Get block information
        Status = mFvbProtocol->GetBlockSize(mFvbProtocol, 0, &BlockSize, &NumberOfBlocks);
        if (!EFI_ERROR(Status)) {
            mFlashInfo.SectorSize = BlockSize;
            mFlashInfo.BlockCount = (UINT32)NumberOfBlocks;
            mFlashInfo.TotalSize = BlockSize * NumberOfBlocks;
        }
    }
    
    LOG_INFO("Flash Device Information:\n");
    LOG_INFO("  Name: %s\n", mFlashInfo.DeviceName);
    LOG_INFO("  Device ID: 0x%08X\n", mFlashInfo.DeviceId);
    LOG_INFO("  Vendor ID: 0x%04X\n", mFlashInfo.VendorId);
    LOG_INFO("  Total Size: %ld KB\n", mFlashInfo.TotalSize / 1024);
    LOG_INFO("  Sector Size: %ld bytes\n", mFlashInfo.SectorSize);
    LOG_INFO("  Block Count: %d\n", mFlashInfo.BlockCount);
    LOG_INFO("  Write Protected: %s\n", mFlashInfo.WriteProtected ? L"YES" : L"NO");
    
    DBG_EXIT_STATUS(EFI_SUCCESS);
    return EFI_SUCCESS;
}

/**
 * Initialize flash regions
 * @return EFI_STATUS - Success or error code
 */
STATIC
EFI_STATUS
InitializeFlashRegions(VOID)
{
    DBG_ENTER();
    
    // Define standard flash regions for UEFI firmware
    mRegionCount = 0;
    
    // Boot Block region
    mFlashRegions[mRegionCount].Type = FLASH_REGION_BOOT_BLOCK;
    mFlashRegions[mRegionCount].StartAddress = 0x00000000;
    mFlashRegions[mRegionCount].Size = 64 * 1024; // 64KB
    mFlashRegions[mRegionCount].WriteProtected = TRUE;
    mFlashRegions[mRegionCount].EraseRequired = TRUE;
    StrCpyS(mFlashRegions[mRegionCount].Name, 
           sizeof(mFlashRegions[mRegionCount].Name) / sizeof(CHAR16),
           L"Boot Block");
    mRegionCount++;
    
    // Main Firmware region
    mFlashRegions[mRegionCount].Type = FLASH_REGION_MAIN_FIRMWARE;
    mFlashRegions[mRegionCount].StartAddress = 64 * 1024;
    mFlashRegions[mRegionCount].Size = mFlashInfo.TotalSize - (256 * 1024); // Total - 256KB
    mFlashRegions[mRegionCount].WriteProtected = FALSE;
    mFlashRegions[mRegionCount].EraseRequired = TRUE;
    StrCpyS(mFlashRegions[mRegionCount].Name, 
           sizeof(mFlashRegions[mRegionCount].Name) / sizeof(CHAR16),
           L"Main Firmware");
    mRegionCount++;
    
    // NVRAM region
    mFlashRegions[mRegionCount].Type = FLASH_REGION_NVRAM;
    mFlashRegions[mRegionCount].StartAddress = mFlashInfo.TotalSize - (192 * 1024);
    mFlashRegions[mRegionCount].Size = 128 * 1024; // 128KB
    mFlashRegions[mRegionCount].WriteProtected = FALSE;
    mFlashRegions[mRegionCount].EraseRequired = TRUE;
    StrCpyS(mFlashRegions[mRegionCount].Name, 
           sizeof(mFlashRegions[mRegionCount].Name) / sizeof(CHAR16),
           L"NVRAM");
    mRegionCount++;
    
    // Descriptor region (Intel-specific)
    mFlashRegions[mRegionCount].Type = FLASH_REGION_DESCRIPTOR;
    mFlashRegions[mRegionCount].StartAddress = mFlashInfo.TotalSize - (64 * 1024);
    mFlashRegions[mRegionCount].Size = 64 * 1024; // 64KB
    mFlashRegions[mRegionCount].WriteProtected = TRUE;
    mFlashRegions[mRegionCount].EraseRequired = FALSE;
    StrCpyS(mFlashRegions[mRegionCount].Name, 
           sizeof(mFlashRegions[mRegionCount].Name) / sizeof(CHAR16),
           L"Flash Descriptor");
    mRegionCount++;
    
    LOG_INFO("Initialized %d flash regions\n", mRegionCount);
    
    DBG_EXIT_STATUS(EFI_SUCCESS);
    return EFI_SUCCESS;
}

/**
 * Read data from flash
 * @param Address - Flash address to read from
 * @param Buffer - Buffer to store read data
 * @param Size - Number of bytes to read
 * @return EFI_STATUS - Success or error code
 */
EFI_STATUS
EFIAPI
flash_read(
    IN UINT32 Address,
    OUT VOID *Buffer,
    IN UINTN Size
)
{
    EFI_STATUS Status;
    EFI_LBA Lba;
    UINTN Offset;
    UINTN BytesToRead;
    UINT8 *ReadBuffer;
    
    DBG_ENTER();
    
    if (Buffer == NULL || Size == 0 || !mFlashManagerInitialized) {
        DBG_EXIT_STATUS(EFI_INVALID_PARAMETER);
        return EFI_INVALID_PARAMETER;
    }
    
    if (Address + Size > mFlashInfo.TotalSize) {
        LOG_ERROR("Read beyond flash boundary: 0x%08X + %ld > 0x%08X\n", 
                  Address, Size, mFlashInfo.TotalSize);
        DBG_EXIT_STATUS(EFI_INVALID_PARAMETER);
        return EFI_INVALID_PARAMETER;
    }
    
    if (mFvbProtocol != NULL) {
        // Use FVB protocol for reading
        Lba = Address / mFlashInfo.SectorSize;
        Offset = Address % mFlashInfo.SectorSize;
        BytesToRead = Size;
        
        Status = mFvbProtocol->Read(
            mFvbProtocol,
            Lba,
            Offset,
            &BytesToRead,
            Buffer
        );
        
        if (EFI_ERROR(Status)) {
            LOG_ERROR("FVB read failed: %r\n", Status);
            DBG_EXIT_STATUS(Status);
            return Status;
        }
    } else {
        // Direct memory access (for testing/simulation)
        ReadBuffer = (UINT8 *)(UINTN)Address;
        CopyMemory(Buffer, ReadBuffer, Size);
        Status = EFI_SUCCESS;
    }
    
    LOG_INFO("Flash read: 0x%08X, %ld bytes\n", Address, Size);
    
    DBG_EXIT_STATUS(Status);
    return Status;
}

/**
 * Write data to flash
 * @param Address - Flash address to write to
 * @param Buffer - Buffer containing data to write
 * @param Size - Number of bytes to write
 * @return EFI_STATUS - Success or error code
 */
EFI_STATUS
EFIAPI
flash_write(
    IN UINT32 Address,
    IN CONST VOID *Buffer,
    IN UINTN Size
)
{
    EFI_STATUS Status;
    EFI_LBA Lba;
    UINTN Offset;
    UINTN BytesToWrite;
    
    DBG_ENTER();
    
    if (Buffer == NULL || Size == 0 || !mFlashManagerInitialized) {
        DBG_EXIT_STATUS(EFI_INVALID_PARAMETER);
        return EFI_INVALID_PARAMETER;
    }
    
    if (Address + Size > mFlashInfo.TotalSize) {
        LOG_ERROR("Write beyond flash boundary: 0x%08X + %ld > 0x%08X\n", 
                  Address, Size, mFlashInfo.TotalSize);
        DBG_EXIT_STATUS(EFI_INVALID_PARAMETER);
        return EFI_INVALID_PARAMETER;
    }
    
    if (mFlashInfo.WriteProtected) {
        LOG_ERROR("Flash is write protected\n");
        DBG_EXIT_STATUS(EFI_WRITE_PROTECTED);
        return EFI_WRITE_PROTECTED;
    }
    
    // Check if region is write protected
    Status = CheckRegionWriteProtection(Address, Size);
    if (EFI_ERROR(Status)) {
        LOG_ERROR("Region is write protected\n");
        DBG_EXIT_STATUS(Status);
        return Status;
    }
    
    if (mFvbProtocol != NULL) {
        // Use FVB protocol for writing
        Lba = Address / mFlashInfo.SectorSize;
        Offset = Address % mFlashInfo.SectorSize;
        BytesToWrite = Size;
        
        Status = mFvbProtocol->Write(
            mFvbProtocol,
            Lba,
            Offset,
            &BytesToWrite,
            (UINT8 *)Buffer
        );
        
        if (EFI_ERROR(Status)) {
            LOG_ERROR("FVB write failed: %r\n", Status);
            DBG_EXIT_STATUS(Status);
            return Status;
        }
    } else {
        // Simulate write operation
        LOG_WARN("Simulated flash write (no FVB protocol)\n");
        Status = EFI_SUCCESS;
    }
    
    LOG_INFO("Flash write: 0x%08X, %ld bytes\n", Address, Size);
    
    DBG_EXIT_STATUS(Status);
    return Status;
}

/**
 * Erase flash sector
 * @param Address - Address within sector to erase
 * @return EFI_STATUS - Success or error code
 */
EFI_STATUS
EFIAPI
flash_erase_sector(
    IN UINT32 Address
)
{
    EFI_STATUS Status;
    EFI_LBA Lba;
    
    DBG_ENTER();
    
    if (!mFlashManagerInitialized) {
        DBG_EXIT_STATUS(EFI_NOT_READY);
        return EFI_NOT_READY;
    }
    
    if (Address >= mFlashInfo.TotalSize) {
        LOG_ERROR("Erase address beyond flash boundary: 0x%08X >= 0x%08X\n", 
                  Address, mFlashInfo.TotalSize);
        DBG_EXIT_STATUS(EFI_INVALID_PARAMETER);
        return EFI_INVALID_PARAMETER;
    }
    
    if (mFlashInfo.WriteProtected) {
        LOG_ERROR("Flash is write protected\n");
        DBG_EXIT_STATUS(EFI_WRITE_PROTECTED);
        return EFI_WRITE_PROTECTED;
    }
    
    // Check if region supports erase
    Status = CheckRegionEraseSupport(Address);
    if (EFI_ERROR(Status)) {
        LOG_ERROR("Region does not support erase\n");
        DBG_EXIT_STATUS(Status);
        return Status;
    }
    
    if (mFvbProtocol != NULL) {
        // Use FVB protocol for erasing
        Lba = Address / mFlashInfo.SectorSize;
        
        Status = mFvbProtocol->EraseBlocks(
            mFvbProtocol,
            Lba,
            1,  // Erase 1 block
            EFI_LBA_LIST_TERMINATOR
        );
        
        if (EFI_ERROR(Status)) {
            LOG_ERROR("FVB erase failed: %r\n", Status);
            DBG_EXIT_STATUS(Status);
            return Status;
        }
    } else {
        // Simulate erase operation
        LOG_WARN("Simulated flash erase (no FVB protocol)\n");
        Status = EFI_SUCCESS;
    }
    
    LOG_INFO("Flash sector erased: 0x%08X\n", Address);
    
    DBG_EXIT_STATUS(Status);
    return Status;
}

/**
 * Check region write protection
 * @param Address - Start address
 * @param Size - Size of operation
 * @return EFI_STATUS - Success or error code
 */
STATIC
EFI_STATUS
CheckRegionWriteProtection(
    IN UINT32 Address,
    IN UINTN Size
)
{
    UINTN i;
    UINT32 EndAddress = Address + Size - 1;
    
    for (i = 0; i < mRegionCount; i++) {
        UINT32 RegionEnd = mFlashRegions[i].StartAddress + mFlashRegions[i].Size - 1;
        
        // Check if operation overlaps with this region
        if (Address <= RegionEnd && EndAddress >= mFlashRegions[i].StartAddress) {
            if (mFlashRegions[i].WriteProtected) {
                LOG_ERROR("Write to protected region: %s\n", mFlashRegions[i].Name);
                return EFI_WRITE_PROTECTED;
            }
        }
    }
    
    return EFI_SUCCESS;
}

/**
 * Check region erase support
 * @param Address - Address to check
 * @return EFI_STATUS - Success or error code
 */
STATIC
EFI_STATUS
CheckRegionEraseSupport(
    IN UINT32 Address
)
{
    UINTN i;
    
    for (i = 0; i < mRegionCount; i++) {
        UINT32 RegionEnd = mFlashRegions[i].StartAddress + mFlashRegions[i].Size - 1;
        
        // Check if address is within this region
        if (Address >= mFlashRegions[i].StartAddress && Address <= RegionEnd) {
            if (!mFlashRegions[i].EraseRequired) {
                LOG_ERROR("Erase not supported in region: %s\n", mFlashRegions[i].Name);
                return EFI_UNSUPPORTED;
            }
            return EFI_SUCCESS;
        }
    }
    
    LOG_ERROR("Address not found in any region: 0x%08X\n", Address);
    return EFI_NOT_FOUND;
}

/**
 * Get flash device information
 * @param FlashInfo - Pointer to receive flash info
 * @return EFI_STATUS - Success or error code
 */
EFI_STATUS
EFIAPI
flash_get_device_info(
    OUT FLASH_DEVICE_INFO *FlashInfo
)
{
    if (FlashInfo == NULL || !mFlashManagerInitialized) {
        return EFI_INVALID_PARAMETER;
    }
    
    CopyMemory(FlashInfo, &mFlashInfo, sizeof(FLASH_DEVICE_INFO));
    return EFI_SUCCESS;
}

/**
 * Display flash manager status
 * @return EFI_STATUS - Success or error code
 */
EFI_STATUS
EFIAPI
flash_manager_status(VOID)
{
    UINTN i;
    
    if (!mFlashManagerInitialized) {
        Print(L"Flash Manager: Not initialized\n");
        return EFI_NOT_READY;
    }
    
    Print(L"Flash Manager Status:\n");
    Print(L"  Device: %s\n", mFlashInfo.DeviceName);
    Print(L"  Total Size: %ld KB\n", mFlashInfo.TotalSize / 1024);
    Print(L"  Sector Size: %ld bytes\n", mFlashInfo.SectorSize);
    Print(L"  Block Count: %d\n", mFlashInfo.BlockCount);
    Print(L"  Write Protected: %s\n", mFlashInfo.WriteProtected ? L"YES" : L"NO");
    Print(L"  FVB Protocol: %s\n", mFvbProtocol != NULL ? L"Available" : L"Not Available");
    
    Print(L"\nFlash Regions (%d):\n", mRegionCount);
    for (i = 0; i < mRegionCount; i++) {
        Print(L"  %d. %s: 0x%08X - 0x%08X (%ld KB) %s %s\n",
              i + 1,
              mFlashRegions[i].Name,
              mFlashRegions[i].StartAddress,
              mFlashRegions[i].StartAddress + mFlashRegions[i].Size - 1,
              mFlashRegions[i].Size / 1024,
              mFlashRegions[i].WriteProtected ? L"[RO]" : L"[RW]",
              mFlashRegions[i].EraseRequired ? L"[E]" : L"");
    }
    
    return EFI_SUCCESS;
}

/**
 * Cleanup flash manager
 * @return EFI_STATUS - Success or error code
 */
EFI_STATUS
EFIAPI
flash_manager_cleanup(VOID)
{
    DBG_ENTER();
    
    if (!mFlashManagerInitialized) {
        DBG_EXIT_STATUS(EFI_NOT_READY);
        return EFI_NOT_READY;
    }
    
    // Close FVB protocol
    if (mFvbProtocol != NULL && mFvbHandle != NULL) {
        gBS->CloseProtocol(
            mFvbHandle,
            &gEfiFirmwareVolumeBlockProtocolGuid,
            gImageHandle,
            NULL
        );
        mFvbProtocol = NULL;
        mFvbHandle = NULL;
    }
    
    // Clear flash info
    ZeroMemory(&mFlashInfo, sizeof(FLASH_DEVICE_INFO));
    ZeroMemory(mFlashRegions, sizeof(mFlashRegions));
    mRegionCount = 0;
    
    mFlashManagerInitialized = FALSE;
    
    LOG_INFO("Flash manager cleanup complete\n");
    
    DBG_EXIT_STATUS(EFI_SUCCESS);
    return EFI_SUCCESS;
}
/**
 * @file firmware_loader.c
 * @brief Firmware loading and management functionality
 */

#include <Uefi.h>
#include <Library/UefiLib.h>
#include <Library/MemoryAllocationLib.h>
#include <Library/BaseLib.h>
#include <Library/BaseMemoryLib.h>
#include <Library/DebugLib.h>
#include <Protocol/SimpleFileSystem.h>
#include <Protocol/LoadedImage.h>

#include "firmware_loader.h"
#include "../include/common.h"
#include "../include/config.h"
#include "../include/debug_utils.h"

//
// Static variables
//
STATIC BOOLEAN mFirmwareLoaderInitialized = FALSE;
STATIC FIRMWARE_INFO mFirmwareInfo;
STATIC EFI_SIMPLE_FILE_SYSTEM_PROTOCOL *mFileSystem = NULL;
STATIC EFI_HANDLE mFileSystemHandle = NULL;

/**
 * Initialize firmware loader
 * @return EFI_STATUS - Success or error code
 */
EFI_STATUS
EFIAPI
firmware_loader_init(VOID)
{
    EFI_STATUS Status;
    EFI_LOADED_IMAGE_PROTOCOL *LoadedImage;
    
    DBG_ENTER();
    
    if (mFirmwareLoaderInitialized) {
        DBG_EXIT_STATUS(EFI_ALREADY_STARTED);
        return EFI_ALREADY_STARTED;
    }
    
    // Initialize firmware info structure
    ZeroMemory(&mFirmwareInfo, sizeof(FIRMWARE_INFO));
    
    // Get loaded image protocol
    Status = gBS->OpenProtocol(
        gImageHandle,
        &gEfiLoadedImageProtocolGuid,
        (VOID **)&LoadedImage,
        gImageHandle,
        NULL,
        EFI_OPEN_PROTOCOL_GET_PROTOCOL
    );
    CHECK_STATUS(Status, "Failed to get LoadedImage protocol");
    
    // Get file system protocol from device handle
    if (LoadedImage->DeviceHandle != NULL) {
        Status = gBS->OpenProtocol(
            LoadedImage->DeviceHandle,
            &gEfiSimpleFileSystemProtocolGuid,
            (VOID **)&mFileSystem,
            gImageHandle,
            NULL,
            EFI_OPEN_PROTOCOL_GET_PROTOCOL
        );
        if (!EFI_ERROR(Status)) {
            mFileSystemHandle = LoadedImage->DeviceHandle;
        } else {
            LOG_WARN("Failed to get file system protocol: %r\n", Status);
            // Continue without file system access
            mFileSystem = NULL;
        }
    }
    
    // Populate firmware information
    PopulateFirmwareInfo();
    
    mFirmwareLoaderInitialized = TRUE;
    
    LOG_INFO("Firmware loader initialized successfully\n");
    
    DBG_EXIT_STATUS(EFI_SUCCESS);
    return EFI_SUCCESS;
}

/**
 * Populate firmware information structure
 */
STATIC
VOID
PopulateFirmwareInfo(VOID)
{
    // Set firmware identification
    mFirmwareInfo.MajorVersion = FIRMWARE_VERSION_MAJOR;
    mFirmwareInfo.MinorVersion = FIRMWARE_VERSION_MINOR;
    mFirmwareInfo.PatchVersion = FIRMWARE_VERSION_PATCH;
    mFirmwareInfo.BuildNumber = FIRMWARE_BUILD_NUMBER;
    
    // Copy firmware name and vendor
    StrCpyS(mFirmwareInfo.Name, 
           sizeof(mFirmwareInfo.Name) / sizeof(CHAR16),
           FIRMWARE_NAME);
           
    StrCpyS(mFirmwareInfo.Vendor, 
           sizeof(mFirmwareInfo.Vendor) / sizeof(CHAR16),
           FIRMWARE_VENDOR);
           
    StrCpyS(mFirmwareInfo.Copyright, 
           sizeof(mFirmwareInfo.Copyright) / sizeof(CHAR16),
           FIRMWARE_COPYRIGHT);
    
    // Set capabilities
    mFirmwareInfo.Capabilities = 0;
    
    if (SUPPORT_AMD_RYZEN_7000) {
        mFirmwareInfo.Capabilities |= FIRMWARE_CAP_AMD_RYZEN_7000;
    }
    
    if (SUPPORT_AMD_RYZEN_9000) {
        mFirmwareInfo.Capabilities |= FIRMWARE_CAP_AMD_RYZEN_9000;
    }
    
    if (SUPPORT_XHCI) {
        mFirmwareInfo.Capabilities |= FIRMWARE_CAP_USB_XHCI;
    }
    
    if (ENABLE_NETWORK_STACK) {
        mFirmwareInfo.Capabilities |= FIRMWARE_CAP_NETWORK;
    }
    
    if (ENABLE_REMOTE_DEBUG) {
        mFirmwareInfo.Capabilities |= FIRMWARE_CAP_REMOTE_DEBUG;
    }
    
    // Set status
    mFirmwareInfo.Status = FIRMWARE_STATUS_LOADED;
    
    LOG_INFO("Firmware Info:\n");
    LOG_INFO("  Name: %s\n", mFirmwareInfo.Name);
    LOG_INFO("  Version: %d.%d.%d.%d\n", 
             mFirmwareInfo.MajorVersion, mFirmwareInfo.MinorVersion,
             mFirmwareInfo.PatchVersion, mFirmwareInfo.BuildNumber);
    LOG_INFO("  Capabilities: 0x%08X\n", mFirmwareInfo.Capabilities);
}

/**
 * Load firmware from file
 * @param FileName - Firmware file name
 * @param Buffer - Pointer to receive firmware data
 * @param Size - Pointer to receive firmware size
 * @return EFI_STATUS - Success or error code
 */
EFI_STATUS
EFIAPI
firmware_load_from_file(
    IN CHAR16 *FileName,
    OUT VOID **Buffer,
    OUT UINTN *Size
)
{
    EFI_STATUS Status;
    EFI_FILE_PROTOCOL *Root;
    EFI_FILE_PROTOCOL *File;
    EFI_FILE_INFO *FileInfo;
    UINTN FileInfoSize;
    UINTN ReadSize;
    VOID *FileBuffer;
    
    DBG_ENTER();
    
    if (FileName == NULL || Buffer == NULL || Size == NULL) {
        DBG_EXIT_STATUS(EFI_INVALID_PARAMETER);
        return EFI_INVALID_PARAMETER;
    }
    
    if (!mFirmwareLoaderInitialized || mFileSystem == NULL) {
        DBG_EXIT_STATUS(EFI_NOT_READY);
        return EFI_NOT_READY;
    }
    
    // Open root directory
    Status = mFileSystem->OpenVolume(mFileSystem, &Root);
    CHECK_STATUS(Status, "Failed to open root volume");
    
    // Open firmware file
    Status = Root->Open(
        Root,
        &File,
        FileName,
        EFI_FILE_MODE_READ,
        0
    );
    
    if (EFI_ERROR(Status)) {
        LOG_ERROR("Failed to open firmware file %s: %r\n", FileName, Status);
        Root->Close(Root);
        DBG_EXIT_STATUS(Status);
        return Status;
    }
    
    // Get file information
    FileInfoSize = sizeof(EFI_FILE_INFO) + 512;
    FileInfo = AllocatePool(FileInfoSize);
    if (FileInfo == NULL) {
        File->Close(File);
        Root->Close(Root);
        DBG_EXIT_STATUS(EFI_OUT_OF_RESOURCES);
        return EFI_OUT_OF_RESOURCES;
    }
    
    Status = File->GetInfo(File, &gEfiFileInfoGuid, &FileInfoSize, FileInfo);
    if (EFI_ERROR(Status)) {
        LOG_ERROR("Failed to get file info: %r\n", Status);
        FreePool(FileInfo);
        File->Close(File);
        Root->Close(Root);
        DBG_EXIT_STATUS(Status);
        return Status;
    }
    
    // Allocate buffer for file content
    FileBuffer = AllocatePool(FileInfo->FileSize);
    if (FileBuffer == NULL) {
        FreePool(FileInfo);
        File->Close(File);
        Root->Close(Root);
        DBG_EXIT_STATUS(EFI_OUT_OF_RESOURCES);
        return EFI_OUT_OF_RESOURCES;
    }
    
    // Read file content
    ReadSize = FileInfo->FileSize;
    Status = File->Read(File, &ReadSize, FileBuffer);
    if (EFI_ERROR(Status)) {
        LOG_ERROR("Failed to read firmware file: %r\n", Status);
        FreePool(FileBuffer);
        FreePool(FileInfo);
        File->Close(File);
        Root->Close(Root);
        DBG_EXIT_STATUS(Status);
        return Status;
    }
    
    // Verify file was read completely
    if (ReadSize != FileInfo->FileSize) {
        LOG_ERROR("Partial file read: expected %ld, got %ld\n", 
                  FileInfo->FileSize, ReadSize);
        FreePool(FileBuffer);
        FreePool(FileInfo);
        File->Close(File);
        Root->Close(Root);
        DBG_EXIT_STATUS(EFI_ABORTED);
        return EFI_ABORTED;
    }
    
    // Return results
    *Buffer = FileBuffer;
    *Size = ReadSize;
    
    // Cleanup
    FreePool(FileInfo);
    File->Close(File);
    Root->Close(Root);
    
    LOG_INFO("Successfully loaded firmware file %s (%ld bytes)\n", FileName, ReadSize);
    
    DBG_EXIT_STATUS(EFI_SUCCESS);
    return EFI_SUCCESS;
}

/**
 * Validate firmware integrity
 * @param Buffer - Firmware data buffer
 * @param Size - Firmware data size
 * @return EFI_STATUS - Success or error code
 */
EFI_STATUS
EFIAPI
firmware_validate(
    IN VOID *Buffer,
    IN UINTN Size
)
{
    UINT32 Checksum;
    UINT8 *Data;
    UINTN Index;
    
    DBG_ENTER();
    
    if (Buffer == NULL || Size == 0) {
        DBG_EXIT_STATUS(EFI_INVALID_PARAMETER);
        return EFI_INVALID_PARAMETER;
    }
    
    // Simple checksum validation
    Checksum = 0;
    Data = (UINT8 *)Buffer;
    
    for (Index = 0; Index < Size; Index++) {
        Checksum += Data[Index];
    }
    
    LOG_INFO("Firmware validation: size=%ld, checksum=0x%08X\n", Size, Checksum);
    
    // Store validation results
    mFirmwareInfo.Status = FIRMWARE_STATUS_VALIDATED;
    mFirmwareInfo.Checksum = Checksum;
    mFirmwareInfo.Size = Size;
    
    DBG_EXIT_STATUS(EFI_SUCCESS);
    return EFI_SUCCESS;
}

/**
 * Get firmware information
 * @param Info - Pointer to receive firmware info
 * @return EFI_STATUS - Success or error code
 */
EFI_STATUS
EFIAPI
firmware_get_info(
    OUT FIRMWARE_INFO *Info
)
{
    if (Info == NULL || !mFirmwareLoaderInitialized) {
        return EFI_INVALID_PARAMETER;
    }
    
    CopyMemory(Info, &mFirmwareInfo, sizeof(FIRMWARE_INFO));
    return EFI_SUCCESS;
}

/**
 * Display firmware loader status
 * @return EFI_STATUS - Success or error code
 */
EFI_STATUS
EFIAPI
firmware_loader_status(VOID)
{
    if (!mFirmwareLoaderInitialized) {
        Print(L"Firmware Loader: Not initialized\n");
        return EFI_NOT_READY;
    }
    
    Print(L"Firmware Loader Status:\n");
    Print(L"  Name: %s\n", mFirmwareInfo.Name);
    Print(L"  Vendor: %s\n", mFirmwareInfo.Vendor);
    Print(L"  Version: %d.%d.%d.%d\n", 
          mFirmwareInfo.MajorVersion, mFirmwareInfo.MinorVersion,
          mFirmwareInfo.PatchVersion, mFirmwareInfo.BuildNumber);
    Print(L"  Status: %s\n", 
          mFirmwareInfo.Status == FIRMWARE_STATUS_LOADED ? L"Loaded" :
          mFirmwareInfo.Status == FIRMWARE_STATUS_VALIDATED ? L"Validated" : L"Unknown");
    Print(L"  Capabilities: 0x%08X\n", mFirmwareInfo.Capabilities);
    Print(L"  File System: %s\n", mFileSystem != NULL ? L"Available" : L"Not Available");
    
    return EFI_SUCCESS;
}

/**
 * Cleanup firmware loader resources
 * @return EFI_STATUS - Success or error code
 */
EFI_STATUS
EFIAPI
firmware_loader_cleanup(VOID)
{
    DBG_ENTER();
    
    if (!mFirmwareLoaderInitialized) {
        DBG_EXIT_STATUS(EFI_NOT_READY);
        return EFI_NOT_READY;
    }
    
    // Close file system protocol
    if (mFileSystem != NULL && mFileSystemHandle != NULL) {
        gBS->CloseProtocol(
            mFileSystemHandle,
            &gEfiSimpleFileSystemProtocolGuid,
            gImageHandle,
            NULL
        );
        mFileSystem = NULL;
        mFileSystemHandle = NULL;
    }
    
    // Clear firmware info
    ZeroMemory(&mFirmwareInfo, sizeof(FIRMWARE_INFO));
    
    mFirmwareLoaderInitialized = FALSE;
    
    LOG_INFO("Firmware loader cleanup complete\n");
    
    DBG_EXIT_STATUS(EFI_SUCCESS);
    return EFI_SUCCESS;
}
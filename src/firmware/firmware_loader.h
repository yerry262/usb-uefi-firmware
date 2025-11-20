// firmware_loader.h
#ifndef _FIRMWARE_LOADER_H_
#define _FIRMWARE_LOADER_H_

#include <Uefi.h>

//
// Firmware Status Definitions
//
#define FIRMWARE_STATUS_UNKNOWN         0x00
#define FIRMWARE_STATUS_LOADED          0x01
#define FIRMWARE_STATUS_VALIDATED       0x02
#define FIRMWARE_STATUS_CORRUPTED       0x03
#define FIRMWARE_STATUS_INCOMPATIBLE    0x04

//
// Firmware Capability Flags
//
#define FIRMWARE_CAP_AMD_RYZEN_7000     0x00000001
#define FIRMWARE_CAP_AMD_RYZEN_9000     0x00000002
#define FIRMWARE_CAP_USB_XHCI           0x00000010
#define FIRMWARE_CAP_USB_EHCI           0x00000020
#define FIRMWARE_CAP_NETWORK            0x00000100
#define FIRMWARE_CAP_REMOTE_DEBUG       0x00000200
#define FIRMWARE_CAP_SECURE_BOOT        0x00001000
#define FIRMWARE_CAP_TPM                0x00002000
#define FIRMWARE_CAP_FLASH_UPDATE       0x00010000

//
// Firmware Information Structure
//
typedef struct {
    CHAR16 Name[64];
    CHAR16 Vendor[64];
    CHAR16 Copyright[128];
    
    UINT16 MajorVersion;
    UINT16 MinorVersion;
    UINT16 PatchVersion;
    UINT32 BuildNumber;
    
    UINT32 Status;
    UINT32 Capabilities;
    UINT32 Checksum;
    UINTN Size;
    
    EFI_TIME BuildDate;
    EFI_GUID Identifier;
} FIRMWARE_INFO;

//
// Firmware Update Package Header
//
#pragma pack(1)
typedef struct {
    UINT32 Signature;          // 'FWPK'
    UINT32 HeaderSize;
    UINT32 PackageSize;
    UINT32 Version;
    UINT32 Checksum;
    EFI_GUID TargetGuid;
    CHAR16 Description[64];
} FIRMWARE_PACKAGE_HEADER;
#pragma pack()

//
// Function Prototypes
//

/**
 * Initialize firmware loader
 * @return EFI_STATUS - Success or error code
 */
EFI_STATUS
EFIAPI
firmware_loader_init(
    VOID
    );

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
    );

/**
 * Load firmware from memory
 * @param Address - Physical address of firmware
 * @param Size - Size of firmware data
 * @param Buffer - Pointer to receive firmware copy
 * @return EFI_STATUS - Success or error code
 */
EFI_STATUS
EFIAPI
firmware_load_from_memory(
    IN EFI_PHYSICAL_ADDRESS Address,
    IN UINTN Size,
    OUT VOID **Buffer
    );

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
    );

/**
 * Validate firmware package
 * @param Package - Firmware package data
 * @param PackageSize - Package size
 * @param Header - Pointer to receive package header
 * @return EFI_STATUS - Success or error code
 */
EFI_STATUS
EFIAPI
firmware_validate_package(
    IN VOID *Package,
    IN UINTN PackageSize,
    OUT FIRMWARE_PACKAGE_HEADER **Header
    );

/**
 * Extract firmware from package
 * @param Package - Firmware package data
 * @param PackageSize - Package size
 * @param FirmwareBuffer - Pointer to receive firmware
 * @param FirmwareSize - Pointer to receive firmware size
 * @return EFI_STATUS - Success or error code
 */
EFI_STATUS
EFIAPI
firmware_extract_from_package(
    IN VOID *Package,
    IN UINTN PackageSize,
    OUT VOID **FirmwareBuffer,
    OUT UINTN *FirmwareSize
    );

/**
 * Get firmware information
 * @param Info - Pointer to receive firmware info
 * @return EFI_STATUS - Success or error code
 */
EFI_STATUS
EFIAPI
firmware_get_info(
    OUT FIRMWARE_INFO *Info
    );

/**
 * Set firmware information
 * @param Info - Firmware information to set
 * @return EFI_STATUS - Success or error code
 */
EFI_STATUS
EFIAPI
firmware_set_info(
    IN CONST FIRMWARE_INFO *Info
    );

/**
 * Check firmware compatibility
 * @param Buffer - Firmware data buffer
 * @param Size - Firmware data size
 * @param Compatible - Pointer to receive compatibility result
 * @return EFI_STATUS - Success or error code
 */
EFI_STATUS
EFIAPI
firmware_check_compatibility(
    IN VOID *Buffer,
    IN UINTN Size,
    OUT BOOLEAN *Compatible
    );

/**
 * Display firmware loader status
 * @return EFI_STATUS - Success or error code
 */
EFI_STATUS
EFIAPI
firmware_loader_status(
    VOID
    );

/**
 * Cleanup firmware loader
 * @return EFI_STATUS - Success or error code
 */
EFI_STATUS
EFIAPI
firmware_loader_cleanup(
    VOID
    );

//
// Internal Functions
//
STATIC
VOID
PopulateFirmwareInfo(
    VOID
    );

STATIC
UINT32
CalculateChecksum(
    IN VOID *Buffer,
    IN UINTN Size
    );

STATIC
EFI_STATUS
VerifySignature(
    IN VOID *Buffer,
    IN UINTN Size
    );

#endif // _FIRMWARE_LOADER_H_
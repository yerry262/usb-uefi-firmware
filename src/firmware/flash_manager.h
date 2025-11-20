:build
:clean
:package
:test
:install
:check_hw
:edk2_setup
:all
:check_prerequisites
/**
 * @file flash_manager.h
 * @brief Flash manager public interface
 */

#ifndef _FLASH_MANAGER_H_
#define _FLASH_MANAGER_H_

#include <Uefi.h>
#include <Library/UefiLib.h>

#define MAX_FLASH_REGIONS 16
#define MAX_FLASH_NAME_LEN 64

typedef enum {
    FLASH_REGION_BOOT_BLOCK,
    FLASH_REGION_MAIN_FIRMWARE,
    FLASH_REGION_NVRAM,
    FLASH_REGION_DESCRIPTOR,
    FLASH_REGION_CUSTOM
} FLASH_REGION_TYPE;

typedef struct {
    FLASH_REGION_TYPE Type;
    UINT32 StartAddress;
    UINT32 Size;
    BOOLEAN WriteProtected;
    BOOLEAN EraseRequired;
    CHAR16 Name[MAX_FLASH_NAME_LEN];
} FLASH_REGION;

typedef struct {
    CHAR16 DeviceName[128];
    UINT32 DeviceId;
    UINT16 VendorId;
    UINT64 TotalSize;
    UINT32 SectorSize;
    BOOLEAN WriteProtected;
    UINT32 BlockCount;
} FLASH_DEVICE_INFO;

// Public API
EFI_STATUS
EFIAPI
flash_manager_init(VOID);

EFI_STATUS
EFIAPI
flash_read(
    IN UINT32 Address,
    OUT VOID *Buffer,
    IN UINTN Size
    );

EFI_STATUS
EFIAPI
flash_write(
    IN UINT32 Address,
    IN CONST VOID *Buffer,
    IN UINTN Size
    );

EFI_STATUS
EFIAPI
flash_erase_sector(
    IN UINT32 Address
    );

EFI_STATUS
EFIAPI
flash_get_device_info(
    OUT FLASH_DEVICE_INFO *FlashInfo
    );

EFI_STATUS
EFIAPI
flash_manager_status(VOID);

EFI_STATUS
EFIAPI
flash_manager_cleanup(VOID);

#endif // _FLASH_MANAGER_H_
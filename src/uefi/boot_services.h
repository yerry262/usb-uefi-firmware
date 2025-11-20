// boot_services.h
// This header file defines the boot services provided by UEFI, including memory management and device handling.

#ifndef _BOOT_SERVICES_H_
#define _BOOT_SERVICES_H_

#include <Uefi.h>
#include <Library/UefiBootServicesTableLib.h>

//
// Boot Services Wrapper Functions
// These provide simplified interfaces to commonly used UEFI boot services
//

//
// Memory Management Wrappers
//

/**
 * Allocate memory with alignment
 * @param MemoryType - Type of memory to allocate
 * @param Size - Size in bytes
 * @param Alignment - Required alignment
 * @param Buffer - Pointer to receive allocated buffer
 * @return EFI_STATUS - Success or error code
 */
EFI_STATUS
EFIAPI
AllocateAlignedMemory(
    IN EFI_MEMORY_TYPE MemoryType,
    IN UINTN Size,
    IN UINTN Alignment,
    OUT VOID **Buffer
    );

/**
 * Free aligned memory
 * @param Buffer - Buffer to free
 * @param Size - Size of buffer
 * @return EFI_STATUS - Success or error code
 */
EFI_STATUS
EFIAPI
FreeAlignedMemory(
    IN VOID *Buffer,
    IN UINTN Size
    );

//
// Protocol Management Wrappers
//

/**
 * Locate and open protocol safely
 * @param Handle - Handle to open protocol on
 * @param Protocol - Protocol GUID
 * @param Interface - Pointer to receive interface
 * @param AgentHandle - Agent handle
 * @param ControllerHandle - Controller handle
 * @param Attributes - Open attributes
 * @return EFI_STATUS - Success or error code
 */
EFI_STATUS
EFIAPI
SafeOpenProtocol(
    IN EFI_HANDLE Handle,
    IN EFI_GUID *Protocol,
    OUT VOID **Interface,
    IN EFI_HANDLE AgentHandle,
    IN EFI_HANDLE ControllerHandle,
    IN UINT32 Attributes
    );

/**
 * Close protocol safely
 * @param Handle - Handle to close protocol on
 * @param Protocol - Protocol GUID
 * @param AgentHandle - Agent handle
 * @param ControllerHandle - Controller handle
 * @return EFI_STATUS - Success or error code
 */
EFI_STATUS
EFIAPI
SafeCloseProtocol(
    IN EFI_HANDLE Handle,
    IN EFI_GUID *Protocol,
    IN EFI_HANDLE AgentHandle,
    IN EFI_HANDLE ControllerHandle
    );

//
// Event Management Wrappers
//

/**
 * Create and signal event
 * @param Type - Event type
 * @param NotifyTpl - Notification TPL
 * @param NotifyFunction - Notification function
 * @param NotifyContext - Notification context
 * @param Event - Pointer to receive event
 * @return EFI_STATUS - Success or error code
 */
EFI_STATUS
EFIAPI
CreateAndSignalEvent(
    IN UINT32 Type,
    IN EFI_TPL NotifyTpl,
    IN EFI_EVENT_NOTIFY NotifyFunction OPTIONAL,
    IN VOID *NotifyContext OPTIONAL,
    OUT EFI_EVENT *Event
    );

/**
 * Create timer event with callback
 * @param NotifyFunction - Timer callback function
 * @param NotifyContext - Timer callback context
 * @param TriggerTime - Timer trigger time
 * @param Event - Pointer to receive event
 * @return EFI_STATUS - Success or error code
 */
EFI_STATUS
EFIAPI
CreateTimerEvent(
    IN EFI_EVENT_NOTIFY NotifyFunction OPTIONAL,
    IN VOID *NotifyContext OPTIONAL,
    IN UINT64 TriggerTime,
    OUT EFI_EVENT *Event
    );

//
// Handle Database Wrappers
//

/**
 * Locate handles by protocol with error checking
 * @param SearchType - Search type
 * @param Protocol - Protocol GUID
 * @param SearchKey - Search key
 * @param NoHandles - Pointer to receive number of handles
 * @param Buffer - Pointer to receive handle buffer
 * @return EFI_STATUS - Success or error code
 */
EFI_STATUS
EFIAPI
SafeLocateHandleBuffer(
    IN EFI_LOCATE_SEARCH_TYPE SearchType,
    IN EFI_GUID *Protocol OPTIONAL,
    IN VOID *SearchKey OPTIONAL,
    OUT UINTN *NoHandles,
    OUT EFI_HANDLE **Buffer
    );

/**
 * Check if handle supports protocol
 * @param Handle - Handle to check
 * @param Protocol - Protocol GUID
 * @return BOOLEAN - TRUE if supported, FALSE otherwise
 */
BOOLEAN
EFIAPI
HandleSupportsProtocol(
    IN EFI_HANDLE Handle,
    IN EFI_GUID *Protocol
    );

//
// Device Path Wrappers
//

/**
 * Get device path from handle
 * @param Handle - Device handle
 * @param DevicePath - Pointer to receive device path
 * @return EFI_STATUS - Success or error code
 */
EFI_STATUS
EFIAPI
GetDevicePathFromHandle(
    IN EFI_HANDLE Handle,
    OUT EFI_DEVICE_PATH_PROTOCOL **DevicePath
    );

/**
 * Compare device paths
 * @param DevicePath1 - First device path
 * @param DevicePath2 - Second device path
 * @return BOOLEAN - TRUE if equal, FALSE otherwise
 */
BOOLEAN
EFIAPI
CompareDevicePaths(
    IN EFI_DEVICE_PATH_PROTOCOL *DevicePath1,
    IN EFI_DEVICE_PATH_PROTOCOL *DevicePath2
    );

//
// Image Management Wrappers
//

/**
 * Get loaded image information
 * @param ImageHandle - Image handle
 * @param LoadedImage - Pointer to receive loaded image protocol
 * @return EFI_STATUS - Success or error code
 */
EFI_STATUS
EFIAPI
GetLoadedImageInfo(
    IN EFI_HANDLE ImageHandle,
    OUT EFI_LOADED_IMAGE_PROTOCOL **LoadedImage
    );

//
// System Table Access Wrappers
//

/**
 * Get console input protocol
 * @return EFI_SIMPLE_TEXT_INPUT_PROTOCOL* - Console input protocol
 */
EFI_SIMPLE_TEXT_INPUT_PROTOCOL*
EFIAPI
GetConsoleInput(
    VOID
    );

/**
 * Get console output protocol  
 * @return EFI_SIMPLE_TEXT_OUTPUT_PROTOCOL* - Console output protocol
 */
EFI_SIMPLE_TEXT_OUTPUT_PROTOCOL*
EFIAPI
GetConsoleOutput(
    VOID
    );

/**
 * Initialize boot services wrappers
 * @return EFI_STATUS - Success or error code
 */
EFI_STATUS
EFIAPI
InitializeBootServicesWrappers(
    VOID
    );

#endif // _BOOT_SERVICES_H_

// Note: Ensure to implement the corresponding functions in the appropriate source file.
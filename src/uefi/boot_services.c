/**
 * @file boot_services.c
 * @brief UEFI Boot Services wrapper implementations
 */

#include <Uefi.h>
#include <Library/UefiLib.h>
#include <Library/UefiBootServicesTableLib.h>
#include <Library/MemoryAllocationLib.h>
#include <Library/BaseMemoryLib.h>
#include <Library/DevicePathLib.h>
#include <Protocol/LoadedImage.h>

#include "boot_services.h"
#include "../../include/common.h"
#include "../../include/debug_utils.h"

/**
 * Initialize boot services wrappers
 * @return EFI_STATUS - Success or error code
 */
EFI_STATUS
EFIAPI
InitializeBootServicesWrappers(VOID)
{
    DBG_ENTER();
    
    // Verify boot services table is available
    if (gBS == NULL) {
        DBG_EXIT_STATUS(EFI_NOT_READY);
        return EFI_NOT_READY;
    }
    
    LOG_INFO("Boot services wrappers initialized\n");
    
    DBG_EXIT_STATUS(EFI_SUCCESS);
    return EFI_SUCCESS;
}

/**
 * Allocate memory with alignment
 */
EFI_STATUS
EFIAPI
AllocateAlignedMemory(
    IN EFI_MEMORY_TYPE MemoryType,
    IN UINTN Size,
    IN UINTN Alignment,
    OUT VOID **Buffer
)
{
    EFI_STATUS Status;
    VOID *Memory;
    UINTN AlignedSize;
    
    if (Buffer == NULL || Size == 0) {
        return EFI_INVALID_PARAMETER;
    }
    
    // Calculate aligned size
    AlignedSize = ALIGN_UP(Size, Alignment);
    
    // Allocate memory
    Status = gBS->AllocatePool(MemoryType, AlignedSize, &Memory);
    if (EFI_ERROR(Status)) {
        return Status;
    }
    
    // Check if already aligned
    if (((UINTN)Memory & (Alignment - 1)) == 0) {
        *Buffer = Memory;
        return EFI_SUCCESS;
    }
    
    // Need to reallocate with proper alignment
    gBS->FreePool(Memory);
    
    // Allocate extra space for alignment
    Status = gBS->AllocatePool(MemoryType, AlignedSize + Alignment, &Memory);
    if (EFI_ERROR(Status)) {
        return Status;
    }
    
    // Calculate aligned address
    *Buffer = (VOID *)ALIGN_UP((UINTN)Memory, Alignment);
    
    return EFI_SUCCESS;
}

/**
 * Free aligned memory
 */
EFI_STATUS
EFIAPI
FreeAlignedMemory(
    IN VOID *Buffer,
    IN UINTN Size
)
{
    if (Buffer == NULL) {
        return EFI_INVALID_PARAMETER;
    }
    
    return gBS->FreePool(Buffer);
}

/**
 * Locate and open protocol safely
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
)
{
    if (Handle == NULL || Protocol == NULL || Interface == NULL) {
        return EFI_INVALID_PARAMETER;
    }
    
    return gBS->OpenProtocol(
        Handle,
        Protocol,
        Interface,
        AgentHandle,
        ControllerHandle,
        Attributes
    );
}

/**
 * Close protocol safely
 */
EFI_STATUS
EFIAPI
SafeCloseProtocol(
    IN EFI_HANDLE Handle,
    IN EFI_GUID *Protocol,
    IN EFI_HANDLE AgentHandle,
    IN EFI_HANDLE ControllerHandle
)
{
    if (Handle == NULL || Protocol == NULL) {
        return EFI_INVALID_PARAMETER;
    }
    
    return gBS->CloseProtocol(
        Handle,
        Protocol,
        AgentHandle,
        ControllerHandle
    );
}

/**
 * Create and signal event
 */
EFI_STATUS
EFIAPI
CreateAndSignalEvent(
    IN UINT32 Type,
    IN EFI_TPL NotifyTpl,
    IN EFI_EVENT_NOTIFY NotifyFunction OPTIONAL,
    IN VOID *NotifyContext OPTIONAL,
    OUT EFI_EVENT *Event
)
{
    EFI_STATUS Status;
    
    if (Event == NULL) {
        return EFI_INVALID_PARAMETER;
    }
    
    Status = gBS->CreateEvent(
        Type,
        NotifyTpl,
        NotifyFunction,
        NotifyContext,
        Event
    );
    
    if (EFI_ERROR(Status)) {
        return Status;
    }
    
    // Signal the event
    return gBS->SignalEvent(*Event);
}

/**
 * Create timer event with callback
 */
EFI_STATUS
EFIAPI
CreateTimerEvent(
    IN EFI_EVENT_NOTIFY NotifyFunction OPTIONAL,
    IN VOID *NotifyContext OPTIONAL,
    IN UINT64 TriggerTime,
    OUT EFI_EVENT *Event
)
{
    EFI_STATUS Status;
    
    if (Event == NULL) {
        return EFI_INVALID_PARAMETER;
    }
    
    Status = gBS->CreateEvent(
        EVT_TIMER | EVT_NOTIFY_SIGNAL,
        TPL_CALLBACK,
        NotifyFunction,
        NotifyContext,
        Event
    );
    
    if (EFI_ERROR(Status)) {
        return Status;
    }
    
    // Set the timer
    return gBS->SetTimer(*Event, TimerRelative, TriggerTime);
}

/**
 * Locate handles by protocol with error checking
 */
EFI_STATUS
EFIAPI
SafeLocateHandleBuffer(
    IN EFI_LOCATE_SEARCH_TYPE SearchType,
    IN EFI_GUID *Protocol OPTIONAL,
    IN VOID *SearchKey OPTIONAL,
    OUT UINTN *NoHandles,
    OUT EFI_HANDLE **Buffer
)
{
    if (NoHandles == NULL || Buffer == NULL) {
        return EFI_INVALID_PARAMETER;
    }
    
    return gBS->LocateHandleBuffer(
        SearchType,
        Protocol,
        SearchKey,
        NoHandles,
        Buffer
    );
}

/**
 * Check if handle supports protocol
 */
BOOLEAN
EFIAPI
HandleSupportsProtocol(
    IN EFI_HANDLE Handle,
    IN EFI_GUID *Protocol
)
{
    EFI_STATUS Status;
    VOID *Interface;
    
    if (Handle == NULL || Protocol == NULL) {
        return FALSE;
    }
    
    Status = gBS->HandleProtocol(Handle, Protocol, &Interface);
    return !EFI_ERROR(Status);
}

/**
 * Get device path from handle
 */
EFI_STATUS
EFIAPI
GetDevicePathFromHandle(
    IN EFI_HANDLE Handle,
    OUT EFI_DEVICE_PATH_PROTOCOL **DevicePath
)
{
    if (Handle == NULL || DevicePath == NULL) {
        return EFI_INVALID_PARAMETER;
    }
    
    return gBS->HandleProtocol(
        Handle,
        &gEfiDevicePathProtocolGuid,
        (VOID **)DevicePath
    );
}

/**
 * Compare device paths
 */
BOOLEAN
EFIAPI
CompareDevicePaths(
    IN EFI_DEVICE_PATH_PROTOCOL *DevicePath1,
    IN EFI_DEVICE_PATH_PROTOCOL *DevicePath2
)
{
    UINTN Size1, Size2;
    
    if (DevicePath1 == NULL && DevicePath2 == NULL) {
        return TRUE;
    }
    
    if (DevicePath1 == NULL || DevicePath2 == NULL) {
        return FALSE;
    }
    
    Size1 = GetDevicePathSize(DevicePath1);
    Size2 = GetDevicePathSize(DevicePath2);
    
    if (Size1 != Size2) {
        return FALSE;
    }
    
    return (CompareMem(DevicePath1, DevicePath2, Size1) == 0);
}

/**
 * Get loaded image information
 */
EFI_STATUS
EFIAPI
GetLoadedImageInfo(
    IN EFI_HANDLE ImageHandle,
    OUT EFI_LOADED_IMAGE_PROTOCOL **LoadedImage
)
{
    if (ImageHandle == NULL || LoadedImage == NULL) {
        return EFI_INVALID_PARAMETER;
    }
    
    return gBS->HandleProtocol(
        ImageHandle,
        &gEfiLoadedImageProtocolGuid,
        (VOID **)LoadedImage
    );
}

/**
 * Get console input protocol
 */
EFI_SIMPLE_TEXT_INPUT_PROTOCOL*
EFIAPI
GetConsoleInput(VOID)
{
    if (gST == NULL) {
        return NULL;
    }
    
    return gST->ConIn;
}

/**
 * Get console output protocol  
 */
EFI_SIMPLE_TEXT_OUTPUT_PROTOCOL*
EFIAPI
GetConsoleOutput(VOID)
{
    if (gST == NULL) {
        return NULL;
    }
    
    return gST->ConOut;
}

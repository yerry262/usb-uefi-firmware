/**
 * @file debug_utils.c
 * @brief Debug utility implementations
 */

#include <Uefi.h>
#include <Library/UefiLib.h>
#include <Library/DebugLib.h>
#include <Library/PrintLib.h>
#include <Library/BaseLib.h>
// Avoid TimerLib dependency; use TSC via BaseLib for lightweight timing

#include "../include/debug_utils.h"
#include "../include/config.h"

//
// Global debug control variables (defined in main.c)
//
extern UINT32 gDebugLevel;
extern UINT32 gDebugCategories;

/**
 * Print hex dump of memory
 */
VOID
EFIAPI
DebugHexDump(
    IN CONST VOID *Data,
    IN UINTN Length,
    IN CONST CHAR8 *Description
)
{
#if ENABLE_USB_DEBUG
    CONST UINT8 *Bytes = (CONST UINT8 *)Data;
    UINTN i, j;
    
    if (Data == NULL || Length == 0) {
        return;
    }
    
    DEBUG((EFI_D_INFO, "=== Hex Dump: %a (%d bytes) ===\n", 
           Description ? Description : "Data", Length));
    
    for (i = 0; i < Length; i += 16) {
        // Print offset
        DEBUG((EFI_D_INFO, "%04X: ", i));
        
        // Print hex values
        for (j = 0; j < 16; j++) {
            if (i + j < Length) {
                DEBUG((EFI_D_INFO, "%02X ", Bytes[i + j]));
            } else {
                DEBUG((EFI_D_INFO, "   "));
            }
            
            if (j == 7) {
                DEBUG((EFI_D_INFO, " "));
            }
        }
        
        DEBUG((EFI_D_INFO, " | "));
        
        // Print ASCII representation
        for (j = 0; j < 16 && i + j < Length; j++) {
            UINT8 c = Bytes[i + j];
            if (c >= 32 && c <= 126) {
                DEBUG((EFI_D_INFO, "%c", c));
            } else {
                DEBUG((EFI_D_INFO, "."));
            }
        }
        
        DEBUG((EFI_D_INFO, "\n"));
    }
    
    DEBUG((EFI_D_INFO, "=========================\n"));
#endif
}

/**
 * Convert status code to string
 */
CONST CHAR16*
EFIAPI
StatusToString(
    IN EFI_STATUS Status
)
{
    switch (Status) {
        case EFI_SUCCESS:
            return L"Success";
        case EFI_LOAD_ERROR:
            return L"Load Error";
        case EFI_INVALID_PARAMETER:
            return L"Invalid Parameter";
        case EFI_UNSUPPORTED:
            return L"Unsupported";
        case EFI_BAD_BUFFER_SIZE:
            return L"Bad Buffer Size";
        case EFI_BUFFER_TOO_SMALL:
            return L"Buffer Too Small";
        case EFI_NOT_READY:
            return L"Not Ready";
        case EFI_DEVICE_ERROR:
            return L"Device Error";
        case EFI_WRITE_PROTECTED:
            return L"Write Protected";
        case EFI_OUT_OF_RESOURCES:
            return L"Out of Resources";
        case EFI_VOLUME_CORRUPTED:
            return L"Volume Corrupted";
        case EFI_VOLUME_FULL:
            return L"Volume Full";
        case EFI_NO_MEDIA:
            return L"No Media";
        case EFI_MEDIA_CHANGED:
            return L"Media Changed";
        case EFI_NOT_FOUND:
            return L"Not Found";
        case EFI_ACCESS_DENIED:
            return L"Access Denied";
        case EFI_NO_RESPONSE:
            return L"No Response";
        case EFI_NO_MAPPING:
            return L"No Mapping";
        case EFI_TIMEOUT:
            return L"Timeout";
        case EFI_NOT_STARTED:
            return L"Not Started";
        case EFI_ALREADY_STARTED:
            return L"Already Started";
        case EFI_ABORTED:
            return L"Aborted";
        case EFI_PROTOCOL_ERROR:
            return L"Protocol Error";
        case EFI_INCOMPATIBLE_VERSION:
            return L"Incompatible Version";
        case EFI_SECURITY_VIOLATION:
            return L"Security Violation";
        default:
            return L"Unknown Error";
    }
}

/**
 * Debug print with level and category filtering
 */
VOID
EFIAPI
DebugPrint(
    IN UINT32 Level,
    IN UINT32 Category,
    IN CONST CHAR8 *Format,
    ...
)
{
#if ENABLE_USB_DEBUG
    VA_LIST Marker;
    CHAR8 Buffer[DEBUG_BUFFER_SIZE];
    
    // Check if this message should be printed
    if ((gDebugLevel & Level) == 0) {
        return;
    }
    
    if ((gDebugCategories & Category) == 0) {
        return;
    }
    
    // Format the message
    VA_START(Marker, Format);
    AsciiVSPrint(Buffer, sizeof(Buffer), Format, Marker);
    VA_END(Marker);
    
    // Print based on level
    if (Level & DEBUG_LEVEL_ERROR) {
        DEBUG((EFI_D_ERROR, "%a", Buffer));
    } else if (Level & DEBUG_LEVEL_WARN) {
        DEBUG((EFI_D_WARN, "%a", Buffer));
    } else if (Level & DEBUG_LEVEL_INFO) {
        DEBUG((EFI_D_INFO, "%a", Buffer));
    } else {
        DEBUG((EFI_D_VERBOSE, "%a", Buffer));
    }
#endif
}

/**
 * Start performance timer
 */
VOID
EFIAPI
DebugTimerStart(
    IN OUT DEBUG_TIMER *Timer,
    IN CONST CHAR8 *Description
)
{
#if ENABLE_USB_DEBUG
    if (Timer == NULL) {
        return;
    }
    
    Timer->Description = Description;
    Timer->StartTick = AsmReadTsc();
    Timer->EndTick = 0;
    
    DBG_USB_VERBOSE("Timer started: %a\n", Description ? Description : "Unknown");
#endif
}

/**
 * End performance timer and print results
 */
VOID
EFIAPI
DebugTimerEnd(
    IN OUT DEBUG_TIMER *Timer
)
{
#if ENABLE_USB_DEBUG
    UINT64 ElapsedTicks;
    
    if (Timer == NULL || Timer->StartTick == 0) {
        return;
    }
    
    Timer->EndTick = AsmReadTsc();
    ElapsedTicks = Timer->EndTick - Timer->StartTick;
    DBG_USB_INFO("Timer ended: %a - Elapsed: %ld ticks\n",
                Timer->Description ? Timer->Description : "Unknown",
                ElapsedTicks);
#endif
}

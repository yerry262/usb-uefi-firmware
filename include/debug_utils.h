#ifndef _DEBUG_UTILS_H_
#define _DEBUG_UTILS_H_

#include <Uefi.h>
#include <Library/DebugLib.h>
#include <Library/PrintLib.h>
#include "config.h"

//
// Debug Level Definitions
//
#define DEBUG_LEVEL_NONE            0x00000000
#define DEBUG_LEVEL_ERROR           0x00000001
#define DEBUG_LEVEL_WARN            0x00000002
#define DEBUG_LEVEL_INFO            0x00000004
#define DEBUG_LEVEL_VERBOSE         0x00000008
#define DEBUG_LEVEL_ALL             0xFFFFFFFF

//
// Debug Categories
//
#define DEBUG_CAT_USB               0x00000010
#define DEBUG_CAT_FIRMWARE          0x00000020
#define DEBUG_CAT_UEFI              0x00000040
#define DEBUG_CAT_BOOT              0x00000080
#define DEBUG_CAT_NETWORK           0x00000100

//
// Global debug control
//
extern UINT32 gDebugLevel;
extern UINT32 gDebugCategories;

//
// Enhanced debug macros
//
#if ENABLE_USB_DEBUG

#define DBG_ENTER()                 DebugPrint(DEBUG_LEVEL_VERBOSE, DEBUG_CAT_USB, "ENTER: %a\n", __FUNCTION__)
#define DBG_EXIT()                  DebugPrint(DEBUG_LEVEL_VERBOSE, DEBUG_CAT_USB, "EXIT:  %a\n", __FUNCTION__)
#define DBG_EXIT_STATUS(Status)     DebugPrint(DEBUG_LEVEL_VERBOSE, DEBUG_CAT_USB, "EXIT:  %a - Status: %r\n", __FUNCTION__, Status)

#define DBG_USB_ERROR(...)          DebugPrint(DEBUG_LEVEL_ERROR, DEBUG_CAT_USB, __VA_ARGS__)
#define DBG_USB_WARN(...)           DebugPrint(DEBUG_LEVEL_WARN, DEBUG_CAT_USB, __VA_ARGS__)
#define DBG_USB_INFO(...)           DebugPrint(DEBUG_LEVEL_INFO, DEBUG_CAT_USB, __VA_ARGS__)
#define DBG_USB_VERBOSE(...)        DebugPrint(DEBUG_LEVEL_VERBOSE, DEBUG_CAT_USB, __VA_ARGS__)

#define DBG_FIRMWARE_ERROR(...)     DebugPrint(DEBUG_LEVEL_ERROR, DEBUG_CAT_FIRMWARE, __VA_ARGS__)
#define DBG_FIRMWARE_INFO(...)      DebugPrint(DEBUG_LEVEL_INFO, DEBUG_CAT_FIRMWARE, __VA_ARGS__)

#define DBG_UEFI_ERROR(...)         DebugPrint(DEBUG_LEVEL_ERROR, DEBUG_CAT_UEFI, __VA_ARGS__)
#define DBG_UEFI_INFO(...)          DebugPrint(DEBUG_LEVEL_INFO, DEBUG_CAT_UEFI, __VA_ARGS__)

#else

#define DBG_ENTER()
#define DBG_EXIT()
#define DBG_EXIT_STATUS(Status)
#define DBG_USB_ERROR(...)
#define DBG_USB_WARN(...)
#define DBG_USB_INFO(...)
#define DBG_USB_VERBOSE(...)
#define DBG_FIRMWARE_ERROR(...)
#define DBG_FIRMWARE_INFO(...)
#define DBG_UEFI_ERROR(...)
#define DBG_UEFI_INFO(...)

#endif

//
// Memory dump utilities
//
VOID
EFIAPI
DebugHexDump(
    IN CONST VOID *Data,
    IN UINTN Length,
    IN CONST CHAR8 *Description
    );

//
// Status code to string conversion
//
CONST CHAR16*
EFIAPI
StatusToString(
    IN EFI_STATUS Status
    );

//
// Debug print with level and category filtering
//
VOID
EFIAPI
DebugPrint(
    IN UINT32 Level,
    IN UINT32 Category,
    IN CONST CHAR8 *Format,
    ...
    );

//
// Assert macros for debugging
//
#if ENABLE_USB_DEBUG
#define DBG_ASSERT(Expression) \
    do { \
        if (!(Expression)) { \
            DebugPrint(DEBUG_LEVEL_ERROR, DEBUG_CAT_USB, \
                "ASSERT FAILED: %a:%d - %a\n", __FILE__, __LINE__, #Expression); \
            CpuDeadLoop(); \
        } \
    } while (0)
#else
#define DBG_ASSERT(Expression)
#endif

//
// Performance measurement utilities
//
typedef struct {
    UINT64 StartTick;
    UINT64 EndTick;
    CONST CHAR8 *Description;
} DEBUG_TIMER;

VOID
EFIAPI
DebugTimerStart(
    IN OUT DEBUG_TIMER *Timer,
    IN CONST CHAR8 *Description
    );

VOID
EFIAPI
DebugTimerEnd(
    IN OUT DEBUG_TIMER *Timer
    );

#endif // _DEBUG_UTILS_H_
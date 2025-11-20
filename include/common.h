// common.h - Common definitions and utility functions for the USB-based UEFI firmware interface

#ifndef _COMMON_H_
#define _COMMON_H_

#include <Uefi.h>
#include <Library/UefiLib.h>
#include <Library/DebugLib.h>
#include <Library/MemoryAllocationLib.h>
#include <Library/BaseMemoryLib.h>
#include <Library/UefiBootServicesTableLib.h>
#include <Library/UefiRuntimeServicesTableLib.h>

//
// Common Status Codes
//
#define STATUS_SUCCESS              EFI_SUCCESS
#define STATUS_ERROR                EFI_ABORTED
#define STATUS_INVALID_PARAMETER    EFI_INVALID_PARAMETER
#define STATUS_NOT_FOUND            EFI_NOT_FOUND
#define STATUS_NOT_READY            EFI_NOT_READY

//
// Common Macros
//
#define ARRAY_SIZE(x)               (sizeof(x) / sizeof((x)[0]))
#define MIN(a, b)                   ((a) < (b) ? (a) : (b))
#define MAX(a, b)                   ((a) > (b) ? (a) : (b))
#define ALIGN_UP(x, align)          (((x) + ((align) - 1)) & ~((align) - 1))
#define ALIGN_DOWN(x, align)        ((x) & ~((align) - 1))

//
// Memory allocation wrappers
//
#define AllocateMemory(Size)        AllocatePool(Size)
#define FreeMemory(Buffer)          FreePool(Buffer)
#define ZeroMemory(Buffer, Size)    ZeroMem(Buffer, Size)
#define CopyMemory(Dest, Src, Size) CopyMem(Dest, Src, Size)

// Global handles (declared in main.c)
extern EFI_HANDLE gImageHandle;
extern EFI_SYSTEM_TABLE *gST;

//
// Debug output macros
//
#define LOG_INFO(...)               DEBUG((EFI_D_INFO, __VA_ARGS__))
#define LOG_WARN(...)               DEBUG((EFI_D_WARN, __VA_ARGS__))
#define LOG_ERROR(...)              DEBUG((EFI_D_ERROR, __VA_ARGS__))

//
// String manipulation macros
//
#define STRING_EQUAL(s1, s2)        (StrCmp(s1, s2) == 0)
#define STRING_LENGTH(s)            StrLen(s)

//
// Bit manipulation macros
//
#define SET_BIT(value, bit)         ((value) |= (1 << (bit)))
#define CLEAR_BIT(value, bit)       ((value) &= ~(1 << (bit)))
#define TEST_BIT(value, bit)        (((value) & (1 << (bit))) != 0)

//
// Common return value check
//
#define CHECK_STATUS(status, msg) \
    do { \
        if (EFI_ERROR(status)) { \
            LOG_ERROR(msg ": %r\n", status); \
            return status; \
        } \
    } while (0)

//
// Null pointer check
//
#define CHECK_NULL(ptr, status) \
    do { \
        if ((ptr) == NULL) { \
            LOG_ERROR("Null pointer detected\n"); \
            return (status); \
        } \
    } while (0)

#endif // _COMMON_H_
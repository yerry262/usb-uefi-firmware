/**
 * @file test_runner.h
 * @brief Test Framework Header
 */

#ifndef _TEST_RUNNER_H_
#define _TEST_RUNNER_H_

#include <Uefi.h>

//
// Test Statistics Structure
//
typedef struct {
    UINTN TotalTests;
    UINTN PassedTests;
    UINTN FailedTests;
} TEST_STATS;

//
// Test Function Prototypes
//
EFI_STATUS
EFIAPI
RunUsbTests(VOID);

EFI_STATUS
EFIAPI
RunUefiTests(VOID);

EFI_STATUS
EFIAPI
RunFlashTests(VOID);

EFI_STATUS
EFIAPI
RunErrorHandlingTests(VOID);

EFI_STATUS
EFIAPI
RunHardwareCompatibilityTests(VOID);

EFI_STATUS
EFIAPI
RunAllTests(VOID);

#endif // _TEST_RUNNER_H_
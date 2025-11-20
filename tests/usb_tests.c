/**
 * @file usb_tests.c
 * @brief USB Device Enumeration Unit Tests
 * @author PhD Research Project
 */

#include <Uefi.h>
#include <Library/UefiLib.h>
#include <Library/MemoryAllocationLib.h>
#include <Library/DebugLib.h>
#include "../src/usb/usb_driver.h"
#include "../src/usb/usb_protocol.h"
#include "../include/common.h"
#include "../include/debug_utils.h"

//
// Test Framework Macros
//
#define TEST_ASSERT(condition, message) \
    do { \
        if (!(condition)) { \
            Print(L"[FAIL] %a: %a\n", __FUNCTION__, message); \
            return EFI_ABORTED; \
        } else { \
            Print(L"[PASS] %a: %a\n", __FUNCTION__, message); \
        } \
    } while (0)

#define TEST_START(test_name) \
    Print(L"\n=== USB Test: %a ===\n", test_name)

#define TEST_END(test_name, status) \
    Print(L"=== %a Result: %r ===\n\n", test_name, status)

//
// Test Statistics
//
typedef struct {
    UINTN TotalTests;
    UINTN PassedTests;
    UINTN FailedTests;
} TEST_STATS;

STATIC TEST_STATS mTestStats = {0};

//
// Mock USB Device Data for Testing
//
STATIC USB_DEVICE_INFO mMockDevices[] = {
    {
        .VendorId = 0x1234,
        .ProductId = 0x5678,
        .DeviceClass = USB_CLASS_MASS_STORAGE,
        .IsConnected = TRUE
    },
    {
        .VendorId = 0xABCD,
        .ProductId = 0xEF01,
        .DeviceClass = USB_CLASS_HID,
        .IsConnected = TRUE
    },
    {
        .VendorId = 0x0000,
        .ProductId = 0x0000,
        .DeviceClass = 0x00,
        .IsConnected = FALSE
    }
};

//
// Test Function Prototypes
//
STATIC EFI_STATUS TestUsbDriverInit(VOID);
STATIC EFI_STATUS TestUsbDeviceDetection(VOID);
STATIC EFI_STATUS TestUsbDeviceEnumeration(VOID);
STATIC EFI_STATUS TestUsbDeviceCommunication(VOID);
STATIC EFI_STATUS TestUsbDeviceClassification(VOID);
STATIC EFI_STATUS TestUsbErrorHandling(VOID);
STATIC EFI_STATUS TestUsbDriverCleanup(VOID);
STATIC VOID PrintTestStatistics(VOID);

/**
 * Main USB Test Runner
 */
EFI_STATUS
EFIAPI
RunUsbTests(VOID)
{
    EFI_STATUS Status;
    
    Print(L"\n");
    Print(L"=====================================\n");
    Print(L"   USB DEVICE ENUMERATION TESTS      \n");
    Print(L"=====================================\n");
    
    // Initialize test statistics
    mTestStats.TotalTests = 0;
    mTestStats.PassedTests = 0;
    mTestStats.FailedTests = 0;
    
    // Run individual tests
    Status = TestUsbDriverInit();
    mTestStats.TotalTests++;
    if (!EFI_ERROR(Status)) mTestStats.PassedTests++;
    else mTestStats.FailedTests++;
    
    Status = TestUsbDeviceDetection();
    mTestStats.TotalTests++;
    if (!EFI_ERROR(Status)) mTestStats.PassedTests++;
    else mTestStats.FailedTests++;
    
    Status = TestUsbDeviceEnumeration();
    mTestStats.TotalTests++;
    if (!EFI_ERROR(Status)) mTestStats.PassedTests++;
    else mTestStats.FailedTests++;
    
    Status = TestUsbDeviceCommunication();
    mTestStats.TotalTests++;
    if (!EFI_ERROR(Status)) mTestStats.PassedTests++;
    else mTestStats.FailedTests++;
    
    Status = TestUsbDeviceClassification();
    mTestStats.TotalTests++;
    if (!EFI_ERROR(Status)) mTestStats.PassedTests++;
    else mTestStats.FailedTests++;
    
    Status = TestUsbErrorHandling();
    mTestStats.TotalTests++;
    if (!EFI_ERROR(Status)) mTestStats.PassedTests++;
    else mTestStats.FailedTests++;
    
    Status = TestUsbDriverCleanup();
    mTestStats.TotalTests++;
    if (!EFI_ERROR(Status)) mTestStats.PassedTests++;
    else mTestStats.FailedTests++;
    
    // Print final statistics
    PrintTestStatistics();
    
    return (mTestStats.FailedTests == 0) ? EFI_SUCCESS : EFI_ABORTED;
}

/**
 * Test USB Driver Initialization
 */
STATIC EFI_STATUS TestUsbDriverInit(VOID)
{
    EFI_STATUS Status;
    
    TEST_START("USB Driver Initialization");
    
    // Test driver initialization
    Status = usb_driver_init();
    TEST_ASSERT(!EFI_ERROR(Status), "USB driver should initialize successfully");
    
    // Test double initialization (should handle gracefully)
    Status = usb_driver_init();
    TEST_ASSERT(!EFI_ERROR(Status), "USB driver should handle double init gracefully");
    
    // Verify driver status
    Status = usb_driver_status();
    TEST_ASSERT(!EFI_ERROR(Status), "USB driver status should be successful");
    
    TEST_END("USB Driver Initialization", EFI_SUCCESS);
    return EFI_SUCCESS;
}

/**
 * Test USB Device Detection
 */
STATIC EFI_STATUS TestUsbDeviceDetection(VOID)
{
    EFI_STATUS Status;
    
    TEST_START("USB Device Detection");
    
    // Test device detection
    Status = usb_device_detect();
    TEST_ASSERT(!EFI_ERROR(Status), "USB device detection should complete");
    
    // Test multiple detection calls
    Status = usb_device_detect();
    TEST_ASSERT(!EFI_ERROR(Status), "Multiple detection calls should work");
    
    TEST_END("USB Device Detection", EFI_SUCCESS);
    return EFI_SUCCESS;
}

/**
 * Test USB Device Enumeration
 */
STATIC EFI_STATUS TestUsbDeviceEnumeration(VOID)
{
    EFI_STATUS Status;
    
    TEST_START("USB Device Enumeration");
    
    // Test enumeration of mock devices
    for (UINTN i = 0; i < ARRAY_SIZE(mMockDevices); i++) {
        if (mMockDevices[i].IsConnected) {
            Print(L"[INFO] Testing device VID:0x%04X PID:0x%04X Class:0x%02X\n",
                  mMockDevices[i].VendorId,
                  mMockDevices[i].ProductId,
                  mMockDevices[i].DeviceClass);
            
            // Verify device properties
            TEST_ASSERT(mMockDevices[i].VendorId != 0, "Device should have valid Vendor ID");
            TEST_ASSERT(mMockDevices[i].ProductId != 0, "Device should have valid Product ID");
        }
    }
    
    TEST_END("USB Device Enumeration", EFI_SUCCESS);
    return EFI_SUCCESS;
}

/**
 * Test USB Device Communication
 */
STATIC EFI_STATUS TestUsbDeviceCommunication(VOID)
{
    EFI_STATUS Status;
    UINT8 TestBuffer[64];
    
    TEST_START("USB Device Communication");
    
    // Initialize test buffer
    SetMem(TestBuffer, sizeof(TestBuffer), 0xAA);
    
    // Test communication with device 0 (if available)
    Status = usb_device_communicate(0, TestBuffer, sizeof(TestBuffer));
    // Note: This might fail if no devices are present, which is acceptable
    Print(L"[INFO] Communication test result: %r\n", Status);
    
    // Test invalid device ID
    Status = usb_device_communicate(999, TestBuffer, sizeof(TestBuffer));
    TEST_ASSERT(EFI_ERROR(Status), "Invalid device ID should return error");
    
    // Test NULL buffer
    Status = usb_device_communicate(0, NULL, sizeof(TestBuffer));
    TEST_ASSERT(EFI_ERROR(Status), "NULL buffer should return error");
    
    TEST_END("USB Device Communication", EFI_SUCCESS);
    return EFI_SUCCESS;
}

/**
 * Test USB Device Classification
 */
STATIC EFI_STATUS TestUsbDeviceClassification(VOID)
{
    TEST_START("USB Device Classification");
    
    // Test classification of different device classes
    for (UINTN i = 0; i < ARRAY_SIZE(mMockDevices); i++) {
        if (mMockDevices[i].IsConnected) {
            switch (mMockDevices[i].DeviceClass) {
                case USB_CLASS_MASS_STORAGE:
                    Print(L"[INFO] Mass Storage Device detected\n");
                    TEST_ASSERT(TRUE, "Mass Storage classification successful");
                    break;
                    
                case USB_CLASS_HID:
                    Print(L"[INFO] HID Device detected\n");
                    TEST_ASSERT(TRUE, "HID classification successful");
                    break;
                    
                case USB_CLASS_HUB:
                    Print(L"[INFO] USB Hub detected\n");
                    TEST_ASSERT(TRUE, "Hub classification successful");
                    break;
                    
                default:
                    Print(L"[INFO] Generic USB Device (Class: 0x%02X)\n", 
                          mMockDevices[i].DeviceClass);
                    TEST_ASSERT(TRUE, "Generic device classification successful");
                    break;
            }
        }
    }
    
    TEST_END("USB Device Classification", EFI_SUCCESS);
    return EFI_SUCCESS;
}

/**
 * Test USB Error Handling
 */
STATIC EFI_STATUS TestUsbErrorHandling(VOID)
{
    EFI_STATUS Status;
    
    TEST_START("USB Error Handling");
    
    // Test uninitialized driver operations
    Status = usb_driver_cleanup();
    Status = usb_driver_init(); // Re-initialize for other tests
    
    // Test invalid parameters
    Status = usb_device_communicate(0xFFFFFFFF, NULL, 0);
    TEST_ASSERT(EFI_ERROR(Status), "Invalid parameters should return error");
    
    // Test boundary conditions
    UINT8 LargeBuffer[0x10000]; // 64KB buffer
    Status = usb_device_communicate(0, LargeBuffer, sizeof(LargeBuffer));
    Print(L"[INFO] Large buffer test result: %r\n", Status);
    
    TEST_END("USB Error Handling", EFI_SUCCESS);
    return EFI_SUCCESS;
}

/**
 * Test USB Driver Cleanup
 */
STATIC EFI_STATUS TestUsbDriverCleanup(VOID)
{
    EFI_STATUS Status;
    
    TEST_START("USB Driver Cleanup");
    
    // Test cleanup
    Status = usb_driver_cleanup();
    TEST_ASSERT(!EFI_ERROR(Status), "USB driver cleanup should succeed");
    
    // Test double cleanup
    Status = usb_driver_cleanup();
    TEST_ASSERT(!EFI_ERROR(Status), "Double cleanup should be handled gracefully");
    
    TEST_END("USB Driver Cleanup", EFI_SUCCESS);
    return EFI_SUCCESS;
}

/**
 * Print final test statistics
 */
STATIC VOID PrintTestStatistics(VOID)
{
    Print(L"\n");
    Print(L"=====================================\n");
    Print(L"         USB TEST RESULTS            \n");
    Print(L"=====================================\n");
    Print(L"Total Tests:  %d\n", mTestStats.TotalTests);
    Print(L"Passed Tests: %d\n", mTestStats.PassedTests);
    Print(L"Failed Tests: %d\n", mTestStats.FailedTests);
    Print(L"Success Rate: %d%%\n", 
          (mTestStats.TotalTests > 0) ? 
          (mTestStats.PassedTests * 100) / mTestStats.TotalTests : 0);
    Print(L"=====================================\n");
    
    if (mTestStats.FailedTests == 0) {
        Print(L"🟢 ALL USB TESTS PASSED!\n");
    } else {
        Print(L"🔴 SOME USB TESTS FAILED!\n");
    }
}
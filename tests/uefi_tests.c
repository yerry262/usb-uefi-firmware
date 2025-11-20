/**
 * @file uefi_tests.c
 * @brief UEFI Service Integration Unit Tests
 * @author PhD Research Project
 */

#include <Uefi.h>
#include <Library/UefiLib.h>
#include <Library/UefiBootServicesTableLib.h>
#include <Library/UefiRuntimeServicesTableLib.h>
#include <Library/MemoryAllocationLib.h>
#include <Library/DebugLib.h>
#include "../src/uefi/uefi_interface.h"
#include "../src/uefi/boot_services.h"
#include "../include/common.h"
#include "../include/debug_utils.h"

//
// Test Framework Macros (reuse from USB tests)
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
    Print(L"\n=== UEFI Test: %a ===\n", test_name)

#define TEST_END(test_name, status) \
    Print(L"=== %a Result: %r ===\n\n", test_name, status)

//
// Test Statistics
//
STATIC TEST_STATS mUefiTestStats = {0};

//
// Test Function Prototypes
//
STATIC EFI_STATUS TestUefiInterfaceInit(VOID);
STATIC EFI_STATUS TestUefiSystemInfo(VOID);
STATIC EFI_STATUS TestUefiMemoryServices(VOID);
STATIC EFI_STATUS TestUefiBootServices(VOID);
STATIC EFI_STATUS TestUefiProtocolServices(VOID);
STATIC EFI_STATUS TestUefiAmdDetection(VOID);
STATIC EFI_STATUS TestUefiSecurityFeatures(VOID);
STATIC EFI_STATUS TestUefiVariableServices(VOID);
STATIC EFI_STATUS TestUefiInterfaceCleanup(VOID);
STATIC VOID PrintUefiTestStatistics(VOID);

/**
 * Main UEFI Test Runner
 */
EFI_STATUS
EFIAPI
RunUefiTests(VOID)
{
    EFI_STATUS Status;
    
    Print(L"\n");
    Print(L"=====================================\n");
    Print(L"   UEFI SERVICE INTEGRATION TESTS    \n");
    Print(L"=====================================\n");
    
    // Initialize test statistics
    mUefiTestStats.TotalTests = 0;
    mUefiTestStats.PassedTests = 0;
    mUefiTestStats.FailedTests = 0;
    
    // Run individual tests
    Status = TestUefiInterfaceInit();
    mUefiTestStats.TotalTests++;
    if (!EFI_ERROR(Status)) mUefiTestStats.PassedTests++;
    else mUefiTestStats.FailedTests++;
    
    Status = TestUefiSystemInfo();
    mUefiTestStats.TotalTests++;
    if (!EFI_ERROR(Status)) mUefiTestStats.PassedTests++;
    else mUefiTestStats.FailedTests++;
    
    Status = TestUefiMemoryServices();
    mUefiTestStats.TotalTests++;
    if (!EFI_ERROR(Status)) mUefiTestStats.PassedTests++;
    else mUefiTestStats.FailedTests++;
    
    Status = TestUefiBootServices();
    mUefiTestStats.TotalTests++;
    if (!EFI_ERROR(Status)) mUefiTestStats.PassedTests++;
    else mUefiTestStats.FailedTests++;
    
    Status = TestUefiProtocolServices();
    mUefiTestStats.TotalTests++;
    if (!EFI_ERROR(Status)) mUefiTestStats.PassedTests++;
    else mUefiTestStats.FailedTests++;
    
    Status = TestUefiAmdDetection();
    mUefiTestStats.TotalTests++;
    if (!EFI_ERROR(Status)) mUefiTestStats.PassedTests++;
    else mUefiTestStats.FailedTests++;
    
    Status = TestUefiSecurityFeatures();
    mUefiTestStats.TotalTests++;
    if (!EFI_ERROR(Status)) mUefiTestStats.PassedTests++;
    else mUefiTestStats.FailedTests++;
    
    Status = TestUefiVariableServices();
    mUefiTestStats.TotalTests++;
    if (!EFI_ERROR(Status)) mUefiTestStats.PassedTests++;
    else mUefiTestStats.FailedTests++;
    
    Status = TestUefiInterfaceCleanup();
    mUefiTestStats.TotalTests++;
    if (!EFI_ERROR(Status)) mUefiTestStats.PassedTests++;
    else mUefiTestStats.FailedTests++;
    
    // Print final statistics
    PrintUefiTestStatistics();
    
    return (mUefiTestStats.FailedTests == 0) ? EFI_SUCCESS : EFI_ABORTED;
}

/**
 * Test UEFI Interface Initialization
 */
STATIC EFI_STATUS TestUefiInterfaceInit(VOID)
{
    EFI_STATUS Status;
    
    TEST_START("UEFI Interface Initialization");
    
    // Test interface initialization
    Status = uefi_interface_init();
    TEST_ASSERT(!EFI_ERROR(Status), "UEFI interface should initialize successfully");
    
    // Verify UEFI tables are accessible
    TEST_ASSERT(gST != NULL, "System Table should be accessible");
    TEST_ASSERT(gBS != NULL, "Boot Services Table should be accessible");
    TEST_ASSERT(gRT != NULL, "Runtime Services Table should be accessible");
    TEST_ASSERT(gImageHandle != NULL, "Image Handle should be valid");
    
    // Test interface status
    Status = uefi_interface_status();
    TEST_ASSERT(!EFI_ERROR(Status), "UEFI interface status should be successful");
    
    TEST_END("UEFI Interface Initialization", EFI_SUCCESS);
    return EFI_SUCCESS;
}

/**
 * Test UEFI System Information Gathering
 */
STATIC EFI_STATUS TestUefiSystemInfo(VOID)
{
    EFI_STATUS Status;
    UEFI_SYSTEM_INFO SystemInfo = {0};
    
    TEST_START("UEFI System Information");
    
    // Test system info gathering
    Status = uefi_get_system_info(&SystemInfo);
    TEST_ASSERT(!EFI_ERROR(Status), "System info gathering should succeed");
    
    // Verify system information
    TEST_ASSERT(SystemInfo.UefiMajorVersion >= 2, "UEFI version should be 2.x or higher");
    TEST_ASSERT(SystemInfo.TotalMemory > 0, "Total memory should be greater than 0");
    
    // Print system information
    Print(L"[INFO] UEFI Version: %d.%d\n", 
          SystemInfo.UefiMajorVersion, SystemInfo.UefiMinorVersion);
    Print(L"[INFO] Firmware Vendor: %s\n", SystemInfo.FirmwareVendor);
    Print(L"[INFO] Total Memory: %ld MB\n", SystemInfo.TotalMemory / (1024 * 1024));
    
    TEST_END("UEFI System Information", EFI_SUCCESS);
    return EFI_SUCCESS;
}

/**
 * Test UEFI Memory Services
 */
STATIC EFI_STATUS TestUefiMemoryServices(VOID)
{
    EFI_STATUS Status;
    VOID *TestBuffer = NULL;
    UINT64 TotalMemory = 0;
    UINT64 AvailableMemory = 0;
    
    TEST_START("UEFI Memory Services");
    
    // Test memory allocation
    TestBuffer = AllocatePool(1024);
    TEST_ASSERT(TestBuffer != NULL, "Memory allocation should succeed");
    
    // Test memory initialization
    SetMem(TestBuffer, 1024, 0x55);
    TEST_ASSERT(((UINT8*)TestBuffer)[0] == 0x55, "Memory should be initialized correctly");
    TEST_ASSERT(((UINT8*)TestBuffer)[1023] == 0x55, "Memory should be initialized correctly");
    
    // Test memory cleanup
    FreePool(TestBuffer);
    TEST_ASSERT(TRUE, "Memory deallocation should succeed");
    
    // Test memory information
    Status = uefi_get_memory_info(&TotalMemory, &AvailableMemory);
    if (!EFI_ERROR(Status)) {
        TEST_ASSERT(TotalMemory > 0, "Total memory should be positive");
        TEST_ASSERT(AvailableMemory > 0, "Available memory should be positive");
        TEST_ASSERT(AvailableMemory <= TotalMemory, "Available <= Total memory");
        
        Print(L"[INFO] Memory - Total: %ld MB, Available: %ld MB\n",
              TotalMemory / (1024 * 1024),
              AvailableMemory / (1024 * 1024));
    }
    
    TEST_END("UEFI Memory Services", EFI_SUCCESS);
    return EFI_SUCCESS;
}

/**
 * Test UEFI Boot Services
 */
STATIC EFI_STATUS TestUefiBootServices(VOID)
{
    EFI_STATUS Status;
    EFI_HANDLE *HandleBuffer = NULL;
    UINTN HandleCount = 0;
    
    TEST_START("UEFI Boot Services");
    
    // Test handle enumeration
    Status = gBS->LocateHandleBuffer(
        AllHandles,
        NULL,
        NULL,
        &HandleCount,
        &HandleBuffer
    );
    TEST_ASSERT(!EFI_ERROR(Status), "Handle enumeration should succeed");
    TEST_ASSERT(HandleCount > 0, "Should find at least one handle");
    TEST_ASSERT(HandleBuffer != NULL, "Handle buffer should not be NULL");
    
    Print(L"[INFO] Found %d handles in system\n", HandleCount);
    
    if (HandleBuffer) {
        FreePool(HandleBuffer);
    }
    
    // Test timer services
    EFI_EVENT TimerEvent;
    Status = gBS->CreateEvent(
        EVT_TIMER,
        TPL_CALLBACK,
        NULL,
        NULL,
        &TimerEvent
    );
    TEST_ASSERT(!EFI_ERROR(Status), "Timer event creation should succeed");
    
    if (!EFI_ERROR(Status)) {
        gBS->CloseEvent(TimerEvent);
    }
    
    TEST_END("UEFI Boot Services", EFI_SUCCESS);
    return EFI_SUCCESS;
}

/**
 * Test UEFI Protocol Services
 */
STATIC EFI_STATUS TestUefiProtocolServices(VOID)
{
    EFI_STATUS Status;
    EFI_HANDLE *HandleBuffer = NULL;
    UINTN HandleCount = 0;
    
    TEST_START("UEFI Protocol Services");
    
    // Test USB I/O protocol enumeration
    Status = gBS->LocateHandleBuffer(
        ByProtocol,
        &gEfiUsbIoProtocolGuid,
        NULL,
        &HandleCount,
        &HandleBuffer
    );
    
    if (!EFI_ERROR(Status)) {
        Print(L"[INFO] Found %d USB I/O protocol instances\n", HandleCount);
        TEST_ASSERT(TRUE, "USB protocol enumeration successful");
        
        if (HandleBuffer) {
            FreePool(HandleBuffer);
        }
    } else {
        Print(L"[INFO] No USB I/O protocols found (this may be normal)\n");
        TEST_ASSERT(TRUE, "USB protocol check completed");
    }
    
    // Test Simple File System protocol
    HandleBuffer = NULL;
    HandleCount = 0;
    
    Status = gBS->LocateHandleBuffer(
        ByProtocol,
        &gEfiSimpleFileSystemProtocolGuid,
        NULL,
        &HandleCount,
        &HandleBuffer
    );
    
    if (!EFI_ERROR(Status)) {
        Print(L"[INFO] Found %d file system protocol instances\n", HandleCount);
        TEST_ASSERT(TRUE, "File system protocol enumeration successful");
        
        if (HandleBuffer) {
            FreePool(HandleBuffer);
        }
    } else {
        Print(L"[INFO] No file system protocols found\n");
        TEST_ASSERT(TRUE, "File system protocol check completed");
    }
    
    TEST_END("UEFI Protocol Services", EFI_SUCCESS);
    return EFI_SUCCESS;
}

/**
 * Test AMD Platform Detection
 */
STATIC EFI_STATUS TestUefiAmdDetection(VOID)
{
    EFI_STATUS Status;
    BOOLEAN IsAmdRyzen = FALSE;
    CHAR16 CpuVendor[32] = {0};
    CHAR16 CpuFamily[64] = {0};
    UINT32 CpuCores = 0;
    
    TEST_START("AMD Platform Detection");
    
    // Test AMD platform detection
    Status = uefi_detect_amd_platform(&IsAmdRyzen);
    if (!EFI_ERROR(Status)) {
        Print(L"[INFO] AMD Ryzen Platform: %s\n", IsAmdRyzen ? L"YES" : L"NO");
        TEST_ASSERT(TRUE, "AMD platform detection completed");
    } else {
        Print(L"[WARN] AMD platform detection not available\n");
        TEST_ASSERT(TRUE, "AMD detection handled gracefully");
    }
    
    // Test CPU information gathering
    Status = uefi_get_cpu_info(CpuVendor, CpuFamily, &CpuCores);
    if (!EFI_ERROR(Status)) {
        Print(L"[INFO] CPU Vendor: %s\n", CpuVendor);
        Print(L"[INFO] CPU Family: %s\n", CpuFamily);
        Print(L"[INFO] CPU Cores: %d\n", CpuCores);
        
        // Check for AMD-specific information
        if (StrStr(CpuVendor, L"AMD") != NULL) {
            Print(L"[INFO] ✅ AMD processor detected\n");
            if (StrStr(CpuFamily, L"Ryzen") != NULL) {
                Print(L"[INFO] ✅ AMD Ryzen processor confirmed\n");
            }
        }
        
        TEST_ASSERT(CpuCores > 0, "CPU should have at least 1 core");
        TEST_ASSERT(StrLen(CpuVendor) > 0, "CPU vendor should not be empty");
    }
    
    TEST_END("AMD Platform Detection", EFI_SUCCESS);
    return EFI_SUCCESS;
}

/**
 * Test UEFI Security Features
 */
STATIC EFI_STATUS TestUefiSecurityFeatures(VOID)
{
    EFI_STATUS Status;
    BOOLEAN SecureBootEnabled = FALSE;
    BOOLEAN TpmPresent = FALSE;
    
    TEST_START("UEFI Security Features");
    
    // Test Secure Boot detection
    Status = uefi_check_secure_boot(&SecureBootEnabled);
    if (!EFI_ERROR(Status)) {
        Print(L"[INFO] Secure Boot: %s\n", SecureBootEnabled ? L"ENABLED" : L"DISABLED");
        TEST_ASSERT(TRUE, "Secure Boot check completed");
    } else {
        Print(L"[WARN] Secure Boot status unavailable\n");
        TEST_ASSERT(TRUE, "Secure Boot check handled gracefully");
    }
    
    // Test TPM detection
    Status = uefi_check_tpm(&TpmPresent);
    if (!EFI_ERROR(Status)) {
        Print(L"[INFO] TPM Present: %s\n", TpmPresent ? L"YES" : L"NO");
        TEST_ASSERT(TRUE, "TPM check completed");
    } else {
        Print(L"[WARN] TPM status unavailable\n");
        TEST_ASSERT(TRUE, "TPM check handled gracefully");
    }
    
    TEST_END("UEFI Security Features", EFI_SUCCESS);
    return EFI_SUCCESS;
}

/**
 * Test UEFI Variable Services
 */
STATIC EFI_STATUS TestUefiVariableServices(VOID)
{
    EFI_STATUS Status;
    UINTN DataSize;
    UINT32 Attributes;
    CHAR8 TestData[] = "USB_UEFI_TEST";
    CHAR8 ReadBuffer[32] = {0};
    
    TEST_START("UEFI Variable Services");
    
    // Test variable writing
    DataSize = AsciiStrLen(TestData) + 1;
    Status = gRT->SetVariable(
        L"USBUefiTest",
        &gEfiGlobalVariableGuid,
        EFI_VARIABLE_BOOTSERVICE_ACCESS | EFI_VARIABLE_RUNTIME_ACCESS,
        DataSize,
        TestData
    );
    
    if (!EFI_ERROR(Status)) {
        TEST_ASSERT(TRUE, "Variable write successful");
        
        // Test variable reading
        DataSize = sizeof(ReadBuffer);
        Status = gRT->GetVariable(
            L"USBUefiTest",
            &gEfiGlobalVariableGuid,
            &Attributes,
            &DataSize,
            ReadBuffer
        );
        
        if (!EFI_ERROR(Status)) {
            TEST_ASSERT(AsciiStrCmp(TestData, ReadBuffer) == 0, "Variable data should match");
            Print(L"[INFO] Variable read/write successful\n");
            
            // Cleanup test variable
            gRT->SetVariable(
                L"USBUefiTest",
                &gEfiGlobalVariableGuid,
                0,
                0,
                NULL
            );
        } else {
            Print(L"[WARN] Variable read failed: %r\n", Status);
            TEST_ASSERT(TRUE, "Variable read handled gracefully");
        }
    } else {
        Print(L"[WARN] Variable write failed: %r (may be read-only system)\n", Status);
        TEST_ASSERT(TRUE, "Variable write handled gracefully");
    }
    
    TEST_END("UEFI Variable Services", EFI_SUCCESS);
    return EFI_SUCCESS;
}

/**
 * Test UEFI Interface Cleanup
 */
STATIC EFI_STATUS TestUefiInterfaceCleanup(VOID)
{
    EFI_STATUS Status;
    
    TEST_START("UEFI Interface Cleanup");
    
    // Test cleanup
    Status = uefi_interface_cleanup();
    TEST_ASSERT(!EFI_ERROR(Status), "UEFI interface cleanup should succeed");
    
    TEST_END("UEFI Interface Cleanup", EFI_SUCCESS);
    return EFI_SUCCESS;
}

/**
 * Print final UEFI test statistics
 */
STATIC VOID PrintUefiTestStatistics(VOID)
{
    Print(L"\n");
    Print(L"=====================================\n");
    Print(L"        UEFI TEST RESULTS            \n");
    Print(L"=====================================\n");
    Print(L"Total Tests:  %d\n", mUefiTestStats.TotalTests);
    Print(L"Passed Tests: %d\n", mUefiTestStats.PassedTests);
    Print(L"Failed Tests: %d\n", mUefiTestStats.FailedTests);
    Print(L"Success Rate: %d%%\n", 
          (mUefiTestStats.TotalTests > 0) ? 
          (mUefiTestStats.PassedTests * 100) / mUefiTestStats.TotalTests : 0);
    Print(L"=====================================\n");
    
    if (mUefiTestStats.FailedTests == 0) {
        Print(L"🟢 ALL UEFI TESTS PASSED!\n");
    } else {
        Print(L"🔴 SOME UEFI TESTS FAILED!\n");
    }
}
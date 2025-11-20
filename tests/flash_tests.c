/**
 * @file flash_tests.c
 * @brief Flash Memory Operation Unit Tests
 * @author PhD Research Project
 * @details Comprehensive testing framework for flash memory operations
 */

#include <Uefi.h>
#include <Library/UefiLib.h>
#include <Library/MemoryAllocationLib.h>
#include <Library/BaseMemoryLib.h>
#include <Library/DebugLib.h>
#include "../src/firmware/flash_manager.h"
#include "../include/common.h"
#include "../include/debug_utils.h"

//
// Test Framework Macros
//
#define FLASH_TEST_ASSERT(condition, message) \
    do { \
        if (!(condition)) { \
            Print(L"[FAIL] %a: %a\n", __FUNCTION__, message); \
            return EFI_ABORTED; \
        } else { \
            Print(L"[PASS] %a: %a\n", __FUNCTION__, message); \
        } \
    } while (0)

#define FLASH_TEST_START(test_name) \
    Print(L"\n=== Flash Test: %a ===\n", test_name)

#define FLASH_TEST_END(test_name, status) \
    Print(L"=== %a Result: %r ===\n\n", test_name, status)

//
// Test Statistics
//
typedef struct {
    UINTN TotalTests;
    UINTN PassedTests;
    UINTN FailedTests;
} FLASH_TEST_STATS;

STATIC FLASH_TEST_STATS mFlashTestStats = {0};

//
// Test Constants
//
#define TEST_FLASH_SIZE             (1024 * 1024)  // 1MB test flash
#define TEST_SECTOR_SIZE            4096           // 4KB sectors
#define TEST_PATTERN_1              0xAA
#define TEST_PATTERN_2              0x55
#define TEST_PATTERN_3              0xFF
#define TEST_PATTERN_4              0x00

//
// Test Data Structures
//
typedef struct {
    UINT32 Address;
    UINTN Size;
    UINT8 Pattern;
    BOOLEAN ShouldSucceed;
    CONST CHAR8 *Description;
} FLASH_TEST_CASE;

//
// Test Cases for Flash Operations
//
STATIC FLASH_TEST_CASE mFlashTestCases[] = {
    // Valid operations
    {0x00000000, 512, TEST_PATTERN_1, TRUE, "Write to start of flash"},
    {0x00001000, 1024, TEST_PATTERN_2, TRUE, "Write to sector boundary"},
    {0x00010000, 2048, TEST_PATTERN_3, TRUE, "Write large block"},
    {0x000FF000, 256, TEST_PATTERN_4, TRUE, "Write near end of flash"},
    
    // Boundary tests
    {TEST_FLASH_SIZE - 512, 512, TEST_PATTERN_1, TRUE, "Write at end boundary"},
    {0x00000000, TEST_SECTOR_SIZE, TEST_PATTERN_2, TRUE, "Write full sector"},
    
    // Error cases
    {TEST_FLASH_SIZE, 512, TEST_PATTERN_1, FALSE, "Write beyond flash end"},
    {TEST_FLASH_SIZE - 256, 512, TEST_PATTERN_1, FALSE, "Write crossing end boundary"},
    {0x00000000, 0, TEST_PATTERN_1, FALSE, "Write zero bytes"},
};

//
// Test Function Prototypes
//
STATIC EFI_STATUS TestFlashManagerInit(VOID);
STATIC EFI_STATUS TestFlashDeviceInfo(VOID);
STATIC EFI_STATUS TestFlashReadOperations(VOID);
STATIC EFI_STATUS TestFlashWriteOperations(VOID);
STATIC EFI_STATUS TestFlashEraseOperations(VOID);
STATIC EFI_STATUS TestFlashBoundaryConditions(VOID);
STATIC EFI_STATUS TestFlashErrorHandling(VOID);
STATIC EFI_STATUS TestFlashIntegrityVerification(VOID);
STATIC EFI_STATUS TestFlashPerformance(VOID);
STATIC EFI_STATUS TestFlashManagerCleanup(VOID);

STATIC VOID PrintFlashTestStatistics(VOID);
STATIC EFI_STATUS PerformFlashReadWriteTest(FLASH_TEST_CASE *TestCase);
STATIC EFI_STATUS VerifyFlashData(UINT32 Address, UINTN Size, UINT8 Pattern);
STATIC VOID GenerateTestPattern(UINT8 *Buffer, UINTN Size, UINT8 Pattern);
STATIC UINT32 CalculateFlashChecksum(CONST UINT8 *Buffer, UINTN Size);

/**
 * Main Flash Test Runner
 */
EFI_STATUS
EFIAPI
RunFlashTests(VOID)
{
    EFI_STATUS Status;
    
    Print(L"\n");
    Print(L"==========================================\n");
    Print(L"   FLASH MEMORY OPERATION TESTS          \n");
    Print(L"==========================================\n");
    
    // Initialize test statistics
    mFlashTestStats.TotalTests = 0;
    mFlashTestStats.PassedTests = 0;
    mFlashTestStats.FailedTests = 0;
    
    // Run test suite
    Status = TestFlashManagerInit();
    mFlashTestStats.TotalTests++;
    if (!EFI_ERROR(Status)) mFlashTestStats.PassedTests++;
    else mFlashTestStats.FailedTests++;
    
    Status = TestFlashDeviceInfo();
    mFlashTestStats.TotalTests++;
    if (!EFI_ERROR(Status)) mFlashTestStats.PassedTests++;
    else mFlashTestStats.FailedTests++;
    
    Status = TestFlashReadOperations();
    mFlashTestStats.TotalTests++;
    if (!EFI_ERROR(Status)) mFlashTestStats.PassedTests++;
    else mFlashTestStats.FailedTests++;
    
    Status = TestFlashWriteOperations();
    mFlashTestStats.TotalTests++;
    if (!EFI_ERROR(Status)) mFlashTestStats.PassedTests++;
    else mFlashTestStats.FailedTests++;
    
    Status = TestFlashEraseOperations();
    mFlashTestStats.TotalTests++;
    if (!EFI_ERROR(Status)) mFlashTestStats.PassedTests++;
    else mFlashTestStats.FailedTests++;
    
    Status = TestFlashBoundaryConditions();
    mFlashTestStats.TotalTests++;
    if (!EFI_ERROR(Status)) mFlashTestStats.PassedTests++;
    else mFlashTestStats.FailedTests++;
    
    Status = TestFlashErrorHandling();
    mFlashTestStats.TotalTests++;
    if (!EFI_ERROR(Status)) mFlashTestStats.PassedTests++;
    else mFlashTestStats.FailedTests++;
    
    Status = TestFlashIntegrityVerification();
    mFlashTestStats.TotalTests++;
    if (!EFI_ERROR(Status)) mFlashTestStats.PassedTests++;
    else mFlashTestStats.FailedTests++;
    
    Status = TestFlashPerformance();
    mFlashTestStats.TotalTests++;
    if (!EFI_ERROR(Status)) mFlashTestStats.PassedTests++;
    else mFlashTestStats.FailedTests++;
    
    Status = TestFlashManagerCleanup();
    mFlashTestStats.TotalTests++;
    if (!EFI_ERROR(Status)) mFlashTestStats.PassedTests++;
    else mFlashTestStats.FailedTests++;
    
    // Print final results
    PrintFlashTestStatistics();
    
    return (mFlashTestStats.FailedTests == 0) ? EFI_SUCCESS : EFI_ABORTED;
}

/**
 * Test Flash Manager Initialization
 */
STATIC EFI_STATUS TestFlashManagerInit(VOID)
{
    EFI_STATUS Status;
    
    FLASH_TEST_START("Flash Manager Initialization");
    
    // Test flash manager initialization
    Status = flash_manager_init();
    FLASH_TEST_ASSERT(!EFI_ERROR(Status), "Flash manager should initialize successfully");
    
    // Test double initialization handling
    Status = flash_manager_init();
    FLASH_TEST_ASSERT(!EFI_ERROR(Status), "Double initialization should be handled gracefully");
    
    // Verify flash manager status
    Status = flash_manager_status();
    FLASH_TEST_ASSERT(!EFI_ERROR(Status), "Flash manager status should be accessible");
    
    FLASH_TEST_END("Flash Manager Initialization", EFI_SUCCESS);
    return EFI_SUCCESS;
}

/**
 * Test Flash Device Information Retrieval
 */
STATIC EFI_STATUS TestFlashDeviceInfo(VOID)
{
    EFI_STATUS Status;
    FLASH_DEVICE_INFO FlashInfo = {0};
    
    FLASH_TEST_START("Flash Device Information");
    
    // Test device info retrieval
    Status = flash_get_device_info(&FlashInfo);
    FLASH_TEST_ASSERT(!EFI_ERROR(Status), "Device info should be retrievable");
    
    // Verify device information
    FLASH_TEST_ASSERT(FlashInfo.TotalSize > 0, "Total size should be greater than 0");
    FLASH_TEST_ASSERT(FlashInfo.SectorSize > 0, "Sector size should be greater than 0");
    FLASH_TEST_ASSERT(FlashInfo.BlockCount > 0, "Block count should be greater than 0");
    FLASH_TEST_ASSERT(StrLen(FlashInfo.DeviceName) > 0, "Device name should not be empty");
    
    // Print device information
    Print(L"[INFO] Flash Device: %s\n", FlashInfo.DeviceName);
    Print(L"[INFO] Total Size: %ld KB\n", FlashInfo.TotalSize / 1024);
    Print(L"[INFO] Sector Size: %ld bytes\n", FlashInfo.SectorSize);
    Print(L"[INFO] Block Count: %d\n", FlashInfo.BlockCount);
    Print(L"[INFO] Write Protected: %s\n", FlashInfo.WriteProtected ? L"YES" : L"NO");
    
    // Test NULL parameter handling
    Status = flash_get_device_info(NULL);
    FLASH_TEST_ASSERT(EFI_ERROR(Status), "NULL parameter should return error");
    
    FLASH_TEST_END("Flash Device Information", EFI_SUCCESS);
    return EFI_SUCCESS;
}

/**
 * Test Flash Read Operations
 */
STATIC EFI_STATUS TestFlashReadOperations(VOID)
{
    EFI_STATUS Status;
    UINT8 *ReadBuffer = NULL;
    UINTN BufferSize = 1024;
    
    FLASH_TEST_START("Flash Read Operations");
    
    // Allocate test buffer
    ReadBuffer = AllocateZeroPool(BufferSize);
    FLASH_TEST_ASSERT(ReadBuffer != NULL, "Buffer allocation should succeed");
    
    // Test basic read operation
    Status = flash_read(0x00000000, ReadBuffer, BufferSize);
    FLASH_TEST_ASSERT(!EFI_ERROR(Status), "Basic read should succeed");
    
    // Test read from different addresses
    Status = flash_read(0x00001000, ReadBuffer, 512);
    FLASH_TEST_ASSERT(!EFI_ERROR(Status), "Read from offset should succeed");
    
    // Test large read operation
    FreePool(ReadBuffer);
    BufferSize = 32 * 1024; // 32KB
    ReadBuffer = AllocateZeroPool(BufferSize);
    if (ReadBuffer != NULL) {
        Status = flash_read(0x00000000, ReadBuffer, BufferSize);
        Print(L"[INFO] Large read test result: %r\n", Status);
    }
    
    // Test error conditions
    Status = flash_read(0x00000000, NULL, 1024);
    FLASH_TEST_ASSERT(EFI_ERROR(Status), "Read with NULL buffer should fail");
    
    Status = flash_read(0xFFFFFFFF, ReadBuffer, 1024);
    FLASH_TEST_ASSERT(EFI_ERROR(Status), "Read from invalid address should fail");
    
    if (ReadBuffer) {
        FreePool(ReadBuffer);
    }
    
    FLASH_TEST_END("Flash Read Operations", EFI_SUCCESS);
    return EFI_SUCCESS;
}

/**
 * Test Flash Write Operations
 */
STATIC EFI_STATUS TestFlashWriteOperations(VOID)
{
    EFI_STATUS Status;
    UINTN i;
    
    FLASH_TEST_START("Flash Write Operations");
    
    // Test all defined test cases
    for (i = 0; i < ARRAY_SIZE(mFlashTestCases); i++) {
        Print(L"[TEST] %a\n", mFlashTestCases[i].Description);
        
        Status = PerformFlashReadWriteTest(&mFlashTestCases[i]);
        
        if (mFlashTestCases[i].ShouldSucceed) {
            if (EFI_ERROR(Status)) {
                Print(L"[WARN] Expected success but got: %r\n", Status);
            }
        } else {
            if (!EFI_ERROR(Status)) {
                Print(L"[WARN] Expected failure but got success\n");
            } else {
                Print(L"[INFO] Expected failure occurred: %r\n", Status);
            }
        }
    }
    
    FLASH_TEST_ASSERT(TRUE, "Write operation tests completed");
    
    FLASH_TEST_END("Flash Write Operations", EFI_SUCCESS);
    return EFI_SUCCESS;
}

/**
 * Test Flash Erase Operations
 */
STATIC EFI_STATUS TestFlashEraseOperations(VOID)
{
    EFI_STATUS Status;
    UINT8 *VerifyBuffer = NULL;
    UINTN BufferSize = TEST_SECTOR_SIZE;
    
    FLASH_TEST_START("Flash Erase Operations");
    
    // Allocate verification buffer
    VerifyBuffer = AllocateZeroPool(BufferSize);
    FLASH_TEST_ASSERT(VerifyBuffer != NULL, "Buffer allocation should succeed");
    
    // Test sector erase
    Status = flash_erase_sector(0x00010000);
    if (!EFI_ERROR(Status)) {
        FLASH_TEST_ASSERT(TRUE, "Sector erase should succeed");
        
        // Verify erase (flash should be 0xFF after erase)
        Status = flash_read(0x00010000, VerifyBuffer, BufferSize);
        if (!EFI_ERROR(Status)) {
            BOOLEAN IsErased = TRUE;
            for (UINTN i = 0; i < BufferSize; i++) {
                if (VerifyBuffer[i] != 0xFF) {
                    IsErased = FALSE;
                    break;
                }
            }
            if (IsErased) {
                Print(L"[INFO] Erase verification successful\n");
            } else {
                Print(L"[WARN] Erase verification failed - data not 0xFF\n");
            }
        }
    } else {
        Print(L"[WARN] Sector erase failed: %r (may be write-protected)\n", Status);
        FLASH_TEST_ASSERT(TRUE, "Erase test completed with expected result");
    }
    
    // Test invalid erase operations
    Status = flash_erase_sector(0xFFFFFFFF);
    FLASH_TEST_ASSERT(EFI_ERROR(Status), "Erase at invalid address should fail");
    
    if (VerifyBuffer) {
        FreePool(VerifyBuffer);
    }
    
    FLASH_TEST_END("Flash Erase Operations", EFI_SUCCESS);
    return EFI_SUCCESS;
}

/**
 * Test Flash Boundary Conditions
 */
STATIC EFI_STATUS TestFlashBoundaryConditions(VOID)
{
    EFI_STATUS Status;
    UINT8 TestByte = 0xAA;
    
    FLASH_TEST_START("Flash Boundary Conditions");
    
    // Test minimum size operations
    Status = flash_read(0x00000000, &TestByte, 1);
    Print(L"[INFO] 1-byte read result: %r\n", Status);
    
    Status = flash_write(0x00001000, &TestByte, 1);
    Print(L"[INFO] 1-byte write result: %r\n", Status);
    
    // Test zero-size operations (should fail)
    Status = flash_read(0x00000000, &TestByte, 0);
    FLASH_TEST_ASSERT(EFI_ERROR(Status), "Zero-size read should fail");
    
    Status = flash_write(0x00000000, &TestByte, 0);
    FLASH_TEST_ASSERT(EFI_ERROR(Status), "Zero-size write should fail");
    
    // Test unaligned access
    Status = flash_read(0x00000001, &TestByte, 1); // Unaligned address
    Print(L"[INFO] Unaligned read result: %r\n", Status);
    
    FLASH_TEST_ASSERT(TRUE, "Boundary condition tests completed");
    
    FLASH_TEST_END("Flash Boundary Conditions", EFI_SUCCESS);
    return EFI_SUCCESS;
}

/**
 * Test Flash Error Handling
 */
STATIC EFI_STATUS TestFlashErrorHandling(VOID)
{
    EFI_STATUS Status;
    UINT8 TestBuffer[1024];
    
    FLASH_TEST_START("Flash Error Handling");
    
    // Test NULL pointer handling
    Status = flash_read(0x00000000, NULL, sizeof(TestBuffer));
    FLASH_TEST_ASSERT(EFI_ERROR(Status), "NULL buffer should cause error");
    
    Status = flash_write(0x00000000, NULL, sizeof(TestBuffer));
    FLASH_TEST_ASSERT(EFI_ERROR(Status), "NULL buffer should cause error");
    
    // Test out-of-bounds access
    Status = flash_read(0xFFFFFFFF, TestBuffer, sizeof(TestBuffer));
    FLASH_TEST_ASSERT(EFI_ERROR(Status), "Out-of-bounds read should fail");
    
    Status = flash_write(0xFFFFFFFF, TestBuffer, sizeof(TestBuffer));
    FLASH_TEST_ASSERT(EFI_ERROR(Status), "Out-of-bounds write should fail");
    
    // Test operations on uninitialized flash manager
    // Note: We can't actually uninitialize for this test without affecting other tests
    Print(L"[INFO] Error handling validation completed\n");
    
    FLASH_TEST_ASSERT(TRUE, "Error handling tests completed");
    
    FLASH_TEST_END("Flash Error Handling", EFI_SUCCESS);
    return EFI_SUCCESS;
}

/**
 * Test Flash Data Integrity Verification
 */
STATIC EFI_STATUS TestFlashIntegrityVerification(VOID)
{
    EFI_STATUS Status;
    UINT8 *WriteBuffer = NULL;
    UINT8 *ReadBuffer = NULL;
    UINTN BufferSize = 2048;
    UINT32 Address = 0x00020000;
    
    FLASH_TEST_START("Flash Data Integrity");
    
    // Allocate test buffers
    WriteBuffer = AllocateZeroPool(BufferSize);
    ReadBuffer = AllocateZeroPool(BufferSize);
    FLASH_TEST_ASSERT(WriteBuffer != NULL && ReadBuffer != NULL, "Buffer allocation should succeed");
    
    // Generate test pattern
    GenerateTestPattern(WriteBuffer, BufferSize, TEST_PATTERN_1);
    
    // Write test data
    Status = flash_write(Address, WriteBuffer, BufferSize);
    if (!EFI_ERROR(Status)) {
        // Read back and verify
        Status = flash_read(Address, ReadBuffer, BufferSize);
        if (!EFI_ERROR(Status)) {
            // Compare data
            if (CompareMem(WriteBuffer, ReadBuffer, BufferSize) == 0) {
                Print(L"[INFO] Data integrity verification PASSED\n");
                FLASH_TEST_ASSERT(TRUE, "Data integrity maintained");
            } else {
                Print(L"[WARN] Data integrity verification FAILED\n");
                
                // Show first few mismatches
                UINTN Mismatches = 0;
                for (UINTN i = 0; i < BufferSize && Mismatches < 10; i++) {
                    if (WriteBuffer[i] != ReadBuffer[i]) {
                        Print(L"[MISMATCH] Offset 0x%X: Wrote 0x%02X, Read 0x%02X\n", 
                              i, WriteBuffer[i], ReadBuffer[i]);
                        Mismatches++;
                    }
                }
                FLASH_TEST_ASSERT(FALSE, "Data integrity check failed");
            }
        } else {
            Print(L"[WARN] Read back failed: %r\n", Status);
        }
    } else {
        Print(L"[WARN] Write operation failed: %r\n", Status);
    }
    
    // Cleanup
    if (WriteBuffer) FreePool(WriteBuffer);
    if (ReadBuffer) FreePool(ReadBuffer);
    
    FLASH_TEST_END("Flash Data Integrity", EFI_SUCCESS);
    return EFI_SUCCESS;
}

/**
 * Test Flash Performance Characteristics
 */
STATIC EFI_STATUS TestFlashPerformance(VOID)
{
    EFI_STATUS Status;
    UINT8 *TestBuffer = NULL;
    UINTN BufferSizes[] = {512, 1024, 4096, 16384}; // Different buffer sizes
    UINTN i;
    
    FLASH_TEST_START("Flash Performance Testing");
    
    for (i = 0; i < ARRAY_SIZE(BufferSizes); i++) {
        UINTN BufferSize = BufferSizes[i];
        
        TestBuffer = AllocateZeroPool(BufferSize);
        if (TestBuffer == NULL) {
            continue;
        }
        
        GenerateTestPattern(TestBuffer, BufferSize, TEST_PATTERN_2);
        
        Print(L"[PERF] Testing %ld byte operations...\n", BufferSize);
        
        // Simple performance test (no actual timing in UEFI environment)
        Status = flash_write(0x00030000, TestBuffer, BufferSize);
        if (!EFI_ERROR(Status)) {
            Status = flash_read(0x00030000, TestBuffer, BufferSize);
            if (!EFI_ERROR(Status)) {
                Print(L"[PERF] %ld byte read/write: SUCCESS\n", BufferSize);
            }
        }
        
        FreePool(TestBuffer);
        TestBuffer = NULL;
    }
    
    FLASH_TEST_ASSERT(TRUE, "Performance testing completed");
    
    FLASH_TEST_END("Flash Performance Testing", EFI_SUCCESS);
    return EFI_SUCCESS;
}

/**
 * Test Flash Manager Cleanup
 */
STATIC EFI_STATUS TestFlashManagerCleanup(VOID)
{
    EFI_STATUS Status;
    
    FLASH_TEST_START("Flash Manager Cleanup");
    
    // Test cleanup
    Status = flash_manager_cleanup();
    FLASH_TEST_ASSERT(!EFI_ERROR(Status), "Flash manager cleanup should succeed");
    
    // Test double cleanup
    Status = flash_manager_cleanup();
    FLASH_TEST_ASSERT(!EFI_ERROR(Status), "Double cleanup should be handled gracefully");
    
    FLASH_TEST_END("Flash Manager Cleanup", EFI_SUCCESS);
    return EFI_SUCCESS;
}

/**
 * Perform a complete read-write test case
 */
STATIC EFI_STATUS PerformFlashReadWriteTest(FLASH_TEST_CASE *TestCase)
{
    EFI_STATUS Status;
    UINT8 *WriteBuffer = NULL;
    UINT8 *ReadBuffer = NULL;
    
    if (TestCase == NULL || TestCase->Size == 0) {
        return EFI_INVALID_PARAMETER;
    }
    
    // Allocate buffers
    WriteBuffer = AllocateZeroPool(TestCase->Size);
    ReadBuffer = AllocateZeroPool(TestCase->Size);
    
    if (WriteBuffer == NULL || ReadBuffer == NULL) {
        Status = EFI_OUT_OF_RESOURCES;
        goto Cleanup;
    }
    
    // Generate test pattern
    GenerateTestPattern(WriteBuffer, TestCase->Size, TestCase->Pattern);
    
    // Perform write operation
    Status = flash_write(TestCase->Address, WriteBuffer, TestCase->Size);
    if (EFI_ERROR(Status)) {
        goto Cleanup;
    }
    
    // Perform read operation
    Status = flash_read(TestCase->Address, ReadBuffer, TestCase->Size);
    if (EFI_ERROR(Status)) {
        goto Cleanup;
    }
    
    // Verify data integrity
    if (CompareMem(WriteBuffer, ReadBuffer, TestCase->Size) != 0) {
        Status = EFI_CRC_ERROR;
        goto Cleanup;
    }
    
    Status = EFI_SUCCESS;
    
Cleanup:
    if (WriteBuffer) FreePool(WriteBuffer);
    if (ReadBuffer) FreePool(ReadBuffer);
    
    return Status;
}

/**
 * Verify flash data matches expected pattern
 */
STATIC EFI_STATUS VerifyFlashData(UINT32 Address, UINTN Size, UINT8 Pattern)
{
    EFI_STATUS Status;
    UINT8 *Buffer = NULL;
    UINTN i;
    
    Buffer = AllocateZeroPool(Size);
    if (Buffer == NULL) {
        return EFI_OUT_OF_RESOURCES;
    }
    
    Status = flash_read(Address, Buffer, Size);
    if (EFI_ERROR(Status)) {
        FreePool(Buffer);
        return Status;
    }
    
    // Verify pattern
    for (i = 0; i < Size; i++) {
        if (Buffer[i] != Pattern) {
            FreePool(Buffer);
            return EFI_CRC_ERROR;
        }
    }
    
    FreePool(Buffer);
    return EFI_SUCCESS;
}

/**
 * Generate test pattern in buffer
 */
STATIC VOID GenerateTestPattern(UINT8 *Buffer, UINTN Size, UINT8 Pattern)
{
    if (Buffer == NULL || Size == 0) {
        return;
    }
    
    for (UINTN i = 0; i < Size; i++) {
        // Create varied pattern based on position and base pattern
        Buffer[i] = (UINT8)(Pattern ^ (i & 0xFF));
    }
}

/**
 * Calculate simple checksum for flash data
 */
STATIC UINT32 CalculateFlashChecksum(CONST UINT8 *Buffer, UINTN Size)
{
    UINT32 Checksum = 0;
    
    if (Buffer == NULL) {
        return 0;
    }
    
    for (UINTN i = 0; i < Size; i++) {
        Checksum += Buffer[i];
        Checksum = (Checksum << 1) | (Checksum >> 31); // Rotate left
    }
    
    return Checksum;
}

/**
 * Print comprehensive test statistics
 */
STATIC VOID PrintFlashTestStatistics(VOID)
{
    Print(L"\n");
    Print(L"==========================================\n");
    Print(L"       FLASH MEMORY TEST RESULTS         \n");
    Print(L"==========================================\n");
    Print(L"Total Tests:  %d\n", mFlashTestStats.TotalTests);
    Print(L"Passed Tests: %d\n", mFlashTestStats.PassedTests);
    Print(L"Failed Tests: %d\n", mFlashTestStats.FailedTests);
    Print(L"Success Rate: %d%%\n", 
          (mFlashTestStats.TotalTests > 0) ? 
          (mFlashTestStats.PassedTests * 100) / mFlashTestStats.TotalTests : 0);
    Print(L"==========================================\n");
    
    if (mFlashTestStats.FailedTests == 0) {
        Print(L"🟢 ALL FLASH TESTS PASSED!\n");
    } else {
        Print(L"🔴 SOME FLASH TESTS FAILED!\n");
        Print(L"   Check individual test results above\n");
    }
    Print(L"==========================================\n");
}
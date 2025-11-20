/**
 * @file main.c
 * @brief Main entry point for USB UEFI Firmware Interface
 * @details This is the main application entry point that initializes all subsystems
 *          and provides the debugging interface for AM5/Ryzen systems
 */

#include <Uefi.h>
#include <Library/UefiLib.h>
#include <Library/UefiApplicationEntryPoint.h>
#include <Library/MemoryAllocationLib.h>
#include <Library/DebugLib.h>
#include <Library/UefiBootServicesTableLib.h>

#include "../include/common.h"
#include "../include/config.h"
#include "../include/debug_utils.h"
#include "usb/usb_driver.h"
#include "uefi/uefi_interface.h"
#include "firmware/firmware_loader.h"

#ifdef ENABLE_UNIT_TESTS
#include "../tests/test_runner.h"
#endif

//
// Global variables
//
EFI_HANDLE gImageHandle;
EFI_SYSTEM_TABLE *gST;
UINT32 gDebugLevel = DEBUG_LEVEL_ALL;
UINT32 gDebugCategories = DEBUG_CAT_USB | DEBUG_CAT_FIRMWARE | DEBUG_CAT_UEFI;

//
// Forward declarations
//
STATIC EFI_STATUS InitializeSubsystems(VOID);
STATIC EFI_STATUS RunMainLoop(VOID);
STATIC VOID CleanupAndExit(EFI_STATUS ExitStatus);
STATIC VOID PrintBanner(VOID);
STATIC EFI_STATUS ProcessUserCommands(VOID);

/**
 * Main entry point for the UEFI application
 * @param ImageHandle - The firmware allocated handle for this image
 * @param SystemTable - Pointer to the EFI System Table
 * @return EFI_STATUS - Success or error code
 */
EFI_STATUS
EFIAPI
UefiMain(
    IN EFI_HANDLE ImageHandle,
    IN EFI_SYSTEM_TABLE *SystemTable
)
{
    EFI_STATUS Status;
    
    // Store global handles
    gImageHandle = ImageHandle;
    gST = SystemTable;
    // Initialize library globals (gST, gBS, etc.)
    InitializeLib(ImageHandle, SystemTable);
    
    DBG_ENTER();
    
    // Print application banner
    PrintBanner();
    
    // Initialize all subsystems
    Status = InitializeSubsystems();
    if (EFI_ERROR(Status)) {
        LOG_ERROR("Failed to initialize subsystems: %r\n", Status);
        CleanupAndExit(Status);
        return Status;
    }
    
    LOG_INFO("USB UEFI Firmware Interface started successfully\n");
    
    // Run main application loop
    Status = RunMainLoop();
    
    // Cleanup and exit
    CleanupAndExit(Status);
    
    DBG_EXIT_STATUS(Status);
    return Status;
}

/**
 * Initialize all subsystems in order
 * @return EFI_STATUS - Success or error code
 */
STATIC
EFI_STATUS
InitializeSubsystems(VOID)
{
    EFI_STATUS Status;
    DEBUG_TIMER Timer;
    
    DBG_ENTER();
    
    DebugTimerStart(&Timer, "Subsystem Initialization");
    
    // Initialize UEFI interface
    LOG_INFO("Initializing UEFI interface...\n");
    Status = uefi_interface_init();
    CHECK_STATUS(Status, "UEFI interface initialization failed");
    
    // Initialize USB driver
    LOG_INFO("Initializing USB driver...\n");
    Status = usb_driver_init();
    CHECK_STATUS(Status, "USB driver initialization failed");
    
    // Detect USB devices
    LOG_INFO("Detecting USB devices...\n");
    Status = usb_device_detect();
    if (EFI_ERROR(Status)) {
        LOG_WARN("USB device detection failed: %r\n", Status);
        // Continue execution even if no devices found
    }
    
    // Initialize firmware loader
    LOG_INFO("Initializing firmware loader...\n");
    Status = firmware_loader_init();
    CHECK_STATUS(Status, "Firmware loader initialization failed");
    
    DebugTimerEnd(&Timer);
    
    LOG_INFO("All subsystems initialized successfully\n");
    
    DBG_EXIT_STATUS(Status);
    return EFI_SUCCESS;
}

/**
 * Main application loop
 * @return EFI_STATUS - Success or error code
 */
STATIC
EFI_STATUS
RunMainLoop(VOID)
{
    EFI_STATUS Status;
    UINTN EventIndex;
    EFI_EVENT TimerEvent;
    EFI_EVENT WaitList[2];
    
    DBG_ENTER();
    
    // Create timer event for periodic tasks
    Status = gBS->CreateEvent(
        EVT_TIMER,
        TPL_CALLBACK,
        NULL,
        NULL,
        &TimerEvent
    );
    CHECK_STATUS(Status, "Failed to create timer event");
    
    // Set timer to fire every second
    Status = gBS->SetTimer(TimerEvent, TimerPeriodic, 10000000); // 1 second
    CHECK_STATUS(Status, "Failed to set timer");
    
    WaitList[0] = gST->ConIn->WaitForKey;
    WaitList[1] = TimerEvent;
    
    LOG_INFO("Entering main loop - Press any key for commands\n");
    LOG_INFO("System ready for debugging operations\n");
    
    // Main event loop
    while (TRUE) {
        Status = gBS->WaitForEvent(2, WaitList, &EventIndex);
        if (EFI_ERROR(Status)) {
            LOG_ERROR("WaitForEvent failed: %r\n", Status);
            break;
        }
        
        switch (EventIndex) {
            case 0: // Keyboard input
                Status = ProcessUserCommands();
                if (Status == EFI_ABORTED) {
                    // User requested exit
                    LOG_INFO("Exit requested by user\n");
                    gBS->CloseEvent(TimerEvent);
                    DBG_EXIT_STATUS(EFI_SUCCESS);
                    return EFI_SUCCESS;
                }
                break;
                
            case 1: // Timer event
                // Perform periodic tasks
                usb_driver_status();
                break;
                
            default:
                LOG_WARN("Unexpected event index: %d\n", EventIndex);
                break;
        }
    }
    
    gBS->CloseEvent(TimerEvent);
    DBG_EXIT_STATUS(Status);
    return Status;
}

/**
 * Process user commands from keyboard input
 * @return EFI_STATUS - Success, error code, or EFI_ABORTED to exit
 */
STATIC
EFI_STATUS
ProcessUserCommands(VOID)
{
    EFI_INPUT_KEY Key;
    EFI_STATUS Status;
    
    // Read the key
    Status = gST->ConIn->ReadKeyStroke(gST->ConIn, &Key);
    if (EFI_ERROR(Status)) {
        return Status;
    }
    
    Print(L"\nCommand received: %c\n", Key.UnicodeChar);
    
    switch (Key.UnicodeChar) {
        case L'h':
        case L'H':
        case L'?':
            Print(L"\nUSB UEFI Firmware Debug Commands:\n");
            Print(L"  h/H/?  - Show this help\n");
            Print(L"  u/U    - USB device status\n");
            Print(L"  f/F    - Firmware information\n");
            Print(L"  s/S    - System information\n");
            Print(L"  d/D    - Debug level control\n");
            Print(L"  r/R    - Reset USB subsystem\n");
            Print(L"  q/Q    - Quit application\n");
            Print(L"  test   - Run comprehensive test suite\n");
            Print(L"  test-usb - Run USB-specific tests\n");
            Print(L"  test-uefi - Run UEFI-specific tests\n");
            Print(L"Press any key to continue...\n");
            break;
            
        case L'u':
        case L'U':
            Print(L"\nUSB Device Status:\n");
            usb_driver_status();
            break;
            
        case L'f':
        case L'F':
            Print(L"\nFirmware Information:\n");
            firmware_loader_status();
            break;
            
        case L's':
        case L'S':
            Print(L"\nSystem Information:\n");
            uefi_interface_status();
            break;
            
        case L'd':
        case L'D':
            Print(L"\nDebug Level: 0x%08X, Categories: 0x%08X\n", 
                  gDebugLevel, gDebugCategories);
            break;
            
        case L'r':
        case L'R':
            Print(L"\nResetting USB subsystem...\n");
            usb_driver_cleanup();
            usb_driver_init();
            usb_device_detect();
            Print(L"USB subsystem reset complete\n");
            break;
            
        case L'q':
        case L'Q':
            Print(L"\nExiting USB UEFI Firmware Interface...\n");
            return EFI_ABORTED; // Signal to exit main loop
            
        case L't':
        case L'T':
#ifdef ENABLE_UNIT_TESTS
            Print(L"Running comprehensive test suite...\n");
            Status = RunAllTests();
            if (EFI_ERROR(Status)) {
                Print(L"Tests failed: %r\n", Status);
            } else {
                Print(L"All tests passed successfully!\n");
            }
#else
            Print(L"Unit tests not enabled in this build\n");
#endif
            break;
            break;
            
        default:
            Print(L"\nUnknown command. Press 'h' for help.\n");
            break;
    }
    
    return EFI_SUCCESS;
}

/**
 * Cleanup all resources and prepare for exit
 * @param ExitStatus - The status to exit with
 */
STATIC
VOID
CleanupAndExit(
    EFI_STATUS ExitStatus
)
{
    DBG_ENTER();
    
    LOG_INFO("Cleaning up resources...\n");
    
    // Cleanup subsystems in reverse order
    firmware_loader_cleanup();
    usb_driver_cleanup();
    uefi_interface_cleanup();
    
    if (EFI_ERROR(ExitStatus)) {
        Print(L"\nApplication exiting with error: %r\n", ExitStatus);
    } else {
        Print(L"\nApplication exiting normally\n");
    }
    
    DBG_EXIT();
}

/**
 * Print application banner and system information
 */
STATIC
VOID
PrintBanner(VOID)
{
    Print(L"\n");
    Print(L"=====================================================\n");
    Print(L"  USB UEFI Firmware Interface v%d.%d.%d\n", 
          FIRMWARE_VERSION_MAJOR, FIRMWARE_VERSION_MINOR, FIRMWARE_VERSION_PATCH);
    Print(L"  %s\n", FIRMWARE_COPYRIGHT);
    Print(L"  AMD AM5/Ryzen Debug Tool\n");
    Print(L"=====================================================\n");
    Print(L"\n");
    Print(L"Firmware: %s\n", FIRMWARE_NAME);
    Print(L"Vendor:   %s\n", FIRMWARE_VENDOR);
    Print(L"Build:    %d\n", FIRMWARE_BUILD_NUMBER);
    Print(L"\n");
}
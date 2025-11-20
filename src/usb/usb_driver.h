// usb_driver.h
// USB Driver Header File
// This file declares the functions and structures used in the USB driver implementation.

// Include guard to prevent multiple inclusions
#ifndef _USB_DRIVER_H_
#define _USB_DRIVER_H_

// Include necessary headers
#include <Uefi.h>
#include <Protocol/UsbIo.h>
#include <Protocol/Usb2HostController.h>
#include "usb_protocol.h"
#include "../../include/common.h"

//
// USB Device Information Structure
//
typedef struct {
    EFI_HANDLE Handle;
    EFI_USB_IO_PROTOCOL *UsbIo;
    UINT16 VendorId;
    UINT16 ProductId;
    UINT8 DeviceClass;
    BOOLEAN IsConnected;
    CHAR16 DeviceName[64];
    UINT8 InterfaceCount;
    UINT8 ConfigurationValue;
} USB_DEVICE_INFO;

//
// USB Class Definitions
//
#define USB_CLASS_MASS_STORAGE      0x08
#define USB_CLASS_HID               0x03
#define USB_CLASS_HUB               0x09
#define USB_CLASS_CDC_DATA          0x0A
#define USB_CLASS_SMART_CARD        0x0B
#define USB_CLASS_VIDEO             0x0E
#define USB_CLASS_WIRELESS          0xE0

//
// USB Request Types and Requests
//
#define USB_REQ_TYPE_STANDARD       0x00
#define USB_REQ_TYPE_CLASS          0x20
#define USB_REQ_TYPE_VENDOR         0x40
#define USB_DIR_OUT                 0x00
#define USB_DIR_IN                  0x80

#define USB_REQ_GET_STATUS          0x00
#define USB_REQ_CLEAR_FEATURE       0x01
#define USB_REQ_SET_FEATURE         0x03
#define USB_REQ_SET_ADDRESS         0x05
#define USB_REQ_GET_DESCRIPTOR      0x06
#define USB_REQ_SET_DESCRIPTOR      0x07
#define USB_REQ_GET_CONFIGURATION   0x08
#define USB_REQ_SET_CONFIGURATION   0x09

//
// Function Prototypes
//
EFI_STATUS
EFIAPI
usb_driver_init(
    VOID
    );

EFI_STATUS
EFIAPI
usb_device_detect(
    VOID
    );

EFI_STATUS
EFIAPI
usb_device_communicate(
    IN UINTN DeviceId,
    IN OUT VOID *Data,
    IN UINTN Length
    );

EFI_STATUS
EFIAPI
usb_driver_status(
    VOID
    );

EFI_STATUS
EFIAPI
usb_driver_cleanup(
    VOID
    );

//
// Internal Functions
//
STATIC
EFI_STATUS
usb_process_device(
    IN EFI_HANDLE Handle,
    IN UINTN DeviceIndex
    );

STATIC
EFI_STATUS
InitializeMassStorageDevice(
    IN EFI_USB_IO_PROTOCOL *UsbIo,
    IN UINTN DeviceIndex
    );

STATIC
EFI_STATUS
InitializeHidDevice(
    IN EFI_USB_IO_PROTOCOL *UsbIo,
    IN UINTN DeviceIndex
    );

//
// External Variables
// (Driver maintains internal state; do not expose internal arrays directly)

#endif // _USB_DRIVER_H_
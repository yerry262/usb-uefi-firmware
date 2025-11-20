/**
 * @file usb_protocol.h
 * @brief USB protocol definitions for the project (UEFI-friendly types)
 */

#ifndef _USB_PROTOCOL_H_
#define _USB_PROTOCOL_H_

#include <Uefi.h>

// USB Standard Request Types
#define USB_REQ_TYPE_STANDARD       0x00
#define USB_REQ_TYPE_CLASS          0x20
#define USB_REQ_TYPE_VENDOR         0x40

// USB Standard Requests
#define USB_REQ_GET_STATUS          0x00
#define USB_REQ_CLEAR_FEATURE       0x01
#define USB_REQ_SET_FEATURE         0x03
#define USB_REQ_SET_ADDRESS         0x05
#define USB_REQ_GET_DESCRIPTOR      0x06
#define USB_REQ_SET_DESCRIPTOR      0x07
#define USB_REQ_GET_CONFIGURATION   0x08
#define USB_REQ_SET_CONFIGURATION   0x09
#define USB_REQ_GET_INTERFACE       0x0A
#define USB_REQ_SET_INTERFACE       0x0B
#define USB_REQ_SYNC_FRAME          0x0C

// USB Descriptor Types
#define USB_DESC_TYPE_DEVICE        0x01
#define USB_DESC_TYPE_CONFIG        0x02
#define USB_DESC_TYPE_STRING        0x03
#define USB_DESC_TYPE_INTERFACE     0x04
#define USB_DESC_TYPE_ENDPOINT      0x05

// USB Device Classes
#define USB_CLASS_AUDIO             0x01
#define USB_CLASS_COMM              0x02
#define USB_CLASS_HID               0x03
#define USB_CLASS_PHYSICAL          0x05
#define USB_CLASS_IMAGE             0x06
#define USB_CLASS_PRINTER           0x07
#define USB_CLASS_MASS_STORAGE      0x08
#define USB_CLASS_HUB               0x09
#define USB_CLASS_DATA              0x0A
#define USB_CLASS_SMART_CARD        0x0B
#define USB_CLASS_VIDEO             0x0E
#define USB_CLASS_WIRELESS          0xE0

// USB Endpoint Types
#define USB_ENDPOINT_CONTROL        0x00
#define USB_ENDPOINT_ISOCHRONOUS    0x01
#define USB_ENDPOINT_BULK           0x02
#define USB_ENDPOINT_INTERRUPT      0x03

// USB Transfer Directions
#define USB_DIR_OUT                 0x00
#define USB_DIR_IN                  0x80

// Request structure
#pragma pack(1)
typedef struct {
    UINT8   RequestType;
    UINT8   Request;
    UINT16  Value;
    UINT16  Index;
    UINT16  Length;
} USB_DEVICE_REQUEST;
#pragma pack()

#pragma pack(1)
typedef struct {
    UINT8   Length;
    UINT8   DescriptorType;
    UINT16  TotalLength;
    UINT8   NumInterfaces;
    UINT8   ConfigurationValue;
    UINT8   Configuration;
    UINT8   Attributes;
    UINT8   MaxPower;
} USB_CONFIG_DESCRIPTOR;
#pragma pack()

#pragma pack(1)
typedef struct {
    UINT8   Length;
    UINT8   DescriptorType;
    UINT8   InterfaceNumber;
    UINT8   AlternateSetting;
    UINT8   NumEndpoints;
    UINT8   InterfaceClass;
    UINT8   InterfaceSubClass;
    UINT8   InterfaceProtocol;
    UINT8   Interface;
} USB_INTERFACE_DESCRIPTOR;
#pragma pack()

#pragma pack(1)
typedef struct {
    UINT8   Length;
    UINT8   DescriptorType;
    UINT8   EndpointAddress;
    UINT8   Attributes;
    UINT16  MaxPacketSize;
    UINT8   Interval;
} USB_ENDPOINT_DESCRIPTOR;
#pragma pack()

#endif // _USB_PROTOCOL_H_
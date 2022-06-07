/****************************************************************************************************************************
  get_report.ino
  For ESP32 S series boards

  ESP32 USB Host HID Brigde is a library for the ESP32/Arduino platform
  Built by Jeff Leung https://github.com/badjeff/ESP32-USB-Host-HID-Bridge
  Licensed under MIT license
  
  Version: 1.0.0

  Version Modified By   Date      Comments
  ------- -----------  ---------- -----------
  1.0.0   Jeff Leung   06/07/2022 Initial coding
 *****************************************************************************************************************************/

#include <Arduino.h>

#define DAEMON_TASK_LOOP_DELAY  3 // ticks
#define CLASS_TASK_LOOP_DELAY   3 // ticks
#define DAEMON_TASK_COREID      0
#define CLASS_TASK_COREID       0
#include "usb_host_hid_bridge.h"

void config_desc_cb(const usb_config_desc_t *config_desc);
void device_info_cb(usb_device_info_t *dev_info);
void hid_report_descriptor_cb(usb_transfer_t *transfer);
void hid_report_cb(usb_transfer_t *transfer);
UsbHostHidBridge hidBridge;
// int32_t usb_input_ch[] = { 0,0,0,0, 0,0,0,0 };

void setup(void)
{
    delay(2000); // await monitor port wakeup
    Serial.begin(115200);

    hidBridge.setOnConfigDescriptorReceived( config_desc_cb );
    hidBridge.setOnDeviceInfoReceived( device_info_cb );
    hidBridge.setOnHidReportDescriptorReceived( hid_report_descriptor_cb );
    hidBridge.setOnReportReceived( hid_report_cb );
    hidBridge.begin();
}

void loop() {
}

void config_desc_cb(const usb_config_desc_t *config_desc) {
    usb_print_config_descriptor(config_desc, NULL);
}

void device_info_cb(usb_device_info_t *dev_info) {
    if (dev_info->str_desc_manufacturer) usb_print_string_descriptor(dev_info->str_desc_manufacturer);
    if (dev_info->str_desc_product)      usb_print_string_descriptor(dev_info->str_desc_product);
    if (dev_info->str_desc_serial_num)   usb_print_string_descriptor(dev_info->str_desc_serial_num);
}

void hid_report_descriptor_cb(usb_transfer_t *transfer) {
    //>>>>> for HID Report Descriptor
    // Explanation: https://electronics.stackexchange.com/questions/68141/
    // USB Descriptor and Request Parser: https://eleccelerator.com/usbdescreqparser/#
    //<<<<<
    Serial.printf("\nstatus %d, actual number of bytes transferred %d\n", transfer->status, transfer->actual_num_bytes);
    for(int i=0; i < transfer->actual_num_bytes; i++) {
        if (i == USB_SETUP_PACKET_SIZE) {
            Serial.printf("\n\n>>> Goto https://eleccelerator.com/usbdescreqparser/ \n");
            Serial.printf(">>> Copy & paste below HEX and parser as... USB HID Report Descriptor\n\n");
        }
        Serial.printf("%02X ", transfer->data_buffer[i]);
    }
    Serial.printf("\n\n");
    // Serial.printf("HID Report Descriptor\n");
    uint8_t *const data = (uint8_t *const)(transfer->data_buffer + USB_SETUP_PACKET_SIZE);
    size_t len = transfer->actual_num_bytes - USB_SETUP_PACKET_SIZE;
    // Serial.printf("> size: %ld bytes\n", len);
    bool isGamepad = false;
    bool isVenDef  = false;
    if (len >= 5) {
        uint8_t gamepadUsagePage[] = { 0x05, 0x01, 0x09, 0x05 };
        uint8_t vdrDefUsagePage[] = { 0x06, 0x00, 0xFF, 0x09, 0x01 };
        isGamepad = memcmp(data, gamepadUsagePage, sizeof(gamepadUsagePage)) == 0;
        isVenDef  = memcmp(data, vdrDefUsagePage, sizeof(vdrDefUsagePage)) == 0;
    }
    Serial.printf(">>> best guess: %s\n", isGamepad ? "HID Gamepad" : isVenDef ? "Vendor Defined" : "Unkown");
}

void hid_report_cb(usb_transfer_t *transfer) {
    //
    // check HID Report Descriptor for usage
    //
    unsigned char *const data = (unsigned char *const)(transfer->data_buffer);
    for (int i=0; i<transfer->actual_num_bytes && i<11; i++) {
        // Serial.printf("%d ", data[i]);
        // Serial.printf("%02X ", data[i]);
        for (int b = 8; b != -1; b--) Serial.printf("%d", (data[i] & (1 << b)) >> b );
        Serial.printf(" ");
    }
    Serial.printf("\n");
    // usb_input_ch[0] = data[0] * 16; //map(data[0], 0, 255, 0, 4096);
    // usb_input_ch[1] = data[1] * 16; //map(data[1], 0, 255, 0, 4096);
    // usb_input_ch[2] = data[2] * 16; //map(data[2], 0, 255, 0, 4096);
    // usb_input_ch[3] = data[3] * 16; //map(data[3], 0, 255, 0, 4096);
    // for (int i=0; i<4; i++) Serial.printf("%d ", usb_input_ch[i]);
    // Serial.printf("\n");
}

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
  1.0.1   Jeff Leung   06/12/2022 Deprecapte semaphores
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
int32_t usb_input_ch[] = { 0,0,0,0, 0,0,0,0 };

#include <stdint.h>
#include <stdio.h>
#include <inttypes.h>
uint32_t get_offset_bits(uint8_t *data, uint32_t offset, uint32_t count);

const int servoPin = 16; // GPIO16
int dutyCycle = 0;
const int PWMFreq = 333;
const int PWMChannel = 0;
const int PWMResolution = 8;
//const int MAX_DUTY_CYCLE = (int)(pow(2, PWMResolution) - 1);

void setup(void)
{
    delay(2000); // await monitor port wakeup
    Serial.begin(115200);

    hidBridge.onConfigDescriptorReceived = config_desc_cb;
    hidBridge.onDeviceInfoReceived = device_info_cb;
    hidBridge.onHidReportDescriptorReceived = hid_report_descriptor_cb;
    hidBridge.onReportReceived = hid_report_cb;
    hidBridge.begin();

    ledcSetup(PWMChannel, PWMFreq, PWMResolution);
    ledcAttachPin(servoPin, PWMChannel);
    ledcWrite(PWMChannel, dutyCycle);
}

void loop() {
    dutyCycle = map(usb_input_ch[0], 0, 255, 180/5, 179);
    ledcWrite(PWMChannel, dutyCycle);
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
    uint8_t *data = (uint8_t *)(transfer->data_buffer);

    // for (int i=0; i<transfer->actual_num_bytes && i<11; i++) {
    //     // Serial.printf("%d ", data[i]);
    //     // Serial.printf("%02X ", data[i]);
    //     for (int b=0; b<8; b++) Serial.printf("%d", (data[i] & (1 << b)) >> b );
    //     Serial.printf(" ");
    // }
    // Serial.printf("\n");

    // usb_input_ch[0] = data[0];
    // usb_input_ch[1] = data[1];
    // usb_input_ch[2] = data[2];
    // usb_input_ch[3] = data[3];
    // for (int i=0; i<4; i++) Serial.printf("%d ", usb_input_ch[i]);
    // Serial.printf("\n");

    usb_input_ch[0] = get_offset_bits(data, 8,  8);
    usb_input_ch[1] = get_offset_bits(data, 16, 8);
    usb_input_ch[2] = get_offset_bits(data, 24, 8);
    usb_input_ch[3] = get_offset_bits(data, 32, 8);
    for (int i=0; i<4; i++) Serial.printf("%d ", usb_input_ch[i]);
    Serial.printf("\n");
}

uint32_t get_offset_bits(uint8_t *data, uint32_t offset, uint32_t count) {
	int shft;
	uint32_t ret, byte;
	byte = offset / 8;
	shft = offset & 7;
	ret = (((uint32_t)data[byte + 0]) << (shft + 24));
	if (count + shft > 8)
		ret |= (((uint32_t)data[byte + 1]) << (shft + 16));
	if (count + shft > 16)
		ret |= (((uint32_t)data[byte + 2]) << (shft +  8));
	if (count + shft > 24)
		ret |= (((uint32_t)data[byte + 3]) << (shft +  0));
	if (count + shft > 32)
		ret |= (((uint32_t)data[byte + 4]) << (shft -  8));
	return ret >> (32 - count);
}

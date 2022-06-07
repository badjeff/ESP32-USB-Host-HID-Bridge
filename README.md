# ESP32 USB Host HID Brigde

An experimental library for bridging USB-OTG Host stack on Espressif ESP32 (S Series). This project is based on sample code on [ESP-IDF Programming Guide](https://docs.espressif.com/projects/esp-idf/en/latest/esp32s3/get-started/index.html). Developed for Arduino framework on espressif32 (4.4.0) on PlatformIO.

---
## 🙈 TL;DR;
This [🎥 Video](https://imgur.com/pYu87BP) demo how it works in [🕹📡 simpleTx_esp32](https://github.com/badjeff/simpleTx_esp32)

---


## What does this library do?
- Create daemon and driver tasks like the official doc said
- Fetch endpoints from interfaces, pick one EP-IN to interrupt later
- Get HID Report Descriptor, print it to serial. No fancy parser implemented
- Repeatly, request report packet from EP-IN

HID Report Descriptor is not parsed due to calibration is needed and will do the job in real life application. User can dump the HEXes from serial ouptut, and decode it via [an online parser](https://eleccelerator.com/usbdescreqparser/#).


## Supporting Boards
Only tested on [NodeMCU-32-S2-Kit](https://www.waveshare.com/product/nodemcu-32-s2-kit.htm), an ESP32-S2 WiFi Development Board, with ESP-12K Module. An USB Type-A female connector is connected to USB_D- (GPIO19) & USB_D+ (GPIO20) for USB Hosting. All other S series boards should work with those exposeing USB data pins.


## How To Use

Define the global variables before `void Setup()`
```cpp
void hid_report_descriptor_cb(usb_transfer_t *transfer);
void hid_report_cb(usb_transfer_t *transfer);
UsbHostHidBridge hidBridge;
```

Put these lines inside `void Setup()`
```cpp
hidBridge.setOnHidReportDescriptorReceived( hid_report_descriptor_cb );
hidBridge.setOnReportReceived( hid_report_cb );
hidBridge.begin();
```

Define a callback function to decode the raw HID_REPORT_DESCRIPTOR packet
```cpp
void hid_report_descriptor_cb(usb_transfer_t *transfer) {
    for(int i=0; i < transfer->actual_num_bytes; i++) {
        if (i == USB_SETUP_PACKET_SIZE) {
            Serial.printf("\n\n>>> Goto https://eleccelerator.com/usbdescreqparser/ \n");
            Serial.printf(">>> Copy & paste below HEX and parser as... USB HID Report Descriptor\n\n");
        }
        Serial.printf("%02X ", transfer->data_buffer[i]);
    }
}
```

Define a callback function to decode the raw GET_REPORT packet
```cpp
void hid_report_cb(usb_transfer_t *transfer) {
    unsigned char *const data = (unsigned char *const)(transfer->data_buffer);
    // print data_buffer in binary format
    for (int i=0; i<transfer->actual_num_bytes && i<11; i++) { // prints first 11 bytes only!!!
        for (int b = 8; b != -1; b--)
            Serial.printf("%d", (data[i] & (1 << b)) >> b );
        Serial.printf(" ");
    }
    Serial.printf("\n");
}
```

After `Build, Run, & Monitoring`, and plug an HID device (e.g. a PS4 gamepad) to USB-A connector. The serial output on IDE should shows the bit stream from ypur HID device.

Have fun!


## License
MIT


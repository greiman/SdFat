// This example demonstrates use of an external BlockDevice.
// It was tested on an AVR Mega 2560 with an Adafruit MicroSD
// card breakout board+ and a MAX3421E-based USB Host Shield.
//
// Location of the USB library:
// https://github.com/felis/USB_Host_Shield_2.0
// Used library USB_Host_Shield_2.0-1.7.0
//
// The SD breakout:
// https://www.adafruit.com/product/254

#include "UsbMscDriver.h"

#if !defined(USE_BLOCK_DEVICE_INTERFACE) || USE_BLOCK_DEVICE_INTERFACE == 0
// Edit SdFatConfig.h and enable generic block devices.
// #define USE_BLOCK_DEVICE_INTERFACE 1
#error USE_BLOCK_DEVICE_INTERFACE
#endif  // USE_BLOCK_DEVICE_INTERFACE check

#define USE_SD 1       // Set to one for test of SD and USB.
#define USB_CS_PIN 10  // USB Shield chip select.
#define SD_CS_PIN 53   // SD card chip select.

USB usb;
BulkOnly bulk(&usb);
UsbMscDriver usbKey(&bulk);

// full exFAT FAT32 FAT16
// FsVolume key;
// FsFile file;

// FAT32 FAT16 only
FatVolume key;
File32 file;

#if USE_SD
SdFs sd;
#endif  // USE_SD

// uint8_t lun;

void setup() {
  Serial.begin(9600);
  while (!Serial) {
  }
  Serial.println(F("\nType any character to start"));
  while (!Serial.available()) {
  }
#if USE_SD
  pinMode(USB_CS_PIN, OUTPUT);
  digitalWrite(USB_CS_PIN, HIGH);
  Serial.println(F("\nList SD files."));
  if (!sd.begin(SD_CS_PIN)) sd.initErrorHalt();
  sd.ls(LS_DATE | LS_SIZE);
  sd.end();
#endif  // USE_SD
  Serial.println(F("\nBegin USB test."));
  if (!initUSB(&usb)) {
    Serial.println("initUSB failed");
    while (1) {
    }
  }

  // Must set USE_BLOCK_DEVICE_INTERFACE non-zero in SdFatConfig.h
  if (!key.begin(&usbKey)) {
    Serial.println(F("key.begin failed"));
    while (1) {
    }
  }
  if (!file.open("usbtest.txt", FILE_WRITE)) {
    Serial.println("file.open failed");
    while (1) {
    }
  }
  file.println("test line");
  file.close();
  Serial.println(F("\nList USB files."));
  key.ls(LS_DATE | LS_SIZE);
  key.end();
}

void loop() {}

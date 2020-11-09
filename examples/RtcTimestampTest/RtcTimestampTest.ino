// Test of time-stamp callback.
// Set the callback with this statement.
// FsDateTime::setCallback(dateTime);
#include "SdFat.h"
// https://github.com/adafruit/RTClib
#include "RTClib.h"

// SD_FAT_TYPE = 0 for SdFat/File as defined in SdFatConfig.h,
// 1 for FAT16/FAT32, 2 for exFAT, 3 for FAT16/FAT32 and exFAT.
#define SD_FAT_TYPE 0
/*
  Change the value of SD_CS_PIN if you are using SPI and
  your hardware does not use the default value, SS.  
  Common values are:
  Arduino Ethernet shield: pin 4
  Sparkfun SD shield: pin 8
  Adafruit SD shields and modules: pin 10
*/

// SDCARD_SS_PIN is defined for the built-in SD on some boards.
#ifndef SDCARD_SS_PIN
const uint8_t SD_CS_PIN = SS;
#else  // SDCARD_SS_PIN
// Assume built-in SD is used.
const uint8_t SD_CS_PIN = SDCARD_SS_PIN;
#endif  // SDCARD_SS_PIN

// Try to select the best SD card configuration.
#if HAS_SDIO_CLASS
#define SD_CONFIG SdioConfig(FIFO_SDIO)
#elif ENABLE_DEDICATED_SPI
#define SD_CONFIG SdSpiConfig(SD_CS_PIN, DEDICATED_SPI)
#else  // HAS_SDIO_CLASS
#define SD_CONFIG SdSpiConfig(SD_CS_PIN, SHARED_SPI)
#endif  // HAS_SDIO_CLASS

#if SD_FAT_TYPE == 0
SdFat sd;
File file;
#elif SD_FAT_TYPE == 1
SdFat32 sd;
File32 file;
#elif SD_FAT_TYPE == 2
SdExFat sd;
ExFile file;
#elif SD_FAT_TYPE == 3
SdFs sd;
FsFile file;
#else  // SD_FAT_TYPE
#error Invalid SD_FAT_TYPE
#endif  // SD_FAT_TYPE

RTC_DS1307 rtc;
//------------------------------------------------------------------------------
// Call back for file timestamps.  Only called for file create and sync().
void dateTime(uint16_t* date, uint16_t* time, uint8_t* ms10) {
  DateTime now = rtc.now();

  // Return date using FS_DATE macro to format fields.
  *date = FS_DATE(now.year(), now.month(), now.day());

  // Return time using FS_TIME macro to format fields.
  *time = FS_TIME(now.hour(), now.minute(), now.second());
  
  // Return low time bits in units of 10 ms, 0 <= ms10 <= 199.
  *ms10 = now.second() & 1 ? 100 : 0;
}
//------------------------------------------------------------------------------
void printField(Print* pr, char sep, uint8_t v) {
  if (sep) {
    pr->write(sep);
  }  
  if (v < 10) {
    pr->write('0');
  }
  pr->print(v);
}
//------------------------------------------------------------------------------  
void printNow(Print* pr) {
  DateTime now = rtc.now();
  pr->print(now.year());
  printField(pr, '-',now.month());
  printField(pr, '-',now.day());  
  printField(pr, ' ',now.hour());
  printField(pr, ':',now.minute());
  printField(pr, ':',now.second());
}
//------------------------------------------------------------------------------
void setup() {
  Serial.begin(9600);
  while (!Serial) {
    yield();
  }
  Serial.println(F("Type any character to begin"));
  while (!Serial.available()) {
    yield();
  }
  if (!rtc.begin()) {
    Serial.println(F("rtc.begin failed"));
    return;
  }
  if (!rtc.isrunning()) {
    Serial.println("RTC is NOT running!");
    return;
    // following line sets the RTC to the date & time this sketch was compiled
    // rtc.adjust(DateTime(F(__DATE__), F(__TIME__)));
    // This line sets the RTC with an explicit date & time, for example to set
    // January 21, 2014 at 3am you would call:
    // rtc.adjust(DateTime(2014, 1, 21, 3, 0, 0));
  }
  Serial.print(F("DateTime::now "));
  printNow(&Serial);
  Serial.println();

  // Set callback
  FsDateTime::setCallback(dateTime);
  
  if (!sd.begin(SD_CONFIG)) {
    sd.initErrorHalt(&Serial);
  }
  // Remove old version to set create time.
  if (sd.exists("RtcTest.txt")) {
    sd.remove("RtcTest.txt");
  }
  if (!file.open("RtcTest.txt", FILE_WRITE)) {
    Serial.println(F("file.open failed"));
    return;
  }
  // Print current date time to file.
  file.print(F("Test file at: "));
  printNow(&file);
  file.println();
  
  file.close();
  // List files in SD root.
  sd.ls(LS_DATE | LS_SIZE);
  Serial.println(F("Done"));
}
//------------------------------------------------------------------------------
void loop() {
}
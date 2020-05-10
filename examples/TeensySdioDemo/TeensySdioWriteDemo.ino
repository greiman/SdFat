/*
 * 
 * Modified version of Sdio demo that allows SD writing.
 * By Jared Reabow May 2020, written for teensy 3.5/3.6
 * 
 * 
  */

#include "SdFat.h"
SdFatSdioEX sdEx;
File file;

void runTest() {

  if (!file.open("TeensyDemo.txt", O_WRITE | O_CREAT | O_AT_END)) {
        Serial.println("SD CARD FILE NOT OPENED");
  } else {
    Serial.println("SD CARD FILE OPENED");
  }
  if (!file.println("Test write line")) {
        Serial.println("write failed");
  } else {
    Serial.println("write success");
    delay(1000);
  }
  file.close();
}


void setup() {
  Serial.begin(9600);
  while (!Serial) {
  }


}
void loop() {
    if (!sdEx.begin()) {
      Serial.println("SdFatSdioEX begin() failed");
    }
    // make sdEx the current volume.
    //only required if you open more than one instance of SdFatSdioEX sdEx;
    //sdEx.chvol();

  runTest();
}

/*
 * Program to compare size of Arduino SD library with SdFat.
 * See SdFatSize.ino for SdFat program.
 */
#include <SPI.h>
#include <SD.h>

File file;
//------------------------------------------------------------------------------
void setup() {
  Serial.begin(9600);
  while (!Serial) {}  // wait for Leonardo

  if (!SD.begin()) {
    Serial.println("begin failed");
    return;
  }
  file = SD.open("TEST_SD.TXT", FILE_WRITE);

  file.println("Hello");

  file.close();
  Serial.println("Done");
}
//------------------------------------------------------------------------------
void loop() {}

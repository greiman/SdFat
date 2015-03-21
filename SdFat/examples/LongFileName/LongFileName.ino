// Example use of lfnOpenNext and open by index.
// You can use test files located in
// SdFat/examples/LongFileName/testFiles.
#include<SPI.h>
#include <SdFat.h>
#include <SdFatUtil.h>

// SD card chip select pin.
const uint8_t SD_CS_PIN = SS;

SdFat sd;
SdFile file;
SdFile dirFile;

// Number of files found.
uint16_t n = 0;

// Max of ten files since files are selected with a single digit.
const uint16_t nMax = 10;

// Position of file's directory entry.
uint16_t dirIndex[nMax];
//------------------------------------------------------------------------------
void setup() {
  Serial.begin(9600);
  while (!Serial) {}
  delay(1000);

  // Print the location of some test files.
  Serial.println(F("\r\n"
                   "You can use test files located in\r\n"
                   "SdFat/examples/LongFileName/testFiles"));

  if (!sd.begin(SD_CS_PIN)) {
    sd.initErrorHalt();
  }
  Serial.print(F("Free RAM: "));
  Serial.println(FreeRam());
  Serial.println();

  // List files in root directory.
  if (!dirFile.open("/", O_READ)) {
    sd.errorHalt("open root failed");
  }
  while (n < nMax && file.openNext(&dirFile, O_READ)) {

    // Skip directories and hidden files.
    if (!file.isSubDir() && !file.isHidden()) {

      // Save dirIndex of file in directory.
      dirIndex[n] = file.dirIndex();

      // Print the file number and name.
      Serial.print(n++);
      Serial.write(' ');
      file.printName(&Serial);
      Serial.println();
    }
    file.close();
  }
}
//------------------------------------------------------------------------------
void loop() {
  int c;

  // Discard any Serial input.
  while (Serial.read() > 0) {}
  Serial.print(F("\r\nEnter File Number: "));

  while ((c = Serial.read()) < 0) {};
  if (!isdigit(c) || (c -= '0') >= n) {
    Serial.println(F("Invald number"));
    return;
  }
  Serial.println(c);
  if (!file.open(&dirFile, dirIndex[c], O_READ)) {
    sd.errorHalt(F("open"));
  }
  Serial.println();

  char last;

  // Copy up to 500 characters to Serial.
  for (int i = 0; i < 500 && (c = file.read()) > 0; i++)  {
    Serial.write(last = (char)c);
  }
  // Add new line if missing from last line.
  if (last != '\n') {
    Serial.println();
  }
  file.close();
  Serial.flush();
  delay(100);
}
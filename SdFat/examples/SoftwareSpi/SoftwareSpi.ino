// An example of the SdFatSoftSpi template class.
// This example is for an Adafruit Data Logging Shield on a Mega.
// Software SPI is required on Mega since this shield connects to pins 10-13.
// This example will also run on an Uno and other boards using software SPI.
//
#include <SPI.h>
#include <SdFat.h>
#if SD_SPI_CONFIGURATION >= 3  // Must be set in SdFat/SdFatConfig.h
//
// Pin numbers in templates must be constants.
const uint8_t SOFT_MISO_PIN = 12;
const uint8_t SOFT_MOSI_PIN = 11;
const uint8_t SOFT_SCK_PIN  = 13;
//
// Chip select may be constant or RAM variable.
const uint8_t SD_CHIP_SELECT_PIN = 10;

// SdFat software SPI template
SdFatSoftSpi<SOFT_MISO_PIN, SOFT_MOSI_PIN, SOFT_SCK_PIN> sd;

// Test file.
SdFile file;

void setup() {
  Serial.begin(9600);
  while (!Serial) {}  // Wait for Leonardo

  Serial.println("Type any character to start");
  while (Serial.read() <= 0) {}

  if (!sd.begin(SD_CHIP_SELECT_PIN)) {
    sd.initErrorHalt();
  }

  if (!file.open("SoftSPI.txt", O_CREAT | O_RDWR)) {
    sd.errorHalt(F("open failed"));
  }
  file.println(F("This line was printed using software SPI."));

  file.close();

  Serial.println(F("Done."));
}
//------------------------------------------------------------------------------
void loop() {}
#else  // SD_SPI_CONFIGURATION >= 3
#error SD_SPI_CONFIGURATION must be set to 3 in SdFat/SdFatConfig.h
#endif  //SD_SPI_CONFIGURATION >= 3
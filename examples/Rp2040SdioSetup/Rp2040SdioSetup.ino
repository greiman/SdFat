// RP2040 PIO SDIO setup and test.
/*
This example requires a SDIO Card socket with the following six lines.

CLK - A clock signal sent to the card by the MCU.
CMD - A bidirectional line for for commands and responses.
DAT[0:3] - Four bidirectional lines for data transfer.

CLK and CMD can be connected to any GPIO pins. DAT[0:3] can be connected
to any four consecutive GPIO pins in the order DAT0, DAT1, DAT2, DAT3.

For testing, I use several RP2040/RP3350 boards.
The Adafruit Metro RP2040 which has a builtin SDIO socket.

https://learn.adafruit.com/adafruit-metro-rp2040

I use this SD socket breakout board for other boards.

https://learn.adafruit.com/adafruit-microsd-spi-sdio

Wires should be short since signals can be as faster than 50 MHz.
*/
#define DISABLE_FS_H_WARNING  // Disable warning for type File not defined.
#include "SdFat.h"
//------------------------------------------------------------------------------
// Example GPIO definitions I use for debug. Edit for your setup.
// Run this example as is to print the symbol for your variant.
//
#if defined(ARDUINO_ADAFRUIT_METRO_RP2040)
#define RP_CLK_GPIO 18
#define RP_CMD_GPIO 19
#define RP_DAT0_GPIO 20  // DAT1: GPIO21, DAT2: GPIO22, DAT3: GPIO23.
#elif defined(ARDUINO_RASPBERRY_PI_PICO) || defined(ARDUINO_RASPBERRY_PI_PICO_2)
#define RP_CLK_GPIO 16
#define RP_CMD_GPIO 17
#define RP_DAT0_GPIO 18  // DAT1: GPIO19, DAT2: GPIO20, DAT3: GPIO21.
#elif defined(ARDUINO_ADAFRUIT_FEATHER_RP2350_HSTX)
#define RP_CLK_GPIO 11
#define RP_CMD_GPIO 10
#define RP_DAT0_GPIO 22  // DAT1: GPIO23, DAT2: GPIO24, DAT3: GPIO25.
#endif // defined(ARDUINO_ADAFRUIT_METRO_RP2040))

#if defined(RP_CLK_GPIO) && defined(RP_CMD_GPIO) && defined(RP_DAT0_GPIO)
#define SD_CONFIG SdioConfig(RP_CLK_GPIO, RP_CMD_GPIO, RP_DAT0_GPIO)
#else  // defined(RP_CLK_GPIO) && defined(RP_CMD_GPIO) && defined(RP_DAT0_GPIO)
#warning "Undefined SD_CONFIG. Run this program for the Variant Symbol."
#endif  // defined(RP_CLK_GPIO) && defined(RP_CMD_GPIO) && defined(RP_DAT0_GPIO)
//------------------------------------------------------------------------------
// Class File is not defined by SdFat since the RP2040 system defines it.
// 1 for FAT16/FAT32, 2 for exFAT, 3 for FAT16/FAT32 and exFAT.
#define SD_FAT_TYPE 3

#if SD_FAT_TYPE == 1
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

void setup() {
  Serial.begin(9600);
  while (!Serial) {
    yield();
  }
  Serial.println("Type any character to start\n");
  while (!Serial.available()) {
    yield();
  }
  Serial.print("Variant Symbol: ");
  Serial.print("ARDUINO_");
  Serial.println(BOARD_NAME);
  Serial.println();
#if defined(SD_CONFIG)
  if (!sd.begin(SD_CONFIG)) {
    sd.initErrorHalt(&Serial);
  }
  Serial.println("Card successfully initialized.");
  Serial.println("\nls:");
  sd.ls(LS_A | LS_DATE | LS_SIZE);  // Add LS_R for recursive list.
  Serial.println("\nDone! Try the bench example next.");
#else  // #if defined(SD_CONFIG)
  Serial.println("Error: SD_CONFIG undefined for your board.");
  Serial.println("Define RP_CLK_GPIO, RP_CMD_GPIO, and RP_DAT0_GPIO above.");
#endif
}

void loop() {}

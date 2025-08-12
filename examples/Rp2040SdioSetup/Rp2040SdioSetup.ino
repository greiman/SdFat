// RP2040/RP2350 PIO SDIO setup and test.
/*
This example requires a SDIO Card socket with the following six lines.

CLK - A clock signal sent to the card by the MCU.
CMD - A bidirectional line for for commands and responses.
DAT[0:3] - Four bidirectional lines for data transfer.

CLK and CMD can be connected to any GPIO pins. DAT[0:3] can be connected
to any four consecutive GPIO pins in the order DAT0, DAT1, DAT2, DAT3.

For testing, I use several RP2040/RP2350 boards.

These Adafruit boards have a builtin SDIO socket.

https://learn.adafruit.com/adafruit-metro-rp2040

https://learn.adafruit.com/adafruit-metro-rp2350

https://learn.adafruit.com/adafruit-feather-rp2040-adalogger

The Feather RP2350 Adalogger is coming soon.

I use this SD socket breakout board for other boards.

https://learn.adafruit.com/adafruit-microsd-spi-sdio

Wires should be short since signals can be as faster than 50 MHz.
*/
#ifndef DISABLE_FS_H_WARNING
#define DISABLE_FS_H_WARNING  // Disable warning for type File not defined.
#endif                        // DISABLE_FS_H_WARNING
#include "SdFat.h"
//------------------------------------------------------------------------------
// Example GPIO definitions I use for debug. Edit for your setup.
// Run this example as is to print the symbol for your variant.
//
#if defined(HAS_BUILTIN_PIO_SDIO)
// Note: fourth paramter of SdioConfig is the PIO clkDiv with default 1.00.
#define SD_CONFIG SdioConfig(PIN_SD_CLK, PIN_SD_CMD_MOSI, PIN_SD_DAT0_MISO)
#elif defined(ARDUINO_RASPBERRY_PI_PICO) || defined(ARDUINO_RASPBERRY_PI_PICO_2)
// CLK: GPIO10, CMD: GPIO11, DAT[0,3]: GPIO[12, 15].
#define SD_CONFIG SdioConfig(10u, 11u, 12u)
#elif defined(ARDUINO_ADAFRUIT_FEATHER_RP2350_HSTX)
// CLK: GPIO10, CMD: GPIO11, DAT[0,3]: GPIO[22, 25].
#define SD_CONFIG SdioConfig(10u, 11u, 22u)
#else  // defined(ARDUINO_ARCH_RP2040)
#warning "Undefined SD_CONFIG. Run this program for the Variant Symbol."
#endif  // defined(ARDUINO_ARCH_RP2040)
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
  Serial.printf("Variant Symbol: ARDUINO_%s\n\n", BOARD_NAME);
#if defined(SD_CONFIG)
  SdioConfig cfg = SD_CONFIG;
  Serial.printf("clkPin: %d, cmdPin: %d, dat0Pin: %d, clkDiv: %4.2f\n",
                cfg.clkPin(), cfg.cmdPin(), cfg.dat0Pin(), cfg.clkDiv());
  if (!sd.begin(SD_CONFIG)) {
    sd.initErrorHalt(&Serial);
  }
  Serial.println("Card successfully initialized.");
  Serial.println("\nls:");
  sd.ls(LS_A | LS_DATE | LS_SIZE);  // Add LS_R for recursive list.
  Serial.println("\nDone! Try the bench example next.");
#else  // #if defined(SD_CONFIG)
  Serial.println("Error: SD_CONFIG undefined for your board.");
  Serial.println("Define clkPin, cmdPin, and dat0Pin above.");
#endif
}

void loop() {}

#pragma once
// File to be included in SdFatConfig.h for debug definitions.
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
#endif  // defined(ARDUINO_ADAFRUIT_METRO_RP2040))

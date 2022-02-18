/**
 * Copyright (c) 2011-2020 Bill Greiman
 * This file is part of the SdFat library for SD memory cards.
 *
 * MIT License
 *
 * Permission is hereby granted, free of charge, to any person obtaining a
 * copy of this software and associated documentation files (the "Software"),
 * to deal in the Software without restriction, including without limitation
 * the rights to use, copy, modify, merge, publish, distribute, sublicense,
 * and/or sell copies of the Software, and to permit persons to whom the
 * Software is furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included
 * in all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS
 * OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
 * FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER
 * DEALINGS IN THE SOFTWARE.
 */
#include "SdSpiDriver.h"

#if defined(SD_USE_CUSTOM_SPI) && defined(__SAMD21G18A__)
/* Use SCK Maximum Frequency Override */
// Reference For Maximum Frequency (12 MHz): https://microchipsupport.force.com/s/article/SPI-max-clock-frequency-in-SAMD-SAMR-devices
#define USE_SAMD21_MAX_SCK_OVERRIDE 1

/* Determine SERCOM Peripheral to Use */
// In Order:
// MZero, Zero, MKR Zero
#if ((PIN_SPI_MISO == 18u) && (PIN_SPI_MOSI == 21u) && (PIN_SPI_SCK == 20u)) \
  || ((PIN_SPI_MISO == 22u) && (PIN_SPI_MOSI == 23u) && (PIN_SPI_SCK == 24u)) \
  || ((PIN_SPI1_MISO == 29u) && (PIN_SPI1_MOSI == 26u) && (PIN_SPI1_SCK == 27u)) 
  #define SPI_SERCOM SERCOM4
#elif (PIN_SPI_MISO == 30u) && (PIN_SPI_MOSI == 31u) && (PIN_SPI_SCK == 32u)
  // Circuitplay
  #define SPI_SERCOM SERCOM3
#else
  #define SPI_SERCOM SERCOM1
#endif

#if (SPI_INTERFACES_COUNT == 2) && defined(SDCARD_SS_PIN)
  // MKR Zero
  #define SD_SPI_CLASS SPI1
#else
  #define SD_SPI_CLASS SPI
#endif

//------------------------------------------------------------------------------
void SdSpiArduinoDriver::begin(SdSpiConfig spiConfig) {
  (void)spiConfig;
  SD_SPI_CLASS.begin();
}

//------------------------------------------------------------------------------
void SdSpiArduinoDriver::end() {
  m_spi->end();
}

//------------------------------------------------------------------------------
void SdSpiArduinoDriver::activate() {
#if USE_SAMD21_MAX_SCK_OVERRIDE
  // Reinitialize SPI Configuration if Specified SCK Clock > SD_SCK_MHZ(12)
  if(m_spiSettings.getClockFreq() > SD_SCK_MHZ(12))
    m_spiSettings = SPISettings(SD_SCK_MHZ(12), m_spiSettings.getBitOrder(), m_spiSettings.getDataMode());
#endif // USE_SAMD21_MAX_SCK_OVERRIDE
  SD_SPI_CLASS.beginTransaction(m_spiSettings);
}

//------------------------------------------------------------------------------
void SdSpiArduinoDriver::deactivate() {
  SD_SPI_CLASS.endTransaction();
}

//------------------------------------------------------------------------------
static inline uint8_t spiTransfer(uint8_t b) {
  SPI_SERCOM->SPI.DATA.bit.DATA = b;
  while(!SPI_SERCOM->SPI.INTFLAG.bit.RXC);
  return SPI_SERCOM->SPI.DATA.bit.DATA;
}

//------------------------------------------------------------------------------
// SPI RX Wait Cycle Optimization Table
static const uint8_t rx_wait_clk_tbl[] = {
  0, // SPI SCK @ 24 MHz (Invalid)
  7, // SPI SCK @ 12 MHz
  13, // SPI SCK @ 8 MHz
  18, // SPI SCK @ 6 MHz
  23, // SPI SCK @ 4.8 MHz
  29 // SPI SCK @ 4 MHz
};

//------------------------------------------------------------------------------
uint8_t SdSpiArduinoDriver::receive() {
  return spiTransfer(0XFF);
}

//------------------------------------------------------------------------------
uint8_t SdSpiArduinoDriver::receive(uint8_t* buf, size_t count) {
  int rtn = 0;
  uint8_t wait_clk = (SPI_SERCOM->SPI.BAUD.bit.BAUD > 5) ? 0 : rx_wait_clk_tbl[SPI_SERCOM->SPI.BAUD.bit.BAUD];

  if (wait_clk > 0) {
    for(size_t i = 0; i < count; i++) {
      SPI_SERCOM->SPI.DATA.bit.DATA = 0XFF;
      while(!SPI_SERCOM->SPI.INTFLAG.bit.DRE);
      __asm__ __volatile__(
      "   mov r7, %0   \n" // Use R7
      "1:              \n"
      "   sub r7, #1   \n" // Substract 1 From R7
      "   bne 1b       \n" // If Result is Not 0, Jump to Label 1
      :                    // No Output
      :  "r" (wait_clk)    // Read wait_clk Into R7
      :  "r7"              // Clobber R7
      );
      buf[i] = SPI_SERCOM->SPI.DATA.bit.DATA;
    }
  } else {
    // SPI SCK Lower Than 4 MHz
    for(size_t i = 0; i < count; i++) {
      SPI_SERCOM->SPI.DATA.bit.DATA = 0XFF;
      while(!SPI_SERCOM->SPI.INTFLAG.bit.RXC);
      buf[i] = SPI_SERCOM->SPI.DATA.bit.DATA;
    }
  }

  return rtn;
}

//------------------------------------------------------------------------------
// SPI TX Wait Cycle Optimization Table
static const uint8_t tx_wait_clk_tbl[] = {
  0, // SPI SCK @ 24 MHz (Invalid)
  1, // SPI SCK @ 12 MHz
  11, // SPI SCK @ 8 MHz
  17, // SPI SCK @ 6 MHz
  22, // SPI SCK @ 4.8 MHz
  27 // SPI SCK @ 4 MHz
};

//------------------------------------------------------------------------------
void SdSpiArduinoDriver::send(uint8_t data) {
  spiTransfer(data);
}

//------------------------------------------------------------------------------
void SdSpiArduinoDriver::send(const uint8_t* buf , size_t count) {
  uint8_t wait_clk = (SPI_SERCOM->SPI.BAUD.bit.BAUD > 5) ? 0 : tx_wait_clk_tbl[SPI_SERCOM->SPI.BAUD.bit.BAUD];

  if(wait_clk > 0) {
    for(size_t i = 0; i < count; i++) {
      SPI_SERCOM->SPI.DATA.bit.DATA = buf[i];
      while(!SPI_SERCOM->SPI.INTFLAG.bit.DRE);
      __asm__ __volatile__(
      "   mov r7, %0   \n" // Use R7
      "1:              \n"
      "   sub r7, #1   \n" // Substract 1 From R7
      "   bne 1b       \n" // If Result is Not 0, Jump to Label 1
      :                    // No Output
      :  "r" (wait_clk)    // Read wait_clk Into R7
      :  "r7"              // Clobber R7
      );
    }
  } else {
    // SPI SCK Lower Than 4 MHz
    for(size_t i = 0; i < count; i++) {
      SPI_SERCOM->SPI.DATA.bit.DATA = buf[i];
      while(!SPI_SERCOM->SPI.INTFLAG.bit.RXC);
    }
  }

  // Clear RX Overflow and Buffers
  SPI_SERCOM->SPI.STATUS.bit.BUFOVF = 1;
  SPI_SERCOM->SPI.CTRLB.bit.RXEN = 0;
  while(SPI_SERCOM->SPI.SYNCBUSY.bit.CTRLB); // Wait for Sync
  SPI_SERCOM->SPI.CTRLB.bit.RXEN = 1;
  while(SPI_SERCOM->SPI.SYNCBUSY.bit.CTRLB); // Wait for Sync
}
#endif  // defined(SD_USE_CUSTOM_SPI) && defined(__SAMD21G18A__)

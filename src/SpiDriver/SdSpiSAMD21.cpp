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
uint8_t SdSpiArduinoDriver::receive() {
  return spiTransfer(0XFF);
}

//------------------------------------------------------------------------------
uint8_t SdSpiArduinoDriver::receive(uint8_t* buf, size_t count) {
  int rtn = 0;

  for(size_t i = 0; i < count; i++) {
    SPI_SERCOM->SPI.DATA.bit.DATA = 0XFF;
    while(!SPI_SERCOM->SPI.INTFLAG.bit.RXC);
    buf[i] = SPI_SERCOM->SPI.DATA.bit.DATA;
  }

  return rtn;
}

//------------------------------------------------------------------------------
void SdSpiArduinoDriver::send(uint8_t data) {
  spiTransfer(data);
}

//------------------------------------------------------------------------------
void SdSpiArduinoDriver::send(const uint8_t* buf , size_t count) {
  for(size_t i = 0; i < count; i++) {
    SPI_SERCOM->SPI.DATA.bit.DATA = buf[i];
    while(!SPI_SERCOM->SPI.INTFLAG.bit.RXC);
  }

  // Clear RX Overflow and Buffers
  SPI_SERCOM->SPI.STATUS.bit.BUFOVF = 1;
  SPI_SERCOM->SPI.CTRLB.bit.RXEN = 0;
  while(SPI_SERCOM->SPI.SYNCBUSY.bit.CTRLB); // Wait for Sync
  SPI_SERCOM->SPI.CTRLB.bit.RXEN = 1;
  while(SPI_SERCOM->SPI.SYNCBUSY.bit.CTRLB); // Wait for Sync
}
#endif  // defined(SD_USE_CUSTOM_SPI) && defined(__SAMD21G18A__)

/* Arduino SdSpiAltDriver Library
 * Copyright (C) 2013 by William Greiman
 *
 * This file is part of the Arduino SdSpiAltDriver Library
 *
 * This Library is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This Library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with the Arduino SdSpiAltDriver Library.  If not, see
 * <http://www.gnu.org/licenses/>.
 */
#if defined(__STM32F1__) || defined(__STM32F4__)
#include "SdSpiDriver.h"
#if defined(__STM32F1__)
#define USE_STM32_DMA 1
#elif defined(__STM32F4__)
#define USE_STM32_DMA 0
#else  // defined(__STM32F1__)
#error Unknown STM32 type
#endif  // defined(__STM32F1__)
//------------------------------------------------------------------------------
static SPIClass m_SPI1(1);
#if BOARD_NR_SPI >= 2
static SPIClass m_SPI2(2);
#endif  // BOARD_NR_SPI >= 2
#if BOARD_NR_SPI >= 3
static SPIClass m_SPI3(3);
#endif  // BOARD_NR_SPI >= 3
#if BOARD_NR_SPI > 3
#error BOARD_NR_SPI too large
#endif
//------------------------------------------------------------------------------
/** Set SPI options for access to SD/SDHC cards.
 *
 * \param[in] divisor SCK clock divider relative to the APB1 or APB2 clock.
 */
void SdSpiAltDriver::activate() {
  m_spi->beginTransaction(m_spiSettings);
}
//------------------------------------------------------------------------------
/** Initialize the SPI bus.
 *
 * \param[in] chipSelectPin SD card chip select pin.
 */
void SdSpiAltDriver::begin(uint8_t csPin) {
  m_csPin = csPin;
  pinMode(m_csPin, OUTPUT);
  digitalWrite(m_csPin, HIGH);
  m_spi->begin();
}
//------------------------------------------------------------------------------
/**
 * End SPI transaction.
 */
void SdSpiAltDriver::deactivate() {
  m_spi->endTransaction();
}
//------------------------------------------------------------------------------
/** Receive a byte.
 *
 * \return The byte.
 */
uint8_t SdSpiAltDriver::receive() {
  return m_spi->transfer(0XFF);
}
//------------------------------------------------------------------------------
/** Receive multiple bytes.
 *
 * \param[out] buf Buffer to receive the data.
 * \param[in] n Number of bytes to receive.
 *
 * \return Zero for no error or nonzero error code.
 */
uint8_t SdSpiAltDriver::receive(uint8_t* buf, size_t n) {
#if USE_STM32_DMA
  return m_spi->dmaTransfer(0, buf, n);
#else  // USE_STM32_DMA
  m_spi->read(buf, n);
  return 0;
#endif  // USE_STM32_DMA
}
//------------------------------------------------------------------------------
/** Send a byte.
 *
 * \param[in] b Byte to send
 */
void SdSpiAltDriver::send(uint8_t b) {
  m_spi->transfer(b);
}
//------------------------------------------------------------------------------
/** Send multiple bytes.
 *
 * \param[in] buf Buffer for data to be sent.
 * \param[in] n Number of bytes to send.
 */
void SdSpiAltDriver::send(const uint8_t* buf , size_t n) {
#if USE_STM32_DMA
  m_spi->dmaTransfer(const_cast<uint8*>(buf), 0, n);
#else  // USE_STM32_DMA
  m_spi->write(const_cast<uint8*>(buf), n);
#endif  // USE_STM32_DMA
}
//------------------------------------------------------------------------------
void SdSpiAltDriver::setPort(uint8_t portNumber) {
  m_spi = &m_SPI1;
#if BOARD_NR_SPI >= 2
  if (portNumber == 2) {
    m_spi = &m_SPI2;
  }
#endif  // BOARD_NR_SPI >= 2
#if BOARD_NR_SPI >= 3
  if (portNumber == 3) {
    m_spi = &m_SPI3;
  }
#endif  // BOARD_NR_SPI >= 2
}
#endif  // defined(__STM32F1__) || defined(__STM32F4__)

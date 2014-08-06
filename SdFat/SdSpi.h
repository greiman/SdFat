/* Arduino SdSpi Library
 * Copyright (C) 2013 by William Greiman
 *
 * This file is part of the Arduino SdSpi Library
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
 * along with the Arduino SdSpi Library.  If not, see
 * <http://www.gnu.org/licenses/>.
 */
 /**
 * \file
 * \brief SdSpi class for V2 SD/SDHC cards
 */
#ifndef SdSpi_h
#define SdSpi_h
#include <Arduino.h>
#include <SdFatConfig.h>

#if !USE_ARDUINO_SPI_LIBRARY
// AVR Arduinos
#ifdef __AVR__
#if USE_SOFTWARE_SPI
#define USE_AVR_SOFTWARE_SPI 1
#elif LEONARDO_SOFT_SPI && defined(__AVR_ATmega32U4__) && !defined(CORE_TEENSY)
#define USE_AVR_SOFTWARE_SPI 1
#elif MEGA_SOFT_SPI&&(defined(__AVR_ATmega1280__)||defined(__AVR_ATmega2560__))
#define USE_AVR_SOFTWARE_SPI 1
#else  // USE_SOFTWARE_SPI
#define USE_AVR_S0FTWARE_SPI 0
#define USE_NATIVE_AVR_SPI 1
#endif  // USE_SOFTWARE_SPI
#endif  // __AVR__
// Due
#if defined(__arm__) && !defined(CORE_TEENSY)
/** Nonzero - use native SAM3X SPI */
#define USE_NATIVE_SAM3X_SPI 1
#else  //  USE_NATIVE_SAM3X_SPI
/** Zero - don't use native SAM3X SPI */
#define USE_NATIVE_SAM3X_SPI 0
#endif  // USE_NATIVE_SAM3X_SPI
// Teensy 3.0
#if defined(__arm__) && defined(CORE_TEENSY)
/** Nonzero - use native MK20DX128 SPI */
#define USE_NATIVE_MK20DX128_SPI 1
#else  // USE_NATIVE_MK20DX128_SPI
/** Zero - don't use native MK20DX128 SPI */
#define USE_NATIVE_MK20DX128_SPI 0
#endif  // USE_NATIVE_MK20DX128_SPI
#endif  // USE_ARDUINO_SPI_LIBRARY
//------------------------------------------------------------------------------
// define default chip select pin
//
#if !USE_AVR_SOFTWARE_SPI
/** The default chip select pin for the SD card is SS. */
uint8_t const  SD_CHIP_SELECT_PIN = SS;
#else  // USE_AVR_SOFTWARE_SPI
/** SPI chip select pin for software SPI. */
uint8_t const SD_CHIP_SELECT_PIN = SOFT_SPI_CS_PIN;
#endif  // USE_AVR_SOFTWARE_SPI

//------------------------------------------------------------------------------
/**
 * \class SdSpi
 * \brief SPI class for access to SD and SDHC flash memory cards.
 */
class SdSpi {
 public:
  /** Initialize the SPI bus */
  void begin();
  /** Set SPI options for access to SD/SDHC cards.
   * 
   * \param[in] spiDivisor SCK clock divider relative to the system clock.
   */
  void init(uint8_t spiDivisor);
  /** Receive a byte. 
   *
   * \return The byte.
   */
  uint8_t receive();
  /** Receive multiple bytes.
   *
   * \param[out] buf Buffer to receive the data.   
   * \param[in] n Number of bytes to receive.
   *
   * \return Zero for no error or nonzero error code.
   */   
  uint8_t receive(uint8_t* buf, size_t n);
  /** Send a byte.
   *
   * \param[in] data Byte to send
   */
  void send(uint8_t data);
   /** Send multiple bytes.
   *
   * \param[in] buf Buffer for data to be sent.   
   * \param[in] n Number of bytes to send.
   */   
  void send(const uint8_t* buf, size_t n);
};
//------------------------------------------------------------------------------
// Use of inline for AVR results in up to 10% better write performance.
// Inline also save a little flash memory.
/** inline avr native functions if nonzero. */
#define USE_AVR_NATIVE_SPI_INLINE 1
#if USE_NATIVE_AVR_SPI && USE_AVR_NATIVE_SPI_INLINE
inline uint8_t SdSpi::receive() {
  SPDR = 0XFF;
  while (!(SPSR & (1 << SPIF))) {}
  return SPDR;
}
inline uint8_t SdSpi::receive(uint8_t* buf, size_t n) {
  if (n-- == 0) return 0;
  SPDR = 0XFF;
  for (size_t i = 0; i < n; i++) {
    while (!(SPSR & (1 << SPIF))) {}
    uint8_t b = SPDR;
    SPDR = 0XFF;
    buf[i] = b;
  }
  while (!(SPSR & (1 << SPIF))) {}
  buf[n] = SPDR;
  return 0;
}
inline void SdSpi::send(uint8_t data) {
  SPDR = data;
  while (!(SPSR & (1 << SPIF))) {}
}
inline void SdSpi::send(const uint8_t* buf , size_t n) {
  if (n == 0) return;
  SPDR = buf[0];
  if (n > 1) {
    uint8_t b = buf[1];
    size_t i = 2;
    while (1) {
      while (!(SPSR & (1 << SPIF))) {}
      SPDR = b;
      if (i == n) break;
      b = buf[i++];
    }
  }
  while (!(SPSR & (1 << SPIF))) {}
}
#endif  // USE_NATIVE_AVR_SPI && USE_AVR_NATIVE_SPI_INLINE
#endif  // SdSpi_h


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
 */#include <SdSpi.h>
#if USE_NATIVE_AVR_SPI
//------------------------------------------------------------------------------
void SdSpi::begin() {
 // set SS high - may be chip select for another SPI device
  digitalWrite(SS, HIGH);

  // SS must be in output mode even it is not chip select
  pinMode(SS, OUTPUT);
  pinMode(MISO, INPUT);
  pinMode(MOSI, OUTPUT);
  pinMode(SCK, OUTPUT);
}
//------------------------------------------------------------------------------
void SdSpi::init(uint8_t sckDivisor) {
  uint8_t r = 0;
  
  for (uint8_t b = 2; sckDivisor > b && r < 6; b <<= 1, r++);
  // See avr processor documentation
  SPCR = (1 << SPE) | (1 << MSTR) | (r >> 1);
  SPSR = r & 1 || r == 6 ? 0 : 1 << SPI2X;
}
#if !USE_AVR_NATIVE_SPI_INLINE
//------------------------------------------------------------------------------
uint8_t SdSpi::receive() {
  SPDR = 0XFF;
  while (!(SPSR & (1 << SPIF)));
  return SPDR;
}
//------------------------------------------------------------------------------
uint8_t SdSpi::receive(uint8_t* buf, size_t n) {
  if (n-- == 0) return 0;
  SPDR = 0XFF;
  for (size_t i = 0; i < n; i++) {
    while (!(SPSR & (1 << SPIF)));
    uint8_t b = SPDR;
    SPDR = 0XFF;
    buf[i] = b;
  }
  while (!(SPSR & (1 << SPIF)));
  buf[n] = SPDR;
  return 0;
}
//------------------------------------------------------------------------------
void SdSpi::send(uint8_t data) {
  SPDR = data;
  while (!(SPSR & (1 << SPIF)));
}
//------------------------------------------------------------------------------
void SdSpi::send(const uint8_t* buf , size_t n) {
  if (n == 0) return;
  SPDR = buf[0];
  if (n > 1) {
    uint8_t b = buf[1];
    size_t i = 2;
    while (1) {
      while (!(SPSR & (1 << SPIF)));
      SPDR = b;
      if (i == n) break;
      b = buf[i++];
    }
  }
  while (!(SPSR & (1 << SPIF)));
}
#endif  // !USE_AVR_NATIVE_SPI_INLINE
#endif  // USE_NATIVE_AVR_SPI
//==============================================================================
#if USE_AVR_SOFTWARE_SPI
#include <SoftSPI.h>
static
SoftSPI<SOFT_SPI_MISO_PIN, SOFT_SPI_MOSI_PIN, SOFT_SPI_SCK_PIN, 0> softSpiBus;
//------------------------------------------------------------------------------
/**
 * initialize SPI pins
 */
void SdSpi::begin() {
  softSpiBus.begin();
}
//------------------------------------------------------------------------------
/**
 * Initialize hardware SPI - dummy for soft SPI
 */
void SdSpi::init(uint8_t sckDivisor) {}
//------------------------------------------------------------------------------
/** Soft SPI receive byte */
uint8_t SdSpi::receive() {
  return softSpiBus.receive();
}
//------------------------------------------------------------------------------
/** Soft SPI read data */
uint8_t SdSpi::receive(uint8_t* buf, size_t n) {
  for (size_t i = 0; i < n; i++) {
    buf[i] = receive();
  }
  return 0;
}
//------------------------------------------------------------------------------
/** Soft SPI send byte */
void SdSpi::send(uint8_t data) {
  softSpiBus.send(data);
}
//------------------------------------------------------------------------------
void SdSpi::send(const uint8_t* buf , size_t n) {
  for (size_t i = 0; i < n; i++) {
    send(buf[i]);
  }
}
#endif  // USE_AVR_SOFTWARE_SPI

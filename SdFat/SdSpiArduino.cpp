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
#include <SdSpi.h>
#if USE_ARDUINO_SPI_LIBRARY
#include <SPI.h>
//------------------------------------------------------------------------------
void SdSpi::begin() {
  SPI.begin();
}
//------------------------------------------------------------------------------
void SdSpi::init(uint8_t sckDivisor) {
  SPI.setBitOrder(MSBFIRST);
  SPI.setDataMode(SPI_MODE0);
#ifndef SPI_CLOCK_DIV128
  SPI.setClockDivider(sckDivisor);
#else  // SPI_CLOCK_DIV128
  int v;
  if (sckDivisor <= 2) v = SPI_CLOCK_DIV2;
  else  if (sckDivisor <= 4) v = SPI_CLOCK_DIV4;
  else  if (sckDivisor <= 8) v = SPI_CLOCK_DIV8;
  else  if (sckDivisor <= 16) v = SPI_CLOCK_DIV16;
  else  if (sckDivisor <= 32) v = SPI_CLOCK_DIV32;
  else  if (sckDivisor <= 64) v = SPI_CLOCK_DIV64;
  else  v = SPI_CLOCK_DIV128;
  SPI.setClockDivider(v);
#endif  // SPI_CLOCK_DIV128
}
//------------------------------------------------------------------------------
/** SPI receive a byte */
uint8_t SdSpi::receive() {
  return SPI.transfer(0XFF);
}
//------------------------------------------------------------------------------
/** SPI receive multiple bytes */
uint8_t SdSpi::receive(uint8_t* buf, size_t n) {
  for (size_t i = 0; i < n; i++) {
    buf[i] = SPI.transfer(0XFF);
  }
  return 0;
}
//------------------------------------------------------------------------------
/** SPI send a byte */
void SdSpi::send(uint8_t b) {
  SPI.transfer(b);
}
//------------------------------------------------------------------------------
/** SPI send multiple bytes */
void SdSpi::send(const uint8_t* buf , size_t n) {
  for (size_t i = 0; i < n; i++) {
    SPI.transfer(buf[i]);
  }
}
#endif  // USE_ARDUINO_SPI_LIBRARY

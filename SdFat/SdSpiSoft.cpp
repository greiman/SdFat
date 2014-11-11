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
#if USE_SOFTWARE_SPI
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
#endif  // USE_SOFTWARE_SPI
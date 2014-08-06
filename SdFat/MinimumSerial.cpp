/* Arduino SdFat Library
 * Copyright (C) 2012 by William Greiman
 *
 * This file is part of the Arduino SdFat Library
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
 * along with the Arduino SdFat Library.  If not, see
 * <http://www.gnu.org/licenses/>.
 */
#include <Arduino.h>
#if defined(UDR0) || defined(DOXYGEN)
#include <MinimumSerial.h>
//------------------------------------------------------------------------------
/**
 * Set baud rate for serial port zero and enable in non interrupt mode.
 * Do not call this function if you use another serial library.
 * \param[in] baud rate
 */
void MinimumSerial::begin(uint32_t baud) {
  uint16_t baud_setting;
  // don't worry, the compiler will squeeze out F_CPU != 16000000UL
  if (F_CPU != 16000000UL || baud != 57600) {
    // Double the USART Transmission Speed
    UCSR0A = 1 << U2X0;
    baud_setting = (F_CPU / 4 / baud - 1) / 2;
  } else {
    // hardcoded exception for compatibility with the bootloader shipped
    // with the Duemilanove and previous boards and the firmware on the 8U2
    // on the Uno and Mega 2560.
    UCSR0A = 0;
    baud_setting = (F_CPU / 8 / baud - 1) / 2;
  }
  // assign the baud_setting
  UBRR0H = baud_setting >> 8;
  UBRR0L = baud_setting;
  // enable transmit and receive
  UCSR0B |= (1 << TXEN0) | (1 << RXEN0);
}
//------------------------------------------------------------------------------
/**
 *  Unbuffered read
 *  \return -1 if no character is available or an available character.
 */
int MinimumSerial::read() {
  if (UCSR0A & (1 << RXC0)) return UDR0;
  return -1;
}
//------------------------------------------------------------------------------
/**
 * Unbuffered write
 *
 * \param[in] b byte to write.
 * \return 1
 */
size_t MinimumSerial::write(uint8_t b) {
  while (((1 << UDRIE0) & UCSR0B) || !(UCSR0A & (1 << UDRE0))) {}
  UDR0 = b;
  return 1;
}
MinimumSerial MiniSerial;
#endif  //  defined(UDR0) || defined(DOXYGEN)

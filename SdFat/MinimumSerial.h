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
#ifndef MinimumSerial_h
#define MinimumSerial_h
/**
 * \class MinimumSerial
 * \brief mini serial class for the %SdFat library.
 */
class MinimumSerial : public Print {
 public:
  void begin(uint32_t baud);
  int read();
  size_t write(uint8_t b);
  using Print::write;
};
#ifdef UDR0
extern MinimumSerial MiniSerial;
#endif  // UDR0
#endif  // MinimumSerial_h

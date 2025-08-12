/*
  Compat.h - Compatibility layer for Arduino API
  Copyright (c) 2018 Arduino LLC. All right reserved.

  This library is free software; you can redistribute it and/or
  modify it under the terms of the GNU Lesser General Public
  License as published by the Free Software Foundation; either
  version 2.1 of the License, or (at your option) any later version.

  This library is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
  Lesser General Public License for more details.

  You should have received a copy of the GNU Lesser General Public
  License along with this library; if not, write to the Free Software
  Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA 02110-1301 USA
*/

#ifndef __COMPAT_H__
#define __COMPAT_H__

namespace arduino {

inline void pinMode(pin_size_t pinNumber, int mode) {
	pinMode(pinNumber, (PinMode)mode);
};

inline void digitalWrite(pin_size_t pinNumber, int status) {
	digitalWrite(pinNumber, (PinStatus)status);
};

}

#endif
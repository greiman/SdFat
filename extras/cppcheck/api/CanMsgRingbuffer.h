/*
  CanMsgRingbuffer.h - Library for CAN message handling
  Copyright (c) 2023 Arduino. All right reserved.

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

#ifndef ARDUINOCORE_API_CAN_MSG_RING_BUFFER_H_
#define ARDUINOCORE_API_CAN_MSG_RING_BUFFER_H_

/**************************************************************************************
 * INCLUDE
 **************************************************************************************/

#include <stdint.h>

#include "CanMsg.h"

/**************************************************************************************
 * NAMESPACE
 **************************************************************************************/

namespace arduino
{

/**************************************************************************************
 * CLASS DECLARATION
 **************************************************************************************/

class CanMsgRingbuffer
{
public:
  static size_t constexpr RING_BUFFER_SIZE = 32U;

  CanMsgRingbuffer();

  inline bool isFull() const { return (_num_elems == RING_BUFFER_SIZE); }
  void enqueue(CanMsg const & msg);

  inline bool isEmpty() const { return (_num_elems == 0); }
  CanMsg dequeue();

  inline size_t available() const { return _num_elems; }

private:
  CanMsg _buf[RING_BUFFER_SIZE];
  volatile size_t _head;
  volatile size_t _tail;
  volatile size_t _num_elems;

  inline size_t next(size_t const idx) const { return ((idx + 1) % RING_BUFFER_SIZE); }
};

/**************************************************************************************
 * NAMESPACE
 **************************************************************************************/

} /* arduino */

#endif /* ARDUINOCORE_API_CAN_MSG_RING_BUFFER_H_ */

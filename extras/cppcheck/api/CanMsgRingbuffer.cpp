/*
  CanMsgRingbuffer.cpp - Library for CAN message handling
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

/**************************************************************************************
 * INCLUDE
 **************************************************************************************/

#include "CanMsgRingbuffer.h"

/**************************************************************************************
 * NAMESPACE
 **************************************************************************************/

namespace arduino
{

/**************************************************************************************
 * CTOR/DTOR
 **************************************************************************************/

CanMsgRingbuffer::CanMsgRingbuffer()
: _head{0}
, _tail{0}
, _num_elems{0}
{
}

/**************************************************************************************
 * PUBLIC MEMBER FUNCTIONS
 **************************************************************************************/

void CanMsgRingbuffer::enqueue(CanMsg const & msg)
{
  if (isFull())
    return;

  _buf[_head] = msg;
  _head = next(_head);
  _num_elems = _num_elems + 1;
}

CanMsg CanMsgRingbuffer::dequeue()
{
  if (isEmpty())
    return CanMsg();

  CanMsg const msg = _buf[_tail];
  _tail = next(_tail);
  _num_elems = _num_elems - 1;

  return msg;
}

/**************************************************************************************
 * NAMESPACE
 **************************************************************************************/

} /* arduino */

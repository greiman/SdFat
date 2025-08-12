/*
  CanMsg.cpp - Library for CAN message handling
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

#include "CanMsg.h"

/**************************************************************************************
 * NAMESPACE
 **************************************************************************************/

namespace arduino
{

/**************************************************************************************
 * STATIC CONST DEFINITION
 **************************************************************************************/

uint8_t  const CanMsg::MAX_DATA_LENGTH;
uint32_t const CanMsg::CAN_EFF_FLAG;
uint32_t const CanMsg::CAN_SFF_MASK;
uint32_t const CanMsg::CAN_EFF_MASK;

/**************************************************************************************
 * NAMESPACE
 **************************************************************************************/

} /* arduino */

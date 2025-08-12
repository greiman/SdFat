/*
  HardwareCAN.h - CAN bus interface for Arduino core
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

#ifndef ARDUINOCORE_API_HARDWARECAN_H
#define ARDUINOCORE_API_HARDWARECAN_H

/**************************************************************************************
 * INCLUDE
 **************************************************************************************/

#include "CanMsg.h"
#include "CanMsgRingbuffer.h"

/**************************************************************************************
 * TYPEDEF
 **************************************************************************************/

enum class CanBitRate : int
{
  BR_125k  =  125000,
  BR_250k  =  250000,
  BR_500k  =  500000,
  BR_1000k = 1000000,
};

/**************************************************************************************
 * NAMESPACE
 **************************************************************************************/

namespace arduino
{

/**************************************************************************************
 * CLASS DECLARATION
 **************************************************************************************/

class HardwareCAN
{
public:
  virtual ~HardwareCAN() {}


  /**
   * Initialize the CAN controller.
   *
   * @param can_bitrate the bus bit rate
   * @return true if initialization succeeded and the controller is operational
   */
  virtual bool begin(CanBitRate const can_bitrate) = 0;

  /**
   * Disable the CAN controller.
   *
   * Whether any messages that are buffered will be sent is _implementation defined_.
   */
  virtual void end() = 0;

  /**
   * Enqueue a message for transmission to the CAN bus.
   *
   * This call returns when the message has been enqueued for transmission.
   * Due to bus arbitration and error recovery there may be a substantial delay
   * before the message is actually sent.
   *
   * An implementation must ensure that all messages with the same CAN priority
   * are sent in the order in which they are enqueued.
   *
   * It is _implementation defined_ whether multiple messages can be enqueued
   * for transmission, and if messages with higher CAN priority can preempt the
   * transmission of previously enqueued messages. The default configuration for
   * and implementation should not allow multiple messages to be enqueued.
   *
   * @param msg the message to send
   * @return 1 if the message was enqueued, an _implementation defined_ error code < 0 if there was an error
   * @todo define specific error codes, especially "message already pending"
   */
  virtual int write(CanMsg const &msg) = 0;

  /**
   * Determine if any messages have been received and buffered.
   *
   * @return the number of unread messages that have been received
   */
  virtual size_t available() = 0;

  /**
   * Returns the first message received, or an empty message if none are available.
   *
   * Messages must be returned in the order received.
   *
   * @return the first message in the receive buffer
   */
  virtual CanMsg read() = 0;
};

/**************************************************************************************
 * NAMESPACE
 **************************************************************************************/

} /* arduino */

#endif /* ARDUINOCORE_API_HARDWARECAN_H */

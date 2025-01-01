/**
 * Copyright (c) 2011-2024 Bill Greiman
 * This file is part of the SdFat library for SD memory cards.
 *
 * MIT License
 *
 * Permission is hereby granted, free of charge, to any person obtaining a
 * copy of this software and associated documentation files (the "Software"),
 * to deal in the Software without restriction, including without limitation
 * the rights to use, copy, modify, merge, publish, distribute, sublicense,
 * and/or sell copies of the Software, and to permit persons to whom the
 * Software is furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included
 * in all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS
 * OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
 * FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER
 * DEALINGS IN THE SOFTWARE.
 */
#pragma once
#include <Arduino.h>

#include "DbgLog.h"
//------------------------------------------------------------------------------
static inline void gpioStatus(uint gpio) {
  logmsgln("gpio", gpio, " drive: ", gpio_get_drive_strength(gpio));
  //  logmsgln("gpio", gpio,
  //           " drive: ", static_cast<int>(gpio_get_drive_strength(gpio)));
  logmsgln("gpio", gpio, " slew:  ", gpio_get_slew_rate(gpio));
  logmsgln("gpio", gpio, " hyst:  ", gpio_is_input_hysteresis_enabled(gpio));
  logmsgln("gpio", gpio, " pull:  ", gpio_is_pulled_up(gpio));
}
//------------------------------------------------------------------------------
static inline void pioRegs(PIO pio) {
  logmsgln("ctrl:    0b", Bin(pio->ctrl));
  logmsgln("fstat:   0b", Bin(pio->fstat));
  logmsgln("fdebug:  0b", Bin(pio->fdebug));
  logmsgln("flevel:  0b", Bin(pio->flevel));
  logmsgln("padout:  0b", Bin(pio->dbg_padout));
  logmsgln("padoe:   0b", Bin(pio->dbg_padoe));
  logmsgln("cfginfo: 0x", Hex(pio->dbg_cfginfo));
  logmsgln("sync_bypass: 0b", Bin(pio->input_sync_bypass));
}
//------------------------------------------------------------------------------
static inline void pioSmRegs(PIO pio, uint sm) {
  logmsgln("sm", sm, " clkdiv: 0x", Hex(pio->sm[sm].clkdiv));
  logmsgln("sm", sm, " execctrl: 0x", Hex(pio->sm[sm].execctrl));
  logmsgln("sm", sm, " shiftctrl: 0x", Hex(pio->sm[sm].shiftctrl));
  logmsgln("sm", sm, " addr: 0x", Hex(pio->sm[sm].addr));
  logmsgln("sm", sm, " pinctrl: 0x", Hex(pio->sm[sm].pinctrl));
}

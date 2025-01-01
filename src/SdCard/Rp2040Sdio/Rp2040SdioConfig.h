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
/**
 * \class SdioConfig
 * \brief SDIO card configuration.
 */
class SdioConfig {
 public:
  /**
   * SdioConfig constructor.
   * \param[in] clkPin gpio pin for SDIO CLK.
   * \param[in] cmdPin gpio pin for SDIO CMD.
   * \param[in] dat0Pin gpio start pin for SDIO DAT[4].
   */
  SdioConfig(uint clkPin, uint cmdPin, uint dat0Pin)
      : m_clkPin(clkPin), m_cmdPin(cmdPin), m_dat0Pin(dat0Pin) {}
  /** \return gpio for SDIO CLK */
  uint clkPin() { return m_clkPin; }
  /** \return gpio for SDIO CMD */
  uint cmdPin() { return m_cmdPin; }
  /** \return gpio for SDIO DAT0 */
  uint dat0Pin() { return m_dat0Pin; }

 private:
  SdioConfig() : m_clkPin(31u), m_cmdPin(31u), m_dat0Pin(31u) {}
  const uint8_t m_clkPin;
  const uint8_t m_cmdPin;
  const uint8_t m_dat0Pin;
};

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
#include <SdFat.h>
#ifndef PSTR
#define PSTR(x) x
#define PGM_P const char*
#endif
//------------------------------------------------------------------------------
static void pstrPrint(PGM_P str) {
  for (uint8_t c; (c = pgm_read_byte(str)); str++) SdFat::stdOut()->write(c);
}
//------------------------------------------------------------------------------
static void pstrPrintln(PGM_P str) {
  pstrPrint(str);
  SdFat::stdOut()->println();
}
//------------------------------------------------------------------------------
/** %Print any SD error code and halt. */
void SdFat::errorHalt() {
  errorPrint();
  while (1) {}
}
//------------------------------------------------------------------------------
/** %Print msg, any SD error code, and halt.
 *
 * \param[in] msg Message to print.
 */
void SdFat::errorHalt(char const* msg) {
  errorPrint(msg);
  while (1) {}
}
//------------------------------------------------------------------------------
/** %Print msg, any SD error code, and halt.
 *
 * \param[in] msg Message in program space (flash memory) to print.
 */
void SdFat::errorHalt_P(PGM_P msg) {
  errorPrint_P(msg);
  while (1) {}
}
//------------------------------------------------------------------------------
/** %Print any SD error code. */
void SdFat::errorPrint() {
  if (!m_card.errorCode()) return;
  pstrPrint(PSTR("SD errorCode: 0X"));
  m_stdOut->print(m_card.errorCode(), HEX);
  pstrPrint(PSTR(",0X"));
  m_stdOut->println(m_card.errorData(), HEX);
}
//------------------------------------------------------------------------------
/** %Print msg, any SD error code.
 *
 * \param[in] msg Message to print.
 */
void SdFat::errorPrint(char const* msg) {
  pstrPrint(PSTR("error: "));
  m_stdOut->println(msg);
  errorPrint();
}
//------------------------------------------------------------------------------
/** %Print msg, any SD error code.
 *
 * \param[in] msg Message in program space (flash memory) to print.
 */
void SdFat::errorPrint_P(PGM_P msg) {
  pstrPrint(PSTR("error: "));
  pstrPrintln(msg);
  errorPrint();
}
//------------------------------------------------------------------------------
/** %Print error details and halt after SdFat::init() fails. */
void SdFat::initErrorHalt() {
  initErrorPrint();
  while (1) {}
}
//------------------------------------------------------------------------------
/**Print message, error details, and halt after SdFat::init() fails.
 *
 * \param[in] msg Message to print.
 */
void SdFat::initErrorHalt(char const *msg) {
  m_stdOut->println(msg);
  initErrorHalt();
}
//------------------------------------------------------------------------------
/**Print message, error details, and halt after SdFat::init() fails.
 *
 * \param[in] msg Message in program space (flash memory) to print.
 */
void SdFat::initErrorHalt_P(PGM_P msg) {
  pstrPrintln(msg);
  initErrorHalt();
}
//------------------------------------------------------------------------------
/** Print error details after SdFat::init() fails. */
void SdFat::initErrorPrint() {
  if (m_card.errorCode()) {
    pstrPrintln(PSTR("Can't access SD card. Do not reformat."));
    if (m_card.errorCode() == SD_CARD_ERROR_CMD0) {
      pstrPrintln(PSTR("No card, wrong chip select pin, or SPI problem?"));
    }
    errorPrint();
  } else if (m_vol.fatType() == 0) {
    pstrPrintln(PSTR("Invalid format, reformat SD."));
  } else if (!m_vwd.isOpen()) {
    pstrPrintln(PSTR("Can't open root directory."));
  } else {
    pstrPrintln(PSTR("No error found."));
  }
}
//------------------------------------------------------------------------------
/**Print message and error details and halt after SdFat::init() fails.
 *
 * \param[in] msg Message to print.
 */
void SdFat::initErrorPrint(char const *msg) {
  m_stdOut->println(msg);
  initErrorPrint();
}
//------------------------------------------------------------------------------
/**Print message and error details after SdFat::init() fails.
 *
 * \param[in] msg Message in program space (flash memory) to print.
 */
void SdFat::initErrorPrint_P(PGM_P msg) {
  pstrPrintln(msg);
  initErrorHalt();
}

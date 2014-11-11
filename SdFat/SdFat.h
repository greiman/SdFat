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
#ifndef SdFat_h
#define SdFat_h
/**
 * \file
 * \brief SdFat class
 */
//------------------------------------------------------------------------------
/** Macro for debug. */
#define DBG_FAIL_MACRO  // Serial.print(__FILE__);Serial.println(__LINE__)
//------------------------------------------------------------------------------
/** SdFat version YYYYMMDD */
#define SD_FAT_VERSION 20141111
//------------------------------------------------------------------------------
/** error if old IDE */
#if !defined(ARDUINO) || ARDUINO < 100
#error Arduino IDE must be 1.0 or greater
#endif  // ARDUINO < 100
//------------------------------------------------------------------------------
#include <SdFile.h>
#include <SdStream.h>
#include <StdioStream.h>
#include <ArduinoStream.h>
#include <MinimumSerial.h>
//------------------------------------------------------------------------------
/**
 * \class SdFat
 * \brief Integration class for the %SdFat library.
 */
class SdFat {
 public:
  SdFat() {}
  /** \return a pointer to the Sd2Card object. */
  Sd2Card* card() {return &m_card;}
  bool chdir(bool set_cwd = false);
  bool chdir(const char* path, bool set_cwd = false);
  void chvol();
  void errorHalt();
  void errorHalt(char const *msg);
  void errorPrint();
  void errorPrint(char const *msg);
  bool exists(const char* name);
  bool begin(uint8_t chipSelectPin = SD_CHIP_SELECT_PIN,
    uint8_t sckDivisor = SPI_FULL_SPEED);
  void initErrorHalt();
  void initErrorHalt(char const *msg);
  void initErrorPrint();
  void initErrorPrint(char const *msg);
  void ls(uint8_t flags = 0);
  void ls(const char* path, uint8_t flags = 0);
  void ls(Print* pr, uint8_t flags = 0);
  void ls(Print* pr, const char* path, uint8_t flags = 0);
  bool mkdir(const char* path, bool pFlag = true);
  bool remove(const char* path);
  bool rename(const char *oldPath, const char *newPath);
  bool rmdir(const char* path);
  bool truncate(const char* path, uint32_t length);
  /** \return a pointer to the SdVolume object. */
  SdVolume* vol() {return &m_vol;}
  /** \return a pointer to the volume working directory. */
  SdBaseFile* vwd() {return &m_vwd;}
  //----------------------------------------------------------------------------
  void errorHalt_P(PGM_P msg);
  void errorPrint_P(PGM_P msg);
  void initErrorHalt_P(PGM_P msg);
  void initErrorPrint_P(PGM_P msg);
  //----------------------------------------------------------------------------
  /**
   *  Set stdOut Print stream for messages.
   * \param[in] stream The new Print stream.
   */
  static void setStdOut(Print* stream) {m_stdOut = stream;}
  /** \return Print stream for messages. */
  static Print* stdOut() {return m_stdOut;}
  //----------------------------------------------------------------------------
  /** open a file 
   *
   * \param[in] path location of file to be opened.
   * \param[in] mode open mode flags.
   * \return a File object.
   */  
  File open(const char *path, uint8_t mode = FILE_READ) {
    File tmpFile;
    tmpFile.open(&m_vwd, path, mode);
    return tmpFile;
  }

 private:
  Sd2Card m_card;
  SdVolume m_vol;
  SdBaseFile m_vwd;
  static Print* m_stdOut;
};
#endif  // SdFat_h

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
/**
 * \file
 * \brief SdFile class
 */
#include <SdBaseFile.h>
#ifndef SdFile_h
#define SdFile_h
//------------------------------------------------------------------------------
/**
 * \class SdFile
 * \brief SdBaseFile with Print.
 */
class SdFile : public SdBaseFile, public Print {
 public:
  SdFile() {}
  SdFile(const char* name, uint8_t oflag);
#if DESTRUCTOR_CLOSES_FILE
  ~SdFile() {}
#endif  // DESTRUCTOR_CLOSES_FILE
  /** \return value of writeError */
  bool getWriteError() {return SdBaseFile::getWriteError();}
  /** Set writeError to zero */
  void clearWriteError() {SdBaseFile::clearWriteError();}
  size_t write(uint8_t b);
  int write(const char* str);
  int write(const void* buf, size_t nbyte);
  size_t write(const uint8_t *buf, size_t size) {
    return SdBaseFile::write(buf, size);}
  void write_P(PGM_P str);
  void writeln_P(PGM_P str);
};
#endif  // SdFile_h

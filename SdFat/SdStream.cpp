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

//==============================================================================
  /// @cond SHOW_PROTECTED
int16_t SdStreamBase::getch() {
  uint8_t c;
  int8_t s = read(&c, 1);
  if (s != 1) {
    if (s < 0) {
      setstate(badbit);
    } else {
      setstate(eofbit);
    }
    return -1;
  }
  if (c != '\r' || (getmode() & ios::binary)) return c;
  s = read(&c, 1);
  if (s == 1 && c == '\n') return c;
  if (s == 1) seekCur(-1);
  return '\r';
}
//------------------------------------------------------------------------------
void SdStreamBase::open(const char* path, ios::openmode mode) {
uint8_t flags;
  switch (mode & (app | in | out | trunc)) {
    case app | in:
    case app | in | out:
      flags = O_RDWR | O_APPEND | O_CREAT;
      break;

    case app:
    case app | out:
      flags = O_WRITE | O_APPEND | O_CREAT;
      break;

    case in:
      flags = O_READ;
      break;

    case in | out:
      flags = O_RDWR;
      break;

    case in | out | trunc:
      flags = O_RDWR | O_TRUNC | O_CREAT;
      break;

    case out:
    case out | trunc:
      flags = O_WRITE | O_TRUNC | O_CREAT;
      break;

    default:
      goto fail;
  }
  if (mode & ios::ate) flags |= O_AT_END;
  if (!SdBaseFile::open(path, flags)) goto fail;
  setmode(mode);
  clear();
  return;

 fail:
  SdBaseFile::close();
  setstate(failbit);
  return;
}
//------------------------------------------------------------------------------
void SdStreamBase::putch(char c) {
  if (c == '\n' && !(getmode() & ios::binary)) {
    write('\r');
  }
  write(c);
  if (writeError) setstate(badbit);
}
//------------------------------------------------------------------------------
void SdStreamBase::putstr(const char* str) {
  size_t n = 0;
  while (1) {
    char c = str[n];
    if (c == '\0' || (c == '\n' && !(getmode() & ios::binary))) {
      if (n > 0) write(str, n);
      if (c == '\0') break;
      write('\r');
      str += n;
      n = 0;
    }
    n++;
  }
  if (writeError) setstate(badbit);
}
//------------------------------------------------------------------------------
/** Internal do not use
 * \param[in] off
 * \param[in] way
 */
bool SdStreamBase::seekoff(off_type off, seekdir way) {
  pos_type pos;
  switch (way) {
    case beg:
      pos = off;
      break;

    case cur:
      pos = curPosition() + off;
      break;

    case end:
      pos = fileSize() + off;
      break;

    default:
      return false;
  }
  return seekpos(pos);
}
//------------------------------------------------------------------------------
/** Internal do not use
 * \param[in] pos
 */
bool SdStreamBase::seekpos(pos_type pos) {
  return seekSet(pos);
}
//------------------------------------------------------------------------------
int SdStreamBase::write(const void* buf, size_t n) {
  return SdBaseFile::write(buf, n);
}
//------------------------------------------------------------------------------
void SdStreamBase::write(char c) {
  write(&c, 1);
}
/// @endcond

/**
 * Copyright (c) 2011-2020 Bill Greiman
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
#define DBG_FILE "ExFatFilePrint.cpp"
#include "../common/DebugMacros.h"
#include "ExFatFile.h"
#include "upcase.h"
#include "ExFatVolume.h"
//-----------------------------------------------------------------------------
size_t ExFatFile::printFileSize(print_t* pr) {
  uint64_t n = m_validLength;
  char buf[21];
  char *str = &buf[sizeof(buf) - 1];
  char *bgn = str - 12;
  *str = '\0';
  do {
    uint64_t m = n;
    n /= 10;
    *--str = m - 10*n + '0';
  } while (n);
  while (str > bgn) {
    *--str = ' ';
  }
  return pr->write(str);
}
//-----------------------------------------------------------------------------
size_t ExFatFile::printAccessDateTime(print_t* pr) {
  DirFile_t* df = reinterpret_cast<DirFile_t*>
                 (m_vol->dirCache(&m_dirPos, FsCache::CACHE_FOR_READ));
  if (!df) {
    DBG_FAIL_MACRO;
    goto fail;
  }
  return fsPrintDateTime(pr, getLe32(df->accessTime));
fail:
  return 0;
}
//-----------------------------------------------------------------------------
size_t ExFatFile::printCreateDateTime(print_t* pr) {
  DirFile_t* df = reinterpret_cast<DirFile_t*>
                 (m_vol->dirCache(&m_dirPos, FsCache::CACHE_FOR_READ));
  if (!df) {
    DBG_FAIL_MACRO;
    goto fail;
  }
  return fsPrintDateTime(pr, getLe32(df->createTime));
fail:
  return 0;
}
//-----------------------------------------------------------------------------
size_t ExFatFile::printModifyDateTime(print_t* pr) {
  DirFile_t* df = reinterpret_cast<DirFile_t*>
                 (m_vol->dirCache(&m_dirPos, FsCache::CACHE_FOR_READ));
  if (!df) {
    DBG_FAIL_MACRO;
    goto fail;
  }
  return fsPrintDateTime(pr, getLe32(df->modifyTime));
fail:
  return 0;
}
//------------------------------------------------------------------------------
bool ExFatFile::ls(print_t* pr) {
  ExFatFile file;
  if (!isDir()) {
    DBG_FAIL_MACRO;
    goto fail;
  }
  rewind();
  while (file.openNext(this, O_RDONLY)) {
    if (!file.isHidden()) {
      file.printName(pr);
      if (file.isDir()) {
        pr->write('/');
      }
      pr->write('\r');
      pr->write('\n');
    }
    file.close();
  }
  if (getError()) {
    DBG_FAIL_MACRO;
    goto fail;
  }
  return true;

 fail:
  return false;
}
//------------------------------------------------------------------------------
bool ExFatFile::ls(print_t* pr, uint8_t flags, uint8_t indent) {
  ExFatFile file;
  if (!isDir()) {
    DBG_FAIL_MACRO;
    goto fail;
  }
  rewind();
  while (file.openNext(this, O_RDONLY)) {
    // indent for dir level
    if (!file.isHidden() || (flags & LS_A)) {
      for (uint8_t i = 0; i < indent; i++) {
        pr->write(' ');
      }
      if (flags & LS_DATE) {
        file.printModifyDateTime(pr);
        pr->write(' ');
      }
      if (flags & LS_SIZE) {
        file.printFileSize(pr);
        pr->write(' ');
      }
      file.printName(pr);
      if (file.isDir()) {
        pr->write('/');
      }
      pr->write('\r');
      pr->write('\n');
      if ((flags & LS_R) && file.isDir()) {
        file.ls(pr, flags, indent + 2);
      }
    }
    file.close();
  }
  if (getError()) {
    DBG_FAIL_MACRO;
    goto fail;
  }
  return true;

 fail:
  return false;
}




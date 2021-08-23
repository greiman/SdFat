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
#ifndef ExFatTypes_h
#define ExFatTypes_h
#include "ExFatConfig.h"
#include "../common/FsUtf.h"
#include "../common/FsName.h"
//------------------------------------------------------------------------------
/**
 * \struct DirPos_t
 * \brief Internal type for position in directory file.
 */
struct DirPos_t {
  /** current cluster */
  uint32_t cluster;
  /** offset */
  uint32_t position;
  /** directory is contiguous */
  bool     isContiguous;
};
//------------------------------------------------------------------------------
/**
 * \class ExName_t
 * \brief Internal type for file name - do not use in user apps.
 */
class ExName_t : public FsName {
 public:
  /** Length of UTF-16 name */
  size_t nameLength;
  /** Hash for UTF-16 name */
  uint16_t nameHash;
};
#endif  // ExFatTypes_h

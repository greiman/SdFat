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
#include "Printable.h"

#define NUM64_HEX_A 'a'
#define ENABLE_DBG_MSG 1
#define DBG_LOG_PORT Serial

#if defined(DBG_FILE)
#elif defined(__FILE_NAME__)
#define DBG_FILE __FILE_NAME__
#else
#define DBG_FILE __FILE__
#endif

#if ENABLE_DBG_MSG
#define DBG_MSG(...)                                          \
  do {                                                        \
    logmsgln(F(DBG_FILE), ":", __LINE__, " ", ##__VA_ARGS__); \
  } while (0)
#else  // ENABLE_DBG_MSG
#define DBG_MSG(...) \
  do {               \
  } while (0)
#endif  // ENABLE_DBG_MSG

// Print binary with byte and nibble separators.
class Bin : public Printable {
  // Bin not supported for 64-bits.
  explicit Bin(int64_t, uint8_t = 0) {}
  explicit Bin(uint64_t, uint8_t = 0) {}

 public:
  template <typename T>
  explicit Bin(T n, uint8_t p = 0) : n_(n), p_(p) {}
  size_t printTo(Print& pr) const {
    auto n = n_;
    uint8_t p = p_ > 8 * sizeof(n) ? 8 * sizeof(n) : p_;
    char buf[10 * sizeof(n) + 1];
    char* end = buf + sizeof(buf);
    char* str = end;
    uint8_t i = 0;
    do {
      if (i && (i % 4) == 0) {
        *--str = i % 8 ? '\'' : '|';
      }
      *--str = n & 1 ? '1' : '0';
      n /= 2;
      i++;
    } while (n || i < p);
    return pr.write(str, end - str);
  }
  uint32_t n_;
  uint8_t p_;
};

// Print floating point with precision.
struct Dbl : public Printable {
  Dbl(double n_, int p_) : n(n_), p(p_) {}
  size_t printTo(Print& pr) const { return pr.print(n, p); }
  double n;
  int p;
};

// Print in hex format.
#if __cplusplus > 201700L
template <typename T>
struct Hex : public Printable {
  explicit Hex(T n_) : n(n_) {}
  size_t printTo(Print& pr) const { return pr.print(n, HEX); }
  T n;
};
#else   // __cplusplus > 201700L
class Hex : public Printable {
  // No 64-bit support unless C++17 or better.
  explicit Hex(int64_t) {}
  explicit Hex(uint64_t) {}

 public:
  template <typename T>
  explicit Hex(T n_) : n(n_) {}
  size_t printTo(Print& pr) const { return pr.print(n, HEX); }
  uint32_t n;
};
#endif  // __cplusplus > 201700L

// For boards with no 64-bit print, lower case hex, alt binary or precision.
class Num : public Printable {
  uint64_t n_;
  uint8_t b_;
  uint8_t p_;
  char s_;

 public:
  template <typename T>
  explicit Num(T n, uint8_t b = 0, uint8_t p = 0, char s = 0) : b_(b), p_(p) {
    n_ = b == 10 && n < 0 ? -n : n;
    s_ = b == 10 && n < 0 ? '-' : s;
  }
  size_t printTo(Print& pr) const {
    char buf[8 * sizeof(uint64_t) + 1];
    char* end = buf + sizeof(buf);
    char* str = end;
    uint64_t n = n_;
    uint8_t p = p_ > 8 * sizeof(n) ? 8 * sizeof(n) : p_;
    uint8_t base = b_ < 2 || b_ > 16 ? 10 : b_;
    uint8_t i = 0;
    do {
      uint8_t d = n % base;
      *--str = d < 10 ? d + '0' : d + NUM64_HEX_A - 10;
      n /= base;
      i++;
    } while (n || i < p);
    if (s_) {
      *--str = s_;
    }
    return pr.write(str, end - str);
  }
};

template <typename T>
size_t logmsg(T arg) {
  return DBG_LOG_PORT.print(arg);
}
size_t logmsg(bool b) { return logmsg(b ? F("true") : F("false")); }

inline size_t logmsg() { return 0; }

template <typename T, typename... Types>
size_t logmsg(T var1, Types... var2) {
  size_t n = logmsg(var1);
  return n += logmsg(var2...);
}

template <typename... Types>
size_t logmsgln(Types... params) {
  size_t n = logmsg(params...);
  return n + logmsg("\r\n");
}

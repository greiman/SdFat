/**
 * Copyright (c) 2011-2025 Bill Greiman
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
/**
 * \file
 * \brief Classes for Debug messages.
 */
#pragma once
#include "Printable.h"

/** Character used in Num class for HEX format */
#define NUM64_HEX_A 'a'

/** Enable or disable debug messages */
#define ENABLE_DBG_MSG 0

/** Port for debug messages */
#define DBG_LOG_PORT Serial

/** Filename to print in message. */
#if defined(DBG_FILE)
#elif defined(__FILE_NAME__)
#define DBG_FILE __FILE_NAME__
#else
#define DBG_FILE __FILE__
#endif

/** Macro for debug messages */
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

/**
 * \class Bin
 * \brief Print binary with byte and nibble separators.
 */
class Bin : public Printable {
  uint32_t n_;
  uint8_t p_;
  // Bin not supported for 64-bits.
  // cppcheck-suppress-begin uninitMemberVarPrivate
  explicit Bin(int64_t, uint8_t = 0) {}
  explicit Bin(uint64_t, uint8_t = 0) {}
  // cppcheck-suppress-end uninitMemberVarPrivate
 public:
  template <typename T>
  /**
   * \param[in] n Number to print.
   * \param[in] p Precision.
   */
  explicit Bin(T n, uint8_t p = 0) : n_(n), p_(p) {}
  /**
   * \param[in] pr Print stream.
   * \return Number of bytes printed.
   */
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
};
/**
 * \class Dbl
 * \brief Print floating point with precision.
 */
class Dbl : public Printable {
  double n_;
  int p_;

 public:
  /**
   * \param[in] n Number to print.
   * \param[in] p Precision.
   */
  Dbl(double n, int p) : n_(n), p_(p) {}
  /**
   * \param[in] pr Print stream.
   * \return Number of bytes printed.
   */
  size_t printTo(Print& pr) const { return pr.print(n_, p_); }
};
/**
 * \class Hex
 * \brief Print in hex format.
 */
#if __cplusplus > 201700L
template <typename T>
class Hex : public Printable {
  T n_;

 public:
  /**
   * \param[in] n Number to print.
   */
  explicit Hex(T n) : n_(n) {}
  /**
   * \param[in] pr Print stream.
   * \return Number of bytes printed.
   */
  size_t printTo(Print& pr) const { return pr.print(n_, HEX); }
};
#else   // __cplusplus > 201700L
class Hex : public Printable {
  uint32_t n_;
  // No 64-bit support unless C++17 or better.
  explicit Hex(int64_t) {}
  explicit Hex(uint64_t) {}

 public:
  /**
   * \param[in] n Number to print.
   */
  template <typename T>
  explicit Hex(T n) : n_(n) {}
  /**
   * \param[in] pr Print stream.
   * \return Number of bytes printed.
   */
  size_t printTo(Print& pr) const { return pr.print(n_, HEX); }
};
#endif  // __cplusplus > 201700L
/**
 * \class Num
 * \brief Print 64-bit print, lower case hex, alt binary or precision.
 */
class Num : public Printable {
  uint64_t n_;
  uint8_t b_;
  uint8_t p_;
  char s_;

 public:
  /**
   * \param[in] n Number to print.
   * \param[in] b Base.
   * \param[in] p Precision.
   * \param[in] s Sign.
   */
  template <typename T>
  explicit Num(T n, uint8_t b = 0, uint8_t p = 0, char s = 0) : b_(b), p_(p) {
    n_ = b == 10 && n < 0 ? -n : n;
    s_ = b == 10 && n < 0 ? '-' : s;
  }
  /**
   * \param[in] pr Print stream.
   * \return Number of bytes printed.
   */
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
/**
 * \param[in] arg Item to print.
 * \return Number of bytes printed.
 */
template <typename T>
inline size_t logmsg(T arg) {
  return DBG_LOG_PORT.print(arg);
}
/**
 * \param[in] b Item to print.
 * \return Number of bytes printed.
 */
inline size_t logmsg(bool b) { return logmsg(b ? F("true") : F("false")); }

/** \return Zero to end recursive template */
inline size_t logmsg() { return 0; }

/**
 * \param[in] var1 Next item to print.
 * \param[in] vars Rest of items to print.
 * \return number of bytes printed.
 */
template <typename T, typename... Types>
inline size_t logmsg(T var1, Types... vars) {
  size_t n = logmsg(var1);
  return n += logmsg(vars...);
}
/**
 * \param[in] vars List of items to print.
 * \return number of bytes printed.
 */
template <typename... Types>
inline size_t logmsgln(Types... vars) {
  size_t n = logmsg(vars...);
  return n + logmsg("\r\n");  // cppcheck-suppress incorrectStringBooleanError
}

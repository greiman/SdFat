/* Teensyduino Core Library
 * http://www.pjrc.com/teensy/
 * Copyright (c) 2017 PJRC.COM, LLC.
 *
 * Permission is hereby granted, free of charge, to any person obtaining
 * a copy of this software and associated documentation files (the
 * "Software"), to deal in the Software without restriction, including
 * without limitation the rights to use, copy, modify, merge, publish,
 * distribute, sublicense, and/or sell copies of the Software, and to
 * permit persons to whom the Software is furnished to do so, subject to
 * the following conditions:
 *
 * 1. The above copyright notice and this permission notice shall be
 * included in all copies or substantial portions of the Software.
 *
 * 2. If the Software is incorporated into a build system that allows
 * selection among a list of target devices, then similar target
 * devices manufactured by PJRC.COM must be included in the list of
 * target devices and selectable in the same manner.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
 * EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
 * MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND
 * NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS
 * BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN
 * ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN
 * CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 * SOFTWARE.
 */

#ifndef Print_h
#define Print_h

#include <inttypes.h>
#include <stdio.h> // for size_t - gives sprintf and other stuff to all sketches & libs
#include <stdarg.h>
#include "core_id.h"
#include "WString.h"
#include "Printable.h"

#define DEC 10
#define HEX 16
#define OCT 8
#define BIN 2

// BYTE was defined in very old versions of Arduino
// maybe this now causes more trouble than it's worth?
//#ifndef BYTE
//#define BYTE 0
//#endif

class __FlashStringHelper;

class Print
{
  public:
	constexpr Print() : write_error(0) {}
	virtual size_t write(uint8_t b) = 0;
	size_t write(const char *str)			{ if (str == nullptr) return 0;
							  return write((const uint8_t *)str, strlen(str)); }
	virtual size_t write(const uint8_t *buffer, size_t size);
	virtual int availableForWrite(void)		{ return 0; }
	virtual void flush()				{ }
	size_t write(const char *buffer, size_t size)	{ return write((const uint8_t *)buffer, size); }
	// Print a string
	size_t print(const String &s);
	// Print a single character
	size_t print(char c)				{ return write((uint8_t)c); }
	// Print a string
	size_t print(const char s[])			{ return write(s); }
	// Print a string
	size_t print(const __FlashStringHelper *f)	{ return write((const char *)f); }
	// Print an unsigned number
	size_t print(uint8_t b)				{ return printNumber(b, 10, 0); }
	// Print a signed number
	size_t print(int n)				{ return print((long)n); }
	// Print an unsigned number
	size_t print(unsigned int n)			{ return printNumber(n, 10, 0); }
	// Print a signed number
	size_t print(long n);
	// Print an unsigned number
	size_t print(unsigned long n)			{ return printNumber(n, 10, 0); }
	// Print a signed number
	size_t print(int64_t n);
	// Print an unsigned number
	size_t print(uint64_t n)			{ return printNumber64(n, 10, 0); }

	// Print a number in any number base (eg, BIN, HEX, OCT)
	size_t print(unsigned char n, int base)		{ return printNumber(n, base, 0); }
	// Print a number in any number base (eg, BIN, HEX, OCT)
	size_t print(int n, int base)			{ return (base == 10) ? print(n) : printNumber(n, base, 0); }
	// Print a number in any number base (eg, BIN, HEX, OCT)
	size_t print(unsigned int n, int base)		{ return printNumber(n, base, 0); }
	// Print a number in any number base (eg, BIN, HEX, OCT)
	size_t print(long n, int base)			{ return (base == 10) ? print(n) : printNumber(n, base, 0); }
	// Print a number in any number base (eg, BIN, HEX, OCT)
	size_t print(unsigned long n, int base)		{ return printNumber(n, base, 0); }
	// Print a number in any number base (eg, BIN, HEX, OCT)
	size_t print(int64_t n, int base)		{ return (base == 10) ? print(n) : printNumber64(n, base, 0); }
	// Print a number in any number base (eg, BIN, HEX, OCT)
	size_t print(uint64_t n, int base)		{ return printNumber64(n, base, 0); }

	// Print a floating point (decimal) number
	size_t print(double n, int digits = 2)		{ return printFloat(n, digits); }
	// Print an object instance in human readable format
	size_t print(const Printable &obj)		{ return obj.printTo(*this); }
	// Print a newline
	size_t println(void);
	// Print a string and newline
	size_t println(const String &s)			{ return print(s) + println(); }
	// Print a single character and newline
	size_t println(char c)				{ return print(c) + println(); }
	// Print a string and newline
	size_t println(const char s[])			{ return print(s) + println(); }
	// Print a string and newline
	size_t println(const __FlashStringHelper *f)	{ return print(f) + println(); }

	// Print an unsigned number and newline
	size_t println(uint8_t b)			{ return print(b) + println(); }
	// Print a signed number and newline
	size_t println(int n)				{ return print(n) + println(); }
	// Print an unsigned number and newline
	size_t println(unsigned int n)			{ return print(n) + println(); }
	// Print a signed number and newline
	size_t println(long n)				{ return print(n) + println(); }
	// Print an unsigned number and newline
	size_t println(unsigned long n)			{ return print(n) + println(); }
	// Print a signed number and newline
	size_t println(int64_t n)			{ return print(n) + println(); }
	// Print an unsigned number and newline
	size_t println(uint64_t n)			{ return print(n) + println(); }

	// Print a number in any number base (eg, BIN, HEX, OCT) and a newline
	size_t println(unsigned char n, int base)	{ return print(n, base) + println(); }
	// Print a number in any number base (eg, BIN, HEX, OCT) and a newline
	size_t println(int n, int base)			{ return print(n, base) + println(); }
	// Print a number in any number base (eg, BIN, HEX, OCT) and a newline
	size_t println(unsigned int n, int base)	{ return print(n, base) + println(); }
	// Print a number in any number base (eg, BIN, HEX, OCT) and a newline
	size_t println(long n, int base)		{ return print(n, base) + println(); }
	// Print a number in any number base (eg, BIN, HEX, OCT) and a newline
	size_t println(unsigned long n, int base)	{ return print(n, base) + println(); }
	// Print a number in any number base (eg, BIN, HEX, OCT) and a newline
	size_t println(int64_t n, int base)		{ return print(n, base) + println(); }
	// Print a number in any number base (eg, BIN, HEX, OCT) and a newline
	size_t println(uint64_t n, int base)		{ return print(n, base) + println(); }

	// Print a floating point (decimal) number and a newline
	size_t println(double n, int digits = 2)	{ return print(n, digits) + println(); }
	// Print an object instance in human readable format, and a newline
	size_t println(const Printable &obj)		{ return obj.printTo(*this) + println(); }
	int getWriteError() { return write_error; }
	void clearWriteError() { setWriteError(0); }

	// printf is a C standard function which allows you to print any number of variables using a somewhat cryptic format string
	int printf(const char *format, ...);
	// printf is a C standard function which allows you to print any number of variables using a somewhat cryptic format string
	int printf(const __FlashStringHelper *format, ...);
	// vprintf is a C standard function that allows you to print a variable argument list with a format string
	int vprintf(const char *format, va_list ap) { return vdprintf((int)this, format, ap); }

	// format warnings are too pedantic - disable until newer toolchain offers better...
	// https://forum.pjrc.com/threads/62473?p=256873&viewfull=1#post256873
	// int printf(const char *format, ...) __attribute__ ((format (printf, 2, 3)));

  protected:
	void setWriteError(int err = 1) { write_error = err; }
  private:
	int write_error;
	size_t printFloat(double n, uint8_t digits);
	size_t printNumber(unsigned long n, uint8_t base, uint8_t sign);
	size_t printNumber64(uint64_t n, uint8_t base, uint8_t sign);
};


#endif

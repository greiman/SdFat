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
#ifndef FsDateTime_h
#define FsDateTime_h
#include <stdint.h>
#include "SysCall.h"
/** Backward compatible definition. */
#define FAT_DATE(y, m, d) FS_DATE(y, m, d)

/** Backward compatible definition. */
#define FAT_TIME(h, m, s) FS_TIME(h, m, s)
/** Date time callback */
namespace FsDateTime {
  /** Date time callback. */
  extern void (*callback)(uint16_t* date, uint16_t* time, uint8_t* ms10);
  /** Date time callback. */
  extern void (*callback2)(uint16_t* date, uint16_t* time);
  /** Cancel callback */
  void clearCallback();
   /** Set the date/time callback function
   *
   * \param[in] dateTime The user's call back function.  The callback
   * function is of the form:
   *
   * \code
   * void dateTime(uint16_t* date, uint16_t* time) {
   *   uint16_t year;
   *   uint8_t month, day, hour, minute, second;
   *
   *   // User gets date and time from GPS or real-time clock here
   *
   *   // return date using FAT_DATE macro to format fields
   *   *date = FAT_DATE(year, month, day);
   *
   *   // return time using FAT_TIME macro to format fields
   *   *time = FAT_TIME(hour, minute, second);
   * }
   * \endcode
   *
   * Sets the function that is called when a file is created or when
   * a file's directory entry is modified by sync(). All timestamps,
   * access, creation, and modify, are set when a file is created.
   * sync() maintains the last access date and last modify date/time.
   *
   */
  void setCallback(void (*dateTime)(uint16_t* date, uint16_t* time));
   /** Set the date/time callback function
   *
   * \param[in] dateTime The user's call back function.  The callback
   * function is of the form:
   *
   * \code
   * void dateTime(uint16_t* date, uint16_t* time, uint8_t* ms10) {
   *   uint16_t year;
   *   uint8_t month, day, hour, minute, second;
   *
   *   // User gets date and time from GPS or real-time clock here
   *
   *   // return date using FAT_DATE macro to format fields
   *   *date = FAT_DATE(year, month, day);
   *
   *   // return time using FAT_TIME macro to format fields
   *   *time = FAT_TIME(hour, minute, second);
   *
   *   *ms10 = <tens of ms since second/1>
   * }
   * \endcode
   *
   * Sets the function that is called when a file is created or when
   * a file's directory entry is modified by sync(). All timestamps,
   * access, creation, and modify, are set when a file is created.
   * sync() maintains the last access date and last modify date/time.
   *
   */
  void setCallback(
    void (*dateTime)(uint16_t* date, uint16_t* time, uint8_t *ms10));
}  // namespace FsDateTime
/** date field for directory entry
 * \param[in] year [1980,2107]
 * \param[in] month [1,12]
 * \param[in] day [1,31]
 *
 * \return Packed date for directory entry.
 */
static inline uint16_t FS_DATE(uint16_t year, uint8_t month, uint8_t day) {
  year -= 1980;
  return year > 127 || month > 12 || day > 31 ? 0 :
         year << 9 | month << 5 | day;
}
static inline uint16_t FS_YEAR(uint16_t fatDate) {
  return 1980 + (fatDate >> 9);
}
/** month part of FAT directory date field
 * \param[in] fatDate Date in packed dir format.
 *
 * \return Extracted month [1,12]
 */
static inline uint8_t FS_MONTH(uint16_t fatDate) {
  return (fatDate >> 5) & 0XF;
}
/** day part of FAT directory date field
 * \param[in] fatDate Date in packed dir format.
 *
 * \return Extracted day [1,31]
 */
static inline uint8_t FS_DAY(uint16_t fatDate) {
  return fatDate & 0X1F;
}
/** time field for directory entry
 * \param[in] hour [0,23]
 * \param[in] minute [0,59]
 * \param[in] second [0,59]
 *
 * \return Packed time for directory entry.
 */
static inline uint16_t FS_TIME(uint8_t hour, uint8_t minute, uint8_t second) {
  return hour > 23 || minute > 59 || second > 59 ? 0 :
         hour << 11 | minute << 5 | second >> 1;
}
/** hour part of FAT directory time field
 * \param[in] fatTime Time in packed dir format.
 *
 * \return Extracted hour [0,23]
 */
static inline uint8_t FS_HOUR(uint16_t fatTime) {
  return fatTime >> 11;
}
/** minute part of FAT directory time field
 * \param[in] fatTime Time in packed dir format.
 *
 * \return Extracted minute [0,59]
 */
static inline uint8_t FS_MINUTE(uint16_t fatTime) {
  return (fatTime >> 5) & 0X3F;
}
/** second part of FAT directory time field
 * N\note second/2 is stored in packed time.
 *
 * \param[in] fatTime Time in packed dir format.
 *
 * \return Extracted second [0,58]
 */
static inline uint8_t FS_SECOND(uint16_t fatTime) {
  return 2*(fatTime & 0X1F);
}
char* fsFmtDate(char* str, uint16_t date);
char* fsFmtTime(char* str, uint16_t time);
char* fsFmtTime(char* str, uint16_t time, uint8_t sec100);
char* fsFmtTimeZone(char* str, int8_t tz);
size_t fsPrintDate(print_t* pr, uint16_t date);
size_t fsPrintDateTime(print_t* pr, uint16_t date, uint16_t time);
size_t fsPrintDateTime(print_t* pr, uint32_t dateTime);
size_t fsPrintDateTime(print_t* pr, uint32_t dateTime, uint8_t s100, int8_t tz);
size_t fsPrintTime(print_t* pr, uint16_t time);
size_t fsPrintTime(print_t* pr, uint16_t time, uint8_t sec100);
size_t fsPrintTimeZone(print_t* pr, int8_t tz);
#endif  // FsDateTime_h

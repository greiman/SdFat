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
#ifndef SdBaseFile_h
#define SdBaseFile_h
/**
 * \file
 * \brief SdBaseFile class
 */
#ifdef __AVR__
#include <avr/pgmspace.h>
#else  // __AVR__
#ifndef PGM_P
/** pointer to flash for ARM */
#define PGM_P const char*
#endif  // PGM_P
#ifndef PSTR
/** store literal string in flash for ARM */
#define PSTR(x) (x)
#endif  // PSTR
#ifndef pgm_read_byte
/** read 8-bits from flash for ARM */
#define pgm_read_byte(addr) (*(const unsigned char*)(addr))
#endif  // pgm_read_byte
#ifndef pgm_read_word
/** read 16-bits from flash for ARM */
#define pgm_read_word(addr) (*(const uint16_t*)(addr))
#endif  // pgm_read_word
#ifndef PROGMEM
/** store in flash for ARM */
#define PROGMEM const
#endif  // PROGMEM
#endif  // __AVR__
#include <Arduino.h>
#include <SdFatConfig.h>
#include <SdVolume.h>
#include <utility/FatApiConstants.h>
//------------------------------------------------------------------------------
/**
 * \struct FatPos_t
 * \brief internal type for istream
 * do not use in user apps
 */
struct FatPos_t {
  /** stream position */
  uint32_t position;
  /** cluster for position */
  uint32_t cluster;
  FatPos_t() : position(0), cluster(0) {}
};

// values for m_type
/** This file has not been opened. */
uint8_t const FAT_FILE_TYPE_CLOSED = 0;
/** A normal file */
uint8_t const FAT_FILE_TYPE_NORMAL = 1;
/** A FAT12 or FAT16 root directory */
uint8_t const FAT_FILE_TYPE_ROOT_FIXED = 2;
/** A FAT32 root directory */
uint8_t const FAT_FILE_TYPE_ROOT32 = 3;
/** A subdirectory file*/
uint8_t const FAT_FILE_TYPE_SUBDIR = 4;
/** Test value for directory type */
uint8_t const FAT_FILE_TYPE_MIN_DIR = FAT_FILE_TYPE_ROOT_FIXED;

//------------------------------------------------------------------------------
/**
 * \class SdBaseFile
 * \brief Base class for SdFile with Print and C++ streams.
 */
class SdBaseFile {
 public:
  /** Create an instance. */
  SdBaseFile() : writeError(false), m_type(FAT_FILE_TYPE_CLOSED) {}
  SdBaseFile(const char* path, uint8_t oflag);
#if DESTRUCTOR_CLOSES_FILE
  ~SdBaseFile() {if(isOpen()) close();}
#endif  // DESTRUCTOR_CLOSES_FILE
  /**
   * writeError is set to true if an error occurs during a write().
   * Set writeError to false before calling print() and/or write() and check
   * for true after calls to print() and/or write().
   */
  bool writeError;
  /** \return value of writeError */
  bool getWriteError() {return writeError;}
  /** Set writeError to zero */
  void clearWriteError() {writeError = 0;}
  //----------------------------------------------------------------------------
  // helpers for stream classes
  /** get position for streams
   * \param[out] pos struct to receive position
   */
  void getpos(FatPos_t* pos);
  /** set position for streams
   * \param[out] pos struct with value for new position
   */
  void setpos(FatPos_t* pos);
  //----------------------------------------------------------------------------
  /** \return number of bytes available from yhe current position to EOF */
  uint32_t available() {return fileSize() - curPosition();}
  bool close();
  bool contiguousRange(uint32_t* bgnBlock, uint32_t* endBlock);
  bool createContiguous(SdBaseFile* dirFile,
          const char* path, uint32_t size);
  /** \return The current cluster number for a file or directory. */
  uint32_t curCluster() const {return m_curCluster;}
  /** \return The current position for a file or directory. */
  uint32_t curPosition() const {return m_curPosition;}
  /** \return Current working directory */
  static SdBaseFile* cwd() {return m_cwd;}
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
   * See the timestamp() function.
   */
  static void dateTimeCallback(
    void (*dateTime)(uint16_t* date, uint16_t* time)) {
    m_dateTime = dateTime;
  }
  /**  Cancel the date/time callback function. */
  static void dateTimeCallbackCancel() {m_dateTime = 0;}
  bool dirEntry(dir_t* dir);
  static void dirName(const dir_t& dir, char* name);
  bool exists(const char* name);
  int16_t fgets(char* str, int16_t num, char* delim = 0);
  /** \return The total number of bytes in a file or directory. */
  uint32_t fileSize() const {return m_fileSize;}
  /** \return The first cluster number for a file or directory. */
  uint32_t firstCluster() const {return m_firstCluster;}
  bool getFilename(char* name);
  /** \return True if this is a directory else false. */
  bool isDir() const {return m_type >= FAT_FILE_TYPE_MIN_DIR;}
  /** \return True if this is a normal file else false. */
  bool isFile() const {return m_type == FAT_FILE_TYPE_NORMAL;}
  /** \return True if this is an open file/directory else false. */
  bool isOpen() const {return m_type != FAT_FILE_TYPE_CLOSED;}
  /** \return True if this is a subdirectory else false. */
  bool isSubDir() const {return m_type == FAT_FILE_TYPE_SUBDIR;}
  /** \return True if this is the root directory. */
  bool isRoot() const {
    return m_type == FAT_FILE_TYPE_ROOT_FIXED || m_type == FAT_FILE_TYPE_ROOT32;
  }
  void ls(Print* pr, uint8_t flags = 0, uint8_t indent = 0);
  void ls(uint8_t flags = 0);
  bool mkdir(SdBaseFile* dir, const char* path, bool pFlag = true);
  // alias for backward compactability
  bool makeDir(SdBaseFile* dir, const char* path) {
    return mkdir(dir, path, false);
  }
  bool open(SdBaseFile* dirFile, uint16_t index, uint8_t oflag);
  bool open(SdBaseFile* dirFile, const char* path, uint8_t oflag);
  bool open(const char* path, uint8_t oflag = O_READ);
  bool openNext(SdBaseFile* dirFile, uint8_t oflag);
  bool openRoot(SdVolume* vol);
  int peek();
  bool printCreateDateTime(Print* pr);
  static void printFatDate(uint16_t fatDate);
  static void printFatDate(Print* pr, uint16_t fatDate);
  static void printFatTime(uint16_t fatTime);
  static void printFatTime(Print* pr, uint16_t fatTime);
  int printField(float value, char term, uint8_t prec = 2);
  int printField(int16_t value, char term);
  int printField(uint16_t value, char term);
  int printField(int32_t value, char term);
  int printField(uint32_t value, char term);
  bool printModifyDateTime(Print* pr);
  size_t printName();
  size_t printName(Print* pr);
  size_t printFileSize(Print* pr);
  int16_t read();
  int read(void* buf, size_t nbyte);
  int8_t readDir(dir_t* dir);
  static bool remove(SdBaseFile* dirFile, const char* path);
  bool remove();
  /** Set the file's current position to zero. */
  void rewind() {seekSet(0);}
  bool rename(SdBaseFile* dirFile, const char* newPath);
  bool rmdir();
  // for backward compatibility
  bool rmDir() {return rmdir();}
  bool rmRfStar();
  /** Set the files position to current position + \a pos. See seekSet().
   * \param[in] offset The new position in bytes from the current position.
   * \return true for success or false for failure.
   */
  bool seekCur(int32_t offset) {
    return seekSet(m_curPosition + offset);
  }
  /** Set the files position to end-of-file + \a offset. See seekSet().
   * \param[in] offset The new position in bytes from end-of-file.
   * \return true for success or false for failure.
   */
  bool seekEnd(int32_t offset = 0) {return seekSet(m_fileSize + offset);}
  bool seekSet(uint32_t pos);
  bool sync();
  bool timestamp(SdBaseFile* file);
  bool timestamp(uint8_t flag, uint16_t year, uint8_t month, uint8_t day,
          uint8_t hour, uint8_t minute, uint8_t second);
  /** Type of file.  You should use isFile() or isDir() instead of type()
   * if possible.
   *
   * \return The file or directory type.
   */
  uint8_t type() const {return m_type;}
  bool truncate(uint32_t size);
  /** \return SdVolume that contains this file. */
  SdVolume* volume() const {return m_vol;}
  int write(const void* buf, size_t nbyte);
//------------------------------------------------------------------------------
 private:
  // allow SdFat to set m_cwd
  friend class SdFat;
  /** experimental don't use */
  bool openParent(SdBaseFile* dir);

  // private functions
  bool addCluster();
  cache_t* addDirCluster();
  dir_t* cacheDirEntry(uint8_t action);
  int8_t lsPrintNext(Print *pr, uint8_t flags, uint8_t indent);
  static bool make83Name(const char* str, uint8_t* name, const char** ptr);
  bool mkdir(SdBaseFile* parent, const uint8_t dname[11]);
  bool open(SdBaseFile* dirFile, const uint8_t dname[11], uint8_t oflag);
  bool openCachedEntry(uint8_t cacheIndex, uint8_t oflags);
  dir_t* readDirCache();
  static void setCwd(SdBaseFile* cwd) {m_cwd = cwd;}
  bool setDirSize();

  // bits defined in m_flags
  // should be 0X0F
  static uint8_t const F_OFLAG = (O_ACCMODE | O_APPEND | O_SYNC);
  // sync of directory entry required
  static uint8_t const F_FILE_DIR_DIRTY = 0X80;

  // global pointer to cwd dir
  static SdBaseFile* m_cwd;
  // data time callback function
  static void (*m_dateTime)(uint16_t* date, uint16_t* time);
  // private data
  uint8_t   m_flags;         // See above for definition of m_flags bits
  uint8_t   m_type;          // type of file see above for values
  uint8_t   m_dirIndex;      // index of directory entry in dirBlock
  SdVolume* m_vol;           // volume where file is located
  uint32_t  m_curCluster;    // cluster for current file position
  uint32_t  m_curPosition;   // current file position in bytes from beginning
  uint32_t  m_dirBlock;      // block for this files directory entry
  uint32_t  m_fileSize;      // file size in bytes
  uint32_t  m_firstCluster;  // first cluster of file
};
#endif  // SdBaseFile_h

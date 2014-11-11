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
#ifndef SdFile_h
#define SdFile_h
#include <limits.h>
#include <SdBaseFile.h>
//------------------------------------------------------------------------------
/**
 * \class SdFile
 * \brief SdBaseFile with Arduino Stream.
 */
#if SD_FILE_USES_STREAM
class SdFile : public SdBaseFile, public Stream {
#else  // SD_FILE_USES_STREAM
class SdFile : public SdBaseFile, public Print {
#endif  // SD_FILE_USES_STREAM
 public:
  SdFile() {}
  SdFile(const char* name, uint8_t oflag);
#if DESTRUCTOR_CLOSES_FILE
  ~SdFile() {}
#endif  // DESTRUCTOR_CLOSES_FILE
  /** \return number of bytes available from the current position to EOF
   *   or INT_MAX if more than INT_MAX bytes are available.
   */
  int available() {
    uint32_t n = SdBaseFile::available();
    return n > INT_MAX ? INT_MAX : n;
  }
  /** Ensure that any bytes written to the file are saved to the SD card. */
  void flush() {SdBaseFile::sync();}
  /** Return the next available byte without consuming it.
   *
   * \return The byte if no error and not at eof else -1;
   */  
  int peek() {return SdBaseFile::peek();}
  /** Read the next byte from a file.
   *
   * \return For success return the next byte in the file as an int.
   * If an error occurs or end of file is reached return -1.
   */  
  int read() {return SdBaseFile::read();}
  /** Read data from a file starting at the current position.
   *
   * \param[out] buf Pointer to the location that will receive the data.
   *
   * \param[in] nbyte Maximum number of bytes to read.
   *
   * \return For success read() returns the number of bytes read.
   * A value less than \a nbyte, including zero, will be returned
   * if end of file is reached.
   * If an error occurs, read() returns -1.  Possible errors include
   * read() called before a file has been opened, corrupt file system
   * or an I/O error occurred.
   */  
  int read(void* buf, size_t nbyte) {return SdBaseFile::read(buf, nbyte);}
  /** \return value of writeError */
  bool getWriteError() {return SdBaseFile::getWriteError();}
  /** Set writeError to zero */
  void clearWriteError() {SdBaseFile::clearWriteError();}
  size_t write(uint8_t b);

  int write(const char* str);
  /** Write data to an open file.
   *
   * \note Data is moved to the cache but may not be written to the
   * storage device until sync() is called.
   *
   * \param[in] buf Pointer to the location of the data to be written.
   *
   * \param[in] nbyte Number of bytes to write.
   *
   * \return For success write() returns the number of bytes written, always
   * \a nbyte.  If an error occurs, write() returns -1.  Possible errors
   * include write() is called before a file has been opened, write is called
   * for a read-only file, device is full, a corrupt file system or an 
   * I/O error.
   */  
  int write(const void* buf, size_t nbyte);
  /** Write data to an open file.  Form required by Print.
   *
   * \note Data is moved to the cache but may not be written to the
   * storage device until sync() is called.
   *
   * \param[in] buf Pointer to the location of the data to be written.
   *
   * \param[in] size Number of bytes to write.
   *
   * \return For success write() returns the number of bytes written, always
   * \a nbyte.  If an error occurs, write() returns -1.  Possible errors
   * include write() is called before a file has been opened, write is called
   * for a read-only file, device is full, a corrupt file system or an 
   * I/O error.
   */  
  size_t write(const uint8_t *buf, size_t size) {
    return SdBaseFile::write(buf, size);}
  void write_P(PGM_P str);
  void writeln_P(PGM_P str);
};
//------------------------------------------------------------------------------
/** Arduino SD.h style flag for open for read. */
#define FILE_READ O_READ
/** Arduino SD.h style flag for open at EOF for read/write with create. */
#define FILE_WRITE (O_RDWR | O_CREAT | O_AT_END)
/**
 * \class File
 * \brief Arduino SD.h style File API
 */
class File : public SdBaseFile, public Stream {
 public:
 /** The parenthesis operator.
   *
   * \return true if a file is open.
   */
  operator bool() {return isOpen();}
  /** \return number of bytes available from the current position to EOF
   *   or INT_MAX if more than INT_MAX bytes are available.
   */
  int available() {
    uint32_t n = SdBaseFile::available();
    return n > INT_MAX ? INT_MAX : n;
  }
  /** Set writeError to zero */
  void clearWriteError() {SdBaseFile::clearWriteError();}
  /** Ensure that any bytes written to the file are saved to the SD card. */
  void flush() {sync();}
  /** \return value of writeError */
  bool getWriteError() {return SdBaseFile::getWriteError();}
   /** This function reports if the current file is a directory or not.
   * \return true if the file is a directory.
   */  
  bool isDirectory() {return isDir();}
  /** \return a pointer to the file's name. */
  char* name() {
    m_name[0] = 0;
    getFilename(m_name);
    return m_name;
  }
  /** Return the next available byte without consuming it.
   *
   * \return The byte if no error and not at eof else -1;
   */  
  int peek() {return SdBaseFile::peek();}
  /** \return the current file position. */
  uint32_t position() {return curPosition();}
  /** Opens the next file or folder in a directory.
   *
   * \param[in] mode open mode flags.
   * \return a File object.
   */
  File openNextFile(uint8_t mode = O_READ) {
    File tmpFile;
    tmpFile.openNext(this, mode);
    return tmpFile;
  }
  /** Read the next byte from a file.
   *
   * \return For success return the next byte in the file as an int.
   * If an error occurs or end of file is reached return -1.
   */  
  int read() {return SdBaseFile::read();}
  /** Read data from a file starting at the current position.
   *
   * \param[out] buf Pointer to the location that will receive the data.
   *
   * \param[in] nbyte Maximum number of bytes to read.
   *
   * \return For success read() returns the number of bytes read.
   * A value less than \a nbyte, including zero, will be returned
   * if end of file is reached.
   * If an error occurs, read() returns -1.  Possible errors include
   * read() called before a file has been opened, corrupt file system
   * or an I/O error occurred.
   */  
  int read(void* buf, size_t nbyte) {return SdBaseFile::read(buf, nbyte);}
  /** Rewind a file if it is a directory */
  void rewindDirectory() {
    if (isDir()) rewind();
  }
  /** 
   * Seek to a new position in the file, which must be between
   * 0 and the size of the file (inclusive).
   *
   * \param[in] pos the new file position.
   * \return true for success else false.
   */
  bool seek(uint32_t pos) {return seekSet(pos);}
  /** \return the file's size. */
  uint32_t size() {return fileSize();}
  /** Write a byte to a file. Required by the Arduino Print class.
   * \param[in] b the byte to be written.
   * Use getWriteError to check for errors.
   * \return 1 for success and 0 for failure.
   */
  size_t write(uint8_t b) {
    return SdBaseFile::write(&b, 1) == 1 ? 1 : 0;
  }
  /** Write data to an open file.
   *
   * \note Data is moved to the cache but may not be written to the
   * storage device until sync() is called.
   *
   * \param[in] buf Pointer to the location of the data to be written.
   *
   * \param[in] nbyte Number of bytes to write.
   *
   * \return For success write() returns the number of bytes written, always
   * \a nbyte.  If an error occurs, write() returns -1.  Possible errors
   * include write() is called before a file has been opened, write is called
   * for a read-only file, device is full, a corrupt file system or an 
   * I/O error.
   */  
  int write(const void* buf, size_t nbyte);
  /** Write data to an open file.  Form required by Print.
   *
   * \note Data is moved to the cache but may not be written to the
   * storage device until sync() is called.
   *
   * \param[in] buf Pointer to the location of the data to be written.
   *
   * \param[in] size Number of bytes to write.
   *
   * \return For success write() returns the number of bytes written, always
   * \a nbyte.  If an error occurs, write() returns -1.  Possible errors
   * include write() is called before a file has been opened, write is called
   * for a read-only file, device is full, a corrupt file system or an 
   * I/O error.
   */  
  size_t write(const uint8_t *buf, size_t size) {
    return SdBaseFile::write(buf, size);
  }

 private:
  char m_name[13];
};
#endif  // SdFile_h

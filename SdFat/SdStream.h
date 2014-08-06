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
#ifndef SdStream_h
#define SdStream_h
/**
 * \file
 * \brief \ref fstream, \ref ifstream, and \ref ofstream classes
 */
#include <SdBaseFile.h>
#include <iostream.h>
//==============================================================================
/**
 * \class SdStreamBase
 * \brief Base class for SD streams
 */
class SdStreamBase : protected SdBaseFile, virtual public ios {
 protected:
  /// @cond SHOW_PROTECTED
  int16_t getch();
  void putch(char c);
  void putstr(const char *str);
  void open(const char* path, ios::openmode mode);
  /** Internal do not use
   * \return mode
   */
  ios::openmode getmode() {return m_mode;}
  /** Internal do not use
   * \param[in] mode
   */
  void setmode(ios::openmode mode) {m_mode = mode;}
  bool seekoff(off_type off, seekdir way);
  bool seekpos(pos_type pos);
  int write(const void* buf, size_t n);
  void write(char c);
  /// @endcond
 private:
  ios::openmode m_mode;
};
//==============================================================================
/**
 * \class fstream
 * \brief SD file input/output stream.
 */
class fstream : public iostream, SdStreamBase {
 public:
  using iostream::peek;
  fstream() {}
  /** Constructor with open
   *
   * \param[in] path path to open
   * \param[in] mode open mode
   */
  explicit fstream(const char* path, openmode mode = in | out) {
    open(path, mode);
  }
#if DESTRUCTOR_CLOSES_FILE
  ~fstream() {}
#endif  // DESTRUCTOR_CLOSES_FILE
  /** Clear state and writeError
   * \param[in] state new state for stream
   */
  void clear(iostate state = goodbit) {
    ios::clear(state);
    SdBaseFile::writeError = false;
  }
  /**  Close a file and force cached data and directory information
   *  to be written to the storage device.
   */
  void close() {SdBaseFile::close();}
  /** Open a fstream
   * \param[in] path file to open
   * \param[in] mode open mode
   *
   * Valid open modes are (at end, ios::ate, and/or ios::binary may be added):
   *
   * ios::in - Open file for reading.
   *
   * ios::out or ios::out | ios::trunc - Truncate to 0 length, if existent,
   * or create a file for writing only.
   *
   * ios::app or ios::out | ios::app - Append; open or create file for
   * writing at end-of-file.
   *
   * ios::in | ios::out - Open file for update (reading and writing).
   *
   * ios::in | ios::out | ios::trunc - Truncate to zero length, if existent,
   * or create file for update.
   *
   * ios::in | ios::app or ios::in | ios::out | ios::app - Append; open or
   * create text file for update, writing at end of file.
   */
  void open(const char* path, openmode mode = in | out) {
    SdStreamBase::open(path, mode);
  }
  /** \return True if stream is open else false. */
  bool is_open () {return SdBaseFile::isOpen();}

 protected:
  /// @cond SHOW_PROTECTED
  /** Internal - do not use
   * \return
   */
  int16_t getch() {return SdStreamBase::getch();}
    /** Internal - do not use
   * \param[out] pos
   */
  void getpos(FatPos_t* pos) {SdBaseFile::getpos(pos);}
  /** Internal - do not use
   * \param[in] c
   */
  void putch(char c) {SdStreamBase::putch(c);}
  /** Internal - do not use
   * \param[in] str
   */
  void putstr(const char *str) {SdStreamBase::putstr(str);}
  /** Internal - do not use
   * \param[in] pos
   */
  bool seekoff(off_type off, seekdir way) {
    return SdStreamBase::seekoff(off, way);
  }
  bool seekpos(pos_type pos) {return SdStreamBase::seekpos(pos);}
  void setpos(FatPos_t* pos) {SdBaseFile::setpos(pos);}
  bool sync() {return SdStreamBase::sync();}
  pos_type tellpos() {return SdStreamBase::curPosition();}
  /// @endcond
};
//==============================================================================
/**
 * \class ifstream
 * \brief SD file input stream.
 */
class ifstream : public istream, SdStreamBase {
 public:
  using istream::peek;
  ifstream() {}
  /** Constructor with open
   * \param[in] path file to open
   * \param[in] mode open mode
   */
  explicit ifstream(const char* path, openmode mode = in) {
    open(path, mode);
  }
#if DESTRUCTOR_CLOSES_FILE
  ~ifstream() {}
#endif  // DESTRUCTOR_CLOSES_FILE
  /**  Close a file and force cached data and directory information
   *  to be written to the storage device.
   */
  void close() {SdBaseFile::close();}
  /** \return True if stream is open else false. */
  bool is_open() {return SdBaseFile::isOpen();}
  /** Open an ifstream
   * \param[in] path file to open
   * \param[in] mode open mode
   *
   * \a mode See fstream::open() for valid modes.
   */
  void open(const char* path, openmode mode = in) {
    SdStreamBase::open(path, mode | in);
  }

 protected:
  /// @cond SHOW_PROTECTED
  /** Internal - do not use
   * \return
   */
  int16_t getch() {return SdStreamBase::getch();}
  /** Internal - do not use
   * \param[out] pos
   */
  void getpos(FatPos_t* pos) {SdBaseFile::getpos(pos);}
  /** Internal - do not use
   * \param[in] pos
   */
  bool seekoff(off_type off, seekdir way) {
    return SdStreamBase::seekoff(off, way);
  }
  bool seekpos(pos_type pos) {return SdStreamBase::seekpos(pos);}
  void setpos(FatPos_t* pos) {SdBaseFile::setpos(pos);}
  pos_type tellpos() {return SdStreamBase::curPosition();}
  /// @endcond
};
//==============================================================================
/**
 * \class ofstream
 * \brief SD card output stream.
 */
class ofstream : public ostream, SdStreamBase {
 public:
  ofstream() {}
  /** Constructor with open
   * \param[in] path file to open
   * \param[in] mode open mode
   */
  explicit ofstream(const char* path, ios::openmode mode = out) {
    open(path, mode);
  }
#if DESTRUCTOR_CLOSES_FILE
  ~ofstream() {}
#endif  // DESTRUCTOR_CLOSES_FILE
  /** Clear state and writeError
   * \param[in] state new state for stream
   */
  void clear(iostate state = goodbit) {
    ios::clear(state);
    SdBaseFile::writeError = false;
  }
  /**  Close a file and force cached data and directory information
   *  to be written to the storage device.
   */
  void close() {SdBaseFile::close();}
  /** Open an ofstream
   * \param[in] path file to open
   * \param[in] mode open mode
   *
   * \a mode See fstream::open() for valid modes.
   */
  void open(const char* path, openmode mode = out) {
    SdStreamBase::open(path, mode | out);
  }
  /** \return True if stream is open else false. */
  bool is_open() {return SdBaseFile::isOpen();}

 protected:
  /// @cond SHOW_PROTECTED
  /**
   * Internal do not use
   * \param[in] c
   */
  void putch(char c) {SdStreamBase::putch(c);}
  void putstr(const char* str) {SdStreamBase::putstr(str);}
  bool seekoff(off_type off, seekdir way) {
    return SdStreamBase::seekoff(off, way);
  }
  bool seekpos(pos_type pos) {return SdStreamBase::seekpos(pos);}
  /**
   * Internal do not use
   * \param[in] b
   */
  bool sync() {return SdStreamBase::sync();}
  pos_type tellpos() {return SdStreamBase::curPosition();}
  /// @endcond
};
//------------------------------------------------------------------------------
#endif  // SdStream_h

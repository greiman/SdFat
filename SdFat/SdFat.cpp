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
//------------------------------------------------------------------------------
#if USE_SERIAL_FOR_STD_OUT || !defined(UDR0)
Print* SdFat::m_stdOut = &Serial;
#else  // USE_SERIAL_FOR_STD_OUT
#include <MinimumSerial.h>
Print* SdFat::m_stdOut = &MiniSerial;
#endif  // USE_SERIAL_FOR_STD_OUT
//------------------------------------------------------------------------------
/**
 * Initialize an SdFat object.
 *
 * Initializes the SD card, SD volume, and root directory.
 *
 * \param[in] chipSelectPin SD chip select pin. See Sd2Card::init().
 * \param[in] sckDivisor value for SPI SCK divisor. See Sd2Card::init().
 *
 * \return The value one, true, is returned for success and
 * the value zero, false, is returned for failure.
 */
bool SdFat::begin(uint8_t chipSelectPin, uint8_t sckDivisor) {
  return m_card.begin(chipSelectPin, sckDivisor)
         && m_vol.init(&m_card) && chdir(1);
}
//------------------------------------------------------------------------------
/** Change a volume's working directory to root
 *
 * Changes the volume's working directory to the SD's root directory.
 * Optionally set the current working directory to the volume's
 * working directory.
 *
 * \param[in] set_cwd Set the current working directory to this volume's
 *  working directory if true.
 *
 * \return The value one, true, is returned for success and
 * the value zero, false, is returned for failure.
 */
bool SdFat::chdir(bool set_cwd) {
  if (set_cwd) SdBaseFile::setCwd(&m_vwd);
  if (m_vwd.isOpen()) m_vwd.close();
  return m_vwd.openRoot(&m_vol);
}
//------------------------------------------------------------------------------
/** Change a volume's working directory
 *
 * Changes the volume working directory to the \a path subdirectory.
 * Optionally set the current working directory to the volume's
 * working directory.
 *
 * Example: If the volume's working directory is "/DIR", chdir("SUB")
 * will change the volume's working directory from "/DIR" to "/DIR/SUB".
 *
 * If path is "/", the volume's working directory will be changed to the
 * root directory
 *
 * \param[in] path The name of the subdirectory.
 *
 * \param[in] set_cwd Set the current working directory to this volume's
 *  working directory if true.
 *
 * \return The value one, true, is returned for success and
 * the value zero, false, is returned for failure.
 */
bool SdFat::chdir(const char *path, bool set_cwd) {
  SdBaseFile dir;
  dir.open(&m_vwd, path, O_READ);
  // Check for correctly open directory.
  if (!dir.isDir()) goto fail;
  m_vwd = dir;
  if (set_cwd) SdBaseFile::setCwd(&m_vwd);
  return true;

 fail:
  return false;
}
//------------------------------------------------------------------------------
/** Set the current working directory to a volume's working directory.
 *
 * This is useful with multiple SD cards.
 *
 * The current working directory is changed to this volume's working directory.
 *
 * This is like the Windows/DOS \<drive letter>: command.
 */
void SdFat::chvol() {
  SdBaseFile::setCwd(&m_vwd);
}
//------------------------------------------------------------------------------
/**
 * Test for the existence of a file.
 *
 * \param[in] name Name of the file to be tested for.
 *
 * \return true if the file exists else false.
 */
bool SdFat::exists(const char* name) {
  return m_vwd.exists(name);
}
//------------------------------------------------------------------------------
/** List the directory contents of the volume working directory to stdOut.
 *
 * \param[in] flags The inclusive OR of
 *
 * LS_DATE - %Print file modification date
 *
 * LS_SIZE - %Print file size.
 *
 * LS_R - Recursive list of subdirectories.
 */
void SdFat::ls(uint8_t flags) {
  m_vwd.ls(m_stdOut, flags);
}
//------------------------------------------------------------------------------
/** List the directory contents of the volume working directory to stdOut.
 *
 * \param[in] path directory to list.
 *
 * \param[in] flags The inclusive OR of
 *
 * LS_DATE - %Print file modification date
 *
 * LS_SIZE - %Print file size.
 *
 * LS_R - Recursive list of subdirectories.
 */
void SdFat::ls(const char* path, uint8_t flags) {
  ls(m_stdOut, path, flags);
}
//------------------------------------------------------------------------------
/** List the directory contents of the volume working directory.
 *
 * \param[in] pr Print stream for the list.
 *
 * \param[in] flags The inclusive OR of
 *
 * LS_DATE - %Print file modification date
 *
 * LS_SIZE - %Print file size.
 *
 * LS_R - Recursive list of subdirectories.
 */
void SdFat::ls(Print* pr, uint8_t flags) {
  m_vwd.ls(pr, flags);
}
//------------------------------------------------------------------------------
/** List the directory contents of the volume working directory to stdOut.
 *
 * \param[in] pr Print stream for the list.
 *
 * \param[in] path directory to list.
 *
 * \param[in] flags The inclusive OR of
 *
 * LS_DATE - %Print file modification date
 *
 * LS_SIZE - %Print file size.
 *
 * LS_R - Recursive list of subdirectories.
 */
void SdFat::ls(Print* pr, const char* path, uint8_t flags) {
  SdBaseFile dir(path, O_READ);
  dir.ls(pr, flags);
}
//------------------------------------------------------------------------------
/** Make a subdirectory in the volume working directory.
 *
 * \param[in] path A path with a valid 8.3 DOS name for the subdirectory.
 *
 * \param[in] pFlag Create missing parent directories if true.
 *
 * \return The value one, true, is returned for success and
 * the value zero, false, is returned for failure.
 */
bool SdFat::mkdir(const char* path, bool pFlag) {
  SdBaseFile sub;
  return sub.mkdir(&m_vwd, path, pFlag);
}
//------------------------------------------------------------------------------
/** Remove a file from the volume working directory.
*
* \param[in] path A path with a valid 8.3 DOS name for the file.
*
* \return The value one, true, is returned for success and
* the value zero, false, is returned for failure.
*/
bool SdFat::remove(const char* path) {
  return SdBaseFile::remove(&m_vwd, path);
}
//------------------------------------------------------------------------------
/** Rename a file or subdirectory.
 *
 * \param[in] oldPath Path name to the file or subdirectory to be renamed.
 *
 * \param[in] newPath New path name of the file or subdirectory.
 *
 * The \a newPath object must not exist before the rename call.
 *
 * The file to be renamed must not be open.  The directory entry may be
 * moved and file system corruption could occur if the file is accessed by
 * a file object that was opened before the rename() call.
 *
 * \return The value one, true, is returned for success and
 * the value zero, false, is returned for failure.
 */
bool SdFat::rename(const char *oldPath, const char *newPath) {
  SdBaseFile file;
  if (!file.open(oldPath, O_READ)) return false;
  return file.rename(&m_vwd, newPath);
}
//------------------------------------------------------------------------------
/** Remove a subdirectory from the volume's working directory.
 *
 * \param[in] path A path with a valid 8.3 DOS name for the subdirectory.
 *
 * The subdirectory file will be removed only if it is empty.
 *
 * \return The value one, true, is returned for success and
 * the value zero, false, is returned for failure.
 */
bool SdFat::rmdir(const char* path) {
  SdBaseFile sub;
  if (!sub.open(path, O_READ)) return false;
  return sub.rmdir();
}
//------------------------------------------------------------------------------
/** Truncate a file to a specified length.  The current file position
 * will be maintained if it is less than or equal to \a length otherwise
 * it will be set to end of file.
 *
 * \param[in] path A path with a valid 8.3 DOS name for the file.
 * \param[in] length The desired length for the file.
 *
 * \return The value one, true, is returned for success and
 * the value zero, false, is returned for failure.
 * Reasons for failure include file is read only, file is a directory,
 * \a length is greater than the current file size or an I/O error occurs.
 */
bool SdFat::truncate(const char* path, uint32_t length) {
  SdBaseFile file;
  if (!file.open(path, O_WRITE)) return false;
  return file.truncate(length);
}

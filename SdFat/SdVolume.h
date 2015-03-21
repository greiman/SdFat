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
#ifndef SdVolume_h
#include "SdSpiCard.h"
#include "utility/FatLib.h"
#define SdVolume_h
#ifndef USE_SD_VOLUME
#error SdVolume is deperacated.  Remove this line to continue using this class.
#endif   // USE_SD_VOLUME
//==============================================================================
/**
 * \class SdVolume
 * \brief SdVolume Soon to be removed.
 */
class SdVolume : public FatVolume {
 public:
  /** Initialize a FAT volume.  Try partition one first then try super
  * floppy format.
  *
  * \param[in] dev The Sd2Card where the volume is located.
  *
  * \return true for success else false.
  */
  bool init(Sd2Card* dev) {
    return init(dev, 1) ? true : init(dev, 0);
  }
  /** Initialize a FAT volume.
  *
  * \param[in] dev The Sd2Card where the volume is located.
  * \param[in] part the partition to use. Zero for super floppy or 1-4.
  * \return true for success else false.
  */
  bool init(Sd2Card* dev, uint8_t part) {
    m_sdCard = dev;
    return FatVolume::init(part);
  }

 private:
//  friend class FatFile;
  bool readBlock(uint32_t block, uint8_t* dst) {
    return m_sdCard->readBlock(block, dst);
  }
  bool writeBlock(uint32_t block, const uint8_t* src) {
    return m_sdCard->writeBlock(block, src);
  }
  bool readBlocks(uint32_t block, uint8_t* dst, size_t n) {
    return m_sdCard->readBlocks(block, dst, n);
  }
  bool writeBlocks(uint32_t block, const uint8_t* src, size_t n) {
    return m_sdCard->writeBlocks(block, src, n);
  }
  Sd2Card* m_sdCard;             // Sd2Card object for cache
};
#endif  // SdVolume_h

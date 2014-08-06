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
#define SdVolume_h
/**
 * \file
 * \brief SdVolume class
 */
#include <SdFatConfig.h>
#include <Sd2Card.h>
#include <utility/FatStructs.h>
//==============================================================================
// SdVolume class
/**
 * \brief Cache for an SD data block
 */
union cache_t {
           /** Used to access cached file data blocks. */
  uint8_t  data[512];
           /** Used to access cached FAT16 entries. */
  uint16_t fat16[256];
           /** Used to access cached FAT32 entries. */
  uint32_t fat32[128];
           /** Used to access cached directory entries. */
  dir_t    dir[16];
           /** Used to access a cached Master Boot Record. */
  mbr_t    mbr;
           /** Used to access to a cached FAT boot sector. */
  fat_boot_t fbs;
           /** Used to access to a cached FAT32 boot sector. */
  fat32_boot_t fbs32;
           /** Used to access to a cached FAT32 FSINFO sector. */
  fat32_fsinfo_t fsinfo;
};
//------------------------------------------------------------------------------
/**
 * \class SdVolume
 * \brief Access FAT16 and FAT32 volumes on SD and SDHC cards.
 */
class SdVolume {
 public:
  /** Create an instance of SdVolume */
  SdVolume() : m_fatType(0) {}
  /** Clear the cache and returns a pointer to the cache.  Used by the WaveRP
   * recorder to do raw write to the SD card.  Not for normal apps.
   * \return A pointer to the cache buffer or zero if an error occurs.
   */
  cache_t* cacheClear() {
    if (!cacheSync()) return 0;
    m_cacheBlockNumber = 0XFFFFFFFF;
    return &m_cacheBuffer;
  }
  /** Initialize a FAT volume.  Try partition one first then try super
   * floppy format.
   *
   * \param[in] dev The Sd2Card where the volume is located.
   *
   * \return The value one, true, is returned for success and
   * the value zero, false, is returned for failure.  Reasons for
   * failure include not finding a valid partition, not finding a valid
   * FAT file system or an I/O error.
   */
  bool init(Sd2Card* dev) { return init(dev, 1) ? true : init(dev, 0);}
  bool init(Sd2Card* dev, uint8_t part);

  // inline functions that return volume info
  /** \return The volume's cluster size in blocks. */
  uint8_t blocksPerCluster() const {return m_blocksPerCluster;}
  /** \return The number of blocks in one FAT. */
  uint32_t blocksPerFat()  const {return m_blocksPerFat;}
  /** \return The total number of clusters in the volume. */
  uint32_t clusterCount() const {return m_clusterCount;}
  /** \return The shift count required to multiply by blocksPerCluster. */
  uint8_t clusterSizeShift() const {return m_clusterSizeShift;}
  /** \return The logical block number for the start of file data. */
  uint32_t dataStartBlock() const {return clusterStartBlock(2);}
  /** \return The number of FAT structures on the volume. */
  uint8_t fatCount() const {return m_fatCount;}
  /** \return The logical block number for the start of the first FAT. */
  uint32_t fatStartBlock() const {return m_fatStartBlock;}
  /** \return The FAT type of the volume. Values are 12, 16 or 32. */
  uint8_t fatType() const {return m_fatType;}
  int32_t freeClusterCount();
  /** \return The number of entries in the root directory for FAT16 volumes. */
  uint32_t rootDirEntryCount() const {return m_rootDirEntryCount;}
  /** \return The logical block number for the start of the root directory
       on FAT16 volumes or the first cluster number on FAT32 volumes. */
  uint32_t rootDirStart() const {return m_rootDirStart;}
  /** Sd2Card object for this volume
   * \return pointer to Sd2Card object.
   */
  Sd2Card* sdCard() {return m_sdCard;}

  /** Debug access to FAT table
   *
   * \param[in] n cluster number.
   * \param[out] v value of entry
   * \return true for success or false for failure
   */
  bool dbgFat(uint32_t n, uint32_t* v) {return fatGet(n, v);}
//------------------------------------------------------------------------------
 private:
  // Allow SdBaseFile access to SdVolume private data.
  friend class SdBaseFile;
//------------------------------------------------------------------------------
  uint32_t m_allocSearchStart;   // Start cluster for alloc search.
  uint8_t m_blocksPerCluster;    // Cluster size in blocks.
  uint8_t m_clusterBlockMask;    // Mask to extract block of cluster.
  uint32_t m_clusterCount;       // Clusters in one FAT.
  uint8_t m_clusterSizeShift;    // Cluster count to block count shift.
  uint32_t m_dataStartBlock;     // First data block number.
  uint32_t m_fatStartBlock;      // Start block for first FAT.
  uint8_t m_fatType;             // Volume type (12, 16, OR 32).
  uint16_t m_rootDirEntryCount;  // Number of entries in FAT16 root dir.
  uint32_t m_rootDirStart;       // Start block for FAT16, cluster for FAT32.
//------------------------------------------------------------------------------
// block caches
// use of static functions save a bit of flash - maybe not worth complexity
//
  static const uint8_t CACHE_STATUS_DIRTY = 1;
  static const uint8_t CACHE_STATUS_FAT_BLOCK = 2;
  static const uint8_t CACHE_STATUS_MASK
     = CACHE_STATUS_DIRTY | CACHE_STATUS_FAT_BLOCK;
  static const uint8_t CACHE_OPTION_NO_READ = 4;
  // value for option argument in cacheFetch to indicate read from cache
  static uint8_t const CACHE_FOR_READ = 0;
  // value for option argument in cacheFetch to indicate write to cache
  static uint8_t const CACHE_FOR_WRITE = CACHE_STATUS_DIRTY;
  // reserve cache block with no read
  static uint8_t const CACHE_RESERVE_FOR_WRITE
     = CACHE_STATUS_DIRTY | CACHE_OPTION_NO_READ;
#if USE_MULTIPLE_CARDS
  uint8_t m_fatCount;           // number of FATs on volume
  uint32_t m_blocksPerFat;      // FAT size in blocks
  cache_t m_cacheBuffer;        // 512 byte cache for device blocks
  uint32_t m_cacheBlockNumber;  // Logical number of block in the cache
  Sd2Card* m_sdCard;            // Sd2Card object for cache
  uint8_t m_cacheStatus;        // status of cache block
#if USE_SEPARATE_FAT_CACHE
  cache_t m_cacheFatBuffer;       // 512 byte cache for FAT
  uint32_t m_cacheFatBlockNumber;  // current Fat block number
  uint8_t  m_cacheFatStatus;       // status of cache Fatblock
#endif  // USE_SEPARATE_FAT_CACHE
#else  // USE_MULTIPLE_CARDS
  static uint8_t m_fatCount;            // number of FATs on volume
  static uint32_t m_blocksPerFat;       // FAT size in blocks
  static cache_t m_cacheBuffer;        // 512 byte cache for device blocks
  static uint32_t m_cacheBlockNumber;  // Logical number of block in the cache
  static uint8_t m_cacheStatus;        // status of cache block
#if USE_SEPARATE_FAT_CACHE
  static cache_t m_cacheFatBuffer;       // 512 byte cache for FAT
  static uint32_t m_cacheFatBlockNumber;  // current Fat block number
  static uint8_t  m_cacheFatStatus;       // status of cache Fatblock
#endif  // USE_SEPARATE_FAT_CACHE
  static Sd2Card* m_sdCard;            // Sd2Card object for cache
#endif  // USE_MULTIPLE_CARDS

  cache_t *cacheAddress() {return &m_cacheBuffer;}
  uint32_t cacheBlockNumber() {return m_cacheBlockNumber;}
#if USE_MULTIPLE_CARDS
  cache_t* cacheFetch(uint32_t blockNumber, uint8_t options);
  cache_t* cacheFetchData(uint32_t blockNumber, uint8_t options);
  cache_t* cacheFetchFat(uint32_t blockNumber, uint8_t options);
  void cacheInvalidate();
  bool cacheSync();
  bool cacheWriteData();
  bool cacheWriteFat();
#else  // USE_MULTIPLE_CARDS
  static cache_t* cacheFetch(uint32_t blockNumber, uint8_t options);
  static cache_t* cacheFetchData(uint32_t blockNumber, uint8_t options);
  static cache_t* cacheFetchFat(uint32_t blockNumber, uint8_t options);
  static void cacheInvalidate();
  static bool cacheSync();
  static bool cacheWriteData();
  static bool cacheWriteFat();
#endif  // USE_MULTIPLE_CARDS
//------------------------------------------------------------------------------
  bool allocContiguous(uint32_t count, uint32_t* curCluster);
  uint8_t blockOfCluster(uint32_t position) const {
    return (position >> 9) & m_clusterBlockMask;}
  uint32_t clusterStartBlock(uint32_t cluster) const;
  bool fatGet(uint32_t cluster, uint32_t* value);
  bool fatPut(uint32_t cluster, uint32_t value);
  bool fatPutEOC(uint32_t cluster) {
    return fatPut(cluster, 0x0FFFFFFF);
  }
  bool freeChain(uint32_t cluster);
  bool isEOC(uint32_t cluster) const {
    if (FAT12_SUPPORT && m_fatType == 12) return  cluster >= FAT12EOC_MIN;
    if (m_fatType == 16) return cluster >= FAT16EOC_MIN;
    return  cluster >= FAT32EOC_MIN;
  }
  bool readBlock(uint32_t block, uint8_t* dst) {
    return m_sdCard->readBlock(block, dst);}
  bool writeBlock(uint32_t block, const uint8_t* dst) {
    return m_sdCard->writeBlock(block, dst);
  }
};
#endif  // SdVolume

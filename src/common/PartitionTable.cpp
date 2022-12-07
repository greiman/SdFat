/**
 * Copyright (c) 2011-2022 Bill Greiman
 * 
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

#include "PartitionTable.h"
#include "DebugMacros.h"
#include "FsStructs.h"

// Read MBR format partition table, retrieve the start sector number of a volume
static uint32_t partitionTableGetVolumeStartSectorMBR(FsCache& fscache, uint8_t part)
{
  if (part <= 0 || part > 4) {
    DBG_FAIL_MACRO;
    return 0;
  }

  const uint8_t *sector = fscache.prepare(0, FsCache::CACHE_FOR_READ);
  const MbrSector_t *mbr = reinterpret_cast<const MbrSector_t*>(sector);
  if (!mbr) {
    DBG_FAIL_MACRO;
    return 0;
  }
  
  const MbrPart_t *mp = mbr->part + part - 1;
  if (mp->type == 0 || getLe32(mp->totalSectors) == 0) {
    DBG_FAIL_MACRO;
    return 0;
  }

  return getLe32(mp->relativeSectors);
}

// Read GPT format partition table, retrieve the start sector number of a volume
static uint32_t partitionTableGetVolumeStartSectorGPT(FsCache& fscache, uint8_t part)
{
  if (part <= 0 || part > 4) {
    DBG_FAIL_MACRO;
    return 0;
  }

  const uint8_t *sector = fscache.prepare(1, FsCache::CACHE_FOR_READ);
  const GPT_Header_t *gpt = reinterpret_cast<const GPT_Header_t*>(sector);

  if (getLe64(gpt->signature) != 0x5452415020494645ULL) {
    DBG_FAIL_MACRO;
    return 0;
  }

  // First 4 partition table entries are in LBA 2
  sector = fscache.prepare(2, FsCache::CACHE_FOR_READ);
  const GPT_PartitionEntry_t *partentry =
    reinterpret_cast<const GPT_PartitionEntry_t*>(sector + 128 * (part - 1));

  uint64_t startSector = getLe64(partentry->first_lba);
  uint32_t startSector32 = static_cast<uint32_t>(startSector);
  if (startSector32 != startSector)
  {
    // Currently limited to 2^32 sectors = 2 TB by other parts of SdFat code.
    DBG_FAIL_MACRO;
    return 0;
  }

  return startSector32;
}

uint32_t partitionTableGetVolumeStartSector(FsCache& fscache, uint8_t part)
{
  uint32_t start;
  
  // Check for GPT partition table first, because it has clearly identifiable
  // signature. It is also common for GPT-partitioned drives to have MBR-style
  // fallback boot record at the start.
  start = partitionTableGetVolumeStartSectorGPT(fscache, part);

  if (start == 0)
  {
    start = partitionTableGetVolumeStartSectorMBR(fscache, part);
  }

  return start;
}

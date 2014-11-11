/* Arduino Sd2Card Library
 * Copyright (C) 2012 by William Greiman
 *
 * This file is part of the Arduino Sd2Card Library
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
 * along with the Arduino Sd2Card Library.  If not, see
 * <http://www.gnu.org/licenses/>.
 */
#include <Sd2Card.h>
#include <SdSpi.h>
#if !USE_SOFTWARE_SPI && ENABLE_SPI_TRANSACTION
#include <SPI.h>
#endif  // !USE_SOFTWARE_SPI && defined(SPI_HAS_TRANSACTION)
// debug trace macro
#define SD_TRACE(m, b)
// #define SD_TRACE(m, b) Serial.print(m);Serial.println(b);
//------------------------------------------------------------------------------
SdSpi Sd2Card::m_spi;
//==============================================================================
#if USE_SD_CRC
// CRC functions
//------------------------------------------------------------------------------
static uint8_t CRC7(const uint8_t* data, uint8_t n) {
  uint8_t crc = 0;
  for (uint8_t i = 0; i < n; i++) {
    uint8_t d = data[i];
    for (uint8_t j = 0; j < 8; j++) {
      crc <<= 1;
      if ((d & 0x80) ^ (crc & 0x80)) crc ^= 0x09;
      d <<= 1;
    }
  }
  return (crc << 1) | 1;
}
//------------------------------------------------------------------------------
#if USE_SD_CRC == 1
// slower CRC-CCITT
// uses the x^16,x^12,x^5,x^1 polynomial.
static uint16_t CRC_CCITT(const uint8_t *data, size_t n) {
  uint16_t crc = 0;
  for (size_t i = 0; i < n; i++) {
    crc = (uint8_t)(crc >> 8) | (crc << 8);
    crc ^= data[i];
    crc ^= (uint8_t)(crc & 0xff) >> 4;
    crc ^= crc << 12;
    crc ^= (crc & 0xff) << 5;
  }
  return crc;
}
#elif USE_SD_CRC > 1  // CRC_CCITT
//------------------------------------------------------------------------------
// faster CRC-CCITT
// uses the x^16,x^12,x^5,x^1 polynomial.
#ifdef __AVR__
static const uint16_t crctab[] PROGMEM = {
#else  // __AVR__
static const uint16_t crctab[] = {
#endif  // __AVR__
  0x0000, 0x1021, 0x2042, 0x3063, 0x4084, 0x50A5, 0x60C6, 0x70E7,
  0x8108, 0x9129, 0xA14A, 0xB16B, 0xC18C, 0xD1AD, 0xE1CE, 0xF1EF,
  0x1231, 0x0210, 0x3273, 0x2252, 0x52B5, 0x4294, 0x72F7, 0x62D6,
  0x9339, 0x8318, 0xB37B, 0xA35A, 0xD3BD, 0xC39C, 0xF3FF, 0xE3DE,
  0x2462, 0x3443, 0x0420, 0x1401, 0x64E6, 0x74C7, 0x44A4, 0x5485,
  0xA56A, 0xB54B, 0x8528, 0x9509, 0xE5EE, 0xF5CF, 0xC5AC, 0xD58D,
  0x3653, 0x2672, 0x1611, 0x0630, 0x76D7, 0x66F6, 0x5695, 0x46B4,
  0xB75B, 0xA77A, 0x9719, 0x8738, 0xF7DF, 0xE7FE, 0xD79D, 0xC7BC,
  0x48C4, 0x58E5, 0x6886, 0x78A7, 0x0840, 0x1861, 0x2802, 0x3823,
  0xC9CC, 0xD9ED, 0xE98E, 0xF9AF, 0x8948, 0x9969, 0xA90A, 0xB92B,
  0x5AF5, 0x4AD4, 0x7AB7, 0x6A96, 0x1A71, 0x0A50, 0x3A33, 0x2A12,
  0xDBFD, 0xCBDC, 0xFBBF, 0xEB9E, 0x9B79, 0x8B58, 0xBB3B, 0xAB1A,
  0x6CA6, 0x7C87, 0x4CE4, 0x5CC5, 0x2C22, 0x3C03, 0x0C60, 0x1C41,
  0xEDAE, 0xFD8F, 0xCDEC, 0xDDCD, 0xAD2A, 0xBD0B, 0x8D68, 0x9D49,
  0x7E97, 0x6EB6, 0x5ED5, 0x4EF4, 0x3E13, 0x2E32, 0x1E51, 0x0E70,
  0xFF9F, 0xEFBE, 0xDFDD, 0xCFFC, 0xBF1B, 0xAF3A, 0x9F59, 0x8F78,
  0x9188, 0x81A9, 0xB1CA, 0xA1EB, 0xD10C, 0xC12D, 0xF14E, 0xE16F,
  0x1080, 0x00A1, 0x30C2, 0x20E3, 0x5004, 0x4025, 0x7046, 0x6067,
  0x83B9, 0x9398, 0xA3FB, 0xB3DA, 0xC33D, 0xD31C, 0xE37F, 0xF35E,
  0x02B1, 0x1290, 0x22F3, 0x32D2, 0x4235, 0x5214, 0x6277, 0x7256,
  0xB5EA, 0xA5CB, 0x95A8, 0x8589, 0xF56E, 0xE54F, 0xD52C, 0xC50D,
  0x34E2, 0x24C3, 0x14A0, 0x0481, 0x7466, 0x6447, 0x5424, 0x4405,
  0xA7DB, 0xB7FA, 0x8799, 0x97B8, 0xE75F, 0xF77E, 0xC71D, 0xD73C,
  0x26D3, 0x36F2, 0x0691, 0x16B0, 0x6657, 0x7676, 0x4615, 0x5634,
  0xD94C, 0xC96D, 0xF90E, 0xE92F, 0x99C8, 0x89E9, 0xB98A, 0xA9AB,
  0x5844, 0x4865, 0x7806, 0x6827, 0x18C0, 0x08E1, 0x3882, 0x28A3,
  0xCB7D, 0xDB5C, 0xEB3F, 0xFB1E, 0x8BF9, 0x9BD8, 0xABBB, 0xBB9A,
  0x4A75, 0x5A54, 0x6A37, 0x7A16, 0x0AF1, 0x1AD0, 0x2AB3, 0x3A92,
  0xFD2E, 0xED0F, 0xDD6C, 0xCD4D, 0xBDAA, 0xAD8B, 0x9DE8, 0x8DC9,
  0x7C26, 0x6C07, 0x5C64, 0x4C45, 0x3CA2, 0x2C83, 0x1CE0, 0x0CC1,
  0xEF1F, 0xFF3E, 0xCF5D, 0xDF7C, 0xAF9B, 0xBFBA, 0x8FD9, 0x9FF8,
  0x6E17, 0x7E36, 0x4E55, 0x5E74, 0x2E93, 0x3EB2, 0x0ED1, 0x1EF0
};
static uint16_t CRC_CCITT(const uint8_t* data, size_t n) {
  uint16_t crc = 0;
  for (size_t i = 0; i < n; i++) {
#ifdef __AVR__
    crc = pgm_read_word(&crctab[(crc >> 8 ^ data[i]) & 0XFF]) ^ (crc << 8);
#else  // __AVR__
    crc = crctab[(crc >> 8 ^ data[i]) & 0XFF] ^ (crc << 8);
#endif  // __AVR__
  }
  return crc;
}
#endif  // CRC_CCITT
#endif  // USE_SD_CRC
//==============================================================================
// Sd2Card member functions
//------------------------------------------------------------------------------
/**
 * Initialize an SD flash memory card.
 *
 * \param[in] chipSelectPin SD chip select pin number.
 * \param[in] sckDivisor SPI SCK clock rate divisor.
 *
 * \return The value one, true, is returned for success and
 * the value zero, false, is returned for failure.  The reason for failure
 * can be determined by calling errorCode() and errorData().
 */
bool Sd2Card::begin(uint8_t chipSelectPin, uint8_t sckDivisor) {
  m_errorCode = m_type = 0;
  m_chipSelectPin = chipSelectPin;
  // 16-bit init start time allows over a minute
  uint16_t t0 = (uint16_t)millis();
  uint32_t arg;

  pinMode(m_chipSelectPin, OUTPUT);
  digitalWrite(m_chipSelectPin, HIGH);
  m_spi.begin();

  // set SCK rate for initialization commands
  m_sckDivisor = SPI_SCK_INIT_DIVISOR;
  m_spi.init(m_sckDivisor);

  // must supply min of 74 clock cycles with CS high.
  for (uint8_t i = 0; i < 10; i++) m_spi.send(0XFF);

  // command to go idle in SPI mode
  while (cardCommand(CMD0, 0) != R1_IDLE_STATE) {
    if (((uint16_t)millis() - t0) > SD_INIT_TIMEOUT) {
      error(SD_CARD_ERROR_CMD0);
      goto fail;
    }
  }
#if USE_SD_CRC
  if (cardCommand(CMD59, 1) != R1_IDLE_STATE) {
    error(SD_CARD_ERROR_CMD59);
    goto fail;
  }
#endif  // USE_SD_CRC
  // check SD version
  while (1) {
    if (cardCommand(CMD8, 0x1AA) == (R1_ILLEGAL_COMMAND | R1_IDLE_STATE)) {
      type(SD_CARD_TYPE_SD1);
      break;
    }
    for (uint8_t i = 0; i < 4; i++) m_status = m_spi.receive();
    if (m_status == 0XAA) {
      type(SD_CARD_TYPE_SD2);
      break;
    }
    if (((uint16_t)millis() - t0) > SD_INIT_TIMEOUT) {
      error(SD_CARD_ERROR_CMD8);
      goto fail;
    }
  }
  // initialize card and send host supports SDHC if SD2
  arg = type() == SD_CARD_TYPE_SD2 ? 0X40000000 : 0;

  while (cardAcmd(ACMD41, arg) != R1_READY_STATE) {
    // check for timeout
    if (((uint16_t)millis() - t0) > SD_INIT_TIMEOUT) {
      error(SD_CARD_ERROR_ACMD41);
      goto fail;
    }
  }
  // if SD2 read OCR register to check for SDHC card
  if (type() == SD_CARD_TYPE_SD2) {
    if (cardCommand(CMD58, 0)) {
      error(SD_CARD_ERROR_CMD58);
      goto fail;
    }
    if ((m_spi.receive() & 0XC0) == 0XC0) type(SD_CARD_TYPE_SDHC);
    // Discard rest of ocr - contains allowed voltage range.
    for (uint8_t i = 0; i < 3; i++) m_spi.receive();
  }
  chipSelectHigh();
  m_sckDivisor = sckDivisor;
  return true;

 fail:
  chipSelectHigh();
  return false;
}
//------------------------------------------------------------------------------
// send command and return error code.  Return zero for OK
uint8_t Sd2Card::cardCommand(uint8_t cmd, uint32_t arg) {
  // select card
  chipSelectLow();

  // wait if busy
  waitNotBusy(SD_WRITE_TIMEOUT);

  uint8_t *pa = reinterpret_cast<uint8_t *>(&arg);

#if USE_SD_CRC
  // form message
  uint8_t d[6] = {cmd | 0X40, pa[3], pa[2], pa[1], pa[0]};

  // add crc
  d[5] = CRC7(d, 5);

  // send message
  for (uint8_t k = 0; k < 6; k++) m_spi.send(d[k]);
#else  // USE_SD_CRC
  // send command
  m_spi.send(cmd | 0x40);

  // send argument
  for (int8_t i = 3; i >= 0; i--) m_spi.send(pa[i]);

  // send CRC - correct for CMD0 with arg zero or CMD8 with arg 0X1AA
  m_spi.send(cmd == CMD0 ? 0X95 : 0X87);
#endif  // USE_SD_CRC

  // skip stuff byte for stop read
  if (cmd == CMD12) m_spi.receive();

  // wait for response
  for (uint8_t i = 0; ((m_status = m_spi.receive()) & 0X80) && i != 0XFF; i++) {
  }
  return m_status;
}
//------------------------------------------------------------------------------
/**
 * Determine the size of an SD flash memory card.
 *
 * \return The number of 512 byte data blocks in the card
 *         or zero if an error occurs.
 */
uint32_t Sd2Card::cardSize() {
  csd_t csd;
  if (!readCSD(&csd)) return 0;
  if (csd.v1.csd_ver == 0) {
    uint8_t read_bl_len = csd.v1.read_bl_len;
    uint16_t c_size = (csd.v1.c_size_high << 10)
                      | (csd.v1.c_size_mid << 2) | csd.v1.c_size_low;
    uint8_t c_size_mult = (csd.v1.c_size_mult_high << 1)
                          | csd.v1.c_size_mult_low;
    return (uint32_t)(c_size + 1) << (c_size_mult + read_bl_len - 7);
  } else if (csd.v2.csd_ver == 1) {
    uint32_t c_size = 0X10000L * csd.v2.c_size_high + 0X100L
                      * (uint32_t)csd.v2.c_size_mid + csd.v2.c_size_low;
    return (c_size + 1) << 10;
  } else {
    error(SD_CARD_ERROR_BAD_CSD);
    return 0;
  }
}
//------------------------------------------------------------------------------
void Sd2Card::spiYield() {
#if ENABLE_SPI_YIELD && !USE_SOFTWARE_SPI && defined(SPI_HAS_TRANSACTION)
  chipSelectHigh();
  chipSelectLow();
#endif  // ENABLE_SPI_YIELD && !USE_SOFTWARE_SPI && defined(SPI_HAS_TRANSACTION)
}
//------------------------------------------------------------------------------
void Sd2Card::chipSelectHigh() {
  digitalWrite(m_chipSelectPin, HIGH);
  // insure MISO goes high impedance
  m_spi.send(0XFF);
#if !USE_SOFTWARE_SPI && defined(SPI_HAS_TRANSACTION)
  SPI.endTransaction();
#endif  // !USE_SOFTWARE_SPI && defined(SPI_HAS_TRANSACTION)
}
//------------------------------------------------------------------------------
void Sd2Card::chipSelectLow() {
#if !USE_SOFTWARE_SPI && defined(SPI_HAS_TRANSACTION)
  SPI.beginTransaction(SPISettings());
#endif  // !USE_SOFTWARE_SPI && defined(SPI_HAS_TRANSACTION)
  m_spi.init(m_sckDivisor);
  digitalWrite(m_chipSelectPin, LOW);
}
//------------------------------------------------------------------------------
/** Erase a range of blocks.
 *
 * \param[in] firstBlock The address of the first block in the range.
 * \param[in] lastBlock The address of the last block in the range.
 *
 * \note This function requests the SD card to do a flash erase for a
 * range of blocks.  The data on the card after an erase operation is
 * either 0 or 1, depends on the card vendor.  The card must support
 * single block erase.
 *
 * \return The value one, true, is returned for success and
 * the value zero, false, is returned for failure.
 */
bool Sd2Card::erase(uint32_t firstBlock, uint32_t lastBlock) {
  csd_t csd;
  if (!readCSD(&csd)) goto fail;
  // check for single block erase
  if (!csd.v1.erase_blk_en) {
    // erase size mask
    uint8_t m = (csd.v1.sector_size_high << 1) | csd.v1.sector_size_low;
    if ((firstBlock & m) != 0 || ((lastBlock + 1) & m) != 0) {
      // error card can't erase specified area
      error(SD_CARD_ERROR_ERASE_SINGLE_BLOCK);
      goto fail;
    }
  }
  if (m_type != SD_CARD_TYPE_SDHC) {
    firstBlock <<= 9;
    lastBlock <<= 9;
  }
  if (cardCommand(CMD32, firstBlock)
    || cardCommand(CMD33, lastBlock)
    || cardCommand(CMD38, 0)) {
      error(SD_CARD_ERROR_ERASE);
      goto fail;
  }
  if (!waitNotBusy(SD_ERASE_TIMEOUT)) {
    error(SD_CARD_ERROR_ERASE_TIMEOUT);
    goto fail;
  }
  chipSelectHigh();
  return true;

 fail:
  chipSelectHigh();
  return false;
}
//------------------------------------------------------------------------------
/** Determine if card supports single block erase.
 *
 * \return The value one, true, is returned if single block erase is supported.
 * The value zero, false, is returned if single block erase is not supported.
 */
bool Sd2Card::eraseSingleBlockEnable() {
  csd_t csd;
  return readCSD(&csd) ? csd.v1.erase_blk_en : false;
}
//------------------------------------------------------------------------------
/**
 * Check for busy.  MISO low indicates the card is busy.
 * 
 * \return true if busy else false.
 */
bool Sd2Card::isBusy() {
  bool rtn;
  chipSelectLow();
  for (uint8_t i = 0; i < 8; i++) {
    rtn = m_spi.receive() != 0XFF;
    if (!rtn) break;
  }
  chipSelectHigh();
  return rtn;
}
//------------------------------------------------------------------------------
/**
 * Read a 512 byte block from an SD card.
 *
 * \param[in] blockNumber Logical block to be read.
 * \param[out] dst Pointer to the location that will receive the data.

 * \return The value one, true, is returned for success and
 * the value zero, false, is returned for failure.
 */
bool Sd2Card::readBlock(uint32_t blockNumber, uint8_t* dst) {
  SD_TRACE("RB", blockNumber);
  // use address if not SDHC card
  if (type()!= SD_CARD_TYPE_SDHC) blockNumber <<= 9;
  if (cardCommand(CMD17, blockNumber)) {
    error(SD_CARD_ERROR_CMD17);
    goto fail;
  }
  return readData(dst, 512);

 fail:
  chipSelectHigh();
  return false;
}
//------------------------------------------------------------------------------
/** Read one data block in a multiple block read sequence
 *
 * \param[in] dst Pointer to the location for the data to be read.
 *
 * \return The value one, true, is returned for success and
 * the value zero, false, is returned for failure.
 */
bool Sd2Card::readData(uint8_t *dst) {
  chipSelectLow();
  return readData(dst, 512);
}
//------------------------------------------------------------------------------
bool Sd2Card::readData(uint8_t* dst, size_t count) {
#if USE_SD_CRC
  uint16_t crc;
#endif  // USE_SD_CRC
  // wait for start block token
  uint16_t t0 = millis();
  while ((m_status = m_spi.receive()) == 0XFF) {
    if (((uint16_t)millis() - t0) > SD_READ_TIMEOUT) {
      error(SD_CARD_ERROR_READ_TIMEOUT);
      goto fail;
    }
    spiYield();
  }
  if (m_status != DATA_START_BLOCK) {
    error(SD_CARD_ERROR_READ);
    goto fail;
  }
  // transfer data
  if ((m_status = m_spi.receive(dst, count))) {
    error(SD_CARD_ERROR_SPI_DMA);
    goto fail;
  }

#if USE_SD_CRC
  // get crc
  crc = (m_spi.receive() << 8) | m_spi.receive();
  if (crc != CRC_CCITT(dst, count)) {
    error(SD_CARD_ERROR_READ_CRC);
    goto fail;
  }
#else
  // discard crc
  m_spi.receive();
  m_spi.receive();
#endif  // USE_SD_CRC
  chipSelectHigh();
  return true;

 fail:
  chipSelectHigh();
  return false;
}
//------------------------------------------------------------------------------
/** Read OCR register.
 *
 * \param[out] ocr Value of OCR register.
 * \return true for success else false.
 */
bool Sd2Card::readOCR(uint32_t* ocr) {
  uint8_t *p = reinterpret_cast<uint8_t*>(ocr);
  if (cardCommand(CMD58, 0)) {
    error(SD_CARD_ERROR_CMD58);
    goto fail;
  }
  for (uint8_t i = 0; i < 4; i++) p[3-i] = m_spi.receive();

  chipSelectHigh();
  return true;

 fail:
  chipSelectHigh();
  return false;
}
//------------------------------------------------------------------------------
/** read CID or CSR register */
bool Sd2Card::readRegister(uint8_t cmd, void* buf) {
  uint8_t* dst = reinterpret_cast<uint8_t*>(buf);
  if (cardCommand(cmd, 0)) {
    error(SD_CARD_ERROR_READ_REG);
    goto fail;
  }
  return readData(dst, 16);

 fail:
  chipSelectHigh();
  return false;
}
//------------------------------------------------------------------------------
/** Start a read multiple blocks sequence.
 *
 * \param[in] blockNumber Address of first block in sequence.
 *
 * \note This function is used with readData() and readStop() for optimized
 * multiple block reads.  SPI chipSelect must be low for the entire sequence.
 *
 * \return The value one, true, is returned for success and
 * the value zero, false, is returned for failure.
 */
bool Sd2Card::readStart(uint32_t blockNumber) {
  SD_TRACE("RS", blockNumber);
  if (type()!= SD_CARD_TYPE_SDHC) blockNumber <<= 9;
  if (cardCommand(CMD18, blockNumber)) {
    error(SD_CARD_ERROR_CMD18);
    goto fail;
  }
  chipSelectHigh();
  return true;

 fail:
  chipSelectHigh();
  return false;
}
//------------------------------------------------------------------------------
/** End a read multiple blocks sequence.
 *
* \return The value one, true, is returned for success and
 * the value zero, false, is returned for failure.
 */
bool Sd2Card::readStop() {
  if (cardCommand(CMD12, 0)) {
    error(SD_CARD_ERROR_CMD12);
    goto fail;
  }
  chipSelectHigh();
  return true;

 fail:
  chipSelectHigh();
  return false;
}
//------------------------------------------------------------------------------
// wait for card to go not busy
bool Sd2Card::waitNotBusy(uint16_t timeoutMillis) {
  uint16_t t0 = millis();
  while (m_spi.receive() != 0XFF) {
    if (((uint16_t)millis() - t0) >= timeoutMillis) goto fail;
    spiYield();
  }
  return true;

 fail:
  return false;
}
//------------------------------------------------------------------------------
/**
 * Writes a 512 byte block to an SD card.
 *
 * \param[in] blockNumber Logical block to be written.
 * \param[in] src Pointer to the location of the data to be written.
 * \return The value one, true, is returned for success and
 * the value zero, false, is returned for failure.
 */
bool Sd2Card::writeBlock(uint32_t blockNumber, const uint8_t* src) {
  SD_TRACE("WB", blockNumber);
  // use address if not SDHC card
  if (type() != SD_CARD_TYPE_SDHC) blockNumber <<= 9;
  if (cardCommand(CMD24, blockNumber)) {
    error(SD_CARD_ERROR_CMD24);
    goto fail;
  }
  if (!writeData(DATA_START_BLOCK, src)) goto fail;

#define CHECK_PROGRAMMING 0
#if CHECK_PROGRAMMING
  // wait for flash programming to complete
  if (!waitNotBusy(SD_WRITE_TIMEOUT)) {
    error(SD_CARD_ERROR_WRITE_TIMEOUT);
    goto fail;
  }
  // response is r2 so get and check two bytes for nonzero
  if (cardCommand(CMD13, 0) || m_spi.receive()) {
    error(SD_CARD_ERROR_WRITE_PROGRAMMING);
    goto fail;
  }
#endif  // CHECK_PROGRAMMING

  chipSelectHigh();
  return true;

 fail:
  chipSelectHigh();
  return false;
}
//------------------------------------------------------------------------------
/** Write one data block in a multiple block write sequence
 * \param[in] src Pointer to the location of the data to be written.
 * \return The value one, true, is returned for success and
 * the value zero, false, is returned for failure.
 */
bool Sd2Card::writeData(const uint8_t* src) {
  chipSelectLow();
  // wait for previous write to finish
  if (!waitNotBusy(SD_WRITE_TIMEOUT)) goto fail;
  if (!writeData(WRITE_MULTIPLE_TOKEN, src)) goto fail;
  chipSelectHigh();
  return true;

 fail:
  error(SD_CARD_ERROR_WRITE_MULTIPLE);
  chipSelectHigh();
  return false;
}
//------------------------------------------------------------------------------
// send one block of data for write block or write multiple blocks
bool Sd2Card::writeData(uint8_t token, const uint8_t* src) {
#if USE_SD_CRC
  uint16_t crc = CRC_CCITT(src, 512);
#else  // USE_SD_CRC
  uint16_t crc = 0XFFFF;
#endif  // USE_SD_CRC
  m_spi.send(token);
  m_spi.send(src, 512);
  m_spi.send(crc >> 8);
  m_spi.send(crc & 0XFF);

  m_status = m_spi.receive();
  if ((m_status & DATA_RES_MASK) != DATA_RES_ACCEPTED) {
    error(SD_CARD_ERROR_WRITE);
    goto fail;
  }
  return true;

 fail:
  chipSelectHigh();
  return false;
}
//------------------------------------------------------------------------------
/** Start a write multiple blocks sequence.
 *
 * \param[in] blockNumber Address of first block in sequence.
 * \param[in] eraseCount The number of blocks to be pre-erased.
 *
 * \note This function is used with writeData() and writeStop()
 * for optimized multiple block writes.
 *
 * \return The value one, true, is returned for success and
 * the value zero, false, is returned for failure.
 */
bool Sd2Card::writeStart(uint32_t blockNumber, uint32_t eraseCount) {
  SD_TRACE("WS", blockNumber);
  // send pre-erase count
  if (cardAcmd(ACMD23, eraseCount)) {
    error(SD_CARD_ERROR_ACMD23);
    goto fail;
  }
  // use address if not SDHC card
  if (type() != SD_CARD_TYPE_SDHC) blockNumber <<= 9;
  if (cardCommand(CMD25, blockNumber)) {
    error(SD_CARD_ERROR_CMD25);
    goto fail;
  }
  chipSelectHigh();
  return true;

 fail:
  chipSelectHigh();
  return false;
}
//------------------------------------------------------------------------------
/** End a write multiple blocks sequence.
 *
* \return The value one, true, is returned for success and
 * the value zero, false, is returned for failure.
 */
bool Sd2Card::writeStop() {
  chipSelectLow();
  if (!waitNotBusy(SD_WRITE_TIMEOUT)) goto fail;
  m_spi.send(STOP_TRAN_TOKEN);
  if (!waitNotBusy(SD_WRITE_TIMEOUT)) goto fail;
  chipSelectHigh();
  return true;

 fail:
  error(SD_CARD_ERROR_STOP_TRAN);
  chipSelectHigh();
  return false;
}

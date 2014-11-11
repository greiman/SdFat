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
#ifndef SpiCard_h
#define SpiCard_h
/**
 * \file
 * \brief Sd2Card class for V2 SD/SDHC cards
 */
#include <Arduino.h>
#include <SdFatConfig.h>
#include <SdInfo.h>
#include <SdSpi.h>
//------------------------------------------------------------------------------
// SD card errors
/** timeout error for command CMD0 (initialize card in SPI mode) */
uint8_t const SD_CARD_ERROR_CMD0 = 0X1;
/** CMD8 was not accepted - not a valid SD card*/
uint8_t const SD_CARD_ERROR_CMD8 = 0X2;
/** card returned an error response for CMD12 (stop multiblock read) */
uint8_t const SD_CARD_ERROR_CMD12 = 0X3;
/** card returned an error response for CMD17 (read block) */
uint8_t const SD_CARD_ERROR_CMD17 = 0X4;
/** card returned an error response for CMD18 (read multiple block) */
uint8_t const SD_CARD_ERROR_CMD18 = 0X5;
/** card returned an error response for CMD24 (write block) */
uint8_t const SD_CARD_ERROR_CMD24 = 0X6;
/**  WRITE_MULTIPLE_BLOCKS command failed */
uint8_t const SD_CARD_ERROR_CMD25 = 0X7;
/** card returned an error response for CMD58 (read OCR) */
uint8_t const SD_CARD_ERROR_CMD58 = 0X8;
/** SET_WR_BLK_ERASE_COUNT failed */
uint8_t const SD_CARD_ERROR_ACMD23 = 0X9;
/** ACMD41 initialization process timeout */
uint8_t const SD_CARD_ERROR_ACMD41 = 0XA;
/** card returned a bad CSR version field */
uint8_t const SD_CARD_ERROR_BAD_CSD = 0XB;
/** erase block group command failed */
uint8_t const SD_CARD_ERROR_ERASE = 0XC;
/** card not capable of single block erase */
uint8_t const SD_CARD_ERROR_ERASE_SINGLE_BLOCK = 0XD;
/** Erase sequence timed out */
uint8_t const SD_CARD_ERROR_ERASE_TIMEOUT = 0XE;
/** card returned an error token instead of read data */
uint8_t const SD_CARD_ERROR_READ = 0XF;
/** read CID or CSD failed */
uint8_t const SD_CARD_ERROR_READ_REG = 0X10;
/** timeout while waiting for start of read data */
uint8_t const SD_CARD_ERROR_READ_TIMEOUT = 0X11;
/** card did not accept STOP_TRAN_TOKEN */
uint8_t const SD_CARD_ERROR_STOP_TRAN = 0X12;
/** card returned an error token as a response to a write operation */
uint8_t const SD_CARD_ERROR_WRITE = 0X13;
/** attempt to write protected block zero */
uint8_t const SD_CARD_ERROR_WRITE_BLOCK_ZERO = 0X14;  // REMOVE - not used
/** card did not go ready for a multiple block write */
uint8_t const SD_CARD_ERROR_WRITE_MULTIPLE = 0X15;
/** card returned an error to a CMD13 status check after a write */
uint8_t const SD_CARD_ERROR_WRITE_PROGRAMMING = 0X16;
/** timeout occurred during write programming */
uint8_t const SD_CARD_ERROR_WRITE_TIMEOUT = 0X17;
/** incorrect rate selected */
uint8_t const SD_CARD_ERROR_SCK_RATE = 0X18;
/** init() not called */
uint8_t const SD_CARD_ERROR_INIT_NOT_CALLED = 0X19;
/** card returned an error for CMD59 (CRC_ON_OFF) */
uint8_t const SD_CARD_ERROR_CMD59 = 0X1A;
/** invalid read CRC */
uint8_t const SD_CARD_ERROR_READ_CRC = 0X1B;
/** SPI DMA error */
uint8_t const SD_CARD_ERROR_SPI_DMA = 0X1C;
//------------------------------------------------------------------------------
// card types
/** Standard capacity V1 SD card */
uint8_t const SD_CARD_TYPE_SD1  = 1;
/** Standard capacity V2 SD card */
uint8_t const SD_CARD_TYPE_SD2  = 2;
/** High Capacity SD card */
uint8_t const SD_CARD_TYPE_SDHC = 3;
//------------------------------------------------------------------------------
/**
 * \class Sd2Card
 * \brief Raw access to SD and SDHC flash memory cards.
 */
class Sd2Card {
 public:
  /** Construct an instance of Sd2Card. */
  Sd2Card() : m_errorCode(SD_CARD_ERROR_INIT_NOT_CALLED), m_type(0) {}
  bool begin(uint8_t chipSelectPin = SD_CHIP_SELECT_PIN,
            uint8_t sckDivisor = SPI_FULL_SPEED);
  uint32_t cardSize();
  bool erase(uint32_t firstBlock, uint32_t lastBlock);
  bool eraseSingleBlockEnable();
  /**
   *  Set SD error code.
   *  \param[in] code value for error code.
   */
  void error(uint8_t code) {m_errorCode = code;}
  /**
   * \return error code for last error. See Sd2Card.h for a list of error codes.
   */
  int errorCode() const {return m_errorCode;}
  /** \return error data for last error. */
  int errorData() const {return m_status;}
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
  bool init(uint8_t sckDivisor = SPI_FULL_SPEED,
            uint8_t chipSelectPin = SD_CHIP_SELECT_PIN) {
    return begin(chipSelectPin, sckDivisor);
  }
  bool isBusy();
  bool readBlock(uint32_t block, uint8_t* dst);
  /**
   * Read a card's CID register. The CID contains card identification
   * information such as Manufacturer ID, Product name, Product serial
   * number and Manufacturing date. 
   *
   * \param[out] cid pointer to area for returned data.
   *
   * \return true for success or false for failure.
   */
  bool readCID(cid_t* cid) {
    return readRegister(CMD10, cid);
  }
  /**
   * Read a card's CSD register. The CSD contains Card-Specific Data that
   * provides information regarding access to the card's contents.
   *
   * \param[out] csd pointer to area for returned data.
   *
   * \return true for success or false for failure.
   */
  bool readCSD(csd_t* csd) {
    return readRegister(CMD9, csd);
  }
  bool readData(uint8_t *dst);
  bool readOCR(uint32_t* ocr);
  bool readStart(uint32_t blockNumber);
  bool readStop();
  /** Return SCK divisor.
   *
   * \return Requested SCK divisor.
   */
  uint8_t sckDivisor() {return m_sckDivisor;}
  /** Return the card type: SD V1, SD V2 or SDHC
   * \return 0 - SD V1, 1 - SD V2, or 3 - SDHC.
   */
  int type() const {return m_type;}
  bool writeBlock(uint32_t blockNumber, const uint8_t* src);
  bool writeData(const uint8_t* src);
  bool writeStart(uint32_t blockNumber, uint32_t eraseCount);
  bool writeStop();

 private:
  //----------------------------------------------------------------------------
  // private functions
  uint8_t cardAcmd(uint8_t cmd, uint32_t arg) {
    cardCommand(CMD55, 0);
    return cardCommand(cmd, arg);
  }
  uint8_t cardCommand(uint8_t cmd, uint32_t arg);
  bool readData(uint8_t* dst, size_t count);
  bool readRegister(uint8_t cmd, void* buf);
  void chipSelectHigh();
  void chipSelectLow();
  void spiYield();
  void type(uint8_t value) {m_type = value;}
  bool waitNotBusy(uint16_t timeoutMillis);
  bool writeData(uint8_t token, const uint8_t* src);
  // private data
  static SdSpi m_spi;
  uint8_t m_chipSelectPin;
  uint8_t m_errorCode;
  uint8_t m_sckDivisor;
  uint8_t m_status;
  uint8_t m_type;
};
#endif  // SpiCard_h

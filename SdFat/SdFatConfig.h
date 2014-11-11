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
 * \brief configuration definitions
 */
#ifndef SdFatConfig_h
#define SdFatConfig_h
#include <stdint.h>
#ifdef __AVR__
#include <avr/io.h>
#endif  // __AVR__
//------------------------------------------------------------------------------
/**
 * Set SD_FILE_USES_STREAM nonzero to use Stream instead of Print for SdFile.
 * Using Stream will use more flash and may cause compatibility problems
 * with code written for older versions of SdFat. 
 */
#define SD_FILE_USES_STREAM 0

//------------------------------------------------------------------------------
/**
 * To enable SD card CRC checking set USE_SD_CRC nonzero.
 *
 * Set USE_SD_CRC to 1 to use a smaller slower CRC-CCITT function.
 *
 * Set USE_SD_CRC to 2 to used a larger faster table driven CRC-CCITT function.
 */
#define USE_SD_CRC 0
//------------------------------------------------------------------------------
/**
 * To use multiple SD cards set USE_MULTIPLE_CARDS nonzero.
 *
 * Using multiple cards costs about 200  bytes of flash.
 *
 * Each card requires about 550 bytes of SRAM so use of a Mega is recommended.
 */
#define USE_MULTIPLE_CARDS 0
//------------------------------------------------------------------------------
/**
 * Set DESTRUCTOR_CLOSES_FILE nonzero to close a file in its destructor.
 *
 * Causes use of lots of heap in ARM.
 */
#define DESTRUCTOR_CLOSES_FILE 0
//------------------------------------------------------------------------------
/**
 * For AVR
 *
 * Set USE_SERIAL_FOR_STD_OUT nonzero to use Serial (the HardwareSerial class)
 * for error messages and output from print functions like ls().
 *
 * If USE_SERIAL_FOR_STD_OUT is zero, a small non-interrupt driven class
 * is used to output messages to serial port zero.  This allows an alternate
 * Serial library like SerialPort to be used with SdFat.
 *
 * You can redirect stdOut with SdFat::setStdOut(Print* stream) and
 * get the current stream with SdFat::stdOut().
 */
#define USE_SERIAL_FOR_STD_OUT 0
//------------------------------------------------------------------------------
/**
 * Set FAT12_SUPPORT nonzero to enable use if FAT12 volumes.
 * FAT12 has not been well tested and requires additional flash.
 */
#define FAT12_SUPPORT 0
//------------------------------------------------------------------------------
/**
 * Set ENABLE_SPI_TRANSACTION nonzero to enable the SPI transaction feature
 * of the standard Arduino SPI library.  You must include SPI.h in your
 * sketches when ENABLE_SPI_TRANSACTION is nonzero.
 */
#define ENABLE_SPI_TRANSACTION 0
//------------------------------------------------------------------------------
/**
 * Set ENABLE_SPI_YIELD nonzero to enable release of the SPI bus during
 * SD card busy waits.  
 *
 * This will allow interrupt routines to access the SPI bus if 
 * ENABLE_SPI_TRANSACTION is nonzero.
 * 
 * Setting ENABLE_SPI_YIELD will introduce some extra overhead and will
 * slightly slow transfer rates.  A few older SD cards may fail when 
 * ENABLE_SPI_YIELD is nonzero.
 */
#define ENABLE_SPI_YIELD 0
//------------------------------------------------------------------------------
/**
 * Set USE_ARDUINO_SPI_LIBRARY nonzero to force use of Arduino Standard
 * SPI library. This will override native and software SPI for all boards.
 */
#define USE_ARDUINO_SPI_LIBRARY 0
//------------------------------------------------------------------------------
/**
 * Set AVR_SOFT_SPI nonzero to use software SPI on all AVR Arduinos.
 */
#define AVR_SOFT_SPI 0
//------------------------------------------------------------------------------
/**
 * Set DUE_SOFT_SPI nonzero to use software SPI on Due Arduinos.
 */
#define DUE_SOFT_SPI 0
//------------------------------------------------------------------------------

/**
 * Set LEONARDO_SOFT_SPI nonzero to use software SPI on Leonardo Arduinos.
 * LEONARDO_SOFT_SPI allows an unmodified 328 Shield to be used
 * on Leonardo Arduinos.
 */
#define LEONARDO_SOFT_SPI 0
//------------------------------------------------------------------------------
/**
 * Set MEGA_SOFT_SPI nonzero to use software SPI on Mega Arduinos.
 * MEGA_SOFT_SPI allows an unmodified 328 Shield to be used
 * on Mega Arduinos.
 */
#define MEGA_SOFT_SPI 0
//------------------------------------------------------------------------------
/**
 * Set TEENSY3_SOFT_SPI nonzero to use software SPI on Teensy 3.x boards.
 */
#define TEENSY3_SOFT_SPI 0
//------------------------------------------------------------------------------
/** 
 * Define software SPI pins.  Default allows Uno shields to be used on other 
 * boards.
 */
// define software SPI pins
/** Default Software SPI chip select pin */
uint8_t const SOFT_SPI_CS_PIN = 10;
/** Software SPI Master Out Slave In pin */
uint8_t const SOFT_SPI_MOSI_PIN = 11;
/** Software SPI Master In Slave Out pin */
uint8_t const SOFT_SPI_MISO_PIN = 12;
/** Software SPI Clock pin */
uint8_t const SOFT_SPI_SCK_PIN = 13;
//------------------------------------------------------------------------------
/**
 * Call flush for endl if ENDL_CALLS_FLUSH is nonzero
 *
 * The standard for iostreams is to call flush.  This is very costly for
 * SdFat.  Each call to flush causes 2048 bytes of I/O to the SD.
 *
 * SdFat has a single 512 byte buffer for SD I/O so it must write the current
 * data block to the SD, read the directory block from the SD, update the
 * directory entry, write the directory block to the SD and read the data
 * block back into the buffer.
 *
 * The SD flash memory controller is not designed for this many rewrites
 * so performance may be reduced by more than a factor of 100.
 *
 * If ENDL_CALLS_FLUSH is zero, you must call flush and/or close to force
 * all data to be written to the SD.
 */
#define ENDL_CALLS_FLUSH 0
//------------------------------------------------------------------------------
/**
 * SPI SCK divisor for SD initialization commands.
 * or greater
 */
#ifdef __AVR__
const uint8_t SPI_SCK_INIT_DIVISOR = 64;
#else
const uint8_t SPI_SCK_INIT_DIVISOR = 128;
#endif
//------------------------------------------------------------------------------
/**
 * Set USE_SEPARATE_FAT_CACHE nonzero to use a second 512 byte cache
 * for FAT table entries.  Improves performance for large writes that
 * are not a multiple of 512 bytes.
 */
#ifdef __arm__
#define USE_SEPARATE_FAT_CACHE 1
#else  // __arm__
#define USE_SEPARATE_FAT_CACHE 0
#endif  // __arm__
//------------------------------------------------------------------------------
/**
 * Set USE_MULTI_BLOCK_SD_IO nonzero to use multi-block SD read/write.
 *
 * Don't use mult-block read/write on small AVR boards.
 */
#if defined(RAMEND) && RAMEND < 3000
#define USE_MULTI_BLOCK_SD_IO 0
#else  // RAMEND
#define USE_MULTI_BLOCK_SD_IO 1
#endif  // RAMEND
#endif  // SdFatConfig_h

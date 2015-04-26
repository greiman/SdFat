/* Arduino SdSpi Library
 * Copyright (C) 2013 by William Greiman
 *
 * STM32F1 code for Maple and Maple Mini support, 2015 by Victor Perez
 *
 * This file is part of the Arduino SdSpi Library
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
 * along with the Arduino SdSpi Library.  If not, see
 * <http://www.gnu.org/licenses/>.
 */
 
 
#if defined (__PIC32MX__)

	#include "SdSpi.h"
	#include <DSPI.h>
	//May have to manually include DSPI.h in main sketch (unless using UECIDE)
	 
	#define SWAPENDIAN_32(x)            (uint32_t)((((x) & 0xFF000000UL) >> 24UL) | (((x) & 0x00FF0000UL) >> 8UL) | \
			                                               (((x) & 0x0000FF00UL) << 8UL)  | (((x) & 0x000000FFUL) << 24UL))

	//Use the default SPI port because SDFat begin()
	//  doesn't allow passing application-specific variables
	//This SHOULD be stored internally...
	DSPI *_spi=new DSPI0;

/*
		//------------------------------------------------------------------------------
		void SdSpi::chipSelectHigh() {
			//TODO: Needs to be implemented via direct register writes...
		}
		//------------------------------------------------------------------------------
		void SdSpi::chipSelectLow() {
			//TODO: Needs to be implemented via direct register writes...
		}
*/


		//------------------------------------------------------------------------------
		void SdSpi::begin() {
		  _spi->begin();
		  //TODO: Needs to instantiate the user-selected DSPI object (0-4)
		  //      Also needs to be passed the ChipSelect pin being used,
		  //          otherwise it uses Pin 10 by default.
		}
		//------------------------------------------------------------------------------
		void SdSpi::init(uint8_t divisor) {
			//_spi->setOrder does not exist
			_spi->setMode(DSPI_MODE0);
			_spi->setSpeed( F_CPU/(2*((uint32_t) divisor+1)) );	
		}
		//------------------------------------------------------------------------------
		uint8_t SdSpi::receive() {
			return _spi->transfer(0XFF);
		}
		//------------------------------------------------------------------------------
		uint8_t SdSpi::receive(uint8_t* buf, size_t n) {
			size_t m =(n >> 2);		
			if (m) {
				uint32_t* byteptr = (uint32_t*)buf;
				_spi->setTransferSize(DSPI_32BIT);	
				
				for (size_t i=0;i<m;i++) {

					*byteptr=_spi->transfer( (uint32_t) 0xFFFFFFFF );
					*byteptr=SWAPENDIAN_32(*byteptr);
					byteptr++;
				}
			
				m = n+1;
				n &= 0x3;
				_spi->setTransferSize(DSPI_8BIT);
			}
			if(n) {
				_spi->transfer(n,0xFF, &(buf[m]));
			}
			return 0;
		}
		//------------------------------------------------------------------------------
		void SdSpi::send(uint8_t data) {
			_spi->transfer(data);
		}
		//------------------------------------------------------------------------------
		void SdSpi::send(const uint8_t* buf , size_t n) {
			size_t m =(n >> 2);			
			if (m) {
				uint32_t* byteptr = (uint32_t*)buf;
				_spi->setTransferSize(DSPI_32BIT);	
				
				for (size_t i=0;i<m;i++) {
					_spi->transfer( SWAPENDIAN_32(*byteptr));
					byteptr++;
				}
			
				m = n+1;
				n &= 0x3;
				_spi->setTransferSize(DSPI_8BIT);
			}
			if(n) {
				_spi->transfer(n,(uint8_t*)&(buf[m]));
			}
			
		}
		
#endif  // __PIC32MX__


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
#if defined(__STM32F1__)
#include "SdSpi.h"
#include <SPI.h>
#include <libmaple/dma.h>
/** Use STM32 DMAC if nonzero */
#define USE_STM32F1_DMAC 1
/** Time in ms for DMA receive timeout */
#define STM32F1_DMA_TIMEOUT 100
/** DMAC receive channel */
#define SPI1_DMAC_RX_CH  DMA_CH2
/** DMAC transmit channel */
#define SPI1_DMAC_TX_CH  DMA_CH3

volatile bool SPI_DMA_TX_Active = false;
volatile bool SPI_DMA_RX_Active = false;

/** ISR for DMA TX event. */
inline void SPI_DMA_TX_Event() {
  SPI_DMA_TX_Active = false;
  dma_disable(DMA1, SPI_DMAC_TX_CH);
}

/** ISR for DMA RX event. */
inline void SPI_DMA_RX_Event() {
  SPI_DMA_RX_Active = false;
  dma_disable(DMA1, SPI1_DMAC_RX_CH);
}
//------------------------------------------------------------------------------

/** Disable DMA Channel. */
static void dmac_channel_disable(dma_channel ul_num) {
  dma_disable(DMA1, ul_num);
}
/** Enable DMA Channel. */
static void dmac_channel_enable(dma_channel ul_num) {
  dma_enable(DMA1, ul_num);
}
//------------------------------------------------------------------------------
void SdSpi::begin() {
  SPI.begin();
}
//------------------------------------------------------------------------------
// start RX DMA

static void spiDmaRX(uint8_t* dst, uint16_t count) {
//  spi_rx_dma_enable(SPI1);
  if (count < 1) return;
  dma_setup_transfer(DMA1, SPI1_DMAC_RX_CH, &SPI1->regs->DR, DMA_SIZE_8BITS,
                     dst, DMA_SIZE_8BITS, (DMA_MINC_MODE | DMA_TRNS_CMPLT));
  dma_set_num_transfers(DMA1, SPI1_DMAC_RX_CH, count);  // 2 bytes per pixel
  SPI_DMA_RX_Active = true;
  dma_enable(DMA1, SPI1_DMAC_RX_CH);
}
//------------------------------------------------------------------------------
// start TX DMA
static void spiDmaTX(const uint8_t* src, uint16_t count) {
  if (count < 1) return;
  static uint8_t ff = 0XFF;

  if (!src) {
    src = &ff;
    dma_setup_transfer(DMA1, SPI1_DMAC_TX_CH, &SPI1->regs->DR, DMA_SIZE_8BITS,
                       const_cast<uint8_t*>(src), DMA_SIZE_8BITS,
                      (DMA_FROM_MEM | DMA_TRNS_CMPLT));
  } else {
    dma_setup_transfer(DMA1, SPI1_DMAC_TX_CH, &SPI1->regs->DR, DMA_SIZE_8BITS,
                       const_cast<uint8_t*>(src), DMA_SIZE_8BITS,
                      (DMA_MINC_MODE  |  DMA_FROM_MEM | DMA_TRNS_CMPLT));
  }
  dma_set_num_transfers(DMA1, SPI1_DMAC_TX_CH, count);  // 2 bytes per pixel
  SPI_DMA_TX_Active = true;
  dma_enable(DMA1, SPI1_DMAC_TX_CH);
}
//------------------------------------------------------------------------------
//  initialize SPI controller STM32F1
void SdSpi::init(uint8_t sckDivisor) {
  if (sckDivisor < SPI_CLOCK_DIV2 || sckDivisor > SPI_CLOCK_DIV256) {
    sckDivisor = SPI_CLOCK_DIV2;  // may not be needed, testing.
  }
  SPI.setClockDivider(sckDivisor);
  SPI.setBitOrder(MSBFIRST);
  SPI.setDataMode(SPI_MODE0);

#if USE_STM32F1_DMAC
  dma_init(DMA1);
  dma_attach_interrupt(DMA1, SPI1_DMAC_TX_CH, SPI_DMA_TX_Event);
  dma_attach_interrupt(DMA1, SPI1_DMAC_RX_CH, SPI_DMA_RX_Event);
  spi_tx_dma_enable(SPI1);
  spi_rx_dma_enable(SPI1);
#endif  // USE_STM32F1_DMAC
}
//------------------------------------------------------------------------------
// STM32
static inline uint8_t spiTransfer(uint8_t b) {
  return SPI.transfer(b);
}
//------------------------------------------------------------------------------
// should be valid for STM32
/** SPI receive a byte */
uint8_t SdSpi::receive() {
  return spiTransfer(0xFF);
}
//------------------------------------------------------------------------------
/** SPI receive multiple bytes */
// check and finish.

uint8_t SdSpi::receive(uint8_t* buf, size_t n) {
  int rtn = 0;

#if USE_STM32F1_DMAC

  spiDmaRX(buf, n);
  spiDmaTX(0, n);

  uint32_t m = millis();
  while (SPI_DMA_RX_Active) {
    if ((millis() - m) > STM32F1_DMA_TIMEOUT)  {
      dmac_channel_disable(SPI_DMAC_RX_CH);
      dmac_channel_disable(SPI_DMAC_TX_CH);
      rtn = 2;
      break;
    }
  }

#else  // USE_STM32F1_DMAC
  for (size_t i = 0; i < n; i++) {
    buf[i] = SPI.transfer(0xFF);
  }
#endif  // USE_STM32F1_DMAC
  return rtn;
}
//------------------------------------------------------------------------------
/** SPI send a byte */
void SdSpi::send(uint8_t b) {
  spiTransfer(b);
}
//------------------------------------------------------------------------------
void SdSpi::send(const uint8_t* buf , size_t n) {
#if USE_STM32F1_DMAC
  spiDmaTX(buf, n);
  while (SPI_DMA_TX_Active) {}

#else  // #if USE_STM32F1_DMAC
  SPI.write(buf, n);
#endif  // #if USE_STM32F1_DMAC
  // leave RX register empty
  //  while (spi_is_rx_nonempty(SPI1))
  uint8_t b = spi_rx_reg(SPI1);
}
#endif  // USE_NATIVE_STM32F1_SPI

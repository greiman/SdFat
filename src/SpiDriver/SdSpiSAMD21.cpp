/**
 * Copyright (c) 20011-2017 Bill Greiman
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
#include "SdSpiDriver.h"
#if defined(__SAMD21G18A__)
/** Use SAMD21 DMAC if nonzero */
#define USE_SAMD21_DMAC 1
/** Time in ms for DMA receive timeout */
#define SAMD21_DMA_TIMEOUT 100
/** DMAC receive channel */
#define SPI_DMAC_RX_CH  1
/** DMAC transmit channel */
#define SPI_DMAC_TX_CH  0
//------------------------------------------------------------------------------
typedef struct {
	uint16_t btctrl;
	uint16_t btcnt;
	uint32_t srcaddr;
	uint32_t dstaddr;
	uint32_t descaddr;
} dmacdescriptor;
volatile dmacdescriptor wrb[12] __attribute__ ((aligned (16)));
dmacdescriptor descriptor_section[12] __attribute__ ((aligned (16)));
dmacdescriptor descriptor __attribute__ ((aligned (16)));
enum XfrType { DoTX, DoRX, DoTXRX };
static XfrType xtype;
static uint8_t rxsink[1], txsrc[1] = {0xff};
volatile uint32_t dmadone;
void DMAC_Handler() {
  uint8_t active_channel;
  __disable_irq();
  active_channel =  DMAC->INTPEND.reg & DMAC_INTPEND_ID_Msk;
  DMAC->CHID.reg = DMAC_CHID_ID(active_channel);
  dmadone = DMAC->CHINTFLAG.reg;
  DMAC->CHINTFLAG.reg = DMAC_CHINTENCLR_TCMPL;
  DMAC->CHINTFLAG.reg = DMAC_CHINTENCLR_TERR;
  DMAC->CHINTFLAG.reg = DMAC_CHINTENCLR_SUSP;
  __enable_irq();
}
//------------------------------------------------------------------------------
/** Disable DMA Controller. */
static void dmac_disable() {
  DMAC->CTRL.reg |= DMAC_CTRL_DMAENABLE;
}
/** Enable DMA Controller. */
static void dmac_enable() {
  DMAC->CTRL.reg = DMAC_CTRL_DMAENABLE | DMAC_CTRL_LVLEN(0xf);
}
/** Disable DMA Channel. */
static void dmac_channel_disable(uint32_t ul_num) {
  DMAC->CHID.reg = DMAC_CHID_ID(ul_num);
  DMAC->CHCTRLA.reg &= ~DMAC_CHCTRLA_ENABLE;
}
/** Enable DMA Channel. */
static void dmac_channel_enable(uint32_t ul_num) {
	DMAC->CHID.reg = DMAC_CHID_ID(ul_num);
  DMAC->CHCTRLA.reg |= DMAC_CHCTRLA_ENABLE;
}
//------------------------------------------------------------------------------
void SdSpiAltDriver::begin(uint8_t csPin) {
  m_csPin = csPin;
  pinMode(m_csPin, OUTPUT);
  digitalWrite(m_csPin, HIGH);
  SPI.begin();
#if USE_SAMD21_DMAC
  PM->AHBMASK.reg |= PM_AHBMASK_DMAC;
  PM->APBBMASK.reg |= PM_APBBMASK_DMAC;
  NVIC_EnableIRQ(DMAC_IRQn);
  DMAC->BASEADDR.reg = (uint32_t)descriptor_section;
  DMAC->WRBADDR.reg = (uint32_t)wrb;
#endif  // USE_SAMD21_DMAC
}
//------------------------------------------------------------------------------
Sercom *sercom = (Sercom   *)SERCOM4;  // SPI SERCOM
void spi_xfr(void *txdata, void *rxdata,  size_t n) {
  uint32_t temp_CHCTRLB_reg;
  DMAC->CHID.reg = DMAC_CHID_ID(SPI_DMAC_TX_CH); 
  DMAC->CHCTRLA.reg &= ~DMAC_CHCTRLA_ENABLE;
  DMAC->CHCTRLA.reg = DMAC_CHCTRLA_SWRST;
  DMAC->SWTRIGCTRL.reg &= (uint32_t)(~(1 << SPI_DMAC_TX_CH));
  temp_CHCTRLB_reg = DMAC_CHCTRLB_LVL(0) | 
  DMAC_CHCTRLB_TRIGSRC(SERCOM4_DMAC_ID_TX) | DMAC_CHCTRLB_TRIGACT_BEAT;
  DMAC->CHCTRLB.reg = temp_CHCTRLB_reg;
  DMAC->CHINTENSET.reg = DMAC_CHINTENSET_MASK;
  descriptor.descaddr = 0;
  descriptor.dstaddr = (uint32_t) &sercom->SPI.DATA.reg;
  descriptor.btcnt =  n;
  descriptor.srcaddr = (uint32_t)txdata;
  descriptor.btctrl =  DMAC_BTCTRL_VALID;
  if (xtype != DoRX) {
    descriptor.srcaddr += n;
    descriptor.btctrl |= DMAC_BTCTRL_SRCINC;
  }
  memcpy(&descriptor_section[SPI_DMAC_TX_CH],&descriptor, sizeof(dmacdescriptor));

  DMAC->CHID.reg = DMAC_CHID_ID(SPI_DMAC_RX_CH); 
  DMAC->CHCTRLA.reg &= ~DMAC_CHCTRLA_ENABLE;
  DMAC->CHCTRLA.reg = DMAC_CHCTRLA_SWRST;
  DMAC->SWTRIGCTRL.reg &= (uint32_t)(~(1 << SPI_DMAC_RX_CH));
  temp_CHCTRLB_reg = DMAC_CHCTRLB_LVL(0) | 
  DMAC_CHCTRLB_TRIGSRC(SERCOM4_DMAC_ID_RX) | DMAC_CHCTRLB_TRIGACT_BEAT;
  DMAC->CHCTRLB.reg = temp_CHCTRLB_reg;
  DMAC->CHINTENSET.reg = DMAC_CHINTENSET_MASK;
  dmadone = 0;
  descriptor.descaddr = 0;
  descriptor.srcaddr = (uint32_t) &sercom->SPI.DATA.reg;
  descriptor.btcnt =  n;
  descriptor.dstaddr = (uint32_t)rxdata;
  descriptor.btctrl =  DMAC_BTCTRL_VALID;
  if (xtype != DoTX) {
    descriptor.dstaddr += n;
    descriptor.btctrl |= DMAC_BTCTRL_DSTINC;
  }
  memcpy(&descriptor_section[SPI_DMAC_RX_CH],&descriptor, sizeof(dmacdescriptor));

  DMAC->CHID.reg = DMAC_CHID_ID(SPI_DMAC_TX_CH);
  DMAC->CHCTRLA.reg |= DMAC_CHCTRLA_ENABLE;
  DMAC->CHID.reg = DMAC_CHID_ID(SPI_DMAC_RX_CH);
  DMAC->CHCTRLA.reg |= DMAC_CHCTRLA_ENABLE;

  dmac_channel_enable(SPI_DMAC_TX_CH);
  dmac_channel_enable(SPI_DMAC_RX_CH);
}
//------------------------------------------------------------------------------
// start RX DMA
static void spiDmaRX(uint8_t* dst, size_t n) {
	xtype = DoRX;
  spi_xfr(txsrc,&dst,n);
}
//------------------------------------------------------------------------------
// start TX DMA
static void spiDmaTX(const uint8_t* src, size_t n) {
  xtype = DoTX;
  spi_xfr(&src,rxsink,n);
}
//------------------------------------------------------------------------------
// start TXRX DMA
static void spiDmaTXRX(const uint8_t* src, uint8_t* dst, size_t n) {
  xtype = DoTXRX;
  spi_xfr(&src,&dst,n);
}
//------------------------------------------------------------------------------
//  initialize SPI controller
void SdSpiAltDriver::activate() {
  SPI.beginTransaction(m_spiSettings);
}
//------------------------------------------------------------------------------
void SdSpiAltDriver::deactivate() {
  SPI.endTransaction();
}
//------------------------------------------------------------------------------
/** SPI receive a byte */
uint8_t SdSpiAltDriver::receive() {
  return SPI.transfer(0XFF);
}
//------------------------------------------------------------------------------
/** SPI receive multiple bytes */
uint8_t SdSpiAltDriver::receive(uint8_t* buf, size_t n) {
  int rtn = 0;
#if USE_SAMD21_DMAC
  spiDmaRX(buf, n);
  spiDmaTX(0, n);

  uint32_t m = millis();
  while (!dmadone) {
    if ((millis() - m) > SAMD21_DMA_TIMEOUT)  {
      rtn = 2;
      break;
    }
  }
  dmac_channel_disable(SPI_DMAC_RX_CH);
  dmac_channel_disable(SPI_DMAC_TX_CH);
#else  // USE_SAMD21_DMAC
  for (size_t i = 0; i < n; i++) {
    buf[i] = SPI.transfer(0XFF);
  }
#endif  // USE_SAMD21_DMAC
  return rtn;
}
//------------------------------------------------------------------------------
/** SPI send a byte */
void SdSpiAltDriver::send(uint8_t b) {
  SPI.transfer(b);
}
//------------------------------------------------------------------------------
void SdSpiAltDriver::send(const uint8_t* buf , size_t n) {
#if USE_SAMD21_DMAC
  spiDmaTX(buf, n);
  while (!dmadone) {}
  dmac_channel_disable(SPI_DMAC_RX_CH);
  dmac_channel_disable(SPI_DMAC_TX_CH);
#else  // #if USE_SAMD21_DMAC
  for (size_t i = 0; i < n; i++) {
    SPI.transfer(buf[i]);
  }
#endif  // #if USE_SAMD21_DMAC
}
#endif  // defined(__SAMD21G18A__)

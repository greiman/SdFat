/**
 * Copyright (c) 2011-2024 Bill Greiman
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
#ifdef ARDUINO_ARCH_RP2040
#define DEBUG_FILE "PioSdioCard.cpp"
#include "../SdCardInfo.h"
#include "../SdioCard.h"
#include "PioDbgInfo.h"
#include "PioSdioCard.pio.h"
//------------------------------------------------------------------------------
// Do not enable - not implemented
#define HIGH_SPEED_MODE 0  // non-zero to debug High Speed Mode
// USE_DEBUG_MODE 0 - no debug, 1 - print message, 2 - Use scope/analyzer.
#define USE_DEBUG_MODE 0

const uint PIO_CLK_DIV_INIT = 2;

const uint PIO_CLK_DIV_RUN = 1;

const uint PIN_SDIO_UNDEFINED = 31u;

const uint DAT_FIFO_DEPTH = 8;
const uint CMD0_RETRIES = 10;
const uint CMD8_RETRIES = 3;

//==============================================================================
// Command definitions.
enum { RSP_R0 = 0, RSP_R1 = 1, RSP_R2 = 2, RSP_R3 = 3, RSP_R6 = 6, RSP_R7 = 7 };

class CmdRsp_t {
 public:
  CmdRsp_t(uint8_t idx_, uint8_t rsp_) : idx(idx_), rsp(rsp_) {}
  uint8_t idx;
  uint8_t rsp;
};

static const CmdRsp_t CMD0_R0(CMD0, RSP_R0);
static const CmdRsp_t CMD2_R2(CMD2, RSP_R2);
static const CmdRsp_t CMD3_R6(CMD3, RSP_R6);
static const CmdRsp_t CMD6_R1(CMD6, RSP_R1);
static const CmdRsp_t CMD7_R1(CMD7, RSP_R1);
static const CmdRsp_t CMD8_R7(CMD8, RSP_R7);
static const CmdRsp_t CMD9_R2(CMD9, RSP_R2);
static const CmdRsp_t CMD10_R2(CMD10, RSP_R2);
static const CmdRsp_t CMD12_R1(CMD12, RSP_R1);
static const CmdRsp_t CMD13_R1(CMD13, RSP_R1);
static const CmdRsp_t CMD18_R1(CMD18, RSP_R1);
static const CmdRsp_t CMD25_R1(CMD25, RSP_R1);
static const CmdRsp_t CMD32_R1(CMD32, RSP_R1);
static const CmdRsp_t CMD33_R1(CMD33, RSP_R1);
static const CmdRsp_t CMD38_R1(CMD38, RSP_R1);
static const CmdRsp_t CMD55_R1(CMD55, RSP_R1);
static const CmdRsp_t ACMD6_R1(ACMD6, RSP_R1);
static const CmdRsp_t ACMD13_R1(ACMD13, RSP_R1);
static const CmdRsp_t ACMD41_R3(ACMD41, RSP_R3);
static const CmdRsp_t ACMD51_R1(ACMD51, RSP_R1);
//==============================================================================
// Global variables.
static float g_clkDiv = 0;
static uint g_clkPin = PIN_SDIO_UNDEFINED;
static uint g_cmdPin = PIN_SDIO_UNDEFINED;
static uint g_dat0Pin = PIN_SDIO_UNDEFINED;
static uint g_cardRsp;
static uint g_errorCode;
static uint g_errorLine;
static bool g_highCapacity;
static bool g_initDone = false;
static uint g_ocr;
static bool g_version2;
static PIO g_pio = nullptr;
static int g_sm0 = -1;
static int g_sm1 = -1;
static int g_cmdRspOffset = -1;
static pio_sm_config g_cmdConfig;
static int g_rdDataOffset = -1;
static pio_sm_config g_rdDataConfig;
static int g_rdClkOffset = -1;
static pio_sm_config g_rdClkConfig;
static int g_wrDataOffset = -1;
static pio_sm_config g_wrDataConfig;
static int g_wrRespOffset = -1;
static pio_sm_config g_wrRespConfig;
static uint g_rca;
static cid_t g_cid;
static csd_t g_csd;
static scr_t g_scr;
static sds_t g_sds;
//==============================================================================
class Timeout {
 public:
  explicit Timeout(uint ms) : _usStart(0), _usTimeout(1000 * ms) {}
  bool timedOut() {
    if (_usStart) {
      return (usSinceBoot() - _usStart) > _usTimeout;
    }
    _usStart = usSinceBoot();
    return false;
  }
  uint32_t usSinceBoot() { return to_us_since_boot(get_absolute_time()); }
  uint32_t _usStart;
  uint32_t _usTimeout;
};
//==============================================================================
// Error function and macro.
#define PIO_ERROR_ADD_PROGRAM 99
#if USE_DEBUG_MODE
#define SDIO_FAIL() logmsgln(DEBUG_FILE, '.', __LINE__, F("SDIO_FAIL"))
#define sdError(code) setSdErrorCode(code, __LINE__, #code)
static void setSdErrorCode(uint8_t code, uint32_t line, const char* txt) {
  g_errorCode = code;
  g_errorLine = line;
  logmsgln(F(DEBUG_FILE), '.', line, ' ', txt);
}
#else  // USE_DEBUG_MODE
#define SDIO_FAIL()
#define sdError(code) setSdErrorCode(code, __LINE__)
static inline void setSdErrorCode(uint8_t code, uint32_t line) {
  g_errorCode = code;
  g_errorLine = line;
}
#endif  // USE_DEBUG_MODE
//==============================================================================
// CRC functions.
//------------------------------------------------------------------------------
// See this library's extras folder.
static const uint8_t crc7_table[256] = {
    0x00, 0x12, 0x24, 0x36, 0x48, 0x5a, 0x6c, 0x7e,  // 00 - 07
    0x90, 0x82, 0xb4, 0xa6, 0xd8, 0xca, 0xfc, 0xee,  // 08 - 0f
    0x32, 0x20, 0x16, 0x04, 0x7a, 0x68, 0x5e, 0x4c,  // 10 - 17
    0xa2, 0xb0, 0x86, 0x94, 0xea, 0xf8, 0xce, 0xdc,  // 18 - 1f
    0x64, 0x76, 0x40, 0x52, 0x2c, 0x3e, 0x08, 0x1a,  // 20 - 27
    0xf4, 0xe6, 0xd0, 0xc2, 0xbc, 0xae, 0x98, 0x8a,  // 28 - 2f
    0x56, 0x44, 0x72, 0x60, 0x1e, 0x0c, 0x3a, 0x28,  // 30 - 37
    0xc6, 0xd4, 0xe2, 0xf0, 0x8e, 0x9c, 0xaa, 0xb8,  // 38 - 3f
    0xc8, 0xda, 0xec, 0xfe, 0x80, 0x92, 0xa4, 0xb6,  // 40 - 47
    0x58, 0x4a, 0x7c, 0x6e, 0x10, 0x02, 0x34, 0x26,  // 48 - 4f
    0xfa, 0xe8, 0xde, 0xcc, 0xb2, 0xa0, 0x96, 0x84,  // 50 - 57
    0x6a, 0x78, 0x4e, 0x5c, 0x22, 0x30, 0x06, 0x14,  // 58 - 5f
    0xac, 0xbe, 0x88, 0x9a, 0xe4, 0xf6, 0xc0, 0xd2,  // 60 - 67
    0x3c, 0x2e, 0x18, 0x0a, 0x74, 0x66, 0x50, 0x42,  // 68 - 6f
    0x9e, 0x8c, 0xba, 0xa8, 0xd6, 0xc4, 0xf2, 0xe0,  // 70 - 77
    0x0e, 0x1c, 0x2a, 0x38, 0x46, 0x54, 0x62, 0x70,  // 78 - 7f
    0x82, 0x90, 0xa6, 0xb4, 0xca, 0xd8, 0xee, 0xfc,  // 80 - 87
    0x12, 0x00, 0x36, 0x24, 0x5a, 0x48, 0x7e, 0x6c,  // 88 - 8f
    0xb0, 0xa2, 0x94, 0x86, 0xf8, 0xea, 0xdc, 0xce,  // 90 - 97
    0x20, 0x32, 0x04, 0x16, 0x68, 0x7a, 0x4c, 0x5e,  // 98 - 9f
    0xe6, 0xf4, 0xc2, 0xd0, 0xae, 0xbc, 0x8a, 0x98,  // a0 - a7
    0x76, 0x64, 0x52, 0x40, 0x3e, 0x2c, 0x1a, 0x08,  // a8 - af
    0xd4, 0xc6, 0xf0, 0xe2, 0x9c, 0x8e, 0xb8, 0xaa,  // b0 - b7
    0x44, 0x56, 0x60, 0x72, 0x0c, 0x1e, 0x28, 0x3a,  // b8 - bf
    0x4a, 0x58, 0x6e, 0x7c, 0x02, 0x10, 0x26, 0x34,  // c0 - c7
    0xda, 0xc8, 0xfe, 0xec, 0x92, 0x80, 0xb6, 0xa4,  // c8 - cf
    0x78, 0x6a, 0x5c, 0x4e, 0x30, 0x22, 0x14, 0x06,  // d0 - d7
    0xe8, 0xfa, 0xcc, 0xde, 0xa0, 0xb2, 0x84, 0x96,  // d8 - df
    0x2e, 0x3c, 0x0a, 0x18, 0x66, 0x74, 0x42, 0x50,  // e0 - e7
    0xbe, 0xac, 0x9a, 0x88, 0xf6, 0xe4, 0xd2, 0xc0,  // e8 - ef
    0x1c, 0x0e, 0x38, 0x2a, 0x54, 0x46, 0x70, 0x62,  // f0 - f7
    0x8c, 0x9e, 0xa8, 0xba, 0xc4, 0xd6, 0xe0, 0xf2   // f8 - ff
};
//------------------------------------------------------------------------------
inline static uint8_t CRC7(const uint8_t* data, uint8_t n) {
  uint8_t crc = 0;
  for (uint8_t i = 0; i < n; i++) {
    crc = crc7_table[crc ^ data[i]];
  }
  return crc | 1;
}
//------------------------------------------------------------------------------
// Modified from sdio_crc16_4bit_checksum() in
// https://github.com/ZuluSCSI/ZuluSCSI-firmware
//
static inline __attribute__((always_inline)) uint64_t crc16(uint64_t crc,
                                                            uint32_t data_in) {
  // Shift out 8 bits for each line
  uint32_t data_out = crc >> 32;
  crc <<= 32;

  // XOR outgoing data to itself with 4 bit delay
  data_out ^= (data_out >> 16);

  // XOR incoming data to outgoing data with 4 bit delay
  data_out ^= (data_in >> 16);

  // XOR outgoing and incoming data to accumulator at each tap
  uint64_t xorred = data_out ^ data_in;
  crc ^= xorred;
  crc ^= xorred << (5 * 4);
  crc ^= xorred << (12 * 4);
  return crc;
}
//==============================================================================
static void pioConfig(float clkDiv) {
  g_cmdConfig =
      pio_cmd_rsp_program_config(g_cmdRspOffset, g_cmdPin, g_clkPin, clkDiv);
  g_rdClkConfig =
      pio_rd_clk_program_config(g_rdClkOffset, g_dat0Pin, g_clkPin, clkDiv);
  g_rdDataConfig =
      pio_rd_data_program_config(g_rdDataOffset, g_dat0Pin, clkDiv);
  g_wrDataConfig =
      pio_wr_data_program_config(g_wrDataOffset, g_dat0Pin, g_clkPin, clkDiv);
  g_wrRespConfig =
      pio_wr_resp_program_config(g_wrRespOffset, g_dat0Pin, g_clkPin, clkDiv);
}
//------------------------------------------------------------------------------
static void pioEnd() {
  if (!g_pio) {
    return;
  }
  if (g_sm0 >= 0) {
    pio_sm_unclaim(g_pio, g_sm0);
    g_sm0 = -1;
  }
  if (g_sm1 >= 0) {
    pio_sm_unclaim(g_pio, g_sm1);
    g_sm1 = -1;
  }
  if (g_cmdRspOffset >= 0) {
    pio_remove_program(g_pio, &cmd_rsp_program, g_cmdRspOffset);
    g_cmdRspOffset = -1;
  }
  if (g_rdClkOffset >= 0) {
    pio_remove_program(g_pio, &rd_clk_program, g_rdClkOffset);
    g_rdClkOffset = -1;
  }
  if (g_rdDataOffset >= 0) {
    pio_remove_program(g_pio, &rd_data_program, g_rdDataOffset);
    g_rdDataOffset = -1;
  }
  if (g_wrDataOffset >= 0) {
    pio_remove_program(g_pio, &wr_data_program, g_wrDataOffset);
    g_wrDataOffset = -1;
  }
  if (g_wrRespOffset >= 0) {
    pio_remove_program(g_pio, &wr_resp_program, g_wrRespOffset);
    g_wrRespOffset = -1;
  }
}
//------------------------------------------------------------------------------
static bool pioInit() {
  uint pin[] = {g_clkPin,      g_cmdPin,      g_dat0Pin,
                g_dat0Pin + 1, g_dat0Pin + 2, g_dat0Pin + 3};
  if (g_wrRespOffset < 0) {
    uint16_t patched_inst[rd_data_program.length];  // NOLINT
    struct pio_program tmp_program;
    tmp_program.instructions = nullptr;
    tmp_program.length = cmd_rsp_program.length + rd_data_program.length +
                         rd_clk_program.length + wr_data_program.length +
                         wr_resp_program.length;
    tmp_program.origin = -1;
    if (pio_can_add_program(pio0, &tmp_program)) {
      g_pio = pio0;
    } else if (pio_can_add_program(pio1, &tmp_program)) {
      g_pio = pio1;
#if NUM_PIOS > 2
    } else if (pio_can_add_program(pio2, &tmp_program)) {
      g_pio = pio2;
#endif
    } else {
      sdError(PIO_ERROR_ADD_PROGRAM);
      goto fail;
    }
    g_sm0 = pio_claim_unused_sm(g_pio, false);
    if (g_sm0 < 0) {
      sdError(PIO_ERROR_ADD_PROGRAM);
      goto fail;
    }
    g_sm1 = pio_claim_unused_sm(g_pio, false);
    if (g_sm0 < 0) {
      sdError(PIO_ERROR_ADD_PROGRAM);
      goto fail;
    }
    g_cmdRspOffset = pio_add_program(g_pio, &cmd_rsp_program);
    if (g_cmdRspOffset < 0) {
      sdError(PIO_ERROR_ADD_PROGRAM);
      goto fail;
    }
    rd_data_patch_program(&tmp_program, patched_inst, g_clkPin);
    g_rdDataOffset = pio_add_program(g_pio, &tmp_program);
    if (g_rdDataOffset < 0) {
      sdError(PIO_ERROR_ADD_PROGRAM);
      goto fail;
    }
    g_rdClkOffset = pio_add_program(g_pio, &rd_clk_program);
    if (g_rdClkOffset < 0) {
      sdError(PIO_ERROR_ADD_PROGRAM);
      goto fail;
    }
    g_wrDataOffset = pio_add_program(g_pio, &wr_data_program);
    if (g_wrDataOffset < 0) {
      sdError(PIO_ERROR_ADD_PROGRAM);
      goto fail;
    }
    g_wrRespOffset = pio_add_program(g_pio, &wr_resp_program);
    if (g_wrRespOffset < 0) {
      sdError(PIO_ERROR_ADD_PROGRAM);
      goto fail;
    }
  }
  for (uint i = 0; i < 6U; i++) {
    gpio_pull_up(pin[i]);
  }
  gpio_set_drive_strength(g_clkPin, GPIO_DRIVE_STRENGTH_8MA);
  gpio_set_slew_rate(g_clkPin, GPIO_SLEW_RATE_FAST);
  for (uint i = 0; i < 6U; i++) {
    pio_gpio_init(g_pio, pin[i]);
  }

  g_pio->input_sync_bypass |=
      (1 << g_clkPin) | (1 << g_cmdPin) | (0XF << g_dat0Pin);
  pio_sm_set_consecutive_pindirs(g_pio, g_sm0, g_clkPin, 1, true);
  pio_sm_set_consecutive_pindirs(g_pio, g_sm0, g_cmdPin, 1, true);
  pio_sm_set_consecutive_pindirs(g_pio, g_sm0, g_dat0Pin, 4, false);
  return true;

fail:
  pioEnd();
  return false;
}
//------------------------------------------------------------------------------
static bool cardCmd(CmdRsp_t cmd, uint32_t arg, void* rsp = nullptr) {
  uint8_t buf[6];
  uint nRsp = cmd.rsp == RSP_R0 ? 0 : cmd.rsp == RSP_R2 ? 17 : 6;
  io_ro_8* rxFifo = reinterpret_cast<io_ro_8*>(&g_pio->rxf[g_sm0]);
  io_wo_8* txFifo = reinterpret_cast<io_wo_8*>(&g_pio->txf[g_sm0]);
  pio_sm_set_enabled(g_pio, g_sm1, false);
  pio_sm_init(g_pio, g_sm0, g_cmdRspOffset, &g_cmdConfig);
  pio_sm_exec(g_pio, g_sm0, pio_encode_set(pio_pindirs, 1));
  *txFifo = 55;
  pio_sm_exec(g_pio, g_sm0, pio_encode_out(pio_x, 8));
  *txFifo = nRsp ? 8 * nRsp - 1 : 0;
  pio_sm_exec(g_pio, g_sm0, pio_encode_out(pio_y, 8));
  pio_sm_set_enabled(g_pio, g_sm0, true);
  uint n = 0;
  buf[n++] = (uint8_t)(cmd.idx | 0x40);
  buf[n++] = (uint8_t)(arg >> 24U);
  buf[n++] = (uint8_t)(arg >> 16U);
  buf[n++] = (uint8_t)(arg >> 8U);
  buf[n++] = (uint8_t)arg;
  buf[n++] = CRC7(buf, 5);
  *txFifo = 0XFF;
  for (uint i = 0; i < n; i++) {
    while (pio_sm_is_tx_fifo_full(g_pio, g_sm0)) {
    }
    *txFifo = buf[i];
  }

  Timeout timeout(SD_CMD_TIMEOUT);
  if (!nRsp) {
    uint32_t fdebug_tx_stall = 1u << (PIO_FDEBUG_TXSTALL_LSB + g_sm0);
    g_pio->fdebug = fdebug_tx_stall;
    while (!(g_pio->fdebug & fdebug_tx_stall)) {
      if (timeout.timedOut()) {
        sdError(SD_CARD_ERROR_CMD0);
        goto fail;
      }
    }
    goto done;
  }
  uint8_t rtn[20];

  for (uint i = 0; i < nRsp; i++) {
    while (pio_sm_is_rx_fifo_empty(g_pio, g_sm0)) {
      if (timeout.timedOut()) {
        sdError(SD_CARD_ERROR_READ_TIMEOUT);
        goto fail;
      }
    }
    rtn[i] = *rxFifo;
  }
  if (cmd.rsp == RSP_R3) {
    if (rtn[0] != 0X3F || rtn[5] != 0XFF) {
      sdError(SD_CARD_ERROR_ACMD41);
      goto fail;
    }
  } else {
    uint8_t crc;
    if (cmd.rsp == RSP_R2) {
      crc = CRC7(rtn + 1, nRsp - 2);
    } else {
      crc = CRC7(rtn, nRsp - 1);
    }
    if (rtn[nRsp - 1] != crc) {
#if USE_DEBUG_MODE
      Serial.printf("CHK: %02X, CRC: %02X\n", rtn[nRsp - 1], crc);
      for (uint i = 0; i < nRsp; i++) {
        Serial.printf(" %02X", rtn[i]);
      }
      Serial.println();
#endif  // USE_DEBUG_MODE
      sdError(SD_CARD_ERROR_READ_CRC);
      goto fail;
    }
  }
  if (nRsp == 6) {
    g_cardRsp = (rtn[1] << 24) | (rtn[2] << 16) | (rtn[3] << 8) | rtn[4];
    if (rsp) {
      *reinterpret_cast<uint32_t*>(rsp) = g_cardRsp;
    }
  } else if (rsp && nRsp == 17) {
    memcpy(rsp, rtn + 1, 16);
  }

done:
  pio_sm_set_enabled(g_pio, g_sm0, false);
  return true;

fail:
#if USE_DEBUG_MODE
  Serial.printf("CMD%d failed\n", cmd.idx);
#endif  // USE_DEBUG_MODE
  pio_sm_set_enabled(g_pio, g_sm0, false);
  return false;
}
//------------------------------------------------------------------------------
static bool cardAcmd(uint32_t rca, CmdRsp_t cmdRsp, uint32_t arg) {
  return cardCmd(CMD55_R1, rca) && cardCmd(cmdRsp, arg);
}
//------------------------------------------------------------------------------
static bool __time_critical_func(readDat)(void* dst, uint n8) {
  uint32_t buf[128];
  uint n32 = n8 / 4;
  uint nr = n32 + 2;
  const uint mask = (1ul << g_sm0) | (1ul << g_sm1);
  io_wo_8* txFifo = reinterpret_cast<io_wo_8*>(&g_pio->txf[g_sm1]);
  pio_sm_init(g_pio, g_sm0, g_rdDataOffset, &g_rdDataConfig);
  pio_sm_init(g_pio, g_sm1, g_rdClkOffset, &g_rdClkConfig);
  pio_set_sm_mask_enabled(g_pio, mask, true);

  uint nf = nr < DAT_FIFO_DEPTH ? nr : DAT_FIFO_DEPTH;
  for (uint it = 0; it < nf; it++) {
    *txFifo = 0XFF;
  }
  io_ro_32* rxFifo = reinterpret_cast<io_ro_32*>(&g_pio->rxf[g_sm0]);
  uint32_t* dst32 = (uint)dst & 3 ? buf : reinterpret_cast<uint32_t*>(dst);
  uint64_t crc = 0;
  uint64_t chk = 0;
  Timeout timeout(SD_READ_TIMEOUT);
  uint ir = 0;
  if (nf < nr) {
    uint nb = nr - nf;
    while (true) {
      while (pio_sm_get_rx_fifo_level(g_pio, g_sm0) < 4) {
        if (timeout.timedOut()) {
          sdError(SD_CARD_ERROR_READ_TIMEOUT);
          goto fail;
        }
      }
      uint32_t tmp = *rxFifo;
      *txFifo = 0XFF;
      dst32[ir++] = __builtin_bswap32(tmp);
      crc = crc16(crc, tmp);
      tmp = *rxFifo;
      *txFifo = 0XFF;
      dst32[ir++] = __builtin_bswap32(tmp);
      crc = crc16(crc, tmp);
      if (ir == nb) {
        break;
      }
      tmp = *rxFifo;
      *txFifo = 0XFF;
      dst32[ir++] = __builtin_bswap32(tmp);
      crc = crc16(crc, tmp);
      tmp = *rxFifo;
      *txFifo = 0XFF;
      dst32[ir++] = __builtin_bswap32(tmp);
      crc = crc16(crc, tmp);
    }
  }
  for (; ir < nr; ir++) {
    while (pio_sm_is_rx_fifo_empty(g_pio, g_sm0)) {
      if (timeout.timedOut()) {
        sdError(SD_CARD_ERROR_READ_TIMEOUT);
        goto fail;
      }
    }
    uint32_t tmp = *rxFifo;
    if (ir < n32) {
      dst32[ir] = __builtin_bswap32(tmp);
      crc = crc16(crc, tmp);
    } else {
      chk <<= 32;
      chk |= tmp;
    }
  }
  if (crc != chk) {
#if USE_DEBUG_MODE
    Serial.printf("crc: %llX\r\nchk: %llX\r\n", crc, chk);
#endif  // USE_DEBUG_MODE
    sdError(SD_CARD_ERROR_READ_CRC);
    goto fail;
  }
  pio_set_sm_mask_enabled(g_pio, mask, false);
  if (dst32 == buf) {
    memcpy(dst, buf, n8);
  }
  return true;

fail:
  pio_set_sm_mask_enabled(g_pio, mask, false);
  return false;
}
//------------------------------------------------------------------------------
static bool __time_critical_func(writeDat)(const uint8_t* src) {
  const uint32_t* src32;
  uint32_t buf[128];
  if ((uint)src & 3) {
    memcpy(buf, src, 512);
    src32 = (const uint32_t*)buf;
  } else {
    src32 = (const uint32_t*)src;
  }
  uint32_t tmp;
  io_wo_32* txFifo = reinterpret_cast<io_wo_32*>(&g_pio->txf[g_sm0]);
  io_ro_32* rxFifo = reinterpret_cast<io_ro_32*>(&g_pio->rxf[g_sm1]);
  uint8_t rsp;
  uint mask = (1ul << g_sm0) | (1ul << g_sm1);
  uint64_t crc = 0;

  Timeout timeout(SD_WRITE_TIMEOUT);
  while (!gpio_get(g_dat0Pin)) {
    if (timeout.timedOut()) {
      sdError(SD_CARD_ERROR_WRITE_TIMEOUT);
      goto fail;
    }
  }
  pio_sm_init(g_pio, g_sm0, g_wrDataOffset, &g_wrDataConfig);
  pio_sm_init(g_pio, g_sm1, g_wrRespOffset, &g_wrRespConfig);
  *txFifo = 1048;  // 8 + 1024 + 16 + 1 - 1;
  pio_sm_exec(g_pio, g_sm0, pio_encode_out(pio_x, 32));
  pio_sm_exec(g_pio, g_sm0, pio_encode_set(pio_pindirs, 0XF));
  *txFifo = 0xFFFFFFF0;
  pio_set_sm_mask_enabled(g_pio, mask, true);
  for (int i = 0; i < 128;) {
    while (pio_sm_get_tx_fifo_level(g_pio, g_sm0) > 4) {
      if (timeout.timedOut()) {
        sdError(SD_CARD_ERROR_WRITE_FIFO);
        goto fail;
      }
    }
    tmp = __builtin_bswap32(src32[i++]);
    crc = crc16(crc, tmp);
    *txFifo = tmp;
    tmp = __builtin_bswap32(src32[i++]);
    crc = crc16(crc, tmp);
    *txFifo = tmp;
    tmp = __builtin_bswap32(src32[i++]);
    crc = crc16(crc, tmp);
    *txFifo = tmp;
    tmp = __builtin_bswap32(src32[i++]);
    crc = crc16(crc, tmp);
    *txFifo = tmp;
  }
  while (pio_sm_get_tx_fifo_level(g_pio, g_sm0) > 5) {
    if (timeout.timedOut()) {
      sdError(SD_CARD_ERROR_WRITE_FIFO);
      goto fail;
    }
  }
  *txFifo = (uint32_t)(crc >> 32);
  *txFifo = (uint32_t)crc;
  *txFifo = 0xFFFFFFFF;

  while (pio_sm_is_rx_fifo_empty(g_pio, g_sm1)) {
    if (timeout.timedOut()) {
      sdError(SD_CARD_ERROR_READ_FIFO);
      goto fail;
    }
  }
  rsp = *rxFifo;
  if ((rsp & 0X1F) != 0b101) {
#if USE_DEBUG_MODE
    Serial.printf("wr rsp: %02X\n", rsp);
#endif  // USE_DEBUG_MODE
    sdError(SD_CARD_ERROR_WRITE_DATA);
    goto fail;
  }
  return true;
fail:
  pio_set_sm_mask_enabled(g_pio, mask, false);
  return false;
}
//==============================================================================
// add to SdioCard class int the future.
// SdioCard::SdioCard()
// SdioCard::~SdioCard()
//------------------------------------------------------------------------------
bool SdioCard::begin(SdioConfig sdioConfig) {
  uint32_t arg;
  m_curState = IDLE_STATE;
  g_errorCode = SD_CARD_ERROR_NONE;
  g_highCapacity = false;
  g_initDone = false;
  g_version2 = false;
  g_clkDiv = PIO_CLK_DIV_INIT;
  g_clkPin = sdioConfig.clkPin();
  g_cmdPin = sdioConfig.cmdPin();
  g_dat0Pin = sdioConfig.dat0Pin();
  pioInit();
  pioConfig(g_clkDiv);
#if USE_DEBUG_MODE == 2
  Serial.println();
  pioRegs(g_pio);
  pioSmRegs(g_pio, g_sm0);
  pioSmRegs(g_pio, g_sm1);
  gpioStatus(g_clkPin);
  gpioStatus(g_cmdPin);
  while (Serial.read() >= 0) {
  }
  Serial.println("Logic Analyzer on");
  while (!Serial.available()) {
  }
#endif  // USE_DEBUG_MODE
  Timeout timeout(SD_INIT_TIMEOUT);
  for (uint i = 0; i < CMD0_RETRIES; i++) {
    if (!cardCmd(CMD0_R0, 0)) {
      sdError(SD_CARD_ERROR_CMD0);
      goto fail;
    }
  }
  // Try several times for case of reset delay.
  for (uint32_t i = 0; i < CMD8_RETRIES; i++) {
    if (cardCmd(CMD8_R7, 0X1AA)) {
      if (g_cardRsp != 0X1AA) {
        sdError(SD_CARD_ERROR_CMD8);
        goto fail;
      }
      g_version2 = true;
      break;
    }
    g_errorCode = SD_CARD_ERROR_NONE;
  }
  arg = g_version2 ? 0X40300000 : 0x00300000;
  while (true) {
    if (!cardAcmd(0, ACMD41_R3, arg)) {
      sdError(SD_CARD_ERROR_ACMD41);
      goto fail;
    }
    if (g_cardRsp & 0x80000000) {
      break;
    }
    if (timeout.timedOut()) {
      sdError(SD_CARD_ERROR_ACMD41);
      goto fail;
    }
  }
  g_ocr = g_cardRsp;
  if (g_cardRsp & 0x40000000) {
    // Is high capacity.
    g_highCapacity = true;
  }
  if (!cardCmd(CMD2_R2, 0)) {
    sdError(SD_CARD_ERROR_CMD2);
    goto fail;
  }
  if (!cardCmd(CMD3_R6, 0)) {
    sdError(SD_CARD_ERROR_CMD3);
    goto fail;
  }
  g_rca = g_cardRsp & 0xFFFF0000;
  if (!cardCmd(CMD9_R2, g_rca, &g_csd)) {
    sdError(SD_CARD_ERROR_CMD9);
    goto fail;
  }
  if (!cardCmd(CMD10_R2, g_rca, &g_cid)) {
    sdError(SD_CARD_ERROR_CMD10);
    goto fail;
  }
  if (!cardCmd(CMD7_R1, g_rca)) {
    sdError(SD_CARD_ERROR_CMD7);
    goto fail;
  }

  if (!cardAcmd(g_rca, ACMD6_R1, 2)) {
    sdError(SD_CARD_ERROR_ACMD6);
    goto fail;
  }
  if (!cardAcmd(g_rca, ACMD51_R1, 0) || !readDat(&g_scr, sizeof(g_scr))) {
    sdError(SD_CARD_ERROR_ACMD51);
    goto fail;
  }
  if (!cardAcmd(g_rca, ACMD13_R1, 0) || !readDat(&g_sds, sizeof(g_sds))) {
    sdError(SD_CARD_ERROR_ACMD13);
    goto fail;
  }
#if HIGH_SPEED_MODE
  // Determine if High Speed mode is supported and set frequency.
  // Check status[16] for error 0XF or status[16] for new mode 0X1.
  uint8_t status[64];
  if (g_scr.sdSpec() > 0 && cardCMD6(0X00FFFFFF, status) && (2 & status[13]) &&
      cardCMD6(0X80FFFFF1, status) && (status[16] & 0XF) == 1) {
    //   kHzSdClk = 50000;
    Serial.println("High Speed Mode");
    goto fail;
  } else {
    //   kHzSdClk = 25000;
    Serial.println("Default Speed Mode");
  }
#endif  // HIGH_SPEED_MODE
  g_clkDiv = PIO_CLK_DIV_RUN;
  pioConfig(g_clkDiv);
  g_initDone = true;

  return true;
fail:
  return false;
}
//------------------------------------------------------------------------------
bool SdioCard::cardCMD6(uint32_t arg, uint8_t* status) {
  if (!cardCmd(CMD6_R1, arg) || !readDat(status, 64)) {
    sdError(SD_CARD_ERROR_CMD6);
    goto fail;
  }
  return true;
fail:
  return false;
}
//------------------------------------------------------------------------------
void SdioCard::end() { pioEnd(); }
//------------------------------------------------------------------------------
bool SdioCard::erase(uint32_t firstSector, uint32_t lastSector) {
  Timeout timeout(SD_ERASE_TIMEOUT);
  if (!syncDevice()) {
    SDIO_FAIL();
    goto fail;
  }
  // check for single sector erase
  if (!g_csd.eraseSingleBlock()) {
    // erase size mask
    uint8_t m = g_csd.eraseSize() - 1;
    if ((firstSector & m) != 0 || ((lastSector + 1) & m) != 0) {
      // error card can't erase specified area
      sdError(SD_CARD_ERROR_ERASE_SINGLE_SECTOR);
      goto fail;
    }
  }
  if (!g_highCapacity) {
    firstSector <<= 9;
    lastSector <<= 9;
  }
  if (!cardCmd(CMD32_R1, firstSector)) {
    sdError(SD_CARD_ERROR_CMD32);
    goto fail;
  }
  if (!cardCmd(CMD33_R1, lastSector)) {
    sdError(SD_CARD_ERROR_CMD33);
    goto fail;
  }
  if (!cardCmd(CMD38_R1, 0)) {
    sdError(SD_CARD_ERROR_CMD38);
    goto fail;
  }
  while (isBusy()) {
    if (timeout.timedOut()) {
      sdError(SD_CARD_ERROR_ERASE_TIMEOUT);
      goto fail;
    }
  }
  return true;
fail:
  return false;
}
//------------------------------------------------------------------------------
uint8_t SdioCard::errorCode() const { return g_errorCode; }
//------------------------------------------------------------------------------
uint32_t SdioCard::errorData() const { return g_cardRsp; }
//------------------------------------------------------------------------------
uint32_t SdioCard::errorLine() const { return g_errorLine; }
//------------------------------------------------------------------------------
bool SdioCard::isBusy() {
  return gpio_get(g_dat0Pin) ? false : !(status() & CARD_STATUS_READY_FOR_DATA);
}
//------------------------------------------------------------------------------
bool SdioCard::readSector(uint32_t sector, uint8_t* dst) {
  if (m_curState != READ_STATE || sector != m_curSector) {
    if (!syncDevice()) {
      SDIO_FAIL();
      goto fail;
    }
    if (!readStart(sector)) {
      sdError(SD_CARD_ERROR_READ_START);
      goto fail;
    }
    m_curSector = sector;
    m_curState = READ_STATE;
  }
  if (!readData(dst)) {
    SDIO_FAIL();
    goto fail;
  }
  m_curSector++;
  return true;

fail:
  return false;
}
//------------------------------------------------------------------------------
bool SdioCard::readSectors(uint32_t sector, uint8_t* dst, size_t ns) {
  for (size_t i = 0; i < ns; i++) {
    if (!readSector(sector + i, dst + i * 512UL)) {
      SDIO_FAIL();
      goto fail;
    }
  }
  return true;

fail:
  return false;
}
//------------------------------------------------------------------------------
bool SdioCard::readCID(cid_t* cid) {
  memcpy(cid, &g_cid, sizeof(cid_t));
  return true;
}
//------------------------------------------------------------------------------
bool SdioCard::readCSD(csd_t* csd) {
  memcpy(csd, &g_csd, sizeof(csd_t));
  return true;
}
//------------------------------------------------------------------------------
bool __time_critical_func(SdioCard::readData)(uint8_t* dst) {
  return readDat(dst, 512);
}
//------------------------------------------------------------------------------
bool SdioCard::readOCR(uint32_t* ocr) {
  *ocr = g_ocr;
  return true;
}
//------------------------------------------------------------------------------
bool SdioCard::readSCR(scr_t* scr) {
  memcpy(scr, &g_scr, sizeof(scr_t));
  return true;
}
//------------------------------------------------------------------------------
bool SdioCard::readSDS(sds_t* sds) {
  memcpy(sds, &g_sds, sizeof(sds_t));
  return true;
}
//------------------------------------------------------------------------------
bool SdioCard::readStart(uint32_t sector) {
  uint arg = g_highCapacity ? sector : 512 * sector;
  if (!cardCmd(CMD18_R1, arg)) {
    sdError(SD_CARD_ERROR_CMD18);
    goto fail;
  }
  return true;

fail:
  return false;
}
//------------------------------------------------------------------------------
bool SdioCard::readStop() { return syncDevice(); }
//------------------------------------------------------------------------------
uint32_t SdioCard::status() {
  return cardCmd(CMD13_R1, g_rca) ? g_cardRsp : CARD_STATUS_ERROR;
}
//------------------------------------------------------------------------------
uint32_t SdioCard::sectorCount() {
  csd_t csd;
  return readCSD(&csd) ? csd.capacity() : 0;
}
//------------------------------------------------------------------------------
bool SdioCard::syncDevice() {
  if (m_curState != IDLE_STATE) {
    Timeout timeout(SD_INIT_TIMEOUT);
    if (!cardCmd(CMD12_R1, 0)) {
      sdError(SD_CARD_ERROR_CMD12);
      goto fail;
    }
    while (isBusy()) {
      if (timeout.timedOut()) {
        sdError(SD_CARD_ERROR_CMD12);
        goto fail;
      }
    }
    m_curState = IDLE_STATE;
  }
  return true;

fail:
  return false;
}
//------------------------------------------------------------------------------
uint8_t SdioCard::type() const {
  return !g_initDone       ? 0
         : !g_version2     ? SD_CARD_TYPE_SD1
         : !g_highCapacity ? SD_CARD_TYPE_SD2
                           : SD_CARD_TYPE_SDHC;
}
//------------------------------------------------------------------------------
bool SdioCard::writeSector(uint32_t sector, const uint8_t* src) {
  return writeSectors(sector, src, 1);
}
//------------------------------------------------------------------------------
bool SdioCard::writeSectors(uint32_t sector, const uint8_t* src, size_t ns) {
  if (m_curState != WRITE_STATE || m_curSector != sector) {
    if (!syncDevice()) {
      sdError(SD_CARD_ERROR_CMD12);
      goto fail;
    }
    if (!writeStart(sector)) {
      sdError(SD_CARD_ERROR_WRITE_START);
      goto fail;
    }
    m_curSector = sector;
    m_curState = WRITE_STATE;
  }
  for (size_t i = 0; i < ns; i++, src += 512) {
    if (!writeData(src)) {
      sdError(SD_CARD_ERROR_WRITE_DATA);
      goto fail;
    }
  }
  m_curSector += ns;
  return true;
fail:
  return false;
}
//------------------------------------------------------------------------------
bool SdioCard::writeData(const uint8_t* src) { return writeDat(src); }
//------------------------------------------------------------------------------
bool SdioCard::writeStart(uint32_t sector) {
  uint arg = g_highCapacity ? sector : 512 * sector;
  if (!cardCmd(CMD25_R1, arg)) {
    sdError(SD_CARD_ERROR_CMD25);
    goto fail;
  }
  return true;
fail:
  return false;
}
//------------------------------------------------------------------------------
bool SdioCard::writeStop() { return syncDevice(); }
#endif  //  ARDUINO_ARCH_RP2040

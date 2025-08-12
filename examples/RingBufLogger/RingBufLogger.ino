// This example uses a ring buffer to log at high sample rates.
//
// SD cards can have a long write latency while flash is being erased or
// programmed.  When an SD card indicates not busy, there is internal buffer
// space for at least one 512 byte transfer.
//
// This example uses a ring buffer to store data until the SD is not busy.
// The maximum sample rate is determined by the time to read the sensor
// and write 512 bytes to the SD.  Also the ring buffer must be large enough.
//
// Note: The RingBuf class can be use to log from a ISR - see the 
// TeensyDmaAdcLogger example.
//
#ifndef DISABLE_FS_H_WARNING
#define DISABLE_FS_H_WARNING  // Disable warning for type File not defined.
#endif                        // DISABLE_FS_H_WARNING
#include "RingBuf.h"
#include "SdFat.h"

// Try to select the best SD card configuration.
#if defined(HAS_TEENSY_SDIO)
// The driver writes to the uSDHC controller's FIFO then returns
// while the controller writes the data to the SD.  The first sector
// puts the controller in write mode and takes about 11 usec on a
// Teensy 4.1. About 5 usec is required to write a sector when the
// controller is in write mode.
// Read ADC0 - about 17 usec on Teensy 4, Teensy 3.6 is faster.
#define LOG_INTERVAL_USEC 50
#define SD_CONFIG SdioConfig(FIFO_SDIO)
#elif defined(HAS_BUILTIN_PIO_SDIO)
// Try 5,000 Samples Per Second - adjust for your setup.
#define LOG_INTERVAL_USEC 200
// See the Rp2040SdioSetup example for boards without a builtin SDIO socket.
#define SD_CONFIG SdioConfig(PIN_SD_CLK, PIN_SD_CMD_MOSI, PIN_SD_DAT0_MISO)
#elif ENABLE_DEDICATED_SPI
// Use Dedicated SPI. Edit settings for your setup.
#define SPI_CLOCK SD_SCK_MHZ(50)
#define SD_CS_PIN SS
#define SD_CONFIG SdSpiConfig(SD_CS_PIN, DEDICATED_SPI, SPI_CLOCK)
// If 1000 usec is too fast, increase LOG_INTERVAL_USEC or edit SdfatCongig.h
// and set USE_SPI_ARRAY_TRANSFER non-zero to improve SPI data rate.
#define LOG_INTERVAL_USEC 1000
#else  // defined(HAS_TEENSY_SDIO)
#error "Shared SPI is not supported"
#endif  //defined(HAS_TEENSY_SDIO)


// Sample rate.
#define SAMPLES_PER_SECOND (1000000 / LOG_INTERVAL_USEC)

// Space to hold 10 byte lines for 1/5 second in 512 byte sectors.
// This is far larger than is needed for most modern SD cards.
#define RING_BUF_SECTORS ((10 * SAMPLES_PER_SECOND) / 5 + 511) / 512

// Size to log 10 byte lines for more than ten minutes.
#define LOG_FILE_SIZE 10 * SAMPLES_PER_SECOND * 600

#define LOG_FILENAME "RingBufLog.csv"

SdFs sd;
FsFile file;

// RingBuf for File type FsFile.
RingBuf<FsFile, 512 * RING_BUF_SECTORS> rb;

void logData() {
  // Initialize the SD.
  if (!sd.begin(SD_CONFIG)) {
    sd.initErrorHalt(&Serial);
  }
  // Open or create file - truncate existing file.
  if (!file.open(LOG_FILENAME, O_RDWR | O_CREAT | O_TRUNC)) {
    Serial.println("open failed\n");
    return;
  }
  // File must be pre-allocated to avoid huge
  // delays searching for free clusters.
  if (!file.preAllocate(LOG_FILE_SIZE)) {
    Serial.println("preAllocate failed\n");
    file.close();
    return;
  }

  // initialize the RingBuf.
  rb.begin(&file);
  Serial.println("Type any character to stop");

  // Max RingBuf used bytes. Useful to understand RingBuf overrun.
  size_t maxUsed = 0;

  // Min spare micros in loop.
  int32_t minSpareMicros = INT32_MAX;

  // Start time.
  uint32_t logTime = micros();
  // Log data until Serial input or file full.
  while (!Serial.available()) {
    // Amount of data in ringBuf.
    size_t n = rb.bytesUsed();
    if ((n + file.curPosition()) > (LOG_FILE_SIZE - 20)) {
      Serial.println("File full - quitting.");
      break;
    }
    if (n > maxUsed) {
      maxUsed = n;
    }
    if (n >= 512 && !file.isBusy()) {
      // Not busy only allows one sector before possible busy wait.
      // Write one sector from RingBuf to file.
      if (512 != rb.writeOut(512)) {
        Serial.println("writeOut failed");
        break;
      }
    }
    // Time for next point.
    logTime += LOG_INTERVAL_USEC;
    int32_t spareMicros = logTime - micros();
    if (spareMicros < minSpareMicros) {
      minSpareMicros = spareMicros;
    }
    if (spareMicros <= 0) {
      Serial.print("Rate too fast ");
      Serial.println(spareMicros);
      break;
    }
    // Wait until time to log data.
    while (micros() < logTime) {
    }

    // Read ADC0
    uint16_t adc = analogRead(A0);
    // Print spareMicros into the RingBuf as test data.
    rb.print(spareMicros);
    rb.write(',');
    // Print adc into RingBuf.
    rb.println(adc);
    if (rb.getWriteError()) {
      // Error caused by too few free bytes in RingBuf.
      Serial.println("WriteError");
      break;
    }
  }
  // Write any RingBuf data to file.
  rb.sync();
  // Remove any pre-allocated space after current file position.
  file.truncate();
  file.rewind();
  // Print first hundred lines of file.
  Serial.println("spareMicros,ADC0");
  for (uint8_t n = 0; n < 100 && file.available();) {
    int c = file.read();
    if (c < 0) {
      break;
    }
    Serial.write(c);
    if (c == '\n') n++;
  }
  Serial.print("\nfileSize: ");
  Serial.println((uint32_t)file.fileSize());
  Serial.print("maxBytesUsed: ");
  Serial.println(maxUsed);
  Serial.print("minSpareMicros: ");
  Serial.println(minSpareMicros);
  file.close();
  sd.end();
}
void clearSerialInput() {
  for (uint32_t m = micros(); micros() - m < 10000;) {
    if (Serial.read() >= 0) {
      m = micros();
    }
  }
}
void setup() {
  Serial.begin(9600);
  while (!Serial) {
  }
  uint32_t m = 0;
  while (!Serial.available()) {
    if (m == 0 || millis() - m > 4000) {
      Serial.println("Type any character to begin");
      m = millis();
    }
  }
  Serial.print("\nSAMPLES_PER_SECOND: ");
  Serial.println(SAMPLES_PER_SECOND);
  Serial.print("RING_BUF_SECTORS: ");
  Serial.println(RING_BUF_SECTORS);
  Serial.print("LOG_FILE_SIZE: ");
  Serial.println(LOG_FILE_SIZE);
}

void loop() {
  clearSerialInput();
  Serial.println("\nType any character to log data");
  while (!Serial.available()) {
  }
  clearSerialInput();
  logData();
}
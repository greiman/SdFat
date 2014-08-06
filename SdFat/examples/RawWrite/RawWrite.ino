/*
 * This sketch illustrates raw write functions in SdFat that
 * can be used for high speed data logging.  These functions
 * are used in the WaveRP library to record audio with the
 * Adafruit Wave Shield using the built-in Arduino ADC.
 *
 * The WaveRP library captures data from the ADC in an ISR
 * that is driven driven by timer one.  Data is collected in
 * two 512 byte buffers and written to the SD card.
 *
 * This sketch simulates logging from a source that produces
 * data at a constant rate of one block every MICROS_PER_BLOCK.
 *
 * If a high quality SanDisk card is used with this sketch
 * no overruns occur and the maximum block write time is
 * under 2000 micros.
 *
 * Note: WaveRP creates a very large file then truncates it
 * to the length that is used for a recording. It only takes
 * a few seconds to erase a 500 MB file since the card only
 * marks the blocks as erased; no data transfer is required.
 */

#include <SdFat.h>
#include <SdFatUtil.h>

// SD chip select pin
const uint8_t chipSelect = SS;

// number of blocks in the contiguous file
const uint32_t BLOCK_COUNT = 10000UL;

// time to produce a block of data
const uint32_t MICROS_PER_BLOCK = 10000;

// file system
SdFat sd;

// test file
SdFile file;

// file extent
uint32_t bgnBlock, endBlock;

// Serial output stream
ArduinoOutStream cout(Serial);
//------------------------------------------------------------------------------
// store error strings in flash to save RAM
#define error(s) sd.errorHalt_P(PSTR(s))
//------------------------------------------------------------------------------
// log of first overruns
#define OVER_DIM 20
struct {
  uint32_t block;
  uint32_t micros;
} over[OVER_DIM];
//------------------------------------------------------------------------------
void setup(void) {
  Serial.begin(9600);
  while (!Serial) {}  // wait for Leonardo
}
//------------------------------------------------------------------------------
void loop(void) {
  while (Serial.read() >= 0) {}
  // pstr stores strings in flash to save RAM
  cout << pstr("Type any character to start\n");
  while (Serial.read() <= 0) {}
  delay(400);  // catch Due reset problem

  cout << pstr("Free RAM: ") << FreeRam() << endl;

  // initialize the SD card at SPI_FULL_SPEED for best performance.
  // try SPI_HALF_SPEED if bus errors occur.
  if (!sd.begin(chipSelect, SPI_FULL_SPEED)) sd.initErrorHalt();

  // delete possible existing file
  sd.remove("RAW.TXT");

  // create a contiguous file
  if (!file.createContiguous(sd.vwd(), "RAW.TXT", 512UL*BLOCK_COUNT)) {
    error("createContiguous failed");
  }
  // get the location of the file's blocks
  if (!file.contiguousRange(&bgnBlock, &endBlock)) {
    error("contiguousRange failed");
  }
  //*********************NOTE**************************************
  // NO SdFile calls are allowed while cache is used for raw writes
  //***************************************************************

  // clear the cache and use it as a 512 byte buffer
  uint8_t* pCache = (uint8_t*)sd.vol()->cacheClear();

  // fill cache with eight lines of 64 bytes each
  memset(pCache, ' ', 512);
  for (uint16_t i = 0; i < 512; i += 64) {
    // put line number at end of line then CR/LF
    pCache[i + 61] = '0' + (i/64);
    pCache[i + 62] = '\r';
    pCache[i + 63] = '\n';
  }

  cout << pstr("Start raw write of ") << file.fileSize() << pstr(" bytes at\n");
  cout << 512000000UL/MICROS_PER_BLOCK << pstr(" bytes per second\n");
  cout << pstr("Please wait ") << (BLOCK_COUNT*MICROS_PER_BLOCK)/1000000UL;
  cout << pstr(" seconds\n");

  // tell card to setup for multiple block write with pre-erase
  if (!sd.card()->writeStart(bgnBlock, BLOCK_COUNT)) {
    error("writeStart failed");
  }
  // init stats
  uint16_t overruns = 0;
  uint32_t maxWriteTime = 0;
  uint32_t t = micros();
  uint32_t tNext = t;

  // write data
  for (uint32_t b = 0; b < BLOCK_COUNT; b++) {
    // write must be done by this time
    tNext += MICROS_PER_BLOCK;

    // put block number at start of first line in block
    uint32_t n = b;
    for (int8_t d = 5; d >= 0; d--){
      pCache[d] = n || d == 5 ? n % 10 + '0' : ' ';
      n /= 10;
    }
    // write a 512 byte block
    uint32_t tw = micros();
    if (!sd.card()->writeData(pCache)) error("writeData failed");
    tw = micros() - tw;

    // check for max write time
    if (tw > maxWriteTime) {
      maxWriteTime = tw;
    }
    // check for overrun
    if (micros() > tNext) {
      if (overruns < OVER_DIM) {
        over[overruns].block = b;
        over[overruns].micros = tw;
      }
      overruns++;
      // advance time to reflect overrun
      tNext = micros();
    }
    else {
      // wait for time to write next block
      while(micros() < tNext);
    }
  }
  // total write time
  t = micros() - t;

  // end multiple block write mode
  if (!sd.card()->writeStop()) error("writeStop failed");

  cout << pstr("Done\n");
  cout << pstr("Elapsed time: ") << setprecision(3)<< 1.e-6*t;
  cout << pstr(" seconds\n");
  cout << pstr("Max write time: ") << maxWriteTime << pstr(" micros\n");
  cout << pstr("Overruns: ") << overruns << endl;
  if (overruns) {
    uint8_t n = overruns > OVER_DIM ? OVER_DIM : overruns;
    cout << pstr("fileBlock,micros") << endl;
    for (uint8_t i = 0; i < n; i++) {
      cout << over[i].block << ',' << over[i].micros << endl;
    }
  }
  // close file for next pass of loop
  file.close();
  Serial.println();
}
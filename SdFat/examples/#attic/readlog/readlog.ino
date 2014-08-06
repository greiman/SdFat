/*
 * Read the logfile created by the eventlog.pde example.
 * Demo of pathnames and working directories
 */
#include <SdFat.h>

// SD chip select pin
const uint8_t chipSelect = SS;

// file system object
SdFat sd;

// define a serial output stream
ArduinoOutStream cout(Serial);
//------------------------------------------------------------------------------
void setup() {
  int c;
  Serial.begin(9600);
  while (!Serial) {}  // wait for Leonardo

  // initialize the SD card at SPI_HALF_SPEED to avoid bus errors with
  // breadboards.  use SPI_FULL_SPEED for better performance.
  if (!sd.begin(chipSelect, SPI_HALF_SPEED)) sd.initErrorHalt();

  // set current working directory
  if (!sd.chdir("LOGS/2011/JAN/")) {
    sd.errorHalt("chdir failed. Did you run eventlog.pde?");
  }
  // open file in current working directory
  ifstream file("LOGFILE.TXT");

  if (!file.is_open()) sd.errorHalt("open failed");

  // copy the file to Serial
  while ((c = file.get()) >= 0) cout << (char)c;

  cout << "Done" << endl;
}
//------------------------------------------------------------------------------
void loop() {}
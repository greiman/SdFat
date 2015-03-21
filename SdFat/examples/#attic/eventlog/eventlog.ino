/*
 * Append a line to a file - demo of pathnames and streams
 */
#include <SPI.h>
#include <SdFat.h>

// SD chip select pin
const uint8_t chipSelect = SS;

// file system object
SdFat sd;

// define a serial output stream
ArduinoOutStream cout(Serial);
//------------------------------------------------------------------------------
/*
 * Append a line to logfile.txt
 */
void logEvent(const char *msg) {
  // create dir if needed
  sd.mkdir("logs/2014/Jan");

  // create or open a file for append
  ofstream sdlog("logs/2014/Jan/logfile.txt", ios::out | ios::app);

  // append a line to the file
  sdlog << msg << endl;

  // check for errors
  if (!sdlog) {
    sd.errorHalt("append failed");
  }

  sdlog.close();
}
//------------------------------------------------------------------------------
void setup() {
  Serial.begin(9600);
  while (!Serial) {}  // wait for Leonardo

  // F() stores strings in flash to save RAM
  cout << F("Type any character to start\n");
  while (Serial.read() <= 0) {}
  delay(400);  // catch Due reset problem

  // initialize the SD card at SPI_HALF_SPEED to avoid bus errors with
  // breadboards.  use SPI_FULL_SPEED for better performance.
  if (!sd.begin(chipSelect, SPI_HALF_SPEED)) {
    sd.initErrorHalt();
  }

  // append a line to the logfile
  logEvent("Another line for the logfile");

  cout << F("Done - check /logs/2014/Jan/logfile.txt on the SD") << endl;
}
//------------------------------------------------------------------------------
void loop() {}

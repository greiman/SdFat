/*
 * Append Example
 *
 * This sketch shows how to use open for append.
 * The sketch will append 100 line each time it opens the file.
 * The sketch will open and close the file 100 times.
 */
#include <SdFat.h>

// SD chip select pin
const uint8_t chipSelect = SS;

// file system object
SdFat sd;

// create Serial stream
ArduinoOutStream cout(Serial);

// store error strings in flash to save RAM
#define error(s) sd.errorHalt_P(PSTR(s))
//------------------------------------------------------------------------------
void setup() {
  // filename for this example
  char name[] = "APPEND.TXT";

  Serial.begin(9600);
  while (!Serial) {}  // wait for Leonardo

  // pstr() stores strings in flash to save RAM
  cout << endl << pstr("Type any character to start\n");
  while (Serial.read() <= 0) {}
  delay(400);  // Catch Due reset problem

  // initialize the SD card at SPI_HALF_SPEED to avoid bus errors with
  // breadboards.  use SPI_FULL_SPEED for better performance.
  if (!sd.begin(chipSelect, SPI_HALF_SPEED)) sd.initErrorHalt();

  cout << pstr("Appending to: ") << name;
  
  for (uint8_t i = 0; i < 100; i++) {
    // open stream for append
    ofstream sdout(name, ios::out | ios::app);
    if (!sdout) error("open failed");

    // append 100 lines to the file
    for (uint8_t j = 0; j < 100; j++) {
      // use int() so byte will print as decimal number
      sdout << "line " << int(j) << " of pass " << int(i);
      sdout << " millis = " << millis() << endl;
    }
    // close the stream
    sdout.close();

    if (!sdout) error("append data failed");

    // output progress indicator
    if (i % 25 == 0) cout << endl;
    cout << '.';
  }
  cout << endl << "Done" << endl;
}
//------------------------------------------------------------------------------
void loop() {}

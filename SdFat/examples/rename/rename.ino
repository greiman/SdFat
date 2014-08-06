/*
 * This sketch demonstrates use of SdFile::rename() 
 * and SdFat::rename().
 */
#include <SdFat.h>

// SD chip select pin
const uint8_t chipSelect = SS;

// file system
SdFat sd;

// Serial print stream
ArduinoOutStream cout(Serial);
//------------------------------------------------------------------------------
// store error strings in flash to save RAM
#define error(s) sd.errorHalt_P(PSTR(s))
//------------------------------------------------------------------------------
void setup() {
  Serial.begin(9600);
  while (!Serial) {}  // wait for Leonardo

  cout << pstr("Insert an empty SD.  Type any character to start.") << endl;
  while (Serial.read() <= 0) {}
  delay(400);  // catch Due reset problem

  // initialize the SD card at SPI_HALF_SPEED to avoid bus errors with
  // breadboards.  use SPI_FULL_SPEED for better performance.
  if (!sd.begin(chipSelect, SPI_HALF_SPEED)) sd.initErrorHalt();

  // create a file and write one line to the file
  SdFile file("NAME1.TXT", O_WRITE | O_CREAT);
  if (!file.isOpen()) error("NAME1");
  file.println("A test line for NAME1.TXT");

  // rename the file NAME2.TXT and add a line.
  // sd.vwd() is the volume working directory, root.
  if (!file.rename(sd.vwd(), "NAME2.TXT")) error("NAME2");
  file.println("A test line for NAME2.TXT");

  // list files
  cout << pstr("------") << endl;
  sd.ls(LS_R);

  // make a new directory - "DIR1"
  if (!sd.mkdir("DIR1")) error("DIR1");

  // move file into DIR1, rename it NAME3.TXT and add a line
  if (!file.rename(sd.vwd(), "DIR1/NAME3.TXT")) error("NAME3");
  file.println("A line for DIR1/NAME3.TXT");

  // list files
  cout << pstr("------") << endl;
  sd.ls(LS_R);

  // make directory "DIR2"
  if (!sd.mkdir("DIR2")) error("DIR2");

  // close file before rename(oldPath, newPath)
  file.close();

  // move DIR1 into DIR2 and rename it DIR3
  if (!sd.rename("DIR1", "DIR2/DIR3")) error("DIR2/DIR3");

  // open file for append in new location and add a line
  if (!file.open("DIR2/DIR3/NAME3.TXT", O_WRITE | O_APPEND)) {
    error("DIR2/DIR3/NAME3.TXT");
  }
  file.println("A line for DIR2/DIR3/NAME3.TXT");

  // list files
  cout << pstr("------") << endl;
  sd.ls(LS_R);

  cout << pstr("Done") << endl;
}
void loop() {}

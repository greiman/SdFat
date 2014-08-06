/*
 * This sketch tests the dateTimeCallback() function
 * and the timestamp() function.
 */
#include <SdFat.h>

SdFat sd;

SdFile file;

// Default SD chip select is SS pin
const uint8_t chipSelect = SS;

// create Serial stream
ArduinoOutStream cout(Serial);
//------------------------------------------------------------------------------
// store error strings in flash to save RAM
#define error(s) sd.errorHalt_P(PSTR(s))
//------------------------------------------------------------------------------
/*
 * date/time values for debug
 * normally supplied by a real-time clock or GPS
 */
// date 1-Oct-09
uint16_t year = 2009;
uint8_t month = 10;
uint8_t day = 1;

// time 20:30:40
uint8_t hour = 20;
uint8_t minute = 30;
uint8_t second = 40;
//------------------------------------------------------------------------------
/*
 * User provided date time callback function.
 * See SdFile::dateTimeCallback() for usage.
 */
void dateTime(uint16_t* date, uint16_t* time) {
  // User gets date and time from GPS or real-time
  // clock in real callback function

  // return date using FAT_DATE macro to format fields
  *date = FAT_DATE(year, month, day);

  // return time using FAT_TIME macro to format fields
  *time = FAT_TIME(hour, minute, second);
}
//------------------------------------------------------------------------------
/*
 * Function to print all timestamps.
 */
void printTimestamps(SdFile& f) {
  dir_t d;
  if (!f.dirEntry(&d)) error("f.dirEntry failed");

  cout << pstr("Creation: ");
  f.printFatDate(d.creationDate);
  cout << ' ';
  f.printFatTime(d.creationTime);
  cout << endl;

  cout << pstr("Modify: ");
  f.printFatDate(d.lastWriteDate);
  cout <<' ';
  f.printFatTime(d.lastWriteTime);
  cout << endl;

  cout << pstr("Access: ");
  f.printFatDate(d.lastAccessDate);
  cout << endl;
}
//------------------------------------------------------------------------------
void setup(void) {
  Serial.begin(9600);
  while (!Serial) {}  // wait for Leonardo

  cout << pstr("Type any character to start\n");
  while (!Serial.available());
  delay(400);  // catch Due reset problem
  
  // initialize the SD card at SPI_HALF_SPEED to avoid bus errors with
  // breadboards.  use SPI_FULL_SPEED for better performance.
  if (!sd.begin(chipSelect, SPI_HALF_SPEED)) sd.initErrorHalt();

  // remove files if they exist
  sd.remove("CALLBACK.TXT");
  sd.remove("DEFAULT.TXT");
  sd.remove("STAMP.TXT");

  // create a new file with default timestamps
  if (!file.open("DEFAULT.TXT", O_CREAT | O_WRITE)) {
    error("open DEFAULT.TXT failed");
  }
  cout << pstr("\nOpen with default times\n");
  printTimestamps(file);

  // close file
  file.close();
  /*
   * Test the date time callback function.
   *
   * dateTimeCallback() sets the function
   * that is called when a file is created
   * or when a file's directory entry is
   * modified by sync().
   *
   * The callback can be disabled by the call
   * SdFile::dateTimeCallbackCancel()
   */
  // set date time callback function
  SdFile::dateTimeCallback(dateTime);

  // create a new file with callback timestamps
  if (!file.open("CALLBACK.TXT", O_CREAT | O_WRITE)) {
    error("open CALLBACK.TXT failed");
  }
  cout << ("\nOpen with callback times\n");
  printTimestamps(file);

  // change call back date
  day += 1;

  // must add two to see change since FAT second field is 5-bits
  second += 2;

  // modify file by writing a byte
  file.write('t');

  // force dir update
  file.sync();

  cout << pstr("\nTimes after write\n");
  printTimestamps(file);

  // close file
  file.close();
  /*
   * Test timestamp() function
   *
   * Cancel callback so sync will not
   * change access/modify timestamp
   */
  SdFile::dateTimeCallbackCancel();

  // create a new file with default timestamps
  if (!file.open("STAMP.TXT", O_CREAT | O_WRITE)) {
    error("open STAMP.TXT failed");
  }
  // set creation date time
  if (!file.timestamp(T_CREATE, 2009, 11, 10, 1, 2, 3)) {
    error("set create time failed");
  }
  // set write/modification date time
  if (!file.timestamp(T_WRITE, 2009, 11, 11, 4, 5, 6)) {
    error("set write time failed");
  }
  // set access date
  if (!file.timestamp(T_ACCESS, 2009, 11, 12, 7, 8, 9)) {
    error("set access time failed");
  }
  cout << pstr("\nTimes after timestamp() calls\n");
  printTimestamps(file);

  file.close();
  cout << pstr("\nDone\n");
}

void loop(void){}
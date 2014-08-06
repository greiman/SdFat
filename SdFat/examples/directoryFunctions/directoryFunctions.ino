/*
 * Example use of chdir(), ls(), mkdir(), and  rmdir().
 */
#include <SdFat.h>
// SD card chip select pin.
const uint8_t SD_CHIP_SELECT = SS;
//------------------------------------------------------------------------------
// Permit SD to be wiped if ALLOW_WIPE is true.
const bool ALLOW_WIPE = false;

// File system object.
SdFat sd;

// Use for file creation in folders.
SdFile file;

// Create a Serial output stream.
ArduinoOutStream cout(Serial);

// Buffer for Serial input.
char cinBuf[40];

// Create a serial input stream.
ArduinoInStream cin(Serial, cinBuf, sizeof(cinBuf));
//==============================================================================
// Error messages stored in flash.
#define error(msg) error_P(PSTR(msg))
//------------------------------------------------------------------------------
void error_P(const char* msg) {
  sd.errorHalt_P(msg);
}
//------------------------------------------------------------------------------
void setup() {
  Serial.begin(9600);
  while (!Serial) {} // wait for Leonardo
  delay(1000);
  
  cout << pstr("Type any character to start\n");
  // Wait for input line and discard.
  cin.readline();

  // Initialize the SD card at SPI_HALF_SPEED to avoid bus errors with
  // breadboards.  use SPI_FULL_SPEED for better performance.
  if (!sd.begin(SD_CHIP_SELECT, SPI_HALF_SPEED)) sd.initErrorHalt();
  
  // Check for empty SD.
  if (file.openNext(sd.vwd(), O_READ)) {
    cout << pstr("Found files/folders in the root directory.\n");    
    if (!ALLOW_WIPE) {
      error("SD not empty, use a blank SD or set ALLOW_WIPE true.");  
    } else {
      cout << pstr("Type: 'WIPE' to delete all SD files.\n");
      char buf[10];
      cin.readline();
      cin.get(buf, sizeof(buf));
      if (cin.fail() || strncmp(buf, "WIPE", 4) || buf[4] >= ' ') {
        error("Invalid WIPE input");
      }
      file.close();
      sd.vwd()->rmRfStar();
      cout << pstr("***SD wiped clean.***\n\n");
    }
  }
  
  // Create a new folder.
  if (!sd.mkdir("FOLDER1")) error("Create FOLDER1 failed");
  cout << pstr("Created FOLDER1\n");
  
  // Create a file in FOLDER1 using a path.
  if (!file.open("FOLDER1/FILE1.TXT", O_CREAT | O_WRITE)) {
    error("create FOLDER1/FILE1.TXT failed");
  }
  file.close();
  cout << pstr("Created FOLDER1/FILE1.TXT\n");
  
  // Change volume working directory to FOLDER1.
  if (!sd.chdir("FOLDER1")) error("chdir failed for FOLDER1.\n");
  cout << pstr("chdir to FOLDER1\n");
  
  // Create FILE2.TXT in current directory.
  if (!file.open("FILE2.TXT", O_CREAT | O_WRITE)) {
    error("create FILE2.TXT failed");
  }
  file.close();
  cout << pstr("Created FILE2.TXT in current directory\n");
  
  cout << pstr("List of files on the SD.\n");
  sd.ls("/", LS_R);

  // Remove files from current directory.
  if (!sd.remove("FILE1.TXT") || !sd.remove("FILE2.TXT")) error("remove failed");
  cout << pstr("\nFILE1.TXT and FILE2.TXT removed.\n");

  // Change current directory to root.
  if (!sd.chdir()) error("chdir to root failed.\n");
  
  cout << pstr("List of files on the SD.\n");
  sd.ls(LS_R);
  
  // Remove FOLDER1.
  if (!sd.rmdir("FOLDER1")) error("rmdir for FOLDER1 failed\n");
  
  cout << pstr("\nFOLDER1 removed, SD empty.\n");
  cout << pstr("Done!\n");
}
//------------------------------------------------------------------------------
// Nothing happens in loop.
void loop() {}
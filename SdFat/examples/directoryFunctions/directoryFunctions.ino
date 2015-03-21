/*
 * Example use of chdir(), ls(), mkdir(), and  rmdir().
 */
#include <SPI.h>
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
#define error(msg) sd.errorHalt(F(msg))
//------------------------------------------------------------------------------
void setup() {
  Serial.begin(9600);
  while (!Serial) {} // wait for Leonardo
  delay(1000);

  cout << F("Type any character to start\n");
  // Wait for input line and discard.
  cin.readline();

  // Initialize the SD card at SPI_HALF_SPEED to avoid bus errors with
  // breadboards.  use SPI_FULL_SPEED for better performance.
  if (!sd.begin(SD_CHIP_SELECT, SPI_HALF_SPEED)) {
    sd.initErrorHalt();
  }

  // Check for empty SD.
  if (file.openNext(sd.vwd(), O_READ)) {
    cout << F("Found files/folders in the root directory.\n");
    if (!ALLOW_WIPE) {
      error("SD not empty, use a blank SD or set ALLOW_WIPE true.");
    } else {
      cout << F("Type: 'WIPE' to delete all SD files.\n");
      char buf[10];
      cin.readline();
      cin.get(buf, sizeof(buf));
      if (cin.fail() || strncmp(buf, "WIPE", 4) || buf[4] >= ' ') {
        error("Invalid WIPE input");
      }
      file.close();
      if (!sd.vwd()->rmRfStar()) {
        error("wipe failed");
      }
      cout << F("***SD wiped clean.***\n\n");
    }
  }

  // Create a new folder.
  if (!sd.mkdir("Folder1")) {
    error("Create Folder1 failed");
  }
  cout << F("Created Folder1\n");

  // Create a file in Folder1 using a path.
  if (!file.open("Folder1/file1.txt", O_CREAT | O_WRITE)) {
    error("create Folder1/file1.txt failed");
  }
  file.close();
  cout << F("Created Folder1/file1.txt\n");

  // Change volume working directory to Folder1.
  if (!sd.chdir("Folder1")) {
    error("chdir failed for Folder1.\n");
  }
  cout << F("chdir to Folder1\n");

  // Create File2.txt in current directory.
  if (!file.open("File2.txt", O_CREAT | O_WRITE)) {
    error("create File2.txt failed");
  }
  file.close();
  cout << F("Created File2.txt in current directory\n");

  cout << F("List of files on the SD.\n");
  sd.ls("/", LS_R);

  // Remove files from current directory.
  if (!sd.remove("file1.txt") || !sd.remove("File2.txt")) {
    error("remove failed");
  }
  cout << F("\nfile1.txt and File2.txt removed.\n");

  // Change current directory to root.
  if (!sd.chdir()) {
    error("chdir to root failed.\n");
  }

  cout << F("List of files on the SD.\n");
  sd.ls(LS_R);

  // Remove Folder1.
  if (!sd.rmdir("Folder1")) {
    error("rmdir for Folder1 failed\n");
  }

  cout << F("\nFolder1 removed, SD empty.\n");
  cout << F("Done!\n");
}
//------------------------------------------------------------------------------
// Nothing happens in loop.
void loop() {}
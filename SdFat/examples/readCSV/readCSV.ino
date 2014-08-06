/*
 *  This example reads a simple CSV, comma-separated values, file.
 *  Each line of the file has three values, a long and two floats.
 */
#include <SdFat.h>

// SD chip select pin
const uint8_t chipSelect = SS;

// file system object
SdFat sd;

// create Serial stream
ArduinoOutStream cout(Serial);

char fileName[] = "TESTFILE.CSV";
//------------------------------------------------------------------------------
// store error strings in flash to save RAM
#define error(s) sd.errorHalt_P(PSTR(s))
//------------------------------------------------------------------------------
// read and print CSV test file
void readFile() {
  long lg;
  float f1, f2;
  char text[10];
  char c1, c2, c3;  // space for commas.
  
  // open input file
  ifstream sdin(fileName);
  
  // check for open error
  if (!sdin.is_open()) error("open");
  
  // read until input fails
  while (1) {
    // Get text field.
    sdin.get(text, sizeof(text), ',');
    
    // Assume EOF if fail.
    if (sdin.fail()) break;
    
    // Get commas and numbers.
    sdin >> c1 >> lg >> c2 >> f1 >> c3 >> f2;
    
    // Skip CR/LF.
    sdin.skipWhite();
    
    if (sdin.fail()) error("bad input");
    
    // error in line if not commas
    if (c1 != ',' || c2 != ',' || c3 != ',') error("comma");
    
    // print in six character wide columns
    cout << text << setw(6) << lg << setw(6) << f1 << setw(6) << f2 << endl;
  }
  // Error in an input line if file is not at EOF.
  if (!sdin.eof()) error("readFile");
}
//------------------------------------------------------------------------------
// write test file
void writeFile() {

  // create or open and truncate output file
  ofstream sdout(fileName);
  
  // write file from string stored in flash
  sdout << pstr(
    "Line 1,1,2.3,4.5\n"
    "Line 2,6,7.8,9.0\n"
    "Line 3,9,8.7,6.5\n"
    "Line 4,-4,-3.2,-1\n") << flush;

  // check for any errors
  if (!sdout) error("writeFile");
  
  sdout.close();
}
//------------------------------------------------------------------------------
void setup() {
  Serial.begin(9600);
  while (!Serial) {} // wait for Leonardo
  cout << pstr("Type any character to start\n");
  while (Serial.read() <= 0) {}
  delay(400);  // catch Due reset problem
  
  // initialize the SD card at SPI_HALF_SPEED to avoid bus errors with
  // breadboards.  use SPI_FULL_SPEED for better performance
  if (!sd.begin(chipSelect, SPI_HALF_SPEED)) sd.initErrorHalt();
  
  // create test file
  writeFile();
  
  cout << endl;

  // read and print test
  readFile();  
  
  cout << "\nDone!" << endl;
}
void loop() {}
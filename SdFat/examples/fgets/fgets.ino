// Demo of fgets function to read lines from a file.
#include <SdFat.h>

// SD chip select pin
const uint8_t chipSelect = SS;

SdFat sd;
// print stream
ArduinoOutStream cout(Serial);
//------------------------------------------------------------------------------
// store error strings in flash memory
#define error(s) sd.errorHalt_P(PSTR(s))
//------------------------------------------------------------------------------
void demoFgets() {
  char line[25];
  int n;
  // open test file
  SdFile rdfile("FGETS.TXT", O_READ);
  
  // check for open error
  if (!rdfile.isOpen()) error("demoFgets");
  
  cout << endl << pstr(
    "Lines with '>' end with a '\\n' character\n"
    "Lines with '#' do not end with a '\\n' character\n"
    "\n");
    
  // read lines from the file
  while ((n = rdfile.fgets(line, sizeof(line))) > 0) {
    if (line[n - 1] == '\n') {
      cout << '>' << line;
    } else {
      cout << '#' << line << endl;
    }
  }
}
//------------------------------------------------------------------------------
void makeTestFile() {
  // create or open test file
  SdFile wrfile("FGETS.TXT", O_WRITE | O_CREAT | O_TRUNC);
  
  // check for open error
  if (!wrfile.isOpen()) error("MakeTestFile");
  
  // write test file
  wrfile.write_P(PSTR(
    "Line with CRLF\r\n"
    "Line with only LF\n"
    "Long line that will require an extra read\n"
    "\n"  // empty line
    "Line at EOF without NL"
  ));
  wrfile.close();
}
//------------------------------------------------------------------------------
void setup(void) {
  Serial.begin(9600);
  while (!Serial) {}  // Wait for Leonardo

  cout << pstr("Type any character to start\n");
  while (Serial.read() <= 0) {}
  delay(400);  // catch Due reset problem
  
  // initialize the SD card at SPI_HALF_SPEED to avoid bus errors with
  // breadboards.  use SPI_FULL_SPEED for better performance.
  if (!sd.begin(chipSelect, SPI_HALF_SPEED)) sd.initErrorHalt();
  
  makeTestFile();
  
  demoFgets();
  
  cout << pstr("\nDone\n");
}
void loop(void) {}

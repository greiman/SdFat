// Demo of rewriting a line read by fgets
#include <SdFat.h>

// SD card chip select pin
const uint8_t chipSelect = SS;

// file system
SdFat sd;

// print stream
ArduinoOutStream cout(Serial);
//------------------------------------------------------------------------------
// store error strings in flash memory
#define error(s) sd.errorHalt_P(PSTR(s))
//------------------------------------------------------------------------------
void demoFgets() {
  char line[25];
  int c;
  uint32_t pos;
  
  // open test file
  SdFile rdfile("FGETS.TXT", O_RDWR);
  
  // check for open error
  if (!rdfile.isOpen()) error("demoFgets");
  
  // list file
  cout << pstr("-----Before Rewrite\r\n");
  while ((c = rdfile.read()) >= 0) Serial.write(c); 
  
  rdfile.rewind();  
  // read lines from the file to get position
  while (1) {
    pos = rdfile.curPosition();
    if (rdfile.fgets(line, sizeof(line)) < 0) {
      error("Line not found");
    }
    // find line that contains "Line C"
    if (strstr(line, "Line C"))break;
  }
  
  // rewrite line with 'C'  
  if (!rdfile.seekSet(pos))error("seekSet");
  rdfile.println("Line R");
  rdfile.rewind();
  
  // list file
  cout << pstr("\r\n-----After Rewrite\r\n");
  while ((c = rdfile.read()) >= 0) Serial.write(c);
  
  // close so rewrite is not lost
  rdfile.close();
}
//------------------------------------------------------------------------------
void makeTestFile() {
  // create or open test file
  SdFile wrfile("FGETS.TXT", O_WRITE | O_CREAT | O_TRUNC);
  
  // check for open error
  if (!wrfile.isOpen()) error("MakeTestFile");
  
  // write test file
  wrfile.write_P(PSTR(
    "Line A\r\n"
    "Line B\r\n"
    "Line C\r\n"
    "Line D\r\n"
    "Line E\r\n"
  ));
  wrfile.close();
}
//------------------------------------------------------------------------------
void setup(void) {
  Serial.begin(9600);
  while (!Serial){}  // wait for Leonardo

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
